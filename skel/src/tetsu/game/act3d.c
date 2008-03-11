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

typedef struct { 
	SCENE_ACT*		sceneAct;

}ACT_WORK;//setup.cで指定されているサイズに収めるように注意

static void movePlayer( GFL_G3D_SCENEOBJ* sceneObj, void* work );

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

#include "src/sample_graphic/haruka.naix"
static const char g3DarcPath[] = {"src/sample_graphic/haruka.narc"};
//------------------------------------------------------------------
/**
 * @brief	システムおよび共通リソースのセットアップと破棄
 */
//------------------------------------------------------------------
//３Ｄグラフィックリソーステーブル
enum {
	G3DRES_COMMON_HUMAN_COMMON_BCA = 0,
	G3DRES_COMMON_SWORD_ATTACK_BCA,
	G3DRES_COMMON_HUMAN2_SHOOT_BCA,
	G3DRES_COMMON_HUMAN2_SPELL_BCA,
};

static const GFL_G3D_UTIL_RES g3DresTblCommon[] = {
	{ (u32)g3DarcPath, NARC_haruka_armor_common_nsbca,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_haruka_sword_attack_nsbca,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_haruka_human2_shoot_nsbca,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_haruka_human2_spell_nsbca,  GFL_G3D_UTIL_RESPATH },
};

//---------------------
//３Ｄオブジェクトアニメーション定義テーブル
enum {
	HUMAN_ANM_COMMON = 0,
	HUMAN_ANM_SWORD_ATTACK,
	HUMAN2_ANM_SHOOT,
	HUMAN2_ANM_SPELL,
};

static const GFL_G3D_UTIL_ANM g3DhumanAnmTblCommon[] = {
	{ G3DRES_COMMON_HUMAN_COMMON_BCA, 0 },
	{ G3DRES_COMMON_SWORD_ATTACK_BCA, 0 },
	{ G3DRES_COMMON_HUMAN2_SHOOT_BCA, 0 },
	{ G3DRES_COMMON_HUMAN2_SPELL_BCA, 0 },
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
	{ (u32)g3DarcPath, NARC_haruka_armor_head_nsbmd,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_haruka_armor_body_nsbmd,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_haruka_armor_arms_nsbmd,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_haruka_armor_legs_nsbmd,  GFL_G3D_UTIL_RESPATH },

	{ (u32)g3DarcPath, NARC_haruka_armor2_head_nsbmd,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_haruka_armor2_body_nsbmd,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_haruka_armor2_arms_nsbmd,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_haruka_armor2_legs_nsbmd,  GFL_G3D_UTIL_RESPATH },

	{ (u32)g3DarcPath, NARC_haruka_sword_nsbmd,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_haruka_shield_nsbmd,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_haruka_bow_nsbmd,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_haruka_staff_nsbmd,  GFL_G3D_UTIL_RESPATH },
};

//---------------------
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
	sceneAct->actID = GFL_G3D_SCENEOBJ_Add
					( g3Dscene, g3DsceneObjData, NELEMS(g3DsceneObjData), objUnitIdx );
	sceneAct->g3Dscene = g3Dscene;

	sceneAct->animeCmd		= 0;
	sceneAct->animeCmdNow	= sceneAct->animeCmd;
	sceneAct->animeID		= 0;
	sceneAct->accesoryID	= 0;
	sceneAct->accesoryIDNow	= sceneAct->accesoryID;
	sceneAct->busyFlag		= FALSE;
	sceneAct->anmSetFlag	= TRUE;
	{
		ACT_WORK*	work;
		u16			objID;

		for( i=0; i<NELEMS(g3DsceneObjData); i++ ){
			objID = sceneAct->actID + i; 
			work = (ACT_WORK*)GFL_G3D_SCENEOBJ_GetWork
						( GFL_G3D_SCENEOBJ_Get( g3Dscene, objID ) ); 
			work->sceneAct = sceneAct;
		}
	}
	//共通アニメを関連付ける（HEAD,BODY,ARMS,LEGS）
	for( i=0; i<4; i++ ){
		GFL_G3D_UTIL_SetObjAnotherUnitAnime
			( GFL_G3D_SCENE_GetG3Dutil( g3Dscene ), objUnitIdx, G3DOBJ_HEAD + i,
			SceneActSys->g3DutilUnitIdx, g3DhumanAnmTblCommon, NELEMS(g3DhumanAnmTblCommon) );
		//アニメーションの初期有効設定(0番のアニメーションを有効にする)
		GFL_G3D_OBJECT_EnableAnime
			( GFL_G3D_SCENE_GetG3DutilObjHandle( g3Dscene, objUnitIdx + G3DOBJ_HEAD + i), 0 );
	}
