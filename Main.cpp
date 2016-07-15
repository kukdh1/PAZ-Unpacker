#include "Main.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
DWORD WINAPI FileThread(LPVOID arg);
DWORD WINAPI ExtractThread(LPVOID arg);

AppData app;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) {
	HWND hWnd;
	WNDCLASS wndclass;
	MSG msg;
	INITCOMMONCONTROLSEX iccex;
  std::wstring lpszClass = app.CSetting.getString(kukdh1::Setting::ID_CAPTION);

	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	iccex.dwICC = ICC_WIN95_CLASSES | ICC_PROGRESS_CLASS | ICC_TREEVIEW_CLASSES;

	if (!InitCommonControlsEx(&iccex)) {
		return -1;
	}

	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
  wndclass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hInstance = hInstance;
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = lpszClass.c_str();
	wndclass.lpszMenuName = NULL;
	wndclass.style = CS_VREDRAW | CS_HREDRAW;

	RegisterClass(&wndclass);
	
	hWnd = CreateWindow(lpszClass.c_str(), lpszClass.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

BOOL Cls_OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct) {
  /* Create controls */
  app.hButtonOpen = CreateWindow(WC_BUTTON, app.CSetting.getString(kukdh1::Setting::ID_OPEN).c_str(), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hWnd, (HMENU)ID_BUTTON_OPEN, lpCreateStruct->hInstance, NULL);
  app.hButtonExctact = CreateWindow(WC_BUTTON, app.CSetting.getString(kukdh1::Setting::ID_EXTRACT).c_str(), WS_CHILD | WS_VISIBLE | WS_DISABLED | BS_PUSHBUTTON, 0, 0, 0, 0, hWnd, (HMENU)ID_BUTTON_EXTRACT, lpCreateStruct->hInstance, NULL);
	app.hTreeFileSystem = CreateWindow(WC_TREEVIEW, NULL, WS_CHILD | WS_VISIBLE | TVS_DISABLEDRAGDROP | TVS_HASBUTTONS | TVS_TRACKSELECT | TVS_LINESATROOT, 0, 0, 0, 0, hWnd, (HMENU)ID_TREE_FILESYSTEM, lpCreateStruct->hInstance, NULL);
  app.hStatusBar = CreateWindow(STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, hWnd, (HMENU)ID_STATUSBAR, lpCreateStruct->hInstance, NULL);
  app.hStaticInfo = CreateWindow(WC_STATIC, NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, 0, 0, 0, 0, hWnd, (HMENU)ID_STATIC, lpCreateStruct->hInstance, NULL);

  /* Set theme*/
  SetWindowTheme(app.hTreeFileSystem, L"Explorer", NULL);

  /* Create/Apply font */
  app.hFont = CreateFont(FONT_SIZE, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_ROMAN, FONT_FACE);

  SendMessage(app.hButtonOpen, WM_SETFONT, (WPARAM)app.hFont, TRUE);
  SendMessage(app.hButtonExctact, WM_SETFONT, (WPARAM)app.hFont, TRUE);
  SendMessage(app.hTreeFileSystem, WM_SETFONT, (WPARAM)app.hFont, TRUE);
  SendMessage(app.hStatusBar, WM_SETFONT, (WPARAM)app.hFont, TRUE);
  SendMessage(app.hStaticInfo, WM_SETFONT, (WPARAM)app.hFont, TRUE);

  /* Set status bar */
  int parts[STATUSBAR_SECTION_COUNT] = { STATUSBAR_SECTION1, STATUSBAR_SECTION2, -1 };
  
  SendMessage(app.hStatusBar, SB_SETPARTS, STATUSBAR_SECTION_COUNT, (LPARAM)parts);
  SendMessage(app.hStatusBar, SB_SETTEXT, NULL, (LPARAM)app.CSetting.getString(kukdh1::Setting::ID_STATUS_IDLE).c_str());

  /* Set Progress bar */
  RECT rtArea;

  SendMessage(app.hStatusBar, SB_GETRECT, 1, (LPARAM)&rtArea);
  app.hProgressBar = CreateWindow(PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, rtArea.left, rtArea.top, rtArea.right - rtArea.left, rtArea.bottom - rtArea.top, app.hStatusBar, NULL, lpCreateStruct->hInstance, NULL);

	return TRUE;
}

void Cls_OnDestroy(HWND hWnd) {
  {
    SAFE_DELETE(app.CTree);
    SAFE_DELETE(app.CMeta);
    SAFE_FREE(app.wpszFolderPath);
  }

  DeleteObject(app.hFont);

	PostQuitMessage(0);
}

void Cls_OnSize(HWND hWnd, UINT state, int cx, int cy) {
  int nTreeWidth = (int)(cx * DIVIDE_RATIO + 0.5f);
  int nStaticWidth = cx - nTreeWidth;
  int nStatusbarHeight = GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYBORDER) * 2;

	MoveWindow(app.hTreeFileSystem, 0, 0, nTreeWidth, cy - nStatusbarHeight, TRUE);
  MoveWindow(app.hStaticInfo, nTreeWidth, 0, nStaticWidth - BUTTON_WIDTH, cy - nStatusbarHeight, TRUE);
  MoveWindow(app.hStatusBar, 0, 0, 0, 0, TRUE);
  MoveWindow(app.hButtonOpen, cx - BUTTON_WIDTH, 0, BUTTON_WIDTH, BUTTON_HEIGHT, TRUE);
  MoveWindow(app.hButtonExctact, cx - BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, TRUE);
}

