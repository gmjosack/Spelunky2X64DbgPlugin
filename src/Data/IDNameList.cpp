#include "Data/IDNameList.h"
#include "pluginmain.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <filesystem>
#include <fstream>

S2Plugin::IDNameList::IDNameList(const std::string& relFilePath, const std::regex& regex)
{
    char buffer[MAX_PATH] = {0};
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    auto pathQStr = QFileInfo(QString(buffer)).dir().filePath(QString::fromStdString(relFilePath));
    if (!QFile(pathQStr).exists())
    {
        displayError((relFilePath + " not found").c_str());
        return;
    }

    std::ifstream fp(pathQStr.toStdString());
    while (fp)
    {
        std::string line;
        if (!std::getline(fp, line))
        {
            break;
        }
        std::smatch m;
        if (std::regex_match(line, m, regex))
        {
            auto id = std::stoi(m[1].str());
            auto name = m[2].str();
            mEntries[id] = name;
            mNames << QString::fromStdString(name);
            if (id > mHighestID)
            {
                mHighestID = id;
            }
        }
    }
    fp.close();
}

uint32_t S2Plugin::IDNameList::idForName(const std::string& searchName) const
{
    for (const auto& [id, name] : mEntries)
    {
        if (name == searchName)
        {
            return id;
        }
    }
    return 0;
}

std::string S2Plugin::IDNameList::nameForID(uint32_t id) const
{
    if (auto it = mEntries.find(id); it != mEntries.end())
    {
        return it->second;
    }
    return "UNKNOWN ID: " + std::to_string(id);
}

uint32_t S2Plugin::IDNameList::highestID() const noexcept
{
    return mHighestID;
}

QStringList S2Plugin::IDNameList::names() const noexcept
{
    return mNames;
}

bool S2Plugin::IDNameList::isValidID(uint32_t id) const
{
    return (mEntries.count(id) > 0);
}

size_t S2Plugin::IDNameList::count() const noexcept
{
    return mEntries.size();
}

const std::unordered_map<uint32_t, std::string>& S2Plugin::IDNameList::entries() const
{
    return mEntries;
}

static const std::regex regexEntityLine("^([0-9]+): ENT_TYPE_(.*?)$", std::regex_constants::ECMAScript);

S2Plugin::EntityList::EntityList() : IDNameList("plugins/Spelunky2Entities.txt", regexEntityLine) {}

static const std::regex regexParticleLine("^([0-9]+): PARTICLEEMITTER_(.*?)$", std::regex_constants::ECMAScript);

S2Plugin::ParticleEmittersList::ParticleEmittersList() : IDNameList("plugins/Spelunky2ParticleEmitters.txt", regexParticleLine) {}
