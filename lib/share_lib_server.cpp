#include "share_lib_server.h"

// costruttore classe
SharedLibServer::SharedLibServer(int argc, char* argv[]) {
    
    // gestione segnali
#ifdef __linux__
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
#endif
    
    
    
    
    WCHAR* _path = NULL;

    // trova il percorso temp
    #ifdef _WIN32
    _path = (WCHAR*)Calloc(MAX_PATH+1, sizeof(WCHAR)); // instanzio un array di MAX_PATH caratteri, MAX_PATH è definito da windows

    GetTempPathW(MAX_PATH, _path); // chiedo al sistema il percorso temporaneo, _t conterrà una cella vuota alla fine

    wcscat_s(_path, MAX_PATH, L"server.log");
    if (errno) {
        ShowErr("errore appendere nome file a percorso log");
    }

    #else
    _path = (WCHAR*)Calloc(sizeof("/tmp/server.log"),sizeof(WCHAR)); // sizeof("123") + \0 = 3+1, conta in automatico un \0 alla fine
    _path = (WCHAR*)"/tmp/server.log";
    #endif // _WIN32

    // parametri di default
    this->parametri = { 8888, 10, NULL, false, _path };

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

            this->parametri.port = atoi(argv[argc + 1]);
            if (errno) {
                ShowErr("il parametro di -p non risulta un numero");

            }

            if (this->parametri.port == 0) {
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

            this->parametri.nthread = atoi(argv[argc + 1]);
            if (errno) {
                ShowErr("il parametro di -n non risulta un numero");
            }
            thread_number = this->parametri.nthread;

            if (this->parametri.nthread == 0) {
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

            this->parametri.configPath = (char*)Calloc(sizeof(argv[argc + 1])+1, sizeof(char));
            this->parametri.configPath = argv[argc + 1];

        }

        // -s
        if (strcmp(argv[argc], "-s") == 0) {
            this->parametri.printToken = true;
        }

        // -l <path>
        if (strcmp(argv[argc], "-l") == 0) {

            if (argc + 1 >= maxArg) {
                ShowErr("parametro -l incompleto");

            }

            if (argv[argc + 1][0] == '-') {
                ShowErr("parametro -l incompleto");
            }

            this->parametri.logPath = (WCHAR*)Calloc(sizeof(argv[argc + 1]) + 1, sizeof(WCHAR));
            //this->parametri.logPath = (WCHAR*)argv[argc + 1];
            #ifdef _WIN32
            mbstowcs_s(NULL,
                this->parametri.logPath,
                sizeof(argv[argc + 1]) + 1,
                argv[argc + 1],
                sizeof(argv[argc + 1])
            );
            if (errno) {
                ShowErr("errore nel convertire stringa -l");
            }
            #else
            if (mbstowcs(this->parametri.logPath,
            argv[argc + 1],
            sizeof(argv[argc + 1])
            ) == -1) {
                ShowErr("errore nel convertire stringa -l");
            }
            #endif // _WIN32
        }

    }

    // debug print
    wprintf(L"---\nPorta: %d\nNumero thread: %d\nConfig path: %hs \nLog path: %ls \nStampa token: %d\n---\n",
        this->parametri.port,
        this->parametri.nthread,
        this->parametri.configPath,
        this->parametri.logPath,
        this->parametri.printToken);


    // se definito leggi il config
    // sovrascrivi le impostazioni che sono defaut
    this->parseConfig();

    // debug print
    wprintf(L"---\nPorta: %d\nNumero thread: %d\nConfig path: %hs \nLog path: %ls \nStampa token: %d\n---\n",
        this->parametri.port,
        this->parametri.nthread,
        this->parametri.configPath,
        this->parametri.logPath,
        this->parametri.printToken);

}
// distruttore classe
SharedLibServer::~SharedLibServer() {
    // alla distruzione di questa classe
    printf("distruzione classe\n");
    if(this->FileDescLog != NULL)
        this->closeLog();

    this->clearSocket();
}

