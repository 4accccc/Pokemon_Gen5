//============================================================================
/**
 *  @file   zukan_info.c
 *  @brief  図鑑情報
 *  @author Koji Kawada
 *  @data   2009.12.03
 *  @note   infowin.c, touchbar.c, ui_template.cを参考にして作成しました。
 *
 *  モジュール名：ZUKAN_INFO
 */
//============================================================================
// lib
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"




#include "print/printsys.h"
#include "print/wordset.h"
#include "print/global_msg.h"
#define PSTATUS_BG_PLT_FONT (4)
enum
{
  ZUKAN_INFO_MSG_TOROKU,
  ZUKAN_INFO_MSG_NAME,
  ZUKAN_INFO_MSG_PHYSICAL,
  ZUKAN_INFO_MSG_EXPLAIN,

  ZUKAN_INFO_MSG_MAX,
};

#include "ui/ui_easy_clwk.h"
#define PLTID_OBJ_TYPEICON_M (3)  // 3本使用

#include "system/poke2dgra.h"
#define PLTID_OBJ_POKE_M (12)

#include "poke_tool/pokefoot.h"
#define PLTID_OBJ_POKEFOOT_M (13)




// アーカイブ
#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"
#include "zukan_gra.naix"

// メッセージ
#include "msg/msg_zkn.h"
#include "msg/msg_zkn_type.h"
#include "msg/msg_zkn_height.h"
#include "msg/msg_zkn_gram.h"
#include "msg/msg_zkn_comment_00.h"

// zukan_info
#include "zukan_info.h"

//=============================================================================
/**
 *  定数定義
 */
//=============================================================================
#define ZUKAN_INFO_BG_PAL_LINE_SIZE (0x20)
#define ZUKAN_INFO_BG_PAL_NUM       (3)
#define ZUKAN_INFO_BACK_BG_PAL_NO   (0)
#define ZUKAN_INFO_FORE_BG_PAL_NO_D (1)
#define ZUKAN_INFO_FORE_BG_PAL_NO_L (2)
#define ZUKAN_INFO_BG_CHARA_SIZE    ( 32 * 4 * GFL_BG_1CHRDATASIZ )
#define ZUKAN_INFO_BG_SCREEN_W      (32)
#define ZUKAN_INFO_BG_SCREEN_H      (24)
#define ZUKAN_INFO_BG_SCREEN_SIZE   ( ZUKAN_INFO_BG_SCREEN_W * ZUKAN_INFO_BG_SCREEN_H * GFL_BG_1SCRDATASIZ )
#define ZUKAN_INFO_BAR_SCREEN_X     (0)
#define ZUKAN_INFO_BAR_SCREEN_Y     (0)
#define ZUKAN_INFO_BAR_SCREEN_W     (32)
#define ZUKAN_INFO_BAR_SCREEN_H     (3)

#define ZUKAN_INFO_FPS (60)
#define ZUKAN_INFO_BAR_BLINK_COUNT_MAX ( ZUKAN_INFO_FPS / 2 )
#define ZUKAN_INFO_BACK_BG_SCREEN_SCROLL_X_VALUE (1)
#define ZUKAN_INFO_VBLANK_TCB_PRI (1)

#define ZUKAN_INFO_BG_UPDATE_BIT_RESET          (0)
#define ZUKAN_INFO_FORE_BG_UPDATE_BIT_BAR_BLINK (1<<0)
#define ZUKAN_INFO_BACK_BG_UPDATE_BIT_SCROLL    (1<<0)




#define TEST_MSG
#define TEST_TYPEICON_TWICE
#define UI_TEST_POKE2D
#define TEST_POKEFOOT




#define ZUKAN_INFO_BG_FRAME_M_MSG (GFL_BG_FRAME3_M)
#define ZUKAN_INFO_BG_FRAME_M_FORE (GFL_BG_FRAME2_M)
#define ZUKAN_INFO_BG_FRAME_M_BACK (GFL_BG_FRAME1_M)

#define ZUKAN_INFO_BG_FRAME_S_MSG (GFL_BG_FRAME3_S)
#define ZUKAN_INFO_BG_FRAME_S_FORE (GFL_BG_FRAME2_S)
#define ZUKAN_INFO_BG_FRAME_S_BACK (GFL_BG_FRAME1_S)

#define ZUKAN_INFO_BG_PAL_NO (0)

typedef enum
{
  ZUKAN_INFO_STEP_COLOR_WAIT,
  ZUKAN_INFO_STEP_COLOR,
  ZUKAN_INFO_STEP_MOVE_WAIT,
  ZUKAN_INFO_STEP_MOVE,  // TOROKU
  ZUKAN_INFO_STEP_CENTER,  // NICKNAME
  ZUKAN_INFO_STEP_CENTER_STILL,  // TOROKU or NICKNAME
}
ZUKAN_INFO_STEP;

#define ZUKAN_INFO_START_POKEMON_POS_X (8*6+2)//(8*6)  // ポケモン2Dは12chara x 12chara  // 左上のピクセル座標が(0, 24)もしくは(2,24)になるように。
#define ZUKAN_INFO_START_POKEMON_POS_Y (8*(3+6))

#define ZUKAN_INFO_CENTER_POKEMON_POS_X (8*16)
#define ZUKAN_INFO_CENTER_POKEMON_POS_Y (8*(3+6))

#define ZUKAN_INFO_WND_DOWN_Y_START (8*17)
#define ZUKAN_INFO_WND_DOWN_Y_GOAL (8*2)
#define ZUKAN_INFO_WND_UP_Y (8*1)
#define ZUKAN_INFO_WND_LEFT_X (0)
#define ZUKAN_INFO_WND_RIGHT_X (100)

//=============================================================================
/**
 *  構造体宣言
 */
//=============================================================================
struct _ZUKAN_INFO_WORK
{
  // fore = setumei, back = base
  GFL_ARCUTIL_TRANSINFO fore_bg_chara_info;  ///< foreのBGキャラ領域
  GFL_ARCUTIL_TRANSINFO back_bg_chara_info;  ///< backのBGキャラ領域
  u8                    fore_bg_update;  ///< foreのBG更新ビットフラグ
  u8                    back_bg_update;  ///< backのBG更新ビットフラグ
 
