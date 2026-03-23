#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <SDL2/SDL.h>
#include "../include/actions.h"
#include "../include/game.h"

#define CELL_SIZE 6
#define WINDOW_SIZE (BOARD_SIZE * CELL_SIZE)

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

    for (int i = 0; i < game->num_players; i++)
    {
        game->players[i].id = i + 1;
        game->players[i].credits = INITIAL_CREDITS;
        game->players[i].is_active = 1;
        game->players[i].x = start_x[i];
        game->players[i].y = start_y[i];

        game->players[i].handle = dlopen(argv[i + 1], RTLD_LAZY);
        if (!game->players[i].handle)
        {
            fprintf(stderr, "Erreur chargement joueur %d : %s\n", i + 1, dlerror());
            exit(1);
        }
        game->players[i].get_action = dlsym(game->players[i].handle, "get_action");
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

int play_round(Game *game)
{
    int active_players = 0;
    for (int i = 0; i < game->num_players; i++)
    {
        Player *p = &game->players[i];
        if (p->credits <= 0)
            continue;
        active_players++;

        char action = p->get_action();
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
        case ACTION_STILL:
            cost = 1;
            break;
        default:
            cost = 1;
            break;
        }

        if (p->credits >= cost)
        {
            p->credits -= cost;
            p->x = wrap_coord(nx);
            p->y = wrap_coord(ny);
            game->board[p->x][p->y] = p->id;
        }
        else
        {
            p->credits = 0;
        }
    }
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