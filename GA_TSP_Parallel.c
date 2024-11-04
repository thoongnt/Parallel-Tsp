#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>

int chromo_length_parallel;
int popl_size_parallel;
int no_generation_parallel;
float **dist_matrix_parallel;
float crossover_threshold_parallel = 0.7;

float mutation_probability_parallel = 0.05;       // Xác suất để đột biến xảy ra
float swap_probability_parallel = 0.4;           // Xác suất chọn `swap_mutation_parallel`
float insert_probability_parallel = 0.3;         // Xác suất chọn `insert_mutation_parallel`
float reverse_probability_parallel = 0.3;        // Xác suất chọn `reverse_mutation_parallel`

typedef struct {
    int *genes;
    float fitness;
} Chromosome;
Chromosome *population_parallel;

// Hàm tính fitness
void calculate_fitness_parallel(Chromosome *chrom) {
    float fitness = 0;
    for (int i = 0; i < chromo_length_parallel - 1; i++) {
        fitness += dist_matrix_parallel[chrom->genes[i]][chrom->genes[i + 1]];
    }
    fitness += dist_matrix_parallel[chrom->genes[chromo_length_parallel - 1]][chrom->genes[0]];
    chrom->fitness = 10 / log10(fitness);
}

// Hàm tạo quần thể ban đầu
void init_population_parallel(int size) {
    chromo_length_parallel = size;
    population_parallel = (Chromosome *)malloc(popl_size_parallel * sizeof(Chromosome));

    #pragma omp parallel for
    for (int i = 0; i < popl_size_parallel; i++) {
        population_parallel[i].genes = malloc(chromo_length_parallel * sizeof(int));
        for (int j = 0; j < chromo_length_parallel; j++) {
            population_parallel[i].genes[j] = j;
        }
        for (int k = 0; k < chromo_length_parallel; k++) {
            int rand_pos = rand() % chromo_length_parallel;
            int temp = population_parallel[i].genes[k];
            population_parallel[i].genes[k] = population_parallel[i].genes[rand_pos];
            population_parallel[i].genes[rand_pos] = temp;
        }
        calculate_fitness_parallel(&population_parallel[i]);
    }
}

// Hàm chọn lựa theo Roulette Wheel Selection
void roulette_wheel_selection_parallel(Chromosome *population, int popl_size, Chromosome *selected) {
    float total_fitness = 0;

    // Tính tổng fitness của toàn bộ quần thể
    #pragma omp parallel for reduction(+:total_fitness)
    for (int i = 0; i < popl_size; i++) {
        total_fitness += population[i].fitness;
    }

    float pick = ((float)rand() / RAND_MAX) * total_fitness;
    float current = 0;
    int found = 0; // Biến cờ để đánh dấu

    #pragma omp parallel for shared(found)
    for (int i = 0; i < popl_size; i++) {
        if (!found) { // Chỉ tiếp tục nếu chưa tìm thấy
            #pragma omp atomic
            current += population[i].fitness;

            if (current >= pick && !found) {
                #pragma omp critical
                {
                    if (!found) { // Kiểm tra lại trong vùng an toàn
                        *selected = population[i];
                        found = 1; // Đánh dấu là đã tìm thấy
                    }
                }
            }
        }
    }
}



// Hàm lai ghép (crossover)
// void crossover_parallel(Chromosome parent1, Chromosome parent2, Chromosome *child) {
//     int crossover_point = chromo_length_parallel / 2;
//     for (int i = 0; i < crossover_point; i++) {
//         child->genes[i] = parent1.genes[i];
//     }
//     int index = crossover_point;
//     for (int i = 0; i < chromo_length_parallel; i++) {
//         int gene = parent2.genes[i];
//         int exists = 0;
//         for (int j = 0; j < crossover_point; j++) {
//             if (child->genes[j] == gene) {
//                 exists = 1;
//                 break;
//             }
//         }
//         if (!exists) {
//             child->genes[index++] = gene;
//         }
//     }
// }

// Hàm lai ghép
void order_crossover_parallel(Chromosome parent1, Chromosome parent2, Chromosome *child) {
    int start = rand() % chromo_length_parallel;
    int end = start + rand() % (chromo_length_parallel - start);

    for (int i = 0; i < chromo_length_parallel; i++) {
        child->genes[i] = -1;
    }

    for (int i = start; i <= end; i++) {
        child->genes[i] = parent1.genes[i];
    }

    int current_pos = (end + 1) % chromo_length_parallel;
    for (int i = 0; i < chromo_length_parallel; i++) {
        int gene = parent2.genes[(end + 1 + i) % chromo_length_parallel];
        int exists = 0;
        for (int j = start; j <= end; j++) {
            if (child->genes[j] == gene) {
                exists = 1;
                break;
            }
        }

        if (!exists) {
            child->genes[current_pos] = gene;
            current_pos = (current_pos + 1) % chromo_length_parallel;
        }
    }
}

// Đột biến: hoán đổi vị trí của hai gene ngẫu nhiên trong nhiễm sắc thể.
void swap_mutation_parallel(Chromosome *chrom, int length) {
    int index1 = rand() % length;
    int index2 = rand() % length;
    int temp = chrom->genes[index1];
    chrom->genes[index1] = chrom->genes[index2];
    chrom->genes[index2] = temp;
}

