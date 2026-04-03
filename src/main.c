#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <limits.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "../include/actions.h"
#include "../include/game.h"

#define CELL_SIZE 6
#define WINDOW_SIZE (BOARD_SIZE * CELL_SIZE)

// Forward declarations
int parse_text_file(const char *filename, Player *player);
char get_text_action(Player *player);

void init_game(Game *game, int argc, char *argv[])
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            game->board[i][j] = 0;
        }
    }
    game->num_players = argc - 1;
    if (game->num_players > MAX_PLAYERS)
        game->num_players = MAX_PLAYERS;

    int start_x[] = {25, 75, 25, 75};
    int start_y[] = {25, 25, 75, 75};

    // Initialiser les bombes et clones
    game->num_bombs = 0;
    game->num_clones = 0;
    game->current_turn = 0;

    for (int i = 0; i < game->num_players; i++)
    {
        game->players[i].id = i + 1;
        game->players[i].credits = INITIAL_CREDITS;
        game->players[i].is_active = 1;
        game->players[i].x = start_x[i];
        game->players[i].y = start_y[i];

        // Initialiser les effets
        game->players[i].mute_turns = 0;
        game->players[i].mute_caster_id = 0;
        game->players[i].swap_turns = 0;
        game->players[i].swap_caster_id = 0;
        game->players[i].fork_turns = 0;
        game->players[i].fork_clone_id = 0;

        // Déterminer si c'est un joueur texte ou binaire
        const char *filename = argv[i + 1];
        int is_txt = strlen(filename) > 4 && strcmp(filename + strlen(filename) - 4, ".txt") == 0;
        
        if (is_txt)
        {
            // Charger le fichier texte
            if (!parse_text_file(filename, &game->players[i]))
            {
                fprintf(stderr, "Erreur chargement joueur texte %d : %s\n", i + 1, filename);
                exit(1);
            }
            game->players[i].handle = NULL;
            game->players[i].get_action = NULL;
        }
        else
        {
            // Charger la librairie dynamique
            game->players[i].is_text_player = 0;
            game->players[i].handle = dlopen(filename, RTLD_LAZY);
            if (!game->players[i].handle)
            {
                fprintf(stderr, "Erreur chargement joueur %d : %s\n", i + 1, dlerror());
                exit(1);
            }
            game->players[i].get_action = dlsym(game->players[i].handle, "get_action");
        }
        game->board[game->players[i].x][game->players[i].y] = game->players[i].id;
    }
}

void cleanup_game(Game *game)
{
    for (int i = 0; i < game->num_players; i++)
    {
        if (game->players[i].handle)
            dlclose(game->players[i].handle);
    }
}

int wrap_coord(int coord)
{
    if (coord < 0)
        return BOARD_SIZE - 1;
    if (coord >= BOARD_SIZE)
        return 0;
    return coord;
}

// Fonction pour placer une bombe
void place_bomb(Game *game, int x, int y, int owner_id)
{
    if (game->num_bombs < MAX_BOMBS)
    {
        game->bombs[game->num_bombs].x = x;
        game->bombs[game->num_bombs].y = y;
        game->bombs[game->num_bombs].owner_id = owner_id;
        game->bombs[game->num_bombs].creation_turn = game->current_turn;
        game->bombs[game->num_bombs].is_active = 1;
        game->num_bombs++;
    }
}

// Fonction pour déclencher les bombes
void trigger_bombs(Game *game)
{
    for (int i = 0; i < game->num_bombs; i++)
    {
        if (!game->bombs[i].is_active)
            continue;
        
        // Vérifier si la bombe doit exploser (après 5 tours)
        if (game->current_turn - game->bombs[i].creation_turn >= 5)
        {
            int bx = game->bombs[i].x;
            int by = game->bombs[i].y;
            int owner_id = game->bombs[i].owner_id;
            
            // Marquer 9 cases autour de la bombe (carré 3x3 centré)
            for (int dx = -1; dx <= 1; dx++)
            {
                for (int dy = -1; dy <= 1; dy++)
                {
                    int nx = wrap_coord(bx + dx);
                    int ny = wrap_coord(by + dy);
                    game->board[nx][ny] = owner_id;
                }
            }
            
            // Désactiver la bombe
            game->bombs[i].is_active = 0;
        }
    }
    
    // Supprimer les bombes inactives
    int write_pos = 0;
    for (int i = 0; i < game->num_bombs; i++)
    {
        if (game->bombs[i].is_active)
        {
            game->bombs[write_pos] = game->bombs[i];
            write_pos++;
        }
    }
    game->num_bombs = write_pos;
}

