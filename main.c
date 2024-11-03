#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include "graph.h"

// Khai báo hàm tsp_ga_serial cho thuật toán GA
float tsp_ga_serial(float **dist_matrix, int size, int **best_path);
// khai báo hàm từ `ga_tsp_parallel.c`
float tsp_ga_parallel(float **dist_matrix, int size, int **best_path);


int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <number_of_nodes>\n", argv[0]);
        return 1;
    }

    int size = atoi(argv[1]);
    int **graph = create_graph(size);

    printf("Running sequential version...\n");
    clock_t start = clock();
    // int min_cost_sequential = tsp_bruteforce(graph, size);
    clock_t end = clock();
    double time_sequential = (double)(end - start) / CLOCKS_PER_SEC;
    // printf("Minimum Cost (Sequential): %d\n", min_cost_sequential);
    printf("Time (Sequential): %f seconds\n", time_sequential);

    printf("\nRunning parallel version...\n");
    start = clock();
    // int min_cost_parallel = tsp_bruteforce_parallel(graph, size);
    end = clock();
    double time_parallel = (double)(end - start) / CLOCKS_PER_SEC;
    // printf("Minimum Cost (Parallel): %d\n", min_cost_parallel);
    printf("Time (Parallel): %f seconds\n", time_parallel);


    // Chuyển đổi `graph` sang `dist_matrix` cho hàm GA
    float **dist_matrix = (float **)malloc(size * sizeof(float *));
    for (int i = 0; i < size; i++) {
        dist_matrix[i] = (float *)malloc(size * sizeof(float));
        for (int j = 0; j < size; j++) {
            dist_matrix[i][j] = (float)graph[i][j];
        }
    }

    printf("\nRunning GA (Genetic Algorithm) version...\n");
    int *best_path = NULL;  // Khởi tạo con trỏ best_path
    start = clock();
    float best_cost = tsp_ga_serial(dist_matrix, size, &best_path);  // Gọi hàm GA và lấy best_path
    end = clock();
    double time_ga = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Time (GA): %f seconds\n", time_ga);
    printf("Best Path (GA): ");
    for (int i = 0; i < size; i++) {
        printf("%d ", best_path[i]);
    }
    printf("\nMinimum Cost (GA): %f\n", best_cost);

    free(best_path);  // Giải phóng bộ nhớ cho best_path

    // Chạy thuật toán GA song song
    printf("\nRunning GA (Genetic Algorithm) Parallel version...\n");
    int *best_path_parallel = NULL;  // Khởi tạo con trỏ best_path cho GA song song
    start = omp_get_wtime();
    float best_cost_parallel = tsp_ga_parallel(dist_matrix, size, &best_path_parallel);
    double time_ga_parallel = omp_get_wtime() - start;

    printf("Time (GA Parallel): %f seconds\n", time_ga_parallel);
    printf("Best Path (GA Parallel): ");
    for (int i = 0; i < size; i++) {
        printf("%d ", best_path_parallel[i]);
    }
    printf("\nMinimum Cost (GA Parallel): %f\n", best_cost_parallel);

    free(best_path_parallel);  // Giải phóng bộ nhớ cho best_path_parallel
    // Giải phóng bộ nhớ
    for (int i = 0; i < size; i++) {
        free(dist_matrix[i]);
    }
    free(dist_matrix);

    free_graph(graph, size);
    return 0;
}
