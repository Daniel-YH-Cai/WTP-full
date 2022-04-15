//
// Created by daniel-cai on 4/4/22.
//

#include "PacketHeader.h"
#include "crc32.h"
#include <string>
#define DATA_SIZE 1024
#define START 0;
class Packet
{
    int length;

public:
    Packet()
    {
        header = {4, 0, 0, 0};
        // size of payload of data packet
        length = 0;
    }
    // construct a data packet
    Packet(const char *data, int seqNum,int size)
    {
        this->length = size;
        std::memcpy( this->data, data, this->length);
        this->header.type = 2;
        this->header.length = this->length;
        this->header.seqNum = seqNum;
        this->header.checksum = crc32(this->data, this->length);
    }
    // create an ack packet
    Packet(unsigned int seqNum)
    {
        header = PacketHeader{3, seqNum, 0, 0};
        // size of payload of data packet
        length = 0;
    }
    // create a start packet
    static Packet StartPacket(unsigned int  initialSeq)
    {
        Packet p;
        p.header = {0, initialSeq, 0, 0};
        return p;
    }
    // create an end packet
    static Packet EndPacket(unsigned int initialSeq)
    {
        Packet p;
        p.header = {1, initialSeq, 0, 0};
        return p;
    }

    static void deserializeHeader(Packet *p, char *buffer)
    {
        char *b = buffer;
        memcpy(&p->header.type, b, 4);
        b += 4;
        memcpy(&p->header.seqNum, b, 4);
        b += 4;
        memcpy(&p->header.length, b, 4);
        b += 4;
        memcpy(&p->header.checksum, b, 4);
        p->length = p->header.length;
    }
    static void deserializeBody(Packet *p, char *buffer)
    {
        memcpy(p->data, buffer, p->header.length);
    }

    // create a packet from raw bytes received from socket
     static void deserialize(Packet *p, char *buffer)
     {
         char *b = buffer;
         memcpy(&p->header.type, b, 4);
         b += 4;
         memcpy(&p->header.seqNum, b, 4);
         b += 4;
         memcpy(&p->header.length, b, 4);
         b += 4;
         memcpy(&p->header.checksum, b, 4);
         b += 4;
         memcpy(p->data, b, p->header.length);
         p->length = p->header.length;

     }
    // serialize the packet into bytes and store them in buffer
    static int serialize(Packet *p, char *buffer)
    {
        char *b = buffer;
        memcpy(b, &p->header.type, 4);
        b += 4;
        memcpy(b, &p->header.seqNum, 4);
        b += 4;
        memcpy(b, &p->header.length, 4);
        b += 4;
        memcpy(b, &p->header.checksum, 4);
        b += 4;
        memcpy(b, p->data, p->header.length);
        return 16 + p->length;
    }
    // check sum
    bool checkSum()
    {
        if (this->header.checksum == crc32(this->data, this->length))
            return true;
        return false;
    }

    void setLength(int len)
    {
        this->header.length = len;
        this->length = len;
    }

    // getters
    // 0: START; 1: END; 2: DATA; 3: ACK
    std::string get_type()
    {
        std::string ans;
        switch (header.type) {
            case 0:
                ans="START";
                break;
            case 1:
                ans="END";
                break;
            case 2:
                ans="DATA";
                break;
            case 3:
                ans="ACK";
                break;
            default:
                ans="INVALID";
                std::cout<<"Invalid header!\n";
        }
        return ans;
    }
    unsigned int get_seqNum()
    {
        return header.seqNum;
    }
    unsigned int get_length()
    {
        return length;
    }
    unsigned int get_checksum()
    {
        return header.checksum;
    }

    // check if a ack packet is valid
    bool isValidACK()
    {
        if (checkSum() && get_type() == "ACK" && get_length() == 0)
        {
            return true;
        }
        return false;
    }

    void reset()
    {
        header = {4, 0, 0, 0};
        // size of payload of data packet
        length = 0;
    }

    char data[1024] = {0};
// const int for type
PacketHeader header;
};
