// libreria portabile usabile sia su linux che su windows
// qui tutte le funzioni devono lavorare sia su linux che su windows
// è cossibile creare una d
// 
// urls di riferimento
// https://stackoverflow.com/questions/142508/how-do-i-check-os-with-a-preprocessor-directive

#pragma once

#include <stdio.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif // _WIN32

class ShareFunction {

public:
	ShareFunction();

};