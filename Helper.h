#pragma once

#ifndef _HELPER_H_
#define _HELPER_H_

#include <Windows.h>
#include <CommCtrl.h>
#include <ShlObj.h>
#include <vector>
#include <string>
#include <sstream>

namespace kukdh1 {
  HTREEITEM AddTreeItem(HWND hTree, HTREEITEM hParent, HTREEITEM hInsertAfter, LPWSTR pszText, LPARAM lParam);
  BOOL BrowseFolder(HWND hParent, LPCWSTR szTitle, LPCWSTR szStartPath, WCHAR *szFolder, DWORD dwBufferLength);
  void ParsePath(std::string path, std::vector<std::string> &folders);
  void ConvertWidechar(std::string &in, std::wstring &out);
  void ConvertCapacity(LARGE_INTEGER &value, std::wstring &out);
}

#endif
