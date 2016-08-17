#pragma once

#ifndef _TREE_H_
#define _TREE_H_

#include <Windows.h>
#include <CommCtrl.h>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>

#include "PazFile.h"
#include "Helper.h"

namespace kukdh1 {
  class Tree {
    public:
      enum TREE_TYPE {
        TREE_TYPE_ROOT,
        TREE_TYPE_FOLDER,
        TREE_TYPE_FILE
      };

    private:
      Tree *pParent;
      std::vector<Tree *> vChildFiles;
      std::vector<Tree *> vChildFolders;
      std::string sName;

      TREE_TYPE ttType;
      HTREEITEM hThis;
      BOOL bAdded;

      FileInfo fiFileInfo;
      LARGE_INTEGER liCapacity;

    public:
      Tree(TREE_TYPE type);
      ~Tree();

      void AddToTree(HWND hTree);
      void AddChildsToTree(HWND hTree);
      void AddGrandchildsToTree(HWND hTree, LPVOID arg, std::function<void(LPVOID, size_t, size_t)> callback);
      void SetFileInfo(Tree* pParent, std::string &name, FileInfo &fiFile);
      void SetFolderInfo(Tree* pParent, std::string &name);
      void AddChild(Tree *pChild);
      void SortChild();
      LARGE_INTEGER UpdateCapacity();

      TREE_TYPE GetType();
      FileInfo GetFileInfo();
      HTREEITEM GetHandle();
      std::string GetName();
      LARGE_INTEGER GetCapacity();
      size_t GetFileCount();
      size_t GetFolderCount();
      Tree *GetChildFolderWithName(std::string name);
      void GetFileList(std::vector<kukdh1::FileInfo> &vList);
  };
}

#endif
