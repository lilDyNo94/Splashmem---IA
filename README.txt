Dylan Deshauteurs et Samba Niang

# Splashmem IA - Moteur de Jeu en C

Bienvenue dans mon projet Splashmem IA. Il s'agit d'un moteur de jeu d'arene en tour par tour developpe en C, ou jusqu'a 4 programmes (Intelligences Artificielles) s'affrontent sur une grille pour conquerir le plus grand territoire possible.

## Regles du Jeu et Fonctionnement

Le concept principal repose sur la gestion de ressources et l'occupation de l'espace :
* Le Plateau : Une grille de 100x100 cases avec un effet "Pacman" (un joueur sortant d'un cote de l'ecran reapparait du cote oppose).
* Les Credits : Chaque joueur debute la partie avec 9000 credits.
* Le But : Remplir un maximum de cases avec sa couleur. La partie s'arrete lorsque tous les joueurs ont epuise leurs credits. Le vainqueur est celui qui possede le plus de cases a la fin.
* Les Actions : A chaque tour, les joueurs depensent des credits pour agir :
  * Deplacement simple d'une case (Haut, Bas, Gauche, Droite) : Coute 1 credit.
  * Dash (Saut de 8 cases) : Coute 10 credits.
  * Teleportation (Saut de 8 cases) : Coute 2 credits.

Le moteur de jeu charge dynamiquement les "cerveaux" des joueurs au format bibliotheque partagee (.so) grace a la bibliotheque <dlfcn.h> et dessine le resultat en temps reel grace a SDL2.

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

Pour lancer une bataille a 4 joueurs, utilisez l'executable splash en lui passant les 4 fichiers .so en parametres :
./splash players/p1.so players/p2.so players/p3.so players/p4.so

Une fenetre s'ouvrira pour afficher le combat en temps reel. Une fois que tous les joueurs sont a court de credits, une boite de dialogue (pop-up) apparaitra pour annoncer le grand vainqueur et son score final !

## Architecture du Projet

.
├── include/
│   ├── actions.h       # Dictionnaire des actions (MOVE, DASH...)
│   └── game.h          # Structures du jeu (Player, Game)
├── players/
│   ├── p1.c / p1.so    # Code et binaire de l'IA 1
│   ├── p2.c / p2.so    # Code et binaire de l'IA 2
│   ├── p3.c / p3.so    # Code et binaire de l'IA 3
│   └── p4.c / p4.so    # Code et binaire de l'IA 4
├── src/
│   └── main.c          # Coeur du moteur de jeu et affichage SDL2
├── Makefile            # Script d'automatisation de la compilation
└── README.md           # Ce fichier
