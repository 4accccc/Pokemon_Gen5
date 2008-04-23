//============================================================================================
/**
 * @file	act3d.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"

#include "setup.h"
#include "include\system\gfl_use.h"

//------------------------------------------------------------------
/**
 * @brief	型宣言
 */
//------------------------------------------------------------------
struct _SCENE_ACTSYS {
	GFL_G3D_SCENE*	g3Dscene;
	u16				g3DutilUnitIdx;
	int				actID;
	int				actCount;
};

struct _SCENE_ACT {
	GFL_G3D_SCENE*	g3Dscene;
	u16				g3DutilUnitIdx;
	int				actID;
	int				actCount;

	int				seq;

	VecFx32			rotateVec;
	u8				animeCmd;
	u8				animeCmdNow;
	u8				animeID;
	u8				accesoryID;
	u8				accesoryIDNow;
	BOOL			busyFlag;
	BOOL			anmSetFlag;
};

#define ACT_FREEWORK_SIZ (15)
typedef struct { 
	SCENE_ACT*		sceneAct;
	u32				work[ ACT_FREEWORK_SIZ ];
}ACT_WORK;//setup.cで指定されているサイズに収めるように注意

enum {
	ACTANMSEQ_INC_LOOP = 0,
	ACTANMSEQ_INC_END,
	ACTANMSEQ_INC_END_NEXT,
	ACTANMSEQ_INC_END_PREV,

	ACTANMSEQ_DEC_LOOP,
	ACTANMSEQ_DEC_END,
	ACTANMSEQ_DEC_END_NEXT,
	ACTANMSEQ_DEC_END_PREV,
};

typedef struct {
	int animeID; 
	int animeSeq; 
	int start; 
	int end; 
	int rate; 
}ANMIDATA_TBL;

#define SYNC_WAIT (1)

#include "graphic_data/test_graphic/test3d.naix"
//------------------------------------------------------------------
/**
 *
 *
 *
 * @brief	システムおよび共通リソースのセットアップと破棄
 *
 *
 *
 */
//------------------------------------------------------------------
//３Ｄグラフィックリソーステーブル
enum {
	G3DRES_COMMON_HUMAN_COMMON_BCA = 0,
	G3DRES_COMMON_SWORD_ATTACK_BCA,
	G3DRES_COMMON_HUMAN2_SHOOT_BCA,
	G3DRES_COMMON_HUMAN2_SPELL_BCA,
	G3DRES_COMMON_NPC_RIZ_BCA,
};

static const GFL_G3D_UTIL_RES g3DresTblCommon[] = {
	{ ARCID_SAMPLE, NARC_haruka_armor_common_nsbca,  GFL_G3D_UTIL_RESARC },
	{ ARCID_SAMPLE, NARC_haruka_sword_attack_nsbca,  GFL_G3D_UTIL_RESARC },
	{ ARCID_SAMPLE, NARC_haruka_human2_shoot_nsbca,  GFL_G3D_UTIL_RESARC },
	{ ARCID_SAMPLE, NARC_haruka_human2_spell_nsbca,  GFL_G3D_UTIL_RESARC },
	{ ARCID_SAMPLE, NARC_haruka_riz_nsbca,  GFL_G3D_UTIL_RESARC },
};

//---------------------
//３Ｄオブジェクトアニメーション定義テーブル
enum {
	G3DANM_COMMON_HUMAN_COMMON = 0,
	G3DANM_COMMON_HUMAN_SWORD_ATTACK,
	G3DANM_COMMON_HUMAN_SHOOT,
	G3DANM_COMMON_HUMAN_SPELL,

	G3DANM_COMMON_NPC_RIZ,
};

static const GFL_G3D_UTIL_ANM g3DanmTblCommon[] = {
	{ G3DRES_COMMON_HUMAN_COMMON_BCA, 0 },
	{ G3DRES_COMMON_SWORD_ATTACK_BCA, 0 },
	{ G3DRES_COMMON_HUMAN2_SHOOT_BCA, 0 },
	{ G3DRES_COMMON_HUMAN2_SPELL_BCA, 0 },
	{ G3DRES_COMMON_NPC_RIZ_BCA, 0 },
};

//---------------------
//g3Dscene 初期設定テーブルデータ
static const GFL_G3D_UTIL_SETUP g3DsetupCommon = {
	g3DresTblCommon, NELEMS(g3DresTblCommon),
	NULL, 0,
};

SCENE_ACTSYS*	SceneActSys;

SCENE_ACTSYS*	Create3DactSys( GFL_G3D_SCENE* g3Dscene, HEAPID heapID )
{
	SCENE_ACTSYS* sceneActSys = GFL_HEAP_AllocMemory( heapID, sizeof(SCENE_ACTSYS) );
	sceneActSys->g3Dscene = g3Dscene;

	//３Ｄデータセットアップ
	sceneActSys->g3DutilUnitIdx = GFL_G3D_SCENE_AddG3DutilUnit( g3Dscene, &g3DsetupCommon );

	SceneActSys = sceneActSys;
	return sceneActSys;
}

void			Delete3DactSys( SCENE_ACTSYS* sceneActSys )
{
	GFL_G3D_SCENE_DelG3DutilUnit( sceneActSys->g3Dscene, sceneActSys->g3DutilUnitIdx );
	GFL_HEAP_FreeMemory( sceneActSys );
}

//------------------------------------------------------------------
/**
 *
 *
 *
 * @brief	ローカル共通関数
 *
 *
 *
 */
//------------------------------------------------------------------
//------------------------------------------------------------------
/**
 * @brief		回転マトリクス変換
 */
//------------------------------------------------------------------
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
/**
 * @brief		ワーク初期化
 */
