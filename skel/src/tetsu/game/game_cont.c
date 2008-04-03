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

#include "game_cont.h"

#include "mouse_event.h"

#include "text_cont.h"
#include "camera_cont.h"
#include "skill_cont.h"
#include "status_cont.h"

//============================================================================================
//
//
//	ゲームコントロール
//
//
//============================================================================================
static void ControlKey( PLAYER_CONTROL* pc, GAME_CONTROL* gc );
static void PutMessageWinMine( GAME_CONTROL* gc, MSGID msgID, PLAYER_CONTROL* pc );
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

	MOUSE_EVENT_SYS*		mes;

	PLAYER_CONTROL*			myPc;
	TEAM_CONTROL*			myTc;
	int						teamCount;
	GAME_NETWORK_PLAYDATA	gnd[PLAYER_SETUP_NUM]; 

	TEAM_CONTROL*			tc[TEAM_COUNT_MAX]; 
	CAMERA_CONTROL*			cc; 
	SKILL_CONTROL*			sc; 

	STATUSWIN_CONTROL*		swc; 
	MSGWIN_CONTROL*			mwc; 
	MAPWIN_CONTROL*			mpwc; 

	STATUS_CONTROL*			stc; 

	BOOL					onGameFlag;
	BOOL					endGameFlag;
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

	//テキストウインドウ登録
	{
		{//ステータスウインドウ
			STATUSWIN_SETUP statuswinSetUp;

			statuswinSetUp.heapID		= gc->heapID;
			statuswinSetUp.targetBmpwin	= Get_GS_BmpWin( gc->gs, G2DBMPWIN_STATUS ); 
			statuswinSetUp.p_tc			= gc->tc;
			statuswinSetUp.teamCount	= gc->teamCount;

			gc->swc = AddStatusWinControl( &statuswinSetUp );
		}
		{//メッセージウインドウ
			MSGWIN_SETUP msgwinSetUp;

			msgwinSetUp.heapID			= gc->heapID;
			msgwinSetUp.targetBmpwin	= Get_GS_BmpWin( gc->gs, G2DBMPWIN_MSG ); 

			gc->mwc = AddMessageWinControl( &msgwinSetUp );
		}
		{//マップウインドウ
			MAPWIN_SETUP mapwinSetUp;

			mapwinSetUp.heapID			= gc->heapID;
			mapwinSetUp.targetBmpwin	= Get_GS_BmpWin( gc->gs, G2DBMPWIN_MAP ); 
			mapwinSetUp.gs				= gc->gs;
			mapwinSetUp.p_tc			= gc->tc;
			mapwinSetUp.teamCount		= gc->teamCount;
			mapwinSetUp.myPc			= gc->myPc;
			mapwinSetUp.myTc			= gc->myTc;

			gc->mpwc = AddMapWinControl( &mapwinSetUp );
		}
	}
	//カメラコントローラ登録
	gc->cc = AddCameraControl( gc->gs, gc->heapID );

	//スキルコントローラ登録
	{
		SKILLCONT_SETUP skillcontSetUp;

		skillcontSetUp.heapID		= gc->heapID;
		skillcontSetUp.gs			= gc->gs;
		skillcontSetUp.p_tc			= gc->tc;
		skillcontSetUp.teamCount	= gc->teamCount;
		gc->sc = AddSkillControl( &skillcontSetUp );
	}
	//ＯＢＪコントローラ登録
	{
		{//ステータスバー
			STATUS_SETUP statusSetUp;

			statusSetUp.heapID			= gc->heapID;
			statusSetUp.gs				= gc->gs;
			statusSetUp.p_tc			= gc->tc;
			statusSetUp.teamCount		= gc->teamCount;
			statusSetUp.cc				= gc->cc;
			statusSetUp.myPc			= gc->myPc;

			gc->stc = AddStatusControl( &statusSetUp );
		}
	}
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

	RemoveStatusControl( gc->stc );

	RemoveSkillControl( gc->sc );
	RemoveCameraControl( gc->cc );

	RemoveMapWinControl( gc->mpwc );
	RemoveMessageWinControl( gc->mwc );
	RemoveStatusWinControl( gc->swc );

	for( i=0; i<gc->teamCount; i++ ){
		RemoveTeamControl( gc->tc[i] );
	}
	ExitMouseEvent( gc->mes );

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
static void CalcCastleDamage( GAME_CONTROL* gc, int castleID, int* damage );
static void SetMapWinMask( GFL_BMPWIN* targetBmpwin, TEAM_CONTROL* tc );

