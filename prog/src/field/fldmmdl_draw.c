//======================================================================
/**
 * @file	fieldobj_draw.c
 * @brief	フィールド動作モデル 描画系
 * @author	kagaya
 * @data	05.07.25
 *
 */
//======================================================================
#include "fldmmdl.h"
#include "fldmmdl_procdraw.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	struct
//======================================================================

//======================================================================
//	proto
//======================================================================
static void FldMMdlSys_ArcHandleOpen( FLDMMDLSYS *fos );
static void FldMMdlSys_ArcHandleClose( FLDMMDLSYS *fos );

//======================================================================
//	フィールド動作モデル　描画システム
//======================================================================
//--------------------------------------------------------------
/**
 * FIELD OBJ 描画系初期化
 * @param	fos			FLDMMDLSYS *
 * @param	tex_max		テクスチャを登録できる最大人数
 * @param	reg_tex_max	tex_max中、常に常駐する最大人数
 * @param	tex_tbl		常に常駐する人物をまとめたテーブル BABYBOY1等
 * @param	frm_trans_max	1フレームで転送できるデータ最大数 add pl
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_InitDraw( FLDMMDLSYS *fos,
		int tex_max, int reg_tex_max,
		const int *tex_tbl, int frm_trans_max )
{
	#ifndef FLDMMDL_PL_NULL
	int max,pri;
	FLDMMDL_BLACT_CONT *cont;
	
	GF_ASSERT( FLDMMDLSYS_CheckCompleteDrawInit(fos) == FALSE );
	FldMMdlSys_ArcHandleOpen( fos );
	
	max = FLDMMDLSYS_GetFldMMdlMax( fos );
	pri = FLDMMDLSYS_GetTCBPriority( fos ) - 1; //追加処理は標準より上
	cont = FLDMMDLSYS_BlActContGet( fos );
	
	FLDMMDL_BlActCont_Init(
		cont, fos, max, pri, tex_max, reg_tex_max, tex_tbl, frm_trans_max );
	FLDMMDLSYS_StatusBit_ON( fos, FLDMMDLSYS_STABIT_DRAW_INIT_COMP );
	#endif
}

//--------------------------------------------------------------
/**
 * FIELD OBJ 描画処理を削除
 * @param	fos		FLDMMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_DeleteDraw( FLDMMDLSYS *fos )
{
	#ifndef FLDMMDL_PL_NULL
	GF_ASSERT( FLDMMDLSYS_CheckCompleteDrawInit(fos) == TRUE );
	
	FLDMMDL_BlActCont_Delete( FLDMMDLSYS_BlActContGet(fos) );
	
	FLDMMDLSYS_StatusBit_OFF( fos, FLDMMDLSYS_STABIT_DRAW_INIT_COMP );
	FldMMdlSys_ArcHandleClose( fos );
	#endif
}

//======================================================================
//	アーカイブ
//======================================================================
//--------------------------------------------------------------
/**
 * アーカイブハンドルセット
 * @param	fos		FLDMMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdlSys_ArcHandleOpen( FLDMMDLSYS *fos )
{
#ifndef FLDMMDL_PL_NULL
	ARCHANDLE *handle = ArchiveDataHandleOpen( ARC_MMODEL, HEAPID_FIELD );
	FLDMMDLSYS_SetArcHandle( fos, handle );
#endif
}

//--------------------------------------------------------------
/**
 * アーカイブハンドルクローズ
 * @param	fos		FLDMMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdlSys_ArcHandleClose( FLDMMDLSYS *fos )
{
#ifndef FLDMMDL_PL_NULL
	ARCHANDLE *handle = FLDMMDLSYS_GetArcHandle( fos );
	ArchiveDataHandleClose( handle );
#endif
}

//======================================================================
//	フィールド動作モデル 描画処理
//======================================================================
//--------------------------------------------------------------
/**
 * フィールド動作モデル描画
 * @param	fmmdl		FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_UpdateDraw( FLDMMDL * fmmdl )
{
	const FLDMMDLSYS *fos = FLDMMDL_GetFldMMdlSys( fmmdl );
	
	if( FLDMMDLSYS_CheckStatusBit(fos,FLDMMDLSYS_STABIT_DRAW_PROC_STOP) ){
		return;
	}
	
	if( FLDMMDL_CheckStatusBitCompletedDrawInit(fmmdl) == FALSE ){
		return;
	}
	
	if( FLDMMDL_CheckStatusBitMoveProcPause(fmmdl) == FALSE ||
		FLDMMDL_CheckStatusBitAcmd(fmmdl) ){
		FLDMMDL_CallDrawProc( fmmdl );
	}
}

//======================================================================
//	フィールド動作モデル 描画ステータス
//======================================================================
//--------------------------------------------------------------
/**
 * OBJコードから描画ステータス取得
 * @param	code	表示コード HERO等
 * @retval	OBJCODE_STATE*
 */
//--------------------------------------------------------------
const OBJCODE_STATE * FLDMMDL_TOOL_GetOBJCodeState( u16 code )
{
	const OBJCODE_STATE *state = DATA_FieldOBJCodeDrawStateTbl;
	
	do{
		if( state->code == code ){
			return( state );
		}
		state++;
	}while( state->code != OBJCODEMAX );
	
	GF_ASSERT( 0 );
	return( NULL );
}

