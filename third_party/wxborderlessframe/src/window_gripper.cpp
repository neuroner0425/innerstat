/////////////////////////////////////////////////////////////////////////////
// Name:        window_gripper.cpp
// Purpose:     wxWindowGripper factory
// Author:      Łukasz Świszcz
// Modified by:
// Created:     2022-04-17
// Copyright:   (c) Łukasz Świszcz
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include <wxbf/window_gripper.h>
#if defined(_WIN32)
#include <wxbf/window_gripper_msw.h>
#elif defined(__WXGTK__)
#include <wxbf/window_gripper_gtk.h>
#elif defined(__WXOSX__)
#include <wxbf/window_gripper_osx.h>
#endif

wxWindowGripper* wxWindowGripper::Create()
{
#if defined(_WIN32)
    return new wxWindowGripperMSW();
#elif defined(__WXGTK__)
    return new wxWindowGripperGTK();
#elif defined(__WXOSX__)
    return new wxWindowGripperOSX();
#endif

    wxLogError("Could not find implementation of wxWindowGripper for this platform");
    return nullptr;
}
