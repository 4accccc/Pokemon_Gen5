//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_view.c
 *	@brief	ランダムマッチ表示関係
 *	@author	Toru=Nagihashi
 *	@data		2009.11.04
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//	lib
#include <gflib.h>

//	SYSTEM
#include "system/gf_date.h"
#include "system/bmp_winframe.h"

//	module
#include "pokeicon/pokeicon.h"
#include "ui/print_tool.h"
#include "system/pms_draw.h"
#include "system/nsbtx_to_clwk.h"

//	archive
#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"
#include "wifimatch_gra.naix"
#include "msg/msg_wifi_match.h"
#include "app/app_menu_common.h"
#include "wifi_unionobj_plt.cdat"
#include "wifi_unionobj.naix"

//	print
#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"

// mine
#include "wifibattlematch_view.h"

//-------------------------------------
///	トレーナーOBJ
//=====================================
#define WBM_CARD_TRAINER_RES_NCGR(x)      (NARC_wifi_unionobj_front00_NCGR+x)
#define WBM_CARD_TRAINER_RES_NCER         (NARC_wifi_unionobj_front00_NCER)
#define WBM_CARD_TRAINER_RES_NANR         (NARC_wifi_unionobj_front00_NANR)
#define WBM_CARD_TRAINER_RES_NCLR         (NARC_wifi_unionobj_wf_match_top_trainer_NCLR)
#define WBM_CARD_TRAINER_RES_PLT_OFS(x)   (sc_wifi_unionobj_plt[x])

#define WBM_CARD_INIT_POS_X (256)

//-------------------------------------
///	共通で使う
//    ランク＝星の数になっている
//=====================================
typedef enum
{ 
  WBM_CARD_RANK_NORMAL,
  WBM_CARD_RANK_COPPER,
  WBM_CARD_RANK_BRONZE,
  WBM_CARD_RANK_SILVER,
  WBM_CARD_RANK_GOLD,
  WBM_CARD_RANK_BLACK,
}WBM_CARD_RANK;

//-------------------------------------
///	プロトタイプ
//=====================================
static WBM_CARD_RANK CalcRank( u32 btl_cnt, u32 win, u32 lose );
static void BG_MainPltAnm( NNS_GFD_DST_TYPE type, u16 *p_buff, u16 *p_cnt, u16 add, u8 plt_num, u8 plt_col, GXRgb start, GXRgb end );

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					リソース読み込み
 *					  共通素材を読み込みます
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	定数
//=====================================

//-------------------------------------
///	リソース管理ワーク
//=====================================
struct _WIFIBATTLEMATCH_VIEW_RESOURCE
{ 
  u32 res[WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_MAX];
  u16 plt[0x10];
  u16 plt_original[0x10];
  u16 plt_next[0x10];
  u16 cnt;
};

//-------------------------------------
///	パブリック
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  リソース読み込み
 *
 *	@param	GFL_CLUNIT *p_unit  CLUNIT
 *	@param  mode                読み込みモード
 *	@param	heapID              ヒープID 
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
WIFIBATTLEMATCH_VIEW_RESOURCE *WIFIBATTLEMATCH_VIEW_LoadResource( GFL_CLUNIT *p_unit, WIFIBATTLEMATCH_VIEW_RES_MODE mode, HEAPID heapID )
{ 
  WIFIBATTLEMATCH_VIEW_RESOURCE	*	p_wk;

  //ワーク作成
	p_wk	= GFL_HEAP_AllocMemory( heapID, sizeof(WIFIBATTLEMATCH_VIEW_RESOURCE) );
	GFL_STD_MemClear( p_wk, sizeof(WIFIBATTLEMATCH_VIEW_RESOURCE) );
  //ランダムマッチリソースファイルーーーーーーーーーー
	{	
		ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_WIFIMATCH_GRA, GFL_HEAP_LOWID( heapID ) );


    //●--BG--

		//PLT
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_wifimatch_gra_bg_NCLR,
				PALTYPE_MAIN_BG, 0, 0, GFL_HEAP_LOWID( heapID ) );
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_wifimatch_gra_bg_NCLR,
				PALTYPE_SUB_BG, 0, 0, GFL_HEAP_LOWID( heapID ) );

		//CHR
    //  CARD,BACKは共通キャラ
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_wifimatch_gra_bg_back_NCGR, 
				BG_FRAME_M_BACK, 0, 0, FALSE, GFL_HEAP_LOWID( heapID ) );
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_wifimatch_gra_bg_back_NCGR, 
				BG_FRAME_S_BACK, 0, 0, FALSE, GFL_HEAP_LOWID( heapID ) );

		//SCR
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_wifimatch_gra_back_NSCR,
				BG_FRAME_M_BACK, 0, 0, FALSE, GFL_HEAP_LOWID( heapID ) );
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_wifimatch_gra_back2_NSCR,
				BG_FRAME_S_BACK, 0, 0, FALSE, GFL_HEAP_LOWID( heapID ) );



    switch( mode )
    { 
    case WIFIBATTLEMATCH_VIEW_RES_MODE_RANDOM:
      GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_wifimatch_gra_random_card_NSCR,
				BG_FRAME_S_CARD, 0, 0, FALSE, GFL_HEAP_LOWID( heapID ) );
      break;

    case WIFIBATTLEMATCH_VIEW_RES_MODE_DIGITALCARD:
      GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_wifimatch_gra_wcs_card_NSCR,
				BG_FRAME_M_CARD, 0, 0, FALSE, GFL_HEAP_LOWID( heapID ) );
      break;

    case WIFIBATTLEMATCH_VIEW_RES_MODE_WIFI:
      /* fallthr */
    case WIFIBATTLEMATCH_VIEW_RES_MODE_LIVE:
      GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_wifimatch_gra_wcs_card_NSCR,
				BG_FRAME_S_CARD, 0, 0, FALSE, GFL_HEAP_LOWID( heapID ) );
      break;
    }

    //●--OBJ--
    p_wk->res[ WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_PLT_M ] = 
      GFL_CLGRP_PLTT_Register( p_handle, NARC_wifimatch_gra_obj_NCLR, CLSYS_DRAW_MAIN, PLT_OBJ_M * 0x20, GFL_HEAP_LOWID( heapID ) );
    p_wk->res[ WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_CHR_M ] = 
      GFL_CLGRP_CGR_Register( p_handle, NARC_wifimatch_gra_obj_NCGR, FALSE, CLSYS_DRAW_MAIN, GFL_HEAP_LOWID( heapID ) );
    p_wk->res[ WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_CEL_M ] = 
      GFL_CLGRP_CELLANIM_Register( p_handle, NARC_wifimatch_gra_obj_NCER, NARC_wifimatch_gra_obj_NANR,
          GFL_HEAP_LOWID( heapID ) );
    p_wk->res[ WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_PLT_S ] = 
      GFL_CLGRP_PLTT_Register( p_handle, NARC_wifimatch_gra_obj_NCLR, CLSYS_DRAW_SUB, PLT_OBJ_S * 0x20, GFL_HEAP_LOWID( heapID ) );
    p_wk->res[ WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_CHR_S ] = 
      GFL_CLGRP_CGR_Register( p_handle, NARC_wifimatch_gra_obj_NCGR, FALSE, CLSYS_DRAW_SUB, GFL_HEAP_LOWID( heapID ) );
    p_wk->res[ WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_CEL_S ] = 
      GFL_CLGRP_CELLANIM_Register( p_handle, NARC_wifimatch_gra_obj_NCER, NARC_wifimatch_gra_obj_NANR,
          GFL_HEAP_LOWID( heapID ) );


    //パレットフェードする色を保存しておく
    { 
      void *p_buff;
      NNSG2dPaletteData *p_plt;
      const u16 *cp_plt_adrs;

      //もとのパレットから色情報を保存
      p_buff  = GFL_ARCHDL_UTIL_LoadPalette( p_handle, NARC_wifimatch_gra_bg_NCLR, &p_plt, heapID );
      cp_plt_adrs = p_plt->pRawData;
      GFL_STD_MemCopy( cp_plt_adrs, p_wk->plt_original, sizeof(u16) * 0x10 );
      GFL_STD_MemCopy( (u8*)cp_plt_adrs + 9 * 0x20, p_wk->plt_next, sizeof(u16) * 0x10 );

      //パレット破棄
      GFL_HEAP_FreeMemory( p_buff );
    }

		GFL_ARC_CloseDataHandle( p_handle );
  }

  //共通リソースファイルーーーーーーーーーー
  { 
		GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr,
				PALTYPE_MAIN_BG, PLT_FONT_M*0x20, 0x20, GFL_HEAP_LOWID( heapID ) );
		GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr,
				PALTYPE_SUB_BG, PLT_FONT_S*0x20, 0x20, GFL_HEAP_LOWID( heapID ) );

    
    GFL_BG_FillCharacter( BG_FRAME_M_TEXT, 0, 1,  CGR_OFS_M_CLEAR );
    BmpWinFrame_GraphicSet( BG_FRAME_M_TEXT, CGR_OFS_M_LIST, PLT_LIST_M, MENU_TYPE_SYSTEM, GFL_HEAP_LOWID( heapID ) );
    TalkWinFrame_GraphicSet( BG_FRAME_M_TEXT, CGR_OFS_M_TEXT, PLT_TEXT_M, MENU_TYPE_SYSTEM, GFL_HEAP_LOWID( heapID ) );
	}


  //カードは横にずらしておく
  GFL_BG_SetScroll( BG_FRAME_M_CARD, GFL_BG_SCROLL_X_SET, -WBM_CARD_INIT_POS_X );
  GFL_BG_SetScroll( BG_FRAME_M_FONT, GFL_BG_SCROLL_X_SET, -WBM_CARD_INIT_POS_X );
  GFL_BG_SetScroll( BG_FRAME_S_CARD, GFL_BG_SCROLL_X_SET, -WBM_CARD_INIT_POS_X );
  GFL_BG_SetScroll( BG_FRAME_S_FONT, GFL_BG_SCROLL_X_SET, -WBM_CARD_INIT_POS_X );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  リソース破棄
 *
 *	@param	WIFIBATTLEMATCH_VIEW_RESOURCE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_VIEW_UnLoadResource( WIFIBATTLEMATCH_VIEW_RESOURCE *p_wk )
{ 
  //リソース解放
  { 
    GFL_CLGRP_PLTT_Release( p_wk->res[ WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_PLT_M ] );
    GFL_CLGRP_CGR_Release( p_wk->res[ WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_CHR_M ]  );
    GFL_CLGRP_CELLANIM_Release( p_wk->res[ WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_CEL_M ] );
    GFL_CLGRP_PLTT_Release( p_wk->res[ WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_PLT_S ] );
    GFL_CLGRP_CGR_Release( p_wk->res[ WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_CHR_S ]  );
    GFL_CLGRP_CELLANIM_Release( p_wk->res[ WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_CEL_S ] );

    GFL_BG_FillCharacterRelease(BG_FRAME_M_TEXT, 1, CGR_OFS_M_CLEAR ); 
  }

  //ワーク破棄
	GFL_HEAP_FreeMemory( p_wk ); 
}

//----------------------------------------------------------------------------
/**
 *	@brief  リソース取得
 *
 *	@param	const WIFIBATTLEMATCH_VIEW_RESOURCE *cp_wk  ワーク
 *	@param	type  
 *
 *	@return
 */
//-----------------------------------------------------------------------------
u32 WIFIBATTLEMATCH_VIEW_GetResource( const WIFIBATTLEMATCH_VIEW_RESOURCE *cp_wk, WIFIBATTLEMATCH_VIEW_RES_TYPE type )
{ 
  return cp_wk->res[ type ];
}
//----------------------------------------------------------------------------
/**
 *	@brief  メイン
 *
 *	@param	WIFIBATTLEMATCH_VIEW_RESOURCE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_VIEW_Main( WIFIBATTLEMATCH_VIEW_RESOURCE *p_wk )
{ 
  int i;
  const u16 add = 0x200;

  //パレットアニメ
  if( p_wk->cnt + add >= 0x10000 )
  {
    p_wk->cnt = p_wk->cnt+add-0x10000;
  }
  else
  {
    p_wk->cnt += add;
  }

  for( i = 0; i < 0x10; i++ )
  { 
    BG_MainPltAnm( NNS_GFD_DST_2D_BG_PLTT_MAIN, &p_wk->plt[i], &p_wk->cnt, 0x0, PLT_BG_M, i, p_wk->plt_original[i], p_wk->plt_next[i] );
    BG_MainPltAnm( NNS_GFD_DST_2D_BG_PLTT_SUB, &p_wk->plt[i], &p_wk->cnt, 0x0, PLT_BG_S, i, p_wk->plt_original[i], p_wk->plt_next[i] );
    BG_MainPltAnm( NNS_GFD_DST_2D_OBJ_PLTT_MAIN, &p_wk->plt[i], &p_wk->cnt, 0x0, 2, i, p_wk->plt_original[i], p_wk->plt_next[i] );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  BGパレットフェード
 *
 *  @param  u16 *p_buff 色保存バッファ
 *  @param  *p_cnt      カウンタバッファ
 *  @param  add         カウンタ加算値
 *  @param  plt_num     パレット縦番号
 *  @param  plt_col     パレット横番号
 *  @param  start       開始色
 *  @param  end         終了色
 */
