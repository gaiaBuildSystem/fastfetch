#pragma once

#include "fastfetch.h"

typedef struct FFAktualizrResult
{
    bool detected;
    FFstrbuf registered_name;
} FFAktualizrResult;

const FFAktualizrResult* ffDetectAktualizr();
