#include "socketincludes.h"
#include "config.h"
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <sys/time.h>
#include <stdio.h>

// Convert a number to char
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

// Append single char to char array
//https://stackoverflow.com/questions/12939370/c-appending-char-to-char
int append(char*s, char c)
{
    int len = strlen(s);
    s[len] = c;
    s[len+1] = '\0';
    return 0;
}

// NOT CALLSAFE: the passed strings have to be freed manually. ofc.
void splitString(char* first, char* second, char* inputBufToSplit, char delimiter)
{
    int delimiterCount = 0;
    char ic;
    int i = 0;

    for(i = 0; i < strlen(inputBufToSplit); i++)
    {
        ic = inputBufToSplit[i];
        printf("%c", ic);
        if(ic == delimiter) // If read delim, increase
        {
            delimiterCount ++;
            continue;
        }

        switch(delimiterCount)
        {
            case 0: // Append to timestamp
                append(first, ic);
            break;

            case 1:
                append(second, ic);
            break;

        }
    }
}


char* convertLongLongToCharPtr(long long num)
{
    int arrlen = (int)((ceil(log10(num))+1)*sizeof(char));
    printf("Requires %d slots for number %lld\n", arrlen, num);

    // Allocate mem to
    char* arrbuf = NULL;
    arrbuf = malloc(arrlen * sizeof(char));
    // Write to buffer
    sprintf(arrbuf, "%lld", num);

    return arrbuf;

}

long long convertCharPtrToLongLong(char* str)
{
    char* e;
    errno = 0;
    long long int n = strtoll(str, &e, 10); // base 10
    if(*e != 0 || errno != 0)
    {
        printf("[!] Error: in function convertCharPtrToLongLong(...) \n");
    }
    return n;
}

//https://stackoverflow.com/questions/3756323/how-to-get-the-current-time-in-milliseconds-from-c-in-linux
long long getTimestampMillis()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long long time_in_mill = (tv.tv_sec) * 1000LL + (tv.tv_usec) / 1000;
    //printf("getTimestampMillis: %lld", time_in_mill);
    return time_in_mill;
}

long long getTimestampUSEC()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return  1000000 * tv.tv_sec + tv.tv_usec;
}


