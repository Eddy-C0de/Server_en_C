////srv_v2.c////

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <syslog.h>

// int8 = unsigned char 
// int16 = unsigned short int
// int 32 = 

int main(int argc, char *argv[]) {
	unsigned char ret; // int8
	unsigned char buf[128];
	unsigned char *host, *port_conv;
	unsigned short int port; // int16
	unsigned int s; // int32

	if (argc != 2)
		fprintf(stderr, "Utilisation : %s <port> [host]\n", argv[0]);
	else {
		host = (unsigned char*) "127.0.0.1"; 
		port = (unsigned char *) argv[1];	
		port_conv = (unsigned short int) atoi((char *) port); 
		}
}
