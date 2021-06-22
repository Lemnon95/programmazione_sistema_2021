// libreria portabile usabile sia su linux che su windows
// qui tutte le funzioni devono lavorare sia su linux che su windows
// � cossibile creare una d
// 
// urls di riferimento
// https://stackoverflow.com/questions/142508/how-do-i-check-os-with-a-preprocessor-directive
// https://en.wikipedia.org/wiki/C_data_types
// https://en.cppreference.com/w/cpp/header/stdexcept

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string>

#ifdef _WIN32
//#include <iostream>
#include <Windows.h>
#include <winsock.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

// alias di WinSock per la chiusura del socket
#define closesocket close

// typedef definite in Windows.h
typedef wchar_t WCHAR;
#endif // _WIN32

#define MAX_PATH 260




// struttura per mantenere le impostazioni passate da argomenti
/*
parametri opzionali:
-p <TCP port>		porta TCP su cui stare in ascolto (default 8888);
-n <thread max>		numero massimo di thread per gestire le richieste (default 10);
-c <path file>		percorso di configurazione;
-s					stampa su stdout il token T_s quando viene generato;
-l <path file>		percorso di log (default /tmp/server.log)
*/
struct params {
	short unsigned int port; // 0 - 65535
	short unsigned int nthread; //0 - 65535, non pi� di 65534 poich� saturerebbero le porte disponibili (65535-1 dove -1 � del server stesso)
	char* configPath; // stringa di lunghezza variabile
	bool printToken; // indice booleano per indicare se printare o meno il token T_s
	WCHAR* logPath; // stringa variabile del percoso dei log
};


class SharedLibServer {

public:
	SharedLibServer(int argc, char* argv[]);
	~SharedLibServer();
	unsigned long int getToken_s();
	void spawnSockets();

private:
	params parametri;
	unsigned long int T_s = 0;
	FILE* FileDescLog = NULL;
	void parseConfig();
	int socketMaster = 0;
	int* socketChild = NULL;

	void getPassphrase(char* passphrase);
	unsigned long int generateToken();
	void clearSocket();
	void openLog();
	void closeLog();

};

// calloc Wrapper
void* Calloc(unsigned long int nmemb, unsigned long int size);
// fprintf Wrapper
void ShowErr(const char* str);
// free Wrapper
void Free(void * arg, int size);