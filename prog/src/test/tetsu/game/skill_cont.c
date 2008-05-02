//============================================================================================
/**
 * @file	skill_cont.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"

#include "setup.h"
#include "team_cont.h"
#include "skill_cont.h"

//============================================================================================
//
//
//	スキルコントロール
//
//
//============================================================================================
#define	SKILL_SETUP_MAX	(64)
//------------------------------------------------------------------
/**
 * @brief	構造体定義
 */
//------------------------------------------------------------------
typedef struct {
	PLAYER_SKILL_COMMAND	skillCommand;
	u16						seq;
	TEAM_CONTROL*			tc;			//発生させたプレーヤーのチーム
	PLAYER_CONTROL*			pc;			//発生させたプレーヤー
	VecFx32					trans;
	u16						direction;
	u32						work[16];
}SKILL_WORK;

typedef struct {
	SKILL_WORK*		sw;
	u16				work[8];
}SKILL_EFFECT_WORK;

struct _SKILL_CONTROL {
	HEAPID			heapID;
	GAME_SYSTEM*	gs;
	TEAM_CONTROL**	p_tc;
	int				teamCount;

	SKILL_WORK		skill[SKILL_SETUP_MAX];
	u16				g3DutilUnitIdx;
	u16				g3DutilObjIdx;
	BOOL			onGameFlag;
};

typedef void	(SKILL_FUNC)( SKILL_CONTROL*, SKILL_WORK* );

typedef struct {
	SKILL_FUNC*	init;
	SKILL_FUNC*	main;
}SKILL_PROC;

//------------------------------------------------------------------
/**
 * @brief	３Ｄセットアップデータ
 */
//------------------------------------------------------------------
#include "graphic_data/test_graphic/test3d.naix"

//------------------------------------------------------------------
enum {
	G3DRES_EFFECT_ARROW = 0,
};

//３Ｄグラフィックリソーステーブル
static const GFL_G3D_UTIL_RES g3DresTbl[] = {
	{ ARCID_SAMPLE, NARC_haruka_effect_arrow_nsbmd,  GFL_G3D_UTIL_RESARC },
};

//------------------------------------------------------------------
enum {
	G3DOBJ_EFFECT_ARROW = 0,
};

//３Ｄオブジェクト定義テーブル
static const GFL_G3D_UTIL_OBJ g3DobjTbl[] = {
	{ G3DRES_EFFECT_ARROW, 0, G3DRES_EFFECT_ARROW, NULL, 0 },
};

//------------------------------------------------------------------
//設定テーブルデータ
static const GFL_G3D_UTIL_SETUP g3Dsetup = {
	g3DresTbl, NELEMS(g3DresTbl),
	g3DobjTbl, NELEMS(g3DobjTbl),
};

//------------------------------------------------------------------
static void moveTraceSkill( GFL_G3D_SCENEOBJ* sceneObj, void* work );

typedef struct {
	const GFL_G3D_SCENEOBJ_DATA*	data;
	int								count;
}SKILL_EFFECT_TBL;

