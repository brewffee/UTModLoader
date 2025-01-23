#include "rar.h"
#include "../format.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <unarr.h>

namespace fs = std::filesystem;
using namespace format;

void extract_rar(const std::string& archive_path, const std::string& archive_name) {
    ar_stream *st = ar_open_file(archive_path.c_str());
    if (!st) { std::cerr << red("Failed to open file at " + archive_path) << std::endl; return; }

    std::cout << underline("\nContents of ") + underline(bold(green(archive_path))) << std::endl;

    ar_archive *ar = ar_open_rar_archive(st);
    if (!ar) { std::cerr << red("Failed to open archive at " + archive_path) << std::endl; return; }

    while (ar_parse_entry(ar)) {
        std::cout << "parsing entry" << std::endl;
        size_t size = ar_entry_get_size(ar);
        const char *name = ar_entry_get_name(ar);
        if (name) {
            std::cout << "- " << gray(name) << std::endl;
        }

        // todo: bad !!!
        std::string store_path = "/home/kofy/db/CLionProjects/UTModLoader/store";
        std::string file_path = store_path + "/" += archive_name + "/" + name;

        while (size > 0) {
            const auto contents = new char[size];
            if (!ar_entry_uncompress(ar, contents, size)) break;

            create_directories(fs::path(file_path).parent_path());

            std::ofstream file_out(file_path, std::ios::out | std::ios::binary);
            file_out.write(contents, static_cast<std::streamsize>(size));
            file_out.close();

            delete[] contents;
            size -= size;
        }

        if (size > 0) {
            std::cerr << "Failed to decompress file: " << name << std::endl;
        }
    }

    ar_close_archive(ar);
    ar_close(st);
}
