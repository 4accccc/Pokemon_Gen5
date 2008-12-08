/*---------------------------------------------------------------------------*
  Project:  Revolution MPDS demo
  File:	 mpdsmodel.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: mpdsmodel.c,v $
  Revision 1.21  2008/06/05 09:50:49  seiki_masashi
  MPWaitPseudoVBlank 関数の追加

  Revision 1.20  2008/06/03 05:03:00  seiki_masashi
  Pseudo VBlank 通知を利用するように変更

  Revision 1.19  2007/11/29 03:20:27  seiki_masashi
  const 修飾子の追加

  Revision 1.18  2007/11/21 06:53:14  yosizaki
  ロールバックした内容を再反映。

  Revision 1.16  2007/10/27 11:20:46  seiki_masashi
  small fix

  Revision 1.15  2007/10/26 12:15:00  yosizaki
  ボール表示を追加。

  Revision 1.14  2007/10/26 09:31:17  seiki_masashi
  コードの整理

  Revision 1.13  2007/10/26 07:29:56  seiki_masashi
  コードの整理

  Revision 1.12  2007/10/26 07:05:28  seiki_masashi
  リンクレベル表示の修正

  Revision 1.10  2007/10/25 03:20:53  seiki_masashi
  子機のニックネームの表示に関して、ROM フォントのエンコードに応じた変換への対応

  Revision 1.8  2007/10/24 13:50:49  seiki_masashi
  Lobby でのニックネーム表示機能の追加

  Revision 1.5  2007/10/24 01:38:51  seiki_masashi
  接続状態の表示方法を修正

  Revision 1.3  2007/10/23 13:43:34  seiki_masashi
  パッド入力を送信するように修正

  Revision 1.2  2007/10/23 13:08:49  seiki_masashi
  マルチスレッド化

  Revision 1.1  2007/10/23 00:05:57  seiki_masashi
  mpdsmodel デモの追加

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

//
// NITRO-SDK の demos/wm/dataShare-Model デモと通信を行うサンプルです。
//
// 子機からの新規接続をロビー画面で受け付け、新規接続を締め切ってから
// 対戦を開始するというシークエンスのデモになっています。
//

#include <string.h>
#include <revolution.h>
#include <revolution/mp.h>
#include <revolution/mpds.h>
#include <revolution/mem.h>
#include <revolution/enc.h>

#include "rexdemo/demokpad.h"
#include "rexdemo/graphic.h"

#include "draw.h"

#include "ball.h"
#include "mpdsmodel.h"
#include "ds_net.h"
#include "common_def.h"
#include "net_system.h"

#define	 USERHEAP_SIZE ( 0x20000 )

/*===========================================================================*/

// アプリ固有の UserGameInfo
// DS に合わせて Little Endian で処理する必要があることに注意
#pragma pack(push,1)
typedef struct MyGameInfo
{
	u8	  nickName[10 * 2];
	u8	  nickNameLength;
	u8	  entryCount;
	u16	 periodicalCount;
}
MyGameInfo;
#pragma pack(pop)

enum
{
	SHARECMD_NONE = 0,				 // 特に無し（ノーマル）
	SHARECMD_EXITLOBBY,				// ロビー画面終了の合図
	SHARECMD_NUM
};

enum
{
	STATE_IDLE = 0,
	STATE_GOING_TO_LOBBY,
	STATE_LOBBY,
	STATE_GOING_TO_COMMUNICATION,
	STATE_COMMUNICATION,
	STATE_GOING_TO_IDLE
};

enum
{
	RESULT_OK = 0,
	RESULT_ERROR = -1,
	RESULT_QUIT = -2
};

/*===========================================================================*/

static const GXColor white = { 0xFF, 0xFF, 0xFF, };
static const GXColor yellow = { 0xFF, 0xFF, 0x00, };
static const GXColor gray = { 0x80, 0x80, 0x80, };
static const GXColor black = { 0x00, 0x00, 0x00, };
static const GXColor red = { 0xFF, 0x00, 0x18 };
static const GXColor green = { 0x00, 0xFF, 0x00 };

static MEMHeapHandle	sUserHeap;

static ShareData sRecvData[MY_MAX_NODES+1];
static BOOL sRecvFlag[MY_MAX_NODES+1];
static BOOL sConnectedFlag[MY_MAX_NODES+1];
static u8 sConnectedMacAddress[MY_MAX_NODES+1][MP_SIZE_MACADDRESS];
static u16 sConnectedNickName[MY_MAX_NODES+1][10 + 1];

static u32 sFrameCount = 0;
static s32 sState = STATE_IDLE;
static OSThreadQueue sStateChangeThreadQueue;
static BOOL sQuitFlag = FALSE;

static OSMutex sGlobalMutex;

static OSThread sStateManagementThread;
static OSThread sDataSharingThread;
static OSThread sUpdateGameInfoThread;

static u8 sStateManagementThreadStack[STACK_SIZE] ATTRIBUTE_ALIGN(32);
static u8 sDataSharingThreadStack[STACK_SIZE] ATTRIBUTE_ALIGN(32);
static u8 sUpdateGameInfoThreadStack[STACK_SIZE] ATTRIBUTE_ALIGN(32);

static MPDSContext sDSContext;
static DS_COMM_USERDATAINFO sMyGameInfo;
static ENCContext sEncContext;

/*===========================================================================*/

static void* StateManagementThreadFunc( void* arg );
static void* DataSharingThreadFunc( void* arg );
static void* UpdateGameInfoThreadFunc( void* arg );

static void Initialize( void );
static void MainLoop( void );

static void Draw( void );
static void SetState( s32 state );
static inline s32 GetState( void );
static s32 WaitForStateChange( void );
static void QuitMP( void );
static BOOL IsQuitting( void );
static s32 TransToLobby( void );
static s32 TransToCommunication( void );
static s32 TransToIdle( void );
static s32 DoLobbyMode( void );
static s32 DoCommunicationMode( void );
static void DataSharingCallback( s32 type, MPPortCallbackInfo* info );
static void IndicationCallback( s32 type, MPIndicationInfo* info );

