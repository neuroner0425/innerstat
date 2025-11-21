#include "innerstat/slapp/disk.h"
#include "innerstat/slapp/main.h" // For MainFrame

#include <wx/msgdlg.h>
#include <wx/sizer.h>

#include <fstream>
#include <random>
#include <cstdio> // For std::remove

enum { ID_DISK_START = wxID_HIGHEST+300, ID_DISK_STOP };

wxBEGIN_EVENT_TABLE(DiskLoadPanel, wxPanel)
    EVT_BUTTON(ID_DISK_START, DiskLoadPanel::OnStart)
    EVT_BUTTON(ID_DISK_STOP, DiskLoadPanel::OnStop)
wxEND_EVENT_TABLE()

DiskLoadPanel::DiskLoadPanel(wxWindow* parent, MainFrame* mainFrame)
    : wxPanel(parent), m_parentFrame(mainFrame), m_running(false)
{
    wxStaticBoxSizer* sbox = new wxStaticBoxSizer(wxVERTICAL, this, L"디스크 I/O 부하");
    
    wxBoxSizer* fbox = new wxBoxSizer(wxHORIZONTAL);
    fbox->Add(new wxStaticText(this, wxID_ANY, L"파일명:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    m_txtFileName = new wxTextCtrl(this, wxID_ANY, "disk_test.dat");
    fbox->Add(m_txtFileName, 1, wxRIGHT, 8);

    wxBoxSizer* sbox2 = new wxBoxSizer(wxHORIZONTAL);
    sbox2->Add(new wxStaticText(this, wxID_ANY, L"크기(MB):"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    m_txtFileSize = new wxTextCtrl(this, wxID_ANY, "50");
    sbox2->Add(m_txtFileSize, 1, wxRIGHT, 0);

    m_btnStart = new wxButton(this, ID_DISK_START, L"시작");
    m_btnStop = new wxButton(this, ID_DISK_STOP, L"중지");
    m_btnStop->Disable();
    m_lblStatus = new wxStaticText(this, wxID_ANY, L"상태: 대기");

    sbox->Add(fbox, 0, wxALL|wxEXPAND, 4);
    sbox->Add(sbox2, 0, wxALL|wxEXPAND, 4);
    sbox->Add(m_btnStart, 0, wxALL|wxEXPAND, 4);
    sbox->Add(m_btnStop, 0, wxALL|wxEXPAND, 4);
    sbox->Add(m_lblStatus, 0, wxALL, 4);

    SetSizer(sbox);
}

DiskLoadPanel::~DiskLoadPanel() {
    m_running = false;
    if (m_thread.joinable()) m_thread.join();
    // Final cleanup, just in case
    std::string fileName = m_txtFileName->GetValue().ToStdString();
    if (!fileName.empty()) {
        std::remove(fileName.c_str());
    }
}

void DiskLoadPanel::OnStart(wxCommandEvent&) {
    m_parentFrame->UpdateLoadStatus("disk", true);
    long mb=0;
    m_txtFileSize->GetValue().ToLong(&mb);
    if (mb<=0) {
        wxMessageBox(L"1 이상 MB 입력", L"오류", wxOK|wxICON_ERROR); return;
    }
    wxString fname = m_txtFileName->GetValue();
    if (fname.IsEmpty()) {
        wxMessageBox(L"파일명 입력", L"오류", wxOK|wxICON_ERROR); return;
    }
    std::string fileName(fname.mb_str());
    size_t sizeBytes = (size_t)mb*1024*1024;
    
    m_running = true;
    m_btnStart->Disable(); m_btnStop->Enable();
    m_txtFileName->Disable(); m_txtFileSize->Disable();
    m_lblStatus->SetLabel(L"상태: 쓰기/읽기 반복 중");
    
    m_thread = std::thread(&DiskLoadPanel::DiskLoadTask, this, fileName, sizeBytes);
}

void DiskLoadPanel::OnStop(wxCommandEvent&) {
    m_parentFrame->UpdateLoadStatus("disk", false);
    m_running = false;
    if (m_thread.joinable()) m_thread.join();
    
    // Clean up the file on stop
    std::string fileName = m_txtFileName->GetValue().ToStdString();
    if (!fileName.empty()) {
        std::remove(fileName.c_str());
    }

    m_btnStart->Enable(); m_btnStop->Disable();
    m_txtFileName->Enable(); m_txtFileSize->Enable();
    m_lblStatus->SetLabel(L"상태: 대기");
}

void DiskLoadPanel::DiskLoadTask(std::string fileName, size_t fileSize)
{
    std::vector<char> buffer(1024 * 1024); // 1MB buffer
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    for (auto& b : buffer) b = (char)dis(gen);

    while (m_running) {
        // Delete before writing
        std::remove(fileName.c_str());

        // === WRITE PHASE ===
        std::ofstream outFile(fileName, std::ios::binary);
        if (!outFile) {
            // Can't wxLog from thread, so just update status and stop
            m_running = false;
            CallAfter([this]() {
                m_lblStatus->SetLabel(L"상태: 쓰기 오류");
            });
            break;
        }
        for (size_t i = 0; i < fileSize / buffer.size() && m_running; ++i) {
            outFile.write(buffer.data(), buffer.size());
        }
        outFile.close();
        if (!m_running) break;

        // === READ PHASE ===
        std::ifstream inFile(fileName, std::ios::binary);
        if (!inFile) {
            m_running = false;
            CallAfter([this]() {
                m_lblStatus->SetLabel(L"상태: 읽기 오류");
            });
            break;
        }
        while (inFile.read(buffer.data(), buffer.size()) && m_running) {
            // Reading...
        }
        inFile.close();
        if (!m_running) break;

        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Small delay between cycles
    }

    // Final cleanup
    std::remove(fileName.c_str());
}
