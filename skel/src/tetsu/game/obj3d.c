//============================================================================================
/**
 * @file	obj3d.c
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
struct _SCENE_OBJ {
	GFL_G3D_SCENE*	g3Dscene;
	int				objID;
	int				objCount;
};

typedef struct { 
	int			seq;
	VecFx32		rotateVec;
	u8			animeCmd;
	u8			animeCmdNow;
	u8			animeID;
	u8			accesoryID;
	u8			accesoryIDNow;
	BOOL		busyFlag;
}OBJ_WORK;//setup.cで指定されているサイズに収めるように注意

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

//------------------------------------------------------------------
/**
 * @brief	３Ｄデータ
 */
//------------------------------------------------------------------
//g3Dscene オブジェクト設定テーブルデータ
static const GFL_G3D_SCENEOBJ_DATA g3DsceneObjData[] = {
	{	G3DOBJ_HUMAN2, 0, 1, 31, FALSE, TRUE, 
		{	{ 0, 0, 0 },
			{ FX32_ONE/4, FX32_ONE/4, FX32_ONE/4 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},movePlayer,
	},
};

//---------------------
//g3Dscene アクセサリー設定テーブルデータ
static const GFL_G3D_SCENEACCESORY_DATA g3DsceneAccesoryData1[] = {
	{	G3DOBJ_ACCE_SWORD, 31,
		{	{ 0, 0, 0 },
			{ FX32_ONE/2, FX32_ONE/2, FX32_ONE/2 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},28	//右手jointID
	},
	{	G3DOBJ_ACCE_SHIELD, 31,
		{	{ -FX32_ONE*8, FX32_ONE*4, 0 },
			{ FX32_ONE/2, FX32_ONE/2, FX32_ONE/2 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},20	//左手jointID
	},
};

static const GFL_G3D_SCENEACCESORY_DATA g3DsceneAccesoryData2[] = {
	{	G3DOBJ_ACCE_BOW, 31,
		{	{ 0, 0, 0 },
			{ FX32_ONE/2, FX32_ONE/2, FX32_ONE/2 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},20	//左手jointID
	},
};

static const GFL_G3D_SCENEACCESORY_DATA g3DsceneAccesoryData3[] = {
	{	G3DOBJ_ACCE_STAFF, 31,
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
SCENE_OBJ* Create3Dobj( GFL_G3D_SCENE* g3Dscene, HEAPID heapID )
{
	SCENE_OBJ* sceneObj = GFL_HEAP_AllocMemory( heapID, sizeof(SCENE_OBJ) );

	//自機作成
	sceneObj->objID = GFL_G3D_SCENEOBJ_Add
						( g3Dscene, g3DsceneObjData, NELEMS(g3DsceneObjData) );
	sceneObj->g3Dscene = g3Dscene;
	sceneObj->objCount = NELEMS(g3DsceneObjData);

	return sceneObj;
}
	
//------------------------------------------------------------------
/**
 * @brief		３Ｄオブジェクト破棄
 */
//------------------------------------------------------------------
void	Delete3Dobj( SCENE_OBJ* sceneObj )
{
	GFL_G3D_SCENEOBJ_Remove( sceneObj->g3Dscene, sceneObj->objID, sceneObj->objCount ); 
	GFL_HEAP_FreeMemory( sceneObj );
}

//------------------------------------------------------------------
/**
 * @brief		３Ｄオブジェクトハンドルポインタ取得
 */
//------------------------------------------------------------------
GFL_G3D_SCENEOBJ*	Get3DobjHandle( SCENE_OBJ* sceneObj, u32 idx )
{
	return GFL_G3D_SCENEOBJ_Get( sceneObj->g3Dscene, sceneObj->objID + idx );
}

//------------------------------------------------------------------
/**
 * @brief		３Ｄオブジェクトワークポインタ取得
 */
//------------------------------------------------------------------
void*	Get3DobjWork( SCENE_OBJ* sceneObj, u32 idx )
{
	return GFL_G3D_SCENEOBJ_GetWork( GFL_G3D_SCENEOBJ_Get( sceneObj->g3Dscene, idx ) );
}

//------------------------------------------------------------------
/**
 * @brief		３Ｄオブジェクト座標の取得と設定
 */
//------------------------------------------------------------------
void	Get3DobjTrans( SCENE_OBJ* sceneObj, u32 idx, VecFx32* trans )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneObj->g3Dscene, 
														sceneObj->objID + idx );
	GFL_G3D_SCENEOBJ_GetPos( g3DsceneObj, trans );
}

void	Set3DobjTrans( SCENE_OBJ* sceneObj, u32 idx, VecFx32* trans )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneObj->g3Dscene, 
														sceneObj->objID + idx );
	GFL_G3D_SCENEOBJ_SetPos( g3DsceneObj, trans );
}

//------------------------------------------------------------------
/**
 * @brief		３Ｄオブジェクト回転の取得と設定
 */
//------------------------------------------------------------------
void	Get3DobjRotate( SCENE_OBJ* sceneObj, u32 idx, VecFx32* rotate )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneObj->g3Dscene, 
														sceneObj->objID + idx );
	OBJ_WORK* work = (OBJ_WORK*)GFL_G3D_SCENEOBJ_GetWork( g3DsceneObj );

	*rotate = work->rotateVec;
}

