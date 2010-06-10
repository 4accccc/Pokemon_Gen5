//=============================================================================
/**
 *
 *	@file		demo3d_cmd.c
 *	@brief  3Dデモコマンド
 *	@author	hosaka genya → miyuki iwasawa
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
#include "gamesystem/msgspeed.h"

//アーカイブ
#include "arc_def.h"
#include "message.naix"
#include "demo3d_cmd.h"

#include "sound/pm_sndsys.h" // for SEQ_SE_XXX

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
#define CMD_ELEM_MAX 9999  ///< コマンドループ抜け用

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
static void cmd_SystemWorkInit( DEMO3D_CMD_WORK* wk );
static void cmd_SystemWorkRelease( DEMO3D_CMD_WORK* wk );
static BOOL cmd_setup( DEMO3D_CMD_WORK* wk, DEMO3D_ENGINE_WORK* core, int* out_idx );
static void cmd_exec( DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, const DEMO3D_CMD_DATA* data );

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
 *	@param	heapID 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
DEMO3D_CMD_WORK* Demo3D_CMD_Init( DEMO3D_ENGINE_WORK* core, HEAPID heapID )
{
  DEMO3D_CMD_WORK* wk;
  
  GF_ASSERT_MSG( GFL_HEAP_CheckHeapSafe( heapID ) == TRUE,"Demo3D HeapError!\n" );
  GF_ASSERT_MSG( GFL_HEAP_CheckHeapSafe( 0 ) == TRUE,"System HeapError!\n" );

  // メインワーク アロケーション
  wk = GFL_HEAP_AllocClearMemory( heapID, sizeof( DEMO3D_CMD_WORK ) );

  // メンバ初期化
  wk->core = core;
  wk->heapID = heapID;
  wk->tmpHeapID = GFL_HEAP_LOWID(heapID);
  wk->demo_id = core->env.demo_id;
  wk->pre_frame = -1;
  
  cmd_SystemWorkInit( wk );

  //イニシャルコマンド列実行
  wk->is_cmd_end = cmd_setup( wk, core, &wk->cmd_idx );

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
  DEMO3D_ENGINE_WORK* core = wk->core;
  const DEMO3D_CMD_DATA* data;
  int i;
  
  
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
      cmd_exec( wk, core, data );
      data++;
    }
  }

  //ここで残っているタスクを自殺させる
  wk->cmdsys_end_f = TRUE;
  GFL_TCBL_Main( wk->tcbsys );

  //システムワークを解放
  cmd_SystemWorkRelease( wk );

  // ヒープ開放
  GFL_HEAP_FreeMemory( wk );
#if 0  
  GF_ASSERT_MSG( GFL_HEAP_CheckHeapSafe( 0 ) == TRUE,"System HeapError!\n" );
  GF_ASSERT_MSG( GFL_HEAP_CheckHeapSafe( HEAPID_DEMO3D ) == TRUE,"Demo3D HeapError!\n" );
#endif
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
void Demo3D_CMD_Main( DEMO3D_CMD_WORK* wk, fx32 now_frame, u32 delayCount )
{
  DEMO3D_ENGINE_WORK* core = wk->core;
  const DEMO3D_CMD_DATA* data;

  //遅延フレームかどうかチェック
  wk->delay_f = (delayCount>0);

  // TCB主処理
  GFL_TCBL_Main( wk->tcbsys );

  // コマンドを末端までなめたら、コマンドサーチはもうしない
  if( wk->is_cmd_end || (now_frame == (-FX32_ONE))){
    return;
  }
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
      cmd_exec( wk, core, data );
      // 次のコマンドがENDならフラグを立てる
      data++;
      wk->is_cmd_end = ( data->type == DEMO3D_CMD_TYPE_END );

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
 *	@brief  3Dデモコマンドコントローラー システムワーク初期化 
 *
 *	@param	DEMO3D_CMD_WORK* wk 
 *	@retval
 */
//-----------------------------------------------------------------------------
static void cmd_SystemWorkInit( DEMO3D_CMD_WORK* wk )
{
  // TCB初期化
  wk->tcbsys = GFL_TCBL_Init( wk->heapID, wk->heapID, 16, 128 );

	wk->fontHandle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
		GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );
	
  wk->h_2dgra	= GFL_ARC_OpenDataHandle( ARCID_DEMO3D_2DGRA, wk->heapID );

  //メッセージスピード取得
  wk->msg_spd  = MSGSPEED_GetWait();
}

//-----------------------------------------------------------------------------
/**
 *	@brief  3Dデモコマンドコントローラー システムワーク解放
 *
 *	@param	DEMO3D_CMD_WORK* wk 
 *	@retval
 */
//-----------------------------------------------------------------------------
static void cmd_SystemWorkRelease( DEMO3D_CMD_WORK* wk )
{
  ///////////////////////////////////////////////////
  //デモ中断時のコマンド処理破棄

  //モーションブラーが生きていたら削除
  if( wk->mb != NULL ){
    DEMO3D_MotionBlExit( wk->mb );
  }
  //今生きている全てのタスクを削除
  GFL_TCBL_DeleteAll( wk->tcbsys );

  //BGM Push/Popチェック
  if( wk->bgm_push_f ){ 
    if( wk->core->end_result == DEMO3D_RESULT_FINISH ){
      GF_ASSERT_MSG(0,"BGMがpopされずに終了します\nX+Y+L+R同時押しでアサートを抜けて、コマンド列を見直してください\n");
    }
    PMSND_PopBGM();
    PMSND_PauseBGM(FALSE);
    PMSND_FadeInBGM( 60 );
  }

  ///////////////////////////////////////////////////
  //システムワーク破棄
  GFL_ARC_CloseDataHandle( wk->h_2dgra );
  GFL_FONT_Delete(wk->fontHandle);

  // TCB削除
  GFL_TCBL_Exit( wk->tcbsys );
}

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
static BOOL cmd_setup( DEMO3D_CMD_WORK* wk, DEMO3D_ENGINE_WORK* core, int* out_idx )
{
  int i;
  const DEMO3D_CMD_DATA* data;
  
  data = Demo3D_DATA_GetCmdData( core->env.demo_id );

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
      cmd_exec( wk, core, &data[i] );
    }
    else if( data[i].frame >= core->env.start_frame )
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
static void cmd_exec( DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, const DEMO3D_CMD_DATA* data )
{
  GF_ASSERT( data->type != DEMO3D_CMD_TYPE_NULL );
  GF_ASSERT( data->type < DEMO3D_CMD_TYPE_END );
  
#if 1
  OS_TPrintf("call cmd type=%d [%d,%d,%d,%d]\n", data->type,
      data->param[0],
      data->param[1],
      data->param[2],
      data->param[3]
      );
#endif

  DATA_Demo3D_CmdTable[ data->type ]( wk, core, data->param );
}


