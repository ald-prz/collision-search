#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <mpi.h>
#include <unistd.h>

#include "auxiliary.h"

int main(int argc, char *argv[])
{
    int id, nb_instance, len;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    unsigned char buff[1];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_instance);
    MPI_Get_processor_name(processor_name, &len);

    printf("ID:%d/%d\n", id, nb_instance);

    MPI_Status status;

    if (id == 0)
    {
        MPI_Recv(&buff, 1, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE
                 , MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        printf("From:%d;Received:%d\n", status.MPI_SOURCE, (int)buff[0]);

        MPI_Recv(&buff, 1, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE
                 , MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        printf("From:%d;Received:%d\n", status.MPI_SOURCE, (int)buff[0]);
    }
    else if (id == 1)
    {
        sleep(3);

        buff[0] = (unsigned char) 10;

        MPI_Send(&buff, 1, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
    }
    else if (id == 2)
    {
        sleep(5);

        buff[0] = (unsigned char) 15;

        MPI_Send(&buff, 1, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}
