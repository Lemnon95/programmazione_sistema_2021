// server.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//

#include <iostream>
#include "../../lib/share_lib_server.h"


int main(int argc, char* argv[]) {
    
    // parser input
    SharedLibServer(argc, argv);


    // genera token
    getToken_s();

    // crea threads
    spawnSockets();
    #ifdef __linux__
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGHUP);
    if((sigprocmask(SIG_BLOCK, &sigset, nullptr)== -1))
    	printf("Failure sigprocmask");
    #endif
    // rimani in ascolto
    beginServer();

    return 0;
}

