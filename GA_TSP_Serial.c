#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

int chromo_length_serial;
int popl_size_serial;
int no_generation_serial;
float **dist_matrix_serial;
float crossover_threshold_serial = 0.8;  // Ngưỡng lai ghép

float mutation_probability_serial = 0.1;       // Xác suất để đột biến xảy ra
float swap_probability_serial = 0.4;           // Xác suất chọn `swap_mutation`
float insert_probability_serial = 0.3;         // Xác suất chọn `insert_mutation`
float reverse_probability_serial = 0.3;        // Xác suất chọn `reverse_mutation`

typedef struct {
    int *genes;
    float fitness;
} Chromosome;
Chromosome *population_serial;

// Hàm tính fitness
void calculate_fitness_serial(Chromosome *chrom) {
    float fitness = 0;
    for (int i = 0; i < chromo_length_serial - 1; i++) {
        fitness += dist_matrix_serial[chrom->genes[i]][chrom->genes[i + 1]];
    }
    fitness += dist_matrix_serial[chrom->genes[chromo_length_serial - 1]][chrom->genes[0]];
    chrom->fitness = 10 / log10(fitness);
}

// Hàm tạo quần thể ban đầu
void init_population_serial(int size) {
    chromo_length_serial = size;
    population_serial = (Chromosome *)malloc(popl_size_serial * sizeof(Chromosome));

    for (int i = 0; i < popl_size_serial; i++) {
        population_serial[i].genes = malloc(chromo_length_serial * sizeof(int));
        for (int j = 0; j < chromo_length_serial; j++) {
            population_serial[i].genes[j] = j;
        }
        // Sắp xếp ngẫu nhiên
        for (int k = 0; k < chromo_length_serial; k++) {
            int rand_pos = rand() % chromo_length_serial;
            int temp = population_serial[i].genes[k];
            population_serial[i].genes[k] = population_serial[i].genes[rand_pos];
            population_serial[i].genes[rand_pos] = temp;
        }
        calculate_fitness_serial(&population_serial[i]);  // Tính fitness cho mỗi cá thể ban đầu
    }
}

void roulette_wheel_selection(Chromosome *population, int popl_size, Chromosome *selected) {
    float total_fitness = 0;
    for (int i = 0; i < popl_size; i++) {
        total_fitness += population[i].fitness;
    }

    float pick = ((float) rand() / RAND_MAX) * total_fitness;
    float current = 0;

    for (int i = 0; i < popl_size; i++) {
        current += population[i].fitness;
        if (current >= pick) {
            *selected = population[i];
            return;
        }
    }
}

// Hàm lai ghép (crossover)
// void crossover_serial(Chromosome parent1, Chromosome parent2, Chromosome *child) {
//     int crossover_point = chromo_length_serial / 2;
//     for (int i = 0; i < crossover_point; i++) {
//         child->genes[i] = parent1.genes[i];
//     }
//     int index = crossover_point;
//     for (int i = 0; i < chromo_length_serial; i++) {
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

// Hàm lai ghép order_crossover
void order_crossover_serial(Chromosome parent1, Chromosome parent2, Chromosome *child) {
    int start = rand() % chromo_length_serial;
    int end = start + rand() % (chromo_length_serial - start);

    for (int i = 0; i < chromo_length_serial; i++) {
        child->genes[i] = -1;
    }

    for (int i = start; i <= end; i++) {
        child->genes[i] = parent1.genes[i];
    }

    int current_pos = (end + 1) % chromo_length_serial;
    for (int i = 0; i < chromo_length_serial; i++) {
        int gene = parent2.genes[(end + 1 + i) % chromo_length_serial];
        int exists = 0;
        for (int j = start; j <= end; j++) {
            if (child->genes[j] == gene) {
                exists = 1;
                break;
            }
        }

        if (!exists) {
            child->genes[current_pos] = gene;
            current_pos = (current_pos + 1) % chromo_length_serial;
        }
    }
}


// Đột biến: hoán đổi vị trí của hai gene ngẫu nhiên trong nhiễm sắc thể.
void swap_mutation_serial(Chromosome *chrom, int length) {
    int index1 = rand() % length;
    int index2 = rand() % length;
    int temp = chrom->genes[index1];
    chrom->genes[index1] = chrom->genes[index2];
    chrom->genes[index2] = temp;
}

