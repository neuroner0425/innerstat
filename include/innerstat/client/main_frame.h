#ifndef INNERSTAT_CLIENT_MAIN_FRAME_H
#define INNERSTAT_CLIENT_MAIN_FRAME_H

#ifndef INNERSTAT_CLIENT_BASE_H
    #include "innerstat/client/client.h"
#endif

#include <wx/wx.h>
#include <wx/treectrl.h>
#include <wx/splitter.h>
#include <wx/srchctrl.h>
#include <wxbf/borderless_frame.h>
#include <wxbf/system_buttons.h>
#include <wxbf/window_gripper.h>

#include "innerstat/client/canvas.h"
#include "innerstat/client/dialog.h"
#include "innerstat/client/color_manager.h"
#include "innerstat/client/agent_dialog.h"

#include <memory>

INNERSTAT_BEGIN_NAMESPACE

class MainFrame : public wxBorderlessFrame {
protected:
    virtual wxWindowPart GetWindowPart(wxPoint mousePosition) const wxOVERRIDE { return systemButtons->GetWindowPart(mousePosition); }

private:
    MainCanvas *canvas = nullptr;
    wxPoint dragStart;
    bool dragging = false;
    std::unique_ptr<wxWindowGripper> gripper;
    wxSystemButtonsBase* systemButtons;
    wxPanel* menuBar;
    wxPanel* leftToolbar;
    wxPanel* mainContent;
    wxButton* addAreaBtn;
    int titlebarHeight;
    int leftbarWidth = 54;

    inline void addTopLevelArea(){
        AreaProperties* area = ShowAddAreaDialog(this, 2);
        if (area != nullptr) {
            canvas->AddNewArea(area->label, area->areaType);
            delete area;
        }
    }
    
public:
    /** @brief 키보드 눌림 처리 */
    inline void OnKeyDown(wxKeyEvent& evt){
        printf("Key Down: %d\n", evt.GetKeyCode());
        if (evt.GetKeyCode() == 70) { // 'F' key
            AgentComDialog* dlg = new AgentComDialog(this);
            dlg->Show();
        }
        this->keyPressed[evt.GetKeyCode()] = true;
        if (canvas) canvas->OnKeyDown(evt);
        else evt.Skip();
    }
    
    /** @brief 키보드 눌림 해제 처리 */
    inline void OnKeyUp(wxKeyEvent& evt){
        printf("Key Up: %d\n", evt.GetKeyCode());
        this->keyPressed[evt.GetKeyCode()] = false;
        if (canvas) canvas->OnKeyUp(evt);
        else evt.Skip();
    }

    wxTreeCtrl *shapeTree;
    static bool keyPressed[400]; // 0~255: 일반키, 256~: 특수키

    /** @brief MainFrame 생성자 - UI 초기화 및 이벤트 바인딩 */
    MainFrame();
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void SetUpGUI();
    wxButton* makeMenuBtn(const wxString& label, const wxArrayString subItems);
    wxButton* makeToolBtn(const wxString& label);
    void SetSystemButtonColor();
    void SetWindowColor(wxWindow* windows, const wxColour color_default_fore, const wxColour color_default_back, 
        const wxColour color_hover_fore = wxColor(1,2,3), const wxColour color_hover_back = wxColor(1,2,3));
};

INNERSTAT_END_NAMESPACE

#endif