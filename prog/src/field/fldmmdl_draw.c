//******************************************************************************
/**
 *
 * @file	fieldobj_draw.c
 * @brief	フィールド動作モデル 描画系
 * @author	kagaya
 * @data	05.07.25
 *
 */
//******************************************************************************
#include "fldmmdl.h"

//==============================================================================
//	define
//==============================================================================
//#ifdef PM_DEBUG
//#define DEBUG_DRAW
//#endif

//==============================================================================
//	プロトタイプ
//==============================================================================
static void fmmdlsys_ArcHandleOpen( FLDMMDLSYS *fos );
static void fmmdlsys_ArcHandleClose( FLDMMDLSYS *fos );

static const OBJCODE_STATE * fmmdl_OBJCodeDrawStateGet( int code );

#ifdef DEBUG_DRAW
static void DEBUG_Print( void );
#endif

//==============================================================================
//	フィールド動作モデル　描画システム
//==============================================================================
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
void FLDMMDLSYS_DrawInit( FLDMMDLSYS *fos,
		int tex_max, int reg_tex_max,
		const int *tex_tbl, int frm_trans_max )
{
	int max,pri;
	FLDMMDL_BLACT_CONT *cont;
	
	GF_ASSERT( FLDMMDLSYS_StatusBit_Check(fos,FLDMMDLSYS_STABIT_DRAW_INIT_COMP) == 0 &&
			"FLDMMDLSYS_DrawInit()が重ねて呼び出れています\n" );
	
	fmmdlsys_ArcHandleOpen( fos );
	
	max = FLDMMDLSYS_OBJMaxGet( fos );
	pri = FLDMMDLSYS_TCBStandardPriorityGet( fos ) - 1; //ゲストビルボード追加処理は標準より上
	cont = FLDMMDLSYS_BlActContGet( fos );
	
#if 0
	FLDMMDL_BlActCont_Init( cont, fos, max, pri, tex_max, reg_tex_max, tex_tbl, frm_trans_max );
#endif

#if 0	
	{
		FLDMMDL_RENDER_CONT_PTR rdcont = FLDMMDL_RenderCont_Init( fos, HEAPID_FIELD );
		FLDMMDLSYS_RenderContSet( fos, rdcont );
		FLDMMDL_RenderCont_RegistInit( rdcont, DATA_FIELDOBJ_RenderOBJMax );
	}
#endif
	
	FLDMMDLSYS_StatusBit_ON( fos, FLDMMDLSYS_STABIT_DRAW_INIT_COMP );
	
	OS_Printf( "フィールド動作モデル 描画処理を初期化しました\n" );
	
#ifdef DEBUG_DRAW	
	DEBUG_Print();
#endif
}

//--------------------------------------------------------------
/**
 * FIELD OBJ 描画処理を削除
 * @param	fos		FLDMMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_DrawDelete( FLDMMDLSYS *fos )
{
	int ret;
	
	ret = FLDMMDLSYS_DrawInitCompCheck( fos );
#if 0
	GF_ASSERT( ret == TRUE && "フィールド動作モデル 描画処理削除が二重に呼び出されています\n" );
#else
	if( ret == TRUE ){
	}
#endif

#if 0
	FLDMMDL_BlActCont_Delete( FLDMMDLSYS_BlActContGet(fos) );
#endif

#if 0	
	{
		FLDMMDL_RENDER_CONT_PTR rdcont = FLDMMDLSYS_RenderContGet( fos );
		FLDMMDL_RenderCont_Delete( rdcont );
		FLDMMDLSYS_RenderContSet( fos, NULL );
	}
#endif
	
	FLDMMDLSYS_StatusBit_OFF( fos, FLDMMDLSYS_STABIT_DRAW_INIT_COMP );
	fmmdlsys_ArcHandleClose( fos );
	
	OS_Printf( "フィールド動作モデル 描画処理を削除しました\n" );
}

//==============================================================================
//	アーカイブ
//==============================================================================
//--------------------------------------------------------------
/**
 * アーカイブハンドルセット
 * @param	fos		FLDMMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdlsys_ArcHandleOpen( FLDMMDLSYS *fos )
{
#if 0
	ARCHANDLE *handle = ArchiveDataHandleOpen( ARC_MMODEL, HEAPID_FIELD );
	FLDMMDLSYS_ArcHandleSet( fos, handle );
#endif
}

//--------------------------------------------------------------
/**
 * アーカイブハンドルクローズ
 * @param	fos		FLDMMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdlsys_ArcHandleClose( FLDMMDLSYS *fos )
{
#if 0
	ARCHANDLE *handle = FLDMMDLSYS_ArcHandleGet( fos );
	ArchiveDataHandleClose( handle );
#endif
}

//==============================================================================
//	フィールド動作モデル 描画処理
//==============================================================================
//--------------------------------------------------------------
/**
 * フィールド動作モデル描画
 * @param	fmmdl		FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_Draw( FLDMMDL * fmmdl )
{
	const FLDMMDLSYS *fos = FLDMMDL_FieldOBJSysGet( fmmdl );
	
	if( FLDMMDLSYS_StatusBit_Check(fos,FLDMMDLSYS_STABIT_DRAW_PROC_STOP) ){
		return;
	}
	
	if( FLDMMDL_StatusBit_Check(fmmdl,FLDMMDL_STABIT_DRAW_PROC_INIT_COMP) == FALSE ){
		return;
	}
	
	if( FLDMMDL_MovePauseCheck(fmmdl) == FALSE || FLDMMDL_StatusBitCheck_Acmd(fmmdl) ){
		FLDMMDL_DrawProcCall( fmmdl );
	}
}

//==============================================================================
//	フィールド動作モデル 描画ステータス
//==============================================================================
//--------------------------------------------------------------
/**
 * OBJコードから描画ステータス取得
 * @param	fmmdl	 FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
const OBJCODE_STATE * FLDMMDL_OBJCodeDrawStateGet( const FLDMMDL * fmmdl )
{
	int code = FLDMMDL_OBJCodeGet( fmmdl );
	const OBJCODE_STATE *state = fmmdl_OBJCodeDrawStateGet( code );
	return( state );
}

//--------------------------------------------------------------
/**
 * DATA_FieldOBJCodeDrawStateTbl[]取得
 * @param	code	HERO等
 * @retval	OBJCODE_STATE codeに対応したOBJCODE_STATE
 */
