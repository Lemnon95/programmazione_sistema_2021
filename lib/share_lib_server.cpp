#include "share_lib_server.h"

// costruttore classe
void SharedLibServer(int argc, char* argv[]) {

    char* _path = NULL;
    _path = (char*)Calloc(MAX_PATH + 1, sizeof(char));
    // trova il percorso temp
    #ifdef _WIN32
    WCHAR* _Tpath = (WCHAR*)Calloc(MAX_PATH+1, sizeof(WCHAR));
    // instanzio una stringa di MAX_PATH caratteri, MAX_PATH è definito da Windows

    GetTempPathW(MAX_PATH, _Tpath);
    // chiedo al sistema il percorso temporaneo, _Tpath conterrà una cella vuota alla fine

    // concateno il testo "server.log" al path base della cartella temporanea
    wcscat_s(_Tpath, MAX_PATH, L"server.log");
    if (errno) {
        ShowErr("errore appendere nome file a percorso log");
    }
    // converto il percorso da WCHAR a char
    wcstombs_s(NULL, _path, MAX_PATH+1, _Tpath, MAX_PATH+1);
    if (errno) {
        ShowErr("errore convertire WCHAR in char*");
    }
    // pulisco la variabile _Tpath
    Free(_Tpath, MAX_PATH + 1);
    #else
    //_path = (char*)Calloc(sizeof("/tmp/server.log")+1,sizeof(char)); // sizeof("123") + \0 = 3+1, conta in automatico un \0 alla fine
    Strcpy(_path, MAX_PATH, "/tmp/server.log");
    #endif // _WIN32

    // parametri di default
    parametri = { 8888, 10, NULL, false, _path };

    // parsing argomenti
    unsigned long long maxArg = argc;
    while (argc > 0) {

        argc -= 1;

        // -p <port>
        if (strcmp(argv[argc], "-p") == 0) {

            if (argc + 1 >= maxArg) {
                ShowErr("parametro -p incompleto");

            }

            if (argv[argc + 1][0] == '-') {
                ShowErr("parametro -p incompleto");

            }
            // conversione stringa in unsigned short
            parametri.port = (unsigned short)strtoul(argv[argc + 1], NULL, 0);
            if (errno) {
                ShowErr("il parametro di -p non risulta un numero");

            }

            if (parametri.port == 0) {
                ShowErr("porta 0 non è valida");
            }

        }

        // -n <num>
        if (strcmp(argv[argc], "-n") == 0) {

            if (argc + 1 >= maxArg) {
                ShowErr("parametro -n incompleto");

            }

            if (argv[argc + 1][0] == '-') {
                ShowErr("parametro -n incompleto");
            }

            parametri.nthread = atoi(argv[argc + 1]);
            if (errno) {
                ShowErr("il parametro di -n non risulta un numero");
            }

            if (parametri.nthread == 0) {
                ShowErr("numero di thread invalido");
            }


        }

        // -c <path>
        if (strcmp(argv[argc], "-c") == 0) {

            if (argc + 1 >= maxArg) {
                ShowErr("parametro -c incompleto");

            }

            if (argv[argc + 1][0] == '-') {
                ShowErr("parametro -c incompleto");
            }

            parametri.configPath = (char*)Calloc(sizeof(argv[argc + 1])+1, sizeof(char));
            parametri.configPath = argv[argc + 1];

        }

        // -s
        if (strcmp(argv[argc], "-s") == 0) {
            parametri.printToken = true;
        }

        // -l <path>
        if (strcmp(argv[argc], "-l") == 0) {

            if (argc + 1 >= maxArg) {
                ShowErr("parametro -l incompleto");

            }

            if (argv[argc + 1][0] == '-') {
                ShowErr("parametro -l incompleto");
            }

            parametri.logPath = (char*)Calloc(sizeof(argv[argc + 1]) + 1, sizeof(char));
            parametri.logPath = argv[argc + 1];

        }

    }

    // debug print
    printf("---\nPorta: %d\nNumero thread: %d\nConfig path: %s \nLog path: %s \nStampa token: %d\n---\n",
        parametri.port,
        parametri.nthread,
        parametri.configPath,
        parametri.logPath,
        parametri.printToken);


    // se definito leggi il config
    // sovrascrivi le impostazioni che sono defaut
    parseConfig();

    // debug print
    printf("---\nPorta: %d\nNumero thread: %d\nConfig path: %s \nLog path: %s \nStampa token: %d\n---\n",
        parametri.port,
        parametri.nthread,
        parametri.configPath,
        parametri.logPath,
        parametri.printToken);

}
// distruttore classe
/*void ~SharedLibServer() {
    // alla distruzione di questa classe
    printf("distruzione classe\n");
    if(FileDescLog != NULL)
        closeLog();

    clearSocket();
}*/

