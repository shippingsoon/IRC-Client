/*
	Copyright 2010, Shipping Soon. All Rights Reserved.
*/

#define DEBUG_MODE
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
#define pause printf("\nPress any key to exit\n"); getchar();  exit(1)
#define TEST 1

typedef unsigned char u8;
typedef unsigned int u32;

typedef struct {
	int sd;
	u8 mode;
	char channel[20];
}arguments;

//Used for debugging.
void die(const char *s, int sock, struct addrinfo *res);

//Get the local time. This will be used for CTCP time.
void getlocaltime(char (*ptime), int n);

//Client input.
void *talk(void *emotion);

//Set a random alias.
char *ranalias(char *s, const char **p, u8 min, u8 max, bool cat);

//Return a pointer to a random alias.
const char *ranaliasp(const char **p, u8 min, u8 max);

//Concat an alias.
char *alias(char *s, const char **p, u8 n, bool cat);



