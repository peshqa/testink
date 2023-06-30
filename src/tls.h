#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <cstdlib>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include "tls_constants.h"

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "443"

int WriteClientHello(unsigned char buf[], unsigned int length);

void PrintTLSHandshake(unsigned char msg[])
{
	int hs_type = msg[0];
	int length = (msg[1] << 16) + (msg[2] << 8) + msg[3];
	switch (hs_type)
	{
	default:
		std::cout << "Unrecognised handshake type\n";
		break;
	case HS_TYPE_CLIENT_HELLO:
		std::cout << "Client Hello\n";
		break;
	}
	printf("Length: %d\n", length);
	printf("Data: ");
	for (int i=0; i<length; i++)
		std::cout << std::hex << (unsigned int)msg[i+4] << " ";
	std::cout << "\n";
}

void PrintTLSMsg(unsigned char msg[])
{
	// Read msg header
	int msg_type = msg[0];
	int major_ver = msg[1];
	int minor_ver = msg[2];
	int length = (msg[3] << 8) + msg[4];
	
	switch (msg_type)
	{
	default:
		std::cout << "Unrecognised msg type\n";
		break;
	case MSG_TYPE_HANDSHAKE:
		std::cout << "Handshake\n";
		break;
	case MSG_TYPE_ALERT:
		std::cout << "Alert\n";
		break;
	}
	
	printf("Version: %d.%d\n", major_ver, minor_ver);
	printf("Length: %d\n", length);
	printf("Data: ");
	for (int i=0; i<length; i++)
		std::cout << std::hex << (unsigned int)msg[i+5] << " ";
	std::cout << "\n";
	
	if (msg_type == MSG_TYPE_HANDSHAKE)
	{
		std::cout << "\n";
		PrintTLSHandshake(msg+5);
	}
}

int InitConn()
{
	WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    char sendbuf[DEFAULT_BUFLEN];
	WriteClientHello((unsigned char*)sendbuf, 131-3);
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo("google.com", DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    // Send an initial buffer
	sendbuf[131]='\r';
	sendbuf[132]='\n';
    iResult = send( ConnectSocket, (const char*)sendbuf, 131+2, 0 );
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    printf("Bytes Sent: %ld\n", iResult);

    // shutdown the connection since no more data will be sent
    //iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Receive until the peer closes the connection
    do {

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 ) {
            printf("Bytes received: %d\n", iResult);
			printf("%s\n", recvbuf);
			for (int i=0; i<iResult; i++)
				std::cout << std::hex << (unsigned int)recvbuf[i] << " ";
			std::cout << "\n";
		}
        else if ( iResult == 0 )
            printf("Connection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());

    } while( iResult > 0 );

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

	return 0;
}

void WriteRandomBytes(unsigned char buf[], unsigned int length)
{
	for (unsigned int i = 0; i < length; ++i)
	{
		buf[i] = rand() % 256;
	}
}

int WriteClientHello(unsigned char buf[], unsigned int length)
{
	//if (length > 0b100000000000000)
	//{
	//	//The length MUST NOT exceed 2^14.
	//	return 1;
	//}
	
	// Message Header
	buf[0] = 22; // content type - handshake
	// TLS version 1.0
	buf[1] = 3; // major version
	buf[2] = 1; // minor version
	buf[3] = (unsigned char)(length << 8);
	buf[4] = (unsigned char)length;
	
	// Handshake Header
	unsigned int hs_length = length - 4;
	buf[5] = 1; // handshake type - client hello
	buf[6] = (unsigned char)(hs_length << 16);
	buf[7] = (unsigned char)(hs_length << 8);
	buf[8] = (unsigned char)hs_length;
	
	// Handshake Body
	// TLS version 1.2
	buf[9] = 3; // major version
	buf[10] = 3; // minor version
	// 32 random bytes
	WriteRandomBytes(buf+11, 32);
	buf[43] = 0; // session id length (0 bytes)
	unsigned int ci_length = 2;
	buf[44] = (unsigned char)(ci_length << 8);
	buf[45] = (unsigned char)ci_length;
	
	// Ciphers
	buf[46] = 0xC0;
	buf[47] = 0x2F; // TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256
	
	// Compression methods
	buf[48] = 1; // length
	buf[49] = 0; // null method
	
	// Extensions
	buf[50] = 0;
	buf[51] = 78; // Extendsions length
	
	// Ext - Server name
	buf[52] = 0;
	buf[53] = 0; // extention type
	buf[54] = 0;
	buf[55] = 15; // data length
	
	buf[56] = 0;
	buf[57] = 13; // server name list length
	
	buf[58] = 0; // name type
	buf[59] = 0;
	buf[60] = 10; // host name length
	
	buf[61] = 'g';
	buf[62] = 'o';
	buf[63] = 'o';
	buf[64] = 'g';
	buf[65] = 'l';
	buf[66] = 'e';
	buf[67] = '.';
	buf[68] = 'c';
	buf[69] = 'o';
	buf[70] = 'm';
	
	// Ext - status request
	buf[71] = 0;
	buf[72] = 5; // ext type
	buf[73] = 0;
	buf[74] = 5; // data length
	
	buf[75] = 1; // status type
	buf[76] = 0;
	buf[77] = 0;
	buf[78] = 0;
	buf[79] = 0; // ?
	
	// Ext - Supported groups
	buf[80] = 0;
	buf[81] = 10; // ext type
	buf[82] = 0;
	buf[83] = 10; // data length
	
	buf[84] = 0;
	buf[85] = 8; // groups length
	buf[86] = 0;
	buf[87] = 0x1d;
	buf[88] = 0;
	buf[89] = 0x17;
	buf[90] = 0;
	buf[91] = 0x18;
	buf[92] = 0;
	buf[93] = 0x19;
	
	// Ext - ec_point_formats
	buf[94] = 0;
	buf[95] = 11; // ext type
	buf[96] = 0;
	buf[97] = 2; // data length
	
	buf[98] = 1; // list length
	buf[99] = 0;
	
	// Ext - signature_algorithms
	buf[100] = 0;
	buf[101] = 13; // ext type
	buf[102] = 0;
	buf[103] = 0x12; // data length
	
	buf[104] = 0;
	buf[105] = 0x10;
	buf[106] = 4;
	buf[107] = 1;
	buf[108] = 4;
	buf[109] = 3;
	buf[110] = 5;
	buf[111] = 1;
	buf[112] = 5;
	buf[113] = 3;
	buf[114] = 6;
	buf[115] = 1;
	buf[116] = 6;
	buf[117] = 3;
	buf[118] = 2;
	buf[119] = 1;
	buf[120] = 2;
	buf[121] = 3;
	
	// Ext - renegotiation_info
	buf[122] = 0xff;
	buf[123] = 1; // ext type
	buf[124] = 0;
	buf[125] = 1; // data length
	buf[126] = 0;
	
	// ext - SCT
	buf[127] = 0;
	buf[128] = 0x12; // ext type
	buf[129] = 0;
	buf[130] = 0; // data length
	
	
	return 0;
}

int tls_test()
{
	unsigned char hey[256] = "Hey";
	WriteClientHello(hey, 131-3);
	//for (int i=0; i<131; i++)
	//	std::cout << std::hex << (unsigned int)hey[i] << " ";
	//std::cout << "\n";
	
	PrintTLSMsg(hey);
	//InitConn();
}