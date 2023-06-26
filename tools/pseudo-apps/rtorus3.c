/**
*@file rtorus3.c
*@author Javier Navaridas Palma.
*
*@brief	This file creates a nearest neighbor communication in a 3D torus.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "alog.h"
#define THREAD 0
#define MPI_COMM_WORLD 0

long n_x=4;		///< Number of nodes in X dimension of the trace.
long n_y=4;		///< Number of nodes in Y dimension of the trace.
long n_z=4;		///< Number of nodes in Z dimension of the trace.
long nodes;		///< The total amount of nodes.
long size=10000;	///< Size of the burst.

/**
* Prints a trace in the standard output moving in a 3D rectangular torus towards X, Y and Z.
*
*@param nodes	The number of nodes. It should be a quadratic value, if not an error is printed in the error output.
*@param size		The size of message.
*/
void rtorus3d(long n_x, long n_y, long n_z, long size)
{
	long i;	///< Xcoordinate of the node.
	long j;	///< Ycoordinate of the node.
	long k;	///< Zcoordinate of the node.

	for (k=0; k< n_z; k++)
		for (j=0; j<n_y; j++)
			for (i=0; i<n_x; i++)
			{
				insert_send(i+(j*n_x)+(k*n_x*n_y),	((i+1)%n_x)+(j*n_x)+(k*n_x*n_y),		0,	size); // +X
				insert_send(i+(j*n_x)+(k*n_x*n_y),	((i+n_x-1)%n_x)+(j*n_x)+(k*n_x*n_y),	0,	size); // -X
				insert_send(i+(j*n_x)+(k*n_x*n_y),	i+(((j+1)%n_y)*n_x)+(k*n_x*n_y),		0,	size); // +Y
				insert_send(i+(j*n_x)+(k*n_x*n_y),	i+(((j+n_y-1)%n_y)*n_x)+(k*n_x*n_y),	0,	size); // -Y
				insert_send(i+(j*n_x)+(k*n_x*n_y),	i+(j*n_x)+(((k+1)%n_z)*n_x*n_y),		0,	size); // +Z
				insert_send(i+(j*n_x)+(k*n_x*n_y),	i+(j*n_x)+(((k+n_z-1)%n_z)*n_x*n_y),	0,	size); // -Z

				insert_recv(i+(j*n_x)+(k*n_x*n_y),	((i+1)%n_x)+(j*n_x)+(k*n_x*n_y),		0,	size); // +X
				insert_recv(i+(j*n_x)+(k*n_x*n_y),	((i+n_x-1)%n_x)+(j*n_x)+(k*n_x*n_y),	0,	size); // -X
				insert_recv(i+(j*n_x)+(k*n_x*n_y),	i+(((j+1)%n_y)*n_x)+(k*n_x*n_y),		0,	size); // +Y
				insert_recv(i+(j*n_x)+(k*n_x*n_y),	i+(((j+n_y-1)%n_y)*n_x)+(k*n_x*n_y),	0,	size); // -Y
				insert_recv(i+(j*n_x)+(k*n_x*n_y),	i+(j*n_x)+(((k+1)%n_z)*n_x*n_y),		0,	size); // +Z
				insert_recv(i+(j*n_x)+(k*n_x*n_y),	i+(j*n_x)+(((k+n_z-1)%n_z)*n_x*n_y),	0,	size); // -Z
			}
}

/**
* Main Function. Accepts up to four parameters: number of nodes per each dimension (X, Y, Z) and message size.
*
*@param argc The number of parameters of the function call.
*@param argv The arguments of the program.
*@return A finalization code.
*/
int main(int argc, char *argv[])
{
	long i;		///< index in loops.

	if (argc>1)
		n_x=atol(argv[1]);
	if (argc>2)
		n_y=atol(argv[2]);
	if (argc>3)
		n_z=atol(argv[3]);
	if (argc>4)
		size=atol(argv[4]);
	if (argc>5){
		printf("WARNING: Too many arguments!!!\n         Use %s <nodes_x> <nodes_y> <nodes_z> <msg_size>\n",argv[0]);
		exit(-1);
	}
	nodes=n_x*n_y*n_z;

	if (DIMEMAS_HEADER_NEEDED != 0){
		printf("#DIMEMAS:rtorus3d_%d_%d_%d_%d:0:%d(1",n_x,n_y,n_z,size,nodes);
		for (i=1; i<nodes; i++)
			printf(",1");
		printf("),0\n");
	}

	rtorus3d(n_x, n_y, n_z, size);
	return 0;
}