void	Set3DobjRotate( SCENE_OBJ* sceneObj, u32 idx, VecFx32* rotate )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneObj->g3Dscene, 
														sceneObj->objID + idx );
	OBJ_WORK* work = (OBJ_WORK*)GFL_G3D_SCENEOBJ_GetWork( g3DsceneObj );
	MtxFx33			rotateMtx;

	work->rotateVec = *rotate;
	rotateCalc( &work->rotateVec, &rotateMtx );
	GFL_G3D_SCENEOBJ_SetRotate( g3DsceneObj, &rotateMtx );
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
void	Set3DobjPlayerChrAnimeCmd( SCENE_OBJ* sceneObj, u32 idx, u8 animeCmd )
{
	OBJ_WORK* work = (OBJ_WORK*)GFL_G3D_SCENEOBJ_GetWork
						( GFL_G3D_SCENEOBJ_Get( sceneObj->g3Dscene,sceneObj->objID + idx )); 
	work->animeCmd = animeCmd;
}

void	Set3DobjPlayerChrAccesory( SCENE_OBJ* sceneObj, u32 idx, u8 accesoryID )
{
	OBJ_WORK* work = (OBJ_WORK*)GFL_G3D_SCENEOBJ_GetWork
						( GFL_G3D_SCENEOBJ_Get( sceneObj->g3Dscene,sceneObj->objID + idx )); 
	work->accesoryID = accesoryID;
}

BOOL	Check3DobjPlayerBusy( SCENE_OBJ* sceneObj, u32 idx )
{
	OBJ_WORK* work = (OBJ_WORK*)GFL_G3D_SCENEOBJ_GetWork
						( GFL_G3D_SCENEOBJ_Get( sceneObj->g3Dscene,sceneObj->objID + idx )); 
	return work->busyFlag;
}

//------------------------------------------------------------------
enum {
	OBJANMSEQ_PLAYER_SET = 0,
	OBJANMSEQ_PLAYER_STAY,
	OBJANMSEQ_PLAYER_WALK,
	OBJANMSEQ_PLAYER_RUN,
	OBJANMSEQ_PLAYER_ATTACK,
	OBJANMSEQ_PLAYER_SHOOT,
	OBJANMSEQ_PLAYER_SPELL,
	OBJANMSEQ_PLAYER_SITDOWN,
	OBJANMSEQ_PLAYER_STANDUP,
	OBJANMSEQ_PLAYER_TAKE,
};

typedef struct {
	int animeID; 
	int animeSeq; 
	int start; 
	int end; 
	int wait; 
}PLAYER_ANMIDATA_TBL;

