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
static void IDCodeIndex_RegistCode( IDCODEIDX *idx, u16 code, u16 id );
static void IDCodeIndex_DeleteCode( IDCODEIDX *idx, u16 code );
static u16 IDCodeIndex_SearchCode( IDCODEIDX *idx, u16 code );
static void BBDResUnitIndex_Init( FLDMMDLSYS *fmmdlsys, int max );
static void BBDResUnitIndex_Delete( FLDMMDLSYS *fmmdlsys );
static void BBDResUnitIndex_AddResUnit( FLDMMDLSYS *fmmdlsys, u16 obj_code );
static void BBDResUnitIndex_RemoveResUnit(FLDMMDLSYS *fmmdlsys,u16 obj_code);
static u16 BBDResUnitIndex_SearchResID( FLDMMDLSYS *fmmdlsys, u16 obj_code );

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
 * FLDMMDL_BLACTCONT リソース追加 test
 * @param	fmmdlsys	FLDMMDLSYS
 * @param	code
 * @param	max
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_BLACTCONT_AddResourceTex(
	FLDMMDLSYS *fmmdlsys, const u16 *code, int max )
{
	while( max ){
		BBDResUnitIndex_AddResUnit( fmmdlsys, *code );
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
	VecFx32 pos;
	GFL_BBDACT_ACTDATA actData;
	GFL_BBDACT_ACTUNIT_ID actID;
	const FLDMMDL_BBDACT_ANMTBL *anmTbl;
	const OBJCODE_PARAM *prm;
	FLDMMDLSYS *fmmdlsys = (FLDMMDLSYS*)FLDMMDL_GetFldMMdlSys( fmmdl );
	FLDMMDL_BLACTCONT *pBlActCont = FLDMMDLSYS_GetBlActCont( fmmdlsys );
	
	prm = FLDMMDLSYS_GetOBJCodeParam( fmmdlsys, code );
	FLDMMDL_GetDrawVectorPos( fmmdl, &pos );
	
	actData.resID = BBDResUnitIndex_SearchResID( fmmdlsys, code );
	GF_ASSERT( actData.resID != REGIDCODE_MAX );

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
	FLDMMDLSYS *pFMMdlSys = (FLDMMDLSYS*)FLDMMDL_GetFldMMdlSys( fmmdl );
	FLDMMDL_BLACTCONT *pBlActCont = FLDMMDLSYS_GetBlActCont( pFMMdlSys );
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
 * IDCODEIDX 指定されたコードとIDを登録
 * @param	idx IDCODEIDX
 * @param	code コード
 * @param	id ID
 * @retval	nothing
 */
//--------------------------------------------------------------
static void IDCodeIndex_RegistCode( IDCODEIDX *idx, u16 code, u16 id )
{
	int i = 0;
	IDCODE *tbl = idx->pIDCodeBuf;
	do{
		if( tbl->code == REGIDCODE_MAX ){
			tbl->code = code;
			tbl->id = id;
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
			return;
		}
		tbl++;
		i++;
	}while( i < idx->max );
	GF_ASSERT( 0 );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX 指定されたコードに一致するIDを返す
 * @param	idx IDCODEIDX
 * @param	code OBJコード
 * @retval	u16 codeに一致したID REGIDCODE_MAX=一致無し
 */
//--------------------------------------------------------------
static u16 IDCodeIndex_SearchCode( IDCODEIDX *idx, u16 code )
{
	int i = 0;
	IDCODE *tbl = idx->pIDCodeBuf;
	do{
		if( tbl->code == code ){
			return( tbl->id );
		}
		tbl++;
		i++;
	}while( i < idx->max );
	return( REGIDCODE_MAX );
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
 * @retval	nothing
 */
//--------------------------------------------------------------
static void BBDResUnitIndex_AddResUnit( FLDMMDLSYS *fmmdlsys, u16 obj_code )
{
	GFL_BBDACT_RESDATA data;
	const OBJCODE_PARAM *prm;
	GFL_BBDACT_RESUNIT_ID id;
	FLDMMDL_BLACTCONT *pBlActCont = FLDMMDLSYS_GetBlActCont( fmmdlsys );
	
	prm = FLDMMDLSYS_GetOBJCodeParam( fmmdlsys, obj_code );
	data.arcID = ARCID_FLDMMDL_RES;
	data.datID = prm->res_idx;
	data.texFmt = GFL_BBD_TEXFMT_PAL16;
//	data.texSiz = prm->tex_size;	//古い値 借りたい書
	if( obj_code == HERO ){
		data.texSiz = GFL_BBD_TEXSIZ_32x1024;
	}else{
		data.texSiz = GFL_BBD_TEXSIZ_32x512;
	}
	data.celSizX = 32;	//いずれmdl_sizeから
	data.celSizY = 32;
	data.dataCut = GFL_BBDACT_RESTYPE_DATACUT;
	id = GFL_BBDACT_AddResourceUnit( pBlActCont->pBbdActSys, &data, 1 );
	IDCodeIndex_RegistCode( &pBlActCont->BBDResUnitIdx, obj_code, id );
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
	id = IDCodeIndex_SearchCode( &pBlActCont->BBDResUnitIdx, obj_code );
	GF_ASSERT( id != REGIDCODE_MAX );
	GFL_BBDACT_RemoveResourceUnit( pBlActCont->pBbdActSys, id, 1 );
	IDCodeIndex_DeleteCode( &pBlActCont->BBDResUnitIdx, obj_code );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT　指定コードのリソースIDを取得
 * @param	pBlActCont FLDMMDL_BLACTCONT
 * @param	code	OBJコード
 * @retval	u16	リソースID
 */
//--------------------------------------------------------------
static u16 BBDResUnitIndex_SearchResID( FLDMMDLSYS *fmmdlsys, u16 obj_code )
{
	FLDMMDL_BLACTCONT *pBlActCont = FLDMMDLSYS_GetBlActCont( fmmdlsys );
	return( IDCodeIndex_SearchCode(&pBlActCont->BBDResUnitIdx,obj_code) );
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
	
	res = IDCodeIndex_SearchCode( &pBlActCont->BBDResUnitIdx, code );

	if( res == REGIDCODE_MAX ){
		return( FALSE );
	}

	return( TRUE );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT 指定コードのリソースを追加
 * @param	fmmdlssy	FLDMMDLSYS
 * @param	code	OBJコード
 * @retval	BOOL	TRUE=追加した。FALSE=既に追加済み
 */
//--------------------------------------------------------------
BOOL FLDMMDL_BLACTCONT_AddOBJCodeRes( FLDMMDLSYS *fmmdlsys, u16 code )
{
	if( FLDMMDL_BLACTCONT_CheckOBJCodeRes(fmmdlsys,code) == FALSE ){
		BBDResUnitIndex_AddResUnit( fmmdlsys, code );
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
