//============================================================================================
/**
 * @file    shinka_demo.c
 * @brief   12末ロム用に間に合わせで作った進化デモ（デモと呼べる代物じゃないので、作りなおしてください）
 * @author  soga
 * @date    2009.12.09
 */
//============================================================================================
#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>

#include "demo/shinka_demo.h"

#include "gamesystem/game_beacon.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "arc_def.h"
#include "msg/msg_shinka_demo.h"
#include "msg/msg_waza_oboe.h"
#include "msg/msg_yesnomenu.h"
#include "system/main.h"
#include "system/gfl_use.h"
#include "font/font.naix"
#include "battle/app/b_plist.h"
#include "message.naix"

FS_EXTERN_OVERLAY(battle_b_app);
FS_EXTERN_OVERLAY(battle_plist);

enum{
  BACK_COL = 0,
  LETTER_COL,
  SHADOW_COL,

  BUFLEN_SHINKA_DEMO_MSG = 192,
  MSG_WAIT = 80,
  STR_X_CENTERING = 0x1000,
  SHINKADEMO_TCB_MAX = 8,
};

enum{ 
  BMPWIN_MAIN = 0,
  BMPWIN_SUB,

  BMPWIN_MAX,
  BMPWIN_NO_CLEAR = 0x80000000,
};

#define G2D_BACKGROUND_COL  (0x0000)
#define G2D_FONT_COL        (0x7fff)
#define G2D_SHADOW_COL      (0x3def)

typedef struct
{
  GFL_TCBSYS          *tcb_sys;
  void                *tcb_work;
  GFL_TCB             *v_tcb;
	PALETTE_FADE_PTR    pfd;	// パレットフェードデータ
  HEAPID              heapID;
  GFL_TCBLSYS*        tcbl_sys;
  GFL_BMPWIN*         bmpwin[ BMPWIN_MAX ];
  GFL_MSGDATA*        shinka_msg;
  GFL_MSGDATA*        wazaoboe_msg;
  GFL_MSGDATA*        yesno_msg;
  GFL_FONT*           font;
  WORDSET*            wordset;
  STRBUF*             expbuf;
  PRINT_STREAM*       ps;
  POKEMON_PARAM*      pp;
  SHINKA_DEMO_PARAM*  param;
  BPLIST_DATA         plistData;
  int                 wait;
  int                 work;
  WazaID              wazaoboe_no;
  int                 wazaoboe_index;
  u8                  cursor_flag;
}SHINKA_DEMO_WORK;

struct _SHINKA_DEMO_PARAM
{
  const POKEPARTY*  ppt;
  u16               after_mons_no;        //進化後のポケモンナンバー
  u8                shinka_pos;           //進化するポケモンのPOKEPARTY内の位置
  u8                shinka_cond;          //進化条件（ヌケニンチェックに使用）
};


static  void  SHINKADEMO_InitBG( SHINKA_DEMO_WORK* wk );
static  void  SHINKADEMO_InitBGMain( SHINKA_DEMO_WORK* wk );
static  void  SHINKADEMO_InitBGSub( SHINKA_DEMO_WORK* wk );
static  void  SHINKADEMO_ExitBG( SHINKA_DEMO_WORK* wk );
static  void  SHINKADEMO_ExitBGMain( SHINKA_DEMO_WORK* wk );
static  void  SHINKADEMO_ExitBGSub( SHINKA_DEMO_WORK* wk );
static  void  TextPrint( SHINKA_DEMO_WORK *wk, int bmp_win, int x, int y, GFL_MSGDATA* msg, u32 msgID );
static  BOOL  TextWait( SHINKA_DEMO_WORK* wk );
static  void  TextClear( SHINKA_DEMO_WORK* wk, int bmp_win );
static  void  FontLenGet( const STRBUF *str, GFL_FONT *font, int *ret_dot_len, int *ret_char_len );
static  void  SHINKADEMO_VBlank( GFL_TCB *tcb, void *work );

