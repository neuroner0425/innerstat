#pragma once
#include <wx/frame.h>

class CpuLoadPanel;
class MemoryLoadPanel;
class DiskLoadPanel;

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title);
    ~MainFrame();

private:
    CpuLoadPanel* m_cpuPanel;
    MemoryLoadPanel* m_memPanel;
    DiskLoadPanel* m_diskPanel;
};
