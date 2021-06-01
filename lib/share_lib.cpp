#include "share_lib.h"

// costruttore classe
ShareFunction::ShareFunction(int argc, char* argv[]) {
    char* _path=NULL;

    

    #ifdef _WIN32
    WCHAR _t[MAX_PATH + 1] = { 0 }; // instanzio un array di MAX_PATH caratteri, MAX_PATH è definito da windows
    GetTempPathW(MAX_PATH, _t); // chiedo al sistema il percorso temporaneo, _t conterrà una cella vuota alla fine 
    
    _path = (char*)Calloc(MAX_PATH+1,1);// instanzio la variabile path
    
    size_t lenTempPath;// variabile che conterrà la grandezza effettiva del path
    
    // https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/wcstombs-s-wcstombs-s-l?view=msvc-160
    wcstombs_s(&lenTempPath,
        _path, 
        MAX_PATH + 1,
        (const WCHAR*)_t,
        MAX_PATH + 1);

    //printf("%d", lenTempPath); // posso ridurre _path da MAX_PATH+1 a 
    

    #else
    _path = (char*)Calloc(sizeof("/tmp/server.log"), 1); // sizeof("123") + \0 = 3+1, conta in automatico un \0 alla fine
    _path = (char*)"/tmp/server.log";
    #endif // _WIN32

    // parametri di default
    this->parametri = { 8888, 10, NULL, false, _path };

    printf("percorso temporaneo: %s\n",this->parametri.logPath);

    // parsing argomenti

    unsigned long long maxArg = argc;

    while (argc > 0) {

        argc -= 1;
        
        // -p <port>
        if (strcmp(argv[argc], "-p") == 0) {

            if (argc + 1 >= maxArg) {
                showErr("parametro -p incompleto");
                
            }

            if (argv[argc + 1][0] == '-') {
                showErr("parametro -p incompleto");
                
            }
            
            this->parametri.port = atoi(argv[argc + 1]);
            if (errno) {
                showErr("il parametro di -p non risulta un numero");
                
            }

            if (this->parametri.port == 0) {
                showErr("porta 0 non è valida");
            }

        }

        // -n <num>
        if (strcmp(argv[argc], "-n") == 0) {

            if (argc + 1 >= maxArg) {
                showErr("parametro -n incompleto");
                
            }

            if (argv[argc + 1][0] == '-') {
                showErr("parametro -n incompleto");
            }

            this->parametri.nthread = atoi(argv[argc + 1]);
            if (errno) {
                showErr("il parametro di -n non risulta un numero");
            }

            if (this->parametri.nthread == 0) {
                showErr("numero di thread invalido");
            }


        }

        // -c <path>
        if (strcmp(argv[argc], "-c") == 0) {

            if (argc + 1 >= maxArg) {
                showErr("parametro -c incompleto");

            }

            if (argv[argc + 1][0] == '-') {
                showErr("parametro -c incompleto");
            }

            this->parametri.configPath = (char*)Calloc(sizeof(argv[argc + 1])+1, 1);
            this->parametri.configPath = argv[argc + 1];
        }

        // -s
        if (strcmp(argv[argc], "-s") == 0) {
            this->parametri.printToken = true;
        }

        // -l <path>
        if (strcmp(argv[argc], "-l") == 0) {

            if (argc + 1 >= maxArg) {
                showErr("parametro -l incompleto");

            }

            if (argv[argc + 1][0] == '-') {
                showErr("parametro -l incompleto");
            }

            this->parametri.logPath = (char*)Calloc(sizeof(argv[argc + 1]) + 1, 1);
            this->parametri.logPath = argv[argc + 1];
        }

    }
    

    printf("---\n%d\n%d\n%s\n%s\n%d\n---\n", 
        this->parametri.port, 
        this->parametri.nthread, 
        this->parametri.configPath,
        this->parametri.logPath,
        this->parametri.printToken);

}

// calloc Wrapper
void* Calloc(size_t nmemb, size_t size) {
    
    if (nmemb == 0 || size == 0) {
        fprintf(stderr, "Uno dei due numeri del Calloc è impostato a 0");
        exit(1);
        return NULL;
    }
        
    void* _t = calloc(nmemb, size);
    if (_t == 0) {
        fprintf(stderr, "Impossibile allocare memoria");
        exit(1);
        return NULL;
    }

    return _t;
}

// fprintf Wrapper
void showErr(const char* str) {

    fprintf(stderr, "%s\n", str);
    exit(1);
    return;

}
