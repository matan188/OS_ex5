

#include "emServer.h"

using namespace std;

void writeToLog(string msg) {
    cout << "LOG\t" << msg << endl;
}

int main(int argc, char * argv[]) {

    if(argc != 2) {
        cout << "Usage: emServer portNum" << endl;
        exit(0);
    }

    emServer * ems = new emServer();
    cout << ems->addEvent("testEvent", "25/06/16", "description text") << endl;
    cout << ems->removeEvent(1) << endl;
    cout << ems->addClient("x") << endl;
    cout << ems->removeClient("x") << endl;
    cout << ems->addClient("x") << endl;

    int portNum = atoi(argv[1]); // set port number
    int socket_desc , client_sock , c , read_size;
    struct sockaddr_in server , client;
    char client_message[2000];

    // Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        cout << "Could not create socket" << endl;
    }
    cout << "Socket created" << endl;

    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(portNum);

    // Bind
    if( bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        // Print the error message
        cerr << "bind failed. Error" << endl;
        return 1;
    }
    cout << "bind done" << endl;

    // Listen
    listen(socket_desc , 3);

    // Accept incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    // Accept connection from an incoming client
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
    puts("Connection accepted");

    // Receive a message from client
    while( (read_size = recv(client_sock , client_message , 2000 , 0)) > 0 )
    {
        // Send the message back to client
        write(client_sock , client_message , strlen(client_message));
    }

    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }

    return 0;
}

emServer::emServer() {
    _eventsMut = PTHREAD_MUTEX_INITIALIZER;

}

emServer::~emServer() {
    pthread_mutex_destroy(&_eventsMut);
    for(auto it : _events) {
        delete it->first;
        delete it->second;
        delete it;
    }
}

/**
 * creates new event and adds it to the first empty cell in _events vector.
 * if there is no empty cell, the new event is added at the end.
 */
int emServer::addEvent(string title, string date, string description) {
    // find empty cell and add event
    vector<string> * clients = new vector<string>;
    Event * event = new Event(title, date, description);
    auto p = new pair<Event*, vector<string>*>(event, clients);
    int i = 0;
    int newEventId = -1;
    pthread_mutex_lock(&_eventsMut);
    for(auto it : _events) {
        if(it == nullptr) {
            _events.at(i) = p;
            newEventId = i;
            break;
        }
        ++i;
    }
    // if there is no empty cell, push new event in vector's end
    if(newEventId == -1) {
        // no empty cell. use pushback
        newEventId = (int) _events.size();
        _events.push_back(p);
    }
    pthread_mutex_unlock(&_eventsMut);

    return newEventId;
}

/**
 * remove event from _events
 */
int emServer::removeEvent(int id) {
    int ret = -1;
    pthread_mutex_lock(&_eventsMut);
    if(id < _events.size() && id >= 0) {
        _events.at(id) = nullptr;
        ret = 0;
    }
    pthread_mutex_unlock(&_eventsMut);
    return ret;
}

/**
 * Add client (only name string actually) to clients list, if a client with
 * the same name is not exists yet
 */
int emServer::addClient(string name) {
    int ret = 0;
    pthread_mutex_lock(&_clientsMut);
    for(auto otherName : _clients) {
        if(name.compare(otherName) == 0) {
            ret = -1;
            break;
        }
    }
    if(ret == -1) {
        // error - client name already exists
        writeToLog("ERROR: the client " + name + " was already registered.\n");
    } else {
        _clients.push_back(name);
    }
    pthread_mutex_unlock(&_clientsMut);
    return ret;
}

/**
 * remove client from list by name
 */
int emServer::removeClient(string name) {
    int ret = -1;
    pthread_mutex_lock(&_clientsMut);
    auto it = _clients.begin();
    for(auto otherName : _clients) {
        if(name.compare(otherName) == 0) {
            _clients.erase(it, it + 1);
            ret = 0;
            break;
        }
        ++it;
    }
    pthread_mutex_unlock(&_clientsMut);
    return ret;
}