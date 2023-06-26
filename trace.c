/**
* @file
* @brief	Tools for trace guided simulation.
*
* In this file are the functions for reading trace files
* & the trace guided simulation running module.
* This file is only used when compiling with TRACE_SUPPORT != 0
*
*@author Javier Navaridas

FSIN Functional Simulator of Interconnection Networks
Copyright (2003-2011) J. Miguel-Alonso, J. Navaridas

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <string.h>

#include "globals.h"
#include "dimemas.h"

#if (TRACE_SUPPORT != 0)

#define FILE_TIME 73		///< Default delay for accessing a file.
#define FILE_SCALE 3		///< Default scale for file accesses based on the size.
#define op_per_cycle 50		///< balances the computation time (cpu time/op_per_cycle)==fsin cycles.
#define cpuspeed  1e6		///< The cpu speed in Mhz.
#define BUFSIZE 131072		///< The size of the buffer,

#define CPU_SCALE 2

void read_dimemas();
void read_fsin_trc();
void read_alog();

void random_placement();
void consecutive_placement();
void shuffle_placement();
void shift_placement();
void column_placement();
void quadrant_placement();
void diagonal_placement();
void icube_placement();
void circulant_placement();
void file_placement();
void apx_placement();
void apx_traces();

long **translation;	///< A matrix containing the simulation nodes for each trace task.

/**
* The trace reader dispatcher selects the format type and calls to the correct trace read.
*
* The selection reads the first character in the file. This could be: '#' for dimemas,
* 'c', 's' or 'r' for fsin trc, and '-' for alog (in complete trace the header is "-1",
* or in filtered trace could be "-101" / "-102"). This is a very naive decision, so we
* probably have to change this, but for the moment it works.
*
*@see read_dimemas
*@see read_fsin_trc
*@see read_alog
*/
void read_trace(){
	FILE * ftrc;
	char c;
	long i;

	translation=alloc(trace_nodes*sizeof(long *));
	for (i=0; i<trace_nodes; i++)
		translation[i]=alloc(trace_instances*sizeof(long));

	switch (placement){
		case CONSECUTIVE_PLACE:
		case ROW_PLACE:
			consecutive_placement();
			break;
		case SHUFFLE_PLACE:
			shuffle_placement();
			break;
		case RANDOM_PLACE:
			random_placement();
			break;
		case SHIFT_PLACE:
			shift_placement();
			break;
		case COLUMN_PLACE:
			column_placement();
			break;
		case QUADRANT_PLACE:
			quadrant_placement();
			break;
		case DIAGONAL_PLACE:
			diagonal_placement();
			break;
		case ICUBE_PLACE:
			icube_placement();
			break;
		case CIRC_PLACE:
			circulant_placement();
			break;
		case FILE_PLACE:
			file_placement();
			break;
        case MPA_PLACE:
            break;
		default:
			panic("Undefined placement strategy");
			break;
	}
    if(placement==MPA_PLACE){
        apx_placement();
        apx_traces();
    }else{
        if((ftrc = fopen(trcfile, "r")) == NULL){
            char message[100];
            sprintf(message, "Trace file not found in current directory - %s",trcfile);
            panic(message);
        }
        c=(char)fgetc(ftrc);
        fclose(ftrc);

        switch (c){
            case '#':
                read_dimemas();
                break;
            case '-':
                read_alog();
                break;
            case 'c':
            case 's':
            case 'r':
                read_fsin_trc();
                break;
            case -1:
                printf("WARNING: Trace file is empty!!!\n");
                break;
            default:
                panic("Unsupported trace format");
                break;
        }
    }

}

void trace_finish(){

        long i;

	for (i=0; i<trace_nodes; i++)
		free(translation[i]);
        free(translation);
}