static u16 ConvKPadKeyToDSPad(u32 mixedKey);

inline void EnterCriticalSection( void )
{
	(void)OSLockMutex( &sGlobalMutex );
}

void LeaveCriticalSection( void )
{
	(void)OSUnlockMutex( &sGlobalMutex );
}

/*===========================================================================*/

// GGID はアプリケーション毎に割り当てられる MP 通信用のユニーク ID
//   実アプリケーションでは、任天堂からの GGID の割り当てを受ける必要があります。
//   ここでは NITRO-SDK の dataShare-Model デモと同一の GGID を使用します。
#define MY_GGID	 (0x346)//0x003fff13

// MP 通信の設定
static MPConfig sMpConfig =
{
	mpAlloc,
	mpFree,

	8,				  // threadPriority; メインスレッドより4つ以上高い優先度に設定する

	MP_MODE_PARENT,	 // mode

	MY_GGID,			// ggid
	MP_TGID_AUTO,	   // tgid

	MP_CHANNEL_AUTO,	// channel; 通常は MP_CHANNEL_AUTO

	MP_LIFETIME_DEFAULT,// lifeTime; 通常は MP_LIFETIME_DEFAULT

	MP_BEACON_PERIOD_AUTO,  // beaconPeriod; 通常は MP_BEACON_PERIOD_AUTO
	MY_MAX_NODES,	   // maxNodes
	MY_PARENT_MAX_SIZE, // parentMaxSize
	MY_DS_LENGTH,	   // childMaxSize
	TRUE,			   // entryFlag
	FALSE,			  // multiBootFlag; 通常は FALSE

	1,				  // frequency

	0,				  // userGameInfoLength
	{ 0, },			 // userGameInfo; プログラムで MyGameInfo を設定する

	NULL,			   // indicationCallbackFunction

	/// ...			 // port 設定 (MPSetPortConfig で設定可能)
};

// DataSharing の設定
static MPDSConfig sMpdsConfig =
{
	MY_DS_LENGTH,	   // dataLength

	MY_DS_PORT,		 // port (12～15 の Sequential 通信用ポートを使う必要がある)
	MP_PRIORITY_HIGH,   // priority

	TRUE,			   // isParent; 常に TRUE
	(1 << MY_DS_NODES)-1,   // aidBits (MY_DS_NODES 個の1が並ぶ2進数の値)
	TRUE,			   // isDoubleMode
	TRUE,			   // isAutoStart; 常に TRUE
	DataSharingCallback // mpdsCallback; 不要なら NULL
};


void main()
{
	OSReport("test program started.\n");

	Initialize();
	
	MainLoop();
}

void Initialize( void )
{
#if 1
	/* initialize OS and memory heap */
	REXDEMOKPadInit();
//	REXDEMOInitScreen( FALSE );
	REXDEMOSetGroundColor( black );
	REXDEMOSetFontSize( 10, 20 );
//	REXDEMOBeginRender();
//	REXDEMOWaitRetrace();
	(void)PADInit();
#endif

	/* Initialize heap for MP library */
	{
		void*   heapAddress;

		heapAddress = OSGetMEM2ArenaLo();
		OSSetMEM2ArenaLo( (void*)OSRoundUp32B( (u32)heapAddress + USERHEAP_SIZE ) );
		sUserHeap   = MEMCreateExpHeapEx( heapAddress, USERHEAP_SIZE, MEM_HEAP_OPT_THREAD_SAFE );
		if( sUserHeap == NULL )
		{
			OSHalt( "Could not create heap.\n" );
		}
	}
	MPDS_Init();

	OSInitMutex( &sGlobalMutex );
	OSInitThreadQueue( &sStateChangeThreadQueue );

#if 0
	// 文字列表示に ROM フォントを使用しているため
	// ROM フォントのエンコードに合わせて ENCContext を準備する
	(void)ENCInitContext(&sEncContext);
	(void)ENCSetExternalEncoding(&sEncContext,
								 ( OSGetFontEncode() == OS_FONT_ENCODE_SJIS )
								 ? (const u8*)"Shift_JIS" : (const u8*)"ISO-8859-1");
	(void)ENCSetBreakType(&sEncContext, ENC_BR_KEEP);
	(void)ENCSetAlternativeCharacter(&sEncContext, L'?', L'?');
#endif

	sState = STATE_IDLE;

	// 状態管理スレッドの作成
	(void)OSCreateThread( &sStateManagementThread, StateManagementThreadFunc, NULL,
						  (void*)((u32)sStateManagementThreadStack + STACK_SIZE), STACK_SIZE,
						  THREAD_PRIORITY, OS_THREAD_ATTR_DETACH );
	(void)OSResumeThread( &sStateManagementThread );
	
	InitDrawSystem();
}

void MainLoop( void )
{
	while (TRUE)
	{

		// 他のスレッドと共有するグローバル変数に触る際には排他を行う
		EnterCriticalSection();
		REXDEMOKPadRead();
		LeaveCriticalSection();

		if ( OSIsThreadTerminated(&sStateManagementThread) )
		{
			// 全てのスレッドが終了したのでメインループを抜ける
			OSReport("All threads are terminated; exit from MainLoop\n");
			break;
		}

//		Draw();

		UpdateDrawSystem();

//		REXDEMOWaitRetrace();
		sFrameCount++;
	}
}

