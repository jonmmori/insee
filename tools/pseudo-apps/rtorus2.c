/**
*@file rtorus3.c
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

long nodes_x=8;		///< Number of nodes in X dimension of the trace.
long nodes_y=8;		///< Number of nodes in Y dimension of the trace.
long nodes;		///< The total amount of nodes.
long size=10000;	///< Size of the burst.

/**
* Prints a trace in the standard output moving in a 2D rectangular torus towards X & Y.
*
*@param nodes	The number of nodes. It should be a quadratic value, if not an error is printed in the error output.
*@param size		The size of message.
*/
void torus2d(long nodes_x, long nodes_y, long size)
{
	long i;	///< Xcoordinate of the node.
	long j;	///< Ycoordinate of the node.

	for (j=0; j<nodes_y; j++)
		for (i=0; i<nodes_x; i++)
		{
			insert_send(i+(j*nodes_x),	((i+1)%nodes_x)+(j*nodes_x),			0,	size); // +X
			insert_send(i+(j*nodes_x),	((i+nodes_x-1)%nodes_x)+(j*nodes_x),	0,	size); // -X
			insert_send(i+(j*nodes_x),	i+(((j+1)%nodes_y)*nodes_x),			0,	size); // +Y
			insert_send(i+(j*nodes_x),	i+(((j+nodes_y-1)%nodes_y)*nodes_x),	0,	size); // -Y

			insert_recv(i+(j*nodes_x),	((i+1)%nodes_x)+(j*nodes_x),			0,	size); // +X
			insert_recv(i+(j*nodes_x),	((i+nodes_x-1)%nodes_x)+(j*nodes_x),	0,	size); // -X
			insert_recv(i+(j*nodes_x),	i+(((j+1)%nodes_y)*nodes_x),			0,	size); // +Y
			insert_recv(i+(j*nodes_x),	i+(((j+nodes_y-1)%nodes_y)*nodes_x),	0,	size); // -Y
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
		size=atol(argv[3]);
	if (argc>4){
		printf("WARNING: Too many arguments!!!\n         Use %s <nodes_x> <nodes_y> <msg_size>\n",argv[0]);
		exit(-1);
	}
	nodes=nodes_x*nodes_y;

	if (DIMEMAS_HEADER_NEEDED != 0){
		printf("#DIMEMAS:rtorus2d_%d_%d_%d:0:%d(1",nodes_x,nodes_y,size,nodes);
		for (i=1; i<nodes; i++)
			printf(",1");
		printf("),0\n");
	}

	torus2d(nodes_x, nodes_y, size);
	return 0;
}
