//============================================================================
/**
 *  @file   yesno_menu.c
 *  @brief  下画面を使っての二択メニュー
 *  @author Koji Kawada
 *  @data   2010.01.14
 *  @note   
 *
 *  モジュール名：YESNO_MENU
 */
//============================================================================
//#define NO_USE_OBJ

// 必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

// インクルード
#include "print/gf_font.h"
#include "print/printsys.h"
#ifndef NO_USE_OBJ
#include "ui/ui_easy_clwk.h"
#endif

// サウンド
#include "sound/pm_sndsys.h"

// アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "battle/battgra_wb.naix"
#include "app_menu_common.naix"

// yesno_menu
#include "ui/yesno_menu.h"

//=============================================================================
/**
 *  定数定義
 */
//=============================================================================
// カーソルOBJ
#define CURSOR_NUM (4)

typedef enum
{
  CURSOR_ORIGIN_UP,
  CURSOR_ORIGIN_DOWN,

  CURSOR_ORIGIN_MAX
}
CURSOR_ORIGIN;

typedef enum
{
  CURSOR_POS_X,
  CURSOR_POS_Y,

  CURSOR_POS_MAX
}
CURSOR_POS;

// ボタンのパレットアニメーション
typedef enum
{
  BTN_PAL_ANM_0,
  BTN_PAL_ANM_1,
  BTN_PAL_ANM_2,
  BTN_PAL_ANM_MAX,

  BTN_PAL_ANM_START   = BTN_PAL_ANM_0,
  BTN_PAL_ANM_END     = BTN_PAL_ANM_2,
}
BTN_PAL_ANM;

// ステップ
typedef enum
{
  STEP_BEFORE_OPEN,      // 開く前
  STEP_OPEN,             // 開く
  STEP_SELECT,           // 選択中
  STEP_AFTER_DECIDE,     // 決定後のボタンのパレットアニメーション演出中
  STEP_BEFORE_CLOSE,     // 閉じる前
}
STEP;

//=============================================================================
/**
 *  構造体宣言
 */
//=============================================================================
// カーソルOBJ
typedef struct
{
  GFL_CLWK*               clwk[CURSOR_NUM];
#ifndef NO_USE_OBJ
  UI_EASY_CLWK_RES        res;
#endif
  CURSOR_ORIGIN           origin;
}
CURSOR_SET;

// ワーク
struct _YESNO_MENU_WORK
{
  // 他のところのを覚えているだけで生成や破棄はしない。
  HEAPID                 heap_id;
  GFL_CLUNIT*            clunit;
  GFL_FONT*              font;
  PRINT_QUE*             print_que;

  // ここで管理する。
  PALTYPE                bg_paltype;

  u8                     btn_bg_frame;
  u8                     btn_bg_frame_prio;
  u8                     btn_yes_bg_pal;
  u8                     btn_no_bg_pal;

  u8                     mes_bg_frame;
  u8                     mes_bg_frame_prio;
  u8                     mes_bg_pal;

  CLSYS_DRAW_TYPE        cursor_obj_draw_type;
  u8                     cursor_obj_pal;         // cursor_obj_palとcursor_obj_pal+1の2本を使用する

  BOOL                   key;                    // キー操作のときTRUE  // タッチで選択したらFALSEにする

  u16                    btn_pal_col[BTN_PAL_ANM_MAX][0x10];
  GFL_ARCUTIL_TRANSINFO  btn_transinfo;
  
  GFL_BMPWIN*            mes_yes_bmpwin;
  GFL_BMPWIN*            mes_no_bmpwin;

  STEP                   step;
  YESNO_MENU_SEL         sel;                    // 最後の選択結果
  CURSOR_SET             cursor_set;
  u8                     btn_pal_anm;            // BTN_PAL_ANM  // 決定したボタンのパレットアニメーション
  u16                    count;                  // 決定してから数フレーム待つ際のカウント
};

