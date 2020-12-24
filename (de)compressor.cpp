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
        /**统计词频**/
        {
            unsigned char uchar;
            while(inFile.read((char *)&uchar, sizeof(char))) {
                freqMap[uchar]++;
            }
        }
        /**建立词频小顶堆**/
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
        /**构建哈夫曼树**/
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
        /**用哈夫曼树编码**/
        encode(root,0,"");
        /**写入压缩文件头部**/
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
        /**写入压缩文件主体**/
        {
            inFile.clear();
            inFile.seekg(0);
            string buf;
            unsigned char uchar;
            while(inFile.read((char *)&uchar,sizeof(unsigned char))) {
                buf+=codeMap[uchar];
                //cout<<"当前buf长："<<buf.length()<<endl;
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
        /**写入头部预留的补零字段**/
            outFile.clear();
            outFile.seekp(0);
            uchar=zeroNum;
            //cout<<endl<<"补零数："<<zeroNum<<endl;
            outFile.write((char *)&uchar,sizeof(unsigned char));
        }
        inFile.close();
        outFile.close();
        return true;
    }
};
class Decompressor{
    map<unsigned char,string> codeMap;
    struct haffNode{
        unsigned char uchar;
        string code;
        struct haffNode* left=0;
        struct haffNode* right=0;
    };

    void insert_node(haffNode*father,unsigned char uchar,string code){
        if(code.empty()){
            father->uchar=uchar;
            return;
        }
        char way=code[0];
        if(way=='0'){
            if(!(father->left)){
                haffNode*son=new(haffNode);
                father->left=son;
            }
            insert_node(father->left,uchar,code.substr(1));
        }else{
            if(!(father->right)){
                haffNode*son=new(haffNode);
                father->right=son;
            }
            insert_node(father->right,uchar,code.substr(1));
        }
    }

    void dfs(haffNode*pn,string code){
        if(pn->left) dfs(pn->left,code+"0");
        if(pn->right) dfs(pn->right,code+"1");
        if(!(pn->left||pn->right)){
            //cout<<int(pn->uchar)<<"  "<<code<<endl;
            codeMap[pn->uchar]=code;
        }
    }

    public:
    bool decompress(string sourcePath,string destinationPath){
        if(sourcePath.substr(sourcePath.find_last_of(".")+1)!="bak") return false;
        ifstream inFile;
        inFile.open(sourcePath, ios::in | ios::binary);
        if(!inFile) return false;
        ofstream outFile;
        string newFileName=destinationPath+sourcePath.substr(0,sourcePath.find_last_of("."))+".tar";
        outFile.open(newFileName, ios::out | ios::binary);
        if(!outFile){
            inFile.close();
            return false;
        }
        /**读出文件头，创建code树**/
        int zeroNum;
        unsigned char uchar;
        short codeShort;
        string codeString;
        haffNode*root=new(haffNode);
        inFile.read((char *)&uchar, sizeof(char));
        zeroNum=uchar;
        for(unsigned i=0;i<256;i++){
            inFile.read((char *)&codeShort,sizeof(codeShort));
            bitset<16> bs=codeShort;
            codeString=bs.to_string();
            //cout<<i<<" "<<codeString<<" "<<codeShort<<endl;
            int j=0;
            for(;codeString[j]=='0';j++);
            codeString=codeString.substr(j+1);
            //cout<<i<<" "<<codeString<<" "<<codeShort<<endl;
            insert_node(root,i,codeString);
        }
        dfs(root,"");
        for(int i=0;i<256;i++){
            unsigned char uc=i;
            //string codeString="1"+codeMap[uc];
            //short codeShort=string_to_short(codeString);
            //cout<<i<<" "<<codeMap[uc]<<endl;
            //outFile.write((char*)&codeShort, sizeof(codeShort));
        }
        /**读出主体，用code树解码**/
        haffNode*decodePointer=root;
        string buf,now;
        inFile.read((char *)&uchar,sizeof(unsigned char));
        bitset<8> bs=uchar;
        buf=bs.to_string();
        while(inFile.read((char *)&uchar,sizeof(unsigned char))) {
            bitset<8> bs=uchar;
            now=buf;
            buf=bs.to_string();
            for(char i=0;i<8;i++){
                if(now[i]=='0'){
                    decodePointer=decodePointer->left;
                }else{
                    decodePointer=decodePointer->right;
                }
                if(!(decodePointer->left||decodePointer->right)){
                    //cout<<int(decodePointer->uchar)<<" ";
                    outFile.write((char *)&(decodePointer->uchar),sizeof(unsigned char));
                    decodePointer=root;
                }
            }
        }
        now=buf;
        for(char i=0;i<(8-zeroNum)%8;i++){
            if(now[i]=='0'){
                decodePointer=decodePointer->left;
            }else{
                decodePointer=decodePointer->right;
            }
            if(!(decodePointer->left||decodePointer->right)){
                //cout<<int(decodePointer->uchar)<<" ";
                outFile.write((char *)&(decodePointer->uchar),sizeof(unsigned char));
                decodePointer=root;
            }
        }

        inFile.close();
        outFile.close();
        if(decodePointer==root) return true;
        return false;
    }
};
/*
int main(void){
    Compressor y;
    y.compress("文章.tar","");
    Decompressor j;
    j.decompress("文章.bak","");
}*/
