//============================================================================================
/**
 * @file	billboard_act.c
 * @brief	ビルボードアクトシステム
 */
//============================================================================================
#include "gflib.h"

//注）GFL_G3D_SYSTEM, GFL_BBD上での動作
//------------------------------------------------------------------
/**
 *
 * @brief	型宣言
 *
 */
//------------------------------------------------------------------
#define RES_NULL		(0xffff)
#define ACT_NULL		(0xffff)

typedef struct {
	int					billboardIdx;		//対応するビルボード配列ＩＮＤＥＸ
	u16					resIdx;				//使用しているリソースＩＮＤＥＸ
	GFL_BBDACT_ANMTBL	animeTable;			//アニメーションデータテーブル
	u16					animeTableCount;	//アニメーションデータ数
	u16					animeIdx;			//アニメーションＩＮＤＥＸ
	u16					animeFrmIdx;		//アニメーションフレームＩＮＤＥＸ
	u16					animeWait;			//アニメーションウェイト
	u16					animeLpCnt;			//アニメーションループカウント
	BOOL				animeEnable;		//アニメーション許可フラグ
	GFL_BBDACT_FUNC*	func;				//動作関数
	void*				work;				//ワークポインタ
}BBD_ACT;

struct _GFL_BBDACT_SYS {
	HEAPID			heapID;
	GFL_BBD_SYS*	bbdSys;
	u16*			bbdRes;
	u16				bbdResMax;
	GFL_AREAMAN*	bbdResAreaman;
	BBD_ACT*		bbdAct;
	u16				bbdActMax;
	GFL_AREAMAN*	bbdActAreaman;
};

static void	GFL_BBDACT_InitAct( BBD_ACT* bbdAct );
//------------------------------------------------------------------
/**
 *
 * @brief	セットアップデータ
 *
 */
//------------------------------------------------------------------

//------------------------------------------------------------------
/**
 * @brief	ビルボードアクトシステム作成
 */
//------------------------------------------------------------------
GFL_BBDACT_SYS*	GFL_BBDACT_CreateSys( const u16 bbdResMax, const u16 bbdActMax, HEAPID heapID )
{
	GFL_BBDACT_SYS* bbdActSys = GFL_HEAP_AllocClearMemory( heapID, sizeof(GFL_BBDACT_SYS) );
	int	i;

	bbdActSys->heapID = heapID;
	bbdActSys->bbdResMax = bbdResMax;
	bbdActSys->bbdActMax = bbdActMax;
	{
		//システムセットアップ
		GFL_BBD_SETUP billboardSetup = {	0, 0, 
											{ FX32_ONE, FX32_ONE, FX32_ONE },	//scale
											GX_RGB(31, 31, 31),					//diffuse
											GX_RGB(16, 16, 16),					//ambient
											GX_RGB(16, 16, 16),					//specular
											GX_RGB(0, 0, 0),					//emission
											63,									//polID
										};
		billboardSetup.resCountMax = bbdActSys->bbdResMax;
		billboardSetup.objCountMax = bbdActSys->bbdActMax;

		bbdActSys->bbdSys = GFL_BBD_CreateSys( &billboardSetup, heapID );
	}

	bbdActSys->bbdRes = GFL_HEAP_AllocClearMemory( heapID, sizeof(u16)*bbdActSys->bbdResMax );
	bbdActSys->bbdResAreaman = GFL_AREAMAN_Create( bbdActSys->bbdResMax, heapID );

	bbdActSys->bbdAct = GFL_HEAP_AllocClearMemory( heapID, sizeof(BBD_ACT)*bbdActSys->bbdActMax );
	bbdActSys->bbdActAreaman = GFL_AREAMAN_Create( bbdActSys->bbdActMax, heapID );

	for( i=0; i<bbdActSys->bbdResMax; i++ ){
		bbdActSys->bbdRes[i] = RES_NULL;
	}
	for( i=0; i<bbdActSys->bbdActMax; i++ ){
		GFL_BBDACT_InitAct( &bbdActSys->bbdAct[i] );
	}
	return bbdActSys;
}

//------------------------------------------------------------------
/**
 * @brief	ビルボードアクトシステム破棄
 */
