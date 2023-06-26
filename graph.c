/**
 * @file
 * @brief	graph topology tools.
 *
 *@author Jose A. Pascual and Javier Navaridas

 FSIN Functional Simulator of Interconnection Networks
 Copyright (2003-2017) J. Miguel-Alonso, J. Navaridas, Jose A. Pascual

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "globals.h"
#include "graph.h"
#include <stdlib.h>

/**
 * Creates a rrg graph topology from a file.
 *
 * This function defines all the links between the elements in the network.
 */
void create_graph(){

	long i, j, k, nr, np;	//neighbor router and port.
	long p, paux;		//current stage, router and port.

	graph_t *graph;

	// Initialize structure to load the graph (defaults to -1)
	graph = alloc(nswitches * sizeof(graph_t));
	for(i = 0; i < nswitches; i++){
		graph[i].nedges = stUp;
		graph[i].active = 1;
		graph[i].edge = alloc(stUp * sizeof(edge_t));
		for(j = 0; j < stUp; j++){
			graph[i].edge[j].n_node = -1;
			graph[i].edge[j].n_edge = -1;
			graph[i].edge[j].active = 1;
		}
	}
	if(topo == RRG){
		load_graph(graph);
	}
	else if(topo == EXA){
		load_graph_exa(graph);
	}
	else if(topo == GDBG){
		gen_gdbg_graph(graph);
	}
	else if(topo == KAUTZ){
		gen_kautz_graph(graph);
	}

	for(i = 0; i < nprocs; i++){

		// Initializating processors. Only 1 transit port plus injection queues.

		network[i].rcoord[SWITCH_ID] = -1;

		for (j = 0; j < ninj; j++)
			inj_init_queue(&network[i].qi[j]);
		init_ports(i);
		for(k = 0; k < nnics; k++){
			nr = ((i / stDown) * nnics) + nprocs + k;
			np = (i % stDown); // Procs connected to the first ports
			//printf("Node %ld: Port %ld to node %ld port %ld\n", i, k, nr,np); 
			network[i].nbor[k] = nr;
			network[i].nborp[k] = np;
			network[i].op_i[k] = ESCAPE;
			network[nr].nbor[np] = i;
			network[nr].nborp[np] = k;
			network[nr].op_i[np] = ESCAPE;
		}
		for (j = nnics; j < radix; j++){
			network[i].nbor[j] = NULL_PORT;
			network[i].nborp[j] = NULL_PORT;
			network[i].op_i[j] = ESCAPE;
		}
	}

	if(routing == CAM_ROUTING)
		init_cams();
	// Initializing switches. No injection queues needed.
	for (i = nprocs; i < NUMNODES; i++ ){

		network[i].rcoord[SWITCH_ID] = i;

		init_ports(i);
		for (p = 0; p < stUp; p++) {
			paux = p + stDown;
			if(network[i].nbor[paux] == NULL_PORT){
				nr = graph[i - nprocs].edge[p].n_node + nprocs;
				np = graph[i - nprocs].edge[p].n_edge + stDown;
				network[i].nbor[paux] = nr;			// neighbor router
				network[i].nborp[paux] = np;			// neighbor's port
				network[i].op_i[paux] = ESCAPE;
			}
		}

		if(routing == CAM_ROUTING)
			fill_cam(i, graph);
	}
	if(routing == SPANNING_TREE_ROUTING)
		create_spanning_trees(nchan, stDown);

	for(i = 0; i < nswitches; i++)
		free(graph[i].edge);
	free(graph);
}

/**
 * Generates the routing record for a graph.
 *
 *
 * @param source The source node of the packet.
 * @param destination The destination node of the packet.
 * @return The routing record needed to go from source to destination.
 */