// Fonction pour nettoyer un carré 7x7
void clean_area(Game *game, int x, int y)
{
    // Carré 7x7 centré sur (x, y)
    for (int dx = -3; dx <= 3; dx++)
    {
        for (int dy = -3; dy <= 3; dy++)
        {
            int nx = wrap_coord(x + dx);
            int ny = wrap_coord(y + dy);
            game->board[nx][ny] = 0;  // Remettre au noir
        }
    }
}

// Fonction pour trouver l'ennemi le plus proche
int find_nearest_enemy(Player *p, Game *game)
{
    int nearest_id = -1;
    int min_distance = INT_MAX;
    
    for (int i = 0; i < game->num_players; i++)
    {
        if (game->players[i].id == p->id || game->players[i].credits <= 0)
            continue;  // Ignorer le joueur lui-même et les inactifs
        
        // Calculer la distance euclidienne
        int dx = game->players[i].x - p->x;
        int dy = game->players[i].y - p->y;
        
        // Distance en tenant compte du wrapping (Pacman)
        int dx_wrapped = dx;
        int dy_wrapped = dy;
        
        if (dx < -BOARD_SIZE/2)
            dx_wrapped = dx + BOARD_SIZE;
        else if (dx > BOARD_SIZE/2)
            dx_wrapped = dx - BOARD_SIZE;
            
        if (dy < -BOARD_SIZE/2)
            dy_wrapped = dy + BOARD_SIZE;
        else if (dy > BOARD_SIZE/2)
            dy_wrapped = dy - BOARD_SIZE;
        
        int distance = dx_wrapped * dx_wrapped + dy_wrapped * dy_wrapped;
        
        if (distance < min_distance)
        {
            min_distance = distance;
            nearest_id = game->players[i].id;
        }
    }
    
    return nearest_id;
}

// Fonction pour parser un fichier texte d'actions
int parse_text_file(const char *filename, Player *player)
{
    FILE *file = fopen(filename, "r");
    if (!file)
        return 0;  // Fichier non trouvable
    
    char buffer[8192];
    if (fgets(buffer, sizeof(buffer), file) == NULL)
    {
        fclose(file);
        return 0;
    }
    fclose(file);
    
    // Copier le buffer dans player->text_actions
    strncpy(player->text_actions, buffer, MAX_ACTIONS - 1);
    player->text_actions[MAX_ACTIONS - 1] = '\0';
    
    // Compter les actions (virgules + 1)
    player->text_action_count = 1;
    for (int i = 0; player->text_actions[i]; i++)
    {
        if (player->text_actions[i] == ',')
            player->text_action_count++;
    }
    
    player->text_action_index = 0;
    player->is_text_player = 1;
    return 1;
}

