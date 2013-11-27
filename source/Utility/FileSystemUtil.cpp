// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#include "stdafx.h"
#include "FileSystemUtil.h"
#include "ocuType.h"
#include "ocuConfig.h"
#include "CurrentApplication.h"
#ifdef _WIN32
#include <Windows.h>
#include <Shellapi.h>
#include <direct.h>
#else // POSIX
#if __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#include <sys/sysctl.h>
#else
#include <fstream>
#endif
#include <unistd.h>
#include <sys/stat.h>
#include <cstdlib>
#include <cstdio>
#endif
#include <string>
#include <sstream>
#include <time.h>
#include <boost/filesystem.hpp>
#include EXCEPTION_PTR_HEADER

const char FileSystemUtil::PATH_SEPARATOR_WINDOWS = '\\';
const char FileSystemUtil::PATH_SEPARATOR_UNIX = '/';

#ifdef _WIN32
const char FileSystemUtil::PATH_SEPARATOR = PATH_SEPARATOR_WINDOWS;
#include "UserEnv.h"
#pragma comment (lib,"UserEnv.lib")
#else // POSIX
const char FileSystemUtil::PATH_SEPARATOR = PATH_SEPARATOR_UNIX;
#undef _mkdir
#define _mkdir(path) mkdir(path, 0700)
#undef _getcwd
#define _getcwd getcwd
#undef _chdir
#define _chdir chdir
#endif

std::string FileSystemUtil::GetUserAppDirectory() {
#if _WIN32
  const char* appdata = getenv("APPDATA");
  return std::string(appdata ? appdata : "C:/");
#else
  const char* home = getenv("HOME");
  std::string appDir(home ? home : "~");
#if __APPLE__
  appDir += "/Library/Application Support";
#endif
  return appDir;
#endif
}

bool FileSystemUtil::DirectoryExists(const std::string& path) {
  std::string currentDir = GetWorkingDirectory();
  if (!ChangeWorkingDirectory(path)) {
    return false; // change directory failed, so the path doesn't exist
  }
  ChangeWorkingDirectory(currentDir); // change directory succeeded, so restore our original working directory
  return true;
}

bool FileSystemUtil::FileExists(const std::string& path) {
  return boost::filesystem::exists(path);
}

bool FileSystemUtil::MakeDirectory(const std::string& path) {
  // note that only the last directory in 'path' may be new... all preceding ones must already exist
  return (_mkdir(path.c_str()) == 0); // returns true if creation was successful
}

std::string FileSystemUtil::GetWorkingDirectory() {
  const size_t bufferSize = 1024;
  char buffer[bufferSize];
  if (_getcwd(buffer, bufferSize) != NULL) {
    return std::string(buffer);
  } else {
    return std::string();
  }
}

std::string FileSystemUtil::GetBinaryDirectory() {
  // used only on Linux for now, use /proc/self/exe to find where LeapApp came from
#ifdef LEAP_OS_LINUX
  const size_t bufferSize = 1024;
  char buffer[bufferSize];
  ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer)-1);
  if (len != -1) {
    buffer[len] = '\0';
    boost::filesystem::path app(buffer);
    return app.branch_path().string();
  }
  throw_rethrowable std::runtime_error("Path names too long (>= 1024 chars) to identify where we are");
#else
  CurrentApplication appInfo;
  boost::filesystem::path appPath(appInfo.app());
  return appPath.branch_path().string();
  //throw_rethrowable std::runtime_error("GetBinaryDirectory() not used on Windows or OS X");
#endif
}

bool FileSystemUtil::ChangeWorkingDirectory(const std::string& path) {
  return (_chdir(path.c_str()) == 0); // returns true if change was successful
}

std::string FileSystemUtil::FindUniquePath(const std::string& path) {
  std::stringstream ss;
  int curSuffix = 1;
  if (!DirectoryExists(path)) {
    return path;
  }
  while (true) {
    ss.str(path);
    ss << curSuffix;
    if (!DirectoryExists(ss.str())) {
      return ss.str();
    }
    curSuffix++;
  }
}

