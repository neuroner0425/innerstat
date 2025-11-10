#ifndef INNERSTAT_CLIENT_AGENTSELECTIONDIALOG_H
#define INNERSTAT_CLIENT_AGENTSELECTIONDIALOG_H

#include "innerstat/client/client.h"
#include <wx/wx.h>
#include <wx/listbox.h>
#include <vector>
#include <string>
#include <map>
#include "innerstat/common/system_info.h"

INNERSTAT_BEGIN_NAMESPACE

class MainCanvas; // Forward declaration

class AgentSelectionDialog : public wxDialog {
public:
    AgentSelectionDialog(wxWindow* parent, MainCanvas* canvas);
    ~AgentSelectionDialog();
    void UpdateAgentList();

private:
    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);

    MainCanvas* canvas_;
    wxListBox* agent_list_box_;
    std::vector<std::string> mac_addresses_;
};

INNERSTAT_END_NAMESPACE

#endif // INNERSTAT_CLIENT_AGENTSELECTIONDIALOG_H
