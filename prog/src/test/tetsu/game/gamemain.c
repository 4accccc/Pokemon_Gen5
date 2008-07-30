//============================================================================================
/**
 * @file	gamemain.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"
#include "include/system/gfl_use.h"
#include "include/net/network_define.h"
#include "textprint.h"

#include "setup.h"
#include "game_net.h"
#include "game_cont.h"

void	GameBoot( HEAPID heapID );
void	GameEnd( void );
BOOL	GameMain( void );

static void _initRecvBuffer( void );
static void _sendGamePlay( void );
#define CAMERA_MOVE_SPEED	(0x0100)
//------------------------------------------------------------------
/**
 * @brief	仮データ
 */
//------------------------------------------------------------------
static const PLAYER_STATUS	status1 = { {"Player1"},1000, 100, 1000, 100, 50, 50, 50, 50, 50 };
static const PLAYER_STATUS	status2 = { {"Player2"},1000, 100, 1000, 100, 50, 50, 50, 50, 50 };
static const PLAYER_STATUS	status3 = { {"Player3"},1000, 100, 1000, 100, 50, 50, 50, 50, 50 };
static const PLAYER_STATUS	status4 = { {"Player4"},1000, 100, 1000, 100, 50, 50, 50, 50, 50 };
static const PLAYER_STATUS	status5 = { {"Player5"},1000, 100, 1000, 100, 50, 50, 50, 50, 50 };
static const PLAYER_STATUS	status6 = { {"Player6"},1000, 100, 1000, 100, 50, 50, 50, 50, 50 };
static const PLAYER_STATUS	status7 = { {"Player7"},1000, 100, 1000, 100, 50, 50, 50, 50, 50 };
static const PLAYER_STATUS	status8 = { {"Player8"},1000, 100, 1000, 100, 50, 50, 50, 50, 50 };

static const PLAYER_STATUS*	testStatus[] = { 
	&status1, &status2, &status3, &status4, &status5, &status6, &status7, &status8,
};

//============================================================================================
//
//
//	メインコントロール
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	構造体定義
 */
//------------------------------------------------------------------
typedef struct {
	HEAPID			heapID;
	GAME_SYSTEM*	gs;
	GAME_CONTROL*	gc; 

	int				myNetID;
	int				seq;
	int				mainContSeq;
	int				timer;
//-------------
	int				totalPlayerCount;
	int				playNetID;
//-------------
}GAME_WORK;

//------------------------------------------------------------------
/**
 * @brief	ローカル宣言
 */
//------------------------------------------------------------------
static BOOL GameEndCheck( int cont );

GAME_WORK* gw;

//------------------------------------------------------------------
/**
 * @brief	ワークの確保と破棄
 */
//------------------------------------------------------------------
void	GameBoot( HEAPID heapID )
{
	gw = GFL_HEAP_AllocClearMemory( heapID, sizeof(GAME_WORK) );
	gw->heapID = heapID;

	GFL_UI_TP_Init( gw->heapID );
}

void	GameEnd( void )
{
	GFL_UI_TP_Exit();

	GFL_HEAP_FreeMemory( gw );
}

//------------------------------------------------------------------
/**
 * @brief	メイン
 */
