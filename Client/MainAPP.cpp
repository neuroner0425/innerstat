#include <wx/wx.h>
#include <wx/treectrl.h>
#include "Canvas.h"

class MainFrame : public wxFrame {
private:
    MyCanvas *canvas = nullptr;

public:
    /**
     * @brief MainFrame 생성자 - UI 초기화 및 이벤트 바인딩
     */

    MainFrame() : wxFrame(nullptr, wxID_ANY, "Inner Stat", wxDefaultPosition, wxSize(1000, 600)) {
        wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
        wxPanel *leftPanel = new wxPanel(this);
        wxBoxSizer *leftSizer = new wxBoxSizer(wxVERTICAL);
        wxTreeCtrl *shapeTree = new wxTreeCtrl(leftPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS);
        wxButton *addAreaBtn = new wxButton(leftPanel, wxID_ANY, "Add Area");

        leftSizer->Add(addAreaBtn, 0, wxEXPAND | wxBOTTOM, 3);
        leftSizer->Add(shapeTree, 1, wxEXPAND | wxBOTTOM, 5);
        leftPanel->SetSizer(leftSizer);

        canvas = new MyCanvas(this, shapeTree);

        sizer->Add(leftPanel, 0, wxEXPAND | wxALL, 5);
        sizer->Add(canvas, 1, wxEXPAND | wxALL, 5);

        SetSizer(sizer);

        // 버튼 및 리스트 이벤트 바인딩
        addAreaBtn->Bind(wxEVT_BUTTON, [=](wxCommandEvent &) {
            canvas->AddNewArea("os");
        });
        shapeTree->Bind(wxEVT_TREE_SEL_CHANGED, [=](wxTreeEvent &evt) {
            canvas->OnTreeSelectionChanged(evt.GetItem());
        });

        wxMenuBar *menuBar = new wxMenuBar();
        wxMenu *fileMenu = new wxMenu();
        fileMenu->Append(wxID_OPEN, "열기(&O)");
        fileMenu->Append(wxID_SAVE, "저장(&S)");

        menuBar->Append(fileMenu, "File");
        SetMenuBar(menuBar);

        // 메뉴 이벤트 바인딩
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