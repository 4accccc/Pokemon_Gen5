//============================================================================
/**
 *  @file   zukan_detail_common.c
 *  @brief  図鑑詳細画面共通のデータをまとめたもの
 *  @author Koji Kawada
 *  @data   2010.02.02
 *  @note   
 *  モジュール名：ZUKAN_DETAIL_COMMON
 */
//============================================================================
// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "gamesystem/gamedata_def.h"
#include "savedata/zukan_savedata.h"

#include "zukan_detail_def.h"
#include "zukan_detail_common.h"

// アーカイブ
#include "arc_def.h"
#include "zukan_gra.naix"

// サウンド

// オーバーレイ


//=============================================================================
/**
*  定数定義
*/
//=============================================================================


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// ワーク
//=====================================
struct _ZKNDTL_COMMON_WORK
{
  GAMEDATA*                      gamedata;
  ZUKAN_DETAIL_GRAPHIC_WORK*     graphic;
  GFL_FONT*                      font;
  PRINT_QUE*                     print_que;
  ZUKAN_DETAIL_TOUCHBAR_WORK*    touchbar;
  ZUKAN_DETAIL_HEADBAR_WORK*     headbar;
  u16*                           list;
  u16                            num;
  u16*                           no;
};


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
//------------------------------------------------------------------
/**
 *  @brief        
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
ZKNDTL_COMMON_WORK* ZKNDTL_COMMON_Init(
                HEAPID                       heap_id,
                GAMEDATA*                    gamedata,
                ZUKAN_DETAIL_GRAPHIC_WORK*   graphic,
                GFL_FONT*                    font,
                PRINT_QUE*                   print_que,
                ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar,
                ZUKAN_DETAIL_HEADBAR_WORK*   headbar,
                u16*                         list,
                u16                          num,
                u16*                         no )
{
  ZKNDTL_COMMON_WORK*   cmn;

  // ワーク生成
  cmn = GFL_HEAP_AllocClearMemory( heap_id, sizeof(ZKNDTL_COMMON_WORK) );

  // セット
  cmn->gamedata   = gamedata;
  cmn->graphic    = graphic;
  cmn->font       = font;
  cmn->print_que  = print_que;
  cmn->touchbar   = touchbar;
  cmn->headbar    = headbar;
  cmn->list       = list;
  cmn->num        = num;
  cmn->no         = no;

  return cmn;
}

//------------------------------------------------------------------
/**
 *  @brief        
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
void ZKNDTL_COMMON_Exit( ZKNDTL_COMMON_WORK* cmn )
{
  // ワーク破棄
  GFL_HEAP_FreeMemory( cmn );
}

//------------------------------------------------------------------
/**
 *  @brief        
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
GAMEDATA*                    ZKNDTL_COMMON_GetGamedata( ZKNDTL_COMMON_WORK* cmn )
{
  return cmn->gamedata;
}
ZUKAN_DETAIL_GRAPHIC_WORK*   ZKNDTL_COMMON_GetGraphic( ZKNDTL_COMMON_WORK* cmn )
{
  return cmn->graphic;
}
GFL_FONT*                    ZKNDTL_COMMON_GetFont( ZKNDTL_COMMON_WORK* cmn )
{
  return cmn->font;
}
PRINT_QUE*                   ZKNDTL_COMMON_GetPrintQue( ZKNDTL_COMMON_WORK* cmn )
{
  return cmn->print_que;
}
ZUKAN_DETAIL_TOUCHBAR_WORK*  ZKNDTL_COMMON_GetTouchbar( ZKNDTL_COMMON_WORK* cmn )
{
  return cmn->touchbar;
}
ZUKAN_DETAIL_HEADBAR_WORK*   ZKNDTL_COMMON_GetHeadbar( ZKNDTL_COMMON_WORK* cmn )
{
  return cmn->headbar;
}

//------------------------------------------------------------------
/**
 *  @brief        
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
u16   ZKNDTL_COMMON_GetCurrPoke( ZKNDTL_COMMON_WORK* cmn )  // monsnoを返す
{
  return cmn->list[*(cmn->no)];
}
void  ZKNDTL_COMMON_GoToNextPoke( ZKNDTL_COMMON_WORK* cmn )
{
  u16   start_no   = *(cmn->no);
  BOOL  see_flag   = FALSE;

  ZUKAN_SAVEDATA* zukan_savedata = GAMEDATA_GetZukanSave( cmn->gamedata );
  
  do
  {
    *(cmn->no) += 1;
    if( *(cmn->no) >= cmn->num ) *(cmn->no) = 0;

    if( *(cmn->no) == start_no )
    {
      break;
    }
    else
    {
      u16 monsno = cmn->list[*(cmn->no)];
      see_flag = ZUKANSAVE_GetPokeSeeFlag( zukan_savedata, monsno );
    }
  }
  while( !see_flag );
}
void  ZKNDTL_COMMON_GoToPrevPoke( ZKNDTL_COMMON_WORK* cmn )
{
  u16   start_no   = *(cmn->no);
  BOOL  see_flag   = FALSE;

  ZUKAN_SAVEDATA* zukan_savedata = GAMEDATA_GetZukanSave( cmn->gamedata );
 
  do
  {
    if( *(cmn->no) == 0 ) *(cmn->no) = cmn->num -1;
    else                  *(cmn->no) -= 1;
    
    if( *(cmn->no) == start_no )
    {
      break;
    }
    else
    {
      u16 monsno = cmn->list[*(cmn->no)];
      see_flag = ZUKANSAVE_GetPokeSeeFlag( zukan_savedata, monsno );
    }
  }
  while( !see_flag );
}


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// 
//=====================================




// 一部分フェード
//=============================================================================
/**
*  定数定義
*/
//=============================================================================

