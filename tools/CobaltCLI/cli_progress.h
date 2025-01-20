#ifndef CBLT_CLI_PROGRESS_H
#define CBLT_CLI_PROGRESS_H

namespace cblt::cli {

void printProgress(int percent, const char *status = nullptr);

}  // namespace cblt::cli

#endif  // CBLT_CLI_PROGRESS_H