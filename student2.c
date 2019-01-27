#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "project2.h"

/* ***************************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for unidirectional or bidirectional
   data transfer protocols from A to B and B to A.
   Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets may be delivered out of order.

   Compile as gcc -g project2.c student2.c -o p2
**********************************************************************/

// Holds state information
unsigned int A_seqnum;
unsigned int B_seqnum;
unsigned int active;

// Holds constants
const int timeout = 100;
const int traceOutputLevel = 3;
const int A_buffer_size = 100;

// Structs in A
struct pkt A_pkt;
struct msg A_buffer[100];
unsigned int A_buffer_start;
unsigned int A_buffer_end;

// Structs in B
struct pkt B_pkt;
struct msg B_message;

// Global variables from project2.c
int TraceLevel;
double CurrentSimTime;

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
/* 
 * The routines you will write are detailed below. As noted above, 
 * such procedures in real-life would be part of the operating system, 
 * and would be called by other procedures in the operating system.  
 * All these routines are in layer 4.
 */

/**
 * Gets the 32-bit weighted checksum of a message
 * @param packet The packet to get a checksum for
 * @return The checksum
 */
int getChecksum(struct pkt packet) {
	unsigned int checksum = 0;

	// Sum checksum from payload
	unsigned int weight = 7;
	while(weight <= MESSAGE_LENGTH + 6) {
		unsigned int next = packet.payload[weight - 7];
		checksum += next * weight;
		checksum = (checksum & 0x00000000FFFFFFFF) + ((checksum & 0xFFFFFFFF00000000) >> 32);
		// Second addition at different weight helps prevent duplicate checksums
		checksum += next * (weight - 5);
		checksum = (checksum & 0x00000000FFFFFFFF) + ((checksum & 0xFFFFFFFF00000000) >> 32);
		weight++;
	}
	// Sum checksum with acknum and seqnum
	checksum += (3 + packet.seqnum) * 99;
	checksum = (checksum & 0x00000000FFFFFFFF) + ((checksum & 0xFFFFFFFF00000000) >> 32);
	checksum += (3 + packet.acknum) * 77;
	checksum = (checksum & 0x00000000FFFFFFFF) + ((checksum & 0xFFFFFFFF00000000) >> 32);
	//Flip bits
	checksum = ~checksum;
	return checksum;
}

/**
 * Clears a pkt
 * @param packet The packet to clear
 */

void clearPkt(struct pkt packet) {
	packet.seqnum = 0;
	packet.acknum = 0;
	packet.checksum = 0;
	memset(packet.payload, 0, sizeof(char) * MESSAGE_LENGTH);
	//packet.payload[MESSAGE_LENGTH - 1] = '\0';
}

/**
 * Clears a msg
 * @param message The msg to clear
 */
void clearMsg(struct msg message) {
	memset(message.data, 0, sizeof(char) * MESSAGE_LENGTH);
	//message.data[MESSAGE_LENGTH -1] = '\0';
}

/**
 * Deep copies a msg
 * @param message The msg to copy
 * @return A deep copy of the msg
 */
struct msg copyMsg(struct msg message) {
	struct msg copy;
	strcpy(copy.data, message.data);
	return copy;
}

/**
 * Print pkt
 */
void printPkt(struct pkt packet) {
	printf("%3d %3d %8d %s\n", packet.seqnum, packet.acknum, packet.checksum, packet.payload);
}

/* 
 * A_output(message), where message is a structure of type msg, containing 
 * data to be sent to the B-side. This routine will be called whenever the 
 * upper layer at the sending side (A) has a message to send. It is the job 
 * of your protocol to insure that the data in such a message is delivered 
 * in-order, and correctly, to the receiving side upper layer.
 */
void A_output(struct msg message) {
	// If packet not yet ACKed, buffer
	if(active) {
		A_buffer[A_buffer_end] = copyMsg(message);
		A_buffer_end++;
		if(A_buffer_end >= A_buffer_size - 1) {
			A_buffer_end = 0;
		}
	} else {
		// Create and send a packet to B
		clearPkt(A_pkt);
		strcpy(A_pkt.payload, message.data);
		A_pkt.seqnum = A_seqnum;
		A_pkt.checksum = getChecksum(A_pkt);
		tolayer3(AEntity, A_pkt);
		startTimer(AEntity, timeout);

		active = 1;
		if(TraceLevel >= traceOutputLevel) {
			printf("%10s%25s", "", "A_pkt sent: ");
			printPkt(A_pkt);
		}
	}
}

/*
 * Just like A_output, but residing on the B side.  USED only when the 
 * implementation is bi-directional.
 */
void B_output(struct msg message)  {

}

