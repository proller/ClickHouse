#pragma once

#include <utility>
#include <DB/DataTypes/IDataType.h>

namespace DB
{
namespace DataTypeTraits
{
	/// This type is declared in a separate header in order to increase
	/// compilation speed.
	using EnrichedDataTypePtr = std::pair<DataTypePtr, DataTypePtr>;
}
}
