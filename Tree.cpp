#include "Tree.h"

namespace kukdh1 {
  bool TreeCompare(Tree *a, Tree *b) {
    if (a->GetName().compare(b->GetName()) <= 0) {
      return true;
    }

    return false;
  }

  Tree::Tree(TREE_TYPE type) {
    pParent = NULL;
    ttType = type;
    hThis = NULL;
    liCapacity.QuadPart = 0;
  }

  Tree::~Tree() {
    for (auto iter = vChildFiles.begin(); iter != vChildFiles.end(); iter++) {
      delete (*iter);
    }
    for (auto iter = vChildFolders.begin(); iter != vChildFolders.end(); iter++) {
      delete (*iter);
    }
  }

  void Tree::AddToTree(HWND hTree) {
    if (ttType == TREE_TYPE_ROOT) {
      hThis = AddTreeItem(hTree, NULL, TVI_ROOT, L"/", (LPARAM)this);

    }
    else if (pParent != NULL) {
      std::wstring temp;

      ConvertWidechar(sName, temp);
      hThis = AddTreeItem(hTree, pParent->GetHandle(), TVI_LAST, (WCHAR *)temp.c_str(), (LPARAM)this);
    }

    if (ttType != TREE_TYPE_FILE) {
      for (auto iter = vChildFolders.begin(); iter != vChildFolders.end(); iter++) {
        (*iter)->AddToTree(hTree);
      }
      for (auto iter = vChildFiles.begin(); iter != vChildFiles.end(); iter++) {
        (*iter)->AddToTree(hTree);
      }
    }
  }

  void Tree::SetFileInfo(Tree* pParent, std::string &name, FileInfo &fiFile) {
    this->pParent = pParent;
    fiFileInfo = fiFile;
    sName = name;

    ttType = TREE_TYPE_FILE;
  }

  void Tree::SetFolderInfo(Tree* pParent, std::string &name) {
    this->pParent = pParent;
    sName = name;

    ttType = TREE_TYPE_FOLDER;
  }

  void Tree::AddChild(Tree *pChild) {
    std::string insertString = pChild->GetName();
    bool bInserted = false;

    if (pChild->GetType() == TREE_TYPE_FILE) {
      vChildFiles.push_back(pChild);
    }
    else {
      vChildFolders.push_back(pChild);
    }
  }

  void Tree::SortChild() {
    // Can be parallelize
    if (ttType != TREE_TYPE_FILE) {
      for (auto iter = vChildFolders.begin(); iter != vChildFolders.end(); iter++) {
        (*iter)->SortChild();
      }

      std::sort(vChildFolders.begin(), vChildFolders.end(), TreeCompare);
      std::sort(vChildFiles.begin(), vChildFiles.end(), TreeCompare);
    }
  }

  LARGE_INTEGER Tree::UpdateCapacity() {
    liCapacity.QuadPart = 0;

    if (ttType == TREE_TYPE_FILE) {
      liCapacity.QuadPart = fiFileInfo.uiOriginalSize;
    }
    else {
      for (auto iter = vChildFiles.begin(); iter != vChildFiles.end(); iter++) {
        liCapacity.QuadPart += (*iter)->UpdateCapacity().QuadPart;
      }

      for (auto iter = vChildFolders.begin(); iter != vChildFolders.end(); iter++) {
        liCapacity.QuadPart += (*iter)->UpdateCapacity().QuadPart;
      }
    }

    return liCapacity;
  }

  Tree* Tree::GetChildFolderWithName(std::string name) {
    Tree *ptr = NULL;

    for (auto iter = vChildFolders.begin(); iter != vChildFolders.end(); iter++) {
      if ((*iter)->GetName().compare(name) == 0) {
        ptr = (*iter);
        break;
      }
    }

    return ptr;
  }

  Tree::TREE_TYPE Tree::GetType() {
    return ttType;
  }

  FileInfo Tree::GetFileInfo() {
    return fiFileInfo;
  }

  HTREEITEM Tree::GetHandle() {
    return hThis;
  }

  std::string Tree::GetName() {
    return sName;
  }

  LARGE_INTEGER Tree::GetCapacity() {
    return liCapacity;
  }

  size_t Tree::GetFileCount() {
    return vChildFiles.size();
  }

  size_t Tree::GetFolderCount() {
    return vChildFolders.size();
  }
}
