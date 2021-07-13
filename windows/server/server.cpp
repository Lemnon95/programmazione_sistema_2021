// server.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//

#include <iostream>
#include "../../lib/share_lib_server.h"

SharedLibServer* a = NULL;

int main(int argc, char* argv[]) {
    // parser input
    // gestione segnali
    #ifdef __linux__
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGHUP, &sigIntHandler, NULL);
    #endif

    a = new SharedLibServer(argc, argv);

    // genera token
    a->getToken_s();

    // crea threads
    a->spawnSockets();

    // rimani in ascolto
    a->beginServer();

    return 0;
}

/* Handler di CTRL+C */
#ifdef __linux__
void my_handler(int s) {
  esci = true;
  pthread_mutex_lock(&mutex);
  pthread_cond_signal(&cond_var);
  pthread_mutex_unlock(&mutex);
  while(chiusura < 10);
}
#else //windows

#endif // __linux__
