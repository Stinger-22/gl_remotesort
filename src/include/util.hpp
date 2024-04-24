#ifndef REMOTESORT_UTIL_HPP
#define REMOTESORT_UTIL_HPP

#include <string>
#include <ctime>

struct FileInfo
{
    std::string filename;
    std::string extension;
    std::time_t time;
};

std::string remove_extension(const std::string& filename);
int compareByFilename(const FileInfo& left, const FileInfo& right);
int compareByExtension(const FileInfo& left, const FileInfo& right);
int compareByTime(const FileInfo& left, const FileInfo& right);

#endif
