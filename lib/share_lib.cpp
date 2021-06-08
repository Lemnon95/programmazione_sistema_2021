#include "share_lib.h"

// costruttore classe
ShareFunction::ShareFunction(int argc, char* argv[]) {
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

            this->parametri.configPath = (WCHAR*)Calloc(sizeof(argv[argc + 1])+1, sizeof(WCHAR));
            this->parametri.configPath = (WCHAR*)argv[argc + 1];
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
            this->parametri.logPath = (WCHAR*)argv[argc + 1];
        }

    }
    
    // debug print
    printf("---\nPorta: %d\nNumero thread: %d\nConfig path: %ls\nLog path: %ls\nStampa token: %d\n---\n", 
        this->parametri.port, 
        this->parametri.nthread, 
        this->parametri.configPath,
        this->parametri.logPath,
        this->parametri.printToken);

}
// distruttore classe
ShareFunction::~ShareFunction() {
    // alla distruzione di questa classe
    printf("distruzione classe\n");
    if(this->FileDescLog != NULL)
        this->closeLog();
}

void ShareFunction::getPassphrase(char* passphrase) {
    printf("Immetti passphrase (max 254): ");
    fgets(passphrase, 254, stdin);
}

unsigned long int ShareFunction::generateToken() {

    char* passphrase = (char*)Calloc(256, sizeof(char));

    this->getPassphrase(passphrase);

    unsigned long int k= 5381;
    // hashing
    // stessa phrase stresso hash
    // no fattori randomici
    // no fattori di tempo
    // bisogna basarci solo sull'input
    // e/o valori costanti
    for (int i = 0; i < strlen(passphrase); ++i)
        k = (k * 27) + passphrase[i];

    // reset passphrase
    Free(passphrase, 256);

    return k;
}

void ShareFunction::openLog() {
    // controlla se il file è già aperto
    if (this->FileDescLog == NULL) {
        // se non è aperto
        // apri il file in modalità Append
        fopen_s(&(this->FileDescLog),(char*)(this->parametri.logPath), "a");
        // se da errore
        if (this->FileDescLog == NULL) {
            ShowErr("errore nell'aprire il file");
        }
    }
    
}

void ShareFunction::closeLog() {
    // se è aperto il file
    if (this->FileDescLog != NULL) {
        // tenta di chiuderlo
        if (fclose(this->FileDescLog)) {
            ShowErr("errore nel chiudere il file log");
        }
    }
    
}

unsigned long int ShareFunction::getToken_s() {

    this->T_s = this->generateToken();

    if (this->parametri.printToken) {
        printf("\ntoken: %lu\n", this->T_s);
    }

    return T_s;

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
