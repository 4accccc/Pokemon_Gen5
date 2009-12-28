//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		fld_scenearea.c
 *	@brief  フィールド　特殊シーン領域管理
 *	@author	tomoya takahashi
 *	@date		2009.05.22
 *
 *	モジュール名：FLD_SCENEAREA
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "gflib.h"

#include "fld_scenearea.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	シーンエリア
//=====================================
struct _FLD_SCENEAREA {
  // カメラ
  FIELD_CAMERA* p_camera;
  
  // データ
  const FLD_SCENEAREA_DATA* cp_data;
  const FLD_SCENEAREA_FUNC* cp_func;
  u32 datanum;

  // 起動フラグ
  u32 active_area;

	// 動作フラグ
	u32 active_flag;


	// コール関数ID
	u32 call_funcID;
};

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
static BOOL FUNC_IsAreaCheckFunc( const FLD_SCENEAREA_FUNC* cp_func, u32 id );
static BOOL FUNC_IsUpdateFunc( const FLD_SCENEAREA_FUNC* cp_func, u32 id );

static BOOL FUNC_CallAreaCheckFunc( const FLD_SCENEAREA_FUNC* cp_func, u32 id, const FLD_SCENEAREA* cp_sys, const FLD_SCENEAREA_DATA* cp_data, const VecFx32* cp_pos );
static void FUNC_CallUpdateFunc( const FLD_SCENEAREA_FUNC* cp_func, u32 id, const FLD_SCENEAREA* cp_sys, const FLD_SCENEAREA_DATA* cp_data, const VecFx32* cp_pos );

//----------------------------------------------------------------------------
/**
 *	@brief  シーンエリア　作成
 *
 *	@param	heapID      ヒープID
 *	@param	p_camera    カメラワーク
 *
 *	@return シーンエリア管理システム
 */
//-----------------------------------------------------------------------------
FLD_SCENEAREA* FLD_SCENEAREA_Create( u32 heapID, FIELD_CAMERA * p_camera )
{
  FLD_SCENEAREA* p_sys;

  p_sys = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLD_SCENEAREA) );

  p_sys->p_camera = p_camera;

  // アクティブ領域なし
  p_sys->active_area = FLD_SCENEAREA_ACTIVE_NONE;

  p_sys->active_flag = TRUE;

  return p_sys;
}

//----------------------------------------------------------------------------
/**
 *	@brief  情報の破棄
 *
 *	@param	p_sys   システムワーク
 */
//-----------------------------------------------------------------------------
void FLD_SCENEAREA_Delete( FLD_SCENEAREA* p_sys )
{
  FLD_SCENEAREA_Release( p_sys );
  GFL_HEAP_FreeMemory( p_sys );
}


//----------------------------------------------------------------------------
/**
 *	@brief  シーンエリア  情報設定
 *
 *	@param	p_sys     システムワーク
 *	@param	cp_data   シーンエリア情報
 *	@param	datanum   エリア数
 *	@param	heapID    ヒープID
 */
