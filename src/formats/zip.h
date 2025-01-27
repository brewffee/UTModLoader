#ifndef ZIP_H
#define ZIP_H

#include <filesystem>
#include "../modfile.h"

/**
 * Extracts the contents of a ZIP archive to a specified directory.
 *
 * @param mod The ModFile object containing the ZIP archive to extract.
 * @param store_path The directory where the extracted files will be stored.
 *
 * @return 0 on success, non-zero on failure.
 */
int extract_zip(const ModFile &mod, const std::filesystem::path &store_path);

#endif // ZIP_H
