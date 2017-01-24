#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace DB
{
using Names = std::vector<std::string>;
using NameSet = std::unordered_set<std::string>;
using NameToNameMap = std::unordered_map<std::string, std::string>;
}