/**
* Reads a trace from a dimemas file.
*
* Read a trace from a dimemas file whose name is in global variable #trcfile
* It only consideres events for CPU and point to point operations. File I/O could be
* considered as a cpu event if FILEIO is defined.
*/
void read_dimemas() {
	FILE * ftrc;
	char buffer[BUFSIZE];
	long i, n, inst;	///< The number of nodes is read here.
	char sep[]=":";		///< Dimemas record separator.
	char tsep[]="(),";	///< Separators to get the task info.

	char *op_id;		///< Operation id.
	long type;			///< Global operation id(for collectives), point-to-point type.
	long task_id;		///< Task id.
	long th_id;			///< Thread id.
	long t_id;			///< The other task id. (sender or receiver)
	//long comm_id;		///< The communicator id.

	double cpu_burst;	///< CPU time(in seconds).
	long size;			///< Message size(in bytes).
	long tag;			///< Tag of the MPI operation.
	long comm;			///< Communicator id.

	long rtask_id;		///< Root task in collectives.
	long rth_id;		///< Root thread in collectives.
	long bsent;			///< Bytes sent in collectives.
	long brecv;			///< Bytes received in collectives.
	event ev;			///< The read event.

	if((ftrc = fopen(trcfile, "r")) == NULL){
		char message[100];
		sprintf(message, "Trace file not found in current directory - %s",trcfile);
		panic(message);
	}

	if (fgets(buffer,BUFSIZE,ftrc)==NULL)
		panic("Error reading from trace file");

	if (strncmp("#DIMEMAS", buffer, 8))
		/// Could try to open traces in ALOG or FSIN trc format instead of panic....
		panic("Header line is missing, maybe not a dimemas file");

	strtok( buffer, sep );	// Drops the #DIMEMAS.
	strtok( NULL, sep );	// Drops trace_name.
	strtok( NULL, sep);		// Offsets are dropped here.
	n=atol(strtok(NULL, tsep));	// task info has diferent separators
	if (n>trace_nodes)
		panic("Trace has more nodes than stated in trace_nodes");

	while(fgets(buffer, BUFSIZE, ftrc) != NULL) {
		op_id=strtok( buffer, sep);
		if (!strcmp(op_id,"s")) // Offset
			// As we parse the whole file, it is not important for us.
			continue;

		if (!strcmp(op_id,"d")){ // Definitions.
			type=atol(strtok( NULL, sep));
			switch (type){
				case COMMUNICATOR:
					// Not implemented yet.
					break;
				case FILE_IO:
					// It doesn't care about what files are accessed during the execution of the traces.
					// It doesn't even if the FILEIO is active.
					break;
				case OSWINDOW:
					// We could treat the one side windows as MPI communicators.
					break;
				default:
				panic("Wrong definition when reading dimemas file");
			}
			continue;
		}
		task_id=atol(strtok( NULL, sep)); //We have the task id.
		if ( task_id>n || task_id <0 )
			panic ("Task id out of range when reading dimemas file");
		th_id=atol(strtok( NULL, sep)); //We have the thread id.
		switch (atol(op_id)){
		case CPU:
			cpu_burst=atof(strtok( NULL, sep)); //We have the time taken by the CPU.
			ev.type=COMPUTATION;
			ev.length=(long)ceil((cpu_burst*cpuspeed)/op_per_cycle); // Computation time.
			if (ev.length>0){
                ev.count=0;	// Elapsed time.
                if (task_id<trace_nodes && task_id>=0)
                    for (inst=0; inst<trace_instances; inst++){
                        ev.pid=translation[task_id][inst];
                        ins_event(&network[ev.pid].events, ev); // Add event to its node event queue
                    }
                else
                    panic("Adding cpu event into an undefined task when reading dimemas file");
			}
			break;

		case SEND:
			t_id=atol(strtok( NULL, sep)); //We have the destination task id.
			if ( t_id>nprocs || t_id <0 )
				panic ("Destination task id out of range when reading dimemas file");
			size=atol(strtok( NULL, sep)); //We have the size.
			tag=atol(strtok( NULL, sep));  //We have the tag.
			comm=atol(strtok( NULL, sep)); //We have the communicator id.
			type=atol(strtok( NULL, sep)); //We have the send type (I, B, S or -).
			switch (type){
			case NONE:       // This should be Bsend (buffered)
			case RENDEZVOUS: // This should be Ssend (synchronized)
			case IMMEDIATE:  // This should be Isend (inmediate)
			case BOTH:       // This should be Issend(inmediate & synchronized)
				ev.type=SENDING;
				if (task_id !=t_id) { // Valid event
					ev.task=tag; // Type of message
					ev.length=size; // Length of message
					ev.count=0; // Packets sent or received
					if (ev.length == 0)
						ev.length=1;
					ev.length = (long)ceil ( (double)ev.length/(pkt_len*phit_len));
					if (task_id<trace_nodes && t_id<trace_nodes && task_id>=0 && t_id>=0)
						for (inst=0; inst<trace_instances; inst++){
							i=translation[task_id][inst]; // Node to add event
							ev.pid=translation[t_id][inst]; // event's PID: destination when we are sending
							ins_event(&network[i].events, ev); // Add event to its node event queue
						}
					else
						panic("Adding comm event into an undefined task when reading dimemas file");
				}
				break;
			default:
				printf("WARNING: Unexpected Send type when reading dimemas file: %ld!!!\n", type);
				continue;
			}
			break;

		case RECEIVE:
			t_id=atol(strtok( NULL, sep)); //We have the source task id.
			if ( t_id>nprocs || t_id <0 )
				panic ("Source task id out of range when reading dimemas file");
			size=atol(strtok( NULL, sep)); //We have the size.
			tag=atol(strtok( NULL, sep));  //We have the tag.
			comm=atol(strtok( NULL, sep)); //We have the communicator id.
			type=atol(strtok( NULL, sep)); //We have the recv type (Recv, Irecv or Wait).
			switch (type){
			case IRECV: // This is not useful for us.
				break;
			// A reception and a wait is the same for us.
			case RECV:
			case WAIT:
				ev.type=RECEPTION;
				if (t_id!=task_id) {// Valid event
					ev.task=tag; // Type of message
					ev.length=size; // Length of message
					ev.count=0; // Packets sent or received
					if (ev.length == 0)
						ev.length=1;
					ev.length = (long)ceil ( (double)ev.length/(pkt_len*phit_len));
					if (task_id<trace_nodes && t_id<trace_nodes && task_id>=0 && t_id>=0)
						for (inst=0; inst<trace_instances; inst++){
							i=translation[task_id][inst]; // Node to add event
							ev.pid=translation[t_id][inst]; // event's PID: destination when we are sending
							ins_event(&network[i].events, ev); // Add event to its node event queue
						}
					else
						panic("Adding comm event into an undefined task when reading dimemas file");
				}
				break;
			default:
				printf("WARNING: Unexpected Reception type when reading dimemas file: %ld!!!\n",type);
				continue;
			}
			break;

		case COLLECTIVE:
			type=atol(strtok( NULL, sep));     //We have the global operation id.
			comm=atol(strtok( NULL, sep));     //We have the communicator id.
			rtask_id=atol(strtok( NULL, sep)); //We have the root task_id.
			rth_id=atol(strtok( NULL, sep));   //We have the root thread_id.
			bsent=atol(strtok( NULL, sep));    //We have the sent byte count.
			brecv=atol(strtok( NULL, sep));    //We have the received byte count.
			switch (type){
			case OP_MPI_Barrier:
				break;
			case OP_MPI_Bcast:
				break;
			case OP_MPI_Gather:
				break;
			case OP_MPI_Gatherv:
				break;
			case OP_MPI_Scatter:
				break;
			case OP_MPI_Scatterv:
				break;
			case OP_MPI_Allgather:
				break;
			case OP_MPI_Allgatherv:
				break;
			case OP_MPI_Alltoall:
				break;
			case OP_MPI_Alltoallv:
				break;
			case OP_MPI_Reduce:
				break;
			case OP_MPI_Allreduce:
				break;
			case OP_MPI_Reduce_Scatter:
				break;
			case OP_MPI_Scan:
				break;
			default:
				printf("WARNING: Unexpected Collective type when reading dimemas file!!!\n");
				continue;
			}
			break;

		case EVENT:
			// This will be useful to generate paraver output files.
			break;

// IO events could be treated as CPU or NETWORK events.
		case FREAD:
		case FWRITE:
#ifdef FILEIO
			strtok( NULL, sep);       // File Descriptor is dropped here.
			strtok( NULL, sep);       // Required size is dropped here.
			cpu_burst=atol(strtok( NULL, sep));   //We have the size.
			ev.type=COMPUTATION;
			ev.length=(long)ceil((FILE_TIME+(FILE_SCALE*cpu_burst)/op_per_cycle)); // Computation time.
			if (ev.length>0){
                ev.count=0;				// Elapsed time.
                if (task_id<trace_nodes && task_id>=0)
                    for (inst=0; inst<trace_instances; inst++){
                        ev.pid=translation[task_id][inst];
                        ins_event(&network[ev.pid].events, ev); // Add event to its node event queue
                    }
                else
                    panic("Adding cpu event into an undefined task when reading dimemas file");
			}
#endif
			break;

		case FOPEN:
		case FSEEK:
		case FCLOSE:
		case FDUP:
		case FUNLINK:
#ifdef FILEIO
			ev.type=COMPUTATION;
			ev.length=(long)ceil((FILE_TIME)/op_per_cycle); // Computation time.
			if (ev.length>0){
                ev.count=0; // Elapsed time.
                if (task_id<trace_nodes && task_id>=0)
                    for (inst=0; inst<trace_instances; inst++){
                        ev.pid=translation[task_id][inst];
                        ins_event(&network[ev.pid].events, ev); // Add event to its node event queue
                    }
                else
                    panic("Adding cpu event into an undefined task when reading dimemas file");
			}
#endif

			break;
		case IOCOLL:
			break;
		case IOBLOCKNCOLL:
			break;
		case IOBLOCKCOLL:
			break;
		case IONBLOCKNCOLLBEGIN:
			break;
		case IONBLOCKNCOLLEND:
			break;
		case IONBLOCKCOLLBEGIN:
			break;
		case IONBLOCKCOLLEND:
			break;
		case ONESIDEGENOP:
			break;
		case ONESIDEFENCE:
			break;
		case ONESIDELOCK:
			break;
		case ONESIDEPOST:
			break;
		case LAPIOP:
			/// These are communication with different semantic values of the MPI. In study...
#ifdef LAPI
			type=atol(strtok( NULL, sep));      //We have the LAPI operation.
			strtok( NULL, sep);                 //Handler dropped here.
			t_id=atol(strtok( NULL, sep));      //We have the destination task id.
			if ( t_id>=trace_nodes || t_id <0 ){
				printf ("Destination task id is not defined (%ld): Aborting!!!\n",task_id);
				return -1;
			}

			size=atol(strtok( NULL, sep));      //We have the size.

			switch (type){
				case LAPI_Init:
				case LAPI_End:
					break;
				case LAPI_Put:
					break;
				case LAPI_Get:
					break;
				case LAPI_Fence: // Could be a Wait
					break;
				case LAPI_Barrier: // Could be a Barrier
					break;
				case LAPI_Alltoall: // Could be an Alltoall
					break;
				default:
					printf ("Undefined LAPI operation: %ld\n");
					break;
			}
#endif
			break;
		default:
			printf("WARNING: Unexpected operation when reading dimemas file!!!\n");
		}
	}
	fclose(ftrc);
}

