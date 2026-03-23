#include "../include/actions.h"
#include <stdlib.h>

char get_action()
{
    int choix = (rand() % 4) + 1;
    return (char)choix;
}