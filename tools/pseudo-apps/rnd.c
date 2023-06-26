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
#include "event.h"

#define THREAD 0
#define MPI_COMM_WORLD 0

long nodes=64;		///< Number of nodes of the trace.
long size=10000;	///< Size of the burst.
long rnds=13;		///< The random seed.
long its=100;		///< The number of iterations.
long coupled=10;		///< The size of send and recv clusters.

/**
* Prints a randomly generated trace in the standard output. (different coupling levels allowed)
*
*@param nodes	The number of nodes. It should be a power of two value, if not an error is printed in the error output.
*@param size	The size of the burst.
*/
void randomcomm(long nodes, long its, long coupled, long size)
{
	long i,		///< The iteration.
	     s,		///< Source node.
	     d;		///< Destination node.
	event_q *send;	///< A queue containing all the sends of each node.
	event_q *recv;	///< A queue containing all the recvs of each node.
	event_t e;	///< An event.

	send=malloc(nodes*sizeof(event_q));
	recv=malloc(nodes*sizeof(event_q));
	for (i=0; i<nodes; i++)
	{
		init_event(&send[i]);
		init_event(&recv[i]);
	}

	for (i=0; i<its; i++)
	{
		do
		{
			s=rand()%nodes;
			d=rand()%nodes;
		} while (d==s);
		e.pid=d;
		e.task=i;
		ins_event(&send[s], e);
		e.pid=s;
		ins_event(&recv[d], e);
		if ((i%coupled)==coupled-1)
		{
			for (s=0; s<nodes; s++)
			{
				while(!event_empty(&send[s]))
				{
                                        e=head_event(&send[s]);
                                        insert_send(s, e.pid, e.task, size);
                                        rem_head_event(&send[s]);
                                }
				while(!event_empty(&recv[s]))
				{
                                        e=head_event(&recv[s]);
                                        insert_recv(s, e.pid, e.task, size);
                                        rem_head_event(&recv[s]);
                                }
			}
		}
	}

	for (s=0; s<nodes; s++)
	{
		while(!event_empty(&send[s]))
		{
			e=head_event(&send[s]);
			insert_send(s, e.pid, e.task, size);
			rem_head_event(&send[s]);
		}
		while(!event_empty(&recv[s]))
		{
			e=head_event(&recv[s]);
			insert_recv(s, e.pid, e.task, size);
			rem_head_event(&recv[s]);
		}
	}

	free(send);
	free(recv);
}

/**
* Main Function. Accepts up to five parameters: number of nodes, total number of messages, messages per wave,  message size and random seed.
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
		coupled=atol(argv[3]);
	if (argc>4)
		size=atol(argv[4]);
	if (argc>5)
		rnds=atol(argv[5]);
	else
		time(&rnds);
	if (argc>6){
		printf("WARNING: Too many arguments!!!\n         Use %s <nodes> <total_msg> <busrt_size> <msg_size> <rnd_seed>\n",argv[0]);
		exit(-1);
	}

	if (DIMEMAS_HEADER_NEEDED != 0){
		printf("#DIMEMAS:gups_%d_%d_%d_%d_%d:0:%d(1",nodes,its,coupled,size,rnds,nodes);
		for (i=1; i<nodes; i++)
			printf(",1");
		printf("),0\n");
	}

	srand((unsigned int) rnds);
	randomcomm(nodes, its, coupled, size);
	return 0;
}
