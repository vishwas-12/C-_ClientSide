#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring> 
#include "json.hpp"

#pragma comment(lib, "ws2_32.lib")
using namespace std;
using json1 = nlohmann::json;

#define SERVER_PORT 3000
#define BUFFER_SIZE 1024

struct Packet
{
    char symbol[4];
    char buySellIndicator;
    int32_t quantity;
    int32_t price;
    int32_t sequenceNumber;

    void parseFromBuffer(const char *buffer)
    {
        std::memcpy(symbol, buffer, 4);
        buySellIndicator = buffer[4];
        quantity = ntohl(*reinterpret_cast<const int32_t *>(buffer + 5));
        price = ntohl(*reinterpret_cast<const int32_t *>(buffer + 9));
        sequenceNumber = ntohl(*reinterpret_cast<const int32_t *>(buffer + 13));
    }

    json1 toJSON() const
    {
        return json1{
            {"symbol", std::string(symbol, 4)},
            {"buySellIndicator", std::string(1, buySellIndicator)},
            {"quantity", quantity},
            {"price", price},
            {"sequenceNumber", sequenceNumber}};
    }
};

void handleStreamAllPackets(SOCKET socketFD, std::set<int32_t> &receivedSequences, json1 &jsonArray)
{
    char buffer[BUFFER_SIZE];
    int bytesRead;

    while ((bytesRead = recv(socketFD, buffer, sizeof(buffer), 0)) > 0)
    {
        for (int i = 0; i < bytesRead; i += 17)
        { // Each packet is 17 bytes
            Packet packet;
            packet.parseFromBuffer(buffer + i);
            receivedSequences.insert(packet.sequenceNumber);
            jsonArray.push_back(packet.toJSON());
        }
    }
}

void requestResendPacket(SOCKET socketFD, int32_t sequenceNumber, json1 &jsonArray)
{
    char request[2];
    request[0] = 2; // Call Type 2: Resend Packet
    request[1] = static_cast<char>(sequenceNumber);

    if (send(socketFD, request, sizeof(request), 0) == SOCKET_ERROR)
    {
        std::cerr << "Error sending resend request: " << WSAGetLastError() << '\n';
        return;
    }

    char buffer[17];
    if (recv(socketFD, buffer, sizeof(buffer), 0) > 0)
    {
        Packet packet;
        packet.parseFromBuffer(buffer);
        jsonArray.push_back(packet.toJSON());
    }
    else
    {
        std::cerr << "Error receiving resent packet: " << WSAGetLastError() << '\n';
    }
}
int main()
{
    cout << "started" << endl;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed with error code: " << WSAGetLastError() << '\n';
        return 1; // Exit the program with an error code
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    SOCKET socketFD;
    sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (serverAddr.sin_addr.s_addr == INADDR_NONE)
    {
        std::cerr << "Invalid address/ Address not supported\n";
        WSACleanup();
        return 1;
    }

    if ((socketFD = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << '\n';
        WSACleanup();
        return 1;
    }

    // Connect to server
    if (connect(socketFD, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Connection failed: " << WSAGetLastError() << '\n';
        closesocket(socketFD);
        WSACleanup();
        return 1;
    }

    char request[2] = {1, 0}; // Call Type 1: Stream All Packets
    if (send(socketFD, request, sizeof(request), 0) == SOCKET_ERROR)
    {
        std::cerr << "Error sending request: " << WSAGetLastError() << '\n';
        closesocket(socketFD);
        WSACleanup();
        return 1;
    }

    std::set<int32_t> receivedSequences;
    json1 jsonArray = json1::array();

    handleStreamAllPackets(socketFD, receivedSequences, jsonArray);

    closesocket(socketFD);

    int32_t lastSequence = *receivedSequences.rbegin();
    for (int32_t i = 1; i <= lastSequence; ++i)
    {
        if (receivedSequences.find(i) == receivedSequences.end())
        {
            std::cout << "Missing sequence: " << i << '\n';

            if ((socketFD = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
            {
                std::cerr << "Socket creation failed: " << WSAGetLastError() << '\n';
                WSACleanup();
                return 1;
            }

            if (connect(socketFD, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
            {
                std::cerr << "Connection failed: " << WSAGetLastError() << '\n';
                closesocket(socketFD);
                WSACleanup();
                return 1;
            }

            requestResendPacket(socketFD, i, jsonArray);

            closesocket(socketFD);
        }
    }
    std::ofstream outFile("output.json");
    if (outFile.is_open())
    {
        outFile << jsonArray.dump(4); // Pretty print with 4 spaces
        outFile.close();
        std::cout << "JSON data written to output.json\n";
    }
    else
    {
        std::cerr << "Failed to open file for writing\n";
    }
    cout << "end reached" << endl;

    WSACleanup();
    system("pause");
    return 0;
}
