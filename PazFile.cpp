#include "PazFile.h"

namespace kukdh1 {
	FileInfo::FileInfo(uint8_t *buffer) {
		memcpy(&uiCRC, buffer + 0, 4);
		memcpy(&uiFolderID, buffer + 4, 4);
		memcpy(&uiFileID, buffer + 8, 4);
		memcpy(&uiOffset, buffer + 12, 4);
		memcpy(&uiCompressedSize, buffer + 16, 4);
		memcpy(&uiOriginalSize, buffer + 20, 4);
	}

  FileInfo::FileInfo() {
    // DO NOTHING
  }

	PazFile::PazFile(wchar_t *wpszPazFolder, uint32_t uiPazIndex, CryptICE &cipher) {
		std::wstring path(wpszPazFolder);
		std::wstringstream wss(path, std::ios_base::out | std::ios_base::in | std::ios_base::ate);

		wss << L"\\PAD" << std::setw(5) << std::setfill(L'0') << uiPazIndex << L".PAZ";

		path = wss.str();

		std::fstream file;

		file.open(path, std::ios::binary | std::ios::in);
		if (file.is_open()) {
			uint8_t buffer[64];
			uint32_t uiFileCount;
			uint32_t uiPathLength;

			// Read Header
			file.read((char *)buffer, 12);
			memcpy(&uiCRC, buffer + 0, 4);
			memcpy(&uiFileCount, buffer + 4, 4);
			memcpy(&uiPathLength, buffer + 8, 4);

			// Read file info
      uint8_t *infos = (uint8_t *)calloc(uiFileCount * 24, 1);
      file.read((char *)infos, uiFileCount * 24);

			for (uint32_t i = 0; i < uiFileCount; i++) {
				vFileInfo.push_back(FileInfo(infos + i * 24));
        vFileInfo.back().wsPazFullPath = path;
			}

      free(infos);

			// Read encrypted file names
			uint8_t *encrypted;
			uint8_t *decrypted;

			encrypted = (uint8_t *)calloc(uiPathLength, 1);
			file.read((char *)encrypted, uiPathLength);

			// Decrypt
			cipher.decrypt(encrypted, uiPathLength, &decrypted, &uiPathLength);

			// Parse
      std::vector<std::string> paths;
      char *ptr = (char *)decrypted;

      for (; ptr < (char *)decrypted + uiPathLength; ) {
        paths.push_back(ptr);
        ptr += paths.back().length() + 1;
      }

      //* can be parallelize
			for (auto iter = vFileInfo.begin(); iter != vFileInfo.end(); iter++) {
        iter->sFullPath = paths.at(iter->uiFolderID);
        iter->sFullPath.append(paths.at(iter->uiFileID));
			}

			// Cleanup
			free(encrypted);
			free(decrypted);
		}
	}
}
