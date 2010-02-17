//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *  @file   debug_template.c
 *  @brief  テンプレートテストプロック
 *  @author Toru=Nagihashi
 *  @data   2009.09.29
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//  lib
#include <gflib.h>
#include "system/main.h"  //HEAPID
#include "system/gfl_use.h"

//  SYSTEM
#include "gamesystem/gamesystem.h"

//  module
#include "app/app_menu_common.h"
#include "app/app_taskmenu.h"
#include "infowin/infowin.h"
#include "ui/touchbar.h"
#include "../../ui/debug/ui_template_graphic.h"
#include "system/poke2dgra.h"

//archive
#include "arc_def.h"
//#include "config_gra.naix"
#include "ui/msgsearch.h"
#include "arc/wifileadingchar.naix"

//  print
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_config.h"
#include "print/gf_font.h"
#include "print/printsys.h"

//MCSS
#include "system/mcss.h"
#include "system/mcss_tool.h"
#include "ui/ui_easy_clwk.h"

//  mine
#include "debug_template.h"
#include "debug/debug_nagihashi.h"

#include "debug/debug_str_conv.h"

static const u16 PMS_SrcFileID[] = {
  NARC_message_monsname_dat,
  NARC_message_wazaname_dat,
  NARC_message_typename_dat,
  NARC_message_tokusei_dat,
  NARC_message_pms_word06_dat,
  NARC_message_pms_word07_dat,
  NARC_message_pms_word08_dat,
  NARC_message_pms_word09_dat,
  NARC_message_pms_word10_dat,
  NARC_message_pms_word11_dat,
  NARC_message_pms_word12_dat,
};

FS_EXTERN_OVERLAY(ui_common);
FS_EXTERN_OVERLAY(ui_debug);

//=============================================================================
/**
 *          定数宣言
*/
//=============================================================================
//-------------------------------------
/// フレーム
//=====================================
enum
{
  BG_FRAME_SKB_M  = GFL_BG_FRAME2_M,
  BG_FRAME_BAR_M  = GFL_BG_FRAME1_M,
  BG_FRAME_POKEMON_M  = GFL_BG_FRAME2_M,
};
//-------------------------------------
/// パレット
//=====================================
enum
{
  //メインBG
  PLTID_BG_POKEMON_M    = 0,
  PLTID_BG_SKB01_M      = 1,
  PLTID_BG_SKB02_M      = 2,
  PLTID_BG_TOUCHBAR_M   = 13,
  PLTID_BG_INFOWIN_M    = 15,

  //メインOBJ
  PLTID_OBJ_POKEMON_M   = 0,
  PLTID_OBJ_HERO_M      = 1,
  PLTID_OBJ_TOUCHBAR_M  = 13,
};
//-------------------------------------
/// リソースインデックス
//=====================================
enum
{
  OBJRESID_PM_CHR,
  OBJRESID_PM_PLT,
  OBJRESID_PM_CEL,

  OBJRESID_MAX,
} ;
//-------------------------------------
/// メッセージsearch用
//=====================================
#define STRBUF_LEN  512
#define MSGSEARCH_RESULT_MAX  (10)
#define MSGDATA_MAX (11)


//=============================================================================
/**
 *          構造体宣言
*/
//=============================================================================

//-------------------------------------
///	ポケモン描画
//=====================================
typedef struct 
{
  u32           objres[OBJRESID_MAX];
  GFL_CLWK      *p_clwk;
} POKEOBJ_WORK;


//-------------------------------------
/// シーケンス管理
//=====================================
typedef struct _SEQ_WORK SEQ_WORK;  //関数型作るため前方宣言
typedef void (*SEQ_FUNCTION)( SEQ_WORK *p_wk, int *p_seq, void *p_param );
struct _SEQ_WORK
{
  SEQ_FUNCTION  seq_function;
  BOOL is_end;
  int seq;
  void *p_param;
};

