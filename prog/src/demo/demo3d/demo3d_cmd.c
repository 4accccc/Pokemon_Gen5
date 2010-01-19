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
#include "system/brightness.h"

// 文字列関連
#include "font/font.naix"
#include "print/gf_font.h"
#include "print/wordset.h"
#include "print/printsys.h" //PRINT_QUE

//アーカイブ
#include "arc_def.h"

#include "demo3d_graphic.h"
#include "demo3d_mb.h" 

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
///	コマンドに渡すパラメータ
//==============================================================
typedef struct {
  // [IN]
  DEMO3D_CMD_WORK* mwk;
  // [PRIVATE]
  // いまのところなし
} CMD_UNIT;

//--------------------------------------------------------------
///	コマンドワーク
//==============================================================
struct _DEMO3D_CMD_WORK {
  // [IN]
  HEAPID heap_id;
  DEMO3D_ID demo_id;
  // [PRIVATE]
  BOOL  is_cmd_end;
  int pre_frame; ///< 1sync=1
  int cmd_idx;
  DEMO3D_MBL_WORK* mb;
  GFL_TCBSYS*   tcbsys;
  void*         tcbsys_work;
};

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
static BOOL cmd_setup( DEMO3D_CMD_WORK* mwk, DEMO3D_ID id, u32 now_frame, int* out_idx );
static void cmd_exec( DEMO3D_CMD_WORK* mwk, const DEMO3D_CMD_DATA* data );

static void CMD_SE(CMD_UNIT* unit, int* param);
static void CMD_SE_STOP(CMD_UNIT* unit, int* param);
static void CMD_BRIGHTNESS_REQ(CMD_UNIT* unit, int* param);
static void CMD_MOTIONBL_START(CMD_UNIT* unit, int* param);
static void CMD_MOTIONBL_END(CMD_UNIT* unit, int* param);

// DEMO3D_CMD_TYPE と対応
//--------------------------------------------------------------
///	コマンドテーブル (関数ポインタテーブル)
//==============================================================
static void (*c_cmdtbl[ DEMO3D_CMD_TYPE_MAX ])() = 
{ 
  NULL, // null
  CMD_SE,
  CMD_SE_STOP,
  CMD_BRIGHTNESS_REQ,
  CMD_MOTIONBL_START,
  CMD_MOTIONBL_END,
  NULL, // end
};

