#pragma once
#include <wx/panel.h>
#include <wx/spinctrl.h>
#include <wx/button.h>
#include <wx/stattext.h>

#include <vector>
#include <thread>
#include <atomic>

class CpuLoadPanel : public wxPanel {
public:
    CpuLoadPanel(wxWindow* parent);
    ~CpuLoadPanel();

private:
    wxSpinCtrl* m_spinNumCores;
    wxButton* m_btnStart;
    wxButton* m_btnStop;
    wxStaticText* m_lblStatus;

    std::vector<std::thread> m_threads;
    std::atomic<bool> m_running;

    void OnStart(wxCommandEvent& evt);
    void OnStop(wxCommandEvent& evt);
    void CpuTask();

    wxDECLARE_EVENT_TABLE();
};