//------------------------------------------------------------------
/**
 * 進化デモ用パラメータ構造体生成
 *
 * @param   heapID        ヒープID
 * @param   ppt           POKEPARTY構造体
 * @param   after_mons_no 進化後のポケモンナンバー
 * @param   pos           進化するポケモンのPOKEPARTY内のインデックス
 * @param   cond          進化条件
 *
 * @retval  SHINKA_DEMO_PARAM
 */
//------------------------------------------------------------------
SHINKA_DEMO_PARAM*  SHINKADEMO_AllocParam( HEAPID heapID, const POKEPARTY* ppt, u16 after_mons_no, u8 pos, u8 cond )
{ 
  SHINKA_DEMO_PARAM*  sdp = GFL_HEAP_AllocMemory( heapID, sizeof( SHINKA_DEMO_PARAM ) );

  sdp->ppt = ppt;
  sdp->after_mons_no = after_mons_no;
  sdp->shinka_pos = pos;
  sdp->shinka_cond = cond;

  return sdp;
}

//------------------------------------------------------------------
/**
 * 進化デモ用パラメータワーク解放
 *
 * @param   sdp           パラメータワーク構造体
 *
 * @retval  SHINKA_DEMO_PARAM
 */
//------------------------------------------------------------------
void  SHINKADEMO_FreeParam( SHINKA_DEMO_PARAM* sdp )
{ 
  GFL_HEAP_FreeMemory( sdp );
}

