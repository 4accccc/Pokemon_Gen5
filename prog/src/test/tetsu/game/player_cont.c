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
#include "include\system\gfl_use.h"

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

	PLAYER_CONTROL_COMMAND	contCommand;
	PLAYER_SKILL_COMMAND	skillCommand;

	BOOL					forceMovingFlag;
	BOOL					skillBusyFlag;
	BOOL					hitEnableFlag;

	u16						contDirection;
	u16						nowDirection;
	VecFx32					contTrans;
	VecFx32					moveVec;

	PLAYER_STATUS			status;
	BOOL					anmSetFlag;
	int						anmSetID;

	CALC_PH_MV*				calcPHMV;
	BOOL					calcMoveEnable;

	GFL_BBDACT_RESUNIT_ID	bbdActResUnitID;
	GFL_BBDACT_ACTUNIT_ID	bbdActActUnitID;

	u16						work[8];
};

#define WALK_SPEED	(FX32_ONE*2)
#define RUN_SPEED	(FX32_ONE*4)
#define JUMP_SPEED	(FX32_ONE*6)

enum {
	ANMTYPE_STOP = 0,
	ANMTYPE_WALK,
	ANMTYPE_RUN,
	ANMTYPE_JUMP,
};

static const PLAYER_STATUS	statusDefault = { {"Null"},1000, 100, 1000, 100, 50, 50, 50, 50, 50 };

static void playerBBDactSetUp( PLAYER_CONTROL* pc );
static void playerBBDactRelease( PLAYER_CONTROL* pc );
static void playerBBDactSetAnm( PLAYER_CONTROL* pc );
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
	playerBBDactSetUp( pc );

	pc->contCommand = PCC_NOP;
	pc->skillCommand = PSC_NOP;

	pc->forceMovingFlag = FALSE;
	pc->skillBusyFlag = FALSE;
	pc->hitEnableFlag = FALSE;

	pc->contDirection = 0;
	pc->nowDirection = 0;

	pc->contTrans.x = 0;
	pc->contTrans.y = 0;
	pc->contTrans.z = 0;

	pc->anmSetFlag = TRUE;
	pc->anmSetID = ANMTYPE_STOP;

	pc->jumpSeq = 0;
	pc->status = statusDefault;

	{
		PHMV_SETUP setup;

		setup.getGroundVecN_func = GLOBAL_GetGroundPlaneVecN;
		setup.getGroundHeight_func = GLOBAL_GetGroundPlaneHeight;

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
	playerBBDactRelease( pc );
	GFL_HEAP_FreeMemory( pc ); 
}

//------------------------------------------------------------------
/**
 * @brief	プレーヤーメインコントロール
 */
//------------------------------------------------------------------
static void anmReset( PLAYER_CONTROL* pc )
{
}

//------------------------------------------------------------------
static void anmSet( PLAYER_CONTROL* pc, int anmID )
{
	if( pc->anmSetID != anmID ){
		pc->anmSetFlag = TRUE;
		pc->anmSetID = anmID;

		playerBBDactSetAnm( pc );
	}
}

//------------------------------------------------------------------
static void anmSetForce( PLAYER_CONTROL* pc, int anmID )
{
	pc->anmSetFlag = TRUE;
	pc->anmSetID = anmID;

	playerBBDactSetAnm( pc );
}

//------------------------------------------------------------------
static void directionSetRAD( PLAYER_CONTROL* pc, u16 direction )
{
	VecFx32	rotVec;		

	pc->nowDirection = direction;
	rotVec.x = 0;
	rotVec.y = direction + 0x8000;
	rotVec.z = 0;
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
}