  GFL_TCB* vblank_tcb;  ///< VBlank中のTCB

  u32 bar_blink_pal_no;  ///< バーのパレット番号
  u8  bar_blink_count;   ///< バーの点滅カウント

  HEAPID heap_id;  // 他のところのを覚えているだけで生成や破棄はしない。
  POKEMON_PARAM* pp;  // 他のところのを覚えているだけで生成や破棄はしない。
  ZUKAN_INFO_DISP disp;
  u8 bg_priority;
  GFL_CLUNIT* clunit;  // 他のところのを覚えているだけで生成や破棄はしない。
  GFL_FONT* font;  // 他のところのを覚えているだけで生成や破棄はしない。
  PRINT_QUE* print_que;  // 他のところのを覚えているだけで生成や破棄はしない。

  PALTYPE bg_paltype;
  u8 msg_bg_frame;
  u8 fore_bg_frame;
  u8 back_bg_frame;

  u32 monsno;
  u32 monsno_msg;

  ZUKAN_INFO_LAUNCH launch;
  ZUKAN_INFO_STEP step;
  BOOL move_flag;

  int wnd_down_y;  // ウィンドウの右下Y座標

#ifdef TEST_MSG  // prog/src/app/p_status/p_sta_local_def.h, p_sta_sys.c  // queはProcにまかせる
  //MSG系
  GFL_BMPWIN*  bmpwin[ZUKAN_INFO_MSG_MAX];
#endif //TEST_MSG


#ifdef TEST_TYPEICON_TWICE
  u32       typeicon_cg_idx[2];
  u32       typeicon_cl_idx;
  u32       typeicon_cean_idx;
  GFL_CLWK* typeicon_clwk[2];
#endif //TEST_TYPEICON_TWICE


#ifdef UI_TEST_POKE2D
	u32												ncg_poke2d;
	u32												ncl_poke2d;
	u32												nce_poke2d;
	GFL_CLWK									*clwk_poke2d;
#endif //UI_TEST_POKE2D


#ifdef TEST_POKEFOOT
  UI_EASY_CLWK_RES clres_pokefoot;
  GFL_CLWK* clwk_pokefoot;
#endif //TEST_POKEFOOT


};

//=============================================================================
/**
 *  
 */
//=============================================================================
static void Zukan_Info_VBlankFunc( GFL_TCB* tcb, void* wk );
static void Zukan_Info_ChangeBarPal( ZUKAN_INFO_WORK* work );


#ifdef TEST_MSG  // PSTATUS_InitMessage
static void Zukan_Info_CreateMessage( ZUKAN_INFO_WORK* work );
static void Zukan_Info_DeleteMessage( ZUKAN_INFO_WORK* work );
#endif

#ifdef TEST_TYPEICON_TWICE
static void Zukan_Info_CreateTypeicon( ZUKAN_INFO_WORK* work, PokeType type1, PokeType type2, GFL_CLUNIT* clunit, HEAPID heap_id );
static void Zukan_Info_DeleteTypeicon( ZUKAN_INFO_WORK* work );
#endif //TEST_TYPEICON_TWICE

#ifdef UI_TEST_POKE2D
//-------------------------------------
///	ポケモンOBJ,BG読みこみ
//=====================================
static void UITemplate_POKE2D_LoadResourceOBJ( ZUKAN_INFO_WORK *wk, HEAPID heapID );
static void UITemplate_POKE2D_UnLoadResourceOBJ( ZUKAN_INFO_WORK *wk );
static void UITemplate_POKE2D_CreateCLWK( ZUKAN_INFO_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID, u16 pos_x, u16 pos_y );
static void UITemplate_POKE2D_DeleteCLWK( ZUKAN_INFO_WORK *wk );
#endif //UI_TEST_POKE2D

#ifdef TEST_POKEFOOT
static void Zukan_Info_CreatePokefoot( ZUKAN_INFO_WORK* work, u32 monsno, GFL_CLUNIT* unit, HEAPID heap_id );
static void Zukan_Info_DeletePokefoot( ZUKAN_INFO_WORK* work );
#endif //TEST_POKEFOOT


static void Zukan_Info_CreateOthers( ZUKAN_INFO_WORK* work );

//=============================================================================
/**
 *  外部公開関数定義
 */
