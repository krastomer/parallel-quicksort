#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define READ_FILENAME "data.txt"
#define WRITE_FILENAME "result.txt"
#define MASTER_NODE 0

int rank, sizeProcessor;

float *ReadFile(char *filename, int *n)
{
    float *arr;
    FILE *fp = fopen(filename, "r");
    if (fp != NULL)
    {
        fscanf(fp, "%d", n);
        arr = malloc(sizeof(float) * *n);
        for (int i = 0; i < *n; i++)
        {
            fscanf(fp, "%f", (arr + i));
        }
    }
    fclose(fp);
    return arr;
}

void *writeFile(char *filename, float *A, int n)
{
    FILE *fp = fopen(filename, "w");
    if (fp != NULL)
    {
        fprintf(fp, "%d\n", n);
        for (int i = 0; i < n; i++)
        {
            fprintf(fp, "%f\n", A[i]);
        }
    }
    fclose(fp);
}

void swap(float *a, float *b)
{
    float temp = *a;
    *a = *b;
    *b = temp;
}

int *generateSendCounts(int n)
{
    int *arr = malloc(sizeProcessor * sizeof(int));
    for (int i = 0; i < sizeProcessor; i++)
    {
        arr[i] = n / sizeProcessor;
    }
    arr[0] += n % sizeProcessor;
    return arr;
}

int *CreateDisplacement(int *A)
{
    int *arr = malloc(sizeProcessor * sizeof(int));
    int index = 0;
    for (int i = 0; i < sizeProcessor; i++)
    {
        arr[i] = index;
        index += A[i];
    }
    return arr;
}

int partitionWithPivot(float *A, int low, int high, float pivot)
{
    int i = low - 1, j = high + 1;
    while (1)
    {
        do
        {
            i++;
        } while (A[i] < pivot);
        do
        {
            j--;
        } while (A[j] > pivot);
        if (i >= j)
        {
            return j;
        }
        swap(&A[i], &A[j]);
    }
}

int globalRearrangement(float *A, int *displsPivot, int *displs, int n)
{
    int count = 0, pivot;
    float *B = malloc(n * sizeof(float));

    for (int i = 0; i < sizeProcessor; i++)
    {
        for (int j = displs[i]; j <= displsPivot[i] + displs[i]; j++)
        {
            B[count] = A[j];
            count++;
        }
    }

    pivot = count;

    for (int i = 0; i < sizeProcessor; i++)
    {
        int end = (sizeProcessor - 1 == i) ? n : displs[i + 1];
        for (int j = displsPivot[i] + displs[i] + 1; j < end; j++)
        {
            B[count] = A[j];
            count++;
        }
    }
    memcpy(A, B, n * sizeof(float));
    return pivot;
}

void quicksort(float *A, int low, int high, int level)
{
    if (low >= 0 && high >= 0 && low < high)
    {
        if (level == 0)
        {
            float pivot;
            int *displsPivot = malloc(sizeProcessor * sizeof(float));
            if (rank == MASTER_NODE)
            {
                pivot = A[0];
            }
            MPI_Bcast(&pivot, 1, MPI_FLOAT, MASTER_NODE, MPI_COMM_WORLD);

            int *sc = generateSendCounts(high + 1);
            int *displs = CreateDisplacement(sc);

            float *subA = malloc(sc[rank] * sizeof(float));

            MPI_Barrier(MPI_COMM_WORLD);
            MPI_Scatterv(A, sc, displs, MPI_FLOAT, subA, sc[rank], MPI_FLOAT, MASTER_NODE, MPI_COMM_WORLD);

            int p = partitionWithPivot(subA, 0, sc[rank] - 1, pivot);

            MPI_Barrier(MPI_COMM_WORLD);
            MPI_Gatherv(subA, sc[rank], MPI_FLOAT, A, sc, displs, MPI_FLOAT, MASTER_NODE, MPI_COMM_WORLD);

            MPI_Barrier(MPI_COMM_WORLD);
            MPI_Gather(&p, 1, MPI_INT, displsPivot, 1, MPI_INT, MASTER_NODE, MPI_COMM_WORLD);

            if (rank == MASTER_NODE)
            {
                int j = globalRearrangement(A, displsPivot, displs, high + 1);

                int processPivot = (int)(((float)j / (float)(high + 1)) * sizeProcessor);

                // quicksort();
                for (int i = 0; i < high + 1; i++)
                {
                    printf("%d, %f\n", i, A[i]);
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    int n;
    float *A = ReadFile(READ_FILENAME, &n);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &sizeProcessor);

    quicksort(A, 0, n - 1, 0);
    // writeFile(WRITE_FILENAME, A, n);
    MPI_Finalize();
    return 0;
}