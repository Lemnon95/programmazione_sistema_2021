#include "share_lib_server.h"

// funzione init
void SharedLibServer(int argc, char* argv[]) {

    unsigned long long maxArg = argc;
    char* _path = (char*)Calloc(MAX_PATH + 1, sizeof(char));

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
            parametri.port = (unsigned short)strtoul(argv[argc + 1], NULL, 0);
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

            parametri.nthread = atoi(argv[argc + 1]);
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

            parametri.configPath = (char*)Calloc(sizeof(argv[argc + 1]) +1, sizeof(char));
            parametri.configPath = argv[argc + 1];

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

void CloseServer() {

    #ifdef WIN32
    DeleteCriticalSection(&FileLock);
    #else
        // TODO: distruzione pthread lock
    #endif

    if(FileDescLog != NULL)
        closeLog();

    clearSocket();
}

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

//////////////////////////////////////////////////////////////////////////////////

void spawnSockets() {
    #ifdef _WIN32
    // Win32 WinSock Init
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
    if (socketMaster <= 0) {
        clearSocket();
        ShowErr("Errore creazione socket master");
    }

    /*int _e = false;
    if (setsockopt(socketMaster, SOL_SOCKET, SO_REUSEADDR, (char*)&_e, sizeof(_e)) < 0) {
        ShowErr("impossibile impostare il socket server");
    }
    */

    // crea figli
    #ifdef _WIN32
    threadChild = (void**)Calloc(parametri.nthread, sizeof(HANDLE));
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
    #ifdef __linux__
    // spawno anche il thread che gestisce i segnali:
    if (pthread_create(&thread_handler, NULL, SigHandler, NULL) != 0)
        printf("Failed to create signal handling thread\n");
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
    if (listen(socketMaster, parametri.nthread) < 0) {
        ShowErr("Impossibile stare in ascolto sulla porta specificata");
    }

    char _t[17] = {0};
    inet_ntop(AF_INET, &masterSettings.sin_addr, _t, 17);
    printf("\nServer in ascolto su %s:%d\n", _t, masterSettings.sin_port);

}

void beginServer() {

    socklen_t addr_size;
    SOCKET newSocket = 0;

    openLog();

    #ifdef WIN32
    InitializeCriticalSection(&FileLock);
    #else
    // TODO: inizializzazione pthread lock
    #endif

    while (1) {
        //Accept call creates a new socket for the incoming connection
        addr_size = sizeof(socketChild);
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

#ifdef __linux__
    // terminazione programma (sigint) o restart (sighup)
    esci = true;
    pthread_mutex_lock(&mutex);
    pthread_cond_broadcast(&cond_var);
    pthread_mutex_unlock(&mutex);
    for (long i = 0; i < parametri.nthread; i++) {
      pthread_join(threadChild[i], NULL);
    }
#endif

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

//////////////////////////////////////////////////////////////////////////////////

void openLog() {
    // controlla se il file è già aperto
    if (FileDescLog == NULL) {
        // se non è aperto
        // apri il file in modalità Append
        #ifdef _WIN32
        fopen_s(&FileDescLog, (char*)(parametri.logPath), "a");
        #else
        FileDescLog = fopen((char*)(parametri.logPath), "a");
        #endif
        // se da errore
        if (errno) {
            ShowErr("errore nell'aprire il file");
        }
    }
}

void writeLog(unsigned long int Tpid, SOCKET soc, char* command) {
    if (FileDescLog == NULL) ShowErr("Impossibile scrivere su log, file non aperto");
#ifdef WIN32
    EnterCriticalSection(&FileLock);
#else
    // TODO: pthread enter critcial
#endif

    // timestamp
    time_t rawtime;
    struct tm timeinfo;
    char time_stamp[80];
    if (time(&rawtime) < 0) ShowErr("Impossibile ottenere il tempo della macchina");

    #ifdef WIN32
    localtime_s(&timeinfo, &rawtime);
    #else //__linux___
    localtime_r(&rawtime, &timeinfo);
    #endif
    if (errno) ShowErr("Impossibile convertire il tempo locale");

    strftime(time_stamp, sizeof(time_stamp), "%d-%m-%Y %H:%M:%S", &timeinfo);

    //ip:port
    sockaddr_in client;
    socklen_t s = sizeof(client);
    getsockname(soc, (sockaddr*)&client, &s);

    char ip[17] = { 0 };
    inet_ntop(AF_INET, &client.sin_addr, ip, 17);

    printf("%s %lu %s:%u %s\n", time_stamp, Tpid, ip, client.sin_port, command);
    fprintf(FileDescLog, "%s %lu %s:%u %s\n", time_stamp, Tpid, ip, client.sin_port, command);
    fflush(FileDescLog);


#ifdef WIN32
    LeaveCriticalSection(&FileLock);
#else
    // TODO: pthread exit critical
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
#ifdef __linux__
//funzione per il thread dedicato a gestire i segnali
void* SigHandler (void* dummy){
  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGHUP);
  sigaddset(&sigset, SIGINT);
  while (1) {
    printf("inside handler before\n");
    signum = sigwaitinfo(&sigset, NULL);
    printf("inside handler after\n");
    CloseServer();
  }
  return NULL;
}
#endif

// Dopo che un thread viene creato esegue questa funzione
void* Accept(void* rank) {
    #ifdef __linux__
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGHUP);
    sigaddset(&sigset, SIGINT);
    pthread_sigmask(SIG_BLOCK, &sigset, nullptr);
    #endif
    // inizializzo rand
    srand(time(NULL));

    SOCKET socket_descriptor;

    unsigned long int Tpid = 0;

    #ifdef WIN32
    Tpid = GetCurrentThreadId();
    #else
    // TODO: pthread thread id
    Tpid = syscall(__NR_gettid);
    #endif

    while (1) {

        // i thread vanno a dormire
        #ifdef _WIN32
        EnterCriticalSection(&CritSec);
        //printf("thread id: %lu\n", GetCurrentThreadId());
        SleepConditionVariableCS(&Threadwait, &CritSec, INFINITE);
        #else //linux
        pthread_mutex_lock(&mutex);
        while (wake_one) {
          pthread_cond_wait(&cond_var, &mutex);
          if (esci) {
            chiusura++;
            pid_t x = syscall(__NR_gettid);
            printf("Exit Thread number: %d\n", x);
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

        if (Autenticazione(socket_descriptor)){
          continue;
        }
        GestioneComandi(socket_descriptor, Tpid);

    }
#ifdef __linux__
    chiusura++;
    pid_t x = syscall(__NR_gettid);
    printf("Exit Thread number: %d\n", x);
#endif

    return NULL;
}

int Autenticazione(SOCKET socket_descriptor) {
  /*
  1. ricevere HELO
  2. generare challenge
      challenge = T_s XOR rand
  3. inviare challenge
  4. ricevere AUTH dal client
  5. risolve l'auth e risponde nel caso 200 o 400
  risposte ai comandi + log
  */
  unsigned long int T_c = 0;
  unsigned long int nonce = 0;
  unsigned long int challenge = 0;
  unsigned long int challenge_nonce = 0;

  char* endP = NULL;
  char* next_tok = NULL;

  char _t[1024] = { 0 };
  char _auth[1024] = { 0 };
  char _command[1024] = { 0 };
  // passo 1
  Recv(socket_descriptor, _t);

  if (strncmp(_t, "HELO", 4) != 0) {
      closesocket(socket_descriptor);
      return 1;
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
      return 1;
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
      return 1;
  }
  Send(socket_descriptor, "200"); // nonce accettato

  printf("\nClient Accettato\n");


  return 0;
}

void GestioneComandi(SOCKET socket_descriptor, unsigned long int Tpid) {
  char* command = (char*)Calloc(1024, sizeof(char));
  char* dup_cmd = (char*)Calloc(1024, sizeof(char));
  char* brkt = NULL;

  Recv(socket_descriptor, command);

  Strcpy(dup_cmd, 1024, command);

  command = strtok_r(command, " ", &brkt);
  if (strncmp(command, "LSF", 3) == 0) {
    command = strtok_r(NULL, " ", &brkt);
    if (LSF(socket_descriptor, command) == 0) {
        writeLog(Tpid, socket_descriptor, dup_cmd);
    }
  }
  else if (strncmp(command, "EXEC", 4) == 0) {
      command = strtok_r(NULL, "", &brkt);
      if (EXEC(socket_descriptor, command) == 0) {
          writeLog(Tpid, socket_descriptor, dup_cmd);
      }
  }
  // altri comandi


  Free(dup_cmd, strlen(dup_cmd));
}

int LSF(SOCKET socket_descriptor, char* path) {
    if (!std::filesystem::exists(path)){
        Send(socket_descriptor, "400");
        return 1;
    }
    Send(socket_descriptor, "300");
    char* records = (char*)Calloc(1, sizeof(char));
    char* buffer = NULL;
    int n = 0;

    for (auto& p : std::filesystem::directory_iterator(path, std::filesystem::directory_options::skip_permission_denied)) {

        if (p.is_directory()) continue;

        std::filesystem::path file = p.path();
        uintmax_t size = std::filesystem::file_size(file);


        #ifdef WIN32

        n = snprintf(NULL, 0, "%llu %ls\r\n", size, file.c_str()) + 1; // taglia l'ultimo carattere
        buffer = (char*)Calloc(n, sizeof(char));
        snprintf(buffer, n, "%llu %ls\r\n", size, file.c_str());

        #else
        n = asprintf(&buffer, "%lu %s\r\n", size, file.c_str())+1;
        #endif

        int q = strlen(records);
        
        if ((records = (char*)realloc(records, (q + n))) == NULL) {
            ShowErr("Impossibile allocare memoria per i file della funzione LSF");
        }

        #ifdef WIN32
        strcat_s(records, strlen(records)+n, buffer);
        if (errno) {
            ShowErr("Errore in strcat dentro LSF");
        }
        #else
        strcat(records, buffer);
        #endif

        Free(buffer, strlen(buffer));

    }

    if ((records = (char*)realloc(records, strlen(records) + sizeof(" \r\n.\r\n"))) == NULL) {
        ShowErr("Impossibile reallocare memoria");
    }

    #ifdef WIN32
    strcat_s(records, strlen(records) + sizeof(" \r\n.\r\n"), " \r\n.\r\n");
    #else
    strncat(records, " \r\n.\r\n", sizeof(" \r\n.\r\n")+1);
    #endif

    SendAll(socket_descriptor, records);


    Free(records, strlen(records)+1);
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

        Send(socket_descriptor, "400");
        return 1;

    }


    char* result = NULL;
    
    // copy
    if (strcmp(command, "copy") == 0) {
        int i = 0, n = 0;
        char* _t = NULL, *buffer = NULL;
        char** list = (char**)Calloc(1, sizeof(char*)); // lista di stringhe
        
        result = (char*)Calloc(1, sizeof(char));


        while ((_t = strtok_r(NULL, " ", &fin)) != NULL) {

            if ((list[i] = (char*)Calloc(strlen(_t) + 1, sizeof(char))) == NULL) {
                ShowErr("Impossibile allocare memoria");
            }
            Strcpy(list[i], strlen(_t)+1, _t);
            i++;
            list = (char**)realloc(list, (i+1)* sizeof(char**));
            if (list == NULL) {
                ShowErr("Errore nell'allocare lista in EXEC comando copy");
            }
        }
        // copy  || copy path1
        if (i < 2) {
            Send(socket_descriptor, "400");
            return 1;
        }

        // copy path1 path2 path3
        if (i > 2 && !std::filesystem::is_directory(list[i-1])) {
            Send(socket_descriptor, "400");
            return 1;
        }

        std::error_code err;
        std::error_condition ok;
        
        // copy path1 path2  ||  copy path1 path2 dir1
        for (int k = 0; k < i-1; k++) {
            if (std::filesystem::exists(list[k]) ){
                std::filesystem::copy(list[k], list[i - 1], std::filesystem::copy_options::skip_existing, err);
                if (err != ok) {
                    Free(list[k], strlen(list[k]));
                    continue;
                }

                #ifdef WIN32
                n = snprintf(NULL, 0, "%s\r\n", list[k]) + 1; // taglia l'ultimo carattere
                buffer = (char*)Calloc(n, sizeof(char));
                snprintf(buffer, n, "%s\r\n", list[k]);
                #else
                n = asprintf(&buffer, "%s\r\n", list[k]) + 1;
                #endif

                
                if ((result = (char*)realloc(result, (strlen(result) + n))) == NULL) {
                    ShowErr("Impossibile allocare memoria per i file della funzione LSF");
                }

                #ifdef WIN32
                strcat_s(result, strlen(result) + n, buffer);
                if (errno) {
                    ShowErr("Errore in strcat dentro EXEC");
                }
                #else
                strcat(result, buffer);
                #endif

                Free(list[k], strlen(list[k]));
                Free(buffer, strlen(buffer));
            }
        }

        result = (char*)realloc(result, strlen(result) + sizeof(" \r\n.\r\n"));

        #ifdef WIN32
        strcat_s(result, strlen(result) + sizeof(" \r\n.\r\n"), " \r\n.\r\n");
        #else
        strncat(result, " \r\n.\r\n", sizeof(" \r\n.\r\n") + 1);
        #endif


        Send(socket_descriptor, "300");
        SendAll(socket_descriptor, result);


        Free(list, i);


    }

    // remove
    if (strcmp(command, "remove") == 0) {
        int i = 0, n = 0;
        char* _t = NULL, *buffer = NULL;
        char** list = (char**)Calloc(1, sizeof(char*)); // lista di stringhe
        result = (char*)Calloc(1, sizeof(char));
        

        while ((_t = strtok_r(NULL, " ", &fin)) != NULL) {

            if ((list[i] = (char*)Calloc(strlen(_t) + 1, sizeof(char))) == NULL) {
                ShowErr("Impossibile allocare memoria");
            }
            Strcpy(list[i], strlen(_t) + 1, _t);
            i++;
            list = (char**)realloc(list, (i + 1) * sizeof(char**));
            if (list == NULL) {
                ShowErr("Errore nell'allocare lista in EXEC comando remove");
            }
        }
        
        // remove
        if (i < 1) {
            Send(socket_descriptor, "400");
            return 1;
        }


        // remove <...>
        for (int k = 0; k < i; k++) {
            if (std::filesystem::exists(list[k])) {
                if (!std::filesystem::remove(list[k])) {
                    Send(socket_descriptor, "400");
                    return 1;
                }
                

                #ifdef WIN32
                n = snprintf(NULL, 0, "%s\r\n", list[k]) + 1; // taglia l'ultimo carattere
                buffer = (char*)Calloc(n, sizeof(char));
                snprintf(buffer, n, "%s\r\n", list[k]);
                #else
                n = asprintf(&buffer, "%s\r\n", list[k]) + 1;
                #endif

                if ((result = (char*)realloc(result, (strlen(result) + n))) == NULL) {
                    ShowErr("Impossibile allocare memoria per i file della funzione LSF");
                }

                #ifdef WIN32
                strcat_s(result, strlen(result) + n, buffer);
                if (errno) {
                    ShowErr("Errore in strcat dentro EXEC");
                }
                #else
                strcat(result, buffer);
                #endif

                Free(buffer, strlen(buffer));
                Free(list[k], strlen(list[k]));

            }
        }

        result = (char*)realloc(result, strlen(result) + sizeof(" \r\n.\r\n"));

#ifdef WIN32
        strcat_s(result, strlen(result) + sizeof(" \r\n.\r\n"), " \r\n.\r\n");
#else
        strcat(result, " \r\n.\r\n");
#endif


        Send(socket_descriptor, "300");
        SendAll(socket_descriptor, result);

    }

    // printworkdir
    if (strcmp(command, "printworkdir") == 0) {
        #ifdef WIN32
        int n = snprintf(NULL, 0, "%ls\r\n", std::filesystem::current_path().c_str()+1);
        result = (char*)Calloc(n, sizeof(char));
        sprintf_s(result, n, "%ls\r\n", std::filesystem::current_path().c_str());
        #else
        asprintf(&result, "%s\r\n", std::filesystem::current_path().c_str());
        #endif

        result = (char*)realloc(result, strlen(result) + sizeof(" \r\n.\r\n"));

        #ifdef WIN32
        strcat_s(result, strlen(result) + sizeof(" \r\n.\r\n"), " \r\n.\r\n");
        #else
        strcat(result, " \r\n.\r\n");
        #endif

        Send(socket_descriptor, "300");
        SendAll(socket_descriptor, result);
    }

    
    //char* result = _exec(cmd);

    Free(result);
    return 0;
}


char* _exec(const char* cmd) {

    FILE* pipe = popen(cmd, "r");
    if (!pipe) ShowErr("popen() failed!");

    char buffer[128] = {0};
    char* result = (char*)Calloc(1, sizeof(char));

    try {
        while (fgets(buffer, sizeof(buffer)-1, pipe) != NULL) {

            result = (char*)realloc(result, strlen(result)+strlen(buffer)+1 );
            if (result == NULL) {
                ShowErr("Impossibile allocare memoria per _exec");
            }
#ifdef WIN32
            strcat_s(result, strlen(result) + strlen(buffer) + 1, buffer);
#else
            strcat(result, buffer);
#endif


        }
    }
    catch (...) {
        ;
    }
    int i = pclose(pipe);

    if (i != 0) {
        Free(result, strlen(result));

        char* _t = (char*)Calloc(100, sizeof(char));
#ifdef WIN32
        sprintf_s(_t, 100, "%d", i);
#else
        sprintf(_t, "%d", i);
#endif
        return _t;
    }

#ifdef WIN32
    strcat_s(result, strlen(result) + sizeof(" \r\n.\r\n"), " \r\n.\r\n");
#else
    strncat(result, " \r\n.\r\n", sizeof(" \r\n.\r\n") );
#endif

    return result;
}

//////////////////////////////////////////////////////////////////////////////////


void SendAll(SOCKET soc, const char* str) {
    if (str == NULL) {
        return;
    }
    
    unsigned long long size = strlen(str) + 1;
    int point = 0;
    char* buffer = (char*)Calloc(1024, sizeof(char));
    
    while ((point*1024) < size) {
        memset(buffer, '\0', 1024);
        if ((point * 1024) - size > 0) {
            memcpy(buffer, str + (point * 1024), size - (point * 1024) );
        }
        else {
            memcpy(buffer, str + (point * 1024), 1024);
        }

        
        Send(soc, buffer);
        point++;

    }
    Free(buffer, 1024);
    Send(soc, "");

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
    if (recv(soc, _return, 1024, 0) < 0) {
        ShowErr("Errore nel ricevere un messaggio dal client");
    }

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

//////////////////////////////////////////////////////////////////////////////////
// Wrapper

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
    char t[1024] = {0};
    #ifdef _WIN32
        strerror_s(t, 1023, errno);
    #else
        strerror_r(errno, t, 1023);
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

    if (size != 0) {
        memset(arg, '\0', size);
    }


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