//--------------------------------------------------------------
static const OBJCODE_STATE * fmmdl_OBJCodeDrawStateGet( int code )
{
	const OBJCODE_STATE *state = DATA_FieldOBJCodeDrawStateTbl;
	do{ if(state->code==code){return(state);} state++; }while( state->code != OBJCODEMAX );
	OS_Printf( "fmmdl_OBJCodeDrawStateGet() OBJ CODE ERROR!! CODE = 0x%x\n", code );
	GF_ASSERT( 0 );
	return( NULL );
}

//==============================================================================
//	パーツ
//==============================================================================
//--------------------------------------------------------------
/**
 * フィールド動作モデル 描画ポーズチェック
 * @param	fmmdl	FLDMMDL *
 * @retval	int		TRUE=描画ポーズ
 */
//--------------------------------------------------------------
int FLDMMDL_DrawPauseCheck( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_MovePauseCheck(fmmdl) == TRUE ){
		if( FLDMMDL_StatusBitCheck_Acmd(fmmdl) == FALSE ){
			return( TRUE );
		}
	}
	
	if( FLDMMDL_StatusBit_Check(fmmdl,FLDMMDL_STABIT_PAUSE_ANM) ){
		return( TRUE );
	}
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * アーカイブデータロード
 * @param	fos		FLDMMDLSYS *
 * @param	datID	アーカイブデータID
 * @param	fb		TRUE=GFL_HEAP_AllocClearMemory() FALSE=GFL_HEAP_AllocClearMemoryLo()
 * @retval	void*	取得したデータ
 */
//--------------------------------------------------------------
void * FLDMMDL_DrawArcDataAlloc( const FLDMMDLSYS *fos, u32 datID, int fb )
{
#if 0
	void *buf;
	ARCHANDLE *handle = FLDMMDLSYS_ArcHandleGet( fos );
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
void FLDMMDL_DrawVecPosTotalGet( const FLDMMDL * fmmdl, VecFx32 *vec )
{
	VecFx32 vec_pos,vec_offs,vec_out,vec_attr;
	
	FLDMMDL_VecPosGet( fmmdl, &vec_pos );
	FLDMMDL_VecDrawOffsGet( fmmdl, &vec_offs );
	FLDMMDL_VecDrawOffsOutSideGet( fmmdl, &vec_out );
	FLDMMDL_VecAttrOffsGet( fmmdl, &vec_attr );
	
	vec->x = vec_pos.x + vec_offs.x + vec_out.x + vec_attr.x;
	vec->y = vec_pos.y + vec_offs.y + vec_out.y + vec_attr.y;
	vec->z = vec_pos.z + vec_offs.z + vec_out.z + vec_attr.z;
}

//--------------------------------------------------------------
/**
 * 方向をセットする。描画系も併せて更新
 * @param	fmmdl	FLDMMDL *
 * @param	dir		方向、DIR_UP等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DirDispDrawSet( FLDMMDL * fmmdl, int dir )
{
	FLDMMDL_DirDispCheckSet( fmmdl, dir );
	
	if( FLDMMDL_StatusBitCheck_DrawProcInitComp(fmmdl) == TRUE ){
		FLDMMDL_DrawProcCall( fmmdl );
	}
}

//==============================================================================
//	描画無し
//==============================================================================
//--------------------------------------------------------------
/**
 * 描画無し　初期化
 * @param	fmmdl		FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawNon_Init( FLDMMDL * fmmdl )
{
	FLDMMDL_StatusBitSet_Vanish( fmmdl, TRUE );
	FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_SHADOW_VANISH );
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

//==============================================================================
//	debug
//==============================================================================
#ifdef DEBUG_DRAW
#include "fieldobj_test.c"

static void DEBUG_Print( void )
{
	u32 i;
	
	OS_Printf( "const OBJCODE_STATE DATA_FieldOBJCodeStateTbl[] =\n" );
	OS_Printf( "{\r\n" );
	
	for( i = 0; i <= KOIKING; i++ ){
		OS_Printf( " {" );
		OS_Printf( "%s,", DEBUGDATA_CharTbl[i] );
		OS_Printf( "FLDMMDL_DRAWTYPE_BLACT," );
		OS_Printf( "FLDMMDL_SHADOW_ON," );
		OS_Printf( "FLDMMDL_FOOTMARK_NORMAL," );
		OS_Printf( "FLDMMDL_REFLECT_BLACT," );
		OS_Printf( "0},\r\n" );
		{ u32 f; for( f = 0; f < 0xffffffff; f++ ){}; }
	}
	
	{
		int j;
		for( j = 0; j < 2; j++ ){};
	}
}
#endif	//DEBUG_DRAW
