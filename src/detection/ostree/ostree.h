#pragma once

#include "fastfetch.h"

typedef struct FFOStreeResult
{
    bool detected;
    FFstrbuf commit_hash;
} FFOStreeResult;

const FFOStreeResult* ffDetectDeployment();