routing_r graph_rr_static (long source, long destination) {

	long i, p_src, p_dst, sw_src, sw_dst, current_path, length;
	routing_r res;

	if (source == destination)
		panic("Self-sent packet");

	p_src = rand() % nnics;
	p_dst = (destination) % stDown;
	sw_src = ((source / stDown) * nnics) + nprocs + p_src;
	sw_dst = ((destination / stDown) * nnics) + p_src;
	//printf("S: %ld D: %ld SS: %ld SD: %ld\n", source, destination, sw_src, sw_dst);
	current_path = network[sw_src].cam[sw_dst].l_path++;
	length = network[sw_src].cam[sw_dst].ports[current_path][0];
	res.rr = alloc((length + 2) * sizeof(long));
	res.rr[0] = p_src;
	res.rr[length + 1] = p_dst;
	for(i = 0; i < length; i++){
		res.rr[i + 1] =  network[sw_src].cam[sw_dst].ports[current_path][i + 1];
	}
	res.size = length + 2;
	network[sw_src].cam[sw_dst].l_path %= network[sw_src].cam[sw_dst].n_paths;

	return(res);
}

/**
 * Generates the routing record for a graph.
 *
 *
 * @param source The source node of the packet.
 * @param destination The destination node of the packet.
 * @return The routing record needed to go from source to destination.
 */
routing_r graph_rnd_static (long source, long destination) {

	long i, p_src, p_dst, sw_src, sw_dst, current_path, length;
	routing_r res;

	if (source == destination)
		panic("Self-sent packet");

	p_src = rand() % nnics;
	p_dst = (destination) % stDown;
	sw_src = ((source / stDown) * nnics) + nprocs + p_src;
	sw_dst = ((destination / stDown) * nnics) + p_src;
	current_path = rand() % network[sw_src].cam[sw_dst].n_paths;
	length = network[sw_src].cam[sw_dst].ports[current_path][0];
	res.rr = alloc((length + 2) * sizeof(long));
	res.rr[0] = p_src;
	res.rr[length + 1] = p_dst;
	for(i = 0; i < length; i++){
		res.rr[i + 1] =  network[sw_src].cam[sw_dst].ports[current_path][i + 1];
	}
	res.size = length + 2;

	return res;
}


/**
 * Generates the routing record for a graph.
 *
 * This function allows adaptive routing because no route is defined here.
 *
 * @param source The source node of the packet.
 * @param destination The destination node of the packet.
 * @return The routing record needed to go from source to destination.
 */
routing_r graph_rr_adaptive (long source, long destination) {

	routing_r res;

	if (source == destination)
		panic("Self-sent packet");

	res.rr = alloc((diameter_r + 1)* sizeof(long));;
	res.rr[0] = diameter_r + 1;
	res.size = 0;

	return(res);
}

void deactivate_edge(graph_t *rg, long node_src, long node_dst){

	long i;

	for(i = 0; i < rg[node_src].nedges; i++) {
		if(rg[node_src].edge[i].n_node == node_dst) {
			rg[node_src].edge[i].active = 0;
			rg[node_dst].edge[rg[node_src].edge[i].n_edge].active = 0;
			break;
		}
	}
}

void activate_edge(graph_t *rg, long node_src, long node_dst){

	long i;

	for(i = 0; i < rg[node_src].nedges; i++) {
		if(rg[node_src].edge[i].n_node == node_dst) {
			rg[node_src].edge[i].active = 1;
			rg[node_dst].edge[rg[node_src].edge[i].n_edge].active = 1;
			break;
		}
	}
}

void deactivate_node(graph_t *rg, long node){

	rg[node].active = 0;
}

void activate_node(graph_t *rg, long node){

	rg[node].active = 1;
}

/** Load the description of the topology (graph) from a file
 *  Format of the file:
 *  1st line: Number of switches
 *  2nd line: Radix of the switches
 *  3rd line: Number of ports for connections (Rest of the ports computing nodes or not used)
 *  Rest of the lines: 1st column: Source switch
 *                     Rest of the columns: Switches to which is connected
 *  @param The name of the file
 */