void Draw( void )
{
	s32 state;
	s32 i;

	REXDEMOBeginRender();

	{
		// 他のスレッドと共有するグローバル変数に触る際には排他を行う
		EnterCriticalSection(); // 排他開始
		state = GetState();

		switch ( state )
		{
		case STATE_LOBBY:
			REXDEMOSetTextColor(green);
			REXDEMOPrintf(40, 8, 0, "Lobby mode");
			REXDEMOSetTextColor(white);
			REXDEMOPrintf(4, 400, 0, "push <A> button to start.");

			for (i = 0; i < MY_MAX_NODES+1; i++)
			{
				if (sConnectedFlag[i])
				{
					char nickName[20+1];
					{
						ENCContext convCtx;
						s32 dstlen, srclen;
						dstlen = sizeof(nickName) - 1 /* NULL termination */;
						srclen = -1; // NULL 終端までを変換する
						(void)NETMemSet(nickName, 0, sizeof(nickName));
						(void)ENCDuplicateContext(&convCtx, &sEncContext);
						(void)ENCConvertFromInternalEncoding(&convCtx, (u8*)nickName, &dstlen, sConnectedNickName[i], &srclen);
					}
					REXDEMOSetTextColor(yellow);
					REXDEMOPrintf(40, (s16)(80 + i * 20), 0,
								  "AID(%02d): entry %02X:%02X:%02X:%02X:%02X:%02X %s",
								  i,
								  sConnectedMacAddress[i][0],
								  sConnectedMacAddress[i][1],
								  sConnectedMacAddress[i][2],
								  sConnectedMacAddress[i][3],
								  sConnectedMacAddress[i][4],
								  sConnectedMacAddress[i][5],
								  nickName);
				}
				else
				{
					REXDEMOSetTextColor(gray);
					REXDEMOPrintf(40, (s16)(80 + i * 20), 0, "AID(%02d): --------", i);
				}
			}
			break;

		case STATE_COMMUNICATION:
			REXDEMOSetTextColor(green);
			REXDEMOPrintf(40, 8, 0, "Parent mode");
			REXDEMOSetTextColor(white);
			REXDEMOPrintf(4, 400, 0, "push <B> button to restart.");

			REXDEMOSetTextColor(yellow);
			REXDEMOPrintf(4, 30, 0, "Send:	 0x%04X-0x%04X", 0, sFrameCount & 0xffff);
			REXDEMOPrintf(4, 52, 0, "Receive:");

			for (i = 0; i < MY_MAX_NODES+1; i++)
			{
				if (sRecvFlag[i])
				{
				/*
					REXDEMOSetTextColor(yellow);
					REXDEMOPrintf(40, (s16)(80 + i * 20), 0,
								  "AID(%02d): %04X-%04X",
								  i,
								  MPMPToH16((u16)sRecvData[i].key), MPMPToH16((u16)(sRecvData[i].count & 0xffff)) );
				*/
				}
				else
				{
					REXDEMOSetTextColor(red);
					REXDEMOPrintf(40, (s16)(80 + i * 20), 0, "No Data");
				}
			}
			{
				static const GXColor	paletteTable[] =
				{
					{ 0x80, 0x80, 0x80, },  // gray
					{ 0x00, 0xFF, 0x00, },  // green
					{ 0xFF, 0xFF, 0x00, },  // yellow
				};
				s16	 x   = 320;
				s16	 y   = 10;
				s16	 z   = -1023;
				for (i = 0; i < BALL_PLAYER_MAX; ++i)
				{
					REXDEMOSetTextColor(paletteTable[shared->ball[i].plt]);
					REXDEMOPrintf(x + shared->ball[i].x, y + shared->ball[i].y, 0, "%c", shared->ball[i].chr);
				}
				{
					static const u32 font[] ATTRIBUTE_ALIGN(32) =
					{
						0xFFFFFFFF,
						0xFFFFFFFF,
						0xFFFFFFFF,
						0xFFFFFFFF,
						0xFFFFFFFF,
						0xFFFFFFFF,
						0xFFFFFFFF,
						0xFFFFFFFF,
					};
					static const u16 pal[16] ATTRIBUTE_ALIGN(32) = 
					{
					GXPackedRGB5A3(0x00, 0x00, 0x00, 0xFF), /* 0 : black */ 
					GXPackedRGB5A3(0x80, 0x00, 0x00, 0xFF), /* 1 : dark-red */ 
					GXPackedRGB5A3(0x00, 0x80, 0x00, 0xFF), /* 2 : dark-green */ 
					GXPackedRGB5A3(0x80, 0x80, 0x00, 0xFF), /* 3 : dark-yellow */ 
					GXPackedRGB5A3(0x00, 0x00, 0x80, 0xFF), /* 4 : dark-blue */ 
					GXPackedRGB5A3(0x80, 0x00, 0x80, 0xFF), /* 5 : dark-purple */ 
					GXPackedRGB5A3(0x00, 0x80, 0x80, 0x00), /* 6 : (clear) */ 
					GXPackedRGB5A3(0x80, 0x80, 0x80, 0xFF), /* 7 : gray */ 
					GXPackedRGB5A3(0xC0, 0xC0, 0xC0, 0xFF), /* 8 : light-gray */ 
					GXPackedRGB5A3(0xFF, 0x00, 0x00, 0xFF), /* 9 : red */ 
					GXPackedRGB5A3(0x00, 0xFF, 0x00, 0xFF), /* A : green */ 
					GXPackedRGB5A3(0xFF, 0xFF, 0x00, 0xFF), /* B : yellow */ 
					GXPackedRGB5A3(0x00, 0x00, 0xFF, 0xFF), /* C : blue */ 
					GXPackedRGB5A3(0xFF, 0x00, 0xFF, 0xFF), /* D : purple */ 
					GXPackedRGB5A3(0x00, 0xFF, 0xFF, 0xFF), /* E : cyan */ 
					GXPackedRGB5A3(0xFF, 0xFF, 0xFF, 0xFF), /* F : white */ 
					};
					static GXTlutObj tlut;
					Mtx mtx;
					GXTexObj obj;
					GXInitTlutObj(&tlut, (void *)pal, GX_TL_RGB5A3, 16);
					GXLoadTlut(&tlut, GX_TLUT0);
					GXInitTexObjCI(&obj, (void *)font,
								   8, 8, GX_TF_C4,
								   GX_CLAMP, GX_CLAMP, GX_FALSE, GX_TLUT0);
					GXInitTexObjLOD(&obj, GX_NEAR, GX_NEAR,
									0.0f, 0.0f, 0.0f, GX_DISABLE,
									GX_FALSE, GX_ANISO_1);
					GXLoadTexObj(&obj, GX_TEXMAP4);
					MTXScale(mtx, 1.0f / (float)8, 1.0f / (float)8, 1.0f);
					GXLoadTexMtxImm(mtx, GX_TEXMTX4, GX_MTX2x4);
					// REXDEMOBeginRender 設定を一時的に変更
					GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX4);
					GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP4, GX_COLOR0A0);
					GXBegin(GX_QUADS, GX_VTXFMT0, 4);
					{
						GXPosition3s16((s16) (x), (s16) (y), (s16) z);
						GXTexCoord2s16((s16) (0), (s16) (0));
						GXPosition3s16((s16) (x + BALL_FIELD_WIDTH), (s16) (y), (s16) z);
						GXTexCoord2s16((s16) (8), (s16) (0));
						GXPosition3s16((s16) (x + BALL_FIELD_WIDTH), (s16) (y + BALL_FIELD_HEIGHT), (s16) z);
						GXTexCoord2s16((s16) (8), (s16) (8));
						GXPosition3s16((s16) (x), (s16) (y + BALL_FIELD_HEIGHT), (s16) z);
						GXTexCoord2s16((s16) (0), (s16) (8));
					}
					GXEnd();
					// REXDEMOBeginRender 設定を復元
					GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
					GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
				}
			}

			break;

		default:
			REXDEMOSetTextColor(green);
			REXDEMOPrintf(40, 8, 0, "Working...");
			break;
		}

		// Wii のガイドラインでは、リンクレベルの表示は必須ではありません。
		REXDEMOSetTextColor(green);
		{
			s32 linkLevel = MPGetLinkLevel();
			REXDEMOPrintf(480, 400, 0, "Link Level: %d", (linkLevel >= 0) ? linkLevel : 0);
		}

		REXDEMOSetTextColor(white);

		LeaveCriticalSection(); // 排他終了
	}
}