//-----------------------------------------------------------------------------
static void BG_MainPltAnm( NNS_GFD_DST_TYPE type, u16 *p_buff, u16 *p_cnt, u16 add, u8 plt_num, u8 plt_col, GXRgb start, GXRgb end )
{ 

  {
    //1～0に変換
    const fx16 cos = (FX_CosIdx(*p_cnt)+FX16_ONE)/2;
    const u8 start_r  = (start & GX_RGB_R_MASK ) >> GX_RGB_R_SHIFT;
    const u8 start_g  = (start & GX_RGB_G_MASK ) >> GX_RGB_G_SHIFT;
    const u8 start_b  = (start & GX_RGB_B_MASK ) >> GX_RGB_B_SHIFT;
    const u8 end_r  = (end & GX_RGB_R_MASK ) >> GX_RGB_R_SHIFT;
    const u8 end_g  = (end & GX_RGB_G_MASK ) >> GX_RGB_G_SHIFT;
    const u8 end_b  = (end & GX_RGB_B_MASK ) >> GX_RGB_B_SHIFT;

    const u8 r = start_r + (((end_r-start_r)*cos)>>FX16_SHIFT);
    const u8 g = start_g + (((end_g-start_g)*cos)>>FX16_SHIFT);
    const u8 b = start_b + (((end_b-start_b)*cos)>>FX16_SHIFT);
    
    *p_buff = GX_RGB(r, g, b);
    
    NNS_GfdRegisterNewVramTransferTask( type,
                                        plt_num * 32 + plt_col * 2,
                                        p_buff, 2 );
  }
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					プレイヤー情報表示
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
enum
{	
	PLAYERINFO_BMPWIN_MAX	= 30,
	PLAYERINFO_STAR_MAX	  = 5,
};
enum
{	
	PLAYERINFO_CLWK_POKEICON_TOP,
	PLAYERINFO_CLWK_POKEICON_END	= PLAYERINFO_CLWK_POKEICON_TOP + TEMOTI_POKEMAX,
	PLAYERINFO_CLWK_TRAINER,
	PLAYERINFO_CLWK_LOCK,
  PLAYERINFO_CLWK_STAR_TOP,
  PLAYERINFO_CLWK_STAR_END = PLAYERINFO_CLWK_STAR_TOP + PLAYERINFO_STAR_MAX,
	PLAYERINFO_CLWK_MAX,
};
enum
{	
	PLAYERINFO_RESID_POKEICON_CGR_TOP,
	PLAYERINFO_RESID_POKEICON_CGR_END	= PLAYERINFO_RESID_POKEICON_CGR_TOP+TEMOTI_POKEMAX,
	PLAYERINFO_RESID_POKEICON_CELL,
	PLAYERINFO_RESID_POKEICON_PLT,
	PLAYERINFO_RESID_TRAINER_PLT,
	PLAYERINFO_RESID_TRAINER_CGR,
	PLAYERINFO_RESID_TRAINER_CELL,
	PLAYERINFO_RESID_MAX,
};

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	定数
//=====================================
#define CARD_WAIT_SYNC (20)

//-------------------------------------
///	プレイヤー情報表示ワーク
//=====================================
struct _PLAYERINFO_WORK
{	
	u32					bmpwin_max;
	GFL_BMPWIN	*p_bmpwin[ PLAYERINFO_BMPWIN_MAX ];
	PRINT_UTIL	print_util[ PLAYERINFO_BMPWIN_MAX ];
	GFL_CLWK		*p_clwk[ PLAYERINFO_CLWK_MAX ];
	u32					res[PLAYERINFO_RESID_MAX];
	u32					clwk_max;
  u32         cnt;
  s16         clwk_x[ PLAYERINFO_CLWK_MAX ];
  u8          frm;
  u8          plt;
  u8          pokemonicon_plt;
  u8          trainer_plt;
  CLSYS_DRAW_TYPE cl_draw_type;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	BMPWIN作成
//=====================================
static void PlayerInfo_Bmpwin_Rnd_Create( PLAYERINFO_WORK * p_wk, BOOL is_rate, const PLAYERINFO_RANDOMMATCH_DATA *cp_data, const MYSTATUS* p_my, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, BOOL is_black, HEAPID heapID );
static void PlayerInfo_Bmpwin_Wifi_Create( PLAYERINFO_WORK * p_wk, BOOL is_limit, const PLAYERINFO_WIFICUP_DATA *cp_data, const MYSTATUS* p_my, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, HEAPID heapID );
static void PlayerInfo_Bmpwin_Live_Create( PLAYERINFO_WORK * p_wk, const PLAYERINFO_LIVECUP_DATA *cp_data,const MYSTATUS* p_my, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, HEAPID heapID );
static BOOL PlayerInfo_Bmpwin_PrintMain( PLAYERINFO_WORK * p_wk, PRINT_QUE *p_que );
static void PlayerInfo_Bmpwin_Delete( PLAYERINFO_WORK * p_wk );
//-------------------------------------
///	CLWK作成
//=====================================
static void PlayerInfo_POKEICON_Create( PLAYERINFO_WORK * p_wk, GFL_CLUNIT *p_unit, BATTLE_BOX_SAVE * p_btlbox_sv, HEAPID heapID );
static void PlayerInfo_POKEICON_Delete( PLAYERINFO_WORK * p_wk );

static void PlayerInfo_TRAINER_Cleate( PLAYERINFO_WORK * p_wk, u32 trainerID, GFL_CLUNIT *p_unit, HEAPID heapID );
static void PlayerInfo_TRAINER_Delete( PLAYERINFO_WORK * p_wk );

static void PlayerInfo_LOCK_Cleate( PLAYERINFO_WORK * p_wk, GFL_CLUNIT *p_unit, const WIFIBATTLEMATCH_VIEW_RESOURCE *cp_res, HEAPID heapID );
static void PlayerInfo_LOCK_Delete( PLAYERINFO_WORK * p_wk );

static void PlayerInfo_STAR_Cleate( PLAYERINFO_WORK * p_wk, const PLAYERINFO_RANDOMMATCH_DATA *cp_data, GFL_CLUNIT *p_unit, const WIFIBATTLEMATCH_VIEW_RESOURCE *cp_res, HEAPID heapID );
static void PlayerInfo_STAR_Delete( PLAYERINFO_WORK * p_wk );
//=============================================================================
/**
 *					ランダムマッチ外部公開関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ランダムマッチプレイヤー情報表示	初期化
 *
 *	@param	cp_data							      情報構造体
 *	@param	*p_unit										セル作成のためのユニット
 *	@param	PRINT_QUE *p_que					文字表示用キュー
 *	@param	*p_msg										文字表示用メッセージデータ
 *	@param	*p_word										文字表示用単語登録
 *	@param	heapID										ヒープID
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
PLAYERINFO_WORK *PLAYERINFO_RND_Init( const PLAYERINFO_RANDOMMATCH_DATA *cp_data, BOOL is_rate, const MYSTATUS* p_my, GFL_CLUNIT *p_unit, const WIFIBATTLEMATCH_VIEW_RESOURCE *cp_res, GFL_FONT	*p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, HEAPID heapID )
{
  PLAYERINFO_WORK	*	p_wk;
  WBM_CARD_RANK rank;

  //ワーク作成
	p_wk	= GFL_HEAP_AllocMemory( heapID, sizeof(PLAYERINFO_WORK) );
	GFL_STD_MemClear( p_wk, sizeof(PLAYERINFO_WORK) );

    p_wk->frm = PLAYERINFO_BG_FRAME;
    p_wk->plt = PLAYERINFO_PLT_BG_FONT;
    p_wk->pokemonicon_plt = PLAYERINFO_PLT_OBJ_POKEICON;
    p_wk->trainer_plt = PLAYERINFO_PLT_OBJ_TRAINER;
    p_wk->cl_draw_type  = CLSYS_DRAW_SUB;

    G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG3, PLAYERINFO_ALPHA_EV1, PLAYERINFO_ALPHA_EV2);

  //BMPWIN作成
  rank  = CalcRank( cp_data->btl_cnt, cp_data->win_cnt, cp_data->lose_cnt );
  PlayerInfo_Bmpwin_Rnd_Create( p_wk, is_rate, cp_data, p_my, p_font, p_que, p_msg, p_word, rank == WBM_CARD_RANK_BLACK, heapID );
  //トレーナー作成
  PlayerInfo_TRAINER_Cleate( p_wk, cp_data->trainerID, p_unit, heapID );
  {	
    GFL_CLACTPOS	pos;
    pos.x	= PLAYERINFO_TRAINER_RND_X + WBM_CARD_INIT_POS_X;
    pos.y = PLAYERINFO_TRAINER_RND_Y;
    GFL_CLACT_WK_SetPos( p_wk->p_clwk[ PLAYERINFO_CLWK_TRAINER ], &pos, CLSYS_DRAW_SUB );
  }
  //星作成
  PlayerInfo_STAR_Cleate( p_wk, cp_data, p_unit, cp_res, heapID );

  { 
    GFL_BG_ChangeScreenPalette( BG_FRAME_S_CARD, 0, 0, 32, 6, 1 );
    GFL_BG_ChangeScreenPalette( BG_FRAME_S_CARD, 0, 6, 32, 18, 3 + rank );
    GFL_BG_LoadScreenReq( BG_FRAME_S_CARD );
  }

  G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG3, PLAYERINFO_ALPHA_EV1, PLAYERINFO_ALPHA_EV2);

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ランダムマッチプレイヤー情報表示  破棄
 *
 *	@param	PLAYERINFO_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void PLAYERINFO_RND_Exit( PLAYERINFO_WORK *p_wk )
{ 
  G2S_BlendNone();

  //星破棄
  PlayerInfo_STAR_Delete( p_wk );

  //BMPWIN破棄
  PlayerInfo_Bmpwin_Delete( p_wk );

  //ワーク破棄
	GFL_HEAP_FreeMemory( p_wk );
}
//=============================================================================
/**
 *					WIFI大会外部公開関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	WIFI大会プレイヤー情報表示	初期化
 *
 *	@param	cp_data							      情報構造体
 *	@param	*p_unit										セル作成のためのユニット
 *	@param	PRINT_QUE *p_que					文字表示用キュー
 *	@param	*p_msg										文字表示用メッセージデータ
 *	@param	*p_word										文字表示用単語登録
 *	@param  p_btl_box                 バトルボックスセーブデータ
 *	@param	heapID										ヒープID
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
PLAYERINFO_WORK *PLAYERINFO_WIFI_Init( const PLAYERINFO_WIFICUP_DATA *cp_data, BOOL is_limit, const MYSTATUS* p_my, GFL_CLUNIT *p_unit, const WIFIBATTLEMATCH_VIEW_RESOURCE *cp_res, GFL_FONT	*p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, BATTLE_BOX_SAVE * p_btlbox_sv, BOOL is_main, HEAPID heapID )
{ 
  PLAYERINFO_WORK	*	p_wk;

  //ワーク作成
	p_wk	= GFL_HEAP_AllocMemory( heapID, sizeof(PLAYERINFO_WORK) );
	GFL_STD_MemClear( p_wk, sizeof(PLAYERINFO_WORK) );

  //メインとサブで読み込み先変更
  if( is_main )
  { 
    p_wk->frm = PLAYERINFO_BG_FRAME_MAIN;
    p_wk->plt = PLAYERINFO_PLT_BG_FONT_MAIN;
    p_wk->pokemonicon_plt = PLAYERINFO_PLT_OBJ_POKEICON_MAIN;
    p_wk->trainer_plt = PLAYERINFO_PLT_OBJ_TRAINER_MAIN;
    p_wk->cl_draw_type  = CLSYS_DRAW_MAIN;

    G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG3, PLAYERINFO_ALPHA_EV1, PLAYERINFO_ALPHA_EV2);
  }
  else
  { 
    p_wk->frm = PLAYERINFO_BG_FRAME;
    p_wk->plt = PLAYERINFO_PLT_BG_FONT;
    p_wk->pokemonicon_plt = PLAYERINFO_PLT_OBJ_POKEICON;
    p_wk->trainer_plt = PLAYERINFO_PLT_OBJ_TRAINER;
    p_wk->cl_draw_type  = CLSYS_DRAW_SUB;

    G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG3, PLAYERINFO_ALPHA_EV1, PLAYERINFO_ALPHA_EV2);
  }

  //BMPWIN作成
  PlayerInfo_Bmpwin_Wifi_Create( p_wk, is_limit, cp_data, p_my, p_font, p_que, p_msg, p_word, heapID );

  //ポケアイコン作成
  PlayerInfo_POKEICON_Create( p_wk, p_unit, p_btlbox_sv, heapID );

  //トレーナー作成
  PlayerInfo_TRAINER_Cleate( p_wk, cp_data->trainerID, p_unit, heapID );
  {	
    GFL_CLACTPOS	pos;
    pos.x	= PLAYERINFO_TRAINER_CUP_X + WBM_CARD_INIT_POS_X;
    pos.y = PLAYERINFO_TRAINER_CUP_Y;
    GFL_CLACT_WK_SetPos( p_wk->p_clwk[ PLAYERINFO_CLWK_TRAINER ], &pos, p_wk->cl_draw_type );
  }

  //鍵作成
  PlayerInfo_LOCK_Cleate( p_wk, p_unit, cp_res, heapID );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  WIFI大会プレイヤー情報表示  破棄
 *
 *	@param	PLAYERINFO_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void PLAYERINFO_WIFI_Exit( PLAYERINFO_WORK *p_wk )
{ 
  G2S_BlendNone();

  PlayerInfo_LOCK_Delete( p_wk );

	PlayerInfo_POKEICON_Delete( p_wk );

	PlayerInfo_Bmpwin_Delete( p_wk );

	GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  WIFI大会プレイヤー情報  データ更新
 *
 *	@param	PLAYERINFO_WORK *p_wk ワーク
 *	@param	type                  更新タイプ
 *
 */
//-----------------------------------------------------------------------------
void PLAYERINFO_WIFI_RenewalData( PLAYERINFO_WORK *p_wk, PLAYERINFO_WIFI_UPDATE_TYPE type, GFL_MSGDATA *p_msg, PRINT_QUE *p_que, GFL_FONT *p_font, HEAPID heapID )
{ 
  int i;
  STRBUF	*p_strbuf;

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_wk->p_bmpwin[7]), 0 );

  switch( type )
  { 
  case PLAYERINFO_WIFI_UPDATE_TYPE_LOCK:
    for( i = 0; i < p_wk->clwk_max; i++ )
		{	
      if( p_wk->p_clwk[i] )
      { 
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
      }
		}
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[PLAYERINFO_CLWK_LOCK], TRUE );

    p_strbuf  = GFL_MSG_CreateString( p_msg, WIFIMATCH_STR_017 );
    PRINT_UTIL_PrintColor( &p_wk->print_util[7], p_que, 0, 0, p_strbuf, p_font, PLAYERINFO_STR_COLOR_WHITE );
    break;

  case PLAYERINFO_WIFI_UPDATE_TYPE_UNLOCK:
    for( i = 0; i < p_wk->clwk_max; i++ )
		{	
      if( p_wk->p_clwk[i] )
      { 
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
      }
		}
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[PLAYERINFO_CLWK_LOCK], FALSE );

    p_strbuf  = GFL_MSG_CreateString( p_msg, WIFIMATCH_STR_017_01 );
    PRINT_UTIL_PrintColor( &p_wk->print_util[7], p_que, 0, 0, p_strbuf, p_font, PLAYERINFO_STR_COLOR_WHITE );
    break;

  case PLAYERINFO_WIFI_UPDATE_TYPE_UNREGISTER:
    for( i = 0; i < p_wk->clwk_max; i++ )
		{	
      if( p_wk->p_clwk[i] )
      { 
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], FALSE );
      }
		}
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[PLAYERINFO_CLWK_LOCK], FALSE );

    p_strbuf  = GFL_MSG_CreateString( p_msg, WIFIMATCH_STR_017_02 );
    PRINT_UTIL_PrintColor( &p_wk->print_util[7], p_que, 0, 0, p_strbuf, p_font, PLAYERINFO_STR_COLOR_WHITE );
    break;
  }

  GFL_STR_DeleteBuffer( p_strbuf );
}
//=============================================================================
/**
 *					LIVE大会外部公開関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	LIVE大会プレイヤー情報表示	初期化
 *
 *	@param	cp_data							      情報構造体
 *	@param	*p_unit										セル作成のためのユニット
 *	@param	PRINT_QUE *p_que					文字表示用キュー
 *	@param	*p_msg										文字表示用メッセージデータ
 *	@param	*p_word										文字表示用単語登録
 *	@param	heapID										ヒープID
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
PLAYERINFO_WORK *PLAYERINFO_LIVE_Init( const PLAYERINFO_LIVECUP_DATA *cp_data, const MYSTATUS* p_my, GFL_CLUNIT *p_unit, const WIFIBATTLEMATCH_VIEW_RESOURCE *cp_res, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, BATTLE_BOX_SAVE * p_btlbox_sv, HEAPID heapID )
{ 
  PLAYERINFO_WORK	*	p_wk;

  //ワーク作成
	p_wk	= GFL_HEAP_AllocMemory( heapID, sizeof(PLAYERINFO_WORK) );
	GFL_STD_MemClear( p_wk, sizeof(PLAYERINFO_WORK) );

    p_wk->frm = PLAYERINFO_BG_FRAME;
    p_wk->plt = PLAYERINFO_PLT_BG_FONT;
    p_wk->pokemonicon_plt = PLAYERINFO_PLT_OBJ_POKEICON;
    p_wk->trainer_plt = PLAYERINFO_PLT_OBJ_TRAINER;
    p_wk->cl_draw_type  = CLSYS_DRAW_SUB;

    G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG3, PLAYERINFO_ALPHA_EV1, PLAYERINFO_ALPHA_EV2);

  //BMPWIN作成
  PlayerInfo_Bmpwin_Live_Create( p_wk, cp_data, p_my, p_font, p_que, p_msg, p_word, heapID );

  //ポケアイコン作成
  PlayerInfo_POKEICON_Create( p_wk, p_unit, p_btlbox_sv, heapID );

  //トレーナー作成
  PlayerInfo_TRAINER_Cleate( p_wk, cp_data->trainerID, p_unit, heapID );
  {	
    GFL_CLACTPOS	pos;
    pos.x	= PLAYERINFO_TRAINER_CUP_X + WBM_CARD_INIT_POS_X;
    pos.y = PLAYERINFO_TRAINER_CUP_Y;
    GFL_CLACT_WK_SetPos( p_wk->p_clwk[ PLAYERINFO_CLWK_TRAINER ], &pos, CLSYS_DRAW_SUB );
  }
  G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG3, PLAYERINFO_ALPHA_EV1, PLAYERINFO_ALPHA_EV2);

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  LIVE大会プレイヤー情報表示  破棄
 *
 *	@param	PLAYERINFO_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void PLAYERINFO_LIVE_Exit( PLAYERINFO_WORK *p_wk )
{ 
  G2S_BlendNone();

	PlayerInfo_POKEICON_Delete( p_wk );

	PlayerInfo_Bmpwin_Delete( p_wk );

	GFL_HEAP_FreeMemory( p_wk );
}
//=============================================================================
/**
 *					共通外部公開関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	プリントメイン
 *
 *	@param	PLAYERINFO_WORK * p_wk	ワーク
 *	@param	PRINT_QUE	*p_que				プリントキュー
 *
 *	@return	TRUEで全てのプリント終了	FALSEでまだ
 */
