//============================================================================================
/**
 * @file	game_cont.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"

#include "setup.h"

#include "game_net.h"
#include "game_cont.h"

#include "mouse_event.h"

#include "fld_act.h"
#include "camera_cont.h"
#include "include/net/network_define.h"

//============================================================================================
//
//
//	ゲームコントロール
//
//
//============================================================================================
static void ControlKey( PLAYER_CONTROL* pc, GAME_CONTROL* gc );
//------------------------------------------------------------------
/**
 * @brief	構造体定義
 */
//------------------------------------------------------------------
#define TEAM_COUNT_MAX	(2) 

struct _GAME_CONTROL {
	HEAPID					heapID;
	GAME_SYSTEM*			gs;
	int						seq;

	FLD_ACTSYS*				fldActSys;
	MOUSE_EVENT_SYS*		mes;

	PLAYER_CONTROL*			myPc;
	TEAM_CONTROL*			myTc;
	int						teamCount;
	GAME_NETWORK_PLAYDATA	gnd[PLAYER_SETUP_NUM]; 

	TEAM_CONTROL*			tc[TEAM_COUNT_MAX]; 
	CAMERA_CONTROL*			cc; 

	BOOL					onGameFlag;
	int						time;

	GAME_CONT_KEYDATA		key;
	int						tp_blank;
};

//------------------------------------------------------------------
/**
 * @brief	ゲームコントロール起動
 */
//------------------------------------------------------------------
GAME_CONTROL* AddGameControl( GAME_SYSTEM* gs, GAME_CONT_SETUP* setup, HEAPID heapID )
{
	int i, j, pcID;
	GAME_CONTROL* gc = GFL_HEAP_AllocClearMemory( heapID, sizeof(GAME_CONTROL) );
	PLAYER_CONTROL* pc;

	gc->heapID = heapID;
	gc->gs = gs;
	gc->seq = 0;
	gc->myPc = NULL;
	gc->time = 0;
	gc->tp_blank = 0;

	//アクターシステム作成
	gc->fldActSys = CreateFieldActSys( gc->gs, gc->heapID );

	//チームコントローラ登録
	for( i=0; i<setup->teamCount; i++ ){
		gc->tc[i] = AddTeamControl( gc->gs, gc->heapID );
		for( j=0; j<setup->team[i].playerCount; j++ ){
			pcID = RegistPlayer(	gc->tc[i], 
									setup->team[i].player[j].ps, 
									setup->team[i].player[j].objID, 
									setup->team[i].player[j].netID, 
									&setup->team[i].startTrans );
			pc = GetTeamPlayerControl( gc->tc[i], pcID );

			if( setup->team[i].player[j].objID == setup->myID ){
				gc->myPc = pc;
				gc->myTc = gc->tc[i];
			}
		}
	}
	gc->teamCount = setup->teamCount;
	GF_ASSERT( gc->myPc != NULL );

	//カメラコントローラ登録
	gc->cc = AddCameraControl( gc->gs, gc->heapID );

	//マウス（タッチペン）コントローラ登録
	{
		gc->mes = InitMouseEvent( gs, heapID );
		StartMouseEvent( gc->mes );
	}
	gc->onGameFlag = FALSE;

	return gc;
}

//------------------------------------------------------------------
/**
 * @brief	ゲームコントロール終了
 */
//------------------------------------------------------------------
void RemoveGameControl( GAME_CONTROL* gc )
{
	int i;

	ExitMouseEvent( gc->mes );

	RemoveCameraControl( gc->cc );

	for( i=0; i<gc->teamCount; i++ ){
		RemoveTeamControl( gc->tc[i] );
	}
	DeleteFieldActSys( gc->fldActSys );

	GFL_HEAP_FreeMemory( gc ); 
}

//------------------------------------------------------------------
/**
 * @brief	ゲームキーデータ設定
 */
//------------------------------------------------------------------
void SetGameControlKey( GAME_CONTROL* gc, GAME_CONT_KEYDATA* data )
{
	if( data->tpTrg == TRUE ){
		gc->key.tpTrg = TRUE;
		gc->key.tpx	= data->tpx;
		gc->key.tpy	= data->tpy;
		return;
	}
	gc->key.tpCont = data->tpCont;
	gc->key.tpx	= data->tpx;
	gc->key.tpy	= data->tpy;
}

void ResetGameControlKey( GAME_CONTROL* gc )
{
	gc->key.keyTrg	= 0;
	gc->key.keyCont	= 0;
	gc->key.tpTrg	= FALSE;
	gc->key.tpCont	= FALSE;
	gc->key.tpx		= 0;
	gc->key.tpy		= 0;
}

//------------------------------------------------------------------
/**
 * @brief	ゲームメインコントロール
 */
typedef struct {
	GAME_CONTROL*	gc;
	TEAM_CONTROL*	tc;
	CAMERA_CONTROL*	cc;
	int				tID;
}MAINCONT_CALLBACK_WORK;

