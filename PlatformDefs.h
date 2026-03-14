#ifndef PLATFORM_DEFS_H
#define PLATFORM_DEFS_H

// ============================================================================
// PlatformDefs.h - Central platform abstraction for Chaotic-DAW
//
// Provides cross-platform replacements for Windows-specific APIs:
//   - Mutex (HANDLE + CreateMutex/WaitForSingleObject/ReleaseMutex)
//   - Dynamic library loading (HMODULE + LoadLibrary/FreeLibrary/GetProcAddress)
//   - CRITICAL_SECTION
//   - POSIX-compatible string/path functions
// ============================================================================

// --- Platform detection (mirrors StdAfx.h / Awful.h) ---
#if defined(_WIN32) || defined(_WIN64) || defined(_WIN32_WINNT)
  #ifndef USE_WIN32
    #define USE_WIN32 1
  #endif
#else
  #ifdef LINUX
    #ifndef USE_LINUX
      #define USE_LINUX 1
    #endif
  #endif
#endif

// ============================================================================
// WINDOWS
// ============================================================================
#ifdef USE_WIN32

#include <windows.h>

// Mutex types - just use native Windows API directly
typedef HANDLE              PlatformMutex;
#define PlatformMutex_Create()          CreateMutex(NULL, FALSE, NULL)
#define PlatformMutex_Lock(h)           WaitForSingleObject(h, INFINITE)
#define PlatformMutex_Unlock(h)         ReleaseMutex(h)
#define PlatformMutex_Destroy(h)        CloseHandle(h)

// Critical section
typedef CRITICAL_SECTION    PlatformCriticalSection;
#define PlatformCS_Init(cs)             InitializeCriticalSection(cs)
#define PlatformCS_Delete(cs)           DeleteCriticalSection(cs)
#define PlatformCS_Enter(cs)            EnterCriticalSection(cs)
#define PlatformCS_Leave(cs)            LeaveCriticalSection(cs)

// Dynamic library loading
typedef HMODULE             PlatformDynLib;
#define PlatformDynLib_Load(path)       LoadLibrary(path)
#define PlatformDynLib_Free(h)          FreeLibrary(h)
#define PlatformDynLib_GetProc(h, name) GetProcAddress(h, name)

// Path/string compat - Windows uses underscore-prefixed POSIX names
// (these are already available on Windows)

// ============================================================================
// LINUX / POSIX
// ============================================================================
#elif defined(USE_LINUX) || defined(LINUX)

#include <pthread.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <strings.h>
#include <string.h>
#include <cstdio>
#include <cmath>
#include <limits.h>

// Mutex - wraps pthread_mutex_t behind a pointer-like interface
// We use a struct wrapper so PlatformMutex can be a value type like HANDLE
struct PlatformMutexImpl {
    pthread_mutex_t mtx;
};

typedef PlatformMutexImpl*  PlatformMutex;

static inline PlatformMutex PlatformMutex_Create_Func() {
    PlatformMutexImpl* m = new PlatformMutexImpl;
    pthread_mutex_init(&m->mtx, NULL);
    return m;
}
static inline void PlatformMutex_Lock_Func(PlatformMutex h) {
    if (h) pthread_mutex_lock(&h->mtx);
}
static inline void PlatformMutex_Unlock_Func(PlatformMutex h) {
    if (h) pthread_mutex_unlock(&h->mtx);
}
static inline void PlatformMutex_Destroy_Func(PlatformMutex h) {
    if (h) { pthread_mutex_destroy(&h->mtx); delete h; }
}

#define PlatformMutex_Create()          PlatformMutex_Create_Func()
#define PlatformMutex_Lock(h)           PlatformMutex_Lock_Func(h)
#define PlatformMutex_Unlock(h)         PlatformMutex_Unlock_Func(h)
#define PlatformMutex_Destroy(h)        PlatformMutex_Destroy_Func(h)

// Critical section - mapped to pthread_mutex_t
typedef pthread_mutex_t     PlatformCriticalSection;

static inline void PlatformCS_Init_Func(PlatformCriticalSection* cs) {
    pthread_mutex_init(cs, NULL);
}
static inline void PlatformCS_Delete_Func(PlatformCriticalSection* cs) {
    pthread_mutex_destroy(cs);
}
static inline void PlatformCS_Enter_Func(PlatformCriticalSection* cs) {
    pthread_mutex_lock(cs);
}
static inline void PlatformCS_Leave_Func(PlatformCriticalSection* cs) {
    pthread_mutex_unlock(cs);
}