//=============================================================================
//-------------------------------------
/// 初期化
//=====================================
ZUKAN_INFO_WORK* ZUKAN_INFO_Init( HEAPID a_heap_id, POKEMON_PARAM* a_pp,
                                         ZUKAN_INFO_LAUNCH a_launch,
                                         ZUKAN_INFO_DISP a_disp, u8 a_bg_priority,
                                         GFL_CLUNIT* a_clunit,
                                         GFL_FONT* a_font,
                                         PRINT_QUE* a_print_que )
{
  ZUKAN_INFO_WORK* work;

  // ワーク生成、初期化
  {
    u32 size = sizeof(ZUKAN_INFO_WORK);
    work = GFL_HEAP_AllocMemory( a_heap_id, size );
    GFL_STD_MemClear( work, size );
  }
  {
    work->heap_id = a_heap_id;
    work->pp = a_pp;
    work->launch = a_launch;
    work->disp = a_disp;
    work->bg_priority = a_bg_priority;
    work->clunit = a_clunit;
    work->font = a_font;
    work->print_que = a_print_que;
  }
  {
    if( work->disp == ZUKAN_INFO_DISP_M )
    {
      work->bg_paltype = PALTYPE_MAIN_BG;
      work->msg_bg_frame = ZUKAN_INFO_BG_FRAME_M_MSG; 
      work->fore_bg_frame = ZUKAN_INFO_BG_FRAME_M_FORE; 
      work->back_bg_frame = ZUKAN_INFO_BG_FRAME_M_BACK; 
    }
    else
    {
      work->bg_paltype = PALTYPE_SUB_BG;
      work->msg_bg_frame = ZUKAN_INFO_BG_FRAME_S_MSG; 
      work->fore_bg_frame = ZUKAN_INFO_BG_FRAME_S_FORE; 
      work->back_bg_frame = ZUKAN_INFO_BG_FRAME_S_BACK; 
    }
    GFL_BG_SetPriority( work->msg_bg_frame, work->bg_priority );  // 最前面
    GFL_BG_SetPriority( work->fore_bg_frame, work->bg_priority +1 );
    GFL_BG_SetPriority( work->back_bg_frame, work->bg_priority +2 );

    work->monsno = PP_Get( work->pp, ID_PARA_monsno, NULL );
    if( work->monsno > 493 )
    {
      work->monsno_msg = 1;
    }
    else
    {
      work->monsno_msg = work->monsno;
    }

    switch( work->launch )
    {
    case ZUKAN_INFO_LAUNCH_TOROKU:
      {
        work->step = ZUKAN_INFO_STEP_COLOR_WAIT;
      }
      break;
    case ZUKAN_INFO_LAUNCH_NICKNAME:
      {
        work->step = ZUKAN_INFO_STEP_CENTER;
      }
      break;
    }
    work->move_flag = FALSE;
  }
  {
    work->bar_blink_count = ZUKAN_INFO_BAR_BLINK_COUNT_MAX;
    work->bar_blink_pal_no = ZUKAN_INFO_FORE_BG_PAL_NO_D;

    work->fore_bg_update = ZUKAN_INFO_BG_UPDATE_BIT_RESET;
    work->back_bg_update = ZUKAN_INFO_BG_UPDATE_BIT_RESET;
  }

  // リソース読み込み
  {
    ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, work->heap_id );
    GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_zukan_gra_info_zukan_bgu_NCLR, work->bg_paltype,
                                      ZUKAN_INFO_BG_PAL_NO * ZUKAN_INFO_BG_PAL_LINE_SIZE, ZUKAN_INFO_BG_PAL_NUM * ZUKAN_INFO_BG_PAL_LINE_SIZE,
                                      work->heap_id );

    // fore
    work->fore_bg_chara_info = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( handle, NARC_zukan_gra_info_zukan_bgu_NCGR, work->fore_bg_frame,
                                                                            ZUKAN_INFO_BG_CHARA_SIZE, FALSE, work->heap_id );
    GF_ASSERT_MSG( work->fore_bg_chara_info != GFL_ARCUTIL_TRANSINFO_FAIL, "ZUKAN_INFO : BGキャラ領域が足りませんでした。\n" );
    
    GFL_ARCHDL_UTIL_TransVramScreen( handle, NARC_zukan_gra_info_setumei_bgu_NSCR, work->fore_bg_frame,
                                     GFL_ARCUTIL_TRANSINFO_GetPos( work->fore_bg_chara_info ), ZUKAN_INFO_BG_SCREEN_SIZE, FALSE, work->heap_id );
    //GFL_BG_ChangeScreenPalette( work->fore_bg_frame, 0, 0, ZUKAN_INFO_BG_SCREEN_W, ZUKAN_INFO_BG_SCREEN_H,
    //                            work->bg_pal_no + ZUKAN_INFO_FORE_BG_PAL_NO_D );

    //Zukan_Info_ChangeBarPal( work );

    // back
    work->back_bg_chara_info = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( handle, NARC_zukan_gra_info_zukan_bgu_NCGR, work->back_bg_frame,
                                                                            ZUKAN_INFO_BG_CHARA_SIZE, FALSE, work->heap_id );
    GF_ASSERT_MSG( work->back_bg_chara_info != GFL_ARCUTIL_TRANSINFO_FAIL, "ZUKAN_INFO : BGキャラ領域が足りませんでした。\n" );
        
    GFL_ARCHDL_UTIL_TransVramScreen( handle, NARC_zukan_gra_info_base_bgu_NSCR, work->back_bg_frame,
                                     GFL_ARCUTIL_TRANSINFO_GetPos( work->back_bg_chara_info ), ZUKAN_INFO_BG_SCREEN_SIZE, FALSE, work->heap_id );
    //GFL_BG_ChangeScreenPalette( work->back_bg_frame, 0, 0, ZUKAN_INFO_BG_SCREEN_W, ZUKAN_INFO_BG_SCREEN_H,
    //                            work->bg_pal_no + ZUKAN_INFO_BACK_BG_PAL_NO );
    
    GFL_ARC_CloseDataHandle( handle );
  }

  // モンスターボールのマークを付ける
  {
    GFL_BMP_DATA* bmp_data = GFL_BMP_LoadCharacter( ARCID_ZUKAN_GRA, NARC_zukan_gra_info_zukan_bgu_NCGR, 0, work->heap_id );
    GFL_BMPWIN* bmpwin = GFL_BMPWIN_Create( work->fore_bg_frame, 12, 4, 4, 3, 0x1, GFL_BMP_CHRAREA_GET_F );
    GFL_BMP_Print( bmp_data, GFL_BMPWIN_GetBmp(bmpwin), 8*5, 8*0, 0, 0, 8*4, 8*3, 0x0 );
    GFL_BMPWIN_MakeTransWindow( bmpwin );
    GFL_BMPWIN_Delete( bmpwin );
    GFL_BMP_Delete( bmp_data );
  }

  work->vblank_tcb = GFUser_VIntr_CreateTCB( Zukan_Info_VBlankFunc, work, ZUKAN_INFO_VBLANK_TCB_PRI );

  GFL_BG_LoadScreenReq( work->fore_bg_frame );
  GFL_BG_LoadScreenReq( work->back_bg_frame );

  Zukan_Info_CreateOthers( work );


#ifdef UI_TEST_POKE2D
	//ポケモンOBJ,BG読みこみ
	{
    u16 pos_x, pos_y;
    switch(work->launch)
    {
    case ZUKAN_INFO_LAUNCH_TOROKU:
      {
        pos_x = ZUKAN_INFO_START_POKEMON_POS_X;
        pos_y = ZUKAN_INFO_START_POKEMON_POS_Y;
      }
      break;
    case ZUKAN_INFO_LAUNCH_NICKNAME:
      {
        pos_x = ZUKAN_INFO_CENTER_POKEMON_POS_X;
        pos_y = ZUKAN_INFO_CENTER_POKEMON_POS_Y;
      }
      break;
    }
		UITemplate_POKE2D_LoadResourceOBJ( work, work->heap_id );
		UITemplate_POKE2D_CreateCLWK( work, work->clunit, work->heap_id, pos_x, pos_y );
	}