//------------------------------------------------------------------
static void initActWork( SCENE_ACT*	sceneAct )
{
	ACT_WORK*	work;
	int i, j;

	sceneAct->animeCmd		= 0;
	sceneAct->animeCmdNow	= sceneAct->animeCmd;
	sceneAct->animeID		= 0;
	sceneAct->accesoryID	= 0;
	sceneAct->accesoryIDNow	= sceneAct->accesoryID;
	sceneAct->busyFlag		= FALSE;
	sceneAct->anmSetFlag	= TRUE;

	for( i=0; i<sceneAct->actCount; i++ ){
		work = (ACT_WORK*)GFL_G3D_SCENEOBJ_GetWork
					( GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene, sceneAct->actID+i ) ); 
		work->sceneAct = sceneAct;
		for( j=0; j<ACT_FREEWORK_SIZ; j++ ){
			work->work[j] = 0;
		}
	}
}

//------------------------------------------------------------------
/**
 * @brief		共通アニメを関連付ける
 */
//------------------------------------------------------------------
static void bindCommonAnm
	( u16 objUnitIdx, u16 objIdx, const GFL_G3D_UTIL_ANM* anmTbl, u32 anmCount )
{
	//共通リソースからアニメ設定
	GFL_G3D_UTIL_SetObjAnotherUnitAnime(	GFL_G3D_SCENE_GetG3Dutil( SceneActSys->g3Dscene ), 
											objUnitIdx, objIdx,
											SceneActSys->g3DutilUnitIdx, anmTbl, anmCount );
	//アニメーションの初期有効設定(0番のアニメーションを有効にする)
	GFL_G3D_OBJECT_EnableAnime
		( GFL_G3D_SCENE_GetG3DutilObjHandle( SceneActSys->g3Dscene, objUnitIdx + objIdx ), 0 );
}

//------------------------------------------------------------------
/**
 * @brief		アニメコントロール
 */
//------------------------------------------------------------------
static void setAnimetion( SCENE_ACT* sceneAct, int anmID, fx32 start )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj;
	int i;

	if( anmID != sceneAct->animeID ){
		for( i=0; i<sceneAct->actCount; i++ ){
			g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene, sceneAct->actID + i );
			GFL_G3D_SCENEOBJ_DisableAnime( g3DsceneObj, sceneAct->animeID );
			GFL_G3D_SCENEOBJ_EnableAnime( g3DsceneObj, anmID );
		}
		sceneAct->animeID = anmID;
	}
	for( i=0; i<sceneAct->actCount; i++ ){
		g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene, sceneAct->actID + i );
		GFL_G3D_SCENEOBJ_SetAnimeFrame( g3DsceneObj, anmID, (int*)&start );
	}
}

static void setAnimeFrame( SCENE_ACT* sceneAct, int anmID, fx32 frame )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj;
	int i;

	for( i=0; i<sceneAct->actCount; i++ ){
		g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene, sceneAct->actID + i );
		GFL_G3D_SCENEOBJ_SetAnimeFrame( g3DsceneObj, anmID, (int*)&frame );
	}
}

static BOOL incAnimetion( SCENE_ACT* sceneAct, int anmID, fx32 end, fx32 rate )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj;
	fx32	nowFrm, setFrm;
	int		i;
	BOOL	result = TRUE;;

	g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene, sceneAct->actID );	//0のみ参照
	if( GFL_G3D_SCENEOBJ_GetAnimeFrame( g3DsceneObj, anmID, (int*)&nowFrm ) == FALSE ){
		return FALSE;
	}
	setFrm = nowFrm + rate;
	if( setFrm >= end ){
		setFrm = end;
		result = FALSE;
	}
	for( i=0; i<sceneAct->actCount; i++ ){
		g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene, sceneAct->actID + i );
		GFL_G3D_SCENEOBJ_SetAnimeFrame( g3DsceneObj, anmID, (int*)&setFrm );
	}
	return result;
}

static BOOL decAnimetion( SCENE_ACT* sceneAct, int anmID, fx32 end, fx32 rate )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj;
	fx32	nowFrm, setFrm;
	int		i;
	BOOL	result = TRUE;;

	g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene, sceneAct->actID );	//0のみ参照
	if( GFL_G3D_SCENEOBJ_GetAnimeFrame( g3DsceneObj, anmID, (int*)&nowFrm ) == FALSE ){
		return FALSE;
	}
	setFrm = nowFrm - rate;
	if( setFrm <= end ){
		setFrm = end;
		result = FALSE;
	}
	for( i=0; i<sceneAct->actCount; i++ ){
		g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene, sceneAct->actID + i );
		GFL_G3D_SCENEOBJ_SetAnimeFrame( g3DsceneObj, anmID, (int*)&setFrm );
	}
	return result;
}

