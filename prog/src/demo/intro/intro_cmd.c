//=============================================================================
/**
 *
 *	@file		intro_cmd.c
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

#include "intro_sys.h"

#include "intro_graphic.h"

//データ
#include "intro_cmd_data.h"

#include "intro_cmd.h"

#include "sound/pm_sndsys.h" // for SEQ_SE_XXX

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
enum
{ 
  STORE_NUM = 8,  ///< 同時実行コマンドの限界数
};


//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
//--------------------------------------------------------------
///	コマンドワーク
//==============================================================
struct _INTRO_CMD_WORK {
  // [IN]
  HEAPID heap_id;
  // [PRIVATE]
  INTRO_SCENE_ID scene_id;
  const INTRO_CMD_DATA* store[ STORE_NUM ];
  int cmd_idx;
};

// コマンド
static BOOL CMD_BG_LOAD( INTRO_CMD_WORK* wk, int* param );
static BOOL CMD_SE( INTRO_CMD_WORK* wk, int* param );
static BOOL CMD_SE_STOP( INTRO_CMD_WORK* wk, int* param);
static BOOL CMD_KEY_WAIT( INTRO_CMD_WORK* wk, int* param );

// INTRO_CMD_TYPE と対応
//--------------------------------------------------------------
///	コマンドテーブル (関数ポインタテーブル)
//==============================================================
static BOOL (*c_cmdtbl[ INTRO_CMD_TYPE_MAX ])() = 
{ 
  NULL, // null
  CMD_BG_LOAD,
  CMD_SE,
  CMD_SE_STOP,
  CMD_KEY_WAIT,
  NULL, // end
};

//-----------------------------------------------------------------------------
/**
 *	@brief  グラフィックロード
 *
 *	@param	INTRO_CMD_WORK* wk
 *	@param	param 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
#include "mictest.naix"
static BOOL CMD_BG_LOAD( INTRO_CMD_WORK* wk, int* param )
{
  //@TODO とりあえずマイクテストのリソース
  HEAPID heap_id;
	ARCHANDLE	*handle;

  heap_id = wk->heap_id;
	handle	= GFL_ARC_OpenDataHandle( ARCID_MICTEST_GRA, heap_id );

	// 上下画面ＢＧパレット転送
	GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_mictest_back_bg_down_NCLR, PALTYPE_MAIN_BG, PLTID_BG_BACK_M, 0x20, heap_id );
	GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_mictest_back_bg_up_NCLR, PALTYPE_SUB_BG, PLTID_BG_BACK_S, 0x20, heap_id );
	
  //	----- 下画面 -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, NARC_mictest_back_bg_down_NCGR,
						BG_FRAME_BACK_S, 0, 0, 0, heap_id );
	GFL_ARCHDL_UTIL_TransVramScreen(	handle, NARC_mictest_back_bg_down_NSCR,
						BG_FRAME_BACK_S, 0, 0, 0, heap_id );	

	//	----- 上画面 -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, NARC_mictest_back_bg_up_NCGR,
						BG_FRAME_BACK_M, 0, 0, 0, heap_id );
	GFL_ARCHDL_UTIL_TransVramScreen(	handle, NARC_mictest_back_bg_up_NSCR,
						BG_FRAME_BACK_M, 0, 0, 0, heap_id );		

	GFL_ARC_CloseDataHandle( handle );

  return TRUE;
}

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
static BOOL CMD_SE( INTRO_CMD_WORK* wk, int* param )
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

  return TRUE;
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
static BOOL CMD_SE_STOP( INTRO_CMD_WORK* wk, int* param)
{
  int player_no;

  player_no = GFL_SOUND_GetPlayerNo( param[0] );

  GFL_SOUND_StopPlayerNo( player_no );

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キーウェイト
 *
 *	@param	INTRO_CMD_WORK* wk
 *	@param	param 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_KEY_WAIT( INTRO_CMD_WORK* wk, int* param )
{
  if( GFL_UI_TP_GetTrg() || GFL_UI_KEY_GetTrg() )
  {
    return TRUE;
  }

  return FALSE;
}

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
static BOOL cmd_store( INTRO_CMD_WORK* wk, const INTRO_CMD_DATA* data );
static BOOL cmd_store_exec( INTRO_CMD_WORK* wk );

//=============================================================================
/**
 *								外部公開関数
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  3Dデモコマンドコントローラー 初期化
 *
 *	@param  start_frame
 *	@param	heap_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
INTRO_CMD_WORK* Intro_CMD_Init( HEAPID heap_id )
{
  INTRO_CMD_WORK* wk;

  // メインワーク アロケーション
  wk = GFL_HEAP_AllocClearMemory(  heap_id, sizeof( INTRO_CMD_WORK ) );

  // メンバ初期化
  wk->heap_id = heap_id;

  return wk;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  3Dデモコマンドコントローラー 破棄
 *
 *	@param	INTRO_CMD_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void Intro_CMD_Exit( INTRO_CMD_WORK* wk )
{

  // ヒープ開放
  GFL_HEAP_FreeMemory( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  3Dデモコマンドコントローラー 主処理
 *
 *	@param	INTRO_CMD_WORK* wk 
 *
 *	@retval TRUE:継続 FALSE:終了
 */
