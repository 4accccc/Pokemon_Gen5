//============================================================================================
/**
 * @file	player_cont.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"
#include "calc_ph.h"

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
	GAME_SYSTEM*			gs;
	int						netID;

	u16						jumpSeq;
	u16						slideSeq;

	PLAYER_MOVE_DIR			moveDir;
	PLAYER_CONTROL_COMMAND	contCommand;
	PLAYER_SKILL_COMMAND	skillCommand;
	PLAYER_BUILD_COMMAND	buildCommand;
	s16						damage;
	HP_INCDEC_ONTIME		dotStatus;
	HP_INCDEC_ONTIME		regenerateStatus;

	BOOL					forceMovingFlag;
	BOOL					sitDownFlag;
	BOOL					skillBusyFlag;
	BOOL					hitEnableFlag;
	BOOL					deadFlag;
	BOOL					slideFlag;

	u16						contDirection;
	u16						nowDirection;
	VecFx32					contTrans;
	u16						nowAccesary;
	u16						recoverTimer;
	VecFx32					moveVec;
	int						attackID;

	PLAYER_STATUS			status;
	BOOL					anmSetFlag;	//通信用
	int						anmSetID;	//通信用
#if 0
	GFL_CLWK*				statusObj;
#endif
	SCENE_ACT*				sceneAct;
	CALC_PH_MV*				calcPHMV;
	BOOL					calcMoveEnable;

	u16						work[8];
};

#define WALK_SPEED	(4 << FX32_SHIFT)
#define RUN_SPEED	(4 << FX32_SHIFT)
#define JUMP_SPEED	(4 << FX32_SHIFT)
#define HITOFS		(16)

static const PLAYER_STATUS	statusDefault = { {"Null"},1000, 100, 1000, 100, 50, 50, 50, 50, 50 };
static const GFL_CLWK_DATA clwkData = { 0, 0, 0, 0, 0 }; 
//------------------------------------------------------------------
/**
 * @brief	プレーヤーコントロール起動
 */
//------------------------------------------------------------------
PLAYER_CONTROL* AddPlayerControl( GAME_SYSTEM* gs, int netID, HEAPID heapID )
{
	PLAYER_CONTROL* pc = GFL_HEAP_AllocClearMemory( heapID, sizeof(PLAYER_CONTROL) );
	pc->gs = gs;
	pc->netID = netID;
	pc->sceneAct = Create3Dact( Get_GS_G3Dscene(gs), heapID );

	pc->moveDir = 0;
	pc->contCommand = PCC_NOP;
	pc->skillCommand = PSC_NOP;
	pc->buildCommand = PBC_NOP;
	pc->damage = 0;
	pc->dotStatus.count = 0;
	pc->regenerateStatus.count = 0;

	pc->forceMovingFlag = FALSE;
	pc->sitDownFlag = FALSE;
	pc->skillBusyFlag = FALSE;
	pc->hitEnableFlag = FALSE;
	pc->deadFlag = FALSE;

	pc->contDirection = 0;
	pc->nowDirection = 0;

	pc->contTrans.x = 0;
	pc->contTrans.y = 0;
	pc->contTrans.z = 0;

	pc->nowAccesary = 0;
	pc->recoverTimer = 0;
	pc->attackID = 0;

	pc->jumpSeq = 0;
	pc->slideSeq = 0;
	pc->status = statusDefault;

	Set3DactDrawSw( pc->sceneAct, TRUE );
	//Set3DactDrawSw( pc->sceneAct, FALSE );
	{
		PHMV_SETUP setup;

		setup.getGroundVecN_func = GetGroundPlaneVecN;
		setup.getGroundHeight_func = GetGroundPlaneHeight;

		//setup.gravityMove = 9.8f * FX32_ONE/30;//86;
		//setup.gravityFall = 9.8f * FX32_ONE/15;
		setup.gravityMove = 0;
		setup.gravityFall = 9.8f * FX32_ONE/15;
		setup.planeMarginTheta = 0x0100;
		setup.absorbVal = PHMV_FULL_ABSORB;
		setup.enableMoveTheta = 0x3800;

		pc->calcPHMV = CreateCalcPhisicsMoving( heapID, &setup );
		pc->calcMoveEnable = TRUE;
	}
	{
		int i;
		for( i=0; i<8; i++ ){
			pc->work[i] = 0;
		}
	}
	return pc;
}

//------------------------------------------------------------------
/**
 * @brief	プレーヤーコントロール終了
 */
