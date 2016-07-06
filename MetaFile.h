#pragma once

#ifndef _META_FILE_H_
#define _META_FILE_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace kukdh1 {
  struct PAZTable {
    uint32_t uiPazFileID;
    uint32_t uiCRC;             // CRC of PAZ (same as first 4bytes of PAZ file)
    uint32_t uiSize;            // PAZ File Size
		
		PAZTable(uint8_t *buffer);
  };

  struct Meta {
    std::vector<PAZTable> vPAZs;

    uint32_t uiVersion;
    uint32_t uiPAZFileCount;

    Meta(wchar_t *wpszPazFolder);
  };
}

#endif
