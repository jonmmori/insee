/**
*@file mesh2.c
*@author Javier Navaridas Palma.
*
*@brief	This file creates a trace moving in a 2D Mesh towards X & Y.
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
* Prints a trace in the standard output moving in a 2D Mesh towards X & Y.
*
*@param nodes	The number of nodes. It should be a quadratic value, if not an error is printed in the error output.
*@param size		The size of message.
*/
void sweep2d(long nodes, long size)
{
	long dist;	///< nodes ^ 1/2

	if ((dist=(long)sqrt(nodes))!=(long)ceil(sqrt(nodes)))
		perror("Number of Nodes is not a quadratic value\n");
	else
	{
		long i;		///< The number of the node.
		long t=0;	///< The tag
		for (i=0; i<nodes; i++){
			if ((i/dist)!=0)		// Waits for -dist
				insert_recv(i,	i-dist,	t,	size);
			if ((i%dist)!=0)		// Waits for -1
				insert_recv(i,	i-1,	t,	size);
			if ((i%dist)<(dist-1))	// Sends to +1
				insert_send(i,	i+1,	t,	size);
			if ((i/dist)<(dist-1))	// Sends to +dist
				insert_send(i,	i+dist,	t,	size);

// RETURN WAVE
//			if ((i%dist)<(dist-1))	// Waits for +1
//				insert_recv(i,	i+1,	t,	size);
//			if ((i/dist)<(dist-1))	// Waits for +dist
//				insert_recv(i,	i+dist,	t,	size);
//			if ((i/dist)!=0)		// Sends to -dist
//				insert_send(i,	i-dist,	t,	size);
//			if ((i%dist)!=0)		// Sends to -1
//				insert_send(i,	i-1,	t,	size);
		}
	}
}

/**
* Main Function. Accepts up to two parameters: number of nodes and message size.
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
		printf("#DIMEMAS:sweep2d_%d_%d:0:%d(1",nodes,size,nodes);
		for (i=1; i<nodes; i++)
			printf(",1");
		printf("),0\n");
	}
	sweep2d(nodes, size);
	return 0;
}
