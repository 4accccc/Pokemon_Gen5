//============================================================================================
/**
 * @file  mb_main.c
 * @brief メールボックス画面 メイン処理
 * @author  Hiroyuki Nakamura
 * @date  09.01.31
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"
#include "arc/arc_def.h"
#include "font/font.naix"
#include "print/printsys.h"
#include "poke_tool/pokeparty.h"
#include "arc/message.naix"
#include "msg/msg_mailbox.h"

#include "mb_main.h"
#include "mb_seq.h"
#include "mb_bmp.h"

#include "mail_gra.naix"


//============================================================================================
//  定数定義
//============================================================================================
#define BGWF_MAILBTN_SX     ( 14 )
#define BGWF_MAILBTN_SY     ( 4 )
#define BGWF_MAILBTN_PX     ( 1 )
#define BGWF_MAILBTN_PY     ( 0 )
#define BGWF_MAILBTN_SPACE_X  ( 16 )
#define BGWF_MAILBTN_SPACE_Y  ( 4 )

#define BGWF_SELMAIL_PX     ( 4 )   // 選択したメールタブ表示Ｘ座標
#define BGWF_SELMAIL_PY     ( 7 )   // 選択したメールタブ表示Ｙ座標

// ボタン背景キャラ位置
#define BTN_U_BACK_CGX_POS  ( 32+7 * 0x20 )
#define BTN_D_BACK_CGX_POS  ( 32+7 * 0x20 )

#define EXP_BUF_SIZE    ( 1024 )  // 汎用文字列展開領域サイズ

#define SELBOX_LIST_NUM   ( MBMAIN_MENU_MAX )   // 選択ボックス項目数

#define SELBOX_PX ( 18 )  // メニュー表示Ｘ座標
#define SELBOX_PY_4 ( 5 ) // メニュー表示Ｙ座標（４項目）
#define SELBOX_PY_3 ( 11 )  // メニュー表示Ｙ座標（２項目）
#define SELBOX_SX ( 11 )  // メニューＸサイズ


//============================================================================================
//  プロトタイプ宣言
//============================================================================================


//============================================================================================
//  グローバル変数
//============================================================================================

// VRAM割り振り
const GFL_DISP_VRAM MAILBOX_VramTbl = {
  GX_VRAM_BG_128_B,       // メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット

  GX_VRAM_SUB_BG_128_C,     // サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット

  GX_VRAM_OBJ_64_E,       // メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット

  GX_VRAM_SUB_OBJ_16_I,     // サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット

  GX_VRAM_TEX_0_A,        // テクスチャイメージスロット
  GX_VRAM_TEXPLTT_01_FG,     // テクスチャパレットスロット

  GX_OBJVRAMMODE_CHAR_1D_32K,
  GX_OBJVRAMMODE_CHAR_1D_32K,
};
/*
// 選択ボックスヘッダデータ
static const SELBOX_HEAD_PRM SelBoxHedParam = {
  TRUE,         // ループ
  SBOX_OFSTYPE_CENTER,  // センタリング
  0,            // Ｘ座標表示オフセット
  MBMAIN_BGF_MSG_M,   // 表示ＢＧ
  MBMAIN_MBG_PAL_SELBOX,  // パレット
  0,            // BGプライオリティ ( ? )
  0,            // ソフトウェアプライオリティ ( ?? )
  MBMAIN_SELBOX_CGX_NUM,  // 文字列領域cgx(キャラ単位)
  MBMAIN_SELBOX_FRM_NUM,  // フレーム領域cgx(キャラ単位)
  MBMAIN_SELBOX_CGX_SIZ,  // 占有するcgx領域サイズ(キャラ単位)
};
*/


