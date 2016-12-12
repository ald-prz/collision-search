#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <mpi.h>
#include <unistd.h>

#include "digest_match.h"
#include "word_match.h"
#include "increment.h"

void process_host(int process_id, int process_num);
void process_worker(int process_id, int search_bytes, int match_bits);
void output_word(unsigned char *word, int size);
void output_collision(const char *filename, int size, unsigned char *word1, unsigned char *word2, unsigned char *digest1, unsigned char *digest2);
void rewrite_file(const char *filename);

int main(int argc, char *argv[])
{
    int len;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int process_id, process_num;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_id);
    MPI_Comm_size(MPI_COMM_WORLD, &process_num);
    MPI_Get_processor_name(processor_name, &len);

    if (argc < 5)
       printf("Arguments are not provided");
    else
    {
        if (process_num < 2)
            printf("Need atleast 2 processes (1 host+1 worker");
        else
        {
            if (process_id == 0)
                process_host(process_id, process_num);
            else
                process_worker(process_id, atoi(argv[2]), atoi(argv[4]));
        }
    }

    MPI_Finalize();

    return 0;
}


void process_host(int process_id, int process_num)
{
    unsigned char buff[2];
    unsigned char byte;
    int i;
    MPI_Status status;

    printf("ID:%d;Start host\n", process_id);

    byte = 255; // to start from 0

    do
    {
        byte++;
        MPI_Recv(&buff, 0, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        buff[0] = 1;
        buff[1] = byte;
        MPI_Send(&buff, 2, MPI_UNSIGNED_CHAR, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
    }
    while (byte != 254);

    for (i = 0; i < process_num - 1; i++)
    {
        MPI_Recv(&buff, 0, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        buff[0] = 0;
        buff[1] = 0;
        MPI_Send(&buff, 2, MPI_UNSIGNED_CHAR, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
    }

    printf("ID:%d;Stop host\n", process_id);
}

void process_worker(int process_id, int search_bytes, int match_bits)
{
    unsigned char msg[2];
    unsigned char *word1, *word2, *word_end1, *word_end2;
    unsigned char digest1[20], digest2[20];
    MPI_Status status;
    int i;
    int match;

    msg[0] = 1;
    word1 = (unsigned char *) malloc(search_bytes);
    word2 = (unsigned char *) malloc(search_bytes);
    word_end1 = (unsigned char *) malloc(search_bytes);
    word_end2 = (unsigned char *) malloc(search_bytes);

    char filename[60];
    sprintf(filename, "collisions_%d_bits_on_%d_bytes_msg_%d.csv", match_bits, search_bytes, process_id);

    rewrite_file(filename);

    printf("ID:%d;Start worker\n", process_id);

    while (msg[0] == 1) // the host orders to continue to work
    {
        MPI_Send(&msg, 0, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
        MPI_Recv(&msg, 2, MPI_UNSIGNED_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        //printf("ID:%d;Status:%d;Work:%d\n", id, (int) msg[0], (int) msg[1]);

        if (msg[0] == 1)
        {
            // set word1 to [byte].0.0...0 form

            word1[0] = msg[1];

            for (i = 1; i < search_bytes; i++)
                word1[i] = 0;

            // set word_end1 to [byte+1].0.0...0 form

            word_end1[0] = word1[0] + 1;

            for (i = 1; i < search_bytes; i++)
                word_end1[i] = 0;

            /*printf("----word1=[");
            output_word(word1, search_bytes);
            printf("]-[");
            output_word(word_end1, search_bytes);
            printf("]\n");*/

            // conduct search on word1 = [byte].x.x...x space

            do
            {
                // set word2 = word1

                for (i = 0; i < search_bytes; i++)
                    word2[i] = word1[i];

                // set word_end2 to [byte].255.255...255 form

                if (search_bytes == 1)
                    word_end2[0] = 255;
                else
                {
                    word_end2[0] = word1[0];

                    for (i = 1; i < search_bytes; i++)
                        word_end2[i] = 255;
                }

                /*printf("--word2=[");
                output_word(word2, search_bytes);
                printf("]-[");
                output_word(word_end2, search_bytes);
                printf("]\n");*/

                do
                {
                    increment(word2, search_bytes);

                    //output_word(word2, search_bytes);
                    //printf("\n");

                    SHA1(word1, search_bytes, digest1);
                    SHA1(word2, search_bytes, digest2);

                    if (digest_match(digest1, digest2, match_bits) == 1)
                        output_collision(filename, search_bytes, word1, word2, digest1, digest2);


                    /*output_word(word2, search_bytes);
                printf(" ??? ");
                output_word(word_end2, search_bytes);
                printf("\n");

                printf("%d\n", match);
                printf("s*8=%d\n", search_bytes * 8);*/
                }
                while (word_match(word2, word_end2, search_bytes) == 0);

                increment(word1, search_bytes);
            }
            while (word_match(word1, word_end1, search_bytes) == 0);
        }
    }

    free(word1);
    free(word2);
    free(word_end1);
    free(word_end2);


    printf("ID:%d;Stop worker\n", process_id);
}


void output_word(unsigned char *word, int size)
{
    int i;

    for (i = 0; i < size; i++)
        printf("%02x", word[i]);
}

void rewrite_file(const char *filename)
{
    FILE *file = fopen(filename, "w");
    fclose(file);
}

void output_collision(const char *filename, int size, unsigned char *word1, unsigned char *word2, unsigned char *digest1, unsigned char *digest2)
{
    int i;

    for (i = 0; i < size; i++)
        printf("%d ", word1[i]);

    printf(";");

    for (i = 0; i < 20; i++)
        printf("%02x", digest1[i]);

    printf("\n");

    for (i = 0; i < size; i++)
        printf("%d ", word2[i]);

    printf(";");

    for (i = 0; i < 20; i++)
        printf("%02x", digest2[i]);

    printf("\n\n");

    FILE *file = fopen(filename, "a");

    for (i = 0; i < size; i++)
        fprintf(file, "%02x", word1[i]);

    fprintf(file, ";");

    for (i = 0; i < 20; i++)
        fprintf(file, "%02x", digest1[i]);

    fprintf(file, "\n");

    for (i = 0; i < size; i++)
        fprintf(file, "%02x", word2[i]);

    fprintf(file, ";");

    for (i = 0; i < 20; i++)
        fprintf(file, "%02x", digest2[i]);

    fprintf(file, "\n");

    fclose(file);
}