void parseConfig() {

    // se definito leggi il config
    // sovrascrivi le impostazioni che sono defaut
    if (parametri.configPath != NULL) {
        FILE* _tConf = NULL;
    #ifdef _WIN32
        fopen_s(&_tConf, parametri.configPath, "r");
    #else // linux
        _tConf = fopen(parametri.configPath, "r");
    #endif
        if (errno) { // errno viene settato anche con la fopen, come richiesto da POSIX
            ShowErr("impossibile aprire (o inesistente) file config");
        }

        char line[1024];

        // gestione config
        while (fgets(line, 1024, _tConf)) {

            char* next_tok = NULL;
            char* configData = NULL;
            int i;
            int printTok,nthread,port = 0;


            // il primo strtok prende il numero del config
            i = std::stoi(strtok_r(line, " ", &next_tok));
            // il secondo strtok ottiene il valore associato
            configData = strtok_r(next_tok, " ", &next_tok);

            if (configData == NULL) {
                continue;
            }

            // i corrisponde al primo valore presente nel file
            // nello shwitch associo un numero al rispettivo valore
            // di parametri
            // non è ammessa la modifica del configPath (dato in input)
            switch (i) 	{
                case 0:
                    port = std::stoi(configData);
                    if (port > 0 && port < 65536) {
                        parametri.port = port;
                    }
                    else {
                        ShowErr("errore nel valore del config per i = 0");
                    }
                    break;
                case 1:
                    nthread = std::stoi(configData);
                    if (nthread > 0 && nthread < 65536) {
                        parametri.nthread = nthread;
                    }
                    else {
                        ShowErr("errore nel valore del config per i = 1");
                    }
                    break;
                case 2:
                    printTok = std::stoi(configData);
                    if (printTok > 0) {
                        parametri.printToken = true;
                    }
                    break;
                case 3:
                    parametri.logPath = configData;

                    break;
                default:
                    break;
            }


        }
        fclose(_tConf);

    }

}

void getPassphrase(char* passphrase) {
    printf("Immetti passphrase (max 254): ");
    fgets(passphrase, 254, stdin);
}

unsigned long int generateToken() {

    char* passphrase = (char*)Calloc(256, sizeof(char));

    getPassphrase(passphrase);

    unsigned long int k = hashToken(passphrase);

    // reset passphrase
    Free(passphrase, 256);

    return k;
}

unsigned long int hashToken(char* token) {


    unsigned long int k = 5381;
    // hashing
    // stessa phrase stresso hash
    // no fattori randomici
    // no fattori di tempo
    // bisogna basarci solo sull'input
    // e/o valori costanti
    for (int i = 0; i < strlen(token); ++i)
        k = (k * 27) + token[i];

    return k;
}

void clearSocket() {
    // se instanziato, chiudi il socket
    if (socketMaster != 0) {
        closesocket(socketMaster);
    }
    // chiudi i socket in ascolto
    for (int i = 0; i < parametri.nthread; i++) {
      #ifdef _WIN32
        closesocket((SOCKET)threadChild[i]);
      #else //_linux_
        closesocket(threadChild[i]);
      #endif
    }


#ifdef _WIN32
    WSACleanup();
#endif
}

// TODO: nel scrivedere su log usare flock()
void openLog() {
    // controlla se il file è già aperto
    if (FileDescLog == NULL) {
        // se non è aperto
        // apri il file in modalità Append
        #ifdef _WIN32
        fopen_s(&(FileDescLog),(char*)(parametri.logPath), "a");
        #else
        FileDescLog = fopen((char*)(parametri.logPath), "a");
        #endif
        // se da errore
        if (errno) {
            ShowErr("errore nell'aprire il file");
        }
    }

}

void closeLog() {
    // se è aperto il file
    if (FileDescLog != NULL) {
        // tenta di chiuderlo
        if (fclose(FileDescLog)) {
            ShowErr("errore nel chiudere il file log");
        }
    }

}

unsigned long int getToken_s() {

    T_s = generateToken();

    if (parametri.printToken) {
        printf("\ntoken: %lu\n", T_s);
    }

    return T_s;

}

void spawnSockets() {
#ifdef _WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        ShowErr("impossibile avviare WinSock");
    }

    InitializeConditionVariable(&Threadwait);
    InitializeCriticalSection(&CritSec);
