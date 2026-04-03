#ifndef GAME_H
#define GAME_H

// Les constantes du jeu basées sur le sujet
#define BOARD_SIZE 100
#define MAX_PLAYERS 4
#define INITIAL_CREDITS 9000
#define MAX_BOMBS 100
#define MAX_CLONES 100
#define MAX_ACTIONS 1000  // Max d'actions dans un fichier texte

// Structure pour représenter une Bombe
typedef struct {
    int x, y;           // Position de la bombe
    int owner_id;       // ID du joueur qui a posé la bombe
    int creation_turn;  // Tour de création
    int is_active;      // 1 si active, 0 si déjà explodée
} Bomb;

// Structure pour représenter un Clone (Fork)
typedef struct {
    int parent_id;              // ID du joueur parent
    int creation_turn;          // Tour de création du clone
    int x, y;                   // Position du clone
    int is_active;              // 1 si actif, 0 si fin (20 tours)
    int action_index;           // Index dans la liste d'actions du parent
    int cost_multiplier;        // Multiplicateur de coûts (1x normal, 2x si fork actif)
} Clone;

// Structure pour représenter un Robot (Joueur)
typedef struct {
    int id;               // Numéro du joueur (1 à 4) pour l'identifier/couleur
    int x, y;             // Ses coordonnées actuelles sur la grille
    int credits;          // Ses points d'action restants (démarre à 9000)
    void *handle;         // La référence de sa bibliothèque .so
    char (*get_action)(); // Le "cerveau" : sa fonction pour choisir une action
    int is_active;        // 1 s'il a encore des crédits, 0 s'il est éliminé
    
    // Nouveaux champs pour les effets
    int mute_turns;       // Nombre de tours restants en MUTE (0 si pas muté)
    int mute_caster_id;   // ID du joueur qui a casté MUTE
    int swap_turns;       // Nombre de tours restants en SWAP (0 si pas swappé)
    int swap_caster_id;   // ID du joueur qui a casté SWAP
    int fork_turns;       // Nombre de tours restants du FORK (0 si pas en fork)
    int fork_clone_id;    // ID du clone associé au fork
    
    // Champs pour joueur texte
    char text_actions[MAX_ACTIONS];  // Buffer contenant les actions du fichier texte
    int text_action_count;           // Nombre total d'actions du fichier
    int text_action_index;           // Index actuel dans la liste d'actions
    int is_text_player;              // 1 si c'est un joueur texte, 0 si .so
} Player;

// Structure pour représenter la partie complète
typedef struct {
    int board[BOARD_SIZE][BOARD_SIZE]; // La grille de 100x100 cases
    Player players[MAX_PLAYERS];       // Le tableau contenant les joueurs
    int num_players;                   // Le nombre réel de joueurs (entre 1 et 4)
    
    // Nouveaux champs pour les éléments dynamiques
    Bomb bombs[MAX_BOMBS];             // Tableau des bombes actives
    int num_bombs;                     // Nombre de bombes actuelles
    Clone clones[MAX_CLONES];          // Tableau des clones actifs
    int num_clones;                    // Nombre de clones actuels
    int current_turn;                  // Tour actuel du jeu
} Game;

#endif