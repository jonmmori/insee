/*
 * dragonfly.c
 *
 *  Created on: 3 Jul 2017
 *      Author: yzy
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "rg_gen.h"

/*
 * Parameters (a,p,h) for the dragonfly topology;
 */

long topo;
long param_p; ///< p: Number of servers connected to each switch
long param_a; ///< a: Number of switches in each group
long param_h; ///< h: Number of uplinks

long grps; ///< Total number of groups
long switches;///< Total number of switches
long servers;///< Total number of servers
long ports;///< Total number of links
long intra_ports; ///<  Total number of ports in one group connecting to other routers in the group

long proxy_grp; ///< The switch group to use as a proxy.
long n_hops;	///< hops taken; only used for detecting errors, should be removed soon.

long max_paths;
long ports_switches;

graph_t *df_graph;
/**
 * declare the number of global connections between groups;
 */
int main (int argc, char *argv[]) {

    long i,j;
    char filename_params[100];
    //Check the parameters in configuration file
    if(argc < 5) {
        printf("4 parameters are needed for the dragonfly topology <type, p, a, h>\n");
        exit(-1);
    }

    topo = atoi(argv[1]);
    param_p = atoi(argv[2]); // number of servers per switch
    param_a = atoi(argv[3]); // number of switches per group
    param_h = atoi(argv[4]); // number of uplinks per switch

    //    dim = param_a - 1;
    intra_ports = param_a - 1;
    // check and calculate the number of global connections
    if(param_p < 1) {
        printf("param_p must be a positive number; %ld has been inserted", param_p);
        exit(-1);
    }
    if(param_a < 1) {
        printf("param_a must be a positive number; %ld has been inserted", param_a);
        exit(-1);
    }
    if(param_h < 1) {
        printf("param_h must be a positive number; %ld has been inserted", param_h);
        exit(-1);
    }

    grps = param_a * param_h + 1; // uplinks * switches_per_group +1
    switches = param_a * grps;

    ports = param_a * (param_p + param_h + param_a - 1) * grps;
    servers = param_p * switches;
    
    ports_switches = param_p + param_h + param_a - 1;

    df_graph = malloc(sizeof(graph_t) * switches);

        for(i = 0; i < switches; i++) {
        df_graph[i].nedges = ports_switches - param_p;
        df_graph[i].fedges = df_graph[i].nedges;
        df_graph[i].active = 1;
        df_graph[i].edge = malloc(sizeof(edge_t) * df_graph[i].nedges);
        for(j = 0; j < df_graph[i].nedges; j++) {
            df_graph[i].edge[j].neighbour.node = -1;
            df_graph[i].edge[j].neighbour.edge = -1;
            df_graph[i].edge[j].active = 1;
        }
    }
    
    generate_df(df_graph, servers + switches, ports_switches);
    export_graph_df_insee(df_graph, switches, filename_params);
    finish_topo_dragonfly(switches);
    return(1);
}

void finish_topo_dragonfly(long switche){
    long i;

    for(i = 0; i < switches; i++) {
        free(df_graph[i].edge);
    }
    free(df_graph);

}

long get_servers_dragonfly()
{
    return servers;
}

long get_radix_dragonfly(long n)
{

    if ( n < servers )
        return 1;	// This is a server
    else{
        return param_h + param_p + param_a -1; // This is a switch with h uplinks, p downlinks.
    }

}

n_t connection_dragonfly(long node, long port) {
    
    n_t res={-1,-1};
    long gen_switch_id; // switch id in the general switch count
    long sw_id, grp_id, port_id; // switch (within a group), group and port id for calculating connections
    long next_grp, next_port; // group and port id of the target for calculating connections
    
    if( node < servers ) { // The node is a server
        if( port == 0 ) {
            res.node = servers + node / param_p ; // The server's router
            res.edge = node % param_p; // The server's port number
        } // servers only have one connection
    }
    else{ // the node is a switch
        gen_switch_id=node - servers; // id of the switch relative to other switches
        grp_id=gen_switch_id/param_a; // id of the group relative to other groups
        if( port < param_p ) {// This is a downlink to a server
            res.node = (gen_switch_id * param_p) + (port % param_p); // The sequence of the server
            res.edge = 0 ; // Every processor only has one port.
        }
        else if ( port < ( param_p + intra_ports ) ){ // Intra-group connection
            sw_id = gen_switch_id % param_a;
            port_id = port - param_p;
            if (port_id>=sw_id){
                res.node = servers + (grp_id * param_a) + port_id+1;
                res.edge = param_p + sw_id;
            } else {
                res.node = servers + (grp_id * param_a) + port_id;
                res.edge = param_p + sw_id-1;
            }
        }
        else if (port < param_h + intra_ports + param_p ) { // uplinks; many connections possible here
            sw_id = gen_switch_id % param_a; // the switch id relative to the switch group
            port_id = port - param_p - intra_ports + (sw_id*param_h); // the port id relative to the switch group

            /// Let's calculate the next group and its link, based on the connection arrangement.
            switch(topo) {
                case 0:
                    if (port_id>=grp_id){
                        next_grp= port_id+1;
                        next_port=grp_id;
                    } else {
                        next_grp=port_id;
                        next_port=grp_id-1;
                    }
                    break;
                case 1:
                    next_grp = (grp_id+port_id+1)%grps;
                    next_port = (param_a*param_h)-(port_id+1);
                    break;
                case 2:
                    if (port_id % 2){ // odd ports connect counterclockwise
						next_grp = (grps+grp_id-(port_id/2)-1)%grps;
						next_port = port_id-1;
                    } else { // even ports connect clockwise
                        next_grp = (grp_id+(port_id/2)+1)%grps;
						if (port_id==grps-2) // will happen when param_h and param_a uneven, the last port connects with itself
							next_port = port_id;
						else
							next_port = port_id+1;
                    }
                    break;
                default:
                    printf("Not a valid dragonfly");
                    exit(-1);
                    break;
            }
            res.node = servers + (next_grp * param_a) + (next_port/param_h);
            res.edge = param_p + intra_ports + (next_port%param_h);
        }
        else {
            // Should never get here
            res.node = -1;
            res.edge = -1;
        }
    }
    //if (node>=servers && res.node>=servers)
    //    printf("node-port %ld,%ld <--> res.node-res.edge %ld %ld\n", node,port,res.node,res.edge);
    return res;
}


void generate_df(graph_t *rg, long n, long p){
    
    long i, j;
    n_t res;

    for(i = 0; i < n; i++){
        for(j = 0; j < p; j++){
            res = connection_dragonfly(i, j);
            if(i >= servers && j >= param_p){
                rg[i - servers].edge[j - param_p].neighbour.node = res.node - servers;
                rg[i - servers].edge[j - param_p].neighbour.edge = res.edge - param_p;
                //printf("(%ld, %ld) --> (%ld, %ld)\n", i, j, res.node, res.edge,servers);
            }
        }  
    }
}

/**
 * Print the network structure (without node) as a an adjacency matrix. Just for testing purposes.
 */
void export_graph_df_insee(graph_t *rg, long switches, char *topo_name)
{

    long i, k;
        
    for(i = 0; i < switches; i++) {
            for(k = 0; k < rg[i].nedges; k++) {
                    printf("(%ld,%ld) ", rg[i].edge[k].neighbour.node, rg[i].edge[k].neighbour.edge);
            }
            printf("\n");
    }
}