// Đột biến: một gene được chọn và di chuyển đến một vị trí ngẫu nhiên khác trong chuỗi.
void insert_mutation_serial(Chromosome *chrom, int length) {
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
void reverse_mutation_serial(Chromosome *chrom, int length) {
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

void apply_mutation_serial(Chromosome *chrom, int length) {
    if ((float)rand() / RAND_MAX < mutation_probability_serial) {  // Kiểm tra xác suất đột biến
        float mutation_choice = (float)rand() / RAND_MAX;
        if (mutation_choice < swap_probability_serial) {
            swap_mutation_serial(chrom, length);
        } else if (mutation_choice < swap_probability_serial + insert_probability_serial) {
            insert_mutation_serial(chrom, length);
        } else {
            reverse_mutation_serial(chrom, length);
        }
    }
}



// Hàm đột biến (mutation) code cũ
// void mutation_serial(Chromosome *chrom) {
//     int index1 = rand() % chromo_length_serial;
//     int index2 = rand() % chromo_length_serial;
//     int temp = chrom->genes[index1];
//     chrom->genes[index1] = chrom->genes[index2];
//     chrom->genes[index2] = temp;
// }

// GA tuần tự, trả về best_cost và lưu đường đi tốt nhất vào best_path
float tsp_ga_serial(float **dist_matrix, int size, int **best_path) {
    chromo_length_serial = size;
    dist_matrix_serial = dist_matrix;
    popl_size_serial = 200;
    no_generation_serial = 500;

    init_population_serial(size);

    // Biến để lưu kết quả tốt nhất toàn bộ các thế hệ
    float best_overall_fitness = 0;
    int *best_overall_genes = malloc(chromo_length_serial * sizeof(int));

    for (int gen = 0; gen < no_generation_serial; gen++) {
        Chromosome *new_population = (Chromosome *)malloc(popl_size_serial * sizeof(Chromosome));
        int new_population_size = 0;

        while (new_population_size < popl_size_serial) {
            Chromosome parent1, parent2;
            roulette_wheel_selection(population_serial, popl_size_serial, &parent1);
            roulette_wheel_selection(population_serial, popl_size_serial, &parent2);

            if ((float)rand() / RAND_MAX < crossover_threshold_serial) {
                Chromosome child;
                child.genes = malloc(chromo_length_serial * sizeof(int));
                // crossover_serial(parent1, parent2, &child);
                order_crossover_serial(parent1, parent2, &child);
                // mutation_serial(&child);
                apply_mutation_serial(&child, chromo_length_serial);   // Kiểm tra và áp dụng đột biến
                calculate_fitness_serial(&child);

                new_population[new_population_size++] = child;
            } else {
                Chromosome copy = (rand() % 2) ? parent1 : parent2;
                Chromosome new_chromosome;
                new_chromosome.genes = malloc(chromo_length_serial * sizeof(int));
                for (int j = 0; j < chromo_length_serial; j++) {
                    new_chromosome.genes[j] = copy.genes[j];
                }
                new_chromosome.fitness = copy.fitness;
                new_population[new_population_size++] = new_chromosome;
            }
        }

        for (int i = 0; i < popl_size_serial; i++) {
            free(population_serial[i].genes);
        }
        free(population_serial);
        population_serial = new_population;

        float best_gen_fitness = population_serial[0].fitness;
        int *best_gen_genes = population_serial[0].genes;

        for (int i = 1; i < popl_size_serial; i++) {
            if (population_serial[i].fitness > best_gen_fitness) {
                best_gen_fitness = population_serial[i].fitness;
                best_gen_genes = population_serial[i].genes;
            }
        }

        // printf("Generation %d, Best fitness: %f\n", gen, best_gen_fitness);

        if (best_gen_fitness > best_overall_fitness) {
            best_overall_fitness = best_gen_fitness;
            for (int i = 0; i < chromo_length_serial; i++) {
                best_overall_genes[i] = best_gen_genes[i];
            }
        }
    }

    *best_path = malloc(chromo_length_serial * sizeof(int));
    for (int i = 0; i < chromo_length_serial; i++) {
        (*best_path)[i] = best_overall_genes[i];
    }

    float best_cost = pow(10, 10 / best_overall_fitness);

    free(best_overall_genes);
    for (int i = 0; i < popl_size_serial; i++) {
        free(population_serial[i].genes);
    }
    free(population_serial);

    printf("Best fitness: %f\n", best_overall_fitness);
    return best_cost;
}