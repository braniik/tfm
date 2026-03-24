#include "app/app.hpp"

#include <cstdlib>
#include <string>

int main(int argc, char* argv[]) {
    std::string cd_file;

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--cd-file" && i + 1 < argc) {
            cd_file = argv[++i];
        }
    }

    if (cd_file.empty()) {
        if (const char* env = std::getenv("TFM_CD_FILE"))
            cd_file = env;
    }

    run_app(cd_file);
    return 0;
}