#endif //UI_TEST_POKE2D


  if( work->launch == ZUKAN_INFO_LAUNCH_TOROKU )
  {
    work->wnd_down_y = ZUKAN_INFO_WND_DOWN_Y_START;
    GX_SetVisibleWnd( GX_WNDMASK_W0 );
    G2_SetWnd0Position( ZUKAN_INFO_WND_LEFT_X, ZUKAN_INFO_WND_UP_Y, ZUKAN_INFO_WND_RIGHT_X, work->wnd_down_y );
    G2_SetWnd0InsidePlane( GX_WND_PLANEMASK_BG0 | GX_WND_PLANEMASK_BG1 | GX_WND_PLANEMASK_BG2 | GX_WND_PLANEMASK_BG3 | GX_WND_PLANEMASK_OBJ,  TRUE );
    G2_SetWndOutsidePlane( GX_WND_PLANEMASK_BG0 | GX_WND_PLANEMASK_BG1 | GX_WND_PLANEMASK_BG2 | GX_WND_PLANEMASK_BG3 | GX_WND_PLANEMASK_OBJ, FALSE );
    G2_SetBlendBrightness( GX_BLEND_PLANEMASK_OBJ,
                           -16 );
  }


  return work;
}

//-------------------------------------
/// 破棄
//=====================================
void ZUKAN_INFO_Exit( ZUKAN_INFO_WORK* work )
{

#ifdef UI_TEST_POKE2D
	UITemplate_POKE2D_DeleteCLWK( work );
	UITemplate_POKE2D_UnLoadResourceOBJ( work );
#endif //UI_TEST_POKE2D

  if( work->launch != ZUKAN_INFO_LAUNCH_TOROKU )
  {
    ZUKAN_INFO_DeleteOthers( work );
  }

  GFL_TCB_DeleteTask( work->vblank_tcb );

  // 読み込んだリソースの破棄
  {
    // back
    GFL_BG_FreeCharacterArea( work->back_bg_frame, GFL_ARCUTIL_TRANSINFO_GetPos( work->back_bg_chara_info ),
                              GFL_ARCUTIL_TRANSINFO_GetSize( work->back_bg_chara_info ) );
      
    // fore
    GFL_BG_FreeCharacterArea( work->fore_bg_frame, GFL_ARCUTIL_TRANSINFO_GetPos( work->fore_bg_chara_info ),
                              GFL_ARCUTIL_TRANSINFO_GetSize( work->fore_bg_chara_info ) );
  }

  // ワーク破棄
  {
    GFL_HEAP_FreeMemory( work );
  }
}

//-------------------------------------
/// メイン
//=====================================
void ZUKAN_INFO_Main( ZUKAN_INFO_WORK* work )
{
  {
    work->bar_blink_count--;
    if( work->bar_blink_count == 0 )
    {
      work->bar_blink_pal_no = ( work->bar_blink_pal_no == ZUKAN_INFO_FORE_BG_PAL_NO_D ) ?
                               ZUKAN_INFO_FORE_BG_PAL_NO_L : ZUKAN_INFO_FORE_BG_PAL_NO_D;
      work->bar_blink_count = ZUKAN_INFO_BAR_BLINK_COUNT_MAX;
    
      work->fore_bg_update |= ZUKAN_INFO_FORE_BG_UPDATE_BIT_BAR_BLINK; 
    }
  }

  {
    work->back_bg_update |= ZUKAN_INFO_BACK_BG_UPDATE_BIT_SCROLL;
  }

  switch(work->step)
  {
  case ZUKAN_INFO_STEP_COLOR_WAIT:
    {
    }
    break;
  case ZUKAN_INFO_STEP_COLOR:
    {
      work->wnd_down_y--;
      G2_SetWnd0Position( ZUKAN_INFO_WND_LEFT_X, ZUKAN_INFO_WND_UP_Y, ZUKAN_INFO_WND_RIGHT_X, work->wnd_down_y );
      if( work->wnd_down_y == ZUKAN_INFO_WND_DOWN_Y_GOAL )
      {
        GX_SetVisibleWnd( GX_WNDMASK_NONE );
        G2_SetBlendBrightness( GX_BLEND_PLANEMASK_OBJ,
                               0 );
        work->step = ZUKAN_INFO_STEP_MOVE_WAIT;
      }
    }
    break;
  case ZUKAN_INFO_STEP_MOVE_WAIT:
    {
      if(work->move_flag)
      {
        work->step = ZUKAN_INFO_STEP_MOVE;
      }
    }
    break;
  case ZUKAN_INFO_STEP_MOVE:
    {
      BOOL goal_x = FALSE;
      BOOL goal_y = FALSE;

#ifdef UI_TEST_POKE2D
      GFL_CLACTPOS pos;
      GFL_CLACT_WK_GetWldPos( work->clwk_poke2d, &pos );
      pos.x += 1;
      pos.y += 1;
      if( pos.x > ZUKAN_INFO_CENTER_POKEMON_POS_X )
      {
        pos.x = ZUKAN_INFO_CENTER_POKEMON_POS_X;
        goal_x = TRUE;
      }
      if( pos.y > ZUKAN_INFO_CENTER_POKEMON_POS_Y )
      {
        pos.y = ZUKAN_INFO_CENTER_POKEMON_POS_Y;
        goal_y = TRUE;
      }
      GFL_CLACT_WK_SetWldPos( work->clwk_poke2d, &pos );
#endif //UI_TEST_POKE2D
            
      if(goal_x && goal_y)
      {
        work->step = ZUKAN_INFO_STEP_CENTER_STILL;
      }
    }
    break;
  case ZUKAN_INFO_STEP_CENTER:
    {
      work->step = ZUKAN_INFO_STEP_CENTER_STILL;
    }
    break;
  case ZUKAN_INFO_STEP_CENTER_STILL:
    {
    }
    break;
  }

}

