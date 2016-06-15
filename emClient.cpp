//
// Created by root on 6/12/16.
//

#include "emClient.h"
#include <unistd.h>
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <cstdlib>
#include <iostream>
#include <cstring>

using namespace std;

int main(int argc , char *argv[]) {


    if(argc != 4) {
        cout << "Usage: emClient clientName serverAddress serverPort" << endl;
        exit(0);
    }

    string clientName = string(argv[1]);
    char* serverAddress = argv[2];
    int portNum = atoi(argv[3]); // set port number

    int sock;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr(serverAddress);
    server.sin_family = AF_INET;
    server.sin_port = htons( portNum );

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }

    puts("Connected\n");

    //keep communicating with server
    while(1)
    {
        char inputMessage[2000];
        string strMessage;
        getline(cin, strMessage);

        //strMessage = string(inputMessage);
        cout << "CIN: " << strMessage << endl;

        strMessage = clientName + " " + strMessage;

        const char* message;
        message = strMessage.c_str();
        //Send some data
        cout << "message " << message << endl;

        if( send(sock , message , strlen(message) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }

        //Receive a reply from the server
        if( recv(sock , server_reply , 2000 , 0) < 0)
        {
            puts("recv failed");
            break;
        }

        puts("Server reply :");
        puts(server_reply);

        memset(server_reply, 0, sizeof(server_reply));
    }

    close(sock);
    return 0;
}