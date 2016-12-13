#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <mpi.h>
#include <unistd.h>

#include "digest_match.h"
#include "word_match.h"
#include "increment.h"

void process_host(int process_id, int process_num, int search_bytes);
void process_worker(int process_id, int search_bytes, int match_bits);
void output_word(unsigned char *word, int size);
void output_collision(const char *filename, int size, unsigned char *word1, unsigned char *word2, unsigned char *digest1, unsigned char *digest2);
void rewrite_file(const char *filename);
void output_runtime(int *runtime, int size);

/*
 * TODO:
 * 1) Disable all printf
 * 2) Try 16 processes once again
*/
int main(int argc, char *argv[])
{
    int len;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int process_id, process_num;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_id);
    MPI_Comm_size(MPI_COMM_WORLD, &process_num);
    MPI_Get_processor_name(processor_name, &len);

    printf("start");

    if (argc < 5)
       printf("Arguments are not provided");
    else
    {
        if (process_num < 2)
            printf("Need atleast 2 processes (1 host+1 worker");
        else
        {
            if (process_id == 0)
                process_host(process_id, process_num, atoi(argv[2]));
            else
                process_worker(process_id, atoi(argv[2]), atoi(argv[4]));
        }
    }

    MPI_Finalize();

    return 0;
}


void process_host(int process_id, int process_num, int search_bytes)
{
    unsigned char buff_char[2];
    int buff_int[1];
    unsigned char byte, target_byte;
    double time1, time2;
    int *runtime;
    int i;
    MPI_Status status;

    time1 = MPI_Wtime();

    printf("ID:%d;Start host\n", process_id);


    if (search_bytes == 1)
        target_byte = 254;
    else
        target_byte = 255;

    byte = 255; // to start from 0

    // sending tasks

    do
    {
        byte++;
        MPI_Recv(&buff_char, 0, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        buff_char[0] = 1;
        buff_char[1] = byte;
        MPI_Send(&buff_char, 2, MPI_UNSIGNED_CHAR, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
    }
    while (byte != target_byte);

    printf("ID:%d;Recieving finish\n", process_id);

    // sending finish commands

    for (i = 1; i < process_num; i++)
    {
        MPI_Recv(&buff_char, 0, MPI_UNSIGNED_CHAR, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        buff_char[0] = 0;
        buff_char[1] = 0;
        MPI_Send(&buff_char, 2, MPI_UNSIGNED_CHAR, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
    }

    printf("ID:%d;Recieving runtime\n", process_id);

    // recieving runtimes

    runtime = (int *) malloc(sizeof(int) * process_num);

    for (i = 1; i < process_num; i++)
    {
        MPI_Recv(&buff_int, 1, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        runtime[status.MPI_SOURCE] = buff_int[0];
    }


    printf("ID:%d;Output\n", process_id);

    time2 = MPI_Wtime();
    runtime[0] = (int) (time2 - time1);

    // output runtimes

    output_runtime(runtime, process_num);

    free(runtime);

    printf("ID:%d;Stop host\n", process_id);
}

void output_runtime(int *runtime, int size)
{
    FILE *file = fopen("time.txt", "w");
    int i;

    for (i = 0; i < size; i++)
        fprintf(file, "Process %d : %d sec\n", i, runtime[i]);

    fclose(file);
}

void process_worker(int process_id, int search_bytes, int match_bits)
{
    unsigned char buff_char[2];
    int buff_int[1];
    unsigned char *word1, *word2, *word_end1, *word_end2;
    unsigned char digest1[20], digest2[20];
    MPI_Status status;
    double time1, time2;
    int i;
    int match;

    time1 = MPI_Wtime();

    buff_char[0] = 1;
    word1 = (unsigned char *) malloc(search_bytes);
    word2 = (unsigned char *) malloc(search_bytes);
    word_end1 = (unsigned char *) malloc(search_bytes);
    word_end2 = (unsigned char *) malloc(search_bytes);

    char filename[60];
    sprintf(filename, "collisions_%d_bits_on_%d_bytes_msg_%d.csv", match_bits, search_bytes, process_id);

    rewrite_file(filename);

    printf("ID:%d;Start worker\n", process_id);

    while (buff_char[0] == 1) // the host orders to continue to work
    {
        MPI_Send(&buff_char, 0, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
        MPI_Recv(&buff_char, 2, MPI_UNSIGNED_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        printf("ID:%d;Status:%d;Work:%d\n", process_id, (int) buff_char[0], (int) buff_char[1]);

        if (buff_char[0] == 1)
        {
            // set word1 to [byte].0.0...0 form

            word1[0] = buff_char[1];

            for (i = 1; i < search_bytes; i++)
                word1[i] = 0;

            // set word_end1

            if (buff_char[1] == 255)
            {
                word_end1[0] = buff_char[1];

                for (i = 1; i < search_bytes - 1; i++)
                    word_end1[i] = 255;

                word_end1[search_bytes - 1] = 254;
            }
            else
            {
                word_end1[0] = buff_char[1] + 1;

                for (i = 1; i < search_bytes; i++)
                    word_end1[i] = 0;
            }

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

                // set word_end2 to 255.255...255 form

                for (i = 0; i < search_bytes; i++)
                    word_end2[i] = 255;


                /*printf("--word2=[");
                output_word(word2, search_bytes);
                printf("]-[");
                output_word(word_end2, search_bytes);
                printf("]\n");*/

                SHA1(word1, search_bytes, digest1);

                do
                {
                    increment(word2, search_bytes);

                    //output_word(word2, search_bytes);
                    //printf("\n");                    

                    SHA1(word2, search_bytes, digest2);

                    if (digest_match(digest1, digest2, match_bits) == 1)
                        //if (word_match(word1, word2, search_bytes) != 1)
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

    time2 = MPI_Wtime();
    buff_int[0] = time2 - time1;
    MPI_Send(&buff_int, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

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

    FILE *file = fopen(filename, "a");

    for (i = 0; i < size; i++)
        fprintf(file, "%02x", word1[i]);

    fprintf(file, ";");

    for (i = 0; i < 20; i++)
        fprintf(file, "%02x", digest1[i]);

    fprintf(file, "\n");

    for (i = 0; i < size; i++)
        fprintf(file, "%02x", word2[i]);

    fprintf(file, ",");

    for (i = 0; i < 20; i++)
        fprintf(file, "%02x", digest2[i]);

    fprintf(file, "\n");

    fclose(file);
}