void SetState( s32 state )
{
	// 他のスレッドと共有するグローバル変数に触る際には排他を行う
	EnterCriticalSection(); // 排他開始
	sState = state;
	LeaveCriticalSection(); // 排他終了
	OSWakeupThread( &sStateChangeThreadQueue );
}

inline s32 GetState( void )
{
	return sState;
}

s32 WaitForStateChange( void )
{
	OSSleepThread( &sStateChangeThreadQueue );
	// 優先度の高いスレッドで SetState が複数回呼ばれた場合などにおいて、
	// sState の変化は最後の1つしか検知できないことに注意
	return GetState();
}

void QuitMP( void )
{
	sQuitFlag = TRUE;
}

BOOL IsQuitting( void )
{
	return sQuitFlag;
}

/////////////////////////////////////////////////////////////////////
// ゲーム状態の管理スレッド
void* StateManagementThreadFunc( void* arg )
{
#pragma unused( arg )
	s32 result;

	///////////////////////////////////////////////////////
	// STATE_IDLE
	///////////////////////////////////////////////////////
state_idle:
	SetState( STATE_IDLE );

	///////////////////////////////////////////////////////
	// STATE_GOING_TO_LOBBY
	///////////////////////////////////////////////////////
state_going_to_lobby:
	SetState( STATE_GOING_TO_LOBBY );

	result = TransToLobby();

	switch ( result )
	{
	case RESULT_OK:
		// go to next state
		break;

	case RESULT_ERROR:
		goto state_error;

	case RESULT_QUIT:
		goto state_going_to_idle;
	}

	///////////////////////////////////////////////////////
	// STATE_LOBBY
	///////////////////////////////////////////////////////
state_lobby:
	SetState( STATE_LOBBY );

	result = DoLobbyMode();

	switch ( result )
	{
	case RESULT_OK:
		// go to next state
		break;

	case RESULT_ERROR:
		goto state_error;

	case RESULT_QUIT:
		goto state_going_to_idle;
	}

	///////////////////////////////////////////////////////
	// STATE_GOING_TO_COMMUNICATION
	///////////////////////////////////////////////////////
state_going_to_communication:
	SetState( STATE_GOING_TO_COMMUNICATION );

	(void)TransToCommunication();

	///////////////////////////////////////////////////////
	// STATE_COMMUNICATION
	///////////////////////////////////////////////////////
state_communication:
	SetState( STATE_COMMUNICATION );

	result = DoCommunicationMode();

	switch ( result )
	{
	case RESULT_OK:
		// go to next state
		goto state_going_to_idle;

	case RESULT_ERROR:
		goto state_error;

	case RESULT_QUIT:
		goto state_going_to_idle;
	}

	///////////////////////////////////////////////////////
	// STATE_GOING_TO_IDLE
	///////////////////////////////////////////////////////
state_going_to_idle:
	SetState( STATE_GOING_TO_IDLE );

	result = TransToIdle();

	switch ( result )
	{
	case RESULT_ERROR:
		goto state_error;
	}

	goto state_idle;

state_error:
	OSReport("Error occurred.\n");

	return NULL;
}

