//============================================================================================
/**
 * @file	player_cont.c
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
typedef struct {
	u8		count;
	u8		timer;
	u8		timerTmp;
	s8		value;
}HP_INCDEC_ONTIME;

struct _PLAYER_CONTROL {
	HEAPID					heapID;
	GAME_SYSTEM*			gs;
	int						targetAct;

	PLAYER_CONTROL_COMMAND	contCommand;
	u16						skillCommand;
	s16						damage;
	HP_INCDEC_ONTIME		dotStatus;
	HP_INCDEC_ONTIME		regenerateStatus;
	BOOL					sitDownFlag;
	BOOL					skillBusyFlag;
	BOOL					hitEnableFlag;

	u16						contDirection;
	u16						nowDirection;
	VecFx32					contTrans;
	u16						nowAccesary;

	PLAYER_STATUS			status;
};

#define WALK_SPEED	(1)		//実質1/2
#define RUN_SPEED	(4)		//実質2
#define HITOFS		(16)
static const PLAYER_STATUS	statusDefault = { {"Null"},1000, 100, 1000, 100, 50, 50, 50, 50, 50 };

//------------------------------------------------------------------
/**
 * @brief	プレーヤーコントロール起動
 */
//------------------------------------------------------------------
PLAYER_CONTROL* AddPlayerControl( GAME_SYSTEM* gs, int targetAct, HEAPID heapID )
{
	PLAYER_CONTROL* pc = GFL_HEAP_AllocClearMemory( heapID, sizeof(PLAYER_CONTROL) );
	pc->heapID = heapID;
	pc->gs = gs;
	pc->targetAct = targetAct;

	pc->contCommand = PCC_NOP;
	pc->skillCommand = 0;
	pc->damage = 0;
	pc->dotStatus.count = 0;
	pc->regenerateStatus.count = 0;

	pc->contDirection = 0;
	pc->nowDirection = 0;
	Get3DactTrans( Get_GS_SceneAct( pc->gs ), pc->targetAct, &pc->contTrans );
	pc->nowAccesary = 0;
	pc->sitDownFlag = FALSE;
	pc->skillBusyFlag = FALSE;
	pc->hitEnableFlag = FALSE;

	pc->status = statusDefault;
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
 * @brief	プレーヤーコントロール座標の取得と設定
 */
//------------------------------------------------------------------
void GetPlayerControlTrans( PLAYER_CONTROL* pc, VecFx32* trans )
{
	*trans = pc->contTrans;
}

void SetPlayerControlTrans( PLAYER_CONTROL* pc, const VecFx32* trans )
{
	pc->contTrans = *trans;
}

//------------------------------------------------------------------
/**
 * @brief	プレーヤーコントロール方向の取得と設定
 */
//------------------------------------------------------------------
void GetPlayerControlDirection( PLAYER_CONTROL* pc, u16* direction )
{
	*direction = pc->contDirection;
}

void SetPlayerControlDirection( PLAYER_CONTROL* pc, const u16* direction )
{
	pc->contDirection = *direction;
}

//------------------------------------------------------------------
/**
 * @brief	プレーヤーステータスポインタの取得
 */
//------------------------------------------------------------------
PLAYER_STATUS* GetPlayerStatusPointer( PLAYER_CONTROL* pc )
{
	return &pc->status;
}

//------------------------------------------------------------------
/**
 * @brief	プレーヤーステータスの取得と設定
 */
//------------------------------------------------------------------
void GetPlayerStatus( PLAYER_CONTROL* pc, PLAYER_STATUS* status )
{
	*status = pc->status;
}

void SetPlayerStatus( PLAYER_CONTROL* pc, const PLAYER_STATUS* status )
{
	pc->status = *status;
}

//------------------------------------------------------------------
/**
 * @brief	プレーヤーコマンドの設定
 */
//------------------------------------------------------------------
void SetPlayerControlCommand( PLAYER_CONTROL* pc, const PLAYER_CONTROL_COMMAND command )
{
	pc->contCommand = command;
}

//------------------------------------------------------------------
/**
 * @brief	スキルコマンドの取得とリセット
 */
//------------------------------------------------------------------
int GetPlayerSkillCommand( PLAYER_CONTROL* pc )
{
	return pc->skillCommand;
}

void ResetPlayerSkillCommand( PLAYER_CONTROL* pc )
{
	pc->skillCommand = 0;
}

//------------------------------------------------------------------
/**
 * @brief	スキル実行中フラグの設定とリセット
 */
//------------------------------------------------------------------
void SetPlayerSkillBusyFlag( PLAYER_CONTROL* pc )
{
	pc->skillBusyFlag = TRUE;
}

void ResetPlayerSkillBusyFlag( PLAYER_CONTROL* pc )
{
	pc->skillBusyFlag = FALSE;
}

//------------------------------------------------------------------
/**
 * @brief	ヒット判定可能フラグの取得
 */
//------------------------------------------------------------------
BOOL GetPlayerHitEnableFlag( PLAYER_CONTROL* pc )
{
	return pc->hitEnableFlag;
}

//------------------------------------------------------------------
/**
 * @brief	ＨＰ減少値の設定
 */
//------------------------------------------------------------------
void SetPlayerDamage( PLAYER_CONTROL* pc, const s16 damage )
{
	pc->damage = damage;
}

//------------------------------------------------------------------
/**
 * @brief	ＤＯＴ（時間経過によるＨＰ減少値）の設定
 */
//------------------------------------------------------------------
void SetPlayerDamageOnTime( PLAYER_CONTROL* pc, const u8 count, const u8 timer, const s8 value )
{
	if(( value < 0 )&&( !pc->dotStatus.count )){
		pc->dotStatus.count = count;
		pc->dotStatus.timer = timer;
		pc->dotStatus.timerTmp = timer;
		pc->dotStatus.value = value;
	}
}

//------------------------------------------------------------------
/**
 * @brief	リジェネレート値の設定
 */
//------------------------------------------------------------------
void SetPlayerRegenerate( PLAYER_CONTROL* pc, const u8 count, const u8 timer, const s8 value )
{
	if( value > 0 ){
		pc->regenerateStatus.count = count;
		pc->regenerateStatus.timer = timer;
		pc->regenerateStatus.timerTmp = timer;
		pc->regenerateStatus.value = value;
	}
}

//------------------------------------------------------------------
/**
 * @brief	プレーヤーメインコントロール
 */
//------------------------------------------------------------------
static void moveSet( PLAYER_CONTROL* pc, u16 direction, int speed, int movecmd )
{
	VecFx32	rotVec, tmpTransVec;		
	u16		mapAttr;
	BOOL	areaOver;

	//簡易ヒットチェック(自分を中心とした円衝突判定)
	pc->nowDirection = direction;
	tmpTransVec.x = pc->contTrans.x + ( -( speed + HITOFS ) * FX_SinIdx( direction ) /2 );
	tmpTransVec.y = pc->contTrans.y;
	tmpTransVec.z = pc->contTrans.z + ( -( speed + HITOFS ) * FX_CosIdx( direction ) /2 );

	//簡易アトリビュート取得
	areaOver = Get3DmapAttr( Get_GS_SceneMap( pc->gs ), &tmpTransVec, &mapAttr );

	if(( areaOver == TRUE )&&( mapAttr == 0 )){
		pc->contTrans.x += ( -speed * FX_SinIdx( direction ) /2 );
		pc->contTrans.z += ( -speed * FX_CosIdx( direction ) /2 );
		Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, movecmd );
	} else {
		Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_STAY );
	}
	rotVec.x = 0;
	rotVec.y = direction + 0x8000;
	rotVec.z = 0;

	Set3DactTrans( Get_GS_SceneAct( pc->gs ), pc->targetAct, &pc->contTrans );
	Set3DactRotate( Get_GS_SceneAct( pc->gs ), pc->targetAct, &rotVec );
}

