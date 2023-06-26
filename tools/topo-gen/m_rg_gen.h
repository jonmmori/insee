#ifndef _rg_gen
#define _rg_gen

/**
 * Structure that defines a port/link location.
 */
typedef struct n_t {
    long node;	///< the node it is connected to (-1 means not connected).
    long edge;	///< the port is connected to.
} n_t;

/**
 * Structure that defines a port/link.
 */
typedef struct edge_t {
    long active;
    n_t neighbour;	///< the node & port it is connected to (-1 means not connected).
} edge_t;

/**
 * Structure that defines a node of the graph.
 */
typedef struct graph_t {
    long active;
    long fedges; ///< free edges
    long nedges;///< number of edges
    edge_t* edge;	///< structure containing all the ports.
} graph_t;

void finish_topo_jellyfish(long switches);

void finish_topo_dragonfly(long switches);

void generate_rrg(graph_t *rg, long n, long k, long seed);

void generate_df(graph_t *rg, long n, long k);

long is_full_connected(graph_t *rg, long n);

long is_difference(graph_t *rg, long node_ref, long node_ref2, long node, long n);

long are_connected(graph_t *rg, long n1, long n2);

void connect_nodes(graph_t *rg, long n1, long n2);

void disconnect_nodes(graph_t *rg, long n1, long n2);

void export_graph_adjacency(graph_t *rg, long switches);

void export_graph_edges(graph_t *rg, long switches, char *topo_name);

void export_graph_rrg_insee(graph_t *rg, long switches, long servers, long param_nic, long port_servers, char *topo_name);

void export_graph_rrg_socnetv(graph_t *rg, long switches, long servers, long param_nic, long port_servers, char *topo_name);

void export_graph_df_insee(graph_t *rg, long switches, char *topo_name);

void deactivate_edge(graph_t *rg, long node_src, long node_dst);

void activate_edge(graph_t *rg, long node_src, long node_dst);

void deactivate_node(graph_t *rg, long node);

void activate_node(graph_t *rg, long node);

void print_all_router_structure(graph_t *rg, long s);
#endif //_rg_gen
