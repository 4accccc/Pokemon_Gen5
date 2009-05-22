//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		fld_scenearea.c
 *	@brief  フィールド　特殊シーン領域管理
 *	@author	tomoya takahashi
 *	@data		2009.05.22
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
#define FLD_SCENEAREA_ACTIVE_NONE (0xffffffff)

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
  u32 datanum;

  // 起動フラグ
  u32 active_area;
};

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


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
void FLD_SCENEAREA_Load( FLD_SCENEAREA* p_sys, const FLD_SCENEAREA_DATA* cp_data, u32 datanum )
{
  // 情報があるかもしれないのでRelease
  FLD_SCENEAREA_Release( p_sys );

  p_sys->cp_data = cp_data;
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
  p_sys->datanum      = 0;
  p_sys->active_area  = FLD_SCENEAREA_ACTIVE_NONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  シーンエリア　更新
 *
 *	@param	p_sys     システム
 *	@param	cp_pos    自機位置
 */
//-----------------------------------------------------------------------------
void FLD_SCENEAREA_Updata( FLD_SCENEAREA* p_sys, const VecFx32* cp_pos )
{
  int i;
  u32 now_active;
  
  // エリアチェック
  now_active = FLD_SCENEAREA_ACTIVE_NONE;
  for( i=0; i<p_sys->datanum; i++ ){
    
    // チェック
    GF_ASSERT( p_sys->cp_data[i].p_checkArea );
    if( p_sys->cp_data[i].p_checkArea( p_sys, &p_sys->cp_data[i], cp_pos ) ){

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
    if( now_active != FLD_SCENEAREA_ACTIVE_NONE ){
      if( p_sys->cp_data[now_active].p_inside ){
        p_sys->cp_data[now_active].p_inside( p_sys, &p_sys->cp_data[now_active], cp_pos );
      }
    }

    if( p_sys->active_area != FLD_SCENEAREA_ACTIVE_NONE ){
      if( p_sys->cp_data[p_sys->active_area].p_outside ){
        p_sys->cp_data[p_sys->active_area].p_outside( p_sys, &p_sys->cp_data[p_sys->active_area], cp_pos );
      }
    }

    p_sys->active_area = now_active;
  }
  else if( p_sys->active_area != FLD_SCENEAREA_ACTIVE_NONE )
  {

    // 更新処理
    if( p_sys->cp_data[p_sys->active_area].p_updata ){
      p_sys->cp_data[p_sys->active_area].p_updata( p_sys, &p_sys->cp_data[p_sys->active_area], cp_pos );
    }
  }
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