/**
* Reads a trace from a file.
*
* Read a trace from a fsin trc file whose name is in global variable #trcfile
* This format only takes in account 'c' CPU, 's' SEND, 'r' RECV, events.
*/
void read_fsin_trc() {
	FILE * ftrc;
	char buffer[512];
	char * tok;
	char sep[]=" \t";
	event ev;
	long i,n1,n2,inst;

	if((ftrc = fopen(trcfile, "r")) == NULL){
		char message[100];
		sprintf(message, "Trace file not found in current directory - %s",trcfile);
		panic(message);
	}

	while(fgets(buffer, 512, ftrc) != NULL) {
		if(buffer[0] != '\n' && buffer[0] != '#') {
			if(buffer[strlen(buffer) - 1] == '\n')
				buffer[strlen(buffer) - 1] = '\0';

			tok = strtok( buffer, sep);

			if (strcmp(tok, "s")==0 || strcmp(tok, "r")==0) { // Communication.
				if (strcmp(tok, "s")==0){
					ev.type=SENDING;
					tok=strtok(NULL, sep); // from
					n1=atol(tok); // Sendet (Node to add event)
					tok=strtok(NULL, sep);
					n2=atol(tok); // event's PID: destination when we are sending
				}
				else{ // if (strcmp(tok, "r")==0)
					ev.type=RECEPTION;
					tok=strtok(NULL, sep); // from
					n2=atol(tok); // event's PID: origin of the message
					tok=strtok(NULL, sep);
					n1=atol(tok); // Destination of the message (local node)
				}

				if (n1!=n2) {
					// Valid event
					tok=strtok(NULL, sep);
					ev.task=atol(tok); // Type of message (tag)
					tok=strtok(NULL, sep);
					ev.length=atol(tok); // Length of message
					ev.count=0; // Packets sent or received
					if (ev.length == 0)
						ev.length=1;
					ev.length = (long)ceil ( (double)ev.length/(pkt_len*phit_len));
					if (n1<trace_nodes && n2<trace_nodes && n1>=0 && n2>=0)
						for (inst=0; inst<trace_instances; inst++){
							i=translation[n1][inst]; // Node to add event
							ev.pid=translation[n2][inst]; // event's PID: destination when we are sending
							ins_event(&network[i].events, ev); // Add event to its node event queue
						}
					else
						panic("Adding comm event into an undefined task when reading trc");
				}
			}

			else if (strcmp(tok, "c")==0){ // Computation.
				ev.type=COMPUTATION;
				tok=strtok(NULL, sep);
				n1=atol(tok); // nodeId.
				tok=strtok(NULL, sep);
				switch (cpu_units){
                    case UNIT_CYCLES:
                        ev.length=((CLOCK_TYPE)atol(tok));
                        break;
                    case UNIT_MILLISECONDS:
                        ev.length=(CLOCK_TYPE)(((atol(tok))*1000*link_bw)/(8*phit_len));
                        break;
                    case UNIT_MICROSECONDS:
                        ev.length=(CLOCK_TYPE)(((atol(tok))*link_bw)/(8*phit_len));
                        break;
                    case UNIT_NANOSECONDS:
                        ev.length=(CLOCK_TYPE)(((atol(tok))*link_bw)/(8000*phit_len));
                        break;
                    default:
                        panic("Unsupported CPU units");

                }
                if (ev.length>0){
                    ev.count=0; // Elapsed time.
                    if (n1<trace_nodes && n1>=0)
                        for (inst=0; inst<trace_instances; inst++){
                            ev.pid=translation[n1][inst]; // Node to add event
                            ins_event(&network[ev.pid].events, ev); // Add event to its node event queue
                        }
                    else
                        panic("Adding cpu event into an undefined task when reading trc");
                }
			}
		}
	}
	fclose(ftrc);
}

