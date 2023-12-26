#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    class GameManager
    {
      public:
        bool loadGameManager();

        std::unordered_map<std::string, size_t>& offsets();
        void refreshOffsets();
        size_t offsetForField(const std::string& fieldName) const;

        void reset();

        size_t saveGameOffset();

      private:
        size_t mGameManagerPtr = 0;
        size_t mSaveGamePtr = 0;
        std::unordered_map<std::string, size_t> mMemoryOffsets; // fieldname -> offset of field value in memory
    };
} // namespace S2Plugin