//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
static u8 YesNo_Menu_GetCursorPos( CURSOR_ORIGIN origin, u8 idx, CURSOR_POS pos );
static void SetPaletteColor( PALTYPE paltype, u8 pal, const u16 pal_col[0x10] );

//=============================================================================
/**
 *  外部公開関数定義
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *  @brief     リソース読み込み
 *
 *  @param[in] heap_id     ヒープID
 *
 *  @retval    生成したワーク
 */
//-----------------------------------------------------------------------------
YESNO_MENU_WORK* YESNO_MENU_CreateRes(
                          HEAPID         heap_id,
                          u8             bg_frame,           // bg_frameとbg_frame+1の2個を使用する
                          u8             bg_frame_prio,      // bg_frame_prioとbg_frame_prio+1に設定する
                          u8             bg_pal,             // bg_palからbg_pal+2の3本を使用する
                          u8             obj_pal,            // obj_palとobj_pal+1の2本を使用する
                          GFL_CLUNIT*    clunit,
                          GFL_FONT*      font,
                          PRINT_QUE*     print_que,
                          BOOL           key )               // キー操作のときTRUE
{
  YESNO_MENU_WORK* work;

  // ワーク
  {
    u32 size = sizeof( YESNO_MENU_WORK );
    work = GFL_HEAP_AllocMemory( heap_id, size );
    GFL_STD_MemClear( work, size );
  }

  // 引数からワークに値を設定する
  {
    work->heap_id               = heap_id;
    work->clunit                = clunit;
    work->font                  = font;
    work->print_que             = print_que;

    work->bg_paltype            = (bg_frame<=GFL_BG_FRAME3_M)?(PALTYPE_MAIN_BG):(PALTYPE_SUB_BG);
    
    work->btn_bg_frame          = bg_frame+1;
    work->mes_bg_frame          = bg_frame;
    GF_ASSERT_MSG( work->btn_bg_frame<=GFL_BG_FRAME3_M || GFL_BG_FRAME0_S<=work->mes_bg_frame, "YESNO_MENU : BGフレームの設定が間違っています。\n" );

    work->btn_bg_frame_prio     = bg_frame_prio+1;
    work->mes_bg_frame_prio     = bg_frame_prio;
    GF_ASSERT_MSG( work->btn_bg_frame_prio<4, "YESNO_MENU : BGフレームのプライオリティ設定が間違っています。\n" );

    work->btn_yes_bg_pal        = bg_pal;
    work->btn_no_bg_pal         = bg_pal+1;
    work->mes_bg_pal            = bg_pal+2;
    GF_ASSERT_MSG( work->mes_bg_pal<16, "YESNO_MENU : BGパレットの設定が間違っています。\n" );

    work->cursor_obj_draw_type  = (bg_frame<=GFL_BG_FRAME3_M)?(CLSYS_DRAW_MAIN):(CLSYS_DRAW_SUB);
    work->cursor_obj_pal        = obj_pal;  // cursor_obj_palとcursor_obj_pal+1の2本を使用する
    GF_ASSERT_MSG( work->cursor_obj_pal+1<16, "YESNO_MENU : OBJパレットの設定が間違っています。\n" );

    work->key                   = key;
  }

  // クリア
  {
    GFL_BG_ClearFrame( work->btn_bg_frame );
    GFL_BG_ClearFrame( work->mes_bg_frame );
  }

  // 開く前の値設定
  {
    work->step                  = STEP_BEFORE_OPEN;
    work->sel                   = YESNO_MENU_SEL_SEL;
    work->cursor_set.origin     = CURSOR_ORIGIN_UP;
  } 

  // btn_bg
  {
    // ファイルオープン
    ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_APP_MENU_COMMON, work->heap_id );

    // パレットアニメーション
    {
      NNSG2dPaletteData* pal_data;
      u16* pal_raw;
      u8 i, j;

      u32 size = GFL_ARC_GetDataSizeByHandle( handle, NARC_app_menu_common_sentaku_win_NCLR );
      void* buf = GFL_HEAP_AllocMemory( work->heap_id, size );
      GFL_ARC_LoadDataByHandle( handle, NARC_app_menu_common_sentaku_win_NCLR, buf );
      NNS_G2dGetUnpackedPaletteData( buf, &pal_data );
      pal_raw = (u16*)(pal_data->pRawData);

      for( i=0; i<BTN_PAL_ANM_MAX; i++ )
      {
        for( j=0; j<0x10; j++ )
        {
          work->btn_pal_col[i][j] = pal_raw[ (1+i)*0x10 + j ];  // 0番目(yesの普通色)と1番目(noの普通色)は同じ色なので、1番目から色を読み込んだ
        }
      }

      GFL_HEAP_FreeMemory( buf );
    }

    // パレット
    GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_app_menu_common_sentaku_win_NCLR, work->bg_paltype,
                                      work->btn_yes_bg_pal*0x20, 2*0x20, work->heap_id );  // 0番目(yesの普通色)と1番目(noの普通色)だけ転送すればいい

    // キャラ
    work->btn_transinfo = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( handle,
                                                                       NARC_app_menu_common_sentaku_win_NCGR,
                                                                       work->btn_bg_frame,
                                                                       13*1*GFL_BG_1CHRDATASIZ,
                                                                       FALSE, work->heap_id );
    GF_ASSERT_MSG( work->btn_transinfo != GFL_ARCUTIL_TRANSINFO_FAIL, "YESNO_MENU : BGキャラ領域が足りませんでした。\n" );

    // スクリーン
    GFL_ARCHDL_UTIL_TransVramScreen( handle, NARC_app_menu_common_sentaku_win_NSCR,
                                     work->btn_bg_frame,
                                     GFL_ARCUTIL_TRANSINFO_GetPos( work->btn_transinfo ),
                                     32*24*GFL_BG_1SCRDATASIZ,
                                     FALSE, work->heap_id );
    GFL_BG_ChangeScreenPalette( work->btn_bg_frame, 0, 0, 32, 11, work->btn_yes_bg_pal );
    GFL_BG_ChangeScreenPalette( work->btn_bg_frame, 0, 11, 32, 13, work->btn_no_bg_pal );
    GFL_BG_LoadScreenReq( work->btn_bg_frame );
 
    // ファイルクローズ 
    GFL_ARC_CloseDataHandle( handle );

    // パレットの色を設定する
    SetPaletteColor( work->bg_paltype, work->btn_yes_bg_pal, work->btn_pal_col[BTN_PAL_ANM_START] );
    SetPaletteColor( work->bg_paltype, work->btn_no_bg_pal, work->btn_pal_col[BTN_PAL_ANM_START] );
    // 非表示
    GFL_BG_SetVisible( work->btn_bg_frame, VISIBLE_OFF );
  }

  // mes_bg
  {
    // パレット
    GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, work->bg_paltype,
                                   work->mes_bg_pal*0x20, 1*0x20, work->heap_id );

    // BMPWIN
    work->mes_yes_bmpwin = GFL_BMPWIN_Create( work->mes_bg_frame,
                                              0, 5, 32, 6,
                                              work->mes_bg_pal, GFL_BMP_CHRAREA_GET_B );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->mes_yes_bmpwin), 0 );
    
    work->mes_no_bmpwin  = GFL_BMPWIN_Create( work->mes_bg_frame,
                                              0, 11, 32, 6,
                                              work->mes_bg_pal, GFL_BMP_CHRAREA_GET_B );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->mes_no_bmpwin), 0 );

    // 非表示
    GFL_BG_SetVisible( work->mes_bg_frame, VISIBLE_OFF );
  }

