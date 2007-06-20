//============================================================================================
/**
 * @file	control.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"

#include "setup.h"
#include "player_cont.h"

//============================================================================================
//
//
//	プレーヤーコントロール
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	構造体定義
 */
//------------------------------------------------------------------
struct _PLAYER_CONTROL {
	HEAPID					heapID;
	GAME_SYSTEM*			gs;
	int						targetAct;
	PLAYER_CONTROL_COMMAND	contCommand;

	u16						contDirection;
	u16						nowDirection;
	VecFx32					contTrans;
	u16						nowAccesary;
	BOOL					sitDownFlag;

};

#define WALK_SPEED	(1)		//実質1/2
#define RUN_SPEED	(4)		//実質2
#define HITOFS		(12)
//------------------------------------------------------------------
/**
 * @brief	プレーヤーコントロール初期化
 */
//------------------------------------------------------------------
PLAYER_CONTROL* AddPlayerControl( GAME_SYSTEM* gs, int targetAct, HEAPID heapID )
{
	PLAYER_CONTROL* pc = GFL_HEAP_AllocClearMemory( heapID, sizeof(PLAYER_CONTROL) );
	pc->heapID = heapID;
	pc->gs = gs;
	pc->targetAct = targetAct;
	pc->contDirection = 0;
	pc->nowDirection = 0;
	Get3DactTrans( Get_GS_SceneAct( pc->gs ), pc->targetAct, &pc->contTrans );
	pc->sitDownFlag = FALSE;

	return pc;
}

//------------------------------------------------------------------
/**
 * @brief	プレーヤーコントロール終了
 */
//------------------------------------------------------------------
void RemovePlayerControl( PLAYER_CONTROL* pc )
{
	GFL_HEAP_FreeMemory( pc ); 
}

//------------------------------------------------------------------
/**
 * @brief	プレーヤー方向の取得
 */
//------------------------------------------------------------------
void GetPlayerDirection( PLAYER_CONTROL* pc, u16* direction )
{
	*direction = pc->nowDirection;
}

//------------------------------------------------------------------
/**
 * @brief	プレーヤーコントロール方向の設定
 */
//------------------------------------------------------------------
void SetPlayerControlDirection( PLAYER_CONTROL* pc, u16* direction )
{
	pc->contDirection = *direction;
}

//------------------------------------------------------------------
/**
 * @brief	プレーヤーコマンドの設定
 */
//------------------------------------------------------------------
void SetPlayerControlCommand( PLAYER_CONTROL* pc, PLAYER_CONTROL_COMMAND command )
{
	pc->contCommand = command;
}

//------------------------------------------------------------------
/**
 * @brief	プレーヤーメインコントロール
 */
//------------------------------------------------------------------
static inline void moveSet( PLAYER_CONTROL* pc, u16 direction, int speed )
{
	VecFx32	rotVec, tmpTransVec;		
	u16		mapAttr;

	//簡易ヒットチェック(自分を中心とした円衝突判定)
	pc->nowDirection = direction;
	tmpTransVec.x = pc->contTrans.x + ( -( speed + HITOFS ) * FX_SinIdx( direction ) /2 );
	tmpTransVec.y = pc->contTrans.y;
	tmpTransVec.z = pc->contTrans.z + ( -( speed + HITOFS ) * FX_CosIdx( direction ) /2 );

	if( Get3DmapAttr( Get_GS_SceneMap( pc->gs ), &tmpTransVec, &mapAttr ) == TRUE ){
		if( mapAttr == 0 ){
			pc->contTrans.x += ( -speed * FX_SinIdx( direction ) /2 );
			pc->contTrans.z += ( -speed * FX_CosIdx( direction ) /2 );
		}
	}
	rotVec.x = 0;
	rotVec.y = direction + 0x8000;
	rotVec.z = 0;

	Set3DactTrans( Get_GS_SceneAct( pc->gs ), pc->targetAct, &pc->contTrans );
	Set3DactRotate( Get_GS_SceneAct( pc->gs ), pc->targetAct, &rotVec );
}

