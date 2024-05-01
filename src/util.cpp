#include <util.hpp>

#include <climits>
#include <unistd.h>
#include <sys/stat.h>

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

std::string getHostName()
{
    char hostname[HOST_NAME_MAX + 1] = {0};
    gethostname(hostname, HOST_NAME_MAX);
    return hostname;
}

std::time_t getFileLastWriteTime(const char *file)
{
    struct stat attr;
    stat(file, &attr);
    return attr.st_mtime;
}