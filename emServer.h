
#ifndef OS_EX5_EMSERVER_H
#define OS_EX5_EMSERVER_H

#include <iostream>
#include <stdio.h>
#include <string.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write#include<stdio.h>
#include <string.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <cstdlib>
#include <map>
#include <vector>
#include <pthread.h>
#include <algorithm>
#include <locale.h>
#include <sstream>
#include <netdb.h>
#include <fstream>
#include "Event.h"

using namespace std;

class emServer {
private:
    vector<pair<Event*, vector<string>*>*> _events; // map event ids to a list of client names
    pthread_mutex_t _eventsMut; // mutex for protecting events data
    vector<string> _clients;
    pthread_mutex_t _clientsMut; // mutex for protecting clients data
    int _eventCounter; // current max event id
public:
    emServer();
    ~emServer();
    int addEvent(string title, string date, string description);
    int removeEvent(int id);
    int addClient(string name);
    int removeClient(string name);
    int getEventCounter();
    int assignClientToEvent(int eventId, string clientName);
    int removeClientFromEvent(int eventId, string clientName);
};


#endif //OS_EX5_EMSERVER_H
