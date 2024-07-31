#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>


struct st_Correct_Pair {
    int vert1;
    int vert2;
};
typedef struct st_Correct_Pair Correct_Pair;


void FreeGraph(int** graph, short n) {
    for (short i = 0; i < n; i++) {
        free(graph[i]);
    }
    free(graph);
}


int Prim(int** graph, short n, Correct_Pair* result) {
    long long* key = malloc(n * sizeof(long long));
    int* parent = malloc(n * sizeof(int));
    bool* inMST = malloc(n * sizeof(bool));

    if (key == NULL || parent == NULL || inMST == NULL) {
        free(key);
        free(parent);
        free(inMST);
        return 0;
    }

    for (int i = 0; i < n; i++) {
        key[i] = LLONG_MAX;
        inMST[i] = false;
        parent[i] = -1;
    }

    key[0] = 0;
    parent[0] = -1;

    for (int count = 0; count < n - 1; count++) {
        int u = -1;
        long long minKey = LLONG_MAX;

        for (int v = 0; v < n; v++) {
            if (!inMST[v] && key[v] < minKey) {
                minKey = key[v];
                u = v;
            }
        }
        if (u == -1) {
            free(key);
            free(parent);
            free(inMST);
            return 0;
        }
        inMST[u] = true;

        for (int v = 0; v < n; v++) {
            if (graph[u][v] && !inMST[v] && graph[u][v] < key[v]) {
                parent[v] = u;
                key[v] = graph[u][v];
            }
        }
    }
    int edgeCount = 0;
    for (int i = 1; i < n; i++) {
        if (parent[i] >= n) {
            free(key);
            free(parent);
            free(inMST);
            return 0;
        }
        if (parent[i] != -1) {
            result[edgeCount].vert1 = parent[i];
            result[edgeCount].vert2 = i;
            edgeCount++;
        }
    }
    free(key);
    free(parent);
    free(inMST);
    return (edgeCount == n - 1);
}


int main(void) {
    FILE* file = fopen("in.txt", "r");
    if (file == NULL)
        return 0;
    short n;
    int m;
    if (fscanf(file, "%hd %d", &n, &m) != 2) {
        fclose(file);
        printf("bad number of lines");
        return 0;
    }
    if (n < 0 || n > 5000) {
        printf("bad number of vertices");
        fclose(file);
        return 0;
    }
    if (m < 0 || m > n * (n - 1) / 2) {
        printf("bad number of edges");
        fclose(file);
        return 0;
    }
    if (n == 0 || (n > 1 && m == 0) || m < (n - 1)) {
        printf("no spanning tree");
        fclose(file);
        return 0;
    }
    int** graph = (int**)malloc(sizeof(int*) * n);
    if (graph == NULL) {
        fclose(file);
        return 0;
    }

    for (int i = 0; i < n; i++) {
        graph[i] = calloc(n, sizeof(int));
        if (graph[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(graph[j]);
            }
            free(graph);
            fclose(file);
            return 0;
        }
    }

    short vert1, vert2;
    long long length;
    for (int i = 0; i < m; i++) {
        if (fscanf(file, "%hd %hd %lld", &vert1, &vert2, &length) != 3) {
            printf("bad number of lines");
            FreeGraph(graph, n);
            fclose(file);
            return 0;
        }
        if (length < 0 || length > INT_MAX) {
            printf("bad length");
            FreeGraph(graph, n);
            fclose(file);
            return 0;
        }
        if (vert1 < 1 || vert1 > n || vert2 < 1 || vert2 > n) {
            printf("bad vertex");
            FreeGraph(graph, n);
            fclose(file);
            return 0;
        }
        if (vert1 == vert2 || graph[vert1 - 1][vert2 - 1]) {
            printf("no spanning tree");
            FreeGraph(graph, n);
            fclose(file);
            return 0;
        }
        graph[vert1 - 1][vert2 - 1] = graph[vert2 - 1][vert1 - 1] = (int)length;
    }
    Correct_Pair* result = malloc(sizeof(Correct_Pair) * (n - 1));
    if (result == NULL) {
        fclose(file);
        FreeGraph(graph, n);
        return 0;
    }
    if (!Prim(graph, n, result)) {
        printf("no spanning tree");
    }
    else {
        for (int i = 0; i < n - 1; i++) {
            printf("%d %d\n", result[i].vert1 + 1, result[i].vert2 + 1);
        }
    }
    free(result);
    FreeGraph(graph, n);
    fclose(file);
    return 0;
}
