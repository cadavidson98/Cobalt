#include "cli_progress.h"

#include <cstdio>
#include <cstring>
#include <sys/ioctl.h>
#include <unistd.h>

namespace cblt::cli {

void printProgress(int percent, const char *status) {
    winsize terminalSize;
    int result = ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminalSize);
    if (result == -1 || percent < 0) {
        return;
    }

    static constexpr int kStatusBarLength = 64;
    static constexpr int kMaxProgressBarLength = 2048;

    char statusBar[kStatusBarLength];
    char progressBar[kMaxProgressBarLength];
    const int currentLineLength = (terminalSize.ws_col < kMaxProgressBarLength) ? terminalSize.ws_col : kMaxProgressBarLength;

    std::memset(statusBar, 0, kStatusBarLength);
    std::memset(progressBar, 0, kMaxProgressBarLength);
    const int statusFormatSize = std::snprintf(statusBar, kStatusBarLength, "[progress %3d]: ", percent);
    if (statusFormatSize < 0) {
        return;
    }

    // progress bar format: [###......]
    const int remainingChars = currentLineLength - statusFormatSize;
    // reserve 2 characters for newline/carriage return & null character
    const int barLength = remainingChars - 2;
    if (barLength <= 0) {
        return;
    }

    if (percent >= 100) {
        const char *completeMessage = "complete.";
        const size_t completeMessageLength = std::strlen(completeMessage);
        std::strncpy(progressBar, "complete.", kMaxProgressBarLength);
        // overwrite null character
        std::memset(progressBar + completeMessageLength, ' ', barLength - completeMessageLength);
        progressBar[barLength] = '\n';
        progressBar[barLength + 1] = '\0';
    } else {
        const int fillLength = barLength - 2;
        const int numFilled = (percent * fillLength) / 100;
        const int numEmpty = fillLength - numFilled;
        progressBar[0] = '[';
        std::memset(progressBar + 1, '#', numFilled);
        std::memset(progressBar + 1 + numFilled, '.', numEmpty);
        progressBar[barLength - 1] = ']';
        progressBar[barLength] = '\r';
        progressBar[barLength + 1] = '\0';
    }

    std::fprintf(stdout, "%s%s", statusBar, progressBar);
    std::fflush(stdout);
}

}  // namespace cblt::cli