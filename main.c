#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include "graph.h"

// Khai báo hàm cho các thuật toán
int tsp_bruteforce(int **graph, int size);
int tsp_bruteforce_parallel(int **graph, int size);
float tsp_ga_serial(float **dist_matrix, int size, int **best_path);
float tsp_ga_parallel(float **dist_matrix, int size, int **best_path);

#define NUM_RUNS 20  // Số lần chạy lặp lại để lấy giá trị trung bình

// Hàm chuyển đổi ma trận đồ thị thành ma trận khoảng cách cho GA
float **convert_graph_to_dist_matrix(int **graph, int size) {
    float **dist_matrix = (float **)malloc(size * sizeof(float *));
    for (int i = 0; i < size; i++) {
        dist_matrix[i] = (float *)malloc(size * sizeof(float));
        for (int j = 0; j < size; j++) {
            dist_matrix[i][j] = (float)graph[i][j];
        }
    }
    return dist_matrix;
}


// Hàm chạy thuật toán brute-force tuần tự
double run_bruteforce(int **graph, int size) {
    clock_t start = clock();
    tsp_bruteforce(graph, size);
    clock_t end = clock();
    return (double)(end - start) / CLOCKS_PER_SEC;
}

// Hàm chạy thuật toán brute-force song song
double run_bruteforce_parallel(int **graph, int size) {
    clock_t start = clock();
    tsp_bruteforce_parallel(graph, size);
    clock_t end = clock();
    return (double)(end - start) / CLOCKS_PER_SEC;
}

// Hàm chạy thuật toán GA tuần tự
double run_ga_serial(float **dist_matrix, int size) {
    int *best_path = NULL;
    clock_t start = clock();
    tsp_ga_serial(dist_matrix, size, &best_path);
    clock_t end = clock();
    free(best_path);
    return (double)(end - start) / CLOCKS_PER_SEC;
}

// Hàm chạy thuật toán GA song song
double run_ga_parallel(float **dist_matrix, int size) {
    int *best_path_parallel = NULL;
    double start = omp_get_wtime();
    tsp_ga_parallel(dist_matrix, size, &best_path_parallel);
    double end = omp_get_wtime();
    // printf("Best Path (GA Parallel): ");
    // for (int i = 0; i < size; i++) {
    //     printf("%d ", best_path_parallel[i]);
    // }
    free(best_path_parallel);
    return end - start;
}

// Hàm tính trung bình thời gian chạy của một thuật toán
double average_runtime(double times[], int num_runs) {
    double total = 0;
    for (int i = 0; i < num_runs; i++) {
        total += times[i];
    }
    return total / num_runs;
}


// Hàm ghi kết quả vào file
void save_results(FILE *file, int size, double avg_time_bruteforce, double avg_time_bruteforce_parallel, double avg_time_ga_serial, double avg_time_ga_parallel) {
    fprintf(file, "%d\t%f\t%f\t%f\t%f\n", size, avg_time_bruteforce, avg_time_bruteforce_parallel, avg_time_ga_serial, avg_time_ga_parallel);
}

// Hàm chạy và đo thời gian của các thuật toán
void run_all_algorithms(int size, FILE *file) {
    int **graph = create_graph(size);  // Random ma trận chỉ một lần
    float **dist_matrix = convert_graph_to_dist_matrix(graph, size);

    double time_bruteforce[NUM_RUNS], time_bruteforce_parallel[NUM_RUNS];
    double time_ga_serial[NUM_RUNS], time_ga_parallel[NUM_RUNS];

    // Vòng lặp chạy nhiều lần để tính trung bình
    for (int run = 0; run < NUM_RUNS; run++) {
        // Đo thời gian brute-force tuần tự
        clock_t start = clock();
        tsp_bruteforce(graph, size);
        clock_t end = clock();
        time_bruteforce[run] = (double)(end - start) / CLOCKS_PER_SEC;

        // Đo thời gian brute-force song song
        start = clock();
        tsp_bruteforce_parallel(graph, size);
        end = clock();
        time_bruteforce_parallel[run] = (double)(end - start) / CLOCKS_PER_SEC;

        // Đo thời gian GA tuần tự
        int *best_path = NULL;
        start = clock();
        tsp_ga_serial(dist_matrix, size, &best_path);
        end = clock();
        time_ga_serial[run] = (double)(end - start) / CLOCKS_PER_SEC;
        free(best_path);

        // Đo thời gian GA song song
        int *best_path_parallel = NULL;
        double omp_start = omp_get_wtime();
        tsp_ga_parallel(dist_matrix, size, &best_path_parallel);
        double omp_end = omp_get_wtime();
        time_ga_parallel[run] = omp_end - omp_start;
        free(best_path_parallel);
    }

    // Tính trung bình thời gian cho mỗi thuật toán
    double avg_time_bruteforce = average_runtime(time_bruteforce, NUM_RUNS);
    double avg_time_bruteforce_parallel = average_runtime(time_bruteforce_parallel, NUM_RUNS);
    double avg_time_ga_serial = average_runtime(time_ga_serial, NUM_RUNS);
    double avg_time_ga_parallel = average_runtime(time_ga_parallel, NUM_RUNS);

    // Ghi kết quả vào file
    save_results(file, size, avg_time_bruteforce, avg_time_bruteforce_parallel, avg_time_ga_serial, avg_time_ga_parallel);

    // Giải phóng bộ nhớ
    for (int i = 0; i < size; i++) {
        free(dist_matrix[i]);
    }
    free(dist_matrix);
    free_graph(graph, size);
}

