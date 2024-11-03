#ifndef GRAPH_H
#define GRAPH_H

int **create_graph(int size);
void free_graph(int **graph, int size);
int tsp_bruteforce(int **graph, int size);
int tsp_bruteforce_parallel(int **graph, int size);

#endif
