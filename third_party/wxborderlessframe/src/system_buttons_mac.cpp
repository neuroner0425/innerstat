/////////////////////////////////////////////////////////////////////////////
// Name:        system_buttons_mac.cpp
// Purpose:     macOS-style system buttons (traffic lights)
// Author:      Added by Copilot
// Created:     2025-09-30
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifdef __WXOSX__

#include <wxbf/system_buttons_mac.h>
#include <wx/dcmemory.h>

wxSize wxMacSystemButtons::MeasureButton(wxSystemButton which, wxCoord& margin) const
{
    margin = 6; // typical gap between traffic lights
    return GetPreferredButtonSize();
}

wxColour wxMacSystemButtons::ColorFor(wxSystemButton which, wxSystemButtonState state) const
{
    // Base colors
    wxColour c = *wxBLACK;
    switch (which) {
        case wxSB_CLOSE: c = wxColour(255, 95, 86); break;   // red
        case wxSB_MINIMIZE: c = wxColour(255, 189, 46); break; // yellow
        case wxSB_MAXIMIZE:
        case wxSB_RESTORE: c = wxColour(39, 201, 63); break; // green
    }
    // Hover/pressed variations
    if (state == wxSB_STATE_HOVER) {
        c = wxColour(std::min(255, c.Red() + 12), std::min(255, c.Green() + 12), std::min(255, c.Blue() + 12));
    } else if (state == wxSB_STATE_PRESSED) {
        c = wxColour(std::max(0, c.Red() - 28), std::max(0, c.Green() - 28), std::max(0, c.Blue() - 28));
    } else if (state == wxSB_STATE_INACTIVE) {
        c = wxColour((c.Red()*2)/3, (c.Green()*2)/3, (c.Blue()*2)/3);
    }
    return c;
}

void wxMacSystemButtons::DrawGlyph(wxDC& dc, wxSystemButton which, const wxRect& rect, const wxColour& fg) const
{
    // Draw mac glyph: close = x, minimize = -, zoom = +
    dc.SetPen(wxPen(fg, 1));
    const wxRect r = rect.Deflate(rect.width/3, rect.height/3);
    switch (which) {
        case wxSB_CLOSE:
            dc.DrawLine(r.GetTopLeft(), r.GetBottomRight());
            dc.DrawLine(wxPoint(r.GetRight(), r.GetTop()), wxPoint(r.GetLeft(), r.GetBottom()));
            break;
        case wxSB_MINIMIZE:
            {
                const int cx = r.GetLeft() + r.GetWidth() / 2;
                const int cy = r.GetTop() + r.GetHeight() / 2;
                (void)cx; // suppress unused in this case
                dc.DrawLine(wxPoint(r.GetLeft(), cy), wxPoint(r.GetRight(), cy));
            }
            break;
        case wxSB_MAXIMIZE:
        case wxSB_RESTORE:
            {
                const int cx = r.GetLeft() + r.GetWidth() / 2;
                const int cy = r.GetTop() + r.GetHeight() / 2;
                dc.DrawLine(wxPoint(r.GetLeft(), cy), wxPoint(r.GetRight(), cy));
                dc.DrawLine(wxPoint(cx, r.GetTop()), wxPoint(cx, r.GetBottom()));
            }
            break;
    }
}

void wxMacSystemButtons::DrawButton(wxDC& dc, wxSystemButton which,
    wxSystemButtonState state, const wxRect& rect)
{
    const wxColour bg = ColorFor(which, state);
    // Outer circle with slight shadow
    dc.SetPen(wxPen(wxColour(0,0,0,40), 1));
    dc.SetBrush(wxBrush(bg));
    const int diameter = std::min(rect.width, rect.height);
    const wxRect circleRect = wxRect(0,0,diameter,diameter).CenterIn(rect);
    dc.DrawEllipse(circleRect);

    // Glyph only on hover/pressed like macOS style
    if (state == wxSB_STATE_HOVER || state == wxSB_STATE_PRESSED) {
        DrawGlyph(dc, which, circleRect, wxColour(60, 60, 60));
    }
}

#endif // __WXOSX__
