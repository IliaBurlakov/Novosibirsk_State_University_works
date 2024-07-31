#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include <stdlib.h>


void dfs(int vert, char** adjacency_matrix, int num_of_vertices, short* state, short* stack, int* stack_index) { //deep-first search
	state[vert] = 1; //visiting
	for (int i = 0; i < num_of_vertices; i++) {
		if (adjacency_matrix[vert][i] == 1) {
			if (state[i] == 0)
				dfs(i, adjacency_matrix, num_of_vertices, state, stack, stack_index);
			else if (state[i] == 1) {
				printf("impossible to sort");
				exit(0);
			}
		}
	}
	state[vert] = 2; //visited
	stack[(*stack_index)++] = vert;
}


void TopologicalSort(char** adjacency_matrix, short* stack, int* stack_index, int num_of_vertices) {
	short* state = (short*)calloc(num_of_vertices, sizeof(short)); // not_visited - 0, visiting - 1, visited - 2
	for (int i = 0; i < num_of_vertices; i++) {
		if (state[i] == 0) {
			dfs(i, adjacency_matrix, num_of_vertices, state, stack, stack_index);
		}
	}
	free(state);
}


int main(void) {
	FILE* file = fopen("in.txt", "r");
	if (file == NULL)
		return 0;
	int num_of_vertices;
	int num_of_edges;
	if (fscanf(file, "%d \n%d", &num_of_vertices, &num_of_edges) != 2) {
		printf("bad number of lines");
		fclose(file);
		return 0;
	}
	if ((num_of_vertices < 0) || (num_of_vertices > 2000)) {
		printf("bad number of vertices");
		fclose(file);
		return 0;
	}
	if ((num_of_edges < 0) || (num_of_edges > num_of_vertices * ((num_of_vertices + 1) / 2))) {
		printf("bad number of edges");
		fclose(file);
		return 0;
	}

	char** adjacency_matrix = (char**)malloc(sizeof(char*) * num_of_vertices);
	for (int i = 0; i < num_of_vertices; i++)
		adjacency_matrix[i] = (char*)calloc(num_of_vertices, sizeof(char));

	for (int i = 0; i < num_of_edges; i++) {
		int from;
		int to;
		if (fscanf(file, "%d %d", &from, &to) != 2) {
			printf("bad number of lines");
			for (int i = 0; i < num_of_vertices; i++)
				free(adjacency_matrix[i]);
			free(adjacency_matrix);
			fclose(file);
			return 0;
		}
		if ((from < 1) || (from > num_of_vertices) || (to < 1) || (to > num_of_vertices)) {
			printf("bad vertex");
			for (int i = 0; i < num_of_vertices; i++)
				free(adjacency_matrix[i]);
			free(adjacency_matrix);
			fclose(file);
			return 0;
		}
		adjacency_matrix[from - 1][to - 1] = 1;
	}
	short* stack = (short*)malloc(sizeof(short) * num_of_vertices);
	int stack_index = 0;
	TopologicalSort(adjacency_matrix, stack, &stack_index, num_of_vertices);
	while (stack_index > 0)
		printf("%d ", stack[--stack_index] + 1);

	fclose(file);
	for (int i = 0; i < num_of_vertices; i++)
		free(adjacency_matrix[i]);
	free(adjacency_matrix);
	free(stack);
	return 0;
}