//--------------------------------------------------------------------------------------------
/**
 * VBLANK関数
 *
 * @param work  メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBMAIN_VBlank( GFL_TCB *tcb, void *work )
{
  MAILBOX_SYS_WORK * syswk = work;

  GFL_BG_VBlankFunc();
  
  GFL_CLACT_SYS_VBlankFunc();

}

//--------------------------------------------------------------------------------------------
/**
 * VRAM設定
 *
 * @param none
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBMAIN_VramInit(void)
{
  GFL_DISP_SetBank( &MAILBOX_VramTbl );
}

//--------------------------------------------------------------------------------------------
/**
 * ＢＧ初期化
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBMAIN_BgInit( MAILBOX_SYS_WORK * syswk )
{
  GFL_BG_Init( HEAPID_MAILBOX_APP );
  { // BG SYSTEM
    GFL_BG_SYS_HEADER sysh = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
    };
    GFL_BG_SetBGMode( &sysh );
  }
  { // メイン画面：メールメッセージ
    GFL_BG_BGCNT_HEADER cnth= {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x18000, GFL_BG_CHRSIZ_256x128,
      GX_BG_EXTPLTT_01,      0, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( MBMAIN_BGF_MAILMES_S, &cnth, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen(  MBMAIN_BGF_MAILMES_S );
    GFL_BG_SetClearCharacter( MBMAIN_BGF_MAILMES_S, 0x20, 0, HEAPID_MAILBOX_APP );
  }
  { // メイン画面：メール背景
    GFL_BG_BGCNT_HEADER cnth= {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x128,
      GX_BG_EXTPLTT_01,    1, 0, 0, FALSE
    };
    GFL_BG_SetBGControl(  MBMAIN_BGF_MAIL_S, &cnth, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen( MBMAIN_BGF_MAIL_S );
    GFL_BG_SetClearCharacter( MBMAIN_BGF_MAIL_S, 0x20, 0, HEAPID_MAILBOX_APP );
  }
  { // メイン画面：文字
    GFL_BG_BGCNT_HEADER cnth= {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x128,
      GX_BG_EXTPLTT_01,      2, 0, 0, FALSE
    };
    GFL_BG_SetBGControl(  MBMAIN_BGF_STR_S, &cnth, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen( MBMAIN_BGF_STR_S );
    GFL_BG_SetClearCharacter( MBMAIN_BGF_STR_S, 0x20, 0, HEAPID_MAILBOX_APP );
  }
  { // メイン画面：背景
    GFL_BG_BGCNT_HEADER cnth= {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x128,
      GX_BG_EXTPLTT_01,      2, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( MBMAIN_BGF_BG_S, &cnth, GFL_BG_MODE_TEXT );
  }

  { // サブ画面：メッセージ
    GFL_BG_BGCNT_HEADER cnth= {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x18000, GFL_BG_CHRSIZ_256x128,
      GX_BG_EXTPLTT_01,      0, 0, 0, FALSE
    };
    GFL_BG_SetBGControl(  MBMAIN_BGF_MSG_M, &cnth, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen( MBMAIN_BGF_MSG_M );
    GFL_BG_SetClearCharacter( MBMAIN_BGF_MSG_M, 0x20, 0, HEAPID_MAILBOX_APP );
  }
  { // サブ画面：文字
    GFL_BG_BGCNT_HEADER cnth= {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x128,
      GX_BG_EXTPLTT_01,      1, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( MBMAIN_BGF_STR_M, &cnth, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen( MBMAIN_BGF_STR_M );
    GFL_BG_SetClearCharacter( MBMAIN_BGF_STR_M, 0x20, 0, HEAPID_MAILBOX_APP );
  }
  { // サブ画面：ボタン
    GFL_BG_BGCNT_HEADER cnth= {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x128,
      GX_BG_EXTPLTT_01,      2, 0, 0, FALSE
    };
    GFL_BG_SetBGControl(  MBMAIN_BGF_BTN_M, &cnth, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen( MBMAIN_BGF_BTN_M );
  }
  { // サブ画面：背景
    GFL_BG_BGCNT_HEADER cnth= {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x128,
      GX_BG_EXTPLTT_01,      3, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( MBMAIN_BGF_BG_M, &cnth, GFL_BG_MODE_TEXT );
  }

  GFL_DISP_GX_SetVisibleControl(  GX_PLANEMASK_BG0, VISIBLE_ON     );
  GFL_DISP_GX_SetVisibleControl(  GX_PLANEMASK_BG1, VISIBLE_ON     );
  GFL_DISP_GX_SetVisibleControl(  GX_PLANEMASK_BG2, VISIBLE_ON     );
  GFL_DISP_GX_SetVisibleControl(  GX_PLANEMASK_BG3, VISIBLE_ON      );
  GFL_DISP_GX_SetVisibleControl(  GX_PLANEMASK_OBJ, VISIBLE_ON      );
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG1, VISIBLE_ON    );
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG2, VISIBLE_ON     );
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_ON      );
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON      );
}

//--------------------------------------------------------------------------------------------
/**
 * ＢＧ解放
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBMAIN_BgExit( MAILBOX_SYS_WORK * syswk )
{
  GFL_BG_FreeBGControl( MBMAIN_BGF_BG_M );
  GFL_BG_FreeBGControl( MBMAIN_BGF_BTN_M );
  GFL_BG_FreeBGControl( MBMAIN_BGF_STR_M );
  GFL_BG_FreeBGControl( MBMAIN_BGF_MSG_M );
  GFL_BG_FreeBGControl( MBMAIN_BGF_BG_S );
  GFL_BG_FreeBGControl( MBMAIN_BGF_STR_S );
  GFL_BG_FreeBGControl( MBMAIN_BGF_MAIL_S );
  GFL_BG_FreeBGControl( MBMAIN_BGF_MAILMES_S );

  GFL_BG_Exit();
}

//--------------------------------------------------------------------------------------------
/**
 * ＢＧグラフィック読み込み
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBMAIN_BgGraphicLoad( MAILBOX_SYS_WORK * syswk )
{
  ARCHANDLE * ah = GFL_ARC_OpenDataHandle( ARCID_MAIL_GRA, HEAPID_MAILBOX_APP );

  GFL_ARCHDL_UTIL_TransVramBgCharacter( ah, NARC_mail_gra_mailbox_u_lz_NCGR,
                                        MBMAIN_BGF_BG_S, 0, 0, TRUE, HEAPID_MAILBOX_APP );
  GFL_ARCHDL_UTIL_TransVramScreen( ah, NARC_mail_gra_mailbox_u_lz_NSCR,
                                   MBMAIN_BGF_BG_S, 0, 0, TRUE, HEAPID_MAILBOX_APP );
  GFL_ARCHDL_UTIL_TransVramPalette( ah, NARC_mail_gra_mailbox_u_NCLR, 
                                    PALTYPE_SUB_BG, 0, 0, HEAPID_MAILBOX_APP );

  GFL_ARCHDL_UTIL_TransVramBgCharacter( ah, NARC_mail_gra_mailbox_d_lz_NCGR,
                                        MBMAIN_BGF_BG_M, 0, 0, TRUE, HEAPID_MAILBOX_APP );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( ah, NARC_mail_gra_mailbox_d_lz_NCGR,
                                        MBMAIN_BGF_STR_M, 0, 0, TRUE, HEAPID_MAILBOX_APP );
  GFL_ARCHDL_UTIL_TransVramScreen( ah, NARC_mail_gra_mailbox_d_lz_NSCR,
                                   MBMAIN_BGF_BG_M, 0, 0, TRUE, HEAPID_MAILBOX_APP );
  GFL_ARCHDL_UTIL_TransVramPalette( ah, NARC_mail_gra_mailbox_d_NCLR, 
                                    PALTYPE_MAIN_BG, 0, 0, HEAPID_MAILBOX_APP );

  { // ボタン背景取得
    NNSG2dCharacterData * chr;
    void * buf;
    u8 * cgx;

    buf = GFL_ARCHDL_UTIL_LoadBGCharacter(
        ah, NARC_mail_gra_mailbox_d_lz_NCGR, 1, &chr, HEAPID_MAILBOX_APP );
    cgx = chr->pRawData;
    MI_CpuCopyFast( &cgx[BTN_U_BACK_CGX_POS], &syswk->app->btn_back_cgx[0x00], 0x20 );
    MI_CpuCopyFast( &cgx[BTN_D_BACK_CGX_POS], &syswk->app->btn_back_cgx[0x20], 0x20 );
    GFL_HEAP_FreeMemory( buf );
  }

  GFL_ARC_CloseDataHandle( ah );
}

//--------------------------------------------------------------------------------------------
/**
 * ウィンドウ読み込み
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBMAIN_WindowLoad( MAILBOX_SYS_WORK * syswk )
{
  MAILBOX_APP_WORK * appwk;

  appwk = syswk->app;

  // 会話ウィンドウ
//  TalkWinGraphicSet(
//    appwk->bgl, MBMAIN_BGF_MSG_M,
//    MBMAIN_TALKWIN_CGX_POS, MBMAIN_MBG_PAL_TALKWIN, 0, HEAPID_MAILBOX_APP );
//  TalkFontPaletteLoad( PALTYPE_MAIN_BG, MBMAIN_MBG_PAL_TALKFNT * 32, HEAPID_MAILBOX_APP );
  TalkWinFrame_GraphicSet( MBMAIN_BGF_MSG_M, MBMAIN_TALKWIN_CGX_POS, MBMAIN_MBG_PAL_TALKWIN, 
                           0, HEAPID_MAILBOX_APP );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 
                                 MBMAIN_MBG_PAL_TALKFNT * 32, 32, HEAPID_MAILBOX_APP);

  // システムフォントパレット
//  SystemFontPaletteLoad( PALTYPE_MAIN_BG, MBMAIN_MBG_PAL_SYSFNT * 32, HEAPID_MAILBOX_APP );
//  SystemFontPaletteLoad( PALTYPE_SUB_BG, MBMAIN_SBG_PAL_SYSFNT * 32, HEAPID_MAILBOX_APP );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, 
                                 MBMAIN_MBG_PAL_SYSFNT * 32, 32, HEAPID_MAILBOX_APP);
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 
                                 MBMAIN_SBG_PAL_SYSFNT * 32,32, HEAPID_MAILBOX_APP);

}

//--------------------------------------------------------------------------------------------
/**
 * BGWINFRM初期化
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBMAIN_BgWinFrmInit( MAILBOX_SYS_WORK * syswk )
{
  u16 * scrn1;
  u16 * scrn2;
  u32 i;

  syswk->app->wfrm = BGWINFRM_Create( BGWINFRM_TRANS_VBLANK, MBMAIN_BGWF_MAX, HEAPID_MAILBOX_APP );

  // メール選択ボタン
  for( i=0; i<MBMAIN_MAILLIST_MAX; i++ ){
    BGWINFRM_Add(
      syswk->app->wfrm, MBMAIN_BGWF_BUTTON1+i,
      MBMAIN_BGF_BTN_M, BGWF_MAILBTN_SX, BGWF_MAILBTN_SY );
  }

  // 選択されたメール
  BGWINFRM_Add(
    syswk->app->wfrm, MBMAIN_BGWF_SELMAIL,
    MBMAIN_BGF_STR_M, BGWF_MAILBTN_SX, BGWF_MAILBTN_SY );


  // スクリーンデータロード
  BGWINFRM_FrameSetArc(
    syswk->app->wfrm, MBMAIN_BGWF_BUTTON1,
    ARCID_MAIL_GRA, NARC_mail_gra_mailbox_win_lz_NSCR, TRUE );

  scrn1 = BGWINFRM_FrameBufGet( syswk->app->wfrm, MBMAIN_BGWF_BUTTON1 );

  for( i=MBMAIN_BGWF_BUTTON1+1; i<=MBMAIN_BGWF_SELMAIL; i++ ){
    scrn2 = BGWINFRM_FrameBufGet( syswk->app->wfrm, i );
    MI_CpuCopy8( scrn1, scrn2, BGWF_MAILBTN_SX*BGWF_MAILBTN_SY*2 );
  }

  // ボタン配置
  for( i=0; i<MBMAIN_MAILLIST_MAX; i++ ){
    BGWINFRM_FramePut(
      syswk->app->wfrm, MBMAIN_BGWF_BUTTON1+i,
      BGWF_MAILBTN_PX + ( BGWF_MAILBTN_SPACE_X * (i&1) ),
      BGWF_MAILBTN_PY + ( BGWF_MAILBTN_SPACE_Y * (i>>1) ) );
  }
}

//--------------------------------------------------------------------------------------------
/**
 * BGWINFRM解放
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBMAIN_BgWinFrmExit( MAILBOX_SYS_WORK * syswk )
{
  BGWINFRM_Exit( syswk->app->wfrm );
}

//--------------------------------------------------------------------------------------------
/**
 * メッセージ関連初期化
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBMAIN_MsgInit( MAILBOX_APP_WORK * appwk )
{
  appwk->mman   = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
                                  NARC_message_mailbox_dat, HEAPID_MAILBOX_APP );
  appwk->wset   = WORDSET_Create( HEAPID_MAILBOX_APP );
  appwk->expbuf = GFL_STR_CreateBuffer( EXP_BUF_SIZE, HEAPID_MAILBOX_APP );

}

//--------------------------------------------------------------------------------------------
/**
 * メッセージ関連解放
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBMAIN_MsgExit( MAILBOX_APP_WORK * appwk )
{
  GFL_STR_DeleteBuffer( appwk->expbuf );
  WORDSET_Delete( appwk->wset );
  GFL_MSG_Delete( appwk->mman );
}

//--------------------------------------------------------------------------------------------
/**
 * はい・いいえ処理初期化
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBMAIN_YesNoWinInit( MAILBOX_SYS_WORK * syswk )
{
//  syswk->app->tsw = TOUCH_SW_AllocWork( HEAPID_MAILBOX_APP );
}

//--------------------------------------------------------------------------------------------
/**
 * はい・いいえ処理解放
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBMAIN_YesNoWinExit( MAILBOX_SYS_WORK * syswk )
{
//  TOUCH_SW_FreeWork( syswk->app->tsw );
}

//--------------------------------------------------------------------------------------------
/**
 * はい・いいえセット
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBMAIN_YesNoWinSet( MAILBOX_SYS_WORK * syswk )
{
  APP_TASKMENU_INITWORK init;

  init.heapId   = HEAPID_MAILBOX_APP;
  init.itemNum  = 2;
  init.itemWork = syswk->app->yn_menuitem;
  init.posType  = ATPT_LEFT_UP;
  init.charPosX = 21; //ウィンドウ開始位置(キャラ単位
  init.charPosY =  8;
  init.w = 10;  //キャラ単位
  init.h =  3;  //キャラ単位
  

  syswk->app->menuwork = APP_TASKMENU_OpenMenu( &init, syswk->app->menures );
/*
  TOUCH_SW_PARAM  prm;

  prm.bg_frame  = MBMAIN_BGF_MSG_M;
  prm.char_offs = MBMAIN_YNWIN_CGX_POS;
  prm.pltt_offs = MBMAIN_MBG_PAL_YNWIN;
  prm.x         = 25;
  prm.y         = 10;
  prm.kt_st     = 0;
  prm.key_pos   = 0;
  prm.type      = TOUCH_SW_TYPE_S;

  TOUCH_SW_Init( syswk->app->tsw, &prm );
*/
}


