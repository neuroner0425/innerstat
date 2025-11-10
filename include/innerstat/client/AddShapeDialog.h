#ifndef INNERSTAT_CLIENT_ADDSHAPEDIALOG_H
#define INNERSTAT_CLIENT_ADDSHAPEDIALOG_H

#include <wx/wx.h>
#include <wx/checklst.h>
#include <string>
#include <vector>
#include "innerstat/common/system_info.h"

class AddShapeDialog : public wxDialog {
public:
    AddShapeDialog(wxWindow* parent, const systemInfo& agent_info);

    std::string GetShapeLabel() const;
    std::vector<LsofItem> GetSelectedProcesses() const;

private:
    wxTextCtrl* label_ctrl_;
    wxCheckListBox* process_list_box_;
    const std::vector<LsofItem>& all_processes_;
};

#endif // INNERSTAT_CLIENT_ADDSHAPEDIALOG_H
