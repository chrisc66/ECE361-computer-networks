#include "client.h"



/* sending datagrams to the address localhost and to the port number you specified */
/* find the transmission rate in bits/s​ and propagation delay in microseconds​ for each of the simulators */

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len;
    struct sockaddr_storage their_addr;

    // time & numbits array
    int timeArray[SIZE];
    int numbytesArray[SIZE];
    for (int j = 0; j < SIZE; j ++){
        timeArray[j] = -1;
        numbytesArray[j] = -1;
    } 

    struct timeval * startTime = malloc(sizeof(struct timeval));
    struct timeval * endTime = malloc(sizeof(struct timeval));

    for (int i = 0; i < SIZE; i++){
        
        // get message from user input
        printf("\n");
        memset(&buffer, (unsigned long)'\0', BUFFER_SIZE);
        printf("%d: Enter text:", i);
        scanf("%s", buffer);

        // get address info
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;
        if ((rv = getaddrinfo(SERVER_IP, SERVERPORT, &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            return 1;
        }

        // loop through result and create socket
        for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
                perror("talker: socket");
                continue;
            }
            break;
        }
        assert(p != NULL);              // failed to create socket

        // record start time
        gettimeofday(startTime, NULL);
        
        // send message to server
        if ((numbytes = sendto(sockfd, buffer, strlen(buffer), 0, p->ai_addr, p->ai_addrlen)) == -1) {
            perror("talker: sendto");
            exit(1);
        }
        printf("talker: sent %d bytes to %s\n", numbytes, SERVER_IP);
        
        // receive ACK from server
        addr_len = sizeof their_addr;
        if ((numbytes = recvfrom(sockfd, buffer, BUFFER_SIZE-1 , 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        printf("listener: packet is %d bytes long\n", numbytes);
        buffer[numbytes] = '\0';
        printf("listener: packet contains \"%s\"\n", buffer);
        
        // record end time
        gettimeofday(endTime, NULL);

        // calculate time and bit rate
        int usecDiff = endTime->tv_usec - startTime->tv_usec;   // microsecond
        int secDiff = endTime->tv_sec - startTime->tv_sec;      // second
        int totalDiff = secDiff * 1000000 + usecDiff;           // microsecond
        timeArray[i] = totalDiff / 2;
        numbytesArray[i] = numbytes;
        printf("time: %d microseconds​\n", totalDiff);

        if(i == 1 && numbytes == numbytesArray[0]){
            i --;
            printf("Error: please enter text of different length\n");
        }

        printf("\n");
    }

    // Ttotal = Tframe + Tprop = Tframe + BitRate * numbytes
    double bitRate = (double)( numbytesArray[0] - numbytesArray[1] ) / (double)( timeArray[0] - timeArray[1] );
    double Tprop = (double)timeArray[0] - (double)numbytesArray[0] / bitRate; // Ttotal - Tframe
    printf("\n\n");
    printf("Propagation delay: %lf micro-seconds\n", Tprop);
    printf("Byte rate: %lf bytes per micro-second\n", bitRate);
    printf("\n\n");

    freeaddrinfo(servinfo);

    close(sockfd);

    return 0;
}
