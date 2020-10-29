#pragma once

#include "C:\Factory\Common\all.h"

autoBlock_t *Stllt_Serializer(void *data, uint depth);
autoBlock_t *Stllt_Serializer_0(autoBlock_t *data);

void *Stllt_Deserializer(autoBlock_t *data, uint depth);
autoBlock_t *Stllt_Deserializer_0(autoBlock_t *data);
