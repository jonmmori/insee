/**
* @file
* @brief	The torus & mesh topologies tools.

FSIN Functional Simulator of Interconnection Networks
Copyright (2003-2011) J. Miguel-Alonso, A. Gonzalez, J. Navaridas

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

/**
* Obtains a neighbor node in torus & mesh topologies.
*
* Given a node address "ad", a direction "wd" (X,Y or Z) and a way "ww" (UP or DOWN)
* returns the address of the neighbor in that direction and way; only valid for torus
* but usable also for mesh.
*
* @param ad A node address.
* @param wd A dimension (X,Y or Z).
* @param ww A way (UP or DOWN).
* @return The address of the neighbor in that direction.
*/
long torus_neighbor (long ad, dim wd, way ww) {
    int i;
    long n;
    long w;

    if (ww == UP)
        w = 1;
    else
        w = -1;
    n = (network[ad].rcoord[wd]+w)%nodes_per_dim[wd];
    if (n < 0) n += nodes_per_dim[wd];

    long address_n = 0;
    long num_nodes = 1;

    for (i = 0; i < ndim; ++i) {
        if (i==wd) {
            if(i==0){
                address_n = n;
            }else{
                num_nodes *= nodes_per_dim[i-1];
                address_n += num_nodes*n;
            }
        }else if(i==0){
            address_n = network[ad].rcoord[i];
        }else{
            num_nodes *= nodes_per_dim[i-1];
            address_n+=num_nodes*network[ad].rcoord[i];
        }
    }

    return address_n;
}

/**
* Generates the routing record for a mesh.
*
* @param source The source node of the packet.
* @param destination The destination node of the packet.
* @return The routing record needed to go from source to destination.
*/
routing_r mesh_rr (long source, long destination) {
    int i;
    routing_r res;
    res.rr = alloc(ndim*sizeof(long));

    if (source == destination)
        panic("Self-sent packet");

    res.size = 0;
    for (i = 0; i < ndim; i++) {
        res.rr[i] = network[destination].rcoord[i]-network[source].rcoord[i];
        res.size += abs(res.rr[i]);
    }

    return res;
}

/**
* Generates the routing record for a torus.
*
* @param source The source node of the packet.
* @param destination The destination node of the packet.
* @return The routing record needed to go from source to destination.
*/
routing_r torus_rr (long source, long destination) {
    routing_r res;
    int i;
    res.rr = alloc(ndim*sizeof(long));

    if(source == destination)
        panic("Self-sent packet");
    res.size = 0;
    for (i = 0; i < ndim; i++) {

        res.rr[i] = (network[destination].rcoord[i]-network[source].rcoord[i])%nodes_per_dim[i];
        if (res.rr[i] < 0)
            res.rr[i] += nodes_per_dim[i];
        if (res.rr[i] > nodes_per_dim[i]/2)
            res.rr[i] = (nodes_per_dim[i]-res.rr[i])*(-1);
        if ((double)res.rr[i] == nodes_per_dim[i]/2.0)
            if (rand() >= (RAND_MAX/2))
                res.rr[i] = (nodes_per_dim[i]-res.rr[i])*(-1);
        res.size += abs(res.rr[i]);

    }
    return res;


}

/**
* Generates the routing record for an unidirectional torus.
*
* @param source The source node of the packet.
* @param destination The destination node of the packet.
* @return The routing record needed to go from source to destination.
*/
routing_r torus_rr_unidir (long source, long destination) {
	long sx, sy, sz, dx, dy, dz;
	routing_r res;

	res.rr=alloc(ndim*sizeof(long));

	if (source == destination)
		panic("Self-sent packet");

	sx=network[source].rcoord[D_X];
	sy=network[source].rcoord[D_Y];
	sz=network[source].rcoord[D_Z];

	dx=network[destination].rcoord[D_X];
	dy=network[destination].rcoord[D_Y];
	dz=network[destination].rcoord[D_Z];

	res.rr[D_X] = (dx-sx)%nodes_x;
	if (res.rr[D_X] < 0)
		res.rr[D_X] += nodes_x;
	res.size = abs(res.rr[D_X]);

	if (ndim >= 2) {
		res.rr[D_Y] = (dy-sy)%nodes_y;
		if (res.rr[D_Y] < 0)
			res.rr[D_Y] += nodes_y;
		res.size += abs(res.rr[D_Y]);
	}

	if (ndim == 3){
		res.rr[D_Z] = (dz-sz)%nodes_z;
		if (res.rr[D_Z] < 0)
			res.rr[D_Z] += nodes_z;
		res.size += abs(res.rr[D_Z]);
	}
	return res;
}