void Cls_OnGetMinMaxInfo(HWND hWnd, LPMINMAXINFO lpMinMaxInfo) {
  lpMinMaxInfo->ptMinTrackSize.x = WINDOW_MIN_WIDTH;
  lpMinMaxInfo->ptMinTrackSize.y = WINDOW_MIN_HEIGHT;
}

void Cls_OnCommand(HWND hWnd, int id, HWND hwndCtl, UINT codeNotify) {
  switch (id) {
    case ID_BUTTON_OPEN:
      {
        WCHAR *pszFolderPath = (WCHAR *)calloc(4096, sizeof(WCHAR));
        std::wstring wsLastPath;

        app.CSetting.getData(SETTING_LAST_FOLDER, wsLastPath, L"C:\\");

        if (kukdh1::BrowseFolder(hWnd, app.CSetting.getString(kukdh1::Setting::ID_SELECT_FOLDER_TO_OPEN).c_str(), wsLastPath.c_str(), pszFolderPath, 4096)) {
          {
            TreeView_DeleteAllItems(app.hTreeFileSystem);
            SendMessage(app.hStaticInfo, WM_SETTEXT, NULL, (LPARAM)L"");
            SAFE_DELETE(app.CTree);
            SAFE_DELETE(app.CMeta);
            SAFE_FREE(app.wpszFolderPath);
          }

          app.wpszFolderPath = (WCHAR *)calloc(wcslen(pszFolderPath) + 1, sizeof(WCHAR));
          wcscpy_s(app.wpszFolderPath, wcslen(pszFolderPath) + 1, pszFolderPath);

          app.CSetting.setData(SETTING_LAST_FOLDER, app.wpszFolderPath);

          wsprintf(pszFolderPath, app.CSetting.getString(kukdh1::Setting::ID_CAPTION_WITH_PATH).c_str(), app.wpszFolderPath);
          SetWindowText(hWnd, pszFolderPath);

          try {
            app.CMeta = new kukdh1::Meta(app.wpszFolderPath);
            app.CTree = new kukdh1::Tree(kukdh1::Tree::TREE_TYPE_ROOT);

            HANDLE hThread = CreateThread(NULL, 0, FileThread, NULL, 0, NULL);
            CloseHandle(hThread);
          }
          catch (std::exception e) {
            MessageBox(hWnd, app.CSetting.getString(kukdh1::Setting::ID_NO_META_FILE_EXISTS).c_str(), app.CSetting.getString(kukdh1::Setting::ID_ALERT).c_str(), MB_OK);
            SAFE_DELETE(app.CMeta);
            SAFE_FREE(app.wpszFolderPath);
            SetWindowText(hWnd, app.CSetting.getString(kukdh1::Setting::ID_CAPTION).c_str());
          }
        }

        free(pszFolderPath);
      }
      
      break;
    case ID_BUTTON_EXTRACT:
      {
        TVITEM tvi;
        HTREEITEM hTree = TreeView_GetSelection(app.hTreeFileSystem);

        tvi.hItem = hTree;
        tvi.mask = TVIF_PARAM;
        TreeView_GetItem(app.hTreeFileSystem, &tvi);

        if (tvi.lParam) {
          HANDLE hThread = CreateThread(NULL, 0, ExtractThread, (LPVOID)tvi.lParam, 0, NULL);
          CloseHandle(hThread);
        }
      }

      break;
  }
}

