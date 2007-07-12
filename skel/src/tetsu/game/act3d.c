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
struct _SCENE_ACT {
	GFL_G3D_SCENE*	g3Dscene;
	int				actID;
	int				actCount;
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
	BOOL		hitCheckFlag;
	BOOL		anmSetFlag;
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

//------------------------------------------------------------------
/**
 * @brief	３Ｄデータ
 */
//------------------------------------------------------------------
//g3Dscene オブジェクト設定テーブルデータ プレーヤーキャラ (=PLAYER_SETUP_NUM)
static const GFL_G3D_SCENEOBJ_DATA g3DsceneObjDataDefault = {
	G3DOBJ_HUMAN1, 0, 1, 31, FALSE, TRUE, 
	{	{ 0, 0, 0 },
		{ FX32_ONE/4, FX32_ONE/4, FX32_ONE/4 },
		{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
	},movePlayer,
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
SCENE_ACT* Create3Dact( GFL_G3D_SCENE* g3Dscene, HEAPID heapID )
{
	SCENE_ACT* sceneAct = GFL_HEAP_AllocMemory( heapID, sizeof(SCENE_ACT) );

	//プレーヤーキャラ作成
	GFL_G3D_SCENEOBJ_DATA*	pdata;
	int	i;

	pdata = GFL_HEAP_AllocMemoryLo( heapID, sizeof(GFL_G3D_SCENEOBJ_DATA) * PLAYER_SETUP_NUM );
	for( i=0; i<PLAYER_SETUP_NUM; i++ ){
		pdata[i] = g3DsceneObjDataDefault;
		pdata[i].objID = G3DOBJ_HUMAN1+i;
	}
	sceneAct->actID = GFL_G3D_SCENEOBJ_Add( g3Dscene, pdata, PLAYER_SETUP_NUM );
	sceneAct->g3Dscene = g3Dscene;
	sceneAct->actCount = PLAYER_SETUP_NUM;
	{
		ACT_WORK*	work;
		VecFx32		initRotate = { 0, 0x8000, 0 };

		for( i=0; i<PLAYER_SETUP_NUM; i++ ){
			work = (ACT_WORK*)GFL_G3D_SCENEOBJ_GetWork
							( GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene, sceneAct->actID + i )); 

			work->animeCmd		= 0;
			work->animeCmdNow	= work->animeCmd;
			work->animeID		= 0;
			work->accesoryID	= 0;
			work->accesoryIDNow	= work->accesoryID;
			work->busyFlag		= FALSE;
			work->hitCheckFlag	= FALSE;
			work->anmSetFlag = TRUE;
			Set3DactRotate( sceneAct, i, &initRotate );
		}
	}
	GFL_HEAP_FreeMemory(pdata);

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
 * @brief		３Ｄオブジェクト座標の取得と設定
 */
//------------------------------------------------------------------
void	Get3DactTrans( SCENE_ACT* sceneAct, u32 idx, VecFx32* trans )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene, 
														sceneAct->actID + idx );
	GFL_G3D_SCENEOBJ_GetPos( g3DsceneObj, trans );
}

void	Set3DactTrans( SCENE_ACT* sceneAct, u32 idx, VecFx32* trans )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene, 
														sceneAct->actID + idx );
	GFL_G3D_SCENEOBJ_SetPos( g3DsceneObj, trans );
}

//------------------------------------------------------------------
/**
 * @brief		３Ｄオブジェクト回転の取得と設定
 */
//------------------------------------------------------------------
void	Get3DactRotate( SCENE_ACT* sceneAct, u32 idx, VecFx32* rotate )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene, 
														sceneAct->actID + idx );
	ACT_WORK* work = (ACT_WORK*)GFL_G3D_SCENEOBJ_GetWork( g3DsceneObj );

	*rotate = work->rotateVec;
}

void	Set3DactRotate( SCENE_ACT* sceneAct, u32 idx, VecFx32* rotate )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene, 
														sceneAct->actID + idx );
	ACT_WORK* work = (ACT_WORK*)GFL_G3D_SCENEOBJ_GetWork( g3DsceneObj );
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
void	Set3DactPlayerChrAnimeCmd( SCENE_ACT* sceneAct, u32 idx, u8 animeCmd )
{
	ACT_WORK* work = (ACT_WORK*)GFL_G3D_SCENEOBJ_GetWork
						( GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene,sceneAct->actID + idx )); 
	if( animeCmd != work->animeCmdNow ){	//アニメ更新チェック
		work->animeCmd = animeCmd;
		work->anmSetFlag = TRUE;
	}
}

void	Set3DactPlayerChrAnimeForceCmd( SCENE_ACT* sceneAct, u32 idx, u8 animeCmd )
{
	ACT_WORK* work = (ACT_WORK*)GFL_G3D_SCENEOBJ_GetWork
						( GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene,sceneAct->actID + idx )); 
	work->animeCmd = animeCmd;
	work->anmSetFlag = TRUE;
}

void	Set3DactPlayerChrAccesory( SCENE_ACT* sceneAct, u32 idx, u8 accesoryID )
{
	ACT_WORK* work = (ACT_WORK*)GFL_G3D_SCENEOBJ_GetWork
						( GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene,sceneAct->actID + idx )); 
	work->accesoryID = accesoryID;
}

