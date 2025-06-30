#ifndef INNERSTAT_CLIENT_MAIN_FRAME_H
#define INNERSTAT_CLIENT_MAIN_FRAME_H

#ifndef INNERSTAT_CLIENT_BASE_H
    #include "innerstat/client/base.h"
#endif

#include <wx/wx.h>
#include <wx/treectrl.h>
#include <wx/splitter.h>
#include "innerstat/client/canvas.h"
#include "innerstat/client/dialog.h"

INNERSTAT_BEGIN_NAMESPACE

class MainFrame : public wxFrame {
private:
    MainCanvas *canvas = nullptr;
    wxPoint m_dragStart;
    bool m_dragging = false;

    void addTopLevelArea(){
        AreaProperties* area = ShowAddAreaDialog(this, 2);
        if (area != nullptr) {
            canvas->AddNewArea(area->label, area->areaType);
            delete area;
        }
    }
    
    /** @brief 키보드 눌림 처리 */
    void OnKeyDown(wxKeyEvent& evt){
        if (evt.GetKeyCode() == WXK_SPACE && canvas) {
            canvas->OnKeyDown(evt);
        } else {
            evt.Skip();
        }
    }
    
    /** @brief 키보드 눌림 해제 처리 */
    void OnKeyUp(wxKeyEvent& evt){
        if (canvas) {
            canvas->OnKeyUp(evt);
        } else {
            evt.Skip();
        }
    }

public:
    wxTreeCtrl *shapeTree;

