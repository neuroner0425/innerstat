/////////////////////////////////////////////////////////////////////////////
// Name:        wxbf/window_gripper_osx.h
// Purpose:     wxWindowGripper implementation for macOS (Cocoa)
// Author:      Added by Copilot
// Created:     2025-09-30
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WXBF_WINDOW_GRIPPER_OSX_H_
#define _WXBF_WINDOW_GRIPPER_OSX_H_

#ifdef __WXOSX__

#include "borderless_frame_common.h"
#include "window_gripper.h"

class BFDLLEXPORT wxWindowGripperOSX : public wxWindowGripper
{
public:
    virtual bool StartDragMove(wxWindow* window) wxOVERRIDE;
    virtual bool StartDragResize(wxWindow* window, wxDirection which) wxOVERRIDE;
};

#endif // __WXOSX__

#endif // _WXBF_WINDOW_GRIPPER_OSX_H_
