#pragma once
#include "InterfaceCodes.h"
#include <Windows.h>
#include <string>
#include <iostream>

using namespace std;

enum eHidStatus;

/// <summary>
/// Stores and retrieves information about the currently focused window
/// </summary>
class CFocusAppInfo
{
public:
	CFocusAppInfo(void);
	~CFocusAppInfo(void);
	
public:
	/// <summary>
	/// A handle to the window that currently has focus
	/// </summary>
	HWND m_hFocus;

	/// <summary>
	/// The process ID of the process that owns this window
	/// </summary>
	DWORD m_pid;

	/// <summary>
	/// The thread ID of the thread that owns this window
	/// </summary>
	DWORD m_tid;

	// Window extent information
	RECT m_rcWindow;
	RECT m_rcClient;

	/// <summary>
	/// The title of the target window
	/// </summary>
	wstring m_windowTitle;

	/// <summary>
	/// The name of the process that owns the focus window
	/// </summary>
	wstring m_ownerExeName;
	
	/// <summary>
	/// The full path to the process's executable
	/// </summary>
	wstring m_ownerExePath;

public:
	/// <summary>
	/// Gets information about the currently focused window
	/// </summary>
	eHidStatus Update(void);
};

/// <summary>
/// Debug stream manipulator for dumping the contents of this structure
/// </summary>
wostream& operator<<(wostream& os, CFocusAppInfo& rhs);