/*
	Copyright © 2010, Shipping Soon. All Rights Reserved.
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#define DEBUG_MODE
#include <ws2tcpip.h>
#define VERSION "IRC Client (ver 0.1)"
#define ISSET(x, y) ((x & (y)) == (y))
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif
#define KBYTE 1024
#define FREE(p)		\
	free(p);		\
	(p) = NULL; 
#define NFREE(p, n) for (int i = 0; i < n; i++)	FREE(*(p+i))
#define pause printf("\nPress any key to exit\n"); getchar(); WSACleanup(); exit(1)
#define TEST 1
typedef unsigned char u8;
typedef unsigned int u32;

typedef struct {
	int sd;
	u8 mode;
	char channel[20];
}arguments;

void die(const char * s, int sock, struct addrinfo * res)
{
	perror(s);
	if (sock) 
			closesocket(sock);
	if (res) 
		freeaddrinfo(res);
	WSACleanup();
	pause;
}

void getlocaltime(char (*ptime))
{
	time_t rtime;
	time(&rtime);
	struct tm * loctime = localtime(&rtime);
	strcpy(ptime, asctime(loctime));
}


void * talk(void * emotion)
{
	
	char buffer[KBYTE/2] = {0};
	arguments * arg = (arguments *) emotion;

	while(true)
	{
		Sleep(1);
		
		if (ISSET(arg->mode, TEST)) {
			sprintf(buffer, "PRIVMSG #%s :\r\n", arg->channel);
		} 
		
		scanf("%s", buffer);
		if(!strcmp(buffer, "quit")) {
			die("Quit", arg->sd, NULL);
		}
		
		sprintf(buffer, "PRIVMSG #%s :%s\r\n", arg->channel, buffer);
		send(arg->sd, buffer, strlen(buffer), 0);
		memset(buffer, 0, sizeof(buffer));
	}
	return NULL;
}

char * ranalias(char * s, const char **p, u8 min, u8 max, bool cat)
{
	srand(time(NULL));
	int r = rand() % max + min;
	(cat) ? strcat(s, p[r]) : strcpy(s, p[r]);
	return s;
	
}

const char * ranalias(const char **p, u8 min, u8 max)
{
	srand(time(NULL));
	int r = rand() % max + min;
	return p[r];
}

char * alias(char * s, const char **p, u8 n, bool cat)
{
	(cat) ? strcat(s, p[n]) : strcpy(s, p[n]);
	return s;
}

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
		case EAI_NODATA:
			return "Valid name, but no data record of requested type.";
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
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
		#ifdef DEBUG_MODE
		fprintf(stderr, "WSAStartup() error\n");
		pause;
		#endif
	}
	
	const char * name[] = {"guest", "anonymous", "anon", "john"};
	const char * nick[] = {"54553",	"doe", "test", "dev"};
	const char * version[] = {"xchat 2.8.6 Ubuntu", "xchat 2.8.6-2 Windows Vista", "mIRC v7.17 Khaled Mardam-Bey"};
	const char * server[] = {"irc.rizon.net"};
	const char * port[] = {"6667", "6668"};
	const char * channel[] = {"donnotenter"};
	struct addrinfo hints, * res;
	pthread_t tid[2];
	char buffer[KBYTE/2] = {0}, msg[3][KBYTE/2] = {0}, loctime[30] = {0}, username[40] = {0}, nickname[40] = {0}, password[] = "changeme", * p = NULL;
	u8 	mode = TEST;
	int sd, r = 0;
	bool usePass = false;
	arguments arg;
	
	arg.sd = sd;
	arg.mode = mode;
	strcpy(arg.channel, channel[0]);
	ranalias(username, name, 1, 6, false);
	strcpy(nickname, username);
	ranalias(nickname, nick, 1, 6, true);
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	while ((r = getaddrinfo(server[0], port[0], &hints, &res)) != 0) {
		#ifdef DEBUG_MODE
		fprintf(stderr, "Getaddrinfo() error: %s\n", gai_strerror_w(r));
		#endif
		Sleep(3 * 60000);
		continue;
	}

	for (struct addrinfo * ai = res; ai != NULL; ai = ai->ai_next) {
		if ((sd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) == -1) 
			continue;
		break;
	}

	if((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
		#ifdef DEBUG_MODE
		die("Socket() error", 0, res);
		#endif
		exit(2);
	}
	
	if (connect(sd, res->ai_addr, res->ai_addrlen) == -1)  {
		#ifdef DEBUG_MODE
		die("Connect() error", sd, res);
		#endif
		exit(2);
	}
	
	#ifdef DEBUG_MODE
	struct sockaddr_in * tmp = (struct sockaddr_in *) res->ai_addr;
	printf("IRC Client has received connection from (%s) on port (%i)\n", (char *) inet_ntoa(tmp->sin_addr), htons(tmp->sin_port));
	#endif
	
	freeaddrinfo(res);
	
	sprintf(buffer, "NICK %s\r\n", nickname);
	send(sd, buffer, strlen(buffer), 0);
	sprintf(buffer, "USER %s 8 \"\" :%s\r\n", ranalias(nick, 0, 6), username);
	send(sd, buffer, strlen(buffer), 0);
	(usePass) ? sprintf(buffer, "JOIN #%s %s\r\n", channel[0], password) : sprintf(buffer, "JOIN #%s\r\n", channel[0]);
	send(sd, buffer, strlen(buffer), 0);
	memset(buffer, 0, sizeof(buffer));
	
	pthread_create(&tid[0], NULL, talk, (void *) &arg);
	
	while (true) {
		if (recv(sd, buffer, sizeof(buffer), 0) <= 0) {
			#ifdef DEBUG_MODE
			die("Recv() r", sd, NULL);
			#endif
		}

		if ((p = strstr(buffer, "Nickname is already in use.\r\n"))) {
			while (p != buffer) {
				if (p[0] == '\n') {
					ranalias(nickname, name, 0, 6, false);
					ranalias(nickname, nick, 0, 6, true);
					sprintf(buffer, "NICK %s\r\n", nickname);
					send(sd, buffer, strlen(buffer), 0);
					sprintf(buffer, "USER %s 8 \"\" :%s\r\n", ranalias(nick, 0, 6), username);
					send(sd, buffer, strlen(buffer), 0);
					(usePass) ? sprintf(buffer, "JOIN #%s %s\r\n", channel[0], password) : sprintf(buffer, "JOIN #%s\r\n", channel[0]);
					send(sd, buffer, strlen(buffer), 0);
					memset(buffer, 0, sizeof(buffer));
					continue;
				}
				--p;
			}
		}
		
		if (sscanf(buffer, "PING :%512s", msg[0]) > 0) {
			sprintf(buffer, "PONG :%512s\r\n", msg[0]);
			send(sd, buffer, strlen(buffer), 0);
			memset(buffer, 0, sizeof(buffer));
			continue;
		}
		
		if (sscanf(buffer, ":%512[^!]!%*s PRIVMSG %*s :\001TIM%512[E]\001", msg[0], msg[1]) > 1) {
			getlocaltime(loctime);
			sprintf(msg[1], "NOTICE %s :\001TIME %s\001\r\n", msg[0], loctime);
			send(sd, msg[1], strlen(msg[1]), 0);
			memset(buffer, 0, sizeof(buffer));
			continue;
		}
		
		if (sscanf(buffer, ":%512[^!]!%*s PRIVMSG %*s :\001VERSIO%512[N]\001", msg[0], msg[1]) > 1) {
			sprintf(msg[1], "NOTICE %s :\001VERSION %s\001\r\n", msg[0], version[0]);
			send(sd, msg[1], strlen(msg[1]), 0);
			memset(buffer, 0, sizeof(buffer));
			continue;
		} 
		
		printf("%s\n", buffer);
		memset(buffer, 0, sizeof(buffer));	
	}
	WSACleanup();
	closesocket(sd);
	return 0;
}
