#ifndef INNERSTAT_CLIENT_BASE_H
    #include "innerstat/client/base.h"
#endif

#include <wx/wx.h>
#include <wx/treectrl.h>
#include <wx/splitter.h>
#include <wx/srchctrl.h>
#include <wxbf/borderless_frame.h>
#include <wxbf/system_buttons.h>
#include <wxbf/window_gripper_msw.h>

#include "innerstat/client/canvas.h"
#include "innerstat/client/dialog.h"
#include "innerstat/client/color_manager.h"
#include "innerstat/client/main_frame.h"
#include "innerstat/client/no_sash_splitter.h"

INNERSTAT_BEGIN_NAMESPACE

MainFrame::MainFrame()
    :wxBorderlessFrame(nullptr, wxID_ANY, L"Inner Stat", wxDefaultPosition, wxSize(1100, 700)), gripper(wxWindowGripper::Create()){
    
    SetUpGUI();
    
    Bind(wxEVT_PAINT, &MainFrame::OnPaint, this);
    Bind(wxEVT_SIZE, &MainFrame::OnSize, this);
    Bind(wxEVT_CHAR_HOOK, &MainFrame::OnKeyDown, this);
    Bind(wxEVT_KEY_UP, &MainFrame::OnKeyUp, this);

    addAreaBtn->Bind(wxEVT_BUTTON, [=](wxCommandEvent &) {
        addTopLevelArea();
    });
    shapeTree->Bind(wxEVT_TREE_SEL_CHANGED, [=](wxTreeEvent &evt) {
        canvas->OnTreeSelectionChanged(evt.GetItem());
    });
    shapeTree->Bind(wxEVT_TREE_ITEM_ACTIVATED, [=](wxTreeEvent &evt) {
        canvas->OnTreeLeftDClick(evt.GetItem());
    });

    wxSizeEvent dummy;
    OnSize(dummy);
}

void MainFrame::SetUpGUI(){
    SetBorderColour(C_TITLE_BAR_BACKGROUND);

    systemButtons = wxSystemButtonsFactory::CreateSystemButtons(this);
    systemButtons->SetButtonSize(wxSize(46, 35));
    systemButtons->UpdateState();
    SetSystemButtonColor();

    titlebarHeight = systemButtons->GetTotalSize().y;

    menuBar = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    menuBar->SetBackgroundColour(C_TITLE_BAR_BACKGROUND);

    wxBoxSizer* menuSizer = new wxBoxSizer(wxHORIZONTAL);

    wxButton* btnFile = makeMenuBtn(L"파일(F)", {L"새 파일", L"열기", L"저장", L"다른 이름으로 저장"});
    wxButton* btnEdit = makeMenuBtn(L"편집(E)", {L"실행 취소", L"다시 실행", L"복사", L"붙여넣기"});
    wxButton* btnSelect = makeMenuBtn(L"선택 영역(S)", {L"모두 선택", L"줄 선택", L"블록 선택"});

    menuSizer->Add(btnFile,   0, wxLEFT | wxALIGN_CENTER_VERTICAL);
    menuSizer->Add(btnEdit,   0, wxLEFT | wxALIGN_CENTER_VERTICAL);
    menuSizer->Add(btnSelect, 0, wxLEFT | wxALIGN_CENTER_VERTICAL);
    menuBar->SetSizer(menuSizer);

    leftToolbar = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(leftbarWidth, 100));
    leftToolbar->SetBackgroundColour(C_TOOL_BAR_BACKGROUND);
    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);

    wxButton* btn1 = makeToolBtn("F");
    wxButton* btn2 = makeToolBtn("S");
    wxButton* btn3 = makeToolBtn("G");

    leftSizer->AddSpacer(10);
    leftSizer->Add(btn1, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 4);
    leftSizer->Add(btn2, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 4);
    leftSizer->Add(btn3, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 4);
    leftSizer->AddStretchSpacer();
    leftToolbar->SetSizer(leftSizer);

    mainContent = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(700, 600));
    mainContent->SetBackgroundColour(C_CLIENT_BACKGROUND);

    wxSplitterWindow *splitter = new wxNoSashSplitterWindow(mainContent);
    wxPanel *leftPanel = new wxPanel(splitter);
    wxBoxSizer *treeViewSizer = new wxBoxSizer(wxVERTICAL);
    shapeTree = new wxTreeCtrl(leftPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS);
    shapeTree->SetWindowStyleFlag(wxBORDER_NONE | wxTRANSPARENT_WINDOW);
    SetWindowColor(shapeTree, C_TREE_VIEW_FOREGROUND, C_TREE_VIEW_BACKGROUND);
    addAreaBtn = new wxButton(leftPanel, wxID_ANY, "Add Shape");
    addAreaBtn->SetWindowStyleFlag(wxBORDER_NONE | wxTRANSPARENT_WINDOW);
    SetWindowColor(addAreaBtn, C_TREE_VIEW_FOREGROUND, C_TREE_VIEW_BACKGROUND, C_TREE_VIEW_FOREGROUND, C_TITLE_BAR_HOVER_BACKGROUND);

    treeViewSizer->Add(addAreaBtn, 0, wxEXPAND | wxBOTTOM, 3);
    treeViewSizer->Add(shapeTree, 1, wxEXPAND | wxBOTTOM, 5);
    leftPanel->SetSizer(treeViewSizer);

    canvas = new MainCanvas(splitter, this);
    canvas->SetBackgroundColour(C_CLIENT_BACKGROUND);

    splitter->SetMinimumPaneSize(120);
    splitter->SetBackgroundColour(C_TREE_VIEW_BACKGROUND);
    splitter->Bind(wxEVT_ERASE_BACKGROUND, [](wxEraseEvent&){});

    splitter->SplitVertically(leftPanel, canvas);

    CallAfter([=]() {
        splitter->SetSashPosition(200);
        splitter->Refresh();
        splitter->Update();
    });

    wxBoxSizer* mainContentSizer = new wxBoxSizer(wxVERTICAL);
    mainContentSizer->Add(splitter, 1, wxEXPAND);
    mainContent->SetSizer(mainContentSizer);

    mainContent->Layout();
    this->SetMinClientSize(wxSize(350, 200));

    Refresh();
}

