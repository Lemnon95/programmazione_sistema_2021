#include "share_lib_server.h"

// funzione init
void SharedLibServer(int argc, char* argv[]) {

    int maxArg = argc;
    char* _path = (char*)Calloc(MAX_PATH + 1, sizeof(char));

    // trova il percorso temp
    #ifdef _WIN32
    // instanzio una stringa di MAX_PATH caratteri, MAX_PATH è definito da Windows

    GetTempPathA(MAX_PATH, _path);
    // chiedo al sistema il percorso temporaneo, _path conterrà una cella vuota alla fine

    // concateno il testo "server.log" al path base della cartella temporanea
    strcat_s(_path, MAX_PATH, "server.log");
    if (errno) {
        ShowErr("errore appendere nome file a percorso log");
    }
    #else
    Strcpy(_path, MAX_PATH+1, "/tmp/server.log");
    #endif // _WIN32

    // parametri di default
    parametri = { 8888, 10, NULL, false, _path };

    // parsing argomenti
    while (argc > 0) {

        argc -= 1;

        // -p <port>
        if (strncmp(argv[argc], "-p", 2) == 0) {

            if (argc + 1 >= maxArg) {
                ShowErr("parametro -p incompleto");
            }

            if (argv[argc + 1][0] == '-') {
                ShowErr("parametro -p incompleto");
            }
            // conversione stringa in unsigned short
            parametri.port = (unsigned short)strtoul(argv[argc + 1], NULL, 10);
            if (errno) {
                ShowErr("il parametro di -p non risulta un numero");
            }

            if (parametri.port == 0) {
                ShowErr("porta 0 non è valida");
            }

        }

        // -n <num>
        if (strncmp(argv[argc], "-n", 2) == 0) {

            if (argc + 1 >= maxArg) {
                ShowErr("parametro -n incompleto");

            }

            if (argv[argc + 1][0] == '-') {
                ShowErr("parametro -n incompleto");
            }

            parametri.nthread = (unsigned short)strtoul(argv[argc + 1], NULL, 10);
            if (errno) {
                ShowErr("il parametro di -n non risulta un numero");
            }

            if (parametri.nthread == 0) {
                ShowErr("numero di thread invalido");
            }


        }

        // -c <path>
        if (strncmp(argv[argc], "-c", 2) == 0) {

            if (argc + 1 >= maxArg) {
                ShowErr("parametro -c incompleto");

            }

            if (argv[argc + 1][0] == '-') {
                ShowErr("parametro -c incompleto");
            }

            std::error_code err;

            if (std::filesystem::exists(argv[argc + 1], err) &&
                !std::filesystem::is_directory(argv[argc + 1], err)) {

                parametri.configPath = (char*)Calloc(strlen(argv[argc + 1]) + 1, sizeof(char));
                //parametri.configPath = argv[argc + 1];
                Strcpy(parametri.configPath, strlen(argv[argc + 1])+1, argv[argc + 1]);

            }

            if (err.value() != 0) {
                ShowErr("path config non valido");
            }

        }

        // -s
        if (strncmp(argv[argc], "-s", 2) == 0) {
            parametri.printToken = true;
        }

        // -l <path>
        if (strncmp(argv[argc], "-l", 2) == 0) {

            if (argc + 1 >= maxArg) {
                ShowErr("parametro -l incompleto");

            }

            if (argv[argc + 1][0] == '-') {
                ShowErr("parametro -l incompleto");
            }

            // non si può controllare il file se esiste poichè deve essere creato
            // controllo se la cartella parente al file esiste
            if (!std::filesystem::exists(std::filesystem::path(argv[argc + 1]).parent_path())) {
                ShowErr("Percorso inesistente per il file log");
            }

            parametri.logPath = (char*)Calloc(strlen(argv[argc + 1]) + 1, sizeof(char));
            //parametri.logPath = argv[argc + 1];
            Strcpy(parametri.logPath, strlen(argv[argc + 1])+1, argv[argc + 1]);
        }

    }

#ifdef _DEBUG
    // debug print
    printf("---\nPorta: %d\nNumero thread: %d\nConfig path: %s \nLog path: %s \nStampa token: %d\n---\n",
        parametri.port,
        parametri.nthread,
        parametri.configPath,
        parametri.logPath,
        parametri.printToken);
#endif

    // se definito leggi il config
    // sovrascrivi le impostazioni che sono defaut
    parseConfig();

#ifdef _DEBUG
    // debug print
    printf("---\nPorta: %d\nNumero thread: %d\nConfig path: %s \nLog path: %s \nStampa token: %d\n---\n",
        parametri.port,
        parametri.nthread,
        parametri.configPath,
        parametri.logPath,
        parametri.printToken);
#endif

}