//------------------------------------------------------------------
void	GFL_BBDACT_DeleteSys( GFL_BBDACT_SYS* bbdActSys )
{
	int i;

	for( i=0; i<bbdActSys->bbdActMax; i++ ){	//残りbbdAct全Delete
		if( bbdActSys->bbdAct[i].billboardIdx != ACT_NULL ){
			GFL_BBD_RemoveObject( bbdActSys->bbdSys, bbdActSys->bbdAct[i].billboardIdx );
		}
	}
	GFL_AREAMAN_Delete( bbdActSys->bbdActAreaman );
	GFL_HEAP_FreeMemory( bbdActSys->bbdAct );

	GFL_AREAMAN_Delete( bbdActSys->bbdResAreaman );
	GFL_HEAP_FreeMemory( bbdActSys->bbdRes );

	GFL_BBD_DeleteSys( bbdActSys->bbdSys );

	GFL_HEAP_FreeMemory( bbdActSys );
}

//------------------------------------------------------------------
/**
 * @brief	ビルボードシステムポインタ取得
 */
//------------------------------------------------------------------
GFL_BBD_SYS*	GFL_BBDACT_GetBBDSystem( GFL_BBDACT_SYS* bbdActSys )
{
	return bbdActSys->bbdSys;
}

//------------------------------------------------------------------
/**
 * @brief	ビルボードアクト初期化
 */
//------------------------------------------------------------------
static void	GFL_BBDACT_InitActAnm( BBD_ACT* bbdAct )
{
	bbdAct->animeFrmIdx		= 0;
	bbdAct->animeWait		= 0;
	bbdAct->animeLpCnt		= 0;
}

static void	GFL_BBDACT_InitAct( BBD_ACT* bbdAct )
{
	bbdAct->billboardIdx	= ACT_NULL;
	bbdAct->animeTable		= NULL;
	bbdAct->animeTableCount	= 0;
	bbdAct->animeIdx		= 0;
	GFL_BBDACT_InitActAnm( bbdAct );
	bbdAct->animeEnable		= FALSE;
	bbdAct->func			= NULL;
	bbdAct->work			= NULL;
}

//------------------------------------------------------------------
/**
 * @brief	ビルボードアクトリソースセットアップ
 */
//------------------------------------------------------------------
GFL_BBDACT_RESUNIT_ID GFL_BBDACT_AddResourceUnit
			( GFL_BBDACT_SYS* bbdActSys, GFL_BBDACT_RES_SETTBL resTbl, u32 resCount )
{
	const GFL_BBDACT_RESDATA* resData;
	u32 idx;
	int i;

	//配置ＩＮＤＥＸ確保
	idx = GFL_AREAMAN_ReserveAuto( bbdActSys->bbdResAreaman, resCount );

	GF_ASSERT( idx != AREAMAN_POS_NOTFOUND );

	for( i=0; i<resCount; i++ ){
		resData = &resTbl[i];
		bbdActSys->bbdRes[idx+i] = GFL_BBD_AddResourceArc(	bbdActSys->bbdSys, 
															resData->arcID, resData->datID,
															resData->texFmt, resData->texSiz,
															resData->celSizX, resData->celSizY );
	}
	return (GFL_BBDACT_RESUNIT_ID)idx;
}

//------------------------------------------------------------------
/**
 * @brief	ビルボードアクトリソース破棄
 */
//------------------------------------------------------------------
void GFL_BBDACT_RemoveResourceUnit
			( GFL_BBDACT_SYS* bbdActSys, GFL_BBDACT_RESUNIT_ID idx, u32 resCount ) 
{
	int i;

	for( i=0; i<resCount; i++ ){
		GFL_BBD_RemoveResource( bbdActSys->bbdSys, bbdActSys->bbdRes[idx+i] );
		bbdActSys->bbdRes[idx+i] = RES_NULL;
	}
	GFL_AREAMAN_Release( bbdActSys->bbdResAreaman, idx, resCount );
}

//------------------------------------------------------------------
/**
 * @brief	ビルボードアクトセットアップ
 */
//------------------------------------------------------------------
GFL_BBDACT_ACTUNIT_ID GFL_BBDACT_AddAct( GFL_BBDACT_SYS* bbdActSys, 
				GFL_BBDACT_RESUNIT_ID resUnitID, GFL_BBDACT_ACT_SETTBL actTbl, u32 actCount )
{
	const GFL_BBDACT_ACTDATA* actData;
	BBD_ACT* bbdAct;
	u32 idx;
	int i, objIdx, resIdx;

	//配置ＩＮＤＥＸ確保
	idx = GFL_AREAMAN_ReserveAuto( bbdActSys->bbdActAreaman, actCount );

	GF_ASSERT( idx != AREAMAN_POS_NOTFOUND );

	for( i=0; i<actCount; i++ ){
		bbdAct = &bbdActSys->bbdAct[idx+i];

		actData = &actTbl[i];
		GFL_BBDACT_InitAct( bbdAct );
		resIdx = bbdActSys->bbdRes[resUnitID+actData->resID];
		objIdx = GFL_BBD_AddObject(	bbdActSys->bbdSys, resIdx, 
									actData->sizX, actData->sizY, 
									&actData->trans, actData->alpha ); 
		bbdAct->billboardIdx = objIdx;
		bbdAct->resIdx = resIdx;
		bbdAct->func = actData->func;
		bbdAct->work = actData->work;
		GFL_BBD_SetObjectDrawSw( bbdActSys->bbdSys, objIdx, &actData->drawEnable );
	}
	return (GFL_BBDACT_ACTUNIT_ID)idx;
}

