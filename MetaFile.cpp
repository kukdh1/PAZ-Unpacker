#include "MetaFile.h"

namespace kukdh1 {
	PAZTable::PAZTable(uint8_t *buffer) {
		memcpy(&uiPazFileID, buffer + 0, 4);
		memcpy(&uiCRC, buffer + 4, 4);
		memcpy(&uiSize, buffer + 8, 4);
	}

  Meta::Meta(wchar_t *wpszPazFolder) {
    std::fstream file;
		std::wstring path(wpszPazFolder);

		path.append(L"\\pad00000.meta");

    file.open(path, std::ios::in | std::ios::binary);
    if (file.is_open()) {
      uint8_t buffer[64];

      // Read Header
      file.read((char *)buffer, 8);
			memcpy(&uiVersion, buffer + 0, 4);
			memcpy(&uiPAZFileCount, buffer + 4, 4);

      // Read PAZ File information
      for (uint32_t idx = 0; idx < uiPAZFileCount; idx++) {
        file.read((char *)buffer, 12);

        vPAZs.push_back(PAZTable(buffer));
      }

      file.close();
    }
    else {
      throw std::exception();
    }
  }
}
