//======================================================================
/**
 * @file	fldmmdl_blact.c
 * @brief	動作モデル ビルボードアクター管理
 * @author	kagaya
 * @data	05.07.13
 */
//======================================================================
#include "fldmmdl.h"
#include "test_graphic/fld_act.naix"

#ifdef MMDL_BLACT_HEAD3_TEST
#include "test_graphic/fldmmdl_btx.naix"
#endif

//======================================================================
//	define
//======================================================================
#define REGIDCODE_MAX (0xffff) ///<BBDRESID最大
#define BLACT_RESID_NULL (0xffff) ///<ビルボードリソース 無効ID

typedef enum
{
  BBDRESBIT_GUEST = (1<<0), ///<リソース識別 ゲスト登録 OFF時=レギュラー
  BBDRESBIT_TRANS = (1<<1), ///<リソース識別 ON=VRAM転送用リソース
  
  ///リソース　VRAM常駐　レギュラー
  BBDRES_VRAM_REGULAR =(0),
  ///リソース　VRAM転送　レギュラー
  BBDRES_TRANS_REGULAR =(BBDRESBIT_TRANS),
  ///リソース　VRAM常駐　ゲスト
  BBDRES_VRAM_GUEST=(BBDRESBIT_GUEST),
  ///リソース　VRAM転送　ゲスト
  BBDRES_TRANS_GUEST=(BBDRESBIT_TRANS|BBDRESBIT_GUEST),
}BBDRESBIT;

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
///	IDCODE
//--------------------------------------------------------------
typedef struct
{
	u16 code; ///<登録コード
	u16 id; ///<登録コードから返すID
  u16 flag; ///<登録内容を表すフラグ
  u16 dmy;
}IDCODE;

//--------------------------------------------------------------
///	IDCODEIDX
//--------------------------------------------------------------
typedef struct
{
	int max;
	IDCODE *pIDCodeBuf;
}IDCODEIDX;

//--------------------------------------------------------------
/// ADDRES_RESERVE
//--------------------------------------------------------------
typedef struct
{
  BOOL compFlag; //データの設定が完了したフラグ
  
  u16 code; //登録用コード
  BBDRESBIT flag; //登録用フラグ
  GFL_G3D_RES *pG3dRes; //登録リソース
}ADDRES_RESERVE;

//--------------------------------------------------------------
/// ADDACT_RESERVE
//--------------------------------------------------------------
typedef struct
{
  BOOL compFlag; //データの設定が完了したフラグ
  
  MMDL *fmmdl;
  u16 code;
  u16 dummy;
  u16 *outID;
  GFL_G3D_RES *pTransActRes; ///<転送用アクターリソース
}ADDACT_RESERVE;

//--------------------------------------------------------------
/// BLACT_RESERVE
//--------------------------------------------------------------
typedef struct
{
  BOOL funcFlag; //TRUE=予約処理を実行する

  u16 resMax;
  u16 actMax;
  ADDRES_RESERVE *pReserveRes;
  ADDACT_RESERVE *pReserveAct;
}BLACT_RESERVE;

//--------------------------------------------------------------
///	MMDL_BLACTCONT
//--------------------------------------------------------------
struct _TAG_MMDL_BLACTCONT
{
  int resourceMax;

	GFL_BBDACT_SYS *pBbdActSys; //ユーザーから
	GFL_BBDACT_RESUNIT_ID bbdActResUnitID;
	GFL_BBDACT_ACTUNIT_ID bbdActActUnitID;
	u16 bbdActResCount;
	u16 bbdActActCount;
  
  ARCHANDLE *arcH_res;
  
	MMDLSYS *fmmdlsys;
  
	IDCODEIDX BBDResUnitIdx;
  BLACT_RESERVE *pReserve;
};

//======================================================================
//	proto
//======================================================================
static void BlActFunc( GFL_BBDACT_SYS *bbdActSys, int actIdx, void *work );

static void IDCodeIndex_Init( IDCODEIDX *idx, int max, HEAPID heapID );
static void IDCodeIndex_RegistCode(
    IDCODEIDX *idx, u16 code, u16 id, u16 flag );
static void IDCodeIndex_DeleteCode( IDCODEIDX *idx, u16 code );
static BOOL IDCodeIndex_SearchCode(
    IDCODEIDX *idx, u16 code, u16 *outID, u16 *outFlag );
static void BBDResUnitIndex_Init( MMDL_BLACTCONT *pBlActCont, int max );
static void BBDResUnitIndex_Delete( MMDL_BLACTCONT *pBlActCont );
static void BBDResUnitIndex_AddResource( MMDL_BLACTCONT *pBlActCont,
    GFL_G3D_RES *g3dres, u16 obj_code, BBDRESBIT flag );
static void BBDResUnitIndex_AddResUnit(
    MMDL_BLACTCONT *pBlActCont, u16 obj_code, BBDRESBIT flag );
static void BBDResUnitIndex_RemoveResUnit(
    MMDL_BLACTCONT *pBlActCont, u16 obj_code );
static BOOL BBDResUnitIndex_SearchResID(
  MMDL_BLACTCONT *pBlActCont, u16 obj_code, u16 *outID, u16 *outFlag );

static void BlActAddReserve_Init( MMDL_BLACTCONT *pBlActCont );
static void BlActAddReserve_Delete( MMDL_BLACTCONT *pBlActCont );
static void BlActAddReserve_Digest( MMDL_BLACTCONT *pBlActCont );
static void BlActAddReserve_RegistResource(
    MMDL_BLACTCONT *pBlActCont, u16 code, BBDRESBIT flag );
static void BlActAddReserve_DigestResource( MMDL_BLACTCONT *pBlActCont );
static BOOL BlActAddReserve_SearchResource(
    MMDL_BLACTCONT *pBlActCont, u16 code, u16 *outFlag );
static BOOL BlActAddReserve_CancelResource(
    MMDL_BLACTCONT *pBlActCont, u16 code );
static void BlActAddReserve_RegistActor(
    MMDL_BLACTCONT *pBlActCont, MMDL *fmmdl, u16 code, u16 *outID );
static void BlActAddReserve_DigestActor( MMDL_BLACTCONT *pBlActCont );
static BOOL BlActAddReserve_SearchActorOBJCode(
    MMDL_BLACTCONT *pBlActCont, u16 code, MMDL *fmmdl );
static void BlActAddReserve_CancelActor(
    MMDL_BLACTCONT *pBlActCont, MMDL *fmmdl );

static const MMDL_BBDACT_ANMTBL * BlActAnm_GetAnmTbl( u32 no );
static GFL_BBDACT_RESUNIT_ID BlActRes_AddRes(
    MMDL_BLACTCONT *pBlActCont, u16 code,
    GFL_G3D_RES *g3dres, GFL_BBDACT_RESTYPE type );

//test data
/*
const GFL_BBDACT_RESDATA testResTable[];
const u32 testResTableMax;
*/

//======================================================================
//	フィールド動作モデル　ビルボードアクター管理
//======================================================================
//--------------------------------------------------------------
/**
 * MMDL_BLACTCONT ビルボードアクター管理　セットアップ
 * @param	fmmdlsys	構築済みのMMDLSYS
 * @param	pBbdActSys	構築済みのGFL_BBDACT_SYS
 * @param res_max リソース数最大
 * @retval	nothing
 */
//--------------------------------------------------------------
void MMDL_BLACTCONT_Setup( MMDLSYS *fmmdlsys,
	GFL_BBDACT_SYS *pBbdActSys, int res_max )
{
	HEAPID heapID;
	GFL_BBDACT_ACTDATA *actData;
	GFL_BBDACT_ACTUNIT_ID actUnitID;
	MMDL_BLACTCONT *pBlActCont;
	
	heapID = MMDLSYS_GetHeapID( fmmdlsys );
	pBlActCont = GFL_HEAP_AllocClearMemory( heapID, sizeof(MMDL_BLACTCONT) );
	pBlActCont->fmmdlsys = fmmdlsys;
	pBlActCont->pBbdActSys = pBbdActSys;
  pBlActCont->arcH_res = GFL_ARC_OpenDataHandle( ARCID_MMDL_RES, heapID );
  pBlActCont->resourceMax = res_max;
  
	MMDLSYS_SetBlActCont( fmmdlsys, pBlActCont );
	
	BBDResUnitIndex_Init( pBlActCont, res_max );
  BlActAddReserve_Init( pBlActCont );
}

//--------------------------------------------------------------
/**
 * MMDL_BLACTCONT ビルボードアクター管理　削除
 * @param	fmmdlsys	MMDLSYS
 * @retval	nothing
 */