void CloseServer() {

    #ifdef _WIN32
    DeleteCriticalSection(&FileLock);
    #else
    pthread_mutex_destroy(&mutex_log);
    #endif
    if (signum == 2) {
      if(FileDescLog != NULL)
          closeLog();
    }

    clearSocket();
}

void parseConfig() {

    // se definito leggi il config
    // sovrascrivi le impostazioni che sono default
    if (parametri.configPath != NULL) {
        FILE* _tConf = NULL;

        if(!Fopen(&_tConf, parametri.configPath, "r")) {
            return;
        }


        // gestione config
        /*
        \r\n windows
        \n UNIX (linux e mac os)
        \r old mac os (non più usato)
        */

        char configData[1024] = {0};
        int i = -1;

        int printTok;
        unsigned short nthread = 0;
        unsigned short port = 0;
        // legge per linea
        // \r non viene aggiunto a configData
        while (fscanf(_tConf, "%d %s", &i, configData
#ifdef _WIN32
            , 1024
#endif // fscanf_s richiede che dopo un array sia specificato la sua grandezza
        ) > 0) {

            if (configData[0] == '\0') {
                continue;
            }

            // i corrisponde al primo valore presente nel file
            // nello shwitch associo un numero al rispettivo valore
            // di parametri
            // non è ammessa la modifica del configPath (dato in input)
            /*
            0 porta
            1 nthread
            2 print token T_s
            3 log path
            */
            switch (i) 	{
                case 0:
                    port = (unsigned short)std::stoul(configData);
                    if (port > 0 && port < 65536) {
                        parametri.port = port;
                    }
                    else {
                        ShowErr("errore nel valore del config per i = 0");
                    }
                    break;
                case 1:
                    nthread = (unsigned short)std::stoul(configData);
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
                    // non si può controllare il file se esiste poichè deve essere creato
                    // controllo se la cartella parente al file esiste
                    if (!std::filesystem::path(configData).parent_path().empty()) {
                        if (!std::filesystem::exists(std::filesystem::path(configData).parent_path())) {
                            ShowErr("Percorso inesistente per il file log");
                        }
                    }
                    Strcpy(parametri.logPath, strlen(configData) + 1, configData);
                    break;
                default:
                    break;
            }
        }
        fclose(_tConf);
    }

}

//////////////////////////////////////////////////////////////////////////////////

void getToken_s() {

    T_s = generateToken();

    if (parametri.printToken) {
        printf("\ntoken: %lu\n", T_s);
    }

}

void getPassphrase(char* passphrase) {
    printf("Immetti passphrase (max 254): ");
    fgets(passphrase, 254, stdin);
}

unsigned long int generateToken() {

    char* passphrase = (char*)Calloc(256, sizeof(char));

    // ottieni l'input dell'utente
    getPassphrase(passphrase);

    // hash it
    unsigned long int k = hashToken(passphrase);

    // clear passphrase
    Free(passphrase, 256);

    return k;
}

unsigned long int hashToken(char* token) {

    int q = 5381;
    unsigned long int k = 0;
    // hashing
    // stessa phrase stresso hash
    // no fattori randomici
    // no fattori di tempo
    // bisogna basarci solo sull'input
    // e/o valori costanti
    for (int i = 0; i < strlen(token); ++i)
        k += q + int(token[i]) * (i+1);

    return k;
}

//////////////////////////////////////////////////////////////////////////////////

void spawnSockets() {
    #ifdef _WIN32
    // Win32 WinSock Init
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
    socketMaster = socket(PF_INET, SOCK_STREAM, 0);
    if (socketMaster <= 0) {
        clearSocket();
        ShowErr("Errore creazione socket master");
    }

    // crea figli
    #ifdef _WIN32
    threadChild = (HANDLE*)Calloc(parametri.nthread, sizeof(HANDLE));
    InitializeConditionVariable(&Threadwait);
    InitializeCriticalSection(&CritSec);
    #else
    threadChild = (pthread_t*)Calloc(parametri.nthread, sizeof(pthread_t));
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_var, NULL);
    #endif
    // instanzio i thread nella lista
    for (int q = 0; q < parametri.nthread; q++) {
        #ifdef _WIN32
        if ((threadChild[q] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(Accept), (LPVOID)(T_s), 0, NULL)) == NULL) {
            ShowErr("Impossibile creare un thread");
        }
        #else // linux
        if (pthread_create(&threadChild[(long)q], NULL, Accept, (void*)T_s) != 0)
            printf("Failed to create thread\n");
        #endif
    }

    #ifdef __linux__
    // spawno 1 thread che gestisce i segnali:
    if (pthread_create(&thread_handler, NULL, SigHandler, NULL) != 0)
        printf("Failed to create signal handling thread\n");
    //inizializzazione mutex per file di log
    pthread_mutex_init(&mutex_log, NULL);
    #else //_WIN32
    InitializeCriticalSection(&FileLock);
    #endif


    // impostazioni base del server
    sockaddr_in masterSettings;
    memset(masterSettings.sin_zero, '\0', sizeof(masterSettings.sin_zero));
    masterSettings.sin_family = AF_INET; // protocollo IP
    masterSettings.sin_port = parametri.port; // Server port
    if (!inet_pton(AF_INET, SERVERLISTEN, &masterSettings.sin_addr)) { // server IP
        ShowErr("non risulta un ip valido");
    }


    // apre il server
    if (bind(socketMaster, (sockaddr*)&masterSettings, sizeof(masterSettings)) < 0) {
        ShowErr("Impossibile aprire il socket del server");
    }
    if (listen(socketMaster, parametri.nthread*4) < 0) {
        ShowErr("Impossibile stare in ascolto sulla porta specificata");
    }

    char _t[17] = {0};
    inet_ntop(AF_INET, &masterSettings.sin_addr, _t, 17);
    printf("\nServer in ascolto su %s:%d\n", _t, masterSettings.sin_port);

}

