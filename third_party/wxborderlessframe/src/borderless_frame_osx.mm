/////////////////////////////////////////////////////////////////////////////
// Name:        borderless_frame_osx.mm
// Purpose:     wxBorderlessFrame implementation for macOS (Cocoa)
// Author:      Added by Copilot
// Created:     2025-09-30
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifdef __WXOSX__

#include <wxbf/borderless_frame_osx.h>

#include <wx/osx/private.h>

#import <Cocoa/Cocoa.h>

static inline NSWindow* GetNSWindowFromWx(wxWindow* win)
{
    WXWidget handle = win->GetHandle();
    if (handle == nullptr) return nil;
    NSView* view = (NSView*)handle;
    return view.window;
}

static inline NSView* GetNSViewFromWx(wxWindow* win)
{
    WXWidget handle = win->GetHandle();
    if (handle == nullptr) return nil;
    return (NSView*)handle;
}

bool wxBorderlessFrameOSX::Create(wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name)
{
    if (!wxFrame::Create(parent, id, title, pos, size, style, name)) {
        return false;
    }

    NSWindow* nswin = GetNSWindowFromWx(this);
    if (nswin) {
        // Use unified toolbar/titlebar look while keeping standard traffic lights
        nswin.titleVisibility = NSWindowTitleHidden;
        nswin.titlebarAppearsTransparent = YES;
        nswin.styleMask |= NSWindowStyleMaskFullSizeContentView;
        nswin.movableByWindowBackground = YES;
        [[nswin standardWindowButton:NSWindowCloseButton] setHidden:NO];
        [[nswin standardWindowButton:NSWindowMiniaturizeButton] setHidden:NO];
        [[nswin standardWindowButton:NSWindowZoomButton] setHidden:NO];

        // Ensure updates
        [nswin layoutIfNeeded];
        [nswin display];
    }

    Bind(wxEVT_LEFT_DOWN, &wxBorderlessFrameOSX::OnMouse, this);
    Bind(wxEVT_LEFT_UP, &wxBorderlessFrameOSX::OnMouse, this);
    Bind(wxEVT_LEFT_DCLICK, &wxBorderlessFrameOSX::OnMouse, this);
    Bind(wxEVT_RIGHT_DOWN, &wxBorderlessFrameOSX::OnMouse, this);
    Bind(wxEVT_RIGHT_UP, &wxBorderlessFrameOSX::OnMouse, this);

    return true;
}

void wxBorderlessFrameOSX::ExecSystemCommand(wxSystemCommand command)
{
    NSWindow* nswin = GetNSWindowFromWx(this);
    if (!nswin) return;

    switch (command) {
    case wxSC_MINIMIZE:
        [nswin miniaturize:nil];
        break;
    case wxSC_MAXIMIZE:
        if (![nswin isZoomed]) [nswin zoom:nil];
        break;
    case wxSC_RESTORE:
        if ([nswin isMiniaturized]) [nswin deminiaturize:nil];
        if ([nswin isZoomed]) [nswin zoom:nil];
        break;
    case wxSC_CLOSE:
        [nswin performClose:nil];
        break;
    }
}

void wxBorderlessFrameOSX::EnableUnifiedTitlebar(bool enable)
{
    m_unifiedTitlebar = enable;
    NSWindow* nswin = GetNSWindowFromWx(this);
    if (!nswin) return;
    nswin.titlebarAppearsTransparent = enable ? YES : NO;
    if (enable) {
        nswin.styleMask |= NSWindowStyleMaskFullSizeContentView;
        nswin.titleVisibility = NSWindowTitleHidden;
    } else {
        nswin.styleMask &= ~NSWindowStyleMaskFullSizeContentView;
        nswin.titleVisibility = NSWindowTitleVisible;
    }
    [nswin layoutIfNeeded];
    [nswin display];
}

void wxBorderlessFrameOSX::SetMovableByBackground(bool movable)
{
    m_movableByBackground = movable;
    NSWindow* nswin = GetNSWindowFromWx(this);
    if (nswin) {
        nswin.movableByWindowBackground = movable ? YES : NO;
    }
}

void wxBorderlessFrameOSX::SetTitlebarAccessory(wxWindow* accessory, int leftPaddingDip, int rightPaddingDip)
{
    m_titlebarAccessory = accessory;
    m_titlebarLeftPad = leftPaddingDip;
    m_titlebarRightPad = rightPaddingDip;
    InstallOrRemoveTitlebarAccessory();
}

