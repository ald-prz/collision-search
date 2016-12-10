module load openmpi/1.4.3/gcc/4.1.2
mpiCC main.c -o collisionSearch
mpirun -np 4 ./collisionSearch?