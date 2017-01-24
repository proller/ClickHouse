#pragma once

#include <functional>
#include <zkutil/ZooKeeper.h>

namespace zkutil
{
using GetZooKeeper = std::function<ZooKeeperPtr()>;
}
