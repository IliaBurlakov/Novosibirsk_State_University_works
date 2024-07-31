#define _CRT_SECURE_NO_WARNINGS 


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>


struct st_DijkstraResult {
    unsigned int* minValue;
    short* shortest_route;
};
typedef struct st_DijkstraResult DijkstraResult;


unsigned int GetEdgeValue(const unsigned int* matrix, short vert1, short vert2) {
    if (vert1 > vert2) {
        short temp = vert1;
        vert1 = vert2;
        vert2 = temp;
    }
    return matrix[(vert2 - 1) * vert2 / 2 + vert1 - 1];
}


short PrintAllRoutesFromS(short n, short f, DijkstraResult result, unsigned int* matrix) {
    short num_of_routes_to_f = 0;
    for (short i = 0; i < n; i++) {
        if (result.minValue[i] == UINT_MAX) {
            printf("oo ");
        }
        else if (result.minValue[i] > (unsigned int)INT_MAX) {
            printf("INT_MAX+ ");
        }
        else {
            printf("%u ", result.minValue[i]);
            if (GetEdgeValue(matrix, f, i + 1) != UINT_MAX) {
                num_of_routes_to_f++;
            }
        }
    }
    return num_of_routes_to_f;
}


void PrintShortestRouteToF(short* shortest_route, short f) {
    printf("%hd ", f);
    if (shortest_route[f - 1] != 0) {
        PrintShortestRouteToF(shortest_route, shortest_route[f - 1]);
    }
}


int Dijkstra(const unsigned int* matrix, short n, short s, DijkstraResult result) {
    for (short i = 0; i < n; i++)
        result.minValue[i] = UINT_MAX;

    result.minValue[--s] = 0;

    char* visited = calloc(n, 1);
    if (visited == NULL) {
        printf("memory allocation error");
        return 0;
    }

    for (int v = 0; v < n; v++) {
        visited[s] = 1;
        unsigned int tempValue = UINT_MAX;
        short tempIndex = 0;
        for (short i = 0; i < n; i++) {
            if (!visited[i]) {
                unsigned int edgeValue = GetEdgeValue(matrix, s + 1, i + 1);
                if (edgeValue != UINT_MAX && result.minValue[i] > (result.minValue[s] + edgeValue)) {
                    if (result.minValue[s] + edgeValue > INT_MAX) {
                        result.minValue[i] = (unsigned int)INT_MAX + 1;
                    }
                    else {
                        result.minValue[i] = result.minValue[s] + edgeValue;
                    }
                    result.shortest_route[i] = (short)(s + 1);
                }
                if (tempValue > result.minValue[i]) {
                    tempValue = result.minValue[i];
                    tempIndex = i;
                }
            }
        }
        s = tempIndex;
    }
    free(visited);
    return 1;
}


int main() {
    FILE* file = fopen("in.txt", "r");
    if (file == NULL)
        return 0;

    short n;
    if (fscanf(file, "%hd", &n) != 1) {
        fclose(file);
        return 0;
    }
    if (n < 0 || n > 5000) {
        printf("bad number of vertices");
        fclose(file);
        return 0;
    }

    short s, f;
    if (fscanf(file, "%hd %hd", &s, &f) != 2) {
        fclose(file);
        return 0;
    }
    if (s < 1 || s > n || f < 1 || f > n) {
        printf("bad vertex");
        fclose(file);
        return 0;
    }

    int m;
    if (fscanf(file, "%d", &m) != 1) {
        fclose(file);
        return 0;
    }
    if (m < 0 || m >(int)n * (n - 1) / 2) {
        printf("bad number of edges");
        fclose(file);
        return 0;
    }

    unsigned int* matrix = calloc(n * (n + 1) / 2, sizeof(unsigned int)); // upper triangular matrix
    if (matrix == NULL) {
        fclose(file);
        return 0;
    }
    for (int i = 0; i < n * (n + 1) / 2; i++) {
        matrix[i] = UINT_MAX;
    }
    for (int i = 0; i < n; i++) {
        matrix[i * (i + 1) / 2 + i] = 0;
    }

    short src, dst;
    long long edge_len;
    for (int i = 0; i < m; i++) {
        if (fscanf(file, "%hd %hd %lld", &src, &dst, &edge_len) != 3) {
            printf("bad number of lines");
            free(matrix);
            fclose(file);
            return 0;
        }
        if (edge_len < 0 || edge_len > INT_MAX) {
            printf("bad length");
            free(matrix);
            fclose(file);
            return 0;
        }
        if (src < 1 || src > n || dst < 1 || dst > n) {
            printf("bad vertex");
            free(matrix);
            fclose(file);
            return 0;
        }
        if (src > dst) {
            short temp = src;
            src = dst;
            dst = temp;
        }
        matrix[(dst - 1) * dst / 2 + src - 1] = (unsigned int)edge_len;
    }

    DijkstraResult result;
    result.minValue = malloc(n * sizeof(unsigned int));
    result.shortest_route = calloc(n, sizeof(short));
    if (!Dijkstra(matrix, n, s, result)) {
        free(matrix);
        fclose(file);
        return 0;
    }

    short num_of_routes_to_f = PrintAllRoutesFromS(n, f, result, matrix);

    printf("\n");
    if (result.minValue[f - 1] == UINT_MAX) {
        printf("no path");
    }
    else if (result.minValue[f - 1] > INT_MAX && num_of_routes_to_f >= 2) {
        printf("overflow");
    }
    else {
        PrintShortestRouteToF(result.shortest_route, f);
    }

    free(result.minValue);
    free(result.shortest_route);
    fclose(file);
    free(matrix);
    return 0;
}
