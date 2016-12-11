#include <stdio.h>
#include <string.h>
//#include <openssl/sha.h>

#include "auxiliary.h"

int main(int argc, char *argv[])
{
    /*unsigned char ibuf[] = "compute sha1";
    unsigned char obuf[20];

    SHA1(ibuf, strlen((const char*)ibuf), obuf);

    int i;
    for (i = 0; i < 20; i++) {
        printf("%02x ", obuf[i]);
    }
    printf("\n");*/

    unsigned char one[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,255,19};
    unsigned char two[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,0,22};

    if (is_match(one, two, 1) != 0)
        printf("yes");
    else
        printf("no");

    return 0;
}
