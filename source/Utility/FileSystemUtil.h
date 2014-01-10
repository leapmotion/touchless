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
  static bool ChangeWorkingDirectory(const std::string& path);
  static void ApplySeparatorStyle(std::string& path, int separatorStyle);
  static std::string GetUserPath(const std::string& filename = "");

  static const char PATH_SEPARATOR;

private:

  static const char PATH_SEPARATOR_WINDOWS;
  static const char PATH_SEPARATOR_UNIX;

};

#endif
