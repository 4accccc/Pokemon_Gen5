//=============================================================================
/**
 *
 *  @file   intro_cmd.c
 *  @brief  3Dデモコマンド
 *  @author hosaka genya
 *  @data   2009.12.09
 *
 */
//=============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/brightness.h"

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

#include "intro_msg.h"        // for INTRO_MSG_WORK
#include "intro_mcss.h"       // for INTRO_MCSS_WORK
#include "intro_g3d.h"        // for INTRO_G3D_WORK
#include "intro_particle.h"   // for INTRO_PARTICLE_WORK

#include "intro_cmd.h"        // for extern宣言

#include "sound/pm_sndsys.h"  // for SEQ_SE_XXX
#include "sound/pm_voice.h"   // for 
#include "sound/sound_manager.h" // for

#include "savedata/save_control.h"      // for
#include "savedata/save_control_intr.h" // for

//=============================================================================
/**
 *                定数定義
 */
//=============================================================================
enum
{ 
  STORE_NUM = 8,  ///< 同時実行コマンドの限界数
};

//=============================================================================
/**
 *                構造体定義
 */
//=============================================================================

//--------------------------------------------------------------
/// 
//==============================================================
typedef struct {
  int   seq;      // コマンド内シーケンス
  int   cnt;      // 汎用カウンタ
  void* wk_user;  // ユーザーワーク
} INTRO_STORE_DATA;

//--------------------------------------------------------------
/// コマンドワーク
//==============================================================
struct _INTRO_CMD_WORK {
  // [IN]
  HEAPID heap_id;
  INTRO_PARAM* init_param;
  INTRO_MCSS_WORK* mcss;
  INTRO_G3D_WORK* g3d;
  // [PRIVATE]
  INTRO_SCENE_ID scene_id;
  const INTRO_CMD_DATA* store[ STORE_NUM ];
  // @TODO 時間があったらリファクタ
  // INTRO_STORE_DATAをINTRO_STORE_DATAに内包する形式の方がスッキリする。
  INTRO_STORE_DATA store_data[ STORE_NUM ]; // 各コマンド用ワーク
  int cmd_idx;
  INTRO_MSG_WORK* wk_msg;
  INTRO_PARTICLE_WORK* ptc;
  INTR_SAVE_CONTROL* intr_save;
};

//=============================================================================
/**
 *              プロトタイプ宣言
 */
//=============================================================================
static BOOL cmd_store( INTRO_CMD_WORK* wk, const INTRO_CMD_DATA* data );
static BOOL cmd_store_exec( INTRO_CMD_WORK* wk );
static void cmd_store_clear( INTRO_CMD_WORK* wk, u8 id );

//=============================================================================
// コマンド
//=============================================================================

