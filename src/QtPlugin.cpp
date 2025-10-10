#include "QtPlugin.h"

#include "Spelunky2.h"
#include "Views/ViewToolbar.h"
#include "Views/ViewVirtualTable.h" // TODO: remove, do it thru toolbar?
#include "pluginmain.h"
#include <QFile>
#include <QMainWindow>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QWidget>

QMainWindow* gsSpelunky2MainWindow;
QMdiArea* gsMDIArea;
S2Plugin::ViewToolbar* gsViewToolbar;
QPixmap gsCavemanIcon;

static HANDLE hSetupEvent;
static HANDLE hStopEvent;

namespace QtPlugin
{
    enum
    {
        MENU_DISASM_LOOKUP_IN_VIRTUAL_TABLE,
        MENU_PLUGIN_SHOW_TAB,
    };
}
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

    gsCavemanIcon = QPixmap(":/icons/caveman.png");
    gsSpelunky2MainWindow = new QMainWindow();
    gsSpelunky2MainWindow->setWindowIcon(QIcon(gsCavemanIcon));
    gsMDIArea = new QMdiArea();
    gsSpelunky2MainWindow->setCentralWidget(gsMDIArea);
    gsSpelunky2MainWindow->setWindowTitle("Spelunky 2");

    gsViewToolbar = new S2Plugin::ViewToolbar(parent);
    gsSpelunky2MainWindow->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, gsViewToolbar);

    GuiAddQWidgetTab(gsSpelunky2MainWindow);

    auto cavemanBytes = getResourceBytes(":/icons/caveman.png");
    ICONDATA icon{cavemanBytes.data(), (duint)(cavemanBytes.size())};
    _plugin_menuseticon(S2Plugin::hMenuDisasm, &icon);
    _plugin_menuaddentry(S2Plugin::hMenuDisasm, MENU_DISASM_LOOKUP_IN_VIRTUAL_TABLE, "Lookup in virtual table");

    _plugin_menuseticon(S2Plugin::hMenu, &icon);
    _plugin_menuaddentry(S2Plugin::hMenu, MENU_PLUGIN_SHOW_TAB, "Show tab");

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
    gsMDIArea->closeAllSubWindows();
    S2Plugin::Spelunky2::reset();
}

void QtPlugin::MenuPrepare([[maybe_unused]] int hMenu) {}

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
            break;
        }
        case MENU_PLUGIN_SHOW_TAB:
        {
            if (!gsSpelunky2MainWindow->isVisible())
                GuiAddQWidgetTab(gsSpelunky2MainWindow);

            ShowTab();
            break;
        }
    }
}

QIcon S2Plugin::getCavemanIcon()
{
    return QIcon(gsCavemanIcon);
}

S2Plugin::ViewToolbar* S2Plugin::getToolbar()
{
    return gsViewToolbar;
}

QMdiSubWindow* S2Plugin::openSubWindow(QWidget* widget)
{
    auto win = gsMDIArea->addSubWindow(widget);
    win->setVisible(true);
    win->setAttribute(Qt::WA_DeleteOnClose);
    return win;
}

void S2Plugin::closeAllSubWindows()
{
    gsMDIArea->closeAllSubWindows();
}
