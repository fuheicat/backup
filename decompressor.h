#ifndef _DECOMPRESSOR_H
#define _DECOMPRESSOR_H

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <bitset>

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
		bool operator ()(const haffNode *a, const haffNode *b) {
			return a->freq > b->freq;
		}
	};
	void insert_node(haffNode*father, unsigned char uchar, string code) {
		if (code.empty()) {
			father->uchar = uchar;
			return;
		}
		char way = code[0];
		if (way == '0') {
			if (!(father->left)) {
				haffNode*son = new(haffNode);
				father->left = son;
			}
			insert_node(father->left, uchar, code.substr(1));
		}
		else {
			if (!(father->right)) {
				haffNode*son = new(haffNode);
				father->right = son;
			}
			insert_node(father->right, uchar, code.substr(1));
		}
	}
	/*void dfs(haffNode*pn,string code){
	if(pn->left) dfs(pn->left,code+"0");
	if(pn->right) dfs(pn->right,code+"1");
	if(!(pn->left||pn->right)){
	//cout<<int(pn->uchar)<<"  "<<code<<endl;
	codeMap[pn->uchar]=code;
	}
	}*/
public:
	/** 返回值说明：
	0：正常执行
	1：源文件扩展名不是bak
	2：打开源文件失败
	3：打开目标文件失败
	4：文件过短，频率表不完整
	5：文件结尾不完整
	**/
	int decompress(string sourcePath, string destinationPath) {
		if (sourcePath.substr(sourcePath.find_last_of(".") + 1) != "bak")
			return 1; // 源文件扩展名不是bak
		ifstream inFile;
		inFile.open(sourcePath, ios::in | ios::binary);
		if (!inFile)
			return 2; // 打开源文件失败
		ofstream outFile;
		string newFileName = destinationPath + sourcePath.substr(0, sourcePath.find_last_of(".")) + ".tar";
		outFile.open(newFileName, ios::out | ios::binary);
		if (!outFile) {
			inFile.close();
			return 3; // 打开目标文件失败
		}
		/**读出频率表**/
		unsigned char uchar;
		inFile.read((char *)&uchar, sizeof(char));
		int zeroNum = uchar;
		ull freq;
		map<unsigned char, ull> freqMap;
		int i = 0;
		for (i = 0; i<256; i++) {
			inFile.read((char *)&freq, sizeof(freq));
			if (freq) {
				freqMap[i] = freq;
			}
		}
		if (i != 256)
			return 4; // 文件过短，频率表不完整
					  //dfs(root, "");
					  /*for(int i=0;i<256;i++){
					  unsigned char uc=i;
					  string codeString="1"+codeMap[uc];
					  short codeShort=string_to_short(codeString);
					  cout<<i<<" "<<codeMap[uc]<<endl;
					  outFile.write((char*)&codeShort, sizeof(codeShort));
					  }*/
					  /**建立词频小顶堆**/
		priority_queue<haffNode*, vector<haffNode*>, cmp> freqHeap;
		map<unsigned char, ull>::reverse_iterator it;
		for (it = freqMap.rbegin(); it != freqMap.rend(); it++) {
			haffNode*pn = new(haffNode);
			pn->freq = it->second;
			pn->uchar = it->first;
			pn->left = pn->right = 0;
			freqHeap.push(pn);
		}
		/**构建哈夫曼树**/
		while (freqHeap.size()>1) {
			haffNode*pn1 = freqHeap.top();
			freqHeap.pop();
			haffNode*pn2 = freqHeap.top();
			freqHeap.pop();
			haffNode*pn = new(haffNode);
			pn->freq = pn1->freq + pn2->freq;
			pn->left = pn1;
			pn->right = pn2;
			freqHeap.push(pn);
		}
		haffNode*root = freqHeap.top();
		codeMap.clear();
		/**读出主体，用哈夫曼树树解码**/
		haffNode*decodePointer = root;
		string buf, now;
		inFile.read((char *)&uchar, sizeof(unsigned char));
		bitset<8> bs = uchar;
		buf = bs.to_string();
		while (inFile.read((char *)&uchar, sizeof(unsigned char))) {
			bitset<8> bs = uchar;
			now = buf;
			buf = bs.to_string();
			for (char i = 0; i<8; i++) {
				if (now[i] == '0') {
					decodePointer = decodePointer->left;
				}
				else {
					decodePointer = decodePointer->right;
				}
				if (!(decodePointer->left || decodePointer->right)) {
					//cout<<int(decodePointer->uchar)<<" ";
					outFile.write((char *)&(decodePointer->uchar), sizeof(unsigned char));
					decodePointer = root;
				}
			}
		}
		now = buf;
		for (char i = 0; i<(8 - zeroNum) % 8; i++) {
			if (now[i] == '0') {
				decodePointer = decodePointer->left;
			}
			else {
				decodePointer = decodePointer->right;
			}
			if (!(decodePointer->left || decodePointer->right)) {
				//cout<<int(decodePointer->uchar)<<" ";
				outFile.write((char *)&(decodePointer->uchar), sizeof(unsigned char));
				decodePointer = root;
			}
		}

		inFile.close();
		outFile.close();
		if (decodePointer == root) return 0; // 正常执行
		return 5; // 文件结尾不完整
	}
};
#endif // _DECOMPRESSOR_H
