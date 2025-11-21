#include <wx/panel.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/event.h>

#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

wxDECLARE_EVENT(EVT_MEM_UPDATE, wxCommandEvent);

class MainFrame; // Forward declaration

class MemoryLoadPanel : public wxPanel {
public:
    MemoryLoadPanel(wxWindow* parent, MainFrame* mainFrame);
    ~MemoryLoadPanel();

private:
    MainFrame* m_parentFrame;
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
    void OnUpdateUI(wxCommandEvent& evt);
    void TimedAllocTask();
    void AddMemory(size_t sz);

    wxDECLARE_EVENT_TABLE();
};