static const int menu_item[][2]={
  { msg_menu01, APP_TASKMENU_WIN_TYPE_NORMAL },
  { msg_menu02, APP_TASKMENU_WIN_TYPE_NORMAL },
  { msg_menu03, APP_TASKMENU_WIN_TYPE_NORMAL },
  { msg_menu04, APP_TASKMENU_WIN_TYPE_RETURN },
};

static const int yn_item[][2]={
  { msg_mailbox_yes,  APP_TASKMENU_WIN_TYPE_NORMAL },
  { msg_mailbox_no,   APP_TASKMENU_WIN_TYPE_NORMAL },
};

#define MAILBOX_MENU01_X  ( 19 )
#define MAILBOX_MENU01_Y  ( 19 )
#define MAILBOX_MENU01_W  (  3 )
//--------------------------------------------------------------------------------------------
/**
 * 選択ボックス作成
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBMAIN_SelBoxInit( MAILBOX_SYS_WORK * syswk )
{
  u32 max;
  u16 i, j;

  // タスクメニューリソース転送
  syswk->app->menures = APP_TASKMENU_RES_Create( GFL_BG_FRAME0_M, 5, syswk->font, 
                                                 syswk->app->printQue, HEAPID_MAILBOX_APP );

  // メニュー項目登録(よむ・けす・もたせる・やめる）
  for(i=0;i<SELBOX_LIST_NUM;i++){
    syswk->app->menuitem[i].str      = GFL_MSG_CreateString( syswk->app->mman, menu_item[i][0] ); //メニューに表示する文字列
    syswk->app->menuitem[i].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;   //文字色。デフォルトでよいならばAPP_TASKMENU_ITEM_MSGCOLOR
    syswk->app->menuitem[i].type     = menu_item[i][1]; //戻るマークの表示

//    syswk->app->menuwork[i] = APP_TASKMENU_WIN_Create( syswk->app->menures, &syswk->app->menuitem[i], 
//                                               MAILBOX_MENU01_X, MAILBOX_MENU01_Y+i*3, 
//                                               MAILBOX_MENU01_W, HEAPID_MAILBOX_APP );
  }
  
  // はい・いいえ
  for(i=0;i<2;i++){
    syswk->app->yn_menuitem[i].str      = GFL_MSG_CreateString( syswk->app->mman, yn_item[i][0] ); //メニューに表示する文字列
    syswk->app->yn_menuitem[i].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;   //文字色。デフォルトでよいならばAPP_TASKMENU_ITEM_MSGCOLOR
    syswk->app->yn_menuitem[i].type     = yn_item[i][1];
  }
/*
  syswk->app->selbox = SelectBoxSys_AllocWork( HEAPID_MAILBOX_APP, NULL );

  max = MailMenuMaxGet( syswk );

  syswk->app->sblist = BmpMenuWork_ListCreate( max, HEAPID_MAILBOX_APP );

  for( i=0; i<SELBOX_LIST_NUM; i++ ){
    if( max == 2 ){
      if( i == MBMAIN_MENU_POKESET || i == MBMAIN_MENU_DELETE ){
        continue;
      }
    }
    BmpMenuWork_ListAddArchiveString( syswk->app->sblist, syswk->app->mman, 
                                      msg_menu01+i, i, HEAPID_MAILBOX_APP );
  }
*/
}

