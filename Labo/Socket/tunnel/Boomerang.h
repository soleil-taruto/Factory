#pragma once

#include "libs\Comm.h"
#include "libs\Tunnel.h"
#include "C:\Factory\Common\Options\SockStream.h"
#include "libs\BoomTools.h"

#define CREDENTIAL_SIZE 16

/*
	Credential - 16 バイト
	Flag       -  1 バイト
	Reserved   -  1 バイト
	CRC16      -  2 バイト
*/
#define HEADER_SIZE 20