void SharedLibServer::parseConfig() {

    // se definito leggi il config
    // sovrascrivi le impostazioni che sono defaut
    if (this->parametri.configPath != NULL) {
        FILE* _tConf = NULL;
    #ifdef _WIN32
        fopen_s(&_tConf, this->parametri.configPath, "r");
    #else // linux
        _tConf = fopen(this->parametri.configPath, "r");
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
            // di this->parametri
            // non è ammessa la modifica del configPath (dato in input)
            switch (i) 	{
                case 0:
                    port = std::stoi(configData);
                    if (port > 0 && port < 65536) {
                        this->parametri.port = port;
                    }
                    else {
                        ShowErr("errore nel valore del config per i = 0");
                    }
                    break;
                case 1:
                    nthread = std::stoi(configData);
                    if (nthread > 0 && nthread < 65536) {
                        this->parametri.nthread = nthread;
                    }
                    else {
                        ShowErr("errore nel valore del config per i = 1");
                    }
                    break;
                case 2:
                    printTok = std::stoi(configData);
                    if (printTok > 0) {
                        this->parametri.printToken = true;
                    }
                    break;
                case 3:
                    #ifdef _WIN32
                    mbstowcs_s(NULL,
                        this->parametri.logPath,
                        sizeof(configData)+1,
                        configData,
                        sizeof(configData)
                    );
                    if (errno) {
                        ShowErr("errore conversione argomento i = 3");
                    }
                    #elif _linux_  //usato elif e non else perché dava problemi di scope con argv e argc
                    if(mbstowcs(this->parametri.logPath,
                 argv[argc + 1],
                    sizeof(argv[argc + 1])
                 ) == -1) {
                    ShowErr("errore conversione argomento i = 3");
                 }
                 #endif //_WIN32
                    break;
                default:
                    break;
            }


        }
        fclose(_tConf);

    }

}

void SharedLibServer::getPassphrase(char* passphrase) {
    printf("Immetti passphrase (max 254): ");
    fgets(passphrase, 254, stdin);
}

unsigned long int SharedLibServer::generateToken() {

    char* passphrase = (char*)Calloc(256, sizeof(char));

    this->getPassphrase(passphrase);

    unsigned long int k = this->hashToken(passphrase);

    // reset passphrase
    Free(passphrase, 256);

    return k;
}

unsigned long int SharedLibServer::hashToken(char* token) {


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

void SharedLibServer::clearSocket() {
    // se instanziato, chiudi il socket
    if (this->socketMaster != 0) {
        closesocket(this->socketMaster);
    }
    // chiudi i socket in ascolto
    for (int i = 0; i < this->parametri.nthread; i++) {
        closesocket(this->threadChild[i]);
    }


#ifdef _WIN32
    WSACleanup();
#endif
}

// TODO: nel scrivedere su log usare flock()
void SharedLibServer::openLog() {
    // controlla se il file è già aperto
    if (this->FileDescLog == NULL) {
        // se non è aperto
        // apri il file in modalità Append
        #ifdef _WIN32
        fopen_s(&(this->FileDescLog),(char*)(this->parametri.logPath), "a");
        #else
        this->FileDescLog = fopen((char*)(this->parametri.logPath), "a");
        #endif
        // se da errore
        if (errno) {
            ShowErr("errore nell'aprire il file");
        }
    }

}

void SharedLibServer::closeLog() {
    // se è aperto il file
    if (this->FileDescLog != NULL) {
        // tenta di chiuderlo
        if (fclose(this->FileDescLog)) {
            ShowErr("errore nel chiudere il file log");
        }
    }

}

unsigned long int SharedLibServer::getToken_s() {

    this->T_s = this->generateToken();

    if (this->parametri.printToken) {
        printf("\ntoken: %lu\n", this->T_s);
    }

    return T_s;

}

void SharedLibServer::spawnSockets() {
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
    this->socketMaster = socket(PF_INET, SOCK_STREAM, 0);
    if (this->socketMaster < 0) {
        this->clearSocket();
        ShowErr("Errore creazione socket master");
    }

    int _e = true;

    if (setsockopt(this->socketMaster, SOL_SOCKET, SO_REUSEADDR, (char*)&_e, sizeof(_e)) < 0) {
        ShowErr("impossibile impostare il socket server");
    }

    // crea figli
    this->threadChild = (int*)Calloc(this->parametri.nthread, sizeof(int));

    // instanzio i thread nella lista
    for (int q = 0; q < this->parametri.nthread; q++) {

    #ifdef _WIN32

        if ((this->threadChild[q] = (int)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(Accept), NULL, 0, NULL)) == NULL) {
            ShowErr("Impossibile creare un thread");
        }

    #else // linux
      pthread_mutex_init(&mutex, NULL);
      pthread_cond_init(&cond_var, NULL);
      // TODO: ricordarsi di fare il destroy
      if (pthread_create(&this->threadChild[q], NULL, Accept, (void*) q) != 0)
            printf("Failed to create thread\n");
    #endif

    }

    // impostazioni base del server
    struct sockaddr_in masterSettings;
    masterSettings.sin_family = AF_INET; // protocollo IP
    inet_pton(AF_INET,"0.0.0.0", &masterSettings.sin_addr); // server IP
    masterSettings.sin_port = htons(this->parametri.port); // Server port

    memset(masterSettings.sin_zero, '\0', sizeof(masterSettings.sin_zero));

    // apre il server
    if (bind(this->socketMaster, (struct sockaddr*)&masterSettings, sizeof(masterSettings)) < 0) {
        ShowErr("Impossibile aprire il socket del server");
    }

    if (listen(this->socketMaster, this->parametri.nthread) < 0) {
        ShowErr("Impossibile stare in ascolto sulla porta specificata");
    }

    printf("Server in ascolto su porta %d\n", this->parametri.port);
}

