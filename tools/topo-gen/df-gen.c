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

long *other_orig2map;
long *other_map2orig;

long ***intergroup_connections;
long **intergroup_route;

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
        printf("type: 0-DRAGONFLY_ABSOLUTE 1-DRAGONFLY_RELATIVE 2-DRAGONFLY_CIRCULANT 3-DRAGONFLY_NAUTILUS 4-DRAGONFLY_HELIX 5-DRAGONFLY_OTHER \n");
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
			case 3:
				next_grp=intergroup_connections[grp_id][sw_id][port - param_p - intra_ports]/(param_h*param_a);
				next_port=intergroup_connections[grp_id][sw_id][port - param_p - intra_ports]%(param_h*param_a);
				break;
			case 4:
				next_grp=intergroup_connections[grp_id][sw_id][port - param_p - intra_ports]/(param_h*param_a);
				next_port=intergroup_connections[grp_id][sw_id][port - param_p - intra_ports]%(param_h*param_a);
				break;
			case 5:
				port_id = other_orig2map[port_id];
				next_grp = (grp_id+port_id+1) % grps;
				next_port = (param_a*param_h) - (port_id+1);
				next_port = other_map2orig[next_port];
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

    if (topo==5){
		long i;

		other_orig2map=malloc(param_a*param_h*sizeof(long));
		other_map2orig=malloc(param_a*param_h*sizeof(long));

		for (i=0; i<param_a*param_h; i++)
		{
			if (i%param_h < (param_h/2))
			{
				other_orig2map[i]=(i%param_h)+((i/param_h)*(param_h/2));
				other_map2orig[other_orig2map[i]]=i;
			}
			else if (param_h%2!=0 && (i%param_h==param_h-1))
			{
				other_orig2map[i]=(i/param_h)+((param_a*(param_h-1)/2));
				other_map2orig[other_orig2map[i]]=i;
			}
			else
			{
				other_orig2map[i]=(param_a*(param_h-(param_h/2)))+((i%param_h)-(param_h/2))+((i/param_h)*(param_h/2));
				other_map2orig[other_orig2map[i]]=i;
			}
		}
	}
	else if (topo==4){
		long g,s,p;	// group, switch and port
		long next_grp, next_sw, next_port; // target group, switch and port

		// to calculate connections between groups
		intergroup_connections=malloc(grps*sizeof(long**));
		for (g=0; g<grps;g++)
		{
			intergroup_connections[g]=malloc(param_a*sizeof(long*));
			for (s=0; s<param_a;s++)
			{
				intergroup_connections[g][s]=malloc(param_h*sizeof(long));
				for (p=0; p<param_h; p++)
					intergroup_connections[g][s][p]=-1; //disconnected
			}

		}

		// to store the port to go through to get from a group 'g' to a target group 'next_grp'
		intergroup_route=malloc(grps*sizeof(long*));
		for (g=0; g<grps;g++)
		{
			intergroup_route[g]=malloc(grps*sizeof(long));
			for (next_grp=0; next_grp<grps; next_grp++)
				intergroup_route[g][next_grp]=-1; // no route between these two groups.
		}

		for (g=0; g<grps; g++)  {
			for (s=0; s<param_a; s++)  {
				for(p=0; p<(param_h/2); p++){
					next_grp = (g + s*(param_h/2) + p + 1)%grps;
					next_sw = (s+1)%param_a;

					next_port=(param_h/2)+(param_h%2);
					while(intergroup_connections[next_grp][next_sw][next_port]!=-1)
					{
						next_port++;
						if (next_port>=param_h)
						{
							printf("Number of ports exceeded when creating dragonfly helix %ld\n",next_port);
							exit(-1);
						}
					}
					intergroup_connections[g][s][p]=(((next_grp*param_a)+next_sw)*param_h)+next_port;
					intergroup_connections[next_grp][next_sw][next_port]=(((g*param_a)+s)*param_h)+p;

					intergroup_route[g][next_grp]=(s*param_h)+p;
					if (intergroup_route[next_grp][g]!=-1 && intergroup_route[next_grp][g]!=(next_sw*param_h)+next_port)
					{
						printf("There is already a route between groups %ld and %ld!\n",next_grp,g);
						//exit(-1);
					}

					intergroup_route[next_grp][g]=(next_sw*param_h)+next_port;

				}
				//Adding the extra edges if odd
				if(param_h%2==1){
					next_grp = (g + (param_h/2)*param_a + s + 1)%grps;
					next_sw = (param_a - s - 1)%param_a;
					next_port=(param_h/2);
					if (g<next_grp){
						while(intergroup_connections[next_grp][next_sw][next_port]!=-1)
						{
							next_port++;
							if (next_port>=param_h)
							{
								printf("Number of ports exceeded when creating dragonfly helix %ld\n",next_port);
								exit(-1);
							}
						}
						intergroup_connections[g][s][p]=(((next_grp*param_a)+next_sw)*param_h)+next_port;
						intergroup_connections[next_grp][next_sw][next_port]=(((g*param_a)+s)*param_h)+p;

						intergroup_route[g][next_grp]=(s*param_h)+p;
						if (intergroup_route[next_grp][g]!=-1 && intergroup_route[next_grp][g]!=(next_sw*param_h)+next_port)
						{
							printf("There is already a route between groups %ld and %ld!\n",next_grp,g);
							exit(-1);
						}

						intergroup_route[next_grp][g]=(next_sw*param_h)+next_port;
					}
				}
			}
		}
	}
	else if (topo==3){
		long g,s,p;	// group, switch and port
		long next_grp, next_sw, next_port; // target group, switch and port
		long cw, ccw;	// distance for next connection: clockwise for + switches (cw) and counter-clockwise for - switches (ccw)

		// to calculate connections between groups
		intergroup_connections=malloc(grps*sizeof(long**));
		for (g=0; g<grps;g++)
		{
			intergroup_connections[g]=malloc(param_a*sizeof(long*));
			for (s=0; s<param_a;s++)
			{
				intergroup_connections[g][s]=malloc(param_h*sizeof(long));
				for (p=0; p<param_h; p++)
					intergroup_connections[g][s][p]=-1; //disconnected
			}

		}

		// to store the port to go through to get from a group 'g' to a target group 'next_grp'
		intergroup_route=malloc(grps*sizeof(long*));
		for (g=0; g<grps;g++)
		{
			intergroup_route[g]=malloc(grps*sizeof(long));
			for (next_grp=0; next_grp<grps; next_grp++)
				intergroup_route[g][next_grp]=-1; // no route between these two groups.
		}

		// Let's calculate connections
		for (g=0; g<grps;g++)
		{
			cw=1;	// switches + start connecting to the group at distance 1 clockwise
			ccw=1;	// switches - start connecting to the group at distance 1 counter-clockwise
			for (s=0; s<param_a;s++)
			{
				for (p=0; p<param_h; p++)
				{
					if(intergroup_connections[g][s][p]==-1) // still not connected
					{
						if (s%2 == 0) // + switch; connect clockwise
						{
							do	// ensure these groups are not connected already, skip to the next clockwise group
							{
								next_grp=(g+cw)%grps;
								cw++;
							} while (intergroup_route[g][next_grp]!=-1);
						}
						else	// - switch; connect counter-clockwise
						{
							do	// ensure these groups are not connected already, skip to the next group counter-clockwise
							{
								next_grp=(grps+g-ccw)%grps;
								ccw++;
							} while (intergroup_route[g][next_grp]!=-1);
						}
						next_sw=g%param_a;
						next_port=0;
						while(intergroup_connections[next_grp][next_sw][next_port]!=-1)
						{
							next_port++;
							if (next_port>=param_h)
							{
								printf("Number of ports exceeded when creating dragonfly nautilus %ld\n",next_port);
								exit(-1);
							}
						}
						intergroup_connections[g][s][p]=(((next_grp*param_a)+next_sw)*param_h)+next_port;
						intergroup_connections[next_grp][next_sw][next_port]=(((g*param_a)+s)*param_h)+p;

						intergroup_route[g][next_grp]=(s*param_h)+p;
						if (intergroup_route[next_grp][g]!=-1)
						{
							printf("There is already a route between groups %ld and %ld!\n",next_grp,g);
							exit(-1);
						}

						intergroup_route[next_grp][g]=(next_sw*param_h)+next_port;
					}
				}
			}
		}
	}

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