//-------------------------------------
/// メインワーク
//=====================================
typedef struct
{
  u32         pokeID;

 	//MCSS
	MCSS_SYS_WORK							*p_mcss_sys;
	MCSS_WORK									*p_mcss_wk; 

  //グラフィックモジュール
  UI_TEMPLATE_GRAPHIC_WORK  *p_graphic;

  //シーケンスシステム
  SEQ_WORK      seq;

  //タッチバー
  TOUCHBAR_WORK *p_touchbar;

  //ポケモン正面OBJ
  POKEOBJ_WORK  pokeobj;

  //共通で使うフォント
  GFL_FONT      *p_font;

  //共通で使うキュー
  PRINT_QUE     *p_que;

  //共通で使うタスクシステム
  GFL_TCBLSYS   *p_tcbl;

  //メッセージsearchで必要
  MSGSEARCH_WORK  *p_search;
  GFL_SKB         *p_skb;
  STRBUF          *p_strbuf;
  MSGSEARCH_RESULT result_tbl[MSGSEARCH_RESULT_MAX];
  GFL_MSGDATA     *p_msg[ MSGDATA_MAX ];

  //主人公OBJ
  UI_EASY_CLWK_RES  hero_res;
  GFL_CLWK          *p_hero;
  u32               hero_anm;
}TEMPLATE_WORK;
//=============================================================================
/**
 *          プロトタイプ宣言
 *              モジュール名が大文字  XXXX_Init
 *                →ソース内グローバル関数（パブリック）
 *              モジュール名が小文字  Xxxx_Init
 *                →モジュール内関数（プライベート）
*/
//=============================================================================
//-------------------------------------
/// プロセス
//=====================================
static GFL_PROC_RESULT DEBUG_TEMPLATE_PROC_Init
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT DEBUG_TEMPLATE_PROC_Exit
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT DEBUG_TEMPLATE_PROC_Main
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
//-------------------------------------
/// SEQ
//=====================================
static void SEQ_Init( SEQ_WORK *p_wk, void *p_param, SEQ_FUNCTION seq_function );
static void SEQ_Exit( SEQ_WORK *p_wk );
static void SEQ_Main( SEQ_WORK *p_wk );
static BOOL SEQ_IsEnd( const SEQ_WORK *cp_wk );
static void SEQ_SetNext( SEQ_WORK *p_wk, SEQ_FUNCTION seq_function );
static void SEQ_End( SEQ_WORK *p_wk );
//-------------------------------------
/// SEQFUNC
//=====================================
static void SEQFUNC_FadeOut( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_FadeIn( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_Main( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_Search( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );

//-------------------------------------
///	POKE２D
//=====================================
static void POKEOBJ_Init( POKEOBJ_WORK *p_wk, GFL_CLUNIT *p_clunit, const POKEMON_PASO_PARAM  *cp_ppp, HEAPID heapID );
static void POKEOBJ_Exit( POKEOBJ_WORK *p_wk );

//-------------------------------------
///	MCSS
//=====================================
static MCSS_SYS_WORK * POKE3D_Init( u16 wk_max, HEAPID heapID );
static void POKE3D_Exit( MCSS_SYS_WORK * mcss );
static void POKE3D_Draw( MCSS_SYS_WORK * mcss );
static MCSS_WORK * POKE3D_CreateWkPP( MCSS_SYS_WORK * mcss, POKEMON_PARAM *pp, const VecFx32 *pos );
static void POKE3D_DeleteWk( MCSS_SYS_WORK * mcss, MCSS_WORK *wk );

//=============================================================================
/**
 *          外部公開
 */
//=============================================================================
//-------------------------------------
/// プロックデータ
//=====================================
const GFL_PROC_DATA DebugTemplate_ProcData =
{
  DEBUG_TEMPLATE_PROC_Init,
  DEBUG_TEMPLATE_PROC_Main,
  DEBUG_TEMPLATE_PROC_Exit,
};
//=============================================================================
/**
 *          データ
 */
//=============================================================================

//=============================================================================
/**
 *          proc
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  テンプレート  メインプロセス初期化
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT DEBUG_TEMPLATE_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  TEMPLATE_WORK   *p_wk;
  TEMPLATE_PARAM  *p_param;

  //オーバーレイ読み込み
  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_debug));

  //引数受け取り
  p_param = p_param_adrs;

  //ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_NAGI_DEBUG_SUB, 0x40000 );
  //プロセスワーク作成
  p_wk  = GFL_PROC_AllocWork( p_proc, sizeof(TEMPLATE_WORK), HEAPID_NAGI_DEBUG_SUB );
  GFL_STD_MemClear( p_wk, sizeof(TEMPLATE_WORK) );
  p_wk->pokeID  = 0x88888888;

  //共通モジュールの作成
  p_wk->p_font    = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
      GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_NAGI_DEBUG_SUB );
  p_wk->p_que     = PRINTSYS_QUE_Create( HEAPID_NAGI_DEBUG_SUB );
  p_wk->p_tcbl    = GFL_TCBL_Init( HEAPID_NAGI_DEBUG_SUB, HEAPID_NAGI_DEBUG_SUB, 32, 32 );

  //モジュール作成
  p_wk->p_graphic = UI_TEMPLATE_GRAPHIC_Init( 0, HEAPID_NAGI_DEBUG_SUB );
  SEQ_Init( &p_wk->seq, p_wk, SEQFUNC_FadeOut );  //最初はFadeOutシーケンス

  //INFOWINの設定
  {
    GAME_COMM_SYS_PTR comm  = NULL;
    if( p_param->p_gamesys )
    {
      comm  = GAMESYSTEM_GetGameCommSysPtr(p_param->p_gamesys);
    }
    INFOWIN_Init( BG_FRAME_BAR_M,
        PLTID_BG_INFOWIN_M,
        comm, HEAPID_NAGI_DEBUG_SUB );
  }

  //タッチバーの設定
  {
    TOUCHBAR_ITEM_ICON touchbar_icon_tbl[]  =
    {
      {
        TOUCHBAR_ICON_RETURN,
        { TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_Y },
      },
      {
        TOUCHBAR_ICON_CLOSE,
        { TOUCHBAR_ICON_X_06, TOUCHBAR_ICON_Y },
      },
      {
        TOUCHBAR_ICON_CHECK,
        { TOUCHBAR_ICON_X_05, TOUCHBAR_ICON_Y_CHECK },
      },
    };
    TOUCHBAR_SETUP  touchbar_setup;
    touchbar_setup.p_item   = touchbar_icon_tbl;
    touchbar_setup.item_num = NELEMS(touchbar_icon_tbl);
    touchbar_setup.p_unit   = UI_TEMPLATE_GRAPHIC_GetClunit( p_wk->p_graphic );
    touchbar_setup.bar_frm  = BG_FRAME_BAR_M;
    touchbar_setup.bg_plt   = PLTID_BG_TOUCHBAR_M;
    touchbar_setup.obj_plt  = PLTID_OBJ_TOUCHBAR_M;
    touchbar_setup.mapping  = APP_COMMON_MAPPING_128K;

    p_wk->p_touchbar  = TOUCHBAR_Init( &touchbar_setup, HEAPID_NAGI_DEBUG_SUB );
  }

#if 0
  {   
    UI_EASY_CLWK_RES_PARAM param;
    GFL_STD_MemClear( &param, sizeof(UI_EASY_CLWK_RES_PARAM) );
    param.draw_type = CLSYS_DRAW_MAIN;
    param.comp_flg  = UI_EASY_CLWK_RES_COMP_NONE;
    param.arc_id  = ARCID_WIFILEADING;
    param.pltt_id = NARC_wifileadingchar_hero_NCLR;
    param.ncg_id  = NARC_wifileadingchar_hero_gts_NCGR;
    param.cell_id = NARC_wifileadingchar_hero_gts_NCER;
    param.anm_id  = NARC_wifileadingchar_hero_gts_NANR;
    param.pltt_line     = PLTID_OBJ_HERO_M;
    param.pltt_src_ofs  = 0;
    param.pltt_src_num  = 1;

    UI_EASY_CLWK_LoadResource( &p_wk->hero_res, &param, NULL, HEAPID_NAGI_DEBUG_SUB );
    p_wk->p_hero  = UI_EASY_CLWK_CreateCLWK( &p_wk->hero_res, UI_TEMPLATE_GRAPHIC_GetClunit( p_wk->p_graphic ), 128, 96, 0, HEAPID_NAGI_DEBUG_SUB );
    GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_hero, TRUE );
  }
#endif

  //ポケモンOBJ
  { 
    POKEMON_PARAM*  p_pp;
    POKEMON_PASO_PARAM  *p_ppp;
    u32 pow_val;

    //パッチール作る
    p_pp  = PP_Create( MONSNO_PATTIIRU, 100, PTL_SETUP_ID_AUTO, HEAPID_NAGI_DEBUG_SUB );
    PP_SetupEx( p_pp, MONSNO_PATTIIRU, 100, PTL_SETUP_ID_AUTO, PTL_SETUP_POW_AUTO, p_wk->pokeID );
    p_ppp = PP_GetPPPPointer( p_pp );

    {
      POKEOBJ_Init( &p_wk->pokeobj, UI_TEMPLATE_GRAPHIC_GetClunit( p_wk->p_graphic ), p_ppp, HEAPID_NAGI_DEBUG_SUB );
    }

    //ポケモンBG
    /*
       {
       POKEMON_PASO_PARAM  *p_ppp;

       p_ppp = (POKEMON_PASO_PARAM *)PP_Create( 6, 0, 0, HEAPID_NAGI_DEBUG_SUB );
       PPP_Put( p_ppp,ID_PARA_tamago_flag, TRUE );
       PPP_Put( p_ppp,ID_PARA_sex, PTL_SEX_FEMALE );

       GFL_BG_FillCharacter( BG_FRAME_POKEMON_M, 0, 1,  0 );

       POKE2DGRA_BG_TransResourcePPP( p_ppp, POKEGRA_DIR_BACK, BG_FRAME_POKEMON_M,
       1, PLTID_BG_POKEMON_M, HEAPID_NAGI_DEBUG_SUB );

       POKE2DGRA_BG_WriteScreen( BG_FRAME_POKEMON_M, 1, PLTID_BG_POKEMON_M, 0, 4 );
       GFL_BG_LoadScreenReq( BG_FRAME_POKEMON_M );

       GFL_HEAP_FreeMemory( p_ppp );
       }*/

    //MCSS初期化&ワーク作成
    p_wk->p_mcss_sys	= POKE3D_Init( 1, HEAPID_NAGI_DEBUG_SUB );

    {
      VecFx32 pos	= { -FX32_ONE*5, 0, 0	};
      p_wk->p_mcss_wk		= POKE3D_CreateWkPP( p_wk->p_mcss_sys, p_pp, &pos );
    }

    GFL_HEAP_FreeMemory( p_pp );
  }

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *  @brief  テンプレート  メインプロセス破棄
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT DEBUG_TEMPLATE_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  TEMPLATE_WORK *p_wk;

  p_wk  = p_wk_adrs;

  //MCSS破棄
  POKE3D_DeleteWk( p_wk->p_mcss_sys, p_wk->p_mcss_wk );
	POKE3D_Exit( p_wk->p_mcss_sys );

  //ポケモンBG破棄
 /* {
    GFL_BG_FillCharacterRelease( BG_FRAME_POKEMON_M, 1, 0 );
  }*/

  //ポケモンOBJ破棄
  POKEOBJ_Exit( &p_wk->pokeobj );
#if 0
  //モジュール破棄
  GFL_CLACT_WK_Remove( p_wk->p_hero );
  UI_EASY_CLWK_UnLoadResource( &p_wk->hero_res );
#endif

  TOUCHBAR_Exit( p_wk->p_touchbar );
  INFOWIN_Exit();
  SEQ_Exit( &p_wk->seq );
  UI_TEMPLATE_GRAPHIC_Exit( p_wk->p_graphic );

  //共通モジュールの破棄
  GFL_TCBL_Exit( p_wk->p_tcbl );
  PRINTSYS_QUE_Delete( p_wk->p_que );
  GFL_FONT_Delete( p_wk->p_font );

  //プロセスワーク破棄
  GFL_PROC_FreeWork( p_proc );
  //ヒープ破棄
  GFL_HEAP_DeleteHeap( HEAPID_NAGI_DEBUG_SUB );

  //オーバーレイ破棄
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_debug));
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));


  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *  @brief  テンプレート  メインプロセスメイン処理
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT DEBUG_TEMPLATE_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  TEMPLATE_WORK *p_wk = p_wk_adrs;

  //シーケンス
  SEQ_Main( &p_wk->seq );

  //描画
  UI_TEMPLATE_GRAPHIC_2D_Draw( p_wk->p_graphic );

	//3D描画
	UI_TEMPLATE_GRAPHIC_3D_StartDraw( p_wk->p_graphic );
	POKE3D_Draw( p_wk->p_mcss_sys );
	UI_TEMPLATE_GRAPHIC_3D_EndDraw( p_wk->p_graphic );
  //INFO
  INFOWIN_Update();

  //TOUCHBAR
  TOUCHBAR_Main( p_wk->p_touchbar );

  //プリント
  PRINTSYS_QUE_Main( p_wk->p_que );

  //タスク
  GFL_TCBL_Main( p_wk->p_tcbl );

  //終了
  if( SEQ_IsEnd( &p_wk->seq ) )
  {
    return GFL_PROC_RES_FINISH;
  }
  else
  {
    return GFL_PROC_RES_CONTINUE;
  }
}
//=============================================================================
/**
 *            SEQ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ 初期化
 *
 *  @param  SEQ_WORK *p_wk  ワーク
 *  @param  *p_param        パラメータ
 *  @param  seq_function    シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Init( SEQ_WORK *p_wk, void *p_param, SEQ_FUNCTION seq_function )
{
  //クリア
  GFL_STD_MemClear( p_wk, sizeof(SEQ_WORK) );

  //初期化
  p_wk->p_param = p_param;

  //セット
  SEQ_SetNext( p_wk, seq_function );
}
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ 破棄
 *
 *  @param  SEQ_WORK *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void SEQ_Exit( SEQ_WORK *p_wk )
{
  //クリア
  GFL_STD_MemClear( p_wk, sizeof(SEQ_WORK) );
}
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ メイン処理
 *
 *  @param  SEQ_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Main( SEQ_WORK *p_wk )
{
  if( !p_wk->is_end )
  {
    p_wk->seq_function( p_wk, &p_wk->seq, p_wk->p_param );
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ 終了取得
 *
 *  @param  const SEQ_WORK *cp_wk   ワーク
 *
 *  @return TRUEならば終了  FALSEならば処理中
 */
