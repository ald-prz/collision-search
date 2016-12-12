#include "increment.h"


void increment(unsigned char *word, int length)
{
    int i;
    int nextFlag = 0;

    word[length - 1]++;
    if (word[length - 1] == 0)
        nextFlag = 1;

    for (i = length - 2; i >= 0; i--)
        if (nextFlag == 1)
        {
            word[i]++;
            if (word[i] != 0)
                nextFlag = 0;
        }
        else
            break;
}
