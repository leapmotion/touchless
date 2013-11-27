// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#ifndef __FileSystemUtil_h__
#define __FileSystemUtil_h__

/// <summary>
/// Functions for interacting with the file system
/// </summary>
/// <remarks>
/// This class abstracts some common file system tasks such as creating/switching directories and opening files.
/// Maintainers: Raffi
/// </remarks>

#include <string>
#include <vector>

class FileSystemUtil
{

public:

  enum { SEPARATOR_STYLE_WINDOWS, SEPARATOR_STYLE_UNIX };

  static std::string GetUserAppDirectory();
  static bool DirectoryExists(const std::string& path);
  static bool FileExists(const std::string& path);
  static bool MakeDirectory(const std::string& path);
  static std::string GetWorkingDirectory();
  static std::string GetBinaryDirectory();
  static bool ChangeWorkingDirectory(const std::string& path);
  static std::string FindUniquePath(const std::string& path);
  static std::string FindSharedDataPath(const std::string& path);
  static void OpenFile(const std::string& path, const std::string& args = "");
  static bool RunInstaller(const std::string& path, bool silent = true);
  static std::string CheckOutput(const std::string& path, const std::string& args = "");
  static std::string FullPath(const std::string& directory, const std::string& filename);
  static void ApplySeparatorStyle(std::string& path, int separatorStyle);
  static std::string CreateNameWithTime(const std::string& prefix, const std::string& extension);
  static int GetFilesInDirectory(const std::string& path, std::vector<std::string>& filenames, const std::string& extension = "", int ageInDays = 0);
  static void DeleteFiles(const std::vector<std::string>& filenames);
  static void DeleteOneFile(const std::string& filename);
  static std::string GetUserPath(const std::string& filename = "");
  static std::string GetDumpPath(const std::string& filename = "");
  static std::string GetOperatingSystemString();
  static bool OperatingSystemSupportsTouch();
  static std::string GetInstallType();
  static bool IsEmbeddedInstall();
#if _WIN32
  static void SetFilePermission(const char* FileName);
#endif

  static const char PATH_SEPARATOR;

private:

  static const char PATH_SEPARATOR_WINDOWS;
  static const char PATH_SEPARATOR_UNIX;

};

#endif