static void Debug_Regenerate( void );
#define MAPOFFS 0//((64-3) * 8 * FX32_ONE)
//------------------------------------------------------------------
BOOL	GameMain( void )
{
	GAME_CONT_KEYDATA key;
	BOOL return_flag = FALSE;
	int i;

	gw->timer++;
	GFL_UI_TP_Main();

	switch( gw->seq ){

	case 0:
		//通信初期化
		InitGameNet();
		_initRecvBuffer();
		gw->seq++;
		break;

	case 1:
		if( ConnectGameNet() == TRUE ){
			gw->myNetID = ((int)GetNetID() -1 );
			gw->playNetID = gw->myNetID;
			gw->seq++;
		}
		break;

	case 2:
		//システムセットアップ
		gw->gs = SetupGameSystem( gw->heapID );

		//ゲームコントローラ設定
		{
			int i;
			GAME_CONT_SETUP* gcSetup = GFL_HEAP_AllocClearMemory
										( gw->heapID, sizeof(GAME_CONT_SETUP) );
			VecFx32			startTrans1 = { -MAPOFFS, 0, -MAPOFFS };
			VecFx32			startTrans2 = { MAPOFFS, 0, MAPOFFS };
			int				pcount = 0;

			for( i=0; i<TEAM_PLAYER_COUNT_MAX; i++ ){
				gcSetup->team[0].player[i].netID = pcount;
				gcSetup->team[0].player[i].objID = pcount;
				gcSetup->team[0].player[i].ps = (PLAYER_STATUS*)testStatus[pcount];
				pcount++;
			}
			gcSetup->team[0].playerCount = i;
			gcSetup->team[0].startTrans = startTrans1;

			for( i=0; i<TEAM_PLAYER_COUNT_MAX; i++ ){
				gcSetup->team[1].player[i].netID = pcount;
				gcSetup->team[1].player[i].objID = pcount;
				gcSetup->team[1].player[i].ps = (PLAYER_STATUS*)testStatus[pcount];
				pcount++;
			}
			gcSetup->team[1].playerCount = i;
			gcSetup->team[1].startTrans = startTrans2;

			gcSetup->teamCount = 2;
			gcSetup->myID = gw->myNetID;
			gw->gc = AddGameControl( gw->gs, gcSetup, gw->heapID );

			GFL_HEAP_FreeMemory(gcSetup); 

			gw->totalPlayerCount = pcount;
		}
		gw->seq++;
		gw->mainContSeq = 0;
		break;

	case 3:
		//MainGameNet();
		{
			key.keyTrg	= GFL_UI_KEY_GetTrg();
			key.keyCont	= GFL_UI_KEY_GetCont();

			key.tpTrg  = GFL_UI_TP_GetPointTrg( &key.tpx, &key.tpy );
			if( key.tpTrg == FALSE ){
				key.tpCont = GFL_UI_TP_GetPointCont( &key.tpx, &key.tpy );
			} else {
				key.tpCont = FALSE;
			}
			SetGameControlKey( gw->gc, &key );
		}

		switch( gw->mainContSeq ){
		case 0:
			GFUser_VIntr_ResetVblankCounter();
			//---------------
			{
				if( key.keyTrg & PAD_BUTTON_SELECT ){
					gw->playNetID++;
					gw->playNetID %= gw->totalPlayerCount;
					ChangeControlPlayer( gw->gc, gw->playNetID );
				}
			}
			//---------------
			if( GameEndCheck( key.keyCont ) == TRUE ){
				gw->seq++;
			}
			MainGameControl( gw->gc );
			MainGameSystem( gw->gs );
			{
				//キー交換通信
				_sendGamePlay();
			}
			ResetGameControl( gw->gc );
			ResetGameControlKey( gw->gc );
			gw->mainContSeq++;
			break;
		case 1:
			key.keyTrg	= GFL_UI_KEY_GetTrg();
			key.keyCont	= GFL_UI_KEY_GetCont();

			key.tpCont = GFL_UI_TP_GetPointCont( &key.tpx, &key.tpy );
			key.tpTrg  = GFL_UI_TP_GetPointTrg( &key.tpx, &key.tpy );

			SetGameControlKey( gw->gc, &key );
			//---------------
			{
				if( key.keyTrg & PAD_BUTTON_SELECT ){
					gw->playNetID++;
					gw->playNetID %= gw->totalPlayerCount;
					ChangeControlPlayer( gw->gc, gw->playNetID );
				}
			}
			//---------------

			if( GFUser_VIntr_GetVblankCounter() > 1 ){
				gw->mainContSeq = 0;
			}
			break;
		}
		break;

	case 4:
		RemoveGameControl( gw->gc );
		RemoveGameSystem( gw->gs );
		gw->seq++;
		break;

	case 5:
		if( ExitGameNet() == TRUE ){
			gw->seq++;
		}
		break;

	case 6:
		return_flag = TRUE;
		break;
	}
	return return_flag;
}

//------------------------------------------------------------------
/**
 * @brief	終了チェック
 */
//------------------------------------------------------------------
static BOOL GameEndCheck( int cont )
{
	int resetCont = PAD_BUTTON_L | PAD_BUTTON_R | PAD_BUTTON_START;

	if( (cont & resetCont ) == resetCont ){
		return TRUE;
	} else {
		return FALSE;
	}
}


//============================================================================================
//
//
//	通信関数
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	受信ワーク初期化
 */
//------------------------------------------------------------------
static void _initRecvBuffer( void )
{
}

//------------------------------------------------------------------
/**
 * @brief	受信関数
 */
//------------------------------------------------------------------
// ローカル通信コマンドの定義
enum _gameCommand_e {
	_GAME_COM_PLAY = GFL_NET_CMD_COMMAND_MAX,
};

//------------------------------------------------------------------
// _GAME_COM_PLAY　ゲーム情報送受信

static void _sendGamePlay( void )
{
	GAME_NETWORK_PLAYDATA gnd;
#ifdef NET_WORK_ON
	GetGameNetWorkPlayData( gw->gc, gw->playNetID, &gnd );
	SendGameNet( _GAME_COM_PLAY, &gnd );
#else
	//GetGameNetWorkPlayData( gw->gc, gw->myNetID, &gnd );
	//SetGameNetWorkPlayData( gw->gc, gw->myNetID, &gnd );
	GetGameNetWorkPlayData( gw->gc, gw->playNetID, &gnd );
	SetGameNetWorkPlayData( gw->gc, gw->playNetID, &gnd );
#endif
}

static void _recvGamePlay
	(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
	GAME_NETWORK_PLAYDATA* pGnd = (GAME_NETWORK_PLAYDATA*)pData;
	int	workp = netID-1;	//DS通信は親=0（内部隠し構造）の1orgin

    if( GFL_NET_IsParentHandle(pNetHandle) == FALSE ){
		SetGameNetWorkPlayData( gw->gc, workp, pGnd );
    }
}

//------------------------------------------------------------------
// ローカル通信テーブル
const NetRecvFuncTable _CommPacketTbl[] = {
    { _recvGamePlay, GFL_NET_COMMAND_SIZE(sizeof(GAME_NETWORK_PLAYDATA)), NULL },
};



//------------------------------------------------------------------
/**
 * @brief	デバッグ
 */
//------------------------------------------------------------------

