#ifndef ACTIONS_H
#define ACTIONS_H

// Énumération des actions selon la spécification Addendum
enum action {
    ACTION_STILL = 0,           // Pas d'action (Coût : 1)
    ACTION_MOVE_L = 1,          // Déplacement gauche (Coût : 1)
    ACTION_MOVE_R = 2,          // Déplacement droite (Coût : 1)
    ACTION_MOVE_U = 3,          // Déplacement haut (Coût : 1)
    ACTION_MOVE_D = 4,          // Déplacement bas (Coût : 1)
    ACTION_DASH_L = 5,          // Saut 8 cases gauche (Coût : 10)
    ACTION_DASH_R = 6,          // Saut 8 cases droite (Coût : 10)
    ACTION_DASH_U = 7,          // Saut 8 cases haut (Coût : 10)
    ACTION_DASH_D = 8,          // Saut 8 cases bas (Coût : 10)
    ACTION_TELEPORT_L = 9,      // Téléport 8 cases gauche (Coût : 2)
    ACTION_TELEPORT_R = 10,     // Téléport 8 cases droite (Coût : 2)
    ACTION_TELEPORT_U = 11,     // Téléport 8 cases haut (Coût : 2)
    ACTION_TELEPORT_D = 12,     // Téléport 8 cases bas (Coût : 2)
    ACTION_BOMB = 13,           // Bombe (Coût : 9, explose tour 5)
    ACTION_FORK = 14,           // Fork clone (Coût : variable)
    ACTION_CLEAN = 15,          // Nettoie 7x7 (Coût : 40)
    ACTION_MUTE = 16,           // Mute ennemi 10 tours (Coût : 30)
    ACTION_SWAP = 17,           // Swap ennemi 5 tours (Coût : 35)
    ACTION_NUMBER               // Nombre total d'actions
};

#endif