/* 
 * A_input(packet), where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the B-side (i.e., as a result 
 * of a tolayer3() being done by a B-side procedure) arrives at the A-side. 
 * packet is the (possibly corrupted) packet sent from the B-side.
 */
void A_input(struct pkt packet) {
	if(TraceLevel >= traceOutputLevel) {
		printf("%10s%25s", "", "B_pkt received: ");
		printPkt(packet);
	}

	if(packet.checksum == getChecksum(packet)  && packet.acknum == A_seqnum && (strcmp(packet.payload, "ACK") == 0) && active) {
		// If successfully recieved a packet
		A_seqnum = 1 - A_seqnum;
		stopTimer(AEntity);
		active = 0;

		if(TraceLevel >= traceOutputLevel) {
			printf("%s\n", "Confirmed proper ACK.");
			printf("%s\n", "-----");
		}
		// If messages in buffer, send those
		if(A_buffer_start != A_buffer_end) {
			if(TraceLevel >= traceOutputLevel) {
				printf("%10s%25s\n", "", "Buffer Message: ");
			}
			A_output(A_buffer[A_buffer_start]);
			clearMsg(A_buffer[A_buffer_start]);
			A_buffer_start++;
			if(A_buffer_start >= A_buffer_size - 1) {
				A_buffer_start = 0;
			}
		}
	} else if(TraceLevel >= traceOutputLevel && packet.checksum != getChecksum(packet)) {
		printf("%10s%25s\n", "", "Corruption in B_pkt.");
	} else if(TraceLevel >= traceOutputLevel && packet.acknum != A_pkt.seqnum){
		printf("%10s%25s\n", "", "Improper ACKnum.");
	} else if(TraceLevel >= traceOutputLevel && (strcmp(packet.payload, "ACK") != 0)) {
		printf("%10s%25s\n", "", "Not a ACK.");
	}
}

/*
 * A_timerinterrupt()  This routine will be called when A's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void A_timerinterrupt() {
	// Resend packet
	tolayer3(AEntity, A_pkt);
	startTimer(AEntity, timeout);

	if(TraceLevel >= traceOutputLevel) {
		printf("%10s%25s %d\n", "", "A TIMER", A_seqnum);
		printf("%10s%25s", "", "Resent A_pkt: ");
		printPkt(A_pkt);
	}


}  

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
	A_seqnum = 0;
	A_buffer_start = 0;
	A_buffer_end = 0;
}


/* 
 * Note that with simplex transfer from A-to-B, there is no routine  B_output() 
 */

/*
 * B_input(packet),where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the A-side (i.e., as a result 
 * of a tolayer3() being done by a A-side procedure) arrives at the B-side. 
 * packet is the (possibly corrupted) packet sent from the A-side.
 */
void B_input(struct pkt packet) {
	if(TraceLevel >= traceOutputLevel) {
		printf("%10s%25s", "", "A_pkt received: ");
		printPkt(packet);
	}
	if(packet.checksum != getChecksum(packet) || B_seqnum != packet.seqnum) {
		// If corruption or wrong seqnum, resend previous ACK
		tolayer3(BEntity, B_pkt);
		if(TraceLevel >= traceOutputLevel && packet.checksum != getChecksum(packet)) {
			printf("%10s%25s\n", "", "Corruption in A_pkt.");
		} else if (TraceLevel >= traceOutputLevel && B_seqnum != packet.seqnum) {
			printf("%10s%25s\n", "", "Improper Seqnum.");
		}
		if(TraceLevel >= traceOutputLevel) {
			printf("%10s%25s", "", "Previous B_ACK sent: ");
			printPkt(B_pkt);
		}
	} else {
		// Send mesg to Layer 5 and send back ACK
		clearMsg(B_message);
		strcpy(B_message.data, packet.payload);
		tolayer5(BEntity, B_message);

		clearPkt(B_pkt);
		strcpy(B_pkt.payload ,"ACK");
		B_pkt.seqnum = -1;
		B_pkt.acknum = B_seqnum;
		B_pkt.checksum = getChecksum(B_pkt);
		B_seqnum = 1 - B_seqnum;
		tolayer3(BEntity, B_pkt);

		if(TraceLevel >= traceOutputLevel) {
			printf("%10s%25s", "", "B_ACK sent: ");
			printPkt(B_pkt);
		}
	}
}

/*
 * B_timerinterrupt()  This routine will be called when B's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void  B_timerinterrupt() {
}

/* 
 * The following routine will be called once (only) before any other   
 * entity B routines are called. You can use it to do any initialization 
 */
void B_init() {
	B_seqnum = 0;
	clearPkt(B_pkt);
	strcpy(B_pkt.payload, "ACK");
	B_pkt.acknum = 1;
	B_pkt.checksum = getChecksum(B_pkt);
}
