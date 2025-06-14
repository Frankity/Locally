#include "../include/websocket.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/sha.h>


WebSocket::WebSocket() = default;

std::string WebSocket::getWebSocketKey(const std::string &request){
    size_t key_start = request.find("Sec-WebSocket-Key: ");
    if (key_start == std::string::npos) return "";
    key_start += 19;
    const size_t key_end = request.find("\r\n", key_start);
    return request.substr(key_start, key_end - key_start);
}

bool WebSocket::sendWebSocketMessage(const SOCKET client, const std::string &message)
{
    try
    {
        std::vector<unsigned char> frame;
        frame.push_back(0x81);

        if (message.size() <= 125)
        {
            frame.push_back(static_cast<unsigned char>(message.size()));
        }
        else if (message.size() <= 65535)
        {
            frame.push_back(126);
            frame.push_back(static_cast<unsigned char>((message.size() >> 8) & 0xFF));
            frame.push_back(static_cast<unsigned char>(message.size() & 0xFF));
        }
        else
        {
            return false;
        }

        frame.insert(frame.end(), message.begin(), message.end());

        return send(client, reinterpret_cast<const char *>(frame.data()), frame.size(), 0) > 0;
    }
    catch (...)
    {
        return false;
    }
}

std::string base64_encode(const unsigned char *input, int length)
{
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, input, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);

    std::string result(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);

    return result;
}

std::string WebSocket::generateWebSocketAccept(const std::string &key)
{
    const std::string magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    std::string combined = key + magic;

    unsigned char sha1[20];
    SHA1(reinterpret_cast<const unsigned char *>(combined.c_str()), combined.size(), sha1);

    return base64_encode(sha1, 20);
}