#include "share_lib_server.h"


#if defined(_WIN32)
#define strtok_r strtok_s
#endif


// costruttore classe
SharedLibServer::SharedLibServer(int argc, char* argv[]) {
    WCHAR* _path = NULL;
    
    // trova il percorso temp
    #ifdef _WIN32
    _path = (WCHAR*)Calloc(MAX_PATH+1, sizeof(WCHAR)); // instanzio un array di MAX_PATH caratteri, MAX_PATH � definito da windows
    
    GetTempPathW(MAX_PATH, _path); // chiedo al sistema il percorso temporaneo, _t conterr� una cella vuota alla fine 
    
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
                ShowErr("porta 0 non � valida");
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
        #else
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
            // non � ammessa la modifica del configPath (dato in input)
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
                    #elif _linux_  //usato elif e non else perch� dava problemi di scope con argv e argc
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
    // controlla se il file � gi� aperto
    if (this->FileDescLog == NULL) {
        // se non � aperto
        // apri il file in modalit� Append
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
    // se � aperto il file
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


    // TODO: spawn thread ed esegui funzione accept
    for (int q = 0; q < this->parametri.nthread; q++) {

    #ifdef _WIN32
        
        this->threadChild[q] = (int)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(Accept), NULL, 0, NULL);

    #else // linux
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

    int i = 0,newSocket = 0;
    // TODO: creare nthread ed eseguire Accept()
    while (i<3) {
        //Accept call creates a new socket for the incoming connection
        addr_size = sizeof(this->socketChild);
        // bloccante
        newSocket = accept(this->socketMaster, (struct sockaddr*)&(this->socketChild), &addr_size);


        #ifdef _WIN32

        i++;
      

        #else // linux
        
        //for each client request creates a thread and assign the client request to it to process
        //so the main thread can entertain next request
        if (pthread_create(&this->threadChild[i++], NULL, (this->Accept), &newSocket) != 0)
            printf("Failed to create thread\n");

        if (i >= this->parametri.nthread) {
            i = 0;
            while (i < this->parametri.nthread) {
                pthread_join(this->threadChild[i++], NULL);
            }
            i = 0;
        }

        #endif // _WIN32


        
    }
}


void Accept() {

    while (1) {
        // TODO: mettere in attesa il thread
    #ifdef _WIN32
            SleepConditionVariableCS(Threadwait,);


    #else //linux
        // pthread condition variable
    #endif


        // TODO: svegliare 1 thread ad una richiesta di accept

        // TODO: ottenere i dati della richiesta

        // TODO: gestire richiesta 
    }

    

}



// calloc Wrapper
void* Calloc(unsigned long int count, unsigned long int size) {
    
    if (count == 0 || size == 0) {
        fprintf(stderr, "Uno dei due numeri del Calloc � impostato a 0");
        exit(1);
        return NULL;
    }
        
    void* _t = calloc(count, size);
    if (_t == 0) {
        fprintf(stderr, "Impossibile allocare memoria");
        exit(1);
        return NULL;
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
        fprintf(stderr, "variabile data a Free() � NULL\n");
        exit(1);
        return;
    }

    memset(arg, '\0', size);
    free(arg);

}