//------------------------------------------------------------------
void RemovePlayerControl( PLAYER_CONTROL* pc )
{
	DeleteCalcPhisicsMoving( pc->calcPHMV );
	Delete3Dact( pc->sceneAct );
	GFL_HEAP_FreeMemory( pc ); 
}

//------------------------------------------------------------------
/**
 * @brief	プレーヤーメインコントロール
 */
//------------------------------------------------------------------
static int dirOffsTable[] = 
	{ 0x0000, 0x8000, 0x4000,-0x4000, 0x2000,-0x2000, 0x6000,-0x6000, };

//------------------------------------------------------------------
static void anmReset( PLAYER_CONTROL* pc )
{
	pc->anmSetFlag = FALSE;
	pc->anmSetID = ACTANM_CMD_STAY;	//=0
}

//------------------------------------------------------------------
static void anmSet( PLAYER_CONTROL* pc, int anmID )
{
	if( Set3DactChrAnimeCmd( pc->sceneAct, anmID ) == TRUE ){
		pc->anmSetFlag = TRUE;
		pc->anmSetID = anmID;
	}
}

//------------------------------------------------------------------
static void anmSetForce( PLAYER_CONTROL* pc, int anmID )
{
	Set3DactChrAnimeForceCmd( pc->sceneAct, anmID );
	pc->anmSetFlag = TRUE;
	pc->anmSetID = anmID;
}

//------------------------------------------------------------------
static void directionSetRAD( PLAYER_CONTROL* pc, u16 direction )
{
	VecFx32	rotVec;		

	pc->nowDirection = direction;
	rotVec.x = 0;
	rotVec.y = direction + 0x8000;
	rotVec.z = 0;
	Set3DactRotate( pc->sceneAct, &rotVec );
}

//------------------------------------------------------------------
static void directionSetPHMV( PLAYER_CONTROL* pc )
{
	VecFx32	vecDir, rotVec;		

	GetMoveDirPHMV( pc->calcPHMV, &vecDir );
	//方向設定
	rotVec.x = 0;
	rotVec.y = FX_Atan2Idx( -vecDir.z, vecDir.x ) + 0x4000;
	rotVec.z = 0;

	pc->nowDirection = rotVec.y - 0x8000;
	Set3DactRotate( pc->sceneAct, &rotVec );
}

//------------------------------------------------------------------
static void directionSetVEC( PLAYER_CONTROL* pc )
{
	VecFx32	rotVec;		

	//方向設定
	rotVec.x = 0;
	rotVec.y = FX_Atan2Idx( -pc->moveVec.z, pc->moveVec.x ) + 0x4000;
	rotVec.z = 0;

	pc->nowDirection = rotVec.y - 0x8000;
	Set3DactRotate( pc->sceneAct, &rotVec );
}

