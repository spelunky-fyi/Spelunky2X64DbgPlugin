#pragma once

#include <QIcon>

class QWidget;
class QMdiSubWindow;

namespace QtPlugin
{
    void Init();
    void Setup();
    void WaitForSetup();
    void Stop();
    void WaitForStop();
    void ShowTab();
    void MenuPrepare(int hMenu);
    void MenuEntry(int hMenu);
    void Detach();
} // namespace QtPlugin

namespace S2Plugin
{
    class ViewToolbar;

    QIcon getCavemanIcon();
    ViewToolbar* getToolbar();
    QMdiSubWindow* openSubWindow(QWidget*);
    void closeAllSubWindows();
} // namespace S2Plugin