enum {
	GAME_CASTLE_BUILD_START = 0,
	GAME_CASTLE_BUILD_ON,
	GAME_START,
	GAME_ON,
	GAME_END,
};

//------------------------------------------------------------------
void MainGameControl( GAME_CONTROL* gc )
{
	int i;
	{
		VecFx32 cTrans;
		GetCameraControlTrans( gc->cc, &cTrans );

		//MainMouseEvent( gc->mes, gc->key.tpTrg, gc->key.tpCont, gc->key.tpx, gc->key.tpy );
	}
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
	//	trans.y = 0;
		SetCameraControlTrans( gc->cc, &trans );
		GetPlayerControlDirection( gc->myPc, &direction );
		SetCameraControlDirection( gc->cc, &direction );
	}
	MainCameraControl( gc->cc );
	MainSkillControl( gc->sc, gc->onGameFlag );

	MainStatusControl( gc->stc );

	MainMapWinControl( gc->mpwc );
	SetMapWinMask( Get_GS_BmpWin( gc->gs, G2DBMPWIN_MASK ), gc->myTc );

	switch( gc->seq ){

	case GAME_CASTLE_BUILD_START:
		gc->onGameFlag = FALSE;
		//gc->onGameFlag = TRUE;
		gc->endGameFlag = FALSE;
		PutMessageWin( gc->mwc, GMSG_GAME_START_WAIT );
		gc->seq = GAME_CASTLE_BUILD_ON;
		break;

	case GAME_CASTLE_BUILD_ON:
		{
			int hp, hpMax;
			BOOL onGameCheckFlag = TRUE;

			for( i=0; i<gc->teamCount; i++ ){
				if( GetCastleHP( gc->tc[i], &hp, &hpMax ) == FALSE ){
					onGameCheckFlag = FALSE;
				}
			}
			if( onGameCheckFlag == TRUE ){
				gc->onGameFlag = TRUE;
				gc->seq = GAME_START;
			}
		}
		break;

	case GAME_START:
		gc->time = 0;
		PutMessageWin( gc->mwc, GMSG_GAME_START );
		gc->seq = GAME_ON;
		break;

	case GAME_ON:
		gc->time++;
		if( !(gc->time%60) ){
			int j, damage, hp, hpMax;

			for( i=0; i<gc->teamCount; i++ ){
				GetCastleAreaDamage( gc->tc[i], &damage );
				for( j=0; j<gc->teamCount; j++ ){
					if( i != j ){
						SetCastleDamage( gc->tc[j], &damage );
					}
				}
			}
		}
		{
			int hp, hpMax;
			BOOL myCastleExistFlag = TRUE;
			int castleCount = gc->teamCount;

			for( i=0; i<gc->teamCount; i++ ){
				if( GetCastleHP( gc->tc[i], &hp, &hpMax ) == TRUE ){
					if( hp <= 0 ){
						if( gc->tc[i] == gc->myTc ){
							myCastleExistFlag = FALSE;
						}
						castleCount--;
					}
				} else {
					castleCount--;
				}
			}
			if( castleCount <= 1 ){
				if( castleCount == 1 ){
					if( myCastleExistFlag == TRUE ){
						//PutMessageWin( gc->mwc, GMSG_GAME_WIN );
					} else {
						//PutMessageWin( gc->mwc, GMSG_GAME_LOSE );
					}
				} else {
					//PutMessageWin( gc->mwc, GMSG_GAME_EVEN );
				}
				gc->onGameFlag = FALSE;
				gc->endGameFlag = TRUE;
				gc->seq = GAME_END;
			}
		}
		break;

	case GAME_END:
		break;
	}

	MainMessageWinControl( gc->mwc );
	MainStatusWinControl( gc->swc );
	SetStatusWinReload( gc->swc );
}