//--------------------------------------------------------------
/**
 * 動作モデルOBJコードから描画ステータス取得
 * @param	code	表示コード HERO等
 * @retval	OBJCODE_STATE*
 */
//--------------------------------------------------------------
const OBJCODE_STATE * FLDMMDL_GetOBJCodeState( const FLDMMDL *fmmdl )
{
	return( FLDMMDL_TOOL_GetOBJCodeState(FLDMMDL_GetOBJCode(fmmdl)) );
}

//======================================================================
//	parts
//======================================================================
//--------------------------------------------------------------
/**
 * フィールド動作モデル 描画ポーズチェック
 * @param	fmmdl	FLDMMDL *
 * @retval	int		TRUE=描画ポーズ
 */
//--------------------------------------------------------------
BOOL FLDMMDL_CheckDrawPause( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_CheckStatusBitMoveProcPause(fmmdl) == TRUE ){
		if( FLDMMDL_CheckStatusBitAcmd(fmmdl) == FALSE ){
			return( TRUE );
		}
	}
	
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_PAUSE_ANM) ){
		return( TRUE );
	}
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * アーカイブデータロード
 * @param	fos		FLDMMDLSYS *
 * @param	datID	アーカイブデータID
 * @param	fb TRUE=GFL_HEAP_AllocClearMemory() FALSE=GFL_HEAP_AllocClearMemoryLo()
 * @retval	void*	取得したデータ
 */
//--------------------------------------------------------------
void * FLDMMDL_DrawArcDataAlloc( const FLDMMDLSYS *fos, u32 datID, int fb )
{
#ifndef FLDMMDL_PL_NULL
	void *buf;
	ARCHANDLE *handle = FLDMMDLSYS_GetArcHandle( fos );
	u32 size = ArchiveDataSizeGetByHandle( handle, datID );
	
	if( fb == TRUE ){
		buf = GFL_HEAP_AllocClearMemory( HEAPID_FIELD, size );
	}else{
		buf = GFL_HEAP_AllocClearMemoryLo( HEAPID_FIELD, size );
	}
	
	#ifdef DEBUG_ONLY_FOR_kagaya
	{
	 u32 free = sys_GetHeapFreeSize( HEAPID_FIELD );
	 OS_Printf( "fmmdl DrawArcData datID(%d) FIELD HEAP FreeSize %xH\n",
			 datID, free );
	}
	#endif
	
	ArchiveDataLoadByHandle( handle, datID, buf );
	return( buf );
#else
	return( NULL );
#endif
}

//--------------------------------------------------------------
/**
 * フィールド動作モデル　基本座標＋オフセットを取得
 * @param	fmmdl	FLDMMDL *
 * @param	vec		座標格納先
 * @retval	void*	取得したデータ
 */
//--------------------------------------------------------------
void FLDMMDL_GetDrawVectorPos( const FLDMMDL * fmmdl, VecFx32 *vec )
{
	VecFx32 vec_pos,vec_offs,vec_out,vec_attr;
	
	FLDMMDL_GetVectorPos( fmmdl, &vec_pos );
	FLDMMDL_GetVectorDrawOffsetPos( fmmdl, &vec_offs );
	FLDMMDL_GetVectorOuterDrawOffsetPos( fmmdl, &vec_out );
	FLDMMDL_GetVectorAttrDrawOffsetPos( fmmdl, &vec_attr );
	vec->x = vec_pos.x + vec_offs.x + vec_out.x + vec_attr.x;
	vec->y = vec_pos.y + vec_offs.y + vec_out.y + vec_attr.y;
	vec->z = vec_pos.z + vec_offs.z + vec_out.z + vec_attr.z;
}

//======================================================================
//	描画無し
//======================================================================
//--------------------------------------------------------------
/**
 * 描画無し　初期化
 * @param	fmmdl		FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawNon_Init( FLDMMDL * fmmdl )
{
	FLDMMDL_SetStatusBitVanish( fmmdl, TRUE );
	FLDMMDL_OnStatusBit( fmmdl, FLDMMDL_STABIT_SHADOW_VANISH );
}

//--------------------------------------------------------------
/**
 * 描画無し　描画
 * @param	fmmdl		FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawNon_Draw( FLDMMDL * fmmdl )
{
}

//--------------------------------------------------------------
/**
 * 描画無し　削除
 * @param	fmmdl	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawNon_Delete( FLDMMDL * fmmdl )
{
}

//--------------------------------------------------------------
/**
 * 描画無し　退避
 * @param	fmmdl		FLDMMDL * 
 * @retval	int			TRUE=初期化成功
 */
//--------------------------------------------------------------
void FLDMMDL_DrawNon_Push( FLDMMDL * fmmdl )
{
}

//--------------------------------------------------------------
/**
 * 描画無し　復帰
 * 退避した情報を元に再描画。
 * @param	fmmdl		FLDMMDL * 
 * @retval	int			TRUE=初期化成功
 */
//--------------------------------------------------------------
void FLDMMDL_DrawNon_Pop( FLDMMDL * fmmdl )
{
}

