#ifndef DECOMPRESSOR_H_INCLUDED
#define DECOMPRESSOR_H_INCLUDED

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <bitset>
#include <cstring>
#include "md5.h"

#ifndef ull
#define ull unsigned long long
#endif // ull

using namespace std;
class Decompressor {
    map<unsigned char, string> codeMap;
    struct haffNode {
        ull freq;
        unsigned char uchar;
        string code;
        struct haffNode* left = 0;
        struct haffNode* right = 0;
    };
    struct cmp {
        bool operator ()(const haffNode* a, const haffNode* b) {
            return a->freq > b->freq;
        }
    };
    void insert_node(haffNode* father, unsigned char uchar, string code) {
        if (code.empty()) {
            father->uchar = uchar;
            return;
        }
        char way = code[0];
        if (way == '0') {
            if (!(father->left)) {
                haffNode* son = new (haffNode);
                father->left = son;
            }
            insert_node(father->left, uchar, code.substr(1));
        } else {
            if (!(father->right)) {
                haffNode* son = new (haffNode);
                father->right = son;
            }
            insert_node(father->right, uchar, code.substr(1));
        }
    }
  public:
    /** 返回值说明：
    0：正常执行
    1：源文件扩展名不是bak
    2：打开源文件失败
    3：打开目标文件失败
    4：文件过短，频率表不完整
    5：文件结尾不完整
    6：密码错误
    **/
    int decompress(string sourcePath, string destinationPath, string pw = "") {
        /**打开源文件**/
        if (sourcePath.substr(sourcePath.find_last_of(".") + 1) != "bak")
            return 1; // 源文件扩展名不是bak
        ifstream inFile;
        inFile.open(sourcePath, ios::in | ios::binary);
        if (!inFile)
            return 2; // 打开源文件失败
        /**密码校验**/
        unsigned char uchar;
        inFile.read((char*)&uchar, sizeof(char));
        int zeroNum = uchar;
        if (zeroNum >= 8) {
            zeroNum -= 8;
            char crMD5_c[33];
            inFile.get(crMD5_c, 33);
            string crMD5 = string(crMD5_c, 32);
            //cout<<"d:crMD5="<<crMD5<<endl;
            string pwMD5 = getMD5(pw);
            //cout<<"d:pwMD5="<<pwMD5<<endl;
            if (crMD5 != pwMD5) {
                inFile.close();
                return 6; // 密码错误
            }
        }
        /**打开目标文件**/
        ofstream outFile;
        string newFileName = destinationPath + sourcePath.substr(0, sourcePath.find_last_of(".")) + ".tar";
        outFile.open(newFileName, ios::out | ios::binary);
        if (!outFile) {
            inFile.close();
            return 3; // 打开目标文件失败
        }
        /**读出频率表**/
        ull freq;
        map<unsigned char, ull> freqMap;
        int i = 0;
        for (i = 0; i < 256; i++) {
            inFile.read((char*)&freq, sizeof(freq));
            if (freq) {
                freqMap[i] = freq;
            }
        }
        if (i != 256)
            return 4; // 文件过短，频率表不完整
        /**建立词频小顶堆**/
        priority_queue<haffNode*, vector<haffNode*>, cmp> freqHeap;
        map<unsigned char, ull>::reverse_iterator it;
        for (it = freqMap.rbegin(); it != freqMap.rend(); it++) {
            haffNode* pn = new (haffNode);
            pn->freq = it->second;
            pn->uchar = it->first;
            pn->left = pn->right = 0;
            freqHeap.push(pn);
        }
        /**构建哈夫曼树**/
        while (freqHeap.size() > 1) {
            haffNode* pn1 = freqHeap.top();
            freqHeap.pop();
            haffNode* pn2 = freqHeap.top();
            freqHeap.pop();
            haffNode* pn = new (haffNode);
            pn->freq = pn1->freq + pn2->freq;
            pn->left = pn1;
            pn->right = pn2;
            freqHeap.push(pn);
        }
        haffNode* root = freqHeap.top();
        codeMap.clear();
        /**读出主体，用哈夫曼树树解码**/
        haffNode* decodePointer = root;
        string buf, now;
        inFile.read((char*)&uchar, sizeof(unsigned char));
        bitset<8> bs = uchar;
        buf = bs.to_string();
        while (inFile.read((char*)&uchar, sizeof(unsigned char))) {
            bitset<8> bs = uchar;
            now = buf;
            buf = bs.to_string();
            for (char i = 0; i < 8; i++) {
                if (now[i] == '0') {
                    decodePointer = decodePointer->left;
                } else {
                    decodePointer = decodePointer->right;
                }
                if (!(decodePointer->left || decodePointer->right)) {
                    //cout<<int(decodePointer->uchar)<<" ";
                    outFile.write((char*) & (decodePointer->uchar), sizeof(unsigned char));
                    decodePointer = root;
                }
            }
        }
        now = buf;
        for (char i = 0; i < (8 - zeroNum) % 8; i++) {
            if (now[i] == '0') {
                decodePointer = decodePointer->left;
            } else {
                decodePointer = decodePointer->right;
            }
            if (!(decodePointer->left || decodePointer->right)) {
                //cout<<int(decodePointer->uchar)<<" ";
                outFile.write((char*) & (decodePointer->uchar), sizeof(unsigned char));
                decodePointer = root;
            }
        }

        inFile.close();
        outFile.close();
        if (decodePointer == root) return 0; // 正常执行
        return 5; // 文件结尾不完整
    }
};
#endif // DECOMPRESSOR_H_INCLUDED
