#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <iostream>
#include "Packet.h"

class UDPSocket
{
    int timeout = 10;
    struct sockaddr_in si_other;
    struct sockaddr_in si_me;
    socklen_t len_other;
    int fd;

public:
    UDPSocket()
    {
        memset(&si_other, 0, addrLen);
        memset(&si_me, 0, addrLen);
        len_other = addrLen;
        fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }
    static const socklen_t addrLen = sizeof(sockaddr_in);
    // bind to a port; should only be used in wReceiver
    int bind_me(int port)
    {
        si_me.sin_family = AF_INET;
        si_me.sin_port = htons(port);
        si_me.sin_addr.s_addr = INADDR_ANY;
        int ret = bind(fd, (struct sockaddr *)&si_me, sizeof(si_me));

        if (ret < 0)
        {
            perror("bind fail:");
            close(fd);
            return 0;
        }
        return 1;
    }
    // set the host to which the packets will be sent to
    // should only be used in wSender
    void set_other(int port, const char *server)
    {
        si_other.sin_family = AF_INET;
        si_other.sin_port = htons(port);
        struct hostent *sp = gethostbyname(server);
        memcpy(&si_other.sin_addr, sp->h_addr, sp->h_length);
    }
    // sent message to host;
    // after receiving, si_other will automatically be set to the sender
    // so set_other is not necessary while sending acks in wReceiver

    int send(const char *message, int buffer_size)
    {
        std::cout << "Sending to host: " << get_other_addr() << " port: " << get_other_port() << "\n";
        // this use strlen is error ,must use number
        return sendto(fd, message, buffer_size, 0, (struct sockaddr *)&si_other, sizeof(si_other));
    }
 

    void receive(char *message, int buff_size)
    {
        std::cout << "Received from host: " << get_other_addr() << " port: " << get_other_port() << "\n";
        recvfrom(fd, message, buff_size, 0, (struct sockaddr *)&si_other, &len_other);
    }

    // receiving  a maximum of buff_size bytes into message in 500ms
    // return -1 if do not receive any data in this period
    // else return the amount of data received
    //    int receiveTimeout(char *message, int buff_size)
    //    {
    //        int result = 0;
    //        bool getSomeData = false;
    //        int total_received = 0;
    //        for (int i = 0; i < 20; i++)
    //        {
    //            result = recvfrom(fd, message + total_received, buff_size - total_received, MSG_DONTWAIT, (struct sockaddr *)&si_other, &len_other);
    //            std::cout << "Received: " << result << "\n";
    //            if (result == -1)
    //                ;
    //            else
    //            {
    //                getSomeData = true;
    //                total_received = total_received + result;
    //                if (total_received == buff_size)
    //                {
    //                    break;
    //                }
    //            }
    //            // unit:milliseconds
    //            usleep(25000);
    //        }
    //        if (!getSomeData)
    //        {
    //            return -1;
    //        }
    //        std::cout << "Received: " << total_received << "\n";
    //        return total_received;
    //    }



    int receiveTimeout(char *message, int buff_size)
    {
        int bytes_received=-2;
        for(int i=0;i<20;i++){
            bytes_received=recvfrom(fd, message , buff_size , MSG_DONTWAIT, (struct sockaddr *)&si_other, &len_other);
            if(bytes_received!=-1){
                break;
            }
            usleep(25000);
        }
        return bytes_received;
    }
    int get_other_port()
    {
        return ntohs(si_other.sin_port);
    }
    const char *get_other_addr()
    {
        return inet_ntoa(si_other.sin_addr);
    }

    void sendPacket(Packet p)
    {

        int buffer_size = 16 + p.get_length(); // header + data

        char *buffer = new char[1024+16];
        Packet::serialize(&p, buffer);
        this->send(buffer, buffer_size);
        delete[] buffer;
        std::cout<<"Sending a "<<p.get_type()<<" packet with seqNum "<<p.get_seqNum()<<"\n";
        std::cout<<p.data<<std::endl;
    }

    // bool: false if time out;
    bool receivePacket(Packet *p)
    {
        char buffer[1024+16] = {0};

        this->receive(buffer, 1024+16);
        Packet::deserialize(p, buffer);
        // this->receive(buffer, p->header.length);
        // Packet::deserializeBody(p, buffer);
        return true;
    }

    bool receivePacketTimeout(Packet *p){
        char* buffer=new char[1024+16];
        memset(buffer,0,1024+16);
        int received=receiveTimeout(buffer,1024+16);
        if(received==-1){
            delete[] buffer;
            return false;
        }
        else{
            Packet::deserialize(p,buffer);

            delete[] buffer;
            return true;
        }
    }
    ~UDPSocket()
    {
        close(fd);
    }
};