// Fonction pour récupérer l'action suivante d'un joueur texte
char get_text_action(Player *player)
{
    if (player->text_action_count == 0)
        return ACTION_STILL;
    
    // Trouver le début de l'action actuelle basée sur l'index
    int action_start = 0;
    int current_action = 0;
    
    for (int i = 0; player->text_actions[i] && current_action < player->text_action_index; i++)
    {
        if (player->text_actions[i] == ',')
        {
            action_start = i + 1;
            current_action++;
        }
    }
    
    // Parser le nom de l'action à partir de action_start
    char action_name[50] = {0};
    int j = 0;
    for (int i = action_start; player->text_actions[i] && j < 49; i++)
    {
        if (player->text_actions[i] == ',')
            break;
        if (player->text_actions[i] != ' ' && player->text_actions[i] != '\n' && player->text_actions[i] != '\r')
            action_name[j++] = player->text_actions[i];
    }
    action_name[j] = '\0';
    
    // Convertir le nom en enum
    char action = ACTION_STILL;
    if (strcmp(action_name, "ACTION_MOVE_L") == 0) action = ACTION_MOVE_L;
    else if (strcmp(action_name, "ACTION_MOVE_R") == 0) action = ACTION_MOVE_R;
    else if (strcmp(action_name, "ACTION_MOVE_U") == 0) action = ACTION_MOVE_U;
    else if (strcmp(action_name, "ACTION_MOVE_D") == 0) action = ACTION_MOVE_D;
    else if (strcmp(action_name, "ACTION_DASH_L") == 0) action = ACTION_DASH_L;
    else if (strcmp(action_name, "ACTION_DASH_R") == 0) action = ACTION_DASH_R;
    else if (strcmp(action_name, "ACTION_DASH_U") == 0) action = ACTION_DASH_U;
    else if (strcmp(action_name, "ACTION_DASH_D") == 0) action = ACTION_DASH_D;
    else if (strcmp(action_name, "ACTION_TELEPORT_L") == 0) action = ACTION_TELEPORT_L;
    else if (strcmp(action_name, "ACTION_TELEPORT_R") == 0) action = ACTION_TELEPORT_R;
    else if (strcmp(action_name, "ACTION_TELEPORT_U") == 0) action = ACTION_TELEPORT_U;
    else if (strcmp(action_name, "ACTION_TELEPORT_D") == 0) action = ACTION_TELEPORT_D;
    else if (strcmp(action_name, "ACTION_BOMB") == 0) action = ACTION_BOMB;
    else if (strcmp(action_name, "ACTION_FORK") == 0) action = ACTION_FORK;
    else if (strcmp(action_name, "ACTION_CLEAN") == 0) action = ACTION_CLEAN;
    else if (strcmp(action_name, "ACTION_MUTE") == 0) action = ACTION_MUTE;
    else if (strcmp(action_name, "ACTION_SWAP") == 0) action = ACTION_SWAP;
    else if (strcmp(action_name, "ACTION_STILL") == 0) action = ACTION_STILL;
    
    // Incrémenter l'index et boucler
    player->text_action_index++;
    if (player->text_action_index >= player->text_action_count)
        player->text_action_index = 0;
    
    return action;
}

// Fonction pour mettre à jour les effets MUTE et SWAP
void update_effects(Game *game)
{
    for (int i = 0; i < game->num_players; i++)
    {
        Player *p = &game->players[i];
        
        // Décrémenter les compteurs d'effet
        if (p->mute_turns > 0)
            p->mute_turns--;
        
        if (p->swap_turns > 0)
            p->swap_turns--;
        
        if (p->fork_turns > 0)
            p->fork_turns--;
    }
}

// Fonction pour créer un clone (après 5 tours du fork)
void create_clone(Game *game, int parent_id, int position)
{
    if (game->num_clones < MAX_CLONES)
    {
        Clone *clone = &game->clones[game->num_clones];
        clone->parent_id = parent_id;
        clone->creation_turn = game->current_turn;
        clone->x = game->players[parent_id - 1].x;
        clone->y = game->players[parent_id - 1].y;
        clone->is_active = 1;
        clone->action_index = position;
        clone->cost_multiplier = 1;  // Pas d'effet FORK sur le clone lui-même
        game->num_clones++;
    }
}

// Fonction pour gérer les clones
void process_clones(Game *game)
{
    int write_pos = 0;
    for (int i = 0; i < game->num_clones; i++)
    {
        Clone *clone = &game->clones[i];
        
        if (!clone->is_active)
            continue;
        
        // Vérifier si le clone doit être supprimé (20 tours)
        if (game->current_turn - clone->creation_turn >= 20)
        {
            clone->is_active = 0;
            continue;
        }
        
        // Garder le clone actif
        game->clones[write_pos] = game->clones[i];
        write_pos++;
    }
    game->num_clones = write_pos;
}

