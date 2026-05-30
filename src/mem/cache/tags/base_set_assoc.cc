/**
 * @file
 * Definitions of a conventional tag store.
 */

#include "mem/cache/tags/base_set_assoc.hh"

#include <string>
#include <algorithm>

#include "base/intmath.hh"

namespace gem5
{

BaseSetAssoc::BaseSetAssoc(const Params &p)
    :BaseTags(p), allocAssoc(p.assoc), blks(p.size / p.block_size),
     sequentialAccess(p.sequential_access),
     replacementPolicy(p.replacement_policy),
     drt(p.drt)
{
    // There must be a indexing policy
    fatal_if(!p.indexing_policy, "An indexing policy is required");

    // Check parameters
    if (blkSize < 4 || !isPowerOf2(blkSize)) {
        fatal("Block size must be at least 4 and a power of 2");
    }
}

void
BaseSetAssoc::tagsInit()
{
    // Initialize all blocks
    for (unsigned blk_index = 0; blk_index < numBlocks; blk_index++) {
        // Locate next cache block
        CacheBlk* blk = &blks[blk_index];

        // Link block to indexing policy
        indexingPolicy->setEntry(blk, blk_index);

        // Associate a data chunk to the block
        blk->data = &dataBlks[blkSize*blk_index];

        // Associate a replacement data entry to the block
        blk->replacementData = replacementPolicy->instantiateEntry();

        // This is not used as of now but we set it for security
        blk->registerTagExtractor(genTagExtractor(indexingPolicy));
    }
}

void
BaseSetAssoc::invalidate(CacheBlk *blk)
{
    // Notify partitioning policies of release of ownership
    if (partitionManager) {
        partitionManager->notifyRelease(blk->getPartitionId());
    }

    BaseTags::invalidate(blk);

    // Decrease the number of tags in use
    stats.tagsInUse--;

    // Invalidate replacement data
    replacementPolicy->invalidate(blk->replacementData);
}

void
BaseSetAssoc::moveBlock(CacheBlk *src_blk, CacheBlk *dest_blk)
{
    BaseTags::moveBlock(src_blk, dest_blk);

    // Since the blocks were using different replacement data pointers,
    // we must touch the replacement data of the new entry, and invalidate
    // the one that is being moved.
    replacementPolicy->invalidate(src_blk->replacementData);
    replacementPolicy->reset(dest_blk->replacementData);
}

CacheBlk*
BaseSetAssoc::getMRU(int set)
{
    CacheBlk* mru = nullptr;
    Tick latest_tick = 0;

    for (int way = 0; way < allocAssoc; way++) {
        // Get the block in the required set and way
        CacheBlk* blk = &blks.at(way + set * allocAssoc);

        if (blk->isValid()) {
            // Get the MRU metadata (latest tick changed)
            auto data = std::static_pointer_cast<replacement_policy::MRU::MRUReplData>(
                blk->replacementData);

            // Changed current block later than saved one
            if (data && data->lastTouchTick >= latest_tick) {
                mru = blk;
                latest_tick = data->lastTouchTick;
            }
        }
    }

    return mru;
}

std::vector<CacheBlk*>
BaseSetAssoc::getNTopMRU(int set, int n)
{
    fatal_if(n > allocAssoc, "MRU N top can't be greater than assoc");

    std::vector<CacheBlk*> set_blks;

    // Fill the vector with the entire set
    for (int way = 0; way < allocAssoc; way++) {
        set_blks.push_back(&blks.at(way + set * allocAssoc));
    }

    // Sort them according to lastTouchTick
    std::sort(set_blks.begin(), set_blks.end(), [](CacheBlk* a, CacheBlk* b){
        auto data_a = std::static_pointer_cast<replacement_policy::MRU::MRUReplData>(
                a->replacementData);
        auto data_b = std::static_pointer_cast<replacement_policy::MRU::MRUReplData>(
                b->replacementData);
        return data_a->lastTouchTick > data_b->lastTouchTick;
    });

    // Get first N
    if (set_blks.size() > n) {
        set_blks.resize(n);
    }

    return set_blks;
}

std::vector<CacheBlk *>
BaseSetAssoc::getSetBlks(int set)
{
    std::vector<CacheBlk *> set_blks;

    // Fill the vector with the entire set
    for (int way = 0; way < allocAssoc; way++) {
        set_blks.push_back(&blks.at(way + set * allocAssoc));
    }

    return set_blks;
}
} // namespace gem5
