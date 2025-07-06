#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/srchctrl.h>
#include <wxbf/borderless_frame.h>
#include <wxbf/system_buttons.h>
#include <wxbf/window_gripper_msw.h>
#include <memory>

#include "innerstat/client/color_manager.h"


class MyApp : public wxApp {
public:
    bool OnInit() override;
};

class MyFrame : public wxBorderlessFrame {
public:
    MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

protected:
    virtual wxWindowPart GetWindowPart(wxPoint mousePosition) const wxOVERRIDE { return m_buttons->GetWindowPart(mousePosition); }
private:
    std::unique_ptr<wxWindowGripper> m_gripper;
    wxSystemButtonsBase* m_buttons;
    wxPanel* m_menuBar;
    wxPanel* m_leftToolbar;
    wxPanel* m_content;
    int m_titlebarHeight;
    int m_leftbarWidth = 54;

    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void SetUpGUI();
    wxButton* makeMenuBtn(const wxString& label, const wxArrayString subItems);
    wxButton* makeToolBtn(const wxString& label);
    void SetSystemButtonColor();
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
    wxInitAllImageHandlers();
    MyFrame* frame = new MyFrame("VSCode style UI", wxPoint(30, 30), wxSize(1100, 700));
    frame->SetDoubleBuffered(true);
    frame->Show(true);
    return true;
}

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
    : wxBorderlessFrame(nullptr, wxID_ANY, title, pos, size),
      m_gripper(wxWindowGripper::Create())
{
    
    SetUpGUI();

    Bind(wxEVT_PAINT, &MyFrame::OnPaint, this);
    Bind(wxEVT_SIZE, &MyFrame::OnSize, this);

    wxSizeEvent dummy;
    OnSize(dummy);
}

void MyFrame::SetUpGUI(){
    m_buttons = wxSystemButtonsFactory::CreateSystemButtons(this);
    m_buttons->SetButtonSize(wxSize(46, 35));
    m_buttons->UpdateState();
    SetSystemButtonColor();

    m_titlebarHeight = m_buttons->GetTotalSize().y;

    m_menuBar = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    m_menuBar->SetBackgroundColour(C_TITLE_BAR_BACKGROUND);

    wxBoxSizer* menuSizer = new wxBoxSizer(wxHORIZONTAL);

    wxButton* btnFile = makeMenuBtn(L"파일(F)", {L"새 파일", L"열기", L"저장", L"다른 이름으로 저장"});
    wxButton* btnEdit = makeMenuBtn(L"편집(E)", {L"실행 취소", L"다시 실행", L"복사", L"붙여넣기"});
    wxButton* btnSelect = makeMenuBtn(L"선택 영역(S)", {L"모두 선택", L"줄 선택", L"블록 선택"});

    menuSizer->Add(btnFile,   0, wxLEFT | wxALIGN_CENTER_VERTICAL);
    menuSizer->Add(btnEdit,   0, wxLEFT | wxALIGN_CENTER_VERTICAL);
    menuSizer->Add(btnSelect, 0, wxLEFT | wxALIGN_CENTER_VERTICAL);
    m_menuBar->SetSizer(menuSizer);

    m_leftToolbar = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(m_leftbarWidth, 100));
    m_leftToolbar->SetBackgroundColour(C_TOOL_BAR_BACKGROUND);
    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);

    wxButton* btn1 = makeToolBtn("F");
    wxButton* btn2 = makeToolBtn("S");
    wxButton* btn3 = makeToolBtn("G");

    leftSizer->AddSpacer(10);
    leftSizer->Add(btn1, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 4);
    leftSizer->Add(btn2, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 4);
    leftSizer->Add(btn3, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 4);
    leftSizer->AddStretchSpacer();
    m_leftToolbar->SetSizer(leftSizer);

    m_content = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(700, 600));
    m_content->SetBackgroundColour(C_CLIENT_BACKGROUND);
}

