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
long size=10000;	///< Size of the burst.

/**
* Prints a trace in the standard output performing a butterfly interchange. (allReduce)
* For each level of the butterfly the size of the message is reduced.
*
*@param nodes	The number of nodes. It should be a power of two value, if not an error is printed in the error output.
*@param size	The size of the burst.
*/
void butterfly(long nodes, long size)
{
	long N=0; ///< log2 nodes
	long k=1; ///< 2 ^ N

	while (k<nodes)
	{
		N++;
		k=2*k;
	}

	if (nodes!=k)
		perror("Number of Nodes is not a power of two\n");
	else
	{
		long i;					///< The number of the node.
		long n;					///< The iteration
		long pn, pn1;				///< 2 ^ n & 2 ^ (n+1)

		for (i=0; i<nodes; i++)
		{
			pn=1;
			pn1=2;
			for (n=0; n<N; n++)
			{
				if (i%pn1<pn)
				{
					insert_send(i,	i+pn,	n,	size);
					insert_recv(i,	i+pn,	n,	size);
				}
				else
				{
					insert_send(i,	i-pn,	n,	size);
					insert_recv(i,	i-pn,	n,	size);
				}
				pn=pn1;
				pn1=2*pn1;
			}
		}
	}
}

/**
* Prints a trace in the standard output performing a butterfly interchange. (allToAll)
*
*@param nodes	The number of nodes. It should be a power of two value, if not an error is printed in the error output.
*@param size	The size of the burst.
*/
void butterfly2(long nodes, long size)
{
	long N=0; ///< log2 nodes
	long k=1; ///< 2 ^ N

	while (k<nodes)
	{
		N++;
		k=2*k;
	}

	if (nodes!=k)
		perror("Number of Nodes is not a power of two\n");
	else
	{
		long i;					///< The number of the node.
		long n;					///< The iteration
		long msgsize;				///< The size of each message.
		long pn, pn1;				///< 2 ^ n & 2 ^ (n+1)
		for (i=0; i<nodes; i++)
		{
			pn=1;
			pn1=2;
			for (n=0; n<N; n++)
			{
				msgsize=size*pn;
				if (i%pn1<pn)		// Waits for -1
				{
					insert_send(i,	i+pn,	n,	msgsize);
					insert_recv(i,	i+pn,	n,	msgsize);
				}
				else
				{
					insert_send(i,	i-pn,	n,	msgsize);
					insert_recv(i,	i-pn,	n,	msgsize);
				}
				pn=pn1;
				pn1=2*pn1;
			}
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
		printf("#DIMEMAS:butterfly_%d_%d:0:%d(1",nodes,size,nodes);
		for (i=1; i<nodes; i++)
			printf(",1");
		printf("),0\n");
	}

	butterfly(nodes, size);
	return 0;
}
