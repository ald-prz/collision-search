#include <stdio.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <mpi.h>
#include <unistd.h>

#include "auxiliary.h"

void process_host();
void process_worker();

int id, nb_instance;

int main(int argc, char *argv[])
{
    int len;
    char processor_name[MPI_MAX_PROCESSOR_NAME];


    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_instance);
    MPI_Get_processor_name(processor_name, &len);

    if (id == 0)
        process_host();
    else
        process_worker();

    MPI_Finalize();

    return 0;
}


void process_host()
{
    unsigned char buff[2];
    unsigned char byte;
    int i;
    MPI_Status status;

    printf("ID:%d;Start host\n", id);

    byte = 255; // to start from 0

    do
    {
        byte++;
        MPI_Recv(&buff, 0, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        buff[0] = 1;
        buff[1] = byte;
        printf("ID:%d;sending %d\n", id, (int)byte);
        MPI_Send(&buff, 2, MPI_UNSIGNED_CHAR, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
    }
    while (byte != 255);

    for (i = 0; i < nb_instance - 1; i++)
    {
        MPI_Recv(&buff, 0, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        buff[0] = 0;
        buff[1] = 0;
        MPI_Send(&buff, 2, MPI_UNSIGNED_CHAR, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
    }

    printf("ID:%d;Stop host\n", id);
}

void process_worker()
{
    unsigned char buff[2];
    MPI_Status status;
    buff[0] = 1;

    printf("ID:%d;Start worker\n", id);

    while (buff[0] == 1)
    {
        MPI_Send(&buff, 0, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
        MPI_Recv(&buff, 2, MPI_UNSIGNED_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        printf("ID:%d;Status:%d;Work:%d\n", id, (int) buff[0], (int) buff[1]);
    }

    printf("ID:%d;Stop worker\n", id);
}
