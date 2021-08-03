#pragma once

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <cstdarg>
#include <string>
#include <cstring>

// calloc Wrapper
void* Calloc(size_t nmemb, size_t size);
//
void* Realloc(void* var, size_t new_size);
// fprintf Wrapper
void ShowErr(const char* str);
// free Wrapper
void Free(void* arg, size_t size = 0);
// strcpy Wrapper
void Strcpy(char* dest, size_t size, const char* src);
// asprintf Wrapper
int Asprintf(char*& buffer, const char* Format, ...);