void beginServer() {

    socklen_t addr_size;
    SOCKET newSocket = 0;
    bool uscita = false;

    openLog();

    // Main Thread Loop
    while (!uscita) {
      while (1) {
          // accept() crea un nuovo file-descriptor per ogni client in entrata
          addr_size = sizeof(socketChild);
          newSocket = accept(socketMaster, (sockaddr*)&(socketChild), &addr_size);
#ifdef _DEBUG
          printf ("Sono dopo accept - newSock:%lu\n", newSocket);
#endif
          if (newSocket == -1) break;

          //ci serve una sezione critica per la variabile size della coda
#ifdef _WIN32
          EnterCriticalSection(&CritSec);
          Enqueue(newSocket, &front, &rear); //inserisco nella coda il nuovo socket descriptor
          wake_one = false;
          WakeConditionVariable(&Threadwait);
          LeaveCriticalSection(&CritSec);
#else // linux
          pthread_mutex_lock(&mutex);
          Enqueue(newSocket, &front, &rear); //inserisco nella coda il nuovo socket descriptor
          wake_one = false;
          pthread_cond_signal(&cond_var);	//sveglio un thread per gestire la nuova connessione
          pthread_mutex_unlock(&mutex);
#endif // _WIN32

      }

  #ifdef __linux__
      // terminazione programma (sigint) o restart (sighup)
      pthread_mutex_lock(&mutex);
      esci = 1;
      pthread_cond_broadcast(&cond_var);
      pthread_mutex_unlock(&mutex);
      for (long i = 0; i < parametri.nthread; i++) {
        pthread_join(threadChild[i], NULL);
      }
      pthread_join(thread_handler, NULL);
      pthread_mutex_destroy(&mutex);
      pthread_cond_destroy(&cond_var);
#else // _WIN32
      EnterCriticalSection(&CritSec);
      esci = 1;
      WakeAllConditionVariable(&Threadwait);
      LeaveCriticalSection(&CritSec);
      // thread join
      WaitForMultipleObjects(parametri.nthread, threadChild, TRUE, INFINITE);

      // no destroy per conditionvariable e criticalsection

#endif
      Free(threadChild);
      if (signum == 2) {
        uscita = true;
      }
      else {
        printf("Riavvio del Server\n");
        esci = 0;
        parseConfig();
        spawnSockets();
      }
  }
}

void clearSocket() {
    // se instanziato, chiudi il socket
    if (socketMaster != 0) {
#ifdef _DEBUG
        printf("sto chiudendo socketmaster\n");
#endif // _DEBUG


#ifdef _WIN32
        shutdown(socketMaster, SD_RECEIVE);
#else
        shutdown(socketMaster, SHUT_RD);
#endif
        closesocket(socketMaster);
    }

#ifdef _WIN32
    WSACleanup();
#endif
}

//////////////////////////////////////////////////////////////////////////////////

void openLog() {
    // controlla se il file è già aperto
    if (FileDescLog == NULL) {
        // se non è aperto
        // apri il file in modalità Append

        Fopen(&FileDescLog, parametri.logPath, "a");
    }
}

