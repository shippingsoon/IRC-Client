/*
	Copyright 2010, Shipping Soon. All Rights Reserved.
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <pthread.h>
#include "irc client.h"

//Get the local time. This will be used for CTCP time.
void getlocaltime(char (*ptime), int n)
{
	struct tm * loctime;
	time_t rtime;
	
	time(&rtime);
	loctime = localtime(&rtime);
	strncpy(ptime, asctime(loctime), n);
}

//Client input.
void *talk(void *emotion)
{
	char buffer[KBYTE] = {0};
	arguments *arg = (arguments *) emotion;

	while (true) {
		sleep(1);
		if (ISSET(arg->mode, TEST))
			snprintf(buffer, KBYTE, "PRIVMSG #%s :\r\n", arg->channel);
		//Retrieve user input.
		scanf("%s", buffer);
		//Quit the channel and exit the program.
		if(!strcmp(buffer, "quit")) {
			die("Quit", arg->sd, NULL);
		}
		//Send private messages.
		snprintf(buffer, KBYTE, "PRIVMSG #%s :%s\r\n", arg->channel, buffer);
		send(arg->sd, buffer, strlen(buffer), 0);
		memset(buffer, 0, sizeof(buffer));
	}
	pthread_exit(NULL);
}

//Set a random alias.
char *ranalias(char *s, const char **p, u8 min, u8 max, bool cat)
{
	srand(time(NULL));
	int r = rand() % max + min;
	(cat)
		? strcat(s, p[r])
		: strcpy(s, p[r]);
	return s;
}

//Return a pointer to a random alias.
const char *ranaliasp(const char **p, u8 min, u8 max)
{
	srand(time(NULL));
	int r = rand() % max + min;
	return p[r];
}

//Concat an alias.
char *alias(char *s, const char **p, u8 n, bool cat)
{
	(cat)
		? strcat(s, p[n])
		: strcpy(s, p[n]);
	return s;
}

//Window's implementation of POSIX sockets did not have the gai_strerror() function.
const char * gai_strerror_w (int ecode)
{
	switch (ecode) {
		case EAI_AGAIN:	
			return "Nonauthoritative host not found.";
   		case EAI_BADFLAGS: 
			return "Invalid argument.";
		case EAI_FAIL: 
			return "This is a nonrecoverable error.";
		case EAI_FAMILY:
			return "Address family not supported by protocol family.";
		case EAI_MEMORY:
			return "Insufficient memory available.";
		case EAI_NONAME:
			return "Host not found.";
		case EAI_SERVICE:
			return "Class type not found.";
		case EAI_SOCKTYPE:
			return "Socket type not supported.";
		default:
			return "Unknown error.";
   }
}

int main(void)
{
	struct sockaddr_in *tmp;
	struct addrinfo hints, *res, *ai;
	pthread_t tid[2];
	u8 	mode;
	int sd, r;
	bool usePass = false;
	arguments arg;
	const char *name[] = {"guest", "anonymous", "anon", "john"};
	const char *nick[] = {"54553",	"doe", "test", "dev"};
	const char *version[] = {"xchat 2.8.6 Ubuntu", "xchat 2.8.6-2 Windows Vista", "mIRC v7.17 Khaled Mardam-Bey"};
	const char *server[] = {"irc.rizon.net"};
	const char *port[] = {"6667", "6668"};
	const char *channel[] = {"donnotenter"};
	char buffer[KBYTE] = {0}, msg[3][KBYTE] = {0}, loctime[30] = {0}, username[40] = {0}, nickname[40] = {0}, password[] = "changeme", *p = NULL;

	mode = TEST;
	strncpy(arg.channel, channel[0], 20);
	ranalias(username, name, 1, 6, false);
	strncpy(nickname, username, 40);
	ranalias(nickname, nick, 1, 6, true);
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	//Use getaddrinfo() to retrieve a linked list of addrinfo structs.
	while ((r = getaddrinfo(server[0], port[0], &hints, &res)) != 0) {
		#ifdef DEBUG_MODE
		fprintf(stderr, "getaddrinfo() error: %s\n", gai_strerror(r));
		#endif
		continue;
	}
	//Retrieve a socket descriptor.
	if((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
		die("socket() error", 0, res);
		exit(2);
	}
	//Establish a blocking connection with the server.
	if (connect(sd, res->ai_addr, res->ai_addrlen) == -1)  {
		die("connect() error", sd, res);
		exit(2);
	}
	#ifdef DEBUG_MODE
	//tmp = (struct sockaddr_in *) res->ai_addr;
	//printf("client has received connection from %s:%i \n", inet_ntoa(tmp->sin_addr), htons(tmp->sin_port));
	#endif
	//Free the address info.
	freeaddrinfo(res);
	//Set the nickname.
	snprintf(buffer, KBYTE, "NICK %s\r\n", nickname);
	send(sd, buffer, strlen(buffer), 0);
	snprintf(buffer, KBYTE, "USER %s 8 \"\" :%s\r\n", ranaliasp(nick, 0, 6), username);
	send(sd, buffer, strlen(buffer), 0);
	//Join a channel.
	(usePass) 
		? snprintf(buffer, KBYTE, "JOIN #%s %s\r\n", channel[0], password) 
		: snprintf(buffer, KBYTE, "JOIN #%s\r\n", channel[0]);
	send(sd, buffer, strlen(buffer), 0);
	memset(buffer, 0, sizeof(buffer));
	arg.sd = sd;
	arg.mode = mode;
	pthread_create(&tid[0], NULL, talk, (void *) &arg);
	
	while (true) {
		if (recv(sd, buffer, sizeof(buffer), 0) == -1) {
			die("recv() error", sd, NULL);
		}
		//Change the nickname if we recieve a nickname is already in use message.
		if ((p = strstr(buffer, "Nickname is already in use.\r\n"))) {
			while (p != buffer) {
				if (p[0] == '\n') {
					ranalias(nickname, name, 0, 6, false);
					ranalias(nickname, nick, 0, 6, true);
					snprintf(buffer, KBYTE, "NICK %s\r\n", nickname);
					send(sd, buffer, strlen(buffer), 0);
					snprintf(buffer, KBYTE, "USER %s 8 \"\" :%s\r\n", ranaliasp(nick, 0, 6), username);
					send(sd, buffer, strlen(buffer), 0);
					(usePass) 
						? snprintf(buffer, KBYTE, "JOIN #%s %s\r\n", channel[0], password)
						: snprintf(buffer, KBYTE, "JOIN #%s\r\n", channel[0]);
					send(sd, buffer, strlen(buffer), 0);
					memset(buffer, 0, sizeof(buffer));
					continue;
				}
				--p;
			}
		}
		//Ping pong protocol. This lets the server know we are still connected.
		if (sscanf(buffer, "PING :%512s", msg[0]) > 0) {
			snprintf(buffer, KBYTE, "PONG :%512s\r\n", msg[0]);
			send(sd, buffer, strlen(buffer), 0);
			memset(buffer, 0, sizeof(buffer));
			continue;
		}
		//CTCP version.
		if (sscanf(buffer, ":%512[^!]!%*s PRIVMSG %*s :\001TIM%512[E]\001", msg[0], msg[1]) > 1) {
			getlocaltime(loctime, sizeof(loctime));
			snprintf(msg[1], KBYTE, "NOTICE %s :\001TIME %s\001\r\n", msg[0], loctime);
			send(sd, msg[1], strlen(msg[1]), 0);
			memset(buffer, 0, sizeof(buffer));
			continue;
		}
		//CTCP time
		if (sscanf(buffer, ":%512[^!]!%*s PRIVMSG %*s :\001VERSIO%512[N]\001", msg[0], msg[1]) > 1) {
			snprintf(msg[1], KBYTE, "NOTICE %s :\001VERSION %s\001\r\n", msg[0], version[0]);
			send(sd, msg[1], strlen(msg[1]), 0);
			memset(buffer, 0, sizeof(buffer));
			continue;
		} 
		
		printf("%s\n", buffer);
		memset(buffer, 0, sizeof(buffer));	
	}
	close(sd);
	pthread_exit(NULL);
}

//Used for debugging.
void die(const char *s, int sock, struct addrinfo *res)
{
	#ifdef DEBUG_MODE
	perror(s);
	#endif
	if (sock) 
			close(sock);
	if (res) 
		freeaddrinfo(res);
	pause;
}