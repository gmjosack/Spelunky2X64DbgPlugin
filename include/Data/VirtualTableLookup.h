#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace S2Plugin
{
    struct Configuration;

    struct VirtualTableEntry
    {
        size_t value;
        size_t offset; // offset in table
        bool isValidAddress;

        bool isAutoSymbol = false; // whether the symbol name was added from x64dbg
        std::unordered_set<std::string> symbols;

        void addSymbol(const std::string& s);
    };

    class VirtualTableLookup
    {
      public:
        explicit VirtualTableLookup(Configuration* config);
        bool loadTable();

        const VirtualTableEntry& entryForOffset(size_t tableOffset);
        std::unordered_set<uint32_t> tableOffsetForFunctionAddress(size_t functionAddress);
        VirtualTableEntry findPrecedingEntryWithSymbols(size_t tableOffset);
        size_t tableAddressForEntry(const VirtualTableEntry& entry);

        void setSymbolNameForOffsetAddress(size_t offsetAddress, const std::string& name);

        size_t count() const noexcept;
        size_t tableStartAddress() const noexcept;
        void reset();

      private:
        Configuration* mConfiguration;
        std::unordered_map<uint32_t, VirtualTableEntry> mOffsetToTableEntries;
        size_t mTableStartAddress = 0;
    };
} // namespace S2Plugin
