#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string>

#ifdef _WIN32
//#include <iostream>
#include <Windows.h>
#include <winsock.h>
#include <ws2tcpip.h>
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

// parametri di configurazione
struct params {
	sockaddr_in server;
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

private:
	params parametri;
	unsigned long int T_s = 0;
	unsigned long int T_c = 0;

	void getPassphrase(const char* printText, char* passphrase);
	unsigned long int generateToken(const char* printText);
	unsigned long int hashToken(char* token);
};

// calloc Wrapper
void* Calloc(unsigned long int nmemb, unsigned long int size);
// realloc Wrapper
void* Realloc(void* memblock, size_t size);
// fprintf Wrapper
void ShowErr(const char* str);
// free Wrapper
void Free(void* arg, int size);