/**
* Reads a trace from a file.
*
* Read a trace from an alog file whose name is in global variable #trcfile
* It only consideres events with id '-101'(send) or '-102'(receive).
* The alog format is as follows:
*         p2p_op process0 task0 process1 task1 timestamp tag size
* where:
*         p2p_op: is the point to point operation, i.e. '-101'(send) or '-102'(receive)
*         process0 task0: are the identification of the local process and task
*         process1 task1: are the identification of the remote process and task
*         timestamp: is the timestamp where the event occurred, its value will be dropt
*         tag: is the tag of the mpi operation, it will be use to match sends and receptions
*         size: the size of the message in bytes. it will be translated to number of packets
*
* Collective operations are not processed, so extended traces showing the point-2-point communications
* within the collectives should be used.
*/
void read_alog() {
	FILE * ftrc;
	char buffer[512];
	char * tok;
	event ev;
	long i,n1,n2,inst;
	char sep[]=" \t";

	if((ftrc = fopen(trcfile, "r")) == NULL){
		char message[100];
		sprintf(message, "Trace file not found in current directory - %s",trcfile);
		panic(message);
	}

	while(fgets(buffer, 512, ftrc) != NULL) {
		if(buffer[0] != '\n' && buffer[0] != '#') {
			if(buffer[strlen(buffer) - 1] == '\n')
				buffer[strlen(buffer) - 1] = '\0';

			tok = strtok( buffer, sep);
			if (strcmp(tok, "-101")==0 || strcmp(tok, "-102")==0) {
				if (strcmp(tok, "-101")==0)
					ev.type=SENDING;
				else
					ev.type=RECEPTION;

				tok=strtok(NULL, sep);
				n1=atol(tok); // Node to add event
				tok=strtok(NULL, sep); // Task: Not in Use
				tok=strtok(NULL, sep);
				n2=atol(tok); // event's PID: origin when receiving and destiny when sending

				if (n1!=n2) { // Valid event
					tok=strtok(NULL, sep); // Cycle: Not in Use
					tok=strtok(NULL, sep); // Timestamp: Not in Use
					tok=strtok(NULL, sep);
					ev.task=atol(tok); // Type of message
					tok=strtok(NULL, sep);
					ev.length=atol(tok); // Length of message
					ev.count=0; // Packets sent or received
					if (ev.length == 0)
						ev.length=1;
					ev.length = (long)ceil ( (double)ev.length/(pkt_len*phit_len));
					if (n1<trace_nodes && n2<trace_nodes && n1>=0 && n2>=0)
						for (inst=0; inst<trace_instances; inst++){
							i=translation[n1][inst]; // Node to add event
							ev.pid=translation[n2][inst]; // event's PID: destination when we are sending
							ins_event(&network[i].events, ev); // Add event to its node event queue
						}
					else
						panic("Adding comm event into an undefined task when reading alog");
				}
			}
		}
	}
	fclose(ftrc);
}