//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
struct _ZKNDTL_COMMON_FADE_WORK
{
  int plane1;
  int plane2;
  s32 bright;
  s32 bright_delta;
  s32 bright_wait;
};

//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
static void Zkndtl_Common_FadeImmediately( ZKNDTL_COMMON_FADE_DISP fade_disp, ZKNDTL_COMMON_FADE_WORK* fade_wk );

//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
ZKNDTL_COMMON_FADE_WORK* ZKNDTL_COMMON_FadeInit( HEAPID heap_id )
{
  ZKNDTL_COMMON_FADE_WORK* fade_wk = GFL_HEAP_AllocClearMemory( heap_id, sizeof(ZKNDTL_COMMON_FADE_WORK) );
  return fade_wk;
}
void ZKNDTL_COMMON_FadeExit( ZKNDTL_COMMON_FADE_WORK* fade_wk )
{
  GFL_HEAP_FreeMemory( fade_wk );
}

void ZKNDTL_COMMON_FadeMain( ZKNDTL_COMMON_FADE_WORK* fade_wk_m, ZKNDTL_COMMON_FADE_WORK* fade_wk_s )
{
  // (黒)0<=bright<=16(見える)                        // brightで現在フェードがかかっているのか否か判定できる
  
  // bright_delta==0 フェードアニメしていない         // bright_deltaで現在フェードアニメしているか否か判定できる
  // bright_delta >0 フェードイン(黒→見える)
  // bright_delta <0 フェードアウト(見える→黒)
  
  // bright_wait 0のとき毎フレームアニメが進む、4のときアニメ、待、待、待、待、アニメ、待、待、待、待、アニメとなる

  u8 i;
  
  ZKNDTL_COMMON_FADE_WORK* fade_wk_tbl[ZKNDTL_COMMON_FADE_DISP_MAX];
  
  fade_wk_tbl[ZKNDTL_COMMON_FADE_DISP_M] = fade_wk_m;
  fade_wk_tbl[ZKNDTL_COMMON_FADE_DISP_S] = fade_wk_s;

  for( i=0; i<ZKNDTL_COMMON_FADE_DISP_MAX; i++ )
  {
    ZKNDTL_COMMON_FADE_WORK* fade_wk = fade_wk_tbl[i];

    if( fade_wk->bright_delta != 0 )
    {
      if( fade_wk->bright_wait == 0 )
      {
        fade_wk->bright_wait = ZKNDTL_COMMON_FADE_BRIGHT_WAIT;
        fade_wk->bright += fade_wk->bright_delta;
        if( fade_wk->bright_delta > 0 )  // フェードイン
        {
          if( fade_wk->bright >= 16 )
          {
            fade_wk->bright = 16;
            fade_wk->bright_delta = 0;
          }
        }
        else                             // フェードアウト
        {
          if( fade_wk->bright <= 0 )
          {
            fade_wk->bright = 0;
            fade_wk->bright_delta = 0;
          }
        }

        Zkndtl_Common_FadeImmediately( i, fade_wk );
      }
      else
      {
        fade_wk->bright_wait--;
      }
    }
  }
}

