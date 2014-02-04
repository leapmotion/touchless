#include "stdafx.h"
#include "FocusAppInfo.h"

CFocusAppInfo::CFocusAppInfo(void):
	m_hFocus(nullptr)
{
}

CFocusAppInfo::~CFocusAppInfo(void)
{
}

eHidStatus CFocusAppInfo::Update(void)
{
	// Get the actual top window:
	m_hFocus = GetForegroundWindow();
	if(!m_hFocus)
		return eHidIntrNoFocusWindow;

	// Get the root parent of the top window:
	m_hFocus = GetAncestor(m_hFocus, GA_ROOT);
	
	// Get the PID and TID about the window:
	m_tid = GetWindowThreadProcessId(m_hFocus, &m_pid);

	// Window position and extent information:
	WINDOWINFO wi;
	wi.cbSize = sizeof(wi);

	// Get information about the focus window
	GetWindowInfo(m_hFocus, &wi);
	m_rcWindow = wi.rcWindow;
	m_rcClient = wi.rcClient;

	// Title window acquisition:
	m_windowTitle.resize(MAX_PATH);
	m_windowTitle.resize(
		GetWindowTextW(
			m_hFocus,
			&m_windowTitle[0],
			MAX_PATH
		)
	);

	// Owner path acquisition:
	HANDLE hProcess = OpenProcess(
		PROCESS_VM_READ | PROCESS_QUERY_LIMITED_INFORMATION,
		false,
		m_pid
	);
	if(hProcess)
	{
		// Owner EXE path allows a maximum of MAX_PATH characters
		m_ownerExePath.resize(MAX_PATH);

		// Resize the path according to the number of returned characters
		m_ownerExePath.resize(
			// Get the module name of the process we just opened
			GetProcessImageFileNameW(
				hProcess,
				&m_ownerExePath[0],
				MAX_PATH
			)
		);

		// Done with the process, close the handle
		CloseHandle(hProcess);

		// Extract just the process name:
		m_ownerExeName = PathFindFileNameW(m_ownerExePath.c_str());
	}

	return eHidIntrSuccess;
}

wostream& operator<<(wostream& wos, CFocusAppInfo& rhs)
{
	// Just output details about the focus application.
	return
		wos << "Extent:       " << "(" << rhs.m_rcWindow.left << ", " << rhs.m_rcWindow.top << ") to ("
								<< rhs.m_rcWindow.right << ", " << rhs.m_rcWindow.bottom << ")" << endl
			<< "Window title: " << rhs.m_windowTitle << endl
			<< "Process name: " << rhs.m_ownerExeName << endl
			<< "Process path: " << rhs.m_ownerExePath << endl;
}
