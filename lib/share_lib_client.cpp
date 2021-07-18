
#include "share_lib_client.h"


SharedLibClient::SharedLibClient(int argc, char* argv[]) {

    this->parametri = { {AF_INET, 8888, {0}}, NULL, NULL, {NULL, NULL}, {NULL,NULL} };

    // parsing argomenti
    unsigned long long maxArg = argc;
    while (argc > 0) {

        argc -= 1;

        // -h <ip>
        if (strcmp(argv[argc], "-h") == 0) {

            if (argc + 1 >= maxArg) {
                ShowErr("parametro -h incompleto");

            }

            if (argv[argc + 1][0] == '-') {
                ShowErr("parametro -h incompleto");

            }


            if (!inet_pton(AF_INET, argv[argc + 1], &this->parametri.server.sin_addr)) {
                ShowErr("il parametro di -h non risulta un ip valido");

            }

        }

        // -p <num>
        if (strcmp(argv[argc], "-p") == 0) {

            if (argc + 1 >= maxArg) {
                ShowErr("parametro -p incompleto");

            }

            if (argv[argc + 1][0] == '-') {
                ShowErr("parametro -p incompleto");
            }

            this->parametri.server.sin_port = atoi(argv[argc + 1]);
            if (errno) {
                ShowErr("il parametro di -p non risulta un numero");
            }

            if (this->parametri.server.sin_port == 0) {
                ShowErr("numero di porta invalido");
            }


        }

        // -l <path>
        if (strcmp(argv[argc], "-l") == 0) {

            if (argc + 1 >= maxArg) {
                ShowErr("parametro -l incompleto");
            }

            if (argv[argc + 1][0] == '-') {
                ShowErr("parametro -l incompleto");
            }

            this->parametri.lsf = (char*)Calloc(sizeof(argv[argc + 1]) + 1, sizeof(char));
            this->parametri.lsf = argv[argc + 1];

        }

        // -e "<cmd> [<args>...]"
        if (strcmp(argv[argc], "-e") == 0) {

            if (argc + 1 >= maxArg) {
                ShowErr("parametro -e incompleto");
            }

            if (argv[argc + 1][0] == '-') {
                ShowErr("parametro -e incompleto");
            }



            this->parametri.exec = (char*)Calloc(sizeof(argv[argc + 1]) + 1, sizeof(char));
            this->parametri.exec = argv[argc + 1];

        }

        // -d <src> <size>
        if (strcmp(argv[argc], "-d") == 0) {

            if (argc + 1 >= maxArg) {
                ShowErr("parametro -d incompleto");
            }

            if (argc + 2 >= maxArg) {
                ShowErr("parametro -d incompleto");
            }

            if (argv[argc + 1][0] == '-') {
                ShowErr("parametro -d incompleto");
            }

            if (argv[argc + 2][0] == '-') {
                ShowErr("parametro -d incompleto");
            }

            this->parametri.download.src = (char*)Calloc(sizeof(argv[argc + 1]) + 1, sizeof(char));
            this->parametri.download.src = argv[argc + 1];

            //this->parametri.download.size = (char*)Calloc(sizeof(argv[argc + 2]) + 1, sizeof(char));
            char* Pend = NULL;
            this->parametri.download.size = strtoull(argv[argc + 2], &Pend, 10);

        }

        // -u <src> <dest>
        if (strcmp(argv[argc], "-u") == 0) {

            if (argc + 1 >= maxArg) {
                ShowErr("parametro -u incompleto");
            }

            if (argc + 2 >= maxArg) {
                ShowErr("parametro -u incompleto");
            }

            if (argv[argc + 1][0] == '-') {
                ShowErr("parametro -u incompleto");
            }

            if (argv[argc + 2][0] == '-') {
                ShowErr("parametro -u incompleto");
            }

            this->parametri.upload.src = (char*)Calloc(sizeof(argv[argc + 1]) + 1, sizeof(char));
            this->parametri.upload.src = argv[argc + 1];

            //this->parametri.upload.size = (char*)Calloc(sizeof(argv[argc + 2]) + 1, sizeof(char));
            char* Pend = NULL;
            this->parametri.upload.size = strtoull(argv[argc + 2], &Pend, 10);

        }
    }

    if (this->parametri.server.sin_addr.s_addr == 0) {
        ShowErr("indirizzo ip mancante (-h)");
    }


    printf("\nserver: %d %d\nlsf: %s\nexec: %s\ndownload: %s %llu\nupload: %s %llu\n",
        this->parametri.server.sin_addr.s_addr,
        this->parametri.server.sin_port,
        this->parametri.lsf,
        this->parametri.exec,
        this->parametri.download.src,
        this->parametri.download.size,
        this->parametri.upload.src,
        this->parametri.upload.size
    );



    this->T_s = this->generateToken("Immetti passphrase del server (max 254): ");

    this->T_c = this->generateToken("Immetti passphrase del client (max 254): ");

#ifdef _DEBUG
    printf("\nT_s: %lu\nT_c: %lu\n", this->T_s, this->T_c);
#endif // _DEBUG

    

}