/**
* Places the tasks in a random way.
*/
void random_placement(){
	long i, j, d;

	for (i=0; i<nprocs; i++)
	    network[i].source=INDEPENDENT_SOURCE;

	for (i=0; i<trace_nodes; i++)
		for (j=0; j<trace_instances; j++) {
			do{
				d=rand()%nprocs;
			} while (network[d].source!=INDEPENDENT_SOURCE);
			translation[i][j]=d;
			network[d].source=OTHER_SOURCE;
	}
}

/**
* Places the tasks consecutively (node 0 in switch 0 port 0; node 1 in switch 0 port 1, and so on). same as row
*/
void consecutive_placement(){
	long i, j, k, d;

	for (i=0; i<nprocs; i++)
	    network[i].source=INDEPENDENT_SOURCE;
	for (j=0; j<trace_instances; j++){
		k=(j*trace_nodes);
		for (i=0; i<trace_nodes; i++){
			d=i+k;
			translation[i][j]=d;
			network[d].source=OTHER_SOURCE;
		}
	}
}

/**
* Places the tasks alternatively one in each switch (node 0 in switch 0 port 0; node 1 in switch 1 port 0, and so on). (for tree and indirect cube )
*/
void shuffle_placement(){
	long i, j, k, d;

	for (i=0; i<nprocs; i++)
	    network[i].source=INDEPENDENT_SOURCE;

	for (j=0; j<trace_instances; j++){
		k=(j*trace_nodes);
		for (i=0; i<trace_nodes; i++){
			d=((i+k)/(nprocs/stDown))+((i+k)%(nprocs/stDown))*stDown;
			translation[i][j]=d;
			network[d].source=OTHER_SOURCE;
		}
	}
}

/**
* Places the tasks consecutively but shifting a given number of places (node 0 in switch 0 port 0; node 1 in switch 0 port 1, and so on).
*/
void shift_placement(){
	long i, j, k, d;

	for (i=0; i<nprocs; i++)
	    network[i].source=INDEPENDENT_SOURCE;

	for (j=0; j<trace_instances; j++){
		k=(j*trace_nodes);
		for (i=0; i<trace_nodes; i++){
		    d=(i+k+shift_plc)%nprocs;
			translation[i][j]=d;
			network[d].source=OTHER_SOURCE;
		}
	}
}

/**
* Places the tasks consecutively in a cube column.
*/
void column_placement(){
	long i, j, k, d;

	for (i=0; i<nprocs; i++)
	    network[i].source=INDEPENDENT_SOURCE;

	for (j=0; j<trace_instances; j++){
		k=(j*trace_nodes);
		for (i=0; i<trace_nodes; i++){
			d=((i+k)/nodes_x)+((i+k)%nodes_x)*nodes_x;
			translation[i][j]=d;
			network[d].source=OTHER_SOURCE;
		}
	}
}

/**
* Places the tasks in quadrants, isolated subtori, half the size in each dimension.
*
* 2 q. for 1-D, 4 q. for 2-D and 8 q. for 3-D.
*/
void quadrant_placement() {
	long i, j, d;
	long n1, n2=1, n3=1; // nodes/2 in each dimension
	long c1, c2, c3; // coordenates in each dimension of the base placement.

	for (i=0; i<nprocs; i++)
	    network[i].source=INDEPENDENT_SOURCE;

	n1=nodes_per_dim[D_X]/2;
	if (ndim>1)
		n2=nodes_per_dim[D_Y]/2;
	if (ndim>2)
		n3=nodes_per_dim[D_Z]/2;
	for (j=0; j<trace_instances; j++)
		for (i=0; i<trace_nodes; i++){
			c1=(i%n1) + ((j%2)*n1);
			c2=((i/n1)%n2) + (((j/2)%2)*n2);
			c3=((i/(n1*n2))%n3) + (((j/4)%2)*n3);
			d=c1 + (c2*nodes_per_dim[D_X])+(c3*nodes_per_dim[D_X]*nodes_per_dim[D_Y]);
			translation[i][j]=d;
			network[d].source=OTHER_SOURCE;
		}
}

