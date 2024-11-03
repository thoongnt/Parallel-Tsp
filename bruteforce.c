#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>

#define MAXN 16
#define INF INT_MAX

// Hàm đổi chỗ hai phần tử
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Hàm sắp xếp tăng dần các phần tử từ vị trí start đến end
void reverse(int arr[], int start, int end) {
    while (start < end) {
        swap(&arr[start], &arr[end]);
        start++;
        end--;
    }
}

// Hàm next_permutation
bool next_permutation(int arr[], int n) {
    int i = n - 2;
    while (i >= 0 && arr[i] >= arr[i + 1]) i--;

    if (i < 0) return false;

    int j = n - 1;
    while (arr[j] <= arr[i]) j--;

    swap(&arr[i], &arr[j]);
    reverse(arr, i + 1, n - 1);
    return true;
}

// Hàm sinh trọng số ngẫu nhiên cho các cạnh trong ma trận trọng số
void assign_edge_weights(int **matrix, int n) {
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            matrix[i][j] = rand() % 10 + 1;
            matrix[j][i] = matrix[i][j];
        }
        matrix[i][i] = 0;
    }
}

// Hàm tính chi phí của một đường đi trong ma trận trọng số
int find_path_cost(int **matrix, int *arr, int n) {
    int cost = 0;
    for (int i = 1; i < n; i++) {
        cost += matrix[arr[i - 1]][arr[i]];
    }
    cost += matrix[arr[n - 1]][arr[0]]; // Quay về điểm đầu
    return cost;
}

// Hàm TSP vét cạn
int tsp_bruteforce(int **matrix, int n) {
    int optimal_value = INF;
    int *nodes = malloc((n - 1) * sizeof(int));

    for (int i = 0; i < n - 1; i++) {
        nodes[i] = i + 1;
    }

    do {
        int temp[MAXN];
        temp[0] = 0;
        for (int i = 1; i < n; i++) {
            temp[i] = nodes[i - 1];
        }

        int val = find_path_cost(matrix, temp, n);
        if (val < optimal_value) {
            optimal_value = val;
        }
    } while (next_permutation(nodes, n - 1));

    free(nodes);
    return optimal_value;
}

// Hàm in ma trận trọng số
void print_matrix(int **matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

// Hàm chính
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Vui lòng cung cấp số lượng thành phố.\n");
        return 1;
    }

    int n = atoi(argv[1]);
    if (n > MAXN || n < 2) {
        printf("Số lượng thành phố phải nằm trong khoảng từ 2 đến %d.\n", MAXN);
        return 1;
    }

    // Khởi tạo ma trận trọng số
    int **matrix = malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++) {
        matrix[i] = malloc(n * sizeof(int));
    }

    assign_edge_weights(matrix, n);
    // print_matrix(matrix, n); // In ma trận trọng số nếu cần kiểm tra

    clock_t start = clock();
    int optimal_cost = tsp_bruteforce(matrix, n);
    clock_t end = clock();

    printf("Chi phí đường đi ngắn nhất: %d\n", optimal_cost);
    printf("Thời gian thực hiện: %.5f giây\n", (double)(end - start) / CLOCKS_PER_SEC);

    for (int i = 0; i < n; i++) {
        free(matrix[i]);
    }
    free(matrix);

    return 0;
}
