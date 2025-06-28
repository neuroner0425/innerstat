#include <wx/wx.h>
#include <wx/treectrl.h>
#include <wx/splitter.h>
#include "innerstat/client/canvas.h"
#include "innerstat/client/main_frame.h"

INNERSTAT_BEGIN_NAMESPACE

class MyApp : public wxApp {
public:
    virtual bool OnInit() {
        MainFrame *frame = new MainFrame();
        frame->Show();
        return true;
    }
};

wxIMPLEMENT_APP(MyApp);
INNERSTAT_END_NAMESPACE