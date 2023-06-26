/**
*@file rsweep2.c
*@author Javier Navaridas Palma.
*
*@brief	This file creates a nearest neighbor communication in a 2D torus.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "alog.h"
#define THREAD 0
#define MPI_COMM_WORLD 0

long nodes_x=4;		///< Number of nodes in X dimension of the trace.
long nodes_y=4;		///< Number of nodes in Y dimension of the trace.
long nodes;		///< The total amount of nodes.
long size=10;	///< Size of the burst.
long its=10;

/**
* Prints a trace in the standard output moving in a 2D rectangular torus towards X & Y.
*
*@param nodes	The number of nodes. It should be a quadratic value, if not an error is printed in the error output.
*@param size		The size of message.
*/
void rwaterfall(long nodes_x, long nodes_y, long size)
{
	long i;	///< Xcoordinate of the node.
	long j;	///< Ycoordinate of the node.
	long n; ///< iteration.

	for (j=0; j<nodes_y; j++)
		for (i=0; i<nodes_x; i++)
			for (n=0; n<its; n++)
			{
				if (i!=0)
					insert_recv(i+(j*nodes_x),	((i+nodes_x-1)%nodes_x)+(j*nodes_x),	n,	size); // -X
				if (j!=0)
					insert_recv(i+(j*nodes_x),	i+(((j+nodes_y-1)%nodes_y)*nodes_x),	n,	size); // -Y

				if (i!=nodes_x-1)
					insert_send(i+(j*nodes_x),      ((i+1)%nodes_x)+(j*nodes_x),            n,      size); // +X
				if (j!=nodes_y-1)
					insert_send(i+(j*nodes_x),      i+(((j+1)%nodes_y)*nodes_x),            n,      size); // +Y
			}
}

/**
* Main Function. Accepts up to three parameters: nodes per row of the torus, nodes per column of the torus and message size.
*
*@param argc The number of parameters of the function call.
*@param argv The arguments of the program.
*@return A finalization code.
*/
int main(int argc, char *argv[])
{
	long i;		///< index in loops.

	if (argc>1)
		nodes_x=atol(argv[1]);
	if (argc>2)
		nodes_y=atol(argv[2]);
	if (argc>3)
		its=atol(argv[3]);
	if (argc>4)
		size=atol(argv[4]);
	if (argc>5){
		printf("WARNING: Too many arguments!!!\n         Use %s <nodes_x> <nodes_y> <iterations> <msg_size>\n",argv[0]);
		exit(-1);
	}

	nodes=nodes_x*nodes_y;

	if (DIMEMAS_HEADER_NEEDED != 0){
		printf("#DIMEMAS:rwaterfall_%d_%d_%d_%d:0:%d(1",nodes_x,nodes_y,its,size,nodes);
		for (i=1; i<nodes; i++)
			printf(",1");
		printf("),0\n");
	}

	rwaterfall(nodes_x, nodes_y, size);
	return 0;
}
