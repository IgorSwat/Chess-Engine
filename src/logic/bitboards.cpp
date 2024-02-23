#include "bitboards.h"
#include <sstream>


namespace Bitboards {

	Bitboard shift_d(Bitboard bb, Direction dir)
	{
		return dir == NORTH ? shift_s<NORTH>(bb) :
			dir == SOUTH ? shift_s<SOUTH>(bb) :
			dir == EAST ? shift_s<EAST>(bb) :
			dir == WEST ? shift_s<WEST>(bb) :
			dir == NORTH_EAST ? shift_s<NORTH_EAST>(bb) :
			dir == NORTH_WEST ? shift_s<NORTH_WEST>(bb) :
			dir == SOUTH_EAST ? shift_s<SOUTH_EAST>(bb) : shift_s<SOUTH_WEST>(bb);
	}

	std::string to_string(Bitboard bb)
	{
		std::ostringstream stream;
		for (int i = 0; i < 8; i++) {
			for (int j = 7; j >= 0; j--) {
				int offset = i * 8 + j;
				Bitboard shifted = bb >> (63 - offset);
				stream << (shifted % 2) << " ";
			}
			stream << std::endl;
		}
		return stream.str();
	}

}