#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string>

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

// alias di WinSock per la chiusura del socket
#define closesocket close

// typedef definite in Windows.h
typedef wchar_t WCHAR;
#endif // _WIN32

#define MAX_PATH 260

// parametri di configurazione
struct params {
	struct sockaddr_in server;
	char* lst;
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
	int socketClient = 0;

	void getPassphrase(const char* printText, char* passphrase);
	unsigned long int generateToken(const char* printText);
	unsigned long int hashToken(char* token);
	void Send(const char* str);
	char* Recv();
	char* Send_Recv(const char* str);
	void Trasmissione();
	void clearSocket();
};

// calloc Wrapper
void* Calloc(unsigned long int nmemb, unsigned long int size);
// fprintf Wrapper
void ShowErr(const char* str);
// free Wrapper
void Free(void* arg, int size);