void ZUKAN_INFO_DeleteOthers( ZUKAN_INFO_WORK* work )
{

if( work->launch == ZUKAN_INFO_LAUNCH_TOROKU )
{

#ifdef TEST_POKEFOOT
  Zukan_Info_DeletePokefoot( work );
#endif //TEST_POKEFOOT

}

if( work->launch == ZUKAN_INFO_LAUNCH_TOROKU )
{

#ifdef TEST_TYPEICON_TWICE
  Zukan_Info_DeleteTypeicon( work ); 
#endif //TEST_TYPEICON_TWICE

}

if( work->launch == ZUKAN_INFO_LAUNCH_TOROKU )
{

#ifdef TEST_MSG
  {
    Zukan_Info_DeleteMessage( work );
  }
#endif //TEST_MSG

}

}


void ZUKAN_INFO_Move( ZUKAN_INFO_WORK* work )
{
  work->move_flag = TRUE;
}

BOOL ZUKAN_INFO_IsMove( ZUKAN_INFO_WORK* work )
{
  BOOL ret = FALSE;
  if( work->step == ZUKAN_INFO_STEP_MOVE ) ret = TRUE;
  return ret;
}

BOOL ZUKAN_INFO_IsCenterStill( ZUKAN_INFO_WORK* work )
{
  BOOL ret = FALSE;
  if( work->step == ZUKAN_INFO_STEP_CENTER_STILL ) ret = TRUE;
  return ret;
}

void ZUKAN_INFO_Start( ZUKAN_INFO_WORK* work )
{
  if( work->step == ZUKAN_INFO_STEP_COLOR_WAIT )
  {
    work->step = ZUKAN_INFO_STEP_COLOR;
  }
}

//=============================================================================
/**
 *  
 */
//=============================================================================
static void Zukan_Info_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  ZUKAN_INFO_WORK* work = (ZUKAN_INFO_WORK*)wk;
  BOOL fore_bg_load = FALSE;

  if( work->fore_bg_update & ZUKAN_INFO_FORE_BG_UPDATE_BIT_BAR_BLINK )
  {
    //Zukan_Info_ChangeBarPal( work );
    fore_bg_load = TRUE;
  }

  if( fore_bg_load )
  {
    GFL_BG_LoadScreenReq( work->fore_bg_frame );
  }

  if( work->back_bg_update & ZUKAN_INFO_BACK_BG_UPDATE_BIT_SCROLL )
  {
    GFL_BG_SetScrollReq( work->back_bg_frame, GFL_BG_SCROLL_X_INC, ZUKAN_INFO_BACK_BG_SCREEN_SCROLL_X_VALUE );
  }

  work->fore_bg_update = ZUKAN_INFO_BG_UPDATE_BIT_RESET;
  work->back_bg_update = ZUKAN_INFO_BG_UPDATE_BIT_RESET;
}

static void Zukan_Info_ChangeBarPal( ZUKAN_INFO_WORK* work )
{
  GFL_BG_ChangeScreenPalette( work->fore_bg_frame,
                              ZUKAN_INFO_BAR_SCREEN_X, ZUKAN_INFO_BAR_SCREEN_X, ZUKAN_INFO_BAR_SCREEN_W, ZUKAN_INFO_BAR_SCREEN_H,
                              ZUKAN_INFO_BG_PAL_NO + work->bar_blink_pal_no );
}




















#ifdef TEST_MSG  // PSTATUS_InitMessage

typedef enum
{
  ZUKAN_INFO_ALIGN_LEFT,
  ZUKAN_INFO_ALIGN_RIGHT,
  ZUKAN_INFO_ALIGN_CENTER,
}
ZUKAN_INFO_ALIGN;

#define ZUKAN_INFO_STRBUF_LEN (128)  // この文字数で足りるかbuflen.hで要確認

static void Zukan_Info_DrawStr( HEAPID heap_id, GFL_BMPWIN* bmpwin, GFL_MSGDATA* msgdata, PRINT_QUE* print_que, GFL_FONT* font,
                                u32 str_id, u16 x, u16 y, u16 color, ZUKAN_INFO_ALIGN align, WORDSET* wordset );

static void Zukan_Info_DrawStr( HEAPID heap_id, GFL_BMPWIN* bmpwin, GFL_MSGDATA* msgdata, PRINT_QUE* print_que, GFL_FONT* font,
                                u32 str_id, u16 x, u16 y, u16 color, ZUKAN_INFO_ALIGN align, WORDSET* wordset )
{
  // LEFTのときは(x,y)が左上になるように書く
  // RIGHTのときは(x,y)が右上になるように書く
  // CENTERのときは(bmpwinの左右中央になる位置,y)になるように書く(xは使用しない)

  GFL_BMP_DATA* bmp_data = GFL_BMPWIN_GetBmp( bmpwin );
  u16 ax;
  STRBUF* strbuf;
  u16 str_width;
  u16 bmp_width;

  if( wordset )
  {
    STRBUF* src_strbuf = GFL_MSG_CreateString( msgdata, str_id );
    strbuf = GFL_STR_CreateBuffer( ZUKAN_INFO_STRBUF_LEN, heap_id );
    WORDSET_ExpandStr( wordset, strbuf, src_strbuf );
    GFL_STR_DeleteBuffer( src_strbuf );
  }
  else
  {
    strbuf = GFL_MSG_CreateString( msgdata, str_id );
  }

  switch( align )
  {
  case ZUKAN_INFO_ALIGN_LEFT:
    {
      ax = x;
    }
    break;
  case ZUKAN_INFO_ALIGN_RIGHT:
    {
      str_width = (u16)( PRINTSYS_GetStrWidth( strbuf, font, 0 ) );
      ax = x - str_width;
    }
    break;
  case ZUKAN_INFO_ALIGN_CENTER:
    {
      str_width = (u16)( PRINTSYS_GetStrWidth( strbuf, font, 0 ) );
      bmp_width = GFL_BMP_GetSizeX( bmp_data );
      ax = ( bmp_width - str_width ) / 2;
    }
    break;
  }
 
  PRINTSYS_PrintQueColor( print_que, bmp_data, 
                          ax, y, strbuf, font, color );
        
  GFL_STR_DeleteBuffer( strbuf );
}

