#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include "graph.h"
#include <windows.h>
#include <math.h>
#include <string.h>

// Khai báo hàm cho các thuật toán
int tsp_bruteforce(int **graph, int size);
int tsp_bruteforce_parallel(int **graph, int size);
float tsp_ga_serial(float **dist_matrix, int size, int **best_path);
float tsp_ga_parallel(float **dist_matrix, int size, int **best_path);

#define NUM_RUNS 5  // Số lần chạy lặp lại để lấy giá trị trung bình
#define INITIAL_LINE_LENGTH 128  // Chiều dài dòng ban đầu

// Hàm tính khoảng cách giữa hai thành phố
float calculate_distance(float x1, float y1, float x2, float y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

// Định nghĩa hàm getline thay thế cho Windows
int my_getline(char **lineptr, size_t *n, FILE *stream) {
    if (*lineptr == NULL || *n == 0) {
        *n = 128;  // Kích thước ban đầu của bộ đệm
        *lineptr = (char *)malloc(*n);
        if (*lineptr == NULL) {
            return -1;
        }
    }

    char *ptr = *lineptr;
    int c;
    size_t i = 0;

    while ((c = fgetc(stream)) != EOF) {
        // Tăng kích thước bộ đệm nếu cần
        if (i + 1 >= *n) {
            *n *= 2;
            char *new_ptr = (char *)realloc(*lineptr, *n);
            if (new_ptr == NULL) {
                return -1;
            }
            ptr = *lineptr = new_ptr;
        }
        ptr[i++] = (char)c;
        if (c == '\n') {
            break;
        }
    }

    if (i == 0 && c == EOF) {
        return -1;  // Đạt tới EOF mà không đọc được ký tự nào
    }

    ptr[i] = '\0';
    return i;
}

// Hàm đọc file TSP và tạo ma trận khoảng cách
float **load_tsp_file(const char *filename, int *num_cities) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        return NULL;
    }

    size_t line_size = 128;
    char *line = (char *)malloc(line_size * sizeof(char));
    float **dist_matrix;
    float coordinates[1000][2];  // Mảng tạm để lưu tọa độ các thành phố (giả định tối đa 1000 thành phố)

    // Đọc số thành phố từ dòng DIMENSION
    while (my_getline(&line, &line_size, file) != -1) {
        if (strstr(line, "DIMENSION") != NULL) {
            sscanf(line, "%*[^0123456789]%d", num_cities);  // Lấy số nguyên đầu tiên xuất hiện trong dòng
            printf("Read DIMENSION: %d from file %s\n", *num_cities, filename);  // In ra để kiểm tra
            break;
        }
    }

    // Đọc đến phần tọa độ của các thành phố
    while (my_getline(&line, &line_size, file) != -1) {
        if (strncmp(line, "NODE_COORD_SECTION", 18) == 0) {
            break;
        }
    }

    // Đọc tọa độ các thành phố
    int city_count = 0;
    while (my_getline(&line, &line_size, file) != -1) {
        if (strncmp(line, "EOF", 3) == 0) {
            break;
        }
        int city_index;
        float x, y;
        sscanf(line, "%d %f %f", &city_index, &x, &y);
        coordinates[city_count][0] = x;
        coordinates[city_count][1] = y;
        city_count++;
    }

    fclose(file);
    free(line);

    // Tạo ma trận khoảng cách dựa trên tọa độ
    dist_matrix = (float **)malloc(*num_cities * sizeof(float *));
    for (int i = 0; i < *num_cities; i++) {
        dist_matrix[i] = (float *)malloc(*num_cities * sizeof(float));
        for (int j = 0; j < *num_cities; j++) {
            dist_matrix[i][j] = calculate_distance(
                coordinates[i][0], coordinates[i][1],
                coordinates[j][0], coordinates[j][1]
            );
        }
    }

    return dist_matrix;
}

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