//-----------------------------------------------------------------------------
BOOL Intro_CMD_Main( INTRO_CMD_WORK* wk )
{
  // ストアされたコマンドを実行
  if( cmd_store_exec( wk ) == FALSE )
  {
    int i;

    // もしコマンドがすべて終了していたら次のコマンドを呼び出す
    for( i=0; i<STORE_NUM+1; i++ )
    {
      const INTRO_CMD_DATA* data;

      GF_ASSERT( i < STORE_NUM ); ///< ストア限界チェック
      
      data = Intro_DATA_GetCmdData( wk->scene_id );
      data += wk->cmd_idx;

      // コマンド終了判定
      if( data->type == INTRO_CMD_TYPE_END )
      {
        // 次のシーン
        wk->scene_id++;
          
        HOSAKA_Printf("next scene_id=%d\n",wk->scene_id);
        
        // 終了チェック
        if( wk->scene_id >= Intro_DATA_GetSceneMax() )
        {
          // 終了
          return FALSE;
        }
        else
        {
          // コマンドセット
          wk->cmd_idx = 0;
          data = Intro_DATA_GetCmdData( wk->scene_id );
        }
      }
      
      HOSAKA_Printf("cmd_idx=%d\n",wk->cmd_idx);

      // 次のコマンドを差しておく
      wk->cmd_idx++;

      // ストア
      if( cmd_store( wk, data ) == FALSE )
      {
        // 次のコマンドを読み込まない場合はループから抜ける
        break;
      }
    }
  }

  return TRUE;
}

//=============================================================================
/**
 *								static関数
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  コマンドをストア
 *
 *	@param	INTRO_CMD_DATA* data 
 *
 *	@retval TRUE:継続して読み込む
 */
//-----------------------------------------------------------------------------
static BOOL cmd_store( INTRO_CMD_WORK* wk, const INTRO_CMD_DATA* data )
{
  GF_ASSERT( data->type != INTRO_CMD_TYPE_NULL );
  GF_ASSERT( data->type < INTRO_CMD_TYPE_END );

  // コマンドをストア
  {
    int i;
    BOOL is_store = FALSE;

    // 空きを探索
    for( i=0; i<STORE_NUM; i++ )
    {
      if( wk->store[i] == NULL )
      {
        wk->store[i] = data;

        is_store = TRUE;
          
        OS_TPrintf("store [%d] cmd type=%d [%d,%d,%d,%d]\n", i, data->type,
            data->param[0],
            data->param[1],
            data->param[2],
            data->param[3]
            );
        
        break;
      }
    }

    GF_ASSERT( is_store == TRUE );
  }
  
  return data->read_next;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ストアされたコマンドを実行
 *
 *	@param	INTRO_CMD_WORK* wk 
 *
 *	@retval TRUE:継続 FALSE:全てのコマンドが終了
 */
//-----------------------------------------------------------------------------
static BOOL cmd_store_exec( INTRO_CMD_WORK* wk )
{
  int i;
  BOOL is_continue = FALSE;

  for( i=0; i<STORE_NUM; i++ )
  {
    if( wk->store[i] )
    {
      const INTRO_CMD_DATA* data;

      data = wk->store[i];

      if( c_cmdtbl[ data->type ]( wk, data->param ) == FALSE )
      {
        // 一個でも実行中のものがあれば次ループも実行
        is_continue = TRUE;
      }
      else
      {
        // 終了したコマンドを消去
        wk->store[i] = NULL;
        HOSAKA_Printf("store [%d] is kill \n", i );
      }
    }
  }

  return is_continue;
}

