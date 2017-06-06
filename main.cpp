#include <stdio.h>
#include <conio.h>
#include <WinSock2.h>

#pragma comment(lib,"ws2_32")

#pragma warning(disable:4996)

#include "packet.h"

#define BUF_SIZE 1024

DWORD WINAPI GameProcThread(PVOID arg);

bool initUser();
void initGame();
void ReDrawPlayer(int, int, int);
void gotoXY(int, int);

USER* g_Player;
char g_Buffer[BUF_SIZE];

bool g_CriticalSection;
bool g_YourTurn;
bool g_EndGame;

int main()
{
	if (!initUser())
		printf("[GAME] 초기화 실패 !!");

	printf("[GAME] USER(%s) 초기화를 성공했습니다.\n", g_Player->nick);
	printf("[GAME] 게임 준비가 완료되면 ENTER키를 입력해주세요.\n");


	while (true)
	{
		if (kbhit())
		{
			break;
		}
	}

	int client = sizeof(SOCKADDR_IN);

	memset(&g_Buffer, 0, sizeof(g_Buffer));

	ST_READY* pReady = (ST_READY*)g_Buffer;
	pReady->PktID = PKT_READY;
	pReady->PktSize = sizeof(ST_READY);

	sendto(g_Player->sock, g_Buffer, pReady->PktSize, 0, (SOCKADDR*)&g_Player->e_addr, sizeof(g_Player->e_addr));

	Sleep(100);

	memset(&g_Buffer, 0, sizeof(g_Buffer));

	do {

		PACKETHEADER* pHeader = (PACKETHEADER*)g_Buffer;

		if (pHeader->PktSize > 0)
		{
			if (pHeader->PktID == PKT_READY)
			{
				break;
			}
		}
		else
		{
			memset(&g_Buffer, 0, sizeof(g_Buffer));

			int r = recvfrom(g_Player->sock, g_Buffer, sizeof(g_Buffer), 0, (SOCKADDR*)&g_Player->e_addr, &client);

			printf("%d %d\n", r, WSAGetLastError());

			Sleep(150);
		}
	} while (true);

	do
	{
		DWORD ThreadID = 0;
		HANDLE gameHandle = CreateThread(0, 0, GameProcThread, NULL, 0, &ThreadID);

		g_YourTurn = true;

		initGame();

		g_EndGame = true;

		do
		{
			int client = g_Player->port - 9000;
			int target = g_Player->e_port - 9000;

			if (g_YourTurn)
			{
				int r = rand() % 3 + 1;

				if (!g_CriticalSection) {
					g_CriticalSection = true;
					ReDrawPlayer(client, g_Player->position, g_Player->position + r);
				}

				g_Player->position += r;

				memset(&g_Buffer, 0, sizeof(g_Buffer));
				ST_MOVE * pMove = (ST_MOVE*)g_Buffer;

				pMove->PktID = PKT_MOVE;
				pMove->PktSize = sizeof(ST_MOVE);
				pMove->move = r;

				sendto(g_Player->sock, g_Buffer, sizeof(g_Buffer), 0, (SOCKADDR*)&g_Player->e_addr, sizeof(g_Player->e_addr));

				Sleep(200);

				g_YourTurn = false;
			}

			// 만약 플레이어가 위치가 80 이상이면 게임 종료 패킷 전송

			if (g_EndGame)
			{
				if (g_Player->position >= 80)
				{
					gotoXY(10, 2);
					printf("%s", g_Player->nick);

					memset(&g_Buffer, 0, sizeof(g_Buffer));
					ST_WINNER* pWinner = (ST_WINNER*)g_Buffer;

					pWinner->PktID = PKT_WINNER;
					pWinner->PktSize = sizeof(ST_WINNER);
					strcpy(pWinner->nick, g_Player->nick);

					sendto(g_Player->sock, g_Buffer, sizeof(g_Buffer), 0, (SOCKADDR*)&g_Player->e_addr, sizeof(g_Player->e_addr));

					break;
				}
			}
		} while (g_EndGame);

		g_Player->e_position = 6;
		g_Player->position = 6;

		gotoXY(0, 4);
		printf("5초 후 게임이 재 시작 됩니다.");
		Sleep(1000);
		gotoXY(0, 4);
		printf("4초 후 게임이 재 시작 됩니다.");
		Sleep(1000);
		gotoXY(0, 4);
		printf("3초 후 게임이 재 시작 됩니다.");
		Sleep(1000);
		gotoXY(0, 4);
		printf("2초 후 게임이 재 시작 됩니다.");
		Sleep(1000);
		gotoXY(0, 4);
		printf("1초 후 게임이 재 시작 됩니다.");
		Sleep(1000);
	} while (true);

	closesocket(g_Player->sock);

	WSACleanup();

	return 0;
}