#ifndef NO_USE_OBJ
  // cursor_obj
  {
    u8 i;
    UI_EASY_CLWK_RES_PARAM param;
    
    param.draw_type       = work->cursor_obj_draw_type;
    param.comp_flg        = UI_EASY_CLWK_RES_COMP_NONE;
    param.arc_id          = ARCID_BATTGRA;
    param.pltt_id         = NARC_battgra_wb_battle_w_obj_NCLR;
    param.ncg_id          = NARC_battgra_wb_battle_w_cursor_NCGR;
    param.cell_id         = NARC_battgra_wb_battle_w_cursor_NCER;
    param.anm_id          = NARC_battgra_wb_battle_w_cursor_NANR;
    param.pltt_line       = work->cursor_obj_pal;
    param.pltt_src_ofs    = 0;  // 空いているところはパレット番号0で描かれており、カーソルはパレット番号1で描かれているので、
    param.pltt_src_num    = 2;  // パレットは2本必要でなる。
  
    UI_EASY_CLWK_LoadResource( &(work->cursor_set.res), &param, work->clunit, work->heap_id );

    for( i=0; i<CURSOR_NUM; i++ )
    {
      work->cursor_set.clwk[i] = UI_EASY_CLWK_CreateCLWK( &(work->cursor_set.res), work->clunit,
                                   YesNo_Menu_GetCursorPos(work->cursor_set.origin, i, CURSOR_POS_X),
                                   YesNo_Menu_GetCursorPos(work->cursor_set.origin, i, CURSOR_POS_Y),
                                   i,
                                   work->heap_id );
      GFL_CLACT_WK_SetBgPri( work->cursor_set.clwk[i], work->mes_bg_frame_prio );

      GFL_CLACT_WK_SetDrawEnable( work->cursor_set.clwk[i], FALSE );
      GFL_CLACT_WK_SetAutoAnmFlag( work->cursor_set.clwk[i], FALSE );
    }

    // 表示
    if( work->cursor_obj_draw_type == CLSYS_DRAW_MAIN )
    {
      GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
    }
    else
    {
      GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
    }
  }
