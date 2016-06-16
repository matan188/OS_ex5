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
#include <algorithm>
#include <locale>
#include <fstream>
#include <time.h>

using namespace std;
std::ofstream logFile;
char logPath[99999];

string getTime(bool withSep) {
    time_t t;
    struct tm * timeinfo;
    char buffer[80];
    if((int) time(&t) < 0) {
        //sysError("time");
    }

    timeinfo = localtime(&t);

    if(withSep) {
        strftime(buffer, 80, "%H:%M:%S", timeinfo);
    } else {
        strftime(buffer, 80, "%H%M%S", timeinfo);
    }
    return string(buffer);
}

/**
 * Write commands to log.
 */
void writeToLog(string msg) {

    cout << "LOG\t" << msg << endl;

    logFile.open(logPath, std::ios_base::app);
    if(logFile.fail()) {
        //sysError("open");
    }

    logFile << getTime(true) << "\t" << msg;
    if(logFile.fail()) {
        //sysError("close");
    }
    logFile.close();
}

int main(int argc , char *argv[]) {

    if(argc != 4) {
        cout << "Usage: emClient clientName serverAddress serverPort" << endl;
        exit(0);
    }

    emClient emc;

    string clientName = string(argv[1]);
    char* serverAddress = argv[2];
    int portNum = atoi(argv[3]); // set port number

    string logFile = clientName + "_" + getTime(false);
    const char * cLogPath = logFile.c_str();
    strcpy(logPath, cLogPath);


    int sock;
    struct sockaddr_in server;
    char message[99999] , server_reply[99999];

    //keep communicating with server
    while(1)
    {
        bool clientError = false;
        char inputMessage[99999];
        string strOriginal, str;
        getline(cin, str);
        strOriginal = str;
        //strMessage = string(inputMessage);
        cout << "CIN: " << str << endl;

        /** parse msg **/

        //remove command from strOriginal
        int pos = strOriginal.find(" ");
        strOriginal = strOriginal.substr(pos + 1);

        pos = str.find(" ");

        string command = str.substr(0, pos);
        str = str.substr(pos + 1);

        locale loc1, loc2;
        for(string::size_type i = 0; i < command.length(); ++i) {
            command[i] = toupper(command[i], loc1);
        }
        for(string::size_type i = 0; i < clientName.length(); ++i) {
            clientName[i] = toupper(clientName[i], loc2);
        }

        cout<<"command: "<<command<<endl;

        string eventId;

        if(!command.compare("REGISTER")) {
            if(emc.getIsRegistered()) {
                writeToLog("Error: the client " + clientName
                           + " was already registered.\n");
                clientError = true;
            }
        } else if(!command.compare("CREATE")) {
            size_t n = count(str.begin(), str.end(), ' ');
            if(n < 3) {
                writeToLog("Error:\t" + command + "\tinvalid arguments.\n");
                clientError = true;
            }

            pos = str.find(" ");
            string eventTitle = str.substr(0, pos);
            str = str.substr(pos + 1);

            pos = str.find(" ");
            string eventDate= str.substr(0, pos);
            str = str.substr(pos + 1);

            string eventDescription = string(str);

            if(eventTitle.size() > 30 || eventDate.size() > 30 ||
                    eventDescription.size() > 256) {
                writeToLog("Error:\t" + command + "\tinvalid argument.\n");
                clientError = true;
            }
        } else if(!command.compare("SEND_RSVP")) {

            size_t n = count(str.begin(), str.end(), ' ');
            if(n < 1) {
                writeToLog("Error:\t" + command + "\tinvalid arguments.\n");
                clientError = true;
            }

            pos = str.find(" ");
            eventId= str.substr(0, pos);
            str = str.substr(pos + 1);

        } else if(!command.compare("GET_RSVPS_LIST")) {
            size_t n = count(str.begin(), str.end(), ' ');
            if(n < 1) {
                writeToLog("Error:\t" + command + "\tinvalid arguments.\n");
                clientError = true;
            }
        } else if(!command.compare("UNREGISTER")) {
            if(!emc.getIsRegistered()) {
                writeToLog("ERROR:\t" + command
                           + "\tclient is not registered.\n");
                clientError = true;
            }
        }

        /** end of parsing **/

        if(clientError) {
            continue;
        }

        str = clientName + " " + command + " " + strOriginal;

        const char* message;
        message = str.c_str();
        //Send some data
        cout << "message " << message << endl;

        /** Connect to server **/
        // Create socket
        sock = socket(AF_INET , SOCK_STREAM , 0);
        if (sock == -1)
        {
            printf("Could not create socket");
        }
        puts("Socket created");

        server.sin_addr.s_addr = inet_addr(serverAddress);
        server.sin_family = AF_INET;
        server.sin_port = htons( portNum );

        // Connect to remote server
        if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
        {
            perror("connect failed. Error");
            return 1;
        }
        /** connected **/


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

        if(!command.compare("REGISTER")) {
            if(atoi(server_reply) == 1) {
                writeToLog("ERROR: the client " + clientName + " was already registered.\n");
            } else {
                writeToLog("Client " + clientName + " was registered successfully.\n");
                emc.setRegister(true);
            }
        } else if(!command.compare("CREATE")) {
            int eventId = atoi(server_reply);
            if(eventId == 0) {
                writeToLog("ERROR: failed to create the event: unknown failure.\n");
            } else {
                writeToLog("Event id " + to_string(eventId) + " was created successfully.\n");
            }
        } else if(!command.compare("GET_TOP_5")) {
            writeToLog("Top 5 newest events are:\n" + string(server_reply));
        } else if(!command.compare("SEND_RSVP")) {
            int res = atoi(server_reply);
            if(res == 0) {
                writeToLog("RSVP to event id " + eventId + " was received successfully.\n");
            } else if(res == 1) {
                writeToLog("ERROR: failed to send RSVP to eventId " + eventId + ": event not exists.\n");
            }

        } else if(!command.compare("GET_RSVPS_LIST")) {
        } else if(!command.compare("UNREGISTER")) {
        }

        puts("Server reply :");
        puts(server_reply);

        memset(server_reply, 0, sizeof(server_reply));
    }

    close(sock);
    return 0;
}

emClient::emClient(): _isRegistered(false) {}