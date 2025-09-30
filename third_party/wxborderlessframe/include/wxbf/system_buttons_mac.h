/////////////////////////////////////////////////////////////////////////////
// Name:        wxbf/system_buttons_mac.h
// Purpose:     macOS-style system buttons (traffic lights)
// Author:      Added by Copilot
// Created:     2025-09-30
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WXBF_SYSTEM_BUTTONS_MAC_H_
#define _WXBF_SYSTEM_BUTTONS_MAC_H_

#ifdef __WXOSX__

#include "system_buttons_base.h"

class BFDLLEXPORT wxMacSystemButtons : public wxSystemButtonsBase {
public:
    explicit wxMacSystemButtons(wxBorderlessFrameBase* frame)
        : wxSystemButtonsBase(frame) {
        wxLogDebug("wxMacSystemButtons: using mac traffic-light buttons (left-aligned)");
    }

    bool AreButtonsRightAligned() const wxOVERRIDE { return false; }
    wxSize GetPreferredButtonSize() const wxOVERRIDE { return wxSize(16, 16); }

protected:
    wxSize MeasureButton(wxSystemButton which, wxCoord& margin) const wxOVERRIDE;
    void DrawButton(wxDC& dc, wxSystemButton which,
        wxSystemButtonState state, const wxRect& rect) wxOVERRIDE;

private:
    wxColour ColorFor(wxSystemButton which, wxSystemButtonState state) const;
    void DrawGlyph(wxDC& dc, wxSystemButton which, const wxRect& rect, const wxColour& fg) const;
};

#endif // __WXOSX__

#endif // _WXBF_SYSTEM_BUTTONS_MAC_H_
