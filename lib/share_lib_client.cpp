
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

            this->parametri.lst = (char*)Calloc(sizeof(argv[argc + 1]) + 1, sizeof(char));
            this->parametri.lst = argv[argc + 1];

        }

        // -e "<cmd> [<args>...]"
        if (strcmp(argv[argc], "-e") == 0) {

            if (argc + 1 >= maxArg) {
                ShowErr("parametro -e incompleto");
            }

            if (argv[argc + 1][0] == '-') {
                ShowErr("parametro -e incompleto");
            }

            // TODO: allocazione fissa? 1024?

            this->parametri.exec = (char*)Calloc(sizeof(argv[argc + 1]) + 1, sizeof(char));
            this->parametri.exec = argv[argc + 1];

        }

        // -d <src> <dest>
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
        ShowErr("indirizzo ip mancante");
    }


    printf("\nserver: %d %d\nlst: %s\nexec: %s\ndownload: %s %s\nupload: %s %s\n",
        this->parametri.server.sin_addr.s_addr,
        this->parametri.server.sin_port,
        this->parametri.lst,
        this->parametri.exec,
        this->parametri.download.src,
        this->parametri.download.dest,
        this->parametri.upload.src,
        this->parametri.upload.dest
    );



    this->T_s = this->generateToken("Immetti passphrase del server (max 254): ");

    this->T_c = this->generateToken("Immetti passphrase del client (max 254): ");

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


    if (connect(this->socketClient, (struct sockaddr*)&this->parametri.server, sizeof(this->parametri.server)) < 0) {
        ShowErr("Impossibile connettersi al server");
    }


}

void SharedLibClient::getPassphrase(const char* printText, char* passphrase) {
    printf(printText);
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
        fprintf(stderr, "Uno dei due numeri del Calloc è impostato a 0");
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
    strerror_s(t, 1024, errno);
    printf("\n%s\n", t);
    fprintf(stderr, "%s\n", str);
    exit(1);
    return;

}

// free Wrapper
void Free(void* arg, int size) {

    if (arg == NULL) {
        fprintf(stderr, "variabile data a Free() è NULL\n");
        exit(1);
        return;
    }

    memset(arg, '\0', size);
    free(arg);

}