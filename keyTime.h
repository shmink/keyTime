/*
 * keyTime.h
 *
 * Copyright (c) 2002-2009 Volkswagen Group Electronic Research
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Volkswagen nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * Alternatively, provided that this notice is retained in full, this
 * software may be distributed under the terms of the GNU General
 * Public License ("GPL") version 2, in which case the provisions of the
 * GPL apply INSTEAD OF those given above.
 *
 * The provided data structures and external interfaces from this code
 * are not restricted to be used by modules with a GPL compatible license.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 */

/*
 * struct canInfoStruct stores all the data you would need in for use in the program.
 * This was set up so that passing data to a function when using threads would work.
 *
 * sendIDinStruct - Stores the CAN ID you want to send to. In the form of a string as that's how parseFrame accepts them.
 * receiveIDinStruct - Stores the CAN ID you want to receive from.
 * sock - Stores the socket you created at an earlier time to then be able to send and receive data from the CAN bus network.
 * beginTime - Stores the time when passed to the timer function.
 * endTime - Stores the time when passed to the timer function.
 * cdata - Stores the values of each byte from a CAN message received from the ECU.
 * loop - Stores the amount of messages that you want to receive from the ECU. An optional counter.
 */
struct canInfoStruct {
		char *sendIDinStruct;
		int receiveIDinStruct;
		int sock;
		long *beginTime;
		long *endTime;
		int cdata[8];
		int loop;
	};

struct canInfoStruct myStruct;

/*
 * timer function is responsible for getting a time values
 * @param timer_val - take a long variable and sets the time equal to it
 */
void timer(long *timer_val);

/*
 * createSocket function creates a socket on the CAN bus network
 * so we're able to send and receive messages from it.
 * @param interface - A string value naming the interface where you want to create the socket.
 */
int createSocket(char *interface);

/*
 *	receiveMsg function is made to receive a message from the CAN network
 *	@param ptr - a void pointer used to accept structs of data
 */
void *receiveMsg(void *ptr);

/*
 *	sendMsg function is made to send a message to the CAN network
 *	@param ptr - a void pointer used to accept structs of data
 */
void *sendMsg(void *ptr);
