/**
*@file torus3.c
*@author Javier Navaridas Palma.
*
*@brief	This file creates a trace moving in a 3D torus towards X, Y & Z.
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
* Prints a trace in the standard output moving in a 3D torus towards X, Y & Z.
*
*@param nodes	The number of nodes. It should be a cubic value, if not an error is printed in the error output.
*@param size		The size of message.
*/
void torus3d(long nodes, long size)
{	long dist;	///< nodes ^ 1/3
	long dist2;	///< nodes ^ 2/3

	dist=(long)pow(nodes, 1.0/3);
	while(dist*dist*dist<nodes)
		dist++;
	if (dist*dist*dist!=nodes)
		perror("Number of Nodes is not a cubic value.\n");
	else
	{
		long i;		///< The number of the node.
		long n;		///< The total amount of bytes sent
		dist2=dist*dist;
		for (i=0; i<nodes; i++)
			for (n=0; n<dist; n++)
			{
				insert_send(i,	(i+dist2)%nodes,			n,	size); // +Z
				insert_send(i,	((i/dist2)*dist2)+((i+dist)%dist2),	n,	size); // +Y
				insert_send(i,	((i/dist)*dist)+((i+1)%dist),		n,	size); // +X
				insert_recv(i,	(i-dist2+nodes)%nodes,			n,	size); // -Z
				insert_recv(i,	((i/dist2)*dist2)+((dist2+i-dist)%dist2),n,	size); // -Y
				insert_recv(i,	((i/dist)*dist)+((dist+i-1)%dist),	n,	size); // -X
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
		printf("#DIMEMAS:torus3d_%d_%d:0:%d(1",nodes,size,nodes);
		for (i=1; i<nodes; i++)
			printf(",1");
		printf("),0\n");
	}

	torus3d(nodes, size);
	return 0;
}
