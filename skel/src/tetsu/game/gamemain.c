//============================================================================================
/**
 * @file	gamemain.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"
#include "textprint.h"

#include "setup.h"
#include "game_net.h"
#include "player_cont.h"
#include "camera_cont.h"
#include "text_cont.h"
#include "skill_cont.h"

void	GameBoot( HEAPID heapID );
void	GameEnd( void );
BOOL	GameMain( void );

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
	u16 trg;
	u16 cont;
}COMM_KEY;

typedef struct {
	HEAPID					heapID;
	GAME_SYSTEM*			gs;
	CAMERA_CONTROL*			cc; 
	SKILL_CONTROL*			sc; 
	PLAYER_CONTROL*			pc[PLAYER_SETUP_NUM]; 
	STATUSWIN_CONTROL*		swc[PLAYER_SETUP_NUM]; 
	int						seq;
	int						playerSide;
	int						timer;

	COMM_KEY				commKey[PLAYER_SETUP_NUM]; 

}GAME_WORK;

//------------------------------------------------------------------
/**
 * @brief	ローカル宣言
 */
//------------------------------------------------------------------
static BOOL GameEndCheck( void );
static void ControlKey( void );
static void StatusWinUpdate( void );

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
}

void	GameEnd( void )
{
	GFL_HEAP_FreeMemory( gw );
}

//------------------------------------------------------------------
/**
 * @brief	メイン
 */
