Jonathan Chang
CS 3516 A18 A01

To compile the project use the following two commands:
	make clean
	make all
	
To run the project use the following command:
	./p2
	
	For the arguments of the project, any number between 0.0 and 1.0 will work
	for the loss, corruption, and out-of-order values. For the time between messages,
	use 1000 to ensure that messages do not come too fast; the buffer is able to 
	handle 100 msgs but may overflow if this input is too small. For testing, use TraceLevel
	1 but to display the trace output of student2.c use TraceLevel 3. Randomization
	can be inputed as 1 but bi-directional must be 0.
	
	Note that there is a rare case with out-of-order in which that the previous 2 
	A_packets will be sent to BEntity, caused by a combination of lost, out-of-order,
	and resent packets; this will cause student2.c to be shifted two packets out of order
	and cause massive amounts of incorrectly received packets. According to the TA and professor, this 
	problem does not need to be handled.
	
	Note that these is also a case where a previous seqnum 0 or 1 packet will be received
	by BEntity, caused by a combination of lost, out-of-order, and resent packets; this
	will cause a single error for that particular packet since the correct packet is rejected but
	the next is accepted. According to the TA and professor, this problem does not need to be handled.
	
	Sample Input:
	Simulator Analysis:
		-----  Network Simulator Version    2.3 -------- 
		
		Enter the number of messages to simulate: 100
		Packet loss probability [enter number between 0.0 and 1.0]: 0.5
		Packet corruption probability [0.0 for no corruption]: 0.5
		Packet out-of-order probability [0.0 for no out-of-order]: 0.5
		Average time between messages from sender's layer5 [ > 0.0]: 1000
		Enter Level of tracing desired: 3
		Do you want actions randomized: (1 = yes, 0 = no)? 1
		Do you want Bidirectional: (1 = yes, 0 = no)? 0
		Input parameters:
		Number of Messages = 100  Lost Packet Prob. =   0.500
		Corrupt Packet Prob. =   0.500  Out Of Order Prob. =   0.500
		Ave. time between messages =  1000.00  Trace level = 1
		Randomize = 1  Bi-directional = 0

	The sample trace output is in the output.pdf file.
	
	Sample Output:
	Simulator Analysis:
		Number of messages sent from 5 to 4: 100
		Number of messages received at Layer 5, side A: 0
		Number of messages received at Layer 5, side B: 100
		Number of messages incorrectly received at layer 5: 0
		Number of packets entering the network: 1665
		Average number of packets already in network:     0.127
		Number of packets that the network lost: 812
		Number of packets that the network corrupted: 447
		Number of packets that the network put out of order: 439
