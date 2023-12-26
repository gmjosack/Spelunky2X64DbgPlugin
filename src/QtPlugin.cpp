#include "QtPlugin.h"
#include "Configuration.h"
#include "Data/CharacterDB.h"
#include "Data/EntityDB.h"
#include "Data/GameManager.h"
#include "Data/LevelGen.h"
#include "Data/Online.h"
#include "Data/ParticleDB.h"
#include "Data/ParticleEmittersList.h"
#include "Data/SaveGame.h"
#include "Data/State.h"
#include "Data/StringsTable.h"
#include "Data/TextureDB.h"
#include "Data/VirtualTableLookup.h"
#include "Spelunky2.h"
#include "Views/ViewToolbar.h"
#include "Views/ViewVirtualTable.h"
#include "pluginmain.h"
#include <QFile>
#include <QMainWindow>
#include <QMdiArea>
#include <QWidget>

QMainWindow* gsSpelunky2MainWindow;
QMdiArea* gsMDIArea;
S2Plugin::ViewToolbar* gsViewToolbar;
S2Plugin::EntityDB* gsEntityDB;
S2Plugin::ParticleDB* gsParticleDB;
S2Plugin::TextureDB* gsTextureDB;
S2Plugin::CharacterDB* gsCharacterDB;
S2Plugin::GameManager* gsGameManager;
S2Plugin::State* gsState;
S2Plugin::SaveGame* gsSaveGame;
S2Plugin::LevelGen* gsLevelGen;
S2Plugin::VirtualTableLookup* gsVirtualTableLookup;
S2Plugin::StringsTable* gsStringsTable;
S2Plugin::Online* gsOnline;

static HANDLE hSetupEvent;
static HANDLE hStopEvent;

enum
{
    MENU_DISASM_LOOKUP_IN_VIRTUAL_TABLE
};

static QByteArray getResourceBytes(const char* path)
{
    QByteArray b;
    QFile s(path);
    if (s.open(QFile::ReadOnly))
        b = s.readAll();
    return b;
}

static QWidget* getParent()
{
    return QWidget::find((WId)S2Plugin::hwndDlg);
}

void QtPlugin::Init()
{
    hSetupEvent = CreateEventW(nullptr, true, false, nullptr);
    hStopEvent = CreateEventW(nullptr, true, false, nullptr);
}

void QtPlugin::Setup()
{
    QWidget* parent = getParent();

    gsSpelunky2MainWindow = new QMainWindow();
    gsSpelunky2MainWindow->setWindowIcon(QIcon(":/icons/caveman.png"));
    gsMDIArea = new QMdiArea();
    gsSpelunky2MainWindow->setCentralWidget(gsMDIArea);
    gsSpelunky2MainWindow->setWindowTitle("Spelunky 2");

    gsEntityDB = new S2Plugin::EntityDB();
    gsParticleDB = new S2Plugin::ParticleDB();
    gsTextureDB = new S2Plugin::TextureDB();
    gsCharacterDB = new S2Plugin::CharacterDB();
    gsState = new S2Plugin::State();
    gsGameManager = new S2Plugin::GameManager();
    gsSaveGame = new S2Plugin::SaveGame(gsGameManager);
    gsLevelGen = new S2Plugin::LevelGen(gsState);
    gsVirtualTableLookup = new S2Plugin::VirtualTableLookup();
    gsStringsTable = new S2Plugin::StringsTable();
    gsOnline = new S2Plugin::Online();

    gsViewToolbar = new S2Plugin::ViewToolbar(gsEntityDB, gsParticleDB, gsTextureDB, gsCharacterDB, gsGameManager, gsSaveGame, gsState, gsLevelGen, gsVirtualTableLookup, gsStringsTable, gsOnline,
                                              gsMDIArea, parent);
    gsSpelunky2MainWindow->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, gsViewToolbar);

    GuiAddQWidgetTab(gsSpelunky2MainWindow);

    auto cavemanBytes = getResourceBytes(":/icons/caveman.png");
    ICONDATA cavemanIcon{cavemanBytes.data(), (duint)(cavemanBytes.size())};
    _plugin_menuseticon(S2Plugin::hMenuDisasm, &cavemanIcon);
    _plugin_menuaddentry(S2Plugin::hMenuDisasm, MENU_DISASM_LOOKUP_IN_VIRTUAL_TABLE, "Lookup in virtual table");

    SetEvent(hSetupEvent);
}

void QtPlugin::WaitForSetup()
{
    WaitForSingleObject(hSetupEvent, INFINITE);
}

void QtPlugin::Stop()
{
    GuiCloseQWidgetTab(gsSpelunky2MainWindow);
    gsSpelunky2MainWindow->close();
    delete gsSpelunky2MainWindow;
    SetEvent(hStopEvent);
}

void QtPlugin::WaitForStop()
{
    WaitForSingleObject(hStopEvent, INFINITE);
}

void QtPlugin::ShowTab()
{
    GuiShowQWidgetTab(gsSpelunky2MainWindow);
}

void QtPlugin::Detach()
{
    gsViewToolbar->resetSpelunky2Data();
}

void QtPlugin::MenuPrepare(int hMenu) {}

void QtPlugin::MenuEntry(int hEntry)
{
    switch (hEntry)
    {
        case MENU_DISASM_LOOKUP_IN_VIRTUAL_TABLE:
        {
            if (!DbgIsDebugging())
                return;

            SELECTIONDATA sel = {0, 0};
            GuiSelectionGet(GUI_DISASSEMBLY, &sel);

            // to find the function start, look for two consecutive 0xCC (not always perfect)
            size_t functionStart = 0;
            size_t counter = 0;
            uint8_t ccFound = 0;
            while (counter < 10000)
            {
                if (Script::Memory::ReadByte(sel.start - counter) == 0xCC)
                {
                    ccFound++;
                    if (ccFound == 2)
                    {
                        functionStart = sel.start - counter + 2;
                        break;
                    }
                }
                else
                {
                    ccFound = 0;
                }
                counter++;
            }
            ShowTab();
            auto window = gsViewToolbar->showVirtualTableLookup();
            if (window != nullptr)
            {
                window->showLookupAddress(functionStart);
            }
        }
        break;
    }
}
