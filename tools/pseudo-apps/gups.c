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
#include <time.h>

#include "alog.h"
#define THREAD 0
#define MPI_COMM_WORLD 0

long nodes=64;		///< Number of nodes of the trace.
long size=10000;	///< Size of the burst.
long rnds=13;		///< random seed
long its=100;		///< iterations (i.e., #messages)

/**
* Prints a trace in the standard output emulating gups (giga update per second). (random)
*
*@param nodes	The number of nodes. It should be a power of two value, if not an error is printed in the error output.
*@param size	The size of the burst.
*/
void gups(long nodes, long its, long size)
{
	long i;   ///< The iteration.
	long s;  ///< The source node.
	long d; ///< The destination node.

	for (i=0; i<its; i++)
	{
		do{
			s=rand()%nodes;
			d=rand()%nodes;
		} while(s==d);
		insert_send(s,	d,	i,	size);
		insert_recv(d,	s,	i,	size);
	}
}

/**
* Main Function. Accepts up to four parameters: number of nodes, number of iterations, message size and random seed.
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
    if (argc>4)
	    rnds=atol(argv[4]);
	else
	    time(&rnds);
	if (argc>5){
		printf("WARNING: Too many arguments!!!\n         Use %s <nodes> <total_msg> <msg_size> <rnd_seed>\n",argv[0]);
		exit(-1);
	}

	if (DIMEMAS_HEADER_NEEDED != 0){
		printf("#DIMEMAS:gups_%d_%d_%d_%d:0:%d(1",nodes,its,size,rnds,nodes);
		for (i=1; i<nodes; i++)
			printf(",1");
		printf("),0\n");
	}

	srand((unsigned int) rnds);
	gups(nodes, its, size);
	return 0;
}