#endif

    /*
    PF_INET = Internet Protocol (IP)
    SOCK_STREAM = TPC/IP
    */
    socketMaster = socket(PF_INET, SOCK_STREAM, 0);
    if (socketMaster < 0) {
        clearSocket();
        ShowErr("Errore creazione socket master");
    }

    int _e = true;

    if (setsockopt(socketMaster, SOL_SOCKET, SO_REUSEADDR, (char*)&_e, sizeof(_e)) < 0) {
        ShowErr("impossibile impostare il socket server");
    }

    // crea figli
    #ifdef _WIN32
    threadChild = (void**)Calloc(parametri.nthread, sizeof(void *));
    #else
    threadChild = (pthread_t*)Calloc(parametri.nthread, sizeof(pthread_t));
    #endif


    // instanzio i thread nella lista
    for (int q = 0; q < parametri.nthread; q++) {

    #ifdef _WIN32
        if ((threadChild[q] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(Accept), (LPVOID)(T_s), 0, NULL)) == NULL) {
            ShowErr("Impossibile creare un thread");
        }

    #else // linux
      pthread_mutex_init(&mutex, NULL);
      pthread_cond_init(&cond_var, NULL);
      // TODO: ricordarsi di fare il destroy
      if (pthread_create(&threadChild[(long)q], NULL, Accept, (void*)T_s) != 0)
            printf("Failed to create thread\n");
    #endif

    }


    // impostazioni base del server
    sockaddr_in masterSettings;
    memset(masterSettings.sin_zero, '\0', sizeof(masterSettings.sin_zero));
    masterSettings.sin_family = AF_INET; // protocollo IP
    if (!inet_pton(AF_INET, SERVERLISTEN, &masterSettings.sin_addr)) { // server IP
        ShowErr("non risulta un ip valido");
    }
    masterSettings.sin_port = parametri.port; // Server port

    // apre il server
    if (bind(socketMaster, (struct sockaddr*)&masterSettings, sizeof(masterSettings)) < 0) {
        ShowErr("Impossibile aprire il socket del server");
    }

    if (listen(socketMaster, parametri.nthread) < 0) {
        ShowErr("Impossibile stare in ascolto sulla porta specificata");
    }


    char _t[17] = {0};
    inet_ntop(AF_INET, &masterSettings.sin_addr, _t,17);
    printf("\nServer in ascolto su %s:%d\n", _t, masterSettings.sin_port);

}

void beginServer() {

    socklen_t addr_size;
    SOCKET newSocket = 0;

    while (1) {
        //Accept call creates a new socket for the incoming connection
        addr_size = sizeof(socketChild);
        // bloccante
        newSocket = accept(socketMaster, (sockaddr*)&(socketChild), &addr_size);
        if (newSocket == -1)
          break;
        Enqueue(newSocket, &front, &rear); //inserisco nella coda il nuovo socket descriptor

        printf("\nConnessione in entrata\n");

        #ifdef _WIN32

        WakeConditionVariable(&Threadwait);


        #else // linux
        pthread_mutex_lock(&mutex);
        wake_one = false;
        pthread_cond_signal(&cond_var);	//sveglio un thread per gestire la nuova connessione
        pthread_mutex_unlock(&mutex);

        #endif // _WIN32

    }
    closesocket(socketMaster);
}


// end class
//////////////////////////////////////////////////////////////////////////////////

// Dopo che un thread viene creato esegue questa funzione
void* Accept(void* rank) {
    #ifdef __linux__
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGHUP);
    pthread_sigmask(SIG_BLOCK, &sigset, nullptr);
    #endif
    // inizializzo rand
    srand(time(NULL));

    SOCKET socket_descriptor;

    unsigned long int T_s = (unsigned long int) rank;
    unsigned long int T_c = 0;
    unsigned long int nonce = 0;
    unsigned long int challenge = 0;
    unsigned long int challenge_nonce = 0;

    char* endP;
    char* next_tok;

    char _t[1024] = { 0 };
    char _auth[1024] = { 0 };
    char _command[1024] = { 0 };

    while (1) {
        // cleanup vars
        T_c = 0;
        nonce = 0;
        challenge = 0;
        challenge_nonce = 0;

        next_tok = NULL;
        endP = NULL;

        memset(_t, '\0', 1024);
        memset(_auth, '\0', 1024);
        memset(_command, '\0', 1024);

        // i thread vanno a dormire
        #ifdef _WIN32
        EnterCriticalSection(&CritSec);

        printf("thread id: %lu\n", GetCurrentThreadId());

        SleepConditionVariableCS(&Threadwait, &CritSec, INFINITE);
        #else //linux
        pthread_mutex_lock(&mutex);
        while (wake_one) {
          pthread_cond_wait(&cond_var, &mutex);
          if (esci) {
            chiusura++;
            pid_t x = syscall(__NR_gettid);
            printf("Exit Thread number: %d\n", x);
            pthread_cond_signal(&cond_var);
            pthread_mutex_unlock(&mutex);
            return NULL;
          }
        }
        wake_one = true;
        #endif
        // sezione critica

        Dequeue(&socket_descriptor, &front, &rear);
        #ifdef _WIN32
        LeaveCriticalSection(&CritSec);
        #else //linux
        pthread_mutex_unlock(&mutex);
        #endif
        // sezione parallela

        /*
        1. ricevere HELO
        2. generare challenge
            challenge = T_s XOR rand
        3. inviare challenge
        4. ricevere AUTH dal client
        5. risolve l'auth e risponde nel caso 200 o 400
        6. printo sul log
        risposte ai comandi + log
        */

        // passo 1
        Recv(socket_descriptor, _t);

        if (strncmp(_t, "HELO", 4) != 0) {
            closesocket(socket_descriptor);
            continue;
        }
        memset(_t, '\0', 1024);

        // passo 2,3,4
        nonce = rand() % 2147483647; // nonce
        challenge = T_s ^ nonce; // T_s XOR nonce

        snprintf(_t, 1024, "%lu", challenge); // unsigned long int to string

        Send_Recv(socket_descriptor, _auth, _t, "300"); // invio lo status e challenge, ottengo l'auth


        // passo 5
        Strcpy(_command, 1024, strtok_r(_auth, " ;", &next_tok)); // AUTH

        if (strncmp(_command, "AUTH", 4) != 0) {
            closesocket(socket_descriptor);
            continue;
        }

        // enc1
        Strcpy(_command, 1024, strtok_r(NULL, " ;", &next_tok)); // T_s XOR nonce XOR T_c
        T_c = T_s ^ nonce ^ strtoul(_command, &endP, 10);

        // enc2
        Strcpy(_command, 1024, strtok_r(NULL, " ;", &next_tok)); // T_c XOR nonce
        challenge_nonce = T_c ^ strtoul(_command, &endP, 10);

        if (challenge_nonce != nonce) {
            Send(socket_descriptor, "400");
            closesocket(socket_descriptor);
            printf("\nClient Rifiutato\n");
            continue;
        }
        Send(socket_descriptor, "200"); // nonce accettato

        printf("\nClient Accettato\n");

        // passo 6
        // TODO: print to log

    }

    return NULL;
}


