// todo: create test mod files for each format

#include <iostream>

#include "config.h"
#include "commands/extract.h"
#include "util/container.h"

static const std::string usage_str =
    "Usage: utmodloader [options] <command> <path>\n"
    "Options:\n"
    "    -h, --help       Display this help message\n"
    "    -v, --version    Display version information\n"
    "Commands:\n"
    "    extract          Extract mod files. Accepts a directory or a single file path";

int main(const int argc, char* argv[]) {
    const std::vector<std::string_view> args(argv, argv + argc);

    if (args.size() == 1) {
        std::cout << "No command specified.\n\n" << usage_str << std::endl;
        return EXIT_FAILURE;
    }

    // This program only accepts global flags, so we can check here safely
    if (find_in(args, "-h") || find_in(args, "--help")) {
        std::cout << usage_str << std::endl;
        return EXIT_SUCCESS;
    }

    if (find_in(args, "-v") || find_in(args, "--version")) {
        std::cout << PROJECT_NAME << " v" << PROJECT_VERSION << std::endl;
        return EXIT_SUCCESS;
    }

    // Subcommands
    for (std::size_t i = 1; i < args.size(); ++i) {
        if (args[i] == "extract") {
            if (i + 1 < args.size()) {
                return extract_mods(args[i + 1].data());
            }

            throw std::runtime_error("No path specified");
        }
    }

    return EXIT_SUCCESS;
}
