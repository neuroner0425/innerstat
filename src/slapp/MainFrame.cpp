#include "innerstat/slapp/main.h"
#include "innerstat/slapp/cpu.h"
#include "innerstat/slapp/memory.h"
#include "innerstat/slapp/disk.h"

#include <wx/sizer.h>
#include <wx/log.h>
#include <iostream>
#include <fstream>
#include <string>

// Thread function now only handles logging
void LogThread(bool logToFile, std::string filePath, bool logToTerminal, std::atomic<bool>* isRunning, std::atomic<double>* frequency) {
    std::ofstream outFile;
    if (logToFile) {
        outFile.open(filePath, std::ios::app);
        if (!outFile.is_open()) {
            std::cerr << "slapp: ERROR opening log file " << filePath << std::endl;
            // Can't wxLog from a thread, and can't easily signal error back, so just exit.
            *isRunning = false;
            return;
        }
    }

    long long counter = 0;
    while (isRunning->load()) {
        double currentFreq = frequency->load();
        
        std::string message = "slapp: Log message " + std::to_string(counter + 1);

        if (logToTerminal) std::cout << message << std::endl;
        if (logToFile) outFile << message << std::endl;
        
        counter++;

        if (currentFreq == 0) {
            break; 
        }

        int sleep_ms = (currentFreq > 0) ? static_cast<int>(1000.0 / currentFreq) : 1000;
        if (sleep_ms < 1) sleep_ms = 1;
        
        for (int i = 0; i < sleep_ms / 10 && isRunning->load(); ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    if (logToFile) outFile.close();
    *isRunning = false;
}


MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)),
      m_logThread(nullptr), m_logThreadRunning(false), m_socketServer(nullptr)
{
    m_currentFrequency = 10.0;
    wxPanel* mainPanel = new wxPanel(this);
    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* hbox_panels = new wxBoxSizer(wxHORIZONTAL);
    m_cpuPanel = new CpuLoadPanel(mainPanel, this);
    m_memPanel = new MemoryLoadPanel(mainPanel, this);
    m_diskPanel = new DiskLoadPanel(mainPanel, this);
    hbox_panels->Add(m_cpuPanel, 1, wxALL | wxEXPAND, 8);
    hbox_panels->Add(m_memPanel, 1, wxALL | wxEXPAND, 8);
    hbox_panels->Add(m_diskPanel, 1, wxALL | wxEXPAND, 8);
    vbox->Add(hbox_panels, 1, wxEXPAND | wxALL, 0);

    // --- Controls ---
    wxBoxSizer* hbox_controls1 = new wxBoxSizer(wxHORIZONTAL);
    m_portLabel = new wxStaticText(mainPanel, wxID_ANY, "Port:");
    m_portInput = new wxTextCtrl(mainPanel, wxID_ANY, "8080");
    m_frequencyLabel = new wxStaticText(mainPanel, wxID_ANY, "Logs/sec:");
    m_frequencyInput = new wxTextCtrl(mainPanel, wxID_ANY, std::to_string(m_currentFrequency.load()));
    
hbox_controls1->Add(m_portLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 10);
    hbox_controls1->Add(m_portInput, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
    hbox_controls1->Add(m_frequencyLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 10);
    hbox_controls1->Add(m_frequencyInput, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);

    m_applyButton = new wxButton(mainPanel, wxID_ANY, "Apply");
    hbox_controls1->Add(m_applyButton, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
    
m_currentFreqDisplay = new wxStaticText(mainPanel, wxID_ANY, "Current Freq: N/A");
    hbox_controls1->Add(m_currentFreqDisplay, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 10);

    vbox->Add(hbox_controls1, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);

    wxBoxSizer* hbox_controls2 = new wxBoxSizer(wxHORIZONTAL);
    m_terminalLogCheck = new wxCheckBox(mainPanel, wxID_ANY, "Log to Terminal");
    m_terminalLogCheck->SetValue(true);
    m_fileLogCheck = new wxCheckBox(mainPanel, wxID_ANY, "Log to File");
    hbox_controls2->Add(m_terminalLogCheck, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 10);
    hbox_controls2->Add(m_fileLogCheck, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 10);
    vbox->Add(hbox_controls2, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);

    wxBoxSizer* hbox_controls3 = new wxBoxSizer(wxHORIZONTAL);
    m_filePathLabel = new wxStaticText(mainPanel, wxID_ANY, "File Path:");
    m_filePathInput = new wxTextCtrl(mainPanel, wxID_ANY, "/tmp/slapp.log");
    hbox_controls3->Add(m_filePathLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 10);
    hbox_controls3->Add(m_filePathInput, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
    vbox->Add(hbox_controls3, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
    
m_startButton = new wxButton(mainPanel, wxID_ANY, "Start");
    vbox->Add(m_startButton, 0, wxALIGN_CENTER | wxALL, 10);

    mainPanel->SetSizer(vbox);
    this->Centre();

    m_startButton->Bind(wxEVT_BUTTON, &MainFrame::OnStartButtonClick, this);
    m_applyButton->Bind(wxEVT_BUTTON, &MainFrame::OnApplyFrequency, this);

    WriteStatusFile(); // Write initial status
}
MainFrame::~MainFrame() {
    if (m_logThreadRunning) {
        m_logThreadRunning = false;
        if (m_logThread && m_logThread->joinable()) {
            m_logThread->join();
        }
        delete m_logThread;
    }
    if (m_socketServer) {
        delete m_socketServer;
    }
    // Final status write on exit
    m_status.cpu = false;
    m_status.disk = false;
    m_status.memory = false;
    m_status.port = 0;
    WriteStatusFile();
}

void MainFrame::WriteStatusFile() {
    std::ofstream statusFile("slinfo.txt");
    if (statusFile.is_open()) {
        statusFile << "cpu=" << (m_status.cpu ? "ON" : "OFF") << std::endl;
        statusFile << "disk=" << (m_status.disk ? "ON" : "OFF") << std::endl;
        statusFile << "memory=" << (m_status.memory ? "ON" : "OFF") << std::endl;
        statusFile << "port=" << m_status.port << std::endl;
    }
}

void MainFrame::UpdateLoadStatus(const std::string& component, bool is_running) {
    if (component == "cpu") m_status.cpu = is_running;
    else if (component == "disk") m_status.disk = is_running;
    else if (component == "memory") m_status.memory = is_running;
    WriteStatusFile();
}

void MainFrame::OnApplyFrequency(wxCommandEvent& event) {
    double newFreq;
    if (m_frequencyInput->GetValue().ToDouble(&newFreq) && newFreq >= 0) {
        m_currentFrequency = newFreq;
        m_currentFreqDisplay->SetLabel(wxString::Format("Current Freq: %.2f", newFreq));
    } else {
        wxLogMessage("ERROR: Invalid frequency. Must be a non-negative number.");
        m_frequencyInput->SetValue(std::to_string(m_currentFrequency.load()));
    }
}

void MainFrame::OnStartButtonClick(wxCommandEvent& event) {
    if (m_logThreadRunning) {
        // Stop the thread and socket
        m_logThreadRunning = false;
        if (m_logThread && m_logThread->joinable()) {
            m_logThread->join();
        }
        delete m_logThread;
        m_logThread = nullptr;

        if (m_socketServer) {
            m_socketServer->Destroy();
            m_socketServer = nullptr;
        }
        m_status.port = 0;
        WriteStatusFile();

        m_startButton->SetLabel("Start");
        
        // Re-enable controls
        m_portInput->Enable();
        m_filePathInput->Enable();
        m_fileLogCheck->Enable();
        m_terminalLogCheck->Enable();
        m_currentFreqDisplay->SetLabel("Current Freq: N/A");

    } else {
        // Start the thread and socket
        long port_long = 0;
        double frequency = 0.0;

        if (!m_portInput->GetValue().ToLong(&port_long)) {
            wxLogMessage("ERROR: Invalid Port. Must be a number."); return;
        }
        if (!m_frequencyInput->GetValue().ToDouble(&frequency)) {
            wxLogMessage("ERROR: Invalid Frequency. Must be a number."); return;
        }
        
        int port = static_cast<int>(port_long);
        bool toFile = m_fileLogCheck->GetValue();
        bool toTerminal = m_terminalLogCheck->GetValue();
        std::string filePath = m_filePathInput->GetValue().ToStdString();

        if ((port <= 0 || port > 65535) && port != 0) {
            wxLogMessage("ERROR: Invalid port number. Use 0 for no port, or 1-65535."); return;
        }
        if (frequency < 0) {
            wxLogMessage("ERROR: Frequency must be non-negative."); return;
        }
        if (toFile && filePath.empty()) {
            wxLogMessage("ERROR: File path cannot be empty when logging to file."); return;
        }
        if (!toFile && !toTerminal) {
            wxLogMessage("ERROR: At least one log destination (Terminal or File) must be selected."); return;
        }
        
        // Setup Socket
        if (port > 0) {
            wxIPV4address addr;
            addr.Service(port);
            m_socketServer = new wxSocketServer(addr);
            if (!m_socketServer->IsOk()) {
                wxLogMessage(wxString::Format("ERROR: Could not listen on port %d. Is it already in use?", port));
                delete m_socketServer;
                m_socketServer = nullptr;
                return;
            }
            m_status.port = port;
            WriteStatusFile();
        }

        // Disable controls
        m_portInput->Disable();
        m_filePathInput->Disable();
        m_fileLogCheck->Disable();
        m_terminalLogCheck->Disable();

        // Start logging thread
        m_currentFrequency = frequency;
        m_currentFreqDisplay->SetLabel(wxString::Format("Current Freq: %.2f", frequency));
        m_logThreadRunning = true;
        m_logThread = new std::thread(LogThread, toFile, filePath, toTerminal, &m_logThreadRunning, &m_currentFrequency);
        
        m_startButton->SetLabel("Stop");
    }
}