////////////////////////////////////////////
// アイドル状態からロビーモードに状態遷移
s32 TransToLobby( void )
{
	s32 result;
	int i;

	// MP 通信の開始前のため、これらのグローバル変数を触るスレッドは他にありません。
	// 排他無しで初期化を行えます。
	for ( i = 0; i < MY_MAX_NODES+1; i++ )
	{
		sRecvFlag[i] = FALSE;
		sConnectedFlag[i] = FALSE;
		(void)NETMemSet(sConnectedMacAddress[i], 0, MP_SIZE_MACADDRESS);
		(void)NETMemSet(sConnectedNickName[i], 0, sizeof(sConnectedNickName[0]));
	}
	// 自分自身の設定
	sConnectedFlag[0] = TRUE;
	(void)NETGetWirelessMacAddress(sConnectedMacAddress[0]);
	(void)NETMemSet(sConnectedNickName[0], 0, sizeof(sConnectedNickName[0]));
	(void)NETMemCpy(sConnectedNickName[0], MY_NICKNAME, MY_NICKNAME_LENGTH * 2);
	ASSERT(MY_NICKNAME_LENGTH * 2 <= sizeof(sConnectedNickName[0]) - 2);

	sQuitFlag = FALSE;


	// セッション全員がローカルに保持すべきゲーム情報を初期化する
	// (MP通信を介して入力情報を交換し、同じルールで内容を更新していく)
	InitBall(NULL, 0, 1, 2);


	// UserGameInfo の初期化
	/*
	(void)NETMemSet(&sMyGameInfo, 0, sizeof(sMyGameInfo));
	sMyGameInfo.nickNameLength = MY_NICKNAME_LENGTH;
	//DS へのデータはエンディアンを変換しなければならないことに注意
	NETSwapAndCopyMemory16(sMyGameInfo.nickName, MY_NICKNAME, MY_NICKNAME_LENGTH*sizeof(u16));
	sMyGameInfo.entryCount = 0;
	sMyGameInfo.periodicalCount = 0;
	(void)NETMemCpy( sMpConfig.userGameInfo, &sMyGameInfo, sizeof(sMyGameInfo) );
	sMpConfig.userGameInfoLength = sizeof(MyGameInfo);
	*/
	
	(void)NETMemSet(&sMyGameInfo,0,sizeof(DS_COMM_USERDATAINFO));
	NETMemCpy(sMyGameInfo.FixHead, "POKEWB", _BEACON_FIXHEAD_SIZE*sizeof(u8));
	sMyGameInfo.serviceNo = 30;
	sMyGameInfo.connectNum = 0;
	(void)NETMemCpy( sMpConfig.userGameInfo, &sMyGameInfo, sizeof(DS_COMM_USERDATAINFO) );
	sMpConfig.userGameInfoLength = sizeof(DS_COMM_USERDATAINFO);
	
	MPSetIndicationConfig( &sMpConfig, IndicationCallback );

	result = MPDSInit( &sDSContext, &sMpdsConfig );
	if ( result < MP_RESULT_OK )
	{
		OSReport( "MPDSInit returns %08x\n", result );
		return RESULT_ERROR;
	}

	result = MPDSSetupPortConfig( &sDSContext, &sMpConfig );
	if ( result < MP_RESULT_OK )
	{
		OSReport( "MPDSSetupPortConfig returns %08x\n", result );
		return RESULT_ERROR;
	}

	// config に従って MP 通信状態を開始
	//   MP 状態になるのを待つためブロックします。
	OSReport("MPStartup()\n");
	result = MPStartup( &sMpConfig );
	if( result != MP_RESULT_OK )
	{
		OSReport( "MPStartup returns %08x\n", result );
		return RESULT_ERROR;
	}

	// 通信用スレッドの作成
	(void)OSCreateThread( &sDataSharingThread, DataSharingThreadFunc, NULL,
						  (void*)((u32)sDataSharingThreadStack + STACK_SIZE), STACK_SIZE,
						  THREAD_PRIORITY, 0 );
	(void)OSResumeThread( &sDataSharingThread );
	(void)OSCreateThread( &sUpdateGameInfoThread, UpdateGameInfoThreadFunc, NULL,
						  (void*)((u32)sUpdateGameInfoThreadStack + STACK_SIZE), STACK_SIZE,
						  THREAD_PRIORITY, 0 );
	(void)OSResumeThread( &sUpdateGameInfoThread );

	return IsQuitting() ? RESULT_QUIT : RESULT_OK;
}

////////////////////////////////////////////
// ロビーモードから通信モードに状態遷移
s32 TransToCommunication( void )
{
	s32 result;
	int i;

	// 新規接続を締め切る
	result = MPSetEntryFlag( FALSE );
	if ( result < MP_RESULT_OK )
	{
		OSReport( "MPSetEntryFlag returns %08x\n", result );
		return RESULT_ERROR;
	}

	// beacon を更新して、接続締め切りを告知
	//   次のビーコンが送信されるまで長期間ブロックします。
	result = MPUpdateBeacon();
	if ( result < MP_RESULT_OK )
	{
		OSReport( "MPUpdateBeacon returns %08x\n", result );
		return RESULT_ERROR;
	}

	// 他のスレッドと共有するグローバル変数に触る際には排他を行う
	EnterCriticalSection();
	for ( i = 0; i < MY_MAX_NODES+1; i++ )
	{
		sRecvFlag[i] = FALSE;
	}
	LeaveCriticalSection();

	return IsQuitting() ? RESULT_QUIT : RESULT_OK;
}

////////////////////////////////////////////
// アイドル状態へ状態遷移
s32 TransToIdle( void )
{
	s32 result;

	// スレッドに終了の指令を出す
	QuitMP();

	// MP 通信状態を終了
	//   全ての子機が切断されるのを待つために長期間ブロックします。
	OSReport("MPCleanup()\n");
	result = MPCleanup();
	if ( result < MP_RESULT_OK )
	{
		OSReport( "MPCleanup returns %08x\n", result );
		return RESULT_ERROR;
	}

	(void)OSJoinThread( &sDataSharingThread, NULL );
	(void)OSJoinThread( &sUpdateGameInfoThread, NULL );

	return RESULT_OK;
}

