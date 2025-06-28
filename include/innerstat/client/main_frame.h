#ifndef INNERSTAT_CLIENT_MAIN_FRAME_H
#define INNERSTAT_CLIENT_MAIN_FRAME_H

#ifndef INNERSTAT_CLIENT_BASE_H
    #include "innerstat/client/base.h"
#endif

#include <wx/wx.h>
#include <wx/treectrl.h>
#include <wx/splitter.h>
#include "innerstat/client/Canvas.h"
#include "innerstat/client/dialog.h"

INNERSTAT_BEGIN_NAMESPACE

class MainFrame : public wxFrame {
private:
    MainCanvas *canvas = nullptr;

    void addTopLevelArea(){
        AreaProperties* area = ShowAddAreaDialog(this);
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
    MainFrame() : wxFrame(nullptr, wxID_ANY, "Inner Stat", wxDefaultPosition, wxSize(1000, 600)) {
        wxSplitterWindow *splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_BORDER | wxSP_LIVE_UPDATE);
        wxPanel *leftPanel = new wxPanel(splitter);
        wxBoxSizer *leftSizer = new wxBoxSizer(wxVERTICAL);
        shapeTree = new wxTreeCtrl(leftPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS);
        wxButton *addAreaBtn = new wxButton(leftPanel, wxID_ANY, "Add Area");

        leftSizer->Add(addAreaBtn, 0, wxEXPAND | wxBOTTOM, 3);
        leftSizer->Add(shapeTree, 1, wxEXPAND | wxBOTTOM, 5);
        leftPanel->SetSizer(leftSizer);

        canvas = new MainCanvas(splitter, this);

        splitter->SetMinimumPaneSize(20);

        splitter->SplitVertically(leftPanel, canvas);

        splitter->SetSashPosition(150);

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

        wxMenuBar *menuBar = new wxMenuBar();
        wxMenu *fileMenu = new wxMenu();
        fileMenu->Append(wxID_OPEN, "열기(&O)");
        fileMenu->Append(wxID_SAVE, "저장(&S)");

        menuBar->Append(fileMenu, "File");
        SetMenuBar(menuBar);

        Bind(wxEVT_MENU, [=](wxCommandEvent &) {
            wxFileDialog dlg(this, "파일 열기", "", "", "텍스트 파일 (*.txt)|*.txt", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
            if (dlg.ShowModal() == wxID_OK) {
                canvas->LoadFromFile(dlg.GetPath().ToStdString());
            }
        }, wxID_OPEN);

        Bind(wxEVT_MENU, [=](wxCommandEvent &) {
            wxFileDialog dlg(this, "파일 저장", "", "", "텍스트 파일 (*.txt)|*.txt", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
            if (dlg.ShowModal() == wxID_OK) {
                canvas->SaveToFile(dlg.GetPath().ToStdString());
            }
        }, wxID_SAVE);
    }
};

INNERSTAT_END_NAMESPACE

#endif