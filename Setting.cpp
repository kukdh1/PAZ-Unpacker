#include "Setting.h"

namespace kukdh1 {
  Setting::Setting() {
    DWORD dwLength;

    pDefaultLanguage = {
      { L"PAZ Unpacker" },
      { L"PAZ Unpacker - %s" },
      { L"Idle" },
      { L"Busy" },
      { L"Select paz folder" },
      { L"Select folder to save file(s)" },
      { L"pad00000.meta doesn't exists" },
      { L"Alert" },
      { L"Error" },
      { L"Version: %d\r\nPAZ file no.: %d\r\nSize: %s" },
      { L"Name: %S\r\nSize: %s\r\nPaz file: %s\r\nPath: %S" },
      { L"Name: %S\r\nSize: %s" },
      { L"(1/4) Processing %d/%d..." },
      { L"(2/4) Sorting..." },
      { L"(3/4) Calculate capacities..." },
      { L"(4/4) Adding data to tree..." },
      { L"(%d/%d) Extracting..." },
      { L"" },
      { L"Fail to create directory" },
      { L"Open" },
      { L"Extract" }
    };

    // Get current directory
    dwLength = GetCurrentDirectory(0, NULL);

    filepath.resize(dwLength);
    GetCurrentDirectory(dwLength, (wchar_t *)filepath.c_str());

    // Delete null-terminator
    filepath.pop_back();

    // Make path of xml file
    if (filepath.back() != L'\\')
      filepath.append(L"\\");

    filepath.append(DEFAULT_SETTING_FILENAME);

    // Open file
    setting.load_file(filepath.c_str());

    // Check existance of setting node
    if (!setting.child(ROOT_NODE_NAME)) {
      setting.append_child(ROOT_NODE_NAME);
    }
    if (!setting.child(LANG_NODE_NAME)) {
      pugi::xml_node node = setting.append_child(LANG_NODE_NAME);

      for (uint32_t i = ID_LANG_BEGIN; i < ID_LANG_END; i++) {
        pugi::xml_node string = node.append_child(std::to_wstring(i).c_str()); 

        if (string) {
          string.text().set(pDefaultLanguage.at(i).c_str());
        }
      }
    }
    else {
      pugi::xml_node node = setting.child(LANG_NODE_NAME);

      for (uint32_t i = ID_LANG_BEGIN; i < ID_LANG_END; i++) {
        pugi::xml_node string = node.child(std::to_wstring(i).c_str());

        if (string) {
          pDefaultLanguage.at(i) = string.child_value();
        }
        else {
          string = node.append_child(std::to_wstring(i).c_str());

          if (string) {
            string.text().set(pDefaultLanguage.at(i).c_str());
          }
        }
      }
    }
  }

  Setting::~Setting() {
    // Save settings
    setting.save_file(filepath.c_str());
  }

  bool Setting::setData(std::wstring name, std::wstring value) {
    pugi::xml_node node = setting.child(ROOT_NODE_NAME).child(name.c_str());

    if (!node) {
      // Create node
      node = setting.child(ROOT_NODE_NAME).append_child(name.c_str());

      if (!node) {
        return false;
      }
    }

    node.text().set(value.c_str());

    return true;
  }

  bool Setting::setData(std::wstring name, int value) {
    return setData(name, std::to_wstring(value));
  }

  void Setting::getData(std::wstring name, std::wstring &value, std::wstring default) {
    pugi::xml_node node = setting.child(ROOT_NODE_NAME).child(name.c_str());

    if (node) {
      value = node.child_value();
    }
    else {
      setData(name, default);
      value = default;
    }
  }

  void Setting::getData(std::wstring name, int &value, int default) {
    std::wstring temp;

    getData(name, temp, std::to_wstring(default));
    value = _wtoi(temp.c_str());
  }

  std::wstring Setting::getString(LANG_ID id) {
    if (id >= ID_LANG_END) {
      return std::wstring();
    }

    return pDefaultLanguage.at(id);
  }
}
