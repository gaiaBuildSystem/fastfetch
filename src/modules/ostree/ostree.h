#pragma once

#include "fastfetch.h"

#define FF_OS_TREE_MODULE_NAME "OStree"

void ffPrintOStreeCommitDeployed(FFOStreeOptions* options);
void ffInitOStreeOptions(FFOStreeOptions* options);
void ffDestroyOStreeOptions(FFOStreeOptions* options);
