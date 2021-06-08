// server.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//

#include <iostream>
#include "../../lib/share_lib.h"

int main(int argc, char* argv[]) {
    
    // parser input
    ShareFunction a(argc, argv);

    // genera token
    a.getToken_s();

    // crea threads

    // rimani in ascolto


    return 0;
}
