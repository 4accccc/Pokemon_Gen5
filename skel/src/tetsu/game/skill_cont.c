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
#include "player_cont.h"
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
	u16				skillCommand;
	u16				seq;
	PLAYER_CONTROL* pc;			//発生させたプレーヤー
	VecFx32			trans;
	u16				direction;
	u32				work[8];
}SKILL_WORK;

typedef struct {
	SKILL_WORK*		sw;
	u16				work[8];
}SKILL_EFFECT_WORK;

struct _SKILL_CONTROL {
	HEAPID			heapID;
	GAME_SYSTEM*	gs;
	PLAYER_CONTROL* pc[PLAYER_SETUP_MAX];		//判定させるプレーヤー配列
	SKILL_WORK		skill[SKILL_SETUP_MAX];
};

typedef void	(SKILL_FUNC)( SKILL_CONTROL*, SKILL_WORK* );

typedef struct {
	SKILL_FUNC*	init;
	SKILL_FUNC*	main;
}SKILL_PROC;

//------------------------------------------------------------------
static void moveTraceSkill( GFL_G3D_SCENEOBJ* sceneObj, void* work );

typedef struct {
	const GFL_G3D_SCENEOBJ_DATA*	data;
	int								count;
}SKILL_EFFECT_TBL;

static const GFL_G3D_SCENEOBJ_DATA skillEffectData1[] = {
	{ 
		G3DOBJ_EFFECT_WALL, 0, 1, 24, FALSE, TRUE, 
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
		G3DOBJ_EFFECT_WALL, 0, 1, 24, FALSE, TRUE, 
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

static const SKILL_PROC skillProcTable[4];
static void SkillEffectAdd( SKILL_CONTROL* sc, int* effectID, SKILL_EFFECTID seID, SKILL_WORK* sw );
static void SkillEffectRemove( SKILL_CONTROL* sc, int* effectID, SKILL_EFFECTID seID );
//------------------------------------------------------------------
/**
 * @brief	スキルコントロールセット
 */
//------------------------------------------------------------------
SKILL_CONTROL* AddSkillControl( GAME_SYSTEM* gs, HEAPID heapID )
{
	int	i,j;

	SKILL_CONTROL* sc = GFL_HEAP_AllocClearMemory( heapID, sizeof(SKILL_CONTROL) );
	sc->heapID = heapID;
	sc->gs = gs;
	for( i=0; i<PLAYER_SETUP_MAX; i++ ){
		sc->pc[i] = NULL;
	}
	for( i=0; i<SKILL_SETUP_MAX; i++ ){
		sc->skill[i].skillCommand = 0;
		sc->skill[i].seq = 0;
		sc->skill[i].pc = NULL;
		for( j=0; j<8; j++ ){
			sc->skill[i].work[j] = 0;
		}
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
	GFL_HEAP_FreeMemory( sc ); 
}

//------------------------------------------------------------------
/**
 * @brief	判定プレーヤーの追加
 */
//------------------------------------------------------------------
void AddSkillCheckPlayer( SKILL_CONTROL* sc, PLAYER_CONTROL* pc )
{
	int	i;

	for( i=0; i<PLAYER_SETUP_MAX; i++ ){
		if( sc->pc[i] == NULL ){
			sc->pc[i] = pc;
			return;
		}
	}
	GF_ASSERT(0);
}

//------------------------------------------------------------------
/**
 * @brief	判定プレーヤーの削除
 */
//------------------------------------------------------------------
void RemoveSkillCheckPlayer( SKILL_CONTROL* sc, PLAYER_CONTROL* pc )
{
	int	i;

	for( i=0; i<PLAYER_SETUP_MAX; i++ ){
		if( sc->pc[i] == pc ){
			sc->pc[i] = NULL;
			return;
		}
	}
	GF_ASSERT(0);
}

//------------------------------------------------------------------
/**
 * @brief	スキルの追加
 */
//------------------------------------------------------------------
void SetSkillControlCommand( SKILL_CONTROL* sc, PLAYER_CONTROL* pc, int skillCommand )
{
	int i;

	if( skillCommand ){
		for( i=0; i<SKILL_SETUP_MAX; i++ ){
			if( sc->skill[i].skillCommand == 0 ){
				sc->skill[i].skillCommand = skillCommand;
				sc->skill[i].seq = 0;
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
void MainSkillControl( SKILL_CONTROL* sc )
{
	int i;

	for( i=0; i<SKILL_SETUP_MAX; i++ ){
		if( sc->skill[i].skillCommand != 0 ){
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
static void DeleteSkill( SKILL_WORK* sw )
{
	sw->skillCommand = 0;
	ResetPlayerSkillBusyFlag( sw->pc );
}
	
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

static BOOL DamageSetOne( SKILL_CONTROL* sc, PLAYER_CONTROL* pc, 
		VecFx32* skillTrans, fx32 hitLen, s16 damage, u8 dotcount, u8 dottimer, s8 dotvalue )
{
	int i;
	VecFx32 targetTrans;

	for( i=0; i<PLAYER_SETUP_MAX; i++ ){
		if(( sc->pc[i] != NULL )&&( sc->pc[i] != pc )){
			GetPlayerControlTrans( sc->pc[i], &targetTrans );

			if( calc_XZhitEasy( skillTrans, &targetTrans, hitLen ) == TRUE ){
				SetPlayerDamage( sc->pc[i], damage );
				if( dotvalue ){
					SetPlayerDamageOnTime( sc->pc[i], dotcount, dottimer, dotvalue );
				}
				return TRUE;
			}
		}
	}
	return FALSE;
}

static BOOL DamageSetAll( SKILL_CONTROL* sc, PLAYER_CONTROL* pc,
		VecFx32* skillTrans, fx32 hitLen, s16 damage, u8 dotcount, u8 dottimer, s8 dotvalue )
{
	int i;
	VecFx32 targetTrans;
	BOOL	hitResult = FALSE;

	for( i=0; i<PLAYER_SETUP_MAX; i++ ){
		if(( sc->pc[i] != NULL )&&( sc->pc[i] != pc )){
			if( GetPlayerHitEnableFlag( sc->pc[i] ) == TRUE ){
				GetPlayerControlTrans( sc->pc[i], &targetTrans );

				if( calc_XZhitEasy( skillTrans, &targetTrans, hitLen ) == TRUE ){
					SetPlayerDamage( sc->pc[i], damage );
					if( dotvalue ){
						SetPlayerDamageOnTime( sc->pc[i], dotcount, dottimer, dotvalue );
					}
					hitResult = TRUE;
				}
			}
		}
	}
	return hitResult;
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
//　剣
//------------------------------------------------------------------
#define	SWORD_HITOFS	(FX32_ONE*0)
#define	SWORD_HITLEN	(FX32_ONE/64)
#define	SWORD_SPEED		(FX32_ONE*16)
#define	SWORD_LIMITLEN	(FX32_ONE*16)
typedef struct {
	u16		seq;
	u16		waitTimer;
	int		effectID;
	fx32	length;
}SKILL_SWORD_WORK;

static void SwordInit( SKILL_CONTROL* sc, SKILL_WORK* sw )
{
	SKILL_SWORD_WORK* sword_w = (SKILL_SWORD_WORK*)sw->work;
	sword_w->seq = 0;
	sword_w->waitTimer = 20;
	sword_w->length = 0;
}

static void SwordMain( SKILL_CONTROL* sc, SKILL_WORK* sw )
{
	SKILL_SWORD_WORK* sword_w = (SKILL_SWORD_WORK*)sw->work;
	BOOL hitResult = FALSE;
	BOOL deleteFlag = FALSE;

	switch( sword_w->seq ){
	case 0:
		if( sword_w->waitTimer ){
			sword_w->waitTimer--;
			return;
		} else {
			SkillEffectAdd( sc, &sword_w->effectID, SKILL_EFFECT_TEST1, sw );
			sword_w->seq++;
		}
		break;
	case 1:
		calc_XZtrans( &sw->trans, SWORD_SPEED, sw->direction );
		hitResult = DamageSetOne( sc, sw->pc, &sw->trans, SWORD_HITLEN, -50, 0, 0, 0 );
		if( hitResult == TRUE ){
			deleteFlag = TRUE;
		} else {
			sword_w->length += SWORD_SPEED;
			if( sword_w->length >= SWORD_LIMITLEN ){
				deleteFlag = TRUE;
			}
		}
		if( deleteFlag == TRUE ){
			SkillEffectRemove( sc, &sword_w->effectID, SKILL_EFFECT_TEST1 );
			DeleteSkill( sw );
		}
	}
}

//------------------------------------------------------------------
//　矢
//------------------------------------------------------------------
#define	ARROW_HITOFS	(FX32_ONE*0)
#define	ARROW_HITLEN	(FX32_ONE/64)
#define	ARROW_SPEED		(FX32_ONE*8)
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
	arrow_w->waitTimer = 30;
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
		calc_XZtrans( &sw->trans, ARROW_SPEED, sw->direction );
		hitResult = DamageSetOne( sc, sw->pc, &sw->trans, ARROW_HITLEN, -20, 0, 0, 0 );
		if( hitResult == TRUE ){
			deleteFlag = TRUE;
		} else {
			arrow_w->length += ARROW_SPEED;
			if( arrow_w->length >= ARROW_LIMITLEN ){
				deleteFlag = TRUE;
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
#define	STAFF_SPEED		(FX32_ONE*2)
#define	STAFF_LIMITLEN	(FX32_ONE*128)
#define	STAFF_OFS_FIRST	(FX32_ONE*8)
typedef struct {
	u16				seq;
	u16				waitTimer;
	int				effectID;
	fx32			length;
	fx32			wait;
	GFL_EMIT_PTR	effectEmitter;
}SKILL_STAFF_WORK;

static void StaffInit( SKILL_CONTROL* sc, SKILL_WORK* sw )
{
	SKILL_STAFF_WORK* staff_w = (SKILL_STAFF_WORK*)sw->work;
	staff_w->seq = 0;
	staff_w->waitTimer = 30;
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
	case 1:
		calc_XZtrans( &sw->trans, STAFF_SPEED, sw->direction );
		GFL_PTC_SetEmitterPosition( staff_w->effectEmitter, &sw->trans );

		staff_w->length += STAFF_SPEED;
		if( staff_w->length >= STAFF_LIMITLEN ){
			staff_w->length = STAFF_LIMITLEN;

			GFL_PTC_DeleteEmitter( Get_GS_Perticle(sc->gs), staff_w->effectEmitter );
			staff_w->wait = 10;
			staff_w->seq++;
		}
		break;
	case 2:
		hitResult = DamageSetAll( sc, sw->pc, &sw->trans, STAFF_HITLEN, -30, 5, 60, -5 );
		if( staff_w->wait ){
			staff_w->wait--;
		} else {
			DeleteSkill( sw );
		}
		break;
	}
}

//------------------------------------------------------------------
/**
 * @brief	スキルテーブル
 */
//------------------------------------------------------------------
static const SKILL_PROC skillProcTable[4] = {
	{ NopInit,		NopMain },
	{ SwordInit,	SwordMain },
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
										skillEffectTable[seID].count );
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




