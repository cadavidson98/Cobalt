#include "cli_progress.h"

#include <sys/ioctl.h>

#include <cstdio>
#include <cstring>

#include <unistd.h>

namespace cblt::cli {

void printProgress(int percent, const char *status) {
    winsize terminalSize;
    int result = ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminalSize);
    if (result == -1 || percent < 0) {
        return;
    }

    static constexpr int kStatusBarLength = 128;

    char statusBar[kStatusBarLength];
    const int currentLineLength = (terminalSize.ws_col < kStatusBarLength) ? terminalSize.ws_col : kStatusBarLength;

    std::memset(statusBar, 0, kStatusBarLength);
    int statusFormatSize = 0;
    if (percent < 100) {
        statusFormatSize = std::snprintf(statusBar, kStatusBarLength, "[progress %3d%%]: %s ", percent, status);
    } else {
        statusFormatSize = std::snprintf(statusBar, kStatusBarLength, "complete.");
    }

    if (statusFormatSize < 0) {
        return;
    }

    const int remainingChars = currentLineLength - statusFormatSize;
    // reserve 2 characters for newline/carriage return & null character
    const int barLength = remainingChars - 2;
    if (barLength <= 0) {
        return;
    }

    const char endLineCharacter = (percent < 100) ? '\r' : '\n';

    // overwrite null character
    std::memset(statusBar + statusFormatSize, ' ', barLength);
    statusBar[barLength] = endLineCharacter;
    statusBar[barLength + 1] = '\0';

    std::fprintf(stdout, "%s", statusBar);
    std::fflush(stdout);
}

} // namespace cblt::cli
