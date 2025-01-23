#ifndef RAR_H
#define RAR_H

#include <filesystem>
#include "../modfile.h"

namespace fs = std::filesystem;

void extract_rar(const ModFile &mod, const fs::path &store_path);

#endif // RAR_H
