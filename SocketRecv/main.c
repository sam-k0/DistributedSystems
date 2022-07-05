#include "socketincludes.h"
#include "config.h"
// Other includes
#include "simdata.h"
#include <math.h>
#include <time.h>
// Configure build target
#define DOCKER


// Convert a number to char *
char* convIntToCharPtr(int num)
{
    // Get req array len
    int arrlen = (int)((ceil(log10(num))+1)*sizeof(char));

    // Allocate mem and write into buffer to be returned
    char* arrbuf = NULL;
    arrbuf = malloc(arrlen * sizeof(char));
    sprintf(arrbuf, "%d", num);
    return arrbuf;
}

// Merge two chararrays into one bigger one
//https://stackoverflow.com/questions/28087519/how-to-copy-from-character-array-to-character-pointer
char* mergeString(char* one, char* two)
{
    // Only combine if length of both char arrays is okay and successfully allocated
    char* merged = NULL;
    if((merged = malloc(strlen(one) + strlen(two)+1)) != NULL)
    {
        // Nullterminate string to recognize its end later on and write both strings into allocated buffer
        merged[0] = '\0';
        strcat(merged, one);
        strcat(merged, two);
    }
    else
    {
        printf("Error combining strings");
    }
    return merged;
}


// Main function to start the process of sending sensor data to control unit over udp
int main()
{
    printf("This is sensor!\n");
    srandom((unsigned int)11010); // Initialize Random Generator

    // Variable defs
    struct sockaddr_in serverAddr, clientAddr;
    int sockfd, len, recvLen;
    char buffer[RECEIVEBUFSIZE];
    char* message = NULL; // This will hold the assembled string
    char* sensorValueStr = NULL; // The value of sensor as string
    char* delimiter = ";";

    // Create UDP socket file descriptor - AF_INET(Adress family IPv4) over SOCK_DGRAM(UDP)
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("! Couldnt create socket\n");
        exit(1);
    }

    // Allocate mem for structs
    memset(&serverAddr, 0, sizeof(serverAddr));
    memset(&clientAddr, 0, sizeof(clientAddr));

    // Get the struct ready (server)
    serverAddr.sin_family = AF_INET; // IP4, INET6 is ip6
    serverAddr.sin_addr.s_addr = INADDR_ANY; // any ipv4 is fine inet_addr(CONTROLUNIT_IPADDR); // 172.17.0.3
    serverAddr.sin_port = htons(PORT); // set port to 8080

    // Bind serverAddr to the port and throw error if unsuccesfull
    if(0 > bind(sockfd, (const struct sockaddr*)&serverAddr, sizeof(serverAddr)))
    {
        perror("Bind failed.");
        exit(1);
    }

    // set length of client address
    len = sizeof(clientAddr);


    // now sockets and vars are set up - start running sensor in a loop to endlessly build and send data
    while(1)
    {
        printf("=====================================\n");
        printf("SENSOR - Waiting for new data request\n");

        // Receive data-bytes through UDP Socket from SocketSender / IoT-Gateway (does not continue until recieved)
        recvLen = recvfrom(sockfd, (char* )buffer, RECEIVEBUFSIZE, 0, (struct sockaddr*)&clientAddr, &len);
        buffer[recvLen] = '\0'; // Nullterminate the message for C
        printf("RECIEVE - Server sent message: %s with len %d\n", buffer, recvLen);


        // Sending a response
        // Assemble data resp string
        sensorValueStr = convIntToCharPtr(getRandomData());
        message = mergeString(&buffer[0], delimiter);
        message = mergeString(message, sensorValueStr);
        sendto(sockfd, (const char*) message, strlen(message), 0, (const struct sockaddr*)&clientAddr, len);
        printf("SENT - Random Num with strlen %d: %s\n", strlen((const char*)sensorValueStr), sensorValueStr);

        // IMPORTANT: free all sent data for next iteration
        free(sensorValueStr);
        free(message);
    }
    // Shutdown
    printf("Closing socket...\n");
    close(sockfd);
    printf("Closed socket!\n");
    return 0;
}