/**** WEBSERVER ****/
// create a webserver
void webserver_create(int* webserversocket)
{
    // SOCK_STREAM use TCP for websocket
    if((*webserversocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Webserver socket creation failed.\n");
        exit(11);
    }
}
// set connection options
void webserver_setoptions(struct sockaddr_in* opts)
{
    memset(opts, 0, sizeof(*opts));
    opts->sin_family = AF_INET;
    opts->sin_port = htons(WEBSERVERPORT);
    opts->sin_addr.s_addr = inet_addr(WEBSERVER_IPADDR);
}
// Connect with the options
void webserver_connect(int* websockfd, struct sockaddr_in* webservAddr)
{
    if(connect(*websockfd, (struct sockaddr*)webservAddr, sizeof(*webservAddr)) < 0)
    {
        perror("Error connection websocket");
        //exit(12);
    }
}
// Set the message buffer
void webserver_setmsgbuffer(char* msgbuf, char* content)
{
    int __len = strlen(content);
    sprintf(msgbuf, "POST /path HTTP/1.0\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s", __len, content );
}

void webserver_sendmessage(int* sock, char* msgbuf)
{
    int webtotal, websent, webbytes;

    webtotal = strlen(msgbuf); // msg len total
    websent = 0;                         // sent of total
    do
    {
        webbytes = write(*sock, msgbuf+websent, webtotal-websent);
        if(webbytes < 0)
        {
            // couldnt write
            perror("Webserver err writing message.\n");
            exit(13);
        }
        if(webbytes == 0){
            break;
        }
        websent += webbytes;
    }
    while(websent < webtotal);
}

// NOT CALLSAFE: ret str has to be freed
long long webserver_recv_response(int* sock)
{
    int total; // recv bytes total
    int received; // total recvd
    int bytes; // buffer
    char respBuf[WEBSERVER_RESPONSE_LEN];

    memset(respBuf, 0, sizeof(respBuf));
    total = sizeof(respBuf)-1;
    received = 0;

    do
    {
        bytes = read(*sock, respBuf + received, total-received);
        if(bytes < 0)
        {
            perror("[!] Could not recv webserver response.");
            exit(1);
        }
        if(bytes == 0)
        {
            break;
        }
        received += bytes;

    }
    while(received < total);

    // Check if enough space available
    if(received == total)
    {
        perror("[!] Ran out of buffer for the webserver response. Message is incomplete.\n");
        //exit(1);
    }

    printf("[DBUG] Response: %s\n", respBuf);
    
    return convertCharPtrToLongLong(&respBuf[0]);
}

void webserver_close(int* sock)
{
    close(*sock);
}

// Includes webserver_setmsgbuffer, sendmessage.
void webserver_sendTimestampValueMsg(int* sock, struct sockaddr_in* addr, char* msgbuf, char* sensorValue)
{
    char* delimiterStr = ";";
    char* timestampstr = convertLongLongToCharPtr(getTimestampUSEC());
    char* timestampWithDelim = mergeString(timestampstr, delimiterStr);
    char* transmitdata = mergeString(timestampWithDelim, sensorValue);

    webserver_setmsgbuffer(msgbuf, transmitdata);

    webserver_sendmessage(sock, msgbuf);

    free(transmitdata);
    free(timestampWithDelim);
    free(timestampstr);

}

// File printing: Save array to file
void save_file(char* filename, int array[], int arraylength, char* title)
{
    // open file
    FILE* fp = fopen(filename, "w");
    if(fp == NULL)
    {
        printf("Error: Could not open file");
        return;
    }
    fflush(stdin);
    fflush(stdout);
    // Write json beginning.
    fprintf(fp, "{\"requestCooldown\":%d,\"name\":\"%s\",\"sensornum\":%d, \"savedElements\":%d ,\"values\": [",REQUEST_COOLDOWN_USEC ,title, SENSORNUM, arraylength);
    int i = 0;
    for(i = 0; i < arraylength; i++)
    {
        if(i != arraylength-1)
        {
            fprintf(fp, "%d,", array[i]);
        }
        else
        {
            fprintf(fp, "%d", array[i]);
        }
    }
    fprintf(fp, "]}");
    fclose(fp);
}


/***** MAIN ****/
int main()
{
    printf("This is main control unit!\n");

    int sockfd;
    char buffer[MAXBUFFER];
    char* message = "placeholder"; // message is set before send
    char input = 'o';
    int exiting = 0; // 0 = keep going, 1 = end while
    // Reading from the socket
    int i; // For for loop
    char ic; // for for loop
    // splitting the actual received data
    char delimiter = ';'; // Delimiter in received data string to split timestamp and data
    char* delimiterStr = ";";
    int delimiterCount = 0; // For disassembling the data string
    // Round trip time measuring
    long long int tempRTT = 0; // temp val for each packet RTT, scratch
    long long int highestRTT = 0;
    int recvdPackets = 0;

    int rttSaveArray[SAVEARRSIZE]; // Round trip times are stored here
    int rttSaveArrayIndex = 0;     // Current writing position
    // the same for webserver rtt...
    int wsrttSaveArray[SAVEARRSIZE];
    int wsrttSaveArrayIndex = 0;
    
    // Parameters
    char* splitSensorValue = NULL;
    char* splitSensorTimestamp = NULL;
    // Networking
    printf("AMOGUShello\n");
    char* sensorAddresses[SENSORNUM] = {
        "172.20.128.3",
        "172.20.5.1",
        "172.20.5.2",
        "172.20.5.3",
        "172.20.5.4",
        "172.20.5.5",
        "172.20.5.6",
        "172.20.5.7",
        "172.20.5.8",
        "172.20.5.9",
        "172.20.5.10",
        "172.20.5.11"
        };
    struct sockaddr_in sensorConnections[SENSORNUM];
    struct sockaddr_in serverAddr;
    // Error Buffer
    errno = 0;
    // webserver stuff
    // https://stackoverflow.com/questions/22077802/simple-c-example-of-doing-an-http-post-and-consuming-the-response
    char* webserverHost = WEBSERVER_IPADDR;
    char webserverMessage[WEBSERVER_MESSAGEBUF_LEN]; // Buffer for the message to send
    struct hostent *webserver;
    struct sockaddr_in webservAddr;
    int websockfd; // the socket for webserver comm.
    char* webserverResponse; // buffer for recv resp.

/*
    // Create socket for webserver communication
    webserver_create(&websockfd);
    // set connection opts
    webserver_setoptions(&webservAddr);
    // Connect the sock
    webserver_connect(&websockfd, &webservAddr);
    // prepare a message
    webserver_setmsgbuffer(&webserverMessage[0], "Webserver Connection test");
    // Send a req.
    webserver_sendmessage(&websockfd ,&webserverMessage[0]);
    
    webserver_recv_response(&websockfd);
    // Close webserver
    webserver_close(&websockfd);
*/
    // Create socket for Sensor communication

    if(0 > (sockfd = socket(AF_INET, SOCK_DGRAM, 0)))
    {
        perror("Socket creation failed.\n");
        exit(1);
    }
    else
    {
        printf("Socket created!\n");
    }

    // prepare information for server  
    for ( i = 0; i < SENSORNUM; i++)
    {   

        memset(&serverAddr, 0, sizeof(serverAddr));

        serverAddr.sin_family = AF_INET; // ip4
        serverAddr.sin_port = htons(PORT); // set port
         // Depend on Build target configuration for IP ADDr
        serverAddr.sin_addr.s_addr = inet_addr(sensorAddresses[i]); 
        sensorConnections[i] = serverAddr;
    }


    // Send / recv vars
    int recvLen, len;
    len = sizeof(struct sockaddr_in);
    // To process the sent value
    int sensorValue = 0;
    

    do {            
            // Loop over all connected sensors.
            for(i= 0; i < SENSORNUM; i++)
            {                
                usleep(REQUEST_COOLDOWN_USEC); // defined in config.h
                message = convertLongLongToCharPtr(getTimestampUSEC());
                printf("[OK] The message to be sent is: %s\n", message);
                
                // SEND.
                sendto(sockfd, (const char*) message, strlen(message), 0, (const struct sockaddr* )&sensorConnections[i], len);
                printf("[OK] Data sent to %d!\n", sensorConnections[i].sin_addr.s_addr);
                free(message); // Destroy string
                // RECEIVE.
                recvLen = recvfrom(sockfd, (char*) buffer, MAXBUFFER, 0 , (struct sockaddr* )&sensorConnections[i],&len);
                buffer[recvLen] = '\0';
                printf("[OK] Sensor sent string: %s (%d bytes)\n", buffer, recvLen);
                recvdPackets += 1;

                if(recvLen == -1) // err check
                {
                    printf("[!] ERROR! recv(..) FAILED!\n");
                    return -99;
                }

                splitSensorTimestamp = malloc(sizeof(char) * SPLITDATASIZE); // Allocate 200 char long buffer (set in config.h)
                splitSensorValue = malloc(sizeof(char) * SPLITDATASIZE);      // same for this one

                splitSensorTimestamp[0] = '\0';
                splitSensorValue[0] = '\0';

                splitString(splitSensorTimestamp, splitSensorValue, &buffer[0], delimiter);

                printf("[DBUG] Split Data: %s , %s\n", splitSensorTimestamp, splitSensorValue);

                // Convert to int
                sensorValue = atoi(splitSensorValue);

                // Calc RTT in Milliseconds!
                //tempRTT = (int)time(NULL) - atoi(splitSensorTimestamp); Old for seconds
                tempRTT = getTimestampUSEC() - convertCharPtrToLongLong(splitSensorTimestamp);
                // Assign highest RTT
                highestRTT = tempRTT > highestRTT ? tempRTT : highestRTT;

                printf("[OK] RTT: %lld us | Highest: %lld us\n", tempRTT, highestRTT);
                //printf("Converted val to int: %d\n", sensorValue);

                // Save RTT to array
                if(rttSaveArrayIndex < SAVEARRSIZE) // Check if there is still space
                {
                    rttSaveArray[rttSaveArrayIndex] = tempRTT; // save to array list
                    rttSaveArrayIndex += 1;                    // increase Index
                }
                else
                {
                    printf("[WARN] RTT Save Array is full.\n");
                    //break;
                }
                /** Bevor der SensorValue als string zerstört wird, senden wir diesen noch schnell an den HTTP server **/
                
                // Das hier muss gefree'd werden, da es überschrieben wird.
                free(splitSensorTimestamp);
                splitSensorTimestamp = NULL;

                // Create socket for webserver communication
                
                    webserver_create(&websockfd);
                    // set connection opts
                    webserver_setoptions(&webservAddr);
                    // Connect the sock
                    webserver_connect(&websockfd, &webservAddr);
                    // prepare a message
                    webserver_sendTimestampValueMsg(&websockfd, &webservAddr, &webserverMessage[0], splitSensorValue);
                    // HTTP  webserver RTT
                    tempRTT = getTimestampUSEC() - webserver_recv_response(&websockfd); // Returned die response als long long int
                    printf("[OK] HTTP Server RTT is: %lld\n", tempRTT);
                    // Auch wieder halt abspeichern, dieses mal halt für WEBSERVER RTT
                    if(wsrttSaveArrayIndex < SAVEARRSIZE) // Check if there is still space
                    {
                        wsrttSaveArray[wsrttSaveArrayIndex] = tempRTT; // save to array list
                        wsrttSaveArrayIndex += 1;                    // increase Index
                    }
                    else
                    {
                        printf("[WARN] WEBSERVER RTT Save Array is full\n");
                        //break;
                    }
                    
                    // Close webserver
                    webserver_close(&websockfd);
                
                /** JETZT: Wird der String gefreed **/

                // free what you mallocd
                free(splitSensorTimestamp);
                splitSensorTimestamp = NULL;

                free(splitSensorValue);
                splitSensorValue = NULL;

                printf("AMO AMO : %d, %d\n", rttSaveArrayIndex, wsrttSaveArrayIndex);
                if(rttSaveArrayIndex == SAVEARRSIZE && SAVEARRSIZE == wsrttSaveArrayIndex)
                {
                    exiting = 1;
                }
            }
            if(exiting == 1)
            {
                break;
            }
        
    printf("------------------------------------------\n"); // End of section
    } while(exiting == 0);
    
    fflush(stdout);
    // Calculate average rtt for udp
    long long int totalRTT = 0;
    for(i = 0; i < SAVEARRSIZE; i++)
    {
        totalRTT += rttSaveArray[i]; // add onto array
    }
    
    printf("============================================\n");
    printf("[UDP-RESULT] The average RTT is: %d us / (%lld us total)\n[RESULT] Saved %d entries\n[RESULT] Ping Cooldown %d\n[RESULT] Packets received: %d)\n", totalRTT/(rttSaveArrayIndex), totalRTT, rttSaveArrayIndex, REQUEST_COOLDOWN_USEC,recvdPackets);
    
    // Calculate avg rtt for HTTP
    totalRTT = 0;
    for(i = 0; i < SAVEARRSIZE; i++)
    {
        totalRTT += wsrttSaveArray[i]; // add onto array
    }

    printf("============================================\n");
    printf("[TCP-RESULT] The average HTTP RTT is: %u us / (%lld us total)\n[RESULT] Saved %d entries\n)", totalRTT/(wsrttSaveArrayIndex), totalRTT, wsrttSaveArrayIndex);



    // Eventually close the socket
    close(sockfd);
    printf("Socket closed!\n");

    // Write to file    
    printf("Printing to file...\n");
    save_file("sensortimes.json",&rttSaveArray[0], SAVEARRSIZE, "Sensor RTTs");
    save_file("httptimes.json",&wsrttSaveArray[0], SAVEARRSIZE, "HTTP RTTs");
    printf("Finished fileprint!\n");
    fflush(stdout);
    
    while(1);

    return 0;
}
