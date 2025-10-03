#include <wx/wx.h>
#include <wx/treectrl.h>
#include <wx/splitter.h>
#include <wx/eventfilter.h>
#include <wx/textentry.h>
#include <algorithm>
#include "innerstat/client/canvas.h"
#include "innerstat/client/main_frame.h"

INNERSTAT_BEGIN_NAMESPACE

// 전역 키 이벤트 필터: 포커스와 무관하게 키 눌림/해제 추적
class GlobalKeyFilter : public wxEventFilter {
public:
    explicit GlobalKeyFilter(MainFrame* frame) : frame(frame) {}
    int FilterEvent(wxEvent& event) override {
        if (!frame) return Event_Skip;
        // 텍스트 입력 위젯에 포커스가 있으면 감지 제외
        if (IsTextEntryTarget(event)) {
            return Event_Skip;
        }
        if (event.GetEventType() == wxEVT_KEY_DOWN || event.GetEventType() == wxEVT_CHAR_HOOK) {
            auto* ke = dynamic_cast<wxKeyEvent*>(&event);
            if (ke) {
                int code = ke->GetKeyCode();
                if (code >= 0 && code < 400) MainFrame::keyPressed[code] = true;
                wxKeyEvent copy(*ke);
                frame->OnKeyDown(copy);
            }
        } else if (event.GetEventType() == wxEVT_KEY_UP) {
            auto* ke = dynamic_cast<wxKeyEvent*>(&event);
            if (ke) {
                int code = ke->GetKeyCode();
                if (code >= 0 && code < 400) MainFrame::keyPressed[code] = false;
                wxKeyEvent copy(*ke);
                frame->OnKeyUp(copy);
            }
        }
        return Event_Skip;
    }
    bool IsTextEntryTarget(wxEvent& event) const {
        wxWindow* srcWin = wxDynamicCast(event.GetEventObject(), wxWindow);
        if (!srcWin) srcWin = wxWindow::FindFocus();
        if (!srcWin) return false;
        // wxTextEntryBase는 RTTI 기반 캐스트 사용 (mixin)
        return dynamic_cast<wxTextEntryBase*>(srcWin) != nullptr;
    }
private:
    MainFrame* frame;
};

class MyApp : public wxApp {
public:
    bool OnInit() override {
        wxInitAllImageHandlers();
        std::fill(std::begin(MainFrame::keyPressed), std::end(MainFrame::keyPressed), false);
        frame = new MainFrame();
        keyFilter = std::make_unique<GlobalKeyFilter>(frame);
        wxEvtHandler::AddFilter(keyFilter.get());
        frame->Show();
        return true;
    }

    int OnExit() override {
        if (keyFilter) {
            wxEvtHandler::RemoveFilter(keyFilter.get());
            keyFilter.reset();
        }
        return wxApp::OnExit();
    }

private:
    std::unique_ptr<GlobalKeyFilter> keyFilter;
    MainFrame* frame = nullptr;

    wxDECLARE_EVENT_TABLE();
    void OnActivateApp(wxActivateEvent& evt);
};

// 앱 활성/비활성 이벤트 처리: 비활성 시 키 상태 리셋
wxBEGIN_EVENT_TABLE(MyApp, wxApp)
    EVT_ACTIVATE_APP(MyApp::OnActivateApp)
wxEND_EVENT_TABLE()

void MyApp::OnActivateApp(wxActivateEvent& evt) {
    if (!evt.GetActive()) {
        std::fill(std::begin(MainFrame::keyPressed), std::end(MainFrame::keyPressed), false);
    }
    evt.Skip();
}

INNERSTAT_END_NAMESPACE
wxIMPLEMENT_APP(innerstat::v1::MyApp);