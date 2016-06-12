#ifndef OS_EX5_EVENT_H
#define OS_EX5_EVENT_H

#include<iostream>
#include<stdio.h>

using namespace std;

class Event {
private:
    string _title;
    string _date;
    string _description;
public:
    Event(string title, string date, string description);
};


#endif //OS_EX5_EVENT_H
