
#include "wrapper.h"
//////////////////////////////////////////////////////////////////////////////////
// Wrapper

// calloc Wrapper
void* Calloc(size_t count, size_t size) {

    if (count == 0 || size == 0) {
        ShowErr("Uno dei due numeri del Calloc è impostato a 0");
    }

    void* _t = calloc(count, size);
    if (_t == NULL) {
        ShowErr("Impossibile allocare memoria");
    }

    return _t;
}

// fprintf Wrapper
void ShowErr(const char* str) {
    char t[1024] = { 0 };
    if (errno != 0) {
#ifdef _WIN32
        strerror_s(t, 1023, errno);
#else
        strerror_r(errno, t, 1023);
#endif

        printf("\n%s\n", t);
    }
    fprintf(stderr, "%s\n", str);

#ifdef _DEBUG
    getchar();
#endif // _DEBUG
    exit(1);
    return;

}

// free Wrapper
void Free(void* arg, size_t size) {

    if (arg == NULL) {
        ShowErr("variabile data a Free() è NULL");
    }

    if (size != 0) {
        memset(arg, '\0', size);
    }


    free(arg);

}

// strcpy Wrapper
void Strcpy(char* dest, size_t size, const char* src) {
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
    n = vasprintf(&buffer, Format, argptr);
#endif

    va_end(argptr);

    return n;
}