void MyFrame::SetSystemButtonColor(){
    auto setButtonColor = [this](int button, int state, int kind, const wxColour& color) {
        for (size_t buttonKind = (button < 0 ? 0 : button); buttonKind <= (button < 0 ? 3 : button); ++buttonKind) {
            for (size_t stateKind = (state < 0 ? 0 : state); stateKind <= (state < 0 ? 4 : state); ++stateKind) {
                m_buttons->SetColourTableEntry(
                    static_cast<wxSystemButton>(buttonKind),
                    static_cast<wxSystemButtonState>(stateKind),
                    static_cast<wxSystemButtonColourKind>(kind),
                    color);
            }
        }
    };

    setButtonColor(-1, -1, wxSystemButtonColourKind::wxSB_COLOUR_BACKGROUND, C_TITLE_BAR_BACKGROUND);
    setButtonColor(-1, -1, wxSystemButtonColourKind::wxSB_COLOUR_FOREGROUND, C_TITLE_BAR_FOREGROUND);

    setButtonColor(-1, wxSystemButtonState::wxSB_STATE_HOVER, wxSystemButtonColourKind::wxSB_COLOUR_BACKGROUND, C_TITLE_BAR_HOVER_BACKGROUND);
    setButtonColor(-1, wxSystemButtonState::wxSB_STATE_PRESSED, wxSystemButtonColourKind::wxSB_COLOUR_BACKGROUND, C_TITLE_BAR_PRESSED_BACKGROUND);

    
    setButtonColor(wxSystemButton::wxSB_CLOSE, wxSystemButtonState::wxSB_STATE_HOVER, wxSystemButtonColourKind::wxSB_COLOUR_BACKGROUND, C_TITLE_BAR_CLOSE_HOVER_BACKGROUND);
    setButtonColor(wxSystemButton::wxSB_CLOSE, wxSystemButtonState::wxSB_STATE_PRESSED, wxSystemButtonColourKind::wxSB_COLOUR_BACKGROUND, C_TITLE_BAR_CLOSE_PRESSED_BACKGROUND);

    setButtonColor(-1, wxSystemButtonState::wxSB_STATE_INACTIVE, wxSystemButtonColourKind::wxSB_COLOUR_BACKGROUND, C_TITLE_BAR_INACTIVE_BACKGROUND);
    setButtonColor(-1, wxSystemButtonState::wxSB_STATE_INACTIVE, wxSystemButtonColourKind::wxSB_COLOUR_FOREGROUND, C_TITLE_BAR_INACTIVE_FOREGROUND);
    
    m_buttons->UpdateState();
}

void MyFrame::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    dc.SetBrush(C_TITLE_BAR_BACKGROUND);
    dc.SetPen(C_TITLE_BAR_BACKGROUND);
    dc.DrawRectangle(0, 0, GetClientSize().x, m_titlebarHeight);

    event.Skip();
}


void MyFrame::OnSize(wxSizeEvent& event)
{
    wxSize sz = GetClientSize();

    m_menuBar->SetSize(20, 0, 220, m_titlebarHeight);

    int searchW = 320, searchH = 30;

    m_leftToolbar->SetSize(0, m_titlebarHeight, m_leftbarWidth, sz.y - m_titlebarHeight);

    m_content->SetSize(m_leftbarWidth, m_titlebarHeight, sz.x - m_leftbarWidth, sz.y - m_titlebarHeight);

    event.Skip();
}

wxButton* MyFrame::makeMenuBtn(const wxString& label, const wxArrayString subItems) {
    wxButton* btn = new wxButton(m_menuBar, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    btn->SetBackgroundColour(C_TITLE_BAR_BACKGROUND);
    btn->SetForegroundColour(C_TITLE_BAR_FOREGROUND);
    btn->SetWindowStyleFlag(wxBORDER_NONE | wxTRANSPARENT_WINDOW);

    int textWidth, textHeight;
    m_menuBar->GetTextExtent(label, &textWidth, &textHeight);
    int padding = 20;
    btn->SetMinSize(wxSize(textWidth + padding, -1));

    btn->Bind(wxEVT_ENTER_WINDOW, [btn](wxMouseEvent&) {
        btn->SetBackgroundColour(C_TITLE_BAR_HOVER_BACKGROUND);
        btn->Refresh();
    });
    btn->Bind(wxEVT_LEAVE_WINDOW, [btn, this](wxMouseEvent&) {
        btn->SetBackgroundColour(C_TITLE_BAR_BACKGROUND);
        btn->Refresh();
    });

    btn->Bind(wxEVT_BUTTON, [this, btn, subItems](wxCommandEvent&) {
        wxMenu popup;
        for(const auto& text : subItems) {
            popup.Append(wxID_ANY, text);
        }
        wxPoint btnScreen = btn->ClientToScreen(wxPoint(0, btn->GetSize().y));
        PopupMenu(&popup, ScreenToClient(btnScreen));
    });

    return btn;
};


wxButton* MyFrame::makeToolBtn(const wxString& label) {
    wxButton* btn = new wxButton(m_leftToolbar, wxID_ANY, label, wxDefaultPosition, wxSize(40, 40));
    btn->SetBackgroundColour(C_TOOL_BAR_BACKGROUND);
    btn->SetForegroundColour(C_TOOL_BAR_FOREGROUND);
    btn->SetWindowStyleFlag(wxBORDER_NONE | wxTRANSPARENT_WINDOW);
    btn->SetCursor(wxCursor(wxCURSOR_HAND));

    btn->Bind(wxEVT_ENTER_WINDOW, [btn](wxMouseEvent&) {
        btn->SetForegroundColour(C_TOOL_BAR_HOVER_FOREGROUND);
        btn->Refresh();
    });
    btn->Bind(wxEVT_LEAVE_WINDOW, [btn, this](wxMouseEvent&) {
        btn->SetForegroundColour(C_TOOL_BAR_FOREGROUND);
        btn->Refresh();
    });

    return btn;
};