//-----------------------------------------------------------------------------
static BOOL SEQ_IsEnd( const SEQ_WORK *cp_wk )
{
  return cp_wk->is_end;
}
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ 次のシーケンスを設定
 *
 *  @param  SEQ_WORK *p_wk  ワーク
 *  @param  seq_function    シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_SetNext( SEQ_WORK *p_wk, SEQ_FUNCTION seq_function )
{
  p_wk->seq_function  = seq_function;
  p_wk->seq = 0;
}
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ 終了
 *
 *  @param  SEQ_WORK *p_wk  ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_End( SEQ_WORK *p_wk )
{
  p_wk->is_end  = TRUE;
}
//=============================================================================
/**
 *          SEQFUNC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  フェードOUT
 *
 *  @param  SEQ_WORK *p_seqwk シーケンスワーク
 *  @param  *p_seq          シーケンス
 *  @param  *p_param        ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_FadeOut( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  enum
  {
    SEQ_FADEOUT_START,
    SEQ_FADEOUT_WAIT,
    SEQ_END,
  };

  switch( *p_seq )
  {
  case SEQ_FADEOUT_START:
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
    *p_seq  = SEQ_FADEOUT_WAIT;
    break;

  case SEQ_FADEOUT_WAIT:
    if( !GFL_FADE_CheckFade() )
    {
      *p_seq  = SEQ_END;
    }
    break;

  case SEQ_END:
    SEQ_SetNext( p_seqwk, SEQFUNC_Main );
    break;

  default:
    GF_ASSERT(0);
  }

}
//----------------------------------------------------------------------------
/**
 *  @brief  フェードIN
 *
 *  @param  SEQ_WORK *p_seqwk シーケンスワーク
 *  @param  *p_seq          シーケンス
 *  @param  *p_param        ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_FadeIn( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  enum
  {
    SEQ_FADEIN_START,
    SEQ_FADEIN_WAIT,
    SEQ_EXIT,
  };

  switch( *p_seq )
  {
  case SEQ_FADEIN_START:
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
    *p_seq  = SEQ_FADEIN_WAIT;
    break;

  case SEQ_FADEIN_WAIT:
    if( !GFL_FADE_CheckFade() )
    {
      *p_seq  = SEQ_EXIT;
    }
    break;

  case SEQ_EXIT:
    SEQ_End( p_seqwk );
    break;

  default:
    GF_ASSERT(0);
  }

}
//----------------------------------------------------------------------------
/**
 *  @brief  設定画面メイン処理
 *
 *  @param  SEQ_WORK *p_seqwk シーケンスワーク
 *  @param  *p_seq          シーケンス
 *  @param  *p_param        ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Main( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{

  TEMPLATE_WORK *p_wk = p_param;
  BOOL is_update  = FALSE;
  const int trg = GFL_UI_KEY_GetTrg();
  const int cont = GFL_UI_KEY_GetCont();

  if( GFL_UI_TP_GetTrg() )
  {
    SEQ_SetNext( p_seqwk, SEQFUNC_FadeIn );
  }

  if( trg )
  { 
    //AA BB XX YY

    u32 bit = 0;
    if( cont & PAD_BUTTON_A )
    { 
      bit = 24;
    }
    else if( cont & PAD_BUTTON_B )
    { 
      bit = 16;
    }
    else if( cont & PAD_BUTTON_X )
    { 
      bit = 8;
    }
    else if( cont & PAD_BUTTON_Y )
    { 
      bit = 0;
    }

    if( trg & PAD_KEY_UP )
    {
      p_wk->hero_anm++;
      p_wk->hero_anm  = MATH_CLAMP( p_wk->hero_anm, 0, 6 );
      //NAGI_Printf( "hero anm%d\n", p_wk->hero_anm);
      //GFL_CLACT_WK_SetAnmSeq( p_wk->p_hero, p_wk->hero_anm );

      p_wk->pokeID -= 0x10 << bit;
    }
    else if( trg & PAD_KEY_DOWN )
    { 
      p_wk->hero_anm--;
      p_wk->hero_anm  = MATH_CLAMP( p_wk->hero_anm, 0, 6 );
      //NAGI_Printf( "hero anm%d\n", p_wk->hero_anm);
      //GFL_CLACT_WK_SetAnmSeq( p_wk->p_hero, p_wk->hero_anm );

      p_wk->pokeID += 0x10 << bit;
    }
    else if( trg & PAD_KEY_LEFT )
    { 
      p_wk->pokeID -= 0x01 << bit;
    }
    else if( trg & PAD_KEY_RIGHT )
    { 
      p_wk->pokeID += 0x01 << bit;
    }
    NAGI_Printf( "RND=[0x%x]\n", p_wk->pokeID );

    is_update = TRUE;
  }

  if( is_update )
  {
    POKEMON_PARAM*  p_pp;
    POKEMON_PASO_PARAM  *p_ppp;

    //パッチール作る
    p_pp  = PP_Create( MONSNO_PATTIIRU, 100, PTL_SETUP_ID_AUTO, HEAPID_NAGI_DEBUG_SUB );
    PP_SetupEx( p_pp, MONSNO_PATTIIRU, 100, PTL_SETUP_ID_AUTO, PTL_SETUP_POW_AUTO, p_wk->pokeID );
    p_ppp = PP_GetPPPPointer( p_pp );

    //ポケモンOBJ作り直し
    POKEOBJ_Exit( &p_wk->pokeobj );
    {
      POKEOBJ_Init( &p_wk->pokeobj, UI_TEMPLATE_GRAPHIC_GetClunit( p_wk->p_graphic ), p_ppp, HEAPID_NAGI_DEBUG_SUB );
    }

    //ポケモンMCSS作り直し
    POKE3D_DeleteWk( p_wk->p_mcss_sys, p_wk->p_mcss_wk );
    { 
      VecFx32 pos	= { -FX32_ONE*5, 0, 0	};
      p_wk->p_mcss_wk		= POKE3D_CreateWkPP( p_wk->p_mcss_sys, p_pp, &pos );
    }

    GFL_HEAP_FreeMemory( p_pp );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  設定画面デバッグ
 *
 *  @param  SEQ_WORK *p_seqwk シーケンスワーク
 *  @param  *p_seq          シーケンス
 *  @param  *p_param        ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Search( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_EXIT,
  };
  TEMPLATE_WORK *p_wk = p_param;


  switch( *p_seq )
  {
  case SEQ_INIT:
    {
      int i;
      static const GFL_SKB_SETUP setup = {
        STRBUF_LEN, GFL_SKB_STRTYPE_STRBUF,
        GFL_SKB_MODE_KATAKANA, TRUE, 0, 0,
        BG_FRAME_SKB_M, PLTID_BG_SKB01_M, PLTID_BG_SKB02_M,
      };
      for( i = 0; i < MSGDATA_MAX; i++ )
      {
        p_wk->p_msg[i]  = GFL_MSG_Create( GFL_MSG_LOAD_FAST, ARCID_MESSAGE, PMS_SrcFileID[i], HEAPID_NAGI_DEBUG_SUB );
      }
      GFL_BG_SetVisible( BG_FRAME_BAR_M, FALSE );
      p_wk->p_strbuf  = GFL_STR_CreateBuffer( STRBUF_LEN, HEAPID_NAGI_DEBUG_SUB );
      p_wk->p_skb     = GFL_SKB_Create( (void*)(p_wk->p_strbuf), &setup, HEAPID_NAGI_DEBUG_SUB );
      p_wk->p_search  = MSGSEARCH_Init( p_wk->p_msg, MSGDATA_MAX, HEAPID_NAGI_DEBUG_SUB );

    }
    *p_seq  = SEQ_MAIN;
    break;

  case SEQ_MAIN:
    {
      BOOL is_search  = FALSE;
      GflSkbReaction reaction = GFL_SKB_Main( p_wk->p_skb );

      switch( reaction ){
      case GFL_SKB_REACTION_QUIT:
        *p_seq  = SEQ_EXIT;
        break;
      case GFL_SKB_REACTION_INPUT:
        break;
      case GFL_SKB_REACTION_PAGE:
        break;
      case GFL_SKB_REACTION_NONE:
        if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
          is_search = TRUE;
        }
        break;
      }

      if( is_search )
      {
        u32 max;
        int i;
        STRBUF *p_str;

        GFL_SKB_PickStr( p_wk->p_skb );

        DEBUG_TICK_PrintStart;
        max = MSGSEARCH_SearchAll( p_wk->p_search, p_wk->p_strbuf, p_wk->result_tbl, MSGSEARCH_RESULT_MAX );
        //max = MSGSEARCH_Search( p_wk->p_search, 0, 0, p_wk->p_strbuf, p_wk->result_tbl, MSGSEARCH_RESULT_MAX );
        DEBUG_TICK_PrintEnd;

        NAGI_Printf( "検索最大数 max=%d\n", max );
        NAGI_Printf( "検索文字=" );
        DEBUG_STRBUF_Print( p_wk->p_strbuf );

        for( i = 0; i < max; i++ )
        {
          p_str = MSGSEARCH_CreateString( p_wk->p_search, &p_wk->result_tbl[i] );
          DEBUG_STRBUF_Print( p_str );
          GFL_STR_DeleteBuffer( p_str );
        }
      }
    }
    break;

  case SEQ_EXIT:

    GFL_STR_DeleteBuffer( p_wk->p_strbuf );
    MSGSEARCH_Exit( p_wk->p_search );
    GFL_SKB_Delete( p_wk->p_skb );
    {
      int i;
      for( i = 0; i < MSGDATA_MAX; i++ )
      {
        GFL_MSG_Delete( p_wk->p_msg[i] );
      }
    }
    GFL_BG_SetVisible( BG_FRAME_BAR_M, TRUE );
    SEQ_SetNext( p_seqwk, SEQFUNC_Main );
    break;
  }
}
//=============================================================================
/**
 *  POKEOBJ
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンOBJ作成
 *  
 *	@param	POKEOBJ_WORK *p_wk      ワーク
 *	@param  GFL_CLUNIT    *p_clunit セルアクター登録ユニット
 *	@param	POKEMON_PASO_PARAM  *cp_ppp 表示させるポケモンデータ
 *	@param	heapID                      ヒープ
 */