BOOL ZKNDTL_COMMON_FadeIsExecute( ZKNDTL_COMMON_FADE_WORK* fade_wk )
{
  return ( fade_wk->bright_delta != 0 );
}
BOOL ZKNDTL_COMMON_FadeIsColorless( ZKNDTL_COMMON_FADE_WORK* fade_wk )
{
  return ( fade_wk->bright == 16 );
}
BOOL ZKNDTL_COMMON_FadeIsBlack( ZKNDTL_COMMON_FADE_WORK* fade_wk )
{
  return ( fade_wk->bright ==  0 );
}

void ZKNDTL_COMMON_FadeSetBlackToColorless( ZKNDTL_COMMON_FADE_WORK* fade_wk )
{
  // 黒で初期化
  fade_wk->bright       = 0;
  fade_wk->bright_delta = ZKNDTL_COMMON_FADE_BRIGHT_DELTA;
  fade_wk->bright_wait  = ZKNDTL_COMMON_FADE_BRIGHT_WAIT;
}
void ZKNDTL_COMMON_FadeSetColorlessToBlack( ZKNDTL_COMMON_FADE_WORK* fade_wk )
{
  // 見えるで初期化
  fade_wk->bright       = 16;
  fade_wk->bright_delta = - ZKNDTL_COMMON_FADE_BRIGHT_DELTA;
  fade_wk->bright_wait  = ZKNDTL_COMMON_FADE_BRIGHT_WAIT;
}

void ZKNDTL_COMMON_FadeSetColorlessImmediately( ZKNDTL_COMMON_FADE_DISP fade_disp, ZKNDTL_COMMON_FADE_WORK* fade_wk )
{
  // 見えるで初期化
  fade_wk->bright       = 16;
  fade_wk->bright_delta = 0;
  fade_wk->bright_wait  = 0;
  
  Zkndtl_Common_FadeImmediately( fade_disp, fade_wk );
}
void ZKNDTL_COMMON_FadeSetBlackImmediately( ZKNDTL_COMMON_FADE_DISP fade_disp, ZKNDTL_COMMON_FADE_WORK* fade_wk )
{
  // 黒で初期化
  fade_wk->bright       = 0;
  fade_wk->bright_delta = 0;
  fade_wk->bright_wait  = 0;
 
  Zkndtl_Common_FadeImmediately( fade_disp, fade_wk );
}

void ZKNDTL_COMMON_FadeSetPlane( ZKNDTL_COMMON_FADE_WORK* fade_wk, int plane1, int plane2 )
{
  fade_wk->plane1 = plane1;
  fade_wk->plane2 = plane2;
}

void ZKNDTL_COMMON_FadeSetPlaneDefault( ZKNDTL_COMMON_FADE_WORK* fade_wk )
{
  fade_wk->plane1 = GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BD;
  fade_wk->plane2 = GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BD;
}


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
static void Zkndtl_Common_FadeImmediately( ZKNDTL_COMMON_FADE_DISP fade_disp, ZKNDTL_COMMON_FADE_WORK* fade_wk )
{
  if( fade_disp == ZKNDTL_COMMON_FADE_DISP_M )
  {
    G2_SetBlendBrightnessExt(  fade_wk->plane1,
                               fade_wk->plane2,
                               fade_wk->bright,
                               0,
                               fade_wk->bright - 16 );
  }
  else
  {
    G2S_SetBlendBrightnessExt( fade_wk->plane1,
                               fade_wk->plane2,
                               fade_wk->bright,
                               0,
                               fade_wk->bright - 16 );
  }
}




// 最背面
//=============================================================================
/**
*  定数定義
*/
//=============================================================================
typedef enum
{
  REAR_DISP_M,
  REAR_DISP_S,
  REAR_DISP_MAX,
}
REAR_DISP;

enum
{
  REAR_DATA_NCL,
  REAR_DATA_NCG,
  REAR_DATA_NSC,
  REAR_DATA_MAX,
};

