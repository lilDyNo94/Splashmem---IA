#ifndef ACTIONS_H
#define ACTIONS_H

// Déplacements simples (Coût : 1 crédit) 
#define ACTION_MOVE_L 1
#define ACTION_MOVE_R 2
#define ACTION_MOVE_U 3
#define ACTION_MOVE_D 4

// Dash : Déplacement de 8 cases (Coût : 10 crédits) 
#define ACTION_DASH_L 5
#define ACTION_DASH_R 6
#define ACTION_DASH_U 7
#define ACTION_DASH_D 8

// Téléportation : Déplacement de 8 cases (Coût : 2 crédits) 
#define ACTION_TELEPORT_L 9
#define ACTION_TELEPORT_R 10
#define ACTION_TELEPORT_U 11
#define ACTION_TELEPORT_D 12

// Pas d'action (Coût : 1 crédit) 
#define ACTION_STILL 13

#endif