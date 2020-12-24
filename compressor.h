#ifndef COMPRESSOR_H_INCLUDED
#define COMPRESSOR_H_INCLUDED

#include <iostream>
#include <fstream>
#include <map>
#include <queue>
#include <vector>
#include <bitset>
#include "md5.h"

#ifndef ull
#define ull unsigned long long
#endif // ull

using namespace std;
class Compressor {
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
	void dfs(haffNode* pn, string code) {
		pn->code = code;
		if (pn->left) dfs(pn->left, code + "0");
		if (pn->right) dfs(pn->right, code + "1");
		if (!pn->left && !pn->right) {
			codeMap[pn->uchar] = code;
		}
	}
public:
	/** 返回值说明：
	0：正常执行
	1：源文件扩展名不是tar
	2：打开源文件失败
	3：打开目标文件失败
    **/
	int compress(string sourcePath, string destinationPath, string pw="") {
		codeMap.clear();
		if (sourcePath.substr(sourcePath.find_last_of(".") + 1) != "tar")
			return 1; // 源文件扩展名不是tar
		ifstream inFile;
		inFile.open(sourcePath, ios::in | ios::binary);
		if (!inFile)
			return 2; // 打开源文件失败
		ofstream outFile;
		string newFileName = destinationPath + sourcePath.substr(0, sourcePath.find_last_of(".")) + ".bak";
		outFile.open(newFileName, ios::out | ios::binary);
		if (!outFile) {
			inFile.close();
			return 3; // 打开目标文件失败
		}
		/**统计词频**/
		unsigned char uchar;
        map<unsigned char, ull> freqMap;
		while (inFile.read((char *)&uchar, sizeof(char))) {
			freqMap[uchar]++;
		}
		/*for(int i=0;i<256;i++){
		printf("%d %d\n",i,freqMap[i]);
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
		/**用哈夫曼树编码**/
		dfs(root, "");
		//cout << endl << codeMap.size() << endl;
		/**写入压缩文件头部：补零数+密码标志（暂时留空）**/
		const unsigned char zeroUC = 0;
		outFile.write((char*)&zeroUC, sizeof(zeroUC));
		/**写入压缩文件头部：密码**/
		if(!pw.empty()){
            string pwMD5=getMD5(pw).c_str();
            cout<<"c:pwMD5="<<pwMD5<<endl;
            //outFile.write((char*)&pwMD5, 16);
            outFile<<pwMD5;
		}
		/**写入压缩文件头部：频率表**/
		const ull zeroULL = 0;
		for (int i = 0; i<256; i++) {
			if (freqMap.count(i) == 0) {
				outFile.write((char*)&zeroULL, sizeof(zeroULL));
			}
			else {
				ull freq = freqMap[i];
				outFile.write((char*)&freq, sizeof(freq));
			}
		}
		//outFile.write((char*)&bi, sizeof(bi));
		/**写入压缩文件主体**/
		{
			inFile.clear();
			inFile.seekg(0);
			string buf;
			unsigned char uchar;
			while (inFile.read((char *)&uchar, sizeof(uchar))) {
				buf += codeMap[uchar];
				//printf("当前buf长：%d\n", buf.length());
				while (buf.length() >= 8) {
					bitset<8> bs(buf.substr(0, 8));
					uchar = bs.to_ulong();
					outFile.write((char *)&uchar, sizeof(uchar));
					buf = buf.substr(8);
				}
			}
			// 末尾处理
			int zeroNum = 8 - buf.length();
			if (zeroNum) {
				for (int i = 0; i<zeroNum; i++) {
					buf += "0";
				}
				bitset<8> bs(buf.substr(0, 8));
				uchar = bs.to_ulong();
				outFile.write((char *)&uchar, sizeof(uchar));
			}
			/**写入头部预留的补零数+密码标志字段**/
			outFile.clear();
			outFile.seekp(0);
			if(!pw.empty()){
                zeroNum+=8;
			}
			uchar = zeroNum;
			//cout<<endl<<"补零数："<<zeroNum<<endl;
			outFile.write((char *)&uchar, sizeof(uchar));
		}
		inFile.close();
		outFile.close();
		return 0; // 正常执行
	}
};
#endif // COMPRESSOR_H_INCLUDED
