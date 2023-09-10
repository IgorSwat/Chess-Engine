#pragma once

#include <string>

namespace FenParsing {
	bool parseFenToConfig(const std::string& FEN, BoardConfig* config);
}