void writeLog(unsigned long int Tpid, SOCKET soc, char* command) {
    if (FileDescLog == NULL) ShowErr("Impossibile scrivere su log, file non aperto");
#ifdef _WIN32
    EnterCriticalSection(&FileLock);
#else
    pthread_mutex_lock(&mutex_log);
#endif

    // timestamp
    time_t rawtime;
    struct tm timeinfo;
    char time_stamp[80];
    if (time(&rawtime) < 0) ShowErr("Impossibile ottenere il tempo della macchina");

    errno = 0;
    #ifdef _WIN32
    localtime_s(&timeinfo, &rawtime);
    if (errno) ShowErr("Impossibile convertire il tempo locale");
    #else //__linux___
    if (localtime_r(&rawtime, &timeinfo) == NULL) ShowErr("Impossibile convertire il tempo locale");
    #endif

    strftime(time_stamp, sizeof(time_stamp), "%d-%m-%Y %H:%M:%S", &timeinfo);

    //ip:port
    sockaddr_in client;
    socklen_t s = sizeof(client);
    getpeername(soc, (sockaddr*)&client, &s);

    char ip[17] = { 0 };
    inet_ntop(AF_INET, &client.sin_addr, ip, 17);

    printf("%s %lu %s:%u %s\n", time_stamp, Tpid, ip, client.sin_port, command);
    fprintf(FileDescLog, "%s %lu %s:%u %s\n", time_stamp, Tpid, ip, client.sin_port, command);
    fflush(FileDescLog);


#ifdef _WIN32
    LeaveCriticalSection(&FileLock);
#else
    pthread_mutex_unlock(&mutex_log);
#endif

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

//////////////////////////////////////////////////////////////////////////////////

//funzione per il thread dedicato a gestire i segnali
#ifdef __linux__
void* SigHandler(void* dummy) {
  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGHUP);
  sigaddset(&sigset, SIGINT);
#ifdef _DEBUG
  printf("inside handler before\n");
#endif
  signum = sigwaitinfo(&sigset, NULL);
#ifdef _DEBUG
  printf("inside handler after, signum: %d\n", signum);
#endif
  CloseServer();
  return NULL;
}
#else
BOOL WINAPI CtrlHandler(DWORD signal) {

    switch (signal) {
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
    case CTRL_C_EVENT:
        signum = 2;
        CloseServer();
        return TRUE;
    default:
        break;
    }

    return FALSE;
}
#endif

// Dopo che un thread viene creato esegue questa funzione
void* Accept(void* rank) {
    // inizializzo rand
    srand(time(NULL));

    SOCKET socket_descriptor;
    size_t Tpid = 0;
    // ottendo il thread id
    #ifdef _WIN32
    Tpid = GetCurrentThreadId();
    #else
    Tpid = syscall(__NR_gettid);
    #endif

    while (!esci) {
        // i thread vanno a dormire
#ifdef _WIN32
        EnterCriticalSection(&CritSec);
#else
        pthread_mutex_lock(&mutex);
#endif
        if (size == 0) {

            while (wake_one) {

#ifdef _WIN32
                SleepConditionVariableCS(&Threadwait, &CritSec, INFINITE);
#else
                pthread_cond_wait(&cond_var, &mutex);
#endif
                    if (esci) {
#ifdef _WIN32
                        LeaveCriticalSection(&CritSec);
#else
                        pthread_mutex_unlock(&mutex);
#endif
                        return NULL;
                    }
            }
            wake_one = true;

        }
        if (esci) {
#ifdef _WIN32
            LeaveCriticalSection(&CritSec);
#else
            pthread_mutex_unlock(&mutex);
#endif
            return NULL;
        }

        // sezione critica

        Dequeue(&socket_descriptor, &front, &rear);
        #ifdef _WIN32
        LeaveCriticalSection(&CritSec);
        #else //linux
        pthread_mutex_unlock(&mutex);
        #endif
        // sezione parallela

        if (Autenticazione(socket_descriptor)){
          continue;
        }

        GestioneComandi(socket_descriptor, Tpid);
        closesocket(socket_descriptor);
    }
    

    return NULL;
}