    /** @brief MainFrame 생성자 - UI 초기화 및 이벤트 바인딩 */
    MainFrame() 
    : wxFrame(nullptr, wxID_ANY, "Inner Stat",
            wxDefaultPosition, wxSize(1000, 600),
            wxDEFAULT_FRAME_STYLE)
    {
        // ---- 1. 커스텀 타이틀바(AppBar) ----
        wxPanel* appBar = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 40));
        appBar->SetBackgroundColour(wxColour(240, 240, 240));
        wxBoxSizer* appBarSizer = new wxBoxSizer(wxHORIZONTAL);

        // 햄버거 메뉴
        wxButton* menuBtn = new wxButton(appBar, wxID_ANY, "≡", wxDefaultPosition, wxSize(36, 32));
        menuBtn->SetBackgroundColour(wxNullColour);
        menuBtn->SetForegroundColour(*wxBLACK);
        menuBtn->SetFont(wxFont(20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
        menuBtn->SetWindowStyle(wxBORDER_NONE);

        // 뒤로가기 버튼
        wxButton* backBtn = new wxButton(appBar, wxID_ANY, "<", wxDefaultPosition, wxSize(32, 32));
        backBtn->SetBackgroundColour(wxNullColour);
        backBtn->SetForegroundColour(*wxBLACK);
        backBtn->SetFont(wxFont(16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
        backBtn->SetWindowStyle(wxBORDER_NONE);

        // 검색창
        wxTextCtrl* searchBox = new wxTextCtrl(appBar, wxID_ANY, "innerStat", wxDefaultPosition, wxSize(250, 28));
        searchBox->SetBackgroundColour(wxColour(255,255,255));
        searchBox->SetForegroundColour(*wxBLACK);

        // Stretch Spacer
        appBarSizer->Add(menuBtn, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
        appBarSizer->Add(backBtn, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
        appBarSizer->AddStretchSpacer(1);
        appBarSizer->Add(searchBox, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 18);

        // 우측: 최소화, 최대화, 닫기 버튼
        wxButton* minBtn = new wxButton(appBar, wxID_ANY, "─", wxDefaultPosition, wxSize(32, 32));
        wxButton* maxBtn = new wxButton(appBar, wxID_ANY, "□", wxDefaultPosition, wxSize(32, 32));
        wxButton* closeBtn = new wxButton(appBar, wxID_ANY, "✕", wxDefaultPosition, wxSize(32, 32));
        minBtn->SetBackgroundColour(wxNullColour);
        maxBtn->SetBackgroundColour(wxNullColour);
        closeBtn->SetBackgroundColour(wxColour(230, 60, 60));
        minBtn->SetForegroundColour(*wxBLACK);
        maxBtn->SetForegroundColour(*wxBLACK);
        closeBtn->SetForegroundColour(*wxWHITE);

        appBarSizer->Add(minBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
        appBarSizer->Add(maxBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
        appBarSizer->Add(closeBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 16);

        appBar->SetSizer(appBarSizer);

        // ---- 2. 메인 Splitter 및 패널 ----
        wxSplitterWindow *splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_BORDER | wxSP_LIVE_UPDATE);
        wxPanel *leftPanel = new wxPanel(splitter);
        wxBoxSizer *leftSizer = new wxBoxSizer(wxVERTICAL);

        wxButton *addAreaBtn = new wxButton(leftPanel, wxID_ANY, "Add Shape");

        shapeTree = new wxTreeCtrl(leftPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS);

        leftPanel->SetBackgroundColour(wxColour(250, 250, 250)); // 미묘하게 밝은 회색으로 (VSCode 사이드바도 완전 흰색 아님)

        addAreaBtn->SetBackgroundColour(wxColour(240,240,240));
        addAreaBtn->SetForegroundColour(*wxBLACK);
        addAreaBtn->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_MEDIUM));

        leftSizer->Add(addAreaBtn, 0, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT | wxRIGHT, 8);
        leftSizer->Add(shapeTree, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);
        leftPanel->SetSizer(leftSizer);

        canvas = new MainCanvas(splitter, this);
        canvas->SetBackgroundColour(wxColour(240, 240, 240)); // VSCode 배경

        splitter->SetMinimumPaneSize(20);
        splitter->SplitVertically(leftPanel, canvas);
        splitter->SetSashPosition(200);

        // ---- 3. 프레임 전체 Sizer ----
        wxBoxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);
        frameSizer->Add(appBar, 0, wxEXPAND);
        frameSizer->Add(splitter, 1, wxEXPAND);

        this->SetSizer(frameSizer);
        this->Layout();

        // ---- 4. 윈도우 이동 처리 (AppBar 드래그) ----
        // 멤버 변수로 wxPoint m_dragStart 선언 필요!
        appBar->Bind(wxEVT_LEFT_DOWN, [=](wxMouseEvent& event){
            m_dragging = true;
            m_dragStart = event.GetPosition();
            appBar->CaptureMouse();
        });

        appBar->Bind(wxEVT_MOTION, [=](wxMouseEvent& event){
            if (m_dragging && appBar->HasCapture() && event.Dragging() && event.LeftIsDown()) {
                wxPoint clientPos = event.GetPosition();
                wxPoint screenPos = ClientToScreen(clientPos - m_dragStart);
                Move(screenPos);
            }
        });

        appBar->Bind(wxEVT_LEFT_UP, [=](wxMouseEvent& event){
            if (m_dragging) {
                m_dragging = false;
                if (appBar->HasCapture()) appBar->ReleaseMouse();
            }
        });

        // 이 부분 추가!
        appBar->Bind(wxEVT_MOUSE_CAPTURE_LOST, [=](wxMouseCaptureLostEvent& evt){
            m_dragging = false;
            // 이미 캡처 해제된 상태라 ReleaseMouse() 안 해도 됨
            // evt.Skip(); // (생략 가능)
        });

        // ---- 5. 윈도우 버튼 이벤트 ----
        closeBtn->Bind(wxEVT_BUTTON, [=](wxCommandEvent&){ Close(); });
        minBtn->Bind(wxEVT_BUTTON, [=](wxCommandEvent&){ Iconize(true); });
        maxBtn->Bind(wxEVT_BUTTON, [=](wxCommandEvent&){ Maximize(!IsMaximized()); });

        addAreaBtn->Bind(wxEVT_BUTTON, [=](wxCommandEvent &) {
            addTopLevelArea();
        });
        shapeTree->Bind(wxEVT_TREE_SEL_CHANGED, [=](wxTreeEvent &evt) {
            canvas->OnTreeSelectionChanged(evt.GetItem());
        });
        shapeTree->Bind(wxEVT_TREE_ITEM_ACTIVATED, [=](wxTreeEvent &evt) {
            canvas->OnTreeLeftDClick(evt.GetItem());
        });


        Bind(wxEVT_CHAR_HOOK, &MainFrame::OnKeyDown, this);
        Bind(wxEVT_KEY_UP, &MainFrame::OnKeyUp, this);

        printf("Main!\n");
    }
};

INNERSTAT_END_NAMESPACE

#endif