// server.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//

#include <iostream>
#include "share_lib_client.h"

int main(int argc, char* argv[]) {

    SharedLibClient a(argc,argv);

    a.Connect();


    return 0;
}