//--------------------------------------------------------------
void MMDL_BLACTCONT_Release( MMDLSYS *fmmdlsys )
{
	MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( fmmdlsys );
  BlActAddReserve_Delete( pBlActCont );
	BBDResUnitIndex_Delete( pBlActCont );
  
  GFL_ARC_CloseDataHandle( pBlActCont->arcH_res );
	
	GFL_HEAP_FreeMemory( pBlActCont );
	MMDLSYS_SetBlActCont( fmmdlsys, NULL );
}

//--------------------------------------------------------------
/**
 * MMDL_BLACTCONT Vブランク処理
 * @param	fmmdlsys	MMDLSYS
 * @retval	nothing
 */
//--------------------------------------------------------------
void MMDL_BLACTCONT_ProcVBlank( MMDLSYS *fmmdlsys )
{
	MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( fmmdlsys );
  BlActAddReserve_Digest( pBlActCont );
}

//======================================================================
//	フィールド動作モデル　ビルボードリソース
//======================================================================
//--------------------------------------------------------------
/**
 * MMDL_BLACTCONT リソース追加 レギュラー、VRAM常駐型で登録
 * @param	fmmdlsys	MMDLSYS
 * @param	code 表示コード配列
 * @param	max code要素数
 * @retval	nothing
 * @note 呼ばれたその場で読み込みが発生する。
 */
//--------------------------------------------------------------
void MMDL_BLACTCONT_AddResourceTex(
	MMDLSYS *fmmdlsys, const u16 *code, int max )
{
  MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( fmmdlsys );
	while( max ){
		BBDResUnitIndex_AddResUnit( pBlActCont, *code, BBDRES_VRAM_REGULAR );
		code++;
		max--;
	}
}

//======================================================================
//	フィールド動作モデル ビルボードアクター追加
//======================================================================
//--------------------------------------------------------------
/**
 * ビルボードアクター アクター追加
 * @param	fmmdl	MMDL
 * @param	resID 追加するアクターで使用するリソースインデックス
 * @retval GFL_BBDACT_ACTUNIT_ID
 */
//--------------------------------------------------------------
static GFL_BBDACT_ACTUNIT_ID blact_AddActor(
    MMDL *fmmdl, u16 code, u16 resID )
{
	VecFx32 pos;
	GFL_BBDACT_ACTDATA actData;
	GFL_BBDACT_ACTUNIT_ID actID;
  
  MMDLSYS *fmmdlsys =
    (MMDLSYS*)MMDL_GetMMdlSys( fmmdl );
	MMDL_BLACTCONT *pBlActCont =
    MMDLSYS_GetBlActCont( fmmdlsys );
	
	MMDL_GetDrawVectorPos( fmmdl, &pos );
	
  actData.resID = resID;
	actData.sizX = FX16_ONE*8-1;
	actData.sizY = FX16_ONE*8-1;
	actData.trans = pos;
	actData.alpha = 31;
	actData.drawEnable = TRUE;
	actData.lightMask = GFL_BBD_LIGHTMASK_01;
	actData.work = fmmdl;
	actData.func = BlActFunc;
	
	actID = GFL_BBDACT_AddAct(
		pBlActCont->pBbdActSys, pBlActCont->bbdActResUnitID, &actData, 1 );
  
  {
	  const OBJCODE_PARAM *prm =
      MMDLSYS_GetOBJCodeParam( fmmdlsys, code );
	  const MMDL_BBDACT_ANMTBL *anmTbl =
      BlActAnm_GetAnmTbl( prm->anm_id );
    
	  if( anmTbl->pAnmTbl != NULL ){
		  GFL_BBDACT_SetAnimeTable( pBlActCont->pBbdActSys,
        actID, (GFL_BBDACT_ANMTBL)anmTbl->pAnmTbl, anmTbl->anm_max );
    }
     
	  GFL_BBDACT_SetAnimeIdxOn( pBlActCont->pBbdActSys, actID, 0 );
	}
	
	return( actID );
}

//--------------------------------------------------------------
/**
 * ビルボードアクター　リソースからアクターセット
 * @param fmmdl MMDL
 * @param code 表示コード
 * @param resID アクターリソースID
 * @param transResID 転送用リソース
 * @retval
 */
//--------------------------------------------------------------
static GFL_BBDACT_ACTUNIT_ID blact_SetResActor(
    MMDL *fmmdl, u16 code, u16 resID, u16 transResID )
{
	GFL_BBDACT_ACTUNIT_ID actID;
	MMDLSYS *fmmdlsys = (MMDLSYS*)MMDL_GetMMdlSys( fmmdl );
	MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( fmmdlsys );
	
  actID = blact_AddActor( fmmdl, code, resID );
  
  if( transResID != BLACT_RESID_NULL ){ //転送用リソースと結ぶ
    GFL_BBDACT_BindActTexRes( pBlActCont->pBbdActSys, actID, transResID );
  }
   
	return( actID );
}

#if 0
//--------------------------------------------------------------
/**
 * ビルボードアクター　転送型アクターセット
 * @param fmmdl MMDL
 * @param code 表示コード
 * @param transResID 転送用リソースID
 * @param pTransActRes 転送用ビルボードリソースID
 * @retval GFL_BBDACT_ACTUNIT_ID
 */
//--------------------------------------------------------------
static GFL_BBDACT_ACTUNIT_ID blact_SetTransActor(
    MMDL *fmmdl, u16 code, u16 transResID, GFL_G3D_RES pTransActRes )
{
  u16 resID;
	GFL_BBDACT_ACTUNIT_ID actID;
	MMDLSYS *fmmdlsys = (MMDLSYS*)MMDL_GetMMdlSys( fmmdl );
	MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( fmmdlsys );
	
  {
  	const OBJCODE_PARAM *prm;
  	GFL_BBDACT_G3DRESDATA data;
	  prm = MMDLSYS_GetOBJCodeParam( pBlActCont->fmmdlsys, obj_code );
    
    data.g3dres = g3dres;
	  data.texFmt = GFL_BBD_TEXFMT_PAL16;
	  data.texSiz = prm->tex_size;
	  data.celSizX = 32;				//いずれmdl_sizeから
	  data.celSizY = 32;
  }

    actID = blact_AddActor( fmmdl, code, resID );
  
  GFL_BBDACT_ACTUNIT_ID blact_SetResActor(
    MMDL *fmmdl, u16 code, u16 resID, resID );
  u16 transResID )
  if( transResID != BLACT_RESID_NULL ){ //転送用リソースと結ぶ
    GFL_BBDACT_BindActTexRes( pBlActCont->pBbdActSys, actID, transResID );
  }
   
	return( actID );
}
#endif

//--------------------------------------------------------------
/**
 * ビルボードアクター　アクターセット
 * @param fmmdl MMDL
 * @param code 表示コード
 * @retval nothing
 */
//--------------------------------------------------------------
#if 0
static GFL_BBDACT_ACTUNIT_ID blact_SetActor( MMDL *fmmdl, u16 code )
{
  u16 resID,flag;
	GFL_BBDACT_ACTUNIT_ID actID;
	MMDLSYS *fmmdlsys = (MMDLSYS*)MMDL_GetMMdlSys( fmmdl );
	MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( fmmdlsys );
	
	BBDResUnitIndex_SearchResID( pBlActCont, code, &resID, &flag );
	
  if( resID == REGIDCODE_MAX ){ //非登録リソース
    MMDL_BLACTCONT_AddOBJCodeRes( fmmdlsys, code,  FALSE, TRUE );
	  BBDResUnitIndex_SearchResID( pBlActCont, code, &resID, &flag );
    GF_ASSERT( resID != REGIDCODE_MAX );
    
    #ifdef DEBUG_MMDL
    KAGAYA_Printf( "MMDL ADD GUEST RESOURCE %xH\n", code );
    #endif
  }
  
  if( (flag&BBDRESBIT_TRANS) ){ //転送型 転送先リソース作成
    //転送用リソースを取得し追加。
    //resID = アクター用のリソース
    GF_ASSERT( 0 ); //現状未対応
  }
  
  actID = blact_AddActor( fmmdl, code, resID );
  
  if( (flag&BBDRESBIT_TRANS) ){ //転送型 リソースと結ぶ
	  BBDResUnitIndex_SearchResID( pBlActCont, code, &resID, &flag );
    GFL_BBDACT_BindActTexRes( pBlActCont->pBbdActSys, actID, resID );
  }
  
	return( actID );
}
#endif

