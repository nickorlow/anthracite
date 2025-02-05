#pragma once 

#include <arpa/inet.h>
#include <cstdlib>
#include <malloc.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include "./socket.hpp"
#include <vector>
#include <array>
#include <iostream>
#include "../log/log.hpp"

namespace anthracite::socket {

constexpr uint32_t TLS_MSGHDR_RXSIZE = 4;


struct __attribute__((packed)) tls_version {
    uint8_t major;
    uint8_t minor;
};

struct __attribute__((packed)) tls_msg_hdr {
    uint8_t msg_type;
    tls_version version;
    uint16_t length;
};

struct tls_extension {
    uint16_t extension_type;
    std::vector<char> data;
};

class ServerHello {
    public:
    struct tls_version server_version;
    std::array<uint8_t, 32> random_bytes;
    std::array<uint8_t, 32> _session_id;
    uint16_t cipher;
    uint8_t compression;

    ServerHello(std::array<uint8_t, 32> session_id) {
        srand(time(nullptr));
        server_version.major = 3;
        server_version.minor = 3;

        for (int i = 0; i < 32; i++) {
            random_bytes[i] = rand() % 256;
        }

        _session_id = session_id;

        //  TLS_RSA_WITH_NULL_MD5 
        cipher = 1;

        // None
        compression = 0;
    }

    int get_buf(char** bufptr) {
        constexpr int msgsize =  2 + 32 + 1 + 32 + 2 + 1 + 7;
        constexpr int mmsgsize =  msgsize + 4;
        constexpr int bufsize = mmsgsize + 5;

        *bufptr = (char*) malloc(bufsize);

        char* buf = *bufptr;

        buf[0] = 0x16;
        buf[1] = 3;
        buf[2] = 1;
        buf[3] = (mmsgsize >> 8) & 0xFF;
        buf[4] = (mmsgsize) & 0xFF;


        buf[5] = 0x02;
        buf[6] = (msgsize >> 16) & 0xFF;
        buf[7] = (msgsize >> 8) & 0xFF;
        buf[8] = (msgsize) & 0xFF;

        buf[9] = server_version.major;
        buf[10] = server_version.minor;

        for(int i = 0; i < 32; i++) {
            buf[i+11] = random_bytes[i];
        }

        buf[43] = 32;
        
        for(int i = 0; i < 32; i++) {
            buf[i+44] = _session_id[i];
        }

        // Cipher
        buf[76] = 00;
        buf[77] = 0x33;

        // Compression
        buf[78] = 00;

        // Extensions Length
        buf[79] = 00;
        buf[80] = 01;
        
        // Renegotiation
        buf[81] = 0xFF;
        buf[82] = 01;

        // Disabled
        buf[83] = 00;
        buf[84] = 01;
        buf[85] = 00;


        return bufsize;
    }

    


};


class ClientHello {
    public:
    struct tls_version client_version;
    std::array<uint8_t, 32> random_bytes;
    std::array<uint8_t, 32> session_id;
    std::vector<uint16_t> cipher_suites;
    std::vector<uint8_t> compression_methods;
    std::vector<tls_extension> extensions;

    static uint32_t deserialize_uint32(char *buffer)
    {
        uint32_t value = 0;

        value |= buffer[0] << 24;
        value |= buffer[1] << 16;
        value |= buffer[2] << 8;
        value |= buffer[3];
        return value;
    }
    
    static uint32_t deserialize_uint24(char *buffer)
    {
        uint32_t value = 0;

        value |= buffer[0] << 16;
        value |= buffer[1] << 8;
        value |= buffer[2];
        return value;
    }

    static uint16_t deserialize_uint16(char *buffer)
    {
        uint32_t value = 0;

        value |= buffer[0] << 8;
        value |= buffer[1];

        return value;
    }


    // TODO: Note that the security of this funciton is terrible and absolutely 
    // can cause nasty things to happen with a malformed message 
    ClientHello(char* buffer, ssize_t size) {
        int bufptr = 0;
        // Get version data
        client_version.major = (uint8_t) buffer[bufptr++]; 
        client_version.minor = (uint8_t) buffer[bufptr++]; 
        log::debug << "TLS Version    : maj " << unsigned(client_version.major) << " min " << unsigned(client_version.minor) << std::endl;

        log::debug << "TLS Random Data: ";
        for(int i = 0; i < 32; i++) {
            random_bytes[i] = buffer[bufptr++];
            log::debug << std::hex << unsigned(random_bytes[i]) << std::hex << " ";
        }
        log::debug << std::endl;
        
        // Get session id 
        int session_id_length = (uint8_t) buffer[bufptr++];
        log::debug << "TLS SesId Data : ";
        for(int i =  0; i < session_id_length; i++) {
            session_id[i] = buffer[bufptr++];
            log::debug << std::hex << unsigned(session_id[i]) << " ";
        }
        log::debug << std::dec;
        log::debug << std::endl;

        // Get cipher suites
        uint16_t cipher_suites_length = deserialize_uint16(&buffer[bufptr]);
        bufptr += 2;

        log::debug << cipher_suites_length << " cipher suites supported" << std::endl;

        for(uint16_t i = 0; i < cipher_suites_length; i++) {
            cipher_suites.push_back(deserialize_uint16(&buffer[bufptr]));
            bufptr += 2;
        }
        
        // Get compression methods 
        uint16_t compression_methods_length = buffer[bufptr++];

        log::debug << compression_methods_length << " compression methods supported" << std::endl;

        for(uint16_t i = 0; i < compression_methods_length; i++) {
            cipher_suites.push_back(buffer[bufptr]);
            bufptr ++;
        }
    }
};

class tls_socket : anthracite_socket {
  


private:
    bool _handshakeDone;

    void perform_handshake();

public:
    tls_socket(int port, int max_queue = MAX_QUEUE_LENGTH);
    void wait_for_conn() override;
    void close_conn() override;
    void send_message(std::string& msg) override;
    std::string recv_message(int buffer_size) override;
};

};
