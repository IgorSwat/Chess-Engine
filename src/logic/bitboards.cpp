#include "bitboards.h"
#include <sstream>

namespace Bitboards {
	std::string bitboardToString(Bitboard bb)
	{
		int bb2D[8][8];
		int i = 7, j = 0;
		for (int k = 0; k < 64; k++)
		{
			bb2D[i][j] = bb % 2;
			bb >>= 1;
			j++;
			if (j == 8)
			{
				i--;
				j = 0;
			}
		}
		std::ostringstream stream;
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 8; j++)
				stream << bb2D[i][j];
			if (i != 7)
				stream << std::endl;
		}
		return stream.str();
	}
}