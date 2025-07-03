#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/srchctrl.h>
#include <wxbf/borderless_frame.h>
#include <wxbf/system_buttons.h>
#include <wxbf/window_gripper_msw.h>
#include <memory>

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
    wxSearchCtrl* m_searchBox;
    wxPanel* m_menuBar;
    wxPanel* m_leftToolbar;
    wxPanel* m_content;
    int m_titlebarHeight;
    int m_leftbarWidth = 54;

    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
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
    m_buttons = wxSystemButtonsFactory::CreateSystemButtons(this);
    m_buttons->UpdateState();
    m_titlebarHeight = m_buttons->GetTotalSize().y;
    if (m_titlebarHeight < 28) m_titlebarHeight = 35;

    m_searchBox = new wxSearchCtrl(this, wxID_ANY, "", wxPoint(), wxSize(320, 30));
    m_searchBox->SetHint("ineerStat");

    m_menuBar = new wxPanel(this, wxID_ANY, wxPoint(), wxDefaultSize);
    m_menuBar->SetBackgroundColour(wxColour(37, 37, 38));

    wxBoxSizer* menuSizer = new wxBoxSizer(wxHORIZONTAL);

    auto makeMenuBtn = [this](const wxString& label, const std::vector<wxString>& subItems) {
        wxButton* btn = new wxButton(m_menuBar, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
        btn->SetForegroundColour(wxColour(225,225,225));
        btn->SetBackgroundColour(m_menuBar->GetBackgroundColour());
        btn->SetWindowStyleFlag(wxBORDER_NONE | wxTRANSPARENT_WINDOW);
        btn->SetCursor(wxCursor(wxCURSOR_HAND));
        btn->Fit();

        btn->Bind(wxEVT_ENTER_WINDOW, [btn](wxMouseEvent&) {
            btn->SetBackgroundColour(wxColour(255,255,255,24));
            btn->Refresh();
        });
        btn->Bind(wxEVT_LEAVE_WINDOW, [btn, this](wxMouseEvent&) {
            btn->SetBackgroundColour(btn->GetParent()->GetBackgroundColour());
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

    wxButton* btnFile = makeMenuBtn(L"파일(F)", {L"새 파일", L"열기", L"저장", L"다른 이름으로 저장"});
    wxButton* btnEdit = makeMenuBtn(L"편집(E)", {L"실행 취소", L"다시 실행", L"복사", L"붙여넣기"});
    wxButton* btnSelect = makeMenuBtn(L"선택 영역(S)", {L"모두 선택", L"줄 선택", L"블록 선택"});

    menuSizer->Add(btnFile,   0, wxLEFT | wxALIGN_CENTER_VERTICAL);
    menuSizer->Add(btnEdit,   0, wxLEFT | wxALIGN_CENTER_VERTICAL);
    menuSizer->Add(btnSelect, 0, wxLEFT | wxALIGN_CENTER_VERTICAL);
    m_menuBar->SetSizer(menuSizer);

    m_leftToolbar = new wxPanel(this, wxID_ANY, wxPoint(), wxSize(m_leftbarWidth, 100));
    m_leftToolbar->SetBackgroundColour(wxColour(37, 37, 38));
    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);

    wxButton* btn1 = new wxButton(m_leftToolbar, wxID_ANY, "F", wxDefaultPosition, wxSize(40, 40));
    wxButton* btn2 = new wxButton(m_leftToolbar, wxID_ANY, "S", wxDefaultPosition, wxSize(40, 40));
    wxButton* btn3 = new wxButton(m_leftToolbar, wxID_ANY, "G", wxDefaultPosition, wxSize(40, 40));
    leftSizer->AddSpacer(10);
    leftSizer->Add(btn1, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 4);
    leftSizer->Add(btn2, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 4);
    leftSizer->Add(btn3, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 4);
    leftSizer->AddStretchSpacer();
    m_leftToolbar->SetSizer(leftSizer);

    m_content = new wxPanel(this, wxID_ANY, wxPoint(), wxSize(700, 600));
    m_content->SetBackgroundColour(wxColour(30, 30, 30));

    Bind(wxEVT_PAINT, &MyFrame::OnPaint, this);
    Bind(wxEVT_SIZE, &MyFrame::OnSize, this);

    wxSizeEvent dummy;
    OnSize(dummy);
}

void MyFrame::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    wxColour barColor(37, 37, 38);
    dc.SetBrush(barColor);
    dc.SetPen(barColor);
    dc.DrawRectangle(0, 0, GetClientSize().x, m_titlebarHeight);

    dc.SetPen(wxColour(50, 50, 50));
    dc.DrawLine(0, m_titlebarHeight - 1, GetClientSize().x, m_titlebarHeight - 1);

    event.Skip();
}


void MyFrame::OnSize(wxSizeEvent& event)
{
    wxSize sz = GetClientSize();

    m_menuBar->SetSize(m_leftbarWidth, 0, 220, m_titlebarHeight);

    int searchW = 320, searchH = 30;
    m_searchBox->SetSize((sz.x - searchW) / 2, 4, searchW, searchH);

    m_leftToolbar->SetSize(0, m_titlebarHeight, m_leftbarWidth, sz.y - m_titlebarHeight);

    m_content->SetSize(m_leftbarWidth, m_titlebarHeight, sz.x - m_leftbarWidth, sz.y - m_titlebarHeight);

    event.Skip();
}