static void Zukan_Info_CreateMessage( ZUKAN_INFO_WORK* work )
{
  GFL_MSGDATA* msgdata_common;
  GFL_MSGDATA* msgdata_kind;
  GFL_MSGDATA* msgdata_height;
  GFL_MSGDATA* msgdata_weight;
  GFL_MSGDATA* msgdata_explain;

  msgdata_common = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_zkn_dat, work->heap_id );
  msgdata_kind = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_zkn_type_dat, work->heap_id );
  msgdata_height = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_zkn_height_dat, work->heap_id );
  msgdata_weight = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_zkn_gram_dat, work->heap_id );
  msgdata_explain = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_zkn_comment_00_dat, work->heap_id );

  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
                                 PSTATUS_BG_PLT_FONT * ZUKAN_INFO_BG_PAL_LINE_SIZE, 1 * ZUKAN_INFO_BG_PAL_LINE_SIZE, work->heap_id );

  {
    s32 i;

    // キャラクター単位でX座標、Y座標、Xサイズ、Yサイズを指定する
    const u8 pos_siz[ZUKAN_INFO_MSG_MAX][4] =
    {
      {  2,  0, 28,  3 },
      { 16,  4, 15,  5 },
      { 18, 12, 13,  5 },
      {  2, 17, 28,  7 },
    };

    for( i=0; i<ZUKAN_INFO_MSG_MAX; i++ )
    {
      // bmpwinのビットマップを透明色で塗り潰しておいたほうがいい？
      work->bmpwin[i] = GFL_BMPWIN_Create( work->msg_bg_frame,
                                           pos_siz[i][0], pos_siz[i][1], pos_siz[i][2], pos_siz[i][3],
                                           PSTATUS_BG_PLT_FONT, GFL_BMP_CHRAREA_GET_B );
    }

    Zukan_Info_DrawStr( work->heap_id, work->bmpwin[ZUKAN_INFO_MSG_TOROKU], msgdata_common, work->print_que, work->font,
                        ZKN_POKEGET_00, 0, 5, PRINTSYS_LSB_Make(1,2,0), ZUKAN_INFO_ALIGN_CENTER, NULL );  // ポケモンずかん　とうろく　かんりょう！
    {
      WORDSET* wordset = WORDSET_Create( work->heap_id );  // WORDSET_RegisterPokeMonsName, WORDSET_RegisterPokeMonsNamePPP  // PPP_Get( ppp, ID_PARA_monno, NULL );  // POKEMON_PASO_PARAM* ppp
      WORDSET_RegisterNumber( wordset, 0, work->monsno, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
      Zukan_Info_DrawStr( work->heap_id, work->bmpwin[ZUKAN_INFO_MSG_NAME], msgdata_common, work->print_que, work->font,
                          ZNK_POKELIST_17, 8, 7, PRINTSYS_LSB_Make(1,2,0), ZUKAN_INFO_ALIGN_LEFT, wordset );  // 025
      WORDSET_Delete( wordset );
    }
    {
      STRBUF* strbuf;
      strbuf = GFL_MSG_CreateString( GlobalMsg_PokeName, work->monsno );
      PRINTSYS_PrintQueColor( work->print_que, GFL_BMPWIN_GetBmp( work->bmpwin[ZUKAN_INFO_MSG_NAME] ),
                              48, 7, strbuf, work->font, PRINTSYS_LSB_Make(1,2,0) );  // ピカチュウ
      GFL_STR_DeleteBuffer( strbuf );
    }
    Zukan_Info_DrawStr( work->heap_id, work->bmpwin[ZUKAN_INFO_MSG_NAME], msgdata_kind, work->print_que, work->font,
                        ZKN_TYPE_000 + work->monsno_msg, 16, 23, PRINTSYS_LSB_Make(1,2,0), ZUKAN_INFO_ALIGN_LEFT, NULL );  // ねずみポケモン
    Zukan_Info_DrawStr( work->heap_id, work->bmpwin[ZUKAN_INFO_MSG_PHYSICAL], msgdata_common, work->print_que, work->font,
                        ZNK_POKELIST_19, 0, 4, PRINTSYS_LSB_Make(1,2,0), ZUKAN_INFO_ALIGN_LEFT, NULL );  // たかさ
    Zukan_Info_DrawStr( work->heap_id, work->bmpwin[ZUKAN_INFO_MSG_PHYSICAL], msgdata_common, work->print_que, work->font,
                        ZNK_POKELIST_20, 0, 20, PRINTSYS_LSB_Make(1,2,0), ZUKAN_INFO_ALIGN_LEFT, NULL );  // おもさ
    Zukan_Info_DrawStr( work->heap_id, work->bmpwin[ZUKAN_INFO_MSG_PHYSICAL], msgdata_height, work->print_que, work->font,
                        ZKN_HEIGHT_000 + work->monsno_msg, 94, 4, PRINTSYS_LSB_Make(1,2,0), ZUKAN_INFO_ALIGN_RIGHT, NULL );  // ？？？.？m  // 左上指定なら(40, 4)  // mとkgで小数点が揃うように
    Zukan_Info_DrawStr( work->heap_id, work->bmpwin[ZUKAN_INFO_MSG_PHYSICAL], msgdata_weight, work->print_que, work->font,
                        ZKN_GRAM_000 + work->monsno_msg, 100, 20, PRINTSYS_LSB_Make(1,2,0), ZUKAN_INFO_ALIGN_RIGHT, NULL );  // ？？？.？kg  // 左上指定なら(40,20)
    Zukan_Info_DrawStr( work->heap_id, work->bmpwin[ZUKAN_INFO_MSG_EXPLAIN], msgdata_explain, work->print_que, work->font,
                        ZKN_COMMENT_00_000 + work->monsno_msg, 4, 5, PRINTSYS_LSB_Make(0xF,2,0), ZUKAN_INFO_ALIGN_LEFT, NULL );  // かたい　きのみも　でんげきで
    
    for( i=0; i<ZUKAN_INFO_MSG_MAX; i++ )
    {
      GFL_BMPWIN_MakeTransWindow_VBlank( work->bmpwin[i] );
    }
  }

  GFL_MSG_Delete(msgdata_explain);
  GFL_MSG_Delete(msgdata_weight);
  GFL_MSG_Delete(msgdata_height);
  GFL_MSG_Delete(msgdata_kind);
  GFL_MSG_Delete(msgdata_common);
}

