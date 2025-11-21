#include "innerstat/slapp/slapp.h"
#include "innerstat/slapp/main.h"
#include <wx/log.h>

wxIMPLEMENT_APP(AppMain);

bool AppMain::OnInit() {
    new wxLogNull(); // Completely suppress all wxLog messages
    static wxLocale locale;
    locale.Init(wxLANGUAGE_KOREAN);
    MainFrame* frame = new MainFrame(L"System Load Generator");
    frame->Show(true);
    return true;
}
