#pragma once
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>

#include <thread>
#include <atomic>
#include <string>

class MainFrame; // Forward declaration

class DiskLoadPanel : public wxPanel {
public:
    DiskLoadPanel(wxWindow* parent, MainFrame* mainFrame);
    ~DiskLoadPanel();

private:
    MainFrame* m_parentFrame;
    wxTextCtrl* m_txtFileName;
    wxTextCtrl* m_txtFileSize;
    wxButton* m_btnStart;
    wxButton* m_btnStop;
    wxStaticText* m_lblStatus;

    std::thread m_thread;
    std::atomic<bool> m_running;

    void OnStart(wxCommandEvent& evt);
    void OnStop(wxCommandEvent& evt);
    void DiskLoadTask(std::string fileName, size_t fileSize);

    wxDECLARE_EVENT_TABLE();
};