void Send(SOCKET soc, const char* str) {

    if (str == NULL) {
        return;
    }

    if (send(soc, str, 1024, 0) < 0) {
        ShowErr("Errore nell'inviare un messaggio verso il server");
    }
}

void Recv(SOCKET soc, char* _return) {
    char* _t = (char*)Calloc(1024, sizeof(char));
    if (recv(soc, _t, 1024, 0) < 0) {
        ShowErr("Errore nel ricevere un messaggio dal client");
    }

    Strcpy(_return, 1024, _t);
    Free(_t, 1024);
}

void Send_Recv(SOCKET soc, char* _return, const char* str, const char* status) {

    if (status != NULL) {
        Send(soc, status);
    }

    if (str != NULL) {
        Send(soc, str);
    }

    Recv(soc, _return);

}


// funzioni per gestire la pila di socket
void Enqueue(SOCKET socket_descriptor, Queue** front, Queue** rear) {
    Queue* task = (Queue*)malloc(1 * sizeof(Queue));

    if (task == NULL) {
        ShowErr("impossibile allocare coda Enqueue");
    }

    task->socket_descriptor = socket_descriptor;
    task->link = NULL;
    if ((*rear)) {
        (*rear)->link = task;
    }

    *rear = task;

    if (!(*front)) {
        *front = *rear;
    }

    size++;
}

int Dequeue(SOCKET* socket_descriptor, Queue** front, Queue** rear) {

    if (size == 0) {
        return -1;
    }
    Queue* temp = *front;

    *socket_descriptor = temp->socket_descriptor;

    *front = (*front)->link;

    size--;
    free(temp);
    return 0;
}


// calloc Wrapper
void* Calloc(unsigned long int count, unsigned long int size) {

    if (count == 0 || size == 0) {
        ShowErr("Uno dei due numeri del Calloc è impostato a 0");
    }

    void* _t = calloc(count, size);
    if (_t == 0) {
        ShowErr("Impossibile allocare memoria");
    }

    return _t;
}

// fprintf Wrapper
void ShowErr(const char* str) {
    char t[1024];
    #ifdef _WIN32
        strerror_s(t, 1024, errno);
    #else
        strerror_r(errno, t, 1024);
    #endif

    printf("\n%s\n", t);
    fprintf(stderr, "%s\n", str);
    exit(1);
    return;

}

// free Wrapper
void Free(void* arg, int size) {

    if (arg == NULL) {
        ShowErr("variabile data a Free() è NULL\n");
    }

    if (size == 0) {
        ShowErr("Free(_, 0) impossibile svuotare variabile di lunghezza 0");
    }

    memset(arg, '\0', size);
    free(arg);

}

// strcpy Wrapper
void Strcpy(char* dest, unsigned int size, const char* src) {
    memset(dest, '\0', size);
#ifdef _WIN32
    strcpy_s(dest, size, src);
    if (errno) {
        ShowErr("Errore nel copiare una stringa");
    }
#else //linux
    strncpy(dest, src, size);
#endif

}
