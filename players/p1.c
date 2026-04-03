#include "../include/actions.h"
#include <stdlib.h>

char get_action()
{
    int choix = rand() % 18;
    return (char)choix;
}