void SharedLibServer::beginServer() {

    socklen_t addr_size;
    int newSocket = 0;

    // TODO: creare nthread ed eseguire Accept()
    while (1) {
        //Accept call creates a new socket for the incoming connection
        addr_size = sizeof(this->socketChild);
        // bloccante
        newSocket = accept(this->socketMaster, (struct sockaddr*)&(this->socketChild), &addr_size);
        Enqueue(newSocket, &front, &rear); //inserisco nella coda il nuovo socket descriptor


        // TODO: svegliare 1 thread ad una richiesta di accept
        #ifdef _WIN32

        WakeConditionVariable(&Threadwait);
        

        #else // linux
        pthread_mutex_lock(&mutex);
            pthread_cond_signal(&cond_var);	//sveglio un thread per gestire la nuova connessione
            pthread_mutex_unlock(&mutex);

        #endif // _WIN32

    }
}

// end class
//////////////////////////////////////////////////////////////////////////////////

// Dopo che un thread viene creato esegue questa funzione
void* Accept(void* rank) {

    while (!wake_up_all) {
        int socket_descriptor;
        long my_rank = (long) rank;

        // il thread rimane in attesa dello blocco tramite una condition variable
    #ifdef _WIN32
        EnterCriticalSection(&CritSec);

        SleepConditionVariableCS(&Threadwait, &CritSec, INFINITE);
    #else //linux
        pthread_mutex_lock(&mutex);
        while(pthread_cond_wait(&cond_var, &mutex) != 0); //thread goes to sleep
    #endif
        //now I'm awake

        if (!wake_up_all) { //sono sveglio perché ho veramente del lavoro da fare
            Dequeue(&socket_descriptor, &front, &rear);
        }
    #ifdef _WIN32
        LeaveCriticalSection(&CritSec);
    #else //linux
        pthread_mutex_unlock(&mutex);
    #endif

        // TODO: gestire richiesta



    }

    return NULL;
}

void Enqueue(int socket_descriptor, struct queue** front, struct queue** rear) {
    Queue* task = NULL;

    task = (struct queue*)malloc(sizeof(struct queue));
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

int Dequeue(int* socket_descriptor, struct queue** front, struct queue** rear) {
    Queue* temp = NULL;
    if (size == 0) {
        return -1;
    }
    temp = *front;
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

    memset(arg, '\0', size);
    free(arg);

}

/* Handler di CTRL+C */
#ifdef __linux__
void my_handler(int s) {
  printf("Caught signal %d\n",s);
  /* main thread wakes up other threads */
    pthread_mutex_lock(&mutex);
    wake_up_all = true;
    pthread_cond_broadcast(&cond_var);
    pthread_mutex_unlock(&mutex);

  /* main thread joins the other threads */
    for (long i = 0; i < thread_number; i++) {
        pthread_join(threadChild[i], NULL);
    }
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond_var);
  exit(1);
}
#else //windows

#endif // __linux__
