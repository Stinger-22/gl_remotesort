#include <util.hpp>

#include <string>
#include <cstdlib>
#include <cstdio>
#include <ctime>

int (*comparators[3])(const FileInfo&, const FileInfo&) =
{
	compareByFilename, compareByExtension, compareByTime
};

std::string remove_extension(const std::string& filename)
{
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == std::string::npos) return filename;
    return filename.substr(0, lastdot);
}

int compareByFilename(const FileInfo& left, const FileInfo& right)
{
    return left.filename < right.filename;
}

int compareByExtension(const FileInfo& left, const FileInfo& right)
{
    return left.extension < right.extension;
}

int compareByTime(const FileInfo& left, const FileInfo& right)
{
    return left.time > right.time;
}