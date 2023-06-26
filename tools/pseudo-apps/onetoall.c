/**
*@file butterfly.c
*@author Javier Navaridas Palma.
*
*@brief	This file creates a trace performing a butterfly interchange.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "alog.h"
#define THREAD 0
#define MPI_COMM_WORLD 0

long nodes=64;		///< Number of nodes of the trace.
long size=10000;	///< Size of the burst

/**
* Prints a trace in the standard output performing a one-to-all interchange. (non-optimized bcast)
*
*@param nodes	The number of nodes.
*@param size	The size of the burst.
*/
void onetoall(long nodes, long size)
{
	long i=1;

	for (i=1; i<nodes; i++)
	{
		insert_send(0,  i,   i,      size);
		insert_recv(i,  0,   i,      size);
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
		printf("#DIMEMAS:onetoall_%d_%d:0:%d(1",nodes,size,nodes);
		for (i=1; i<nodes; i++)
			printf(",1");
		printf("),0\n");
	}

	onetoall(nodes, size);
	return 0;
}
