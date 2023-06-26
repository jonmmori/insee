/**
* @file
* @brief	Declaration of CAM tables (if used).
*/

#ifndef _ksp_routing
#define _ksp_routing

#include "graph.h"
#include "list.h"

typedef struct path_t{

    long length;
    long next;
    long *nodes;
    long *links;
    long *links_r;
} path_t;

typedef struct edge_removed_t{

    long src;
    long dst;
} edge_removed_t;  


path_t *init_path(long max_length);

void destroy_path(path_t *p);

void print_path(path_t *p);

long find_shortest_path(long switch_src, long *paths, long *paths_dists, long *links, long *links_r, graph_t *graph);

long shortest_path(path_t *l, long *paths, long *links, long *links_r, int start, int end, long length); 

long shortest_path_generic(path_t **k_paths, long *paths, long *links, long *links_r, int start, int end, long length, long K, long ecmp, graph_t *graph);

long shortest_path_generic_limited(path_t **k_paths, long *paths, long *links, long *links_r, int start, int end, long length, long M, long K, long H, long ths, graph_t *graph);

long arrays_eq(path_t *a1, path_t *a2, long i);

long is_path(list *B, path_t *p);

long extract_shortest(list *B, path_t **l_dst, long ecmp, long shortest_path_length);

long extract_shortest_limited(list *B, path_t **l_dst, long n_current, long H, long ths);

#endif


