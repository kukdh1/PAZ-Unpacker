#include "Setting.h"

Setting::Setting() {
  DWORD dwLength;

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
