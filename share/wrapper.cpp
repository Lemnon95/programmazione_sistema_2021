
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

// realloc Wrapper
void* Realloc(void* var, size_t new_size) {

    if (var == NULL) {
        ShowErr("variabile null passata a Realloc");
    }
    var = realloc(var, new_size);
    if (var == NULL) {
        ShowErr("Impossibile ri-allocare memoria");
    }

    return var;
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

// fopen Wrapper
bool Fopen(FILE** f, const char* FileName, const char* Mode) {

    if (FileName == NULL || Mode == NULL) {
        ShowErr("Nome file di Fopen vuoto");
    }
    errno = 0;

#ifdef _WIN32
    fopen_s(f, FileName, Mode);
#else
    *f = fopen(FileName, Mode);
#endif

    if (f == NULL || errno) {
        return false;
    }

    return true;
}

size_t Fread(FILE* file, void* Buffer, size_t BufferLen, size_t SizeSingleElement, size_t ElementCount) {

#ifdef _WIN32
    return fread_s(Buffer, BufferLen, SizeSingleElement, ElementCount, file);
#else
    return fread(Buffer, SizeSingleElement, ElementCount, file);
#endif
}

/////////////////////////////////

// invia in blocco il buffer
// da errore se il buffer è troppo grande
void Send(SOCKET soc, const char* str, size_t bufferMaxLen) {

    if (str == NULL) {
        return;
    }
    if (bufferMaxLen == 0) {
        return;
    }

    if (send(soc, str, bufferMaxLen, 0) < 0) {
        ShowErr("Errore nell'inviare un messaggio verso il server");
    }
}

// invia blocchi da 1024
void SendAll(SOCKET soc, const char* str, size_t bufferMaxLen) {
    if (soc <= 0) return;
    if (str == NULL) return;

    char* buffer = (char*)Calloc(1024, sizeof(char));

    for (size_t i = 0; i < bufferMaxLen; i += 1024) {
        memset(buffer, '\0', 1024);

        if (i + 1024 > bufferMaxLen) {
            memcpy(buffer, str + i, bufferMaxLen - i);
            Send(soc, buffer, bufferMaxLen - i);
        }
        else {
            memcpy(buffer, str + i, 1024);
            Send(soc, buffer, 1024);
        }


    }

    Free(buffer, 1024);
}

void SendReadF(SOCKET soc, FILE* _f, size_t BufferMaxLen) {
    if (_f == NULL) {
        ShowErr("passare un file aperto a RecvWriteF");
    }
    char* buffer_recv = (char*)Calloc(128, sizeof(char));
    size_t len_ans = 0, len = 0;

    while ((len = Fread(_f, buffer_recv, 128, 1, 128)) > 0) {
        Send(soc, buffer_recv, len);

        // clean up
        memset(buffer_recv, '\0', len);
        len_ans += len;

        if (len_ans + 1 > BufferMaxLen) {
            break;
        }
    }
    Free(buffer_recv, 128);
}

// riceve finchè non trova \0
int Recv(SOCKET soc, char* _return, size_t bufferMaxLen) {
    if (bufferMaxLen == 0) {
        return -1;
    }
    if (_return == NULL) {
        return -1;
    }
    int len = 0;
    size_t max = bufferMaxLen;
    char* moreBuf = NULL;

    if ((len = recv(soc, _return, bufferMaxLen, 0)) <= 0) {
        //ShowErr("Errore nel ricevere un messaggio dal client");
        return -1;
    }

    if (_return[max - 1] != '\0') {

        moreBuf = (char*)Calloc(bufferMaxLen, sizeof(char));

        while (_return[max - 1] != '\0') {

            if ((len = recv(soc, moreBuf, bufferMaxLen, 0)) <= 0) {
                //ShowErr("Errore nel ricevere un messaggio dal client");
                return -1;
            }

            _return = (char*)Realloc(_return, max + len);
            memcpy(_return + max, moreBuf, len);
            max += len;
        }
        Free(moreBuf, bufferMaxLen);
    }


    return max;
}

// riceve fino alla seguenza " \r\n.\r\n"
int ReadAll(SOCKET soc, char*& ans) {

    if (ans != NULL) {
        ShowErr("Passare a ReadAll un puntatore nullo");
    }

    char* buffer_recv = (char*)Calloc(128, sizeof(char));
    ans = (char*)Calloc(1, sizeof(char));
    size_t len_ans = 1, len = 0;

    /*
    buffer lungo effettivamente 128, ma leggo solo 127 byte, l'ultimo sarà \0
    */
    while ((len = recv(soc, buffer_recv, 127, 0)) > 0) { // buffer_recv avrà \0 alla fine

        if (len != strlen(buffer_recv)) {
            len = strlen(buffer_recv);
        }

        ans = (char*)Realloc(ans, len_ans + len);

#ifdef _WIN32
        strcat_s(ans, len_ans + len, buffer_recv);
#else
        strcat(ans, buffer_recv);
#endif

        // clean up
        memset(buffer_recv, '\0', len);
        len_ans += len;

        if (_endingSequence(ans, len_ans)) {
            break;
        }
    }

    Free(buffer_recv);

    return len_ans;
}

// ricevo e scrivo su file
size_t RecvWriteF(SOCKET soc, FILE* _f, size_t BufferMaxLen) {
    if (_f == NULL) {
        ShowErr("passare un file aperto a RecvWriteF");
    }

    char* buffer_recv = (char*)Calloc(128, sizeof(char));
    size_t len_ans = 0, len = 0;

    // mentre ricevo dati dal socket, scrivo sul file
    while ((len = recv(soc, buffer_recv, 128, 0)) > 0) {

        fwrite(buffer_recv, 1, len, _f);

        // clean up
        memset(buffer_recv, '\0', len);
        len_ans += len;

        if (len_ans + 1 > BufferMaxLen) {
            break;
        }
    }

    Free(buffer_recv, 128);

    return len_ans;

}

bool _endingSequence(char* buffer, size_t size) {
    char sequence[7] = " \r\n.\r\n"; // conto anche \0
    if (size < 7) {
        return false;
    }

    if (strncmp(buffer + size - 7, sequence, 7) == 0) {
        return true;
    }

    return false;
}
