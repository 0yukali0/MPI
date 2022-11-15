FROM mpioperator/openmpi-builder as builder

COPY prime_mpi.c /src/prime.c
RUN mpicc /src/prime.c -o /prime.exe


FROM mpioperator/openmpi

COPY --from=builder /prime.exe /home/mpiuser/prime.exe
