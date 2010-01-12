//======================================================================
/**
 * @file	fieldobj_draw.c
 * @brief	フィールド動作モデル 描画系
 * @author	kagaya
 * @date	05.07.25
 *
 */
//======================================================================
#include "fldmmdl.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	struct
//======================================================================

//======================================================================
//	proto
//======================================================================
static void MMdlSys_ArcHandleOpen( MMDLSYS *fos );
static void MMdlSys_ArcHandleClose( MMDLSYS *fos );

//======================================================================
//	フィールド動作モデル　描画システム
//======================================================================
//--------------------------------------------------------------
/**
 * FIELD OBJ 描画系初期化
 * @param	fos			MMDLSYS *
 * @param	tex_max		テクスチャを登録できる最大人数
 * @param	reg_tex_max	tex_max中、常に常駐する最大人数
 * @param	tex_tbl		常に常駐する人物をまとめたテーブル BABYBOY1等
 * @param	frm_trans_max	1フレームで転送できるデータ最大数 add pl
 * @retval	nothing
 */
//--------------------------------------------------------------
void MMDLSYS_InitDraw( MMDLSYS *fos )
{
	#ifndef MMDL_PL_NULL
	int max,pri;
	MMDL_BLACT_CONT *cont;
	
	GF_ASSERT( MMDLSYS_CheckCompleteDrawInit(fos) == FALSE );
	MMdlSys_ArcHandleOpen( fos );
	
	max = MMDLSYS_GetMMdlMax( fos );
	pri = MMDLSYS_GetTCBPriority( fos ) - 1; //追加処理は標準より上
	cont = MMDLSYS_BlActContGet( fos );
	
	MMDL_BlActCont_Init(
		cont, fos, max, pri, tex_max, reg_tex_max, tex_tbl, frm_trans_max );
	MMDLSYS_StatusBit_ON( fos, MMDLSYS_STABIT_DRAW_INIT_COMP );
	#else
	MMDLSYS_SetCompleteDrawInit( fos, TRUE );
	#endif
}

//--------------------------------------------------------------
/**
 * FIELD OBJ 描画処理を削除
 * @param	fos		MMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
void MMDLSYS_DeleteDraw( MMDLSYS *fos )
{
	#ifndef MMDL_PL_NULL
	GF_ASSERT( MMDLSYS_CheckCompleteDrawInit(fos) == TRUE );
	
	MMDL_BlActCont_Delete( MMDLSYS_BlActContGet(fos) );
	
	MMDLSYS_StatusBit_OFF( fos, MMDLSYS_STABIT_DRAW_INIT_COMP );
	MMdlSys_ArcHandleClose( fos );
	#else
  if( MMDLSYS_GetBlActCont(fos) != NULL ){
		MMDL_BLACTCONT_Release( fos );
	}

  if( MMDLSYS_GetG3dObjCont(fos) != NULL ){
    MMDL_G3DOBJCONT_Delete( fos );
  }
  
	MMDLSYS_SetCompleteDrawInit( fos, FALSE );
	#endif
}

//======================================================================
//	アーカイブ
//======================================================================
//--------------------------------------------------------------
/**
 * アーカイブハンドルセット
 * @param	fos		MMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void MMdlSys_ArcHandleOpen( MMDLSYS *fos )
{
#ifndef MMDL_PL_NULL
	ARCHANDLE *handle = ArchiveDataHandleOpen( ARC_MMODEL, HEAPID_FIELD );
	MMDLSYS_SetArcHandle( fos, handle );
#endif
}

//--------------------------------------------------------------
/**
 * アーカイブハンドルクローズ
 * @param	fos		MMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void MMdlSys_ArcHandleClose( MMDLSYS *fos )
{
#ifndef MMDL_PL_NULL
	ARCHANDLE *handle = MMDLSYS_GetArcHandle( fos );
	ArchiveDataHandleClose( handle );
#endif
}

//======================================================================
//	フィールド動作モデル 描画処理
//======================================================================
//--------------------------------------------------------------
/**
 * フィールド動作モデル描画
 * @param	fmmdl		MMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void MMDL_UpdateDraw( MMDL * fmmdl )
{
	const MMDLSYS *fos = MMDL_GetMMdlSys( fmmdl );
	
	if( MMDLSYS_CheckStatusBit(fos,MMDLSYS_STABIT_DRAW_PROC_STOP) ){
		return; //システムで描画停止が発生している
	}
	
	if( MMDL_CheckMoveBitCompletedDrawInit(fmmdl) == FALSE ){
		return; //初期化が完了していない
	}
  
  #if 0 //old dp	
  /*
   * 旧来、動作ポーズ中は描画処理もポーズしていたが
   * ポーズを呼び出すイベント側としては色々と不都合が多く
   * 使い勝手が悪かった為これを廃止。
   * 描画処理は常に呼び出し、ポーズは各描画処理側で対応する。
   */
	if( MMDL_CheckMoveBitMoveProcPause(fmmdl) == FALSE ||
		MMDL_CheckStatusBitAcmd(fmmdl) ){
		MMDL_CallDrawProc( fmmdl );
	}
  #else //new wb
	MMDL_CallDrawProc( fmmdl );
  #endif
}