#if 1
	for( i=0; i<4; i++ ){
		GFL_G3D_UTIL_SetObjAnotherUnitAnime
			( GFL_G3D_SCENE_GetG3Dutil( g3Dscene ), objUnitIdx, G3DOBJ_TEST_HEAD + i,
			SceneActSys->g3DutilUnitIdx, g3DhumanAnmTblCommon, NELEMS(g3DhumanAnmTblCommon) );
		//アニメーションの初期有効設定(0番のアニメーションを有効にする)
		GFL_G3D_OBJECT_EnableAnime
			( GFL_G3D_SCENE_GetG3DutilObjHandle( g3Dscene, objUnitIdx + G3DOBJ_TEST_HEAD + i), 0 );
	}
#endif
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

	for( i=0; i<NELEMS(g3DsceneObjData); i++ ){
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

	for( i=0; i<NELEMS(g3DsceneObjData); i++ ){
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

	for( i=0; i<NELEMS(g3DsceneObjData); i++ ){
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

	for( i=0; i<NELEMS(g3DsceneObjData); i++ ){
		g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene, sceneAct->actID + i );
		GFL_G3D_SCENEOBJ_SetBlendAlpha( g3DsceneObj, alpha );
	}
}


//------------------------------------------------------------------
/**
 * @brief		３Ｄオブジェクト動作コントロール
 */
//------------------------------------------------------------------
//------------------------------------------------------------------
//
//	プレーヤーキャラ
//
//------------------------------------------------------------------
BOOL	Set3DactPlayerChrAnimeCmd( SCENE_ACT* sceneAct, u8 animeCmd )
{
	if( animeCmd != sceneAct->animeCmdNow ){	//アニメ更新チェック
		sceneAct->animeCmd = animeCmd;
		sceneAct->anmSetFlag = TRUE;
		return TRUE; 
	} else {
		return FALSE; 
	}
}

void	Set3DactPlayerChrAnimeForceCmd( SCENE_ACT* sceneAct, u8 animeCmd )
{
	sceneAct->animeCmd = animeCmd;
	sceneAct->anmSetFlag = TRUE;
}

void	Set3DactPlayerChrAccesory( SCENE_ACT* sceneAct, u8 accesoryID )
{
	sceneAct->accesoryID = accesoryID;
}

BOOL	Check3DactPlayerBusy( SCENE_ACT* sceneAct )
{
	return sceneAct->busyFlag;
}

//------------------------------------------------------------------
enum {
	ACTANMSEQ_PLAYER_LOOP = 0,
	ACTANMSEQ_PLAYER_END,
	ACTANMSEQ_PLAYER_END2,
	ACTANMSEQ_PLAYER_ENDSTAY,
	ACTANMSEQ_PLAYER_REFENDSTAY,
};

typedef struct {
	int animeID; 
	int animeSeq; 
	int start; 
	int end; 
	int rate; 
}PLAYER_ANMIDATA_TBL;

#define SYNC_WAIT (2)
static const PLAYER_ANMIDATA_TBL anmDataTable[] = {
	//ACTANM_CMD_STAY
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_PLAYER_LOOP,		0,		80,		2 * SYNC_WAIT },
	//ACTANM_CMD_WALK
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_PLAYER_LOOP,		100,	180,	2 * SYNC_WAIT },
	//ACTANM_CMD_RUN
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_PLAYER_LOOP,		200,	280,	3 * SYNC_WAIT },
	//ACTANM_CMD_SITDOWN
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_PLAYER_END,		300,	340,	2 * SYNC_WAIT },
	//ACTANM_CMD_STANDUP
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_PLAYER_END,		340,	380,	2 * SYNC_WAIT },
	//ACTANM_CMD_TAKE
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_PLAYER_END,		300,	380,	2 * SYNC_WAIT },
	//ACTANM_CMD_HIT
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_PLAYER_END,		500,	560,	2 * SYNC_WAIT },
	//ACTANM_CMD_DEAD
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_PLAYER_END,		600,	680,	2 * SYNC_WAIT },
	//ACTANM_CMD_JUMP_RDY
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_PLAYER_END,		400,	420,	2 * SYNC_WAIT },
	//ACTANM_CMD_JUMPUP
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_PLAYER_END2,		420,	455,	2 * SYNC_WAIT },
	//ACTANM_CMD_JUMPDOWN
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_PLAYER_END2,		455,	475,	1 * SYNC_WAIT },
	//ACTANM_CMD_JUMP_END
	{ HUMAN_ANM_COMMON,		ACTANMSEQ_PLAYER_END,		475,	495,	1 * SYNC_WAIT },

	//ACTANM_CMD_SWORD_ATTACK1 ～ ATTACK6
	{ HUMAN_ANM_SWORD_ATTACK,	ACTANMSEQ_PLAYER_END,	300,	500,	4 * SYNC_WAIT },
	{ HUMAN_ANM_SWORD_ATTACK,	ACTANMSEQ_PLAYER_END,	700,	900,	4 * SYNC_WAIT },
	{ HUMAN_ANM_SWORD_ATTACK,	ACTANMSEQ_PLAYER_END,	100,	300,	4 * SYNC_WAIT },
	{ HUMAN_ANM_SWORD_ATTACK,	ACTANMSEQ_PLAYER_END,	500,	700,	4 * SYNC_WAIT },
	{ HUMAN_ANM_SWORD_ATTACK,	ACTANMSEQ_PLAYER_END,	900,	1100,	4 * SYNC_WAIT },
	{ HUMAN_ANM_SWORD_ATTACK,	ACTANMSEQ_PLAYER_END,	1100,	1300,	4 * SYNC_WAIT },