////////////////////////////////////////////
// Lobby Mode 中の状態遷移条件
s32 DoLobbyMode( void )
{
	BOOL fBreak = FALSE;

	// 子機の新規接続の受付
	{
		while ( !fBreak )
		{
			// 他のスレッドと共有するグローバル変数に触る際には排他を行う
			EnterCriticalSection();
			if( REXDEMOGetAnyMixedPadTrigger() & (KPAD_BUTTON_A | (PAD_BUTTON_A << 16)) )
			{
				// A ボタンで次の状態へ遷移
				fBreak = TRUE;
			}
			LeaveCriticalSection();

			VIWaitForRetrace();
		}
	}

	return RESULT_OK;
}

////////////////////////////////////////////
// Communication Mode 中の状態遷移条件
s32 DoCommunicationMode( void )
{
	BOOL fBreak = FALSE;

	// 通信中
	{
		while ( !fBreak )
		{
			// 他のスレッドと共有するグローバル変数に触る際には排他を行う
			EnterCriticalSection();
			if( REXDEMOGetAnyMixedPadTrigger() & (KPAD_BUTTON_B | (PAD_BUTTON_B << 16)) )
			{
				// B ボタンで次の状態へ遷移
				fBreak = TRUE;
			}
			LeaveCriticalSection();

			VIWaitForRetrace();
		}
	}

	return RESULT_OK;
}


/////////////////////////////////////////////////////////////////////
// DataSharing の処理スレッド
void* DataSharingThreadFunc( void* arg )
{
#pragma unused( arg )
//	s32 result;
//	ShareData sendData;
//	MPDSDataSet recvDataSet;
	BOOL fError;
//	s32 i;
	s32 state;

	// Wii と DS では V-Blank 周期が異なるため、
	// DS 間で行われている MP 通信の周期は Wii の V-Blank 周期に合いません。
	// そのため、Wii の V-Blank 周期に合わせて動いている描画用のメインループとは別に
	// DataSharing 用に MP 通信の周期 (MPDSStep 関数が成功する周期) で動く
	// 専用のスレッドが必要になります。

	// スレッドを作成せずに、メインループ内で MPDSTryStep 関数を利用して
	// DataSharing を行うこともできますが、
	// その場合は親子でデータをセットするタイミングが合わず、
	// 高い頻度で DataSharing が失敗します。

	fError = FALSE;

	while ( !IsQuitting() && fError == FALSE )
	{
		// 他のスレッドと共有するグローバル変数に触る際には排他を行う
		EnterCriticalSection(); // 排他開始

		state = GetState();

		switch ( state )
		{
		case STATE_LOBBY:
		case STATE_COMMUNICATION:
			LeaveCriticalSection(); // 排他終了
    		GFL_NET_HANDLE_MainProc();
    		GFL_NET_UpdateSystem( &sDSContext );
			if( GFL_NET_HANDLE_GetNumNegotiation() != 0 )
			{
				GFL_NET_HANDLE_RequestNegotiation();
			}
			
			break;

#if 0
			// DataSharing を実行中の状態の場合

			// 現在のゲーム状態から送信データを作成
			//   Wii と DS ではエンディアンが異なることに注意
			(void)NETMemSet( &sendData, 0, sizeof(sendData) );
			
			//sendData.data[_DS_HEADERNO] = _INVALID_HEADER;
			
			/*
			sendData.count = MPHToMP16( (u16)sFrameCount ); // エンディアン変換
			if ( state == STATE_LOBBY )
			{
				sendData.command = SHARECMD_NONE;
			}
			else
			{
				sendData.command = SHARECMD_EXITLOBBY;
			}
			sendData.key = MPHToMP16(ConvKPadKeyToDSPad(REXDEMOGetAnyMixedPadHold()));
			sendData.level = MPGetLinkLevel();
			*/
			// 排他を握りっぱなしでブロックしてはいけない
			LeaveCriticalSection(); // 排他終了

			// DataSharing を用いてデータの共有を行う
			//   MPDSStep 関数はデータが揃うまで長期間ブロックします。
			result = MPDSTryStep( &sDSContext, &sendData, &recvDataSet );

			// 受信データを解釈し、ゲーム状態を更新する
			{
			
				// 他のスレッドと共有するグローバル変数に触る際には排他を行う
				EnterCriticalSection(); // 排他開始
				//if( result == MP_RESULT_OK )
				{
					// 共有データの受信に成功した
					const u8* data;
					// 共有データの内容を解釈する
					for ( i = 0; i < MY_MAX_NODES+1; i++ )
					{
						data = MPDSGetData( &sDSContext, &recvDataSet, (u32)i );
						if ( data != NULL )
						{
							// i 番目の共有相手からのデータを共有できた
							(void)NETMemCpy( (void*)&sRecvData[i], data, MY_DS_LENGTH );
							//sRecvFlag[i] = TRUE;
							MPDS_PostShareData( data , (u8)i );
						}
						else
						{
							// NULL が帰ってくる場合には、いくつかのケースがあります。
							// 1. DataSharing 開始直後の空読みデータ
							// 2. 該当する AID の子機が接続していない
							// （3. 何らかの内部エラーが発生している）

							// この情報を使い、子機の接続/切断を確認することが可能です。
							// ただし、子機が自由に接続できる状態では、
							// 子機が切断した直後に別の子機が接続してきた場合に
							// それを検出することができません。
							// ゲーム開始後に新規の子機の接続を締め切るには
							// MPSetEntryFlag 関数と MPUpdateBeacon 関数を使用してください。
						}
					}
					
				}
			/*
				if ( state != GetState() )
				{
					// MPDS[Try]Step 関数の呼び出し中に状態が変化してしまった
					//   今回はどの状態でもデータ解釈に差異はないので無視します。
					//
					//   共有されたデータは、親子全てで同様に解釈されないといけませんので、
					//   処理の都合によっては、共有データの中に状態を含めておき、
					//   共有されたデータに含まれている状態に従って処理を分岐したほうがよいでしょう。
				}

				for ( i = 0; i < MY_MAX_NODES+1; i++ )
				{
					sRecvFlag[i] = FALSE;
				}

				if( result == MP_RESULT_OK )
				{
					// 共有データの受信に成功した
					const u8* data;

					// 共有データの内容を解釈する
					for ( i = 0; i < MY_MAX_NODES+1; i++ )
					{
						data = MPDSGetData( &sDSContext, &recvDataSet, (u32)i );
						if ( data != NULL )
						{
							// i 番目の共有相手からのデータを共有できた
							(void)NETMemCpy( (void*)&sRecvData[i], data, MY_DS_LENGTH );
							sRecvFlag[i] = TRUE;
						}
						else
						{
							// NULL が帰ってくる場合には、いくつかのケースがあります。
							// 1. DataSharing 開始直後の空読みデータ
							// 2. 該当する AID の子機が接続していない
							// （3. 何らかの内部エラーが発生している）

							// この情報を使い、子機の接続/切断を確認することが可能です。
							// ただし、子機が自由に接続できる状態では、
							// 子機が切断した直後に別の子機が接続してきた場合に
							// それを検出することができません。
							// ゲーム開始後に新規の子機の接続を締め切るには
							// MPSetEntryFlag 関数と MPUpdateBeacon 関数を使用してください。
						}
					}

					//
					//
					// 子機と親機でデータの共有ができたので、ゲーム内状態を更新します。
					// ここに、共有できたデータを使用してゲーム内状態を更新するロジックを置きます。
					//
					// DataSharing を利用する場合は、MPDS[Try]Step 関数から取得できたデータのみを
					// 使用してゲーム内状態を更新することで、親子の完全な同期を取ることが可能です。
					// 現在のパッド入力などのローカルの情報を直接使用すると同期が崩れますので、
					// 自分の情報であっても、必ず一度 MPDS[Try]Step 関数に渡すようにしてください。
					//
					//

					for (i = 0; i < BALL_PLAYER_MAX; ++i)
					{
						if ((i < MY_MAX_NODES+1) && sRecvFlag[i])
						{
							InputBallKey((int)i, (int)MPMPToH16((u16)sRecvData[i].key));
						}
					}
					UpdateBalls(0);

				}
				else if ( result == MP_RESULT_NO_DATA )
				{
					// MPDSTryStep 関数を使用した場合、
					// 通信状態が悪かったなどの要因によりデータが揃っていなかったときに
					// 普通に発生するエラーです。
					// MPDSStep 関数を使用した場合には発生しません。

					//
					// 親子でデータ共有ができていないということなので、
					// ゲーム内状態を更新せずに次のフレームまで待ちます。
					//
					// この状態が続く期間は通信参加者により異なる場合がありますので、
					// 速度データを用いて座標データを更新する、とか
					// 乱数値を読み込む、といった、プログラムの内部状態を
					// 変更する操作を行ってはいけません。
					//
				}
				else
				{
					// エラー
					OSReport("MPDSStep failed: %08x\n", result);
					fError = TRUE;
				}
			*/
				LeaveCriticalSection(); // 排他終了
			}
			
			break;
#endif
		default:
			// 通信状態ではない場合
			LeaveCriticalSection(); // 排他終了

			// 状態が変わるまでブロックして待つ
			(void)WaitForStateChange();
			break;
		}

		// 子機が同期している仮想 VBlank 周期にループを同期させる
		// これを行うことで、通信のレイテンシが低くなり、
		// 子機に届く送信データが親機の最新に近い状態を反映するようになる
		(void)MPWaitPseudoVBlank();
	}

	return NULL;
}


