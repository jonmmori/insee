/**
*@file waterfall.c
*@author Javier Navaridas Palma.
*
*@brief	This file creates a trace similar to the waterfalls observed in LU.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "alog.h"
#define THREAD 0
#define MPI_COMM_WORLD 0

long nodes=64;		///< Number of nodes of the trace.
long its=10;		///< Number of iterations.
long size=10;	///< Size of the message.

/**
* Prints a trace in the standard output similar to the waterfalls in LU.
*
*@param nodes	The number of nodes. It should be a quadratic value, if not an error is printed in the error output.
*@param size	The size of the burst.
*/
void waterfall(long nodes, long size)
{
	long dist;	///< nodes ^ 1/2

	if ((dist=(long)sqrt(nodes))!=(long)ceil(sqrt(nodes)))
		perror("Number of Nodes is not a quadratic value\n");
	else
	{
		long i;					///< The number of the node.
		long n;					///< The total amount of bytes sent
		for (i=0; i<nodes; i++)
			for (n=0;  n<its; n++)
			{
				if (i%dist)		// Waits for -1
					insert_recv(i,	i-1,	n,	size);
				if (i/dist)		// Waits for -dist
					insert_recv(i,	i-dist,	n,	size);
				if (i%dist<dist-1)	// Sends to +1
					insert_send(i,	i+1,	n,	size);
				if (i/dist<dist-1)	// Sends to +dist
					insert_send(i,	i+dist,	n,	size);
			}
	}
}

/**
* Main Function. Accepts up to two parameters: number of nodes and total ammount of data sent per node.
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
		its=atol(argv[2]);
	if (argc>3)
		size=atol(argv[3]);
	if (argc>4){
		printf("WARNING: Too many arguments!!!\n         Use %s <nodes> <iterations> <msg_size>\n",argv[0]);
		exit(-1);
	}

	if (DIMEMAS_HEADER_NEEDED != 0){
		printf("#DIMEMAS:waterfall_%d_%d_%d:0:%d(1",nodes,its,size,nodes);
		for (i=1; i<nodes; i++)
			printf(",1");
		printf("),0\n");
	}

	waterfall(nodes, size);
	return 0;
}
