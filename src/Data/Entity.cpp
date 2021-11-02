#include "Data/Entity.h"
#include "Configuration.h"
#include "pluginmain.h"
#include <regex>

S2Plugin::Entity::Entity(size_t offset, TreeViewMemoryFields* tree, WidgetMemoryView* memoryView, WidgetMemoryView* comparisonMemoryView, EntityDB* entityDB, S2Plugin::Configuration* config)
    : MemoryMappedData(config), mEntityPtr(offset), mTree(tree), mMemoryView(memoryView), mComparisonMemoryView(comparisonMemoryView)
{
    auto entityID = config->spelunky2()->getEntityTypeID(offset);
    mEntityName = config->spelunky2()->getEntityName(offset, entityDB);
    for (const auto& [regexStr, entityClassType] : mConfiguration->defaultEntityClassTypes())
    {
        auto r = std::regex(regexStr);
        if (std::regex_match(mEntityName, r))
        {
            mEntityType = entityClassType;
            break;
        }
    }
}

void S2Plugin::Entity::refreshOffsets()
{
    mMemoryOffsets.clear();
    auto offset = mEntityPtr;
    auto comparisonOffset = mComparisonEntityPtr;
    auto hierarchy = classHierarchy();
    for (auto c : hierarchy)
    {
        auto headerIdentifier = c;
        MemoryField headerField;
        headerField.name = "<b>" + c + "</b>";
        headerField.type = MemoryFieldType::EntitySubclass;
        headerField.jsonName = c;
        offset = setOffsetForField(headerField, headerIdentifier, offset, mMemoryOffsets, false);
        for (const auto& field : mConfiguration->typeFieldsOfEntitySubclass(c))
        {
            offset = setOffsetForField(field, headerIdentifier + "." + field.name, offset, mMemoryOffsets);
        }

        if (mComparisonEntityPtr != 0)
        {
            MemoryField comparisonHeaderField;
            comparisonHeaderField.name = "<b>Comparison" + c + "</b>";
            comparisonHeaderField.type = MemoryFieldType::EntitySubclass;
            comparisonHeaderField.jsonName = "comparison." + c;

            comparisonOffset = setOffsetForField(comparisonHeaderField, "comparison." + headerIdentifier, comparisonOffset, mMemoryOffsets, false);
            for (const auto& field : mConfiguration->typeFieldsOfEntitySubclass(c))
            {
                comparisonOffset = setOffsetForField(field, "comparison." + headerIdentifier + "." + field.name, comparisonOffset, mMemoryOffsets);
            }
        }
    }
}

void S2Plugin::Entity::refreshValues()
{
    auto offset = mEntityPtr;
    auto hierarchy = classHierarchy();

    // if there's a pointer in the list of fields of the entity subclass hierarchy then
    // refresh all the offsets, as the pointer value may have changed, so in order to show
    // the correct values of the pointer contents, we need to set the new offset
    bool pointerFieldFound = false;
    for (const auto& c : hierarchy)
    {
        for (const auto& field : mConfiguration->typeFieldsOfEntitySubclass(c))
        {
            if (field.type == MemoryFieldType::PointerType)
            {
                pointerFieldFound = true;
                break;
            }
        }
    }
    if (pointerFieldFound)
    {
        refreshOffsets();
    }

    // now update all the values in the treeview
    auto deltaReference = mMemoryOffsets.at("Entity.__vftable");
    for (const auto& c : hierarchy)
    {
        MemoryField headerField;
        headerField.name = "<b>" + c + "</b>";
        headerField.type = MemoryFieldType::EntitySubclass;
        headerField.jsonName = c;
        mTree->updateValueForField(headerField, c, mMemoryOffsets, deltaReference);
    }
}

void S2Plugin::Entity::populateTreeView()
{
    mTreeViewSectionItems.clear();
    auto hierarchy = classHierarchy();
    uint8_t counter = 0;
    for (auto c : hierarchy)
    {
        MemoryField headerField;
        headerField.name = "<b>" + c + "</b>";
        headerField.type = MemoryFieldType::EntitySubclass;
        headerField.jsonName = c;
        auto item = mTree->addMemoryField(headerField, c);
        mTreeViewSectionItems[c] = item;
        if (++counter == hierarchy.size())
        {
            mTree->expandItem(item);
        }
    }
    mTree->setColumnHidden(gsColComparisonValue, true);
    mTree->setColumnHidden(gsColComparisonValueHex, true);
}