#define PlatformCS_Init(cs)             PlatformCS_Init_Func(cs)
#define PlatformCS_Delete(cs)           PlatformCS_Delete_Func(cs)
#define PlatformCS_Enter(cs)            PlatformCS_Enter_Func(cs)
#define PlatformCS_Leave(cs)            PlatformCS_Leave_Func(cs)

// Dynamic library loading
typedef void*               PlatformDynLib;
#define PlatformDynLib_Load(path)       dlopen(path, RTLD_NOW | RTLD_LOCAL)
#define PlatformDynLib_Free(h)          dlclose(h)
#define PlatformDynLib_GetProc(h, name) dlsym(h, name)

// Windows HANDLE equivalent for non-mutex uses (generic)
typedef void*               HANDLE;
#define INVALID_HANDLE_VALUE ((HANDLE)(long)-1)

// Windows compat: POSIX equivalents for underscore-prefixed functions
#define _stricmp            strcasecmp
#define _strnicmp           strncasecmp
#define _getcwd             getcwd
#define _chdir              chdir
#define _mkdir(path)        mkdir(path, 0755)

// Windows message compat stubs
#ifndef WM_USER
#define WM_USER 0x0400
#endif

// DWORD / BOOL compat
#ifndef DWORD
typedef unsigned long       DWORD;
#endif
#ifndef BOOL
typedef int                 BOOL;
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef INFINITE
#define INFINITE 0xFFFFFFFF
#endif
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#ifndef CALLBACK
#define CALLBACK
#endif

// HWND / HCURSOR stubs
typedef void*               HWND;
typedef void*               HCURSOR;

// HINSTANCE / HMODULE stubs
typedef void*               HINSTANCE;
typedef void*               HMODULE;

// LRESULT / WPARAM / LPARAM / UINT stubs
typedef long                LRESULT;
typedef unsigned long       WPARAM;
typedef long                LPARAM;
typedef unsigned int        UINT;
typedef void*               LPVOID;

// _isnan → isnan (MSVC compat)
#define _isnan              isnan

// Window message stubs
#ifndef SW_SHOWNORMAL
#define SW_SHOWNORMAL 1
#endif

// Stub out Windows window functions used in VSTCollection.h CVSTEffWnd
static inline void ShowWindow(HWND, int) {}
static inline void UpdateWindow(HWND) {}
static inline void SetFocus(HWND) {}
static inline void DestroyWindow(HWND) {}

// Windows Virtual Key codes - use actual Windows numeric values
// (these are used as array indices in keymap[], so numeric values must match)
#ifndef VK_BACK
#define VK_BACK     0x08
#define VK_TAB      0x09
#define VK_CLEAR    0x0C
#define VK_RETURN   0x0D
#define VK_SHIFT    0x10
#define VK_CONTROL  0x11
#define VK_MENU     0x12
#define VK_PAUSE    0x13
#define VK_CAPITAL  0x14
#define VK_ESCAPE   0x1B
#define VK_SPACE    0x20
#define VK_PRIOR    0x21
#define VK_NEXT     0x22
#define VK_END      0x23
#define VK_HOME     0x24
#define VK_LEFT     0x25
#define VK_UP       0x26
#define VK_RIGHT    0x27
#define VK_DOWN     0x28
#define VK_INSERT   0x2D
#define VK_DELETE   0x2E
#define VK_NUMPAD0  0x60
#define VK_NUMPAD1  0x61
#define VK_NUMPAD2  0x62
#define VK_NUMPAD3  0x63
#define VK_NUMPAD4  0x64
#define VK_NUMPAD5  0x65
#define VK_NUMPAD6  0x66
#define VK_NUMPAD7  0x67
#define VK_NUMPAD8  0x68
#define VK_NUMPAD9  0x69
#define VK_MULTIPLY 0x6A
#define VK_ADD      0x6B
#define VK_SEPARATOR 0x6C
#define VK_SUBTRACT 0x6D
#define VK_DECIMAL  0x6E
#define VK_DIVIDE   0x6F
#define VK_F1       0x70
#define VK_F2       0x71
#define VK_F3       0x72
#define VK_F4       0x73
#define VK_F5       0x74
#define VK_F6       0x75
#define VK_F7       0x76
#define VK_F8       0x77
#define VK_F9       0x78
#define VK_F10      0x79
#define VK_F11      0x7A
#define VK_F12      0x7B
#define VK_F13      0x7C
#define VK_F14      0x7D
#define VK_F15      0x7E
#define VK_NUMLOCK  0x90
#define VK_SCROLL   0x91
#endif