static GFL_BBDACT_ACTUNIT_ID blact_SetActor( MMDL *fmmdl, u16 code )
{
  u16 resID,flag;
	GFL_BBDACT_ACTUNIT_ID actID;
	MMDLSYS *fmmdlsys = (MMDLSYS*)MMDL_GetMMdlSys( fmmdl );
	MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( fmmdlsys );
	
	BBDResUnitIndex_SearchResID( pBlActCont, code, &resID, &flag );
	GF_ASSERT( resID != REGIDCODE_MAX ); //リソース無し
  GF_ASSERT( !(flag&BBDRESBIT_TRANS) ); //転送タイプ
  
  actID = blact_AddActor( fmmdl, code, resID );
	return( actID );
}

#if 0
//--------------------------------------------------------------
/**
 * ビルボードアクター 追加
 * @param	fmmdl	MMDL
 * @param	code 使用するOBJコード
 * @param outID 追加アクターID格納先 MMDL_BBDACT_ACTID_NULL=追加中
 * @retval BOOL TRUE=追加。FALSE=追加中。
 */
//--------------------------------------------------------------
GFL_BBDACT_ACTUNIT_ID MMDL_BLACTCONT_AddActor( MMDL *fmmdl, u32 code )
{
  u16 flag;
	VecFx32 pos;
	GFL_BBDACT_ACTDATA actData;
	GFL_BBDACT_ACTUNIT_ID actID;
	const MMDL_BBDACT_ANMTBL *anmTbl;
	const OBJCODE_PARAM *prm;
	MMDLSYS *fmmdlsys = (MMDLSYS*)MMDL_GetMMdlSys( fmmdl );
	MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( fmmdlsys );
	
	prm = MMDLSYS_GetOBJCodeParam( fmmdlsys, code );
	MMDL_GetDrawVectorPos( fmmdl, &pos );
	
	BBDResUnitIndex_SearchResID( fmmdlsys, code, &actData.resID, &flag );
	
  if( actData.resID == REGIDCODE_MAX ){ //非登録リソース
    MMDL_BLACTCONT_AddOBJCodeRes( fmmdlsys, code,  FALSE, TRUE );
	  BBDResUnitIndex_SearchResID( fmmdlsys, code, &actData.resID, &flag );
    GF_ASSERT( actData.resID != REGIDCODE_MAX );
    
    #ifdef DEBUG_MMDL
    KAGAYA_Printf( "MMDL ADD GUEST RESOURCE %xH\n", code );
    #endif
  }
  
	actData.sizX = FX16_ONE*8-1;
	actData.sizY = FX16_ONE*8-1;
	actData.trans = pos;
	actData.alpha = 31;
	actData.drawEnable = TRUE;
	actData.lightMask = GFL_BBD_LIGHTMASK_01;
	actData.work = fmmdl;
	actData.func = BlActFunc;
	
	actID = GFL_BBDACT_AddAct(
		pBlActCont->pBbdActSys, pBlActCont->bbdActResUnitID, &actData, 1 );
  
#if 0 //転送用リソースとバインド
  if( flag & == HERO ){
//    GFL_BBDACT_BindActTexRes( pBlActCont->pBbdActSys, actID, actData.resID );
  }
#endif

	anmTbl = BlActAnm_GetAnmTbl( prm->anm_id );
	
	if( anmTbl->pAnmTbl != NULL ){
		GFL_BBDACT_SetAnimeTable( pBlActCont->pBbdActSys,
			actID, (GFL_BBDACT_ANMTBL)anmTbl->pAnmTbl, anmTbl->anm_max );
	}
	
	GFL_BBDACT_SetAnimeIdxOn(
			pBlActCont->pBbdActSys, actID, 0 );
	return( actID );
}
#endif

//--------------------------------------------------------------
/**
 * ビルボードアクター 追加
 * @param	fmmdl	MMDL
 * @param	code 使用するOBJコード
 * @param outID 追加アクターID格納先 MMDL_BLACTID_NULL=追加中
 * @retval BOOL TRUE=追加。FALSE=追加中。
 */
