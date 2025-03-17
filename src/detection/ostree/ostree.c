#include "ostree.h"
#include "common/processing.h"
#include "common/library.h"
#include "util/stringUtils.h"

#include <stdlib.h>

const FFOStreeResult* ffDetectDeployment(void)
{
    static FFOStreeResult result;
    result.detected = false;

    // execute the external process ostree admin status
    const char* error = NULL;
    FF_STRBUF_AUTO_DESTROY _output = ffStrbufCreate();
    if(!(error = ffProcessAppendStdOut(&_output, (char* const[]){
        "ostree",
        "admin",
        "status",
        NULL
    }))) {
        result.detected = true;
        FF_STRBUF_AUTO_DESTROY output = ffStrbufCreate();
        // nice, so now we can get only the commit hash
        error = ffProcessAppendStdOut(&output, (char* const[]){
            "/usr/bin/mars",
            "deploy-hash",
            NULL
        });

        if (!error) {
            ffStrbufSet(&result.commit_hash, &output);
        }
    }

    return &result;
}