void MainFrame::SetSystemButtonColor(){
    auto setButtonColor = [this](int button, int state, int kind, const wxColour& color) {
        for (size_t buttonKind = (button < 0 ? 0 : button); buttonKind <= (button < 0 ? 3 : button); ++buttonKind) {
            for (size_t stateKind = (state < 0 ? 0 : state); stateKind <= (state < 0 ? 4 : state); ++stateKind) {
                systemButtons->SetColourTableEntry(
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
    
    systemButtons->UpdateState();
}

void MainFrame::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    dc.SetBrush(C_TITLE_BAR_BACKGROUND);
    dc.SetPen(C_TITLE_BAR_BACKGROUND);
    dc.DrawRectangle(0, 0, GetClientSize().x, titlebarHeight);

    event.Skip();
}


void MainFrame::OnSize(wxSizeEvent& event)
{
    wxSize sz = GetClientSize();

    menuBar->SetSize(20, 0, 220, titlebarHeight);

    int searchW = 320, searchH = 30;

    leftToolbar->SetSize(0, titlebarHeight, leftbarWidth, sz.y - titlebarHeight);

    mainContent->SetSize(leftbarWidth, titlebarHeight, sz.x - leftbarWidth, sz.y - titlebarHeight);

    event.Skip();
}

wxButton* MainFrame::makeMenuBtn(const wxString& label, const wxArrayString subItems) {
    int textWidth, textHeight;
    int padding = 20;
    wxButton* btn = new wxButton(menuBar, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    SetWindowColor(btn, C_TITLE_BAR_FOREGROUND, C_TITLE_BAR_BACKGROUND, C_TITLE_BAR_FOREGROUND, C_TITLE_BAR_HOVER_BACKGROUND);
    btn->SetWindowStyleFlag(wxBORDER_NONE | wxTRANSPARENT_WINDOW);

    menuBar->GetTextExtent(label, &textWidth, &textHeight);
    btn->SetMinSize(wxSize(textWidth + padding, -1));

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


wxButton* MainFrame::makeToolBtn(const wxString& label) {
    wxButton* btn = new wxButton(leftToolbar, wxID_ANY, label, wxDefaultPosition, wxSize(40, 40));
    SetWindowColor(btn, C_TOOL_BAR_FOREGROUND, C_TOOL_BAR_BACKGROUND, C_TOOL_BAR_HOVER_FOREGROUND, C_TOOL_BAR_BACKGROUND);
    btn->SetWindowStyleFlag(wxBORDER_NONE | wxTRANSPARENT_WINDOW);
    btn->SetCursor(wxCursor(wxCURSOR_HAND));
    return btn;
};

void MainFrame::SetWindowColor(wxWindow* windows, const wxColour color_default_fore, const wxColour color_default_back, 
        const wxColour color_hover_fore, const wxColour color_hover_back){
    windows->SetForegroundColour(color_default_fore);
    windows->SetBackgroundColour(color_default_back);
    
    if(color_hover_fore != wxColor(1,2,3)){
        windows->Bind(wxEVT_ENTER_WINDOW, [=](wxMouseEvent&) {
            windows->SetForegroundColour(color_hover_fore);
            windows->SetBackgroundColour(color_hover_back);
            windows->Refresh();
        });
        windows->Bind(wxEVT_LEAVE_WINDOW, [=](wxMouseEvent&) {
            windows->SetForegroundColour(color_default_fore);
            windows->SetBackgroundColour(color_default_back);
            windows->Refresh();
        });
    }
}

INNERSTAT_END_NAMESPACE