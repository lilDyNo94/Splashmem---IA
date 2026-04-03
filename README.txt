Dylan Deshauteurs et Samba Niang

# Splashmem IA - Moteur de Jeu en C

Bienvenue dans mon projet Splashmem IA. Il s'agit d'un moteur de jeu d'arene en tour par tour developpe en C, ou jusqu'a 4 programmes (Intelligences Artificielles) s'affrontent sur une grille pour conquerir le plus grand territoire possible.

## Regles du Jeu et Fonctionnement

Le concept principal repose sur la gestion de ressources et l'occupation de l'espace :
* Le Plateau : Une grille de 100x100 cases avec un effet "Pacman" (un joueur sortant d'un cote de l'ecran reapparait du cote oppose).
* Les Credits : Chaque joueur debute la partie avec 9000 credits.
* Le But : Remplir un maximum de cases avec sa couleur. La partie s'arrete lorsque tous les joueurs ont epuise leurs credits. Le vainqueur est celui qui possede le plus de cases a la fin.

## Actions Disponibles

Les joueurs disposent de 18 actions differentes pour dominer le jeu :

### Actions de Deplacement (Coûts Faibles)
* ACTION_MOVE_L, ACTION_MOVE_R, ACTION_MOVE_U, ACTION_MOVE_D : Deplacement simple d'une case - 1 credit
* ACTION_DASH_L, ACTION_DASH_R, ACTION_DASH_U, ACTION_DASH_D : Saut de 8 cases - 10 credits
* ACTION_TELEPORT_L, ACTION_TELEPORT_R, ACTION_TELEPORT_U, ACTION_TELEPORT_D : Teleportation inversée (8 cases) - 2 credits

### Actions Offensives (Addendum Phase 2+)
* ACTION_BOMB : Place une bombe qui explose après 5 tours, couvrant 9 cases - 9 credits
* ACTION_CLEAN : Nettoie un carré 7x7 autour du joueur - 40 credits
* ACTION_MUTE : Rend un ennemi proche muet pendant 10 tours (se déplace mais colore en noir) - 30 credits
* ACTION_SWAP : Inverse la couleur d'un ennemi proche pendant 5 tours - 35 credits
* ACTION_FORK : Crée un clone qui reproduit les actions 20 tours (double les coûts du joueur) - 20 credits base
* ACTION_STILL : Reste inactif (pas d'action)

Le moteur de jeu charge dynamiquement les "cerveaux" des joueurs au format bibliotheque partagee (.so) grace a la bibliotheque <dlfcn.h> et dessine le resultat en temps reel grace a SDL2.

Les joueurs peuvent egalement etre des fichiers texte (.txt) contenant une liste d'actions separees par des virgules, permettant de creer des strategies statiques et reproductibles. Cela permet de tester et de verifier des sequences specifiques sans avoir a recompiler une bibliotheque.

### Format du fichier joueur texte (.txt)

Un fichier joueur texte doit contenir une liste d'actions separees par des virgules. Ces actions seront repetees en boucle pendant toute la partie.

Exemple : players/p5_text.txt
```
ACTION_MOVE_R,ACTION_MOVE_R,ACTION_DASH_U,ACTION_BOMB,ACTION_CLEAN,ACTION_STILL
```

Lors du chargement du joueur, le parseur va :
1. Lire le fichier texte
2. Compter le nombre d'actions (nombre de virgules + 1)
3. Cycler dans cette liste d'actions tour par tour
4. Recommencer au debut une fois le dernier action executee

Cette fonctionnalite permet de :
- Tester rapidement des strategies sans recompiler
- Verifier des sequences d'actions complexes
- Creer des joueurs "scenario" pour des tests reproductibles
- Faciliter le debug en controlant precisement les actions

## Les 4 Profils d'IA inclus

Pour tester le moteur, 4 profils de joueurs ont ete developpes dans le dossier players/ :
1. P1 (Rouge) : Deplacement aleatoire.
2. P2 (Bleu) : Deplacement aleatoire.
3. P3 (Vert) : Deplacement aleatoire.
4. P4 (Jaune) : Deplacement aleatoire.

Tous les robots utilisent actuellement la fonction rand() pour choisir leurs actions et se deplacer de facon erratique afin de couvrir la plus large zone possible.

## Prerequis et Installation

Ce projet a ete developpe et teste sous Linux (Ubuntu).
Pour compiler et executer le projet avec l'interface graphique, vous devez installer la bibliotheque SDL2.

Ouvrez un terminal et executez les commandes suivantes :
sudo apt-get update
sudo apt-get install libsdl2-dev

## Compilation

Le projet integre un Makefile complet pour automatiser la compilation du moteur et des 4 bibliotheques dynamiques des joueurs.

Pour tout compiler, placez-vous a la racine du projet et tapez :
make

(Pour nettoyer les fichiers compiles, vous pouvez utiliser la commande make clean).

## Lancement d'une partie

Pour lancer une bataille a 4 joueurs, utilisez l'executable splash en lui passant les 4 fichiers .so (ou .txt) en parametres :
./splash players/p1.so players/p2.so players/p3.so players/p4.so

Vous pouvez aussi melanger joueurs IA et joueurs texte :
./splash players/p1.so players/p2.so players/p3.so players/p5_text.txt

Format fichier texte (.txt) : Une ligne avec actions separees par des virgules
ACTION_MOVE_R,ACTION_MOVE_R,ACTION_MOVE_D,ACTION_MOVE_D,ACTION_BOMB,ACTION_CLEAN

Une fenetre s'ouvrira pour afficher le combat en temps reel. Une fois que tous les joueurs sont a court de credits, une boite de dialogue (pop-up) apparaitra pour annoncer le grand vainqueur et son score final !

## Architecture du Projet

.
├── include/
│   ├── actions.h       # 18 actions enum (MOVE, DASH, TELEPORT, BOMB, CLEAN, MUTE, SWAP, FORK)
│   └── game.h          # Structures du jeu (Player, Game, Bomb, Clone, Effects)
├── players/
│   ├── p1.c / p1.so    # Code et binaire de l'IA 1 (aleatoire)
│   ├── p2.c / p2.so    # Code et binaire de l'IA 2 (aleatoire)
│   ├── p3.c / p3.so    # Code et binaire de l'IA 3 (aleatoire)
│   ├── p4.c / p4.so    # Code et binaire de l'IA 4 (aleatoire)
│   └── p5_text.txt     # Exemple de joueur texte avec actions predefinies
├── src/
│   └── main.c          # Coeur du moteur de jeu (594 lignes)
│                       # - Gestion des actions et effets
│                       # - Systeme de bombes et clones
│                       # - Parseur fichier texte
│                       # - Affichage SDL2
├── Makefile            # Script d'automatisation de la compilation
└── README.txt          # Ce fichier