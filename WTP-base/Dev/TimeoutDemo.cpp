//
// Created by daniel-cai on 4/4/22.
//

#include "../UDPSocket.h"

int main(){
    UDPSocket s;
    s.bind_me(8888);
    char buffer[64]={0}; int size=24;
    for(int i=0;i<5;i++){
        s.receiveTimeout(buffer,size);
        std::cout<<"The "<<i<<" th loop: "<<buffer<<"\n ";
        memset(buffer,0,size);
    }
}