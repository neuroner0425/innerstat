/////////////////////////////////////////////////////////////////////////////
// Name:        system_buttons.cpp
// Purpose:     wxSystemButtonsFactory
// Author:      Łukasz Świszcz
// Modified by:
// Created:     2023-03-27
// Copyright:   (c) Łukasz Świszcz
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "system_menu_png.h"

#include <wxbf/system_buttons.h>
#include <wxbf/system_buttons_fallback.h>
#include <wxbf/system_buttons_win10.h>
#ifdef __WXOSX__
#include <wxbf/system_buttons_mac.h>
#endif

#include <wx/fontenum.h>

wxSystemButtonsBase* wxSystemButtonsFactory::CreateSystemButtons(wxBorderlessFrameBase* frame)
{
#if defined(__WXMSW__)
    // Prefer Windows 10+ vector icon set when available
    if (wxFontEnumerator::IsValidFacename(wxWin10SystemButtons::ICON_FAMILY_NAME)) {
        return new wxWin10SystemButtons(frame);
    }
    // Fall back to PNG atlas on older Windows
    return new wxFallbackSystemButtons(frame, SYSTEM_MENU_DATA, SYSTEM_MENU_SIZE);
#elif defined(__WXOSX__)
    // On macOS always use traffic-light style, left-aligned
    return new wxMacSystemButtons(frame);
#else
    // Other platforms: use generic fallback
    return new wxFallbackSystemButtons(frame, SYSTEM_MENU_DATA, SYSTEM_MENU_SIZE);
#endif
}
