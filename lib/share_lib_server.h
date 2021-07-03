// libreria portabile usabile sia su linux che su windows
// qui tutte le funzioni devono lavorare sia su linux che su windows

#pragma once

// MAX_PATH lunghezza massima per i percorsi di sistema
#define MAX_PATH 260

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

// alias per funzione di linux
#define strtok_r strtok_s
// win32 condition variable 
inline CONDITION_VARIABLE Threadwait;

inline CRITICAL_SECTION CritSec;
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <pthread.h>
#include <signal.h>
//dichiarazione mutex e condition variables linux
pthread_mutex_t mutex;
pthread_cond_t cond_var;
// alias di WinSock per la chiusura del socket
#define closesocket close

// typedef definite in Windows.h
typedef wchar_t WCHAR;
#endif //_WIN32

// definizione della coda
// la struttura conterrà i socket delle connessioni accettate
typedef struct queue {
	int socket_descriptor;
	struct queue* link;
} Queue;

inline Queue* front;
inline Queue* rear;

bool inline wake_up_all = false; //global variable indicating to end all threads 
int inline size = 0; //queue size
int inline thread_number;



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
	short unsigned int nthread; //0 - 65535, non più di 65534 poichè saturerebbero le porte disponibili (65535-1 dove -1 è del server stesso)
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
	void beginServer();

private:
	params parametri;
	unsigned long int T_s = 0;
	FILE* FileDescLog = NULL;
	int socketMaster = 0;
	int* threadChild = NULL;
	struct sockaddr_storage socketChild;

	void parseConfig();
	void getPassphrase(char* passphrase);
	unsigned long int generateToken();
	unsigned long int hashToken(char* token);
	void clearSocket();
	void openLog();
	void closeLog();

};

////////////////////////////////////////////////////////////////////////////////////

void Enqueue(int socket_descriptor, struct queue** front, struct queue** rear);

int Dequeue(int* socket_descriptor, struct queue** front, struct queue** rear);

// calloc Wrapper
void* Calloc(unsigned long int nmemb, unsigned long int size);
// fprintf Wrapper
void ShowErr(const char* str);
// free Wrapper
void Free(void * arg, int size);
//thread function
void* Accept(void* rank);
/* Handler di CTRL+C */
#ifdef __linux__
void my_handler(int s);
#endif // __linux__
