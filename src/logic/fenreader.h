#ifndef FENREADER_H
#define FENREADER_H

#include <string>
using std::string;

class BoardConfig;

class FenReader
{
private:
    string FEN;
    enum class ParsingMode {POSITION, CASTLING};
public:
    FenReader() {}
    FenReader(const string& fen) : FEN(fen) {}
    void setFEN(const string& fen) {this->FEN = fen;}
    string getFEN() const {return FEN;}
    bool parseToConfig(BoardConfig* config) const;
};

#endif // FENREADER_H