static void Zukan_Info_DeleteMessage( ZUKAN_INFO_WORK* work )
{
  s32 i;
  for( i=0; i<ZUKAN_INFO_MSG_MAX; i++ )
  {
    GFL_BMPWIN_Delete( work->bmpwin[i] );
  }
}
#endif //TEST_MSG










#ifdef TEST_TYPEICON_TWICE
static void Zukan_Info_CreateTypeicon( ZUKAN_INFO_WORK* work, PokeType type1, PokeType type2, GFL_CLUNIT* clunit, HEAPID heap_id )
{
  s32 i;
  PokeType type[2];
  GFL_CLWK_DATA data[2] =
  {
    { 8*21, 8*11 -4, 0, 0, 0 },
    { 8*26, 8*11 -4, 0, 0, 0 },
  };

  type[0] = type1;
  type[1] = type2;

  // リソース読み込み
  {
    ARCHANDLE* handle = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), heap_id );

    for( i=0; i<2; i++ )
    {
      work->typeicon_cg_idx[i] = GFL_CLGRP_REGISTER_FAILED;
      if( type[i] == POKETYPE_NULL )
      {
        continue;
      }
      work->typeicon_cg_idx[i] = GFL_CLGRP_CGR_Register( handle,
                                                         APP_COMMON_GetPokeTypeCharArcIdx(type[i]),
                                                         FALSE, CLSYS_DRAW_MAIN, heap_id );
    }

    work->typeicon_cl_idx = GFL_CLGRP_PLTT_RegisterEx( handle,
                                                       APP_COMMON_GetPokeTypePltArcIdx(),
                                                       CLSYS_DRAW_MAIN,
                                                       PLTID_OBJ_TYPEICON_M * 0x20,
                                                       0, 3, heap_id );

    work->typeicon_cean_idx = GFL_CLGRP_CELLANIM_Register( handle,
                                                           APP_COMMON_GetPokeTypeCellArcIdx( APP_COMMON_MAPPING_128K ),
                                                           APP_COMMON_GetPokeTypeAnimeArcIdx( APP_COMMON_MAPPING_128K ),
                                                           heap_id );
  
    GFL_ARC_CloseDataHandle( handle );
  }

  // CLWK作成
  {
    for( i=0; i<2; i++ )
    {
      if( work->typeicon_cg_idx[i] == GFL_CLGRP_REGISTER_FAILED )
      {
        continue;
      }
      work->typeicon_clwk[i] = GFL_CLACT_WK_Create( clunit,
                                                    work->typeicon_cg_idx[i],
                                                    work->typeicon_cl_idx,
                                                    work->typeicon_cean_idx,
                                                    &(data[i]),
                                                    CLSYS_DEFREND_MAIN, heap_id );
      GFL_CLACT_WK_SetPlttOffs( work->typeicon_clwk[i], APP_COMMON_GetPokeTypePltOffset(type[i]),
                                CLWK_PLTTOFFS_MODE_PLTT_TOP );
      GFL_CLACT_WK_SetSoftPri( work->typeicon_clwk[i], 2 );  // 手前 > ポケモン2D > 足跡 > 属性アイコン > 奥
      GFL_CLACT_WK_SetObjMode( work->typeicon_clwk[i], GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので
    }
  }
}
static void Zukan_Info_DeleteTypeicon( ZUKAN_INFO_WORK* work )
{
  s32 i=0;

  // CLWK破棄
  for( i=0; i<2; i++ )
  {
    if( work->typeicon_cg_idx[i] == GFL_CLGRP_REGISTER_FAILED )
    {
      continue;
    }
    GFL_CLACT_WK_Remove( work->typeicon_clwk[i] );
  }

  // リソース破棄
  GFL_CLGRP_PLTT_Release( work->typeicon_cl_idx );
  for( i=0; i<2; i++ )
  {
    if( work->typeicon_cg_idx[i] == GFL_CLGRP_REGISTER_FAILED )
    {
      continue;
    }
    GFL_CLGRP_CGR_Release( work->typeicon_cg_idx[i] );
  }
  GFL_CLGRP_CELLANIM_Release( work->typeicon_cean_idx );
}
#endif //TEST_TYPEICON_TWICE








#ifdef UI_TEST_POKE2D
//=============================================================================
/**
 *	ポケモンOBJ,BG読みこみ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	リソース読みこみ
 *
 *	@param	UI_TEST_MAIN_WORK *wk	ワーク
 *	@param	heapID										ヒープID
 */
//-----------------------------------------------------------------------------
static void UITemplate_POKE2D_LoadResourceOBJ( ZUKAN_INFO_WORK *wk, HEAPID heapID )
{	
	POKEMON_PASO_PARAM	*ppp;
	ARCHANDLE						*handle;

	//PPP作成
	ppp	= (POKEMON_PASO_PARAM	*)PP_Create( (u16)wk->monsno, 0, 0, heapID );

	//ハンドル
	handle	= POKE2DGRA_OpenHandle( heapID );
	//リソース読みこみ
	wk->ncg_poke2d	= POKE2DGRA_OBJ_CGR_RegisterPPP( handle, ppp, POKEGRA_DIR_FRONT, CLSYS_DRAW_MAIN, heapID );
	wk->ncl_poke2d	= POKE2DGRA_OBJ_PLTT_RegisterPPP( handle, ppp, POKEGRA_DIR_FRONT ,CLSYS_DRAW_MAIN,  PLTID_OBJ_POKE_M*0x20,  heapID );
	wk->nce_poke2d	= POKE2DGRA_OBJ_CELLANM_RegisterPPP( ppp, POKEGRA_DIR_FRONT, APP_COMMON_MAPPING_128K, CLSYS_DRAW_MAIN, heapID );
	GFL_ARC_CloseDataHandle( handle );

	//PP破棄
	GFL_HEAP_FreeMemory( ppp );
}
//----------------------------------------------------------------------------
/**
 *	@brief	リソース破棄
 *
 *	@param	UI_TEST_MAIN_WORK *wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void UITemplate_POKE2D_UnLoadResourceOBJ( ZUKAN_INFO_WORK *wk )
{	
	//リソース破棄
	GFL_CLGRP_PLTT_Release( wk->ncl_poke2d );
	GFL_CLGRP_CGR_Release( wk->ncg_poke2d );
	GFL_CLGRP_CELLANIM_Release( wk->nce_poke2d );

}
//----------------------------------------------------------------------------
/**
 *	@brief	CLWK作成
 *
 *	@param	UI_TEST_MAIN_WORK *wk	ワーク
 *	@param	*clunit	ユニット
 *	@param	heapID	ヒープID
 */