int Autenticazione(SOCKET socket_descriptor) {
    /*
    1. ricevere HELO
    2. generare challenge
        challenge = T_s XOR rand
    3. inviare status e challenge
    4. ricevere AUTH dal client
    5. risolve l'auth e risponde nel caso 200 o 400
    risposte ai comandi + log
    */

    size_t T_c = 0;
    size_t nonce = 0;
    size_t challenge = 0;
    size_t challenge_nonce = 0;


    char helo[5] = { 0 }; // ricevo HELO

    char* chall = NULL; // spedisco la challenge
    int _challLen = 0; // lunghezza challenge

    char* auth = (char*)Calloc(1024, sizeof(char));
    int _authLen = 0;


    // passo 1 ////////////////////////////////////////////////////
    Recv(socket_descriptor, helo, 5); // ricevo HELO
    if (strncmp(helo, "HELO", 4) != 0) {
        closesocket(socket_descriptor);
        return 1;
    }
    ///////////////////////////////////////////////////////////////

    // passo 2,3 //////////////////////////////////////////////////
    nonce = rand() % 2147483647; // nonce
    challenge = T_s ^ nonce; // T_s XOR nonce | genero challenge

    _challLen = Asprintf(chall, "%lu", challenge); // unsigned long to string

    Send(socket_descriptor, "300", 4); // invio status
    Send(socket_descriptor, chall, _challLen); // invio challenge
    Free(chall, _challLen);
    ///////////////////////////////////////////////////////////////

    // passo 4 ////////////////////////////////////////////////////
    if ((_authLen = Recv(socket_descriptor, auth, 1024)) <= 0) {
        closesocket(socket_descriptor);
        return 1;
    }
    ///////////////////////////////////////////////////////////////

    // passo 5 ////////////////////////////////////////////////////
    // AUTH
    size_t enc1, enc2;

#ifdef _WIN32
    if (sscanf_s(auth, "AUTH %llu;%llu", &enc1, &enc2) != 2) {
        Send(socket_descriptor, "400", 4);
        closesocket(socket_descriptor);
        return 1;
    }
#else
    if (sscanf(auth, "AUTH %lu;%lu", &enc1, &enc2) != 2) {
        Send(socket_descriptor, "400", 4);
        closesocket(socket_descriptor);
        return 1;
    }
#endif

    // enc1
    // T_s XOR nonce XOR T_c
    T_c = T_s ^ nonce ^ enc1;

    // enc2
    // T_c XOR nonce
    challenge_nonce = T_c ^ enc2;

    Free(auth, _authLen);
    if (challenge_nonce != nonce) {
        // challenge rifiutata
        Send(socket_descriptor, "400", 4);
        closesocket(socket_descriptor);
#ifdef _DEBUG
        printf("\nClient Rifiutato\n");
#endif
        return 1;
    }
    Send(socket_descriptor, "200", 4); // challenge accettata

#ifdef _DEBUG
    printf("\nClient Accettato\n");
#endif

    return 0;
}

void GestioneComandi(SOCKET socket_descriptor, unsigned long int Tpid) {
  char* command = (char*)Calloc(1024, sizeof(char));
  int _commandLen = 1024;
  char* dup_cmd = NULL;
  char* brkt = NULL;
  char* cmd = NULL;

  while ((_commandLen = Recv(socket_descriptor, command, _commandLen)) > 0) {
      brkt = NULL;
      cmd = NULL;
      if (strlen(command) == 0) {
          // parametro di sicurezza per evitare seg fault
#ifdef _DEBUG
          printf("\n!err!\n");
#endif
          break;
      }

      // alloco e duplico il comando ricevuto
      dup_cmd = (char*)Calloc(_commandLen, sizeof(char));
      Strcpy(dup_cmd, _commandLen, command);

      cmd = strtok_r(command, " ", &brkt);
      if (strncmp(cmd, "LSF", 3) == 0) {
          if (LSF(socket_descriptor, brkt) == 0) {
              writeLog(Tpid, socket_descriptor, dup_cmd);
          }
      }
      if (strncmp(cmd, "EXEC", 4) == 0) {
          if (EXEC(socket_descriptor, brkt) == 0) {
              writeLog(Tpid, socket_descriptor, dup_cmd);
          }
      }
      if (strncmp(cmd, "DOWNLOAD", 8) == 0) {
          if (DOWNLOAD(socket_descriptor, brkt) == 0) {
              writeLog(Tpid, socket_descriptor, dup_cmd);
          }
      }
      if (strncmp(cmd, "SIZE", 4) == 0) {
          if (SIZE_(socket_descriptor, brkt) == 0) {
              writeLog(Tpid, socket_descriptor, dup_cmd);
          }
      }
      if (strncmp(cmd, "UPLOAD", 6) == 0) {
          if (UPLOAD(socket_descriptor, brkt) == 0) {
              writeLog(Tpid, socket_descriptor, dup_cmd);
          }
      }
      // altri comandi

      memset(command, '\0', _commandLen);

      command = (char*)Realloc(command, 1024);
      Free(dup_cmd, _commandLen);
  }

  Free(command);
}

