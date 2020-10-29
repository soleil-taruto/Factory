#pragma once

#include <stdlib.h>

typedef unsigned char aes128_u8;
typedef unsigned int aes128_u32;

void aes128_encrypt_init(aes128_u32 ctx[44], aes128_u8 rawKey[16]);
void aes128_encrypt(aes128_u32 ctx[44], aes128_u8 plain[16], aes128_u8 crypt[16]);

void aes128_decrypt_init(aes128_u32 ctx[44], aes128_u8 rawKey[16]);
void aes128_decrypt(aes128_u32 ctx[44], aes128_u8 plain[16], aes128_u8 crypt[16]);
