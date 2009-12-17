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

#include "msg/msg_intro.h"  // for GMM Index

// 文字列関連
#include "font/font.naix"
#include "message.naix"
#include "print/gf_font.h"
#include "print/wordset.h"
#include "print/printsys.h" // for PRINT_QUE

//アーカイブ
#include "arc_def.h" // for ARCID_XXX

#include "intro.naix"

#include "intro_sys.h"

#include "intro_graphic.h"

//データ
#include "intro_cmd_data.h"

#include "intro_msg.h" // for INTRO_MSG_WORK

#include "intro_cmd.h" // for extern宣言

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
///	
//==============================================================
typedef struct {
  int   seq;      // コマンド内シーケンス
  void* wk_user;  // ユーザーワーク
} INTRO_STORE_DATA;

//--------------------------------------------------------------
///	コマンドワーク
//==============================================================
struct _INTRO_CMD_WORK {
  // [IN]
  HEAPID heap_id;
  const INTRO_PARAM* init_param;
  // [PRIVATE]
  INTRO_SCENE_ID scene_id;
  const INTRO_CMD_DATA* store[ STORE_NUM ];
  // @TODO 時間があったらリファクタ
  // INTRO_STORE_DATAを特定タイミングで初期化し、
  // その時に INTRO_CMD_WORK へのポインタも持たせる形式の方がスッキリする
  INTRO_STORE_DATA store_data[ STORE_NUM ]; // 各コマンド用ワーク
  int cmd_idx;
  INTRO_MSG_WORK* wk_msg;
};

