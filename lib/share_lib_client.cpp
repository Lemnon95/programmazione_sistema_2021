
#include "share_lib_client.h"


SharedLibClient::SharedLibClient(int argc, char* argv[]) {
    
    this->parametri = { {AF_INET, 8888, {0}}, NULL, NULL, {NULL, 0}, {NULL, 0} };

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

        // -d <src> <desc>
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

            this->parametri.download.dest = (char*)Calloc(sizeof(argv[argc + 2]) + 1, sizeof(char));
            this->parametri.download.dest = argv[argc + 2];

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

            this->parametri.upload.dest = (char*)Calloc(sizeof(argv[argc + 2]) + 1, sizeof(char));
            this->parametri.upload.dest = argv[argc + 2];

        }
    }

    if (this->parametri.server.sin_addr.s_addr == 0) {
        ShowErr("indirizzo ip mancante (-h)");
    }

#ifdef _DEBUG
    printf("\nserver: %d %d\nlsf: %s\nexec: %s\ndownload: %s %s\nupload: %s %s\n",
        this->parametri.server.sin_addr.s_addr,
        this->parametri.server.sin_port,
        this->parametri.lsf,
        this->parametri.exec,
        this->parametri.download.src,
        this->parametri.download.dest,
        this->parametri.upload.src,
        this->parametri.upload.dest
    );
#endif // DEBUG


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

    // apri un socket TCP
    this->socketClient = socket(PF_INET, SOCK_STREAM, 0);
    if (this->socketClient < 0) {
        this->clearSocket();
        ShowErr("Errore creazione socket master");
    }

    // tentativo di connessione al server dato
    if (connect(this->socketClient, (sockaddr*)&(this->parametri.server), sizeof(this->parametri.server)) < 0) {
        this->clearSocket();
        ShowErr("Impossibile connettersi al server");
    }

    // effettua la connessione ed autenticazione
    this->Trasmissione();
    
#ifdef _DEBUG
    printf("\nConnessione OK\n");
#endif

    // invio e ricezione di più comandi
    this->GestioneComandi();
}

void SharedLibClient::getPassphrase(const char* printText, char* passphrase) {
    // prende in input x caratteri
    printf("%s",printText);
    fgets(passphrase, 254, stdin);
}

