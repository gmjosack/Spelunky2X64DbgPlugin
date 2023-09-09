#include "Data/EntityDB.h"
#include "Configuration.h"
#include "Spelunky2.h"
#include "pluginmain.h"

S2Plugin::EntityDB::EntityDB(Configuration* config) : MemoryMappedData(config) {}

bool S2Plugin::EntityDB::loadEntityDB()
{
    auto afterBundle = mConfiguration->spelunky2()->spelunky2AfterBundle();
    if (afterBundle == 0)
    {
        return false;
    }
    if (mEntityDBPtr != 0)
    {
        return true;
    }

    mEntityList = std::make_unique<EntityList>(mConfiguration->spelunky2());

    mMemoryOffsets.clear();
    auto instructionEntitiesPtr = Script::Pattern::FindMem(afterBundle, mConfiguration->spelunky2()->spelunky2AfterBundleSize(), "A4 84 E4 CA DA BF 4E 83");
    auto entitiesPtr = instructionEntitiesPtr - 33 + 7 + (duint)Script::Memory::ReadDword(instructionEntitiesPtr - 30);
    mEntityDBPtr = Script::Memory::ReadQword(entitiesPtr);

    auto offset = mEntityDBPtr;

    for (auto x = 0; x < mEntityList->highestID() + 1; ++x)
    {
        std::unordered_map<std::string, size_t> offsets;
        for (const auto& field : mConfiguration->typeFields(MemoryFieldType::EntityDB))
        {
            offset = setOffsetForField(field, "EntityDB." + field.name, offset, offsets);
        }
        mMemoryOffsets.emplace_back(offsets);
    }
    return true;
}

S2Plugin::EntityList* S2Plugin::EntityDB::entityList() const noexcept
{
    return mEntityList.get();
}

std::unordered_map<std::string, size_t>& S2Plugin::EntityDB::offsetsForIndex(uint32_t entityDBIndex)
{
    return mMemoryOffsets.at(entityDBIndex);
}

void S2Plugin::EntityDB::reset()
{
    mEntityDBPtr = 0;
}
