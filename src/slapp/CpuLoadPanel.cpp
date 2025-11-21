#include "innerstat/slapp/cpu.h"
#include "innerstat/slapp/main.h" // For MainFrame
#include <wx/sizer.h>

enum { ID_CPU_START = wxID_HIGHEST+100, ID_CPU_STOP };

wxBEGIN_EVENT_TABLE(CpuLoadPanel, wxPanel)
    EVT_BUTTON(ID_CPU_START, CpuLoadPanel::OnStart)
    EVT_BUTTON(ID_CPU_STOP, CpuLoadPanel::OnStop)
wxEND_EVENT_TABLE()

CpuLoadPanel::CpuLoadPanel(wxWindow* parent, MainFrame* mainFrame)
    : wxPanel(parent), m_parentFrame(mainFrame), m_running(false)
{
    wxStaticBoxSizer* sbox = new wxStaticBoxSizer(wxVERTICAL, this, L"CPU 부하");
    wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);

    hbox->Add(new wxStaticText(this, wxID_ANY, L"코어 수:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
    int maxCores = std::thread::hardware_concurrency();
    if (maxCores == 0) maxCores = 8;
    m_spinNumCores = new wxSpinCtrl(this, wxID_ANY, "1", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, maxCores*2, 1);
    hbox->Add(m_spinNumCores, 1, wxEXPAND | wxRIGHT, 8);

    m_btnStart = new wxButton(this, ID_CPU_START, L"시작");
    m_btnStop  = new wxButton(this, ID_CPU_STOP, L"중지");
    m_btnStop->Disable();

    m_lblStatus = new wxStaticText(this, wxID_ANY, L"상태: 대기");

    sbox->Add(hbox, 0, wxALL|wxEXPAND, 4);
    sbox->Add(m_btnStart, 0, wxALL|wxEXPAND, 4);
    sbox->Add(m_btnStop, 0, wxALL|wxEXPAND, 4);
    sbox->Add(m_lblStatus, 0, wxALL|wxEXPAND, 4);

    SetSizer(sbox);
}

CpuLoadPanel::~CpuLoadPanel() {
    m_running = false;
    for(auto& th : m_threads)
        if (th.joinable()) th.join();
}

void CpuLoadPanel::OnStart(wxCommandEvent&) {
    if (!m_running) {
        m_parentFrame->UpdateLoadStatus("cpu", true);
        int cores = m_spinNumCores->GetValue();
        if (cores <= 0) return;
        m_running = true;
        m_btnStart->Disable();
        m_btnStop->Enable();
        m_spinNumCores->Disable();
        m_lblStatus->SetLabel(L"상태: 실행 중(" + wxString::Format("%d", cores) + L"코어)");

        m_threads.clear();
        for (int i = 0; i < cores; ++i)
            m_threads.emplace_back(&CpuLoadPanel::CpuTask, this);
    }
}
void CpuLoadPanel::OnStop(wxCommandEvent&) {
    m_parentFrame->UpdateLoadStatus("cpu", false);
    m_running = false;
    for(auto& th : m_threads)
        if (th.joinable()) th.join();
    m_threads.clear();
    m_btnStart->Enable();
    m_btnStop->Disable();
    m_spinNumCores->Enable();
    m_lblStatus->SetLabel(L"상태: 대기");
}
void CpuLoadPanel::CpuTask() {
    volatile double v = 1.23;
    while (m_running) {
        for (long i=0; i<200000; ++i) {
            if (!m_running) break;
            v *= 1.000000001;
            v /= 1.000000001;
        }
    }
}
