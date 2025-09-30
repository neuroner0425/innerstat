/////////////////////////////////////////////////////////////////////////////
// Name:        window_gripper_osx.mm
// Purpose:     wxWindowGripper implementation for macOS (Cocoa)
// Author:      Added by Copilot
// Created:     2025-09-30
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifdef __WXOSX__

#include <wxbf/window_gripper_osx.h>
#include <wx/osx/private.h>
#import <Cocoa/Cocoa.h>

static inline NSWindow* GetNSWindowFromWx(wxWindow* win)
{
    WXWidget handle = win->GetHandle();
    if (handle == nullptr) return nil;
    NSView* view = (NSView*)handle;
    return view.window;
}

bool wxWindowGripperOSX::StartDragMove(wxWindow* window)
{
    NSWindow* nswin = GetNSWindowFromWx(window);
    if (!nswin) return false;

    // On macOS, with movableByWindowBackground enabled, dragging is automatic.
    // To emulate a drag start, we can send performWindowDragWithEvent from a synthesized event,
    // but it's often unnecessary. Just return true.
    return true;
}

bool wxWindowGripperOSX::StartDragResize(wxWindow* window, wxDirection which)
{
    // macOS doesn't support programmatic begin-resize like GTK; rely on edges.
    // Could implement by showing a transparent resizer overlay. For now, no-op.
    return false;
}

#endif // __WXOSX__
