#include <iostream>
#include <fstream>
#include <map>
#include <queue>
#include <vector>
#include <bitset>
//int sum=0;
//int head=0;

using namespace std;
class Compressor{
    map<unsigned char,string> codeMap;
    struct haffNode{
        int freq;
        unsigned char uchar;
        int dep;
        string code;
        struct haffNode* left=0;
        struct haffNode* right=0;
    };

    struct cmp{
        bool operator ()(const haffNode *a, const haffNode *b){
            return a->freq > b->freq;
        }
    };

    void encode(haffNode* pn,int dep,string code){
        pn->dep=dep;
        pn->code=code;
        if(pn->left) encode(pn->left,dep+1,code+"0");
        if(pn->right) encode(pn->right,dep+1,code+"1");
        if(!(pn->left||pn->right)){
            //cout<<int(pn->uchar)<<" has code:"<<code<<" freq:"<<pn->freq<<endl;
            codeMap[pn->uchar]=code;
            //sum+=pn->freq * dep;
            //head+=dep;
        }
    }

    short string_to_short(string s){
        bitset<16> bs(s);
        return bs.to_ulong();
    }

    unsigned char string_to_uchar(string s){
        bitset<8> bs(s);
        return bs.to_ulong();
    }

    public:
    bool compress(string sourcePath,string destinationPath){
        if(sourcePath.substr(sourcePath.find_last_of(".")+1)!="tar") return false;
        ifstream inFile;
        map<unsigned char,int> freqMap;
        inFile.open(sourcePath, ios::in | ios::binary);
        if(!inFile) return false;
        ofstream outFile;
        string newFileName=destinationPath+sourcePath.substr(0,sourcePath.find_last_of("."))+".bak";
        outFile.open(newFileName, ios::out | ios::binary);
        if(!outFile){
            inFile.close();
            return false;
        }
        /**ͳ�ƴ�Ƶ**/
        {
            unsigned char uchar;
            while(inFile.read((char *)&uchar, sizeof(char))) {
                freqMap[uchar]++;
            }
        }
        /**������ƵС����**/
        priority_queue<haffNode*,vector<haffNode*>,cmp> freqHeap;
        map<unsigned char,int>::reverse_iterator iter;
        for(iter = freqMap.rbegin(); iter != freqMap.rend(); iter++){
            haffNode*pn=new(haffNode);
            pn->freq=iter->second;
            pn->uchar=iter->first;
            pn->left=pn->right=0;
            freqHeap.push(pn);
        }
        //cout<<freqHeap.size()<<endl<<endl;
        /**������������**/
        while(freqHeap.size()>1){
            haffNode*pn1=freqHeap.top();
            freqHeap.pop();
            haffNode*pn2=freqHeap.top();
            freqHeap.pop();
            haffNode*pn=new(haffNode);
            pn->freq=pn1->freq+pn2->freq;
            //pn->uchar=0;
            pn->left=pn1;
            pn->right=pn2;
            //pn->dep=max(pn1->dep,pn2->dep)+1;
            freqHeap.push(pn);
            /*cout<<int(pn1->uchar)<<" "<<pn1->freq<<"       ";
            cout<<int(pn2->uchar)<<" "<<pn2->freq<<"       ";
            cout<<int(pn->uchar)<<" "<<pn->freq<<" \n";*/
        }
        haffNode*root=freqHeap.top();
        freqHeap.pop();
        /**�ù�����������**/
        encode(root,0,"");
        /**д��ѹ���ļ�ͷ��**/
        //cout<<sum/8/1024<<" "<<head/8;
        {
            char c=0;
            outFile.write((char*)&c, sizeof(char));
        }
        {
            for(int i=0;i<256;i++){
                unsigned char uc=i;
                string codeString="1"+codeMap[uc];
                short codeShort=string_to_short(codeString);
                //cout<<i<<" "<<codeMap[uc]<<" "<<codeShort<<endl;
                outFile.write((char*)&codeShort, sizeof(codeShort));
            }
        }
        //outFile.write((char*)&bi, sizeof(bi));
        /**д��ѹ���ļ�����**/
        {
            inFile.clear();
            inFile.seekg(0);
            string buf;
            unsigned char uchar;
            while(inFile.read((char *)&uchar,sizeof(unsigned char))) {
                buf+=codeMap[uchar];
                //cout<<"��ǰbuf����"<<buf.length()<<endl;
                while(buf.length()>=8){
                    uchar=string_to_uchar(buf.substr(0,8));
                    outFile.write((char *)&uchar,sizeof(unsigned char));
                    buf=buf.substr(8);
                }
            }
            int zeroNum=8-buf.length();
            for(int i=0;i<zeroNum%8;i++){
                buf+="0";
            }
            uchar=string_to_uchar(buf);
            outFile.write((char *)&uchar,sizeof(unsigned char));
        /**д��ͷ��Ԥ���Ĳ����ֶ�**/
            outFile.clear();
            outFile.seekp(0);
            uchar=zeroNum;
            //cout<<endl<<"��������"<<zeroNum<<endl;
            outFile.write((char *)&uchar,sizeof(unsigned char));
        }
        inFile.close();
        outFile.close();
        return true;
    }
};
