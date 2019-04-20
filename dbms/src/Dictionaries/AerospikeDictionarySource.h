#pragma once

#include <Common/config.h>
#if USE_AEROSPIKE

#    include "DictionaryStructure.h"
#    include "IDictionarySource.h"
#    include <aerospike/aerospike.h>

namespace DB
{
/// Allows loading dictionaries from Aerospike collection
class AerospikeDictionarySource final : public IDictionarySource
{
public:
    AerospikeDictionarySource(
        const DictionaryStructure & dict_struct,
        as_config * config,
        const Block & sample_block);

    AerospikeDictionarySource(const AerospikeDictionarySource & other);

    ~AerospikeDictionarySource() override;

    BlockInputStreamPtr loadAll() override;

    BlockInputStreamPtr loadUpdatedAll() override
    {
        throw Exception{"Method loadUpdatedAll is unsupported for AerospikeDictionarySource", ErrorCodes::NOT_IMPLEMENTED};
    }

    bool supportsSelectiveLoad() const override { return true; }

    BlockInputStreamPtr loadIds(const std::vector<UInt64> & ids) override;

    BlockInputStreamPtr loadKeys(const Columns & key_columns, const std::vector<size_t> & requested_rows) override;

    /// @todo: for Aerospike, modification date can somehow be determined from the `_id` object field
    bool isModified() const override { return true; }

    ///Not yet supported
    bool hasUpdateField() const override { return false; }

    DictionarySourcePtr clone() const override { return std::make_unique<AerospikeDictionarySource>(*this); }

    std::string toString() const override;
private:
    const DictionaryStructure dict_struct;
    std::string host; // think how to save const here
    UInt16 port; // think how to save const here
    Block sample_block;
    aerospike client; // may be use ptr here
    // may be save global error variable
};

}

#endif