std::string FileSystemUtil::FindSharedDataPath(const std::string& filename) {
#ifdef LEAP_OS_LINUX
  std::string candidate_path(GetBinaryDirectory() + PATH_SEPARATOR + ".." + PATH_SEPARATOR + "share" + PATH_SEPARATOR + "Leap" + PATH_SEPARATOR + filename);
  if (boost::filesystem::exists(candidate_path)) {
    return candidate_path;
  }
#endif
  return filename;
}

void FileSystemUtil::OpenFile(const std::string& path, const std::string& args) {
#ifdef _WIN32
  HINSTANCE instance = ShellExecute(NULL, "open", path.c_str(), args.c_str(), NULL, SW_SHOWNORMAL);
  int returncode = reinterpret_cast<int>(instance);
  if( returncode < 32 ) {
    //failure

    printf("Shell Execute failed with code %d", returncode);
  }
#elif __APPLE__
  if (!path.empty()) {
    CFURLRef urlPathRef;

    if (path[0] != '/' && path[0] != '~') { // Relative Path
      CFBundleRef mainBundle = CFBundleGetMainBundle();
      CFURLRef urlExecRef = CFBundleCopyExecutableURL(mainBundle);
      CFURLRef urlRef = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, urlExecRef);
      CFStringRef pathRef = CFStringCreateWithCString(kCFAllocatorDefault, path.c_str(), kCFStringEncodingUTF8);
      urlPathRef = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, urlRef, pathRef, false);
      CFRelease(pathRef);
      CFRelease(urlRef);
      CFRelease(urlExecRef);
    } else {
      urlPathRef = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, reinterpret_cast<const UInt8 *>(path.data()), path.size(), false);
    }
    CFStringRef stringRef = CFURLCopyPath(urlPathRef);

    if (CFStringHasSuffix(stringRef, CFSTR(".app"))) {
      FSRef fsRef;

      if (CFURLGetFSRef(urlPathRef, &fsRef)) {
        LSApplicationParameters params = {0, kLSLaunchDefaults, &fsRef, 0, 0, 0};
        LSOpenApplication(&params, 0);
      }
    } else {
      LSOpenCFURLRef(urlPathRef, 0);
    }
    CFRelease(stringRef);
    CFRelease(urlPathRef);
  }
#else // Linux
  std::string exec_path = path;

  std::string candidate_path(FileSystemUtil::GetBinaryDirectory() + FileSystemUtil::PATH_SEPARATOR + path);
  if (boost::filesystem::exists(candidate_path)) {
    exec_path = candidate_path;
  }

  pid_t pid = fork();
  if (pid != 0) {
    return;
  }

  if (path.length() > 7) {
    // open up a downloaded package in Archive Manager
    if (path.substr(path.length() - 7).compare(".tar.gz") == 0) {
      execl("/usr/bin/file-roller", "file-roller", exec_path.c_str(), NULL);
      return;
    }
  }

  execl(exec_path.c_str(), exec_path.c_str(), (char*) NULL);
#endif
}

bool FileSystemUtil::RunInstaller(const std::string &path, bool silent) {
#ifdef _WIN32
  HINSTANCE inst = ShellExecute(NULL, "open", path.c_str(), silent ? "/UPGRADE" : "", NULL, silent ? SW_HIDE : SW_SHOWNORMAL);
  if( (unsigned int)inst <= 32 )
    return false;
#elif __APPLE__
  if (!path.empty()) {
    if( silent ) {
//      std::stringstream commandLine("installer -pkg ");
//      commandLine << path;
//      std::cerr << commandLine.str().c_str() << std::endl;
//      system(commandLine.str().c_str());
      OpenFile(path);
    }
    else {
      OpenFile(path);
    }
  }
#else // Linux
  std::string exec_path = path;

  std::string candidate_path(FileSystemUtil::GetBinaryDirectory() + FileSystemUtil::PATH_SEPARATOR + path);
  if (boost::filesystem::exists(candidate_path)) {
    exec_path = candidate_path;
  }

  pid_t pid = fork();
  if (pid != 0) {
    return true;
  }

  if (path.length() > 7) {
    // open up a downloaded package in Archive Manager
    if (path.substr(path.length() - 7).compare(".tar.gz") == 0) {
      execl("/usr/bin/file-roller", "file-roller", exec_path.c_str(), NULL);
      return true; // not reached
    }
  }

  execl(exec_path.c_str(), exec_path.c_str(), (char*) NULL);
#endif
  return true; // not reached
}

