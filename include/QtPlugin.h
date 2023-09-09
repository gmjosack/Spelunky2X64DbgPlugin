#pragma once

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
