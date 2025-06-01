#pragma once
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/radiobox.h>
#include <wx/stattext.h>

#include <thread>
#include <atomic>
#include <string>

class DiskLoadPanel : public wxPanel {
public:
    DiskLoadPanel(wxWindow* parent);
    ~DiskLoadPanel();

private:
    wxRadioBox* m_radioType;
    wxTextCtrl* m_txtFileName;
    wxTextCtrl* m_txtFileSize;
    wxButton* m_btnStart;
    wxButton* m_btnStop;
    wxStaticText* m_lblStatus;

    std::thread m_thread;
    std::atomic<bool> m_running;

    void OnStart(wxCommandEvent& evt);
    void OnStop(wxCommandEvent& evt);
    void DiskWriteTask(std::string fileName, size_t fileSize);
    void DiskReadTask(std::string fileName, size_t fileSize);

    wxDECLARE_EVENT_TABLE();
};
