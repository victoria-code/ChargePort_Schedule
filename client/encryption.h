/*ʵ���������ӳ��*/
#include "main.h"

//�������
class encryption {
public:
    unsigned circleShift(const unsigned& word, const int bits);
    unsigned sha1Fun(const unsigned& B, const unsigned& C, const unsigned& D, const unsigned& t);
    string sha1(const string& strRaw);
};