//------------------------------------------------------------------
/**
 * @brief	ビルボードアクト破棄
 */
//------------------------------------------------------------------
void GFL_BBDACT_RemoveAct( GFL_BBDACT_SYS* bbdActSys, GFL_BBDACT_ACTUNIT_ID idx, u32 actCount ) 
{
	int i;

	for( i=0; i<actCount; i++ ){
		GFL_BBD_RemoveObject( bbdActSys->bbdSys, bbdActSys->bbdAct[idx+i].billboardIdx );
		GFL_BBDACT_InitAct( &bbdActSys->bbdAct[idx+i] );
	}
	GFL_AREAMAN_Release( bbdActSys->bbdActAreaman, idx, actCount );
}

//------------------------------------------------------------------
/**
 *
 * @brief	ビルボードアクト動作描画
 *
 */
static void animeControl( GFL_BBDACT_SYS* bbdActSys, BBD_ACT* bbdAct );
//------------------------------------------------------------------
void	GFL_BBDACT_Main( GFL_BBDACT_SYS* bbdActSys )
{
	BBD_ACT* bbdAct;
	int i;

	for( i=0; i<bbdActSys->bbdActMax; i++ ){
		bbdAct = &bbdActSys->bbdAct[i];
		if( bbdAct->billboardIdx != ACT_NULL ){
			if( bbdAct->func != NULL ){
				bbdAct->func( bbdActSys, i, bbdAct->work );
			}
			animeControl( bbdActSys, bbdAct );
		}
	}
}
	
//アニメーションコントロール
static void animeControl( GFL_BBDACT_SYS* bbdActSys, BBD_ACT* bbdAct )
{
	const GFL_BBDACT_ANM* actAnm;

	if(( bbdAct->animeTable == NULL )||( bbdAct->animeEnable == FALSE )) {	//アニメなし or Stop
		return;
	}
	if( bbdAct->animeWait ){
		bbdAct->animeWait--;
		return;
	}
	actAnm = &bbdAct->animeTable[bbdAct->animeIdx][bbdAct->animeFrmIdx];

	if( actAnm->comData.command == GFL_BBDACT_ANMCOM_END ){
		return;
	}
	bbdAct->animeFrmIdx++;

	switch( actAnm->comData.command ){
		case GFL_BBDACT_ANMCOM_END:
			return;
		case GFL_BBDACT_ANMCOM_CHG:
			{
				u16 animeIdx = actAnm->comData.param;

				GF_ASSERT( animeIdx < bbdAct->animeTableCount );
				bbdAct->animeIdx = animeIdx;
				GFL_BBDACT_InitActAnm( bbdAct );
			}
			return;
		case GFL_BBDACT_ANMCOM_LOOP:
			{
				//上位8bitが回数、下位8bitがループ先頭frmIdx
				u8	lpCnt = ( actAnm->comData.param & 0xff00 )>>8;
				u16 frmIdx = actAnm->comData.param & 0x00ff;

				if( bbdAct->animeLpCnt < lpCnt ){
					bbdAct->animeFrmIdx = frmIdx;
					bbdAct->animeWait = 0;
					bbdAct->animeLpCnt++;
				} else {
					bbdAct->animeLpCnt = 0;
				}
			}
			return;
		case GFL_BBDACT_ANMCOM_JMP:
			bbdAct->animeFrmIdx = actAnm->comData.param;
			bbdAct->animeWait = 0;
			return;
	}
	{
		u16 celIdx = actAnm->anmData.celIdx;
		u16 wait = actAnm->anmData.wait;
		BOOL flipS, flipT;

		GFL_BBD_SetObjectCelIdx( bbdActSys->bbdSys, bbdAct->billboardIdx, &celIdx );
		if(actAnm->anmData.flipS){
			flipS = TRUE;
		} else {
			flipS = FALSE;
		}
		if(actAnm->anmData.flipT){
			flipT = TRUE;
		} else {
			flipT = FALSE;
		}
		GFL_BBD_SetObjectFlipS( bbdActSys->bbdSys, bbdAct->billboardIdx, &flipS );
		GFL_BBD_SetObjectFlipT( bbdActSys->bbdSys, bbdAct->billboardIdx, &flipT );

		if(wait){
			wait--;
		}
		bbdAct->animeWait = wait;
	}
}
	