int play_round(Game *game)
{
    game->current_turn++;
    
    int active_players = 0;
    for (int i = 0; i < game->num_players; i++)
    {
        Player *p = &game->players[i];
        if (p->credits <= 0)
            continue;
        active_players++;

        char action;
        if (p->is_text_player)
            action = get_text_action(p);
        else
            action = p->get_action();
        int nx = p->x, ny = p->y, cost = 0;

        switch (action)
        {
        case ACTION_MOVE_L:
            nx--;
            cost = 1;
            break;
        case ACTION_MOVE_R:
            nx++;
            cost = 1;
            break;
        case ACTION_MOVE_U:
            ny--;
            cost = 1;
            break;
        case ACTION_MOVE_D:
            ny++;
            cost = 1;
            break;
        case ACTION_DASH_L:
            nx -= 8;
            cost = 10;
            break;
        case ACTION_DASH_R:
            nx += 8;
            cost = 10;
            break;
        case ACTION_DASH_U:
            ny -= 8;
            cost = 10;
            break;
        case ACTION_DASH_D:
            ny += 8;
            cost = 10;
            break;
        case ACTION_TELEPORT_L:
            nx -= 8;
            cost = 2;
            break;
        case ACTION_TELEPORT_R:
            nx += 8;
            cost = 2;
            break;
        case ACTION_TELEPORT_U:
            ny -= 8;
            cost = 2;
            break;
        case ACTION_TELEPORT_D:
            ny += 8;
            cost = 2;
            break;
        case ACTION_BOMB:
            cost = 9;
            break;
        case ACTION_FORK:
            // Coût du fork : 20 crédits (pas affecté par le fork lui-même)
            cost = 20;
            break;
        case ACTION_CLEAN:
            cost = 40;
            break;
        case ACTION_MUTE:
            cost = 30;
            break;
        case ACTION_SWAP:
            cost = 35;
            break;
        case ACTION_STILL:
            cost = 1;
            break;
        default:
            cost = 1;
            break;
        }

        // Vérifier si le joueur a un fork actif (doubler le coût)
        int cost_multiplier = 1;
        if (p->fork_turns > 0)
            cost_multiplier = 2;
        
        int actual_cost = cost * cost_multiplier;

        // Vérifier si le joueur a assez de crédits
        if (p->credits < actual_cost)
        {
            p->credits = 0;
            continue;
        }

        p->credits -= actual_cost;

        // Gérer les actions spéciales
        if (action == ACTION_BOMB)
        {
            place_bomb(game, p->x, p->y, p->id);
        }
        else if (action == ACTION_CLEAN)
        {
            clean_area(game, p->x, p->y);
        }
        else if (action == ACTION_MUTE)
        {
            int target_id = find_nearest_enemy(p, game);
            if (target_id > 0)
            {
                Player *target = &game->players[target_id - 1];
                // Annuler tous les effets précédents et appliquer MUTE
                target->swap_turns = 0;
                target->fork_turns = 0;
                target->mute_turns = 10;
                target->mute_caster_id = p->id;
            }
        }
        else if (action == ACTION_SWAP)
        {
            int target_id = find_nearest_enemy(p, game);
            if (target_id > 0)
            {
                Player *target = &game->players[target_id - 1];
                // Annuler tous les effets précédents et appliquer SWAP
                target->mute_turns = 0;
                target->fork_turns = 0;
                target->swap_turns = 5;
                target->swap_caster_id = p->id;
            }
        }
        else if (action == ACTION_FORK)
        {
            // Activer le fork pour 20 tours
            p->fork_turns = 20;
            // Le clone sera créé 5 tours après
            p->fork_clone_id = -1;  // À remplir quand le clone est créé
        }
        else if (action != ACTION_STILL)
        {
            // Actions de mouvement
            p->x = wrap_coord(nx);
            p->y = wrap_coord(ny);
            
            // Marquer la case selon les effets
            if (p->mute_turns > 0)
            {
                // MUTE : colorie en noir (0)
                game->board[p->x][p->y] = 0;
            }
            else if (p->swap_turns > 0)
            {
                // SWAP : colorie avec la couleur du caster
                game->board[p->x][p->y] = p->swap_caster_id;
            }
            else
            {
                // Normal : colorie avec sa propre couleur
                game->board[p->x][p->y] = p->id;
            }
        }
        else if (action == ACTION_STILL)
        {
            // Pas d'action
        }
    }

    // Créer les clones (5 tours après le fork)
    for (int i = 0; i < game->num_players; i++)
    {
        Player *p = &game->players[i];
        if (p->fork_turns == 15)  // 20 - 5 = 15 tours restants quand créer le clone
        {
            create_clone(game, p->id, 0);
        }
    }

    // Traiter les explosions de bombes après tous les mouvements
    trigger_bombs(game);
    
    // Mettre à jour les effets (décrémenter les compteurs)
    update_effects(game);
    
    // Traiter les clones (les supprimer après 20 tours)
    process_clones(game);

    return active_players;
}