static void directionSet( PLAYER_CONTROL* pc, u16 direction )
{
	VecFx32	rotVec;		

	pc->nowDirection = direction;
	rotVec.x = 0;
	rotVec.y = direction + 0x8000;
	rotVec.z = 0;

	Set3DactRotate( Get_GS_SceneAct( pc->gs ), pc->targetAct, &rotVec );
}

//------------------------------------------------------------------
void MainPlayerControl( PLAYER_CONTROL* pc )
{
	VecFx32 move = { 0, 0, 0 };
	BOOL	moveFlag = FALSE;

	//行動不能判定
	if( pc->status.hp <= 0 ){
		return;	//行動不能
	}

	//ＨＰ増減処理
	if( pc->dotStatus.count ){	//ＤＯＴ判定
		if( pc->dotStatus.timerTmp ){
			pc->dotStatus.timerTmp--;
		} else {
			pc->dotStatus.timerTmp = pc->dotStatus.timer;
			pc->dotStatus.count--;
			pc->status.hp += pc->dotStatus.value;
			if( pc->status.hp < 0 ){
				pc->status.hp = 1;
			}
		}
	}
	if( pc->regenerateStatus.count ){	//リジェネレート判定
		if( pc->regenerateStatus.timerTmp ){
			pc->regenerateStatus.timerTmp--;
		} else {
			pc->regenerateStatus.timerTmp = pc->regenerateStatus.timer;
			pc->regenerateStatus.count--;
			pc->status.hp += pc->regenerateStatus.value;
			if( pc->status.hp > pc->status.hpMax ){
				pc->status.hp = pc->status.hpMax;
			}
		}
	}
	if( pc->damage ){	//直接ダメージ
		//ダメージくらい発生
		Set3DactPlayerChrAnimeForceCmd( Get_GS_SceneAct( pc->gs ), pc->targetAct, ACTANM_CMD_HIT );
		pc->status.hp += pc->damage;
		if( pc->status.hp < 0 ){
			pc->status.hp = 0;
		} else if( pc->status.hp > pc->status.hpMax ) {
			pc->status.hp = pc->status.hpMax;
		}
		pc->damage = 0;
		pc->hitEnableFlag = FALSE;	//やられモーション中はヒット判定をしないこともできる
		return;
	}

	//行動処理
	if( Check3DactPlayerBusy( Get_GS_SceneAct( pc->gs ), pc->targetAct ) == TRUE ){
		return;	//強制アニメ中
	}
	pc->hitEnableFlag = TRUE;

	if( pc->skillBusyFlag ){
		return;	//スキル実行中
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
		moveSet( pc, pc->contDirection + 0x0000, WALK_SPEED, ACTANM_CMD_WALK );
		break;
	case PCC_WALK_BACK:
		moveSet( pc, pc->contDirection + 0x8000, WALK_SPEED, ACTANM_CMD_WALK );
		break;
	case PCC_WALK_LEFT:
		moveSet( pc, pc->contDirection + 0x4000, WALK_SPEED, ACTANM_CMD_WALK );
		break;
	case PCC_WALK_RIGHT:
		moveSet( pc, pc->contDirection - 0x4000, WALK_SPEED, ACTANM_CMD_WALK );
		break;
	case PCC_WALK_FRONT_LEFT:
		moveSet( pc, pc->contDirection + 0x2000, WALK_SPEED, ACTANM_CMD_WALK );
		break;
	case PCC_WALK_FRONT_RIGHT:
		moveSet( pc, pc->contDirection - 0x2000, WALK_SPEED, ACTANM_CMD_WALK );
		break;
	case PCC_WALK_BACK_LEFT:
		moveSet( pc, pc->contDirection + 0x6000, WALK_SPEED, ACTANM_CMD_WALK );
		break;
	case PCC_WALK_BACK_RIGHT:
		moveSet( pc, pc->contDirection - 0x6000, WALK_SPEED, ACTANM_CMD_WALK );
		break;
	case PCC_RUN_FRONT:
		moveSet( pc, pc->contDirection + 0x0000, RUN_SPEED, ACTANM_CMD_RUN );
		break;
	case PCC_RUN_BACK:
		moveSet( pc, pc->contDirection + 0x8000, RUN_SPEED, ACTANM_CMD_RUN );
		break;
	case PCC_RUN_LEFT:
		moveSet( pc, pc->contDirection + 0x4000, RUN_SPEED, ACTANM_CMD_RUN );
		break;
	case PCC_RUN_RIGHT:
		moveSet( pc, pc->contDirection - 0x4000, RUN_SPEED, ACTANM_CMD_RUN );
		break;
	case PCC_RUN_FRONT_LEFT:
		moveSet( pc, pc->contDirection + 0x2000, RUN_SPEED, ACTANM_CMD_RUN );
		break;
	case PCC_RUN_FRONT_RIGHT:
		moveSet( pc, pc->contDirection - 0x2000, RUN_SPEED, ACTANM_CMD_RUN );
		break;
	case PCC_RUN_BACK_LEFT:
		moveSet( pc, pc->contDirection + 0x6000, RUN_SPEED, ACTANM_CMD_RUN );
		break;
	case PCC_RUN_BACK_RIGHT:
		moveSet( pc, pc->contDirection - 0x6000, RUN_SPEED, ACTANM_CMD_RUN );
		break;
	case PCC_ATTACK:
		//武器によってモーション変化
		switch( pc->nowAccesary ){
		defasult:
			Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct(pc->gs), pc->targetAct, ACTANM_CMD_STAY );
			break;
		case 1:
			directionSet( pc, pc->contDirection );	//カメラ方向に向き直り
			Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct(pc->gs), pc->targetAct, ACTANM_CMD_ATTACK );
			pc->skillCommand = 1;
			break;
		case 2:
			directionSet( pc, pc->contDirection );	//カメラ方向に向き直り
			Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct(pc->gs), pc->targetAct, ACTANM_CMD_SHOOT );
			pc->skillCommand = 2;
			break;
		case 3:
			directionSet( pc, pc->contDirection );	//カメラ方向に向き直り
			Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct(pc->gs), pc->targetAct, ACTANM_CMD_SPELL );
			pc->skillCommand = 3;
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
	case PCC_PUTON:		
		Set3DactPlayerChrAnimeCmd( Get_GS_SceneAct(pc->gs), pc->targetAct, ACTANM_CMD_TAKE );
		AddObject3Dmap( Get_GS_SceneMap( pc->gs ), EXMAPOBJ_WALL, &pc->contTrans );
		break;
	}
	//pc->contCommand = PCC_NOP;
}

