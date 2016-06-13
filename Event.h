#ifndef OS_EX5_EVENT_H
#define OS_EX5_EVENT_H

#include<iostream>
#include<stdio.h>

using namespace std;

class Event {
private:
    int _id;
    string _title;
    string _date;
    string _description;
public:
    Event(int id, string title, string date, string description);
    int getId() { return _id; };
    string getTitle() { return _title; };
    string getDate() { return _date; };
    string getDescription() { return _description; };
};


#endif //OS_EX5_EVENT_H