bool initUser()
{
	unsigned short port = 0;
	unsigned short port2 = 0;

	WSADATA    wsaData;

	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
		return false;

#ifdef _USER_A_
	port = 9000;
	port2 = 9001;
#else
	port = 9001;
	port2 = 9000;
#endif

	g_Player = new USER;

	g_Player->sock = socket(AF_INET, SOCK_DGRAM, 0);

	g_Player->port = port;
	g_Player->e_port = port2;

	g_Player->position = 6;
	g_Player->e_position = 6;

	if (g_Player->port == 9000)
		strcpy(g_Player->nick, "PLAYER 1");
	else
		strcpy(g_Player->nick, "PLAYER 2");

	if (g_Player->sock == INVALID_SOCKET) {
		return false;
	}

	SOCKADDR_IN addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = g_Player->port;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(g_Player->sock, (SOCKADDR*)&addr, sizeof(addr)) == -1) {
		return false;
	}

	memset(&g_Player->e_addr, 0, sizeof(g_Player->e_addr));
	g_Player->e_addr.sin_family = AF_INET;
	g_Player->e_addr.sin_port = g_Player->e_port;
	g_Player->e_addr.sin_addr.s_addr = inet_addr("115.138.112.178");

	return true;
}

void initGame()
{
	system("cls");

	CONSOLE_CURSOR_INFO info;

	info.bVisible = FALSE;
	info.dwSize = 1;

	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);

	printf("P1 : \nP2 : \nWinner : \n");

	gotoXY(6, 0);
	printf("→");

	gotoXY(6, 1);
	printf("▶");
}

void ReDrawPlayer(int p_num, int post_pos, int curr_pos)
{
	gotoXY(post_pos, p_num);
	printf("  ");
	Sleep(250);

	gotoXY(curr_pos, p_num);
	if (p_num == 0)
		printf("→");
	else
		printf("▶");

	Sleep(250);

	g_CriticalSection = false;
}

void gotoXY(int x, int y)
{
	COORD pos = { x,y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

DWORD WINAPI GameProcThread(PVOID arg)
{
	int target_num = g_Player->e_port - 9000;

	int client = sizeof(SOCKADDR_IN);
	char data[BUF_SIZE] = { 0 };

	while (true)
	{
		memset(&data, 0, sizeof(data));

		int r = recvfrom(g_Player->sock, data, sizeof(data), 0, (SOCKADDR*)&g_Player->e_addr, &client);

		if (r > 0)
		{
			PACKETHEADER* pHeader = (PACKETHEADER*)data;

			if (pHeader->PktID == PKT_MOVE)
			{
				// 다른 플레이어의 위치를 바꿔준다.

				ST_MOVE* pMove = (ST_MOVE*)pHeader;

				if (!g_CriticalSection) {
					g_CriticalSection = true;
					ReDrawPlayer(target_num, g_Player->e_position, g_Player->e_position + pMove->move);
				}

				g_Player->e_position += pMove->move;

				g_YourTurn = true;
			}
			else if (pHeader->PktID == PKT_WINNER)
			{
				// 게임을 종료한다.

				ST_WINNER* pWinner = (ST_WINNER*)pHeader;

				gotoXY(8, 2);
				printf("%s", pWinner->nick);

				g_EndGame = false;
			}
		}
	}
	return 0;
}