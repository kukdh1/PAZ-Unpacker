#pragma once

#ifndef _SETTING_H_
#define _SETTING_H_

#include <Windows.h>
#include <string>
#include "xml/pugixml.hpp"

#define DEFAULT_SETTING_FILENAME    L"Setting.xml"
#define ROOT_NODE_NAME              L"setting"
#define LANG_NODE_NAME              L"language"

#define SETTING_LAST_FOLDER         L"last_folder_path"

class Setting {
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
};

#endif
