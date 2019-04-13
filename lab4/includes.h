#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

// Configuable constants
#define WINDOW_SIZE 32          // listening queue size
#define PORT "5000"             // port number
#define SERVER_IP "0.0.0.0"     // server IP address
#define MAXDATASIZE 100         // max number of bytes we can get at once 

typedef enum{
    NEW = 1,
    LIST = 2,
    OTHER_CLIENT = 3,
    BOARDCAST = 4,
    EXIT = 5
} state;