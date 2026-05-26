#include "mem/cache/tags/indexing_policies/modulo_set_associative.hh"
#include "mem/cache/replacement_policies/replaceable_entry.hh"

namespace gem5{

ModuloSetAssociative::ModuloSetAssociative(const Params &p)
    : BaseIndexingPolicy(
        p,
        p.size / p.entry_size,
        floorLog2(p.entry_size))
{
}

uint32_t
ModuloSetAssociative::extractSet(const Addr addr) const
{
    Addr blkID = addr >> setShift;
    return blkID % numSets;
}

Addr
ModuloSetAssociative::extractTag(const Addr addr) const
{
    Addr blkID = addr >> setShift;
    return blkID / numSets;
}

Addr
ModuloSetAssociative::regenerateAddr(const Addr &tag,
                                     const ReplaceableEntry* entry) const
{
    return ((tag * numSets) + entry->getSet()) << setShift;
}

std::vector<ReplaceableEntry*>
ModuloSetAssociative::getPossibleEntries(const Addr &addr) const
{
    return sets[extractSet(addr)];
}

} // namespace gem5
