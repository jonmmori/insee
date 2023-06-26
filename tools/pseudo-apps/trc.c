/**
*@file alog.c
*@author Javier Navaridas Palma.
*
*@brief	This file defines some operations for trace print.
*/

#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "alog.h"

long DIMEMAS_HEADER_NEEDED=0;
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
	printf ("c %d %d\n",from,size*10);
	printf ("s %d %d %d %d\n",from,to,tag,size);
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
	printf ("r %d %d %d %d\n",from,to,tag,size);
}
