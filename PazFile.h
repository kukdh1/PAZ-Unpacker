#pragma once

#ifndef _PAZ_FILE_H_
#define _PAZ_FILE_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>

#include "Crypt.h"

namespace kukdh1 {
	struct FileInfo {
		uint32_t uiCRC;
		uint32_t uiFolderID;
		uint32_t uiFileID;
		uint32_t uiOffset;
		uint32_t uiCompressedSize;
		uint32_t uiOriginalSize;

		std::string sFullPath;

		FileInfo(uint8_t *buffer);
	};

	struct PazFile {
		uint32_t uiCRC;
		std::vector<FileInfo> vFileInfo;

		PazFile(wchar_t *wpszPazFolder, uint32_t uiPazIndex, CryptICE &cipher);
	};
}

#endif