int LSF(SOCKET socket_descriptor, char* path) {
    if (!std::filesystem::exists(path)){
        Send(socket_descriptor, "400", 4);
        return 1;
    }
    Send(socket_descriptor, "300", 4);
    char* records = (char*)Calloc(1, sizeof(char));
    int _recordsLen = 0;
    char* buffer = NULL;
    int _bufferLen = 0;

    std::error_code err;

    for (auto& p : std::filesystem::directory_iterator(path, std::filesystem::directory_options::skip_permission_denied)) {
        buffer = NULL;

        if (p.is_directory()) continue;

        std::filesystem::path file = p.path();
        size_t size = std::filesystem::file_size(file, err);
        if (err.value() != 0) continue;

        // NON rimpiazzare con il wrapper Asprintf
        /*
        BUG:
            il primo valore di argptr quando viene eseguito -l . (lista dei file nella dir)
            il primo valore conterrebbe il size del file, esso è corrotto
        */
#ifdef _WIN32
        _bufferLen = snprintf(NULL, 0, "%llu %ls\r\n", size, file.c_str()) + 1;
        buffer = (char*)Calloc(_bufferLen, sizeof(char));
        sprintf_s(buffer, _bufferLen, "%llu %ls\r\n", size, file.c_str());
#else
        _bufferLen = asprintf(&buffer, "%llu %s\r\n", size, &(file.c_str())[0])+1;
#endif

        records = (char*)Realloc(records, (_recordsLen + _bufferLen));
        errno = 0;
        #ifdef _WIN32
        strcat_s(records, _recordsLen + _bufferLen, buffer);
        if (errno) ShowErr("Errore in strcat dentro LSF");
        #else
        strcat(records, buffer);
        #endif

        _recordsLen += _bufferLen;
        Free(buffer, _bufferLen);

    }

    _recordsLen += sizeof(" \r\n.\r\n");

    records = (char*)Realloc(records, _recordsLen);

    #ifdef _WIN32
    strcat_s(records, _recordsLen, " \r\n.\r\n");
    #else
    strncat(records, " \r\n.\r\n", sizeof(" \r\n.\r\n"));
    #endif

    SendAll(socket_descriptor, records, _recordsLen);


    Free(records, _recordsLen);
    return 0;
}