std::string FileSystemUtil::CheckOutput(const std::string& path, const std::string& args) {
  // similar to FileSystemUtil::OpenFile, except blocking and returns the output string
  // basically the same behavior at Python's subprocess.check_output
#ifdef _WIN32
  throw_rethrowable std::runtime_error("No known use case for FileSystemUtil::CheckOutput on Windows. abort");
  return "";
#else
  // POSIX
  const uint32_t PIPE_MAXLEN = 128;
  std::string command = path;
  if (args.length() > 0) {
    command = command + " " + args;
  }
  FILE* pipe = popen(path.c_str(), "r");
  if (!pipe) {
    throw_rethrowable std::runtime_error("Could not open a valid pipe");
  }
  char buffer[PIPE_MAXLEN];
  std::string result = "";
  while (!feof(pipe)) {
    if (fgets(buffer, PIPE_MAXLEN, pipe) != NULL) {
      result += buffer;
    }
  }
  pclose(pipe);
  return result;
#endif
}

std::string FileSystemUtil::FullPath(const std::string& directory, const std::string& filename) {
  return directory + PATH_SEPARATOR + filename;
}

void FileSystemUtil::ApplySeparatorStyle(std::string& path, int separatorStyle) {
  char oldChar, newChar;
  if (separatorStyle == SEPARATOR_STYLE_WINDOWS) {
    oldChar = PATH_SEPARATOR_UNIX;
    newChar = PATH_SEPARATOR_WINDOWS;
  } else if (separatorStyle == SEPARATOR_STYLE_UNIX) {
    oldChar = PATH_SEPARATOR_WINDOWS;
    newChar = PATH_SEPARATOR_UNIX;
  } else {
    return;
  }
  for (size_t i=0; i<path.size(); i++) {
    if (path[i] == oldChar) {
      path[i] = newChar;
    }
  }
}

std::string FileSystemUtil::CreateNameWithTime(const std::string& prefix, const std::string& extension) {
  time_t rawtime;
  time(&rawtime);
  std::string temp = ctime(&rawtime);
  temp = temp.substr(0, temp.length() - 1);
  for (size_t i=0; i<temp.length(); i++) {
    if (temp[i] == ' ') {
      temp[i] = '_';
    } else if (temp[i] == ':') {
      temp[i] = '-';
    }
  }
  std::stringstream ss;
  ss << prefix << temp << extension;
  return ss.str();
}

int FileSystemUtil::GetFilesInDirectory(const std::string& path, std::vector<std::string>& filenames, const std::string& extension, int ageInDays) {
  filenames.clear();
  boost::filesystem::path dir(path);
  if (!boost::filesystem::exists(dir)) {
    return 0;
  }
  std::time_t prevTime = ageInDays <= 0 ? time(0) : time(0)-86400*ageInDays;
  boost::filesystem::directory_iterator endIt; // default construction yields past-the-end
  for (boost::filesystem::directory_iterator it(dir); it != endIt; ++it) {
    if (boost::filesystem::is_regular_file(it->status())) {
      if (extension.empty() || !it->path().extension().string().compare(extension)) {
        if (difftime(boost::filesystem::last_write_time(it->path()), prevTime) < 0.0) {
          filenames.push_back(it->path().string());
        }
      }
    }
  }
  return static_cast<int>(filenames.size());
}

void FileSystemUtil::DeleteFiles(const std::vector<std::string>& filenames) {
  for (size_t i = 0; i < filenames.size(); i++) {
    DeleteOneFile(filenames[i]);
  }
}

void FileSystemUtil::DeleteOneFile(const std::string& filename) {
  boost::filesystem::path file(filename);
  if (boost::filesystem::exists(file)) {
    try {
      boost::filesystem::remove(file);
    } catch (...) { }
  }
}

