/////////////////////////////////////////////////////////////////////////////
// Name:        wxbf/borderless_frame_osx.h
// Purpose:     wxBorderlessFrame implementation for macOS (Cocoa)
// Author:      Added by Copilot
// Created:     2025-09-30
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WXBF_BORDERLESS_FRAME_OSX_H_
#define _WXBF_BORDERLESS_FRAME_OSX_H_

#ifdef __WXOSX__

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "borderless_frame_base.h"
#include "borderless_frame_common.h"

/**
 * \brief A platform specific implementation of wxBorderlessFrameBase for macOS (Cocoa).
 *
 * It hides the standard title bar while keeping the window movable and allows
 * minimal system command handling.
 */
class BFDLLEXPORT wxBorderlessFrameOSX : public wxBorderlessFrameBase
{
public:
    wxBorderlessFrameOSX() { }

    wxBorderlessFrameOSX(wxWindow* parent,
        wxWindowID id,
        const wxString& title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_FRAME_STYLE,
        const wxString& name = wxASCII_STR(wxFrameNameStr))
    {
        Create(parent, id, title, pos, size, style, name);
    }

    virtual ~wxBorderlessFrameOSX() {}

    bool Create(wxWindow* parent,
        wxWindowID id,
        const wxString& title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_FRAME_STYLE,
        const wxString& name = wxASCII_STR(wxFrameNameStr));

    virtual void ExecSystemCommand(wxSystemCommand command) wxOVERRIDE;

    void SetWindowStyleFlag(long style) wxOVERRIDE;

    // Appearance knobs (no-op on Cocoa for now, but kept for API compatibility)
    void SetBorderColour(wxColour borderColour) { m_borderColour = borderColour; }
    wxColour GetBorderColour() const { return m_borderColour; }
    void SetBorderThickness(int thicknessPx) { m_borderThickness = thicknessPx; }
    int GetBorderThickness() const { return m_borderThickness; }

    wxByte GetShadowAlpha() const { return m_shadowAlpha; }
    void SetShadowAlpha(wxByte a) { m_shadowAlpha = a; }
    int GetShadowSize() const { return m_shadowSize; }
    void SetShadowSize(int s) { m_shadowSize = s; }
    wxPoint GetShadowOffset() const { return m_shadowOffset; }
    void SetShadowOffset(wxPoint off) { m_shadowOffset = off; }
    bool IsShadowDisabledOnInactiveWindow() const { return m_disableShadowOnInactive; }
    void SetShadowDisabledOnInactiveWindow(bool d) { m_disableShadowOnInactive = d; }

    void PopupSystemMenu() {} // Not applicable on macOS

    // macOS: allow placing custom controls in the titlebar area alongside native buttons
    // The caller provides a wxWindow (e.g., a wxPanel with sizer) to be hosted at the top area.
    // On Cocoa, we'll wrap it with NSVisualEffectView for a native translucent look.
    void EnableUnifiedTitlebar(bool enable = true);
    void SetTitlebarAccessory(wxWindow* accessory, int leftPaddingDip = 8, int rightPaddingDip = 8);
    void SetMovableByBackground(bool movable = true);

protected:
    virtual wxWindowPart GetWindowPart(wxPoint mousePosition) const { return wxWP_CLIENT_AREA; }

private:
    void OnMouse(wxMouseEvent& evnt);
    void InstallOrRemoveTitlebarAccessory();

    wxWindow* m_titlebarAccessory{nullptr};
    int m_titlebarLeftPad{8};
    int m_titlebarRightPad{8};
    bool m_unifiedTitlebar{true};
    bool m_movableByBackground{true};

    wxColour m_borderColour{168,168,168};
    int m_borderThickness{1};
    int m_shadowSize{10};
    wxPoint m_shadowOffset{0,2};
    wxByte m_shadowAlpha{168};
    bool m_disableShadowOnInactive{true};
};

#endif // __WXOSX__

#endif // _WXBF_BORDERLESS_FRAME_OSX_H_
