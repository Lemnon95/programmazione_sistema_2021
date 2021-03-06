
#include "share_lib_client.h"


SharedLibClient::SharedLibClient(int argc, char* argv[]) {

    this->parametri = { {AF_INET, 8888, {0}}, NULL, NULL, {NULL, 0}, {NULL, 0} };

    // parsing argomenti
    size_t maxArg = argc;
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

    // invio e ricezione di pi?? comandi
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

    int q = 5381;
    unsigned long int k = 0;
    // hashing
    // stessa phrase stresso hash
    // no fattori randomici
    // no fattori di tempo
    // bisogna basarci solo sull'input
    // e/o valori costanti
    for (size_t i = 0; i < strlen(token); ++i)
        k += q + token[i] * (i + 1);

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
    Send(this->socketClient, "HELO", 5); // invio HELO
    Recv(this->socketClient, status, 4); // ricevo lo status

    // controllo lo status ottenuto se ?? ok
    if (strncmp(status, "300", 3) != 0) {
        closesocket(this->socketClient);
        ShowErr("status code invalido");
    }
    memset(status, '\0', 4); // pulisco lo status


    if ((_recvChallengeLen = Recv(this->socketClient, recv_Challenge, 11)) <= 0) {
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
    Send(this->socketClient, authmsg, _authMsgLen);
    Recv(this->socketClient, status, 4);

    Free(authmsg, _authMsgLen);
    if (strncmp(status, "200", 3) != 0) { // Error
      closesocket(this->socketClient);
      ShowErr("Auth errato");
    }
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

    Send(this->socketClient, command, _commandLen);
    Recv(this->socketClient, status, 4);

    Free(command, _commandLen);

    // status del comando
    if (strncmp(status, "300", 3) != 0){
        printf("Errore nell'esecuzione di LSF\n");
        return;
    }

    _ansLen = ReadAll(this->socketClient, ans);

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

    Send(this->socketClient, command, _commandLen);
    Recv(this->socketClient, status, 4);

    Free(command, _commandLen);
    if (strncmp(status, "300", 3) != 0) {
        printf("Errore nell'esecuzione di EXEC\n");
        return;
    }

    _ansLen = ReadAll(this->socketClient, ans);

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

    size_t sizeI = std::filesystem::file_size(this->parametri.download.src);


    _commandLen = Asprintf(command, "DOWNLOAD %s;%llu", this->parametri.download.dest, sizeI);

    Send(this->socketClient, command, _commandLen);
    Recv(this->socketClient, status, 4);

    Free(command, _commandLen);
    if (strncmp(status, "300", 3) != 0) {
        printf("Errore ricevuto dal server\n");
        return;
    }

    FILE* _f = NULL;

    Fopen(&_f, this->parametri.download.src, "rb");

    SendReadF(this->socketClient, _f, sizeI);

    fclose(_f);
}

void SharedLibClient::UPLOAD() {
    // upload lato server
    // download lato client

    if (std::filesystem::exists(this->parametri.upload.dest)) {
        ShowErr("File destinazione gi?? presente");
    }

    FILE* _f;

    Fopen(&_f, this->parametri.upload.dest, "wb");

    char status[4] = {0};

    char* command = NULL;
    int _commandLen = 0;

    char* sizeCmd = NULL;
    int _sizeCmdLen = 0;

    char* size = NULL;
    int _sizeLen = 0;

    char* endP = NULL;

    size_t sizeI;


    // Invio SIZE per sapere la dimensione del file
    _sizeCmdLen = Asprintf(sizeCmd, "SIZE %s", this->parametri.upload.src);
    Send(this->socketClient, sizeCmd, _sizeCmdLen);
    Free(sizeCmd, _sizeCmdLen);
    // se percorso esiste allora 300
    Recv(this->socketClient, status, 4);
    if (strncmp(status, "300", 3) != 0) {
        Free(status, 4);
        printf("Errore da parte del server nell'eseguire SIZE\n");
        return;
    }

    // ottengo il peso del file
    size = (char*)Calloc(128, sizeof(char));
    _sizeLen = Recv(this->socketClient, size, 128);

    // peso del file in fomrato integer
    sizeI = strtoull(size, &endP, 10);
    Free(size, _sizeLen);
#ifdef _DEBUG
    printf("size recv: %llu", sizeI);
#endif
    ////////////////

    _commandLen = Asprintf(command, "UPLOAD %s;%llu", this->parametri.upload.src, sizeI);

    Send(this->socketClient, command, _commandLen);
    Recv(this->socketClient, status, 4);

    Free(command, _commandLen);

    if (strncmp(status, "300", 3) != 0) {
        printf("Errore nell'esecuzione di UPLOAD\n");
        return;
    }


    RecvWriteF(this->socketClient, _f, sizeI);

    fclose(_f);
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