//------------------------------------------------------------------
void MainPlayerControl( PLAYER_CONTROL* pc )
{
	VecFx32 move = { 0, 0, 0 };
	BOOL	moveFlag = FALSE;

	if( Check3DactPlayerBusy( Get_GS_SceneAct( pc->gs ), pc->targetAct ) == TRUE ){
		return;
	}
	if(( pc->contCommand != PCC_SIT )&&( pc->sitDownFlag == TRUE )){	//しゃがみ中立ち判定処理
		Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_STANDUP );
		pc->sitDownFlag = FALSE;
		return;
	}
	switch( pc->contCommand ){
	case PCC_NOP:
		break;
	case PCC_STAY:
		Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_STAY );
		break;
	case PCC_WALK_FRONT:
		moveSet( pc, pc->contDirection + 0x0000, WALK_SPEED );
		Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_WALK );
		break;
	case PCC_WALK_BACK:
		moveSet( pc, pc->contDirection + 0x8000, WALK_SPEED );
		Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_WALK );
		break;
	case PCC_WALK_LEFT:
		moveSet( pc, pc->contDirection + 0x4000, WALK_SPEED );
		Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_WALK );
		break;
	case PCC_WALK_RIGHT:
		moveSet( pc, pc->contDirection - 0x4000, WALK_SPEED );
		Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_WALK );
		break;
	case PCC_WALK_FRONT_LEFT:
		moveSet( pc, pc->contDirection + 0x2000, WALK_SPEED );
		Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_WALK );
		break;
	case PCC_WALK_FRONT_RIGHT:
		moveSet( pc, pc->contDirection - 0x2000, WALK_SPEED );
		Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_WALK );
		break;
	case PCC_WALK_BACK_LEFT:
		moveSet( pc, pc->contDirection + 0x6000, WALK_SPEED );
		Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_WALK );
		break;
	case PCC_WALK_BACK_RIGHT:
		moveSet( pc, pc->contDirection - 0x6000, WALK_SPEED );
		Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_WALK );
		break;
	case PCC_RUN_FRONT:
		moveSet( pc, pc->contDirection + 0x0000, RUN_SPEED );
		Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_RUN );
		break;
	case PCC_RUN_BACK:
		moveSet( pc, pc->contDirection + 0x8000, RUN_SPEED );
		Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_RUN );
		break;
	case PCC_RUN_LEFT:
		moveSet( pc, pc->contDirection + 0x4000, RUN_SPEED );
		Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_RUN );
		break;
	case PCC_RUN_RIGHT:
		moveSet( pc, pc->contDirection - 0x4000, RUN_SPEED );
		Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_RUN );
		break;
	case PCC_RUN_FRONT_LEFT:
		moveSet( pc, pc->contDirection + 0x2000, RUN_SPEED );
		Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_RUN );
		break;
	case PCC_RUN_FRONT_RIGHT:
		moveSet( pc, pc->contDirection - 0x2000, RUN_SPEED );
		Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_RUN );
		break;
	case PCC_RUN_BACK_LEFT:
		moveSet( pc, pc->contDirection + 0x6000, RUN_SPEED );
		Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_RUN );
		break;
	case PCC_RUN_BACK_RIGHT:
		moveSet( pc, pc->contDirection - 0x6000, RUN_SPEED );
		Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_RUN );
		break;
	case PCC_ATTACK:
		//武器によってモーション変化
		switch( pc->nowAccesary ){
		defasult:
			Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_STAY );
			break;
		case 1:
			Set3DactPlayerChrAnimeCmd
				( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_ATTACK );
			break;
		case 2:
			Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_SHOOT );
			break;
		case 3:
			Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_SPELL );
			break;
		}
		break;
	case PCC_SIT:
		Set3DactPlayerChrAnimeCmd
			( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_SITDOWN );
		pc->sitDownFlag = TRUE;
		break;
	case PCC_WEPONCHANGE:		
		//ナンバー切り替え
		pc->nowAccesary++;
		pc->nowAccesary&= 3;
		Set3DactPlayerChrAccesory( Get_GS_SceneAct( pc->gs ), pc->targetAct, pc->nowAccesary );
		break;
	}
	//pc->contCommand = PCC_NOP;
}

