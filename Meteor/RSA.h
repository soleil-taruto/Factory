#pragma once

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Collabo.h"
#include "C:\Factory\Common\Options\xml.h"
#include "C:\Factory\Common\Options\RingCipher2.h"
#include "C:\Factory\Common\Options\CRandom.h"

extern uint RSA_PlainSizeMax;

void RSA_GenerateKey(void);
autoBlock_t *RSA_GetPublicKey(void);
autoBlock_t *RSA_Encrypt(autoBlock_t *plainData, autoBlock_t *publicKey);
autoBlock_t *RSA_Decrypt(autoBlock_t *cipherData);