//--------------------------------------------------------------
BOOL MMDL_BLACTCONT_AddActor(
    MMDL *fmmdl, u16 code, GFL_BBDACT_ACTUNIT_ID *outID )
{
  u16 resID,flag;
	MMDLSYS *fmmdlsys = (MMDLSYS*)MMDL_GetMMdlSys( fmmdl );
	MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( fmmdlsys );
  
	BBDResUnitIndex_SearchResID( pBlActCont, code, &resID, &flag );
	
  if( resID != REGIDCODE_MAX && (flag&BBDRESBIT_TRANS) == 0 ){
    (*outID) = blact_SetActor( fmmdl, code );
    return( TRUE );
  }
  
  { //予約登録
#if 1
    *outID = MMDL_BLACTID_NULL;
    BlActAddReserve_RegistActor( pBlActCont, fmmdl, code, outID );
#else
    (*outID) = blact_SetActor( fmmdl, code );
    return( TRUE );
#endif
  }
  
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * ビルボードアクター 削除
 * @param	fmmdl	MMDL*
 * @param	actID GFL_BBDACT_ACTUNIT_ID
 * @retval	nothing
 */
//--------------------------------------------------------------
void MMDL_BLACTCONT_DeleteActor( MMDL *fmmdl, u32 actID )
{
  u16 id,flag;
  u16 code = MMDL_GetOBJCode( fmmdl );
	MMDLSYS *pMMdlSys = (MMDLSYS*)MMDL_GetMMdlSys( fmmdl );
	MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( pMMdlSys );
  
  if( actID == MMDL_BLACTID_NULL ){ //まだ追加されていない。
    BlActAddReserve_CancelActor( pBlActCont, fmmdl ); //予約あればキャンセル
    return;
  }
  
  if( BBDResUnitIndex_SearchResID(pBlActCont,code,&id,&flag) == TRUE )
  {
    if( (flag&BBDRESBIT_TRANS) ) //転送型 アクターの転送用リソース削除
    {
      u16 idx = GFL_BBDACT_GetResIdx( pBlActCont->pBbdActSys, actID );
	    GFL_BBDACT_RemoveResourceUnit( pBlActCont->pBbdActSys, idx, 1 );
    }
    
    if( (flag&BBDRESBIT_GUEST) ) //ゲスト登録 他に利用無ければ削除
    {
      if( MMDL_SearchUseOBJCode(fmmdl,code) == FALSE )
      {
        BBDResUnitIndex_RemoveResUnit( pBlActCont, code );
        
        KAGAYA_Printf(
          "MMDL DEL GUEST RESOURCE CODE=%d, RESID=%d\n", code, id );
      }
    }
  }
  
	GFL_BBDACT_RemoveAct( pBlActCont->pBbdActSys, actID, 1 );
}

//======================================================================
//	フィールド動作モデル　ビルボードアクター動作
//======================================================================
//--------------------------------------------------------------
/**
 * ビルボードアクター動作関数
 * @param	bbdActSys	GFL_BBDACT_SYS
 * @param	actIdx		int
 * @param	work		void*
 * @retval	nothing
 */
//--------------------------------------------------------------
static void BlActFunc( GFL_BBDACT_SYS *bbdActSys, int actIdx, void *work )
{
}

//======================================================================
//	フィールド動作モデル　ビルボードアクター管理　参照
//======================================================================
//--------------------------------------------------------------
/**
 * ビルボードアクター管理　GFL_BBDACT_SYS取得
 * @param	pBlActCont MMDL_BLACTCONT
 * @retval	GFL_BBDACT_SYS
 */
//--------------------------------------------------------------
GFL_BBDACT_SYS * MMDL_BLACTCONT_GetBbdActSys(
		MMDL_BLACTCONT *pBlActCont )
{
	return( pBlActCont->pBbdActSys );
}

//--------------------------------------------------------------
/**
 * ビルボードアクター管理 GFL_BBDACT_RESUNIT_ID取得
 * @param	pBlActCont MMDL_BLACTCONT
 * @retval	GFL_BBDACT_RESUNIT_ID
 */
//--------------------------------------------------------------
GFL_BBDACT_RESUNIT_ID MMDL_BLACTCONT_GetResUnitID(
		MMDL_BLACTCONT *pBlActCont )
{
	return( pBlActCont->bbdActResUnitID );
}

//======================================================================
//	IDCODEIDX
//======================================================================
//--------------------------------------------------------------
/**
 * IDCODEIDX 初期化
 * @param	idx IDCODEIDX*
 * @param	max 登録最大数
 * @param	heapID HEAPID
 * @retval	nothing
 */
//--------------------------------------------------------------
static void IDCodeIndex_Init( IDCODEIDX *idx, int max, HEAPID heapID )
{
	IDCODE *tbl;
	GF_ASSERT( max );
	tbl = GFL_HEAP_AllocClearMemory( heapID, sizeof(IDCODE)*max );
	idx->max = max;
	idx->pIDCodeBuf = tbl;
	do{
		tbl->code = REGIDCODE_MAX;
		tbl->id = REGIDCODE_MAX;
		tbl++;
		max--;
	}while( max );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX 削除
 * @param	idx	IDCODEIDX*
 * @retval	nothing
 */
//--------------------------------------------------------------
static void IDCodeIndex_Delete( IDCODEIDX *idx )
{
	GF_ASSERT( idx->pIDCodeBuf );
	GFL_HEAP_FreeMemory( idx->pIDCodeBuf );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX 指定されたコードとID、フラグを登録
 * @param	idx IDCODEIDX
 * @param	code コード
 * @param	id ID
 * @retval	nothing
 */
//--------------------------------------------------------------
static void IDCodeIndex_RegistCode(
    IDCODEIDX *idx, u16 code, u16 id, u16 flag )
{
	int i = 0;
	IDCODE *tbl = idx->pIDCodeBuf;
	do{
		if( tbl->code == REGIDCODE_MAX ){
			tbl->code = code;
			tbl->id = id;
      tbl->flag = flag;
			return;
		}
		tbl++;
		i++;
	}while( i < idx->max );
	GF_ASSERT( 0 );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX 指定されたコードを削除
 * @param	idx IDCODEIDX
 * @param	code OBJコード
 * @retval	nothing
 */
//--------------------------------------------------------------
static void IDCodeIndex_DeleteCode( IDCODEIDX *idx, u16 code )
{
	int i = 0;
	IDCODE *tbl = idx->pIDCodeBuf;
	do{
		if( tbl->code == code ){
			tbl->code = REGIDCODE_MAX;
			tbl->id = REGIDCODE_MAX;
      tbl->flag = 0;
			return;
		}
		tbl++;
		i++;
	}while( i < idx->max );
	GF_ASSERT( 0 );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX 指定されたコードに一致するID、フラグを返す
 * @param	idx IDCODEIDX
 * @param	code OBJコード
 * @param outID ID格納先 NULL=格納しない
 * @param outFlag フラグ格納先 NULL=格納しない
 * @retval BOOL TRUE=一致有り、FALSE=コードが存在しない
 */
//--------------------------------------------------------------
static BOOL IDCodeIndex_SearchCode(
    IDCODEIDX *idx, u16 code, u16 *outID, u16 *outFlag )
{
	int i = 0;
	IDCODE *tbl = idx->pIDCodeBuf;
  
  if( outID != NULL ){
    *outID = REGIDCODE_MAX;
  }
  
	do{
		if( tbl->code == code ){
      if( outID != NULL ){
        *outID = tbl->id;
      }
      if( outFlag != NULL ){
        *outFlag = tbl->flag;
      }
			return( TRUE );
		}
		tbl++;
		i++;
	}while( i < idx->max );
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT 初期化
 * @param	pBlActCont MMDL_BLACTCONT
 * @param	max	登録最大数
 * @retval	nothing
 */
//--------------------------------------------------------------
static void BBDResUnitIndex_Init( MMDL_BLACTCONT *pBlActCont, int max )
{
	HEAPID heapID = MMDLSYS_GetHeapID( pBlActCont->fmmdlsys );
	IDCodeIndex_Init( &pBlActCont->BBDResUnitIdx, max, heapID );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT 削除
 * @param	pBlActCont MMDL_BLACTCONT
 * @param	max	登録最大数
 * @retval	nothing
 */
//--------------------------------------------------------------
static void BBDResUnitIndex_Delete( MMDL_BLACTCONT *pBlActCont )
{
	int i = 0;
	IDCODE *tbl = pBlActCont->BBDResUnitIdx.pIDCodeBuf;
	
	do{
		if( tbl->code != REGIDCODE_MAX ){
			BBDResUnitIndex_RemoveResUnit( pBlActCont, tbl->code );
		}
		tbl++;
		i++;
	}while( i < pBlActCont->BBDResUnitIdx.max );
	
	IDCodeIndex_Delete( &pBlActCont->BBDResUnitIdx );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT リソース追加
 * @param	pBlActCont MMDL_BLACTCONT
 * @param	code	OBJコード
 * @param flag 登録フラグ BBDRESBIT_VRAM等
 * @retval	nothing
 */
//--------------------------------------------------------------
static void BBDResUnitIndex_AddResource( MMDL_BLACTCONT *pBlActCont,
    GFL_G3D_RES *g3dres, u16 obj_code, BBDRESBIT flag )
{
	GFL_BBDACT_RESUNIT_ID id;
  GFL_BBDACT_RESTYPE type;
  
	type = GFL_BBDACT_RESTYPE_DATACUT; //基本はリソース破棄型
  
  if( (flag&BBDRESBIT_TRANS) ){ //転送用リソース指定
    type = GFL_BBDACT_RESTYPE_TRANSSRC;
  }
  
  id = BlActRes_AddRes( pBlActCont, obj_code, g3dres, type );
	IDCodeIndex_RegistCode( &pBlActCont->BBDResUnitIdx, obj_code, id, flag );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT リソースユニット追加
 * @param	pBlActCont MMDL_BLACTCONT
 * @param	code	OBJコード
 * @param flag 登録フラグ BBDRESBIT_VRAM等
 * @retval	nothing
 */
//--------------------------------------------------------------
static void BBDResUnitIndex_AddResUnit(
    MMDL_BLACTCONT *pBlActCont, u16 obj_code, BBDRESBIT flag )
{
  GFL_G3D_RES *g3dres;
	const OBJCODE_PARAM *prm;
  
	prm = MMDLSYS_GetOBJCodeParam( pBlActCont->fmmdlsys, obj_code );
  g3dres = GFL_G3D_CreateResourceHandle(
      pBlActCont->arcH_res, prm->res_idx );
  BBDResUnitIndex_AddResource( pBlActCont, g3dres, obj_code, flag );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT 指定OBJコードのリソースユニット削除
 * @param	pBlActCont MMDL_BLACTCONT
 * @param	code	OBJコード
 * @retval	nothing
 */
//--------------------------------------------------------------
static void BBDResUnitIndex_RemoveResUnit(
		MMDL_BLACTCONT *pBlActCont, u16 obj_code )
{
	GFL_BBDACT_RESUNIT_ID id;
  BOOL ret = IDCodeIndex_SearchCode(
      &pBlActCont->BBDResUnitIdx, obj_code, &id, NULL );
	GF_ASSERT( ret == TRUE );
	GFL_BBDACT_RemoveResourceUnit( pBlActCont->pBbdActSys, id, 1 );
	IDCodeIndex_DeleteCode( &pBlActCont->BBDResUnitIdx, obj_code );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT　指定コードのリソースIDを取得
 * @param	pBlActCont MMDL_BLACTCONT
 * @param	code	OBJコード
 * @retval	BOOL TRUE=指定コード登録有り
 */
//--------------------------------------------------------------
static BOOL BBDResUnitIndex_SearchResID(
    MMDL_BLACTCONT *pBlActCont, u16 obj_code, u16 *outID, u16 *outFlag )
{
  BOOL ret = IDCodeIndex_SearchCode(
    &pBlActCont->BBDResUnitIdx, obj_code, outID, outFlag );
  return( ret );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT 指定コードのリソースIDが追加済みかチェック
 * @param	fmmdlsys	MMDLSYS
 * @param	code OBJコード
 * @retval	BOOL	TRUE=追加済み FALSE=追加まだ
 */
//--------------------------------------------------------------
BOOL MMDL_BLACTCONT_CheckOBJCodeRes( MMDLSYS *fmmdlsys, u16 code )
{
	u16 res;
	MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( fmmdlsys );
	
	if( IDCodeIndex_SearchCode(&pBlActCont->BBDResUnitIdx,code,NULL,NULL) ){
    return( TRUE );
  }
  
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT 指定コードのリソースを追加
 * @param	fmmdlssy	MMDLSYS
 * @param	code	OBJコード
 * @param trans TRUE=テクスチャ転送型で登録
 * @param guest TRUE=使用されなくなると破棄されるゲストタイプで登録
 * @retval	BOOL	TRUE=追加した。FALSE=既に追加済み
 * @note 呼ばれたその場で読み込みが発生する。
 */
//--------------------------------------------------------------
BOOL MMDL_BLACTCONT_AddOBJCodeRes(
    MMDLSYS *fmmdlsys, u16 code, BOOL trans, BOOL guest )
{
	if( MMDL_BLACTCONT_CheckOBJCodeRes(fmmdlsys,code) == FALSE ){
    MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( fmmdlsys );
    BBDRESBIT flag = 0;
    if( trans ){ flag |= BBDRESBIT_TRANS; }
    if( guest ){ flag |= BBDRESBIT_GUEST; }
		BBDResUnitIndex_AddResUnit( pBlActCont, code, flag );
		return( TRUE );
	}
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT 指定コードのリソースを削除
 * @param	fmmdlssy	MMDLSYS
 * @param	code	OBJコード
 * @retval	nothing
 */
//--------------------------------------------------------------
void MMDL_BLACTCONT_DeleteOBJCodeRes( MMDLSYS *fmmdlsys, u16 code )
{
  MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( fmmdlsys );
	BBDResUnitIndex_RemoveResUnit( pBlActCont, code );
}

//======================================================================
//  アクター追加予約処理
//======================================================================
//--------------------------------------------------------------
/**
 * ビルボードアクター追加予約処理　初期化
 * @param pBlActCont MMDL_BLACTCONT*
 * @retval nothing
 */
//--------------------------------------------------------------
static void BlActAddReserve_Init( MMDL_BLACTCONT *pBlActCont )
{
  HEAPID heapID = MMDLSYS_GetHeapID( pBlActCont->fmmdlsys );
  BLACT_RESERVE *pReserve = 
    GFL_HEAP_AllocClearMemory( heapID, sizeof(BLACT_RESERVE) );
  
  pReserve->resMax = pBlActCont->resourceMax;
  pReserve->actMax = MMDLSYS_GetMMdlMax( pBlActCont->fmmdlsys );
  
  pReserve->pReserveRes = GFL_HEAP_AllocClearMemory(
      heapID, sizeof(ADDRES_RESERVE)*pReserve->resMax );
  pReserve->pReserveAct = GFL_HEAP_AllocClearMemory(
      heapID, sizeof(ADDACT_RESERVE)*pReserve->actMax );
  
  pReserve->funcFlag = TRUE;
  pBlActCont->pReserve = pReserve;
}

//--------------------------------------------------------------
/**
 * ビルボードアクター追加予約処理　削除
 * @param pBlActCont MMDL_BLACTCONT*
 * @retval nothing
 */
//--------------------------------------------------------------
static void BlActAddReserve_Delete( MMDL_BLACTCONT *pBlActCont )
{
  BLACT_RESERVE *pReserve = pBlActCont->pReserve;
  
  if( pReserve != NULL ){
    u32 i;
    pReserve->funcFlag = FALSE;
    
    { //リソース
      ADDRES_RESERVE *pRes = pReserve->pReserveRes;
      for( i = 0; i < pReserve->resMax; i++, pRes++ ){
        if( pRes->pG3dRes != NULL ){
          GFL_G3D_DeleteResource( pRes->pG3dRes );
        }
      }
      GFL_HEAP_FreeMemory( pReserve->pReserveRes );
    }

    { //アクター
      ADDACT_RESERVE *pRes = pReserve->pReserveAct;
      for( i = 0; i < pReserve->actMax; i++, pRes++ ){
        if( pRes->fmmdl != NULL && pRes->pTransActRes != NULL ){
          GFL_G3D_DeleteResource( pRes->pTransActRes );
        }
      }
      GFL_HEAP_FreeMemory( pReserve->pReserveAct );
    }
    
    pBlActCont->pReserve = NULL;
    GFL_HEAP_FreeMemory( pReserve );
  }
}

//--------------------------------------------------------------
/**
 * アクター追加予約処理　予約消化
 * @param MMDLSYS *fmmdlsys
 * @retval nothing
 */
//--------------------------------------------------------------
static void BlActAddReserve_Digest( MMDL_BLACTCONT *pBlActCont )
{
  BLACT_RESERVE *pReserve = pBlActCont->pReserve;
  
  if( pReserve != NULL )
  {
    if( pReserve->funcFlag == TRUE )
    {
      BlActAddReserve_DigestResource( pBlActCont );
      BlActAddReserve_DigestActor( pBlActCont );
    }
  }
}

//--------------------------------------------------------------
/**
 * リソース追加予約処理　予約登録
 * @param pBlActCont MMDL_BLACTCONT
 * @param code 表示コード
 * @param flag BBDRESBIT
 * @retval nothing
 */
//--------------------------------------------------------------
static void BlActAddReserve_RegistResource(
    MMDL_BLACTCONT *pBlActCont, u16 code, BBDRESBIT flag )
{
  u32 i = 0;
  BLACT_RESERVE *pReserve = pBlActCont->pReserve;
  ADDRES_RESERVE *pRes = pReserve->pReserveRes;
  
  for( ; i < pReserve->resMax; i++, pRes++ ){
    if( pRes->pG3dRes == NULL ){
	    const OBJCODE_PARAM *prm;
      prm = MMDLSYS_GetOBJCodeParam( pBlActCont->fmmdlsys, code );
      
      pRes->compFlag = FALSE;
      pRes->code = code;
      pRes->flag = flag;
      pRes->pG3dRes = GFL_G3D_CreateResourceHandle(
              pBlActCont->arcH_res, prm->res_idx );
      pRes->compFlag = TRUE;
      
      KAGAYA_Printf(
        "MMDL BLACT RESERVE ADD RESOURCE CODE=%d,ARCIDX=%d\n",
        pRes->code, prm->res_idx );
      return;
    }
  }
  
  GF_ASSERT( 0 );
}

//--------------------------------------------------------------
/**
 * リソース追加予約処理　予約消化
 * @param pBlActCont MMDL_BLACTCONT
 * @retval nothing
 */
//--------------------------------------------------------------
static void BlActAddReserve_DigestResource( MMDL_BLACTCONT *pBlActCont )
{
  u32 i = 0;
  BLACT_RESERVE *pReserve = pBlActCont->pReserve;
  ADDRES_RESERVE *pRes = pReserve->pReserveRes;
  
  for( ; i < pReserve->resMax; i++, pRes++ ){
    if( pRes->pG3dRes != NULL && pRes->compFlag == TRUE ){
      pRes->compFlag = FALSE;
      BBDResUnitIndex_AddResource(
          pBlActCont, pRes->pG3dRes, pRes->code, pRes->flag );
      pRes->pG3dRes = NULL;
    
#ifdef DEBUG_MMDL_DEVELOP
      {
        u16 id,flag;
        BBDResUnitIndex_SearchResID( pBlActCont, pRes->code, &id, &flag );
        
        KAGAYA_Printf(
          "MMDL BLACT RESERVE DIG RESOURCE CODE=%d, RESID=%d\n",
          pRes->code, id );
      }
#endif
    }
  }
}

//--------------------------------------------------------------
/**
 * リソース追加予約処理　予約検索
 * @param pBlActCont MMDL_BLACTCONT
 * @param code 検索する表示コード
 * @param outFlag *予約の際に指定されたBBDRESBIT格納先
 * @retval BOOL TRUE=予約済み FALSE=予約なし
 */
//--------------------------------------------------------------
static BOOL BlActAddReserve_SearchResource(
    MMDL_BLACTCONT *pBlActCont, u16 code, u16 *outFlag )
{
  u32 i = 0;
  BLACT_RESERVE *pReserve = pBlActCont->pReserve;
  ADDRES_RESERVE *pRes = pReserve->pReserveRes;
  
  for( ; i < pReserve->resMax; i++, pRes++ ){
    if( pRes->code == code && pRes->pG3dRes != NULL ){
      *outFlag = pRes->flag;
      return( TRUE );
    }
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * リソース追加予約処理　予約キャンセル
 * @param pBlActCont MMDL_BLACTCONT
 * @param code キャンセルする表示コード
 * @retval BOOL TRUE=キャンセルした FALSE=予約なし
 */
//--------------------------------------------------------------
static BOOL BlActAddReserve_CancelResource(
    MMDL_BLACTCONT *pBlActCont, u16 code )
{
  u32 i = 0;
  BLACT_RESERVE *pReserve = pBlActCont->pReserve;
  ADDRES_RESERVE *pRes = pReserve->pReserveRes;
  
  for( ; i < pReserve->resMax; i++, pRes++ ){
    if( pRes->pG3dRes != NULL && pRes->code == code ){
      pRes->compFlag = FALSE;
      GFL_G3D_DeleteResource( pRes->pG3dRes );
      pRes->pG3dRes = NULL;
      KAGAYA_Printf( "MMDL BLACT RESERVE CANCEL CODE=%d\n", pRes->code );
      return( TRUE );
    }
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * アクター追加予約処理　予約登録
 * @param pBlActCont MMDL_BLACTCONT
 * @param fmmdl MMDL*
 * @param code 表示コード
 * @param outID アクターID格納先
 * @retval nothing
 */
//--------------------------------------------------------------
static void BlActAddReserve_RegistActor(
    MMDL_BLACTCONT *pBlActCont, MMDL *fmmdl, u16 code, u16 *outID )
{
  u32 i = 0;
  u32 max = MMDLSYS_GetMMdlMax( pBlActCont->fmmdlsys );
  ADDACT_RESERVE *pRes = pBlActCont->pReserve->pReserveAct;
  
  for( ; i < max; i++, pRes++ )
  {
    if( pRes->fmmdl == NULL )
    {
      u16 resID,flag;
	    const OBJCODE_PARAM *prm;
      
      pRes->compFlag = FALSE;
      pRes->fmmdl = fmmdl;
      pRes->code = code;
      pRes->outID = outID;
      
	    prm = MMDLSYS_GetOBJCodeParam( pBlActCont->fmmdlsys, pRes->code );
	    BBDResUnitIndex_SearchResID( pBlActCont, code, &resID, &flag );
      
      if( resID == REGIDCODE_MAX ) //非登録リソース
      {
        if( BlActAddReserve_SearchResource( //リソース予約検索
              pBlActCont,code,&flag) == FALSE )
        {
          flag = BBDRES_VRAM_GUEST; //基本VRAM常駐ゲスト型で登録
          BlActAddReserve_RegistResource( pBlActCont, code, flag );
        }
      }
      
      if( (flag&BBDRESBIT_TRANS) ){ //転送型 転送先リソース作成
        #if 0
        //転送用リソースデータを取得
        pRes->pTransActRes = GFL_G3D_CreateResourceHandle(
              pBlActCont->arcH_res, prm->res_idx );
        #else
        GF_ASSERT( 0 ); //現状未対応
        #endif
      }
      
      pRes->compFlag = TRUE;
      KAGAYA_Printf( "MMDL BLACT RESERVE ADD ACTOR OBJID=%d, CODE=%d\n", 
          MMDL_GetOBJID(fmmdl), pRes->code );
      return;
    }
  }
  
  GF_ASSERT( 0 && "MMDL BLACT RESERVE ADD MAX" );
}

//--------------------------------------------------------------
/**
 * アクター追加予約処理　予約消化
 * @param MMDLSYS *fmmdlsys
 * @retval nothing
 */
//--------------------------------------------------------------
static void BlActAddReserve_DigestActor( MMDL_BLACTCONT *pBlActCont )
{
  u32 i = 0;
  u32 max = MMDLSYS_GetMMdlMax( pBlActCont->fmmdlsys );
  ADDACT_RESERVE *pRes = pBlActCont->pReserve->pReserveAct;
  
  for( ; i < max; i++, pRes++ )
  {
    if( pRes->fmmdl != NULL && pRes->compFlag == TRUE )
    {
      BOOL ret;
      u16 resID,flag;
      u16 transID = BLACT_RESID_NULL;
      
      pRes->compFlag = FALSE;
      
      if( pRes->pTransActRes != NULL ) //VRAM転送型
      {
        resID = BlActRes_AddRes( pBlActCont, pRes->code,
            pRes->pTransActRes, GFL_BBDACT_RESTYPE_DATACUT );
        pRes->pTransActRes = NULL;
        
        ret = BBDResUnitIndex_SearchResID(
            pBlActCont, pRes->code, &transID, &flag );
        
        GF_ASSERT( ret == TRUE &&
          "BLACT ADD RESERVE RESOURCE NONE" ); //転送用リソース無し
        GF_ASSERT( (flag&BBDRESBIT_TRANS) &&
          "BLACT ADD RESERVE RESOURCE ERROR" ); //転送用リソースでは無い
      }
      else //VRAM常駐型
      {
        ret = BBDResUnitIndex_SearchResID(
            pBlActCont, pRes->code, &resID, &flag );

        #ifdef DEBUG_MMDL_DEVELOP
        if( ret != TRUE ){
          OS_Printf( "BLACT ADD RESOURCE ERROR CODE=%xH\n", pRes->code );
        }
        #endif

        if( ret != TRUE ){
          
          ret = BBDResUnitIndex_SearchResID(
              pBlActCont, pRes->code, &resID, &flag );
        }
        
        GF_ASSERT( ret == TRUE &&
          "BLACT ADD RESERVE RESOURCE NONE" ); //リソース無し
      }
      
      *(pRes->outID) = blact_SetResActor( //アクター追加
          pRes->fmmdl, pRes->code, resID, transID );
      MMDL_CallDrawProc( pRes->fmmdl ); //描画処理呼び出し
      
      KAGAYA_Printf( "MMDL BLACT RESERVE DIG ACTOR " );
      KAGAYA_Printf( "OBJID=%d, CODE=%d, RESID=%d,TRANSID=%d\n",
          MMDL_GetOBJID(pRes->fmmdl), pRes->code, resID, transID );
      
      pRes->fmmdl = NULL;
    }
  }
}

//--------------------------------------------------------------
/**
 * アクター追加予約処理　予約済みの表示コードを検索
 * @param pBlActCont MMDL_BLACTCONT
 * @param code 検索する表示コード
 * @param fmmdl 検索対象としないMMDL
 * @retval BOOL TRUE=一致有り FALSE=一致無し
 */
//--------------------------------------------------------------
static BOOL BlActAddReserve_SearchActorOBJCode(
    MMDL_BLACTCONT *pBlActCont, u16 code, MMDL *fmmdl )
{
  u32 i = 0;
  u32 max = MMDLSYS_GetMMdlMax( pBlActCont->fmmdlsys );
  ADDACT_RESERVE *pRes = pBlActCont->pReserve->pReserveAct;
  
  for( ; i < max; i++, pRes++ ){
    if( pRes->fmmdl != NULL && pRes->fmmdl != fmmdl ){
      if( pRes->code == code ){
        return( TRUE );
      }
    }
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * アクター追加予約処理　予約キャンセル
 * @param pBlActCont MMDL_BLACTCONT
 * @param fmmdl 予約したMMDL*
 * @retval nothing
 * @note 追加時に指定されたコードがゲストリソースとして
 * 追加予約に入っていた際はリソース予約キャンセルも合わせて行う。
 */
//--------------------------------------------------------------
static void BlActAddReserve_CancelActor(
    MMDL_BLACTCONT *pBlActCont, MMDL *fmmdl )
{
  u32 i = 0;
  u32 max = MMDLSYS_GetMMdlMax( pBlActCont->fmmdlsys );
  ADDACT_RESERVE *pRes = pBlActCont->pReserve->pReserveAct;
  
  for( ; i < max; i++, pRes++ )
  {
    if( pRes->fmmdl == fmmdl )
    {
      u16 flag;
      pRes->compFlag = FALSE;
      
      if( BlActAddReserve_SearchResource( //リソース予約有り
            pBlActCont,pRes->code,&flag) == TRUE )
      {
        if( (flag&BBDRESBIT_GUEST) ){ //ゲストリソース
          if( BlActAddReserve_SearchActorOBJCode( //他の予約で使用無し
                pBlActCont,pRes->code,pRes->fmmdl) == FALSE )
          {
            BlActAddReserve_CancelResource( pBlActCont, pRes->code );
          }
        }
      }
      
      if( pRes->pTransActRes != NULL ) //転送用リソース有り
      {
        GFL_G3D_DeleteResource( pRes->pTransActRes );
        pRes->pTransActRes = NULL;
      }
      
      pRes->fmmdl = NULL;
      
      KAGAYA_Printf( "MMDL BLACT RESERVE CANCEL OBJID=%d, CODE=%d\n", 
          MMDL_GetOBJID(fmmdl), pRes->code );
      return;
    }
  }
}

//======================================================================
//	parts
//======================================================================
//--------------------------------------------------------------
/**
 * テーブル番号からアクターアニメテーブル取得
 * @param	no	MMDL_BLACT_ANMTBLNO_MAX
 * @retval	MMDL_BBDACT_ANMTBL*
 */
//--------------------------------------------------------------
static const MMDL_BBDACT_ANMTBL * BlActAnm_GetAnmTbl( u32 no )
{
	GF_ASSERT( no < MMDL_BLACT_ANMTBLNO_MAX );
	return( &DATA_MMDL_BBDACT_ANM_ListTable[no] );
}

//--------------------------------------------------------------
/**
 * ビルボードアニメ 描画ステータスからテーブル要素数を取得。基本タイプ
 * @param sta DRAW_STA_STOP等
 * @retval u32 対応するアニメーションテーブル要素数
 */
//--------------------------------------------------------------
u32 FLDMMDL_BLACTCONT_GetAnmTblNo_Status( u32 sta )
{
  GF_ASSERT( sta < DRAW_STA_MAX );
  //上下左右の並びで各４方向
  return( sta * DIR_MAX4 );
}

//--------------------------------------------------------------
/**
 * ビルボードアニメ 描画ステータスからテーブル要素数を取得。自機専用
 * @param sta DRAW_STA_STOP等
 * @retval u32 対応するアニメーションテーブル要素数
 */
//--------------------------------------------------------------
u32 FLDMMDL_BLACTCONT_GetAnmTblNo_StatusHero( u32 sta )
{
  GF_ASSERT( sta < DRAW_STA_MAX_HERO );
  //上下左右の並びで各４方向
  return( sta * DIR_MAX4 );
}

//--------------------------------------------------------------
/**
 * ビルボードアクターリソース追加
 * @param pBlActCont MMDL_BLACTCONT
 * @param code OBJコード
 * g3dres 追加するリソース*
 * @param type リソースタイプ
 * @retval GFL_BBDACT_RESUNIT_ID
 */
//--------------------------------------------------------------
static GFL_BBDACT_RESUNIT_ID BlActRes_AddRes(
    MMDL_BLACTCONT *pBlActCont, u16 code,
    GFL_G3D_RES *g3dres, GFL_BBDACT_RESTYPE type )
{
	GFL_BBDACT_RESUNIT_ID id;
	const OBJCODE_PARAM *prm;
	GFL_BBDACT_G3DRESDATA data;
  
	prm = MMDLSYS_GetOBJCodeParam( pBlActCont->fmmdlsys, code );
  
  data.g3dres = g3dres;
	data.texFmt = GFL_BBD_TEXFMT_PAL16;
	data.texSiz = prm->tex_size;
	data.celSizX = 32;				//いずれmdl_sizeから
	data.celSizY = 32;
  data.dataCut = type;
  
	id = GFL_BBDACT_AddResourceG3DResUnit( pBlActCont->pBbdActSys, &data, 1 );
  
  KAGAYA_Printf( "MMDL ADD RESOURCE CODE=%d,RESID=%d\n", code, id );
  return( id );
}

//======================================================================
//	test data
//======================================================================
/*
static const GFL_BBDACT_RESDATA testResTable[] = {
#ifdef MMDL_BLACT_HEAD3_TEST
	{ ARCID_MMDL_BTX, NARC_fldmmdl_btx_obj_kabi32_nsbtx, 
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
#else
	{ ARCID_FLDMAP_ACTOR, NARC_fld_act_hero_nsbtx, 
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x1024, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
#endif
	{ ARCID_FLDMAP_ACTOR, NARC_fld_act_achamo_nsbtx, 
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDMAP_ACTOR, NARC_fld_act_badman_nsbtx, 
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDMAP_ACTOR, NARC_fld_act_beachgirl_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDMAP_ACTOR, NARC_fld_act_idol_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDMAP_ACTOR, NARC_fld_act_lady_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDMAP_ACTOR, NARC_fld_act_oldman1_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDMAP_ACTOR, NARC_fld_act_policeman_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
};
const u32 testResTableMax = NELEMS( testResTable );
*/

//======================================================================
//	動作モデル管理環境で定義されるテクスチャサイズ指定が
//	ビルボードアクターで定義されているものと同一かどうか
//	プリプロセッサ命令でチェック
//======================================================================
#define MMDL_CHECK_GFL_BBD_TEXSIZDEF ///<定義でチェック有効

#ifdef MMDL_CHECK_GFL_BBD_TEXSIZDEF
#include "field/fldmmdl_list_symbol.h"

#if (GFL_BBD_TEXSIZDEF_8x8-TEXSIZE_8x8)
#error (GFL_BBD_TEXSIZDEF_8x8-TEXSIZE_8x8)
#endif
#if (GFL_BBD_TEXSIZDEF_8x16-TEXSIZE_8x16)
#error (GFL_BBD_TEXSIZDEF_8x16-TEXSIZE_8x16)
#endif
#if (GFL_BBD_TEXSIZDEF_8x32-TEXSIZE_8x32)
#error (GFL_BBD_TEXSIZDEF_8x32-TEXSIZE_8x32)
#endif
#if (GFL_BBD_TEXSIZDEF_8x64-TEXSIZE_8x64)
#error (GFL_BBD_TEXSIZDEF_8x64-TEXSIZE_8x64)
#endif
#if (GFL_BBD_TEXSIZDEF_8x128-TEXSIZE_8x128)
#error (GFL_BBD_TEXSIZDEF_8x128-TEXSIZE_8x128)
#endif
#if (GFL_BBD_TEXSIZDEF_8x256-TEXSIZE_8x256)
#error (GFL_BBD_TEXSIZDEF_8x256-TEXSIZE_8x256)
#endif
#if (GFL_BBD_TEXSIZDEF_8x512-TEXSIZE_8x512)
#error (GFL_BBD_TEXSIZDEF_8x512-TEXSIZE_8x512)
#endif
#if (GFL_BBD_TEXSIZDEF_8x1024-TEXSIZE_8x1024)
#error (GFL_BBD_TEXSIZDEF_8x1024-TEXSIZE_8x1024)
#endif
#if (GFL_BBD_TEXSIZDEF_16x8-TEXSIZE_16x8)
#error (GFL_BBD_TEXSIZDEF_16x8-TEXSIZE_16x8)
#endif
#if (GFL_BBD_TEXSIZDEF_16x16-TEXSIZE_16x16)
#error (GFL_BBD_TEXSIZDEF_16x16-TEXSIZE_16x16)
#endif
#if (GFL_BBD_TEXSIZDEF_16x32-TEXSIZE_16x32)
#error (GFL_BBD_TEXSIZDEF_16x32-TEXSIZE_16x32)
#endif
#if (GFL_BBD_TEXSIZDEF_16x64-TEXSIZE_16x64)
#error (GFL_BBD_TEXSIZDEF_16x64-TEXSIZE_16x64)
#endif
#if (GFL_BBD_TEXSIZDEF_16x128-TEXSIZE_16x128)
#error (GFL_BBD_TEXSIZDEF_16x128-TEXSIZE_16x128)
#endif
#if (GFL_BBD_TEXSIZDEF_16x256-TEXSIZE_16x256)
#error (GFL_BBD_TEXSIZDEF_16x256-TEXSIZE_16x256)
#endif
#if (GFL_BBD_TEXSIZDEF_16x512-TEXSIZE_16x512)
#error (GFL_BBD_TEXSIZDEF_16x512-TEXSIZE_16x512)
#endif
#if (GFL_BBD_TEXSIZDEF_16x1024-TEXSIZE_16x1024)
#error (GFL_BBD_TEXSIZDEF_16x1024-TEXSIZE_16x1024)
#endif
#if (GFL_BBD_TEXSIZDEF_32x8-TEXSIZE_32x8)
#error (GFL_BBD_TEXSIZDEF_32x8-TEXSIZE_32x8)
#endif
#if (GFL_BBD_TEXSIZDEF_32x16-TEXSIZE_32x16)
#error (GFL_BBD_TEXSIZDEF_32x16-TEXSIZE_32x16)
#endif
#if (GFL_BBD_TEXSIZDEF_32x32-TEXSIZE_32x32)
#error (GFL_BBD_TEXSIZDEF_32x32-TEXSIZE_32x32)
#endif
#if (GFL_BBD_TEXSIZDEF_32x64-TEXSIZE_32x64)
#error (GFL_BBD_TEXSIZDEF_32x64-TEXSIZE_32x64)
#endif
#if (GFL_BBD_TEXSIZDEF_32x128-TEXSIZE_32x128)
#error (GFL_BBD_TEXSIZDEF_32x128-TEXSIZE_32x128)
#endif
#if (GFL_BBD_TEXSIZDEF_32x256-TEXSIZE_32x256)
#error (GFL_BBD_TEXSIZDEF_32x256-TEXSIZE_32x256)
#endif
#if (GFL_BBD_TEXSIZDEF_32x512-TEXSIZE_32x512)
#error (GFL_BBD_TEXSIZDEF_32x512-TEXSIZE_32x512)
#endif
#if (GFL_BBD_TEXSIZDEF_32x1024-TEXSIZE_32x1024)
#error (GFL_BBD_TEXSIZDEF_32x1024-TEXSIZE_32x1024)
#endif
#if (GFL_BBD_TEXSIZDEF_64x8-TEXSIZE_64x8)
#error (GFL_BBD_TEXSIZDEF_64x8-TEXSIZE_64x8)
#endif
#if (GFL_BBD_TEXSIZDEF_64x16-TEXSIZE_64x16)
#error (GFL_BBD_TEXSIZDEF_64x16-TEXSIZE_64x16)
#endif
#if (GFL_BBD_TEXSIZDEF_64x32-TEXSIZE_64x32)
#error (GFL_BBD_TEXSIZDEF_64x32-TEXSIZE_64x32)
#endif
#if (GFL_BBD_TEXSIZDEF_64x64-TEXSIZE_64x64)
#error (GFL_BBD_TEXSIZDEF_64x64-TEXSIZE_64x64)
#endif
#if (GFL_BBD_TEXSIZDEF_64x128-TEXSIZE_64x128)
#error (GFL_BBD_TEXSIZDEF_64x128-TEXSIZE_64x128)
#endif
#if (GFL_BBD_TEXSIZDEF_64x256-TEXSIZE_64x256)
#error (GFL_BBD_TEXSIZDEF_64x256-TEXSIZE_64x256)
#endif
#if (GFL_BBD_TEXSIZDEF_64x512-TEXSIZE_64x512)
#error (GFL_BBD_TEXSIZDEF_64x512-TEXSIZE_64x512)
#endif
#if (GFL_BBD_TEXSIZDEF_64x1024-TEXSIZE_64x1024)
#error (GFL_BBD_TEXSIZDEF_64x1024-TEXSIZE_64x1024)
#endif
#if (GFL_BBD_TEXSIZDEF_128x8-TEXSIZE_128x8)
#error (GFL_BBD_TEXSIZDEF_128x8-TEXSIZE_128x8)
#endif
#if (GFL_BBD_TEXSIZDEF_128x16-TEXSIZE_128x16)
#error (GFL_BBD_TEXSIZDEF_128x16-TEXSIZE_128x16)
#endif
#if (GFL_BBD_TEXSIZDEF_128x32-TEXSIZE_128x32)
#error (GFL_BBD_TEXSIZDEF_128x32-TEXSIZE_128x32)
#endif
#if (GFL_BBD_TEXSIZDEF_128x64-TEXSIZE_128x64)
#error (GFL_BBD_TEXSIZDEF_128x64-TEXSIZE_128x64)
#endif
#if (GFL_BBD_TEXSIZDEF_128x128-TEXSIZE_128x128)
#error (GFL_BBD_TEXSIZDEF_128x128-TEXSIZE_128x128)
#endif
#if (GFL_BBD_TEXSIZDEF_128x256-TEXSIZE_128x256)
#error (GFL_BBD_TEXSIZDEF_128x256-TEXSIZE_128x256)
#endif
#if (GFL_BBD_TEXSIZDEF_128x512-TEXSIZE_128x512)
#error (GFL_BBD_TEXSIZDEF_128x512-TEXSIZE_128x512)
#endif
#if (GFL_BBD_TEXSIZDEF_128x1024-TEXSIZE_128x1024)
#error (GFL_BBD_TEXSIZDEF_128x1024-TEXSIZE_128x1024)
#endif
#if (GFL_BBD_TEXSIZDEF_256x8-TEXSIZE_256x8)
#error (GFL_BBD_TEXSIZDEF_256x8-TEXSIZE_256x8)
#endif
#if (GFL_BBD_TEXSIZDEF_256x16-TEXSIZE_256x16)
#error (GFL_BBD_TEXSIZDEF_256x16-TEXSIZE_256x16)
#endif
#if (GFL_BBD_TEXSIZDEF_256x32-TEXSIZE_256x32)
#error (GFL_BBD_TEXSIZDEF_256x32-TEXSIZE_256x32)
#endif
#if (GFL_BBD_TEXSIZDEF_256x64-TEXSIZE_256x64)
#error (GFL_BBD_TEXSIZDEF_256x64-TEXSIZE_256x64)
#endif
#if (GFL_BBD_TEXSIZDEF_256x128-TEXSIZE_256x128)
#error (GFL_BBD_TEXSIZDEF_256x128-TEXSIZE_256x128)
#endif
#if (GFL_BBD_TEXSIZDEF_256x256-TEXSIZE_256x256)
#error (GFL_BBD_TEXSIZDEF_256x256-TEXSIZE_256x256)
#endif
#if (GFL_BBD_TEXSIZDEF_256x512-TEXSIZE_256x512)
#error (GFL_BBD_TEXSIZDEF_256x512-TEXSIZE_256x512)
#endif
#if (GFL_BBD_TEXSIZDEF_256x1024-TEXSIZE_256x1024)
#error (GFL_BBD_TEXSIZDEF_256x1024-TEXSIZE_256x1024)
#endif
#if (GFL_BBD_TEXSIZDEF_512x8-TEXSIZE_512x8)
#error (GFL_BBD_TEXSIZDEF_512x8-TEXSIZE_512x8)
#endif
#if (GFL_BBD_TEXSIZDEF_512x16-TEXSIZE_512x16)
#error (GFL_BBD_TEXSIZDEF_512x16-TEXSIZE_512x16)
#endif
#if (GFL_BBD_TEXSIZDEF_512x32-TEXSIZE_512x32)
#error (GFL_BBD_TEXSIZDEF_512x32-TEXSIZE_512x32)
#endif
#if (GFL_BBD_TEXSIZDEF_512x64-TEXSIZE_512x64)
#error (GFL_BBD_TEXSIZDEF_512x64-TEXSIZE_512x64)
#endif
#if (GFL_BBD_TEXSIZDEF_512x128-TEXSIZE_512x128)
#error (GFL_BBD_TEXSIZDEF_512x128-TEXSIZE_512x128)
#endif
#if (GFL_BBD_TEXSIZDEF_512x256-TEXSIZE_512x256)
#error (GFL_BBD_TEXSIZDEF_512x256-TEXSIZE_512x256)
#endif
#if (GFL_BBD_TEXSIZDEF_512x512-TEXSIZE_512x512)
#error (GFL_BBD_TEXSIZDEF_512x512-TEXSIZE_512x512)
#endif
#if (GFL_BBD_TEXSIZDEF_512x1024-TEXSIZE_512x1024)
#error (GFL_BBD_TEXSIZDEF_512x1024-TEXSIZE_512x1024)
#endif
#if (GFL_BBD_TEXSIZDEF_1024x8-TEXSIZE_1024x8)
#error (GFL_BBD_TEXSIZDEF_1024x8-TEXSIZE_1024x8)
#endif
#if (GFL_BBD_TEXSIZDEF_1024x16-TEXSIZE_1024x16)
#error (GFL_BBD_TEXSIZDEF_1024x16-TEXSIZE_1024x16)
#endif
#if (GFL_BBD_TEXSIZDEF_1024x32-TEXSIZE_1024x32)
#error (GFL_BBD_TEXSIZDEF_1024x32-TEXSIZE_1024x32)
#endif
#if (GFL_BBD_TEXSIZDEF_1024x64-TEXSIZE_1024x64)
#error (GFL_BBD_TEXSIZDEF_1024x64-TEXSIZE_1024x64)
#endif
#if (GFL_BBD_TEXSIZDEF_1024x128-TEXSIZE_1024x128)
#error (GFL_BBD_TEXSIZDEF_1024x128-TEXSIZE_1024x128)
#endif
#if (GFL_BBD_TEXSIZDEF_1024x256-TEXSIZE_1024x256)
#error (GFL_BBD_TEXSIZDEF_1024x256-TEXSIZE_1024x256)
#endif
#if (GFL_BBD_TEXSIZDEF_1024x512-TEXSIZE_1024x512)
#error (GFL_BBD_TEXSIZDEF_1024x512-TEXSIZE_1024x512)
#endif
#if (GFL_BBD_TEXSIZDEF_1024x1024-TEXSIZE_1024x1024)
#error (GFL_BBD_TEXSIZDEF_1024x1024-TEXSIZE_1024x1024)
#endif
#endif //MMDL_CHECK_GFL_BBD_TEXSIZDEF
