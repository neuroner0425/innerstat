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

// Compute the current titlebar region (y, height) within the window's contentView coordinates.
static inline void GetTitlebarRegion(NSWindow* nswin, CGFloat& outY, CGFloat& outH)
{
    outY = 0.0;
    outH = 35.0; // fallback
    if (!nswin) return;
    NSView* contentView = nswin.contentView;
    if (!contentView) return;
    // contentLayoutRect excludes the titlebar/toolbar area; the difference to bounds is the titlebar height
    NSRect layoutRect = nswin.contentLayoutRect; // available content area below titlebar
    CGFloat boundsH = contentView.bounds.size.height;
    CGFloat layoutH = layoutRect.size.height;
    CGFloat titlebarH = boundsH - layoutH;
    if (titlebarH > 0.0 && titlebarH < boundsH) {
        outH = titlebarH;
        outY = layoutH; // titlebar starts just above the layout rect
    }
}

// A visual effect view that allows dragging the window by clicking anywhere on it
@interface DraggableVisualEffectView : NSVisualEffectView
@end

@implementation DraggableVisualEffectView
- (BOOL)mouseDownCanMoveWindow { return YES; }
@end

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
    Bind(wxEVT_SIZE, &wxBorderlessFrameOSX::OnSizeEvt, this);

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
        // 보조: 전체 타이틀바(FullSizeContentView) 배경에서도 드래그되도록 보장
        if (movable) {
            nswin.styleMask |= NSWindowStyleMaskFullSizeContentView;
            nswin.titlebarAppearsTransparent = YES;
        }
    }
}

void wxBorderlessFrameOSX::BeginNativeDrag()
{
    NSWindow* nswin = GetNSWindowFromWx(this);
    if (!nswin) return;
    NSEvent* event = [NSApp currentEvent];
    if (event) {
        [nswin performWindowDragWithEvent:event];
    }
}

void wxBorderlessFrameOSX::SetTitlebarAccessory(wxWindow* accessory, int leftPaddingDip, int rightPaddingDip)
{
    // If replacing an accessory, unbind previous handler to avoid duplicates
    if (m_titlebarAccessory) {
        m_titlebarAccessory->Unbind(wxEVT_LEFT_DOWN, &wxBorderlessFrameOSX::OnAccessoryLeftDown, this);
    }
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
    DraggableVisualEffectView* vev = [[DraggableVisualEffectView alloc] init];
    vev.material = NSVisualEffectMaterialTitlebar;
    vev.blendingMode = NSVisualEffectBlendingModeBehindWindow;
    vev.state = NSVisualEffectStateActive;

    // Compute frame: place accessory to the right of traffic lights with padding
    const CGFloat leftPad = (CGFloat)FromDIP(m_titlebarLeftPad);
    const CGFloat rightPad = (CGFloat)FromDIP(m_titlebarRightPad);
    NSView* contentView = nswin.contentView;
    if (!contentView) return;

    // Use actual titlebar region computed from contentLayoutRect
    CGFloat titlebarY = 0.0, titlebarHeight = 35.0;
    GetTitlebarRegion(nswin, titlebarY, titlebarHeight);
    NSRect frame = NSMakeRect(0, titlebarY, contentView.bounds.size.width, titlebarHeight);
    vev.frame = frame;
    // Ensure it resizes with the window width and stays at the top
    vev.autoresizingMask = NSViewWidthSizable | NSViewMinYMargin;

    // Make accessory fill the entire titlebar width, including behind traffic lights
    // Interactive controls should avoid overlapping traffic lights via their own internal padding/sizers
    int iw = (int)std::max<CGFloat>(contentView.bounds.size.width - rightPad, 0);
    int ih = (int)titlebarHeight;
    if (iw <= 0) iw = 220; // fallback width
    m_titlebarAccessory->SetSize(0, 0, iw, ih);
    // Ensure empty background of accessory initiates native drag
    m_titlebarAccessory->Bind(wxEVT_LEFT_DOWN, &wxBorderlessFrameOSX::OnAccessoryLeftDown, this);

    // Add visual effect view beneath accessory so it stays visible
    NSView* accessoryView = GetNSViewFromWx(m_titlebarAccessory);
    [contentView addSubview:vev positioned:NSWindowBelow relativeTo:accessoryView];
    // Store raw pointer; NSView hierarchy retains subviews, ARC is disabled in this file
    m_titlebarVev = (void*)vev;

    [nswin layoutIfNeeded];
    [nswin display];
}

void wxBorderlessFrameOSX::UpdateTitlebarLayout()
{
    NSWindow* nswin = GetNSWindowFromWx(this);
    if (!nswin) return;
    NSView* contentView = nswin.contentView;
    if (!contentView) return;

    // Determine current titlebar region precisely
    CGFloat titlebarY = 0.0, titlebarHeight = 35.0;
    GetTitlebarRegion(nswin, titlebarY, titlebarHeight);
    NSRect frame = NSMakeRect(0, titlebarY, contentView.bounds.size.width, titlebarHeight);

    if (m_titlebarVev) {
        DraggableVisualEffectView* vev = (DraggableVisualEffectView*)m_titlebarVev;
        vev.frame = frame;
    }

    if (m_titlebarAccessory) {
        int iw = (int)std::max<CGFloat>(contentView.bounds.size.width - (CGFloat)FromDIP(m_titlebarRightPad), 0);
        int ih = (int)titlebarHeight;
        m_titlebarAccessory->SetSize(0, 0, iw, ih);
    }

    [nswin layoutIfNeeded];
    [nswin display];
}

void wxBorderlessFrameOSX::OnAccessoryLeftDown(wxMouseEvent& evnt)
{
    // Only start drag if click is on accessory's background (not on child controls)
    // In wxWidgets, child controls typically handle the event first; this handler
    // effectively covers the empty areas of the accessory panel.
    BeginNativeDrag();
    // Do not propagate further to avoid conflicting behaviors while dragging
}

void wxBorderlessFrameOSX::OnSizeEvt(wxSizeEvent& evnt)
{
    UpdateTitlebarLayout();
    evnt.Skip();
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
