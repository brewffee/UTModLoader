#ifndef RAR_H
#define RAR_H

#include <filesystem>
#include "../modfile.h"

/**
 * Extracts the contents of a RAR archive to a specified directory.
 *
 * @param mod The ModFile object containing the RAR archive to extract.
 * @param store_path The directory where the extracted files will be stored.
 *
 * @return 0 on success, non-zero on failure.
 */
int extract_rar(const ModFile &mod, const std::filesystem::path &store_path);

#endif // RAR_H