static void Debug_Regenerate( void );
//------------------------------------------------------------------
BOOL	GameMain( void )
{
	BOOL return_flag = FALSE;
	int i;
	gw->timer++;

	switch( gw->seq ){

	case 0:
		//通信初期化
		InitGameNet();

		gw->gs = SetupGameSystem( gw->heapID );
		for( i=0; i<PLAYER_SETUP_NUM; i++ ){ 
			gw->pc[i] = AddPlayerControl( gw->gs, G3DSCOBJ_PLAYER1+i, gw->heapID );
			SetPlayerStatus( gw->pc[i], testStatus[i] ); 
			gw->swc[i] = AddStatusWinControl(	GetPlayerStatusPointer( gw->pc[i] ),
												Get_GS_BmpWin( gw->gs, G2DBMPWIN_P1+i ), 
												gw->heapID );
		}
		gw->cc = AddCameraControl( gw->gs, G3DSCOBJ_PLAYER1, gw->heapID );
		{
			gw->sc = AddSkillControl( gw->gs, gw->heapID );
			for( i=0; i<PLAYER_SETUP_NUM; i++ ){ 
				AddSkillCheckPlayer( gw->sc, gw->pc[i] );
			}
		}
		gw->playerSide = 0;

		gw->seq++;
		break;

	case 1:
		if( ConnectGameNet() == TRUE ){
			gw->seq++;
		}
		break;

	case 2:
		//MainGameNet();

		if( GameEndCheck() == TRUE ){
			gw->seq++;
		}
		ControlKey();

		for( i=0; i<PLAYER_SETUP_NUM; i++ ){ 
			MainPlayerControl( gw->pc[i] );
			SetSkillControlCommand( gw->sc, gw->pc[i], GetPlayerSkillCommand( gw->pc[i] ));
			ResetPlayerSkillCommand( gw->pc[i] );
			MainStatusWinControl( gw->swc[i] );
		}
		MainCameraControl( gw->cc );
		MainSkillControl( gw->sc );
		MainGameSystemPref( gw->gs );
		MainGameSystemAfter( gw->gs );
		StatusWinUpdate();
		break;

	case 3:
		if( ExitGameNet() == TRUE ){
			gw->seq++;
		}
		break;

	case 4:
		RemoveSkillControl( gw->sc );
		RemoveCameraControl( gw->cc );
		for( i=0; i<PLAYER_SETUP_NUM; i++ ){ 
			RemoveStatusWinControl( gw->swc[i] );
			RemovePlayerControl( gw->pc[i] );
		}
		RemoveGameSystem( gw->gs );
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
static BOOL GameEndCheck( void )
{
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){
		return TRUE;
	} else {
		return FALSE;
	}
}

//------------------------------------------------------------------
/**
 * @brief	コントロール
 */
static void _sendGameKey( u16 trg, u16 cont );
//------------------------------------------------------------------
static void ControlKey( void )
{
	int	trg = GFL_UI_KEY_GetTrg();
	int	cont = GFL_UI_KEY_GetCont();
	int	speedupFlag = FALSE;
	PLAYER_CONTROL*	pc; 

	_sendGameKey( trg, cont );
	pc = gw->pc[gw->playerSide];

	{
		//カメラ操作
		u16 direction;
		
		if(( cont & PAD_BUTTON_L )&&( cont & PAD_BUTTON_R )){	//リセット（プレーヤー前方）
			//GetPlayerDirection( pc, &direction );
			//SetPlayerControlDirection( pc, &direction );
			//SetCameraControlDirection( gw->cc, &direction );
		} else if( cont & PAD_BUTTON_L ){	//左移動
			GetPlayerControlDirection( pc, &direction );
			direction += CAMERA_MOVE_SPEED;
			SetPlayerControlDirection( pc, &direction );
			SetCameraControlDirection( gw->cc, &direction );
		} else if( cont & PAD_BUTTON_R ){	//右移動
			GetPlayerControlDirection( pc, &direction );
			direction -= CAMERA_MOVE_SPEED;
			SetPlayerControlDirection( pc, &direction );
			SetCameraControlDirection( gw->cc, &direction );
		}
	}
	//武器の変更
	if( trg & PAD_BUTTON_Y ){
		SetPlayerControlCommand( pc, PCC_WEPONCHANGE );
		return;
	}
	//攻撃
	if( trg & PAD_BUTTON_A ){
		SetPlayerControlCommand( pc, PCC_ATTACK );
		return;
	}
	//置く、拾う
	if( trg & PAD_BUTTON_B ){
		SetPlayerControlCommand( pc, PCC_PUTON );
		return;
	}
	//座る
	if( cont & PAD_BUTTON_X ){
		SetPlayerControlCommand( pc, PCC_SIT );
		return;
	}
	//速度チェック
//	if( cont & PAD_BUTTON_B ){
		speedupFlag = TRUE;
//	}
	//移動
	if( cont & PAD_KEY_UP ){
		if( cont & PAD_KEY_LEFT ){
			if( speedupFlag == TRUE ){
				SetPlayerControlCommand( pc, PCC_RUN_FRONT_LEFT );
			} else {
				SetPlayerControlCommand( pc, PCC_WALK_FRONT );
			}
		} else if( cont & PAD_KEY_RIGHT ){
			if( speedupFlag == TRUE ){
				SetPlayerControlCommand( pc, PCC_RUN_FRONT_RIGHT );
			} else {
				SetPlayerControlCommand( pc, PCC_WALK_FRONT_RIGHT );
			}
		} else {
			if( speedupFlag == TRUE ){
				SetPlayerControlCommand( pc, PCC_RUN_FRONT );
			} else {
				SetPlayerControlCommand( pc, PCC_WALK_FRONT );
			}
		}
		return;
	}
	if( cont & PAD_KEY_DOWN ){
		if( cont & PAD_KEY_LEFT ){
			if( speedupFlag == TRUE ){
				SetPlayerControlCommand( pc, PCC_RUN_BACK_LEFT );
			} else {
				SetPlayerControlCommand( pc, PCC_WALK_BACK_LEFT );
			}
		} else if( cont & PAD_KEY_RIGHT ){
			if( speedupFlag == TRUE ){
				SetPlayerControlCommand( pc, PCC_RUN_BACK_RIGHT );
			} else {
				SetPlayerControlCommand( pc, PCC_WALK_BACK_RIGHT );
			}
		} else {
			if( speedupFlag == TRUE ){
				SetPlayerControlCommand( pc, PCC_RUN_BACK );
			} else {
				SetPlayerControlCommand( pc, PCC_WALK_BACK );
			}
		}
		return;
	}
	if( cont & PAD_KEY_LEFT ){
		if( speedupFlag == TRUE ){
			SetPlayerControlCommand( pc, PCC_RUN_LEFT );
		} else {
			SetPlayerControlCommand( pc, PCC_WALK_LEFT );
		}
		return;
	}
	if( cont & PAD_KEY_RIGHT ){
		if( speedupFlag == TRUE ){
			SetPlayerControlCommand( pc, PCC_RUN_RIGHT );
		} else {
			SetPlayerControlCommand( pc, PCC_WALK_RIGHT );
		}
		return;
	}
	SetPlayerControlCommand( pc, PCC_STAY );

	//プレーヤー変更
	if( trg & PAD_BUTTON_SELECT ){
		int ID;
		u16 direction;

		gw->playerSide++;
		if( gw->playerSide >= PLAYER_SETUP_NUM ){
			gw->playerSide = 0;
		}
		pc = gw->pc[gw->playerSide];
		ID = G3DSCOBJ_PLAYER1 + gw->playerSide;

		//カメラ方向をプレーヤーに合わせて再設定
		SetCameraControlTargetID( gw->cc, &ID );
		GetPlayerControlDirection( pc, &direction );
		SetCameraControlDirection( gw->cc, &direction );
	}
}

//------------------------------------------------------------------
/**
 * @brief	ステータスウインドウのアップデート
 *			書き換えタイミングやその他の処理の重複（処理落ち）を避けるための管理
 */
//------------------------------------------------------------------
static void StatusWinUpdate( void )
{
	int win = gw->timer % PLAYER_SETUP_NUM;

	SetStatusWinReload( gw->swc[win] );
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
 * @brief	受信関数
 */
//------------------------------------------------------------------
// ローカル通信コマンドの定義
enum _gameCommand_e {
	_GAME_COM_KEY = GFL_NET_CMD_COMMAND_MAX,
};

//------------------------------------------------------------------
// 位置情報受信
typedef struct {
    u16 keyTrg;
    u16 keyCont;
} COMMWORK_KEY;

static void _sendGameKey( u16 trg, u16 cont )
{
	COMMWORK_KEY commData;
	commData.keyTrg = 0x100;//trg;
	commData.keyCont = 0x200;//cont;

	SendGameNet( _GAME_COM_KEY, &commData );
}

static void _recvGameKey
	(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
	COMMWORK_KEY* commDataP = (COMMWORK_KEY*)pData;
	int	workp = netID-1;	//DS通信は親=0（内部隠し構造）の1orgin

	gw->commKey[workp].trg = commDataP->keyTrg;
	gw->commKey[workp].cont = commDataP->keyCont;
	OS_TPrintf(" netID = %d, keyTrg = %x, keyCont = %x\n", 
				netID, commDataP->keyTrg, commDataP->keyCont );
}

//------------------------------------------------------------------
// ローカル通信テーブル
const NetRecvFuncTable _CommPacketTbl[] = {
    { _recvGameKey, GFL_NET_COMMAND_SIZE(sizeof(COMMWORK_KEY)), NULL },
};



//------------------------------------------------------------------
/**
 * @brief	デバッグ
 */
//------------------------------------------------------------------