static void	MainGameControlSummonCallBack( TEAM_CONTROL* tc, int summonID, int num, void* work );
static void	MainGameControlPlayerCallBack( PLAYER_CONTROL* pc, int num, void* work );
//------------------------------------------------------------------
void MainGameControl( GAME_CONTROL* gc )
{
	int i;

	//OS_Printf("myPcID = %x, ", GetPlayerNetID( gc->myPc ) );

	//チーム別メイン処理
	{
		MAINCONT_CALLBACK_WORK* mccw = GFL_HEAP_AllocClearMemory
									( gc->heapID, sizeof(MAINCONT_CALLBACK_WORK) );
		mccw->gc = gc;
		mccw->cc = gc->cc;

		for( i=0; i<gc->teamCount; i++ ){
			MainTeamControl( gc->tc[i], gc->onGameFlag );
			mccw->tc = gc->tc[i];
			mccw->tID = i;
			ProcessingAllTeamSummonObject( gc->tc[i], MainGameControlSummonCallBack, (void*)mccw );
			ProcessingAllTeamPlayer( gc->tc[i], MainGameControlPlayerCallBack, (void*)mccw );
		}
		GFL_HEAP_FreeMemory( mccw );
	}
	//カメラ反映
	{
		VecFx32	trans;
		u16		direction;

		GetPlayerControlTrans( gc->myPc, &trans );
		SetCameraControlTrans( gc->cc, &trans );
		GetPlayerControlDirection( gc->myPc, &direction );
		SetCameraControlDirection( gc->cc, &direction );
	}
	MainCameraControl( gc->cc );
	MainFieldActSys( gc->fldActSys );
}

//------------------------------------------------------------------
static void	MainGameControlSummonCallBack( TEAM_CONTROL* tc, int summonID, int num, void* work )
{
	MAINCONT_CALLBACK_WORK* mccw = (MAINCONT_CALLBACK_WORK*)work;
	VecFx32 trans;
	BOOL	cullingFlag;

	GetSummonObjectTrans( tc, summonID, &trans );
//	//カメラとのスカラー値によるカリング
//	cullingFlag = CullingCameraScalar( mccw->cc, &trans, 0x0000 );
	//カメラ視界によるカリング
	cullingFlag = CullingCameraBitween( mccw->cc, &trans, 0x2000 );

	SetSummonObjectDrawSW( tc, summonID, &cullingFlag );
}

static void	MainGameControlPlayerCallBack( PLAYER_CONTROL* pc, int num, void* work )
{
	MAINCONT_CALLBACK_WORK* mccw = (MAINCONT_CALLBACK_WORK*)work;
	VecFx32 setTrans;
	int		netID;

	netID = GetPlayerNetID( pc );

	if( pc == mccw->gc->myPc ){
		ControlKey( pc, mccw->gc );
		MainPlayerControl( pc );
	} else {
#ifdef NET_WORK_ON
		MainNetWorkPlayerControl( pc, &mccw->gc->gnd[netID].psn );
#else
		MainPlayerControl( pc );
#endif
	}
}

//------------------------------------------------------------------
/**
 * @brief	ゲームメインコントロール後のフラグリセット処理
 */
static void	ResetGameControlSummonCallBack( TEAM_CONTROL* tc, int summonID, int num, void* work );
static void	ResetGameControlPlayerCallBack( PLAYER_CONTROL* pc, int num, void* work );
//------------------------------------------------------------------
void ResetGameControl( GAME_CONTROL* gc )
{
	int i;
	//チーム別メイン処理

	for( i=0; i<gc->teamCount; i++ ){
		ProcessingAllTeamSummonObject( gc->tc[i], ResetGameControlSummonCallBack, NULL );
		ProcessingAllTeamPlayer( gc->tc[i], ResetGameControlPlayerCallBack, NULL );
	}
}

//------------------------------------------------------------------
static void	ResetGameControlSummonCallBack( TEAM_CONTROL* tc, int summonID, int num, void* work )
{
}

static void	ResetGameControlPlayerCallBack( PLAYER_CONTROL* pc, int num, void* work )
{
	ResetPlayerSkillCommand( pc );
}

//------------------------------------------------------------------
/**
 * @brief	通信ゲームデータ情報の取得と設定
 */
//------------------------------------------------------------------
void GetGameNetWorkPlayData( GAME_CONTROL* gc, int netID, GAME_NETWORK_PLAYDATA* gnd ) 
{
	GetPlayerNetStatus( gc->myPc, &gnd->psn );
	{
		PLAYER_STATUS_NETWORK* psn = &gnd->psn;

		//OS_TPrintf("SEND trans %x,%x,%x, dir %x\n ", 
		//			psn->trans.x, psn->trans.y, psn->trans.z, psn->direction );
	}
}

void SetGameNetWorkPlayData( GAME_CONTROL* gc, int netID, GAME_NETWORK_PLAYDATA* gnd )
{
	gc->gnd[netID] = *gnd;
	{
		PLAYER_STATUS_NETWORK* psn = &gc->gnd[netID].psn;

		//OS_TPrintf("RECV trans %x,%x,%x, dir %x\n ", 
		//			psn->trans.x, psn->trans.y, psn->trans.z, psn->direction );
	}
}

