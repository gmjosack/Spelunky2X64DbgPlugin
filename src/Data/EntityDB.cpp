#include "Data/EntityDB.h"
#include "Configuration.h"

uintptr_t S2Plugin::EntityDB::offsetForIndex(uint32_t idx) const
{
    if (ptr == 0)
        return 0;

    auto getEntityDBSize = []()
    {
        auto& fields = Configuration::get()->typeFields(MemoryFieldType::EntityDB);
        size_t size = 0;
        for (auto& field : fields)
        {
            size += field.get_size();
        }
        return size;
    };
    // [Known Issue]: Static value, have to restart programm for size to update
    static size_t entityDBRecordSize = getEntityDBSize();

    return ptr + idx * entityDBRecordSize;
}
