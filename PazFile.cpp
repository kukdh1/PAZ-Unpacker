#include "PazFile.h"

namespace kukdh1 {
	FileInfo::FileInfo(uint8_t *buffer) {
		memcpy(&uiCRC, buffer + 0, 4);
		memcpy(&uiFolderID, buffer + 0, 4);
		memcpy(&uiFileID, buffer + 0, 4);
		memcpy(&uiOffset, buffer + 0, 4);
		memcpy(&uiCompressedSize, buffer + 0, 4);
		memcpy(&uiOriginalSize, buffer + 0, 4);
	}

	PazFile::PazFile(wchar_t *wpszPazFolder, uint32_t uiPazIndex, CryptICE &cipher) {
		std::wstring path(wpszPazFolder);
		std::wstringstream wss(path);

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
			for (uint32_t i = 0; i < uiFileCount; i++) {
				file.read((char *)buffer, 24);
				vFileInfo.push_back(FileInfo(buffer));
			}

			// Read encrypted file names
			uint8_t *encrypted;
			uint8_t *decrypted;

			encrypted = (uint8_t *)calloc(uiPathLength, 1);
			file.read((char *)encrypted, uiPathLength);

			// Decrypt
			cipher.decrypt(encrypted, uiPathLength, &decrypted, &uiPathLength);

			// Parse
			std::stringstream ss((char *)decrypted);

			for (auto iter = vFileInfo.begin(); iter != vFileInfo.end(); iter++) {
				std::getline(ss, iter->sFullPath, '\0');
			}

			// Cleanup
			free(encrypted);
			free(decrypted);
		}
	}
}
