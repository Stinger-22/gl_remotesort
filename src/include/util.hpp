#ifndef REMOTESORT_UTIL_HPP
#define REMOTESORT_UTIL_HPP

#include <string>
#include <ctime>

enum class SortBy : int
{
    NAME = 0,
    TYPE = 1,
    DATE = 2
};

enum class SortingResult : int
{
    FAILURE_WRONG_SORT_TYPE = -3,
    FAILURE_PATH_IS_NOT_DIRECTORY = -2,
    FAILURE = -1,
    SUCCESS = 0
};

struct FileInfo
{
    std::string filename;
    std::string extension;
    std::time_t time;
};

extern int (*comparators[3])(const FileInfo&, const FileInfo&);

int compareByFilename(const FileInfo& left, const FileInfo& right);
int compareByExtension(const FileInfo& left, const FileInfo& right);
int compareByTime(const FileInfo& left, const FileInfo& right);

std::time_t getFileLastWriteTime(const char *file);

std::string getHostName();

#endif