//------------------------------------------------------------------
static void animeControl( SCENE_ACT* sceneAct, const ANMIDATA_TBL* anmDataTable )
{
	int			anmID;
	fx32		start, end, rate;

	if( sceneAct->anmSetFlag == TRUE ){	//アニメ更新チェック
		anmID = anmDataTable[ sceneAct->animeCmd ].animeID;
		start = FX32_ONE * anmDataTable[ sceneAct->animeCmd ].start;
		sceneAct->seq = anmDataTable[ sceneAct->animeCmd ].animeSeq;
		setAnimetion( sceneAct, anmID, start );
		sceneAct->animeCmdNow = sceneAct->animeCmd;
		sceneAct->anmSetFlag = FALSE;
	}
	start	= FX32_ONE * anmDataTable[ sceneAct->animeCmdNow ].start;
	end		= FX32_ONE * anmDataTable[ sceneAct->animeCmdNow ].end;
	rate	= FX32_ONE * anmDataTable[ sceneAct->animeCmdNow ].rate;

	switch( sceneAct->seq ){

	case ACTANMSEQ_INC_LOOP:
		if( incAnimetion( sceneAct, sceneAct->animeID, end, rate ) == FALSE ){
			setAnimeFrame( sceneAct, sceneAct->animeID, start );
		}
		sceneAct->busyFlag = FALSE;
		break;

	case ACTANMSEQ_INC_END:
		if( incAnimetion( sceneAct, sceneAct->animeID, end, rate ) == FALSE ){
			sceneAct->busyFlag = FALSE;
		} else {
			sceneAct->busyFlag = TRUE;
		}
		break;

	case ACTANMSEQ_INC_END_NEXT:
		if( incAnimetion( sceneAct, sceneAct->animeID, end, rate ) == FALSE ){
			sceneAct->animeCmdNow++;
			anmID = anmDataTable[ sceneAct->animeCmdNow ].animeID;
			start = FX32_ONE * anmDataTable[ sceneAct->animeCmdNow ].start;
			sceneAct->seq = anmDataTable[ sceneAct->animeCmdNow ].animeSeq;
			setAnimetion( sceneAct, anmID, start );
		}
		sceneAct->busyFlag = TRUE;
		break;

	case ACTANMSEQ_INC_END_PREV:
		if( incAnimetion( sceneAct, sceneAct->animeID, end, rate ) == FALSE ){
			sceneAct->animeCmdNow--;
			anmID = anmDataTable[ sceneAct->animeCmdNow ].animeID;
			start = FX32_ONE * anmDataTable[ sceneAct->animeCmdNow ].start;
			sceneAct->seq = anmDataTable[ sceneAct->animeCmdNow ].animeSeq;
			setAnimetion( sceneAct, anmID, start );
		}
		sceneAct->busyFlag = TRUE;
		break;

	case ACTANMSEQ_DEC_LOOP:
		if( decAnimetion( sceneAct, sceneAct->animeID, end, rate ) == FALSE ){
			setAnimeFrame( sceneAct, sceneAct->animeID, start );
		}
		sceneAct->busyFlag = FALSE;
		break;

	case ACTANMSEQ_DEC_END:
		if( decAnimetion( sceneAct, sceneAct->animeID, end, rate ) == FALSE ){
			sceneAct->busyFlag = FALSE;
		} else {
			sceneAct->busyFlag = TRUE;
		}
		break;

	case ACTANMSEQ_DEC_END_NEXT:
		if( decAnimetion( sceneAct, sceneAct->animeID, end, rate ) == FALSE ){
			sceneAct->animeCmdNow++;
			anmID = anmDataTable[ sceneAct->animeCmdNow ].animeID;
			start = FX32_ONE * anmDataTable[ sceneAct->animeCmdNow ].start;
			sceneAct->seq = anmDataTable[ sceneAct->animeCmdNow ].animeSeq;
			setAnimetion( sceneAct, anmID, start );
		}
		sceneAct->busyFlag = TRUE;
		break;

	case ACTANMSEQ_DEC_END_PREV:
		if( decAnimetion( sceneAct, sceneAct->animeID, end, rate ) == FALSE ){
			sceneAct->animeCmdNow--;
			anmID = anmDataTable[ sceneAct->animeCmdNow ].animeID;
			start = FX32_ONE * anmDataTable[ sceneAct->animeCmdNow ].start;
			sceneAct->seq = anmDataTable[ sceneAct->animeCmdNow ].animeSeq;
			setAnimetion( sceneAct, anmID, start );
		}
		sceneAct->busyFlag = TRUE;
		break;
	}
}

//------------------------------------------------------------------
/**
 *
 *
 *
 * @brief	グローバル共通関数
 *
 *
 *
 */
//------------------------------------------------------------------
//------------------------------------------------------------------
/**
 * @brief		３Ｄオブジェクトハンドルポインタ取得
 */
//------------------------------------------------------------------
GFL_G3D_SCENEOBJ*	Get3DactHandle( SCENE_ACT* sceneAct, u32 idx )
{
	return GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene, sceneAct->actID + idx );
}

//------------------------------------------------------------------
/**
 * @brief		３Ｄオブジェクトワークポインタ取得
 */
//------------------------------------------------------------------
void*	Get3DactWork( SCENE_ACT* sceneAct, u32 idx )
{
	return GFL_G3D_SCENEOBJ_GetWork( GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene, idx ) );
}

//------------------------------------------------------------------
/**
 * @brief		３Ｄオブジェクト描画ON/OFF
 */
//------------------------------------------------------------------
void	Set3DactDrawSw( SCENE_ACT* sceneAct, BOOL sw )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj;
	int i;

	for( i=0; i<sceneAct->actCount; i++ ){
		g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene, sceneAct->actID + i );
		GFL_G3D_SCENEOBJ_SetDrawSW( g3DsceneObj, &sw );
	}
}

//------------------------------------------------------------------
/**
 * @brief		３Ｄオブジェクト座標の設定
 */
//------------------------------------------------------------------
void	Set3DactTrans( SCENE_ACT* sceneAct, VecFx32* trans )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj;
	int i;

	for( i=0; i<sceneAct->actCount; i++ ){
		g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene, sceneAct->actID + i );
		GFL_G3D_SCENEOBJ_SetPos( g3DsceneObj, trans );
	}
}

//------------------------------------------------------------------
/**
 * @brief		３Ｄオブジェクト回転の設定
 */
//------------------------------------------------------------------
void	Set3DactRotate( SCENE_ACT* sceneAct, VecFx32* rotate )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj;
	MtxFx33 rotateMtx;
	int i;

	sceneAct->rotateVec = *rotate;
	rotateCalc( &sceneAct->rotateVec, &rotateMtx );

	for( i=0; i<sceneAct->actCount; i++ ){
		g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene, sceneAct->actID + i );
		GFL_G3D_SCENEOBJ_SetRotate( g3DsceneObj, &rotateMtx );
	}
}

//------------------------------------------------------------------
/**
 * @brief		αブレンドの設定
 */
//------------------------------------------------------------------
void	Set3DactBlendAlpha( SCENE_ACT* sceneAct, u8* alpha )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj;
	int i;

	for( i=0; i<sceneAct->actCount; i++ ){
		g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene, sceneAct->actID + i );
		GFL_G3D_SCENEOBJ_SetBlendAlpha( g3DsceneObj, alpha );
	}
}


