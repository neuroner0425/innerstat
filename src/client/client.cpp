#include <wx/wx.h>
#include <wx/treectrl.h>
#include <wx/splitter.h>
#include "innerstat/client/canvas.h"
#include "innerstat/client/main_frame.h"

INNERSTAT_BEGIN_NAMESPACE

class MyApp : public wxApp {
public:
    virtual bool OnInit() {
        // Ensure image handlers available for any PNG usage
        wxInitAllImageHandlers();
        MainFrame *frame = new MainFrame();
        frame->Show();
        return true;
    }
};

INNERSTAT_END_NAMESPACE
wxIMPLEMENT_APP(innerstat::v1::MyApp);