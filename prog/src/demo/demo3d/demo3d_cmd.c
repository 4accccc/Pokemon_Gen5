//=============================================================================
/**
 *
 *	@file		demo3d_cmd.c
 *	@brief  3Dデモコマンド
 *	@author	hosaka genya
 *	@data		2009.12.09
 *
 */
//=============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

// 文字列関連
#include "font/font.naix"
#include "print/gf_font.h"
#include "print/wordset.h"
#include "print/printsys.h" //PRINT_QUE

//アーカイブ
#include "arc_def.h"

#include "demo3d_graphic.h"

//データ
#include "demo3d_data.h"

#include "demo3d_cmd.h"

#include "sound/pm_sndsys.h" // for SEQ_SE_XXX

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
enum
{ 
  CMD_ELEM_MAX = 9999,  ///< コマンドループ抜け用
};

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
//--------------------------------------------------------------
///	コマンドワーク
//==============================================================
struct _DEMO3D_CMD_WORK {
  // [IN]
  HEAPID heap_id;
  DEMO3D_ID demo_id;
  // [PRIVATE]
  BOOL  is_cmd_end;
  int cmd_idx;
};

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
static BOOL cmd_setup( DEMO3D_ID id, u32 now_frame, int* out_idx );
static void cmd_exec( const DEMO3D_CMD_DATA* data );


static void CMD_SE( int* param );

// DEMO3D_CMD_TYPE と対応
//--------------------------------------------------------------
///	コマンドテーブル (関数ポインタテーブル)
//==============================================================
static void (*c_cmdtbl[])() = 
{ 
  NULL, // null
  CMD_SE,
  NULL, // end
};

//-----------------------------------------------------------------------------
/**
 *	@brief  SE再生コマンド
 *
 *	@param	int* param コマンドパラメータの先頭ポインタ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void CMD_SE( int* param )
{
  int player_no;

  player_no = param[0];

  GFL_SOUND_PlaySE( param[0] );

  // volume
  if( param[1] != 0 )
  {
    GFL_SOUND_SetVolume( player_no, param[1] );
  }

  // pan
  if( param[2] != 0 )
  {
    GFL_SOUND_SetPan( player_no, 0xFFFF, param[2] );
  }
}


//=============================================================================
/**
 *								外部公開関数
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  3Dデモコマンドコントローラー 初期化
 *
 *	@param	DEMO3D_ID demo_id
 *	@param  start_frame
 *	@param	heap_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
DEMO3D_CMD_WORK* Demo3D_CMD_Init( DEMO3D_ID demo_id, u32 start_frame, HEAPID heap_id )
{
  DEMO3D_CMD_WORK* wk;

  // メインワーク アロケーション
  wk = GFL_HEAP_AllocClearMemory(  heap_id, sizeof( DEMO3D_CMD_WORK ) );

  // メンバ初期化
  wk->heap_id = heap_id;
  wk->demo_id = demo_id;
  wk->is_cmd_end = cmd_setup( demo_id, start_frame, &wk->cmd_idx );

  return wk;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  3Dデモコマンドコントローラー 破棄
 *
 *	@param	DEMO3D_CMD_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void Demo3D_CMD_Exit( DEMO3D_CMD_WORK* wk )
{
  // ヒープ開放
  GFL_HEAP_FreeMemory( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  3Dデモコマンドコントローラー 主処理
 *
 *	@param	DEMO3D_CMD_WORK* wk 
 *	@param  now_frame
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void Demo3D_CMD_Main( DEMO3D_CMD_WORK* wk, fx32 now_frame )
{
  const DEMO3D_CMD_DATA* data;

  // コマンドを末端までなめたら、もうなにもしない。
  if( wk->is_cmd_end )
  {
    return;
  }

  data  = Demo3D_DATA_GetCmdData( wk->demo_id );

  // 頭出し
  data = &data[ wk->cmd_idx ];
    
  GF_ASSERT( data->frame * FX32_ONE >= now_frame );

  // 指定フレームまで待機
  if( data->frame * FX32_ONE == now_frame )
  {
    int i;
    for( i=0; i<CMD_ELEM_MAX; i++ )
    {
      // 実行
      cmd_exec( data );
      // 次のコマンドがENDならフラグを立てる
      data++;
      wk->is_cmd_end = ( data->type == DEMO3D_CMD_TYPE_END );
     
      wk->cmd_idx++;

      // 終了判定 ENDフラグが立っているか、現在のフレームで実行するコマンドが無くなっていたら処理を抜ける
      if( wk->is_cmd_end == TRUE || data->frame * FX32_ONE != now_frame )
      {
        break;
      }
    }
  }
}

//=============================================================================
/**
 *								static関数
 */
//=============================================================================


//-----------------------------------------------------------------------------
/**
 *	@brief  コマンド初期化
 *
 *	@param	DEMO3D_ID id
 *	@param	now_frame 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL cmd_setup( DEMO3D_ID id, u32 now_frame, int* out_idx )
{
  int i;
  const DEMO3D_CMD_DATA* data;
  
  data = Demo3D_DATA_GetCmdData( id );

  for( i=0; i<CMD_ELEM_MAX; i++ )
  {
    GF_ASSERT( data[i].type != DEMO3D_CMD_TYPE_NULL );

    // コマンド終了検出
    if( data[i].type == DEMO3D_CMD_TYPE_END )
    {
      return TRUE;
    }
    //  初期化パラメータを実行
    else if( data[i].frame == -1 )
    {
      cmd_exec( &data[i] );
    }
    else if( data[i].frame * FX32_ONE >= now_frame )
    {
      // 頭出し終了
      *out_idx = i;
      HOSAKA_Printf("setup cmd_idx=%d \n", i);
      return FALSE;
    }
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  コマンド実行
 *
 *	@param	DEMO3D_CMD_DATA* data 
 *
 *	@retval 
 */
//-----------------------------------------------------------------------------
static void cmd_exec( const DEMO3D_CMD_DATA* data )
{
  GF_ASSERT( data->type != DEMO3D_CMD_TYPE_NULL );
  GF_ASSERT( data->type < DEMO3D_CMD_TYPE_END );
      
  OS_TPrintf("call cmd type=%d [%d,%d,%d,%d]\n", data->type,
      data->param[0],
      data->param[1],
      data->param[2],
      data->param[3]
      );
  
  c_cmdtbl[ data->type ]( data->param );
}
