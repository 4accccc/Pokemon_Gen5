//============================================================================================
/**
 * @file  pdc_main.c
 * @brief ポケモンドリームキャッチメイン（パレスでの捕獲ゲーム）
 * @author  soga
 * @date  2010.03.12
 */
//============================================================================================
#include <gflib.h>

#include "pdc_main.h"
#include "arc_def.h"

#include "battle/btlv/btlv_effect.h"
#include "battle/btlv/btlv_input.h"
#include "tr_tool/trtype_def.h"

#include "font/font.naix"
#include "message.naix"

//============================================================================================
/**
 *  定数宣言
 */
//============================================================================================
typedef PDC_RESULT  ( * PDC_FUNC )( PDC_MAIN_WORK* );

//============================================================================================
/**
 *  構造体宣言
 */
//============================================================================================
struct _PDC_MAIN_WORK
{ 
  GFL_FONT*         large_font;
  GFL_FONT*         small_font;
  GFL_MSGDATA*      msg;

  BTLV_INPUT_WORK*  biw;

  int               seq_no;

  u8                cursor_flag;

  PDC_FUNC*         pFunc;

  HEAPID            heapID;
};

//============================================================================================
/**
 *  プロトタイプ宣言
 */
//============================================================================================
static  void  screen_init( HEAPID heapID );
static  void  screen_exit( void );

static  PDC_RESULT  PDC_SEQ_Encount( PDC_MAIN_WORK* pmw );
static  PDC_RESULT  PDC_SEQ_CheckInput( PDC_MAIN_WORK* pmw );
static  PDC_RESULT  PDC_SEQ_Capture( PDC_MAIN_WORK* pmw );
static  PDC_RESULT  PDC_SEQ_Escape( PDC_MAIN_WORK* pmw );

//--------------------------------------------------------------------------
/**
 * @brief システム初期化
 */
//--------------------------------------------------------------------------
PDC_MAIN_WORK* PDC_MAIN_Init( PDC_SETUP_PARAM* psp, HEAPID heapID )
{ 
  PDC_MAIN_WORK* pmw = GFL_HEAP_AllocMemory( heapID, sizeof( PDC_MAIN_WORK ) );

  pmw->heapID = heapID;

  screen_init( pmw->heapID );

  pmw->large_font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, pmw->heapID );
  pmw->small_font = GFL_FONT_Create( ARCID_FONT, NARC_font_small_batt_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, pmw->heapID );
  pmw->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_soga_dat, pmw->heapID );

  pmw->seq_no = 0;
  {
    u16 tr_type[] = { 
      TRTYPE_HERO, TRTYPE_HERO, 0xffff, 0xffff,
    };
    BTLV_EFFECT_SETUP_PARAM* besp = BTLV_EFFECT_MakeSetUpParam( BTL_RULE_SINGLE, PDC_GetBFS( psp ), FALSE,
                                                                tr_type, pmw->heapID );
    BTLV_EFFECT_Init( besp, pmw->small_font, pmw->heapID );
    GFL_HEAP_FreeMemory( besp );
  }

  pmw->biw = BTLV_INPUT_InitEx( BTLV_INPUT_TYPE_SINGLE, pmw->large_font, &pmw->cursor_flag, pmw->heapID );

  //フェードイン
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 16, 0, 2 );

  return pmw;
}

//--------------------------------------------------------------------------
/**
 * @brief システムメイン
 */
//--------------------------------------------------------------------------
PDC_RESULT  PDC_MAIN_Main( PDC_MAIN_WORK* pmw )
{ 
  BTLV_EFFECT_Main();
  BTLV_INPUT_Main( pmw->biw );

  if( pmw->pFunc )
  { 
    return pmw->pFunc[ 0 ]( pmw );
  }

  return PDC_RESULT_NONE;
}

//--------------------------------------------------------------------------
/**
 * @brief システム終了
 */
//--------------------------------------------------------------------------
void  PDC_MAIN_Exit( PDC_MAIN_WORK* pmw )
{ 
  screen_exit();

  BTLV_EFFECT_Exit();
  BTLV_INPUT_Exit( pmw->biw );

  GFL_HEAP_FreeMemory( pmw );
}

//--------------------------------------------------------------------------
/**
 * @brief 画面初期化
 */