void Cls_OnDrawItem(HWND hWnd, const DRAWITEMSTRUCT * lpDrawItem) {
  if (lpDrawItem->CtlID == ID_STATIC) {
    FillRect(lpDrawItem->hDC, &lpDrawItem->rcItem, GetSysColorBrush(COLOR_BTNFACE));

    uint32_t uiLength = SendMessage(lpDrawItem->hwndItem, WM_GETTEXTLENGTH, 0, 0);
    WCHAR *pszText = (WCHAR *)calloc(uiLength + 1, sizeof(WCHAR));
    SendMessage(lpDrawItem->hwndItem, WM_GETTEXT, uiLength + 1, (LPARAM)pszText);
    DrawText(lpDrawItem->hDC, pszText, uiLength, (LPRECT)&lpDrawItem->rcItem, DT_PATH_ELLIPSIS | DT_WORDBREAK);
    free(pszText);
  }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	switch (iMessage) {
    HANDLE_MSG(hWnd, WM_CREATE, Cls_OnCreate);
    HANDLE_MSG(hWnd, WM_DESTROY, Cls_OnDestroy);
    HANDLE_MSG(hWnd, WM_SIZE, Cls_OnSize);
    HANDLE_MSG(hWnd, WM_GETMINMAXINFO, Cls_OnGetMinMaxInfo);
    HANDLE_MSG(hWnd, WM_COMMAND, Cls_OnCommand);
    HANDLE_MSG(hWnd, WM_DRAWITEM, Cls_OnDrawItem);

    case WM_NOTIFY:
      {
        LPNMHDR hdr = (LPNMHDR)lParam;

        if (hdr->idFrom == ID_TREE_FILESYSTEM) {
          LPNMTREEVIEW ntv = (LPNMTREEVIEW)lParam;

          if (hdr->code == TVN_SELCHANGED) {
            kukdh1::Tree *pTree;
            WCHAR *pszBuffer;
            std::wstring capacity;
            
            pszBuffer = (WCHAR *)calloc(1024, sizeof(WCHAR));
            pTree = (kukdh1::Tree *)ntv->itemNew.lParam;
            if (pTree != NULL) {
              switch (pTree->GetType()) {
                case kukdh1::Tree::TREE_TYPE_ROOT:
                  if (app.CMeta != NULL) {
                    kukdh1::ConvertCapacity(app.CTree->GetCapacity(), capacity);

                    wsprintf(pszBuffer, app.CSetting.getString(kukdh1::Setting::ID_META_FILE_INFO).c_str(), app.CMeta->uiVersion, app.CMeta->uiPAZFileCount, capacity.c_str());
                    SendMessage(app.hStaticInfo, WM_SETTEXT, NULL, (LPARAM)pszBuffer);
                  }

                  break;
                case kukdh1::Tree::TREE_TYPE_FOLDER:
                  kukdh1::ConvertCapacity(pTree->GetCapacity(), capacity);
                  wsprintf(pszBuffer, app.CSetting.getString(kukdh1::Setting::ID_INTERNAL_FOLDER_INFO).c_str(), pTree->GetName().c_str(), capacity.c_str());
                  SendMessage(app.hStaticInfo, WM_SETTEXT, NULL, (LPARAM)pszBuffer);
                  break;
                case kukdh1::Tree::TREE_TYPE_FILE:
                  kukdh1::ConvertCapacity(pTree->GetCapacity(), capacity);
                  wsprintf(pszBuffer, app.CSetting.getString(kukdh1::Setting::ID_INTERNAL_FILE_INFO).c_str(), pTree->GetName().c_str(), capacity.c_str(), pTree->GetFileInfo().wsPazFullPath.c_str(), pTree->GetFileInfo().sFullPath.c_str());
                  SendMessage(app.hStaticInfo, WM_SETTEXT, NULL, (LPARAM)pszBuffer);
                  break;
              }
            }

            free(pszBuffer);
          }
        }
      }

      return 0;
	}

	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}

