// server.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//

#include <iostream>
#include "share_lib_server.h"


int main(int argc, char* argv[]) {
#ifdef __linux__
  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGINT);
  sigaddset(&sigset, SIGHUP);
  if((sigprocmask(SIG_BLOCK, &sigset, nullptr)== -1))
    printf("Failure sigprocmask");
#else //_WIN32
    if (!SetConsoleCtrlHandler(CtrlHandler, TRUE)) { //tutte le funzioni ctrl sono gestite
        ShowErr("Impossibile creare handler ctrl");
    }
#endif
    // parser input
    SharedLibServer(argc, argv);


    // genera token
    getToken_s();

#ifdef __linux__
  #ifndef _DEBUG
    daemon(1, 0);
  #endif
#endif

    // crea threads
    spawnSockets();

    // rimani in ascolto
    beginServer();

    return 0;
}