static const u8 rear_pal_ofs_low[REAR_DISP_MAX][ZKNDTL_COMMON_REAR_TYPE_MAX] =
{
  { 0, 3, 2, 4, },  // main
  { 1, 1, 1, 1, },  // sub
};
static const u8 rear_pal_ofs_high[REAR_DISP_MAX][ZKNDTL_COMMON_REAR_TYPE_MAX] =
{
  { 0, 3, 2, 4, },  // main
  { 0, 3, 2, 4, },  // sub
};
static const ARCDATID rear_data[REAR_DISP_MAX][REAR_DATA_MAX] =
{
  // main
  {
    NARC_zukan_gra_info_info_bgd_NCLR,
    NARC_zukan_gra_info_info_bgd_NCGR,
    NARC_zukan_gra_info_voicebase_bgd_NSCR,
  },
  // sub
  {
    NARC_zukan_gra_info_info_bgu_NCLR,
    NARC_zukan_gra_info_info_bgu_NCGR,
    NARC_zukan_gra_info_infobase_bgu_NSCR,
  },
};
#define REAR_DATA_FORM_M_NSC (NARC_zukan_gra_info_formebase_bgd_NSCR)  // 特別に差し替え


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
struct _ZKNDTL_COMMON_REAR_WORK
{
  GFL_ARCUTIL_TRANSINFO       tinfo;
  u8                          bg_frame;
  u8                          wait;
};


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
ZKNDTL_COMMON_REAR_WORK* ZKNDTL_COMMON_RearInit(
                                     HEAPID                    heap_id,
                                     ZKNDTL_COMMON_REAR_TYPE   type,
                                     u8                        bg_frame,
                                     u8                        pal0,
                                     u8                        pal1
                                )
{
  ZKNDTL_COMMON_REAR_WORK* rear_wk = GFL_HEAP_AllocClearMemory( heap_id, sizeof(ZKNDTL_COMMON_REAR_WORK) );

  ARCHANDLE* handle;
  ARCDATID   data_nsc;

  // BGフレームからメインかサブか判定する
  REAR_DISP disp;
  PALTYPE   paltype;
  
  if( bg_frame < GFL_BG_FRAME0_S )
  {
    disp    = REAR_DISP_M;
    paltype = PALTYPE_MAIN_BG;
  }
  else
  {
    disp    = REAR_DISP_S;
    paltype = PALTYPE_SUB_BG;
  }

  // 読み込んで転送
  handle = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, heap_id );

  GFL_ARCHDL_UTIL_TransVramPaletteEx(
                               handle,
                               rear_data[disp][REAR_DATA_NCL],
                               paltype,
                               rear_pal_ofs_low[disp][type] * 0x20,
                               pal0 * 0x20,
                               1 * 0x20,
                               heap_id );

  GFL_ARCHDL_UTIL_TransVramPaletteEx(
                               handle,
                               rear_data[disp][REAR_DATA_NCL],
                               paltype,
                               rear_pal_ofs_high[disp][type] * 0x20,
                               pal1 * 0x20,
                               1 * 0x20,
                               heap_id );

  rear_wk->tinfo = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan(
                            handle,
                            rear_data[disp][REAR_DATA_NCG],
                            bg_frame,
                            0,
                            FALSE,
                            heap_id );
  GF_ASSERT_MSG( rear_wk->tinfo != GFL_ARCUTIL_TRANSINFO_FAIL, "ZUKAN_DETAIL_COMMON : BGキャラ領域が足りませんでした。\n" );

  data_nsc = rear_data[disp][REAR_DATA_NSC];
  if( type == ZKNDTL_COMMON_REAR_TYPE_FORM && disp == REAR_DISP_M )  // 特別に差し替え
      data_nsc = REAR_DATA_FORM_M_NSC;

  GFL_ARCHDL_UTIL_TransVramScreenCharOfs(
        handle,
        data_nsc,
        bg_frame,
        0,
        GFL_ARCUTIL_TRANSINFO_GetPos( rear_wk->tinfo ),
        0,
        FALSE,
        heap_id );
  GFL_BG_ChangeScreenPalette( bg_frame, 0, 21, 32, 3, pal0 );
  GFL_BG_ChangeScreenPalette( bg_frame, 0, 0, 32, 21, pal1 );

  GFL_ARC_CloseDataHandle( handle );

  GFL_BG_LoadScreenV_Req( bg_frame );

  rear_wk->bg_frame = bg_frame;
  rear_wk->wait = ZKNDTL_SCROLL_WAIT;

  return rear_wk;
}