// 特殊コマンド
static BOOL CMD_SET_SCENE( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_START_SCENE( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_YESNO( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_COMP( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );

// 一般コマンド
static BOOL CMD_LOAD_BG( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_SET_RETCODE( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_WAIT( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_FADE_REQ( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_BRIGHTNESS_SET( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_BRIGHTNESS_REQ( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_BRIGHTNESS_WAIT( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_BGM( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_BGM_FADEOUT( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_SE( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_SE_STOP( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param);
static BOOL CMD_KEY_WAIT( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_LOAD_GMM( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_WORDSET( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_PRINT_MSG( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_MCSS_LOAD( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_MCSS_SET_VISIBLE( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_MCSS_SET_ANIME( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_MCSS_FADE_REQ( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );

// イントロ用コマンド
static BOOL CMD_SELECT_MOJI( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_SELECT_SEX( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_POKEMON_APPER( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_PARTICLE_MONSTERBALL( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_G3D_SELECT_SEX_SET_VISIBLE( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_G3D_SELECT_SEX_SET_FRAME( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_G3D_SELECT_SEX_INIT( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_G3D_SELECT_SEX_MAIN( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_G3D_SELECT_SEX_RETURN( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );

static BOOL CMD_SAVE_INIT( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_SAVE_SASUPEND( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_SAVE_RESUME( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_SAVE_MYSTATUS( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );
static BOOL CMD_SAVE_CHECK_ALL_END( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param );



// INTRO_CMD_TYPE と対応
//--------------------------------------------------------------
/// コマンドテーブル (関数ポインタテーブル)
//==============================================================
static BOOL (*c_cmdtbl[ INTRO_CMD_TYPE_MAX ])() = 
{ 
  NULL, // null
  
  //-----------------------------------------
  // ◆ 特殊コマンド ◆
  //-----------------------------------------
  CMD_SET_SCENE,
  CMD_START_SCENE,
  CMD_YESNO,
  CMD_COMP,

  //-----------------------------------------
  // ◆ 一般コマンド ◆
  //-----------------------------------------
  CMD_LOAD_BG,
  CMD_SET_RETCODE,
  CMD_WAIT,
  CMD_FADE_REQ,
  CMD_BRIGHTNESS_SET,
  CMD_BRIGHTNESS_REQ,
  CMD_BRIGHTNESS_WAIT,
  CMD_BGM,
  CMD_BGM_FADEOUT,
  CMD_SE,
  CMD_SE_STOP,
  CMD_KEY_WAIT,
  CMD_LOAD_GMM,
  CMD_WORDSET,
  CMD_PRINT_MSG,
  CMD_MCSS_LOAD,
  CMD_MCSS_SET_VISIBLE,
  CMD_MCSS_SET_ANIME,
  CMD_MCSS_FADE_REQ,

  //-----------------------------------------
  // ◆ イントロデモ用コマンド ◆
  //-----------------------------------------
  CMD_SELECT_MOJI,
  CMD_SELECT_SEX,
  CMD_POKEMON_APPER,
  CMD_PARTICLE_MONSTERBALL,

  CMD_G3D_SELECT_SEX_SET_VISIBLE,
  CMD_G3D_SELECT_SEX_SET_FRAME,
  CMD_G3D_SELECT_SEX_INIT,
  CMD_G3D_SELECT_SEX_MAIN,
  CMD_G3D_SELECT_SEX_RETURN,

  CMD_SAVE_INIT,
  CMD_SAVE_SASUPEND,
  CMD_SAVE_RESUME,
  CMD_SAVE_MYSTATUS,
  CMD_SAVE_CHECK_ALL_END,

  NULL, // end
};

//=============================================================================
// 判定関数
//=============================================================================
static BOOL CMD_COMP_SEX( INTRO_CMD_WORK* wk );

// INTRO_CMD_COMP と対応
//--------------------------------------------------------------
/// 判定関数テーブル (関数ポインタテーブル)
//==============================================================
static BOOL (*c_cmdtbl_comp[ INTRO_CMD_COMP_MAX ])() = 
{ 
  CMD_COMP_SEX,
};

//=============================================================================
// WORDSET関数
//=============================================================================
static void CMD_WORDSET_TRAINER( INTRO_CMD_WORK* wk, int bufID );

// INTRO_WORDSET と対応
//--------------------------------------------------------------
/// WORDSETテーブル (関数ポインタテーブル)
//==============================================================
static void (*c_cmdtbl_wordset[ INTRO_WORDSET_MAX ])() = 
{ 
  CMD_WORDSET_TRAINER,
};

//-----------------------------------------------------------------------------
/**
 *  @brief  次のシーンをセット
 *
 *  @param  INTRO_CMD_WORK* wk
 *  @param  param 
 *
 *  @retval
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
 *  @brief  シーン開始
 *
 *  @param  param 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_START_SCENE( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  int inner_param[1];
      
  inner_param[0] = wk->init_param->scene_id;

  // 無限ループ対策
  if( inner_param[0] == INTRO_SCENE_ID_INIT )
  {
    GF_ASSERT(0);
    inner_param[0] += 1;
  }

  CMD_SET_SCENE( wk, NULL, inner_param );

  OS_TPrintf("\n*** start scene_id=%d ***\n",wk->scene_id);

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  BMPLISTをストア
 *
 *  @param  INTRO_CMD_WORK* wk
 *  @param  select_id 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void store_bmplist( INTRO_CMD_WORK* wk, int select_id )
{
  const INTRO_CMD_DATA* data;

  data = Intro_DATA_GetCmdData( wk->scene_id );
  data += wk->cmd_idx;

  HOSAKA_Printf( "select_id=%d \n", select_id );

  // リスト開放
  INTRO_MSG_LIST_Finish( wk->wk_msg );

  if( select_id == 0 )
  {
    if( data != NULL )
    {
      // 上が選択された場合は直後のコマンドをストア
      cmd_store( wk, data );
    }
  }
  else
  {
    data++;
    if( data != NULL )
    {
      cmd_store( wk, data );
    }
  }
   
  // 結果コマンドの次をシーク
  wk->cmd_idx += 2;
}


//-----------------------------------------------------------------------------
/**
 *  @brief  YESNO選択
 *
 *  @param  param 
 * 
 *  @note   TRUEなら一個下のコマンドをストア、FALSEなら2個下のコマンドをストア
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_YESNO( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  switch( sdat->seq )
  {
  case 0:
    // SETUP
    {
      INTRO_LIST_DATA data[2];

      data[0].str_id  = param[0];
      data[0].param   = 0;

      data[1].str_id  = param[1];
      data[1].param   = 1;

      INTRO_MSG_LIST_Start( wk->wk_msg, data, NELEMS(data), param[2] );

      sdat->seq++;
    }
    break;

  case 1:
    INTRO_MSG_LIST_Main( wk->wk_msg );

    {
      INTRO_LIST_SELECT select;
      u32 select_id;
      
      select = INTRO_MSG_LIST_IsDecide( wk->wk_msg, &select_id );

      if( select == INTRO_LIST_SELECT_DECIDE )
      {
        store_bmplist( wk, select_id );

        return TRUE;
      }
      else if( select == INTRO_LIST_SELECT_CANCEL )
      {
        store_bmplist( wk, 1 );

        return TRUE;
      }
    }
    break;

  default : GF_ASSERT(0);
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  判定関数呼び出し
 *
 *  @param  PARAM[0] INTRO_CMD_COMP
 
 *  @note   TRUEなら一個下のコマンドをストア、FALSEなら2個下のコマンドをストア
 *
 *  @retval TRUE=コマンド終了
 */
//-----------------------------------------------------------------------------
static BOOL CMD_COMP( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  BOOL is_comp;
  const INTRO_CMD_DATA* data;

  data = Intro_DATA_GetCmdData( wk->scene_id );
  data += wk->cmd_idx;

  is_comp = c_cmdtbl_comp[ param[0] ]( wk );

  // 判定コマンドを実行
  if( is_comp == FALSE )
  {
    data++;
  }
  
  OS_TPrintf("comp = %d \n", is_comp );
  
  if( data != NULL )
  {
    cmd_store( wk, data );
  }
  
  // 結果コマンドの次をシーク
  wk->cmd_idx += 2;

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  グラフィックロード
 *
 *  @param  INTRO_CMD_WORK* wk
 *  @param  param 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_LOAD_BG( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  HEAPID heap_id;
  ARCHANDLE *handle;

  heap_id = wk->heap_id;
  // @TODO 汎用性
  handle  = GFL_ARC_OpenDataHandle( ARCID_INTRO_GRA, heap_id );

  // 上下画面ＢＧパレット転送
  GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_intro_intro_bg_NCLR, PALTYPE_MAIN_BG, PLTID_BG_BACK_M, 0x20, heap_id );
  GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_intro_intro_bg_NCLR, PALTYPE_SUB_BG, PLTID_BG_BACK_S, 0x20, heap_id );
  
  //  ----- 下画面 -----
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_intro_intro_bg_NCGR,
            BG_FRAME_BACK_S, 0, 0, 0, heap_id );
  GFL_ARCHDL_UTIL_TransVramScreen(  handle, NARC_intro_intro_bg_main_NSCR,
            BG_FRAME_BACK_S, 0, 0, 0, heap_id );  

  //  ----- 上画面 -----
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_intro_intro_bg_NCGR,
            BG_FRAME_BACK_M, 0, 0, 0, heap_id );
  GFL_ARCHDL_UTIL_TransVramScreen(  handle, NARC_intro_intro_bg_sub_NSCR,
            BG_FRAME_BACK_M, 0, 0, 0, heap_id );    
  
  GFL_ARC_CloseDataHandle( handle );

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  外部アプリに対する戻り値を設定
 *
 *  @param  param[0] INTRO_RETCODE
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_SET_RETCODE( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  wk->init_param->retcode =  param[0];

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  コマンドウェイト
 *
 *  @param  INTRO_CMD_WORK* wk
 *  @param  sdat
 *  @param  param 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_WAIT( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  if( sdat->cnt > param[0] )
  {
    return TRUE;
  }
  
  sdat->cnt++;

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  フェードリクエスト
 *
 *  PARAM[0]  GFL_FADE_MASTER_BRIGHT_XXX
 *  PARAM[1]  スタート輝度(0～16)
 *  PARAM[2]  エンド輝度(0～16)
 *  PARAM[3]  フェードスピード
 */
//-----------------------------------------------------------------------------
static BOOL CMD_FADE_REQ( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{ 
  switch( sdat->seq )
  {
  case 0:
    GFL_FADE_SetMasterBrightReq( param[0], param[1], param[2], param[3] );
    sdat->seq++;
    break;
  case 1:
    if( GFL_FADE_CheckFade() == FALSE )
    {
      return TRUE;
    }
    break;
  default : GF_ASSERT(0);
  };
  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  ブライドネスをセット
 *
 *  @param  param[0]  輝度
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_BRIGHTNESS_SET( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  SetBrightness( param[0], (PLANEMASK_BG0|PLANEMASK_BG2|PLANEMASK_BG3|PLANEMASK_OBJ), MASK_MAIN_DISPLAY );

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  ブライドネス変更リクエスト
 *
 *  @param  param[0]
 *  @param  param[1]
 *  @param  param[2]
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_BRIGHTNESS_REQ( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  ChangeBrightnessRequest( param[0], param[1], param[2], (PLANEMASK_BG0|PLANEMASK_BG2|PLANEMASK_BG3|PLANEMASK_OBJ), MASK_MAIN_DISPLAY );

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  ブライドネス終了待ち
 *
 *  @param  param[0]
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_BRIGHTNESS_WAIT( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  IsFinishedBrightnessChg( MASK_MAIN_DISPLAY );

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  BGM再生コマンド
 *
 *  @param  param[0] BGM_Label
 *  @param  param[1] fadeInFrame
 *  @param  param[2] fadeOutFrame
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_BGM( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  HOSAKA_Printf( "play bgm =%d fadeInFrame=%d, fadeOutFrame=%d \n", param[0], param[1], param[2] );
//  PMSND_PlayBGM( param[0] );
  PMSND_PlayNextBGM( param[0], param[1], param[2] );

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  BGMフェードアウト
 *
 *  @param  param[0] SYNC
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_BGM_FADEOUT( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  PMSND_FadeOutBGM( param[0] );

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  SE再生コマンド
 *
 *  @param  param[0] SE_Label
 *  @param  param[1] volume : 0なら無効
 *  @param  param[2] pan : 0なら無効
 *
 *  @retval
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
 *  @brief  SE:再生停止
 *
 *  @param  param[0] SE_Label
 *
 *  @retval
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
 *  @brief  キーウェイト
 *
 *  @param  INTRO_CMD_WORK* wk
 *  @param  param 
 *
 *  @retval
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
 *  @brief  GMMをリロード
 *
 *  @param  param[0] GflMsgLoadType
 *  @param  param[1] msg_dat_id
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_LOAD_GMM( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  INTRO_MSG_LoadGmm( wk->wk_msg, param[0], param[1] );

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  WORDSETする
 *
 *  @param  param[0] INTRO_WORDSET レジストする種類を設定
 *  @param  param[1] レジストするIDを指定
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_WORDSET( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  GF_ASSERT( param[0] < INTRO_WORDSET_MAX );

  // 関数テーブルから引っ張り出す
  c_cmdtbl_wordset[ param[0] ]( wk, param[1] );

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  メッセージ表示
 *
 *  @param  param[0] str_id
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_PRINT_MSG( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  switch( sdat->seq )
  {
    case 0:
      INTRO_MSG_SetPrint( wk->wk_msg, param[0] );

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
 *  @brief  イントロ画面用MCSSをロード
 *  @param  param[0] MCSS_ID
 *  @param  param[1] 0=博士, MONSNO=ポケモン
 */
//-----------------------------------------------------------------------------
static BOOL CMD_MCSS_LOAD( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  GF_ASSERT( param[1] <= MONSNO_MAX );

  if( param[1] == 0 )
  {
    static const MCSS_ADD_WORK add = 
    {
      ARCID_INTRO_GRA,
      NARC_intro_intro_doctor_NCBR,
      NARC_intro_intro_doctor_NCLR,
      NARC_intro_intro_doctor_NCER,
      NARC_intro_intro_doctor_NANR,
      NARC_intro_intro_doctor_NMCR,
      NARC_intro_intro_doctor_NMAR,
      NARC_intro_intro_doctor_NCEC,
    };

    // 博士表示
    INTRO_MCSS_Add( wk->mcss, param[2], param[3], 0, &add, param[0] );
  }
  else
  {
    // ポケモン表示
    INTRO_MCSS_AddPoke( wk->mcss, param[2], param[3], FX32_ONE, param[1], param[0] );
  }

  return TRUE;
}


//-----------------------------------------------------------------------------
/**
 *  @brief  MCSS表示切替
 *
 *  @param  param[0] MCSS_ID
 *  @param  param[1] 0:非表示, 1:表示
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_MCSS_SET_VISIBLE( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  INTRO_MCSS_SetVisible( wk->mcss, param[1], param[0] );

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  MCSS アニメーション指定
 *
 *  @param  param[0]  MCSS_ID
 *  @param  param[1]  アニメーションID
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_MCSS_SET_ANIME( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  INTRO_MCSS_SetAnimeIndex( wk->mcss, param[0], param[1] );
  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  MCSS フェードリクエスト
 *
 *  @param  INTRO_CMD_WORK* wk
 *  @param  sdat
 *  @param  param 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_MCSS_FADE_REQ( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  // @TODO 上手くいかない。モザイクみたいになる。
  switch( sdat->seq )
  {
  case 0 :
    sdat->cnt = 31;
    sdat->seq++;
    break;
  case 1 :
    sdat->cnt--;
    if( sdat->cnt <= 0 )
    {
      return TRUE;
    }
    else
    {
      HOSAKA_Printf("mode=%d alpha=%d\n",param[0], sdat->cnt);
      if( param[1] == TRUE )
      {
        INTRO_MCSS_SetAlpha( wk->mcss, param[0], 31-sdat->cnt );
      }
      else if( param[1] == FALSE )
      {
        INTRO_MCSS_SetAlpha( wk->mcss, param[0], sdat->cnt );
      }
      else
      {
        GF_ASSERT(0);
      }
    }
    break;
  default : GF_ASSERT(0);
  }

  return FALSE;
}


//=============================================================================
/**
 * イントロ用コマンド
 */
//=============================================================================

// セーブ処理 開始
static BOOL CMD_SAVE_INIT( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  IntrSave_Start( wk->intr_save );
  return TRUE;
}

//  セーブ中断リクエスト
static BOOL CMD_SAVE_SASUPEND( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  switch( sdat->seq )
  {
  case 0 :
    // 中断リクエストをかける
    IntrSave_ReqSuspend( wk->intr_save );
    sdat->seq++;
    break;
  case 1 :
    // 中断待ち
    if( IntrSave_CheckSuspend( wk->intr_save ) == TRUE )
    {
      return TRUE;
    }
  };
  return FALSE;
}

// セーブ再開
static BOOL CMD_SAVE_RESUME( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  IntrSave_Resume( wk->intr_save );
  return TRUE;
}

// マイステータスを実行
static BOOL CMD_SAVE_MYSTATUS( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  IntrSave_ReqMyStatusSave( wk->intr_save );
  return TRUE;
}

// セーブ終了チェック
static BOOL CMD_SAVE_CHECK_ALL_END( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  switch( sdat->seq )
  {
    case 0 :
      // 全てのセーブが完了しているか調べる
      if( IntrSave_CheckAllSaveEnd( wk->intr_save ) == TRUE )
      {
        return TRUE;
      }
      else
      {
        // ちょっとまってね！
        INTRO_MSG_SetPrint( wk->wk_msg, msg_intro_00 );
        sdat->seq++;
      }
      break;

    case 1 :
      // メッセージプリント
      if( INTRO_MSG_PrintProc( wk->wk_msg ) == TRUE )
      {
        sdat->seq++;
      }
      break;
    case 2:
      // 全てのセーブが完了しているか調べる
      if( IntrSave_CheckAllSaveEnd( wk->intr_save ) == TRUE )
      {
        return TRUE;
      }
      break;
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  ひらがな／漢字モードを決定
 *
 *  @param  param[0] 0=ひらがな ／ 1=漢字
 *
 *  @retval
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

//-----------------------------------------------------------------------------
/**
 *  @brief  性別を決定
 *
 *  @param  param[0]  0=おとこのこ,1=おんなのこ
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_SELECT_SEX( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  MYSTATUS* mystatus;

  mystatus = SaveData_GetMyStatus( wk->init_param->save_ctrl );

  if( param[0] == 0 )
  {
    MyStatus_SetMySex( mystatus , PTL_SEX_MALE );
  }
  else
  { 
    MyStatus_SetMySex( mystatus , PTL_SEX_FEMALE );
  }

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  ポケモン登場演出
 *
 *  @param  param 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_POKEMON_APPER( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  //@TODO

  // 鳴き声
  PMVOICE_Play( param[0], 0, 0, FALSE, 0, 0, FALSE, NULL );

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  パーティクル モンスターボール
 *
 *  @param  INTRO_CMD_WORK* wk
 *  @param  sdat 
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static BOOL CMD_PARTICLE_MONSTERBALL( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  // ブレンド指定
  G2_SetBlendAlpha( GX_PLANEMASK_BG0, GX_PLANEMASK_BG3|GX_PLANEMASK_BG0, 0, 0 );

  // モンスターボールを表示
  INTRO_PARTICLE_CreateEmitterMonsterBall( wk->ptc, param[0], param[1], 0 );

  return TRUE;
}


//-----------------------------------------------------------------------------
/**
 *  @brief  3D選択肢 表示
 *
 *  @param  INTRO_CMD_WORK* wk
 *  @param  sdat
 *  @param  param 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_G3D_SELECT_SEX_SET_VISIBLE( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  INTRO_G3D_SelectVisible( wk->g3d, param[0] ); 

  return TRUE;
}


//-----------------------------------------------------------------------------
/**
 *  @brief  3D男女選択 フレーム直指定
 *
 *  @param  param[0] フレーム値
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_G3D_SELECT_SEX_SET_FRAME( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  INTRO_G3D_SelectSet( wk->g3d, param[0] );
  return TRUE;
}


//-----------------------------------------------------------------------------
/**
 *  @brief  3D男女洗濯開始
 *
 *  @param  INTRO_CMD_WORK* wk
 *  @param  sdat
 *  @param  param 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_G3D_SELECT_SEX_INIT( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{ 
  if( INTRO_G3D_SelectStart( wk->g3d ) )
  {
    return TRUE;
  }
  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  3D男女選択主処理
 *
 *  @param  
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_G3D_SELECT_SEX_MAIN( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{ 
  switch( sdat->seq )
  {
  case 0 :
    INTRO_G3D_SelectSet( wk->g3d, 19 ); // デフォルトフレーム指定
    sdat->seq++;
    break;
  case 1 :
    // キー入力 →
    if( (GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT) )
    {
      if(sdat->cnt!=1){
        GFL_SOUND_PlaySE( SEQ_SE_SELECT1 );
      }
      sdat->cnt = 1;  //おんなのこ
      INTRO_G3D_SelectSet( wk->g3d, 21 );
    }
    // キー入力 ←
    else if( (GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT) )
    {
      if(sdat->cnt!=0){
        GFL_SOUND_PlaySE( SEQ_SE_SELECT1 );
      }
      sdat->cnt = 0;  //おとこのこ
      INTRO_G3D_SelectSet( wk->g3d, 19 );
    }
    // 決定
    else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
      GFL_SOUND_PlaySE( SEQ_SE_DECIDE1 );
      sdat->seq++;
    }
    break;

  case 2 :
    // 性別決定
    {
      BOOL is_woman = sdat->cnt;
      MYSTATUS* mystatus;

      mystatus = SaveData_GetMyStatus( wk->init_param->save_ctrl );

      if( is_woman == FALSE )
      {
        MyStatus_SetMySex( mystatus , PTL_SEX_MALE );
      }
      else
      { 
        MyStatus_SetMySex( mystatus , PTL_SEX_FEMALE );
      }
    
      // 決定演出開始
      INTRO_G3D_SelectDecideStart( wk->g3d, !is_woman );

      sdat->seq++;
    }
    break;

  case 3: 
    // 決定演出終了待ち
    if( INTRO_G3D_SelectDecideWait( wk->g3d ) )
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
 *  @brief
 *
 *  @param  param[0]
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_G3D_SELECT_SEX_RETURN( INTRO_CMD_WORK* wk, INTRO_STORE_DATA* sdat, int* param )
{
  if( INTRO_G3D_SelectDecideReturn( wk->g3d ) )
  {
    return TRUE;
  }

  return FALSE;
}


//=============================================================================
// 判定関数
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *  @brief  男女判定
 *
 *  @param  INTRO_CMD_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL CMD_COMP_SEX( INTRO_CMD_WORK* wk )
{
  MYSTATUS* mystatus;

  mystatus = SaveData_GetMyStatus( wk->init_param->save_ctrl );

  if( MyStatus_GetMySex( mystatus ) == PTL_SEX_MALE )
  {
    return TRUE;
  }

  return FALSE;
}

//=============================================================================
// WORDSET関数
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *  @brief  トレーナー名をレジスト
 *
 *  @param  INTRO_CMD_WORK* wk
 *  @param  bufID 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void CMD_WORDSET_TRAINER( INTRO_CMD_WORK* wk, int bufID )
{
  WORDSET* wordset;
  const MYSTATUS* mystatus;

  wordset   = INTRO_MSG_GetWordSet( wk->wk_msg );
  mystatus  = SaveData_GetMyStatus( wk->init_param->save_ctrl );

  WORDSET_RegisterPlayerName( wordset, bufID, mystatus );
}

//=============================================================================
/**
 *                外部公開関数
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *  @brief  3Dデモコマンドコントローラー 初期化
 *
 *  @param  init_param
 *  @param  heap_id 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
INTRO_CMD_WORK* Intro_CMD_Init( INTRO_G3D_WORK* g3d, INTRO_PARTICLE_WORK* ptc ,INTRO_MCSS_WORK* mcss, INTRO_PARAM* init_param, HEAPID heap_id )
{
  INTRO_CMD_WORK* wk;

  GF_ASSERT( mcss );
  GF_ASSERT( ptc );

  // メインワーク アロケーション
  wk = GFL_HEAP_AllocClearMemory( heap_id, sizeof( INTRO_CMD_WORK ) );

  // メンバ初期化
  wk->heap_id     = heap_id;
  wk->init_param  = init_param;
  wk->mcss  = mcss;
  wk->g3d   = g3d;
  wk->ptc   = ptc;

  // セーブモジュール受け取り
  wk->intr_save = wk->init_param->intr_save;

  // 文字操作モジュール初期化
  wk->wk_msg = INTRO_MSG_Create( heap_id );

  return wk;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  3Dデモコマンドコントローラー 破棄
 *
 *  @param  INTRO_CMD_WORK* wk 
 *
 *  @retval
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
 *  @brief  3Dデモコマンドコントローラー 主処理
 *
 *  @param  INTRO_CMD_WORK* wk 
 *
 *  @retval TRUE:継続 FALSE:終了
 */
//-----------------------------------------------------------------------------
BOOL Intro_CMD_Main( INTRO_CMD_WORK* wk )
{
  INTRO_MSG_Main( wk->wk_msg );

  // ストアされたコマンドを実行
  if( cmd_store_exec( wk ) == FALSE )
  {
    int i;

    // コマンドがすべて終了したら、次のコマンドを呼び出す
    for( i=0; i<STORE_NUM+1; i++ )
    {
      const INTRO_CMD_DATA* data;

      GF_ASSERT( i < STORE_NUM ); ///< ストア限界チェック
      
      data  = Intro_DATA_GetCmdData( wk->scene_id );
      data += wk->cmd_idx;
      
      //=======================================================
      // 例外コマンド
      //=======================================================
      // シーン変更
      if( data->type == INTRO_CMD_TYPE_SET_SCENE )
      {
        // CMD_SET_SCENE
        c_cmdtbl[ data->type ]( wk, &wk->store_data[i], data->param );
                
        OS_TPrintf("\n*** next scene_id=%d ***\n",wk->scene_id);

        // 次のシーンの先頭コマンド
        data = Intro_DATA_GetCmdData( wk->scene_id );
      }
      //---------------------------------------------

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
 *                static関数
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *  @brief  コマンドをストア
 *
 *  @param  INTRO_CMD_DATA* data 
 *
 *  @retval TRUE:継続して読み込む
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
            data->param[3] );
        
        break;
      }
    }

    GF_ASSERT( is_store == TRUE );
  }
  
  return data->read_next;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  ストアされたコマンドを実行
 *
 *  @param  INTRO_CMD_WORK* wk 
 *
 *  @retval TRUE:継続 FALSE:全てのコマンドが終了
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
        // 指定したIDのストアをクリア
        cmd_store_clear( wk, i );
      }
    }
  }

  return is_continue;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  指定したコマンドをクリア
 *
 *  @param  INTRO_CMD_WORK* wk
 *  @param  id 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void cmd_store_clear( INTRO_CMD_WORK* wk, u8 id )
{
  GF_ASSERT( id < STORE_NUM );

  // 終了したコマンドを消去
  wk->store[id] = NULL;
  // シーケンスをクリア
  wk->store_data[id].seq = 0; 
  wk->store_data[id].cnt = 0;

  HOSAKA_Printf("store [%d] is clear \n", id );
}