// sprintf_s → snprintf (MSVC compat)
#define sprintf_s   snprintf

// Windows cursor/path stubs
#define IDC_SIZENS  0
#define IDC_SIZEWE  0
#define IDC_ARROW   0
static inline void* LoadCursor(void*, int) { return NULL; }
static inline void SetCursor(void*) {}

// Windows path API stubs
static inline unsigned long GetModuleFileName(void*, char* buf, unsigned long sz) {
    if (buf && sz > 0) buf[0] = '\0';
    return 0;
}

static inline unsigned long GetCurrentDirectory(unsigned long sz, char* buf) {
    if (sz == 0 && buf == NULL) {
        // Windows pattern: query required buffer size
        char tmp[PATH_MAX];
        if (getcwd(tmp, sizeof(tmp))) return strlen(tmp);
        return 0;
    }
    if (buf && getcwd(buf, sz)) return strlen(buf);
    return 0;
}

static inline int SetCurrentDirectory(const char* path) {
    return chdir(path) == 0;
}

// HDC / HGLRC stubs for OpenGL (unused on Linux for now)
typedef void*               HDC;
typedef void*               HGLRC;

// Windows Registry stubs (registry not available on Linux)
typedef void*               HKEY;
typedef unsigned char*       LPBYTE;
#define HKEY_LOCAL_MACHINE  ((HKEY)(unsigned long)0x80000002)
#define REG_SZ              1
#define ERROR_SUCCESS       0
static inline long RegOpenKey(HKEY, const char*, HKEY*) { return -1; }
static inline long RegQueryValueEx(HKEY, const char*, void*, unsigned long*, unsigned char*, unsigned long*) { return -1; }
static inline long RegCloseKey(HKEY) { return 0; }

// Windows Find File → POSIX opendir/readdir implementation
#include <dirent.h>
#include <fnmatch.h>

typedef struct _WIN32_FIND_DATA {
    unsigned long dwFileAttributes;
    unsigned long nFileSizeHigh;
    unsigned long nFileSizeLow;
    char cFileName[260];
} WIN32_FIND_DATA;
#define FILE_ATTRIBUTE_DIRECTORY 0x10
#define FILE_ATTRIBUTE_HIDDEN   0x02
#define INVALID_FILE_ATTRIBUTES ((unsigned long)-1)

struct FindFileHandle {
    DIR* dir;
    char dirpath[PATH_MAX];
    char pattern[260];
};

