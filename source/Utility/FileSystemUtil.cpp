// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#include "stdafx.h"
#include "FileSystemUtil.h"
#include "common.h"
// #include "CurrentApplication.h"
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

bool FileSystemUtil::ChangeWorkingDirectory(const std::string& path) {
  return (_chdir(path.c_str()) == 0); // returns true if change was successful
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