//-----------------------------------------------------------------------------
BOOL PLAYERINFO_PrintMain( PLAYERINFO_WORK * p_wk, PRINT_QUE *p_que )
{	
 	return PlayerInfo_Bmpwin_PrintMain( p_wk, p_que );
}
//----------------------------------------------------------------------------
/**
 *	@brief  カードをスライドイン
 *
 *	@param	PLAYERINFO_WORK *p_wk  ワーク
 *
 *	@return TRUEで完了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
BOOL PLAYERINFO_MoveMain( PLAYERINFO_WORK * p_wk )
{ 
  if( GFL_BG_GetScrollX( p_wk->frm ) == 0 )
  { 
    return TRUE;
  }
  else
  { 
    BOOL ret  = FALSE;
  
    s32 x;

    if( p_wk->cnt == 0 )
    { 
      int i;
      GFL_CLACTPOS  pos;
      for( i = 0; i < PLAYERINFO_CLWK_MAX; i++ )
      { 
        if( p_wk->p_clwk[i] )
        { 
          GFL_CLACT_WK_GetPos( p_wk->p_clwk[i], &pos, p_wk->cl_draw_type );
          p_wk->clwk_x[i]  = pos.x;
        }
      }
    }

    x = -WBM_CARD_INIT_POS_X + WBM_CARD_INIT_POS_X * p_wk->cnt / CARD_WAIT_SYNC;

    { 
      int i;
      GFL_CLACTPOS  pos;
      for( i = 0; i < PLAYERINFO_CLWK_MAX; i++ )
      { 
        if( p_wk->p_clwk[i] )
        { 
          GFL_CLACT_WK_GetPos( p_wk->p_clwk[i], &pos, p_wk->cl_draw_type );
          pos.x = p_wk->clwk_x[i] - WBM_CARD_INIT_POS_X * p_wk->cnt / CARD_WAIT_SYNC;
          GFL_CLACT_WK_SetPos( p_wk->p_clwk[i], &pos, p_wk->cl_draw_type );
        }
      }
    }

    if( p_wk->cnt++ >= CARD_WAIT_SYNC )
    { 
      x = 0;
      p_wk->cnt = 0;
      ret = TRUE;
    }

    GFL_BG_SetScrollReq( p_wk->frm+1, GFL_BG_SCROLL_X_SET, x );
    GFL_BG_SetScrollReq( p_wk->frm, GFL_BG_SCROLL_X_SET, x );
    return ret;
  }
}
//=============================================================================
/**
 *					BMPWIN
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	BMPWINを作成し、文字表示	ランダムマッチ版
 *
 *	@param	PLAYERINFO_WORK * p_wk	ワーク
 *	@param	mode										モード
 *	@param	PLAYERINFO_RANDOMMATCH_DATA *cp_data	表示するためのデータ
 *	@param	MYSTATUS* p_my												プレイヤーのデータ
 *	@param	*p_font																フォント
 *	@param	*p_que																キュー
 *	@param	*p_msg																メッセージ
 *	@param	*p_word																単語登録
 *	@param	heapID																ヒープID
 */
//-----------------------------------------------------------------------------
static void PlayerInfo_Bmpwin_Rnd_Create( PLAYERINFO_WORK * p_wk, BOOL is_rate, const PLAYERINFO_RANDOMMATCH_DATA *cp_data, const MYSTATUS* p_my, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, BOOL is_black, HEAPID heapID )
{	
	enum
	{	
		PLAYERINFO_RND_BMPWIN_MODE,
		PLAYERINFO_RND_BMPWIN_RULE,
		PLAYERINFO_RND_BMPWIN_PLAYERSCORE,
		PLAYERINFO_RND_BMPWIN_RATE_LABEL,
		PLAYERINFO_RND_BMPWIN_RATE_NUM,
		PLAYERINFO_RND_BMPWIN_BTLCNT_LABEL,
		PLAYERINFO_RND_BMPWIN_BTLCNT_NUM,
		PLAYERINFO_RND_BMPWIN_WIN_LABEL,
		PLAYERINFO_RND_BMPWIN_WIN_NUM,
		PLAYERINFO_RND_BMPWIN_LOSE_LABEL,
		PLAYERINFO_RND_BMPWIN_LOSE_NUM,
		PLAYERINFO_RND_BMPWIN_MAX,
	};
	
	static const struct
	{	
		u8 x;
		u8 y;
		u8 w;
		u8 h;
	}sc_bmpwin_range[PLAYERINFO_RND_BMPWIN_MAX]	=
	{	
		//モード表示
		{	
			2,4,26,2,
		},
		//バトルルール
		{	
			9,7,21,2,
		},
		//○○の成績
		{	
			9,9,21,2,
		},
		//レーティング
		{	
			2,12,20,2,
		},
		//レーティングの数値
		{	
			25, 12, 5, 2,
		},
		//対戦回数
		{	
			2,15,20,2,
		},
		//対戦回数の数値
		{	
			25, 15, 5, 2,
		},
		//勝った数
		{	
			2,18,20,2,
		},
		//勝った数の数値
		{	
			25, 18, 5, 2,
		},
		//負けた数
		{	
			2,21,20,2,
		},
		//負けた数の数値
		{	
			25, 21, 5, 2,
		},
	};
	//BMPWIN登録個数
	p_wk->bmpwin_max	= PLAYERINFO_RND_BMPWIN_MAX;

	//BMPWIN作成
	{	
		int i;
		for( i = 0; i < p_wk->bmpwin_max; i++ )
		{	
			p_wk->p_bmpwin[i]	= GFL_BMPWIN_Create( PLAYERINFO_BG_FRAME, 
					sc_bmpwin_range[i].x, sc_bmpwin_range[i].y, sc_bmpwin_range[i].w, sc_bmpwin_range[i].h,
					PLAYERINFO_PLT_BG_FONT, GFL_BMP_CHRAREA_GET_B );

			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0 );

			GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin[i] );

			if( p_wk->p_bmpwin[i] )
			{	
				PRINT_UTIL_Setup( &p_wk->print_util[i], p_wk->p_bmpwin[i] );
			}
		}
	}

	//文字表示
	{	
		int i;
		STRBUF	*p_src;
		STRBUF	*p_str;
		BOOL is_print;
    PRINTSYS_LSB color;

		p_src	= GFL_STR_CreateBuffer( 128, heapID );
		p_str	= GFL_STR_CreateBuffer( 128, heapID );

		for( i = 0; i < PLAYERINFO_RND_BMPWIN_MAX; i++ )
		{	
      color = is_black == TRUE ? PLAYERINFO_STR_COLOR_WHITE:PLAYERINFO_STR_COLOR_BLACK;

			is_print	= TRUE;
			switch( i )
			{	
			case PLAYERINFO_RND_BMPWIN_MODE:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_000 + is_rate, p_str );
        color = PLAYERINFO_STR_COLOR_WHITE;
				break;
			case PLAYERINFO_RND_BMPWIN_RULE:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_002 + cp_data->btl_rule, p_str );
        color = PLAYERINFO_STR_COLOR_WHITE;
				break;
			case PLAYERINFO_RND_BMPWIN_PLAYERSCORE:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_007, p_src );
				WORDSET_RegisterPlayerName( p_word, 0, p_my );
				WORDSET_ExpandStr( p_word, p_str, p_src );
        color = PLAYERINFO_STR_COLOR_WHITE;
				break;
			case PLAYERINFO_RND_BMPWIN_RATE_LABEL:
				if( is_rate )
				{	
					GFL_MSG_GetString( p_msg, WIFIMATCH_STR_008, p_str );
				}
				else
				{	
					is_print	= FALSE;
				}
				break;
			case PLAYERINFO_RND_BMPWIN_RATE_NUM:
				if( is_rate )
				{	
					GFL_MSG_GetString( p_msg, WIFIMATCH_STR_009, p_src );
					WORDSET_RegisterNumber( p_word, 0, cp_data->rate, 4, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
					WORDSET_ExpandStr( p_word, p_str, p_src );
				}
				else
				{	
					is_print	= FALSE;
				}
				break;
			case PLAYERINFO_RND_BMPWIN_BTLCNT_LABEL:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_010, p_str );
				break;
			case PLAYERINFO_RND_BMPWIN_BTLCNT_NUM:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_011, p_src );
				WORDSET_RegisterNumber( p_word, 0, cp_data->btl_cnt, 5, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
				WORDSET_ExpandStr( p_word, p_str, p_src );
				break;
			case PLAYERINFO_RND_BMPWIN_WIN_LABEL:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_012, p_str );
				break;
			case PLAYERINFO_RND_BMPWIN_WIN_NUM:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_013, p_src );
				WORDSET_RegisterNumber( p_word, 0, cp_data->win_cnt, 5, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
				WORDSET_ExpandStr( p_word, p_str, p_src );
				break;
			case PLAYERINFO_RND_BMPWIN_LOSE_LABEL:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_014, p_str );
				break;
			case PLAYERINFO_RND_BMPWIN_LOSE_NUM:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_015, p_src );
				WORDSET_RegisterNumber( p_word, 0, cp_data->lose_cnt, 5, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
				WORDSET_ExpandStr( p_word, p_str, p_src );
				break;
			}
			if( is_print )
			{	
				PRINT_UTIL_PrintColor( &p_wk->print_util[i], p_que, 0, 0, p_str,  p_font, color );
			}

		}

		GFL_STR_DeleteBuffer( p_src );
		GFL_STR_DeleteBuffer( p_str );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	BMPWINを作成し、文字表示	WIFI大会版
 *
 *	@param	PLAYERINFO_WORK * p_wk	ワーク
 *	@param	mode										モード
 *	@param	PLAYERINFO_RANDOMMATCH_DATA *cp_data	表示するためのデータ
 *	@param	MYSTATUS* p_my												プレイヤーのデータ
 *	@param	*p_font																フォント
 *	@param	*p_que																キュー
 *	@param	*p_msg																メッセージ
 *	@param	*p_word																単語登録
 *	@param	heapID																ヒープID
 */