void ResetAllGameNetWorkPlayData( GAME_CONTROL* gc )
{
	int i;

	for( i=0; i<PLAYER_SETUP_NUM; i++ ){
		ResetPlayerNetStatus( &gc->gnd[i].psn );
	}
}


//------------------------------------------------------------------
/**
 * @brief	コントロール
 */
//------------------------------------------------------------------
#define CAMERA_MOVE_SPEED	(0x0200*2)

static void ControlKey( PLAYER_CONTROL* pc, GAME_CONTROL* gc )
{
	BOOL contLimit = TRUE;
	VecFx32 mvDir;

	//コントロール可否判定
	if( CheckPlayerControlEnable( pc ) == TRUE ){
		contLimit = FALSE;	//制限解除
	}
	MainMouseEvent( gc->mes, gc->key.tpTrg, gc->key.tpCont, gc->key.tpx, gc->key.tpy, contLimit );

	//方向設定
	{
		u16 direction;
		
		if( CheckResetMouseEvent( gc->mes, MOUSE_EVENT_CAMERAMOVE_L ) == TRUE ){	//左移動
			GetPlayerControlDirection( pc, &direction );
			direction += CAMERA_MOVE_SPEED;
			SetPlayerControlDirection( pc, &direction );
			//return;
		} else if( CheckResetMouseEvent( gc->mes, MOUSE_EVENT_CAMERAMOVE_R ) == TRUE ){	//右移動
			GetPlayerControlDirection( pc, &direction );
			direction -= CAMERA_MOVE_SPEED;
			SetPlayerControlDirection( pc, &direction );
			//return;
		}
	}
	//ジャンプ
	if( CheckResetMouseEvent( gc->mes, MOUSE_EVENT_JUMP) == TRUE ){
		GetMousePos( gc->mes, &mvDir );
		SetPlayerMoveCommand( pc, PCC_JUMP, &mvDir );
		return;
	}
	//移動
	if( CheckResetMouseEvent( gc->mes, MOUSE_EVENT_MOVESTART) == TRUE ){
		GetMousePos( gc->mes, &mvDir );
		SetPlayerMoveCommand( pc, PCC_WALK, &mvDir );
		return;
	} else if( CheckResetMouseEvent( gc->mes, MOUSE_EVENT_MOVE) == TRUE ){
		GetMousePos( gc->mes, &mvDir );
		SetPlayerMoveCommand( pc, PCC_RUN, &mvDir );
		return;
	}
	ClearMouseEvent( gc->mes );
	SetPlayerControlCommand( pc, PCC_STAY );
}


//------------------------------------------------------------------
/**
 * @brief	チームコントローラ取得
 */
//------------------------------------------------------------------
TEAM_CONTROL*	GetTeamControl( GAME_CONTROL* gc, int teamID ) 
{
	GF_ASSERT( teamID < gc->teamCount );

	return gc->tc[teamID];
}

//------------------------------------------------------------------
/**
 * @brief	チーム数取得
 */
//------------------------------------------------------------------
int			GetTeamCount( GAME_CONTROL* gc ) 
{
	return gc->teamCount;
}

//------------------------------------------------------------------
/**
 * @brief	プレーヤー切り替え（デバッグ用）
 */
//------------------------------------------------------------------
typedef struct {
	GAME_CONTROL*		gc; 
	CAMERA_CONTROL*		cc; 
	int					playNetID;
	BOOL				result;
}CHANGE_CONTROL_WORK;

static void		ChangeControlPlayerCallBack( PLAYER_CONTROL* pc, int num, void* work ); 

void		ChangeControlPlayer( GAME_CONTROL* gc, int playNetID ) 
{
	int i;
	CHANGE_CONTROL_WORK* ccw = GFL_HEAP_AllocClearMemory( gc->heapID, sizeof(CHANGE_CONTROL_WORK) );
	BOOL sw = TRUE;

	ccw->gc = gc;
	ccw->cc = gc->cc;
	ccw->playNetID = playNetID;
	ccw->result = FALSE;

	for( i=0; i<gc->teamCount; i++ ){
		ProcessingAllTeamPlayer( gc->tc[i], ChangeControlPlayerCallBack, (void*)ccw );
		if( ccw->result == TRUE){
			gc->myTc = gc->tc[i];
			break;
		}
	}
	GFL_HEAP_FreeMemory( ccw );
}

static void		ChangeControlPlayerCallBack( PLAYER_CONTROL* pc, int num, void* work ) 
{
	CHANGE_CONTROL_WORK* ccw = (CHANGE_CONTROL_WORK*)work;
	int pcNetID	= GetPlayerNetID( pc );
	int act3dID;

	if( pcNetID == ccw->playNetID ){
		ccw->gc->myPc = pc;
		ccw->result = TRUE;
	}
}