#endif

  return work;
}

//-----------------------------------------------------------------------------
/**
 *  @brief     リソース破棄
 *
 *  @param[in] work     YESNO_MENU_CreateResで生成したワーク
 *
 *  @retval    
 */
//-----------------------------------------------------------------------------
void YESNO_MENU_DeleteRes( YESNO_MENU_WORK* work )
{
#ifndef NO_USE_OBJ
  // cursor_obj 
  {
    u8 i;
    for( i=0; i<CURSOR_NUM; i++ )
    {
      GFL_CLACT_WK_Remove( work->cursor_set.clwk[i] );
    }
    UI_EASY_CLWK_UnLoadResource( &(work->cursor_set.res) );
  }
#endif

  // mes_bg
  {
    GFL_BMPWIN_Delete( work->mes_yes_bmpwin );
    GFL_BMPWIN_Delete( work->mes_no_bmpwin );
  }

  // btn_bg
  {
    GFL_BG_FreeCharacterArea( work->btn_bg_frame, GFL_ARCUTIL_TRANSINFO_GetPos( work->btn_transinfo ),
                              GFL_ARCUTIL_TRANSINFO_GetSize( work->btn_transinfo ) );
  }

  // ワーク
  {
    GFL_HEAP_FreeMemory( work );
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief     開く
 *
 *  @param[in] work     YESNO_MENU_CreateResで生成したワーク
 *
 *  @retval    
 */
//-----------------------------------------------------------------------------
void YESNO_MENU_OpenMenu( YESNO_MENU_WORK* work, STRBUF* yes, STRBUF* no )
{
  // 開いたときの値設定
  {
    work->step                  = STEP_OPEN;
    work->sel                   = YESNO_MENU_SEL_SEL;
    work->cursor_set.origin     = CURSOR_ORIGIN_UP;

#ifndef NO_USE_OBJ
    // cursor_obj
    {
      u8 i;
      for( i=0; i<CURSOR_NUM; i++ )
      {
        GFL_CLACTPOS pos;
        pos.x = (s16)( YesNo_Menu_GetCursorPos(work->cursor_set.origin, i, CURSOR_POS_X) );
        pos.y = (s16)( YesNo_Menu_GetCursorPos(work->cursor_set.origin, i, CURSOR_POS_Y) );
        GFL_CLACT_WK_SetPos( work->cursor_set.clwk[i], &pos, work->cursor_obj_draw_type );
      }
    }
#endif
  }

  // btn_bg
  {
    // パレットの色を設定する
    SetPaletteColor( work->bg_paltype, work->btn_yes_bg_pal, work->btn_pal_col[BTN_PAL_ANM_START] );
    SetPaletteColor( work->bg_paltype, work->btn_no_bg_pal, work->btn_pal_col[BTN_PAL_ANM_START] );
    // 表示
    GFL_BG_SetVisible( work->btn_bg_frame, VISIBLE_ON );
  }

  // mes_bg
  {
    PRINTSYS_LSB lsb = PRINTSYS_LSB_Make(0xF,2,0);
    u32 yes_width, no_width;

    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->mes_yes_bmpwin), 0 );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->mes_no_bmpwin), 0 );
    
    yes_width = PRINTSYS_GetStrWidth( yes, work->font, 0 );
    PRINTSYS_PrintQueColor( work->print_que, GFL_BMPWIN_GetBmp(work->mes_yes_bmpwin),
                            (u16)( (32*8 - yes_width) / 2 ), 16,
                            yes, work->font, lsb );

    no_width = PRINTSYS_GetStrWidth( no, work->font, 0 );
    PRINTSYS_PrintQueColor( work->print_que, GFL_BMPWIN_GetBmp(work->mes_no_bmpwin),
                            (u16)( (32*8 - no_width) / 2 ), 16,
                            no, work->font, lsb );

    // 表示
    GFL_BG_SetVisible( work->mes_bg_frame, VISIBLE_ON );

    // 転送
    GFL_BMPWIN_MakeTransWindow_VBlank(work->mes_yes_bmpwin);
    GFL_BMPWIN_MakeTransWindow_VBlank(work->mes_no_bmpwin);
  }

  if( work->key )
  {
#ifndef NO_USE_OBJ
    // cursor_obj
    {
      u8 i;
      for( i=0; i<CURSOR_NUM; i++ )
      {
        GFL_CLACT_WK_SetDrawEnable( work->cursor_set.clwk[i], TRUE );
        GFL_CLACT_WK_SetAutoAnmFlag( work->cursor_set.clwk[i], TRUE );
      }
    }
#endif
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief     閉じる
 *
 *  @param[in] work     YESNO_MENU_CreateResで生成したワーク
 *
 *  @retval    
 */
//-----------------------------------------------------------------------------
void YESNO_MENU_CloseMenu( YESNO_MENU_WORK* work )
{
  // btn_bg
  {
    // 非表示
    GFL_BG_SetVisible( work->btn_bg_frame, VISIBLE_OFF );
  }
 
  // mes_bg
  {
    // 非表示
    GFL_BG_SetVisible( work->mes_bg_frame, VISIBLE_OFF );
  }

#ifndef NO_USE_OBJ
  // cursor_obj
  {
    u8 i;
    for( i=0; i<CURSOR_NUM; i++ )
    {
      GFL_CLACT_WK_SetDrawEnable( work->cursor_set.clwk[i], FALSE );
      GFL_CLACT_WK_SetAutoAnmFlag( work->cursor_set.clwk[i], FALSE );
    }
  }
#endif
  
  // 閉じたときの値設定
  {
    work->step = STEP_BEFORE_OPEN;
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief     主処理
 *
 *  @param[in] work     YESNO_MENU_CreateResで生成したワーク
 *
 *  @retval    
 */
//-----------------------------------------------------------------------------
void YESNO_MENU_Main( YESNO_MENU_WORK* work )
{
  BOOL btn_pal_anm_update = FALSE;

  u32 tp_x, tp_y;
  BOOL tp_trg;
  int key_trg = GFL_UI_KEY_GetTrg();
  tp_trg = GFL_UI_TP_GetPointTrg(&tp_x, &tp_y);

  switch( work->step )
  {
  case STEP_BEFORE_OPEN:
    {
      // 何もしない
    }
    break;
  case STEP_OPEN:  // 誤入力を防ぐために、開いた直後に1フレーム待つ
    {
      // 次へ
      work->step = STEP_SELECT;
    }
    break;
  case STEP_SELECT:
    {
      BOOL decide = FALSE;

      // キー操作
      if( !decide )
      {
        if( work->key )
        {
          if( key_trg )
          {
            BOOL move = FALSE;

            if( key_trg & PAD_BUTTON_A )
            {
              PMSND_PlaySE( SEQ_SE_DECIDE2 );
              if( work->cursor_set.origin == CURSOR_ORIGIN_UP )
              {
                work->sel = YESNO_MENU_SEL_YES;
              }
              else
              {
                work->sel = YESNO_MENU_SEL_NO;
              }
              decide = TRUE;
            }
            else if( key_trg & PAD_BUTTON_B )
            {
              PMSND_PlaySE( SEQ_SE_CANCEL2 );
              work->sel = YESNO_MENU_SEL_NO;
              decide = TRUE;
            }
            else if( key_trg & PAD_KEY_UP )
            {
              if( work->cursor_set.origin != CURSOR_ORIGIN_UP )
              {
                work->cursor_set.origin = CURSOR_ORIGIN_UP;
                move = TRUE;
              }
            }
            else if( key_trg & PAD_KEY_DOWN )
            {
              if( work->cursor_set.origin != CURSOR_ORIGIN_DOWN )
              {
                work->cursor_set.origin = CURSOR_ORIGIN_DOWN;
                move = TRUE;
              }
            }
            
            if( move )
            {
#ifndef NO_USE_OBJ
              // cursor_obj
              {
                u8 i;
                for( i=0; i<CURSOR_NUM; i++ )
                {
                  GFL_CLACTPOS pos;
                  pos.x = (s16)( YesNo_Menu_GetCursorPos(work->cursor_set.origin, i, CURSOR_POS_X) );
                  pos.y = (s16)( YesNo_Menu_GetCursorPos(work->cursor_set.origin, i, CURSOR_POS_Y) );
                  GFL_CLACT_WK_SetPos( work->cursor_set.clwk[i], &pos, work->cursor_obj_draw_type );
                }
              }
#endif

              PMSND_PlaySE( SEQ_SE_SELECT1 );
            }
          }
        }
      }

      // タッチ
      if( !decide )
      {
        if( tp_trg )
        {
          const GFL_UI_TP_HITTBL tp_data[] =
          {
            { 32,  80-1, 8, 248-1 },
            { 80, 128-1, 8, 248-1 },
            { GFL_UI_TP_HIT_END, 0,0,0 },
          };

          int hit = GFL_UI_TP_HitSelf( tp_data, tp_x, tp_y );
          if( hit != GFL_UI_TP_HIT_NONE )
          {
            PMSND_PlaySE( SEQ_SE_DECIDE2 );
            if( hit == 0 )
            {
              work->sel = YESNO_MENU_SEL_YES;
            }
            else if( hit == 1 )
            {
              work->sel = YESNO_MENU_SEL_NO;
            }

            decide = TRUE;
            work->key = FALSE;  // タッチで終了
          }
        }
      }

      // 初めてのキー操作
      if( !decide )
      {
        if( !(work->key) )
        {
          if( key_trg )
          {
#ifndef NO_USE_OBJ
            // cursor_obj
            {
              u8 i;
              for( i=0; i<CURSOR_NUM; i++ )
              {
                GFL_CLACT_WK_SetDrawEnable( work->cursor_set.clwk[i], TRUE );
                GFL_CLACT_WK_SetAutoAnmFlag( work->cursor_set.clwk[i], TRUE );
              }
            }
#endif

            work->key = TRUE;
          }
        }
      }

      // 決定したか？
      if( decide )
      {
        // 次へ
        work->step = STEP_AFTER_DECIDE;

        work->btn_pal_anm = BTN_PAL_ANM_START;
        work->btn_pal_anm += 1;
        work->count = 0;

        btn_pal_anm_update = TRUE;
      }
    }
    break;
  case STEP_AFTER_DECIDE:
    {
      if( work->count < 30 )
      {
        work->count++;

        if( work->btn_pal_anm < BTN_PAL_ANM_MAX )
        {
          work->btn_pal_anm += 1;
          if( work->btn_pal_anm <= BTN_PAL_ANM_END )
          {
            btn_pal_anm_update = TRUE;
          }
        }
      }
      else
      {
        // 次へ
        work->step = STEP_BEFORE_CLOSE;
      }
    }
    break;
  case STEP_BEFORE_CLOSE:
    {
      // 何もしない
    }
    break;
  }

  // 更新
  if( btn_pal_anm_update )
  {
    // btn_bg
    {
      // パレットの色を設定する
      SetPaletteColor( work->bg_paltype,
        (work->sel==YESNO_MENU_SEL_YES)?(work->btn_yes_bg_pal):(work->btn_no_bg_pal),
        work->btn_pal_col[work->btn_pal_anm] );
    }
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief     選択結果を得る
 *
 *  @param[in] work     YESNO_MENU_CreateResで生成したワーク
 *
 *  @retval    
 */
//-----------------------------------------------------------------------------
YESNO_MENU_SEL YESNO_MENU_GetSelect( YESNO_MENU_WORK* work )
{
  if( work->step != STEP_BEFORE_OPEN && work->step != STEP_BEFORE_CLOSE )
    return YESNO_MENU_SEL_SEL;  // アニメーションが終わるまでは選択中を返しておく
  return work->sel;
}

//-----------------------------------------------------------------------------
/**
 *  @brief     キー操作で選択したか
 *
 *  @param[in] work     YESNO_MENU_CreateResで生成したワーク
 *
 *  @retval    
 */
//-----------------------------------------------------------------------------
BOOL YESNO_MENU_IsKey( YESNO_MENU_WORK* work )  // キー操作で選択したときTRUEを返す
{
  return work->key;
}

//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// カーソルOBJの表示位置を得る
//=====================================
static u8 YesNo_Menu_GetCursorPos( CURSOR_ORIGIN origin, u8 idx, CURSOR_POS pos )
{
  const u8 cursor_origin[CURSOR_ORIGIN_MAX][CURSOR_POS_MAX] =  // 左上
  {
    { 0, 40 },
    { 0, 88 },
  };
  const u8 cursor_pos[CURSOR_NUM][CURSOR_POS_MAX] =
  {
    {   0   ,  0    },  // 左上
    {   0   , 48 -1 },  // 左下
    { 256 -1,  0    },  // 右上
    { 256 -1, 48 -1 },  // 右下
  };
  return ( cursor_origin[origin][pos] + cursor_pos[idx][pos] );
}

//-------------------------------------
/// パレットの色を設定する
//=====================================
static void SetPaletteColor( PALTYPE paltype, u8 pal, const u16 pal_col[0x10] )
{
  NNS_GFD_DST_TYPE dst_type;

  switch( paltype )
  {
  case PALTYPE_MAIN_BG:   dst_type = NNS_GFD_DST_2D_BG_PLTT_MAIN;    break; 
  case PALTYPE_MAIN_OBJ:  dst_type = NNS_GFD_DST_2D_OBJ_PLTT_MAIN;   break;
  case PALTYPE_SUB_BG:    dst_type = NNS_GFD_DST_2D_BG_PLTT_SUB;     break;
  case PALTYPE_SUB_OBJ:   dst_type = NNS_GFD_DST_2D_OBJ_PLTT_SUB;    break;
  default:                return;                                    break;
  }

  NNS_GfdRegisterNewVramTransferTask( dst_type,
    pal * 0x20,
    (void*)pal_col, 0x20 );
}

