// server.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//

#include <iostream>
#include "../../lib/share_lib_server.h"

int main(int argc, char* argv[]) {
    // parser input
    SharedLibServer a(argc, argv);


#ifdef _WIN32
    
    InitializeConditionVariable(Threadwait);

    // TODO: creare un metodo per la libreria e passare la condition variable

#else
#endif

    

    // genera token
    a.getToken_s();

    // crea threads
    a.spawnSockets();

    // rimani in ascolto
    a.beginServer();

    return 0;
}