//------------------------------------------------------------------
/**
 * @brief		アニメ設定
 */
//------------------------------------------------------------------
BOOL	Set3DactChrAnimeCmd( SCENE_ACT* sceneAct, u8 animeCmd )
{
	if( animeCmd != sceneAct->animeCmd ){	//アニメ更新チェック
		sceneAct->animeCmd = animeCmd;
		sceneAct->anmSetFlag = TRUE;
		return TRUE; 
	} else {
		return FALSE; 
	}
}

void	Set3DactChrAnimeForceCmd( SCENE_ACT* sceneAct, u8 animeCmd )
{
	sceneAct->animeCmd = animeCmd;
	sceneAct->anmSetFlag = TRUE;
}

BOOL	Check3DactBusy( SCENE_ACT* sceneAct )
{
	return sceneAct->busyFlag;
}

//------------------------------------------------------------------
/**
 * @brief		アクセサリーコントロール
 */
//------------------------------------------------------------------
void	Set3DactChrAccesory( SCENE_ACT* sceneAct, u8 accesoryID )
{
	sceneAct->accesoryID = accesoryID;
}

//------------------------------------------------------------------
/**
 *
 *
 *
 * @brief	プレーヤーキャラクター
 *
 *
 *
 */
//------------------------------------------------------------------
static void movePlayer( GFL_G3D_SCENEOBJ* sceneObj, void* work );
//------------------------------------------------------------------
/**
 * @brief	セットアップ
 */
//------------------------------------------------------------------
//３Ｄグラフィックリソーステーブル
enum {
	G3DRES_ARMOR_HEAD_BMD = 0,
	G3DRES_ARMOR_BODY_BMD,
	G3DRES_ARMOR_ARMS_BMD,
	G3DRES_ARMOR_LEGS_BMD,

	G3DRES_ARMOR2_HEAD_BMD,
	G3DRES_ARMOR2_BODY_BMD,
	G3DRES_ARMOR2_ARMS_BMD,
	G3DRES_ARMOR2_LEGS_BMD,

	G3DRES_ACCE_SWORD,
	G3DRES_ACCE_SHIELD,
	G3DRES_ACCE_BOW,
	G3DRES_ACCE_STAFF,
};

static const GFL_G3D_UTIL_RES g3DresTbl[] = {
	{ ARCID_SAMPLE, NARC_haruka_armor_head_nsbmd,  GFL_G3D_UTIL_RESARC },
	{ ARCID_SAMPLE, NARC_haruka_armor_body_nsbmd,  GFL_G3D_UTIL_RESARC },
	{ ARCID_SAMPLE, NARC_haruka_armor_arms_nsbmd,  GFL_G3D_UTIL_RESARC },
	{ ARCID_SAMPLE, NARC_haruka_armor_legs_nsbmd,  GFL_G3D_UTIL_RESARC },

	{ ARCID_SAMPLE, NARC_haruka_armor2_head_nsbmd,  GFL_G3D_UTIL_RESARC },
	{ ARCID_SAMPLE, NARC_haruka_armor2_body_nsbmd,  GFL_G3D_UTIL_RESARC },
	{ ARCID_SAMPLE, NARC_haruka_armor2_arms_nsbmd,  GFL_G3D_UTIL_RESARC },
	{ ARCID_SAMPLE, NARC_haruka_armor2_legs_nsbmd,  GFL_G3D_UTIL_RESARC },

	{ ARCID_SAMPLE, NARC_haruka_sword_nsbmd,  GFL_G3D_UTIL_RESARC },
	{ ARCID_SAMPLE, NARC_haruka_shield_nsbmd,  GFL_G3D_UTIL_RESARC },
	{ ARCID_SAMPLE, NARC_haruka_bow_nsbmd,  GFL_G3D_UTIL_RESARC },
	{ ARCID_SAMPLE, NARC_haruka_staff_nsbmd,  GFL_G3D_UTIL_RESARC },
};

//---------------------
enum {
	HUMAN_ANM_COMMON = 0,
	HUMAN_ANM_SWORD_ATTACK,
	HUMAN2_ANM_SHOOT,
	HUMAN2_ANM_SPELL,
};

//３Ｄオブジェクトアニメーション定義テーブル
static const GFL_G3D_UTIL_ANM g3DhumanAnmTbl[] = {
	{ GFL_G3D_UTIL_NORESOURCE_ID, 0 },
	{ GFL_G3D_UTIL_NORESOURCE_ID, 0 },
	{ GFL_G3D_UTIL_NORESOURCE_ID, 0 },
	{ GFL_G3D_UTIL_NORESOURCE_ID, 0 },
};

//---------------------
//３Ｄオブジェクト定義テーブル
enum {
	G3DOBJ_HEAD = 0,
	G3DOBJ_BODY,
	G3DOBJ_ARMS,
	G3DOBJ_LEGS,
	G3DOBJ_WEAPON_LEFT,
	G3DOBJ_WEAPON_RIGHT,

	G3DOBJ_TEST_HEAD,
	G3DOBJ_TEST_BODY,
	G3DOBJ_TEST_ARMS,
	G3DOBJ_TEST_LEGS,
	G3DOBJ_TEST_WEAPON_LEFT,
	G3DOBJ_TEST_WEAPON_RIGHT,
};