//------------------------------------------------------------------
static void	MainGameControlSummonCallBack( TEAM_CONTROL* tc, int summonID, int num, void* work )
{
	MAINCONT_CALLBACK_WORK* mccw = (MAINCONT_CALLBACK_WORK*)work;
	VecFx32 trans;
	BOOL cullingFlag;
	int hp, hpMax;

	if( mccw->gc->endGameFlag == FALSE ){
		if( mccw->gc->onGameFlag == TRUE ){
			GetSummonObjectHP( tc, summonID, &hp, &hpMax );
			if( hp == 0 ){
				DeleteSummonObject( tc, summonID );
				MakeTeamMapAreaMask( tc );
			}
		}
	} else {
		DeleteSummonObject( tc, summonID );
		MakeTeamMapAreaMask( tc );
	}
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
	BOOL cullingFlag;
	PLAYER_BUILD_COMMAND buildCommand;
	VecFx32 setTrans;
	int i,netID;
	BOOL buildFlag;

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
	SetSkillControlCommand( mccw->gc->sc, mccw->tc, pc, GetPlayerSkillCommand( pc ));
	buildCommand = GetPlayerBuildCommand( pc );

	if( mccw->gc->endGameFlag == FALSE ){
		if( mccw->gc->onGameFlag == TRUE ){
			if( buildCommand == PBC_SUMMON ){
				GetPlayerControlTrans( pc, &setTrans );
				if( GetMapAreaMaskStatus( mccw->tc, &setTrans ) == TRUE ){
					buildFlag = TRUE;
					for( i=0; i<mccw->gc->teamCount; i++ ){
						if( mccw->gc->tc[i] != mccw->tc ){
							if( GetMapAreaMaskStatus( mccw->gc->tc[i], &setTrans ) == TRUE ){
								buildFlag = FALSE;
							}
						}
					}
					if( buildFlag == TRUE ){
						if( CreateSummonObject( mccw->tc, mccw->tID+1, &setTrans ) != -1 ){
							MakeTeamMapAreaMask( mccw->tc );
						} else {
							PutMessageWinMine( mccw->gc, GMSG_SUMMON_MAXERROR, pc );
						}
					} else {
						PutMessageWinMine( mccw->gc, GMSG_SUMMON_AREACONFLICT, pc );
					}
				} else {
					PutMessageWinMine( mccw->gc, GMSG_SUMMON_AREAOUTRANGE, pc );
				}
			}
			if( GetPlayerDeadFlag( pc ) == TRUE ){	//死亡時の拠点ダメージ
				int deadDamage = 100;
	
				SetCastleDamage( mccw->tc, &deadDamage );
				ResetPlayerDeadFlag( pc );
			}
		} else {
			if( buildCommand == PBC_BUILD_CASTLE ){
				GetPlayerControlTrans( pc, &setTrans );
				if( CreateCastle( mccw->tc, &setTrans ) != -1 ){
					MakeTeamMapAreaMask( mccw->tc );
				}
			}
		}
	}
//	ResetPlayerBuildCommand( pc );
//	ResetPlayerDeadFlag( pc );
//	ResetPlayerSkillCommand( pc );
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
	ResetPlayerBuildCommand( pc );
	ResetPlayerDeadFlag( pc );
	ResetPlayerSkillCommand( pc );
}

//------------------------------------------------------------------
/**
 * @brief	自機挙動メッセージ表示
 */
