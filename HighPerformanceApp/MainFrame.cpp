#include "MainFrame.h"
#include "CpuLoadPanel.h"
#include "MemoryLoadPanel.h"
#include "DiskLoadPanel.h"

#include <wx/sizer.h>

MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(800, 420))
{
    wxPanel* mainPanel = new wxPanel(this);
    wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);

    m_cpuPanel = new CpuLoadPanel(mainPanel);
    m_memPanel = new MemoryLoadPanel(mainPanel);
    m_diskPanel = new DiskLoadPanel(mainPanel);

    hbox->Add(m_cpuPanel, 1, wxALL | wxEXPAND, 8);
    hbox->Add(m_memPanel, 1, wxALL | wxEXPAND, 8);
    hbox->Add(m_diskPanel, 1, wxALL | wxEXPAND, 8);

    mainPanel->SetSizer(hbox);
    this->Centre();
}
MainFrame::~MainFrame() = default;
