#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/option.h"
#include "detection/aktualizr/aktualizr.h"
#include "modules/aktualizr/aktualizr.h"
#include "util/stringUtils.h"

#include <ctype.h>

static void buildOutputDefault(const FFAktualizrResult* os, FFstrbuf* result)
{
}

void ffPrintAktualizrRegisteredName(FFAktualizrOptions* options)
{
    const FFAktualizrResult* result = ffDetectAktualizr();

    if (result->detected == false) {
        ffPrintError(FF_AKTUALIZR_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No Aktualizr deployment detected");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_AKTUALIZR_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        puts(result->registered_name.chars);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_AKTUALIZR_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
            FF_FORMAT_ARG(result->registered_name, "registered-name"),
        }));
    }
}

// FIXME: I only used this from copy and paste needs fix
bool ffParseAktualizrCommandOptions(FFAktualizrOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_AKTUALIZR_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

// FIXME: I only used this from copy and paste needs fix
void ffParseAktualizrJsonObject(FFOSOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if(ffStrEqualsIgnCase(key, "type"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_AKTUALIZR_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

// FIXME: I only used this from copy and paste needs fix
void ffGenerateAktualizrJsonConfig(FFAktualizrOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyAktualizrOptions))) FFAktualizrOptions defaultOptions;
    ffInitAktualizrOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

// FIXME: I only used this from copy and paste needs fix
void ffGenerateAktualizrJsonResult(FF_MAYBE_UNUSED FFAktualizrOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
}

static FFModuleBaseInfo ffModuleInfo = {
    .name = FF_AKTUALIZR_MODULE_NAME,
    .description = "Print the actual Aktualizr registered name",
    .parseCommandOptions = (void*) ffParseAktualizrCommandOptions,
    .parseJsonObject = (void*) ffParseAktualizrJsonObject,
    .printModule = (void*) ffPrintAktualizrRegisteredName,
    .generateJsonResult = (void*) ffGenerateAktualizrJsonResult,
    .generateJsonConfig = (void*) ffGenerateAktualizrJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Platform Registered Name", "registered-name"},
    }))
};

void ffInitAktualizrOptions(FFAktualizrOptions* options)
{
    options->moduleInfo = ffModuleInfo;
    ffOptionInitModuleArg(&options->moduleArgs, "󰅠");
}

void ffDestroyAktualizrOptions(FFAktualizrOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
