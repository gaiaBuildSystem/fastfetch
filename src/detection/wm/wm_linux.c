#include "wm.h"

#include "common/processing.h"
#include "common/io.h"
#include "common/binary.h"
#include "common/path.h"
#include "common/strutil.h"
#include "common/debug.h"

const char* ffDetectWMPlugin([[maybe_unused]] FFstrbuf* pluginName) {
    return "Not supported on this platform";
}

static bool extractCommonWmVersion(const char* line, [[maybe_unused]] uint32_t len, void* userdata) {
    int count = 0;
    sscanf(line, "%*d.%*d.%*d%n", &count);
    if (count == 0) {
        return true;
    }

    ffStrbufSetNS((FFstrbuf*) userdata, len, line);
    return false;
}

#if !__ANDROID__
static bool extractHyprlandVersion(const char* line, uint32_t len, void* userdata) {
    if (line[0] != 'v') {
        return true;
    }
    ++line;
    --len;
    int count = 0;
    sscanf(line, "%*d.%*d.%*d%n", &count);
    if (count == 0) {
        return true;
    }

    ffStrbufSetNS((FFstrbuf*) userdata, len, line);
    return false;
}

static const char* getHyprland(FFstrbuf* result) {
    FF_DEBUG("Detecting Hyprland version");
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    FF_DEBUG("Checking for " FF_PATH_PKG_BASE "/include/hyprland/src/version.h file");
    if (ffReadFileBuffer(FF_PATH_PKG_BASE "/include/hyprland/src/version.h", result)) {
        FF_DEBUG("Found version.h file, extracting version");
        if (ffStrbufSubstrAfterFirstS(result, "\n#define GIT_TAG ")) {
            ffStrbufSubstrAfterFirstC(result, '"');
            ffStrbufSubstrBeforeFirstC(result, '"');
            ffStrbufTrimLeft(result, 'v');
            FF_DEBUG("Extracted version from version.h: %s", result->chars);
            return nullptr;
        }
        FF_DEBUG("Failed to extract version from version.h");
        ffStrbufClear(result);
    } else {
        FF_DEBUG("version.h file not found, trying Hyprland executable");
    }

    const char* error = ffFindExecutableInPath("Hyprland", &buffer);
    if (error) {
        FF_DEBUG("Error finding Hyprland executable: %s", error);
        return "Failed to find Hyprland executable path";
    }
    FF_DEBUG("Found Hyprland executable at: %s", buffer.chars);

    ffBinaryExtractStrings(buffer.chars, extractHyprlandVersion, result, (uint32_t) strlen("v0.0.0"));
    if (result->length > 0) {
        FF_DEBUG("Extracted version from binary strings: %s", result->chars);
        return nullptr;
    }
    FF_DEBUG("Failed to extract version from binary strings, trying --version option");

    if (ffProcessAppendStdOut(result, (char* const[]) { buffer.chars, "--version", nullptr }) == nullptr) {
        // Hyprland 0.48.1 built from branch  at commit 29e2e59...
        // Date: ...
        // Tag: v0.48.1, commits: 5937
        // ...

        FF_DEBUG("Raw version output: %s", result->chars);
        // Use tag if available
        if (ffStrbufSubstrAfterFirstS(result, "\nTag: v")) {
            ffStrbufSubstrBeforeFirstC(result, ',');
            FF_DEBUG("Extracted version from Tag: %s", result->chars);
        } else {
            ffStrbufSubstrAfterFirstC(result, ' ');
            ffStrbufSubstrBeforeFirstC(result, ' ');
            FF_DEBUG("Extracted version from output: %s", result->chars);
        }
        return nullptr;
    }
    FF_DEBUG("Failed to run Hyprland --version command");

    return "Failed to run command `Hyprland --version`";
}

static bool extractSwayVersion(const char* line, [[maybe_unused]] uint32_t len, void* userdata) {
    FFstrbuf* result = (FFstrbuf*) userdata;
    if (!ffStrStartsWith(line, "sway")) {
        return true;
    }
    if (ffStrStartsWith(line + 4, " version ")) {
        ffStrbufSetNS(result, len - (uint32_t) strlen("sway version "), line + strlen("sway version "));
        ffStrbufTrimRightSpace(result);
        return false;
    } else {
        char swayfxVer[32], swayVer[32];
        if (sscanf(line + 4, "fx version %31[^ ] (based on sway %31[^)])", swayfxVer, swayVer) == 2) {
            ffStrbufSetF(result, "%s [swayfx %s]", swayVer, swayfxVer);
            return false;
        }
    }

    return true;
}

static const char* getSway(FFstrbuf* result) {
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreate();
    const char* error = ffFindExecutableInPath("sway", &path);
    if (error) {
        return "Failed to find sway executable path";
    }

    ffBinaryExtractStrings(path.chars, extractSwayVersion, result, (uint32_t) strlen("sway version 0.0.0"));
    if (result->length > 0) {
        return nullptr;
    }

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    if (ffProcessAppendStdOut(&buffer, (char* const[]) { path.chars, "--version", nullptr }) == nullptr) { // sway version 1.10
        return extractSwayVersion(buffer.chars, buffer.length, result) ? "Failed to parse sway version output" : nullptr;
    }

    return "Failed to run command `sway --version`";
}

