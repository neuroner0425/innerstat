#ifndef INNERSTAT_CLIENT_NO_SASH_SPLITTER_H
#define INNERSTAT_CLIENT_NO_SASH_SPLITTER_H

#ifndef INNERSTAT_CLIENT_BASE_H
    #include "innerstat/client/base.h"
#endif

#include <wx/splitter.h>
#include <wx/colour.h>
#include <wx/dcclient.h>

INNERSTAT_BEGIN_NAMESPACE

class wxNoSashSplitterWindow : public wxSplitterWindow {
public:
    wxNoSashSplitterWindow(wxWindow* parent, long style = wxSP_LIVE_UPDATE | wxSP_THIN_SASH | wxSP_NO_XP_THEME)
        : wxSplitterWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, style)
    {
        SetDoubleBuffered(true);

        Bind(wxEVT_SPLITTER_SASH_POS_CHANGED, [this](wxSplitterEvent&) {
            Refresh();
            Update();
        });

        Bind(wxEVT_ERASE_BACKGROUND, [](wxEraseEvent&) {});
    }

protected:
    void DrawSash(wxDC& dc) override {
        wxColour bg = this->GetBackgroundColour();
        dc.SetPen(bg);
        dc.SetBrush(bg);

        int sashSize = GetSashSize();
        if (IsSplit()) {
            if (GetSplitMode() == wxSPLIT_VERTICAL) {
                int x = GetSashPosition();
                dc.DrawRectangle(x, 0, sashSize, GetClientSize().y);
            } else {
                int y = GetSashPosition();
                dc.DrawRectangle(0, y, GetClientSize().x, sashSize);
            }
        }
    }
    void DrawSashTracker(int, int) override {
        // do nothing
    }
};

INNERSTAT_END_NAMESPACE

#endif