unsigned long int SharedLibClient::generateToken(const char* printText) {
    // stringa di caratteri
    char* passphrase = (char*)Calloc(256, sizeof(char));

    // ottieni l'input dell'utente, la stringa contiene 2 \0\0 finali
    this->getPassphrase(printText,passphrase);

    // ottieni l'hash dalla stringa data
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



void SharedLibClient::Trasmissione() {

    unsigned long int challenge = 0;
    unsigned long int enc1 = 0;
    unsigned long int enc2 = 0;
    unsigned long int nonce = 0;
    
    int _authMsgLen = 0;
    char* authmsg = NULL;

    char* endP;
    char status[4] = { 0 };
    char* recv_Challenge = (char*)Calloc(11, sizeof(char)); // array della challenge
    int _recvChallengeLen = 0;
    /*
    1. invio HELO
    2. ricevo la challenge
    3. risolvo la challenge
    4. compongo AUTH
    5. invio AUTH
    6. ottengo il risultato
    comandi ulteriori
    */

    // passo 1,2 //////////////////////////////////////////////////
    this->Send("HELO", 5); // invio HELO
    this->Recv(status, 4); // ricevo lo status

    // controllo lo status ottenuto se è ok
    if (strncmp(status, "300", 3) != 0) {
        closesocket(this->socketClient);
        ShowErr("status code invalido");
    }
    memset(status, '\0', 4); // pulisco lo status


    if ((_recvChallengeLen = this->Recv(recv_Challenge, 11)) <= 0) {
        ShowErr("Errore nel ricevere la challenge dal server");
    } // ricevo la challenge
    challenge = strtoul(recv_Challenge, &endP, 10); // strasformo la challenge
    Free(recv_Challenge, _recvChallengeLen);
    ///////////////////////////////////////////////////////////////

    // passo 3 ////////////////////////////////////////////////////
    nonce = challenge ^ this->T_s; // risolvo la challenge
    ///////////////////////////////////////////////////////////////

    // passo 4,5 //////////////////////////////////////////////////
    enc1 = this->T_s ^ nonce ^ this->T_c;
    enc2 = this->T_c ^ nonce;

    _authMsgLen = Asprintf(authmsg, "AUTH %lu;%lu", enc1, enc2); // compongo AUTH enc1;enc2
    ///////////////////////////////////////////////////////////////

    // passo 6 ////////////////////////////////////////////////////
    this->Send(authmsg, _authMsgLen);
    this->Recv(status, 4);

    Free(authmsg, _authMsgLen);
    if (strncmp(status, "200", 3) != 0) ShowErr("Auth errato"); // Error
    ///////////////////////////////////////////////////////////////

    // Auth ok
}

void SharedLibClient::GestioneComandi () {
  if (this->parametri.lsf != NULL) {
      this->LSF();
  }
  if (this->parametri.exec != NULL) {
      this->EXEC();
  }
  if (this->parametri.download.src != NULL && this->parametri.download.dest != NULL ) {
      this->DOWLOAD();
  }
  if (this->parametri.upload.src != NULL && this->parametri.upload.dest != NULL) {
      this->UPLOAD();
  }
}

void SharedLibClient::LSF(){

    char status[4] = { 0 };
    
    char* command = NULL;
    int _commandLen = 0;

    char* ans = NULL;
    int _ansLen = 0;


    _commandLen = Asprintf(command, "LSF %s", this->parametri.lsf);

    this->Send(command, _commandLen);
    this->Recv(status, 4);
 
    Free(command, _commandLen);

    // status del comando
    if (strncmp(status, "300", 3) != 0){
        printf("Errore nell'esecuzione di LSF\n");
        return;
    }

    _ansLen = this->ReadAll(ans);

    printf("\n%s\n", ans);
    Free(ans, _ansLen);
}

void SharedLibClient::EXEC() {
    
    char status[4] = { 0 };

    char* command = NULL;
    int _commandLen = 0;

    char* ans = NULL;
    int _ansLen = 0;

    _commandLen = Asprintf(command, "EXEC %s", this->parametri.exec);

    this->Send(command, _commandLen);
    this->Recv(status, 4);

    Free(command, _commandLen);
    if (strncmp(status, "300", 3) != 0) {
        printf("Errore nell'esecuzione di EXEC\n");
        return;
    }

    _ansLen = this->ReadAll(ans);

    printf("\n%s\n", ans);
    Free(ans, _ansLen);
}

void SharedLibClient::DOWLOAD() {
    // download lato server
    // upload lato client

    if (!std::filesystem::exists(this->parametri.download.src)) {
        ShowErr("file inserito per il download non presente");
    }
    if (std::filesystem::is_directory(this->parametri.download.src)) {
        ShowErr("non puoi caricare una directory");
    }

    char status[4] = { 0 };
    
    char* command = NULL;
    int _commandLen = 0;

    unsigned long long sizeI = std::filesystem::file_size(this->parametri.download.src);

    _commandLen = Asprintf(command, "DOWNLOAD %s;%llu", this->parametri.download.dest, sizeI);

    this->Send(command, _commandLen);
    this->Recv(status, 4);

    Free(command, _commandLen);
    if (strncmp(status, "300", 3) != 0) {
        printf("Errore nell'esecuzione di DOWLOAD\n");
        return;
    }

    

    char* buffer = (char*)Calloc(sizeI+1, sizeof(char));

    FILE* _f = NULL;

#ifdef _WIN32
    fopen_s(&_f, this->parametri.download.src, "r");
    if (errno) {
        ShowErr("Impossibile aprire file per UPLOAD");
    }
#else
    _f = fopen(this->parametri.download.src, "r");
#endif

#ifdef _WIN32
    fread_s(buffer, sizeI, 1, sizeI, _f);
#else
    fread(buffer, 1, sizeI, _f);
#endif

    fclose(_f);
    

    Send(buffer, sizeI);
    Free(buffer);
}

void SharedLibClient::UPLOAD() {
    // upload lato server
    // download lato client

    if (std::filesystem::exists(this->parametri.upload.dest)) {
        ShowErr("File destinazione già presente");
    }
    
    char status[4] = {0};

    char* command = NULL;
    int _commandLen = 0;

    char* sizeCmd = NULL;
    int _sizeCmdLen = 0;

    char* size = NULL;
    int _sizeLen = 0;

    char* endP = NULL;

    unsigned long long sizeI;

    char* ans = NULL;
    int _ansLen = 0;

    // Invio SIZE per sapere la dimensione del file
    _sizeCmdLen = Asprintf(sizeCmd, "SIZE %s", this->parametri.upload.src);
    this->Send(sizeCmd, _sizeCmdLen);
    Free(sizeCmd, _sizeCmdLen);
    // se percorso esiste allora 300
    this->Recv(status, 4);
    if (strncmp(status, "300", 3) != 0) {
        Free(status, 4);
        printf("Errore nell'esecuzione di SIZE\n");
        return;
    }
    
    // ottengo il peso del file
    size = (char*)Calloc(128, sizeof(char));
    _sizeLen = this->Recv(size, 128);

    // peso del file in fomrato integer
    sizeI = strtoull(size, &endP, 10);
    Free(size, _sizeLen);

    ////////////////

    _commandLen = Asprintf(command, "UPLOAD %s;%llu", this->parametri.upload.src, sizeI);

    this->Send(command, _commandLen);
    this->Recv(status, 4);

    Free(command, _commandLen);

    if (strncmp(status, "300", 3) != 0) {
        printf("Errore nell'esecuzione di UPLOAD\n");
        return;
    }

    // leggo al massimo sizeI caratteri (o byte)
    _ansLen = this->ReadMax(ans, sizeI);

    FILE* _f;
#ifdef _WIN32
    fopen_s(&_f, this->parametri.upload.dest, "w");
#else
    _f = fopen(this->parametri.upload.dest, "w");
#endif
    if (errno) {
        ShowErr("Impossibile aprire file in DOWNLOAD");
    }

    fwrite(ans, 1, _ansLen, _f);

    fclose(_f);

    //printf("\n%s\n", ans);
    Free(ans, _ansLen);
}

///////////////////////////////////////

void SharedLibClient::Send(const char* str, unsigned long long bufferMaxLen) {

    if (str == NULL) {
        return;
    }
    if (bufferMaxLen == 0) {
        return;
    }

    if (send(this->socketClient, str, bufferMaxLen, 0) <= 0) {
        ShowErr("Errore nell'inviare un messaggio verso il server");
    }
}

// TODO: si può migliorare
void SharedLibClient::SendAll(const char* str, unsigned long long bufferMaxLen) {
    if (this->socketClient <= 0) return;
    if (str == NULL) return;

    int point = 0;
    char* buffer = (char*)Calloc(1024, sizeof(char));

    for (int i = 0; i < bufferMaxLen; i += 1023) {
        memset(buffer, '\0', 1024);

        if (i + 1023 > bufferMaxLen) {
            memcpy(buffer, str + i, bufferMaxLen - i);
        }
        else {
            memcpy(buffer, str + i, 1023);
        }

        Send(buffer, 1023);
    }

    Free(buffer, 1024);
}

int SharedLibClient::Recv(char* _return, unsigned long long bufferMaxLen) {
    if (bufferMaxLen == 0) {
        return -1;
    }
    if (_return == NULL) {
        return -1;
    }
    int len = 0;
    int max = bufferMaxLen;
    char* moreBuf = NULL;

    if ((len = recv(this->socketClient, _return, bufferMaxLen, 0)) <= 0) {
        //ShowErr("Errore nel ricevere un messaggio dal client");
        return -1;
    }

    if (_return[max - 1] != '\0') {

        moreBuf = (char*)Calloc(bufferMaxLen, sizeof(char));

        while (_return[max - 1] != '\0') {

            if ((len = recv(this->socketClient, moreBuf, bufferMaxLen, 0)) <= 0) {
                //ShowErr("Errore nel ricevere un messaggio dal client");
                return -1;
            }

            _return = (char*)realloc(_return, max + len);
            memcpy(_return + max, moreBuf, len);
            max += len;
        }
        Free(moreBuf, bufferMaxLen);
    }


    return max;

}

int SharedLibClient::ReadAll(char*& ans){

    if (ans != NULL) {
        ShowErr("Passare a ReadAll un puntatore nullo");
    }

    char* buffer_recv = (char*)Calloc(128, sizeof(char));
    ans = (char*)Calloc(1, sizeof(char));
    int len_ans = 1,len = 0;

    /*
    buffer lungo effettivamente 128, ma leggo solo 127 byte, l'ultimo sarà \0
    */
    while ((len = recv(this->socketClient, buffer_recv, 127, 0)) > 0) { // buffer_recv avrà \0 alla fine
        
        if (len != strlen(buffer_recv)) {
            len = strlen(buffer_recv);
        }

        if ((ans = (char*)realloc(ans, len_ans + len)) == NULL) {
            ShowErr("Impossibile allocare memoria per il ReadAll");
        }

#ifdef _WIN32
        strcat_s(ans, len_ans + len, buffer_recv); 
#else
        strcat(ans, buffer_recv);
#endif

        // clean up
        memset(buffer_recv, '\0', len);
        len_ans += len;

        if (this->_endingSequence(ans, len_ans)) {
            break;
        }
    }

    Free(buffer_recv);

    return len_ans;
}

int SharedLibClient::ReadMax(char*& ans, unsigned long long BufferMaxLen) {

    if (ans != NULL) {
        ShowErr("Passare a ReadAll un puntatore nullo");
    }

    char* buffer_recv = (char*)Calloc(128, sizeof(char));
    ans = (char*)Calloc(1, sizeof(char));
    int len_ans = 1, len = 0;

    /*
    buffer lungo effettivamente 128, ma leggo solo 127 byte, l'ultimo sarà \0
    */
    while ((len = recv(this->socketClient, buffer_recv, 127, 0)) > 0) { // buffer_recv avrà \0 alla fine

        if (len != strlen(buffer_recv)) {
            len = strlen(buffer_recv);
        }

        if ((ans = (char*)realloc(ans, len_ans + len)) == NULL) {
            ShowErr("Impossibile allocare memoria per il ReadAll");
        }

#ifdef _WIN32
        strcat_s(ans, len_ans + len, buffer_recv);
#else
        strcat(ans, buffer_recv);
#endif

        // clean up
        memset(buffer_recv, '\0', len);
        len_ans += len;

        if (len_ans+1 >= BufferMaxLen) {
            break;
        }
    }

    Free(buffer_recv);

    return len_ans;

}

///////////////////////////////////////

void SharedLibClient::clearSocket() {
    // se instanziato, chiudi il socket
    if (this->socketClient != 0) {
#ifdef _WIN32
        shutdown(this->socketClient, SD_BOTH);
#else
        shutdown(this->socketClient, SHUT_RDWR);
#endif
        closesocket(this->socketClient);
    }


#ifdef _WIN32
    WSACleanup();
#endif
}

bool SharedLibClient::_endingSequence(char* buffer, unsigned long long size) {
    char sequence[7] = " \r\n.\r\n"; // conto anche \0
    if (size < 7) {
        return false;
    }
    /*
    for (int i = 0; i < size - 7; i++) {

        if (strncmp(buffer + i, sequence, 7) == 0) {
            return true;
        }

    }
    */

    if (strncmp(buffer + size-7, sequence, 7) == 0) {
        return true;
    }




    return false;
}


// calloc Wrapper
void* Calloc(unsigned long int count, unsigned long int size) {

    if (count == 0 || size == 0) {
        fprintf(stderr, "Uno dei due numeri del Calloc e' impostato a 0");
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

    if (size != 0) {
        memset(arg, '\0', size);
    }
    
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

// asprintf Wrapper
int Asprintf(char*& buffer, const char* Format, ...) {
    
    if (buffer != NULL) {
        ShowErr("Asprintf buffer deve essere un puntatore a NULL");
    }
    int n = 0;

    va_list argptr;
    va_start(argptr, Format);

#ifdef _WIN32
    n = vsnprintf(NULL, 0, Format, argptr) + 1;
    buffer = (char*)Calloc(n, sizeof(char));
    vsprintf_s(buffer, n, Format, argptr);
#else
    n = vasprintf(&buffer, Format, argptr) + 1;
#endif

    return n;
}
