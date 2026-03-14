// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#if defined(_MSC_VER) && _MSC_VER > 1000
#pragma once
#endif

// Platform detection and abstraction
#include "PlatformDefs.h"

#ifdef USE_WIN32
  #define WIN32_LEAN_AND_MEAN
  #define _WIN32_WINDOWS 0x0500
#endif

// Windows Header Files (Windows only)
#ifdef USE_WIN32
#include <windows.h>
#include <winuser.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <wingdi.h>
#endif

// Standard C/C++ Headers (cross-platform)
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cmath>
#include <cctype>
#include <ctime>
#include <cwchar>
#include <cwctype>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <limits>
#include <cassert>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <fcntl.h>

#ifdef USE_WIN32
#include <io.h>
#include <tchar.h>
#endif

// Audio file support (provided by build system)
#include <sndfile.h>

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
