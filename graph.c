#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "graph.h"

// Hàm khởi tạo đồ thị với trọng số ngẫu nhiên
int **create_graph(int size) {
    int **graph = (int **)malloc(size * sizeof(int *));
    for (int i = 0; i < size; i++) {
        graph[i] = (int *)malloc(size * sizeof(int));
    }

    srand(time(0)); // Khởi tạo seed cho số ngẫu nhiên
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (i == j) {
                graph[i][j] = 0;
            } else {
                graph[i][j] = rand() % 100 + 1; // Trọng số ngẫu nhiên từ 1 đến 100
            }
        }
    }
    return graph;
}

// Hàm giải phóng bộ nhớ của đồ thị
void free_graph(int **graph, int size) {
    for (int i = 0; i < size; i++) {
        free(graph[i]);
    }
    free(graph);
}

// Hàm in ra ma trận đồ thị
void print_graph(int **graph, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            printf("%d ", graph[i][j]);
        }
        printf("\n");
    }
}