// Hàm để so sánh thời gian của GA serial và GA parallel theo kích thước đầu vào
void compare_ga_serial_vs_parallel() {
    FILE *file = fopen("D:\\Parallel Computing\\Code C\\tsp\\ga_comparison_results.txt", "w");
    if (file == NULL) {
        printf("Unable to open file for writing results.\n");
        return;
    }

    fprintf(file, "InputSize\tGA_Serial_Time\tGA_Parallel_Time\n");

    for (int size = 200; size <= 500; size += 50) {
        double total_time_serial = 0.0;
        double total_time_parallel = 0.0;

        // Tạo một ma trận khoảng cách duy nhất cho kích thước đầu vào hiện tại
        float **dist_matrix = (float **)malloc(size * sizeof(float *));
        for (int i = 0; i < size; i++) {
            dist_matrix[i] = (float *)malloc(size * sizeof(float));
            for (int j = 0; j < size; j++) {
                dist_matrix[i][j] = (float)(rand() % 100 + 1);  // Khoảng cách ngẫu nhiên từ 1 đến 100
            }
        }

        for (int run = 0; run < NUM_RUNS; run++) {
            // Khởi tạo biến lưu best path cho mỗi thuật toán
            int *best_path_serial = NULL;
            int *best_path_parallel = NULL;

            // Đo thời gian cho GA serial
            clock_t start_serial = clock();
            tsp_ga_serial(dist_matrix, size, &best_path_serial);
            clock_t end_serial = clock();
            total_time_serial += (double)(end_serial - start_serial) / CLOCKS_PER_SEC;

            // Đo thời gian cho GA parallel
            double start_parallel = omp_get_wtime();
            tsp_ga_parallel(dist_matrix, size, &best_path_parallel);
            double end_parallel = omp_get_wtime();
            total_time_parallel += end_parallel - start_parallel;

            // Giải phóng bộ nhớ cho best path
            free(best_path_serial);
            free(best_path_parallel);
        }

        // Tính thời gian trung bình cho serial và parallel
        double avg_time_serial = total_time_serial / NUM_RUNS;
        double avg_time_parallel = total_time_parallel / NUM_RUNS;

        // Ghi kết quả trung bình vào file
        fprintf(file, "%d\t%f\t%f\n", size, avg_time_serial, avg_time_parallel);
        printf("Size: %d, Avg GA Serial Time: %f, Avg GA Parallel Time: %f\n", size, avg_time_serial, avg_time_parallel);

        // Giải phóng bộ nhớ cho ma trận khoảng cách
        for (int i = 0; i < size; i++) {
            free(dist_matrix[i]);
        }
        free(dist_matrix);
    }

    fclose(file);
}

int main() {
    // FILE *file = fopen("D:\\Parallel Computing\\Code C\\tsp\\test.txt", "w");
    // if (file == NULL) {
    //     printf("Error opening file!\n");
    //     return 1;
    // }

    // fprintf(file, "Vertices\tBruteForce\tBruteForceParallel\tGASerial\tGAParallel\n");

    // // Vòng lặp cho các số đỉnh từ 9 đến 12
    // for (int size = 9; size <= 12; size++) {
    //     run_all_algorithms(size, file);
    // }

    // fclose(file);
    // printf("Results saved to results.txt\n");


    // So sánh 2 thuật toán GA
    compare_ga_serial_vs_parallel();
    return 0;
}