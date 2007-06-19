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
	int						targetObj;
	PLAYER_CONTROL_COMMAND	contCommand;

	u16						contDirection;
	u16						nowDirection;
	VecFx32					contTrans;
	u16						nowAccesary;
	BOOL					sitDownFlag;

};

#define WALK_SPEED	(1)	//実質1/2
#define RUN_SPEED	(4)	//実質2
//------------------------------------------------------------------
/**
 * @brief	プレーヤーコントロール初期化
 */
//------------------------------------------------------------------
PLAYER_CONTROL* AddPlayerControl( GAME_SYSTEM* gs, int targetObj, HEAPID heapID )
{
	PLAYER_CONTROL* pc = GFL_HEAP_AllocClearMemory( heapID, sizeof(PLAYER_CONTROL) );
	pc->heapID = heapID;
	pc->gs = gs;
	pc->targetObj = targetObj;
	pc->contDirection = 0;
	pc->nowDirection = 0;
	Get3DobjTrans( Get_GS_SceneObj( pc->gs ), pc->targetObj, &pc->contTrans );
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
	VecFx32	rotVec;		

	pc->nowDirection = direction;
	pc->contTrans.x += -speed * FX_SinIdx( direction ) /2;
	pc->contTrans.z += -speed * FX_CosIdx( direction ) /2;

	rotVec.x = 0;
	rotVec.y = direction + 0x8000;
	rotVec.z = 0;

	Set3DobjTrans( Get_GS_SceneObj( pc->gs ), pc->targetObj, &pc->contTrans );
	Set3DobjRotate( Get_GS_SceneObj( pc->gs ), pc->targetObj, &rotVec );
}

