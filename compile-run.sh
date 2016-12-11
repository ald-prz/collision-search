module load openmpi/1.4.3/gcc/4.1.2
mpiCC main.c -o collisionSearch -lcrypto
mpirun -np 3 ./collisionSearch