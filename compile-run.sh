module load openmpi/1.4.3/gcc/4.1.2
mpiCC main.c digest_match.c increment.c word_match.c -o collisionSearch -lcrypto
mpirun -np 3 ./collisionSearch -b 1 -n 12