//	{ HUMAN2_ANM_SHOOT,		ACTANMSEQ_PLAYER_REFENDSTAY,0,		0,		12 },//ACTANM_CMD_SHOOT
//	{ HUMAN2_ANM_SPELL,		ACTANMSEQ_PLAYER_REFENDSTAY,0,		0,		6 },//ACTANM_CMD_SPELL
};

static void setAnimetion( SCENE_ACT* sa, int anmID, fx32 start )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj;
	int i;

	if( anmID != sa->animeID ){
		for( i=0; i<NELEMS(g3DsceneObjData); i++ ){
			g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sa->g3Dscene, sa->actID + i );
			GFL_G3D_SCENEOBJ_DisableAnime( g3DsceneObj, sa->animeID );
			GFL_G3D_SCENEOBJ_EnableAnime( g3DsceneObj, anmID );
		}
		sa->animeID = anmID;
	}
	for( i=0; i<NELEMS(g3DsceneObjData); i++ ){
		g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sa->g3Dscene, sa->actID + i );
		GFL_G3D_SCENEOBJ_SetAnimeFrame( g3DsceneObj, anmID, (int*)&start );
	}
}

static void setAnimeFrame( SCENE_ACT* sa, int anmID, fx32 frame )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj;
	int i;

	for( i=0; i<NELEMS(g3DsceneObjData); i++ ){
		g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sa->g3Dscene, sa->actID + i );
		GFL_G3D_SCENEOBJ_SetAnimeFrame( g3DsceneObj, anmID, (int*)&frame );
	}
}