//--------------------------------------------------------------------------
static  void  screen_init( HEAPID heapID )
{ 
  static const GFL_DISP_VRAM dispvramBank = {
    GX_VRAM_BG_128_D,       // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
    GX_VRAM_SUB_BG_128_C,      // サブ2DエンジンのBG
    GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
    GX_VRAM_OBJ_64_E,       // メイン2DエンジンのOBJ
    GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
    GX_VRAM_SUB_OBJ_16_I,     // サブ2DエンジンのOBJ
    GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
    GX_VRAM_TEX_01_AB,        // テクスチャイメージスロット
    GX_VRAM_TEXPLTT_01_FG,      // テクスチャパレットスロット
    GX_OBJVRAMMODE_CHAR_1D_64K,   // メインOBJマッピングモード
    GX_OBJVRAMMODE_CHAR_1D_32K,   // サブOBJマッピングモード
  };

  GFL_DISP_SetBank( &dispvramBank );

  //VRAMクリア
  MI_CpuClear32((void*)HW_BG_VRAM, HW_BG_VRAM_SIZE);
  MI_CpuClear32((void*)HW_DB_BG_VRAM, HW_DB_BG_VRAM_SIZE);
  MI_CpuClear32((void*)HW_OBJ_VRAM, HW_OBJ_VRAM_SIZE);
  MI_CpuClear32((void*)HW_DB_OBJ_VRAM, HW_DB_OBJ_VRAM_SIZE);
  MI_CpuFill16((void*)HW_BG_PLTT, 0x0000, HW_BG_PLTT_SIZE);

  GX_SetBankForLCDC( GX_VRAM_LCDC_D );

  G2_BlendNone();
  GFL_BG_Init( heapID );
  GFL_BMPWIN_Init( heapID );

  {
    static const GFL_BG_SYS_HEADER sysHeader = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_3D,
    };
    GFL_BG_SetBGMode( &sysHeader );
  }

  {
    ///< main
    GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_OFF );

    ///< sub
    GFL_BG_SetVisible( GFL_BG_FRAME0_S,   VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME1_S,   VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME2_S,   VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME3_S,   VISIBLE_ON );

    ///<obj
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
    GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
  }
  GX_SetDispSelect( GX_DISP_SELECT_MAIN_SUB );

  //3D関連初期化
  {
    int i;
    u32 fog_table[8];

    GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX128K, GFL_G3D_VMANLNK, GFL_G3D_PLT16K, 0, heapID, NULL );
    GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_Z );
    G3X_AlphaBlend( TRUE );
    G3X_EdgeMarking( TRUE );
    G3X_AntiAlias( TRUE );
    G3X_SetFog( FALSE, 0, 0, 0 );
    GFL_BG_SetBGControl3D( 1 );
  }

  //2D画面初期化
  {
    GFL_BG_BGCNT_HEADER TextBgCntDat[] = {
      ///<FRAME1_M
      {
        0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
        GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
      },
      ///<FRAME2_M
      {
        0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
        GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
      },
      ///<FRAME3_M
      {
        0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x0c000, GFL_BG_CHRSIZ_256x256,
        GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
      },
    };
    GFL_BG_SetBGControl(GFL_BG_FRAME1_M, &TextBgCntDat[0], GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen(GFL_BG_FRAME1_M );
    GFL_BG_SetBGControl(GFL_BG_FRAME2_M, &TextBgCntDat[1], GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen(GFL_BG_FRAME2_M );
    GFL_BG_SetBGControl(GFL_BG_FRAME3_M, &TextBgCntDat[2], GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen(GFL_BG_FRAME3_M );
  }

  //ウインドマスク設定（画面両端のエッジマーキングのゴミを消す）
  {
    G2_SetWnd0InsidePlane( GX_WND_PLANEMASK_BG0 |
                 GX_WND_PLANEMASK_BG1 |
                 GX_WND_PLANEMASK_BG2 |
                 GX_WND_PLANEMASK_BG3 |
                 GX_WND_PLANEMASK_OBJ,
                 FALSE );
    G2_SetWndOutsidePlane( GX_WND_PLANEMASK_NONE, FALSE );
    G2_SetWnd0Position( 1, 1, 255, 191 );
    GX_SetVisibleWnd( GX_WNDMASK_W0 );
  }

  {
    static  const GFL_CLSYS_INIT clsysinit = {
      0, 0,
      0, 512,
      4, 124,
      4, 124,
      0,
      56,48,48,48,
      16, 16,
    };
    GFL_CLACT_SYS_Create( &clsysinit, &dispvramBank, heapID );
  }

  GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, 0x0000 );
}

//--------------------------------------------------------------------------
/**
 * @brief 画面終了
 */
//--------------------------------------------------------------------------
static  void  screen_exit( void )
{ 
  GFL_BMPWIN_Exit();
  GFL_BG_Exit();

  //セルアクター削除
  GFL_CLACT_SYS_Delete();

  //3D関連削除 soga
  GFL_G3D_Exit();
}

//--------------------------------------------------------------------------
/**
 * @brief エンカウント処理
 */
//--------------------------------------------------------------------------
static  PDC_RESULT  PDC_SEQ_Encount( PDC_MAIN_WORK* pmw )
{ 

  return PDC_RESULT_NONE;
}

//--------------------------------------------------------------------------
/**
 * @brief キーチェック処理
 */
//--------------------------------------------------------------------------
static  PDC_RESULT  PDC_SEQ_CheckInput( PDC_MAIN_WORK* pmw )
{ 

  return PDC_RESULT_NONE;
}

//--------------------------------------------------------------------------
/**
 * @brief 捕獲処理
 */
//--------------------------------------------------------------------------
static  PDC_RESULT  PDC_SEQ_Capture( PDC_MAIN_WORK* pmw )
{ 

  return PDC_RESULT_NONE;
}

//--------------------------------------------------------------------------
/**
 * @brief 逃げる処理
 */
//--------------------------------------------------------------------------
static  PDC_RESULT  PDC_SEQ_Escape( PDC_MAIN_WORK* pmw )
{ 

  return PDC_RESULT_NONE;
}

