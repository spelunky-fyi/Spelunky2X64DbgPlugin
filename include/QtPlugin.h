#pragma once

#include <QIcon>

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
} // namespace S2Plugin
