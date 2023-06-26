/**
*@file mesh3.c
*@author Javier Navaridas Palma.
*
*@brief	This file creates a trace moving in a 3D Mesh towards X, Y & Z.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "alog.h"
#define THREAD 0
#define MPI_COMM_WORLD 0

long nodes=64;		///< Number of nodes of the trace.
long size=10000;	///< Size of the burst.

/**
* Prints a trace in the standard output moving in a 3D Mesh towards X, Y & Z.
*
*@param nodes	The number of nodes. It should be a cubic value, if not an error is printed in the error output.
*@param size		The size of message.
*/
void mesh3d(long nodes, long size)
{
	long dist;	///< nodes ^ 1/3
	long dist2;	///< nodes ^ 2/3

	dist=(long)pow(nodes, 1.0/3);
	while(dist*dist*dist<nodes)
		dist++;
	if (dist*dist*dist!=nodes)
		perror("Number of Nodes is not a cubic value.\n");
	else
	{
		long i;		///< The number of the node.
		long t=0;	///< The tag
		dist2=dist*dist;
		for (i=0; i<nodes; i++)
		{
			if (i%dist<dist-1)		// Sends to +X
				insert_send(i,	i+1,		t,	size);
			if (i%dist)			// Sends to -X
				insert_send(i,	i-1,		t,	size);
			if ((i/dist)%dist<dist-1)	// Sends to +Y
				insert_send(i,	i+dist,		t,	size);
			if ((i/dist)%dist)		// Sends to -Y
				insert_send(i,	i-dist,		t,	size);
			if (i/dist2<dist-1)		// Sends for +Z
				insert_send(i,	i+dist2,	t,	size);
   			if (i/dist2)			// Sends to -Z
				insert_send(i,	i-dist2,	t,	size);

			if (i%dist<dist-1)		// Waits for +X
				insert_recv(i,	i+1,		t,	size);
			if ((i/dist)%dist<dist-1)	// Waits for +Y
				insert_recv(i,	i+dist,		t,	size);
			if (i/dist2<dist-1)		// Waits for +Z
				insert_recv(i,	i+dist2,	t,	size);
			if (i%dist)			// Waits for -X
				insert_recv(i,	i-1,		t,	size);
			if ((i/dist)%dist)		// Waits for -Y
				insert_recv(i,	i-dist,		t,	size);
        	if (i/dist2)			// Waits for -Z
				insert_recv(i,	i-dist2,	t,	size);
		}
	}
}

/**
* Main Function. Accepts up to two parameters: number of nodes and message size.
*
* The translation is performed enterely here.
*
*@param argc The number of parameters of the function call.
*@param argv The arguments of the program.
*@return A finalization code.
*/
int main(int argc, char *argv[])
{
	long i;		///< index in loops.

	if (argc>1)
		nodes=atol(argv[1]);
	if (argc>2)
		size=atol(argv[2]);
	if (argc>3){
		printf("WARNING: Too many arguments!!!\n         Use %s <nodes> <msg_size>\n",argv[0]);
		exit(-1);
	}

	if (DIMEMAS_HEADER_NEEDED != 0){
		printf("#DIMEMAS:mesh3d_%d_%d:0:%d(1",nodes,size,nodes);
		for (i=1; i<nodes; i++)
			printf(",1");
		printf("),0\n");
	}

	mesh3d(nodes, size);
	return 0;
}
