#include <wx/wx.h>
#include <wx/listbox.h>
#include "Canvas.h"

class MainFrame : public wxFrame
{
private:
    MyCanvas *canvas = nullptr;

public:
    MainFrame() : wxFrame(nullptr, wxID_ANY, "Inner Stat", wxDefaultPosition, wxSize(1000, 600))
    {
        wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
        wxPanel *leftPanel = new wxPanel(this);
        wxBoxSizer *leftSizer = new wxBoxSizer(wxVERTICAL);
        wxListBox *shapeList = new wxListBox(leftPanel, wxID_ANY);
        wxButton *addAreaBtn = new wxButton(leftPanel, wxID_ANY, "Add Area");
        wxButton *addNodeBtn = new wxButton(leftPanel, wxID_ANY, "Add Node");

        leftSizer->Add(addAreaBtn, 0, wxEXPAND | wxBOTTOM, 3);
        leftSizer->Add(addNodeBtn, 0, wxEXPAND | wxBOTTOM, 5);
        leftSizer->Add(shapeList, 1, wxEXPAND | wxBOTTOM, 5);
        leftPanel->SetSizer(leftSizer);

        canvas = new MyCanvas(this, shapeList);

        sizer->Add(leftPanel, 0, wxEXPAND | wxALL, 5);
        sizer->Add(canvas, 1, wxEXPAND | wxALL, 5);

        SetSizer(sizer);

        addAreaBtn->Bind(wxEVT_BUTTON, [=](wxCommandEvent &)
                         { canvas->AddNewArea(); });
        addNodeBtn->Bind(wxEVT_BUTTON, [=](wxCommandEvent &)
                         { canvas->AddNewNode(); });
        shapeList->Bind(wxEVT_LISTBOX, [=](wxCommandEvent &evt)
                        { canvas->SelectShape(evt.GetSelection()); });

        wxMenuBar *menuBar = new wxMenuBar();

        wxMenu *fileMenu = new wxMenu();
        fileMenu->Append(wxID_OPEN, "열기(&O)");
        fileMenu->Append(wxID_SAVE, "저장(&S)");

        menuBar->Append(fileMenu, "File"); // ✅ 반드시 빈 문자열 아님
        SetMenuBar(menuBar);                   // ✅ 반드시 호출
        
        Bind(wxEVT_MENU, [=](wxCommandEvent &)
             {
            wxFileDialog dlg(this, "파일 열기", "", "", "텍스트 파일 (*.txt)|*.txt", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
            if (dlg.ShowModal() == wxID_OK) {
                canvas->LoadFromFile(dlg.GetPath().ToStdString());
            } }, wxID_OPEN);

        Bind(wxEVT_MENU, [=](wxCommandEvent &)
             {
            wxFileDialog dlg(this, "파일 저장", "", "", "텍스트 파일 (*.txt)|*.txt", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
            if (dlg.ShowModal() == wxID_OK) {
                canvas->SaveToFile(dlg.GetPath().ToStdString());
            } }, wxID_SAVE);
    }
};

class MyApp : public wxApp
{
public:
    virtual bool OnInit()
    {
        MainFrame *frame = new MainFrame();
        frame->Show();
        return true;
    }
};

wxIMPLEMENT_APP(MyApp);
