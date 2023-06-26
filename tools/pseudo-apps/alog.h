/**
*@file dimemas.h
*@author Javier Navaridas Palma.
*
*@brief	This file defines the dimemas trace format given by the UPC.
*/

void insert_send(long from, long to, long tag, long size);
void insert_recv(long to, long from, long tag, long size);

extern long DIMEMAS_HEADER_NEEDED;

#define THREAD 0
#define TS 0
#define SEND -101
#define RECEIVE -102

