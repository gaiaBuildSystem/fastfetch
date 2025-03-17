#include "aktualizr.h"
#include "common/processing.h"
#include "common/library.h"
#include "util/stringUtils.h"

#include <stdlib.h>

const FFAktualizrResult* ffDetectAktualizr(void)
{
    static FFAktualizrResult result;
    result.detected = false;

    // execute the external process
    const char* error = NULL;
    FF_STRBUF_AUTO_DESTROY _output = ffStrbufCreate();
    if(!(error = ffProcessAppendStdOut(&_output, (char* const[]){
        "/usr/bin/jq",
        ".registeredName",
        "/var/sota/import/info.json",
        NULL
    }))) {
        result.detected = true;
        ffStrbufSet(&result.registered_name, &_output);
    }

    return &result;
}