static const char* getLabwc(FFstrbuf* result) {
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreate();
    const char* error = ffFindExecutableInPath("labwc", &path);
    if (error) {
        return "Failed to find labwc executable path";
    }

    ffBinaryExtractStrings(path.chars, extractCommonWmVersion, result, (uint32_t) strlen("0.0.0"));
    if (result->length > 0) {
        return nullptr;
    }

    if (ffProcessAppendStdOut(result, (char* const[]) { path.chars, "--version", nullptr }) == nullptr) { // labwc 0.9.0 (+xwayland +nls +rsvg +libsfdo)
        ffStrbufSubstrAfterFirstC(result, ' ');
        ffStrbufSubstrBeforeFirstC(result, ' ');
        return nullptr;
    }

    return "Failed to run command `labwc --version`";
}

static const char* getNiri(FFstrbuf* result) {
    if (ffProcessAppendStdOut(result, (char* const[]) { "niri", "--version", nullptr }) == nullptr) { // niri 25.11 (commit b35bcae)
        ffStrbufSubstrAfterFirstC(result, ' ');
        ffStrbufSubstrBeforeLastC(result, '(');
        ffStrbufTrimRightSpace(result);
        return nullptr;
    }

    return "Failed to run command `niri --version`";
}

static const char* getUmbriel(FFstrbuf* result) {
    if (ffProcessAppendStdOut(result, (char* const[]) { "umbriel", "--version", nullptr }) == nullptr) { // umbriel 0.1.0 (7a448abe550e)
        ffStrbufSubstrAfterFirstC(result, ' ');
        ffStrbufSubstrBeforeLastC(result, '(');
        ffStrbufTrimRightSpace(result);
        return nullptr;
    }

    return "Failed to run command `umbriel --version`";
}

static const char* getWeston(FFstrbuf* result) {
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreate();
    const char* error = ffFindExecutableInPath("weston", &path);
    if (error) {
        return "Failed to find weston executable path";
    }

    if (ffProcessAppendStdOut(result, (char* const[]) { path.chars, "--version", nullptr }) == nullptr) { // weston 8.0.0\n...
        ffStrbufSubstrBeforeFirstC(result, '\n');
        ffStrbufSubstrAfterLastC(result, ' ');
        return nullptr;
    }

    return "Failed to run command `weston --version`";
}

    #ifdef __linux__
static const char* getWslg(FFstrbuf* result) {
    if (!ffAppendFileBuffer("/mnt/wslg/versions.txt", result)) {
        return "Failed to read /mnt/wslg/versions.txt";
    }

    ffStrbufSubstrBeforeFirstC(result, '\n');
    ffStrbufSubstrBeforeFirstC(result, '\r');

    if (ffStrbufStartsWithIgnCaseS(result, "WSLg"))
    {
        ffStrbufSubstrAfterFirstC(result, 'g');
        ffStrbufTrimLeft(result, ' ');

        // Newer WSLg formats include an optional parenthesized descriptor
        // before the actual version token.
        if (ffStrbufStartsWithS(result, "("))
        {
            if (!ffStrbufSubstrAfterFirstC(result, ')'))
                return "Failed to parse WSLg version";
            ffStrbufTrimLeft(result, ' ');
        }

        ffStrbufTrimLeft(result, ':');
        ffStrbufTrimLeft(result, ' ');
    }

    ffStrbufSubstrBeforeFirstC(result, '+');
    ffStrbufTrimRightSpace(result);

    if (result->length == 0)
        return "Failed to parse WSLg version";

    return NULL;
}

const char* ffDetectWMVersion(const FFstrbuf* wmName, FFstrbuf* result, FF_MAYBE_UNUSED FFWMOptions* options)
{
    if (!wmName)
        return "No WM detected";
    }

#if !__ANDROID__
    // Wayland compositors
    if (ffStrbufIgnCaseEqualS(wmName, "Hyprland")) {
        return getHyprland(result);
    }

    if (ffStrbufEqualS(wmName, "sway")) {
        return getSway(result);
    }

    if (ffStrbufEqualS(wmName, "labwc")) {
        return getLabwc(result);
    }

    if (ffStrbufEqualS(wmName, "niri")) {
        return getNiri(result);
    }

    if (ffStrbufEqualS(wmName, "umbriel")) {
        return getUmbriel(result);
    }

    if (ffStrbufEqualS(wmName, "weston")) {
        return getWeston(result);
    }

    #if __linux__
    if (ffStrbufEqualS(wmName, "WSLg")) {
        return getWslg(result);
    }
    #endif
#endif

    // X11 WMs
    if (ffStrbufEqualS(wmName, "i3")) {
        return getI3(result);
    }

    if (ffStrbufEqualS(wmName, "ctwm")) {
        return getCtwm(result);
    }

    if (ffStrbufEqualS(wmName, "fvwm")) {
        return getFvwm(result);
    }

    if (ffStrbufEqualS(wmName, "Openbox")) {
        return getOpenbox(result);
    }

    return "Unsupported WM";
}