//------------------------------------------------------------------
static BOOL jumpControl( PLAYER_CONTROL* pc )
{
	switch( pc->jumpSeq ){
	case 0:
		return TRUE;
	case 1:
		pc->forceMovingFlag = TRUE;	//コマンド受け入れ不可

		directionSetVEC( pc );
		anmSet( pc, ANMTYPE_STOP );
		pc->calcMoveEnable = FALSE;
		pc->jumpSeq++;
		break;
	case 2:
		StartMovePHMV( pc->calcPHMV, &pc->moveVec, JUMP_SPEED, 0x3000 );
		pc->calcMoveEnable = TRUE;
		anmSet( pc, ANMTYPE_JUMP );
		pc->jumpSeq++;
		break;
	case 3:
		if( GetMoveSpeedPHMV( pc->calcPHMV ) == 0 ){
			ResetMovePHMV( pc->calcPHMV );
			pc->calcMoveEnable = FALSE;
			anmSet( pc, ANMTYPE_STOP );
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
static void moveControl( PLAYER_CONTROL* pc )
{
	//移動制御
	if( pc->calcMoveEnable == TRUE ){
		CalcMovePHMV( pc->calcPHMV, &pc->contTrans );
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
		anmSet( pc, ANMTYPE_STOP );
		break;
	case PCC_JUMP:
		pc->jumpSeq = 1;
		break;
	case PCC_WALK:
		pc->calcMoveEnable = TRUE;
		StartMovePHMV( pc->calcPHMV, &pc->moveVec, WALK_SPEED, 0 );
		directionSetPHMV( pc );
		anmSet( pc, ANMTYPE_WALK );
		break;
	case PCC_RUN:
		pc->calcMoveEnable = TRUE;
		StartMovePHMV( pc->calcPHMV, &pc->moveVec, RUN_SPEED, 0 );
		directionSetPHMV( pc );
		anmSet( pc, ANMTYPE_RUN );
		break;
	}
}

//------------------------------------------------------------------
void MainPlayerControl( PLAYER_CONTROL* pc )
{
	anmReset( pc );			//通信アニメコマンドリセット

	moveControl( pc );

	if( jumpControl( pc ) == FALSE ){
		if( pc->contCommand == PCC_JUMP ){
			pc->jumpSeq = 1;
			pc->contCommand = PCC_NOP;
		}
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

	if( psn->anmSetFlag == TRUE ){
		pc->anmSetID = psn->anmSetID;
		playerBBDactSetAnm( pc );

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


//------------------------------------------------------------------
/**
 *
 * @brief	ビルボード
 *
 */
//------------------------------------------------------------------
#include "arc/test_graphic/fld_act.naix"

static const GFL_BBDACT_RESDATA playerBBDactResTable[] = {
	{ ARCID_FLDACT, NARC_fld_act_tex32x32_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x1024, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
//	{ ARCID_FLDACT, NARC_fld_act_hero_nsbtx,
//		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x1024, 32, 32, GFL_BBDACT_RESTYPE_TRANSSRC },
};

static const GFL_BBDACT_ANM stopLAnm[] = {
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};
static const GFL_BBDACT_ANM stopRAnm[] = {
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};
static const GFL_BBDACT_ANM stopUAnm[] = {
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};
static const GFL_BBDACT_ANM stopDAnm[] = {
	{ 21, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};

static const GFL_BBDACT_ANM walkLAnm[] = {
	{ 1, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 3, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM walkRAnm[] = {
	{ 1, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 3, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM walkUAnm[] = {
	{ 9, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 20, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM walkDAnm[] = {
	{ 22, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 21, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 23, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 21, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};

static const GFL_BBDACT_ANM runLAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 14, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 16, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 14, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM runRAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 14, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 16, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 14, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM runUAnm[] = {
	{ 8, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 7, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 10, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 7, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM runDAnm[] = {
	{ 12, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 11, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 13, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 11, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};

static const GFL_BBDACT_ANM jumpLAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};
static const GFL_BBDACT_ANM jumpRAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};
static const GFL_BBDACT_ANM jumpUAnm[] = {
	{ 8, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};
static const GFL_BBDACT_ANM jumpDAnm[] = {
	{ 12, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};

enum {
	ACTSTOP_UP = 0,
	ACTSTOP_DOWN,
	ACTSTOP_LEFT,
	ACTSTOP_RIGHT,

	ACTWALK_UP,
	ACTWALK_DOWN,
	ACTWALK_LEFT,
	ACTWALK_RIGHT,

	ACTRUN_UP,
	ACTRUN_DOWN,
	ACTRUN_LEFT,
	ACTRUN_RIGHT,

	ACTJUMP_UP,
	ACTJUMP_DOWN,
	ACTJUMP_LEFT,
	ACTJUMP_RIGHT,
};

static const GFL_BBDACT_ANM* playerBBDactAnmTable[] = { 
	stopUAnm, stopDAnm, stopLAnm, stopRAnm,
	walkUAnm, walkDAnm, walkLAnm, walkRAnm,
	runUAnm, runDAnm, runLAnm, runRAnm,
	jumpUAnm, jumpDAnm, jumpLAnm, jumpRAnm,
};

static const int playerBBDanmOffsTblMine[] = { 
	ACTSTOP_LEFT - ACTSTOP_UP,
	ACTSTOP_DOWN - ACTSTOP_UP,
	ACTSTOP_RIGHT - ACTSTOP_UP,
	ACTSTOP_UP - ACTSTOP_UP,
};

static u16 getCameraRotate( GFL_G3D_CAMERA* g3Dcamera )
{
	VecFx32 vec, camPos, target;
	
	GFL_G3D_CAMERA_GetPos( g3Dcamera, &camPos );
	GFL_G3D_CAMERA_GetTarget( g3Dcamera, &target );

	VEC_Subtract( &target, &camPos, &vec );
	return FX_Atan2Idx( -vec.z, vec.x ) - 0x4000;
}

static int getPlayerBBDanm( int anmSetID, u16 dir, const int* anmOffsTable )
{
	int	anmBase, datOffs;

	if( (dir > 0x2000)&&(dir < 0x6000)){
		datOffs = 0;
	} else if( (dir >= 0x6000)&&(dir <= 0xa000)){
		datOffs = 1;
	} else if( (dir > 0xa000)&&(dir < 0xe000)){
		datOffs = 2;
	} else {
		datOffs = 3;
	}
	switch( anmSetID ){
	default:
	case ANMTYPE_STOP:
		anmBase = ACTSTOP_UP;
		break;
	case ANMTYPE_WALK:
		anmBase = ACTWALK_UP;
		break;
	case ANMTYPE_RUN:
		anmBase = ACTRUN_UP;
		break;
	case ANMTYPE_JUMP:
		anmBase = ACTJUMP_UP;
		break;
	}
	return anmBase + anmOffsTable[ datOffs ];
}

static void playerBBDactFunc( GFL_BBDACT_SYS* bbdActSys, int actIdx, void* work )
{
	GFL_BBD_SYS*	bbdSys = GFL_BBDACT_GetBBDSystem( bbdActSys );
	PLAYER_CONTROL*	pc = (PLAYER_CONTROL*)work;
	VecFx32	trans = pc->contTrans;
	int		anmID;
	u16		dir;

	dir = pc->nowDirection - getCameraRotate( Get_GS_G3Dcamera( pc->gs, MAINCAMERA_ID ) );
	anmID = getPlayerBBDanm( pc->anmSetID, dir, playerBBDanmOffsTblMine );

	//カメラ補正(アニメ向きの変更をするのに参照)
	GFL_BBDACT_SetAnimeIdxContinue( Get_GS_BillboardActSys( pc->gs ), actIdx, anmID );
	//位置補正
	trans.x = pc->contTrans.x;
	trans.y = pc->contTrans.y + FX32_ONE*7;	//補正
	trans.z = pc->contTrans.z;
	GFL_BBD_SetObjectTrans( bbdSys, actIdx, &trans );
}

static void playerBBDactSetUp( PLAYER_CONTROL* pc )
{
	GFL_BBDACT_SYS* bbdActSys = Get_GS_BillboardActSys( pc->gs );	
	GFL_BBDACT_ACTDATA actData;
	GFL_BBDACT_ACTUNIT_ID actUnitID;
	int		i, objIdx;
	VecFx32	trans;
	u8		alpha;
	BOOL	drawEnable;
	u16		setActNum;

	//リソースセットアップ
	pc->bbdActResUnitID = GFL_BBDACT_AddResourceUnit( bbdActSys, playerBBDactResTable,
														NELEMS(playerBBDactResTable) );
	actData.resID = 0;
	actData.sizX = FX16_ONE*8-1;
	actData.sizY = FX16_ONE*8-1;
	
	actData.trans.x = 0;
	actData.trans.y = 0;
	actData.trans.z = 0;

	actData.alpha = 31;
	actData.drawEnable = TRUE;
	actData.lightMask = GFL_BBD_LIGHTMASK_01;
	actData.func = playerBBDactFunc;
	actData.work = pc;

	pc->bbdActActUnitID = GFL_BBDACT_AddAct( bbdActSys, pc->bbdActResUnitID, &actData, 1 );
	//GFL_BBDACT_BindActTexRes( bbdActSys, pc->bbdActActUnitID, pc->bbdActResUnitID+1 );
	GFL_BBDACT_BindActTexResLoad
		( bbdActSys, pc->bbdActActUnitID, ARCID_FLDACT, NARC_fld_act_hero_nsbtx );

	GFL_BBDACT_SetAnimeTable( bbdActSys, pc->bbdActActUnitID, 
								playerBBDactAnmTable, NELEMS(playerBBDactAnmTable) );
	GFL_BBDACT_SetAnimeIdxOn( bbdActSys, pc->bbdActActUnitID, 0 );
}

static void playerBBDactRelease( PLAYER_CONTROL* pc )
{
	GFL_BBDACT_SYS* bbdActSys = Get_GS_BillboardActSys( pc->gs );	

	GFL_BBDACT_RemoveAct( bbdActSys, pc->bbdActActUnitID, 1 );
	GFL_BBDACT_RemoveResourceUnit(	bbdActSys, pc->bbdActResUnitID, NELEMS(playerBBDactResTable) );
}

static void playerBBDactSetAnm( PLAYER_CONTROL* pc )
{
	int		anmID;
	u16		dir;

	dir = pc->nowDirection - getCameraRotate( Get_GS_G3Dcamera( pc->gs, MAINCAMERA_ID ) );
	anmID = getPlayerBBDanm( pc->anmSetID, dir, playerBBDanmOffsTblMine );

	GFL_BBDACT_SetAnimeIdx( Get_GS_BillboardActSys( pc->gs ), pc->bbdActActUnitID, anmID );
}