//------------------------------------------------------------------
static void PutMessageWinMine( GAME_CONTROL* gc, MSGID msgID, PLAYER_CONTROL* pc )
{
	if( pc == gc->myPc ){
		//PutMessageWin( gc->mwc, msgID );
	}
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

static BOOL checkMoveDirection( int cont, PLAYER_MOVE_DIR* dir )
{
	if( cont & PAD_KEY_UP ){
		if( cont & PAD_KEY_LEFT ){
			*dir = PMV_FRONT_LEFT;
		} else if( cont & PAD_KEY_RIGHT ){
			*dir = PMV_FRONT_RIGHT;
		} else {
			*dir = PMV_FRONT;
		}
		return TRUE;
	}
	if( cont & PAD_KEY_DOWN ){
		if( cont & PAD_KEY_LEFT ){
			*dir = PMV_BACK_LEFT;
		} else if( cont & PAD_KEY_RIGHT ){
			*dir = PMV_BACK_RIGHT;
		} else {
			*dir = PMV_BACK;
		}
		return TRUE;
	}
	if( cont & PAD_KEY_LEFT ){
		*dir = PMV_LEFT;
		return TRUE;
	}
	if( cont & PAD_KEY_RIGHT ){
		*dir = PMV_RIGHT;
		return TRUE;
	}
	return FALSE;
}

//------------------------------------------------------------------
static void ControlKey( PLAYER_CONTROL* pc, GAME_CONTROL* gc )
{
	BOOL contLimit = TRUE;

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
	{
		//テスト
		//武器の変更
		if( CheckResetMouseEvent( gc->mes, MOUSE_EVENT_ACTION_6 ) == TRUE ){
			if( !gc->tp_blank ){
				SetPlayerControlCommand( pc, PCC_WEPONCHANGE );
				gc->tp_blank = 8;
			} else {
				SetPlayerControlCommand( pc, PCC_STAY );
				gc->tp_blank--;
			}
			return;
		}
		//装備の変更
		if( CheckResetMouseEvent( gc->mes, MOUSE_EVENT_ACTION_7 ) == TRUE ){
			if( !gc->tp_blank ){
				SetPlayerControlCommand( pc, PCC_TEST );
				gc->tp_blank = 8;
			} else {
				SetPlayerControlCommand( pc, PCC_STAY );
				gc->tp_blank--;
			}
			return;
		}
		gc->tp_blank = 0;
	}
	//建設
	if( CheckResetMouseEvent( gc->mes, MOUSE_EVENT_ACTION_1 ) == TRUE ){
		SetPlayerControlCommand( pc, PCC_BUILD );
		return;
	}
	//召喚
	if( CheckResetMouseEvent( gc->mes, MOUSE_EVENT_ACTION_2 ) == TRUE ){
		SetPlayerControlCommand( pc, PCC_SUMMON );
		return;
	}
	//攻撃
	{
		if(	CheckResetMouseEvent( gc->mes, MOUSE_EVENT_ATTACK_END ) == TRUE ){
			//攻撃終了
			ClearMouseEvent( gc->mes );
			SetPlayerAttackCommand( pc, PCC_ATTACK_END, 0 );
			return;
		}
		{
			if(	CheckResetMouseEvent( gc->mes, MOUSE_EVENT_ATTACK_3 ) == TRUE ){
				ClearMouseEvent( gc->mes );
				SetPlayerAttackCommand( pc, PCC_ATTACK, 0 );
				return;
			} else if(	CheckResetMouseEvent( gc->mes, MOUSE_EVENT_ATTACK_5 ) == TRUE ){
				ClearMouseEvent( gc->mes );
				SetPlayerAttackCommand( pc, PCC_ATTACK, 1 );
				return;
			} else if(	CheckResetMouseEvent( gc->mes, MOUSE_EVENT_ATTACK_4 ) == TRUE ){
				ClearMouseEvent( gc->mes );
				SetPlayerAttackCommand( pc, PCC_ATTACK, 2 );
				return;
			} else if(	CheckResetMouseEvent( gc->mes, MOUSE_EVENT_ATTACK_6 ) == TRUE ){
				ClearMouseEvent( gc->mes );
				SetPlayerAttackCommand( pc, PCC_ATTACK, 3 );
				return;
			} else if(	CheckResetMouseEvent( gc->mes, MOUSE_EVENT_ATTACK_2 ) == TRUE ){
				ClearMouseEvent( gc->mes );
				SetPlayerAttackCommand( pc, PCC_ATTACK, 4 );
				return;
			} else if(	CheckResetMouseEvent( gc->mes, MOUSE_EVENT_ATTACK_1 ) == TRUE ){
				ClearMouseEvent( gc->mes );
				SetPlayerAttackCommand( pc, PCC_ATTACK, 5 );
				return;
			}
		}
		if(	CheckMouseEvent( gc->mes, MOUSE_EVENT_ATTACK_READY ) == TRUE ){
			//攻撃準備
			SetPlayerAttackCommand( pc, PCC_ATTACK_READY, 0 );
			return;
		}
	}
	//座る
	if( CheckMouseEvent( gc->mes, MOUSE_EVENT_ACTION_3 ) == TRUE ){
		if( gc->key.tpCont == TRUE ){
			SetPlayerControlCommand( pc, PCC_SIT );
			return;
		} 
	}
	//拾う
	if( CheckResetMouseEvent( gc->mes, MOUSE_EVENT_ACTION_4 ) == TRUE ){
		SetPlayerControlCommand( pc, PCC_TAKEOFF );
		return;
	}
	//置く
	if( CheckResetMouseEvent( gc->mes, MOUSE_EVENT_ACTION_5 ) == TRUE ){
		SetPlayerControlCommand( pc, PCC_PUTON );
		return;
	}
	//地形ＵＰＤＯＷＮ
	if( CheckResetMouseEvent( gc->mes, MOUSE_EVENT_GROUNDMAKE_UP ) == TRUE ){
		VecFx32 pos;
		GetMousePos( gc->mes, &pos );
		SetMapGroundUp( &pos );
		SetPlayerControlCommand( pc, PCC_TAKEOFF );
		return;
	} else if( CheckResetMouseEvent( gc->mes, MOUSE_EVENT_GROUNDMAKE_DOWN ) == TRUE ){
		VecFx32 pos;
		GetMousePos( gc->mes, &pos );
		SetMapGroundDown( &pos );
		SetPlayerControlCommand( pc, PCC_PUTON );
		return;
	}
	//ジャンプ
	if( CheckResetMouseEvent( gc->mes, MOUSE_EVENT_JUMP) == TRUE ){
		VecFx32 mvDir;

		GetMousePos( gc->mes, &mvDir );
		SetPlayerMoveCommand( pc, PCC_JUMP, &mvDir );
		return;
	}
	//移動
	if( CheckResetMouseEvent( gc->mes, MOUSE_EVENT_MOVESTART) == TRUE ){
		VecFx32 mvDir;

		GetMousePos( gc->mes, &mvDir );
		SetPlayerMoveCommand( pc, PCC_WALK, &mvDir );
		return;
	} else if( CheckResetMouseEvent( gc->mes, MOUSE_EVENT_MOVE) == TRUE ){
		VecFx32 mvDir;

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
 * @brief	領域マスク反映
 */
//------------------------------------------------------------------
static void SetMapWinMask( GFL_BMPWIN* targetBmpwin, TEAM_CONTROL* tc )
{
	GFL_BMP_DATA *srcBmp, *dstBmp;
	u32	*src, *dst;
	u32 siz;
	BOOL sw = FALSE;

	if( GetMapAreaMaskDrawFlag( tc ) == TRUE ){
		GetMapAreaMask( tc, &srcBmp );
		dstBmp = GFL_BMPWIN_GetBmp( targetBmpwin );

		src = (u32*)GFL_BMP_GetCharacterAdrs( srcBmp );
		dst = (u32*)GFL_BMP_GetCharacterAdrs( dstBmp );
		siz = (GFL_BMP_GetSizeX( dstBmp )/8) * (GFL_BMP_GetSizeY( dstBmp )/8) * 0x20;
		GFL_STD_MemCopy32( srcBmp, dstBmp, siz );

		GFL_BMPWIN_TransVramCharacter( targetBmpwin );
		SetMapAreaMaskDrawFlag( tc, &sw );
	}
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
	ChangeMapWinControl( gc->mpwc, gc->myPc, gc->myTc );
	ChangeStatusControl( gc->stc , gc->myPc );

	SetMapAreaMaskDrawFlag( gc->myTc, &sw );
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



