#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READ_FILENAME "data.txt"
#define WRITE_FILENAME "result.txt"

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

int Partition(double *A, int low, int high)
{
    double pivot = A[(high + low) / 2];
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

int Quicksort(double *A, int low, int high)
{
    if (low >= 0 && high >= 0 && low < high)
    {
        int p = Partition(A, low, high);
        Quicksort(A, low, p);
        Quicksort(A, p + 1, high);
    }
}

int main(int argc, char **argv)
{
    int sizeA;
    double *A = ReadFile(READ_FILENAME, &sizeA);

    Quicksort(A, 0, sizeA - 1);
    WriteFile(WRITE_FILENAME, A, sizeA);

    return 0;
}