static const GFL_G3D_SCENEOBJ_DATA skillEffectData1[] = {
	{ 
		G3DOBJ_EFFECT_ARROW, 0, 1, 24, FALSE, TRUE, 
		{	{ 0, 0, 0 },
			{ FX32_ONE/8 , FX32_ONE/8, FX32_ONE/8 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},moveTraceSkill,
	},
};
static const GFL_G3D_SCENEOBJ_DATA skillEffectData2[] = {
	{ 
		G3DOBJ_EFFECT_ARROW, 0, 1, 24, FALSE, TRUE, 
		{	{ 0, 0, 0 },
			{ FX32_ONE , FX32_ONE, FX32_ONE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},moveTraceSkill,
	},
};
static const GFL_G3D_SCENEOBJ_DATA skillEffectData3[] = {
	{ 
		G3DOBJ_EFFECT_ARROW, 0, 1, 24, FALSE, TRUE, 
		{	{ 0, 0, 0 },
			{ FX32_ONE/2 , FX32_ONE/2, FX32_ONE/2 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},moveTraceSkill,
	},
};

typedef enum {
	SKILL_EFFECT_TEST1 = 0,
	SKILL_EFFECT_TEST2,
	SKILL_EFFECT_TEST3,
}SKILL_EFFECTID;

static const SKILL_EFFECT_TBL skillEffectTable[] = {
	{ skillEffectData1, NELEMS(skillEffectData1) },
	{ skillEffectData2, NELEMS(skillEffectData2) },
	{ skillEffectData3, NELEMS(skillEffectData3) },
};

static const SKILL_PROC skillProcTable[7];
static void SkillEffectAdd( SKILL_CONTROL* sc, int* effectID, SKILL_EFFECTID seID, SKILL_WORK* sw );
static void SkillEffectRemove( SKILL_CONTROL* sc, int* effectID, SKILL_EFFECTID seID );
//------------------------------------------------------------------
/**
 * @brief	スキルコントロールセット
 */
//------------------------------------------------------------------
SKILL_CONTROL* AddSkillControl( SKILLCONT_SETUP* setup )
{
	SKILL_CONTROL* sc = GFL_HEAP_AllocClearMemory( setup->heapID, sizeof(SKILL_CONTROL) );
	TEAM_CONTROL** p_tc = GFL_HEAP_AllocClearMemory
							( setup->heapID, setup->teamCount * sizeof(TEAM_CONTROL*) );
	int i, j;

	for( i=0; i<setup->teamCount; i++ ){
		p_tc[i] = setup->p_tc[i];
	}

	sc->heapID = setup->heapID;
	sc->gs = setup->gs;
	sc->p_tc = p_tc;
	sc->teamCount = setup->teamCount;

	for( i=0; i<SKILL_SETUP_MAX; i++ ){
		sc->skill[i].skillCommand = PSC_NOP;
		sc->skill[i].seq = 0;
		sc->skill[i].pc = NULL;
		for( j=0; j<8; j++ ){
			sc->skill[i].work[j] = 0;
		}
	}
	//３Ｄデータセットアップ
	{
		GFL_G3D_SCENE* g3Dscene = Get_GS_G3Dscene( sc->gs );
		sc->g3DutilUnitIdx = GFL_G3D_SCENE_AddG3DutilUnit( g3Dscene, &g3Dsetup );
		sc->g3DutilObjIdx = GFL_G3D_SCENE_GetG3DutilUnitObjIdx( g3Dscene, sc->g3DutilUnitIdx );
	}
	return sc;
}

//------------------------------------------------------------------
/**
 * @brief	スキルコントロール終了
 */
//------------------------------------------------------------------
void RemoveSkillControl( SKILL_CONTROL* sc )
{
	GFL_G3D_SCENE_DelG3DutilUnit( Get_GS_G3Dscene( sc->gs ), sc->g3DutilUnitIdx );

	GFL_HEAP_FreeMemory( sc->p_tc ); 
	GFL_HEAP_FreeMemory( sc ); 
}

//------------------------------------------------------------------
/**
 * @brief	スキルの追加
 */
//------------------------------------------------------------------
void SetSkillControlCommand
	( SKILL_CONTROL* sc, TEAM_CONTROL* tc, PLAYER_CONTROL* pc, PLAYER_SKILL_COMMAND skillCommand )
{
	int i;

	if( skillCommand ){
		for( i=0; i<SKILL_SETUP_MAX; i++ ){
			if( sc->skill[i].skillCommand == PSC_NOP ){
				sc->skill[i].skillCommand = skillCommand;
				sc->skill[i].seq = 0;
				sc->skill[i].tc = tc;
				sc->skill[i].pc = pc;			//発生させたプレーヤー
				GetPlayerControlTrans( pc, &sc->skill[i].trans );
				GetPlayerDirection( pc, &sc->skill[i].direction );

				SetPlayerSkillBusyFlag( pc );
				( skillProcTable[sc->skill[i].skillCommand].init )( sc, &sc->skill[i] );
				return;
			}
		}
		GF_ASSERT(0);
	}
}

//------------------------------------------------------------------
/**
 * @brief	スキルメインコントロール
 */
//------------------------------------------------------------------
void MainSkillControl( SKILL_CONTROL* sc, BOOL onGameFlag )
{
	int i;

	sc->onGameFlag = onGameFlag;

	for( i=0; i<SKILL_SETUP_MAX; i++ ){
		if( sc->skill[i].skillCommand != PSC_NOP ){
			( skillProcTable[sc->skill[i].skillCommand].main )( sc, &sc->skill[i] );
		}
	}
}





//------------------------------------------------------------------
/**
 *
 *
 *
 * @brief	スキルプログラム
 *
 *
 *
 */
//------------------------------------------------------------------
static BOOL calc_XZhitEasy( VecFx32* vec1, VecFx32* vec2, fx32 len )
{
	fx32	diffX, diffZ, diffLen;
				
	diffX = ( vec2->x - vec1->x ) >> FX32_SHIFT;
	diffZ = ( vec2->z - vec1->z ) >> FX32_SHIFT;
	diffLen = ( diffX * diffX + diffZ * diffZ );
	if( diffLen > len ){
		return FALSE;
	} else {
		return TRUE;
	}
}

static void calc_XZtrans( VecFx32* trans, fx32 lenoffs, u16 direction  )
{
	trans->x += ( -lenoffs * FX_SinIdx( direction )/ FX16_ONE );
	trans->z += ( -lenoffs * FX_CosIdx( direction )/ FX16_ONE );
}

static void calc_VecDir( u16 direction, VecFx32* vecDir )
{
	vecDir->x = -FX_SinIdx( direction );
	vecDir->y = 0;
	vecDir->z = -FX_CosIdx( direction );
}

//------------------------------------------------------------------
//
//	共通判定
//
//------------------------------------------------------------------
typedef struct {
	PLAYER_CONTROL* pc;			//発生させたプレーヤー
	VecFx32*		skillTrans;
	fx32			hitLen;
	s16				damage;
	u8				dotCount;
	u8				dotTimer;
	u8				dotValue;
	BOOL			result;
}SKILL_CALLBACK_WORK;

static BOOL	SkillHitCheckPlayer( PLAYER_CONTROL* pc, SKILL_CALLBACK_WORK* scw )
{
	VecFx32 targetTrans;

	if( scw->result == FALSE ){
		if( GetPlayerHitEnableFlag( pc ) == TRUE ){
			GetPlayerControlTrans( pc, &targetTrans );

			if( calc_XZhitEasy( scw->skillTrans, &targetTrans, scw->hitLen ) == TRUE ){
				SetPlayerDamage( pc, scw->damage );
				if( scw->dotValue ){
					SetPlayerDamageOnTime( pc, scw->dotCount, scw->dotTimer, scw->dotValue );
				}
				return TRUE;
			}
		}
	}
	return FALSE;
}

static BOOL	SkillHitCheckSummon( TEAM_CONTROL* tc, int summonID, SKILL_CALLBACK_WORK* scw )
{
	VecFx32 targetTrans;
	int damage = -scw->damage;

	if( scw->result == FALSE ){
		GetSummonObjectTrans( tc, summonID, &targetTrans );
		if( calc_XZhitEasy( scw->skillTrans, &targetTrans, scw->hitLen ) == TRUE ){
			SetSummonObjectDamage( tc, summonID, &damage );
			return TRUE;
		}
	}
	return FALSE;
}

//------------------------------------------------------------------
static void	SkillHitCheckOnePlayerCallBack( PLAYER_CONTROL* pc, int num, void* work );
static void	SkillHitCheckOneSummonCallBack( TEAM_CONTROL* tc, int summonID, int num, void* work );

static BOOL DamageSetOne( SKILL_CONTROL* sc, SKILL_WORK* sw, fx32 hitLen, 
							s16 damage, u8 dotCount, u8 dotTimer, s8 dotValue )
{
	SKILL_CALLBACK_WORK* scw = GFL_HEAP_AllocClearMemory(sc->heapID, sizeof(SKILL_CALLBACK_WORK));
	int i;
	VecFx32 targetTrans;
	BOOL hitResult;

	scw->pc			= sw->pc;
	scw->skillTrans = &sw->trans;
	scw->hitLen		= hitLen;
	scw->damage		= damage;
	scw->dotCount	= dotCount;
	scw->dotTimer	= dotTimer;
	scw->dotValue	= dotValue;
	scw->result		= FALSE;

	for( i=0; i<sc->teamCount; i++ ){
		if( sc->p_tc[i] != sw->tc ){
			if( sc->onGameFlag == TRUE ){
				ProcessingAllTeamPlayer
					( sc->p_tc[i], SkillHitCheckOnePlayerCallBack, (void*)scw );
				ProcessingAllTeamSummonObject
					( sc->p_tc[i], SkillHitCheckOneSummonCallBack, (void*)scw );
			}
		}
		if( scw->result == TRUE ){
			break;
		}
	}
	hitResult = scw->result;
	GFL_HEAP_FreeMemory(scw);

	return hitResult;
}

static void	SkillHitCheckOnePlayerCallBack( PLAYER_CONTROL* pc, int num, void* work )
{
	SKILL_CALLBACK_WORK* scw = (SKILL_CALLBACK_WORK*)work;

	scw->result = SkillHitCheckPlayer( pc, scw );
}

static void	SkillHitCheckOneSummonCallBack( TEAM_CONTROL* tc, int summonID, int num, void* work )
{
	SKILL_CALLBACK_WORK* scw = (SKILL_CALLBACK_WORK*)work;

	scw->result = SkillHitCheckSummon( tc, summonID, scw );
}

//------------------------------------------------------------------
static void	SkillHitCheckAllPlayerCallBack( PLAYER_CONTROL* pc, int num, void* work );
static void	SkillHitCheckAllSummonCallBack( TEAM_CONTROL* tc, int summonID, int num, void* work );

static BOOL DamageSetAll( SKILL_CONTROL* sc, SKILL_WORK* sw, fx32 hitLen, 
							s16 damage, u8 dotCount, u8 dotTimer, s8 dotValue )
{
	SKILL_CALLBACK_WORK* scw = GFL_HEAP_AllocClearMemory(sc->heapID, sizeof(SKILL_CALLBACK_WORK));
	int i;
	VecFx32 targetTrans;
	BOOL hitResult;

	scw->pc			= sw->pc;
	scw->skillTrans = &sw->trans;
	scw->hitLen		= hitLen;
	scw->damage		= damage;
	scw->dotCount	= dotCount;
	scw->dotTimer	= dotTimer;
	scw->dotValue	= dotValue;
	scw->result		= FALSE;

	for( i=0; i<sc->teamCount; i++ ){
		if( sc->p_tc[i] != sw->tc ){
			if( sc->onGameFlag == TRUE ){
				ProcessingAllTeamPlayer
					( sc->p_tc[i], SkillHitCheckAllPlayerCallBack, (void*)scw );
				ProcessingAllTeamSummonObject
					( sc->p_tc[i], SkillHitCheckAllSummonCallBack, (void*)scw );
			}
		}
	}
	hitResult = scw->result;
	GFL_HEAP_FreeMemory(scw);

	return hitResult;
}

static void	SkillHitCheckAllPlayerCallBack( PLAYER_CONTROL* pc, int num, void* work )
{
	SKILL_CALLBACK_WORK* scw = (SKILL_CALLBACK_WORK*)work;

	SkillHitCheckPlayer( pc, scw );
}

static void	SkillHitCheckAllSummonCallBack( TEAM_CONTROL* tc, int summonID, int num, void* work )
{
	SKILL_CALLBACK_WORK* scw = (SKILL_CALLBACK_WORK*)work;

	SkillHitCheckSummon( tc, summonID, scw );
}

//------------------------------------------------------------------
//	
//
//	各種スキル別関数
//
//
//------------------------------------------------------------------
static void DeleteSkill( SKILL_WORK* sw )
{
	sw->skillCommand = PSC_NOP;
	ResetPlayerSkillBusyFlag( sw->pc );
}
	
//回転マトリクス変換
static inline void rotateCalc( VecFx32* rotSrc, MtxFx33* rotDst )
{
	MtxFx33 tmp;

	MTX_RotX33(	rotDst, FX_SinIdx((u16)rotSrc->x), FX_CosIdx((u16)rotSrc->x) );

	MTX_RotY33(	&tmp, FX_SinIdx((u16)rotSrc->y), FX_CosIdx((u16)rotSrc->y) );
	MTX_Concat33( rotDst, &tmp, rotDst );

	MTX_RotZ33(	&tmp, FX_SinIdx((u16)rotSrc->z), FX_CosIdx((u16)rotSrc->z) );
	MTX_Concat33( rotDst, &tmp, rotDst );
}

//------------------------------------------------------------------
//　なにもしない
//------------------------------------------------------------------
static void NopInit( SKILL_CONTROL* sc, SKILL_WORK* sw )
{
}

static void NopMain( SKILL_CONTROL* sc, SKILL_WORK* sw )
{
}

//------------------------------------------------------------------
//　剣０
//------------------------------------------------------------------
#define	SWORD0_HITLEN	(FX32_ONE/32)
typedef struct {
	u16		seq;
	u16		waitTimer;
	VecFx32	vecDir;
	VecFx32	posTarget;
}SKILL_SWORD0_WORK;

static void Sword0Init( SKILL_CONTROL* sc, SKILL_WORK* sw )
{
	SKILL_SWORD0_WORK* sword_w = (SKILL_SWORD0_WORK*)sw->work;
	sword_w->seq = 0;
	sword_w->waitTimer = 20/2;
	calc_VecDir( sw->direction, &sword_w->vecDir );
	VEC_MultAdd( FX32_ONE*32, &sword_w->vecDir, &sw->trans, &sword_w->posTarget );
}

static void Sword0Main( SKILL_CONTROL* sc, SKILL_WORK* sw )
{
	SKILL_SWORD0_WORK* sword_w = (SKILL_SWORD0_WORK*)sw->work;

	switch( sword_w->seq ){
	case 0:
		if( sword_w->waitTimer ){
			sword_w->waitTimer--;
			return;
		}
		sword_w->waitTimer = 20;
		sword_w->seq = 1;
		break;
	case 1:
		if( sword_w->waitTimer ){
			VecFx32 vecNow; 
			fx32	scalar;

			sword_w->waitTimer--;

			VEC_Subtract( &sword_w->posTarget, &sw->trans, &vecNow );
			scalar = VEC_DotProduct( &sword_w->vecDir, &vecNow );

			if( scalar > 0 ){
				VEC_MultAdd( FX32_ONE*16, &sword_w->vecDir, &sw->trans, &sw->trans );
			}
			if( DamageSetOne( sc, sw, SWORD0_HITLEN, -100, 0, 0, 0 ) == TRUE ){
				sword_w->seq = 2;
			}
		} else {
			sword_w->seq = 2;
		}
		break;
	case 2:
		{
			VecFx32 posFieldChg; 

			//VEC_MultAdd( FX32_ONE*(64+32), &sword_w->vecDir, &sw->trans, &posFieldChg );
			//SetMapGroundUp( &posFieldChg );
		}
		DeleteSkill( sw );
		break;
	}
}

//------------------------------------------------------------------
//　剣１
//------------------------------------------------------------------
#define	SWORD1_HITLEN	(FX32_ONE/32)
typedef struct {
	u16		seq;
	u16		waitTimer;
	VecFx32	vecDir;
	VecFx32	posTarget;
}SKILL_SWORD1_WORK;

static void Sword1Init( SKILL_CONTROL* sc, SKILL_WORK* sw )
{
	SKILL_SWORD1_WORK* sword_w = (SKILL_SWORD1_WORK*)sw->work;
	sword_w->seq = 0;
	sword_w->waitTimer = 20/2;
	calc_VecDir( sw->direction, &sword_w->vecDir );
	VEC_MultAdd( FX32_ONE*16, &sword_w->vecDir, &sw->trans, &sword_w->posTarget );
}

static void Sword1Main( SKILL_CONTROL* sc, SKILL_WORK* sw )
{
	SKILL_SWORD1_WORK* sword_w = (SKILL_SWORD1_WORK*)sw->work;

	switch( sword_w->seq ){
	case 0:
		if( sword_w->waitTimer ){
			sword_w->waitTimer--;
			return;
		}
		SetPlayerMove( sw->pc, &sword_w->vecDir, FX32_ONE*4, 0 );
		sword_w->seq = 1;
		break;
	case 1:
		{
			VecFx32 vecNow; 
			fx32	scalar;

			GetPlayerControlTrans( sw->pc, &sw->trans );
			VEC_Subtract( &sword_w->posTarget, &sw->trans, &vecNow );
			scalar = VEC_DotProduct( &sword_w->vecDir, &vecNow );

			if( scalar <= 0 ){
				SetPlayerMove( sw->pc, &sword_w->vecDir, 0, 0 );
				sword_w->seq = 2;
			}
			if( DamageSetOne( sc, sw, SWORD1_HITLEN, -300, 0, 0, 0 ) == TRUE ){
				SetPlayerMove( sw->pc, &sword_w->vecDir, 0, 0 );
				sword_w->seq = 2;
			}
		}
		break;
	case 2:
		DeleteSkill( sw );
		break;
	}
}

//------------------------------------------------------------------
//　剣２
//------------------------------------------------------------------
#define	SWORD2_HITLEN	(FX32_ONE/32)
typedef struct {
	u16		seq;
	u16		waitTimer;
	VecFx32	vecDir;
	VecFx32	posTarget;
}SKILL_SWORD2_WORK;

static void Sword2Init( SKILL_CONTROL* sc, SKILL_WORK* sw )
{
	SKILL_SWORD2_WORK* sword_w = (SKILL_SWORD2_WORK*)sw->work;
	sword_w->seq = 0;
	sword_w->waitTimer = 20/2;
	calc_VecDir( sw->direction, &sword_w->vecDir );
	VEC_MultAdd( FX32_ONE*128, &sword_w->vecDir, &sw->trans, &sword_w->posTarget );
}

static void Sword2Main( SKILL_CONTROL* sc, SKILL_WORK* sw )
{
	SKILL_SWORD2_WORK* sword_w = (SKILL_SWORD2_WORK*)sw->work;

	switch( sword_w->seq ){
	case 0:
		if( sword_w->waitTimer ){
			sword_w->waitTimer--;
			return;
		}
		SetPlayerMove( sw->pc, &sword_w->vecDir, FX32_ONE*16, 0 );
		sword_w->seq = 1;
		break;
	case 1:
		{
			VecFx32 vecNow; 
			fx32	scalar;

			GetPlayerControlTrans( sw->pc, &sw->trans );
			VEC_Subtract( &sword_w->posTarget, &sw->trans, &vecNow );
			scalar = VEC_DotProduct( &sword_w->vecDir, &vecNow );

			if( scalar <= 0 ){
				SetPlayerMove( sw->pc, &sword_w->vecDir, 0, 0 );
				sword_w->seq = 2;
			}
			if( DamageSetOne( sc, sw, SWORD2_HITLEN, -300, 0, 0, 0 ) == TRUE ){
				SetPlayerMove( sw->pc, &sword_w->vecDir, 0, 0 );
				sword_w->seq = 2;
			}
		}
		break;
	case 2:
		DeleteSkill( sw );
		break;
	}
}

//------------------------------------------------------------------
//　剣３
//------------------------------------------------------------------
#define	SWORD3_HITLEN	(FX32_ONE)
typedef struct {
	u16		seq;
	u16		waitTimer;
	VecFx32	vecDir;
}SKILL_SWORD3_WORK;

static void Sword3Init( SKILL_CONTROL* sc, SKILL_WORK* sw )
{
	SKILL_SWORD3_WORK* sword_w = (SKILL_SWORD3_WORK*)sw->work;
	sword_w->seq = 0;
	sword_w->waitTimer = 20/2;
	calc_VecDir( sw->direction, &sword_w->vecDir );
}

static void Sword3Main( SKILL_CONTROL* sc, SKILL_WORK* sw )
{
	SKILL_SWORD3_WORK* sword_w = (SKILL_SWORD3_WORK*)sw->work;

	switch( sword_w->seq ){
	case 0:
		if( sword_w->waitTimer ){
			sword_w->waitTimer--;
			return;
		}
		SetPlayerMove( sw->pc, &sword_w->vecDir, FX32_ONE*2, 0x2000 );
		sword_w->seq = 1;
		break;
	case 1:
		if( CheckPlayerMoveEnd( sw->pc ) == TRUE ){
			SetPlayerMove( sw->pc, &sword_w->vecDir, 0, 0 );
			sword_w->seq = 2;
		}
		GetPlayerControlTrans( sw->pc, &sw->trans );
		break;
	case 2:
		DamageSetAll( sc, sw, SWORD3_HITLEN, -200, 0, 0, 0 );
		sword_w->seq = 3;
		sword_w->waitTimer = 6;
		break;
	case 3:
		if( sword_w->waitTimer ){
			sword_w->waitTimer--;
			return;
		}
		DeleteSkill( sw );
		break;
	}
}

//------------------------------------------------------------------
//　矢
//------------------------------------------------------------------
#define	ARROW_HITOFS	(FX32_ONE*0)
#define	ARROW_HITLEN	(FX32_ONE/64)
#define	ARROW_SPEED		(FX32_ONE*8*2)
#define	ARROW_LIMITLEN	(FX32_ONE*256)
typedef struct {
	u16		seq;
	u16		waitTimer;
	int		effectID;
	fx32	length;
}SKILL_ARROW_WORK;

static void ArrowInit( SKILL_CONTROL* sc, SKILL_WORK* sw )
{
	SKILL_ARROW_WORK* arrow_w = (SKILL_ARROW_WORK*)sw->work;
	arrow_w->seq = 0;
	arrow_w->waitTimer = 30/2;
	arrow_w->length = 0;
	sw->trans.y = FX32_ONE*8;
}

static void ArrowMain( SKILL_CONTROL* sc, SKILL_WORK* sw )
{
	SKILL_ARROW_WORK* arrow_w = (SKILL_ARROW_WORK*)sw->work;
	BOOL hitResult = FALSE;
	BOOL deleteFlag = FALSE;

	switch( arrow_w->seq ){
	case 0:
		if( arrow_w->waitTimer ){
			arrow_w->waitTimer--;
			return;
		} else {
			SkillEffectAdd( sc, &arrow_w->effectID, SKILL_EFFECT_TEST2, sw );
			{
				VecFx32	rotateVec;		
				MtxFx33	rotateMtx;
				GFL_G3D_SCENEOBJ* g3DsceneObj = GFL_G3D_SCENEOBJ_Get
												( Get_GS_G3Dscene( sc->gs ), arrow_w->effectID );
				rotateVec.x = 0;
				rotateVec.y = sw->direction;
				rotateVec.z = 0;
	
				rotateCalc( &rotateVec, &rotateMtx );
				GFL_G3D_SCENEOBJ_SetRotate( g3DsceneObj, &rotateMtx );
			}
			arrow_w->seq++;
		}
		break;
	case 1:
		hitResult = DamageSetOne( sc, sw, ARROW_HITLEN, -100, 0, 0, 0 );
		calc_XZtrans( &sw->trans, ARROW_SPEED, sw->direction );
		if( hitResult == TRUE ){
			deleteFlag = TRUE;
		} else {
			if( arrow_w->length >= ARROW_LIMITLEN ){
				deleteFlag = TRUE;
			} else {
				arrow_w->length += ARROW_SPEED;
			}
		}
		if( deleteFlag == TRUE ){
			SkillEffectRemove( sc, &arrow_w->effectID, SKILL_EFFECT_TEST2 );
			DeleteSkill( sw );
		}
		break;
	}
}

//------------------------------------------------------------------
//　杖
//------------------------------------------------------------------
#define	STAFF_HITOFS	(FX32_ONE*0)
#define	STAFF_HITLEN	(FX32_ONE/1)
#define	STAFF_SPEED		(FX32_ONE*2*2)
#define	STAFF_LIMITLEN	(FX32_ONE*128)
#define	STAFF_OFS_FIRST	(FX32_ONE*8)
typedef struct {
	u16				seq;
	u16				waitTimer;
	int				effectID;
	fx32			length;
	fx32			wait;
	GFL_EMIT_PTR	effectEmitter;
//	CALC_PH_MV*		calcPHMV;
}SKILL_STAFF_WORK;

static void StaffInit( SKILL_CONTROL* sc, SKILL_WORK* sw )
{
	SKILL_STAFF_WORK* staff_w = (SKILL_STAFF_WORK*)sw->work;
	staff_w->seq = 0;
	staff_w->waitTimer = 30/2;
	staff_w->length = 0;
	sw->trans.y = FX32_ONE*12;
}

static void StaffMain( SKILL_CONTROL* sc, SKILL_WORK* sw )
{
	SKILL_STAFF_WORK* staff_w = (SKILL_STAFF_WORK*)sw->work;
	BOOL hitResult = FALSE;
	BOOL deleteFlag = FALSE;

	switch( staff_w->seq ){
	case 0:
		if( staff_w->waitTimer ){
			staff_w->waitTimer--;
			return;
		} else {
			VecFx16 rotVec;

			calc_XZtrans( &sw->trans, STAFF_OFS_FIRST, sw->direction );
			rotVec.x = FX_SinIdx( sw->direction );
			rotVec.y = 0;
			rotVec.z = FX_CosIdx( sw->direction );

			staff_w->effectEmitter = GFL_PTC_CreateEmitter
										( Get_GS_Perticle(sc->gs), 0, &sw->trans );
			GFL_PTC_SetEmitterAxis( staff_w->effectEmitter, &rotVec );
			GFL_PTC_SetEmitterBaseScale( staff_w->effectEmitter, FX16_ONE*2 );

			ResetPlayerSkillBusyFlag( sw->pc );	//早い段階でプレーヤー動作可能
			staff_w->seq++;
		}
		break;
#if 1
	case 1:
		calc_XZtrans( &sw->trans, STAFF_SPEED, sw->direction );
		GFL_PTC_SetEmitterPosition( staff_w->effectEmitter, &sw->trans );

		staff_w->length += STAFF_SPEED;
		if( staff_w->length >= STAFF_LIMITLEN ){
			staff_w->length = STAFF_LIMITLEN;

			GFL_PTC_DeleteEmitter( Get_GS_Perticle(sc->gs), staff_w->effectEmitter );
			staff_w->wait = 10/2;
			staff_w->seq++;
		}
		break;
	case 2:
		hitResult = DamageSetAll( sc, sw, STAFF_HITLEN, -150, 5, 50, -5 );
		if( staff_w->wait ){
			staff_w->wait--;
		} else {
			DeleteSkill( sw );
		}
		break;
#else
	case 1:
		staff_w->calcPHMV = CreateCalcPhisicsMoving( sc->heapID, FALSE );
		{
			VecFx32 vecMove;
			fx32	speed =	(8 << FX32_SHIFT);
			fx32	theta =	0x1800;

			vecMove.x = -FX_SinIdx( sw->direction );
			vecMove.y = FX32_ONE;
			vecMove.z = -FX_CosIdx( sw->direction );
			VEC_Normalize( &vecMove, &vecMove );	//正規化

			StartMovePHMV( staff_w->calcPHMV, &sw->trans, &vecMove, speed, theta ); 
		}
		staff_w->seq++;
		break;
	case 2:
		CalcMovePHMV( staff_w->calcPHMV, &sw->trans );
		GFL_PTC_SetEmitterPosition( staff_w->effectEmitter, &sw->trans );

		if( CheckMoveEndPHMV( staff_w->calcPHMV ) == TRUE ){
			staff_w->seq++;
		}
		break;
	case 3:
		DeleteCalcPhisicsMoving( staff_w->calcPHMV );
		GFL_PTC_DeleteEmitter( Get_GS_Perticle(sc->gs), staff_w->effectEmitter );
		DeleteSkill( sw );
		break;
#endif
	}
}

//------------------------------------------------------------------
/**
 * @brief	スキルテーブル
 */
//------------------------------------------------------------------
static const SKILL_PROC skillProcTable[7] = {
	{ NopInit,		NopMain },
	{ Sword0Init,	Sword0Main },
	{ Sword1Init,	Sword1Main },
	{ Sword2Init,	Sword2Main },
	{ Sword3Init,	Sword3Main },
	{ ArrowInit,	ArrowMain },
	{ StaffInit,	StaffMain },
};





//------------------------------------------------------------------
/**
 * @brief	スキルエフェクト生成
 */
//------------------------------------------------------------------
static void SkillEffectAdd( SKILL_CONTROL* sc, int* effectID, SKILL_EFFECTID seID, SKILL_WORK* sw )
{
	int	i,j;
	GFL_G3D_SCENE*	g3Dscene = Get_GS_G3Dscene( sc->gs );
	SKILL_EFFECT_WORK* sew;
	GFL_G3D_SCENEOBJ* g3DsceneObj;

	*effectID = GFL_G3D_SCENEOBJ_Add( g3Dscene,	skillEffectTable[seID].data,
										skillEffectTable[seID].count, sc->g3DutilObjIdx );
	for( i=0; i<skillEffectTable[seID].count; i++ ){
		sew = (SKILL_EFFECT_WORK*)GFL_G3D_SCENEOBJ_GetWork
						( GFL_G3D_SCENEOBJ_Get( g3Dscene, *effectID + i )); 
		g3DsceneObj = GFL_G3D_SCENEOBJ_Get( g3Dscene, *effectID + i );

		sew->sw = sw;
		for( j=0; j<8; j++ ){
			sew->work[j] = 0;
		}
		GFL_G3D_SCENEOBJ_SetPos( g3DsceneObj, &sw->trans );
	}
}

static void SkillEffectRemove( SKILL_CONTROL* sc, int* effectID, SKILL_EFFECTID seID )
{
	GFL_G3D_SCENEOBJ_Remove( Get_GS_G3Dscene( sc->gs ), *effectID, skillEffectTable[seID].count );
}

//------------------------------------------------------------------
/**
 * @brief	スキルエフェクト動作
 */
//------------------------------------------------------------------
static void moveTraceSkill( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
	SKILL_EFFECT_WORK*	sew = (SKILL_EFFECT_WORK*)work;
	GFL_G3D_SCENEOBJ_SetPos( sceneObj, &sew->sw->trans );
}




