
#ifndef INNERSTAT_CLIENT_AGENT_DIALOG_H
#define INNERSTAT_CLIENT_AGENT_DIALOG_H

#include <wx/wx.h>

class AgentComDialog : public wxDialog {
public:
    AgentComDialog(wxWindow* parent);

private:
    wxTextCtrl* lsofText;
    wxTextCtrl* psText;
    wxButton* psButton;
    wxTimer* timer;

    void OnPsButtonClick(wxCommandEvent& event);
    void OnTimer(wxTimerEvent& event);
};

#endif // INNERSTAT_CLIENT_AGENT_DIALOG_H
