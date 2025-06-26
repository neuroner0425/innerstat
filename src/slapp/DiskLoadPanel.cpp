#include "innerstat/slapp/disk.h"

#include <wx/msgdlg.h>
#include <wx/sizer.h>

#include <fstream>
#include <random>


enum { ID_DISK_START = wxID_HIGHEST+300, ID_DISK_STOP };

wxBEGIN_EVENT_TABLE(DiskLoadPanel, wxPanel)
    EVT_BUTTON(ID_DISK_START, DiskLoadPanel::OnStart)
    EVT_BUTTON(ID_DISK_STOP, DiskLoadPanel::OnStop)
wxEND_EVENT_TABLE()

DiskLoadPanel::DiskLoadPanel(wxWindow* parent)
    : wxPanel(parent), m_running(false)
{
    wxStaticBoxSizer* sbox = new wxStaticBoxSizer(wxVERTICAL, this, L"디스크 I/O 부하");
    wxString choices[] = { L"쓰기", L"읽기" };
    m_radioType = new wxRadioBox(this, wxID_ANY, L"동작", wxDefaultPosition, wxDefaultSize, 2, choices, 1, wxRA_SPECIFY_COLS);
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

    sbox->Add(m_radioType, 0, wxALL|wxEXPAND, 4);
    sbox->Add(fbox, 0, wxALL|wxEXPAND, 4);
    sbox->Add(sbox2, 0, wxALL|wxEXPAND, 4);
    sbox->Add(m_btnStart, 0, wxALL, 4);
    sbox->Add(m_btnStop, 0, wxALL, 4);
    sbox->Add(m_lblStatus, 0, wxALL, 4);

    SetSizer(sbox);
}

DiskLoadPanel::~DiskLoadPanel() {
    m_running = false;
    if (m_thread.joinable()) m_thread.join();
}

void DiskLoadPanel::OnStart(wxCommandEvent&) {
    std::ofstream("disk_overload_status.txt") << "ON";
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
    m_txtFileName->Disable(); m_txtFileSize->Disable(); m_radioType->Disable();
    m_lblStatus->SetLabel(L"상태: 실행 중");
    if (m_radioType->GetSelection() == 0) {
        m_thread = std::thread(&DiskLoadPanel::DiskWriteTask, this, fileName, sizeBytes);
    } else {
        m_thread = std::thread(&DiskLoadPanel::DiskReadTask, this, fileName, sizeBytes);
    }
}
void DiskLoadPanel::OnStop(wxCommandEvent&) {
    std::ofstream("disk_overload_status.txt") << "OFF";
    m_running = false;
    if (m_thread.joinable()) m_thread.join();
    m_btnStart->Enable(); m_btnStop->Disable();
    m_txtFileName->Enable(); m_txtFileSize->Enable(); m_radioType->Enable();
    m_lblStatus->SetLabel(L"상태: 대기");
}
void DiskLoadPanel::DiskWriteTask(std::string fileName, size_t fileSize)
{
    const int numThreads = 4; // 동시 스레드 수. 시스템 상황에 따라 조정
    std::vector<std::thread> threads;

    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&]() {
            std::vector<char> buf(4 * 1024 * 1024); // 버퍼 4MB
            std::random_device rd; std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 255);
            for (auto& b : buf) b = (char)dis(gen);
            std::uniform_int_distribution<size_t> pos_dis(0, fileSize - buf.size());

            while (m_running) {
                std::fstream f(fileName, std::ios::in | std::ios::out | std::ios::binary);
                if (!f) { // 없으면 새로 생성
                    std::ofstream nf(fileName, std::ios::binary | std::ios::trunc);
                    nf.seekp(fileSize - 1); nf.write("", 1); nf.close();
                    continue;
                }
                for (int i = 0; i < 128 && m_running; ++i) {
                    size_t offset = pos_dis(gen);
                    f.seekp(offset);
                    f.write(buf.data(), buf.size());
                    f.flush(); // flush로 OS 캐시 적중률 감소
                }
                f.close();
                // 슬립 없음 (또는 아주 짧게)
                // std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        });
    }
    for (auto& th : threads) if (th.joinable()) th.join();
}

void DiskLoadPanel::DiskReadTask(std::string fileName, size_t fileSize)
{
    // (파일 여러 개 읽고 싶으면, 파일명+숫자 등으로 리스트 생성)
    const int numThreads = 4;
    std::vector<std::thread> threads;
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&]() {
            std::vector<char> buf(4 * 1024 * 1024); // 4MB
            std::random_device rd; std::mt19937 gen(rd());
            std::uniform_int_distribution<size_t> pos_dis(0, fileSize - buf.size());
            while (m_running) {
                std::ifstream f(fileName, std::ios::binary);
                if (!f) {
                    CallAfter([this]() {
                        m_lblStatus->SetLabel(L"상태: 파일없음/읽기 오류");
                    });
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    continue;
                }
                for (int i = 0; i < 512 && m_running; ++i) {
                    size_t offset = pos_dis(gen);
                    f.seekg(offset);
                    f.read(buf.data(), buf.size());
                }
                f.close();
            }
        });
    }
    for (auto& th : threads) if (th.joinable()) th.join();
}
