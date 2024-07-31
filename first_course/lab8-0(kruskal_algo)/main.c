#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>


struct st_Edge {
    int vertex1;
    int vertex2;
    int length;
};
typedef struct st_Edge Edge;


struct st_Correct_Pair {
    int vertex1;
    int vertex2;
};
typedef struct st_Correct_Pair Correct_Pair;


int CompareFunction(Edge* edge1, Edge* edge2) {
    return edge1->length - edge2->length;
}


int FindRoot(int* set, int vertex) {
    if (set[vertex] == vertex)                //vertex is it's own root
        return vertex;
    set[vertex] = FindRoot(set, set[vertex]); //recursive root search
    return set[vertex];
}


int Kruskal(Edge* graph, int num_of_vertices, int num_of_edges, Correct_Pair* result) {
    qsort(graph, num_of_edges, sizeof(Edge), (int (*)(const void*, const void*)) CompareFunction);
    int* set = malloc(sizeof(int) * num_of_vertices);
    if (set == NULL) {
        return 0;
    }
    for (int i = 0; i < num_of_vertices; i++) //each vertex is a set  
        set[i] = i;
    int index = 0;
    for (int i = 0; i < num_of_edges; i++) {
        int root1 = FindRoot(set, graph[i].vertex1 - 1);
        int root2 = FindRoot(set, graph[i].vertex2 - 1);
        if (root1 != root2) {
            set[root2] = root1;               //unite sets
            result[index].vertex1 = graph[i].vertex1;
            result[index].vertex2 = graph[i].vertex2;
            index++;
        }
    }
    free(set);
    return (index < num_of_vertices - 1) ? 0 : 1;
}


int main(void) {
    FILE* file = fopen("in.txt", "r");
    if (file == NULL)
        return 0;
    int num_of_vertices;
    int num_of_edges;
    if (fscanf(file, "%d \n%d", &num_of_vertices, &num_of_edges) != 2) {
        fclose(file);
        return 0;
    }
    if ((num_of_vertices < 0) || (num_of_vertices > 5000)) {
        printf("bad number of vertices");
        fclose(file);
        return 0;
    }
    if ((num_of_edges < 0) || (num_of_edges > num_of_vertices * (num_of_vertices - 1) / 2)) {
        printf("bad number of edges");
        fclose(file);
        return 0;
    }
    if (num_of_vertices == 0 || (num_of_vertices > 1 && num_of_edges == 0)) {
        printf("no spanning tree");
        fclose(file);
        return 0;
    }
    if (num_of_edges == 0) {
        fclose(file);
        return 0;
    }
    Edge* graph = malloc(sizeof(Edge) * num_of_edges);
    if (graph == NULL) {
        fclose(file);
        return 0;
    }
    for (int i = 0; i < num_of_edges; i++) {
        int vertex1;
        int vertex2;
        long long int length;
        if (fscanf(file, "%d %d %lld", &vertex1, &vertex2, &length) != 3) {
            printf("bad number of lines");
            free(graph);
            fclose(file);
            return 0;
        }
        if (vertex1 < 1 || vertex1 > num_of_vertices || vertex2 < 1 || vertex2 > num_of_vertices) {
            printf("bad vertex");
            free(graph);
            fclose(file);
            return 0;
        }
        if (length < 1 || length > INT_MAX) {
            free(graph);
            printf("bad length");
            fclose(file);
            return 0;
        }
        graph[i].vertex1 = vertex1;
        graph[i].vertex2 = vertex2;
        graph[i].length = length;
    }
    Correct_Pair* result = malloc(sizeof(Correct_Pair) * (num_of_vertices - 1));
    if (result == NULL) {
        free(graph);
        fclose(file);
        return 0;
    }
    if (!Kruskal(graph, num_of_vertices, num_of_edges, result))
        printf("no spanning tree");
    else {
        for (int i = 0; i < (num_of_vertices - 1); i++)
            printf("%d %d\n", result[i].vertex1, result[i].vertex2);
    }
    free(graph);
    free(result);
    fclose(file);
    return 0;
}
