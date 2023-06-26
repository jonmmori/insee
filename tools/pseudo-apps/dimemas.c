/** 
*@file dimemas.c
*@author Javier Navaridas Palma.
*		
*@brief	This file defines some operations for dimemas trace print.
*/

#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "dimemas.h"
#define THREAD 0
#define MPI_COMM_WORLD 0

long DIMEMAS_HEADER_NEEDED=1;

/**
* Print a send line in the standard output.
*
*@param from	The source node.
*@param to	The destination node.
*@param tag	The MPI tag for this message.
*@param size	The size in bytes of the message.
*/
void insert_send(long from, long to, long tag, long size)
{
	printf ("%d:%d:%d:%d:%d:%d:%d:%d\n",SEND,from,THREAD,to,size,tag,MPI_COMM_WORLD,NONE);
}

/**
* Print a recv line in the standard output.
*
*@param to	The destination node.
*@param from	The source node.
*@param tag	The MPI tag for this message.
*@param size	The size in bytes of the message.
*/
void insert_recv(long to, long from, long tag, long size)
{
	printf ("%d:%d:%d:%d:%d:%d:%d:%d\n",RECEIVE,to,THREAD,from,size,tag,MPI_COMM_WORLD,RECV);
}