static const GFL_G3D_UTIL_OBJ g3DobjTbl[] = {
	{ G3DRES_ARMOR2_HEAD_BMD,0,G3DRES_ARMOR2_HEAD_BMD,g3DhumanAnmTbl,NELEMS(g3DhumanAnmTbl)},
	{ G3DRES_ARMOR2_BODY_BMD,0,G3DRES_ARMOR2_BODY_BMD,g3DhumanAnmTbl,NELEMS(g3DhumanAnmTbl)},
	{ G3DRES_ARMOR2_ARMS_BMD,0,G3DRES_ARMOR2_ARMS_BMD,g3DhumanAnmTbl,NELEMS(g3DhumanAnmTbl)},
	{ G3DRES_ARMOR2_LEGS_BMD,0,G3DRES_ARMOR2_LEGS_BMD,g3DhumanAnmTbl,NELEMS(g3DhumanAnmTbl)},
	{ G3DRES_ACCE_SHIELD, 0, G3DRES_ACCE_SHIELD, NULL, 0 },
	{ G3DRES_ACCE_SWORD, 0, G3DRES_ACCE_SWORD, NULL, 0 },

	{ G3DRES_ARMOR_HEAD_BMD,0,G3DRES_ARMOR_HEAD_BMD,g3DhumanAnmTbl,NELEMS(g3DhumanAnmTbl)},
	{ G3DRES_ARMOR_BODY_BMD,0,G3DRES_ARMOR_BODY_BMD,g3DhumanAnmTbl,NELEMS(g3DhumanAnmTbl)},
	{ G3DRES_ARMOR_ARMS_BMD,0,G3DRES_ARMOR_ARMS_BMD,g3DhumanAnmTbl,NELEMS(g3DhumanAnmTbl)},
	{ G3DRES_ARMOR_LEGS_BMD,0,G3DRES_ARMOR_LEGS_BMD,g3DhumanAnmTbl,NELEMS(g3DhumanAnmTbl)},
	{ G3DRES_ACCE_BOW, 0, G3DRES_ACCE_BOW, NULL, 0 },
	{ G3DRES_ACCE_STAFF, 0, G3DRES_ACCE_STAFF, NULL, 0 },
};

//---------------------
//g3Dscene 設定テーブルデータ
static const GFL_G3D_UTIL_SETUP g3Dsetup = {
	g3DresTbl, NELEMS(g3DresTbl),
	g3DobjTbl, NELEMS(g3DobjTbl),
};

