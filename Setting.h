#pragma once

#ifndef _SETTING_H_
#define _SETTING_H_

#include <Windows.h>
#include <string>
#include <vector>
#include "xml/pugixml.hpp"

#define DEFAULT_SETTING_FILENAME    L"Setting.xml"
#define ROOT_NODE_NAME              L"setting"
#define LANG_NODE_NAME              L"language"

#define SETTING_LAST_FOLDER         L"last_folder_path"

namespace kukdh1 {
  class Setting {
    public:
      enum LANG_ID {
        ID_LANG_BEGIN,
        ID_CAPTION = ID_LANG_BEGIN,
        ID_CAPTION_WITH_PATH,
        ID_STATUS_IDLE,
        ID_STATUS_BUSY,
        ID_SELECT_FOLDER_TO_OPEN,
        ID_SELECT_FOLDER_TO_SAVE,
        ID_NO_META_FILE_EXISTS,
        ID_ALERT,
        ID_ERROR,
        ID_META_FILE_INFO,
        ID_INTERNAL_FILE_INFO,
        ID_INTERNAL_FOLDER_INFO,
        ID_PROGRESS_READING,
        ID_PROGRESS_SORTING,
        ID_PROGRESS_CAPACITY,
        ID_PROGRESS_ADDING,
        ID_PROGRESS_EXTRACT,
        ID_PROGRESS_READY,
        ID_DIRECTORY_CREATE_FAILED,
        ID_OPEN,
        ID_EXTRACT,
        ID_LANG_END
      };
      std::vector<std::wstring> pDefaultLanguage;

    private:
      std::wstring filepath;

      pugi::xml_document setting;

    public:
      Setting();
      ~Setting();
      bool setData(std::wstring name, std::wstring value);
      bool setData(std::wstring name, int value);
      void getData(std::wstring name, std::wstring &value, std::wstring default = L"");
      void getData(std::wstring name, int &value, int default = 0);
      std::wstring getString(LANG_ID id);
  };
}

#endif
