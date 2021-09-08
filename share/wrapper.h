#pragma once

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <cstdarg>
#include <string>
#include <cstring>

#ifdef _WIN32
#define _WINSOCKAPI_
#include <Windows.h>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>

#define SOCKET int
#endif // _WIN32


// calloc Wrapper
void* Calloc(size_t nmemb, size_t size);
// realloc Wrapper
void* Realloc(void* var, size_t new_size);
// fprintf Wrapper
void ShowErr(const char* str);
// free Wrapper
void Free(void* arg, size_t size = 0);
// strcpy Wrapper
void Strcpy(char* dest, size_t size, const char* src);
// asprintf Wrapper
int Asprintf(char*& buffer, const char* Format, ...);
// fopen Wrapper
bool Fopen(FILE** f, const char* FileName, const char* Mode);
// fread Wrapper
size_t Fread(FILE* file, void* Buffer, size_t BufferLen, size_t SizeSingleElement, size_t ElementCount);

//////////////////////////

void Send(SOCKET soc, const char* str, size_t bufferMaxLen);
void SendAll(SOCKET soc, const char* str, size_t bufferMaxLen);
void SendReadF(SOCKET soc, FILE* _f, size_t BufferMaxLen);

int Recv(SOCKET soc, char* _return, size_t bufferMaxLen);
int ReadAll(SOCKET soc, char*& ans);
size_t RecvWriteF(SOCKET soc, FILE* _f, size_t BufferMaxLen);

bool _endingSequence(char* buffer, size_t size);