//------------------------------------------------------------------
//g3Dscene オブジェクト設定テーブルデータ プレーヤーキャラ (=PLAYER_SETUP_NUM)
static const GFL_G3D_SCENEOBJ_DATA g3DsceneObjData[] = {
	{
		G3DOBJ_HEAD, 0, 1, 31, TRUE, FALSE, 
		{	{ 0, 0, 0 },
			{ FX32_ONE/4, FX32_ONE/4, FX32_ONE/4 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},movePlayer,
	},
	{
		G3DOBJ_BODY, 0, 1, 31, TRUE, FALSE, 
		{	{ 0, 0, 0 },
			{ FX32_ONE/4, FX32_ONE/4, FX32_ONE/4 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{
		G3DOBJ_ARMS, 0, 1, 31, TRUE, FALSE, 
		{	{ 0, 0, 0 },
			{ FX32_ONE/4, FX32_ONE/4, FX32_ONE/4 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{
		G3DOBJ_LEGS, 0, 1, 31, TRUE, FALSE, 
		{	{ 0, 0, 0 },
			{ FX32_ONE/4, FX32_ONE/4, FX32_ONE/4 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
};

//---------------------
//g3Dscene アクセサリー設定テーブルデータ
static const GFL_G3D_SCENEACCESORY_DATA g3DsceneAccesoryData1[] = {
	{	G3DOBJ_WEAPON_RIGHT, 31,
		{	{ 0, 0, 0 },
			{ FX32_ONE/2, FX32_ONE/2, FX32_ONE/2 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},28	//右手jointID
	},
	{	G3DOBJ_WEAPON_LEFT, 31,
		{	{ -FX32_ONE*8, FX32_ONE*4, 0 },
			{ FX32_ONE/2, FX32_ONE/2, FX32_ONE/2 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},20	//左手jointID
	},
};

static const GFL_G3D_SCENEACCESORY_DATA g3DsceneAccesoryData2[] = {
	{	G3DOBJ_TEST_WEAPON_LEFT, 31,
		{	{ 0, 0, 0 },
			{ FX32_ONE/2, FX32_ONE/2, FX32_ONE/2 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},20	//左手jointID
	},
};

static const GFL_G3D_SCENEACCESORY_DATA g3DsceneAccesoryData3[] = {
	{	G3DOBJ_TEST_WEAPON_RIGHT, 31,
		{	{ 0, 0, 0 },
			{ FX32_ONE/2, FX32_ONE/2, FX32_ONE/2 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},28	//右手jointID
	},
};

//------------------------------------------------------------------
/**
 * @brief		３Ｄオブジェクト生成
 */
//------------------------------------------------------------------
SCENE_ACT* Create3Dact( GFL_G3D_SCENE* g3Dscene, HEAPID heapID )
{
	SCENE_ACT*	sceneAct = GFL_HEAP_AllocMemory( heapID, sizeof(SCENE_ACT) );
	u16			objUnitIdx;
	int			i;

	//３Ｄデータセットアップ
	sceneAct->g3DutilUnitIdx = GFL_G3D_SCENE_AddG3DutilUnit( g3Dscene, &g3Dsetup );
	objUnitIdx = GFL_G3D_SCENE_GetG3DutilUnitObjIdx( g3Dscene, sceneAct->g3DutilUnitIdx );

	//プレーヤーキャラ作成
	sceneAct->actCount = NELEMS(g3DsceneObjData);
	sceneAct->actID = GFL_G3D_SCENEOBJ_Add
					( g3Dscene, g3DsceneObjData, sceneAct->actCount, objUnitIdx );
	sceneAct->g3Dscene = g3Dscene;

	initActWork( sceneAct );

	//共通アニメを関連付ける（HEAD,BODY,ARMS,LEGS）
	for( i=0; i<sceneAct->actCount; i++ ){
		bindCommonAnm(	objUnitIdx, G3DOBJ_HEAD + i, 
						&g3DanmTblCommon[ G3DANM_COMMON_HUMAN_COMMON ], NELEMS(g3DhumanAnmTbl) );
	
#if 1
		bindCommonAnm( objUnitIdx, G3DOBJ_TEST_HEAD + i,
						&g3DanmTblCommon[ G3DANM_COMMON_HUMAN_COMMON ], NELEMS(g3DhumanAnmTbl) );
#endif
	}
	return sceneAct;
}
	
//------------------------------------------------------------------
/**
 * @brief		３Ｄオブジェクト破棄
 */
//------------------------------------------------------------------
void	Delete3Dact( SCENE_ACT* sceneAct )
{
	GFL_G3D_SCENEOBJ_Remove( sceneAct->g3Dscene, sceneAct->actID, sceneAct->actCount ); 
	GFL_G3D_SCENE_DelG3DutilUnit( sceneAct->g3Dscene, sceneAct->g3DutilUnitIdx );
	GFL_HEAP_FreeMemory( sceneAct );
}

//------------------------------------------------------------------
/**
 * @brief		動作関数
 */
//------------------------------------------------------------------
static const ANMIDATA_TBL playerAnmDataTable[] = {
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_INC_LOOP,	   0,   80, 4 * SYNC_WAIT },//ACTANM_CMD_STAY
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_INC_LOOP,	 100,  180, 4 * SYNC_WAIT },//ACTANM_CMD_WALK
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_INC_LOOP,	 200,  280, 6 * SYNC_WAIT },//ACTANM_CMD_RUN
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_INC_END,	 300,  340, 4 * SYNC_WAIT },//ACTANM_CMD_SITDOWN
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_INC_END,	 340,  380, 4 * SYNC_WAIT },//ACTANM_CMD_STANDUP
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_INC_END,	 300,  380, 4 * SYNC_WAIT },//ACTANM_CMD_TAKE
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_INC_END,	 500,  560, 6 * SYNC_WAIT },//ACTANM_CMD_HIT
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_INC_END,	 600,  680, 4 * SYNC_WAIT },//ACTANM_CMD_DEAD
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_INC_END,	 400,  420, 4 * SYNC_WAIT },//ACTANM_CMD_JUMP_RDY
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_INC_END,	 420,  455, 4 * SYNC_WAIT },//ACTANM_CMD_JUMPUP
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_INC_END,	 455,  475, 2 * SYNC_WAIT },//ACTANM_CMD_JUMPDOWN
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_INC_END,	 475,  495, 2 * SYNC_WAIT },//ACTANM_CMD_JUMP_END

	//ACTANM_CMD_SWORD_ATTACK0 ～ ATTACK7
	{ HUMAN_ANM_SWORD_ATTACK,	ACTANMSEQ_INC_END,	  80,  100-2, 8 * SYNC_WAIT },
	{ HUMAN_ANM_SWORD_ATTACK,	ACTANMSEQ_INC_END,	 300,  500-2, 8 * SYNC_WAIT },
	{ HUMAN_ANM_SWORD_ATTACK,	ACTANMSEQ_INC_END,	 700,  900-2, 8 * SYNC_WAIT },
	{ HUMAN_ANM_SWORD_ATTACK,	ACTANMSEQ_INC_END,	 100,  300-2, 8 * SYNC_WAIT },
	{ HUMAN_ANM_SWORD_ATTACK,	ACTANMSEQ_INC_END,	 500,  700-2, 8 * SYNC_WAIT },
	{ HUMAN_ANM_SWORD_ATTACK,	ACTANMSEQ_INC_END,	 900, 1100-2, 8 * SYNC_WAIT },
	{ HUMAN_ANM_SWORD_ATTACK,	ACTANMSEQ_INC_END,	1100, 1300-2, 8 * SYNC_WAIT },
	{ HUMAN_ANM_SWORD_ATTACK,	ACTANMSEQ_INC_END,	1300, 1500-2, 8 * SYNC_WAIT },
//	{ HUMAN2_ANM_SHOOT,		ACTANMSEQ_REFENDSTAY, 0, 0, 12 },//ACTANM_CMD_SHOOT
//	{ HUMAN2_ANM_SPELL,		ACTANMSEQ_REFENDSTAY, 0, 0, 6 },//ACTANM_CMD_SPELL
};

//------------------------------------------------------------------
static void movePlayer( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
	SCENE_ACT*	sceneAct = ((ACT_WORK*)work)->sceneAct;

	animeControl( sceneAct, playerAnmDataTable );

	if( sceneAct->accesoryID != sceneAct->accesoryIDNow ){	//アクセサリー更新チェック
		MtxFx33	rotate;
		VecFx32	rotVec = { 0, 0, 0 };
		BOOL result;
		//アクセサリーは HEAD に付属
		//HEAD.imd は -s オプションでコンバートしてマトリクス履歴を残すようにする

		//アクセサリーを外す(設定されてない場合はなにもしないで帰ってくる)
		GFL_G3D_SCENEOBJ_ACCESORY_Remove( sceneObj );

		switch( sceneAct->accesoryID ){
		case 0:	//手ぶら
			break;
		case 1:	//ハンマー
			result = GFL_G3D_SCENEOBJ_ACCESORY_Add( sceneObj,
							g3DsceneAccesoryData1, NELEMS(g3DsceneAccesoryData1) );
			GF_ASSERT( result == TRUE );
			rotVec.z = 0x4000;
			rotateCalc( &rotVec, &rotate );
			GFL_G3D_SCENEOBJ_ACCESORY_SetRotate( sceneObj, 0, &rotate );
			rotVec.y = 0xc000;
			rotVec.z = 0x0000;
			rotateCalc( &rotVec, &rotate );
			GFL_G3D_SCENEOBJ_ACCESORY_SetRotate( sceneObj, 1, &rotate );
			break;
		case 2:	//弓
			result = GFL_G3D_SCENEOBJ_ACCESORY_Add( sceneObj,
							g3DsceneAccesoryData2, NELEMS(g3DsceneAccesoryData2) );
			GF_ASSERT( result == TRUE );
			rotVec.z = 0x4000;
			rotateCalc( &rotVec, &rotate );
			GFL_G3D_SCENEOBJ_ACCESORY_SetRotate( sceneObj, 0, &rotate );
			break;
		case 3:	//杖
			result = GFL_G3D_SCENEOBJ_ACCESORY_Add( sceneObj,
							g3DsceneAccesoryData3, NELEMS(g3DsceneAccesoryData3) );
			GF_ASSERT( result == TRUE );
			rotVec.z = 0x4000;
			rotateCalc( &rotVec, &rotate );
			GFL_G3D_SCENEOBJ_ACCESORY_SetRotate( sceneObj, 0, &rotate );
			break;
		}
		sceneAct->accesoryIDNow = sceneAct->accesoryID;	//アクセサリー更新
	}
}


//-------------------------------------------------------------------------------------
//
// テスト
//
//-------------------------------------------------------------------------------------
//------------------------------------------------------------------
/**
 * @brief		装備書き換え
 */
//------------------------------------------------------------------
void	ChangeEquipNum( SCENE_ACT* sceneAct, u16 equipID )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj;
	u16		objIdx, chgObjIdx, objOffsIdx;
	int		anmID;
	fx32	nowFrm;
	
	GF_ASSERT( equipID<NELEMS(g3DsceneObjData) );

	g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene, sceneAct->actID + equipID );
	GFL_G3D_SCENEOBJ_GetObjID( g3DsceneObj, &objIdx );
	objOffsIdx = GFL_G3D_SCENE_GetG3DutilUnitObjIdx( sceneAct->g3Dscene, sceneAct->g3DutilUnitIdx );
	objIdx -= objOffsIdx;

	switch( equipID ){
	default:
		return;
	case 0:
		if( objIdx == G3DOBJ_HEAD ){
			chgObjIdx = G3DOBJ_TEST_HEAD;
		} else if( objIdx == G3DOBJ_TEST_HEAD ){
			chgObjIdx = G3DOBJ_HEAD;
		} else {
			GF_ASSERT(0);
		}
		break;
	case 1:
		if( objIdx == G3DOBJ_BODY ){
			chgObjIdx = G3DOBJ_TEST_BODY;
		} else if( objIdx == G3DOBJ_TEST_BODY ){
			chgObjIdx = G3DOBJ_BODY;
		} else {
			GF_ASSERT(0);
		}
		break;
	case 2:
		if( objIdx == G3DOBJ_ARMS ){
			chgObjIdx = G3DOBJ_TEST_ARMS;
		} else if( objIdx == G3DOBJ_TEST_ARMS ){
			chgObjIdx = G3DOBJ_ARMS;
		} else {
			GF_ASSERT(0);
		}
		break;
	case 3:
		if( objIdx == G3DOBJ_LEGS ){
			chgObjIdx = G3DOBJ_TEST_LEGS;
		} else if( objIdx == G3DOBJ_TEST_LEGS ){
			chgObjIdx = G3DOBJ_LEGS;
		} else {
			GF_ASSERT(0);
		}
		break;
	}
	chgObjIdx += objOffsIdx;
	GFL_G3D_SCENEOBJ_SetObjID( g3DsceneObj, &chgObjIdx );
}