/////////////////////////////////////////////////////////////////////
// ビーコン更新用スレッド
void* UpdateGameInfoThreadFunc( void* arg )
{
#pragma unused( arg )
	while ( !IsQuitting() )
	{
		switch ( GetState() )
		{
		case STATE_LOBBY:
		case STATE_COMMUNICATION:
			// 通信中は定期的にビーコン内容を更新する
			sMyGameInfo.connectNum = MPCountPopulation(MPGetConnectedAIDs());
			//sMyGameInfo.periodicalCount++;
			(void)MPSetUserGameInfo(&sMyGameInfo, sizeof(sMyGameInfo));

			// 設定した UserGameInfo をビーコンに反映
			//   更新されたビーコンが送信されるまで長期間ブロックします。
			(void)MPUpdateBeacon();
			break;

		default:
			(void)WaitForStateChange();
			break;
		}
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////
// イベント通知用コールバック
void DataSharingCallback( s32 type, MPPortCallbackInfo* info )
{
	// MPDS ライブラリが MP ライブラリからの通知を forward します。
	// このデモでは接続してきた子機の詳細な情報を取得するために
	// コールバックを利用していますが、DataSharing を利用するだけであれば
	// 必ずしもコールバック関数を用意する必要はありません。

	// この関数は、MP ライブラリのコールバック用スレッドから呼び出されます。
	// 割り込みコールバックではないため、スレッド同期用の関数を呼び出すことも
	// 可能ですが、あまり長時間ブロックしてしまうと、MP 通信に影響を与えます。
	// 排他を行うための最低限のブロックにとどめてください。
	// 場合によっては、OSMessageQueue などを利用した、ブロックしないような
	// データの受け渡し方法も検討してください。

	u32 aid;

	switch ( type )
	{
	case MP_PORT_CB_TYPE_CONNECTED:
		// 子機が接続した
		aid = info->connected.fromAid;
		OSReport("ConnectChild[%d]\n",aid);
		EnterCriticalSection();
		(void)NETMemCpy(sConnectedMacAddress[aid], info->connected.macAddress, MP_SIZE_MACADDRESS);
		(void)NETMemSet(sConnectedNickName[aid], 0, sizeof(sConnectedNickName[0]));
		/* DS からのデータはエンディアンが異なることに注意 */
		NETSwapAndCopyMemory16(sConnectedNickName[aid], info->connected.ssidUserData, info->connected.ssidUserDataLength);
		sConnectedFlag[aid] = TRUE;
		{
			PLAYER_STATE	*plData = MPDS_GetPlayerState( aid );
			plData->isValid_ = FALSE;
		}
		LeaveCriticalSection();
		
		break;

	case MP_PORT_CB_TYPE_DISCONNECTED:
		// 子機が切断した
		aid = info->disconnected.fromAid;

		EnterCriticalSection();
		sConnectedFlag[aid] = FALSE;
		(void)NETMemSet(sConnectedMacAddress[aid], 0, MP_SIZE_MACADDRESS);
		(void)NETMemSet(sConnectedNickName[aid], 0, sizeof(sConnectedNickName[0]));
		
		OSReport("DisConnectChild[%d]\n",aid);
		GFL_NET_InitHandle((int)aid);
		
		LeaveCriticalSection();

		break;
		
	case MP_PORT_CB_TYPE_STARTUP:
	case MP_PORT_CB_TYPE_CLEANUP:
	case MPDS_PORT_CB_TYPE_DATASET_RECEIVED:
	default:
		// 何もすることはありません
		break;
	}
}

void IndicationCallback( s32 type, MPIndicationInfo* info )
{
	// MP ライブラリから送られる各種の一般的な通知を受け取るためのコールバックです。
	// Fatal Error のハンドリングは、ライブラリ関数の返り値の処理で行うことも
	// できますので、必ずしもコールバック関数を用意する必要はありません。

	// この関数は、MP ライブラリのコールバック用スレッドから呼び出されます。
	// 割り込みコールバックではないため、スレッド同期用の関数を呼び出すことも
	// 可能ですが、あまり長時間ブロックしてしまうと、MP 通信に影響を与えます。
	// 排他を行うための最低限のブロックにとどめてください。
	// 場合によっては、OSMessageQueue などを利用した、ブロックしないような
	// データの受け渡し方法も検討してください。

	switch ( type )
	{
	case MP_INDICATION_CB_TYPE_FATAL_ERROR:
		// 致命的エラーの発生

		// メモリ確保エラーなど、内部で復旧不可能なエラーが発生した場合に通知されます。
		OSReport("Fatal Error occurred: error=%d\n", info->value);

		break;

	case MP_INDICATION_CB_TYPE_PSEUDO_VBLANK:
		// 仮想 VBlank 通知

		// MPConfig.mode に MP_MODE_PARENT | MP_MODE_ENABLE_PSEUDO_VBLANK_INDICATION を
		// 指定することで、子機が同期している仮想 VBlank のタイミングでの通知を
		// 受けることができるようになります。
		// このデモでは使用していません。

		break;

	default:
		// 何もすることはありません
		break;
	}
}

/*===========================================================================*/

void* mpAlloc( u32 size )
{
	OSReport("allocated %d bytes\n", size);
	return MEMAllocFromExpHeapEx( sUserHeap, size, 32 );
}

void mpFree( void* ptr )
{
	MEMFreeToExpHeap( sUserHeap, ptr );
}

/*===========================================================================*/

// DS の PAD 定数
#define DSPAD_BUTTON_A			0x0001 // A
#define DSPAD_BUTTON_B			0x0002 // B
#define DSPAD_BUTTON_SELECT	   0x0004 // SELECT
#define DSPAD_BUTTON_START		0x0008 // START
#define DSPAD_KEY_RIGHT		   0x0010 // RIGHT of cross key
#define DSPAD_KEY_LEFT			0x0020 // LEFT  of cross key
#define DSPAD_KEY_UP			  0x0040 // UP	of cross key
#define DSPAD_KEY_DOWN			0x0080 // DOWN  of cross key
#define DSPAD_BUTTON_R			0x0100 // R
#define DSPAD_BUTTON_L			0x0200 // L
#define DSPAD_BUTTON_X			0x0400 // X
#define DSPAD_BUTTON_Y			0x0800 // Y

u16 ConvKPadKeyToDSPad(u32 mixedKey)
{
	u16 key = 0;

	if ( mixedKey & (KPAD_BUTTON_LEFT | (PAD_BUTTON_LEFT << 16)) )
	{
		key |= DSPAD_KEY_LEFT;
	}
	if ( mixedKey & (KPAD_BUTTON_RIGHT | (PAD_BUTTON_RIGHT << 16)) )
	{
		key |= DSPAD_KEY_RIGHT;
	}
	if ( mixedKey & (KPAD_BUTTON_UP | (PAD_BUTTON_UP << 16)) )
	{
		key |= DSPAD_KEY_UP;
	}
	if ( mixedKey & (KPAD_BUTTON_DOWN | (PAD_BUTTON_DOWN << 16)) )
	{
		key |= DSPAD_KEY_DOWN;
	}
	if( mixedKey & (KPAD_BUTTON_A | (PAD_BUTTON_A << 16)) )
	{
		key |= DSPAD_BUTTON_A;
	}
	if( mixedKey & (KPAD_BUTTON_B | (PAD_BUTTON_B << 16)) )
	{
		key |= DSPAD_BUTTON_A;
	}
	if( mixedKey & (KPAD_BUTTON_1 | (PAD_BUTTON_X << 16)) )
	{
		key |= DSPAD_BUTTON_X;
	}
	if( mixedKey & (KPAD_BUTTON_2 | (PAD_BUTTON_Y << 16)) )
	{
		key |= DSPAD_BUTTON_Y;
	}
	if( mixedKey & (KPAD_BUTTON_PLUS | (PAD_TRIGGER_R << 16)) )
	{
		key |= DSPAD_BUTTON_R;
	}
	if( mixedKey & (KPAD_BUTTON_MINUS | (PAD_TRIGGER_L << 16)) )
	{
		key |= DSPAD_BUTTON_L;
	}
	if( mixedKey & (KPAD_BUTTON_HOME | (PAD_BUTTON_START << 16)) )
	{
		key |= DSPAD_BUTTON_START;
	}

	return key;
}