//--------------------------------------------------------------------------------------------
/**
 * 選択ボックス削除
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBMAIN_SelBoxExit( MAILBOX_SYS_WORK * syswk )
{
  int i;
//  BmpMenuWork_ListDelete( syswk->app->sblist );
//  SelectBoxSys_Free( syswk->app->selbox );


  for(i=0;i<SELBOX_LIST_NUM;i++){
    GFL_STR_DeleteBuffer(syswk->app->menuitem[i].str);
  }
  GFL_STR_DeleteBuffer(syswk->app->yn_menuitem[0].str);
  GFL_STR_DeleteBuffer(syswk->app->yn_menuitem[1].str);

  APP_TASKMENU_RES_Delete( syswk->app->menures );

}

//--------------------------------------------------------------------------------------------
/**
 * 選択ボックス表示
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBMAIN_SelBoxPut( MAILBOX_SYS_WORK * syswk )
{
  APP_TASKMENU_INITWORK init;

  init.heapId   = HEAPID_MAILBOX_APP;
  init.itemNum  = 4;
  init.itemWork = syswk->app->menuitem;
  init.posType  = ATPT_LEFT_UP;
  init.charPosX = 20; //ウィンドウ開始位置(キャラ単位
  init.charPosY = 12;
  init.w = 12;  //キャラ単位
  init.h =  3;  //キャラ単位
  

  syswk->app->menuwork = APP_TASKMENU_OpenMenu( &init, syswk->app->menures );
/*  
  SELBOX_HEADER hd;

  hd.prm   = SelBoxHedParam;
  hd.list  = syswk->app->sblist;

  syswk->app->sbwk = SelectBoxSet(
              syswk->app->selbox, &hd, 0,
              SELBOX_PX, SELBOX_PY_4, SELBOX_SX, 0 );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * 選択ボックス非表示
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBMAIN_SelBoxDel( MAILBOX_SYS_WORK * syswk )
{
//  SelectBoxExit( syswk->app->sbwk );
  APP_TASKMENU_CloseMenu( syswk->app->menuwork );
}

//--------------------------------------------------------------------------------------------
/**
 * メールデータ取得
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBMAIN_MailDataInit( MAILBOX_SYS_WORK * syswk )
{
  u32 i;

  syswk->app->mail_max = 0;

  for( i=0; i<MAIL_STOCK_MAX; i++ ){
    syswk->app->mail_list[i] = MBMAIN_MAILLIST_NULL;

    syswk->app->mail[i] = MAIL_AllocMailData(
                syswk->sv_mail, MAILBLOCK_PASOCOM, i, HEAPID_MAILBOX_APP );

    // メールリスト作成
    if( MailData_IsEnable(syswk->app->mail[i]) == TRUE ){
      syswk->app->mail_list[syswk->app->mail_max] = i;
      syswk->app->mail_max++;
    }
  }

  if( syswk->app->mail_max == 0 ){
    syswk->app->page_max = 0;
  }else{
    syswk->app->page_max = (syswk->app->mail_max-1) / MBMAIN_MAILLIST_MAX;
  }
}

//--------------------------------------------------------------------------------------------
/**
 * メールデータ解放
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBMAIN_MailDataExit( MAILBOX_SYS_WORK * syswk )
{
  u32 i;

  for( i=0; i<MAIL_STOCK_MAX; i++ ){
    GFL_HEAP_FreeMemory( syswk->app->mail[i] );
  }
}

//--------------------------------------------------------------------------------------------
/**
 * メール選択ボタン初期化
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBMAIN_MailButtonInit( MAILBOX_SYS_WORK * syswk )
{
  u8 * lst;
  u16 pos;
  u16 win;
  u16 frm;
  u16 i;

  pos = syswk->sel_page * MBMAIN_MAILLIST_MAX;
  lst = &syswk->app->mail_list[ pos ];
  win = MBMAIN_BMPWIN_ID_MAIL01 + pos;
  frm = MBMAIN_BGWF_BUTTON1;

  for( i=0; i<MBMAIN_MAILLIST_MAX; i++ ){
    if( lst[i] == MBMAIN_MAILLIST_NULL ){
      BGWINFRM_FrameOff( syswk->app->wfrm, frm );
    }else{
      MBBMP_MailButtonSet( syswk, frm, win, lst[i] );
      BGWINFRM_FrameOn( syswk->app->wfrm, frm );
    }
    win++;
    frm++;
  }
}

//--------------------------------------------------------------------------------------------
/**
 * 選択したメールのタブ表示
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBMAIN_SelMailTabPut( MAILBOX_SYS_WORK * syswk )
{
  MBBMP_MailButtonSet(
    syswk, MBMAIN_BGWF_SELMAIL,
    MBMAIN_BMPWIN_ID_SELMAIL, syswk->lst_pos );

  BGWINFRM_FramePut( syswk->app->wfrm, MBMAIN_BGWF_SELMAIL, BGWF_SELMAIL_PX, BGWF_SELMAIL_PY );
}

//--------------------------------------------------------------------------------------------
/**
 * ボタンアニメ処理
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @retval  "TRUE = アニメ中"
 * @retval  "FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL MBMAIN_ButtonAnmMain( MAILBOX_SYS_WORK * syswk )
{
  BUTTON_ANM_WORK * bawk = &syswk->app->bawk;

  switch( bawk->btn_seq ){
  case 0:
    if( bawk->btn_mode == MBMAIN_BTN_ANM_MODE_OBJ ){
      GFL_CLACT_WK_SetPlttOffs( syswk->app->clwk[bawk->btn_id], bawk->btn_pal1, CLWK_PLTTOFFS_MODE_PLTT_TOP );
      // ダメだったら3番目の引数をCLWK_PLTTOFFS_MODE_OAM_COLORに
    }else{
      GFL_BG_ChangeScreenPalette( bawk->btn_id, 
                                  bawk->btn_px, bawk->btn_py, bawk->btn_sx, bawk->btn_sy, 
                                  bawk->btn_pal1 );
      GFL_BG_LoadScreenV_Req( bawk->btn_id );
    }
    bawk->btn_seq++;
    break;

  case 1:
    bawk->btn_cnt++;
    if( bawk->btn_cnt != 4 ){ break; }
    if( bawk->btn_mode == MBMAIN_BTN_ANM_MODE_OBJ ){
      GFL_CLACT_WK_SetPlttOffs( syswk->app->clwk[bawk->btn_id], bawk->btn_pal2, CLWK_PLTTOFFS_MODE_PLTT_TOP );
      // ダメだったら3番目の引数をCLWK_PLTTOFFS_MODE_OAM_COLORに
    }else{
      GFL_BG_ChangeScreenPalette( bawk->btn_id,
                                  bawk->btn_px, bawk->btn_py, bawk->btn_sx, bawk->btn_sy, 
                                  bawk->btn_pal2 );
      GFL_BG_LoadScreenV_Req( bawk->btn_id );
    }
    bawk->btn_cnt = 0;
    bawk->btn_seq++;
    break;

  case 2:
    bawk->btn_cnt++;
    if( bawk->btn_cnt == 2 ){
      return FALSE;
    }
  }

  return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * メールグラフィック読み込み
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBMAIN_MailGraphcLoad( MAILBOX_SYS_WORK * syswk )
{
  ARCHANDLE * ah;
  u32 pat;

  ah  = GFL_ARC_OpenDataHandle( ARCID_MAIL_GRA, HEAPID_MAILBOX_APP );
  pat = MailData_GetDesignNo( syswk->app->mail[ syswk->lst_pos ] );

  GFL_ARCHDL_UTIL_TransVramBgCharacter( ah, NARC_mail_gra_mail_000_lz_ncgr+pat,
                                        MBMAIN_BGF_MAIL_S, 0, 0, TRUE, HEAPID_MAILBOX_APP );
  GFL_ARCHDL_UTIL_TransVramPalette( ah, NARC_mail_gra_mail_000_nclr+pat, 
                                    PALTYPE_SUB_BG, 0, 0x20*2, HEAPID_MAILBOX_APP );
  GFL_ARCHDL_UTIL_TransVramScreen( ah, NARC_mail_gra_mail_000_lz_nscr+pat,
                                   MBMAIN_BGF_MAIL_S, 0, 0, TRUE, HEAPID_MAILBOX_APP );

  GFL_ARC_CloseDataHandle( ah );
}

#define PMS_WORD_CLACT_MAX    ( 6 )

//=============================================================================================
/**
 * @brief 簡易会話描画システム初期化
 *
 * @param   syswk   メールボックスシステムワーク
 */
//=============================================================================================
void MBMAIN_PmsDrawInit( MAILBOX_SYS_WORK * syswk )
{
  syswk->app->pmsPrintque = PRINTSYS_QUE_Create( HEAPID_MAILBOX_APP );
  syswk->app->pmsClunit   = GFL_CLACT_UNIT_Create( PMS_WORD_CLACT_MAX, 2, HEAPID_MAILBOX_APP );
  syswk->app->pms_draw_work = PMS_DRAW_Init( syswk->app->pmsClunit, CLSYS_DRAW_SUB, 
                                           syswk->app->pmsPrintque, syswk->font, 
                                           4, 3, HEAPID_MAILBOX_APP );
}


//=============================================================================================
/**
 * @brief 簡易会話描画システム解放
 *
 * @param   syswk   メールボックスシステムワーク
 */
//=============================================================================================
void MBMAIN_PmsDrawExit( MAILBOX_SYS_WORK * syswk )
{
  PMS_DRAW_Exit( syswk->app->pms_draw_work );
  PRINTSYS_QUE_Delete( syswk->app->pmsPrintque );
  GFL_CLACT_UNIT_Delete( syswk->app->pmsClunit );
  
}
