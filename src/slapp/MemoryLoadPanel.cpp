#include "innerstat/slapp/memory.h"

#include <wx/msgdlg.h>
#include <wx/sizer.h>

#include <fstream>

enum { ID_MEM_ALLOC = wxID_HIGHEST+200, ID_MEM_CLEAR, ID_MEM_TIMED_START, ID_MEM_TIMED_STOP };

wxBEGIN_EVENT_TABLE(MemoryLoadPanel, wxPanel)
    EVT_BUTTON(ID_MEM_ALLOC, MemoryLoadPanel::OnAlloc)
    EVT_BUTTON(ID_MEM_CLEAR, MemoryLoadPanel::OnClear)
    EVT_BUTTON(ID_MEM_TIMED_START, MemoryLoadPanel::OnTimedStart)
    EVT_BUTTON(ID_MEM_TIMED_STOP, MemoryLoadPanel::OnTimedStop)
wxEND_EVENT_TABLE()

MemoryLoadPanel::MemoryLoadPanel(wxWindow* parent)
    : wxPanel(parent), m_totalBytes(0), m_timedRunning(false)
{
    wxStaticBoxSizer* sbox = new wxStaticBoxSizer(wxVERTICAL, this, L"메모리 부하(누수)");
    wxBoxSizer* row1 = new wxBoxSizer(wxHORIZONTAL);
    row1->Add(new wxStaticText(this, wxID_ANY, L"크기(MB):"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    m_txtSizeMB = new wxTextCtrl(this, wxID_ANY, "100");
    row1->Add(m_txtSizeMB, 1, wxRIGHT, 8);
    m_btnAlloc = new wxButton(this, ID_MEM_ALLOC, L"할당");
    row1->Add(m_btnAlloc, 0, wxRIGHT, 8);
    m_btnClear = new wxButton(this, ID_MEM_CLEAR, L"해제");
    row1->Add(m_btnClear, 0, wxRIGHT, 0);

    wxBoxSizer* row2 = new wxBoxSizer(wxHORIZONTAL);
    row2->Add(new wxStaticText(this, wxID_ANY, L"주기(초):"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    m_txtIntervalSec = new wxTextCtrl(this, wxID_ANY, "5");
    row2->Add(m_txtIntervalSec, 1, wxRIGHT, 8);
    m_btnTimedStart = new wxButton(this, ID_MEM_TIMED_START, L"주기적 할당 시작");
    row2->Add(m_btnTimedStart, 0, wxRIGHT, 8);
    m_btnTimedStop = new wxButton(this, ID_MEM_TIMED_STOP, L"중지");
    m_btnTimedStop->Disable();
    row2->Add(m_btnTimedStop, 0, wxRIGHT, 0);

    m_lblTotalAlloc = new wxStaticText(this, wxID_ANY, L"총 할당: 0 MB");
    m_lblStatus = new wxStaticText(this, wxID_ANY, L"상태: 대기");

    sbox->Add(row1, 0, wxALL|wxEXPAND, 4);
    sbox->Add(row2, 0, wxALL|wxEXPAND, 4);
    sbox->Add(m_lblTotalAlloc, 0, wxALL, 4);
    sbox->Add(m_lblStatus, 0, wxALL, 4);

    SetSizer(sbox);
}

MemoryLoadPanel::~MemoryLoadPanel() {
    m_timedRunning = false;
    if (m_timedThread.joinable()) m_timedThread.join();
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto* p : m_blocks) delete[] p;
    m_blocks.clear();
    m_totalBytes = 0;
}

void MemoryLoadPanel::AddMemory(size_t sz) {
    std::lock_guard<std::mutex> lock(m_mutex);
    try {
        char* p = new char[sz];
        for (size_t i=0; i<sz; i+=4096) p[i] = (char)(i%256);
        m_blocks.push_back(p);
        m_totalBytes += sz;
        m_lblTotalAlloc->SetLabel(wxString::Format(L"총 할당: %zu MB", m_totalBytes/(1024*1024)));
        m_lblStatus->SetLabel(L"상태: 할당됨");
    } catch (...) {
        m_lblStatus->SetLabel(L"상태: 할당 실패");
    }
}
void MemoryLoadPanel::OnAlloc(wxCommandEvent&) {
    long mb=0; m_txtSizeMB->GetValue().ToLong(&mb);
    if (mb<=0) {
        wxMessageBox(L"1 이상 MB 입력", L"오류", wxOK|wxICON_ERROR); return;
    }
    AddMemory((size_t)mb*1024*1024);
}
void MemoryLoadPanel::OnClear(wxCommandEvent&) {
    if (m_timedRunning) {
        wxMessageBox(L"주기적 할당 중 해제 불가", L"오류", wxOK|wxICON_ERROR); return;
    }
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto* p : m_blocks) delete[] p;
    m_blocks.clear(); m_totalBytes=0;
    m_lblTotalAlloc->SetLabel(L"총 할당: 0 MB");
    m_lblStatus->SetLabel(L"상태: 메모리 해제됨");
}
void MemoryLoadPanel::OnTimedStart(wxCommandEvent&) {
    std::ofstream("mem_overload_status.txt") << "ON";
    long mb=0, sec=0;
    m_txtSizeMB->GetValue().ToLong(&mb); m_txtIntervalSec->GetValue().ToLong(&sec);
    if (mb<=0 || sec<=0) {
        wxMessageBox(L"1 이상 MB, 1 이상 초 입력", L"오류", wxOK|wxICON_ERROR); return;
    }
    m_timedAllocSize = (size_t)mb*1024*1024;
    m_timedAllocInterval = (int)sec;
    m_timedRunning = true;
    m_btnTimedStart->Disable(); m_btnTimedStop->Enable();
    m_lblStatus->SetLabel(L"상태: 주기적 할당 중");
    m_timedThread = std::thread(&MemoryLoadPanel::TimedAllocTask, this);
}
void MemoryLoadPanel::OnTimedStop(wxCommandEvent&) {
    std::ofstream("mem_overload_status.txt") << "OFF";
    m_timedRunning = false;
    if (m_timedThread.joinable()) m_timedThread.join();
    m_btnTimedStart->Enable(); m_btnTimedStop->Disable();
    m_lblStatus->SetLabel(L"상태: 대기");
}
void MemoryLoadPanel::TimedAllocTask() {
    while (m_timedRunning) {
        AddMemory(m_timedAllocSize);
        for (int i=0; i<m_timedAllocInterval*10; ++i) {
            if (!m_timedRunning) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}
