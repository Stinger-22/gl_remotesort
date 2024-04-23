#ifndef REMOTESORT_NETWORKING_HPP
#define REMOTESORT_NETWORKING_HPP

#include <string>

struct SortingRequest
{
	enum SortBy
	{
		NAME,
		TYPE,
		DATE
	};

	std::string startFolder;
	SortBy sortBy;
};

struct SortingResponse
{
	enum SortingResult
	{
		SUCCESS,
		FAILURE,
		FAILURE_ACCESS_DENIED,

	};
	SortingResult result;
	int numberOfFiles;
};

// Send with buffer of 4096

// Client sends request.
// Server responds with Failure:
// 		Client stops execution.
// Server responds with Success:
// 		Client receives number of files.

#endif