void S2Plugin::Entity::populateMemoryView()
{
    static const std::vector<QColor> colors = {QColor(255, 214, 222), QColor(232, 206, 227), QColor(199, 186, 225), QColor(187, 211, 236), QColor(236, 228, 197), QColor(193, 219, 204)};
    mTotalMemorySize = 0;
    mMemoryView->clearHighlights();
    auto hierarchy = classHierarchy();
    uint8_t colorIndex = 0;
    for (auto c : hierarchy)
    {
        auto fields = mConfiguration->typeFieldsOfEntitySubclass(c);
        for (const auto& field : fields)
        {
            highlightField(field, c + "." + field.name, colors.at(colorIndex));
        }
        colorIndex++;
        if (colorIndex >= colors.size())
        {
            colorIndex = 0;
        }
    }
}

void S2Plugin::Entity::highlightField(MemoryField field, const std::string& fieldNameOverride, const QColor& color)
{
    uint8_t fieldSize = 0;
    switch (field.type)
    {
        case MemoryFieldType::Flag:
            break;
        case MemoryFieldType::Skip:
        case MemoryFieldType::UTF16StringFixedSize:
        case MemoryFieldType::UTF8StringFixedSize:
            fieldSize = field.extraInfo;
            break;
        case MemoryFieldType::Bool:
        case MemoryFieldType::Byte:
        case MemoryFieldType::UnsignedByte:
        case MemoryFieldType::Flags8:
        case MemoryFieldType::State8:
        case MemoryFieldType::CharacterDBID:
            fieldSize = 1;
            break;
        case MemoryFieldType::Word:
        case MemoryFieldType::UnsignedWord:
        case MemoryFieldType::Flags16:
        case MemoryFieldType::State16:
        case MemoryFieldType::UTF16Char:
            fieldSize = 2;
            break;
        case MemoryFieldType::Dword:
        case MemoryFieldType::UnsignedDword:
        case MemoryFieldType::Float:
        case MemoryFieldType::Flags32:
        case MemoryFieldType::State32:
        case MemoryFieldType::ParticleDBID:
        case MemoryFieldType::EntityDBID:
        case MemoryFieldType::EntityUID:
        case MemoryFieldType::TextureDBID:
        case MemoryFieldType::StringsTableID:
            fieldSize = 4;
            break;
        case MemoryFieldType::CodePointer:
        case MemoryFieldType::DataPointer:
        case MemoryFieldType::EntityDBPointer:
        case MemoryFieldType::TextureDBPointer:
        case MemoryFieldType::LevelGenPointer:
        case MemoryFieldType::EntityPointer:
        case MemoryFieldType::EntityUIDPointer:
        case MemoryFieldType::ParticleDBPointer:
        case MemoryFieldType::Qword:
        case MemoryFieldType::UnsignedQword:
        case MemoryFieldType::ConstCharPointerPointer:
        case MemoryFieldType::ConstCharPointer:
        case MemoryFieldType::VirtualFunctionTable:
            fieldSize = 8;
            break;
        case MemoryFieldType::PointerType:
        {
            fieldSize = 8;
            break;
        }
        case MemoryFieldType::InlineStructType:
        {
            for (const auto& f : mConfiguration->typeFieldsOfInlineStruct(field.jsonName))
            {
                highlightField(f, fieldNameOverride + "." + f.name, color);
            }
            break;
        }
        case MemoryFieldType::EntitySubclass:
        {
            for (const auto& f : mConfiguration->typeFieldsOfEntitySubclass(field.jsonName))
            {
                highlightField(f, fieldNameOverride + "." + f.name, color);
            }
            break;
        }
        default:
        {
            for (const auto& f : mConfiguration->typeFields(field.type))
            {
                highlightField(f, fieldNameOverride + "." + f.name, color);
            }
            break;
        }
    }
    if (fieldSize > 0)
    {
        mMemoryView->addHighlightedField(fieldNameOverride, mMemoryOffsets.at(fieldNameOverride), fieldSize, color);
    }
    mTotalMemorySize += fieldSize;
}