void ZKNDTL_COMMON_RearExit( ZKNDTL_COMMON_REAR_WORK* rear_wk )
{
  GFL_BG_FreeCharacterArea( rear_wk->bg_frame,
                            GFL_ARCUTIL_TRANSINFO_GetPos( rear_wk->tinfo ),
                            GFL_ARCUTIL_TRANSINFO_GetSize( rear_wk->tinfo ) );

  GFL_HEAP_FreeMemory( rear_wk );
}

void ZKNDTL_COMMON_RearMain( ZKNDTL_COMMON_REAR_WORK* rear_wk )
{
  if( rear_wk->wait == 0 )
  {
    GFL_BG_SetScrollReq( rear_wk->bg_frame, GFL_BG_SCROLL_X_INC, 1 );
    rear_wk->wait = ZKNDTL_SCROLL_WAIT;
  }
  else
  {
    rear_wk->wait--;
  }
}




// BGパネル
//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
                                             // type          TRANS  EXIST

GFL_ARCUTIL_TRANSINFO ZKNDTL_COMMON_PanelCreate(           //  o      x
             ZKNDTL_COMMON_PANEL_CREATE_TYPE    type,      //  o      o
             HEAPID                             heap_id,   //  o      o
             u8                                 bg_frame,  //  o      o
             u8                                 pal_num,   //  o      x
             u8                                 pal_pos,   //  o      o
             u8                                 pal_ofs,   //  o      x
             u32                                arc_id,    //  o      o
             ARCDATID                           data_ncl,  //  o      x
             ARCDATID                           data_ncg,  //  o      x 
             ARCDATID                           data_nsc,  //  o      o
             GFL_ARCUTIL_TRANSINFO              tinfo      //  x      o
       )
{
  ARCHANDLE*  handle;

  // BGフレームからメインかサブか判定する
  PALTYPE   paltype;
  
  if( bg_frame < GFL_BG_FRAME0_S )
  {
    paltype = PALTYPE_MAIN_BG;
  }
  else
  {
    paltype = PALTYPE_SUB_BG;
  }

  // 読み込んで転送
  handle = GFL_ARC_OpenDataHandle( arc_id, heap_id );

  if( type == ZKNDTL_COMMON_PANEL_CREATE_TYPE_TRANS_PAL_CHARA )
  {
    GFL_ARCHDL_UTIL_TransVramPaletteEx(
                               handle,
                               data_ncl,
                               paltype,
                               pal_ofs * 0x20,
                               pal_pos * 0x20,
                               pal_num * 0x20,
                               heap_id );
  }

  if( type == ZKNDTL_COMMON_PANEL_CREATE_TYPE_TRANS_PAL_CHARA )
  {
    tinfo = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan(
                            handle,
                            data_ncg,
                            bg_frame,
                            0,
                            FALSE,
                            heap_id );
    GF_ASSERT_MSG( tinfo != GFL_ARCUTIL_TRANSINFO_FAIL, "ZUKAN_DETAIL_COMMON : BGキャラ領域が足りませんでした。\n" );
  }

  GFL_ARCHDL_UTIL_TransVramScreenCharOfs(
        handle,
        data_nsc,
        bg_frame,
        0,
        GFL_ARCUTIL_TRANSINFO_GetPos( tinfo ),
        0,
        FALSE,
        heap_id );
  GFL_BG_ChangeScreenPalette( bg_frame, 0, 0, 32, 24, pal_pos );

  GFL_ARC_CloseDataHandle( handle );

  GFL_BG_LoadScreenV_Req( bg_frame );
  
  return tinfo;
}

void ZKNDTL_COMMON_PanelDelete(
                       u8                      bg_frame,
                       GFL_ARCUTIL_TRANSINFO   tinfo
        )
{
  GFL_BG_FreeCharacterArea( bg_frame,
                            GFL_ARCUTIL_TRANSINFO_GetPos( tinfo ),
                            GFL_ARCUTIL_TRANSINFO_GetSize( tinfo ) );
}

