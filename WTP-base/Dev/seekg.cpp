#include <fstream>
#include <iostream>
int CHUNK_SIZE=1024;
using namespace std;
int main(){
    ifstream file("./bin.txt",ios::binary);
    file.seekg(0, ios::end);
    int length = file.tellg();
    file.seekg(0, ios::beg);
    for(int i=0;i<3;i++){
        file.clear();
        file.seekg(ios::beg,ios::beg);
//        cout<<"End of file: "<<file.eof()<<"\n";
//        cout<<"fail: "<<file.fail()<<"\n";
        for(int j=0;j<length/CHUNK_SIZE;j++){
            char buffer[1024]={0};
            file.read(buffer,CHUNK_SIZE);
            cout<<"Content "<<j+length/CHUNK_SIZE*i<<" is:\n";
            cout<<buffer<<"\n";
        }
    }
}