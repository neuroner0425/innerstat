#include "innerstat/slapp/slapp.h"
#include "innerstat/slapp/main.h"

wxIMPLEMENT_APP(AppMain);

bool AppMain::OnInit() {
    static wxLocale locale;
    locale.Init(wxLANGUAGE_KOREAN);
    MainFrame* frame = new MainFrame(L"System Load Generator");
    frame->Show(true);
    return true;
}