SharedLibClient::~SharedLibClient() {

    this->clearSocket();

}

void SharedLibClient::Connect() {

#ifdef _WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        ShowErr("impossibile avviare WinSock");
    }
#endif

    this->socketClient = socket(PF_INET, SOCK_STREAM, 0);
    if (this->socketClient < 0) {
        this->clearSocket();
        ShowErr("Errore creazione socket master");
    }


    if (connect(this->socketClient, (sockaddr*)&(this->parametri.server), sizeof(this->parametri.server)) < 0) {
        this->clearSocket();
        ShowErr("Impossibile connettersi al server");
    }

    // effettua la connessione ed autenticazione
    this->Trasmissione();
    /*
    TODO: inviare e gestire comandi passati come args
    */
    this->GestioneComandi();

    closesocket(this->socketClient);
}

void SharedLibClient::getPassphrase(const char* printText, char* passphrase) {
    printf("%s",printText);
    fgets(passphrase, 254, stdin);
}

unsigned long int SharedLibClient::generateToken(const char* printText) {

    char* passphrase = (char*)Calloc(256, sizeof(char));

    this->getPassphrase(printText,passphrase);

    unsigned long int k = this->hashToken(passphrase);

    // reset passphrase
    Free(passphrase, 256);

    return k;
}

unsigned long int SharedLibClient::hashToken(char* token) {
    
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

void SharedLibClient::Send(const char* str) {

    if (str == NULL) {
        return;
    }

    if (send(this->socketClient, str, 1023, 0) < 0) {
        ShowErr("Errore nell'inviare un messaggio verso il server");
    }
}

void SharedLibClient::Recv(char* _return) {
    memset(_return, '\0', 1024);
    if (recv(this->socketClient, _return, 1023, 0) < 0) {
        ShowErr("Errore nel ricevere un messaggio dal server");
    }

}

void SharedLibClient::Send_Recv(char* _return, const char* str, char* status) {

    this->Send(str);

    if (status != NULL) {
        this->Recv(status);
    }

    this->Recv(_return);
}

void SharedLibClient::Trasmissione() {

    unsigned long int challenge = 0;
    unsigned long int enc1 = 0;
    unsigned long int enc2 = 0;
    unsigned long int nonce = 0;

    char* endP;
    char status[1024] = { 0 };
    char _t[1024] = { 0 };
    char authmsg[1024] = { 0 };

    /*
    1. invio HELO
    2. ricevo la challenge
    3. risolvo la challenge
    4. compongo AUTH
    5. invio AUTH
    6. ottengo il risultato
    comandi ulteriori
    */

    // passo 1,2
    this->Send_Recv(_t, "HELO", status);
    challenge = strtoul(_t, &endP, 10);


    if (strncmp(status, "300", 3) != 0) {
        closesocket(this->socketClient);
        ShowErr("status code invalido");
    }
    memset(status, '\0', 1024);

    // passo 3
    nonce = challenge ^ this->T_s; // risolvo la challenge

    // passo 4,5
    enc1 = this->T_s ^ nonce ^ this->T_c;
    enc2 = this->T_c ^ nonce;

    snprintf(authmsg, 1024, "AUTH %lu;%lu", enc1, enc2); // compongo AUTH enc1;enc2

    // passo 6
    this->Send_Recv(status, authmsg);

    if (strncmp(status, "200", 3) != 0) ShowErr("Auth errato");

#ifdef _DEBUG
    printf("\nConnessione OK\n");
#endif


}

void SharedLibClient::GestioneComandi () {
  if (this->parametri.lsf != NULL) {
      this->LSF();
  }
  if (this->parametri.exec != NULL) {
      this->EXEC();
  }
  if (this->parametri.download.src != NULL && this->parametri.download.size != 0 ) {
      this->DOWLOAD();
  }
  if (this->parametri.upload.src != NULL && this->parametri.upload.size != 0) {
      this->UPLOAD();
  }
}

void SharedLibClient::LSF(){

    char* status = (char*)Calloc(1024, sizeof(char));
    char* command = (char*)Calloc(1024, sizeof(char));

    snprintf(command, 1024, "LSF %s", this->parametri.lsf);

    this->Send_Recv(status, command);
 
    Free(command, 1024);

    if (strncmp(status, "300", 3) != 0){
        Free(status, 1024);
        
        printf("Errore nell'esecuzione di LSF\n");
        return;
    }
    Free(status, 1024);

    char* ans = this->ReadAll();

    printf("%s\n", ans);
    Free(ans, strlen(ans));
}

void SharedLibClient::EXEC() {
    char* command = (char*)Calloc(1024, sizeof(char));
    char* status = (char*)Calloc(1024, sizeof(char));

    snprintf(command, 1024, "EXEC %s", this->parametri.exec);
    this->Send_Recv(status, command);

    Free(command, 1024);

    if (strncmp(status, "300", 3) != 0) {
        Free(status, 1024);
        printf("Errore nell'esecuzione di EXEC\n");
        return;
    }
    Free(status, 1024);

    char* ans = this->ReadAll();

    printf("\n%s\n", ans);
}

void SharedLibClient::DOWLOAD() {
    char* command = (char*)Calloc(1024, sizeof(char));
    char* status = (char*)Calloc(1024, sizeof(char));

#ifdef WIN32
    sprintf_s(command, 1023, "DOWNLOAD %s;%llu\r\n", this->parametri.download.src, this->parametri.download.size);
#else
    snprintf(&command, 1023, "DOWNLOAD %s;%llu\r\n", this->parametri.download.src, this->parametri.download.size);
#endif

    this->Send_Recv(status, command);

    Free(command, strlen(command));

    if (strncmp(status, "300", 3) != 0) {
        Free(status, 1024);

        printf("Errore nell'esecuzione di DOWLOAD\n");
        return;
    }
    Free(status, 1024);

    //char* ans = this->ReadAll();

    //printf("%s\n", ans);
    //Free(ans, strlen(ans));

}

void SharedLibClient::UPLOAD() {
    char* command = (char*)Calloc(1024, sizeof(char));
    char* status = (char*)Calloc(1024, sizeof(char));

#ifdef WIN32
    sprintf_s(command, 1023, "UPLOAD %s;%llu\r\n", this->parametri.upload.src, this->parametri.upload.size);
#else
    snprintf(&command, 1023, "UPLOAD %s;%llu\r\n", this->parametri.upload.src, this->parametri.upload.size);
#endif

    this->Send_Recv(status, command);

    Free(command, strlen(command));

    if (strncmp(status, "300", 3) != 0) {
        Free(status, 1024);

        printf("Errore nell'esecuzione di UPLOAD\n");
        return;
    }
    Free(status, 1024);



    char* ans = this->ReadAll();

    printf("%s\n", ans);
    Free(ans, strlen(ans));
}

char* SharedLibClient::ReadAll(){
    char* buffer_recv = (char*)Calloc(1024, sizeof(char));
    char* ans = (char*)Calloc(1, sizeof(char));
    int x = 0;

    while (true) {
        this->Recv(buffer_recv);

        if (strlen(buffer_recv) == 0) break;
    
    
        if ((ans = (char*)realloc(ans, strlen(ans) + strlen(buffer_recv) +1 )) == NULL) {
            ShowErr("Impossibile allocare memoria per il ReadAll");
        }

        memcpy(ans + strlen(ans), buffer_recv, strlen(buffer_recv)+1);
        x++;
  }

  return ans;
}

void SharedLibClient::clearSocket() {
    // se instanziato, chiudi il socket
    if (this->socketClient != 0) {
        closesocket(this->socketClient);
    }


#ifdef _WIN32
    WSACleanup();
#endif
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

// strcpy Wrapper
void Strcpy(char* dest, unsigned int size, const char* src) {

#ifdef _WIN32
    strcpy_s(dest, size, src);
    if (errno) {
        ShowErr("Errore nel copiare una stringa");
    }
#else //linux
    strncpy(dest, src, size);
#endif

}