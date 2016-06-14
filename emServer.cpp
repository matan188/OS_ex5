
#include "emServer.h"

using namespace std;

vector<pthread_t *> threadsVec;
int socket_desc;
emServer * ems = new emServer();

bool cmpEvents(pair<Event*, vector<string>*> * a, pair<Event*, vector<string>*> * b) {
    if(a == nullptr) {
        return -1;
    } else if(b == nullptr) {
        return 1;
    } else {
        return (a->first)->getId() < (b->first)->getId();
    }
}

void writeToLog(string msg) {
    cout << "LOG\t" << msg << endl;
}

void * doJob(void * p) {
    int client_sock = *((int *) p);
    cout << "doing job: " << client_sock << endl;

    char client_message[2000];
    ssize_t read_size;
    // Receive a message from client
    while( (read_size = read(client_sock, client_message, 2000)) > 0 )
    {
        string str = string(client_message);

        size_t pos = str.find(" ");

        string clientName = str.substr(0, pos);
        str = str.substr(pos + 1);

        cout << "client name: " << clientName << endl;

        pos = str.find(" ");

        string command = str.substr(0, pos);
        str = str.substr(pos + 1);

        cout << "command name: " << command << endl;

        if(!command.compare("REGISTER")) {
            // do register
            int ret = ems->addClient(clientName);
            if(ret == -1) {
                //
            } else {
                write(client_sock , client_message, strlen(client_message));
            }
        } else if() {

        }
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

    close(client_sock);
    cout << "END:: doing job" << endl;
    return nullptr;
}

int main(int argc, char * argv[]) {

    if(argc != 2) {
        cout << "Usage: emServer portNum" << endl;
        exit(0);
    }

    /*
    cout << ems->addEvent("testEvent", "25/06/16", "description text") << endl;
    cout << ems->addEvent("testEvent", "25/06/16", "description text") << endl;
    cout << ems->addEvent("testEvent", "25/06/16", "description text") << endl;
    cout << ems->removeEvent(1) << endl;
    cout << ems->removeEvent(2) << endl;
    cout << ems->addEvent("testEvent", "25/06/16", "description text") << endl;
    cout << ems->addClient("x") << endl;
    cout << ems->removeClient("x") << endl;
    cout << ems->addClient("x") << endl;
    */
    int portNum = atoi(argv[1]); // set port number
    int client_sock , c;
    struct sockaddr_in server, client;

    fd_set readset;

    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        cout << "Could not create socket" << endl;
    }
    cout << "Socket created" << endl;

    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(portNum);

    // Bind
    if(bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        // Print the error message
        cerr << "bind failed. Error" << endl;
        return 1;
    }
    cout << "bind done" << endl;

    do {
        // Listen
        listen(socket_desc , 10);

        client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
        if(client_sock < 0) {
            // error: select error
            cout << "cs: " << client_sock << endl;
        }

        pthread_t p;
        threadsVec.push_back(&p);
        int ret = pthread_create(&p, NULL, doJob, (void *) &client_sock);
        if(ret == -1) {
             //TODO
            break;
        }

    } while(true);

    int t_res;
    for(int i = 0; i < threadsVec.size(); ++i) {
        t_res = pthread_join(*threadsVec[i], NULL);
        if(t_res != 0) {
            //TODO
            //sysError("pthread_join");
        }
    }


    /*
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
    */

    return 0;
}

emServer::emServer() {
    _eventsMut = PTHREAD_MUTEX_INITIALIZER;
    _eventCounter = 0;
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
 * returns id of new event on success, -1 on error
 */
int emServer::addEvent(string title, string date, string description) {
    // find empty cell and add event
    vector<string> * clients = new vector<string>;
    int id = getEventCounter();
    Event * event = new Event(id, title, date, description);
    auto p = new pair<Event*, vector<string>*>(event, clients);
    int i = 0;
    bool foundEmptyCell = false;
    pthread_mutex_lock(&_eventsMut);
    for(auto it : _events) {
        if(it == nullptr) {
            _events.at(i) = p;
            foundEmptyCell = true;
            break;
        }
        ++i;
    }
    // if there is no empty cell, push new event in vector's end
    if(!foundEmptyCell) {
        // no empty cell. use push back
        _events.push_back(p);
    }

    sort(_events.begin(), _events.end(), cmpEvents); // keep vector sorted by event id
    pthread_mutex_unlock(&_eventsMut);
    return id;
}

/**
 * remove event from _events
 * returns 0 on success, -1 on error.
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
 * the same name is not exists yet.
 * returns 0 on success, -1 on error.
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
 * remove client from client's list by name
 * returns 0 on success, -1 on error.
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

/**
 * increase current event counter and returns it
 */
int emServer::getEventCounter() {
    ++_eventCounter;
    return _eventCounter;
}

/**
 * assign a client to event's list.
 * returns 0 on success, -1 on error.
 */
int emServer::assignClientToEvent(int eventId, string clientName) {
    int ret = -1;
    bool eventIdExists = false;
    for(auto event : _events) {
        if(event->first->getId() == eventId) {
            eventIdExists = true;
            bool clientAlreadyAssigned = false;
            for(auto client : *(event->second)) {
                if(client.compare(clientName)) {
                    clientAlreadyAssigned = true;
                    break;
                }
            }
            if(clientAlreadyAssigned) {
                // error: client already assigned to this event
            } else {
                event->second->push_back(clientName);
                ret = 0;
            }
        }
    }
    if(!eventIdExists) {
        // error: event id not exists
    }
    return ret;
}

/**
 * remove a client from event's list.
 * returns 0 on success, -1 on error.
 */
int emServer::removeClientFromEvent(int eventId, string clientName) {
    int ret = -1;
    for(auto event : _events) {
        if(event->first->getId() == eventId) {
            bool clientWasAssigned = false;
            auto it = (event->second)->begin();
            for(auto client : *(event->second)) {
                if(client.compare(clientName)) {
                    *(event->second)->erase(it, it + 1);
                    clientWasAssigned = true;
                    ret = 0;
                    break;
                }
                ++it;
            }
            if(!clientWasAssigned) {
                // error - client was not assigned
            }
        }
    }
    return ret;
}