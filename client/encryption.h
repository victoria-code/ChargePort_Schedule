/*ÊµÏÖÃÜÂë¼ÓÃÜÓ³Éä*/
#include "main.h"

//ÃÜÂë¼ÓÃÜ
class encryption {
public:
    unsigned circleShift(const unsigned& word, const int bits);
    unsigned sha1Fun(const unsigned& B, const unsigned& C, const unsigned& D, const unsigned& t);
    string sha1(const string& strRaw);
};
