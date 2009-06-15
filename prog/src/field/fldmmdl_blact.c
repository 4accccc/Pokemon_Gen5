//======================================================================
/**
 * @file	fldmmdl_blact.c
 * @brief	フィールド動作モデル ビルボードアクター管理
 * @author	kagaya
 * @data	05.07.13
 */
//======================================================================
#include "fldmmdl.h"
#include "test_graphic/fld_act.naix"

#ifdef FLDMMDL_BLACT_HEAD3_TEST
#include "test_graphic/fldmmdl_btx.naix"
#endif

//======================================================================
//	define
//======================================================================
#define REGIDCODE_MAX (0xffff) ///<BBDRESID最大

typedef enum
{
  BBDRESBIT_GUEST = (1<<0), ///<リソース識別 ゲスト登録 OFF時=レギュラー
  BBDRESBIT_TRANS = (1<<1), ///<リソース識別 VRAM転送型
  
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
///	FLDMMDL_BLACTCONT
//--------------------------------------------------------------
struct _TAG_FLDMMDL_BLACTCONT{
	GFL_BBDACT_SYS *pBbdActSys;
	GFL_BBDACT_RESUNIT_ID bbdActResUnitID;
	GFL_BBDACT_ACTUNIT_ID bbdActActUnitID;
	u16 bbdActResCount;
	u16 bbdActActCount;
	
	FLDMMDLSYS *fmmdlsys;
	
	IDCODEIDX BBDResUnitIdx;
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
static void BBDResUnitIndex_Init( FLDMMDLSYS *fmmdlsys, int max );
static void BBDResUnitIndex_Delete( FLDMMDLSYS *fmmdlsys );
static void BBDResUnitIndex_AddResUnit(
    FLDMMDLSYS *fmmdlsys, u16 obj_code, BBDRESBIT flag );
static void BBDResUnitIndex_RemoveResUnit(FLDMMDLSYS *fmmdlsys,u16 obj_code);
static BOOL BBDResUnitIndex_SearchResID(
    FLDMMDLSYS *fmmdlsys, u16 obj_code, u16 *outID, u16 *outFlag );

static const FLDMMDL_BBDACT_ANMTBL * BlActAnm_GetAnmTbl( u32 no );

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
 * FLDMMDL_BLACTCONT ビルボードアクター管理　セットアップ
 * @param	fmmdlsys	構築済みのFLDMMDLSYS
 * @param	pBbdActSys	構築済みのGFL_BBDACT_SYS
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_BLACTCONT_Setup( FLDMMDLSYS *fmmdlsys,
	GFL_BBDACT_SYS *pBbdActSys, int res_max )
{
	HEAPID heapID;
	GFL_BBDACT_ACTDATA *actData;
	GFL_BBDACT_ACTUNIT_ID actUnitID;
	FLDMMDL_BLACTCONT *pBlActCont;
	
	heapID = FLDMMDLSYS_GetHeapID( fmmdlsys );
	pBlActCont = GFL_HEAP_AllocClearMemory(
			heapID, sizeof(FLDMMDL_BLACTCONT) );
	pBlActCont->fmmdlsys = fmmdlsys;
	pBlActCont->pBbdActSys = pBbdActSys;
//	pBlActCont->bbdActResCount = tbl_max;
	FLDMMDLSYS_SetBlActCont( fmmdlsys, pBlActCont );
	
	BBDResUnitIndex_Init( fmmdlsys, res_max );
}

#if 0
	{
		int i;
		const OBJCODE_PARAM *prm;
		GFL_BBDACT_RESDATA *res_tbl;
		res_tbl = GFL_HEAP_AllocClearMemory(
				heapID, sizeof(GFL_BBDACT_RESDATA)*tbl_max );
		
		for( i = 0; i < tbl_max; i++ ){
			prm = FLDMMDLSYS_GetOBJCodeParam( fmmdlsys, pOBJCodeTbl[i] );
			res_tbl[i].arcID = ARCID_FLDMMDL_RES;
			res_tbl[i].datID = prm->res_idx;
			res_tbl[i].texFmt = GFL_BBD_TEXFMT_PAL16;
			res_tbl[i].texSiz = prm->tex_size;
			switch( prm->mdl_size ){
			case 0:
			default:
				res_tbl[i].celSizX = 32;
				res_tbl[i].celSizY = 32;
			}
			res_tbl[i].dataCut = GFL_BBDACT_RESTYPE_DATACUT;
		}
		
		pBlActCont->bbdActResUnitID = GFL_BBDACT_AddResourceUnit(
				pBbdActSys, res_tbl, pBlActCont->bbdActResCount );
	
		GFL_HEAP_FreeMemory( res_tbl );
	}
	
	FLDMMDLSYS_SetBlActCont( fmmdlsys, pBlActCont );
}
#endif

//--------------------------------------------------------------
/**
 * FLDMMDL_BLACTCONT ビルボードアクター管理　削除
 * @param	fmmdlsys	FLDMMDLSYS
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_BLACTCONT_Release( FLDMMDLSYS *fmmdlsys )
{
	FLDMMDL_BLACTCONT *pBlActCont = FLDMMDLSYS_GetBlActCont( fmmdlsys );
#if 0	
	GFL_BBDACT_RemoveResourceUnit( pBlActCont->pBbdActSys,
		pBlActCont->bbdActResUnitID, pBlActCont->bbdActResCount );
#else
	BBDResUnitIndex_Delete( fmmdlsys );
#endif
	
	GFL_HEAP_FreeMemory( pBlActCont );
	FLDMMDLSYS_SetBlActCont( fmmdlsys, NULL );
}

//======================================================================
//	フィールド動作モデル　ビルボードリソース
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMMDL_BLACTCONT リソース追加 レギュラー、VRAM常駐型で登録
 * @param	fmmdlsys	FLDMMDLSYS
 * @param	code 表示コード配列
 * @param	max code要素数
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_BLACTCONT_AddResourceTex(
	FLDMMDLSYS *fmmdlsys, const u16 *code, int max )
{
	while( max ){
		BBDResUnitIndex_AddResUnit( fmmdlsys, *code, BBDRES_VRAM_REGULAR );
		code++;
		max--;
	}
}

//======================================================================
//	フィールド動作モデル ビルボードアクター追加
//======================================================================
//--------------------------------------------------------------
/**
 * ビルボードアクター 追加
 * @param	fmmdl	FLDMMDL
 * @param	resID	追加するアクターで使用するリソースID
 * @retval	GFL_BBDACT_ACTUNIT_ID
 */
//--------------------------------------------------------------
GFL_BBDACT_ACTUNIT_ID FLDMMDL_BLACTCONT_AddActor( FLDMMDL *fmmdl, u32 code )
{
  u16 flag;
	VecFx32 pos;
	GFL_BBDACT_ACTDATA actData;
	GFL_BBDACT_ACTUNIT_ID actID;
	const FLDMMDL_BBDACT_ANMTBL *anmTbl;
	const OBJCODE_PARAM *prm;
	FLDMMDLSYS *fmmdlsys = (FLDMMDLSYS*)FLDMMDL_GetFldMMdlSys( fmmdl );
	FLDMMDL_BLACTCONT *pBlActCont = FLDMMDLSYS_GetBlActCont( fmmdlsys );
	
	prm = FLDMMDLSYS_GetOBJCodeParam( fmmdlsys, code );
	FLDMMDL_GetDrawVectorPos( fmmdl, &pos );
	
	BBDResUnitIndex_SearchResID( fmmdlsys, code, &actData.resID, &flag );
	
  if( actData.resID == REGIDCODE_MAX ){ //非登録リソース
    FLDMMDL_BLACTCONT_AddOBJCodeRes( fmmdlsys, code,  FALSE, TRUE );
	  BBDResUnitIndex_SearchResID( fmmdlsys, code, &actData.resID, &flag );
    GF_ASSERT( actData.resID != REGIDCODE_MAX );
    
    #ifdef DEBUG_FLDMMDL
    KAGAYA_Printf( "FLDMMDL ADD GUEST RESOURCE %xH\n", code );
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
  if( code == HERO ){
//    GFL_BBDACT_BindActTexRes( pBlActCont->pBbdActSys, actID, actData.resID );
    GFL_BBDACT_BindActTexRes( pBlActCont->pBbdActSys, actID, actData.resID );
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

//--------------------------------------------------------------
/**
 * ビルボードアクター 削除
 * @param	fmmdl	FLDMMDL*
 * @param	actID GFL_BBDACT_ACTUNIT_ID
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_BLACTCONT_DeleteActor( FLDMMDL *fmmdl, u32 actID )
{
  u16 id,flag;
  u16 code = FLDMMDL_GetOBJCode( fmmdl );
	FLDMMDLSYS *pFMMdlSys = (FLDMMDLSYS*)FLDMMDL_GetFldMMdlSys( fmmdl );
	FLDMMDL_BLACTCONT *pBlActCont = FLDMMDLSYS_GetBlActCont( pFMMdlSys );
  
  if( BBDResUnitIndex_SearchResID(pFMMdlSys,code,&id,&flag) == TRUE ){
    if( (flag&BBDRESBIT_GUEST) ){ //ゲスト登録
      if( FLDMMDL_SearchUseOBJCode(fmmdl,code) == FALSE ){
        BBDResUnitIndex_RemoveResUnit( pFMMdlSys, code );
        
        #ifdef DEBUG_FLDMMDL
        KAGAYA_Printf( "FLDMMDL DEL GUEST RESOURCE %xH\n", code );
        #endif
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
 * @param	pBlActCont FLDMMDL_BLACTCONT
 * @retval	GFL_BBDACT_SYS
 */
//--------------------------------------------------------------
GFL_BBDACT_SYS * FLDMMDL_BLACTCONT_GetBbdActSys(
		FLDMMDL_BLACTCONT *pBlActCont )
{
	return( pBlActCont->pBbdActSys );
}

//--------------------------------------------------------------
/**
 * ビルボードアクター管理 GFL_BBDACT_RESUNIT_ID取得
 * @param	pBlActCont FLDMMDL_BLACTCONT
 * @retval	GFL_BBDACT_RESUNIT_ID
 */
//--------------------------------------------------------------
GFL_BBDACT_RESUNIT_ID FLDMMDL_BLACTCONT_GetResUnitID(
		FLDMMDL_BLACTCONT *pBlActCont )
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
 * @param	pBlActCont FLDMMDL_BLACTCONT
 * @param	max	登録最大数
 * @retval	nothing
 */
//--------------------------------------------------------------
static void BBDResUnitIndex_Init( FLDMMDLSYS *fmmdlsys, int max )
{
	HEAPID heapID = FLDMMDLSYS_GetHeapID( fmmdlsys );
	FLDMMDL_BLACTCONT *pBlActCont = FLDMMDLSYS_GetBlActCont( fmmdlsys );
	IDCodeIndex_Init( &pBlActCont->BBDResUnitIdx, max, heapID );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT 削除
 * @param	pBlActCont FLDMMDL_BLACTCONT
 * @param	max	登録最大数
 * @retval	nothing
 */
//--------------------------------------------------------------
static void BBDResUnitIndex_Delete( FLDMMDLSYS *fmmdlsys )
{
	int i = 0;
	FLDMMDL_BLACTCONT *pBlActCont = FLDMMDLSYS_GetBlActCont( fmmdlsys );
	IDCODE *tbl = pBlActCont->BBDResUnitIdx.pIDCodeBuf;
	
	do{
		if( tbl->code != REGIDCODE_MAX ){
			BBDResUnitIndex_RemoveResUnit( fmmdlsys, tbl->code );
		}
		tbl++;
		i++;
	}while( i < pBlActCont->BBDResUnitIdx.max );
	
	IDCodeIndex_Delete( &pBlActCont->BBDResUnitIdx );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT リソースユニット追加
 * @param	pBlActCont FLDMMDL_BLACTCONT
 * @param	code	OBJコード
 * @param flag 登録フラグ BBDRESBIT_VRAM等
 * @retval	nothing
 */
//--------------------------------------------------------------
static void BBDResUnitIndex_AddResUnit(
    FLDMMDLSYS *fmmdlsys, u16 obj_code, BBDRESBIT flag )
{
	GFL_BBDACT_RESDATA data;
	const OBJCODE_PARAM *prm;
	GFL_BBDACT_RESUNIT_ID id;
	FLDMMDL_BLACTCONT *pBlActCont = FLDMMDLSYS_GetBlActCont( fmmdlsys );
	
	prm = FLDMMDLSYS_GetOBJCodeParam( fmmdlsys, obj_code );
	data.arcID = ARCID_FLDMMDL_RES;
	data.datID = prm->res_idx;
	data.texFmt = GFL_BBD_TEXFMT_PAL16;
	data.texSiz = prm->tex_size;
	data.celSizX = 32;				//いずれmdl_sizeから
	data.celSizY = 32;
  
#if 0	
  case GX_TEXFMT_PLTT16:
		*celDataSiz = 0x20 * res->celSizX/8 * res->celSizY/8;
		break;
	case GX_TEXFMT_PLTT256:
		*celDataSiz = 0x40 * res->celSizX/8 * res->celSizY/8;
		break;
	case GX_TEXFMT_PLTT4:
		*celDataSiz = 0x10 * res->celSizX/8 * res->celSizY/8;
#endif
  
	data.dataCut = GFL_BBDACT_RESTYPE_DATACUT;
  
#if 0 //転送用リソース
  if( obj_code == HERO ){
    data.dataCut = GFL_BBDACT_RESTYPE_TRANSSRC;
  }
#endif
  
	id = GFL_BBDACT_AddResourceUnit( pBlActCont->pBbdActSys, &data, 1 );
  
	IDCodeIndex_RegistCode( &pBlActCont->BBDResUnitIdx, obj_code, id, flag );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT 指定OBJコードのリソースユニット削除
 * @param	pBlActCont FLDMMDL_BLACTCONT
 * @param	code	OBJコード
 * @retval	nothing
 */
//--------------------------------------------------------------
static void BBDResUnitIndex_RemoveResUnit(
		FLDMMDLSYS *fmmdlsys, u16 obj_code )
{
	GFL_BBDACT_RESUNIT_ID id;
	FLDMMDL_BLACTCONT *pBlActCont = FLDMMDLSYS_GetBlActCont( fmmdlsys );
  BOOL ret = IDCodeIndex_SearchCode(
      &pBlActCont->BBDResUnitIdx, obj_code, &id, NULL );
	GF_ASSERT( ret == TRUE );
	GFL_BBDACT_RemoveResourceUnit( pBlActCont->pBbdActSys, id, 1 );
	IDCodeIndex_DeleteCode( &pBlActCont->BBDResUnitIdx, obj_code );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT　指定コードのリソースIDを取得
 * @param	pBlActCont FLDMMDL_BLACTCONT
 * @param	code	OBJコード
 * @retval	BOOL TRUE=指定コード登録有り
 */
//--------------------------------------------------------------
static BOOL BBDResUnitIndex_SearchResID(
    FLDMMDLSYS *fmmdlsys, u16 obj_code, u16 *outID, u16 *outFlag )
{
	FLDMMDL_BLACTCONT *pBlActCont = FLDMMDLSYS_GetBlActCont( fmmdlsys );
  BOOL ret = IDCodeIndex_SearchCode(
    &pBlActCont->BBDResUnitIdx, obj_code, outID, outFlag );
  return( ret );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT 指定コードのリソースIDが追加済みかチェック
 * @param	fmmdlsys	FLDMMDLSYS
 * @param	code OBJコード
 * @retval	BOOL	TRUE=追加済み FALSE=追加まだ
 */
//--------------------------------------------------------------
BOOL FLDMMDL_BLACTCONT_CheckOBJCodeRes( FLDMMDLSYS *fmmdlsys, u16 code )
{
	u16 res;
	FLDMMDL_BLACTCONT *pBlActCont = FLDMMDLSYS_GetBlActCont( fmmdlsys );
	
	if( IDCodeIndex_SearchCode(&pBlActCont->BBDResUnitIdx,code,NULL,NULL) ){
    return( TRUE );
  }
  
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT 指定コードのリソースを追加
 * @param	fmmdlssy	FLDMMDLSYS
 * @param	code	OBJコード
 * @param trans TRUE=テクスチャ転送型で登録
 * @param guest TRUE=使用されなくなると破棄されるゲストタイプで登録
 * @retval	BOOL	TRUE=追加した。FALSE=既に追加済み
 */
//--------------------------------------------------------------
BOOL FLDMMDL_BLACTCONT_AddOBJCodeRes(
    FLDMMDLSYS *fmmdlsys, u16 code, BOOL trans, BOOL guest )
{
	if( FLDMMDL_BLACTCONT_CheckOBJCodeRes(fmmdlsys,code) == FALSE ){
    BBDRESBIT flag = 0;
    if( trans ){ flag |= BBDRESBIT_TRANS; }
    if( guest ){ flag |= BBDRESBIT_GUEST; }
		BBDResUnitIndex_AddResUnit( fmmdlsys, code, flag );
		return( TRUE );
	}
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT 指定コードのリソースを削除
 * @param	fmmdlssy	FLDMMDLSYS
 * @param	code	OBJコード
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_BLACTCONT_DeleteOBJCodeRes( FLDMMDLSYS *fmmdlsys, u16 code )
{
	BBDResUnitIndex_RemoveResUnit( fmmdlsys, code );
}

//======================================================================
//	parts
//======================================================================
//--------------------------------------------------------------
/**
 * テーブル番号からアクターアニメテーブル取得
 * @param	no	FLDMMDL_BLACT_ANMTBLNO_MAX
 * @retval	FLDMMDL_BBDACT_ANMTBL*
 */
//--------------------------------------------------------------
static const FLDMMDL_BBDACT_ANMTBL * BlActAnm_GetAnmTbl( u32 no )
{
	GF_ASSERT( no < FLDMMDL_BLACT_ANMTBLNO_MAX );
	return( &DATA_FLDMMDL_BBDACT_ANM_ListTable[no] );
}

//======================================================================
//	test data
//======================================================================
/*
static const GFL_BBDACT_RESDATA testResTable[] = {
#ifdef FLDMMDL_BLACT_HEAD3_TEST
	{ ARCID_FLDMMDL_BTX, NARC_fldmmdl_btx_obj_kabi32_nsbtx, 
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
#define FLDMMDL_CHECK_GFL_BBD_TEXSIZDEF ///<定義でチェック有効

#ifdef FLDMMDL_CHECK_GFL_BBD_TEXSIZDEF
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
#endif //FLDMMDL_CHECK_GFL_BBD_TEXSIZDEF
