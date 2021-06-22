#pragma once

#include <stdio.h>
#include <stdlib.h>


class SharedLibClient {

public:
	SharedLibClient();
	~SharedLibClient();
	void richiestaFraseServer();
	void richiestaFraseClient();

private:
	unsigned long int T_s = 0;
};