// コマンド
static BOOL CMD_SET_SCENE( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_BG_LOAD( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_BGM( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_SE( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_SE_STOP( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param);
static BOOL CMD_KEY_WAIT( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_PRINT_MSG( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_YESNO( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_SELECT_MOJI( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );

// INTRO_CMD_TYPE と対応
//--------------------------------------------------------------
///	コマンドテーブル (関数ポインタテーブル)
//==============================================================
static BOOL (*c_cmdtbl[ INTRO_CMD_TYPE_MAX ])() = 
{ 
  NULL, // null
  CMD_SET_SCENE,
  CMD_BG_LOAD,
  CMD_BGM,
  CMD_SE,
  CMD_SE_STOP,
  CMD_KEY_WAIT,
  CMD_PRINT_MSG,
  CMD_YESNO,
  CMD_SELECT_MOJI,
  NULL, // end
};

//-----------------------------------------------------------------------------
/**
 *	@brief  次のシーンをセット
 *
 *	@param	INTRO_CMD_WORK* wk
 *	@param	param 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_SET_SCENE( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  // 次のシーン
  wk->scene_id = param[0];
  // コマンドセット
  wk->cmd_idx = 0;

  return TRUE;
}

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
static BOOL CMD_BG_LOAD( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  HEAPID heap_id;
	ARCHANDLE	*handle;

  heap_id = wk->heap_id;
	handle	= GFL_ARC_OpenDataHandle( ARCID_INTRO_GRA, heap_id );

	// 上下画面ＢＧパレット転送
	GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_intro_intro_bg_NCLR, PALTYPE_MAIN_BG, PLTID_BG_BACK_M, 0x20, heap_id );
	GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_intro_intro_bg_NCLR, PALTYPE_SUB_BG, PLTID_BG_BACK_S, 0x20, heap_id );
	
  //	----- 下画面 -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, NARC_intro_intro_bg_NCGR,
						BG_FRAME_BACK_S, 0, 0, 0, heap_id );
	GFL_ARCHDL_UTIL_TransVramScreen(	handle, NARC_intro_intro_bg_main_NSCR,
						BG_FRAME_BACK_S, 0, 0, 0, heap_id );	

	//	----- 上画面 -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, NARC_intro_intro_bg_NCGR,
						BG_FRAME_BACK_M, 0, 0, 0, heap_id );
	GFL_ARCHDL_UTIL_TransVramScreen(	handle, NARC_intro_intro_bg_sub_NSCR,
						BG_FRAME_BACK_M, 0, 0, 0, heap_id );		

	GFL_ARC_CloseDataHandle( handle );

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BGM再生コマンド
 *
 *	@param	param[0] BGM_Label
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_BGM( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  HOSAKA_Printf( "play bgm =%d \n", param[0] );
  PMSND_PlayBGM( param[0] );

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
static BOOL CMD_SE( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
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
static BOOL CMD_SE_STOP( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param)
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
static BOOL CMD_KEY_WAIT( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  if( GFL_UI_TP_GetTrg() || GFL_UI_KEY_GetTrg() )
  {
    return TRUE;
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  メッセージ表示
 *
 *	@param	param[0] str_id
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_PRINT_MSG( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  switch( sdat->seq )
  {
    case 0:
      INTRO_MSG_SetPrint( wk->wk_msg, param[0], NULL, NULL );
      sdat->seq++;
      break;

    case 1:
    if( INTRO_MSG_PrintProc( wk->wk_msg ) )
    {
      return TRUE;
    }
    break;

  default : GF_ASSERT(0);
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  YESNO選択
 *
 *	@param	INTRO_CMD_WORK* wk
 *	@param	param 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_YESNO( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  switch( sdat->seq )
  {
  case 0:
    // SETUP
    {
#if 0
      const BMPMENULIST_HEADER menuH =
      {
        2, 2,
        0, 0, 0, 0,
        1, 2, 15, ///< 文字色
        0, 0, 0,
        0, 0,
      };
#endif
    }
    break;

  case 1:
    {
    }
    break;

  default : GF_ASSERT(0);
  }

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ひらがな／漢字モードを決定
 *
 *	@param	param[0] 0=ひらがな ／ 1=漢字
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_SELECT_MOJI( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  CONFIG* config;
  
  GF_ASSERT( param[0] == 0 || param[0] == 1 );

  config = SaveData_GetConfig( wk->init_param->save_ctrl );
  
  if( param[0] == 0 )
  {
      GFL_MSGSYS_SetLangID( 0 );
      CONFIG_SetMojiMode( config, MOJIMODE_HIRAGANA );
  }
  else
  {
      GFL_MSGSYS_SetLangID( 1 );
      CONFIG_SetMojiMode( config, MOJIMODE_KANJI );
  }

  return TRUE;

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
 *	@param  init_param
 *	@param	heap_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
INTRO_CMD_WORK* Intro_CMD_Init( const INTRO_PARAM* init_param, HEAPID heap_id )
{
  INTRO_CMD_WORK* wk;

  // メインワーク アロケーション
  wk = GFL_HEAP_AllocClearMemory(  heap_id, sizeof( INTRO_CMD_WORK ) );

  // メンバ初期化
  wk->heap_id     = heap_id;
  wk->init_param  = init_param;

  // 選択肢モジュール初期化
  wk->wk_msg = INTRO_MSG_Create( NARC_message_intro_dat ,heap_id );

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
  // 選択肢モジュール破棄
  INTRO_MSG_Exit( wk->wk_msg );

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
  INTRO_MSG_Main( wk->wk_msg );

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
      
      // シーン変更
      if( data->type == INTRO_CMD_TYPE_SET_SCENE )
      {
        // CMD_SET_SCENE
        c_cmdtbl[ data->type ]( wk, &wk->store_data[i], data->param );
                
        HOSAKA_Printf("next scene_id=%d\n",wk->scene_id);

        // 次のシーンの先頭コマンド
        data = Intro_DATA_GetCmdData( wk->scene_id );
      }

      // コマンド終了判定
      if( data->type == INTRO_CMD_TYPE_END )
      {
        // 終了
        return FALSE;
      }

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
          
        // cmd_idxはこの段階で次のIDを指しているので-1
        OS_TPrintf("store [%d] cmd_idx=%d type=%d [%d,%d,%d,%d]\n", i, wk->cmd_idx-1, data->type,
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

      if( c_cmdtbl[ data->type ]( wk, &wk->store_data[i], data->param ) == FALSE )
      {
        // 一個でも実行中のものがあれば次ループも実行
        is_continue = TRUE;
      }
      else
      {
        // 終了したコマンドを消去
        wk->store[i] = NULL;
        // シーケンスをクリア
        wk->store_data[i].seq = 0; 
        HOSAKA_Printf("store [%d] is finish \n", i );
      }
    }
  }

  return is_continue;
}

