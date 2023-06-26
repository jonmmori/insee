#ifndef _spanning_tree
#define _spanning_tree

#include "globals.h"

typedef struct s_t_route_t{
    
    long root;
    long *path;
    long *link;
    long *link_r;

} s_t_route_t;

typedef struct spanning_tree_routing_table_t{
        
        long n;
        long n_servers;
        long l_used;
        s_t_route_t *s_t_route;

} spanning_tree_routing_table_t;

void create_spanning_trees(long n, long nservers);

void destroy_spanning_trees();

void calc_spanning_tree(long switch_src, long s_t_n, long n_servers);

long calc_spanning_tree_rr(long *rr, long start, long end, s_t_route_t *s_t_route); 
#endif