//------------------------------------------------------------------
static BOOL damageSet( PLAYER_CONTROL* pc )
{
	if( pc->status.hp == 0 ){
		return TRUE;
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
	if( pc->damage < 0 ){	//直接ダメージ
		pc->status.hp += pc->damage;
		if( pc->status.hp < 0 ) {
			pc->status.hp = 0;
		}
		pc->damage = 0;
		return TRUE;	//ダメージくらい発生
	}
	if( pc->damage > 0 ){	//回復
		pc->status.hp += pc->damage;
		if( pc->status.hp > pc->status.hpMax ) {
			pc->status.hp = pc->status.hpMax;
		}
		pc->damage = 0;
	}
	return FALSE;
}

//------------------------------------------------------------------
static BOOL damageControl( PLAYER_CONTROL* pc )
{
	if( pc->recoverTimer ){
		if( pc->recoverTimer < 10 + 10*30 ){	//復帰待ち１０秒
			pc->recoverTimer++;
		} else {
			u8 alpha = 31;

			anmSetForce( pc, ACTANM_CMD_STAY );
			pc->recoverTimer = 0;	//復帰待ちカウンタ初期化
			Set3DactBlendAlpha( pc->sceneAct, &alpha );
			pc->status.hp = pc->status.hpMax;	//復帰
			pc->deadFlag = FALSE;
		}
	} else {
		if( damageSet(pc) == TRUE ){
			if( pc->status.hp ){
				//ダメージくらった
				anmSetForce( pc, ACTANM_CMD_HIT );
				pc->hitEnableFlag = FALSE;	//やられモーション中はヒット判定をしない（暫定）
			} else {
				u8 alpha = 20;	//半透明演出
				//死亡
				anmSetForce( pc, ACTANM_CMD_DEAD );
				Set3DactBlendAlpha( pc->sceneAct, &alpha );
				pc->deadFlag = TRUE;
				pc->hitEnableFlag = FALSE;	//死亡時はヒット判定をしない
				pc->recoverTimer = 10;	//復帰待ちカウンタ開始
			}
		} else {
			//行動処理
			if( Check3DactBusy( pc->sceneAct ) == TRUE ){
				return FALSE;	//強制アニメ中
			}
			if( pc->skillBusyFlag ){
				return FALSE;	//スキル実行中
			}
			pc->hitEnableFlag = TRUE;
		}
		return TRUE;
	}
	return FALSE;
}

//------------------------------------------------------------------
#define JUMP_SPEEDY (FX32_ONE*6)

static BOOL jumpControl( PLAYER_CONTROL* pc )
{
	switch( pc->jumpSeq ){
	case 0:
		return TRUE;
	case 1:
		pc->forceMovingFlag = TRUE;	//コマンド受け入れ不可

		directionSetVEC( pc );
		anmSet( pc, ACTANM_CMD_JUMP_RDY );
		pc->calcMoveEnable = FALSE;
		pc->jumpSeq++;
		break;
	case 2:
		StartMovePHMV( pc->calcPHMV, &pc->moveVec, JUMP_SPEEDY, 0x2000 );
		pc->calcMoveEnable = TRUE;
		anmSet( pc, ACTANM_CMD_JUMPUP );
		pc->jumpSeq++;
		break;
	case 3:
		if( GetMoveSpeedPHMV( pc->calcPHMV ) == 0 ){
			ResetMovePHMV( pc->calcPHMV );
			pc->calcMoveEnable = FALSE;
			anmSet( pc, ACTANM_CMD_JUMP_END );
			pc->forceMovingFlag = FALSE;	//コマンド受け入れ復帰
			pc->jumpSeq++;
		}
		break;
	case 4:
		pc->contCommand = PCC_STAY;
		pc->calcMoveEnable = TRUE;
		pc->jumpSeq = 0;
		break;
	}
	return FALSE;
}

//------------------------------------------------------------------
static BOOL slideControl( PLAYER_CONTROL* pc )
{
	switch( pc->slideSeq ){
	case 0:
		return TRUE;
	case 1:
		//pc->forceMovingFlag = TRUE;	//コマンド受け入れ不可

		ResetMovePHMV( pc->calcPHMV );
		anmSet( pc, ACTANM_CMD_JUMPDOWN );
		pc->slideSeq++;
		break;
	case 2:
		{
			BOOL slideEndFlag = FALSE;
			VecFx32	vecMove;

			if( CheckGroundGravityPHMV( pc->calcPHMV, &pc->contTrans ) == FALSE ){
				slideEndFlag = TRUE;
			} else {
				GetMoveVecPHMV( pc->calcPHMV, &vecMove );
				if( vecMove.y >= 0 ){
					//すべり中にもかかわらず上に進んでいるとき
					//次のアクションがあるまで、ループ回避のため一時的に計算を止める
					pc->calcMoveEnable = FALSE;
					slideEndFlag = TRUE;
				}
			}
			if( slideEndFlag == TRUE ){
				if( GetMoveSpeedPHMV( pc->calcPHMV ) >= RUN_SPEED/2 ){
					//着地時一定以上のスピードが出ている場合は着地アニメを入れる
					anmSet( pc, ACTANM_CMD_JUMP_END );
				}
				ResetMovePHMV( pc->calcPHMV );
				pc->slideSeq++;
			}
		}
		break;
	case 3:
		//pc->forceMovingFlag = FALSE;	//コマンド受け入れ復帰
		pc->slideSeq = 0;
		break;
	}
	return FALSE;
}

//------------------------------------------------------------------
static void moveControl( PLAYER_CONTROL* pc )
{
	//移動制御
	if( pc->calcMoveEnable == TRUE ){
		CalcMovePHMV( pc->calcPHMV, &pc->contTrans );
		Set3DactTrans( pc->sceneAct, &pc->contTrans );
	}
}

//------------------------------------------------------------------
static void keepSpeed( PLAYER_CONTROL* pc, fx32 keepSpeed, fx32 nowSpeed, fx32 addSpeed )
{
	fx32 speed0 = keepSpeed - addSpeed;
	fx32 speed1 = keepSpeed + addSpeed;

	if(( nowSpeed >= speed0 )&&( nowSpeed <= speed1 )){
		SetMoveSpeedPHMV( pc->calcPHMV, keepSpeed );
	} else if( nowSpeed < speed0 ){
		AddMoveSpeedPHMV( pc->calcPHMV, addSpeed );
	} else if( nowSpeed > speed1 ){
		AddMoveSpeedPHMV( pc->calcPHMV, -addSpeed );
	}
}

//------------------------------------------------------------------
static void commandControl( PLAYER_CONTROL* pc )
{
	switch( pc->contCommand ){
	case PCC_NOP:
		break;
	case PCC_STAY:
		ResetMovePHMV( pc->calcPHMV );
		anmSet( pc, ACTANM_CMD_STAY );
		break;
	case PCC_JUMP:
		pc->jumpSeq = 1;
		break;
	case PCC_WALK:
		pc->calcMoveEnable = TRUE;
		StartMovePHMV( pc->calcPHMV, &pc->moveVec, RUN_SPEED, 0 );
		directionSetPHMV( pc );
		anmSet( pc, ACTANM_CMD_WALK );
		break;
	case PCC_RUN:
		pc->calcMoveEnable = TRUE;
		{
			fx32 speed = GetMoveSpeedPHMV( pc->calcPHMV );

			if( speed ){
				SetMoveVecPHMV( pc->calcPHMV, &pc->moveVec );
			} else {
				StartMovePHMV( pc->calcPHMV, &pc->moveVec, RUN_SPEED, 0 );
			}
			directionSetPHMV( pc );

			//平地時はなるべく速度を保つようにする
			if( CheckGroundGravityPHMV( pc->calcPHMV, &pc->contTrans ) == FALSE ){
				keepSpeed( pc, RUN_SPEED, speed, RUN_SPEED/32 );
			}
		}
		anmSet( pc, ACTANM_CMD_RUN );
		break;
	case PCC_BUILD:
		anmSet( pc, ACTANM_CMD_TAKE );
		pc->buildCommand = PBC_BUILD_CASTLE;
		break;
	case PCC_SUMMON:
		anmSet( pc, ACTANM_CMD_TAKE );
		pc->buildCommand = PBC_SUMMON;
		break;
	case PCC_SIT:
		anmSet( pc, ACTANM_CMD_SITDOWN );
		pc->sitDownFlag = TRUE;
		break;
	case PCC_PUTON:		
		anmSetForce( pc, ACTANM_CMD_TAKE );
		break;
	case PCC_TAKEOFF:		
		anmSetForce( pc, ACTANM_CMD_TAKE );
		break;
	case PCC_ATTACK_READY:
		directionSetRAD( pc, pc->contDirection );	//カメラ方向に向き直り
		anmSet( pc, ACTANM_CMD_SWORD_ATTACK0 );
		break;
	case PCC_ATTACK_END:
		anmSet( pc, ACTANM_CMD_STAY );
		break;
	case PCC_ATTACK:
		//武器によってモーション変化
#if 0
		switch( pc->nowAccesary ){
		defasult:
			anmSetForce( pc, ACTANM_CMD_STAY );
			break;
		case 1:
			directionSetRAD( pc, pc->contDirection );	//カメラ方向に向き直り
			anmSetForce( pc, ACTANM_CMD_ATTACK );
			pc->skillCommand = PSC_ATTACK_SWORD;
			break;
		case 2:
			directionSetRAD( pc, pc->contDirection );	//カメラ方向に向き直り
			anmSetForce( pc, ACTANM_CMD_SHOOT );
			pc->skillCommand = PSC_ATTACK_BOW;
			break;
		case 3:
			directionSetRAD( pc, pc->contDirection );	//カメラ方向に向き直り
			anmSetForce( pc, ACTANM_CMD_SPELL );
			pc->skillCommand = PSC_ATTACK_FIRE;
			break;
		}
#else
		switch( pc->attackID ){
		case 0:
			anmSetForce( pc, ACTANM_CMD_SWORD_ATTACK1 );
			pc->skillCommand = PSC_ATTACK_SWORD0;
			break;
		case 1:
			anmSetForce( pc, ACTANM_CMD_SWORD_ATTACK2 );
			pc->skillCommand = PSC_ATTACK_SWORD0;//1;
			break;
		case 2:
			anmSetForce( pc, ACTANM_CMD_SWORD_ATTACK3 );
			pc->skillCommand = PSC_ATTACK_SWORD0;//1;
			break;
		case 3:
			anmSetForce( pc, ACTANM_CMD_SWORD_ATTACK4 );
			pc->skillCommand = PSC_ATTACK_SWORD0;
			break;
		case 4:
			anmSetForce( pc, ACTANM_CMD_SWORD_ATTACK5 );
			pc->skillCommand = PSC_ATTACK_SWORD2;
			break;
		case 5:
			anmSetForce( pc, ACTANM_CMD_SWORD_ATTACK7 );
			pc->skillCommand = PSC_ATTACK_SWORD3;
			break;
		}
#endif
		break;
	case PCC_WEPONCHANGE:		
		//ナンバー切り替え
		pc->nowAccesary++;
		pc->nowAccesary&= 3;
		Set3DactChrAccesory( pc->sceneAct, pc->nowAccesary );
		break;
	case PCC_TEST:		
		pc->work[0]++;
		pc->work[0]&=3;
		ChangeEquipNum( pc->sceneAct, pc->work[0] );
		anmSetForce( pc, ACTANM_CMD_STAY );
		break;
	}
	//pc->contCommand = PCC_NOP;
}

//------------------------------------------------------------------
void MainPlayerControl( PLAYER_CONTROL* pc )
{
	anmReset( pc );			//通信アニメコマンドリセット

	moveControl( pc );

	if( damageControl( pc ) == FALSE ){	//ダメージ判定実行中かどうか
		return;
	}
	if( jumpControl( pc ) == FALSE ){
		if( pc->contCommand == PCC_JUMP ){
			pc->jumpSeq = 1;
			pc->contCommand = PCC_NOP;
		}
		return;
	}
	if( slideControl( pc ) == FALSE ){
		if( pc->contCommand == PCC_JUMP ){
			pc->jumpSeq = 1;
			pc->slideSeq = 0;
			pc->contCommand = PCC_NOP;
		}
		return;
	}
	if(( pc->contCommand != PCC_SIT )&&( pc->sitDownFlag == TRUE )){	//しゃがみ中立ち判定処理
		anmSetForce( pc, ACTANM_CMD_STANDUP );
		pc->sitDownFlag = FALSE;
		return;
	}
	if(( CheckGroundGravityPHMV( pc->calcPHMV, &pc->contTrans ) == TRUE )
		&&( GetMoveSpeedPHMV( pc->calcPHMV ) < RUN_SPEED / 4 )
		&&( pc->calcMoveEnable == TRUE )){
		//斜面にいるとき、一定速度以下ならすべり状態にする
		//pc->slideSeq = 1;
		return;
	}

	commandControl( pc );
}

//------------------------------------------------------------------
void MainNetWorkPlayerControl( PLAYER_CONTROL* pc, PLAYER_STATUS_NETWORK* psn )
{
	SetPlayerControlTrans( pc, &psn->trans );
	directionSetRAD( pc, psn->direction );
	pc->skillCommand = psn->skillCommand;
	pc->buildCommand = psn->buildCommand;

	if( damageControl( pc ) == FALSE ){	//ダメージ判定実行中かどうか
		return;
	}
	if( psn->anmSetFlag == TRUE ){
		Set3DactChrAnimeForceCmd( pc->sceneAct, psn->anmSetID );
		psn->anmSetFlag = FALSE;
	}
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
	Set3DactTrans( pc->sceneAct, &pc->contTrans );
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
 * @brief	プレーヤーコントロール可能かどうかのチェック
 */
//------------------------------------------------------------------
BOOL CheckPlayerControlEnable( PLAYER_CONTROL* pc )
{
	if( pc->forceMovingFlag == TRUE ){
		return FALSE;	//強制移動中
	}
	if( Check3DactBusy( pc->sceneAct ) == TRUE ){
		return FALSE;	//強制アニメ中
	}
	if( pc->skillBusyFlag ){
		return FALSE;	//スキル実行中
	}
	return TRUE;
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
 * @brief	プレーヤー攻撃コマンドの設定
 */
//------------------------------------------------------------------
void SetPlayerAttackCommand
	( PLAYER_CONTROL* pc, const PLAYER_CONTROL_COMMAND command, int attackID )
{
	pc->contCommand = command;
	pc->attackID = attackID;
}

//------------------------------------------------------------------
/**
 * @brief	プレーヤー移動コマンドの設定（XZ軸）
 */
//------------------------------------------------------------------
void SetPlayerMoveCommand
	( PLAYER_CONTROL* pc, PLAYER_CONTROL_COMMAND command, VecFx32* mvDir )
{
	VecFx32 tmpVec, rotVec;

	pc->contCommand = command;
	//進行ベクトルの算出
	VEC_Subtract( mvDir, &pc->contTrans, &tmpVec );
	tmpVec.y = 0;	//XZ軸についての精度をあげるため0固定
	VEC_Normalize( &tmpVec, &pc->moveVec );	//正規化
}

//------------------------------------------------------------------
/**
 * @brief	スキルコマンドの取得とリセット
 */
//------------------------------------------------------------------
PLAYER_SKILL_COMMAND GetPlayerSkillCommand( PLAYER_CONTROL* pc )
{
	return pc->skillCommand;
}

void ResetPlayerSkillCommand( PLAYER_CONTROL* pc )
{
	pc->skillCommand = PSC_NOP;
}

//------------------------------------------------------------------
/**
 * @brief	建築コマンドの取得とリセット
 */
//------------------------------------------------------------------
PLAYER_BUILD_COMMAND GetPlayerBuildCommand( PLAYER_CONTROL* pc )
{
	return pc->buildCommand;
}

void ResetPlayerBuildCommand( PLAYER_CONTROL* pc )
{
	pc->buildCommand = PBC_NOP;
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
 * @brief	死亡フラグの取得とリセット
 */
//------------------------------------------------------------------
BOOL GetPlayerDeadFlag( PLAYER_CONTROL* pc )
{
	return pc->deadFlag;
}

void ResetPlayerDeadFlag( PLAYER_CONTROL* pc )
{
	pc->deadFlag = FALSE;
}

//------------------------------------------------------------------
/**
 * @brief	ネットＩＤの取得
 */
//------------------------------------------------------------------
int GetPlayerNetID( PLAYER_CONTROL* pc )
{
	return pc->netID;
}

//------------------------------------------------------------------
/**
 * @brief	ネットワーク用ステータスの取得とリセット
 */
//------------------------------------------------------------------
void GetPlayerNetStatus( PLAYER_CONTROL* pc, PLAYER_STATUS_NETWORK* pNetStatus )
{
	pNetStatus->trans = pc->contTrans;
	pNetStatus->direction = pc->nowDirection;
	pNetStatus->anmSetFlag = pc->anmSetFlag;
	pNetStatus->anmSetID = pc->anmSetID;
	pNetStatus->skillCommand = pc->skillCommand;
	pNetStatus->buildCommand = pc->buildCommand;
}

void ResetPlayerNetStatus( PLAYER_STATUS_NETWORK* pNetStatus )
{
	pNetStatus->trans.x = 0;
	pNetStatus->trans.y = 0;
	pNetStatus->trans.z = 0;
	pNetStatus->direction = 0;
	pNetStatus->anmSetFlag = FALSE;
	pNetStatus->anmSetID = 0;
	pNetStatus->skillCommand = 0;
	pNetStatus->buildCommand = 0;
}

//------------------------------------------------------------------
/**
 * @brief	ＨＰ減少値の設定
 */
//------------------------------------------------------------------
void SetPlayerDamage( PLAYER_CONTROL* pc, const s16 damage )
{
	if( pc->status.hp ){
		pc->damage = damage;
	}
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
 * @brief	プレーヤー移動開始設定
 *			※基本的にはスキル等の外部コントロール用
 *			　動作コマンドとは無関係に移動させるため呼び出しタイミングは注意
 *			止める場合はスピード0
 */
//------------------------------------------------------------------
void SetPlayerMove( PLAYER_CONTROL* pc, VecFx32* mvDir, fx32 speed, u16 theta )
{
	if( speed ){
		VEC_Normalize( mvDir, &pc->moveVec );	//正規化

		directionSetVEC( pc );
		StartMovePHMV( pc->calcPHMV, &pc->moveVec, speed, theta );
	} else {
		ResetMovePHMV( pc->calcPHMV );
	}
	pc->calcMoveEnable = TRUE;
}

//------------------------------------------------------------------
/**
 * @brief	プレーヤー移動終了判定
 */
//------------------------------------------------------------------
BOOL CheckPlayerMoveEnd( PLAYER_CONTROL* pc )
{
	if( GetMoveSpeedPHMV( pc->calcPHMV ) == 0 ){
		return TRUE;
	} else {
		return FALSE;
	}
}