//------------------------------------------------------------------
/**
 *
 * @brief	ビルボードアクト描画
 *
 */
//------------------------------------------------------------------
void	GFL_BBDACT_Draw( GFL_BBDACT_SYS* bbdActSys, GFL_G3D_CAMERA* g3Dcamera )
{
	GFL_BBD_Draw( bbdActSys->bbdSys, g3Dcamera );
}

//------------------------------------------------------------------
/**
 *
 * @brief	ビルボードアクト各種パラメータの取得と変更
 *
 */
//------------------------------------------------------------------
//	描画スイッチ設定
void	GFL_BBDACT_SetDrawSw( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, BOOL drawEnable )
{
	GF_ASSERT( actIdx < bbdActSys->bbdActMax );
	GF_ASSERT( bbdActSys->bbdAct[actIdx].billboardIdx != ACT_NULL );

	GFL_BBD_SetObjectDrawSw(	bbdActSys->bbdSys, 
								bbdActSys->bbdAct[actIdx].billboardIdx, &drawEnable );
}

//	アニメ設定
void	GFL_BBDACT_SetAnimeEnable( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, BOOL animeEnable )
{
	BBD_ACT* bbdAct;
	GF_ASSERT( actIdx < bbdActSys->bbdActMax );
	bbdAct = &bbdActSys->bbdAct[actIdx];
	GF_ASSERT( bbdAct->billboardIdx != ACT_NULL );

	bbdAct->animeEnable	= animeEnable;
}

void	GFL_BBDACT_SetAnimeTable( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, 
									GFL_BBDACT_ANMTBL animeTable, u16 animeTableCount )
{
	BBD_ACT* bbdAct;
	GF_ASSERT( actIdx < bbdActSys->bbdActMax );
	bbdAct = &bbdActSys->bbdAct[actIdx];
	GF_ASSERT( bbdAct->billboardIdx != ACT_NULL );

	bbdAct->animeTable		= animeTable;
	bbdAct->animeTableCount	= animeTableCount;
	bbdAct->animeIdx		= 0;
	GFL_BBDACT_InitActAnm( bbdAct );
}

void	GFL_BBDACT_SetAnimeIdx( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, u16 animeIdx )
{
	BBD_ACT* bbdAct;
	GF_ASSERT( actIdx < bbdActSys->bbdActMax );
	bbdAct = &bbdActSys->bbdAct[actIdx];
	GF_ASSERT( bbdAct->billboardIdx != ACT_NULL );

	GF_ASSERT( animeIdx < bbdAct->animeTableCount );
	bbdAct->animeIdx		= animeIdx;
	GFL_BBDACT_InitActAnm( bbdAct );
}

void	GFL_BBDACT_SetAnimeIdxContinue( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, u16 animeIdx )
{
	BBD_ACT* bbdAct;
	GF_ASSERT( actIdx < bbdActSys->bbdActMax );
	bbdAct = &bbdActSys->bbdAct[actIdx];
	GF_ASSERT( bbdAct->billboardIdx != ACT_NULL );

	GF_ASSERT( animeIdx < bbdAct->animeTableCount );
	//フレーム継続なのでオーバーフローに注意
	bbdAct->animeIdx		= animeIdx;
}

void	GFL_BBDACT_SetAnimeFrmIdx( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, u16 animeFrmIdx )
{
	BBD_ACT* bbdAct;
	GF_ASSERT( actIdx < bbdActSys->bbdActMax );
	bbdAct = &bbdActSys->bbdAct[actIdx];
	GF_ASSERT( bbdAct->billboardIdx != ACT_NULL );

	//エラー検出がないのでテーブルオーバーフローに注意
	GFL_BBDACT_InitActAnm( bbdAct );
	bbdAct->animeFrmIdx = animeFrmIdx;
}

//	動作関数設定
void	GFL_BBDACT_SetFunc( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, GFL_BBDACT_FUNC* func )
{
	BBD_ACT* bbdAct;
	GF_ASSERT( actIdx < bbdActSys->bbdActMax );
	bbdAct = &bbdActSys->bbdAct[actIdx];
	GF_ASSERT( bbdAct->billboardIdx != ACT_NULL );

	bbdAct->func = func;
}

