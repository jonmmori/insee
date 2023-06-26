/**
* @file
* Event management functions used in trace-driven simulation.
*
* Event: A task that must be done (not occurred yet).
* Occurred: A task that has been done (total or partially completed).
*
*@author Javier Navaridas
*/

#include "event.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
* Initializes an event queue.
* @param q a pointer to the queue to be initialized.
*/
void init_event (event_q *q) {
	q->head = NULL;
	q->tail = NULL;
}

/**
* Adds an event to a queue.
* @param q a pointer to a queue.
* @param i the event to be added to q.
*/
void ins_event (event_q *q, event_t i) {
	event_n *e;
	e=malloc(sizeof(event_n));
	e->ev=i;
	e->next = NULL;

	if(q->head==NULL){ // Empty Queue
		q->head = e;
		q->tail = e;
	}
	else{
		q->tail->next = e;
		q->tail = e;
	}
}

/**
* Looks at the first event in a queue.
* @param q A pointer to the queue.
* @return The first event in the queue (without using nor modifying it).
*/
event_t head_event (event_q *q) {
	if (q->head==NULL)
	{
		perror("Getting event from an empty queue");
		exit(0);
	}
	return q->head->ev;
}

/**
* Deletes the first event in a queue.
* @param q A pointer to the queue.
*/
void rem_head_event (event_q *q) {
	event_n *e;
	if (q->head==NULL)
	{
		perror("Deleting event from an empty queue");
		exit(0);
	}
	e = q->head;
	q->head=q->head->next;
	free (e);
	if (q->head==NULL)
		q->tail=NULL;
}

/**
* Is a queue empty?.
* @param q A pointer to the queue.
* @return TRUE if the queue is empty FALSE in other case.
*/
long event_empty (event_q *q){
	return (q->head==NULL);
}

