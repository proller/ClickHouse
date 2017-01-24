#pragma once

#include <map>
#include <DB/Core/Types.h>


namespace DB
{
using StringRange = std::pair<const char *, const char *>;
using StringPtr = std::shared_ptr<String>;


inline String toString(const StringRange & range)
{
	return String(range.first, range.second);
}
}