int EXEC(SOCKET socket_descriptor, char* cmd) {

    char* command = NULL;
    char* fin = NULL;
    command = strtok_r(cmd, " ", &fin);

    if (strcmp(command, "copy") &&
        strcmp(command, "remove") &&
        strcmp(command, "whoami") &&
        strcmp(command, "printworkdir") &&
        strcmp(command, "sort")) {

        Send(socket_descriptor, "400", 4);
        return 1;

    }


    char* result = NULL;

    // copy
    if (strcmp(command, "copy") == 0) {
        int i = 0;
        char* _t = NULL;
        char** list = (char**)Calloc(1, sizeof(char*)); // lista di stringhe

        result = (char*)Calloc(1, sizeof(char));
        int _resultLen = 1;

        // ottengo i parametri del comando copy e li metto nella variabile list
        while ((_t = strtok_r(NULL, " ", &fin)) != NULL) {

            list[i] = (char*)Calloc(strlen(_t) + 1, sizeof(char));

            Strcpy(list[i], strlen(_t)+1, _t);
            i++;
            list = (char**)Realloc(list, (i+1)* sizeof(char**));
            
        }

        // copy  || copy path1
        if (i < 2) {
            for (int k = 0; k < i; k++) {
                Free(list[k]);
            }
            Free(list);
            Send(socket_descriptor, "400", 4);
            return 1;
        }

        // copy path1 path2 path3
        if (i > 2 && !std::filesystem::is_directory(list[i-1])) {
            for (int k = 0; k < i; k++) {
                Free(list[k]);
            }
            Free(list);
            Send(socket_descriptor, "400", 4);
            return 1;
        }


        std::error_code err;

        // copy path1 path2  ||  copy path1 path2 dir1
        for (int k = 0; k < i-1; k++) {
            if (std::filesystem::exists(list[k]) ){
                // effettua la copia del file
                std::filesystem::copy(list[k], list[i - 1], err);
                if (err.value() != 0) {
                    // se la copia va male passa al successivo
                    Free(list[k], strlen(list[k]));
                    continue;
                }
                // il free alla fine riempie il buffer, ridichiaro il buffer
                char*buffer = NULL;
                // componi la stringa
                int _bufferLen = Asprintf(buffer, "%s\r\n", list[k]);

                result = (char*)Realloc(result, (_resultLen + _bufferLen));

                _resultLen += _bufferLen;
                errno = 0;
                #ifdef _WIN32
                strcat_s(result, _resultLen, buffer);
                #else
                strcat(result, buffer);
                #endif
                if (errno) {
                    ShowErr("Errore in strcat dentro EXEC");
                }

                Free(list[k], strlen(list[k]));
                Free(buffer, _bufferLen);
            }
        }

        _resultLen += sizeof(" \r\n.\r\n");

        result = (char*)Realloc(result, _resultLen);

        #ifdef _WIN32
        strcat_s(result, _resultLen, " \r\n.\r\n");
        #else
        strncat(result, " \r\n.\r\n", sizeof(" \r\n.\r\n"));
        #endif


        Send(socket_descriptor, "300", 4);
        SendAll(socket_descriptor, result, _resultLen);

        Free(result, _resultLen);
        Free(list, i);
    }

    // remove
    if (strcmp(command, "remove") == 0) {
        int i = 0;
        char* _t = NULL, *buffer = NULL;
        char** list = (char**)Calloc(1, sizeof(char*)); // lista di stringhe
        result = (char*)Calloc(1, sizeof(char));


        while ((_t = strtok_r(NULL, " ", &fin)) != NULL) {

            if ((list[i] = (char*)Calloc(strlen(_t) + 1, sizeof(char))) == NULL) {
                ShowErr("Impossibile allocare memoria");
            }
            Strcpy(list[i], strlen(_t) + 1, _t);
            i++;
            list = (char**)Realloc(list, (i + 1) * sizeof(char**));
        }

        // remove
        if (i < 1) {
            Free(list);
            Send(socket_descriptor, "400",4);
            return 1;
        }

        std::error_code err;

        // remove <...>
        for (int k = 0; k < i; k++) {
            if (std::filesystem::exists(list[k])) {
                if (!std::filesystem::remove(list[k], err)) {
                    // se la cancellazione fallisce passa al successivo
                    Free(list[k]);
                    continue;
                }

#ifdef _DEBUG
                printf("error code remove %d", err.value());
#endif // _DEBUG

                // il free alla fine riempie il buffer, ridichiaro il buffer
                char* buffer = NULL;
                int _bufferLen = Asprintf(buffer, "%s\r\n", list[k]);

                result = (char*)Realloc(result, (strlen(result) + _bufferLen));

                errno = 0;
                #ifdef _WIN32
                strcat_s(result, strlen(result) + _bufferLen, buffer);
                #else
                strcat(result, buffer);
                #endif
                if (errno) {
                    ShowErr("Errore in strcat dentro EXEC");
                }

                Free(buffer, _bufferLen);
                Free(list[k], strlen(list[k]));

            }
        }

        result = (char*)Realloc(result, strlen(result) + sizeof(" \r\n.\r\n"));
        


#ifdef _WIN32
        strcat_s(result, strlen(result) + sizeof(" \r\n.\r\n"), " \r\n.\r\n");
#else
        strcat(result, " \r\n.\r\n");
#endif


        Send(socket_descriptor, "300",4);
        SendAll(socket_descriptor, result, strlen(result));
        Free(list);
    }

    // whoami
    if (strcmp(command, "whoami") == 0) {
#ifdef _WIN32
        unsigned long size = UNLEN + 1;
        result = (char*)Calloc(size, sizeof(char));
        if (!GetUserNameA(result, &size)) {
            //ShowErr("Impossibile ottenere l'username di whoami");
            Send(socket_descriptor, "400", 4);
            return 1;
        }
#else
        result = (char*)Calloc(257, sizeof(char));
        passwd* user = getpwuid(getuid());
        if(user == NULL) {
            //ShowErr("Impossibile ottenere l'username di whoami");
            Send(socket_descriptor, "400", 4);
            return 1;
        }
        strcat(result, user->pw_name);
#endif

        result = (char*)Realloc(result, strlen(result) + sizeof(" \r\n.\r\n"));
        
#ifdef _WIN32
        strcat_s(result, strlen(result) + sizeof(" \r\n.\r\n"), " \r\n.\r\n");
#else
        strcat(result, " \r\n.\r\n");
#endif

        Send(socket_descriptor, "300",4);
        SendAll(socket_descriptor, result, strlen(result));

        Free(result);
    }

    // printworkdir
    if (strcmp(command, "printworkdir") == 0) {

        std::error_code err;

#ifdef _WIN32
        int _resultLen = Asprintf(result, "%ls\r\n", std::filesystem::current_path(err).c_str());
#else
        int _resultLen = Asprintf(result, "%s\r\n", std::filesystem::current_path(err).c_str());
#endif
        if (err.value() != 0) {
            Send(socket_descriptor, "400", 4);
            return 1;
        }

        result = (char*)Realloc(result, _resultLen + sizeof(" \r\n.\r\n"));

        #ifdef _WIN32
        strcat_s(result, _resultLen + sizeof(" \r\n.\r\n"), " \r\n.\r\n");
        #else
        strcat(result, " \r\n.\r\n");
        #endif

        _resultLen += sizeof(" \r\n.\r\n");

        Send(socket_descriptor, "300",4);
        SendAll(socket_descriptor, result, _resultLen);

        Free(result, _resultLen);
    }

    // sort
    if (strcmp(command, "sort") == 0) {
        char* _t = NULL, *buffer = NULL;
        int _bufferLen = 0;
        int _resultLen = 1;

        // sort
        if ((_t = strtok_r(NULL, " ", &fin)) == NULL) {
            Send(socket_descriptor, "400",4);
            return 1;
        }

        
        _bufferLen = Asprintf(buffer, "sort %s", _t);


        int code = _exec(buffer, result, _resultLen);

        if (code != 0) {
            if (result != NULL) {
                Free(result, _resultLen);
                char* result;
                
            }
            result = NULL;
            _resultLen = Asprintf(result, "%d", code);
        }
        result = (char*)Realloc(result, _resultLen+sizeof(" \r\n.\r\n"));
#ifdef _WIN32
        strcat_s(result, _resultLen + sizeof(" \r\n.\r\n"), " \r\n.\r\n");
#else
        strncat(result, " \r\n.\r\n", sizeof(" \r\n.\r\n"));
#endif
        _resultLen += sizeof(" \r\n.\r\n");

        Send(socket_descriptor, "300",4);
        SendAll(socket_descriptor, result, strlen(result));
        Free(buffer, _bufferLen);
        Free(result, _resultLen);
    }


    
    return 0;
}

