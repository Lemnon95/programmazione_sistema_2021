#pragma once

#define MAX_PATH 260

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string>
#include <filesystem>
#include <stdarg.h>
#include "wrapper.h"

#ifdef _WIN32
#define _WINSOCKAPI_ 
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "Ws2_32.lib")

#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

// alias di WinSock per la chiusura del socket
#define closesocket close
#define SOCKET int

#endif // _WIN32

// parametri di configurazione
struct params {
	struct sockaddr_in server;
	char* lsf;
	char* exec;
	struct download {
		char* src;
		char* dest;
	} download;
	struct upload {
		char* src;
		char* dest;
	} upload;
};

class SharedLibClient {

public:
	SharedLibClient(int argc, char* argv[]);
	~SharedLibClient();
	void Connect();

private:
	params parametri;
	unsigned long int T_s = 0;
	unsigned long int T_c = 0;
	SOCKET socketClient = 0;

	void getPassphrase(const char* printText, char* passphrase);
	unsigned long int generateToken(const char* printText);
	unsigned long int hashToken(char* token);
	
	void Trasmissione();
	void clearSocket();

	void GestioneComandi();
	void LSF();
	void EXEC();
	void DOWLOAD();
	void UPLOAD();

};