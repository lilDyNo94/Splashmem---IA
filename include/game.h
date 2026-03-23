#ifndef GAME_H
#define GAME_H

// Les constantes du jeu basées sur le sujet
#define BOARD_SIZE 100
#define MAX_PLAYERS 4
#define INITIAL_CREDITS 9000

// Structure pour représenter un Robot (Joueur)
typedef struct {
    int id;               // Numéro du joueur (1 à 4) pour l'identifier/couleur
    int x, y;             // Ses coordonnées actuelles sur la grille
    int credits;          // Ses points d'action restants (démarre à 9000)
    void *handle;         // La référence de sa bibliothèque .so
    char (*get_action)(); // Le "cerveau" : sa fonction pour choisir une action
    int is_active;        // 1 s'il a encore des crédits, 0 s'il est éliminé
} Player;

// Structure pour représenter la partie complète
typedef struct {
    int board[BOARD_SIZE][BOARD_SIZE]; // La grille de 100x100 cases
    Player players[MAX_PLAYERS];       // Le tableau contenant les joueurs
    int num_players;                   // Le nombre réel de joueurs (entre 1 et 4)
} Game;

#endif