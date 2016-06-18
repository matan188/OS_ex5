#!/bin/bash

emClient Amit 127.0.0.1 8080 < success.txt
tkdiff emServer.log successServer.log
tkdiff emClient.log successClient.log

#emClient Amit 127.0.0.1 8080 < failure.txt
#tkdiff emServer.log failureServer.log
#tkdiff emClient.log failureClient.log

#emClient Amit 127.0.0.1 8080 < complex.txt
#tkdiff emServer.log complexServer.log
#tkdiff emClient.log complexClient.log

