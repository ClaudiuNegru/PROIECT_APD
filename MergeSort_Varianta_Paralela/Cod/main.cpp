#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <fstream>

void merge(int* arr, int start, int mid, int end) {
    int i, j, k;
    int n1 = mid - start + 1;
    int n2 = end - mid;

    int* L, * R;
    MPI_Alloc_mem(sizeof(int) * n1, MPI_INFO_NULL, &L);
    MPI_Alloc_mem(sizeof(int) * n2, MPI_INFO_NULL, &R);

    for (i = 0; i < n1; i++)
        L[i] = arr[start + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    i = 0;
    j = 0;
    k = start;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        }
        else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }

    MPI_Free_mem(L);
    MPI_Free_mem(R);
}

void mergeSort(int* arr, int start, int end) {
    if (start < end) {
        int mid = start + (end - start) / 2;

        mergeSort(arr, start, mid);
        mergeSort(arr, mid + 1, end);

        merge(arr, start, mid, end);
    }
}

int main(int argc, char** argv) {
    int rank, size;
    int* data{}, * recv_data;
    int  chunk_size;
    int n = 1000000;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double start_time, end_time;
    start_time = MPI_Wtime();

    if (rank == 0) {
        printf("Enter the size of the array: ");
        scanf_s("%d", &n);

        data = (int*)malloc(sizeof(int) * n);

        for (int i = 0; i < n; i++) {
            data[i] = rand() % 1000000;
        }
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    chunk_size = n / size;
    recv_data = (int*)malloc(sizeof(int) * chunk_size);

    MPI_Scatter(data, chunk_size, MPI_INT, recv_data, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

    mergeSort(recv_data, 0, chunk_size - 1);

    int* sorted_data = nullptr;
    if (rank == 0) {
        sorted_data = (int*)malloc(sizeof(int) * n);
    }

    MPI_Gather(recv_data, chunk_size, MPI_INT, sorted_data, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        mergeSort(sorted_data, 0, n - 1);

        // write sorted array to file
        std::ofstream t;
        t.open("test12.txt");
        for (int i = 0; i < n; i++) {
            t << sorted_data[i] << "\n";
      
        }
        t.close();

        end_time = MPI_Wtime();
        printf("Time taken: %lf seconds\n", end_time - start_time);

        free(sorted_data);
        free(data);
    }

    free(recv_data);
    MPI_Finalize();
    return 0;
}
