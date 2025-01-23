#include "zip.h"
#include "../format.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <zip.h>

namespace fs = std::filesystem;
using namespace format;

void extract_zip(const std::string& archive_path, const std::string& archive_name) {
    zip_t *archive = zip_open(archive_path.c_str(), 0, nullptr);
    if (!archive) { std::cerr << red("Failed to open archive at " + archive_path) << std::endl; return; }

    std::cout << underline("\nContents of ") + underline(bold(green(archive_path))) << std::endl;

    const zip_int64_t num_entries = zip_get_num_entries(archive, 0);

    for (int i = 0; i < num_entries; i++) {
        struct zip_stat st{};
        zip_stat_init(&st);

        if (zip_stat_index(archive, i, 0, &st) == 0) {
            std::cout << "- " << gray(st.name) << std::endl;

            // todo: dont hardcode blah blah
            std::string store_path = "/home/kofy/db/CLionProjects/UTModLoader/store";
            std::string file_path = store_path + "/" += archive_name + "/" + st.name;

            auto contents = new char[st.size];
            zip_file *file = zip_fopen_index(archive, i, 0);
            zip_fread(file, contents, st.size);
            zip_fclose(file);

            create_directories(fs::path(file_path).parent_path());

            std::ofstream file_out(file_path, std::ios::out | std::ios::binary);
            file_out.write(contents, static_cast<std::streamsize>(st.size));
            file_out.close();

            delete[] contents;
        }
    }

    std::cout << std::endl;

    zip_close(archive);
}
