#pragma once

#include "libs\Comm.h"
#include "libs\Tunnel.h"
#include "C:\Factory\Common\Options\SockStream.h"
#include "libs\BoomTools.h"

#define CREDENTIAL_SIZE 16

/*
	Credential - 16 �o�C�g
	Flag       -  1 �o�C�g
	Reserved   -  1 �o�C�g
	CRC16      -  2 �o�C�g
*/
#define HEADER_SIZE 20