static const PLAYER_ANMIDATA_TBL anmDataTable[] = {
	{ HUMAN2_ANM_STAY,		OBJANMSEQ_PLAYER_STAY,		0,	0,	2 },	//OBJANM_PLAYER_STAY
	{ HUMAN2_ANM_WALK,		OBJANMSEQ_PLAYER_WALK,		0,	0,	2 },	//OBJANM_PLAYER_WALK
	{ HUMAN2_ANM_RUN,		OBJANMSEQ_PLAYER_RUN,		0,	0,	4 },	//OBJANM_PLAYER_RUN
	{ HUMAN2_ANM_ATTACK,	OBJANMSEQ_PLAYER_ATTACK,	0,	0,	2 },	//OBJANM_PLAYER_ATTACK
	{ HUMAN2_ANM_SHOOT,		OBJANMSEQ_PLAYER_SHOOT,		0,	0,	2 },	//OBJANM_PLAYER_SHOOT
	{ HUMAN2_ANM_SPELL,		OBJANMSEQ_PLAYER_SPELL,		0,	0,	2 },	//OBJANM_PLAYER_SPELL
	{ HUMAN2_ANM_SIT,		OBJANMSEQ_PLAYER_SITDOWN,	0,	20,	1 },	//OBJANM_PLAYER_SITDOWN
	{ HUMAN2_ANM_SIT,		OBJANMSEQ_PLAYER_STANDUP,	20,	0,	1 },	//OBJANM_PLAYER_STANDUP
	{ HUMAN2_ANM_SIT,		OBJANMSEQ_PLAYER_TAKE,		0,	0,	1 },	//OBJANM_PLAYER_TAKE
};

static void movePlayer( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
	OBJ_WORK*	pw = (OBJ_WORK*)work;
	int			anmID;
	fx32		start, end, wait;

	if( pw->animeCmd != pw->animeCmdNow ){	//アニメ更新チェック
		anmID = anmDataTable[pw->animeCmdNow].animeID;
		GFL_G3D_SCENEOBJ_DisableAnime( sceneObj, anmID );

		pw->seq = anmDataTable[pw->animeCmd].animeSeq;
		anmID = anmDataTable[pw->animeCmd].animeID;

		GFL_G3D_SCENEOBJ_EnableAnime( sceneObj, anmID );
		start = FX32_ONE * anmDataTable[pw->animeCmd].start;
		GFL_G3D_SCENEOBJ_SetAnimeFrame( sceneObj, anmID, (int*)&start );

		pw->animeCmdNow = pw->animeCmd;
	}
	anmID	= anmDataTable[pw->animeCmdNow].animeID;
	end		= FX32_ONE * anmDataTable[pw->animeCmd].end;
	wait	= FX32_ONE * anmDataTable[pw->animeCmd].wait;

	switch( pw->seq ){
	case OBJANMSEQ_PLAYER_STAY:
	case OBJANMSEQ_PLAYER_WALK:
	case OBJANMSEQ_PLAYER_RUN:
		GFL_G3D_SCENEOBJ_LoopAnimeFrame( sceneObj, anmID, wait ); 
		pw->busyFlag = FALSE;
		break;
	case OBJANMSEQ_PLAYER_ATTACK:
	case OBJANMSEQ_PLAYER_SHOOT:
	case OBJANMSEQ_PLAYER_SPELL:
	case OBJANMSEQ_PLAYER_TAKE:
	case OBJANMSEQ_PLAYER_STANDUP:
		if( GFL_G3D_SCENEOBJ_IncAnimeFrame( sceneObj, anmID, wait ) == FALSE ){
			pw->animeCmd = OBJANM_CMD_STAY;
		} else {
			pw->busyFlag = TRUE;
		}
		break;
	case OBJANMSEQ_PLAYER_SITDOWN:
		{
			fx32 nowFrm;
			GFL_G3D_SCENEOBJ_GetAnimeFrame( sceneObj, anmID, (int*)&nowFrm );
			if( nowFrm < end ){
				GFL_G3D_SCENEOBJ_IncAnimeFrame( sceneObj, anmID, wait );
				pw->busyFlag = TRUE;
			} else {
				pw->busyFlag = FALSE;
			}
		}
		break;
	}

	if( pw->accesoryID != pw->accesoryIDNow ){	//アクセサリー更新チェック
		MtxFx33	rotate;
		VecFx32	rotVec = { 0, 0, 0 };
		BOOL result;

		//アクセサリーを外す(設定されてない場合はなにもしないで帰ってくる)
		GFL_G3D_SCENEOBJ_ACCESORY_Remove( sceneObj );

		switch( pw->accesoryID ){
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
		pw->accesoryIDNow = pw->accesoryID;	//アクセサリー更新
	}
}


