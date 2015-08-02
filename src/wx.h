#pragma once

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/config.h>

#define _NAME wxT("atem-kontroller")
#ifndef _DEBUG
#define _VERSION wxT("0.1 r1")
#else
#define _VERSION wxT("0.1 r1 [DEBUG VERSION]")
#endif