/**
* Places the tasks in a diagonal. only 1 instance and 2D torus.
*/
void diagonal_placement() {
	long i,x,y;
	x=y=0;

	for (i=0; i<nprocs; i++)
	    network[i].source=INDEPENDENT_SOURCE;

	for (i=0; i<nodes_x*nodes_y; i++){
		translation[(nodes_x*y)+x][0]=i;
		network[i].source=OTHER_SOURCE;
		y++;
		if (y == nodes_y){
			y=x+1;
			x=nodes_x-1;
		}
		else{
			x--;
			if (x < 0){
				x=y;
				y=0;
			}
		}
	}
}

/**
* Places the tasks in an indirect cube, keeping a submesh attached to each switch.
*/
void icube_placement() {
	long i, j, t, d;
	long tnodes_x=nodes_x*pnodes_x, // total nodes in the X dimension of the virtual mesh
		 tnodes_y=nodes_y*pnodes_y, // total nodes in the Y dimension of the virtual mesh
		 tnodes_z=nodes_z*pnodes_z; // total nodes in the Z dimension of the virtual mesh
	long tx,ty,tz; // coordenates in the virtual mesh.
	long sx,sy,sz; // switch coordenates in each dimension.
	long nx,ny,nz; // coordenates in each dimension of the node attached to the switch.

	for (i=0; i<nprocs; i++)
	    network[i].source=INDEPENDENT_SOURCE;

printf("%ld x %ld\n",trace_instances, trace_nodes);
printf("virtual  %ld x %ld x %ld\n",tnodes_x, tnodes_y, tnodes_z);
printf("switches %ld x %ld x %ld\n",nodes_x, nodes_y, nodes_z);
printf("nodes    %ld x %ld x %ld\n",pnodes_x, pnodes_y, pnodes_z);

	for (j=0; j<trace_instances; j++)
		for (i=0; i<trace_nodes; i++){
			t=(j*trace_nodes)+i;  // could be done directly with a single for, however it has been done in two fors for the sake of simplicity

			tx=t%tnodes_x;
			ty=(t/tnodes_x)%tnodes_y;
			tz=t/(tnodes_x*tnodes_y);

			sx=tx/pnodes_x;
			nx=tx%pnodes_x;
            sy=ty/pnodes_y;
			ny=ty%pnodes_y;
			sz=tz/pnodes_z;
			nz=tz%pnodes_z;
			d=((sx+(nodes_x*(sy+(nodes_y*sz))))*nodes_per_switch)+(nx+(pnodes_x*(ny+(pnodes_y*nz))));
			translation[i][j]=d;
			printf("(%ld,%ld,%ld)     %ld, %ld -> %ld\n",tx,ty,tz,i,j,d);
			network[d].source=OTHER_SOURCE;
		}
}

/**
* Places the tasks in a nearly square partition of a circulant graph.
*/
void circulant_placement(){
	long i, j, n, t; // indexes of he loops and translation to node and task.
	long task_non_counted=0;	// number of skipped tasks
	long *task;					// array containing a list of skipped tasks
	long *node;					// status of the nodes 0: not used yet, 1 already used.

	task=alloc(nprocs*sizeof(long));
	node=alloc(nprocs*sizeof(long));
	for (i=0; i<nprocs; i++)
		node[i]=0;

	for (j=0; j<nodes_y; j++){
		for (i=0; i<nodes_x; i++){
			t=(j*nodes_x)+i;
			n=((j*s2)%nprocs)+i;
			if(node[n]){	// this node is already in use, need to skip the task for later.
				task[task_non_counted]=t;
				task_non_counted++;
			} else {	//asign node to task
				node[n]=1;
				translation[t][0]=n;
				network[n].source=OTHER_SOURCE;
			}
		}
	}
	j=0;
	for (i=0; i<task_non_counted; i++){
		while(node[j]){	// look for the next free node
			j++;
            translation[task[i]][0]=j;
            node[j]=1;
        }
	}
	printf ("%ld missplaced nodes\n", task_non_counted);
}

/**
* Places the tasks as defined in #placefile.
* The format of this file is very simple:
* node_id, task_id, instance_id
* separated by one or more of the following characters: '<spc>' '<tab>' '_' ',' '.' ':' '-' '>'
*/
void file_placement() {
	FILE * fp;
	char buffer[512];
	long node,task,inst;
	char sep[]=",.:_-> \t";

	for (node=0; node<nprocs; node++)
	    network[node].source=INDEPENDENT_SOURCE;

	for (task=0; task<trace_nodes; task++)
		for (inst=0; inst<trace_instances; inst++)
			translation[task][inst]=-1;
	if((fp = fopen(placefile, "r")) == NULL){
		char message[100];
		sprintf(message, "Placement file not found in current directory - %s",placefile);
		panic(message);
	}

	while(fgets(buffer, 512, fp) != NULL) {
		if(buffer[0] != '\n' && buffer[0] != '#') {
			if(buffer[strlen(buffer) - 1] == '\n')
				buffer[strlen(buffer) - 1] = '\0';
			node = atol(strtok( buffer, sep));
			task = atol(strtok( NULL, sep));
			inst = atol(strtok( NULL, sep));
			if (translation[task][inst]!=-1)
				printf("Warning: task %ld.%ld is being redefined\n",task,inst);
			if (network[node].source==OTHER_SOURCE)
				printf("Warning: node %ld is being redefined\n",node);
			translation[task][inst]=node;
		    network[node].source=OTHER_SOURCE;
		}
	}
}

