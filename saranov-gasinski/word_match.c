#include "word_match.h"

int word_match(unsigned char *one, unsigned char *two, int number_of_bytes)
{
    int i;

    for (i = 0; i < number_of_bytes; i++)
        if (one[i] != two[i])
            return 0;

    return 1;
}