BOOL	Check3DactPlayerBusy( SCENE_ACT* sceneAct, u32 idx )
{
	ACT_WORK* work = (ACT_WORK*)GFL_G3D_SCENEOBJ_GetWork
						( GFL_G3D_SCENEOBJ_Get( sceneAct->g3Dscene,sceneAct->actID + idx )); 
	return work->busyFlag;
}

//------------------------------------------------------------------
enum {
	ACTANMSEQ_PLAYER_SET = 0,
	ACTANMSEQ_PLAYER_STAY,
	ACTANMSEQ_PLAYER_WALK,
	ACTANMSEQ_PLAYER_RUN,
	ACTANMSEQ_PLAYER_ATTACK,
	ACTANMSEQ_PLAYER_SHOOT,
	ACTANMSEQ_PLAYER_SPELL,
	ACTANMSEQ_PLAYER_SITDOWN,
	ACTANMSEQ_PLAYER_STANDUP,
	ACTANMSEQ_PLAYER_TAKE,
	ACTANMSEQ_PLAYER_HIT,
};

typedef struct {
	int animeID; 
	int animeSeq; 
	int start; 
	int end; 
	int wait; 
}PLAYER_ANMIDATA_TBL;

static const PLAYER_ANMIDATA_TBL anmDataTable[] = {
	{ HUMAN2_ANM_STAY,		ACTANMSEQ_PLAYER_STAY,		0,	0,	2 },	//ACTANM_PLAYER_STAY
	{ HUMAN2_ANM_WALK,		ACTANMSEQ_PLAYER_WALK,		0,	0,	2 },	//ACTANM_PLAYER_WALK
	{ HUMAN2_ANM_RUN,		ACTANMSEQ_PLAYER_RUN,		0,	0,	4 },	//ACTANM_PLAYER_RUN
	{ HUMAN2_ANM_ATTACK,	ACTANMSEQ_PLAYER_ATTACK,	0,	0,	3 },	//ACTANM_PLAYER_ATTACK
	{ HUMAN2_ANM_SHOOT,		ACTANMSEQ_PLAYER_SHOOT,		0,	0,	4 },	//ACTANM_PLAYER_SHOOT
	{ HUMAN2_ANM_SPELL,		ACTANMSEQ_PLAYER_SPELL,		0,	0,	2 },	//ACTANM_PLAYER_SPELL
	{ HUMAN2_ANM_SIT,		ACTANMSEQ_PLAYER_SITDOWN,	0,	20,	1 },	//ACTANM_PLAYER_SITDOWN
	{ HUMAN2_ANM_SIT,		ACTANMSEQ_PLAYER_STANDUP,	20,	0,	1 },	//ACTANM_PLAYER_STANDUP
	{ HUMAN2_ANM_SIT,		ACTANMSEQ_PLAYER_TAKE,		0,	0,	1 },	//ACTANM_PLAYER_TAKE
	{ HUMAN2_ANM_HIT,		ACTANMSEQ_PLAYER_HIT,		0,	0,	1 },	//ACTANM_PLAYER_HIT
};

static void movePlayer( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
	ACT_WORK*	pw = (ACT_WORK*)work;
	int			anmID;
	fx32		start, end, wait;

	if( pw->anmSetFlag == TRUE ){	//アニメ更新チェック
		anmID = anmDataTable[pw->animeCmdNow].animeID;
		GFL_G3D_SCENEOBJ_DisableAnime( sceneObj, anmID );

		pw->seq = anmDataTable[pw->animeCmd].animeSeq;
		anmID = anmDataTable[pw->animeCmd].animeID;

		GFL_G3D_SCENEOBJ_EnableAnime( sceneObj, anmID );
		start = FX32_ONE * anmDataTable[pw->animeCmd].start;
		GFL_G3D_SCENEOBJ_SetAnimeFrame( sceneObj, anmID, (int*)&start );

		pw->animeCmdNow = pw->animeCmd;
		pw->anmSetFlag = FALSE;
	}
	anmID	= anmDataTable[pw->animeCmdNow].animeID;
	end		= FX32_ONE * anmDataTable[pw->animeCmdNow].end;
	wait	= FX32_ONE * anmDataTable[pw->animeCmdNow].wait;

	switch( pw->seq ){
	case ACTANMSEQ_PLAYER_STAY:
	case ACTANMSEQ_PLAYER_WALK:
	case ACTANMSEQ_PLAYER_RUN:
		GFL_G3D_SCENEOBJ_LoopAnimeFrame( sceneObj, anmID, wait ); 
		pw->busyFlag = FALSE;
		break;
	case ACTANMSEQ_PLAYER_ATTACK:
	case ACTANMSEQ_PLAYER_SHOOT:
	case ACTANMSEQ_PLAYER_SPELL:
	case ACTANMSEQ_PLAYER_TAKE:
	case ACTANMSEQ_PLAYER_STANDUP:
		if( GFL_G3D_SCENEOBJ_IncAnimeFrame( sceneObj, anmID, wait ) == FALSE ){
			pw->animeCmd = ACTANM_CMD_STAY;
			pw->anmSetFlag = TRUE;
		} else {
			pw->busyFlag = TRUE;
		}
		break;
	case ACTANMSEQ_PLAYER_HIT:
		if( GFL_G3D_SCENEOBJ_IncAnimeFrame( sceneObj, anmID, wait ) == FALSE ){
			pw->animeCmd = ACTANM_CMD_STAY;
			pw->anmSetFlag = TRUE;
		} else {
			pw->busyFlag = TRUE;
		}
		break;
	case ACTANMSEQ_PLAYER_SITDOWN:
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


