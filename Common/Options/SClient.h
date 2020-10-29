#pragma once

#include "SockClient.h"
#include "SockStream.h" // option

int SClient(char *serverHost, uint serverPort, int (*userPerform)(int, uint), uint userPrm);
