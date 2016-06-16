//
// Created by root on 6/12/16.
//

#ifndef OS_EX5_EMCLIENT_H
#define OS_EX5_EMCLIENT_H

#include <vector>

class emClient {
private:
    bool _isRegistered;
public:
    emClient();
    bool getIsRegistered() { return _isRegistered; };
    void setRegister(bool state) { _isRegistered = state; };
};


#endif //OS_EX5_EMCLIENT_H