DWORD WINAPI FileThread(LPVOID arg) {
  kukdh1::CryptICE cipher(ICE_KEY, ICE_KEY_LEN);
  std::vector<std::string> paths;
  WCHAR buffer[64];

  EnableWindow(app.hButtonOpen, FALSE);
  EnableWindow(app.hTreeFileSystem, FALSE);
  SendMessage(app.hStatusBar, SB_SETTEXT, 0, (LPARAM)app.CSetting.getString(kukdh1::Setting::ID_STATUS_BUSY).c_str());

  SendMessage(app.hProgressBar, PBM_SETRANGE32, 0, app.CMeta->uiPAZFileCount);
  uint32_t i = 0;

  for (auto iter = app.CMeta->vPAZs.begin(); iter != app.CMeta->vPAZs.end(); iter++) {
    SendMessage(app.hProgressBar, PBM_SETPOS, i++, NULL);
    wsprintf(buffer, app.CSetting.getString(kukdh1::Setting::ID_PROGRESS_READING).c_str(), i, app.CMeta->uiPAZFileCount);
    SendMessage(app.hStatusBar, SB_SETTEXT, 2, (LPARAM)buffer);

    kukdh1::PazFile paz(app.wpszFolderPath, iter->uiPazFileID, cipher);

    for (auto file = paz.vFileInfo.begin(); file != paz.vFileInfo.end(); file++) {
      kukdh1::Tree *ptr = app.CTree;
      kukdh1::ParsePath(file->sFullPath, paths);

      for (auto path = paths.begin(); path != paths.end() - 1; path++) {
        kukdh1::Tree *temp = ptr->GetChildFolderWithName(*path);
        if (temp) {
          ptr = temp;
        }
        else {
          temp = new kukdh1::Tree(kukdh1::Tree::TREE_TYPE_FOLDER);
          temp->SetFolderInfo(ptr, *path);
          ptr->AddChild(temp);
          ptr = temp;
        }
      }

      kukdh1::Tree *temp = new kukdh1::Tree(kukdh1::Tree::TREE_TYPE_FILE);
      temp->SetFileInfo(ptr, paths.back(), *file);
      ptr->AddChild(temp);
    }
  }

  SendMessage(app.hProgressBar, PBM_SETPOS, 0, NULL);
  SendMessage(app.hProgressBar, PBM_SETRANGE32, 0, 3);

  SendMessage(app.hStatusBar, SB_SETTEXT, 2, (LPARAM)app.CSetting.getString(kukdh1::Setting::ID_PROGRESS_SORTING).c_str());
  app.CTree->SortChild();
  SendMessage(app.hProgressBar, PBM_SETPOS, 1, NULL);

  SendMessage(app.hStatusBar, SB_SETTEXT, 2, (LPARAM)app.CSetting.getString(kukdh1::Setting::ID_PROGRESS_CAPACITY).c_str());
  app.CTree->UpdateCapacity();
  SendMessage(app.hProgressBar, PBM_SETPOS, 2, NULL);

  SendMessage(app.hStatusBar, SB_SETTEXT, 2, (LPARAM)app.CSetting.getString(kukdh1::Setting::ID_PROGRESS_ADDING).c_str());
  app.CTree->AddToTree(app.hTreeFileSystem);
  SendMessage(app.hProgressBar, PBM_SETPOS, 3, NULL);

  SendMessage(app.hStatusBar, SB_SETTEXT, 0, (LPARAM)app.CSetting.getString(kukdh1::Setting::ID_STATUS_IDLE).c_str());
  EnableWindow(app.hButtonOpen, TRUE);
  EnableWindow(app.hButtonExctact, TRUE);
  EnableWindow(app.hTreeFileSystem, TRUE);

  TreeView_Select(app.hTreeFileSystem, app.CTree->GetHandle(), TVGN_CARET);
  TreeView_Expand(app.hTreeFileSystem, app.CTree->GetHandle(), TVE_EXPAND);

  SendMessage(app.hProgressBar, PBM_SETPOS, 0, NULL);
  SendMessage(app.hStatusBar, SB_SETTEXT, 2, (LPARAM)app.CSetting.getString(kukdh1::Setting::ID_PROGRESS_READY).c_str());

  return 0;
}

