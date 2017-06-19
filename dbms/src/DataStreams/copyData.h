#pragma once

#include <atomic>


namespace DB
{

class IBlockInputStream;
class IBlockOutputStream;

/** Copies data from the InputStream into the OutputStream
  * (for example, from the database to the console, etc.)
  */
void copyData(IBlockInputStream & from, IBlockOutputStream & to, std::atomic<bool> * is_cancelled = nullptr);

}
