#pragma once
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>

#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

class MemoryLoadPanel : public wxPanel {
public:
    MemoryLoadPanel(wxWindow* parent);
    ~MemoryLoadPanel();

private:
    wxTextCtrl* m_txtSizeMB;
    wxButton* m_btnAlloc;
    wxButton* m_btnClear;
    wxButton* m_btnTimedStart;
    wxButton* m_btnTimedStop;
    wxTextCtrl* m_txtIntervalSec;
    wxStaticText* m_lblStatus;
    wxStaticText* m_lblTotalAlloc;

    std::vector<char*> m_blocks;
    size_t m_totalBytes;
    std::mutex m_mutex;

    std::thread m_timedThread;
    std::atomic<bool> m_timedRunning;
    size_t m_timedAllocSize;
    int m_timedAllocInterval;

    void OnAlloc(wxCommandEvent& evt);
    void OnClear(wxCommandEvent& evt);
    void OnTimedStart(wxCommandEvent& evt);
    void OnTimedStop(wxCommandEvent& evt);
    void TimedAllocTask();
    void AddMemory(size_t sz);

    wxDECLARE_EVENT_TABLE();
};