std::string FileSystemUtil::GetUserPath(const std::string& filename) {
  std::stringstream ss;
  ss << FileSystemUtil::GetUserAppDirectory() << FileSystemUtil::PATH_SEPARATOR << APPLICATION_DIRECTORY;
  std::string curPath = ss.str();
  if (!FileSystemUtil::DirectoryExists(curPath)) {
    FileSystemUtil::MakeDirectory(curPath);
  }
  ss << FileSystemUtil::PATH_SEPARATOR << filename;
  curPath = ss.str();
  FileSystemUtil::ApplySeparatorStyle(curPath, FileSystemUtil::SEPARATOR_STYLE_UNIX);
  return curPath;
}

std::string FileSystemUtil::GetDumpPath(const std::string& filename) {
#ifdef _WIN32
  TCHAR pathBuf[1024];
  DWORD count = 1024;
  std::stringstream ss;
  if (GetAllUsersProfileDirectory(pathBuf, &count)) {
    ss << pathBuf << FileSystemUtil::PATH_SEPARATOR << "Leap Motion" << FileSystemUtil::PATH_SEPARATOR;
  }
  std::string currPath = ss.str();
  if (!FileSystemUtil::DirectoryExists(currPath)) {
    FileSystemUtil::MakeDirectory(currPath);
  }
  ss << filename;
  currPath = ss.str();
  FileSystemUtil::ApplySeparatorStyle(currPath, FileSystemUtil::SEPARATOR_STYLE_UNIX);
  return currPath;
#else
  return GetUserPath(filename);
#endif
}

