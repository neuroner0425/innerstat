#include <wx/wx.h>
#include <wx/treectrl.h>
#include <wx/splitter.h>
#include "Canvas.h"
#include "MainFrame.h"

class MyApp : public wxApp {
public:
    /**
     * @brief 앱 초기화 시 MainFrame 생성 및 표시
     * @return true 항상 true 반환
     */
    virtual bool OnInit() {
        MainFrame *frame = new MainFrame();
        frame->Show();
        return true;
    }
};

wxIMPLEMENT_APP(MyApp);