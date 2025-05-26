#include <wx/wx.h>
#include <wx/treectrl.h>
#include <wx/splitter.h>
#include "Canvas.h"

class MainFrame : public wxFrame {
private:
    MyCanvas *canvas = nullptr;

public:
    /**
     * @brief MainFrame 생성자 - UI 초기화 및 이벤트 바인딩
     */

    MainFrame() : wxFrame(nullptr, wxID_ANY, "Inner Stat", wxDefaultPosition, wxSize(1000, 600)) {
        wxSplitterWindow *splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_BORDER | wxSP_LIVE_UPDATE);
        wxPanel *leftPanel = new wxPanel(splitter);
        wxBoxSizer *leftSizer = new wxBoxSizer(wxVERTICAL);
        wxTreeCtrl *shapeTree = new wxTreeCtrl(leftPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS);
        wxButton *addAreaBtn = new wxButton(leftPanel, wxID_ANY, "Add Area");

        leftSizer->Add(addAreaBtn, 0, wxEXPAND | wxBOTTOM, 3);
        leftSizer->Add(shapeTree, 1, wxEXPAND | wxBOTTOM, 5);
        leftPanel->SetSizer(leftSizer);

        canvas = new MyCanvas(splitter, shapeTree);

        splitter->SetMinimumPaneSize(20);

        splitter->SplitVertically(leftPanel, canvas);

        splitter->SetSashPosition(150);

        // 버튼 및 리스트 이벤트 바인딩
        addAreaBtn->Bind(wxEVT_BUTTON, [=](wxCommandEvent &) {
            addTopLevelArea();
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

    void addTopLevelArea(){
        wxDialog dlg(canvas, wxID_ANY, "Add Area");
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        
        wxBoxSizer* midSizer = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
        
        wxArrayString types = { "Other", "OS", "VM", "Container", "Network" };
        
        wxTextCtrl* labelCtrl = new wxTextCtrl(&dlg, wxID_ANY, wxString("New Area"));
        wxChoice* typeCtrl = new wxChoice(&dlg, wxID_ANY, wxDefaultPosition, wxDefaultSize, types);

        midSizer->Add(new wxStaticText(&dlg, wxID_ANY, "Label:"), 0, wxALL, 5);
        midSizer->Add(labelCtrl, 0, wxEXPAND | wxALL, 5);
        midSizer->Add(new wxStaticText(&dlg, wxID_ANY, "Type:"), 0, wxALL, 5);
        midSizer->Add(typeCtrl, 0, wxEXPAND | wxALL, 5);

        bottomSizer->Add(new wxButton(&dlg, wxID_OK), 0, wxALL, 10);
        bottomSizer->Add(new wxButton(&dlg, wxID_CANCEL), 0, wxALL, 10);
        
        sizer->Add(midSizer, 1, wxEXPAND | wxALL, 10);
        sizer->Add(bottomSizer, 0, wxALIGN_CENTER);
        
        dlg.SetSizerAndFit(sizer);
        dlg.SetSize(wxSize(200, -1));

        int status = dlg.ShowModal();
        if (status == wxID_OK) {
            canvas->AddNewArea(labelCtrl->GetValue().ToStdString(), Area::getTypeByInt(typeCtrl->GetSelection()));
        }
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