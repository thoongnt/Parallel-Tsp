#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <omp.h>
#include "graph.h"

// Hàm hoán vị
void swap(int *x, int *y) {
    int temp = *x;
    *x = *y;
    *y = temp;
}

// Tính chi phí của một đường đi
int calculate_cost(int **graph, int *path, int size) {
    int cost = 0;
    for (int i = 0; i < size - 1; i++) {
        cost += graph[path[i]][path[i + 1]];
    }
    cost += graph[path[size - 1]][path[0]]; // Quay lại đỉnh bắt đầu
    return cost;
}

// Phiên bản tuần tự
void tsp_bruteforce_helper(int **graph, int *path, int start, int end, int *min_cost, int size) {
    if (start == end) {
        int current_cost = calculate_cost(graph, path, size);
        if (current_cost < *min_cost) {
            *min_cost = current_cost;
        }
        return;
    }

    for (int i = start; i <= end; i++) {
        swap(&path[start], &path[i]);
        tsp_bruteforce_helper(graph, path, start + 1, end, min_cost, size);
        swap(&path[start], &path[i]);
    }
}

int tsp_bruteforce(int **graph, int size) {
    int *path = (int *)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        path[i] = i;
    }

    int min_cost = INT_MAX;
    tsp_bruteforce_helper(graph, path, 1, size - 1, &min_cost, size);

    free(path);
    return min_cost;
}

// Phiên bản song song với OpenMP
int tsp_bruteforce_parallel(int **graph, int size) {
    int *path = (int *)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        path[i] = i;
    }

    int min_cost = INT_MAX;

    // omp_set_num_threads(16);

    #pragma omp parallel
    {
        int local_min_cost = INT_MAX;

        #pragma omp for
        for (int i = 1; i < size; i++) {
            int *local_path = (int *)malloc(size * sizeof(int));
            for (int j = 0; j < size; j++) {
                local_path[j] = path[j];
            }

            swap(&local_path[1], &local_path[i]);
            tsp_bruteforce_helper(graph, local_path, 2, size - 1, &local_min_cost, size);

            #pragma omp critical
            {
                if (local_min_cost < min_cost) {
                    min_cost = local_min_cost;
                }
            }

            free(local_path);
        }
    }

    free(path);
    return min_cost;
}
