#ifndef _graph
#define _graph

/**
 * Structure that defines a port/link.
 */
typedef struct edge_t {
    long active;
    long n_node;	///< the node it is connected to (-1 means not connected).
    long n_edge;	///< the port is connected to.
} edge_t;

/**
 * Structure that defines a node of the graph.
 */
typedef struct graph_t {
    long active;
    long nedges;///< number of edges
    edge_t* edge;	///< structure containing all the ports.
} graph_t;


void deactivate_edge(graph_t *rg, long node_src, long node_dst);

void activate_edge(graph_t *rg, long node_src, long node_dst);

void deactivate_node(graph_t *rg, long node);

void activate_node(graph_t *rg, long node);

void load_graph(graph_t *graph);

void load_graph_exa(graph_t *graph);

void gen_gdbg_graph(graph_t *graph);

void gen_kautz_graph(graph_t *graph);

void print_graph(graph_t *graph);

void finish_graph();

#endif //_rg_gen