static inline int _FindFileMatch(FindFileHandle* fh, WIN32_FIND_DATA* fd) {
    struct dirent* entry;
    while ((entry = readdir(fh->dir)) != NULL) {
        if (fnmatch(fh->pattern, entry->d_name, 0) == 0) {
            strncpy(fd->cFileName, entry->d_name, 259);
            fd->cFileName[259] = '\0';
            fd->dwFileAttributes = 0;
            fd->nFileSizeHigh = 0;
            fd->nFileSizeLow = 0;
            // Check if directory
            char fullpath[PATH_MAX];
            snprintf(fullpath, sizeof(fullpath), "%s%s", fh->dirpath, entry->d_name);
            struct stat st;
            if (stat(fullpath, &st) == 0) {
                if (S_ISDIR(st.st_mode)) fd->dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
                fd->nFileSizeLow = (unsigned long)(st.st_size & 0xFFFFFFFF);
                fd->nFileSizeHigh = (unsigned long)(st.st_size >> 32);
            }
            // Hidden files on Linux start with '.'
            if (entry->d_name[0] == '.' && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
                fd->dwFileAttributes |= FILE_ATTRIBUTE_HIDDEN;
            return 1;
        }
    }
    return 0;
}

static inline HANDLE FindFirstFile(const char* lpFileName, WIN32_FIND_DATA* fd) {
    // Split path into directory and pattern
    char pathcopy[PATH_MAX];
    strncpy(pathcopy, lpFileName, sizeof(pathcopy) - 1);
    pathcopy[sizeof(pathcopy) - 1] = '\0';

    char* lastslash = strrchr(pathcopy, '/');
    FindFileHandle* fh = new FindFileHandle;
    if (lastslash) {
        strncpy(fh->pattern, lastslash + 1, 259);
        fh->pattern[259] = '\0';
        *(lastslash + 1) = '\0';
        strncpy(fh->dirpath, pathcopy, sizeof(fh->dirpath) - 1);
    } else {
        strncpy(fh->pattern, pathcopy, 259);
        fh->pattern[259] = '\0';
        strcpy(fh->dirpath, "./");
    }

    fh->dir = opendir(fh->dirpath);
    if (!fh->dir) { delete fh; return INVALID_HANDLE_VALUE; }

    if (_FindFileMatch(fh, fd)) return (HANDLE)fh;
    closedir(fh->dir);
    delete fh;
    return INVALID_HANDLE_VALUE;
}

static inline int FindNextFile(HANDLE h, WIN32_FIND_DATA* fd) {
    if (!h || h == INVALID_HANDLE_VALUE) return 0;
    return _FindFileMatch((FindFileHandle*)h, fd);
}

static inline int FindClose(HANDLE h) {
    if (h && h != INVALID_HANDLE_VALUE) {
        FindFileHandle* fh = (FindFileHandle*)h;
        if (fh->dir) closedir(fh->dir);
        delete fh;
    }
    return 1;
}

// Windows threading stubs for renderer (pthread-based replacement)
#define CREATE_SUSPENDED 0x00000004
typedef void* (*LPTHREAD_START_ROUTINE)(void*);

static inline HANDLE CreateThread(void*, size_t, LPTHREAD_START_ROUTINE proc, void* param, DWORD flags, void*) {
    pthread_t* t = new pthread_t;
    if (flags & CREATE_SUSPENDED) {
        // Store the parameters; thread will be started by ResumeThread
        // For simplicity, start immediately (suspend not well-supported in pthreads)
        pthread_create(t, NULL, (void*(*)(void*))proc, param);
    } else {
        pthread_create(t, NULL, (void*(*)(void*))proc, param);
    }
    return (HANDLE)t;
}

static inline DWORD ResumeThread(HANDLE h) {
    // No-op since we start threads immediately
    (void)h;
    return 0;
}

static inline DWORD SuspendThread(HANDLE h) {
    // No-op stub - renderer uses state flag to control thread
    (void)h;
    return 0;
}

static inline int CloseHandle(HANDLE h) {
    if (h && h != INVALID_HANDLE_VALUE) {
        pthread_t* t = (pthread_t*)h;
        pthread_join(*t, NULL);
        delete t;
    }
    return 1;
}

// MSVC secure string functions → standard equivalents
#define strcat_s(dst, sz, src)   strcat(dst, src)
#define strncpy_s(dst, sz, src, cnt) strncpy(dst, src, cnt)

// itoa (non-standard) → snprintf-based
static inline char* itoa(int value, char* buf, int radix) {
    if (radix == 10) sprintf(buf, "%d", value);
    else if (radix == 16) sprintf(buf, "%x", value);
    else buf[0] = '\0';
    return buf;
}

// Windows drive/path functions (not applicable on Linux)
static inline int _getdrive() { return 0; }
static inline char* _getdcwd(int, char* buf, int sz) { return getcwd(buf, sz); }

// MAXDWORD
#ifndef MAXDWORD
#define MAXDWORD 0xFFFFFFFF
#endif

// LPCSTR
typedef const char* LPCSTR;

// RECT struct
typedef struct tagRECT {
    long left, top, right, bottom;
} RECT;

// MessageBox stub (no GUI message boxes on Linux for now)
#define MB_OK           0x00000000
#define MB_ICONERROR    0x00000010
static inline int MessageBox(void*, const char*, const char*, unsigned int) { return 0; }

// ShellExecute stub
static inline void* ShellExecute(void*, const char*, const char*, const char*, const char*, int) { return NULL; }

// Timer stubs
static inline unsigned long SetTimer(void*, unsigned long id, unsigned long, void*) { return id; }
static inline int KillTimer(void*, unsigned long) { return 1; }

// GetFileAttributes stub
static inline unsigned long GetFileAttributes(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) return INVALID_FILE_ATTRIBUTES;
    if (S_ISDIR(st.st_mode)) return FILE_ATTRIBUTE_DIRECTORY;
    return 0;
}

// GetLogicalDrives stub (no drive letters on Linux)
static inline unsigned long GetLogicalDrives() { return 0; }

#endif // platform

#endif // PLATFORM_DEFS_H