// Hàm đo thời gian chạy của GA song song theo số luồng
void measure_ga_parallel_time_by_threads() {
    // Số luồng để thử nghiệm
    int thread_counts[] = {2, 4, 6, 8, 12, 16};
    int num_thread_counts = sizeof(thread_counts) / sizeof(thread_counts[0]);

    // Mở file để ghi kết quả
    FILE *file = fopen("D:\\Parallel Computing\\Code C\\tsp\\ga_parallel_time_by_threads.txt", "w");
    if (file == NULL) {
        printf("Unable to open file for writing results.\n");
        return;
    }

    // Ghi tiêu đề cột
    fprintf(file, "InputSize\tThreads\tAvgTime\n");

    // Duyệt qua từng kích thước đầu vào từ 200 đến 400 với bước nhảy 50
    for (int size = 500; size <= 1500; size += 500) {
        // Tạo một ma trận khoảng cách duy nhất cho mỗi kích thước đầu vào
        // Tạo một ma trận khoảng cách duy nhất cho kích thước đầu vào hiện tại
        float **dist_matrix = (float **)malloc(size * sizeof(float *));
        for (int i = 0; i < size; i++) {
            dist_matrix[i] = (float *)malloc(size * sizeof(float));
            for (int j = 0; j < size; j++) {
                dist_matrix[i][j] = (float)(rand() % 100 + 1);  // Khoảng cách ngẫu nhiên từ 1 đến 100
            }
        }

        // Duyệt qua từng số luồng
        for (int t = 0; t < num_thread_counts; t++) {
            int num_threads = thread_counts[t];
            omp_set_num_threads(num_threads);

            double total_time_parallel = 0.0;

            // Chạy GA song song nhiều lần để lấy thời gian trung bình
            for (int run = 0; run < NUM_RUNS; run++) {
                int *best_path_parallel = NULL;

                double start_parallel = omp_get_wtime();
                tsp_ga_parallel(dist_matrix, size, &best_path_parallel);
                double end_parallel = omp_get_wtime();

                total_time_parallel += end_parallel - start_parallel;
                free(best_path_parallel);
            }

            // Tính thời gian trung bình cho số luồng hiện tại
            double avg_time_parallel = total_time_parallel / NUM_RUNS;

            // Ghi kết quả vào file
            fprintf(file, "%d\t%d\t%f\n", size, num_threads, avg_time_parallel);
            printf("Size: %d, Threads: %d, Avg GA Parallel Time: %f\n", size, num_threads, avg_time_parallel);
        }

        // Giải phóng bộ nhớ cho ma trận khoảng cách và đồ thị
        for (int i = 0; i < size; i++) {
            free(dist_matrix[i]);
        }
        free(dist_matrix);
    }

    fclose(file);
}

// Hàm đo lường tỉ lệ tăng tốc giữa GA tuần tự và song song với số luồng khác nhau
void measure_ga_speedup_by_threads() {
    int thread_counts[] = {2, 4, 6, 8, 12, 16};
    int num_thread_counts = sizeof(thread_counts) / sizeof(thread_counts[0]);
    int sizes[] = {100, 200, 500, 1000};  // Các kích thước đồ thị cần kiểm tra
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    // Mở file để ghi kết quả
    FILE *file = fopen("D:\\Parallel Computing\\Code C\\tsp\\ga_speedup_by_threads.txt", "w");
    if (file == NULL) {
        printf("Unable to open file for writing results.\n");
        return;
    }

    // Ghi tiêu đề cột
    fprintf(file, "InputSize\tThreads\tSpeedup\n");

    for (int s = 0; s < num_sizes; s++) {
        int size = sizes[s];

        // Tạo đồ thị và ma trận khoảng cách cho kích thước hiện tại
        float **dist_matrix = (float **)malloc(size * sizeof(float *));
        for (int i = 0; i < size; i++) {
            dist_matrix[i] = (float *)malloc(size * sizeof(float));
            for (int j = 0; j < size; j++) {
                dist_matrix[i][j] = (float)(rand() % 100 + 1);  // Khoảng cách ngẫu nhiên từ 1 đến 100
            }
        }

        // Đo thời gian chạy GA tuần tự
        double serial_time = 0.0;
        for (int run = 0; run < NUM_RUNS; run++) {
            serial_time += run_ga_serial(dist_matrix, size);  // Tái sử dụng hàm `run_ga_serial`
        }
        serial_time /= NUM_RUNS;  // Tính thời gian trung bình

        // Duyệt qua từng số luồng và tính tỉ lệ tăng tốc
        for (int t = 0; t < num_thread_counts; t++) {
            int num_threads = thread_counts[t];
            omp_set_num_threads(num_threads);  // Thiết lập số luồng cho OpenMP

            double parallel_time = 0.0;

            // Đo thời gian chạy GA song song với số luồng `num_threads`
            for (int run = 0; run < NUM_RUNS; run++) {
                parallel_time += run_ga_parallel(dist_matrix, size);  // Tái sử dụng hàm `run_ga_parallel`
            }
            parallel_time /= NUM_RUNS;  // Tính thời gian trung bình

            // Tính tỉ lệ tăng tốc
            double speedup = serial_time / parallel_time;

            // Ghi kết quả vào file
            fprintf(file, "%d\t%d\t%f\n", size, num_threads, speedup);
            printf("Size: %d, Threads: %d, Speedup: %f\n", size, num_threads, speedup);
        }

        // Giải phóng bộ nhớ
        for (int i = 0; i < size; i++) {
            free(dist_matrix[i]);
        }
        free(dist_matrix);
    }

    fclose(file);
}