//-----------------------------------------------------------------------------
static void POKEOBJ_Init( POKEOBJ_WORK *p_wk, GFL_CLUNIT *p_clunit, const POKEMON_PASO_PARAM  *cp_ppp, HEAPID heapID )
{ 
  //リソース読み込み
  { 
    ARCHANDLE           *p_handle;
    p_handle  = POKE2DGRA_OpenHandle( HEAPID_NAGI_DEBUG_SUB );
    p_wk->objres[ OBJRESID_PM_CHR ] = POKE2DGRA_OBJ_CGR_RegisterPPP( p_handle, cp_ppp, POKEGRA_DIR_FRONT, CLSYS_DRAW_MAIN, HEAPID_NAGI_DEBUG_SUB );
    p_wk->objres[ OBJRESID_PM_PLT ] = POKE2DGRA_OBJ_PLTT_RegisterPPP( p_handle, cp_ppp, POKEGRA_DIR_FRONT ,CLSYS_DRAW_MAIN,  PLTID_OBJ_POKEMON_M,  HEAPID_NAGI_DEBUG_SUB );
    p_wk->objres[ OBJRESID_PM_CEL ] = POKE2DGRA_OBJ_CELLANM_RegisterPPP( cp_ppp, POKEGRA_DIR_FRONT, APP_COMMON_MAPPING_128K, CLSYS_DRAW_MAIN, HEAPID_NAGI_DEBUG_SUB );
    GFL_ARC_CloseDataHandle( p_handle );
  }
  //ワーク作成
  {
    GFL_CLUNIT          *p_unit;
    GFL_CLWK_DATA cldata;
    GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

    cldata.pos_x  = 192 + 16;
    cldata.pos_y  = 90;

    p_wk->p_clwk  = GFL_CLACT_WK_Create( p_clunit,
        p_wk->objres[OBJRESID_PM_CHR],
        p_wk->objres[OBJRESID_PM_PLT],
        p_wk->objres[OBJRESID_PM_CEL],
        &cldata,
        CLSYS_DEFREND_MAIN, HEAPID_NAGI_DEBUG_SUB );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンOBJ破棄
 *
 *	@param	POKEOBJ_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void POKEOBJ_Exit( POKEOBJ_WORK *p_wk )
{ 
  //ポケモンOBJ破棄
  {
    GFL_CLACT_WK_Remove( p_wk->p_clwk );
    GFL_CLGRP_PLTT_Release( p_wk->objres[ OBJRESID_PM_PLT ] );
    GFL_CLGRP_CGR_Release( p_wk->objres[ OBJRESID_PM_CHR ] );
    GFL_CLGRP_CELLANIM_Release( p_wk->objres[ OBJRESID_PM_CEL ] );
  }
}

//=============================================================================
/**
 *		MCSS
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	MCSS初期化
 *
 *	@param	u16 wk_max		MCSSのワーク作成最大数
 *	@param	heapID				ヒープID
 *
 *	@return	MCSS_SYS
 */
//-----------------------------------------------------------------------------
static MCSS_SYS_WORK * POKE3D_Init( u16 wk_max, HEAPID heapID )
{	
	MCSS_SYS_WORK *mcss;
	mcss = MCSS_Init( wk_max , heapID );
	MCSS_SetTextureTransAdrs( mcss, 0 );
	MCSS_SetOrthoMode( mcss );
	return mcss;
}
//----------------------------------------------------------------------------
/**
 *	@brief	MCSS破棄
 *
 *	@param	MCSS_SYS_WORK * mcss	MCSS＿SYS
 */
//-----------------------------------------------------------------------------
static void POKE3D_Exit( MCSS_SYS_WORK * mcss )
{	
	MCSS_Exit(mcss);
}
//----------------------------------------------------------------------------
/**
 *	@brief	MCSS描画
 *
 *	@param	MCSS_SYS_WORK * mcss MCSS＿SYS
 */
//-----------------------------------------------------------------------------
static void POKE3D_Draw( MCSS_SYS_WORK * mcss )
{	
	MCSS_Main( mcss );
	MCSS_Draw( mcss );
}
//----------------------------------------------------------------------------
/**
 *	@brief	MCSSワーク作成
 *
 *	@param	MCSS_SYS_WORK * mcss	MCSS_SYS
 *	@param	*ppp					ポケモンパーソナルパラメータ
 *	@param	VecFx32 *pos	位置
 *
 *	@return	MCSS_WK
 */
//-----------------------------------------------------------------------------
static MCSS_WORK * POKE3D_CreateWkPP( MCSS_SYS_WORK * mcss, POKEMON_PARAM *pp, const VecFx32 *pos )
{	
	MCSS_WORK *poke;
	MCSS_ADD_WORK addWork;

	VecFx32 scale = {FX32_ONE*16,FX32_ONE*16,FX32_ONE};

	MCSS_TOOL_MakeMAWPP( pp , &addWork , MCSS_DIR_FRONT );
	poke = MCSS_Add( mcss, pos->x, pos->y , pos->z, &addWork );
	MCSS_SetScale( poke , &scale );
	return poke;
}
//----------------------------------------------------------------------------
/**
 *	@brief	MCSSワーク破棄
 *
 *	@param	MCSS_SYS_WORK * mcss	MCSS_SYS
 *	@param	*wk										ワーク
 *
 */
//-----------------------------------------------------------------------------
static void POKE3D_DeleteWk( MCSS_SYS_WORK * mcss, MCSS_WORK *wk )
{	
	MCSS_SetVanishFlag( wk );
	MCSS_Del(mcss,wk);
}