void S2Plugin::Entity::highlightComparisonField(MemoryField field, const std::string& fieldNameOverride)
{
    uint8_t fieldSize = 0;
    bool isDifferent = false;
    switch (field.type)
    {
        case MemoryFieldType::Flag:
        case MemoryFieldType::Skip:
        case MemoryFieldType::UTF16StringFixedSize:
        case MemoryFieldType::UTF8StringFixedSize:
            break;
        case MemoryFieldType::Bool:
        case MemoryFieldType::Byte:
        case MemoryFieldType::UnsignedByte:
        case MemoryFieldType::Flags8:
        case MemoryFieldType::State8:
        case MemoryFieldType::CharacterDBID:
            isDifferent = Script::Memory::ReadByte(mMemoryOffsets.at(fieldNameOverride)) != Script::Memory::ReadByte(mMemoryOffsets.at("comparison." + fieldNameOverride));
            fieldSize = 1;
            break;
        case MemoryFieldType::Word:
        case MemoryFieldType::UnsignedWord:
        case MemoryFieldType::Flags16:
        case MemoryFieldType::State16:
        case MemoryFieldType::UTF16Char:
            isDifferent = Script::Memory::ReadWord(mMemoryOffsets.at(fieldNameOverride)) != Script::Memory::ReadWord(mMemoryOffsets.at("comparison." + fieldNameOverride));
            fieldSize = 2;
            break;
        case MemoryFieldType::Dword:
        case MemoryFieldType::UnsignedDword:
        case MemoryFieldType::Float:
        case MemoryFieldType::Flags32:
        case MemoryFieldType::State32:
        case MemoryFieldType::ParticleDBID:
        case MemoryFieldType::EntityDBID:
        case MemoryFieldType::EntityUID:
        case MemoryFieldType::TextureDBID:
        case MemoryFieldType::StringsTableID:
            isDifferent = Script::Memory::ReadDword(mMemoryOffsets.at(fieldNameOverride)) != Script::Memory::ReadDword(mMemoryOffsets.at("comparison." + fieldNameOverride));
            fieldSize = 4;
            break;
        case MemoryFieldType::CodePointer:
        case MemoryFieldType::DataPointer:
        case MemoryFieldType::EntityDBPointer:
        case MemoryFieldType::TextureDBPointer:
        case MemoryFieldType::LevelGenPointer:
        case MemoryFieldType::EntityPointer:
        case MemoryFieldType::EntityUIDPointer:
        case MemoryFieldType::ParticleDBPointer:
        case MemoryFieldType::VirtualFunctionTable:
        case MemoryFieldType::Qword:
        case MemoryFieldType::UnsignedQword:
        case MemoryFieldType::ConstCharPointerPointer:
        case MemoryFieldType::ConstCharPointer:
            isDifferent = Script::Memory::ReadQword(mMemoryOffsets.at(fieldNameOverride)) != Script::Memory::ReadQword(mMemoryOffsets.at("comparison." + fieldNameOverride));
            fieldSize = 8;
            break;
        case MemoryFieldType::PointerType:
        {
            isDifferent = Script::Memory::ReadQword(mMemoryOffsets.at(fieldNameOverride)) != Script::Memory::ReadQword(mMemoryOffsets.at("comparison." + fieldNameOverride));
            fieldSize = 8;
            break;
        }
        case MemoryFieldType::InlineStructType:
        {
            for (const auto& f : mConfiguration->typeFieldsOfInlineStruct(field.jsonName))
            {
                highlightComparisonField(f, fieldNameOverride + "." + f.name);
            }
            break;
        }
        case MemoryFieldType::EntitySubclass:
        {
            for (const auto& f : mConfiguration->typeFieldsOfEntitySubclass(field.jsonName))
            {
                highlightComparisonField(f, fieldNameOverride + "." + f.name);
            }
            break;
        }
        default:
        {
            for (const auto& f : mConfiguration->typeFields(field.type))
            {
                highlightComparisonField(f, fieldNameOverride + "." + f.name);
            }
            break;
        }
    }
    if (fieldSize > 0 && isDifferent)
    {
        static const auto color = QColor(255, 221, 184);
        mComparisonMemoryView->addHighlightedField(fieldNameOverride, mMemoryOffsets.at("comparison." + fieldNameOverride), fieldSize, color);
    }
}