//--------------------------------------------------------------------------
/**
 * PROC Init
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT ShinkaDemoProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  SHINKA_DEMO_WORK* wk;

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_SHINKA_DEMO, 0x80000 );
  wk = GFL_PROC_AllocWork( proc, sizeof( SHINKA_DEMO_WORK ), HEAPID_SHINKA_DEMO );
  MI_CpuClearFast( wk, sizeof( SHINKA_DEMO_WORK ) );
  wk->heapID = HEAPID_SHINKA_DEMO;

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

    { 
      //セルアクター初期化
      static  const GFL_CLSYS_INIT clsysinit = {
        0, 0,
        0, 512,
        4, 124,
        4, 124,
        0,
        48,48,48,48,
        16, 16,
      };
      GFL_CLACT_SYS_Create( &clsysinit, &dispvramBank, wk->heapID );
    }
    //VRAMクリア
    MI_CpuClear32((void*)HW_BG_VRAM, HW_BG_VRAM_SIZE);
    MI_CpuClear32((void*)HW_DB_BG_VRAM, HW_DB_BG_VRAM_SIZE);
    MI_CpuClear32((void*)HW_OBJ_VRAM, HW_OBJ_VRAM_SIZE);
    MI_CpuClear32((void*)HW_DB_OBJ_VRAM, HW_DB_OBJ_VRAM_SIZE);
    MI_CpuFill16((void*)HW_BG_PLTT, 0x0000, HW_BG_PLTT_SIZE);

  }

  GFL_BG_Init( wk->heapID );
  GFL_BMPWIN_Init( wk->heapID );

  //パレットフェード初期化
  wk->pfd = PaletteFadeInit( wk->heapID );
  PaletteTrans_AutoSet( wk->pfd, TRUE );
  PaletteFadeWorkAllocSet( wk->pfd, FADE_MAIN_BG, 0x200, wk->heapID );
  PaletteFadeWorkAllocSet( wk->pfd, FADE_SUB_BG, 0x1e0, wk->heapID );
  PaletteFadeWorkAllocSet( wk->pfd, FADE_MAIN_OBJ, 0x200, wk->heapID );
  PaletteFadeWorkAllocSet( wk->pfd, FADE_SUB_OBJ, 0x1e0, wk->heapID );

  SHINKADEMO_InitBG( wk );

  //TCBL生成
  wk->tcbl_sys = GFL_TCBL_Init( wk->heapID, wk->heapID, 64, 128 );

  //パラメータ初期化
  GF_ASSERT( pwk != NULL );
  wk->param = pwk;
  wk->pp = PokeParty_GetMemberPointer( wk->param->ppt, wk->param->shinka_pos );

  //メッセージ系初期化
  GFL_FONTSYS_Init();
  wk->shinka_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_shinka_demo_dat, wk->heapID );
  wk->wazaoboe_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_waza_oboe_dat, wk->heapID );
  wk->yesno_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_yesnomenu_dat, wk->heapID );
  wk->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, TRUE, wk->heapID );
  wk->wordset = WORDSET_Create( wk->heapID );

  //タスク初期化
  wk->tcb_work = GFL_HEAP_AllocClearMemory( wk->heapID, GFL_TCB_CalcSystemWorkSize( SHINKADEMO_TCB_MAX ) );
  wk->tcb_sys = GFL_TCB_Init( SHINKADEMO_TCB_MAX, wk->tcb_work );

  //フェードイン
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 16, 0, 2 );
  (*seq) = 0;

  //VBlank関数
  wk->v_tcb = GFUser_VIntr_CreateTCB( SHINKADEMO_VBlank, wk, 1 );

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT ShinkaDemoProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  u32 x, y;
  int trg = GFL_UI_KEY_GetTrg();
  int tp  = GFL_UI_TP_GetPointCont( &x, &y );
  SHINKA_DEMO_WORK* wk = mywk;

  GFL_TCB_Main( wk->tcb_sys );
  GFL_TCBL_Main( wk->tcbl_sys );

  switch( (*seq) ){ 
  case 0:
    if( GFL_FADE_CheckFade() == FALSE )
    {
      WORDSET_RegisterPokeNickName( wk->wordset, 0, wk->pp );
      TextPrint( wk, BMPWIN_MAIN, 8, 8, wk->shinka_msg, SHINKADEMO_ShinkaBeforeMsg );
      (*seq)++;
    }
    break;
  case 1:
    if( TextWait( wk ) )
    { 
      WORDSET_RegisterPokeNickName( wk->wordset, 0, wk->pp );
      TextPrint( wk, BMPWIN_MAIN, 8, 8, wk->shinka_msg, SHINKADEMO_ShinkaSelectMsg );
      (*seq)++;
    }
    break;
  case 2:
    if( TextWait( wk ) )
    { 
      if( trg & PAD_BUTTON_A )
      { 
        WORDSET_RegisterPokeNickName( wk->wordset, 0, wk->pp );
        PP_ChangeMonsNo( wk->pp, wk->param->after_mons_no );
        WORDSET_RegisterPokeMonsName( wk->wordset, 1, wk->pp );
        TextPrint( wk, BMPWIN_MAIN, 8, 8, wk->shinka_msg, SHINKADEMO_ShinkaMsg );
        (*seq)++;
      }
      if( trg & PAD_BUTTON_B )
      { 
        WORDSET_RegisterPokeNickName( wk->wordset, 0, wk->pp );
        TextPrint( wk, BMPWIN_MAIN, 8, 8, wk->shinka_msg, SHINKADEMO_ShinkaCancelMsg );
        wk->work = 1000;
        (*seq) = 100;
      }
    }
    break;
  case 3:
    if( TextWait( wk ) )
    { 
      { 
        //すれ違い用データをセット
        STRBUF* buf = GFL_STR_CreateBuffer( BUFLEN_SHINKA_DEMO_MSG, wk->heapID );
        PP_Get( wk->pp, ID_PARA_nickname, buf );
        GAMEBEACON_Set_PokemonEvolution( buf );
        GFL_STR_DeleteBuffer( buf );
      }
      
      //技覚えチェック
      wk->wazaoboe_no = PP_CheckWazaOboe( wk->pp, &wk->wazaoboe_index, wk->heapID );
      if( wk->wazaoboe_no == PTL_WAZAOBOE_NONE )
      {
        (*seq) = 1000;
      }
      else if( wk->wazaoboe_no == PTL_WAZASET_SAME )
      { 
        break;
      }
      else if( wk->wazaoboe_no & PTL_WAZAOBOE_FULL )
      {
        wk->wazaoboe_no &= ( PTL_WAZAOBOE_FULL ^ 0xffff );
        (*seq) = 6;
      }
      else
      {
        (*seq) = 4;
      }
    }
    break;
  case 4:
    WORDSET_RegisterPokeNickName( wk->wordset, 0, wk->pp );
    WORDSET_RegisterWazaName( wk->wordset, 1, wk->wazaoboe_no );
    TextPrint( wk, BMPWIN_MAIN, 8, 8, wk->wazaoboe_msg, msg_waza_oboe_04 );
    (*seq)++;
    break;
  case 5:
    //技覚えた
    if( TextWait( wk ) )
    { 
      wk->wait = MSG_WAIT;
      wk->work = 3; 
      (*seq) = 200;
    }
    break;
  case 6:
    //技の手持ちがいっぱい
    WORDSET_RegisterPokeNickName( wk->wordset, 0, wk->pp );
    WORDSET_RegisterWazaName( wk->wordset, 1, wk->wazaoboe_no );
    TextPrint( wk, BMPWIN_MAIN, 8, 8, wk->wazaoboe_msg, msg_waza_oboe_05 );
    (*seq)++;
    break;
  case 7:
    if( TextWait( wk ) )
    { 
      TextPrint( wk, BMPWIN_SUB, 128 | STR_X_CENTERING, 40, wk->yesno_msg, msgid_yesno_wazawasureru );
      TextPrint( wk, BMPWIN_SUB | BMPWIN_NO_CLEAR, 128 | STR_X_CENTERING, 136, wk->yesno_msg, msgid_yesno_wazawasurenai );
      (*seq)++;
    }
    break;
  case 8:
    //技忘れ確認処理
    if( tp )
    { 
      TextClear( wk, BMPWIN_SUB );
      if( y < 96 )
      { 
        SHINKADEMO_ExitBGSub( wk );
        wk->plistData.pp = (POKEPARTY*)wk->param->ppt;
        wk->plistData.font = wk->font;
        wk->plistData.heap = wk->heapID;
        wk->plistData.mode = BPL_MODE_WAZASET;
        wk->plistData.end_flg = FALSE;
        wk->plistData.sel_poke = wk->param->shinka_pos;
        wk->plistData.chg_waza = wk->wazaoboe_no;
        wk->plistData.rule = 0;
        wk->plistData.cursor_flg = &wk->cursor_flag;
        wk->plistData.tcb_sys = wk->tcb_sys;
        wk->plistData.pfd = wk->pfd;
        GFL_OVERLAY_Load( FS_OVERLAY_ID( battle_b_app ) );
        GFL_OVERLAY_Load( FS_OVERLAY_ID( battle_plist ) );
        BattlePokeList_TaskAdd( &wk->plistData );
        (*seq)++;
      }
      else
      { 
        WORDSET_RegisterPokeNickName( wk->wordset, 0, wk->pp );
        WORDSET_RegisterWazaName( wk->wordset, 1, wk->wazaoboe_no );
        TextPrint( wk, BMPWIN_MAIN, 8, 8, wk->wazaoboe_msg, msg_waza_oboe_08 );
        (*seq) = 50;
      }
    }
    break;
  case 9:
    if( wk->plistData.end_flg )
    {
      GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle_b_app ) );
      GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle_plist ) );
      SHINKADEMO_InitBGSub( wk );
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, 0 );
      (*seq)++;
    }
    break;
  case 10:
    if( GFL_FADE_CheckFade() == FALSE )
    { 
      int result = wk->plistData.sel_wp;
      if( result == BPL_SEL_WP_CANCEL )
      { 
        (*seq) = 6;
      }
      else
      { 
        WazaID forget_wazano = PP_Get( wk->pp, ID_PARA_waza1 + result, NULL );
        PP_SetWazaPos( wk->pp, wk->wazaoboe_no, result );
        WORDSET_RegisterPokeNickName( wk->wordset, 0, wk->pp );
        WORDSET_RegisterWazaName( wk->wordset, 1, forget_wazano );
        TextPrint( wk, BMPWIN_MAIN, 8, 8, wk->wazaoboe_msg, msg_waza_oboe_06 );
        wk->work = 4;
        (*seq) = 100;
      }
    }
    break;
  case 50:
    if( TextWait( wk ) )
    { 
      WORDSET_RegisterWazaName( wk->wordset, 0, wk->wazaoboe_no );
      TextPrint( wk, BMPWIN_SUB, 128 | STR_X_CENTERING, 40, wk->yesno_msg, msgid_yesno_wazaakirameru );
      TextPrint( wk, BMPWIN_SUB | BMPWIN_NO_CLEAR, 128 | STR_X_CENTERING, 136, wk->yesno_msg, msgid_yesno_wazaakiramenai );
      (*seq)++;
    }
    break;
  case 51:
    //技あきらめ確認処理
    if( tp )
    { 
      TextClear( wk, BMPWIN_SUB );
      if( y < 96 )
      { 
        WORDSET_RegisterPokeNickName( wk->wordset, 0, wk->pp );
        WORDSET_RegisterWazaName( wk->wordset, 1, wk->wazaoboe_no );
        TextPrint( wk, BMPWIN_MAIN, 8, 8, wk->wazaoboe_msg, msg_waza_oboe_09 );
        wk->work = 5;
        (*seq) = 100;
      }
      else
      { 
        (*seq) = 5;
      }
    }
    break;
  case 100:
    if( TextWait( wk ) )
    { 
      (*seq) = wk->work;
    }
    break;
  case 200:
    if( --wk->wait == 0 )
    { 
      (*seq) = wk->work;
    }
    break;
  case 1000:
  default:
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 0, 16, 2 );
    return GFL_PROC_RES_FINISH;
  }

  return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------
/**
 * PROC Exit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT ShinkaDemoProcExit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  SHINKA_DEMO_WORK* wk = mywk;

  if( GFL_FADE_CheckFade() == TRUE ){
    return GFL_PROC_RES_CONTINUE;
  }

  GFL_TCB_DeleteTask( wk->v_tcb );

  SHINKADEMO_ExitBG( wk );

  GFL_MSG_Delete( wk->shinka_msg );
  GFL_MSG_Delete( wk->wazaoboe_msg );
  GFL_MSG_Delete( wk->yesno_msg );
  WORDSET_Delete( wk->wordset );
  GFL_FONT_Delete( wk->font );

  GFL_HEAP_FreeMemory( wk->tcb_work );
  GFL_TCBL_Exit( wk->tcbl_sys );
  GFL_TCB_Exit( wk->tcb_sys );

  PaletteFadeWorkAllocFree( wk->pfd, FADE_MAIN_BG );
  PaletteFadeWorkAllocFree( wk->pfd, FADE_SUB_BG );
  PaletteFadeWorkAllocFree( wk->pfd, FADE_MAIN_OBJ );
  PaletteFadeWorkAllocFree( wk->pfd, FADE_SUB_OBJ );
  PaletteFadeFree( wk->pfd );

  //セルアクター削除
  GFL_CLACT_SYS_Delete();

  GFL_BG_Exit();
  GFL_BMPWIN_Exit();

  GFL_PROC_FreeWork( proc );

  GFL_HEAP_DeleteHeap( HEAPID_SHINKA_DEMO );

  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA   ShinkaDemoProcData = {
  ShinkaDemoProcInit,
  ShinkaDemoProcMain,
  ShinkaDemoProcExit,
};

//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
static  void  SHINKADEMO_InitBG( SHINKA_DEMO_WORK* wk )
{ 
  SHINKADEMO_InitBGMain( wk );
  SHINKADEMO_InitBGSub( wk );
}

static  void  SHINKADEMO_InitBGMain( SHINKA_DEMO_WORK* wk )
{ 
  G2_BlendNone();

  {
    static const GFL_BG_SYS_HEADER sysHeader = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_3D,
    };
    GFL_BG_SetBGMode( &sysHeader );
  }

  {
    ///< main
    GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_OFF );
  }
  GX_SetDispSelect( GX_DISP_SELECT_MAIN_SUB );

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

  {
    //フォントパレット作成＆転送
    static  u16 plt[16] = { G2D_BACKGROUND_COL, G2D_FONT_COL, G2D_SHADOW_COL,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    GFL_BG_LoadPalette( GFL_BG_FRAME2_M, &plt, 16*2, 0 );
    PaletteWorkSet_VramCopy( wk->pfd, FADE_MAIN_BG, 0, 0x20 );
  }
  GFL_FONTSYS_SetColor( LETTER_COL, SHADOW_COL, BACK_COL );

  wk->bmpwin[ BMPWIN_MAIN ] = GFL_BMPWIN_Create( GFL_BG_FRAME2_M, 0, 18, 32, 6, 0, GFL_BG_CHRAREA_GET_F );

  GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, 0x0000 );
}

static  void  SHINKADEMO_InitBGSub( SHINKA_DEMO_WORK* wk )
{ 
  {
    static const GFL_BG_SYS_HEADER sysHeader = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_3D,
    };
    GFL_BG_SetBGMode( &sysHeader );
  }

  {
    ///< sub
    GFL_BG_SetVisible( GFL_BG_FRAME0_S,   VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME1_S,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME2_S,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME3_S,   VISIBLE_OFF );

  }
  GX_SetDispSelect( GX_DISP_SELECT_MAIN_SUB );

  //2D画面初期化
  {
    GFL_BG_BGCNT_HEADER TextBgCntDat[] = {
      ///<FRAME0_S
      {
        0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
        GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
      },
    };
    GFL_BG_SetBGControl(GFL_BG_FRAME0_S, &TextBgCntDat[0], GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen(GFL_BG_FRAME0_S );
  }

  {
    //フォントパレット作成＆転送
    static  u16 plt[16] = { G2D_BACKGROUND_COL, G2D_FONT_COL, G2D_SHADOW_COL,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    GFL_BG_LoadPalette( GFL_BG_FRAME0_S, &plt, 16*2, 0 );
    PaletteWorkSet_VramCopy( wk->pfd, FADE_SUB_BG, 0, 0x20 );
  }
  GFL_FONTSYS_SetColor( LETTER_COL, SHADOW_COL, BACK_COL );

  wk->bmpwin[ BMPWIN_SUB ] = GFL_BMPWIN_Create( GFL_BG_FRAME0_S, 0, 0, 32, 24, 0, GFL_BG_CHRAREA_GET_F );

  GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_S, 0x0000 );
}

//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
static  void  SHINKADEMO_ExitBG( SHINKA_DEMO_WORK* wk )
{ 
  SHINKADEMO_ExitBGMain( wk );
  SHINKADEMO_ExitBGSub( wk );
}

static  void  SHINKADEMO_ExitBGMain( SHINKA_DEMO_WORK* wk )
{ 
  GFL_BMPWIN_Delete( wk->bmpwin[ BMPWIN_MAIN ] );
  GFL_BG_FreeBGControl( GFL_BG_FRAME2_M );
}

static  void  SHINKADEMO_ExitBGSub( SHINKA_DEMO_WORK* wk )
{ 
  GFL_BMPWIN_Delete( wk->bmpwin[ BMPWIN_SUB ] );
  GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
}

//======================================================================
//  テキスト表示
//======================================================================
static  void  TextPrint( SHINKA_DEMO_WORK *wk, int bmp_win, int x, int y, GFL_MSGDATA* msg, u32 msgID )
{
  int i;
  STRBUF  *strbuf;

  if( ( bmp_win & BMPWIN_NO_CLEAR ) == 0 )
  { 
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->bmpwin[ bmp_win ] ), 0 );
  }

  bmp_win &= ( BMPWIN_NO_CLEAR ^ 0xffffffff );

  strbuf = GFL_MSG_CreateString( msg, msgID );
  if( bmp_win == BMPWIN_MAIN )
  { 
    wk->expbuf = GFL_STR_CreateBuffer( BUFLEN_SHINKA_DEMO_MSG, wk->heapID );
    WORDSET_ExpandStr( wk->wordset, wk->expbuf, strbuf );
    wk->ps = PRINTSYS_PrintStream( wk->bmpwin[ bmp_win ], x, y, wk->expbuf, wk->font, 1, wk->tcbl_sys, 0, wk->heapID, 0 );
  }
  else
  { 
    STRBUF* expbuf = GFL_STR_CreateBuffer( BUFLEN_SHINKA_DEMO_MSG, wk->heapID );
    WORDSET_ExpandStr( wk->wordset, expbuf, strbuf );
    if( x & STR_X_CENTERING )
    { 
      int dot_len, char_len;
      FontLenGet( expbuf, wk->font, &dot_len, &char_len );
      x = ( x & ( STR_X_CENTERING ^ 0xffffffff ) ) - ( dot_len / 2 );
    }
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( wk->bmpwin[ bmp_win ] ), x, y, expbuf, wk->font );
    GFL_STR_DeleteBuffer( expbuf );
  }
  GFL_HEAP_FreeMemory( strbuf );

  GFL_BMPWIN_TransVramCharacter( wk->bmpwin[ bmp_win ] );
  GFL_BMPWIN_MakeScreen( wk->bmpwin[ bmp_win ] );
  if( bmp_win == BMPWIN_MAIN )
  { 
    GFL_BG_LoadScreenReq( GFL_BG_FRAME2_M );
  }
  else
  { 
    GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
  }
}

static  BOOL  TextWait( SHINKA_DEMO_WORK* wk )
{ 
  BOOL ret = FALSE;

  switch( PRINTSYS_PrintStreamGetState( wk->ps ) ){ 
  case PRINTSTREAM_STATE_PAUSE:
    if( GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg() )
    { 
      PRINTSYS_PrintStreamReleasePause( wk->ps );
    }
    break;
  case PRINTSTREAM_STATE_DONE:
    if( wk->expbuf )
    { 
      GFL_STR_DeleteBuffer( wk->expbuf );
      wk->expbuf = NULL;
    }
    PRINTSYS_PrintStreamDelete( wk->ps );
    ret = TRUE;
    break;
  }
  
  return ret;
}

static  void  TextClear( SHINKA_DEMO_WORK* wk, int bmp_win )
{ 
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->bmpwin[ bmp_win ] ), 0 );
  GFL_BMPWIN_TransVramCharacter( wk->bmpwin[ bmp_win ] );
  GFL_BMPWIN_MakeScreen( wk->bmpwin[ bmp_win ] );
  if( bmp_win == BMPWIN_MAIN )
  { 
    GFL_BG_LoadScreenReq( GFL_BG_FRAME2_M );
  }
  else
  { 
    GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
  }
}

//--------------------------------------------------------------
/**
 * @brief 文字列の長さを取得する
 *
 * @param[in] str           文字列へのポインタ
 * @param[in] font          フォントタイプ
 * @param[in] ret_dot_len   ドット幅代入先
 * @param[in] ret_char_len  キャラ幅代入先
 */
//--------------------------------------------------------------
static void FontLenGet( const STRBUF *str, GFL_FONT *font, int *ret_dot_len, int *ret_char_len )
{
  int dot_len, char_len;

  //文字列のドット幅から、使用するキャラ数を算出する
  dot_len = PRINTSYS_GetStrWidth( str, font, 0 );
  char_len = dot_len / 8;
  if( FX_ModS32( dot_len, 8 ) != 0 ){
    char_len++;
  }

  *ret_dot_len = dot_len;
  *ret_char_len = char_len;
}

//============================================================================================
/**
 *  @brief  VBlank関数
 */
//============================================================================================
static  void  SHINKADEMO_VBlank( GFL_TCB *tcb, void *work )
{
  SHINKA_DEMO_WORK* wk = (SHINKA_DEMO_WORK*)work;

  GFL_CLACT_SYS_VBlankFunc();
  PaletteFadeTrans( wk->pfd );
}

