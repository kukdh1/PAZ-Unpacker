#include "Helper.h"

namespace kukdh1 {
  HTREEITEM AddTreeItem(HWND hTree, HTREEITEM hParent, HTREEITEM hInsertAfter, LPWSTR pszText, LPARAM lParam) {
    TVINSERTSTRUCT tvis;

    tvis.hParent = hParent;
    tvis.hInsertAfter = hInsertAfter;
    tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
    tvis.item.pszText = pszText;
    tvis.item.lParam = lParam;

    return TreeView_InsertItem(hTree, &tvis);
  }

  int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {
    switch (uMsg) {
      case BFFM_INITIALIZED:
        if (lpData != NULL) {
          SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)lpData);
        }
        break;
     }

    return 0;
  }
  
  BOOL BrowseFolder(HWND hParent, LPCWSTR szTitle, LPCWSTR szStartPath, WCHAR *szFolder, DWORD dwBufferLength) {
    LPMALLOC pMalloc;
    LPITEMIDLIST pidl;
    BROWSEINFO bi;

    bi.hwndOwner = hParent;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = NULL;
    bi.lpszTitle = szTitle;
    bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS;
    bi.lpfn = BrowseCallbackProc;
    bi.lParam = (LPARAM)szStartPath;

    pidl = SHBrowseForFolder(&bi);

    if (pidl == NULL) {
      return FALSE;
    }

    SHGetPathFromIDListEx(pidl, szFolder, dwBufferLength, GPFIDL_DEFAULT);

    if (SHGetMalloc(&pMalloc) != NOERROR) {
      return FALSE;
    }

    pMalloc->Free(pidl);
    pMalloc->Release();

    return TRUE;
  }

  void ParsePath(std::string path, std::vector<std::string> &folders) {
    std::stringstream ss(path);

    folders.clear();

    while (!ss.eof()) {
      std::string token;

      std::getline(ss, token, '/');
      folders.push_back(token);
    }
  }

  void ConvertWidechar(std::string &in, std::wstring &out) {
    uint32_t len;

    len = MultiByteToWideChar(CP_ACP, NULL, in.c_str(), in.length(), NULL, NULL);

    out.resize(len);

    len = MultiByteToWideChar(CP_ACP, NULL, in.c_str(), in.length(), (wchar_t *)out.c_str(), out.length());
  }

  void ConvertCapacity(LARGE_INTEGER &values, std::wstring &out) {
    LONGLONG qpart = values.QuadPart;
    std::wstringstream wss;
    int unit = 0;

    while (true) {
      qpart >>= 10;

      if (qpart > 0) {
        unit++;
      }
      else
        break;
    }

    switch (unit) {
      case 1:
        wss << std::to_wstring(values.QuadPart / 1024.0) << L" KB (" << std::to_wstring(values.QuadPart) << " bytes)";
        break;
      case 2:
        wss << std::to_wstring(values.QuadPart / 1024.0 / 1024.0) << L" MB (" << std::to_wstring(values.QuadPart) << " bytes)";
        break;
      case 3:
        wss << std::to_wstring(values.QuadPart / 1024.0 / 1024.0 / 1024.0) << L" GB (" << std::to_wstring(values.QuadPart) << " bytes)";
        break;
      default:
        wss << std::to_wstring(values.QuadPart) << " bytes";
        break;
    }

    out = wss.str();
  }
}
