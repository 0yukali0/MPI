FROM mpioperator/openmpi-builder as builder

COPY bit_mpi.c /src/bits.c
RUN mpicc /src/bits.c -o /bits.exe


FROM mpioperator/openmpi

COPY --from=builder /bits.exe /home/mpiuser/bits.exe
