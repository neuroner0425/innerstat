#pragma once
#include <wx/frame.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/socket.h>
#include <thread>
#include <atomic>
#include <string>

class CpuLoadPanel;
class MemoryLoadPanel;
class DiskLoadPanel;

struct OverloadStatus {
    bool cpu = false;
    bool disk = false;
    bool memory = false;
    int port = 0; // 0 means not occupied
};

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title);
    ~MainFrame();

    void UpdateLoadStatus(const std::string& component, bool is_running);

private:
    void OnStartButtonClick(wxCommandEvent& event);
    void OnApplyFrequency(wxCommandEvent& event);
    void WriteStatusFile();

    CpuLoadPanel* m_cpuPanel;
    MemoryLoadPanel* m_memPanel;
    DiskLoadPanel* m_diskPanel;

    // Controls
    wxStaticText* m_portLabel;
    wxTextCtrl* m_portInput;
    wxStaticText* m_frequencyLabel;
    wxTextCtrl* m_frequencyInput;
    wxCheckBox* m_fileLogCheck;
    wxStaticText* m_filePathLabel;
    wxTextCtrl* m_filePathInput;
    wxCheckBox* m_terminalLogCheck;
    wxButton* m_startButton;
    wxButton* m_applyButton;
    wxStaticText* m_currentFreqDisplay;

    // Thread and Socket management
    std::thread* m_logThread;
    std::atomic<bool> m_logThreadRunning;
    std::atomic<double> m_currentFrequency;
    wxSocketServer* m_socketServer;

    // Centralized status
    OverloadStatus m_status;
};