std::string FileSystemUtil::GetOperatingSystemString() {
#ifdef __APPLE__
  const char* names[6] = {"Tiger", "Leopard", "Snow Leopard", "Lion", "Mountain Lion", "Mavericks"};
  char str[256];
  size_t size = sizeof(str);
  int ret = sysctlbyname("kern.osrelease", str, &size, NULL, 0);
  std::ostringstream ss;

  ss << "Mac OS X";
  if (!ret) {
    std::vector<int> components;
    size_t start = 0, i;
    for (i = 0; i < size; i++) {
      if (str[i] == '.') {
        components.push_back(atoi(&str[start]));
        start = i+1;
      }
    }
    if (start != i) {
      components.push_back(atoi(&str[start]));
    }
    if (components.size() >= 2) {
      if (components[0] >= 8) {
        ss << " 10." << (components[0] - 4) << "." << components[1];
        if (components[0] <= 13) {
          ss << " " << names[components[0] - 8];
        }
      }
    }
  }
  return ss.str();
#elif _WIN32
  typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
  typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);
  OSVERSIONINFOEX osvi;
  SYSTEM_INFO si;
  BOOL bOsVersionInfoEx;
  DWORD dwType;
  ZeroMemory(&si, sizeof(SYSTEM_INFO));
  ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
  bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*) &osvi);
  if (bOsVersionInfoEx == 0) {
    return "Windows";
  }
  PGNSI pGNSI = (PGNSI)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
  if (NULL != pGNSI) {
    pGNSI(&si);
  } else {
    GetSystemInfo(&si);
  }
  if (VER_PLATFORM_WIN32_NT != osvi.dwPlatformId || osvi.dwMajorVersion <= 4) {
    return "Windows";
  }
  std::stringstream ss;
  if (osvi.dwMajorVersion == 6) {
    if (osvi.dwMinorVersion == 0) {
      if (osvi.wProductType == VER_NT_WORKSTATION) {
        ss << "Windows Vista ";
      } else {
        ss << "Windows Server 2008 ";
      }
    } else if (osvi.dwMinorVersion == 1) {
      if (osvi.wProductType == VER_NT_WORKSTATION) {
        ss << "Windows 7 ";
      } else {
        ss << "Windows Server 2008 R2 ";
      }
    } else if (osvi.dwMinorVersion == 2) {
      if (osvi.wProductType == VER_NT_WORKSTATION) {
        ss << "Windows 8 ";
      } else {
        ss << "Windows Server 2012 ";
      }
    } else if (osvi.dwMinorVersion == 3) {
      if (osvi.wProductType == VER_NT_WORKSTATION) {
        ss << "Windows 8.1 ";
      } else {
        ss << "Windows Server 2012 R2 ";
      }
    }
    PGPI pGPI = (PGPI)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetProductInfo");
    pGPI(osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);
    switch (dwType) {
      case PRODUCT_ULTIMATE: ss << "Ultimate Edition"; break;
      case PRODUCT_PROFESSIONAL: ss << "Professional"; break;
      case PRODUCT_HOME_PREMIUM: ss << "Home Premium Edition"; break;
      case PRODUCT_HOME_BASIC: ss << "Home Basic Edition"; break;
      case PRODUCT_ENTERPRISE: ss << "Enterprise Edition"; break;
      case PRODUCT_BUSINESS: ss << "Business Edition"; break;
      case PRODUCT_STARTER: ss << "Starter Edition"; break;
      case PRODUCT_CLUSTER_SERVER: ss << "Cluster Server Edition"; break;
      case PRODUCT_DATACENTER_SERVER: ss << "Datacenter Edition"; break;
      case PRODUCT_DATACENTER_SERVER_CORE: ss << "Datacenter Edition (core installation)"; break;
      case PRODUCT_ENTERPRISE_SERVER: ss << "Enterprise Edition"; break;
      case PRODUCT_ENTERPRISE_SERVER_CORE: ss << "Enterprise Edition (core installation)"; break;
      case PRODUCT_ENTERPRISE_SERVER_IA64: ss << "Enterprise Edition for Itanium-based Systems"; break;
      case PRODUCT_SMALLBUSINESS_SERVER: ss << "Small Business Server"; break;
      case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM: ss << "Small Business Server Premium Edition"; break;
      case PRODUCT_STANDARD_SERVER: ss << "Standard Edition"; break;
      case PRODUCT_STANDARD_SERVER_CORE: ss << "Standard Edition (core installation)"; break;
      case PRODUCT_WEB_SERVER: ss << "Web Server Edition"; break;
    }
  }
  if (osvi.dwMajorVersion == 5) {
    if (osvi.dwMinorVersion == 2) {
      if (GetSystemMetrics(SM_SERVERR2)) {
        ss << "Windows Server 2003 R2, ";
      } else if (osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER) {
        ss << "Windows Storage Server 2003";
      } else if (osvi.wSuiteMask & VER_SUITE_WH_SERVER) {
        ss << "Windows Home Server";
      } else if (osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64) {
        ss << "Windows XP Professional x64 Edition";
      } else {
        ss << "Windows Server 2003, ";
      }
      if (osvi.wProductType != VER_NT_WORKSTATION) {
        if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64) {
          if (osvi.wSuiteMask & VER_SUITE_DATACENTER) {
            ss << "Datacenter Edition for Itanium-based Systems";
          } else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE) {
            ss << "Enterprise Edition for Itanium-based Systems";
          }
        } else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
          if (osvi.wSuiteMask & VER_SUITE_DATACENTER) {
            ss << "Datacenter x64 Edition";
          } else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE) {
            ss << "Enterprise x64 Edition";
          } else {
            ss << "Standard x64 Edition";
          }
        } else {
          if (osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER) {
            ss << "Compute Cluster Edition";
          } else if (osvi.wSuiteMask & VER_SUITE_DATACENTER) {
            ss << "Datacenter Edition";
          } else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE) {
            ss << "Enterprise Edition";
          } else if (osvi.wSuiteMask & VER_SUITE_BLADE) {
            ss << "Web Edition";
          } else {
            ss << "Standard Edition";
          }
        }
      }
    } else if (osvi.dwMinorVersion == 1) {
      ss << "Windows XP ";
      if (osvi.wSuiteMask & VER_SUITE_PERSONAL) {
        ss << "Home Edition";
      } else {
        ss << "Professional";
      }
    } else if (osvi.dwMinorVersion == 0) {
      ss << "Windows 2000 ";
      if (osvi.wProductType == VER_NT_WORKSTATION) {
        ss << "Professional";
      } else {
        if (osvi.wSuiteMask & VER_SUITE_DATACENTER) {
          ss << "Datacenter Server";
        } else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE) {
          ss << "Advanced Server";
        } else {
          ss << "Server";
        }
      }
    }
  }
  if (strlen(osvi.szCSDVersion) > 0) {
    ss << " " << osvi.szCSDVersion;
  }
  ss << " (build " << osvi.dwBuildNumber << ")";
  if (osvi.dwMajorVersion >= 6) {
    if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
      ss <<  ", 64-bit";
    } else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {
      ss << ", 32-bit";
    }
  }
  return ss.str();
