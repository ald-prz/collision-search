#include <mpi.h>
#include <iostream>

using namespace std;

void determine_search_space(int process_id, int process_num, unsigned char *start, unsigned char *end);

int main(int argc, char *argv[])
{
    int id, nb_instance, len;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    unsigned char start, end;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_instance);
    MPI_Get_processor_name(processor_name, &len);

    determine_search_space(id, nb_instance, &start, &end);

    cout << "process [" << id <<"] from " << +start <<" to "<< +end;
    cout << endl;

    MPI_Finalize();

    return 0;
}


void determine_search_space(int process_id, int process_num, unsigned char *start, unsigned char *end)
{
    double step = 256.0 / process_num;

    *start = (unsigned char)(process_id * step + 0.5);
    *end = (unsigned char)((process_id + 1) * step + 0.5) - 1;
}