void load_graph_exa(graph_t *graph){

	long i, k, h;
	long c_node, n_node, check_node;
	FILE *fd;

	print_graph(graph);
	if((fd = fopen(topology_filename, "r")) == NULL)
		panic("Error opening the graph topology file.");

	check_node = 0;
	i = 1;
	while(i < nswitches){
		//if(fscanf(fd,"(%ld,%ld)\n", &graph[i].edge[j].n_node, &graph[i].edge[j].n_edge) < 2)
		if(fscanf(fd,"%ld %ld\n", &c_node, &n_node) < 2)
			panic("Format of the graph file is incorrect.");
		printf("%ld --> %ld \n", c_node, n_node);
		if(check_node != c_node){
			check_node = c_node;
			i++;
		}
		k = 0;
		while(k < stUp){
			if(graph[c_node].edge[k].n_node == -1){
				graph[c_node].edge[k].n_node = n_node;
				h = 0;
				while(h < stUp){
					if(graph[n_node].edge[h].n_node == -1){
						graph[c_node].edge[k].n_edge = h;
						graph[n_node].edge[h].n_edge = k;
						graph[n_node].edge[h].n_node = c_node;
						break;
					}
					else if(graph[n_node].edge[h].n_node == c_node){
						break;
					}
					h++;
				}
				break;
			}
			else if(graph[c_node].edge[k].n_node == n_node){
				break;
			}
			k++;
		}
	}
	fclose(fd);
	print_graph(graph);
}

/** Load the description of the topology (graph) from a file
 *  Format of the file:
 *  1st line: Number of switches
 *  2nd line: Radix of the switches
 *  3rd line: Number of ports for connections (Rest of the ports computing nodes or not used)
 *  Rest of the lines: 1st column: Source switch
 *                     Rest of the columns: Switches to which is connected
 *  @param The name of the file
 */

void load_graph(graph_t *graph){

	long i, j;
	FILE *fd;

	if((fd = fopen(topology_filename, "r")) == NULL)
		panic("Error opening the graph topology file.");

	for(i = 0; i < nswitches; i++){
		for(j = 0; j < stUp; j++){
			if(fscanf(fd,"(%ld,%ld)\t", &graph[i].edge[j].n_node, &graph[i].edge[j].n_edge) < 2)
				panic("Format of the graph file is incorrect.");
		}
	}
	fclose(fd);
}

/** Load the description of the topology (graph) from a file
 *  Format of the file:
 *  1st line: Number of switches
 *  2nd line: Radix of the switches
 *  3rd line: Number of ports for connections (Rest of the ports computing nodes or not used)
 *  Rest of the lines: 1st column: Source switch
 *                     Rest of the columns: Switches to which is connected
 *  @param The name of the file
 */

void gen_gdbg_graph(graph_t *graph){

	long i, j, k, h, n_node;

	for(i = 0; i < nswitches; i++){
		j = 0;
		h = 0;
		while(h < (stUp / 2) && j < stUp){
			if(graph[i].edge[j].n_node == -1){
				n_node = ((i * (stUp / 2)) + h) % nswitches;
				for(k = 0; k < stUp; k++){
					if(graph[n_node].edge[k].n_edge == -1 && i != n_node){
						//printf("%ld,%ld --> %ld,%ld\n", i,j, n_node,k);
						graph[i].edge[j].n_node = n_node;
						graph[i].edge[j].n_edge = k;
						graph[n_node].edge[k].n_edge = j;
						graph[n_node].edge[k].n_node = i;
						break;
					}
				}
				h++;
			}
			j++;
		}
	}
	//printf("\n");
}

void gen_kautz_graph(graph_t *graph){

	long i, j, k, n_node;

	for(i = 0; i < nswitches; i++){
		for(j = 0; j < stUp; j++){
			n_node = ((i * stUp) + j) % nswitches;
			graph[i].edge[j].n_node = n_node;
			for(k = 0; k < stUp; k++){
				if(graph[n_node].edge[k].n_node == -1){
					graph[i].edge[j].n_edge = k;
					break;
				}
			}
		}
	}
}

void print_graph(graph_t *graph){

	long i,j;

	for(i = 0; i < nswitches; i++){
		for(j = 0; j < stUp; j++){
			printf("(%ld,%ld) ", graph[i].edge[j].n_node, graph[i].edge[j].n_edge);
		}
		printf("\n");
	}
}

void finish_graph(){

	if(routing == CAM_ROUTING)
		finish_cams();
	else if(routing == SPANNING_TREE_ROUTING)
		destroy_spanning_trees();

}
