#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <bitset>
//int sum=0;
//int head=0;

using namespace std;
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