/**
* Places the apps in a list as defined in #appmix_file.
* The format of this file is:
* tracefile_name, num_nodes, node_list
* separated by: <'_'>
*/
void apx_placement(){
    FILE *fp;
    char *buffer[NUMNODES*6];
    char *name;
    long *nodelist;
    long n_nodes;
    char *sep = "_";
    int id_t=0;
    int i;
    if((fp = fopen(mpa_file, "r"))==NULL) {
        panic("appmix: appmix_file not found!");
    }
    while(fgets(buffer,NUMNODES*8, fp)!=NULL){
        if(buffer[0]!='\n' && buffer[0] != '#'){
            if(buffer[strlen(buffer)-1] =='\n')
                buffer[strlen(buffer)-1] = '\0';
            name = strtok(buffer,sep);
            n_nodes = atol(strtok(NULL, sep));
            nodelist = malloc(n_nodes*sizeof(long));
            for (i = 0; i <n_nodes ; ++i) {
                nodelist[i] = atol(strtok(NULL, sep));
            }
            head = addapp(head, name, n_nodes, nodelist,id_t);
            id_t++;
        }
    }
}
/**
 * TODO
 */
void apx_traces(){
    FILE *fd;
    char buffer[512];
    char *tok;
    char sep[]=" \t";
    event ev;
    appnode actual;
    actual=head;
    char *tracefile;
    long source, dest;
    long i;
    int j;
    while(actual != NULL){
        if(actual->running==0){
            if(check_node_disp(actual->node_list, actual->nodes)==1){
                return;
            }
            actual->running=1;
            tracefile = actual->filename;
            if((fd = fopen(tracefile, "r")) == NULL) {
                char message[100];
                sprintf(message, "Trace file not found in current directory - %s", tracefile);
                panic(message);
            }
            for(j=0;j<actual->nodes;j++){
                network[actual->node_list[j]].source=OTHER_SOURCE;
                network[actual->node_list[j]].appid=actual->id;
            }
            //load_app
            while(fgets(buffer, 512, fd) != NULL) {
                if(buffer[0] != '\n' && buffer[0] != '#') {
                    if(buffer[strlen(buffer) - 1] == '\n')
                        buffer[strlen(buffer) - 1] = '\0';

                    tok = strtok( buffer, sep);

                    if (strcmp(tok, "s")==0 || strcmp(tok, "r")==0) { // Communication.
                        if (strcmp(tok, "s")==0){
                            ev.type=SENDING;
                            ev.app_id=actual->id;
                            tok=strtok(NULL, sep); // from
                            source=atol(tok); // Sendet (Node to add event)
                            tok=strtok(NULL, sep);
                            dest=atol(tok); // event's PID: destination when we are sending
                        }
                        else{ // if (strcmp(tok, "r")==0)
                            ev.type=RECEPTION;
                            tok=strtok(NULL, sep); // from
                            dest=atol(tok); // event's PID: origin of the message
                            tok=strtok(NULL, sep);
                            source=atol(tok); // Destination of the message (local node)
                        }
                        if (source!=dest) {
                            // Valid event
                            tok=strtok(NULL, sep);
                            ev.task=atol(tok); // Type of message (tag)
                            tok=strtok(NULL, sep);
                            ev.length=atol(tok); // Length of message
                            ev.count=0; // Packets sent or received
                            if (ev.length == 0)
                                ev.length=1;
                            ev.length = (long)ceil ( (double)ev.length/(pkt_len*phit_len));
                            i = actual->node_list[source]; //Node to add event
                            ev.pid = actual->node_list[dest]; //Destination node
                            actual->ini_clock = sim_clock;
                            ins_event(&network[i].events, ev);
                            network[i].appid = actual->id;
                        }
                    }
                    else if (strcmp(tok, "c")==0){ // Computation.
                        ev.type=COMPUTATION;
                        tok=strtok(NULL, sep);
                        source=atol(tok); // nodeId.
                        tok=strtok(NULL, sep);
                        switch (cpu_units){
                            case UNIT_CYCLES:
                                ev.length=((CLOCK_TYPE)atol(tok));
                                break;
                            case UNIT_MILLISECONDS:
                                ev.length=(CLOCK_TYPE)(((atol(tok))*1000*link_bw)/(8*phit_len));
                                break;
                            case UNIT_MICROSECONDS:
                                ev.length=(CLOCK_TYPE)(((atol(tok))*link_bw)/(8*phit_len));
                                break;
                            case UNIT_NANOSECONDS:
                                ev.length=(CLOCK_TYPE)(((atol(tok))*link_bw)/(8000*phit_len));
                                break;
                            default:
                                panic("Unsupported CPU units");

                        }
                        if (ev.length>0){
                            ev.count=0; // Elapsed time.
                            ev.pid = actual->node_list[source];
                            ins_event(&network[ev.pid].events,ev);
                            network[ev.pid].source=OTHER_SOURCE;
                            network[ev.pid].appid=actual->id;
                        }
                    }
                }
            }
            fclose(fd);
        }
        actual=actual->next;
    }

}