//-----------------------------------------------------------------------------
/**
 *	@brief  SE再生コマンド
 *
 *	@param	param[0] SE_Label
 *	@param	param[1] volume : 0なら無効
 *	@param	param[2] pan : 0なら無効
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void CMD_SE(CMD_UNIT* unit, int* param)
{
  int player_no;


  GFL_SOUND_PlaySE( param[0] );
  
  player_no = GFL_SOUND_GetPlayerNo( param[0] );

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

//-----------------------------------------------------------------------------
/**
 *	@brief  SE:再生停止
 *
 *	@param	param[0] SE_Label
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void CMD_SE_STOP(CMD_UNIT* unit, int* param)
{
  int player_no;

  player_no = GFL_SOUND_GetPlayerNo( param[0] );

  GFL_SOUND_StopPlayerNo( player_no );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  フラッシュ
 *
 *  @param  param[0]  sync
 *  @param  param[1]  フェード終了時の輝度
 *  @param  param[2]  フェード開始時の輝度
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void CMD_BRIGHTNESS_REQ(CMD_UNIT* unit, int* param)
{
  ChangeBrightnessRequest( param[0], param[1], param[2], (PLANEMASK_BG0|PLANEMASK_BG1|PLANEMASK_BG2|PLANEMASK_BG3|PLANEMASK_OBJ), MASK_MAIN_DISPLAY );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  モーションブラー 開始
 *
 *	@param	unit
 *	@param	param[0] モーションブラー係数 新しくブレンドする絵
 *	@param	param[1] モーションブラー係数 既にバッファされている絵
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void CMD_MOTIONBL_START(CMD_UNIT* unit, int* param)
{
  GF_ASSERT( unit->mwk->mb == NULL );

	unit->mwk->mb = DEMO3D_MotionBlInit( unit->mwk->tcbsys, param[0], param[1] );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  モーションブラー 停止
 *
 *	@param	unit
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void CMD_MOTIONBL_END(CMD_UNIT* unit, int* param)
{
  DEMO3D_MotionBlExit( unit->mwk->mb );
  unit->mwk->mb = NULL;
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
  wk = GFL_HEAP_AllocClearMemory( heap_id, sizeof( DEMO3D_CMD_WORK ) );

  // TCB初期化
  wk->tcbsys_work = GFL_HEAP_AllocClearMemory( heap_id, GFL_TCB_CalcSystemWorkSize(5) );
  wk->tcbsys = GFL_TCB_Init( 5, wk->tcbsys_work );

  // メンバ初期化
  wk->heap_id = heap_id;
  wk->demo_id = demo_id;
  wk->is_cmd_end = cmd_setup( wk, demo_id, start_frame, &wk->cmd_idx );
  wk->pre_frame = -1;

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
  const DEMO3D_CMD_DATA* data;
  int i;
  
  // TCB削除
  GFL_TCB_Exit( wk->tcbsys );
  GFL_HEAP_FreeMemory( wk->tcbsys_work );
  
  data  = Demo3D_DATA_GetEndCmdData( wk->demo_id );

  // 終了コマンドを実行
  for( i=0; i<CMD_ELEM_MAX; i++ )
  {
    if( data->type == DEMO3D_CMD_TYPE_END )
    {
      break;
    }
    else
    {
      cmd_exec( wk, data );
      data++;
    }
  }

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

  // TCB主処理
  GFL_TCB_Main( wk->tcbsys );
  
  data  = Demo3D_DATA_GetCmdData( wk->demo_id );

  // 頭出し
  data = &data[ wk->cmd_idx ];

  now_frame = (now_frame >> FX32_SHIFT); ///< 整数化
    
  GF_ASSERT_MSG( data->frame >= now_frame, "cmd_idx=%d data->frame=%d now_frame=%d ", wk->cmd_idx, data->frame, now_frame );

  // 指定フレームまで待機 && フレームが進まなかった場合はコマンドを処理しない
  if( data->frame == now_frame && wk->pre_frame != now_frame )
  {
    int i;
    for( i=0; i<CMD_ELEM_MAX; i++ )
    {
      // 実行
      cmd_exec( wk, data );
      // 次のコマンドがENDならフラグを立てる
      data++;
      wk->is_cmd_end = ( data->type == DEMO3D_CMD_TYPE_END );

      //@TODO TCBが残っていたらアサート
     
      wk->cmd_idx++;

      // 終了判定 ENDフラグが立っているか、現在のフレームで実行するコマンドが無くなっていたら処理を抜ける
      if( wk->is_cmd_end == TRUE || data->frame != now_frame )
      {
        break;
      }
    }
  }
  
  wk->pre_frame = now_frame;
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
static BOOL cmd_setup( DEMO3D_CMD_WORK* mwk, DEMO3D_ID id, u32 now_frame, int* out_idx )
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
    else if( data[i].frame == DEMO3D_CMD_SYNC_INIT )
    {
      cmd_exec( mwk, &data[i] );
    }
    else if( data[i].frame >= now_frame )
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
static void cmd_exec( DEMO3D_CMD_WORK* mwk, const DEMO3D_CMD_DATA* data )
{
  CMD_UNIT unit = {0};

  GF_ASSERT( data->type != DEMO3D_CMD_TYPE_NULL );
  GF_ASSERT( data->type < DEMO3D_CMD_TYPE_END );
      
  OS_TPrintf("call cmd type=%d [%d,%d,%d,%d]\n", data->type,
      data->param[0],
      data->param[1],
      data->param[2],
      data->param[3]
      );

  unit.mwk = mwk;

  c_cmdtbl[ data->type ]( &unit, data->param );
}


