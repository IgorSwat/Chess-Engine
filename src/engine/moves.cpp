#include "moves.h"
#include <iomanip>


namespace Moves {

    // -----------------------
    // Move methods - printing
    // -----------------------

    std::ostream& operator<<(std::ostream& os, const Move& move)
    {
        os << std::dec;
        os << "Move (from: " << move.from() << ", to: " << move.to();
        os << std::hex;
        os << ", flags: " << move.flags() << ")";
        os << std::dec;

        return os;
    }
}