#if (SKIP_CPU_BURSTS==1)
CLOCK_TYPE skipped_cycles=0, skipped_periods=0;

/**
* Looks for the next CPU to finish and skips as many cycles as necessary (if network load is 0). Should speed up the execution of CPU-biased applications)
*/
void skip_if_cpu_activity_only(){
    long i;
    CLOCK_TYPE res=CLOCK_MAX;
    event e;

    if (injected_count - rcvd_count != 0){ // there is traffic going through the network
        return ;
    }

    for (i=0; i<nprocs; i++) {
        if (network[i].source!=INDEPENDENT_SOURCE && !event_empty(&network[i].events)){
            e=head_event(&network[i].events);
            if (e.type==COMPUTATION){
                if (e.length-e.count<res)
                    res=e.length-e.count;
            } else if (e.type==SENDING){
                return ;
			}
		}
    }

    if (res!=CLOCK_MAX){
		printf("%11"PRINT_CLOCK":: Skipped %"PRINT_CLOCK" cycles due to CPU-only activity\n",sim_clock,res);
		skipped_cycles+=res;
		skipped_periods++;

        sim_clock+=res; // current cycles hasn't been counted yet.
        for (i=0; i<nprocs; i++) {
            if (network[i].source!=INDEPENDENT_SOURCE && !event_empty(&network[i].events)){
                e=head_event(&network[i].events);
                if (e.type==COMPUTATION){
                    do_event_n_times (&network[i].events, &e, res);
				}
            }
       }
    }
}
#endif //SKIP_CPU_BURSTS

#if (CHECK_TRC_DEADLOCK>0)
/**
* Reports all the tasks that are in RECV state and which is the source task
*/
void report_receiving_tasks(){
    long i;
    event e;

    for (i=0; i<nprocs; i++) {
        if (network[i].source!=INDEPENDENT_SOURCE && !event_empty(&network[i].events)) {
            e=head_event(&network[i].events);
            printf("Task %ld :: ",i);
            if(e.type==RECEPTION)
                printf("waiting for Task %5ld (tag: %ld len: %lld)\n",e.pid,e.task,e.length);
            else if(e.type==SENDING)
                printf("sending to Task %5ld (tag: %ld len: %lld)\n",e.pid,e.task,e.length);
            else
                printf("computing for %"PRINT_CLOCK" cycles (%"PRINT_CLOCK"  left)\n", e.length, e.length-e.count);
        }
#if (REPORT_ALL_TASKS!= 0)
        else{
            if (network[i].source==INDEPENDENT_SOURCE)
                printf("Task %5ld is independent\n",i);
            else // if (event_empty(&network[i].events))
                printf("Task %5ld is done\n",i);
        }
#endif // REPORT_ALL_TASKS
    }
}

/**
* Checks whether a trace execution has reached to a deadlock state, i.e. all tasks waiting for others
*
*@return B_FALSE if any active task which is not in RECV state, B_TRUE otherwise
*/
bool_t deadlocked_trc(){
    long i;
    event e;

    for (i=0; i<nprocs; i++) {
        if (network[i].source!=INDEPENDENT_SOURCE && !event_empty(&network[i].events) && (e=head_event(&network[i].events)).type!=RECEPTION)
            return B_FALSE;
    }
    return B_TRUE;
}

long deadlocked_period=0;
#endif /* CHECK_TRC_DEADLOCK */

/**
* Runs simulation using a trace file as workload.
*
* In this mode, simulation are running until all the events in the nodes queues are done.
* It prints partial results each pinterval simulation cycles & calculates global
* queues states for global congestion control.
*
* @see read_trace()
* @see init_functions
* @see run_network
*/
void run_network_trc() {

	long i;

	read_trace();

	do {
#if (CHECK_TRC_DEADLOCK>0)
		if (deadlocked_trc() && pattern!=MPA){
            if(deadlocked_period++>CHECK_TRC_DEADLOCK){
                report_receiving_tasks();
                abort_sim("All the tasks have been waiting for each other for more than the threshold (CHECK_TRC_DEADLOCK). The application is probably deadlocked");
            }
		} else
            deadlocked_period=0;
#endif /* CHECK_TRC_DEADLOCK */
#if (SKIP_CPU_BURSTS==1)
		skip_if_cpu_activity_only();
#endif /* SKIP_CPU_BURSTS */
		data_movement(B_TRUE);
        /*
         * AQUI CHECK DE TODAS LAS APPS poner INDEPENDENT_SOURCE en todas los nodos liberados
         * Luego llamar a print partials
         */
		sim_clock++;

		if ((pheaders > 0) && (sim_clock % pinterval == 0)) {
            if(pattern!=MPA)
			    print_partials();
		}
		if ((sim_clock%update_period) == 0) {
			global_q_u = global_q_u_current;
			global_q_u_current = injected_count - rcvd_count;
		}
		go_on=B_FALSE;
		for (i=0; i<nprocs; i++) {
			if (!event_empty (&network[i].events) /*&& network[i].source==OTHER_SOURCE*/) {
				go_on=B_TRUE;
				break;
			}
		}
	} while (go_on && !interrupted  && !aborted);
    if(pattern!=MPA)
	    print_partials();
	save_batch_results();
	reset_stats();
}
#endif

