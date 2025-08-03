#include "pluginmain.h"

#include "QtPlugin.h"
#include <QMessageBox>

int S2Plugin::handle;
HWND S2Plugin::hwndDlg;
int S2Plugin::hMenu;
int S2Plugin::hMenuDisasm;
int S2Plugin::hMenuDump;
int S2Plugin::hMenuStack;

PLUG_EXPORT bool pluginit(PLUG_INITSTRUCT* initStruct)
{
    initStruct->pluginVersion = PLUGIN_VERSION;
    initStruct->sdkVersion = PLUG_SDKVERSION;
    strncpy_s(initStruct->pluginName, PLUGIN_NAME, _TRUNCATE);
    S2Plugin::handle = initStruct->pluginHandle;
    QtPlugin::Init();
    return true;
}

PLUG_EXPORT bool plugstop()
{
    GuiExecuteOnGuiThread(QtPlugin::Stop);
    QtPlugin::WaitForStop();
    return true;
}

PLUG_EXPORT void plugsetup(PLUG_SETUPSTRUCT* setupStruct)
{
    S2Plugin::hwndDlg = setupStruct->hwndDlg;
    S2Plugin::hMenu = setupStruct->hMenu;
    S2Plugin::hMenuDisasm = setupStruct->hMenuDisasm;
    S2Plugin::hMenuDump = setupStruct->hMenuDump;
    S2Plugin::hMenuStack = setupStruct->hMenuStack;
    GuiExecuteOnGuiThread(QtPlugin::Setup);
    QtPlugin::WaitForSetup();
}

PLUG_EXPORT void CBDETACH([[maybe_unused]] CBTYPE cbType, [[maybe_unused]] PLUG_CB_DETACH* info)
{
    GuiExecuteOnGuiThread(QtPlugin::Detach);
}

PLUG_EXPORT void CBEXITPROCESS([[maybe_unused]] CBTYPE cbType, [[maybe_unused]] PLUG_CB_EXITPROCESS* info)
{
    GuiExecuteOnGuiThread(QtPlugin::Detach);
}

PLUG_EXPORT void CBMENUPREPARE([[maybe_unused]] CBTYPE cbType, PLUG_CB_MENUPREPARE* info)
{
    QtPlugin::MenuPrepare(info->hMenu);
}

PLUG_EXPORT void CBMENUENTRY([[maybe_unused]] CBTYPE cbType, PLUG_CB_MENUENTRY* info)
{
    QtPlugin::MenuEntry(info->hEntry);
}

void displayError(const char* fmt, ...)
{
    char buffer[1024] = {0};

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowIcon(S2Plugin::getCavemanIcon());
    msgBox.setText(buffer);
    msgBox.setWindowTitle("Spelunky2");
    msgBox.exec();
    _plugin_logprintf("[Spelunky2] %s\n", buffer);
}

void displayError(std::string message)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowIcon(S2Plugin::getCavemanIcon());
    msgBox.setText(message.c_str());
    msgBox.setWindowTitle("Spelunky2");
    msgBox.exec();
    message.insert(0, "[Spelunky2] ");
    message += '\n';
    _plugin_logprint(message.c_str());
}