#else
  std::ifstream ifs("/proc/version");

  if (ifs.good()) {
    char buffer[512];

    ifs.read(buffer, sizeof(buffer));
    size_t n = ifs.gcount();
    if (n > 0 && n <= sizeof(buffer)) {
      // Strip any trailing newlines
      while (n > 0 && buffer[n-1] == '\n') {
        n--;
      }
      if (n > 0) {
        return std::string(buffer, n);
      }
    }
  }
  try {
    return CheckOutput("uname", "-a");
  } catch (...) {}
  return "Linux unknown";
#endif
}

bool FileSystemUtil::OperatingSystemSupportsTouch() {
  std::string os = GetOperatingSystemString();
  // Windows systems older than Windows 7
  if (strncmp(os.c_str(), "Windows 2000", 12) == 0 ||
      strncmp(os.c_str(), "Windows XP", 10) == 0 ||
      strncmp(os.c_str(), "Windows Vista", 13) == 0 ||
      (strncmp(os.c_str(), "Windows Server 2008", 19) == 0 &&
       strncmp(os.c_str(), "Windows Server 2008 R2", 22) != 0)) {
    return false;
  }
  // Linux
  if (strncmp(os.c_str(), "Linux", 5) == 0) {
    return false;
  }
  return true;
}

std::string FileSystemUtil::GetInstallType() {
  // possible outputs: pongo, hops, default
#if _WIN32
  std::string programDataPath = getenv("PROGRAMDATA");
  programDataPath.append("\\Leap Motion\\installtype");

  std::ifstream installTypeFile(programDataPath, std::ios_base::in);
  if( installTypeFile ) {
    std::string installtype;
    installTypeFile >> installtype;
    return installtype;
  }
#endif
  return "default";
}

bool FileSystemUtil::IsEmbeddedInstall() {
  const std::string installtype = FileSystemUtil::GetInstallType();
  return installtype.compare("pongo") == 0 || installtype.compare("hops") == 0;
}

#if _WIN32
#include <Accctrl.h>
#include <Aclapi.h>
void FileSystemUtil::SetFilePermission(const char* FileName)
{
    PSID pEveryoneSID = NULL;
    PACL pACL = NULL;
    EXPLICIT_ACCESS ea[1];
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;

    // Create a well-known SID for the Everyone group.
    AllocateAndInitializeSid(&SIDAuthWorld, 1,
                     SECURITY_WORLD_RID,
                     0, 0, 0, 0, 0, 0, 0,
                     &pEveryoneSID);

    // Initialize an EXPLICIT_ACCESS structure for an ACE.
    // The ACE will allow Everyone read access to the key.
    ZeroMemory(&ea, 1 * sizeof(EXPLICIT_ACCESS));
    ea[0].grfAccessPermissions = 0xFFFFFFFF;
    ea[0].grfAccessMode = GRANT_ACCESS;
    ea[0].grfInheritance= NO_INHERITANCE;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[0].Trustee.ptstrName  = (LPTSTR) pEveryoneSID;

    // Create a new ACL that contains the new ACEs.
    SetEntriesInAcl(1, ea, NULL, &pACL);

    // Initialize a security descriptor.  
    PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, 
                                SECURITY_DESCRIPTOR_MIN_LENGTH); 

    InitializeSecurityDescriptor(pSD,SECURITY_DESCRIPTOR_REVISION);

    // Add the ACL to the security descriptor. 
    SetSecurityDescriptorDacl(pSD, 
            TRUE,     // bDaclPresent flag   
            pACL, 
            FALSE);   // not a default DACL 


    //Change the security attributes
    SetFileSecurity(FileName, DACL_SECURITY_INFORMATION, pSD);

    if (pEveryoneSID) 
        FreeSid(pEveryoneSID);
    if (pACL) 
        LocalFree(pACL);
    if (pSD) 
        LocalFree(pSD);
}
#endif
