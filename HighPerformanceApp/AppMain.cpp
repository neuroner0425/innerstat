#include "AppMain.h"
#include "MainFrame.h"

wxIMPLEMENT_APP(AppMain);

bool AppMain::OnInit() {
    static wxLocale locale;
    locale.Init(wxLANGUAGE_KOREAN);
    MainFrame* frame = new MainFrame(L"System Load Generator(v2.2)");
    frame->Show(true);
    return true;
}