//-----------------------------------------------------------------------------
void FLD_SCENEAREA_Load( FLD_SCENEAREA* p_sys, const FLD_SCENEAREA_DATA* cp_data, u32 datanum, const FLD_SCENEAREA_FUNC* cp_func )
{
  // 情報があるかもしれないのでRelease
  FLD_SCENEAREA_Release( p_sys );

  p_sys->cp_data = cp_data;
  p_sys->cp_func = cp_func;
  p_sys->datanum = datanum;

  // アクティブ領域なし
  p_sys->active_area = FLD_SCENEAREA_ACTIVE_NONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  シーンエリア  情報破棄
 *
 *	@param	p_sys システムワーク
 */
//-----------------------------------------------------------------------------
void FLD_SCENEAREA_Release( FLD_SCENEAREA* p_sys )
{
  p_sys->cp_data      = NULL;
	p_sys->cp_func			= NULL;
  p_sys->datanum      = 0;
  p_sys->active_area  = FLD_SCENEAREA_ACTIVE_NONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  シーンエリア　更新
 *
 *	@param	p_sys     システム
 *	@param	cp_pos    自機位置
 *
 *	@retval FLD_SCENEAREA_UPDATE_NONE 更新なし
 *	@retval その他                    更新したデータのインデックス
 */
//-----------------------------------------------------------------------------
u32 FLD_SCENEAREA_Update( FLD_SCENEAREA* p_sys, const VecFx32* cp_pos )
{
  int i;
  u32 now_active;
	BOOL result;

  if( !p_sys->active_flag )
  {
    return FLD_SCENEAREA_UPDATE_NONE;
  }

	p_sys->call_funcID = FLD_SCENEAREA_FUNC_NULL;
  
  // エリアチェック
  now_active = FLD_SCENEAREA_ACTIVE_NONE;
  for( i=0; i<p_sys->datanum; i++ ){
    
    // チェック
		result = FUNC_CallAreaCheckFunc( p_sys->cp_func, p_sys->cp_data[i].checkArea_func,
				p_sys, &p_sys->cp_data[i], cp_pos );
    if( result ){

      // エリアがかぶっています。
      GF_ASSERT_MSG( now_active == FLD_SCENEAREA_ACTIVE_NONE, "エリア範囲がかぶっています。インデックス%d,%d\n", now_active, i );
      now_active = i;
    }
  }

  // エリアインデックスの変更をチェック
  // 今は、エリアが変わったときは更新処理を呼ばないようにしています。
  // 呼ぶか呼ばないか選べるようにすることも考えています。
  if( now_active != p_sys->active_area )
  {
    if( p_sys->active_area != FLD_SCENEAREA_ACTIVE_NONE ){

			FUNC_CallUpdateFunc( p_sys->cp_func, 
					p_sys->cp_data[p_sys->active_area].outside_func, 
					p_sys, &p_sys->cp_data[p_sys->active_area], cp_pos );
			p_sys->call_funcID = p_sys->cp_data[p_sys->active_area].outside_func;
    }

    
    if( now_active != FLD_SCENEAREA_ACTIVE_NONE ){

			FUNC_CallUpdateFunc( p_sys->cp_func, 
					p_sys->cp_data[now_active].inside_func, 
					p_sys, &p_sys->cp_data[now_active], cp_pos );
			p_sys->call_funcID = p_sys->cp_data[now_active].inside_func;
    }

    p_sys->active_area = now_active;
  }
  else if( p_sys->active_area != FLD_SCENEAREA_ACTIVE_NONE )
  {

    // 更新処理
		FUNC_CallUpdateFunc( p_sys->cp_func, 
				p_sys->cp_data[p_sys->active_area].update_func, 
				p_sys, &p_sys->cp_data[p_sys->active_area], cp_pos );
		p_sys->call_funcID = p_sys->cp_data[p_sys->active_area].update_func;
  }

  return p_sys->active_area;
}

//----------------------------------------------------------------------------
/**
 *	@brief  エリア　読み込み済みか？
 *
 *	@param	cp_sys  システム
 *
 *	@retval TRUE  読み込み済み* 
 *	@retval FLASE 読み込んでない
 */
//-----------------------------------------------------------------------------
BOOL FLD_SCENEAREA_IsLoad( const FLD_SCENEAREA* cp_sys )
{
  GF_ASSERT( cp_sys );
  if( cp_sys->datanum == 0 )
  {
    return FALSE;
  }
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  アクティブエリアの取得
 *
 *	@param	cp_sys  システム
 *
 *	@retval アクティブエリア
 *  @retval FLD_SCENEAREA_ACTIVE_NONE なし
 */
//-----------------------------------------------------------------------------
u32 FLD_SCENEAREA_GetActiveArea( const FLD_SCENEAREA* cp_sys )
{
  GF_ASSERT( cp_sys );
  return cp_sys->active_area;
}

//----------------------------------------------------------------------------
/**
 *	@brief	更新処理で使用した関数のIDの取得
 *
 *	@param	cp_sys	システム
 *
 *	@retval	関数ID
 *	@retval	FLD_SCENEAREA_FUNC_NULL	何もコールしてない
 */
//-----------------------------------------------------------------------------
u32 FLD_SCENEAREA_GetUpdateFuncID( const FLD_SCENEAREA* cp_sys )
{
	GF_ASSERT( cp_sys );
	return cp_sys->call_funcID;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アクティブフラグ設定
 *
 *	@param	p_sys		システム
 *	@param	flag		フラグ
 */	
//-----------------------------------------------------------------------------
void FLD_SCENEAREA_SetActiveFlag( FLD_SCENEAREA* p_sys, BOOL flag )
{
	GF_ASSERT( p_sys );
	p_sys->active_flag = flag;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アクティブフラグの取得
 *
 *	@param	cp_sys	システム
 *
 *	@retval	TRUE	
 *	@retval	FALSE	
 */
//-----------------------------------------------------------------------------
BOOL FLD_SCENEAREA_GetActiveFlag( const FLD_SCENEAREA* cp_sys )
{
	GF_ASSERT( cp_sys );
	return cp_sys->active_flag;
}




//----------------------------------------------------------------------------
/**
 *	@brief  フィールドカメラの取得
 *
 *	@param	cp_sys  システム
 *
 *	@return フィールドカメラ
 */
//-----------------------------------------------------------------------------
FIELD_CAMERA* FLD_SCENEAREA_GetFieldCamera( const FLD_SCENEAREA* cp_sys )
{
  return cp_sys->p_camera;
}




//-----------------------------------------------------------------------------
/**
 *			プライベート関数
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief	エリアチェック関数　があるかチェック
 *
 *	@param	cp_func	関数情報
 *	@param	id			ID
 *
 *	@retval	TRUE	ある
 *	@retval	FALSE	ない
 */
//-----------------------------------------------------------------------------
static BOOL FUNC_IsAreaCheckFunc( const FLD_SCENEAREA_FUNC* cp_func, u32 id )
{
	GF_ASSERT( cp_func );
	if( cp_func->checkarea_count > id )
	{
		if( cp_func->cpp_checkArea[ id ] )
		{
			return TRUE;
		}
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	通常アップデート関数　があるかチェック
 *
 *	@param	cp_func	関数情報
 *	@param	id			ID
 *
 *	@retval	TRUE	ある
 *	@retval	FALSE	ない
 */
//-----------------------------------------------------------------------------
static BOOL FUNC_IsUpdateFunc( const FLD_SCENEAREA_FUNC* cp_func, u32 id )
{
	GF_ASSERT( cp_func );
	if( cp_func->update_count > id )
	{
		if( cp_func->cpp_update[ id ] )
		{
			return TRUE;
		}
	}
	return FALSE;
}



//----------------------------------------------------------------------------
/**
 *	@brief	エリアチェック関数コール
 */
//-----------------------------------------------------------------------------
static BOOL FUNC_CallAreaCheckFunc( const FLD_SCENEAREA_FUNC* cp_func, u32 id, const FLD_SCENEAREA* cp_sys, const FLD_SCENEAREA_DATA* cp_data, const VecFx32* cp_pos )
{
	if( FUNC_IsAreaCheckFunc( cp_func, id ) )
	{
		return cp_func->cpp_checkArea[ id ]( cp_sys, cp_data, cp_pos );
	}

	// だめ
	// エリアチェック関数はあるべき
	GF_ASSERT_MSG( 0, "エリアチェック関数はあるべき。" );
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アップデート
 */	
//-----------------------------------------------------------------------------
static void FUNC_CallUpdateFunc( const FLD_SCENEAREA_FUNC* cp_func, u32 id, const FLD_SCENEAREA* cp_sys, const FLD_SCENEAREA_DATA* cp_data, const VecFx32* cp_pos )
{
	if( FUNC_IsUpdateFunc( cp_func, id ) )
	{
		cp_func->cpp_update[ id ]( cp_sys, cp_data, cp_pos );
	}
}