//------------------------------------------------------------------
void MainPlayerControl( PLAYER_CONTROL* pc )
{
	VecFx32 move = { 0, 0, 0 };
	BOOL	moveFlag = FALSE;

	if( Check3DobjPlayerBusy( Get_GS_SceneObj( pc->gs ), pc->targetObj ) == TRUE ){
		return;
	}
	if(( pc->contCommand != PCC_SIT )&&( pc->sitDownFlag == TRUE )){	//しゃがみ中立ち判定処理
		Set3DobjPlayerChrAnimeCmd( Get_GS_SceneObj( pc->gs ), pc->targetObj, OBJANM_CMD_STANDUP );
		pc->sitDownFlag = FALSE;
		return;
	}
	switch( pc->contCommand ){
	case PCC_NOP:
		break;
	case PCC_STAY:
		Set3DobjPlayerChrAnimeCmd( Get_GS_SceneObj( pc->gs ), pc->targetObj, OBJANM_CMD_STAY );
		break;
	case PCC_WALK_FRONT:
		moveSet( pc, pc->contDirection + 0x0000, WALK_SPEED );
		Set3DobjPlayerChrAnimeCmd( Get_GS_SceneObj( pc->gs ), pc->targetObj, OBJANM_CMD_WALK );
		break;
	case PCC_WALK_BACK:
		moveSet( pc, pc->contDirection + 0x8000, WALK_SPEED );
		Set3DobjPlayerChrAnimeCmd( Get_GS_SceneObj( pc->gs ), pc->targetObj, OBJANM_CMD_WALK );
		break;
	case PCC_WALK_LEFT:
		moveSet( pc, pc->contDirection + 0x4000, WALK_SPEED );
		Set3DobjPlayerChrAnimeCmd( Get_GS_SceneObj( pc->gs ), pc->targetObj, OBJANM_CMD_WALK );
		break;
	case PCC_WALK_RIGHT:
		moveSet( pc, pc->contDirection - 0x4000, WALK_SPEED );
		Set3DobjPlayerChrAnimeCmd( Get_GS_SceneObj( pc->gs ), pc->targetObj, OBJANM_CMD_WALK );
		break;
	case PCC_WALK_FRONT_LEFT:
		moveSet( pc, pc->contDirection + 0x2000, WALK_SPEED );
		Set3DobjPlayerChrAnimeCmd( Get_GS_SceneObj( pc->gs ), pc->targetObj, OBJANM_CMD_WALK );
		break;
	case PCC_WALK_FRONT_RIGHT:
		moveSet( pc, pc->contDirection - 0x2000, WALK_SPEED );
		Set3DobjPlayerChrAnimeCmd( Get_GS_SceneObj( pc->gs ), pc->targetObj, OBJANM_CMD_WALK );
		break;
	case PCC_WALK_BACK_LEFT:
		moveSet( pc, pc->contDirection + 0x6000, WALK_SPEED );
		Set3DobjPlayerChrAnimeCmd( Get_GS_SceneObj( pc->gs ), pc->targetObj, OBJANM_CMD_WALK );
		break;
	case PCC_WALK_BACK_RIGHT:
		moveSet( pc, pc->contDirection - 0x6000, WALK_SPEED );
		Set3DobjPlayerChrAnimeCmd( Get_GS_SceneObj( pc->gs ), pc->targetObj, OBJANM_CMD_WALK );
		break;
	case PCC_RUN_FRONT:
		moveSet( pc, pc->contDirection + 0x0000, RUN_SPEED );
		Set3DobjPlayerChrAnimeCmd( Get_GS_SceneObj( pc->gs ), pc->targetObj, OBJANM_CMD_RUN );
		break;
	case PCC_RUN_BACK:
		moveSet( pc, pc->contDirection + 0x8000, RUN_SPEED );
		Set3DobjPlayerChrAnimeCmd( Get_GS_SceneObj( pc->gs ), pc->targetObj, OBJANM_CMD_RUN );
		break;
	case PCC_RUN_LEFT:
		moveSet( pc, pc->contDirection + 0x4000, RUN_SPEED );
		Set3DobjPlayerChrAnimeCmd( Get_GS_SceneObj( pc->gs ), pc->targetObj, OBJANM_CMD_RUN );
		break;
	case PCC_RUN_RIGHT:
		moveSet( pc, pc->contDirection - 0x4000, RUN_SPEED );
		Set3DobjPlayerChrAnimeCmd( Get_GS_SceneObj( pc->gs ), pc->targetObj, OBJANM_CMD_RUN );
		break;
	case PCC_RUN_FRONT_LEFT:
		moveSet( pc, pc->contDirection + 0x2000, RUN_SPEED );
		Set3DobjPlayerChrAnimeCmd( Get_GS_SceneObj( pc->gs ), pc->targetObj, OBJANM_CMD_RUN );
		break;
	case PCC_RUN_FRONT_RIGHT:
		moveSet( pc, pc->contDirection - 0x2000, RUN_SPEED );
		Set3DobjPlayerChrAnimeCmd( Get_GS_SceneObj( pc->gs ), pc->targetObj, OBJANM_CMD_RUN );
		break;
	case PCC_RUN_BACK_LEFT:
		moveSet( pc, pc->contDirection + 0x6000, RUN_SPEED );
		Set3DobjPlayerChrAnimeCmd( Get_GS_SceneObj( pc->gs ), pc->targetObj, OBJANM_CMD_RUN );
		break;
	case PCC_RUN_BACK_RIGHT:
		moveSet( pc, pc->contDirection - 0x6000, RUN_SPEED );
		Set3DobjPlayerChrAnimeCmd( Get_GS_SceneObj( pc->gs ), pc->targetObj, OBJANM_CMD_RUN );
		break;
	case PCC_ATTACK:
		//武器によってモーション変化
		switch( pc->nowAccesary ){
		defasult:
			Set3DobjPlayerChrAnimeCmd( Get_GS_SceneObj( pc->gs ), pc->targetObj, OBJANM_CMD_STAY );
			break;
		case 1:
			Set3DobjPlayerChrAnimeCmd
				( Get_GS_SceneObj( pc->gs ), pc->targetObj, OBJANM_CMD_ATTACK );
			break;
		case 2:
			Set3DobjPlayerChrAnimeCmd( Get_GS_SceneObj( pc->gs ), pc->targetObj, OBJANM_CMD_SHOOT );
			break;
		case 3:
			Set3DobjPlayerChrAnimeCmd( Get_GS_SceneObj( pc->gs ), pc->targetObj, OBJANM_CMD_SPELL );
			break;
		}
		break;
	case PCC_SIT:
		Set3DobjPlayerChrAnimeCmd
			( Get_GS_SceneObj( pc->gs ), pc->targetObj, OBJANM_CMD_SITDOWN );
		pc->sitDownFlag = TRUE;
		break;
	case PCC_WEPONCHANGE:		
		//ナンバー切り替え
		pc->nowAccesary++;
		pc->nowAccesary&= 3;
		Set3DobjPlayerChrAccesory( Get_GS_SceneObj( pc->gs ), pc->targetObj, pc->nowAccesary );
		break;
	}
	//pc->contCommand = PCC_NOP;
}

