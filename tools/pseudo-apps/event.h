/** 
* @file
* Definition of event, event queue & event list, used for trace driven simulation. 
*/

/**
* Structure that defines an event.
*
* It contains all the needed atributes for distinguish it:
* Other party id (destination/source), the tag and the length in bytes.
*/
typedef struct event {
	long pid;		///< The other node (processor id).
	long task;		///< An id for distinguish messages.
} event_t;
   
/**
* Structure that defines a node for event queues.
*/
typedef struct event_n {
	event_t ev;				///< The event in this position.
	struct event_n * next;	///< The next node in the list/queue.
} event_n;

/**
* Structure that defines an event queue.
*/
typedef struct event_q {
	event_n *head;	///< A pointer to the first event node (for removing).
	event_n *tail;	///< A pointer to the last event node (for enqueuing).
} event_q;

void init_event (event_q *q); 
void ins_event (event_q *q, event_t i); 
event_t head_event (event_q *q); 
void rem_head_event (event_q *q); 
long event_empty (event_q *q);

