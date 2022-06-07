#include "encryption.h"

string encryption::sha1(const string& strRaw){
    string str(strRaw);
    str+=(unsigned char)(0x80);

    // 每个字节8位,所以要乘8,左移3位
    while (str.size()<<3 % 512 != 448){
        str+=(char)0;
    }
 
    // 写入原始数据长度
    for (int i(56);i>=0;i-=8){
        str+=(unsigned char)((strRaw.size()<<3) >> i);
    }
 
    const unsigned K[4]={0x5a827999,0x6ed9eba1,0x8f1bbcdc,0xca62c1d6};
    unsigned A(0x67452301),B(0xefcdab89),C(0x98badcfe),D(0x10325476),E(0xc3d2e1f0),T(0);
    unsigned W[80]={0};
 
    // 每次处理64字节,共512位
    for (unsigned i(0);i!=str.size();i+=64){
        // 前16个字为原始数据
        for (unsigned t(0);t!=16;++t){
            // 将4个8位数据放入一个32位变量中
            W[t]=((unsigned)str[i+4*t] & 0xff)<<24 |
                 ((unsigned)str[i+4*t+1] & 0xff)<<16 |
                 ((unsigned)str[i+4*t+2] & 0xff)<<8 |
                 ((unsigned)str[i+4*t+3] & 0xff);
        }
 
        // 填充
        for (unsigned t(16);t!=80;++t){
            W[t]=encryption::circleShift(W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16],1);
        }
 
        for (unsigned t(0);t!=80;++t){
            T=encryption::circleShift(A,5) + encryption::sha1Fun(B,C,D,t) + E + W[t] + K[t/20];
            E=D; 
            D=C; 
            C=encryption::circleShift(B,30); 
            B=A; 
            A=T;
        }
 
        A+=0x67452301;
        B+=0xefcdab89;
        C+=0x98badcfe;
        D+=0x10325476;
        E+=0xc3d2e1f0;
    }
 
    stringstream ss;
    ss<<setw(8)<<setfill('0')<<hex<<A<<B<<C<<D<<E;
    ss>>str;
 
    return str;
}