// Đột biến: một gene được chọn và di chuyển đến một vị trí ngẫu nhiên khác trong chuỗi.
void insert_mutation_parallel(Chromosome *chrom, int length) {
    int index1 = rand() % length;
    int index2 = rand() % length;

    int gene = chrom->genes[index1];
    if (index1 < index2) {
        for (int i = index1; i < index2; i++) {
            chrom->genes[i] = chrom->genes[i + 1];
        }
    } else if (index1 > index2) {
        for (int i = index1; i > index2; i--) {
            chrom->genes[i] = chrom->genes[i - 1];
        }
    }
    chrom->genes[index2] = gene;
}

// Đột biến: đảo ngược thứ tự của một đoạn ngẫu nhiên trong chuỗi gene.
void reverse_mutation_parallel(Chromosome *chrom, int length) {
    int start = rand() % length;
    int end = start + rand() % (length - start);

    while (start < end) {
        int temp = chrom->genes[start];
        chrom->genes[start] = chrom->genes[end];
        chrom->genes[end] = temp;
        start++;
        end--;
    }
}

// Hàm đột biến
void apply_mutation_parallel(Chromosome *chrom, int length) {
    if ((float)rand() / RAND_MAX < mutation_probability_parallel) {
        float mutation_choice = (float)rand() / RAND_MAX;
        if (mutation_choice < swap_probability_parallel) {
            swap_mutation_parallel(chrom, length);
        } else if (mutation_choice < swap_probability_parallel + insert_probability_parallel) {
            insert_mutation_parallel(chrom, length);
        } else {
            reverse_mutation_parallel(chrom, length);
        }
    }
}



// Hàm đột biến (mutation) code cũ
// void mutation_parallel(Chromosome *chrom) {
//     int index1 = rand() % chromo_length_parallel;
//     int index2 = rand() % chromo_length_parallel;
//     int temp = chrom->genes[index1];
//     chrom->genes[index1] = chrom->genes[index2];
//     chrom->genes[index2] = temp;
// }

// GA song song, trả về best_cost và lưu đường đi tốt nhất vào best_path
float tsp_ga_parallel(float **dist_matrix, int size, int **best_path) {
    chromo_length_parallel = size;
    dist_matrix_parallel = dist_matrix;
    popl_size_parallel = size;
    no_generation_parallel = 1000;

    init_population_parallel(size);

    float best_overall_fitness = 0;
    int *best_overall_genes = malloc(chromo_length_parallel * sizeof(int));

    for (int gen = 0; gen < no_generation_parallel; gen++) {
        Chromosome *new_population = (Chromosome *)malloc(popl_size_parallel * sizeof(Chromosome));
        int new_population_size = 0;

        #pragma omp parallel
        {
            Chromosome parent1, parent2;
            Chromosome child;

            #pragma omp for
            for (int i = 0; i < popl_size_parallel; i++) {
                // Chọn hai cá thể cha mẹ bằng phương pháp roulette
                roulette_wheel_selection_parallel(population_parallel, popl_size_parallel, &parent1);
                roulette_wheel_selection_parallel(population_parallel, popl_size_parallel, &parent2);

                if ((float)rand() / RAND_MAX < crossover_threshold_parallel) {
                    // Thực hiện lai ghép và đột biến nếu vượt ngưỡng lai ghép
                    child.genes = malloc(chromo_length_parallel * sizeof(int));
                    // crossover_parallel(parent1, parent2, &child);
                    order_crossover_parallel(parent1, parent2, &child);
                    // mutation_parallel(&child);
                    apply_mutation_parallel(&child, chromo_length_parallel);  // Kiểm tra và áp dụng đột biến
                    calculate_fitness_parallel(&child);

                    #pragma omp critical
                    new_population[new_population_size++] = child;
                } else {
                    // Nếu không lai ghép, sao chép một trong hai cha mẹ vào quần thể mới
                    Chromosome copy = (rand() % 2) ? parent1 : parent2;
                    Chromosome new_chromosome;
                    new_chromosome.genes = malloc(chromo_length_parallel * sizeof(int));
                    for (int j = 0; j < chromo_length_parallel; j++) {
                        new_chromosome.genes[j] = copy.genes[j];
                    }
                    new_chromosome.fitness = copy.fitness;

                    #pragma omp critical
                    new_population[new_population_size++] = new_chromosome;
                }
            }
        }

        #pragma omp parallel for
        for (int i = 0; i < popl_size_parallel; i++) {
            free(population_parallel[i].genes);
        }
        free(population_parallel);
        population_parallel = new_population;

        float best_gen_fitness = population_parallel[0].fitness;
        int *best_gen_genes = population_parallel[0].genes;

        #pragma omp parallel for
        for (int i = 1; i < popl_size_parallel; i++) {
            if (population_parallel[i].fitness > best_gen_fitness) {
                #pragma omp critical
                {
                    best_gen_fitness = population_parallel[i].fitness;
                    best_gen_genes = population_parallel[i].genes;
                }
            }
        }

        // printf("Generation %d, Best fitness: %f\n", gen, best_gen_fitness);

        if (best_gen_fitness > best_overall_fitness) {
            best_overall_fitness = best_gen_fitness;
            #pragma omp parallel for
            for (int i = 0; i < chromo_length_parallel; i++) {
                best_overall_genes[i] = best_gen_genes[i];
            }
        }
    }

    *best_path = best_overall_genes;
    float best_cost = pow(10, 10 / best_overall_fitness);

    for (int i = 0; i < popl_size_parallel; i++) {
        free(population_parallel[i].genes);
    }
    free(population_parallel);

    return best_cost;
}