int DOWNLOAD(SOCKET socket_descriptor, char* cmd) {
    char* path = NULL, * size = NULL, *Pend = NULL;
    char* endP = NULL;

    std::error_code err;

    if ((path = strtok_r(cmd, ";", &Pend)) == NULL) {
        Send(socket_descriptor, "400",4);
        return 1;
    }
    if ((size = strtok_r(NULL, ";", &Pend)) == NULL) {
        Send(socket_descriptor, "400",4);
        return 1;
    }
    if (std::filesystem::exists(path, err)) {
        Send(socket_descriptor, "400",4);
        return 1;
    }
    FILE* _f;

    if(!Fopen(&_f, path, "wb")) {
        Send(socket_descriptor, "400", 4);
        return 1;
    }

    size_t sizeI = strtoull(size, &endP, 10);
    Send(socket_descriptor, "300", 4);

    RecvWriteF(socket_descriptor, _f, sizeI);

    fclose(_f);

    return 0;
}

int SIZE_(SOCKET socket_descriptor, char* path, bool end) {

    std::error_code err;

    if (!std::filesystem::exists(path, err)) {
        Send(socket_descriptor, "400",4);
        return 1;
    }

    if (err.value() != 0) {
        Send(socket_descriptor, "400", 4);
        return 1;
    }

    err.clear();

    char* buffer = NULL;
    int _bufferLen = 0;
    size_t size = std::filesystem::file_size(path, err);
    if (err.value() != 0) {
        Send(socket_descriptor, "400",4);
        return 1;
    }

    _bufferLen = Asprintf(buffer, "%llu\r\n", size);

    Send(socket_descriptor, "300",4);
    Send(socket_descriptor, buffer, _bufferLen);

    Free(buffer, _bufferLen);

    return 0;
}

int UPLOAD(SOCKET socket_descriptor, char* cmd) {
    char* path = NULL, * size = NULL, * Pend = NULL;
    char* PendLU = NULL, *buffer = NULL;
    FILE* _f = NULL;
    size_t sizeI;
    std::error_code err;

    if ((path = strtok_r(cmd, ";", &Pend)) == NULL) {
        Send(socket_descriptor, "400",4);
        return 1;
    }
    if ((size = strtok_r(NULL, ";", &Pend)) == NULL) {
        Send(socket_descriptor, "400",4);
        return 1;
    }

    sizeI = strtoull(size, &PendLU, 10);

    if (!std::filesystem::exists(path,err)) {
        Send(socket_descriptor, "400",4);
        return 1;
    }
    err.clear();
    size_t Fsize = std::filesystem::file_size(path,err);
    if (sizeI > Fsize) {
        Send(socket_descriptor, "400",4);
        return 1;
    }

    if(!Fopen(&_f, path, "rb")) {
        Send(socket_descriptor, "400", 4);
        return 1;
    }

    Send(socket_descriptor, "300", 4);
    SendReadF(socket_descriptor, _f, sizeI);

    fclose(_f);

    return 0;
}

int _exec(const char* cmd, char*& result, int &_resultLen) {

    FILE* pipe = popen(cmd, "r");
    if (!pipe) ShowErr("popen() failed!");

    char buffer[128] = {0};
    result = (char*)Calloc(1, sizeof(char));

    try {
        while (fgets(buffer, sizeof(buffer)-1, pipe) != NULL) {

            result = (char*)Realloc(result, _resultLen +strlen(buffer) );
            
#ifdef _WIN32
            strcat_s(result, _resultLen + strlen(buffer), buffer);
#else
            strcat(result, buffer);
#endif

            _resultLen += strlen(buffer);

        }
    }
    catch (...) {
        ;
    }
    int i = pclose(pipe);

    return i;
}

//////////////////////////////////////////////////////////////////////////////////
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
    if (*front == NULL) {
        *rear = *front;
    }


    size--;
    free(temp);
    return 0;
}