//------ Fonction Affichage des résultats --------- 
void print_results(Game *game, SDL_Window *window)
{
    int scores[MAX_PLAYERS] = {0};
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            int owner = game->board[i][j];
            if (owner > 0 && owner <= game->num_players)
                scores[owner - 1]++;
        }
    }

    printf("\n=== RESULTATS FINAUX ===\n");
    int max_score = -1, winner_id = -1;
    for (int i = 0; i < game->num_players; i++)
    {
        printf("Joueur %d a rempli %d cases.\n", i + 1, scores[i]);
        if (scores[i] > max_score)
        {
            max_score = scores[i];
            winner_id = i + 1;
        }
        else if (scores[i] == max_score)
        {
            winner_id = 0;
        }
    }

    // Préparation du texte pour le pop-up
    char popup_msg[256];
    if (winner_id > 0)
    {
        printf(">>> GAGNANT: JOUEUR %d (%d CASES) <<<\n", winner_id, max_score);
        sprintf(popup_msg, "Le grand gagnant est le Joueur %d avec %d cases remplies !", winner_id, max_score);
    }
    else
    {
        printf(">>> EGALITE (%d CASES) <<<\n", max_score);
        sprintf(popup_msg, "Egalite parfaite avec %d cases !", max_score);
    }

    // Affichage du Pop-up SDL par-dessus la fenêtre
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Fin de la partie Splashmem", popup_msg, window);
}

// --- FONCTION D'AFFICHAGE ---
void draw_game(SDL_Renderer *renderer, Game *game)
{

    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            int owner = game->board[i][j];
            if (owner > 0)
            {
                switch (owner)
                {
                case 1:
                    SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
                    break;
                case 2:
                    SDL_SetRenderDrawColor(renderer, 50, 150, 255, 255);
                    break;
                case 3:
                    SDL_SetRenderDrawColor(renderer, 50, 255, 50, 255);
                    break;
                case 4:
                    SDL_SetRenderDrawColor(renderer, 255, 255, 50, 255);
                    break;
                }
                SDL_Rect rect = {i * CELL_SIZE, j * CELL_SIZE, CELL_SIZE, CELL_SIZE};
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s p1.so [p2.so] ...\n", argv[0]);
        return 1;
    }

    Game game;
    init_game(&game, argc, argv);

    // Initialisation de SDL2
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Erreur d'initialisation SDL: %s\n", SDL_GetError());
        return 1;
    }

    // Création de la fenêtre et du "pinceau" (renderer)
    SDL_Window *window = SDL_CreateWindow("Splashmem IA", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_SIZE, WINDOW_SIZE, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int running = 1;
    int game_over = 0;

    // Boucle d'affichage
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = 0;
            }
        }

        if (!game_over)
        {
            if (play_round(&game) == 0)
            {
                game_over = 1;
                print_results(&game, window);
                printf("Fermez la fenetre pour quitter.\n");
            }
        }

        draw_game(renderer, &game);

        if (!game_over)
        {
            //SDL_Delay(1);
        }
    }

    // Nettoyage avant de quitter
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    cleanup_game(&game);
    return 0;
}