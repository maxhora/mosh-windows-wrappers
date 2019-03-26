//+build windows

package mosh

// #include "windows.h"
//
//  /*    Until we can specify the platform SDK and target version for Windows.h      *
//   * without breaking our ability to gracefully display an error message, these     *
//   * definitions will be copied from the platform SDK headers and made to work.     *
//   */
//
//  typedef BOOL (* InitializeProcThreadAttributeListProcType)(LPPROC_THREAD_ATTRIBUTE_LIST, DWORD, DWORD, PSIZE_T);
//  typedef BOOL (* UpdateProcThreadAttributeProcType)(
//            LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
//            DWORD                        dwFlags,
//            DWORD_PTR                    Attribute,
//            PVOID                        lpValue,
//            SIZE_T                       cbSize,
//            PVOID                        lpPreviousValue,
//            PSIZE_T                      lpReturnSize
//      );
//  typedef BOOL (* SetProcessDpiAwarenessContextType)(handle_t);
//
//  InitializeProcThreadAttributeListProcType mosh_pfnInitializeProcThreadAttributeList = NULL;
//  UpdateProcThreadAttributeProcType mosh_pfnUpdateProcThreadAttribute = NULL;
//  SetProcessDpiAwarenessContextType mosh_pfnSetProcessDpiAwarenessContext = NULL;
//
//  #define ProcThreadAttributePseudoConsole 22
//
//  #define PROC_THREAD_ATTRIBUTE_NUMBER    0x0000FFFF
//  #define PROC_THREAD_ATTRIBUTE_THREAD    0x00010000  // Attribute may be used with thread creation
//  #define PROC_THREAD_ATTRIBUTE_INPUT     0x00020000  // Attribute is input only
//  #define PROC_THREAD_ATTRIBUTE_ADDITIVE  0x00040000  // Attribute may be "accumulated," e.g. bitmasks, counters, etc.
//
//  #define ProcThreadAttributeValue(Number, Thread, Input, Additive) \
//      (((Number) & PROC_THREAD_ATTRIBUTE_NUMBER) | \
//      ((Thread != FALSE) ? PROC_THREAD_ATTRIBUTE_THREAD : 0) | \
//      ((Input != FALSE) ? PROC_THREAD_ATTRIBUTE_INPUT : 0) | \
//      ((Additive != FALSE) ? PROC_THREAD_ATTRIBUTE_ADDITIVE : 0))
//
//  #define PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE ProcThreadAttributeValue (ProcThreadAttributePseudoConsole, FALSE, TRUE, FALSE)
//
//  typedef struct _STARTUPINFOEXW {
//      STARTUPINFOW                 StartupInfo;
//      LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList;
//  } STARTUPINFOEXW, *LPSTARTUPINFOEXW;
//
//   #ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
//   #define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((handle_t)-4)
//   #endif
//
//  HMODULE mosh_hlibKernel32_Proc = NULL;
//
//  DWORD mosh_initProcKernFuncs()
//  {
//      mosh_hlibKernel32_Proc = LoadLibrary( "kernel32.dll" );
//      if( mosh_hlibKernel32_Proc == NULL )
//      {
//          return -1;
//      }
//
//      mosh_pfnInitializeProcThreadAttributeList = (InitializeProcThreadAttributeListProcType) GetProcAddress(mosh_hlibKernel32_Proc, "InitializeProcThreadAttributeList" );
//      if( mosh_pfnInitializeProcThreadAttributeList == NULL )
//      {
//          return -1;
//      }
//
//      mosh_pfnUpdateProcThreadAttribute = (UpdateProcThreadAttributeProcType) GetProcAddress(mosh_hlibKernel32_Proc, "UpdateProcThreadAttribute" );
//      if( mosh_pfnUpdateProcThreadAttribute == NULL )
//      {
//          return -1;
//      }
//
//      return 0;
//  }
//
//  DWORD mosh_createGuestProcHelper( uintptr_t hpc, LPCWSTR imagePath, uintptr_t * hProcess, DWORD * dwProcessID )
//  {
//      if( mosh_hlibKernel32_Proc == NULL )
//		{
// 			if ( mosh_initProcKernFuncs() )
//				return -1;
//		}
//      STARTUPINFOEXW si;
//      ZeroMemory( &si, sizeof(si) );
//      si.StartupInfo.cb = sizeof(si);
//
//      SIZE_T bytesRequired;
//      (*mosh_pfnInitializeProcThreadAttributeList)( NULL, 1, 0, &bytesRequired );
//
//      si.lpAttributeList = (PPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), 0, bytesRequired);
//      if( !si.lpAttributeList )
//      {
//          return E_OUTOFMEMORY;
//      }
//
//      if (!(*mosh_pfnInitializeProcThreadAttributeList)(si.lpAttributeList, 1, 0, &bytesRequired))
//      {
//          HeapFree(GetProcessHeap(), 0, si.lpAttributeList);
//          return HRESULT_FROM_WIN32(GetLastError());
//      }
//
//      if (!(*mosh_pfnUpdateProcThreadAttribute)(si.lpAttributeList,
//              0,
//              PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
//              (PVOID) hpc,
//              sizeof(hpc),
//              NULL,
//              NULL))
//      {
//          HeapFree(GetProcessHeap(), 0, si.lpAttributeList);
//          return HRESULT_FROM_WIN32(GetLastError());
//      }
//
//      bytesRequired = (wcslen(imagePath) + 1) * sizeof(wchar_t); // +1 null terminator
//      PWSTR cmdLineMutable = (PWSTR)HeapAlloc(GetProcessHeap(), 0, bytesRequired);
//
//      if (!cmdLineMutable)
//      {
//          HeapFree(GetProcessHeap(), 0, si.lpAttributeList);
//          return E_OUTOFMEMORY;
//      }
//
//      wcscpy_s(cmdLineMutable, bytesRequired, imagePath);
//
//      PROCESS_INFORMATION pi;
//      ZeroMemory(&pi, sizeof(pi));
//
//      if (!CreateProcessW(NULL,
//              cmdLineMutable,
//              NULL,
//              NULL,
//              FALSE,
//              EXTENDED_STARTUPINFO_PRESENT,
//              NULL,
//              NULL,
//              &si.StartupInfo,
//              &pi))
//      {
//          HeapFree(GetProcessHeap(), 0, si.lpAttributeList);
//          HeapFree(GetProcessHeap(), 0, cmdLineMutable);
//          return HRESULT_FROM_WIN32(GetLastError());
//      }
//
//      *hProcess = (uintptr_t) pi.hProcess;
//      *dwProcessID = pi.dwProcessId;
//
//      HeapFree(GetProcessHeap(), 0, si.lpAttributeList);
//      HeapFree(GetProcessHeap(), 0, cmdLineMutable);
//      return S_OK;
//  }
import "C"
import (
	"errors"
	"os"
	"unicode/utf16"
)

func startConPtyClient(fullPath string, params []string, hcon uintptr, exit chan int) error {
	path16 := utf16.Encode([]rune(fullPath))

	cpath16 := C.calloc(C.size_t(len(path16)+1), 2)
	defer C.free(cpath16)
	pp := (*[0xffff]uint16)(cpath16)
	copy(pp[:], path16)

	hproc := C.uintptr_t(0)
	dwProcessID := C.DWORD(0)

	hr := C.mosh_createGuestProcHelper(C.uintptr_t(hcon), (C.LPCWSTR)(cpath16), &hproc, &dwProcessID)
	_ = hr

	if dwProcessID == 0 {
		return errors.New("Failed to create process: " + fullPath)
	}

	goProcess, err := os.FindProcess(int(dwProcessID))
	if err != nil {
		return err
	}

	if exit != nil {
		convertProcessExitCodeToChannelMessage(goProcess, exit)
	}
	return err
}