void wxBorderlessFrameOSX::InstallOrRemoveTitlebarAccessory()
{
    NSWindow* nswin = GetNSWindowFromWx(this);
    if (!nswin) return;

    if (!m_titlebarAccessory) {
        // Nothing to install; just ensure layout is refreshed
        [nswin layoutIfNeeded];
        [nswin display];
        return;
    }

    // Prepare NSVisualEffectView for native translucent titlebar look
    NSVisualEffectView* vev = [[NSVisualEffectView alloc] init];
    vev.material = NSVisualEffectMaterialTitlebar;
    vev.blendingMode = NSVisualEffectBlendingModeBehindWindow;
    vev.state = NSVisualEffectStateActive;

    // Compute frame: place accessory to the right of traffic lights with padding
    const CGFloat leftPad = (CGFloat)FromDIP(m_titlebarLeftPad);
    const CGFloat rightPad = (CGFloat)FromDIP(m_titlebarRightPad);
    NSView* contentView = nswin.contentView;
    if (!contentView) return;

    // Position within titlebar height =  titlebar area approximated by toolbar height
    CGFloat titlebarHeight = 28.0; // reasonable default; Cocoa manages actual height
    NSRect frame = NSMakeRect(0, contentView.bounds.size.height - titlebarHeight,
                              contentView.bounds.size.width, titlebarHeight);
    vev.frame = frame;

    // Place accessory view with padding and natural fitting size
    wxSize wxBest = m_titlebarAccessory->GetBestSize();
    // Compute rightmost edge of traffic lights cluster
    NSButton* closeBtn = [nswin standardWindowButton:NSWindowCloseButton];
    NSButton* miniBtn = [nswin standardWindowButton:NSWindowMiniaturizeButton];
    NSButton* zoomBtn = [nswin standardWindowButton:NSWindowZoomButton];
    CGFloat clusterMaxX = MAX(NSMaxX(closeBtn.frame), MAX(NSMaxX(miniBtn.frame), NSMaxX(zoomBtn.frame)));
    CGFloat ax = clusterMaxX + leftPad;
    int iw = (int)wxBest.x;
    int ih = (int)titlebarHeight;
    if (iw <= 0) iw = 220; // fallback width
    m_titlebarAccessory->SetSize(ax, 0, iw, ih);

    // Add visual effect view beneath accessory so it stays visible
    NSView* accessoryView = GetNSViewFromWx(m_titlebarAccessory);
    [contentView addSubview:vev positioned:NSWindowBelow relativeTo:accessoryView];

    [nswin layoutIfNeeded];
    [nswin display];
}

void wxBorderlessFrameOSX::SetWindowStyleFlag(long style)
{
    long oldStyle = GetWindowStyleFlag();
    wxFrame::SetWindowStyleFlag(style);
    static const long MASK = wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxMAXIMIZE | wxCLOSE_BOX;
    if ((oldStyle & MASK) != (style & MASK)) {
        wxPostEvent(this, wxCommandEvent(wxEVT_UPDATE_SYSTEM_BUTTONS));
    }
}

void wxBorderlessFrameOSX::OnMouse(wxMouseEvent& evnt)
{
    if (evnt.GetEventType() == wxEVT_LEFT_DOWN) {
        wxWindowPart part = GetWindowPart(ClientToScreen(evnt.GetPosition()));
        if (part != wxWP_CLIENT_AREA) {
            evnt.SetEventType(wxEVT_NC_LEFT_DOWN);
            wxPostEvent(this, evnt);
            return;
        }
    }

    if (evnt.GetEventType() == wxEVT_LEFT_UP) {
        wxWindowPart part = GetWindowPart(ClientToScreen(evnt.GetPosition()));
        if (part != wxWP_CLIENT_AREA) {
            evnt.SetEventType(wxEVT_NC_LEFT_UP);
            wxPostEvent(this, evnt);
            return;
        }
    }

    if (evnt.GetEventType() == wxEVT_LEFT_DCLICK) {
        if (GetWindowPart(ClientToScreen(evnt.GetPosition())) == wxWP_TITLEBAR) {
            ExecSystemCommand(IsMaximized() ? wxSC_RESTORE : wxSC_MAXIMIZE);
            return;
        }
    }

    if (evnt.GetEventType() == wxEVT_RIGHT_DOWN) {
        if (GetWindowPart(ClientToScreen(evnt.GetPosition())) != wxWP_CLIENT_AREA) {
            evnt.SetEventType(wxEVT_NC_RIGHT_DOWN);
            wxPostEvent(this, evnt);
            return;
        }
    }

    if (evnt.GetEventType() == wxEVT_RIGHT_UP) {
        if (GetWindowPart(ClientToScreen(evnt.GetPosition())) != wxWP_CLIENT_AREA) {
            evnt.SetEventType(wxEVT_NC_RIGHT_UP);
            wxPostEvent(this, evnt);
            return;
        }
    }

    evnt.Skip();
}

#endif // __WXOSX__
