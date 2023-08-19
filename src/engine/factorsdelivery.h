#pragma once

#include "../logic/misc.h"
#include <map>
#include <string>
#include <vector>

using FactorsMap = std::map<std::string, std::vector<Factor> >;
using FactorsVec = std::vector<Factor>;

namespace FactorsDelivery 
{
	static int sideOnMoveValue[2] { 26, -26 };
	FactorsMap getFactors();
}