//------------------------------------------------------------------
//------------------------------------------------------------------
/**
 *
 *
 *
 * @brief	ＮＰＣ
 *
 *
 *
 */
//------------------------------------------------------------------
//------------------------------------------------------------------
static void moveRiz( GFL_G3D_SCENEOBJ* sceneObj, void* work );
//------------------------------------------------------------------
/**
 * @brief	セットアップ
 */
//------------------------------------------------------------------
//３Ｄグラフィックリソーステーブル
enum {
	G3DRES_NPC_RIZ_BMD = 0,
};

static const GFL_G3D_UTIL_RES g3DNPCresTbl[] = {
	{ ARCID_SAMPLE, NARC_haruka_riz_nsbmd,  GFL_G3D_UTIL_RESARC },
};

//---------------------
enum {
	NPC_ANM = 0,
};

//３Ｄオブジェクトアニメーション定義テーブル
static const GFL_G3D_UTIL_ANM g3DNPCanmTbl[] = {
	{ GFL_G3D_UTIL_NORESOURCE_ID, 0 },
};

//---------------------
//３Ｄオブジェクト定義テーブル
enum {
	G3DOBJ_NPC_RIZ = 0,
};

static const GFL_G3D_UTIL_OBJ g3DNPCobjTbl[] = {
	{ G3DRES_NPC_RIZ_BMD,0,G3DRES_NPC_RIZ_BMD,g3DNPCanmTbl,NELEMS(g3DNPCanmTbl)},
};

//---------------------
//g3Dscene 設定テーブルデータ
static const GFL_G3D_UTIL_SETUP g3DNPCsetup = {
	g3DNPCresTbl, NELEMS(g3DNPCresTbl),
	g3DNPCobjTbl, NELEMS(g3DNPCobjTbl),
};

