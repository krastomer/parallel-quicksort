#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define READ_FILENAME "data.txt"
#define WRITE_FILENAME "result.txt"
#define MASTER_NODE 0

int numberRank, sizeProcessor;

double *ReadFile(char *filename, int *size)
{
    double *arr;
    FILE *fp = fopen(filename, "r");
    if (fp != NULL)
    {
        fscanf(fp, "%d", size);
        arr = calloc(*size, sizeof(double));
        for (int i = 0; i < *size; i++)
        {
            fscanf(fp, "%lf", (arr + i));
        }
    }
    fclose(fp);
    return arr;
}

void *WriteFile(char *filename, double *A, int size)
{
    FILE *fp = fopen(filename, "w");
    if (fp != NULL)
    {
        fprintf(fp, "%d\n", size);
        for (int i = 0; i < size; i++)
        {
            fprintf(fp, "%lf\n", A[i]);
        }
    }
    fclose(fp);
}

void swap(double *a, double *b)
{
    double temp = *a;
    *a = *b;
    *b = temp;
}

int *CreateSendCounts(int size)
{
    int *arr = calloc(sizeProcessor, sizeof(int));
    for (int i = 0; i < sizeProcessor; i++)
    {
        arr[i] = size / sizeProcessor;
    }
    arr[0] += size % sizeProcessor;
    return arr;
}

int *CreateDisplacement(int *A)
{
    int *arr = calloc(sizeProcessor, sizeof(int));
    int index = 0;
    for (int i = 0; i < sizeProcessor; i++)
    {
        arr[i] = index;
        index += A[i];
    }
    return arr;
}

int Partition(double *A, int low, int high, double pivot)
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

int Rearrangement(double *A, int *pivotDispls, int *displs, int sizeA)
{
    int count = 0, pivot;
    double *B = calloc(sizeA, sizeof(double));

    for (int i = 0; i < sizeProcessor; i++)
    {
        for (int j = displs[i]; j <= pivotDispls[i] + displs[i]; j++)
        {
            B[count] = A[j];
            count++;
        }
    }

    pivot = count;

    for (int i = 0; i < sizeProcessor; i++)
    {
        int end = (sizeProcessor - 1 == i) ? sizeA : displs[i + 1];
        for (int j = pivotDispls[i] + displs[i] + 1; j < end; j++)
        {
            B[count] = A[j];
            count++;
        }
    }

    memcpy(A, B, sizeA * sizeof(double));
    free(B);

    return pivot;
}

void quicksort(double *A, int low, int high, int level)
{
    double *pivot = calloc(level + 1, sizeof(double));
}

int main(int argc, char **argv)
{
    int sizeA;
    double *A = ReadFile(READ_FILENAME, &sizeA);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &numberRank);
    MPI_Comm_size(MPI_COMM_WORLD, &sizeProcessor);

    quicksort(A, 0, sizeA - 1, 0);

    MPI_Finalize();
    return 0;
}