void ExtractFile(std::wstring &path, kukdh1::FileInfo &file, kukdh1::Crypt &cipher) {
  bool bCompressed = false;

  if (file.uiOriginalSize > file.uiCompressedSize) {
    bCompressed = true;
  }

  std::fstream pazfile;
  std::fstream savefile;

  pazfile.open(file.wsPazFullPath, std::ios::in | std::ios::binary);
  if (!pazfile.is_open()) {
    return;
  }

  savefile.open(path, std::ios::out | std::ios::binary);
  if (!savefile.is_open()) {
    return;
  }

  uint8_t *encrypted;
  uint8_t *decrypted;
  uint32_t length = file.uiCompressedSize;

  encrypted = (uint8_t *)calloc(length, 1);
  pazfile.seekg(file.uiOffset);
  pazfile.read((char *)encrypted, length);
  pazfile.close();

  cipher.decrypt(encrypted, length, &decrypted, &length);
  free(encrypted);

  if (bCompressed) {
    uint8_t *decompressed = (uint8_t *)calloc(file.uiOriginalSize, 1);

    kukdh1::decompress(decrypted, decompressed);
    free(decrypted);
    decrypted = decompressed;
  }

  savefile.write((char *)decrypted, file.uiOriginalSize);
  savefile.close();
  free(decrypted);
}

DWORD WINAPI ExtractThread(LPVOID arg) {
  kukdh1::Tree *CTree = (kukdh1::Tree *)arg;
  kukdh1::CryptICE cipher(ICE_KEY, ICE_KEY_LEN);
  WCHAR buffer[64];
  uint32_t uiFiles;
  std::wstring sFolderPath;

  sFolderPath.resize(4096);

  EnableWindow(app.hButtonExctact, FALSE);

  if (kukdh1::BrowseFolder(NULL, app.CSetting.getString(kukdh1::Setting::ID_SELECT_FOLDER_TO_SAVE).c_str(), app.wpszFolderPath, (WCHAR *)sFolderPath.c_str(), 4096)) {
    std::vector<kukdh1::FileInfo> vFileList;

    CTree->GetFileList(vFileList);
    uiFiles = vFileList.size();

    sFolderPath.resize(wcslen(sFolderPath.c_str()));
    if (sFolderPath.back() != L'\\') {
      sFolderPath.push_back(L'\\');
    }

    SendMessage(app.hProgressBar, PBM_SETRANGE32, 0, uiFiles);
    uint32_t i = 1;

    for (auto iter = vFileList.begin(); iter != vFileList.end(); iter++) {
      std::wstring savePath = sFolderPath;
      std::vector<std::string> paths;
      kukdh1::ParsePath(iter->sFullPath, paths);

      for (auto path = paths.begin(); path != paths.end() - 1; path++) {
        std::wstring folder;
        kukdh1::ConvertWidechar(*path, folder);
        savePath.append(folder);
        
        if (!CreateDirectory(savePath.c_str(), NULL)) {
          if (GetLastError() != ERROR_ALREADY_EXISTS) {
            MessageBox(NULL, app.CSetting.getString(kukdh1::Setting::ID_DIRECTORY_CREATE_FAILED).c_str(), app.CSetting.getString(kukdh1::Setting::ID_ERROR).c_str(), MB_OK | MB_ICONERROR);
            return -1;
          }
        }
        savePath.append(L"\\");
      }

      std::wstring file;
      kukdh1::ConvertWidechar(paths.back(), file);
      savePath.append(file);

      wsprintf(buffer, app.CSetting.getString(kukdh1::Setting::ID_PROGRESS_EXTRACT).c_str(), i, uiFiles);
      SendMessage(app.hStatusBar, SB_SETTEXT, 2, (LPARAM)buffer);
      ExtractFile(savePath, *iter, cipher);
      SendMessage(app.hProgressBar, PBM_SETPOS, i++, NULL);
    }
  }

  SendMessage(app.hProgressBar, PBM_SETPOS, 0, NULL);
  SendMessage(app.hStatusBar, SB_SETTEXT, 2, (LPARAM)app.CSetting.getString(kukdh1::Setting::ID_PROGRESS_READY).c_str());

  EnableWindow(app.hButtonExctact, TRUE);

  return 0;
}
