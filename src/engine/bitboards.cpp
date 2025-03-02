#include "bitboards.h"
#include <algorithm>
#include <bitset>
#include <iterator>
#include <sstream>


namespace Bitboards {

    std::string to_string(Bitboard bb)
    {
        std::ostringstream stream;
		std::ostream_iterator<char> out_it(stream, " ");

        for (int i = 0; i < 8; i++) {
			std::string rank = std::bitset<8>((bb >> (7 - i) * 8) & 0xff).to_string();
			std::reverse(rank.begin(), rank.end());         // Reverse bits to obtain correct mapping to chessboard
            std::copy(rank.begin(), rank.end(), out_it);
			stream << "\n";
		}

        return stream.str();
    }

}