//======================================================================
//	フィールド動作モデル 描画ステータス
//======================================================================
#ifndef MMDL_PL_NULL
//--------------------------------------------------------------
/**
 * OBJコードから描画ステータス取得
 * @param	code	表示コード HERO等
 * @retval	OBJCODE_STATE*
 */
//--------------------------------------------------------------
const OBJCODE_STATE * MMDL_TOOL_GetOBJCodeState( u16 code )
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
const OBJCODE_STATE * MMDL_GetOBJCodeState( const MMDL *fmmdl )
{
	return( MMDL_TOOL_GetOBJCodeState(MMDL_GetOBJCode(fmmdl)) );
}
#endif

//======================================================================
//	parts
//======================================================================
//--------------------------------------------------------------
/**
 * フィールド動作モデル 描画ポーズチェック
 * @param	fmmdl	MMDL *
 * @retval	int		TRUE=描画ポーズ
 */
//--------------------------------------------------------------
BOOL MMDL_CheckDrawPause( const MMDL * fmmdl )
{
  if( MMDL_CheckMoveBitMoveProcPause(fmmdl) ){
		if( MMDL_CheckMoveBitAcmd(fmmdl) == FALSE ){
			return( TRUE );
		}
	}
	
	if( MMDL_CheckStatusBit(fmmdl,MMDL_STABIT_PAUSE_ANM) ){
		return( TRUE );
	}

	return( FALSE );
}

//--------------------------------------------------------------
/**
 * アーカイブデータロード
 * @param	fos		MMDLSYS *
 * @param	datID	アーカイブデータID
 * @param	fb TRUE=GFL_HEAP_AllocClearMemory() FALSE=GFL_HEAP_AllocClearMemoryLo()
 * @retval	void*	取得したデータ
 */
//--------------------------------------------------------------
void * MMDL_DrawArcDataAlloc( const MMDLSYS *fos, u32 datID, int fb )
{
#ifndef MMDL_PL_NULL
	void *buf;
	ARCHANDLE *handle = MMDLSYS_GetArcHandle( fos );
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
 * @param	fmmdl	MMDL *
 * @param	vec		座標格納先
 * @retval	void*	取得したデータ
 */
//--------------------------------------------------------------
void MMDL_GetDrawVectorPos( const MMDL * fmmdl, VecFx32 *vec )
{
	VecFx32 vec_pos,vec_offs,vec_out,vec_attr,vec_ctrl;
	
	MMDL_GetVectorPos( fmmdl, &vec_pos );
	MMDL_GetVectorDrawOffsetPos( fmmdl, &vec_offs );
	MMDL_GetVectorOuterDrawOffsetPos( fmmdl, &vec_out );
	MMDL_GetVectorAttrDrawOffsetPos( fmmdl, &vec_attr );
  MMDL_GetControlOffsetPos( fmmdl, &vec_ctrl );
	vec->x = vec_pos.x + vec_offs.x + vec_out.x + vec_attr.x + vec_ctrl.x;
	vec->y = vec_pos.y + vec_offs.y + vec_out.y + vec_attr.y + vec_ctrl.y;
	vec->z = vec_pos.z + vec_offs.z + vec_out.z + vec_attr.z + vec_ctrl.z;
}
