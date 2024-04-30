#include <util.hpp>

int (*comparators[3])(const FileInfo&, const FileInfo&) =
{
	compareByFilename, compareByExtension, compareByTime
};

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