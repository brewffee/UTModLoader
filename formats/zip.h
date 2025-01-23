#ifndef ZIP_H
#define ZIP_H

#include <filesystem>
#include "../modfile.h"

namespace fs = std::filesystem;

void extract_zip(const ModFile &mod, const fs::path &store_path);

#endif // ZIP_H