//------------------------------------------------------------------
//g3Dscene オブジェクト設定テーブルデータ プレーヤーキャラ (=PLAYER_SETUP_NUM)
static const GFL_G3D_SCENEOBJ_DATA g3DsceneObjNPCData[] = {
	{
		G3DOBJ_NPC_RIZ, 0, 1, 31, TRUE, FALSE, 
		{	{ 0, 0, 0 },
			{ FX32_ONE*2, FX32_ONE*2, FX32_ONE*2 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},moveRiz,
	},
};

//------------------------------------------------------------------
/**
 * @brief		３Ｄオブジェクト生成
 */
//------------------------------------------------------------------
SCENE_ACT* Create3DactNPC( GFL_G3D_SCENE* g3Dscene, HEAPID heapID )
{
	SCENE_ACT*	sceneAct = GFL_HEAP_AllocMemory( heapID, sizeof(SCENE_ACT) );
	u16			objUnitIdx;
	int			i;

	//３Ｄデータセットアップ
	sceneAct->g3DutilUnitIdx = GFL_G3D_SCENE_AddG3DutilUnit( g3Dscene, &g3DNPCsetup );
	objUnitIdx = GFL_G3D_SCENE_GetG3DutilUnitObjIdx( g3Dscene, sceneAct->g3DutilUnitIdx );

	//ＮＰＣキャラ作成
	sceneAct->actCount = NELEMS(g3DsceneObjNPCData);
	sceneAct->actID = GFL_G3D_SCENEOBJ_Add
					( g3Dscene, g3DsceneObjNPCData, NELEMS(g3DsceneObjNPCData), objUnitIdx );
	sceneAct->g3Dscene = g3Dscene;

	initActWork( sceneAct );

	//アニメを関連付ける
	bindCommonAnm( objUnitIdx, G3DOBJ_NPC_RIZ, &g3DanmTblCommon[ G3DANM_COMMON_NPC_RIZ ], 1 );
	
	Set3DactDrawSw( sceneAct, TRUE );
	Set3DactChrAnimeCmd( sceneAct, 0 );

	return sceneAct;
}
	
//------------------------------------------------------------------
/**
 * @brief		３Ｄオブジェクト破棄
 */
//------------------------------------------------------------------
void	Delete3DactNPC( SCENE_ACT* sceneAct )
{
	GFL_G3D_SCENEOBJ_Remove( sceneAct->g3Dscene, sceneAct->actID, sceneAct->actCount ); 
	GFL_G3D_SCENE_DelG3DutilUnit( sceneAct->g3Dscene, sceneAct->g3DutilUnitIdx );
	GFL_HEAP_FreeMemory( sceneAct );
}

//------------------------------------------------------------------
static const ANMIDATA_TBL rizAnmDataTable[] = {
	{ NPC_ANM,		ACTANMSEQ_INC_END_NEXT,	 140,  150,	 1 * SYNC_WAIT },	//STAY1
	{ NPC_ANM,		ACTANMSEQ_DEC_END_PREV,	 150,  140,	 1 * SYNC_WAIT },	//STAY2
	{ NPC_ANM,		ACTANMSEQ_INC_LOOP,		1000, 1200,	 8 * SYNC_WAIT },	//WALK
	{ NPC_ANM,		ACTANMSEQ_INC_END,		   0,  150,	 4 * SYNC_WAIT },	//WAKEUP
	{ NPC_ANM,		ACTANMSEQ_INC_END,		 800,  900,	 2 * SYNC_WAIT },	//SLEEP
	{ NPC_ANM,		ACTANMSEQ_INC_END,		 150,  250,	 8 * SYNC_WAIT },	//CRY
	{ NPC_ANM,		ACTANMSEQ_INC_END,		 250,  300,	 4 * SYNC_WAIT },	//FLYUPSTART1
	{ NPC_ANM,		ACTANMSEQ_INC_END_NEXT,	 300,  350,	 8 * SYNC_WAIT },	//FLYUPSTART2
	{ NPC_ANM,		ACTANMSEQ_INC_END_NEXT,	 350,  400,	 4 * SYNC_WAIT },	//FLYUP
	{ NPC_ANM,		ACTANMSEQ_DEC_END_PREV,	 400,  350,	 4 * SYNC_WAIT },	//FLYUP
	{ NPC_ANM,		ACTANMSEQ_INC_END,		 600,  650,	 8 * SYNC_WAIT },	//FLY
	{ NPC_ANM,		ACTANMSEQ_INC_END,		 400,  550,	 8 * SYNC_WAIT },	//ATTACK
	{ NPC_ANM,		ACTANMSEQ_INC_END,		 650,  700,	 8 * SYNC_WAIT },	//FLYDOWN
	{ NPC_ANM,		ACTANMSEQ_INC_END,		 700,  800,	 8 * SYNC_WAIT },	//FLYDOWNEND
};

//------------------------------------------------------------------
static void moveRiz( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
	ACT_WORK*	actWork = (ACT_WORK*)work;
	SCENE_ACT*	sceneAct = actWork->sceneAct;
	GFL_G3D_SCENEOBJ* g3DsceneObj = GFL_G3D_SCENEOBJ_Get(sceneAct->g3Dscene, sceneAct->actID);
//-------------------
#if 0
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
		u32	anmPat = GFUser_GetPublicRand( NELEMS(rizAnmDataTable) );
		Set3DactChrAnimeCmd( sceneAct, anmPat );
	}
#endif
	if( actWork->work[0] == 0 ){
		u32	timer = GFUser_GetPublicRand( 8 );

		actWork->work[0] = (timer+2) * 30;
		actWork->work[1] = GFUser_GetPublicRand( 0x10000 );
	} else {
		actWork->work[0]--;
	}
	{
		VecFx32 nowTrans, vecMove, rotVec;
		u16	 theta = actWork->work[1];
		fx32 speed = 1 * FX32_ONE;

		GFL_G3D_SCENEOBJ_GetPos( g3DsceneObj, &nowTrans );

		vecMove.x = FX_SinIdx( theta );
		vecMove.y = 0;
		vecMove.z = FX_CosIdx( theta );
		VEC_MultAdd( speed, &vecMove, &nowTrans, &nowTrans );
		if( CheckGroundOutRange( &nowTrans ) == TRUE ){
			Set3DactTrans( sceneAct, &nowTrans );
		}
		rotVec.x = 0;
		rotVec.y = actWork->work[1];
		rotVec.z = 0;

		Set3DactRotate( sceneAct, &rotVec );
	}
	Set3DactChrAnimeCmd( sceneAct, 2 );
//-------------------
	animeControl( sceneAct, rizAnmDataTable );
}