//-----------------------------------------------------------------------------
static void PlayerInfo_Bmpwin_Wifi_Create( PLAYERINFO_WORK * p_wk, BOOL is_limit, const PLAYERINFO_WIFICUP_DATA *cp_data, const MYSTATUS* p_my, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, HEAPID heapID )
{	
	enum
	{	
		PLAYERINFO_CUP_BMPWIN_CUPNAME,
		PLAYERINFO_CUP_BMPWIN_TIME,
		PLAYERINFO_CUP_BMPWIN_PLAYERNAME,
		PLAYERINFO_CUP_BMPWIN_RATE_LABEL,
		PLAYERINFO_CUP_BMPWIN_RATE_NUM,
		PLAYERINFO_CUP_BMPWIN_BTLCNT_LABEL,
		PLAYERINFO_CUP_BMPWIN_BTLCNT_NUM,
		PLAYERINFO_CUP_BMPWIN_REGPOKE,
		PLAYERINFO_CUP_BMPWIN_MAX,
	};
	
	static const struct
	{	
		u8 x;
		u8 y;
		u8 w;
		u8 h;
	}sc_bmpwin_range[PLAYERINFO_CUP_BMPWIN_MAX]	=
	{	
		//大会名表示
		{	
			2,1,28,4,
		},
		//開催日時
		{	
			2,6,28,2,
		},
		//プレイヤー名
		{	
			2,9,9,2,
		},
		//レーティング
		{	
			10,9,15,2,
		},
		//レーティングの数値
		{	
			25, 9, 5, 2,
		},
		//対戦回数
		{	
			10,12,15,2,
		},
		//対戦回数の数値
		{	
			25, 12, 5, 2,
		},
		//登録ポケモン
		{	
			5,18,28,2,
		},
	};


	//BMPWIN登録個数
	p_wk->bmpwin_max	= PLAYERINFO_CUP_BMPWIN_MAX;

	//BMPWIN作成
	{	
		int i;
		for( i = 0; i < p_wk->bmpwin_max; i++ )
		{	
			p_wk->p_bmpwin[i]	= GFL_BMPWIN_Create( p_wk->frm, 
					sc_bmpwin_range[i].x, sc_bmpwin_range[i].y, sc_bmpwin_range[i].w, sc_bmpwin_range[i].h,
					p_wk->plt, GFL_BMP_CHRAREA_GET_B );
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0 );
			GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin[i] );

			if( p_wk->p_bmpwin[i] )
			{	
				PRINT_UTIL_Setup( &p_wk->print_util[i], p_wk->p_bmpwin[i] );
			}
		}
	}

	//文字表示
	{	
		int i;
		STRBUF	*p_src;
		STRBUF	*p_str;
		BOOL	is_print;
    PRINTSYS_LSB color;

		p_src	= GFL_STR_CreateBuffer( 128, heapID );
		p_str	= GFL_STR_CreateBuffer( 128, heapID );

		for( i = 0; i < PLAYERINFO_CUP_BMPWIN_MAX; i++ )
		{	
      color = PLAYERINFO_STR_COLOR_BLACK;

			is_print	= TRUE;
			switch( i )
			{	
			case PLAYERINFO_CUP_BMPWIN_CUPNAME:
				GFL_STR_SetStringCode( p_str, cp_data->cup_name );
        color = PLAYERINFO_STR_COLOR_WHITE;
				break;
			case PLAYERINFO_CUP_BMPWIN_TIME:
				{	
					u16 s_y;
					u8 s_m, s_d;
					u16 e_y;
					u8 e_m, e_d;

					s_y	= 2000 + GFDATE_GetYear( cp_data->start_date );
					s_m	= GFDATE_GetMonth( cp_data->start_date );
					s_d	= GFDATE_GetDay( cp_data->start_date );
					e_y	= 2000 + GFDATE_GetYear( cp_data->end_date );
					e_m	= GFDATE_GetMonth( cp_data->end_date );
					e_d	= GFDATE_GetDay( cp_data->end_date );

					GFL_MSG_GetString( p_msg, WIFIMATCH_STR_016, p_src );
					WORDSET_RegisterNumber( p_word, 0, s_y, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_RegisterNumber( p_word, 1, s_m, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_RegisterNumber( p_word, 2, s_d, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_RegisterNumber( p_word, 3, e_y, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_RegisterNumber( p_word, 4, e_m, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_RegisterNumber( p_word, 5, e_d, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_ExpandStr( p_word, p_str, p_src );
          color = PLAYERINFO_STR_COLOR_WHITE;
				}
				break;
			case PLAYERINFO_CUP_BMPWIN_PLAYERNAME:
				MyStatus_CopyNameString( p_my, p_str );
        color = PLAYERINFO_STR_COLOR_WHITE;
				break;
			case PLAYERINFO_CUP_BMPWIN_RATE_LABEL:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_008, p_str );
				break;
			case PLAYERINFO_CUP_BMPWIN_RATE_NUM:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_009, p_src );
				WORDSET_RegisterNumber( p_word, 0, cp_data->rate, 4, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
				WORDSET_ExpandStr( p_word, p_str, p_src );
				break;
			case PLAYERINFO_CUP_BMPWIN_BTLCNT_LABEL:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_010, p_str );
				break;
			case PLAYERINFO_CUP_BMPWIN_BTLCNT_NUM:
				if( is_limit )
				{	
					PRINTTOOL_PrintFraction(
					&p_wk->print_util[i], p_que, p_font,
					sc_bmpwin_range[i].w*4, 0, cp_data->btl_cnt, cp_data->btl_max, heapID );
					is_print =FALSE;
				}
				else
				{	
					GFL_MSG_GetString( p_msg, WIFIMATCH_STR_011, p_src );
					WORDSET_RegisterNumber( p_word, 0, cp_data->btl_cnt, 5, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
					WORDSET_ExpandStr( p_word, p_str, p_src );
				}
				break;
			case PLAYERINFO_CUP_BMPWIN_REGPOKE:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_017, p_str );
        color = PLAYERINFO_STR_COLOR_WHITE;
				break;
			}
			if( is_print )
			{	
				PRINT_UTIL_PrintColor( &p_wk->print_util[i], p_que, 0, 0, p_str,  p_font, color );
			}
		}

		GFL_STR_DeleteBuffer( p_src );
		GFL_STR_DeleteBuffer( p_str );
	}

}

//----------------------------------------------------------------------------
/**
 *	@brief	BMPWINを作成し、文字表示	ライブ大会版
 *
 *	@param	PLAYERINFO_WORK * p_wk	ワーク
 *	@param	mode										モード
 *	@param	PLAYERINFO_RANDOMMATCH_DATA *cp_data	表示するためのデータ
 *	@param	MYSTATUS* p_my												プレイヤーのデータ
 *	@param	*p_font																フォント
 *	@param	*p_que																キュー
 *	@param	*p_msg																メッセージ
 *	@param	*p_word																単語登録
 *	@param	heapID																ヒープID
 */
//-----------------------------------------------------------------------------
static void PlayerInfo_Bmpwin_Live_Create( PLAYERINFO_WORK * p_wk, const PLAYERINFO_LIVECUP_DATA *cp_data,const MYSTATUS* p_my, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, HEAPID heapID )
{	
	enum
	{	
		PLAYERINFO_LIVE_BMPWIN_CUPNAME,
		PLAYERINFO_LIVE_BMPWIN_TIME,
		PLAYERINFO_LIVE_BMPWIN_PLAYERNAME,
		PLAYERINFO_LIVE_BMPWIN_WIN_LABEL,
		PLAYERINFO_LIVE_BMPWIN_WIN_NUM,
		PLAYERINFO_LIVE_BMPWIN_LOSE_LABEL,
		PLAYERINFO_LIVE_BMPWIN_LOSE_NUM,
		PLAYERINFO_LIVE_BMPWIN_BTLCNT_LABEL,
		PLAYERINFO_LIVE_BMPWIN_BTLCNT_NUM,
		PLAYERINFO_LIVE_BMPWIN_REGPOKE,
		PLAYERINFO_LIVE_BMPWIN_MAX,
	};
	
	static const struct
	{	
		u8 x;
		u8 y;
		u8 w;
		u8 h;
		u32 mode;	//表示モード
	}sc_bmpwin_range[PLAYERINFO_LIVE_BMPWIN_MAX]	=
	{	
		//大会名表示
		{	
			1,1,30,4,
		},
		//開催日時
		{	
			1,6,30,2,
		},
		//プレイヤー名
		{
			1,9,9,2,
		},
		//勝った数
		{
			11,9,12,2,
		},
		//勝った数の数値
		{	
			25,9, 6, 2,
		},
		//負けた数
		{	
			11,12,12,2,
		},
		//負けた数の数値
		{	
			25,12, 6, 2,
		},
		//対戦回数
		{	
			11,15,12,2,
		},
		//対戦回数の数値
		{	
			25,15,6, 2,
		},
		//登録ポケモン
		{	
			1,18,30,2,
		},
	};
	//BMPWIN登録個数
	p_wk->bmpwin_max	= PLAYERINFO_LIVE_BMPWIN_MAX;

	//BMPWIN作成
	{	
		int i;
		for( i = 0; i < p_wk->bmpwin_max; i++ )
		{	
			p_wk->p_bmpwin[i]	= GFL_BMPWIN_Create( PLAYERINFO_BG_FRAME, 
					sc_bmpwin_range[i].x, sc_bmpwin_range[i].y, sc_bmpwin_range[i].w, sc_bmpwin_range[i].h,
					PLAYERINFO_PLT_BG_FONT, GFL_BMP_CHRAREA_GET_B );
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0xE );
			GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin[i] );

			if( p_wk->p_bmpwin[i] )
			{	
				PRINT_UTIL_Setup( &p_wk->print_util[i], p_wk->p_bmpwin[i] );
			}
		}
	}

	//文字表示
	{	
		int i;
		STRBUF	*p_src;
		STRBUF	*p_str;

		BOOL	is_print;

		p_src	= GFL_STR_CreateBuffer( 128, heapID );
		p_str	= GFL_STR_CreateBuffer( 128, heapID );

		for( i = 0; i < PLAYERINFO_LIVE_BMPWIN_MAX; i++ )
		{	
			is_print	= TRUE;
			switch( i )
			{	
			case PLAYERINFO_LIVE_BMPWIN_CUPNAME:
				GFL_STR_SetStringCode( p_str, cp_data->cup_name );
				break;
			case PLAYERINFO_LIVE_BMPWIN_TIME:
				{	
					u16 s_y;
					u8 s_m, s_d;
					u16 e_y;
					u8 e_m, e_d;

					s_y	= 2000 + GFDATE_GetYear( cp_data->start_date );
					s_m	= GFDATE_GetMonth( cp_data->start_date );
					s_d	= GFDATE_GetDay( cp_data->start_date );
					e_y	= 2000 + GFDATE_GetYear( cp_data->end_date );
					e_m	= GFDATE_GetMonth( cp_data->end_date );
					e_d	= GFDATE_GetDay( cp_data->end_date );

					GFL_MSG_GetString( p_msg, WIFIMATCH_STR_016, p_src );
					WORDSET_RegisterNumber( p_word, 0, s_y, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_RegisterNumber( p_word, 1, s_m, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_RegisterNumber( p_word, 2, s_d, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_RegisterNumber( p_word, 3, e_y, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_RegisterNumber( p_word, 4, e_m, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_RegisterNumber( p_word, 5, e_d, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_ExpandStr( p_word, p_str, p_src );
				}
				break;
			case PLAYERINFO_LIVE_BMPWIN_PLAYERNAME:
				//MyStatus_CopyNameString( p_my, p_str );
				break;
			case PLAYERINFO_LIVE_BMPWIN_WIN_LABEL:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_012, p_str );
				break;
			case PLAYERINFO_LIVE_BMPWIN_WIN_NUM:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_013, p_src );
				WORDSET_RegisterNumber( p_word, 0, cp_data->win_cnt, 5, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
				WORDSET_ExpandStr( p_word, p_str, p_src );
				break;
			case PLAYERINFO_LIVE_BMPWIN_LOSE_LABEL:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_014, p_str );
				break;
			case PLAYERINFO_LIVE_BMPWIN_LOSE_NUM:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_015, p_src );
				WORDSET_RegisterNumber( p_word, 0, cp_data->lose_cnt, 5, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
				WORDSET_ExpandStr( p_word, p_str, p_src );
				break;
			case PLAYERINFO_LIVE_BMPWIN_BTLCNT_LABEL:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_010, p_str );
				break;
			case PLAYERINFO_LIVE_BMPWIN_BTLCNT_NUM:
				PRINTTOOL_PrintFraction(
						&p_wk->print_util[i], p_que, p_font,
						sc_bmpwin_range[i].w*4, 0, cp_data->btl_cnt, cp_data->btl_max, heapID );
				is_print =FALSE;
				break;
			case PLAYERINFO_LIVE_BMPWIN_REGPOKE:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_017, p_str );
				break;
			}
			if( is_print )
			{	
			PRINT_UTIL_Print( &p_wk->print_util[i], p_que, 0, 0, p_str,  p_font );
			}
		}

		GFL_STR_DeleteBuffer( p_src );
		GFL_STR_DeleteBuffer( p_str );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	プリントメイン
 *
 *	@param	PLAYERINFO_WORK * p_wk	ワーク
 *	@param	PRINT_QUE	*p_que				プリントキュー
 *
 *	@return	TRUEで全てのプリント終了	FALSEでまだ
 */
//-----------------------------------------------------------------------------
static BOOL PlayerInfo_Bmpwin_PrintMain( PLAYERINFO_WORK * p_wk, PRINT_QUE *p_que )
{	
	int i;
	BOOL ret;
	ret	= TRUE;
	for( i = 0; i < p_wk->bmpwin_max; i++ )
	{	
		if( p_wk->p_bmpwin[i] )
		{	
			ret	&= PRINT_UTIL_Trans( &p_wk->print_util[i], p_que );
		}
	}

	return ret;
}
//----------------------------------------------------------------------------
/**
 *	@brief	BMPWIN破棄
 *
 *	@param	PLAYERINFO_WORK * p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void PlayerInfo_Bmpwin_Delete( PLAYERINFO_WORK * p_wk )
{	
	int i;
	for( i = 0; i < p_wk->bmpwin_max; i++ )
	{	
		if( p_wk->p_bmpwin[i] )
		{	
			GFL_BMPWIN_Delete( p_wk->p_bmpwin[i] );
		}
	}
}
//=============================================================================
/**
 *					CLWK
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ポケアイコン読みこみ
 *
 *	@param	PLAYERINFO_WORK * p_wk	ワーク
 *	@param	mode					モード
 *	@param	u16 *cp_poke	モンスター番号が入った配列6個分
 *	@param	u16 *cp_form	フォルム配列6個分
 *	@param	*p_unit				CLWK読みこみ用
 *	@param	heapID				ヒープＩＤ
 */
//-----------------------------------------------------------------------------
static void PlayerInfo_POKEICON_Create( PLAYERINFO_WORK * p_wk, GFL_CLUNIT *p_unit, BATTLE_BOX_SAVE * p_btlbox_sv, HEAPID heapID )
{	
  POKEPARTY       *p_party;
  POKEMON_PARAM   *p_pp;

  p_party = BATTLE_BOX_SAVE_MakePokeParty( p_btlbox_sv, GFL_HEAP_LOWID(heapID) );
  p_wk->clwk_max	= PokeParty_GetPokeCount( p_party );

	//リソース読みこみ
	{	
		int i;
		ARCHANDLE	*p_handle;

		p_handle	= GFL_ARC_OpenDataHandle( ARCID_POKEICON, heapID );

		p_wk->res[ PLAYERINFO_RESID_POKEICON_PLT ]	= GFL_CLGRP_PLTT_RegisterComp( p_handle,
        POKEICON_GetPalArcIndex(), p_wk->cl_draw_type, p_wk->pokemonicon_plt*0x20, GFL_HEAP_LOWID(heapID) );
    p_wk->res[ PLAYERINFO_RESID_POKEICON_CELL ]	= GFL_CLGRP_CELLANIM_Register( 
						p_handle, POKEICON_GetCellArcIndex(), POKEICON_GetAnmArcIndex(), heapID ); 

		for( i = PLAYERINFO_CLWK_POKEICON_TOP; i < p_wk->clwk_max; i++ )
		{
      p_pp  = PokeParty_GetMemberPointer( p_party, i );

				p_wk->res[ PLAYERINFO_RESID_POKEICON_CGR_TOP + i ]		= GFL_CLGRP_CGR_Register( 
						p_handle, POKEICON_GetCgxArcIndex( PP_GetPPPPointerConst(p_pp) ),
						FALSE, p_wk->cl_draw_type, GFL_HEAP_LOWID(heapID) ); 
		}
		GFL_ARC_CloseDataHandle( p_handle );
	}

	//CLWK作成
	{	
		GFL_CLWK_DATA	clwk_data;
		int i;

		GFL_STD_MemClear( &clwk_data, sizeof(GFL_CLWK_DATA) );
		clwk_data.pos_y		= PLAYERINFO_POKEICON_Y;
		clwk_data.anmseq	= POKEICON_ANM_HPMAX;
    clwk_data.bgpri   = 1;
		for( i = 0; i < p_wk->clwk_max; i++ )
		{	
      p_pp  = PokeParty_GetMemberPointer( p_party, i );

			clwk_data.pos_x	= WBM_CARD_INIT_POS_X + PLAYERINFO_POKEICON_START_X + PLAYERINFO_POKEICON_DIFF_X*i;
			p_wk->p_clwk[i]	= GFL_CLACT_WK_Create( p_unit,
					p_wk->res[ PLAYERINFO_RESID_POKEICON_CGR_TOP + i ],
					p_wk->res[ PLAYERINFO_RESID_POKEICON_PLT ],
					p_wk->res[ PLAYERINFO_RESID_POKEICON_CELL ],
					&clwk_data,
					p_wk->cl_draw_type,
					heapID );

			GFL_CLACT_WK_SetPlttOffs( p_wk->p_clwk[i],
					POKEICON_GetPalNumGetByPPP( PP_GetPPPPointerConst(p_pp) ), 
					CLWK_PLTTOFFS_MODE_OAM_COLOR );
		}
	}

  GFL_HEAP_FreeMemory( p_party );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンアイコン破棄
 *
 *	@param	PLAYERINFO_WORK * p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void PlayerInfo_POKEICON_Delete( PLAYERINFO_WORK * p_wk )
{	
	
	//CLWK破棄
	{	
		int i;
		for( i = 0; i < p_wk->clwk_max; i++ )
		{	
			GFL_CLACT_WK_Remove( p_wk->p_clwk[i] );
		}
	}

	//リソース破棄
	{	
		int i;


		for( i = 0; i < p_wk->clwk_max; i++ )
		{	
			GFL_CLGRP_CGR_Release( p_wk->res[ PLAYERINFO_RESID_POKEICON_CGR_TOP + i ] );
		}	

    GFL_CLGRP_CELLANIM_Release( p_wk->res[ PLAYERINFO_RESID_POKEICON_CELL ] );
		GFL_CLGRP_PLTT_Release( p_wk->res[ PLAYERINFO_RESID_POKEICON_PLT ] );
	
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	自分の見た目を作成
 *
 *	@param	PLAYERINFO_WORK * p_wk	ワーク
 *	@param	trainerID		見た目
 *	@param	*p_unit			ユニット
 *	@param	heapID			ヒープID
 */
//-----------------------------------------------------------------------------
static void PlayerInfo_TRAINER_Cleate( PLAYERINFO_WORK * p_wk, u32 trainerID, GFL_CLUNIT *p_unit, HEAPID heapID )
{	
	//リソース読みこみ
	{	
		ARCHANDLE	*p_handle;

		p_handle	= GFL_ARC_OpenDataHandle( ARCID_WIFIUNIONCHAR, heapID );

		p_wk->res[ PLAYERINFO_RESID_TRAINER_PLT ]	= GFL_CLGRP_PLTT_RegisterEx( p_handle,
        WBM_CARD_TRAINER_RES_NCLR, p_wk->cl_draw_type, PLAYERINFO_PLT_OBJ_TRAINER*0x20,
        WBM_CARD_TRAINER_RES_PLT_OFS(trainerID), 1, heapID );

		p_wk->res[ PLAYERINFO_RESID_TRAINER_CGR ]		= GFL_CLGRP_CGR_Register( 
				p_handle, WBM_CARD_TRAINER_RES_NCGR(trainerID),
				FALSE, p_wk->cl_draw_type, heapID ); 

		p_wk->res[ PLAYERINFO_RESID_TRAINER_CELL ]	= GFL_CLGRP_CELLANIM_Register( 
				p_handle, WBM_CARD_TRAINER_RES_NCER, 
				WBM_CARD_TRAINER_RES_NANR, heapID ); 

		GFL_ARC_CloseDataHandle( p_handle );
	}


	//CLWK
	{	
		GFL_CLWK_DATA	clwk_data;

		GFL_STD_MemClear( &clwk_data, sizeof(GFL_CLWK_DATA) );
		clwk_data.pos_y		= 0;
		clwk_data.pos_x		= 0;
    clwk_data.bgpri   = 1;
		p_wk->p_clwk[PLAYERINFO_CLWK_TRAINER]	= GFL_CLACT_WK_Create( p_unit,
				p_wk->res[ PLAYERINFO_RESID_TRAINER_CGR ],
				p_wk->res[ PLAYERINFO_RESID_TRAINER_PLT ],
				p_wk->res[ PLAYERINFO_RESID_TRAINER_CELL ],
				&clwk_data,
				p_wk->cl_draw_type,
				heapID );

    GFL_CLACT_WK_SetPlttOffs( p_wk->p_clwk[PLAYERINFO_CLWK_TRAINER], 0, CLWK_PLTTOFFS_MODE_PLTT_TOP );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	自分の見た目を破棄
 *
 *	@param	PLAYERINFO_WORK * p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void PlayerInfo_TRAINER_Delete( PLAYERINFO_WORK * p_wk )
{	
	//CLWK破棄
	{	
		GFL_CLACT_WK_Remove( p_wk->p_clwk[PLAYERINFO_CLWK_TRAINER] );
	}

	//リソース破棄
	{	
		GFL_CLGRP_CGR_Release( p_wk->res[ PLAYERINFO_RESID_TRAINER_CGR ] );
		GFL_CLGRP_CELLANIM_Release( p_wk->res[ PLAYERINFO_RESID_TRAINER_CELL ] );
		GFL_CLGRP_PLTT_Release( p_wk->res[ PLAYERINFO_RESID_TRAINER_PLT ] );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief  鍵作成
 *
 *	@param	PLAYERINFO_WORK * p_wk  ワーク
 *	@param	*p_unit                 OBJ作成用ユニット
 *	@param	WIFIBATTLEMATCH_VIEW_RESOURCE *cp_res リソース
 *	@param	heapID                  ヒープID
 */
//-----------------------------------------------------------------------------
static void PlayerInfo_LOCK_Cleate( PLAYERINFO_WORK * p_wk, GFL_CLUNIT *p_unit, const WIFIBATTLEMATCH_VIEW_RESOURCE *cp_res, HEAPID heapID )
{ 
	//CLWK
  GFL_CLWK_DATA	clwk_data;

  GFL_STD_MemClear( &clwk_data, sizeof(GFL_CLWK_DATA) );
  clwk_data.pos_x		= WBM_CARD_LOCK_POS_X + WBM_CARD_INIT_POS_X;
  clwk_data.pos_y		= WBM_CARD_LOCK_POS_Y;
  clwk_data.anmseq	= 7;
  clwk_data.bgpri   = 1;

  p_wk->p_clwk[PLAYERINFO_CLWK_LOCK]	= GFL_CLACT_WK_Create( p_unit,
      WIFIBATTLEMATCH_VIEW_GetResource( cp_res, WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_CHR_M + p_wk->cl_draw_type ),
      WIFIBATTLEMATCH_VIEW_GetResource( cp_res, WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_PLT_M + p_wk->cl_draw_type ),
      WIFIBATTLEMATCH_VIEW_GetResource( cp_res, WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_CEL_M + p_wk->cl_draw_type ),
      &clwk_data,
      p_wk->cl_draw_type,
      heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief  鍵破棄
 *
 *	@param	PLAYERINFO_WORK * p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void PlayerInfo_LOCK_Delete( PLAYERINFO_WORK * p_wk )
{ 
	//CLWK破棄
	{	
		GFL_CLACT_WK_Remove( p_wk->p_clwk[PLAYERINFO_CLWK_LOCK] );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief  星を作成
 *
 *	@param	PLAYERINFO_WORK * p_wk    ワーク
 *	@param	PLAYERINFO_RANDOMMATCH_DATA *cp_data  データ
 *	@param	*p_unit OBJ作成用ユニット
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
static void PlayerInfo_STAR_Cleate( PLAYERINFO_WORK * p_wk, const PLAYERINFO_RANDOMMATCH_DATA *cp_data, GFL_CLUNIT *p_unit, const WIFIBATTLEMATCH_VIEW_RESOURCE *cp_res, HEAPID heapID )
{ 
  u32 star_num;

  //星の計算
  star_num  = CalcRank( cp_data->btl_cnt, cp_data->win_cnt, cp_data->lose_cnt );

	//CLWK
	{	
    int i;
		GFL_CLWK_DATA	clwk_data;

		GFL_STD_MemClear( &clwk_data, sizeof(GFL_CLWK_DATA) );
		clwk_data.pos_x		= WBM_CARD_INIT_POS_X;
		clwk_data.pos_y		= 16;

    for( i = 0; i < star_num; i++ )
    { 
      clwk_data.pos_x	  	+=  16;
      clwk_data.anmseq		=   2+i;
      p_wk->p_clwk[PLAYERINFO_CLWK_STAR_TOP + i]	= GFL_CLACT_WK_Create( p_unit,
				WIFIBATTLEMATCH_VIEW_GetResource( cp_res, WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_CHR_S ),
				WIFIBATTLEMATCH_VIEW_GetResource( cp_res, WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_PLT_S ),
				WIFIBATTLEMATCH_VIEW_GetResource( cp_res, WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_CEL_S ),
				&clwk_data,
				CLSYS_DRAW_SUB,
				heapID );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  星を破棄
 *
 *	@param	PLAYERINFO_WORK * p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void PlayerInfo_STAR_Delete( PLAYERINFO_WORK * p_wk )
{   
  int i;
  for( i = PLAYERINFO_CLWK_STAR_TOP; i < PLAYERINFO_CLWK_STAR_END; i++ )
  { 
    if( p_wk->p_clwk[i] )
    { 
      GFL_CLACT_WK_Remove( p_wk->p_clwk[i] );
      p_wk->p_clwk[i] = NULL;
    }
  }
}

#ifdef DEBUG_DATA_CREATE
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグ用データを作成
 *
 *	@param	WIFIBATTLEMATCH_MODE mode	モード
 *	@param	*p_data										受け取りワーク
 */
//-----------------------------------------------------------------------------
void PLAYERINFO_DEBUG_CreateRndData( WIFIBATTLEMATCH_MODE mode, void *p_data_adrs )
{	
		{	
			PLAYERINFO_RANDOMMATCH_DATA	*p_data	= p_data_adrs;
			GFL_STD_MemClear( p_data, sizeof(PLAYERINFO_RANDOMMATCH_DATA) );
			p_data->btl_rule	= 0;
			p_data->rate			= 1234;
			p_data->btl_cnt		= 99;
			p_data->win_cnt		= 70;
			p_data->lose_cnt	= 29;	
		}
}
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグ用データを作成
 *
 *	@param	WIFIBATTLEMATCH_MODE mode	モード
 *	@param	*p_data										受け取りワーク
 */
//-----------------------------------------------------------------------------
void PLAYERINFO_DEBUG_CreateWifiData( WIFIBATTLEMATCH_MODE mode, void *p_data_adrs )
{ 
		{	
			PLAYERINFO_WIFICUP_DATA	*p_data	= p_data_adrs;
			GFL_STD_MemClear( p_data, sizeof(PLAYERINFO_WIFICUP_DATA) );
			p_data->cup_name[0]	= L'デ';
			p_data->cup_name[1]	= L'バ';
			p_data->cup_name[2]	= L'ッ';
			p_data->cup_name[3]	= L'グ';
			p_data->cup_name[4]	= L'W';
			p_data->cup_name[5]	= L'i';
			p_data->cup_name[6]	= L'-';
			p_data->cup_name[7]	= L'F';
			p_data->cup_name[8]	= L'i';
			p_data->cup_name[9]	= L'た';
			p_data->cup_name[10]	= L'い';
			p_data->cup_name[11]	= L'か';
			p_data->cup_name[12]	= L'い';
			p_data->cup_name[13]	= GFL_STR_GetEOMCode();
			p_data->start_date	= GFDATE_Set( 10, 11, 12, 0);
			p_data->end_date		= GFDATE_Set( 10, 12, 24, 0);
			p_data->rate				= 53;
			p_data->btl_cnt			= 2;
			p_data->btl_max			= 15;
		}

}
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグ用データを作成
 *
 *	@param	WIFIBATTLEMATCH_MODE mode	モード
 *	@param	*p_data										受け取りワーク
 */
//-----------------------------------------------------------------------------
void PLAYERINFO_DEBUG_CreateLiveData( WIFIBATTLEMATCH_MODE mode, void *p_data_adrs )
{ 
		{	
			PLAYERINFO_LIVECUP_DATA	*p_data	= p_data_adrs;
			GFL_STD_MemClear( p_data, sizeof(PLAYERINFO_LIVECUP_DATA) );
			p_data->cup_name[0]	= L'デ';
			p_data->cup_name[1]	= L'バ';
			p_data->cup_name[2]	= L'ッ';
			p_data->cup_name[3]	= L'グ';
			p_data->cup_name[4]	= L'ラ';
			p_data->cup_name[5]	= L'イ';
			p_data->cup_name[6]	= L'ブ';
			p_data->cup_name[7]	= L'た';
			p_data->cup_name[8]	= L'い';
			p_data->cup_name[9]	= L'か';
			p_data->cup_name[10]	= L'い';
			p_data->cup_name[11]	= GFL_STR_GetEOMCode();
			p_data->start_date	= GFDATE_Set( 11, 11, 12, 0);
			p_data->end_date		= GFDATE_Set( 11, 12, 24, 0);
			p_data->win_cnt			= 20;
			p_data->lose_cnt		= 17;
			p_data->btl_cnt			= 2;
			p_data->btl_max			= 15;
		}

}

#endif


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					対戦者情報表示
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
enum
{	
	MATCHINFO_BMPWIN_PLAYER,
	MATCHINFO_BMPWIN_RATE_LABEL,
	MATCHINFO_BMPWIN_RATE_NUM,
	MATCHINFO_BMPWIN_COMEFROM,
	MATCHINFO_BMPWIN_CONTRY,
	MATCHINFO_BMPWIN_PLACE,
	MATCHINFO_BMPWIN_GREET,
	MATCHINFO_BMPWIN_PMS,
	MATCHINFO_BMPWIN_MAX,
};
enum
{	
	MATCHINFO_RESID_TRAINER_PLT,
	MATCHINFO_RESID_TRAINER_CGR,
	MATCHINFO_RESID_TRAINER_CELL,
	MATCHINFO_RESID_MAX,
};
//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	対戦者情報表示ワーク
//=====================================
struct _MATCHINFO_WORK
{	
	GFL_BMPWIN		*p_bmpwin[ MATCHINFO_BMPWIN_MAX ];
	PRINT_UTIL		print_util[ MATCHINFO_BMPWIN_MAX ];
	GFL_CLWK			*p_clwk;
	GFL_CLWK			*p_star[ PLAYERINFO_STAR_MAX ];
	PMS_DRAW_WORK	*p_pms;
	u16 res[MATCHINFO_RESID_MAX];
  u32 cnt;

  u16 clwk_x;
  u16 star_x[PLAYERINFO_STAR_MAX];
};
//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	BMPWIN
//=====================================
static void MatchInfo_Bmpwin_Create( MATCHINFO_WORK * p_wk, const WIFIBATTLEMATCH_ENEMYDATA *cp_data, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, BOOL is_black, HEAPID heapID );
static BOOL MatchInfo_Bmpwin_PrintMain( MATCHINFO_WORK * p_wk, PRINT_QUE *p_que );
static void MatchInfo_Bmpwin_Delete( MATCHINFO_WORK * p_wk );
//-------------------------------------
///	CLWK
//=====================================
static void MatchInfo_TRAINER_Create( MATCHINFO_WORK * p_wk, u32 trainerID, GFL_CLUNIT *p_unit, HEAPID heapID );
static void MatchInfo_TRAINER_Delete( MATCHINFO_WORK * p_wk );

static void MatchInfo_STAR_Cleate( MATCHINFO_WORK * p_wk, const WIFIBATTLEMATCH_ENEMYDATA *cp_data, GFL_CLUNIT *p_unit, const WIFIBATTLEMATCH_VIEW_RESOURCE *cp_res, HEAPID heapID );
static void MatchInfo_STAR_Delete( MATCHINFO_WORK * p_wk );

//=============================================================================
/**
 *					データ
*/
//=============================================================================

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	対戦者情報	作成
 *
 *	@param	const MATCHINFO_DATA *cp_data	データ
 *	@param	PRINT_QUE *p_que							キュー
 *	@param	*p_msg												メッセージ
 *	@param	*p_word												単語登録
 *	@param	heapID												ヒープID
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
MATCHINFO_WORK * MATCHINFO_Init( const WIFIBATTLEMATCH_ENEMYDATA *cp_data, GFL_CLUNIT *p_unit, const WIFIBATTLEMATCH_VIEW_RESOURCE *cp_res, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, WIFIBATTLEMATCH_MODE mode, HEAPID heapID )
{	
	MATCHINFO_WORK	*	p_wk;
  WBM_CARD_RANK rank;
	p_wk	= GFL_HEAP_AllocMemory( heapID, sizeof(MATCHINFO_WORK) );
	GFL_STD_MemClear( p_wk, sizeof(MATCHINFO_WORK) );

	p_wk->p_pms	= PMS_DRAW_Init( p_unit, CLSYS_DRAW_MAIN, p_que, p_font, MATCHINFO_PLT_OBJ_PMS, 1, heapID );
  PMS_DRAW_SetNullColorPallet( p_wk->p_pms, 0 );
	
  rank = CalcRank( cp_data->btl_cnt, cp_data->win_cnt, cp_data->lose_cnt );
	MatchInfo_Bmpwin_Create( p_wk, cp_data, p_font, p_que, p_msg, p_word, rank == WBM_CARD_RANK_BLACK, heapID );
	MatchInfo_TRAINER_Create( p_wk, cp_data->trainer_view, p_unit, heapID );
  MatchInfo_STAR_Cleate( p_wk, cp_data, p_unit, cp_res, heapID );



  {
    GFL_ARC_UTIL_TransVramScreen( ARCID_WIFIMATCH_GRA, NARC_wifimatch_gra_standby_card_NSCR,
				BG_FRAME_M_CARD, 0, 0, FALSE, GFL_HEAP_LOWID( heapID ) );

    switch( mode )
    { 
    case WIFIBATTLEMATCH_MODE_WIFI:
      /* fallthr */
    case WIFIBATTLEMATCH_MODE_LIVE:
      GFL_BG_WriteScreenExpand( BG_FRAME_M_CARD, 0, 0, 32, 2,
        GFL_BG_GetScreenBufferAdrs(BG_FRAME_M_CARD), 0, 24, 32, 32 );
    }

    GFL_BG_ChangeScreenPalette( BG_FRAME_M_CARD, 0, 0, 32, 2, 1 );
    GFL_BG_ChangeScreenPalette( BG_FRAME_M_CARD, 0, 3, 32, 32, 3 + rank );
    GFL_BG_ChangeScreenPalette( BG_FRAME_M_CARD, 0, 3, 1, 2, 1 );
    GFL_BG_ChangeScreenPalette( BG_FRAME_M_CARD, 31, 3, 1, 2, 1 );
    GFL_BG_LoadScreenReq( BG_FRAME_M_CARD );
  }

  GFL_BG_SetScroll( BG_FRAME_M_CARD, GFL_BG_SCROLL_X_SET, -WBM_CARD_INIT_POS_X );
  GFL_BG_SetScroll( BG_FRAME_M_FONT, GFL_BG_SCROLL_X_SET, -WBM_CARD_INIT_POS_X );

  G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG3, PLAYERINFO_ALPHA_EV1, PLAYERINFO_ALPHA_EV2);
	return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	対戦者情報破棄
 *
 *	@param	MATCHINFO_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void MATCHINFO_Exit( MATCHINFO_WORK *p_wk )
{	
  G2_BlendNone();

  MatchInfo_STAR_Delete( p_wk );
	MatchInfo_TRAINER_Delete( p_wk );
	MatchInfo_Bmpwin_Delete( p_wk );

	PMS_DRAW_Exit( p_wk->p_pms );

	GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	対戦者情報	プリント
 *
 *	@param	MATCHINFO_WORK *p_wk	ワーク
 *	@param	*p_que								キュー
 *
 *	@return	TRUEでプリント終了	FALSEで処理中
 */
//-----------------------------------------------------------------------------
BOOL MATCHINFO_PrintMain( MATCHINFO_WORK *p_wk, PRINT_QUE *p_que )
{	
	return MatchInfo_Bmpwin_PrintMain( p_wk, p_que );
}

//----------------------------------------------------------------------------
/**
 *	@brief  対戦者情報  カード動作
 *
 *	@param	MATCHINFO_WORK * p_wk ワーク
 *
 *	@return TRUE  FALSE
 */
//-----------------------------------------------------------------------------
BOOL MATCHINFO_MoveMain( MATCHINFO_WORK * p_wk )
{ 
  BOOL ret  = FALSE;
  
  s32 x;
  
  if( p_wk->cnt == 0 )
  { 
    int i;
    GFL_CLACTPOS  pos;

    if( p_wk->p_clwk )
    { 
      GFL_CLACT_WK_GetPos( p_wk->p_clwk, &pos, CLSYS_DRAW_MAIN );
      p_wk->clwk_x  = pos.x;
    }
    for( i = 0; i < PLAYERINFO_STAR_MAX; i++ )
    { 
      if( p_wk->p_star[i] )
      { 
        GFL_CLACT_WK_GetPos( p_wk->p_star[i], &pos, CLSYS_DRAW_MAIN );
        p_wk->star_x[i] = pos.x;
      }
    }
  }


  x = -WBM_CARD_INIT_POS_X + WBM_CARD_INIT_POS_X * p_wk->cnt / CARD_WAIT_SYNC;

  { 
    int i;
    GFL_CLACTPOS  pos;
    if( p_wk->p_clwk )
    { 
      GFL_CLACT_WK_GetPos( p_wk->p_clwk, &pos, CLSYS_DRAW_MAIN );
      pos.x = p_wk->clwk_x - WBM_CARD_INIT_POS_X * p_wk->cnt / CARD_WAIT_SYNC;
      GFL_CLACT_WK_SetPos( p_wk->p_clwk, &pos, CLSYS_DRAW_MAIN );
    }
    for( i = 0; i < PLAYERINFO_STAR_MAX; i++ )
    { 
      if( p_wk->p_star[i] )
      { 
        GFL_CLACT_WK_GetPos( p_wk->p_star[i], &pos, CLSYS_DRAW_MAIN );
        pos.x = p_wk->star_x[i] - WBM_CARD_INIT_POS_X * p_wk->cnt / CARD_WAIT_SYNC;
        GFL_CLACT_WK_SetPos( p_wk->p_star[i], &pos, CLSYS_DRAW_MAIN );
      }
    }
  }

  if( p_wk->cnt++ >= CARD_WAIT_SYNC )
  { 
    x = 0;
    p_wk->cnt = 0;
    ret = TRUE;
  }

  GFL_BG_SetScrollReq( BG_FRAME_M_CARD, GFL_BG_SCROLL_X_SET, x );
  GFL_BG_SetScrollReq( BG_FRAME_M_FONT, GFL_BG_SCROLL_X_SET, x );


  return ret;
}

#ifdef DEBUG_DATA_CREATE
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグ用データ作成
 *
 *	@param	MATCHINFO_DATA *p_data_adrs データ受け取り
 */
//-----------------------------------------------------------------------------
void MATCHINFO_DEBUG_CreateData( MATCHINFO_DATA *p_data )
{	
	GFL_STD_MemClear( p_data, sizeof(MATCHINFO_DATA) );
	p_data->name[0]	= L'た';
	p_data->name[1]	= L'い';
	p_data->name[2]	= L'せ';
	p_data->name[3]	= L'ん';
	p_data->name[4]	= L'し';
	p_data->name[5]	= L'ゃ';
	p_data->name[6]	= GFL_STR_GetEOMCode();
	p_data->rate		= 999;
	p_data->area		= 0;
	p_data->country	= 0;
	p_data->pms.sentence_type	= 0;
	p_data->pms.sentence_id		= 0;
	p_data->pms.word[0]				= 1;
	p_data->pms.word[1]				= 2;
}
#endif //DEBUG_DATA_CREATE

//=============================================================================
/**
 *				BMPWIN
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	BMPWIN作成
 *
 *	@param	MATCHINFO_WORK * p_wk		ワーク
 *	@param	MATCHINFO_DATA *cp_data	データ
 *	@param	*p_font	フォント
 *	@param	*p_que	キュー
 *	@param	*p_msg	メッセージ
 *	@param	*p_word	単語登録
 *	@param	heapID	ヒープID
 */
//-----------------------------------------------------------------------------
static void MatchInfo_Bmpwin_Create( MATCHINFO_WORK * p_wk, const WIFIBATTLEMATCH_ENEMYDATA *cp_data, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, BOOL is_black, HEAPID heapID )
{	
	static const struct
	{	
		u8 x;
		u8 y;
		u8 w;
		u8 h;
	}sc_bmpwin_range[MATCHINFO_BMPWIN_MAX]	=
	{	
		//プレイヤー名
		{	
			8,5,8,2,
		},
		//レーティング
		{	
			17,5,8,2,
		},
		//レーティングの数値
		{	
			25, 5, 5, 2,
		},
		//住んでいるところ
		{	
			2,9,28,2,
		},
		//国
		{	
			2, 11, 28, 2,
		},
		//場所
		{	
			2, 13, 28, 2,
		},
		//挨拶
		{	
			2, 16, 28, 2,
		},
		//PMS
		{	
			2, 18, 28, 4,
		},
	};

	//BMPWIN作成
	{	
		int i;
		for( i = 0; i < MATCHINFO_BMPWIN_MAX; i++ )
		{	
			p_wk->p_bmpwin[i]	= GFL_BMPWIN_Create( MATCHINFO_BG_FRAME, 
					sc_bmpwin_range[i].x, sc_bmpwin_range[i].y, sc_bmpwin_range[i].w, sc_bmpwin_range[i].h,
					MATCHINFO_PLT_BG_FONT, GFL_BMP_CHRAREA_GET_B );
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0 );
			GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin[i] );

			if( i != MATCHINFO_BMPWIN_PMS )
			{	
				PRINT_UTIL_Setup( &p_wk->print_util[i], p_wk->p_bmpwin[i] );
			}
		}
	}

	//文字表示
	{	
		int i;
		STRBUF	*p_src;
		STRBUF	*p_str;
    PRINTSYS_LSB color;

		BOOL	is_print;

		p_src	= GFL_STR_CreateBuffer( 128, heapID );
		p_str	= GFL_STR_CreateBuffer( 128, heapID );

		for( i = 0; i < MATCHINFO_BMPWIN_MAX; i++ )
		{	
      color = is_black == TRUE ? PLAYERINFO_STR_COLOR_WHITE:PLAYERINFO_STR_COLOR_BLACK;
      is_print	= TRUE;		
			switch( i )
			{	
			case MATCHINFO_BMPWIN_PLAYER:
				GFL_STR_SetStringCode( p_str, cp_data->name );

        color = PLAYERINFO_STR_COLOR_WHITE;
				break;
			case MATCHINFO_BMPWIN_RATE_LABEL:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_008, p_str );
        color = PLAYERINFO_STR_COLOR_WHITE;
				break;
			case MATCHINFO_BMPWIN_RATE_NUM:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_009, p_src );
				WORDSET_RegisterNumber( p_word, 0, cp_data->rate, 4, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
				WORDSET_ExpandStr( p_word, p_str, p_src );
        color = PLAYERINFO_STR_COLOR_WHITE;
				break;
			case MATCHINFO_BMPWIN_COMEFROM:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_018, p_str );
        color = PLAYERINFO_STR_COLOR_WHITE;
				break;
			case MATCHINFO_BMPWIN_CONTRY:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_020, p_src );
				WORDSET_RegisterCountryName( p_word, 0, cp_data->nation );
				WORDSET_ExpandStr( p_word, p_str, p_src );
				break;
			case MATCHINFO_BMPWIN_PLACE:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_021, p_src );
				WORDSET_RegisterLocalPlaceName( p_word, 0, cp_data->nation, cp_data->area );
				WORDSET_ExpandStr( p_word, p_str, p_src );
				break;
			case MATCHINFO_BMPWIN_GREET:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_019, p_str );
        color = PLAYERINFO_STR_COLOR_WHITE;
				break;
			case MATCHINFO_BMPWIN_PMS:
        PMS_DRAW_SetPrintColor( p_wk->p_pms, color );
				PMS_DRAW_Print( p_wk->p_pms, p_wk->p_bmpwin[i], (PMS_DATA*)&cp_data->pms, 0 );
				is_print	= FALSE;
				break;
			}
			if( is_print )
			{	
				PRINT_UTIL_PrintColor( &p_wk->print_util[i], p_que, 0, 0, p_str,  p_font, color );
			}
		}

		GFL_STR_DeleteBuffer( p_src );
		GFL_STR_DeleteBuffer( p_str );
	}

}
//----------------------------------------------------------------------------
/**
 *	@brief	BMPWINプリントチェック
 *
 *	@param	MATCHINFO_WORK * p_wk	ワーク
 *	@param	*p_que									キュー
 *
 *	@return	TRIEでプリント終了	FALSEで処理中
 */
//-----------------------------------------------------------------------------
static BOOL MatchInfo_Bmpwin_PrintMain( MATCHINFO_WORK * p_wk, PRINT_QUE *p_que )
{	
	int i;
	BOOL ret;
	ret	= TRUE;
	for( i = 0; i < MATCHINFO_BMPWIN_MAX; i++ )
	{	
		if( i != MATCHINFO_BMPWIN_PMS )
		{	
			ret	&= PRINT_UTIL_Trans( &p_wk->print_util[i], p_que );
		}
		else
		{	
			PMS_DRAW_Main( p_wk->p_pms );
			ret &= PMS_DRAW_IsPrintEnd( p_wk->p_pms );
		}
	}

	return ret;
}
//----------------------------------------------------------------------------
/**
 *	@brief	BMPWIｎ破棄
 *
 *	@param	MATCHINFO_WORK * p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void MatchInfo_Bmpwin_Delete( MATCHINFO_WORK * p_wk )
{	
	int i;
	for( i = 0; i < MATCHINFO_BMPWIN_MAX; i++ )
	{	
		if( p_wk->p_bmpwin[i] )
		{	
			GFL_BMPWIN_Delete( p_wk->p_bmpwin[i] );
      GFL_BMPWIN_ClearScreen( p_wk->p_bmpwin[i] );
		}
	}
}
//=============================================================================
/**
 *					CLWK
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	対戦相手の見た目を作成
 *
 *	@param	MATCHINFO_WORK * p_wk	ワーク
 *	@param	trainerID			見た目
 *	@param	*p_unit				ユニット
 *	@param	heapID				ヒープID
 */
//-----------------------------------------------------------------------------
static void MatchInfo_TRAINER_Create( MATCHINFO_WORK * p_wk, u32 trainerID, GFL_CLUNIT *p_unit, HEAPID heapID )
{	
	//リソース読みこみ
	{	
		ARCHANDLE	*p_handle;

		p_handle	= GFL_ARC_OpenDataHandle( ARCID_WIFIUNIONCHAR, heapID );

		p_wk->res[ MATCHINFO_RESID_TRAINER_PLT ]	= GFL_CLGRP_PLTT_RegisterEx( p_handle,
        WBM_CARD_TRAINER_RES_NCLR, CLSYS_DRAW_MAIN, MATCHINFO_PLT_OBJ_TRAINER*0x20,
        WBM_CARD_TRAINER_RES_PLT_OFS(trainerID), 1, heapID );

		p_wk->res[ MATCHINFO_RESID_TRAINER_CGR ]		= GFL_CLGRP_CGR_Register( 
				p_handle, WBM_CARD_TRAINER_RES_NCGR(trainerID),
				FALSE, CLSYS_DRAW_MAIN, heapID ); 

		p_wk->res[ MATCHINFO_RESID_TRAINER_CELL ]	= GFL_CLGRP_CELLANIM_Register( 
				p_handle, WBM_CARD_TRAINER_RES_NCER, 
				WBM_CARD_TRAINER_RES_NANR, heapID ); 

		GFL_ARC_CloseDataHandle( p_handle );
	}

	//CLWK
	{	
		GFL_CLWK_DATA	clwk_data;

		GFL_STD_MemClear( &clwk_data, sizeof(GFL_CLWK_DATA) );
		clwk_data.pos_x		= MATCHINFO_TRAINER_X + WBM_CARD_INIT_POS_X;
		clwk_data.pos_y		= MATCHINFO_TRAINER_Y;
		p_wk->p_clwk	= GFL_CLACT_WK_Create( p_unit,
				p_wk->res[ MATCHINFO_RESID_TRAINER_CGR ],
				p_wk->res[ MATCHINFO_RESID_TRAINER_PLT ],
				p_wk->res[ MATCHINFO_RESID_TRAINER_CELL ],
				&clwk_data,
				CLSYS_DRAW_MAIN,
				heapID );

    GFL_CLACT_WK_SetPlttOffs( p_wk->p_clwk, 0, CLWK_PLTTOFFS_MODE_PLTT_TOP );

  }

}
//----------------------------------------------------------------------------
/**
 *	@brief	対戦相手の見た目を破棄
 *
 *	@param	MATCHINFO_WORK * p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void MatchInfo_TRAINER_Delete( MATCHINFO_WORK * p_wk )
{	

	//CLWK破棄
	{	
		GFL_CLACT_WK_Remove( p_wk->p_clwk );
	}

	//リソース破棄
	{	
		GFL_CLGRP_CGR_Release( p_wk->res[ MATCHINFO_RESID_TRAINER_CGR ] );
		GFL_CLGRP_CELLANIM_Release( p_wk->res[ MATCHINFO_RESID_TRAINER_CELL ] );
		GFL_CLGRP_PLTT_Release( p_wk->res[ MATCHINFO_RESID_TRAINER_PLT ] );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief  対戦者の星を破棄
 *
 *	@param	MATCHINFO_WORK * p_wk                 ワーク
 *	@param	WIFIBATTLEMATCH_ENEMYDATA *cp_data    情報
 *	@param	*p_unit                               OBJ作成用ユニット
 *	@param	WIFIBATTLEMATCH_VIEW_RESOURCE *cp_res リソース
 *	@param	heapID                                ヒープID
 */
//-----------------------------------------------------------------------------
static void MatchInfo_STAR_Cleate( MATCHINFO_WORK * p_wk, const WIFIBATTLEMATCH_ENEMYDATA *cp_data, GFL_CLUNIT *p_unit, const WIFIBATTLEMATCH_VIEW_RESOURCE *cp_res, HEAPID heapID )
{ 
  u32 star_num;

  //星の計算
  star_num  = CalcRank( cp_data->btl_cnt, cp_data->win_cnt, cp_data->lose_cnt );

	//CLWK
	{	
    int i;
		GFL_CLWK_DATA	clwk_data;

		GFL_STD_MemClear( &clwk_data, sizeof(GFL_CLWK_DATA) );
		clwk_data.pos_x		= WBM_CARD_INIT_POS_X;
		clwk_data.pos_y		= 16;

    for( i = 0; i < star_num; i++ )
    { 
      clwk_data.pos_x	  	+= 16;
      clwk_data.anmseq		= 2 + i;
      p_wk->p_star[ i ]	= GFL_CLACT_WK_Create( p_unit,
				WIFIBATTLEMATCH_VIEW_GetResource( cp_res, WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_CHR_M ),
				WIFIBATTLEMATCH_VIEW_GetResource( cp_res, WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_PLT_M ),
				WIFIBATTLEMATCH_VIEW_GetResource( cp_res, WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_CEL_M ),
				&clwk_data,
				CLSYS_DRAW_MAIN,
				heapID );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  対戦者の星を破棄
 *
 *	@param	MATCHINFO_WORK * p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void MatchInfo_STAR_Delete( MATCHINFO_WORK * p_wk )
{ 
  int i;
  for( i = 0; i < PLAYERINFO_STAR_MAX; i++ )
  { 
    if( p_wk->p_star[i] != NULL )
    { 
      GFL_CLACT_WK_Remove( p_wk->p_star[i] );
      p_wk->p_star[i] = NULL;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  星の数を計算
 *
 *	@param	u32 btl_cnt 戦闘回数
 *	@param	win         勝ち数
 *	@param	lose        負け数
 *
 *	@return 星の数
 */
//-----------------------------------------------------------------------------
static WBM_CARD_RANK CalcRank( u32 btl_cnt, u32 win, u32 lose )
{ 
  if( win >= WBM_CARD_WIN_BLACK )
  { 
    return WBM_CARD_RANK_BLACK;
  }
  else if( win >= WBM_CARD_WIN_GOLD )
  { 
    return WBM_CARD_RANK_GOLD;
  }
  else if( win >= WBM_CARD_WIN_SILVER )
  { 
    return WBM_CARD_RANK_SILVER;
  }
  else if( win >= WBM_CARD_WIN_BRONZE )
  { 
    return WBM_CARD_RANK_BRONZE;
  }
  else if( win >= WBM_CARD_WIN_COPPER )
  { 
    return WBM_CARD_RANK_COPPER;
  }
  else
  { 
    return WBM_CARD_RANK_NORMAL;
  }
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					待機中OBJ
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	対戦者情報表示ワーク
//=====================================
struct _WBM_WAITICON_WORK
{ 
  GFL_CLWK  *p_light;
  GFL_CLWK  *p_wall;
};

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  待機中OBJ作成
 *
 *	@param	GFL_CLUNIT *p_unit                    OBJ生成ユニット
 *	@param	WIFIBATTLEMATCH_VIEW_RESOURCE *cp_res リソース
 *	@param	heapID                                ヒープID 
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
WBM_WAITICON_WORK	* WBM_WAITICON_Init( GFL_CLUNIT *p_unit,  const WIFIBATTLEMATCH_VIEW_RESOURCE *cp_res, HEAPID heapID )
{ 
  WBM_WAITICON_WORK	*p_wk;
  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(WBM_WAITICON_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(WBM_WAITICON_WORK) );

  //CLWK作成
	{	
		GFL_CLWK_DATA	clwk_data;

		GFL_STD_MemClear( &clwk_data, sizeof(GFL_CLWK_DATA) );
		clwk_data.pos_x		= 128;
		clwk_data.pos_y		= 96;

    clwk_data.anmseq  = 1;
    clwk_data.softpri = 1;
    clwk_data.bgpri   = 1;
		p_wk->p_light	= GFL_CLACT_WK_Create( p_unit,
				WIFIBATTLEMATCH_VIEW_GetResource( cp_res, WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_CHR_M ),
				WIFIBATTLEMATCH_VIEW_GetResource( cp_res, WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_PLT_M ),
				WIFIBATTLEMATCH_VIEW_GetResource( cp_res, WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_CEL_M ),
				&clwk_data,
				CLSYS_DRAW_MAIN,
				heapID );
    GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_light, TRUE );

    clwk_data.anmseq  = 0;
    clwk_data.softpri = 0;
    clwk_data.bgpri   = 1;
		p_wk->p_wall	= GFL_CLACT_WK_Create( p_unit,
				WIFIBATTLEMATCH_VIEW_GetResource( cp_res, WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_CHR_M ),
				WIFIBATTLEMATCH_VIEW_GetResource( cp_res, WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_PLT_M ),
				WIFIBATTLEMATCH_VIEW_GetResource( cp_res, WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_CEL_M ),
				&clwk_data,
				CLSYS_DRAW_MAIN,
				heapID );
    GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_wall, TRUE );
  }


  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  待機中OBJの破棄
 *
 *	@param	WBM_WAITICON_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void WBM_WAITICON_Exit( WBM_WAITICON_WORK *p_wk )
{ 
  GFL_CLACT_WK_Remove( p_wk->p_wall );
  GFL_CLACT_WK_Remove( p_wk->p_light );

  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/** 
 *	@brief  待機中OBJのメイン処理
 *
 *	@param	WBM_WAITICON_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void WBM_WAITICON_Main( WBM_WAITICON_WORK *p_wk )
{ 

}
//----------------------------------------------------------------------------
/**
 *	@brief  待機中OBJの表示設定
 *
 *	@param	WBM_WAITICON_WORK *p_wk ワーク
 *	@param	on_off                  TRUEならば表示  FALSEならば非表示
 */
//-----------------------------------------------------------------------------
void WBM_WAITICON_SetDrawEnable( WBM_WAITICON_WORK *p_wk, BOOL on_off )
{ 
  GFL_CLACT_WK_SetDrawEnable( p_wk->p_light, on_off );
  GFL_CLACT_WK_SetDrawEnable( p_wk->p_wall, on_off );
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  バトルボックス表示
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	定数
//=====================================
//リソース
typedef enum
{
  WBM_BTLBOX_RESID_POKEICON_PLT,
  WBM_BTLBOX_RESID_POKEICON_CGR_TOP,
  WBM_BTLBOX_RESID_POKEICON_CGR_END = WBM_BTLBOX_RESID_POKEICON_CGR_TOP + TEMOTI_POKEMAX,
  WBM_BTLBOX_RESID_POKEICON_CEL,
  WBM_BTLBOX_RESID_POKEITEM_PLT,
  WBM_BTLBOX_RESID_POKEITEM_CGR,
  WBM_BTLBOX_RESID_POKEITEM_CEL,
  WBM_BTLBOX_RESID_MAX,
} WBM_BTLBOX_RESID;

//CLWK
typedef enum
{
  WBM_BTLBOX_CLWCKID_POKE_TOP,
  WBM_BTLBOX_CLWCKID_POKE_END = WBM_BTLBOX_CLWCKID_POKE_TOP + TEMOTI_POKEMAX,
  WBM_BTLBOX_CLWCKID_ITEM_TOP,
  WBM_BTLBOX_CLWCKID_ITEM_END = WBM_BTLBOX_CLWCKID_ITEM_TOP + TEMOTI_POKEMAX,
  WBM_BTLBOX_CLWCKID_MAX,
} WBM_BTLBOX_CLWCKID;

//-------------------------------------
///	対戦者情報表示ワーク
//=====================================
struct _WBM_BTLBOX_WORK
{ 
  u32         res[ WBM_BTLBOX_RESID_MAX ];
  GFL_CLWK    *p_clwk[ WBM_BTLBOX_CLWCKID_MAX ];
  GFL_BMPWIN  *p_bmpwin[ TEMOTI_POKEMAX ];
  PRINT_UTIL  print_util[ TEMOTI_POKEMAX ];
  u32         cnt;
  s16         clwk_x[ WBM_BTLBOX_CLWCKID_MAX ];
};

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  バトルボックスの中身を表示  初期化
 *
 *	@param	const BATTLE_BOX_SAVE * cp_btl_box    バトルボックス  
 *	@param	*p_unit                               OBJ登録用ユニット
 *	@param	WIFIBATTLEMATCH_VIEW_RESOURCE *cp_res リソース
 *	@param	*p_font                               フォント
 *	@param	*p_que                                文字描画キュー
 *	@param	*p_msg                                メッセージ
 *	@param	heapID                                ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
WBM_BTLBOX_WORK	* WBM_BTLBOX_Init( BATTLE_BOX_SAVE * cp_btl_box, GFL_CLUNIT *p_unit, const WIFIBATTLEMATCH_VIEW_RESOURCE *cp_res, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, HEAPID heapID )
{ 
  WBM_BTLBOX_WORK	*p_wk;
  POKEPARTY       *p_party;

  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(WBM_BTLBOX_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(WBM_BTLBOX_WORK) );

  p_party = BATTLE_BOX_SAVE_MakePokeParty( cp_btl_box, GFL_HEAP_LOWID(heapID) );

  {
    GFL_ARC_UTIL_TransVramScreen( ARCID_WIFIMATCH_GRA, NARC_wifimatch_gra_pokesel_NSCR,
				BG_FRAME_M_CARD, 0, 0, FALSE, GFL_HEAP_LOWID( heapID ) );
  }



  //リソース読み込み
  { 
    //ポケモン
    { 
      int i;
      ARCHANDLE	*p_handle;
      POKEMON_PARAM *p_pp;
      p_handle	= GFL_ARC_OpenDataHandle( ARCID_POKEICON, heapID );

      p_wk->res[ WBM_BTLBOX_RESID_POKEICON_PLT ]	= GFL_CLGRP_PLTT_RegisterComp( p_handle,
          POKEICON_GetPalArcIndex(), CLSYS_DRAW_MAIN, PLT_OBJ_POKEMON_M*0x20, GFL_HEAP_LOWID(heapID) );

      p_wk->res[ WBM_BTLBOX_RESID_POKEICON_CEL ]	= GFL_CLGRP_CELLANIM_Register( 
          p_handle, POKEICON_GetCellArcIndex(), POKEICON_GetAnmArcIndex(), GFL_HEAP_LOWID(heapID) ); 

      for( i = 0; i < PokeParty_GetPokeCount( p_party ); i++ )
      {
        p_pp  = PokeParty_GetMemberPointer( p_party, i );
        p_wk->res[ WBM_BTLBOX_RESID_POKEICON_CGR_TOP + i ]		= GFL_CLGRP_CGR_Register( 
            p_handle, POKEICON_GetCgxArcIndex( PP_GetPPPPointerConst(p_pp) ),
            FALSE, CLSYS_DRAW_MAIN, GFL_HEAP_LOWID(heapID) ); 
      }
      GFL_ARC_CloseDataHandle( p_handle );
    }

    //アイテム
    { 
      int i;
      ARCHANDLE	*p_handle;
      p_handle	= GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), heapID );

      p_wk->res[ WBM_BTLBOX_RESID_POKEITEM_PLT ]	= GFL_CLGRP_PLTT_RegisterEx( p_handle,
          APP_COMMON_GetPokeItemIconPltArcIdx(), CLSYS_DRAW_MAIN, PLT_OBJ_POKEITEM_M*0x20, 0, 1, GFL_HEAP_LOWID(heapID) );

      p_wk->res[ WBM_BTLBOX_RESID_POKEITEM_CEL ]	= GFL_CLGRP_CELLANIM_Register( 
          p_handle, APP_COMMON_GetPokeItemIconCellArcIdx( APP_COMMON_MAPPING_128K ),
          APP_COMMON_GetPokeItemIconAnimeArcIdx( APP_COMMON_MAPPING_128K ), GFL_HEAP_LOWID(heapID) ); 

      p_wk->res[ WBM_BTLBOX_RESID_POKEITEM_CGR ]  = GFL_CLGRP_CGR_Register( 
            p_handle, APP_COMMON_GetPokeItemIconCharArcIdx(),
            FALSE, CLSYS_DRAW_MAIN, GFL_HEAP_LOWID(heapID) );


      GFL_ARC_CloseDataHandle( p_handle );
    }
  }

  //CLWK作成
  { 
    int i;
    POKEMON_PARAM *p_pp;
    GFL_CLWK_DATA clwk_data;
    GFL_CLWK_DATA clwk_item_data;
    GFL_STD_MemClear( &clwk_data, sizeof(GFL_CLWK_DATA) );

    clwk_data.pos_x		= 24;
    clwk_data.pos_y		= 48 - 4;
		clwk_data.anmseq	= POKEICON_ANM_HPMAX;
		for( i = 0; i < PokeParty_GetPokeCount( p_party ); i++ )
		{	
      //ポケモン
      p_pp  = PokeParty_GetMemberPointer( p_party, i );
			clwk_data.pos_x	= 24 + i * 40;
			p_wk->p_clwk[ WBM_BTLBOX_CLWCKID_POKE_TOP + i]	= GFL_CLACT_WK_Create( p_unit,
					p_wk->res[ WBM_BTLBOX_RESID_POKEICON_CGR_TOP + i ],
					p_wk->res[ WBM_BTLBOX_RESID_POKEICON_PLT ],
					p_wk->res[ WBM_BTLBOX_RESID_POKEICON_CEL ],
					&clwk_data,
					CLSYS_DRAW_MAIN,
					heapID );

			GFL_CLACT_WK_SetPlttOffs( p_wk->p_clwk[ WBM_BTLBOX_CLWCKID_POKE_TOP + i],
					POKEICON_GetPalNumGetByPPP( PP_GetPPPPointerConst(p_pp) ), 
					CLWK_PLTTOFFS_MODE_OAM_COLOR );

      //道具アイコン
      if( PP_Get( p_pp, ID_PARA_item, NULL) != 0 )
      { 
        clwk_item_data  = clwk_data;
        clwk_item_data.pos_x  += 12;
        clwk_item_data.pos_y  += 12;
        clwk_item_data.softpri  = 1;
        clwk_item_data.anmseq   = 0;
        p_wk->p_clwk[ WBM_BTLBOX_CLWCKID_ITEM_TOP + i]	= GFL_CLACT_WK_Create( p_unit,
          p_wk->res[ WBM_BTLBOX_RESID_POKEITEM_CGR ],
          p_wk->res[ WBM_BTLBOX_RESID_POKEITEM_PLT ],
					p_wk->res[ WBM_BTLBOX_RESID_POKEITEM_CEL ],
					&clwk_item_data,
					CLSYS_DRAW_MAIN,
					heapID );
      }
		}
  }

  //文字描画（性別とレベル）
  {
    int i;
    u8  x;
    u8  w;
    u8  lv;
    POKEMON_PARAM *p_pp;
    WORDSET *p_word;
    STRBUF  *p_strbuf;
    STRBUF  *p_lv_src;
    STRBUF  *p_male_src;
    STRBUF  *p_female_src;

    p_word    = WORDSET_Create( GFL_HEAP_LOWID(heapID) );
    p_strbuf  = GFL_STR_CreateBuffer( 128, GFL_HEAP_LOWID(heapID) );
    p_lv_src  = GFL_MSG_CreateString( p_msg, WIFIMATCH_WIFI_POKE_00 );
    p_male_src  = GFL_MSG_CreateString( p_msg, WIFIMATCH_WIFI_POKE_01 );
    p_female_src  = GFL_MSG_CreateString( p_msg, WIFIMATCH_WIFI_POKE_02 );

    for( i = 0; i < PokeParty_GetPokeCount( p_party ); i++ )
    {
      p_pp  = PokeParty_GetMemberPointer( p_party, i );

      //BMPWIN作成
      w = 5;
      x = 1 + i * w; 
      p_wk->p_bmpwin[i]  = GFL_BMPWIN_Create( BG_FRAME_M_FONT, x, 4, w, 6, PLT_FONT_M, GFL_BMP_CHRAREA_GET_B );
      GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin[i] );
      PRINT_UTIL_Setup( &p_wk->print_util[i], p_wk->p_bmpwin[i] );

      //性別描画
      if( PTL_SEX_MALE == PP_Get( p_pp, ID_PARA_sex, NULL ) )
      { 
        PRINT_UTIL_PrintColor( &p_wk->print_util[i], p_que, 32, 0, p_male_src, p_font, PRINTSYS_LSB_Make( 5, 6, 0 ) );
      }
      else if( PTL_SEX_FEMALE == PP_Get( p_pp, ID_PARA_sex, NULL ) )
      { 
        PRINT_UTIL_PrintColor( &p_wk->print_util[i], p_que, 32, 0, p_female_src, p_font, PRINTSYS_LSB_Make( 3, 4, 0 ) );
      }

      //レベル描画
      lv  = PP_Get( p_pp, ID_PARA_level, NULL );
      WORDSET_RegisterNumber( p_word, 0, lv, 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
      WORDSET_ExpandStr( p_word, p_strbuf, p_lv_src );
      x = 40 /2 - PRINTSYS_GetStrWidth( p_strbuf, p_font, 0 )/2;
      PRINT_UTIL_PrintColor( &p_wk->print_util[i], p_que, x, 36, p_strbuf, p_font, PRINTSYS_LSB_Make( 1, 2, 0 ) );
    }

    GFL_STR_DeleteBuffer( p_female_src );
    GFL_STR_DeleteBuffer( p_male_src );
    GFL_STR_DeleteBuffer( p_lv_src );
    GFL_STR_DeleteBuffer( p_strbuf );
    WORDSET_Delete( p_word );
  }

  //横からスライドインしてくるため、位置をずらしておく
  { 
    int i;
    GFL_CLACTPOS  pos;

    for( i = 0; i < WBM_BTLBOX_CLWCKID_MAX; i++ )
    { 
      if( p_wk->p_clwk[i] )
      { 
        GFL_CLACT_WK_GetPos( p_wk->p_clwk[i], &pos, CLSYS_DRAW_MAIN );
        pos.x +=  WBM_CARD_INIT_POS_X;
        GFL_CLACT_WK_SetPos( p_wk->p_clwk[i], &pos, CLSYS_DRAW_MAIN );
      }
    }

    GFL_BG_SetScroll( BG_FRAME_M_CARD, GFL_BG_SCROLL_X_SET, -WBM_CARD_INIT_POS_X );
    GFL_BG_SetScroll( BG_FRAME_M_FONT, GFL_BG_SCROLL_X_SET, -WBM_CARD_INIT_POS_X );
  }

  GFL_HEAP_FreeMemory( p_party );

  G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG3, PLAYERINFO_ALPHA_EV1, PLAYERINFO_ALPHA_EV2); 

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バトルボックスの中身を表示  破棄
 *
 *	@param	WBM_BTLBOX_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void WBM_BTLBOX_Exit( WBM_BTLBOX_WORK *p_wk )
{ 
  G2_BlendNone();
  //BMPWIN破棄
  { 
    int i;
    for( i = 0; i < TEMOTI_POKEMAX; i++ )
    { 
      if( p_wk->p_bmpwin[i] )
      { 
        GFL_BMPWIN_Delete( p_wk->p_bmpwin[i] );
        GFL_BMPWIN_ClearScreen( p_wk->p_bmpwin[i] );
      }
    }
  }

  //CLWK破棄
  { 
    int i;
    for( i = 0; i < WBM_BTLBOX_CLWCKID_MAX; i++ )
    { 
      if( p_wk->p_clwk[i] )
      { 
        GFL_CLACT_WK_Remove( p_wk->p_clwk[i] );
      }
    }
  }


  //リソース破棄
  { 
    int i;  
		GFL_CLGRP_CGR_Release( p_wk->res[ WBM_BTLBOX_RESID_POKEITEM_CGR ] );
		GFL_CLGRP_CELLANIM_Release( p_wk->res[ WBM_BTLBOX_RESID_POKEITEM_CEL ] );
		GFL_CLGRP_PLTT_Release( p_wk->res[ WBM_BTLBOX_RESID_POKEITEM_PLT ] );

    for( i = 0; i < TEMOTI_POKEMAX; i++ )
    { 
      if( p_wk->p_clwk[i] )
      { 
        GFL_CLGRP_CGR_Release( p_wk->res[ WBM_BTLBOX_RESID_POKEICON_CGR_TOP + i ] );
      }
    }
    GFL_CLGRP_CELLANIM_Release( p_wk->res[ WBM_BTLBOX_RESID_POKEICON_CEL ] );
		GFL_CLGRP_PLTT_Release( p_wk->res[ WBM_BTLBOX_RESID_POKEICON_PLT ] );
  }

  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  バトルボックスの中身を表示  移動
 *
 *	@param	WBM_BTLBOX_WORK *p_wk   ワーク
 *
 *	@return TRUEで移動終了  FALSEで移動中
 */
//-----------------------------------------------------------------------------
BOOL WBM_BTLBOX_MoveInMain( WBM_BTLBOX_WORK *p_wk )
{ 

  if( GFL_BG_GetScrollX( BG_FRAME_M_CARD ) == 0 )
  { 
    return TRUE;
  }
  else
  { 
    BOOL ret  = FALSE;
  
    s32 x;

    if( p_wk->cnt == 0 )
    { 
      int i;
      GFL_CLACTPOS  pos;
      for( i = 0; i < WBM_BTLBOX_CLWCKID_MAX; i++ )
      { 
        if( p_wk->p_clwk[i] )
        { 
          GFL_CLACT_WK_GetPos( p_wk->p_clwk[i], &pos, CLSYS_DRAW_MAIN );
          p_wk->clwk_x[i]  = pos.x;
        }
      }
    }

    x = -WBM_CARD_INIT_POS_X + WBM_CARD_INIT_POS_X * p_wk->cnt / CARD_WAIT_SYNC;

    { 
      int i;
      GFL_CLACTPOS  pos;
      for( i = 0; i < WBM_BTLBOX_CLWCKID_MAX; i++ )
      { 
        if( p_wk->p_clwk[i] )
        { 
          GFL_CLACT_WK_GetPos( p_wk->p_clwk[i], &pos, CLSYS_DRAW_MAIN );
          pos.x = p_wk->clwk_x[i] - WBM_CARD_INIT_POS_X * p_wk->cnt / CARD_WAIT_SYNC;
          GFL_CLACT_WK_SetPos( p_wk->p_clwk[i], &pos, CLSYS_DRAW_MAIN );
        }
      }
    }

    if( p_wk->cnt++ >= CARD_WAIT_SYNC )
    { 
      x = 0;
      p_wk->cnt = 0;
      ret = TRUE;
    }

    GFL_BG_SetScrollReq( BG_FRAME_M_CARD, GFL_BG_SCROLL_X_SET, x );
    GFL_BG_SetScrollReq( BG_FRAME_M_FONT, GFL_BG_SCROLL_X_SET, x );

    return ret;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  バトルボックスの中身を表示  移動
 *
 *	@param	WBM_BTLBOX_WORK *p_wk   ワーク
 *
 *	@return TRUEで移動終了  FALSEで移動中
 */
//-----------------------------------------------------------------------------
BOOL WBM_BTLBOX_MoveOutMain( WBM_BTLBOX_WORK *p_wk )
{ 
  BOOL ret  = FALSE;
  
  s32 x;

  if( p_wk->cnt == 0 )
  { 
    int i;
    GFL_CLACTPOS  pos;
    for( i = 0; i < WBM_BTLBOX_CLWCKID_MAX; i++ )
    { 
      if( p_wk->p_clwk[i] )
      { 
        GFL_CLACT_WK_GetPos( p_wk->p_clwk[i], &pos, CLSYS_DRAW_MAIN );
        p_wk->clwk_x[i]  = pos.x;
      }
    }
  }

  x = WBM_CARD_INIT_POS_X * p_wk->cnt / CARD_WAIT_SYNC;

  { 
    int i;
    GFL_CLACTPOS  pos;
    for( i = 0; i < WBM_BTLBOX_CLWCKID_MAX; i++ )
    { 
      if( p_wk->p_clwk[i] )
      { 
        GFL_CLACT_WK_GetPos( p_wk->p_clwk[i], &pos, CLSYS_DRAW_MAIN );
        pos.x = p_wk->clwk_x[i] - WBM_CARD_INIT_POS_X * p_wk->cnt / CARD_WAIT_SYNC;
        GFL_CLACT_WK_SetPos( p_wk->p_clwk[i], &pos, CLSYS_DRAW_MAIN );
      }
    }
  }

  if( p_wk->cnt++ >= CARD_WAIT_SYNC )
  { 
    x = WBM_CARD_INIT_POS_X;
    p_wk->cnt = 0;
    ret = TRUE;
  }

  GFL_BG_SetScrollReq( BG_FRAME_M_CARD, GFL_BG_SCROLL_X_SET, x );
  GFL_BG_SetScrollReq( BG_FRAME_M_FONT, GFL_BG_SCROLL_X_SET, x );

  return ret;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バトルボックスの中身を表示  文字描画メイン
 *
 *	@param	WBM_BTLBOX_WORK *p_wk   ワーク
 *
 *	@return TRUEで文字描画完了  FALSEで文字描画中
 */
//-----------------------------------------------------------------------------
BOOL WBM_BTLBOX_PrintMain( WBM_BTLBOX_WORK *p_wk, PRINT_QUE *p_que )
{ 
  BOOL ret = TRUE;

  //BMPWIN破棄
  { 
    int i;
    for( i = 0; i < TEMOTI_POKEMAX; i++ )
    { 
      if( p_wk->p_bmpwin[i] )
      {
        ret &= PRINT_UTIL_Trans( &p_wk->print_util[i], p_que );
      }
    }
  }

  return ret;
}