//-----------------------------------------------------------------------------
static void UITemplate_POKE2D_CreateCLWK( ZUKAN_INFO_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID, u16 pos_x, u16 pos_y )
{	
	GFL_CLWK_DATA	cldata;
		GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
		cldata.pos_x	= pos_x;
		cldata.pos_y	= pos_y;
		wk->clwk_poke2d	= GFL_CLACT_WK_Create( clunit, 
				wk->ncg_poke2d,
				wk->ncl_poke2d,
				wk->nce_poke2d,
				&cldata, 
				CLSYS_DEFREND_MAIN, heapID );
  
  GFL_CLACT_WK_SetSoftPri( wk->clwk_poke2d, 0 );  // 手前 > ポケモン2D > 足跡 > 属性アイコン > 奥  // clunitの優先順位は生成時に0にしてあったので、BGで最前面のものより手前になる？
}

//----------------------------------------------------------------------------
/**
 *	@brief	CLWK破棄
 *
 *	@param	UI_TEST_MAIN_WORK *wk ワーク
 */
//-----------------------------------------------------------------------------
static void UITemplate_POKE2D_DeleteCLWK( ZUKAN_INFO_WORK *wk )
{	
	GFL_CLACT_WK_Remove( wk->clwk_poke2d );
}
#endif //UI_TEST_POKE2D




/*
マッピングモードを1Dモード(128K)としているのだが、
CGR charDataのマッピングモードをレジスタの値に書き換えました。register[0x200010] resource=[0x10]
CGR charDataのマッピングモードをレジスタの値に書き換えました。register[0x200010] resource=[0x0]
と表示される。これは表示されるだけで、ASSERTというわけではないので気にしなくていいかな。
ちなみにこれを表示しているのは、タイプアイコンとポケモン2Dであり、ポケモン足跡は関係ない。
*/
#ifdef TEST_POKEFOOT
static void Zukan_Info_CreatePokefoot( ZUKAN_INFO_WORK* work, u32 monsno, GFL_CLUNIT* unit, HEAPID heap_id )
{
  UI_EASY_CLWK_RES_PARAM prm;

  prm.draw_type = CLSYS_DRAW_MAIN;
  prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NCGR;  // NCLR非圧縮、NCGR圧縮、NCER圧縮、NANR圧縮なので、NCGRの圧縮にしか対応していないこのフラグではうまくいかない → NCLR非圧縮、NCGR圧縮、NCER非圧縮、NANR日圧縮に変更した
  prm.arc_id    = PokeFootArcFileGet();
  prm.pltt_id   = PokeFootPlttDataIdxGet();
  prm.ncg_id    = PokeFootCharDataIdxGet((int)monsno);
  prm.cell_id   = PokeFootCellDataIdxGet();
  prm.anm_id    = PokeFootCellAnmDataIdxGet();
  prm.pltt_line = PLTID_OBJ_POKEFOOT_M;
  prm.pltt_src_ofs = 0;
  prm.pltt_src_num = 1;
  
  UI_EASY_CLWK_LoadResource( &work->clres_pokefoot, &prm, unit, heap_id );

  work->clwk_pokefoot = UI_EASY_CLWK_CreateCLWK( &work->clres_pokefoot, unit, 8*15, 8*11, 0, heap_id );
  
  GFL_CLACT_WK_SetSoftPri( work->clwk_pokefoot, 1 );  // 手前 > ポケモン2D > 足跡 > 属性アイコン > 奥
  
  GFL_CLACT_WK_SetObjMode( work->clwk_pokefoot, GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので
}

static void Zukan_Info_DeletePokefoot( ZUKAN_INFO_WORK* work )
{	
	//CLWK破棄
	GFL_CLACT_WK_Remove( work->clwk_pokefoot );
	//リソース破棄
  UI_EASY_CLWK_UnLoadResource( &work->clres_pokefoot );
}
#endif //TEST_POKEFOOT


static void Zukan_Info_CreateOthers( ZUKAN_INFO_WORK* work )
{

if( work->launch == ZUKAN_INFO_LAUNCH_TOROKU )
{

#ifdef TEST_MSG
  {
    Zukan_Info_CreateMessage( work );
  }
#endif //TEST_MSG

}


if( work->launch == ZUKAN_INFO_LAUNCH_TOROKU )
{

#ifdef TEST_TYPEICON_TWICE
  {
    PokeType type1 = (PokeType)( PP_Get( work->pp, ID_PARA_type1, NULL ) );
    PokeType type2 = (PokeType)( PP_Get( work->pp, ID_PARA_type2, NULL ) );
    if( type1 == type2 )
    {
      type2 = POKETYPE_NULL;
    }
    Zukan_Info_CreateTypeicon( work, type1, type2, work->clunit, work->heap_id );
  }
#endif //TEST_TYPEICON_TWICE

}


if( work->launch == ZUKAN_INFO_LAUNCH_TOROKU )
{

#ifdef TEST_POKEFOOT
  Zukan_Info_CreatePokefoot( work, work->monsno_msg, work->clunit, work->heap_id );
#endif //TEST_POKEFOOT

}

}
