
#include <stdio.h>

#include "aacs.h"

#include "../util/macro.h"

AACS *aacs_open(const char *path)
{
    AACS aacs = malloc(sizeof(AACS));
}

void aacs_close(AACS *aacs)
{
    X_FREE(aacs);
}

