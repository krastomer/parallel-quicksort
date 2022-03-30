#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define READ_FILENAME "data.txt"
#define MASTER_NODE 0

int rank, nP, level;

float *readFile(char *filename, int *n)
{
    float *arr;
    FILE *fp = fopen(filename, "r");
    if (fp != NULL)
    {
        fscanf(fp, "%d", n);
        arr = malloc(*n * sizeof(float));
        for (int i = 0; i < *n; i++)
        {
            fscanf(fp, "%f", (arr + i));
        }
    }
    fclose(fp);
    return arr;
}

void swap(float *A, int i, int j)
{
    float temp = A[i];
    A[i] = A[j];
    A[j] = temp;
}

int *sendCounts(int n)
{
    int *arr = malloc(nP * sizeof(int));
    for (int i = 0; i < nP; i++)
    {
        arr[i] = n / nP;
    }
    arr[0] += n % nP;
    return arr;
}

int *displs(int *A)
{
    int *arr = malloc(nP * sizeof(int));
    int index = 0;
    for (int i = 0; i < nP; i++)
    {
        arr[i] = index;
        index += A[i];
    }
    return arr;
}

// int partition(float *A, int low, int high, float *globalPivot)
// {
//     float pivot = (level == 0) ? *globalPivot : A[(high + low) / 2];
//     int i = low, j = high;
//     int count = 0;
//     if (rank == MASTER_NODE)
//     {

//         while (1)
//         {
//             printf("before: ");
//             for (int i = 0; i < high - low; i++)
//             {
//                 printf("%.2lf ", A[i]);
//             }
//             printf("\n");
//             while (A[i] < pivot)
//             {
//                 i++;
//                 printf("+");
//             }
//             printf("\n");
//             while (A[j] > pivot)
//             {
//                 j--;
//                 printf("-");
//             }
//             printf("\n");
//             if (i >= j)
//             {
//                 printf("out\n");
//                 return j;
//             }
//             swap(&A[i], &A[j]);
//             printf("swap: %f %f\n", A[i], A[j]);
//             count++;
//             printf("after: ");
//             for (int i = 0; i < high - low; i++)
//             {
//                 printf("%.2lf ", A[i]);
//             }
//             printf("\n");
//         }
//     }
// }

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
        swap(A, i, j);
    }
}

void quicksort(float *A, int low, int high)
{
    if (low >= 0 && high >= 0 && low < high)
    {
        if (level == 0)
        {
            float pivot, *subA;
            if (rank == MASTER_NODE)
            {
                pivot = A[0];
            }
            MPI_Bcast(&pivot, 1, MPI_FLOAT, MASTER_NODE, MPI_COMM_WORLD);

            int *sC = sendCounts(high - low + 1);
            int *dis = displs(sC);
            subA = malloc(sC[rank] * sizeof(float));
            MPI_Scatterv(A, sC, dis, MPI_FLOAT, subA, sC[rank], MPI_FLOAT, MASTER_NODE, MPI_COMM_WORLD);

            // if (rank == MASTER_NODE)
            // {
            //     printf("%d\n", sC[rank]);
            //     for (int i = 0; i < 25; i++)
            //     {
            //         printf("%f\n", subA[i]);
            //     }
            // }
            int p = partitionWithPivot(A, 0, sC[rank] - 1, pivot);
            if (rank == MASTER_NODE)
            {
                printf("%d\n", p);
                for (int i = 0; i < 25; i++)
                {
                    printf("%d, %f\n", i, subA[i]);
                }
            }
        }
        // quicksort(A, low, p);
        // quicksort(A, p + 1, high);
    }
}

int main(int argc, char **argv)
{
    int n;
    float *A = readFile(READ_FILENAME, &n);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nP);

    level = 0;
    quicksort(A, 0, n - 1);

    MPI_Finalize();
    return 0;
}