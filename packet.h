#ifndef _PACKET_H_
#define _PACKET_H_

#pragma  pack(push)
#pragma  pack(1)

#include <WinSock2.h>

#define PKTHEADERSIZE          6 // 패킷헤더의 크기

typedef struct _tgPacketHeader
{
	DWORD  PktID;
	WORD   PktSize;
}PACKETHEADER;

typedef struct _user
{
	SOCKET sock;
	unsigned short port;

	SOCKADDR_IN e_addr;
	unsigned short e_port;

	char nick[20];
	int position;
	int e_position;
}USER;

#define PKT_READY											0xa0000001
typedef struct _tgReady : public PACKETHEADER {} ST_READY;

#define PKT_MOVE											0xa0000002
typedef struct _tgMove : PACKETHEADER 
{
	unsigned short move;
} ST_MOVE;

#define PKT_WINNER											0xa0000003
typedef struct _tgWinner: PACKETHEADER
{
	char nick[20];
} ST_WINNER;

#pragma pack(pop)

#endif