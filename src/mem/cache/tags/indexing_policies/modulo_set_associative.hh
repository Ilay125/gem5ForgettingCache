#ifndef __MEM_CACHE_INDEXING_POLICIES_MODULO_SET_ASSOCIATIVE_HH__
#define __MEM_CACHE_INDEXING_POLICIES_MODULO_SET_ASSOCIATIVE_HH__

#include <vector>

#include "mem/cache/tags/indexing_policies/base.hh"
#include "params/ModuloSetAssociative.hh"

namespace gem5{

class ReplaceableEntry;

class ModuloSetAssociative : public BaseIndexingPolicy
{
    /**
     * A modulo set associative indexing policy.
     *
     * The module set associative indexing policy can be used even if the
     * number of sets is not a power of 2.
     * Instead of parsing the address for:
     *  |       tag     |   index   |   offset  |
     * We parse it as:
     *  |       ID      |   offset  |
     *  We can get the index by ID % set num, and the tag by ID / set num
     */
    protected:

        /**
         * Apply a hash function to calculate address set.
         * Make it universal so a power of 2 will not be mandatory.
         * Notice - this is not applicable for real uses but only for simulation
         * purposes.
         *
         * @param addr The address to hash
         * @return The set index.
         */
        virtual uint32_t extractSet(const Addr addr) const;

    public:
        typedef ModuloSetAssociativeParams Params;

        ModuloSetAssociative(const Params &p);
        ~ModuloSetAssociative() {};


        /**
         * Generate tag with new format.
         *
         * @param addr The address to get the tag from.
         * @return The tag of the address.
         */
        virtual Addr extractTag(const Addr addr) const override;

        /**
         * Find all possible entries for insertion and replacement of an address.
         * Should be called immediately before ReplacementPolicy's findVictim()
         * not to break cache resizing.
         * Returns entries in all ways belonging to the set of the address.
         *
         * @param addr The addr to a find possible entries for.
         * @return The possible entries.
         */
        std::vector<ReplaceableEntry*> getPossibleEntries(const Addr &addr) const
                                                                        override;

        /**
         * Regenerates an entry's address from this tag and index format.
         *
         * @param tag the tag.
         * @param entry the entry.
         * @return the entry's original addr value.
         */
        Addr regenerateAddr (const Addr &tag,
                             const ReplaceableEntry* entry) const override;
};

} // namespace gem5

#endif // __MEM_CACHE_INDEXING_POLICIES_MODULO_SET_ASSOCIATIVE_HH__