static BOOL incAnimetion( SCENE_ACT* sa, int anmID, fx32 end, fx32 rate )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj;
	fx32 nowFrm;
	int i;

	for( i=0; i<NELEMS(g3DsceneObjData); i++ ){
		g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sa->g3Dscene, sa->actID + i );
		GFL_G3D_SCENEOBJ_IncAnimeFrame( g3DsceneObj, anmID, rate );
	}

	g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sa->g3Dscene, sa->actID );	//0のみ参照
	if( GFL_G3D_SCENEOBJ_GetAnimeFrame( g3DsceneObj, anmID, (int*)&nowFrm ) == FALSE ){
		return FALSE;
	}
	if( nowFrm > end ){
		return FALSE;
	} else {
		return TRUE;
	}
}

//暫定
static BOOL incAnimetionRef( SCENE_ACT* sa, int anmID, fx32 rate )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj;
	fx32 nowFrm;
	int i;
	BOOL result; 

	for( i=0; i<NELEMS(g3DsceneObjData); i++ ){
		g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sa->g3Dscene, sa->actID + i );
		result = GFL_G3D_SCENEOBJ_IncAnimeFrame( g3DsceneObj, anmID, rate );
	}
	return result;
}

//------------------------------------------------------------------
static void movePlayer( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
	SCENE_ACT*	sa = ((ACT_WORK*)work)->sceneAct;
	int			anmID;
	fx32		start, end, rate;

	if( sa->anmSetFlag == TRUE ){	//アニメ更新チェック
		anmID = anmDataTable[ sa->animeCmd ].animeID;
		start = FX32_ONE * anmDataTable[ sa->animeCmd ].start;
		sa->seq = anmDataTable[ sa->animeCmd ].animeSeq;
		setAnimetion( sa, anmID, start );
		sa->animeCmdNow = sa->animeCmd;
		sa->anmSetFlag = FALSE;
	}
	start	= FX32_ONE * anmDataTable[ sa->animeCmdNow ].start;
	end		= FX32_ONE * anmDataTable[ sa->animeCmdNow ].end;
	rate	= FX32_ONE * anmDataTable[ sa->animeCmdNow ].rate;

	switch( sa->seq ){
	case ACTANMSEQ_PLAYER_LOOP:
		if( incAnimetion( sa, sa->animeID, end, rate ) == FALSE ){
			setAnimeFrame( sa, sa->animeID, start );
		}
		sa->busyFlag = FALSE;
		break;

	case ACTANMSEQ_PLAYER_END:
		if( incAnimetion( sa, sa->animeID, end, rate ) == FALSE ){
			setAnimeFrame( sa, sa->animeID, end );
			sa->busyFlag = FALSE;
		} else {
			sa->busyFlag = TRUE;
		}
		break;

	case ACTANMSEQ_PLAYER_END2:
		if( incAnimetion( sa, sa->animeID, end, rate ) == FALSE ){
			setAnimeFrame( sa, sa->animeID, end );
		}
		sa->busyFlag = FALSE;
		break;

	case ACTANMSEQ_PLAYER_ENDSTAY:
		if( incAnimetion( sa, sa->animeID, end, rate ) == FALSE ){
			setAnimeFrame( sa, sa->animeID, end );
			sa->animeCmd = ACTANM_CMD_STAY;
			sa->anmSetFlag = TRUE;
		} else {
			sa->busyFlag = TRUE;
		}
		break;
	case ACTANMSEQ_PLAYER_REFENDSTAY://暫定
		if( incAnimetionRef( sa, sa->animeID, rate ) == FALSE ){
			setAnimeFrame( sa, sa->animeID, end );
			sa->animeCmd = ACTANM_CMD_STAY;
			sa->anmSetFlag = TRUE;
		} else {
			sa->busyFlag = TRUE;
		}
		break;
	}
	if( sa->accesoryID != sa->accesoryIDNow ){	//アクセサリー更新チェック
		MtxFx33	rotate;
		VecFx32	rotVec = { 0, 0, 0 };
		BOOL result;
		//アクセサリーは HEAD に付属
		//HEAD.imd は -s オプションでコンバートしてマトリクス履歴を残すようにする

		//アクセサリーを外す(設定されてない場合はなにもしないで帰ってくる)
		GFL_G3D_SCENEOBJ_ACCESORY_Remove( sceneObj );

		switch( sa->accesoryID ){
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
		sa->accesoryIDNow = sa->accesoryID;	//アクセサリー更新
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