void S2Plugin::Entity::interpretAs(const std::string& classType)
{
    mEntityType = classType;
    mTree->clear();
    populateTreeView();
    refreshOffsets();
    refreshValues();
    populateMemoryView();
    mTree->updateTableHeader();
}

std::deque<std::string> S2Plugin::Entity::classHierarchy() const
{
    auto ech = mConfiguration->entityClassHierarchy();
    std::deque<std::string> hierarchy;
    auto t = mEntityType;
    while (t != "Entity")
    {
        hierarchy.push_front(t);
        if (ech.count(t) == 0)
        {
            dprintf("unknown key requested in Entity::classHierarchy() (t=%s)\n", t.c_str());
        }
        t = ech.at(t);
    }
    hierarchy.push_front("Entity");
    return hierarchy;
}

size_t S2Plugin::Entity::findEntityByUID(uint32_t uidToSearch, State* state)
{
    auto searchUID = [state](uint32_t uid, size_t layerOffset) -> size_t
    {
        auto entityCount = (std::min)(Script::Memory::ReadDword(layerOffset + 28), 10000u);
        auto entities = Script::Memory::ReadQword(layerOffset + 8);

        for (auto x = 0; x < entityCount; ++x)
        {
            auto entityPtr = entities + (x * sizeof(size_t));
            auto entity = Script::Memory::ReadQword(entityPtr);
            auto entityUid = Script::Memory::ReadDword(entity + 56);
            if (entityUid == uid)
            {
                return entity;
            }
        }
        return 0;
    };
    auto layer = Script::Memory::ReadQword(state->offsetForField("layer0"));
    auto result = searchUID(uidToSearch, layer);
    if (result != 0)
    {
        return result;
    }

    layer = Script::Memory::ReadQword(state->offsetForField("layer1"));
    result = searchUID(uidToSearch, layer);
    if (result != 0)
    {
        return result;
    }
    return 0;
}

size_t S2Plugin::Entity::totalMemorySize() const noexcept
{
    return mTotalMemorySize;
}

size_t S2Plugin::Entity::memoryOffset() const noexcept
{
    return mEntityPtr;
}

uint32_t S2Plugin::Entity::uid() const noexcept
{
    return Script::Memory::ReadDword(mEntityPtr + 56);
}

uint32_t S2Plugin::Entity::comparisonUid() const noexcept
{
    return Script::Memory::ReadDword(mComparisonEntityPtr + 56);
}

uint8_t S2Plugin::Entity::cameraLayer() const noexcept
{
    return Script::Memory::ReadByte(mMemoryOffsets.at("Entity.layer"));
}

uint8_t S2Plugin::Entity::comparisonCameraLayer() const noexcept
{
    return Script::Memory::ReadByte(mMemoryOffsets.at("comparison.Entity.layer"));
}

void S2Plugin::Entity::label() const
{
    for (const auto& [fieldName, offset] : mMemoryOffsets)
    {
        DbgSetAutoLabelAt(offset, (mEntityName + "." + fieldName).c_str());
    }
}

void S2Plugin::Entity::compareToEntity(size_t comparisonOffset)
{
    mComparisonEntityPtr = comparisonOffset;
}

size_t S2Plugin::Entity::comparedEntityMemoryOffset() const noexcept
{
    return mComparisonEntityPtr;
}

void S2Plugin::Entity::updateComparedMemoryViewHighlights()
{
    mComparisonMemoryView->clearHighlights();
    if (mComparisonEntityPtr != 0)
    {
        auto hierarchy = classHierarchy();
        for (auto c : hierarchy)
        {
            auto fields = mConfiguration->typeFieldsOfEntitySubclass(c);
            for (const auto& field : fields)
            {
                highlightComparisonField(field, c + "." + field.name);
            }
        }
    }
}

std::string S2Plugin::Entity::entityType() const noexcept
{
    return mEntityType;
}
