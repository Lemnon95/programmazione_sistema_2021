// libreria portabile usabile sia su linux che su windows
// qui tutte le funzioni devono lavorare sia su linux che su windows

#pragma once

// MAX_PATH lunghezza massima per i percorsi di sistema
#define MAX_PATH 260
#define SERVERLISTEN "0.0.0.0"

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string>
#include <time.h>
#include <ctime>
#include <filesystem>
#include <stdarg.h>

#ifdef _WIN32
#define _WINSOCKAPI_
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "Ws2_32.lib")

#include <Lmcons.h>

// alias per funzione di linux
#define strtok_r strtok_s
#define popen _popen
#define pclose _pclose
#define fscanf fscanf_s

// win32 condition variable
inline CONDITION_VARIABLE Threadwait;
inline CRITICAL_SECTION CritSec;
inline CRITICAL_SECTION FileLock;

#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <pthread.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <pwd.h>

//dichiarazione mutex e condition variables linux
inline pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
inline pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
inline pthread_t* threadChild = NULL;
inline pthread_t thread_handler;
inline bool wake_one = true;
inline sig_atomic_t esci = 0;
inline int chiusura = 0;
inline int signum;
// alias di WinSock per la chiusura del socket
#define closesocket close
#define SOCKET int


#endif //_WIN32

//////////////////////////////////////////////////////////////////////////////////
// variabili e strutture

// definizione della coda
// la struttura conterrà i socket delle connessioni accettate
typedef struct queue {
	SOCKET socket_descriptor;
	struct queue* link;
} Queue;

inline Queue* front;
inline Queue* rear;
inline int size = 0; //queue size


/*  struttura per mantenere le impostazioni passate da argomenti
parametri opzionali:
-p <TCP port>		porta TCP su cui stare in ascolto (default 8888);
-n <thread max>		numero massimo di thread per gestire le richieste (default 10);
-c <path file>		percorso di configurazione;
-s					stampa su stdout il token T_s quando viene generato;
-l <path file>		percorso di log (default /tmp/server.log)
*/
typedef struct _params {
	unsigned short int port; // 0 - 65535
	unsigned short int nthread; //0 - 65535, non più di 65534 poichè saturerebbero le porte disponibili (65535-1 dove -1 è del server stesso)
	char* configPath; // stringa di lunghezza variabile
	bool printToken; // indice booleano per indicare se printare o meno il token T_s
	char* logPath; // stringa variabile del percoso dei log
} params;


inline params parametri;
inline unsigned long int T_s = 0;
inline FILE* FileDescLog = NULL;
inline SOCKET socketMaster = 0;
#ifdef _WIN32
inline void** threadChild = NULL;
#endif //WIN32
inline sockaddr_storage socketChild;

////////////////////////////////////////////////////////////////////////////////////
// funzioni

void SharedLibServer(int argc, char* argv[]);
void CloseServer();
void parseConfig();

void getToken_s();
void getPassphrase(char* passphrase);
unsigned long int generateToken();
unsigned long int hashToken(char* token);

void spawnSockets();
void beginServer();
void clearSocket();

void openLog();
void writeLog(unsigned long int Tpid, SOCKET soc, char* command);
void closeLog();

////////////////////////////////////////////////////////////////////////////////////
// thread function

#ifdef __linux__
//void* SigHandler(void* dummy);
#endif

void* Accept(void* rank);
int Autenticazione(SOCKET socket_descriptor);
void GestioneComandi(SOCKET socket_descriptor, unsigned long int Tpid);
int LSF(SOCKET socket_descriptor, char* path);
int EXEC(SOCKET socket_descriptor, char* cmd);
int DOWNLOAD(SOCKET socket_descriptor, char* cmd);
int SIZE_(SOCKET socket_descriptor, char* path, bool end=0);
int UPLOAD(SOCKET socket_descriptor, char* cmd);
char* _exec(const char* cmd);

// send-recv Wrapper
void SendAll(SOCKET soc, const char* str, unsigned long long bufferMaxLen);
void Send(SOCKET soc, const char* str, unsigned long long bufferMaxLen);
int Recv(SOCKET soc, char* _return, unsigned long long bufferMaxLen);
int ReadAll(SOCKET soc, char*& ans);
int ReadMax(SOCKET soc, char*& ans, unsigned long long BufferMaxLen);
int RecvWriteF(SOCKET soc, FILE* _f, unsigned long long BufferMaxLen);

////////////////////////////////////////////////////////////////////////////////////

void Enqueue(SOCKET  socket_descriptor, Queue** front, Queue** rear);
int  Dequeue(SOCKET* socket_descriptor, Queue** front, Queue** rear);



bool _endingSequence(char* buffer, unsigned long long size);

////////////////////////////////////////////////////////////////////////////////////

// calloc Wrapper
void* Calloc(size_t nmemb, size_t size);
// fprintf Wrapper
void ShowErr(const char* str);
// free Wrapper
void Free(void * arg, size_t size=0);
// strcpy Wrapper
void Strcpy(char* dest, size_t size, const char* src);
// asprintf Wrapper
int Asprintf(char*& buffer, const char* Format, ...);
