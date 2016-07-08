#pragma once

#ifndef _MAIN_H_
#define _MAIN_H_

#include <Windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include <Uxtheme.h>

#include "Helper.h"
#include "Tree.h"
#include "MetaFile.h"
#include "PazFile.h"
#include "Setting.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define WINDOW_MIN_WIDTH          1000
#define WINDOW_MIN_HEIGHT         700
#define DIVIDE_RATIO              0.7f
#define STATUSBAR_SECTION_COUNT   3
#define STATUSBAR_SECTION1        70
#define STATUSBAR_SECTION2        530
#define BUTTON_WIDTH              50
#define BUTTON_HEIGHT             25

#define FONT_SIZE                 17
#define FONT_FACE                 L"Segoe UI"

#define ID_BUTTON_OPEN            0
#define ID_BUTTON_EXTRACT         1
#define ID_TREE_FILESYSTEM        10
#define ID_STATIC                 11
#define ID_STATUSBAR              20

#define ICE_KEY                   ((uint8_t *)"\x51\xF3\x0F\x11\x04\x24\x6A\x00")
#define ICE_KEY_LEN               8

#define SAFE_DELETE(ptr)          { if (ptr) { delete ptr; ptr = NULL; } }
#define SAFE_FREE(ptr)            { if (ptr) { free(ptr); ptr = NULL; } }

typedef struct _AppData {
  HWND hButtonOpen;
  HWND hButtonExctact;
  HWND hTreeFileSystem;
  HWND hStatusBar;
  HWND hStaticInfo;
  HWND hProgressBar;

  kukdh1::Tree *CTree;
  kukdh1::Meta *CMeta;
  kukdh1::Setting CSetting;

  WCHAR *wpszFolderPath;
  HFONT hFont;

  _AppData() :
    CTree(NULL),
    CMeta(NULL),
    wpszFolderPath(NULL) {}
  ~_AppData() {
    SAFE_DELETE(CTree);
    SAFE_DELETE(CMeta);
    SAFE_FREE(wpszFolderPath);
  }
} AppData;

#endif
