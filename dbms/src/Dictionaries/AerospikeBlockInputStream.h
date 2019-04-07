#pragma once

#include <aerospike/aerospike.h>
# include <aerospike/as_key.h>
#include <Core/Block.h>
#include <DataStreams/IBlockInputStream.h>
#include <Core/ExternalResultDescription.h>

namespace DB
{
/// Converts MongoDB Cursor to a stream of Blocks
    class AerospikeBlockInputStream final : public IBlockInputStream
    {
    public:
        AerospikeBlockInputStream(
            const aerospike& client,
            std::vector<as_key> keys,
            const Block & sample_block,
            const size_t max_block_size);

        ~AerospikeBlockInputStream() override;

        String getName() const override { return "Aerospike"; }

        Block getHeader() const override { return description.sample_block.cloneEmpty(); }

    private:
        Block readImpl() override;

        size_t cursor = 0;
        aerospike client;
        std::vector<as_key> keys;
        const size_t max_block_size;
        ExternalResultDescription description;
        bool all_read = false;
    };

}
