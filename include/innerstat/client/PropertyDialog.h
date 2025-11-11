#ifndef INNERSTAT_CLIENT_PROPERTYDIALOG_H
#define INNERSTAT_CLIENT_PROPERTYDIALOG_H

#include <wx/wx.h>
#include <wx/spinctrl.h>

// Forward declaration to avoid circular dependencies
namespace innerstat { inline namespace v1 {
    class Shape;
}}

class PropertyDialog : public wxDialog {
public:
    PropertyDialog(wxWindow* parent, innerstat::v1::Shape* shape);

private:
    void OnOk(wxCommandEvent& event);

    innerstat::v1::Shape* shape_;
    
    // Common fields
    wxTextCtrl* label_ctrl_;
    wxTextCtrl* mac_address_ctrl_;

    // OS Shape fields
    wxSpinCtrlDouble* cpu_attention_ctrl_;
    wxSpinCtrlDouble* cpu_warning_ctrl_;
    wxSpinCtrl* os_loss_timeout_ctrl_;

    // PS Shape fields
    wxSpinCtrl* port_number_ctrl_;
    wxSpinCtrl* log_attention_ctrl_;
    wxSpinCtrl* log_warning_ctrl_;
    wxSpinCtrl* ps_loss_timeout_ctrl_;
};

#endif // INNERSTAT_CLIENT_PROPERTYDIALOG_H