// Hàm chạy và so sánh GA tuần tự và GA song song với một file đầu vào
void process_tsp_file(const char *filepath, FILE *output_file) {
    int num_cities;
    float **dist_matrix = load_tsp_file(filepath, &num_cities);

    if (dist_matrix == NULL) {
        printf("Error loading TSP file %s.\n", filepath);
        return;
    }

    double serial_time = 0.0;
    double parallel_time = 0.0;

    // Chạy GA tuần tự và song song, lấy thời gian trung bình
    for (int run = 0; run < NUM_RUNS; run++) {
        serial_time += run_ga_serial(dist_matrix, num_cities);
        parallel_time += run_ga_parallel(dist_matrix, num_cities);
    }
    serial_time /= NUM_RUNS;
    parallel_time /= NUM_RUNS;

    // Ghi kết quả vào file output
    fprintf(output_file, "%d\t%f\t%f\n", num_cities, serial_time, parallel_time);
    printf("Processed %s: Size %d, Serial Time %f, Parallel Time %f\n", filepath, num_cities, serial_time, parallel_time);

    for (int i = 0; i < num_cities; i++) {
        free(dist_matrix[i]);
    }
    free(dist_matrix);
}

// Hàm duyệt thư mục và xử lý từng file trong đó
void process_directory(const char *directory_path, const char *output_filename) {
    WIN32_FIND_DATA find_data;
    HANDLE h_find;

    // Tạo đường dẫn đến tất cả các file trong thư mục (dùng ký tự đại diện *)
    char search_path[1024];
    snprintf(search_path, sizeof(search_path), "%s\\*.tsp", directory_path);

    h_find = FindFirstFile(search_path, &find_data);
    if (h_find == INVALID_HANDLE_VALUE) {
        printf("Error opening directory %s\n", directory_path);
        return;
    }

    FILE *output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        printf("Error opening output file %s\n", output_filename);
        FindClose(h_find);
        return;
    }

    // Ghi tiêu đề cột vào file kết quả
    fprintf(output_file, "InputSize\tGA_Serial_Time\tGA_Parallel_Time\n");

    // Lặp qua tất cả các file trong thư mục
    do {
        if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            char filepath[1024];
            snprintf(filepath, sizeof(filepath), "%s\\%s", directory_path, find_data.cFileName);

            // Xử lý file TSP
            process_tsp_file(filepath, output_file);
        }
    } while (FindNextFile(h_find, &find_data) != 0);

    fclose(output_file);
    FindClose(h_find);
}


int main() {

    //So sánh 4 thuật toán 
    /*
    FILE *file = fopen("D:\\Parallel Computing\\Code C\\tsp\\test.txt", "w");
    if (file == NULL) {
        printf("Error opening file!\n");
        return 1;
    }

    fprintf(file, "Vertices\tBruteForce\tBruteForceParallel\tGASerial\tGAParallel\n");

    // Vòng lặp cho các số đỉnh từ 9 đến 12
    for (int size = 9; size <= 12; size++) {
        run_all_algorithms(size, file);
    }

    fclose(file);
    printf("Results saved to results.txt\n");
    */


    // So sánh 2 thuật toán GA
    // compare_ga_serial_vs_parallel();


    // So sánh tác động của số luồng đến thời gian chạy theo các kích thước đầu vào
    // measure_ga_parallel_time_by_threads();

    // đo lường tỉ lệ tăng tốc giữa GA tuần tự và song song với số luồng khác nhau
    // measure_ga_speedup_by_threads();

    //Test với file .tsp đầu vào
    const char *input_directory = "D:\\Parallel Computing\\Code C\\tsp\\data";  // Đường dẫn tới thư mục chứa các file TSP
    const char *output_filename = "ga_comparison_results_with_inputfile.txt";  // Tên file kết quả

    process_directory(input_directory, output_filename);

    return 0;
}