//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_bvrank_proc.c
 *	@brief	バトルビデオランクプロセス
 *	@author	Toru=Nagihashi
 *	@data		2009.11.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/gfl_use.h"
#include "system/main.h"  //HEAPID
#include "print/wordset.h"
#include "pokeicon/pokeicon.h"
#include "pm_define.h"

//アーカイブ
#include "arc_def.h"
#include "msg/msg_battle_rec.h"

//自分のモジュール
#include "br_pokesearch.h"
#include "br_inner.h"
#include "br_util.h"
#include "br_btn.h"

//外部公開
#include "br_bvrank_proc.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	ランキング
//=====================================
#define BR_RANK_LIST_LINE  (5)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ランキング
//=====================================
typedef struct 
{
  BMP_MENULIST_DATA *p_list_data;
  BR_LIST_WORK      *p_list;
  WORDSET           *p_word;

  u16               plt;
  u16               cel;
  u16               cgr[ BR_RANK_LIST_LINE ][ TEMOTI_POKEMAX ];
  s32               cgr_idx;
  GFL_CLWK          *p_poke[ BR_RANK_LIST_LINE ][ TEMOTI_POKEMAX ];
  
  const BR_OUTLINE_DATA *cp_data;
} BR_RANK_WORK;

//-------------------------------------
///	バトルビデオランクメインワーク
//=====================================
typedef struct 
{
  BMPOAM_SYS_PTR	  p_bmpoam;	//BMPOAMシステム
  BR_BTN_WORK       *p_btn;
  BR_MSGWIN_WORK    *p_msgwin;
  BR_MSGWIN_WORK    *p_title;
  PRINT_QUE         *p_que;
	HEAPID            heapID;

  BR_OUTLINE_DATA   outline;
  BR_RANK_WORK      rank;
  
} BR_BVRANK_WORK;


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	バトルビデオランキングプロセス
//=====================================
static GFL_PROC_RESULT BR_BVRANK_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BR_BVRANK_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BR_BVRANK_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
//-------------------------------------
///	ランキングワーク
//=====================================
static void BR_RANK_Init( BR_RANK_WORK *p_wk, const BR_OUTLINE_DATA *cp_data, BR_RES_WORK *p_res, GFL_CLUNIT *p_unit, HEAPID heapID );
static void BR_RANK_Exit( BR_RANK_WORK *p_wk );
static void BR_RANK_Main( BR_RANK_WORK *p_wk, HEAPID heapID );
static u32  BR_RANK_GetSelect( const BR_RANK_WORK *cp_wk );
static void Br_Rank_CreatePokeIcon( BR_RANK_WORK *p_wk, GFL_CLUNIT *p_unit, HEAPID heapID );
static void Br_Rank_DeletePokeIcon( BR_RANK_WORK *p_wk );
static void Br_Rank_ScrollPokeIcon( BR_RANK_WORK *p_wk, u32 list, s8 dir, HEAPID heapID );

//-------------------------------------
///	その他
//=====================================
static BOOL Br_Rank_IsPushBattleBV( u32 x, u32 y );

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//-------------------------------------
///	バトルビデオランクプロセス
//=====================================
const GFL_PROC_DATA BR_BVRANK_ProcData =
{	
	BR_BVRANK_PROC_Init,
	BR_BVRANK_PROC_Main,
	BR_BVRANK_PROC_Exit,
};

//=============================================================================
/**
 *					データ
 */
//=============================================================================

//=============================================================================
/**
 *					バトルビデオランクプロセス
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	バトルビデオランクプロセス	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_BVRANK_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	BR_BVRANK_WORK				*p_wk;
	BR_BVRANK_PROC_PARAM	*p_param	= p_param_adrs;
  GFL_FONT    *p_font;
  GFL_MSGDATA *p_msg;

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(BR_BVRANK_WORK), BR_PROC_SYS_GetHeapID( p_param->p_procsys ) );
	GFL_STD_MemClear( p_wk, sizeof(BR_BVRANK_WORK) );	
	p_wk->heapID	= BR_PROC_SYS_GetHeapID( p_param->p_procsys );
  p_wk->p_bmpoam	= BmpOam_Init( p_wk->heapID, p_param->p_unit );
  p_wk->p_que     = PRINTSYS_QUE_Create( p_wk->heapID );

	//グラフィック初期化
  BR_RES_LoadBG( p_param->p_res, BR_RES_BG_BVRANK, p_wk->heapID );
  BR_RES_LoadOBJ( p_param->p_res, BR_RES_OBJ_SHORT_BTN_S, p_wk->heapID );

  //モジュール初期化
  p_font  = BR_RES_GetFont( p_param->p_res );
  p_msg   = BR_RES_GetMsgData( p_param->p_res );
  //ボタン
  { 
    GFL_CLWK_DATA cldata;
    BR_RES_OBJ_DATA res;
    BOOL ret;


    GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

    cldata.pos_x    = 80;
    cldata.pos_y    = 168;
    cldata.anmseq   = 0;
    cldata.softpri  = 1;

    ret = BR_RES_GetOBJRes( p_param->p_res, BR_RES_OBJ_SHORT_BTN_S, &res );
    GF_ASSERT( ret );

    p_wk->p_btn = BR_BTN_Init( &cldata, msg_05, BR_BTN_DATA_SHORT_WIDTH, CLSYS_DRAW_SUB, p_param->p_unit, p_wk->p_bmpoam, p_font, p_msg, &res, p_wk->heapID );
;
  }
  //メッセージ
  { 
    //バトルビデオを見る
    p_wk->p_msgwin  = BR_MSGWIN_Init( BG_FRAME_S_FONT, 11, 6, 10, 4, PLT_BG_S_FONT, p_wk->p_que, p_wk->heapID );
    BR_MSGWIN_PrintColor( p_wk->p_msgwin, p_msg, msg_603, p_font, BR_PRINT_COL_NORMAL );

    //バトルビデオさいしん３０件
    p_wk->p_title = BR_MSGWIN_Init( BG_FRAME_M_FONT, 3, 3, 26, 4, PLT_BG_M_FONT, p_wk->p_que, p_wk->heapID );
    BR_MSGWIN_PrintColor( p_wk->p_title, p_msg, msg_605, p_font, BR_PRINT_COL_NORMAL );

  }

  DEBUG_BR_OUTLINE_SetData( &p_wk->outline );
  BR_RANK_Init( &p_wk->rank, &p_wk->outline, p_param->p_res, p_param->p_unit, p_wk->heapID );

  //アイコン作成
  Br_Rank_CreatePokeIcon( &p_wk->rank, p_param->p_unit, p_wk->heapID );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルビデオランクプロセス	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_BVRANK_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	BR_BVRANK_WORK				*p_wk	= p_wk_adrs;
	BR_BVRANK_PROC_PARAM	*p_param	= p_param_adrs;

  //アイコン破棄
  Br_Rank_DeletePokeIcon( &p_wk->rank );

  BR_RANK_Exit( &p_wk->rank );

	//モジュール破棄
  BR_MSGWIN_Exit( p_wk->p_title );
  BR_MSGWIN_Exit( p_wk->p_msgwin );
  BR_BTN_Exit( p_wk->p_btn );
  BR_RES_UnLoadOBJ( p_param->p_res, BR_RES_OBJ_SHORT_BTN_S );
  BR_RES_UnLoadBG( p_param->p_res, BR_RES_BG_BVRANK );

  GFL_BG_LoadScreenReq( BG_FRAME_S_FONT );
  GFL_BG_LoadScreenReq( BG_FRAME_M_FONT );

  BmpOam_Exit( p_wk->p_bmpoam ); 
  PRINTSYS_QUE_Delete( p_wk->p_que );
	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルビデオランクプロセス	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_BVRANK_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
  enum
  { 
    SEQ_FADEIN_START,
    SEQ_FADEIN_WAIT,
    SEQ_MAIN,
    SEQ_FADEOUT_START,
    SEQ_FADEOUT_WAIT,
    SEQ_END,
  };
	BR_BVRANK_WORK	*p_wk	= p_wk_adrs;
	BR_BVRANK_PROC_PARAM	*p_param	= p_param_adrs;

  switch( *p_seq )
  { 
  case SEQ_FADEIN_START:
    BR_FADE_StartFade( p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_IN );
    *p_seq  = SEQ_FADEIN_WAIT;
    break;

  case SEQ_FADEIN_WAIT:
    if( BR_FADE_IsEnd( p_param->p_fade ) )
    { 
      *p_seq  = SEQ_MAIN;
    }
    break;
   
  case SEQ_MAIN:
    BR_RANK_Main( &p_wk->rank, p_wk->heapID );

    {
      u32 x, y;
      if( GFL_UI_TP_GetPointTrg( &x, &y ) )
      {
        if( BR_BTN_GetTrg( p_wk->p_btn, x, y ) )
        { 
          BR_PROC_SYS_Pop( p_param->p_procsys );
          *p_seq  = SEQ_FADEOUT_START;
          break;
        }
        //if( Br_Rank_IsPushBattleBV( x, y ) )
        if(0)
        { 
          BR_RANK_GetSelect( &p_wk->rank );
          BR_PROC_SYS_Push( p_param->p_procsys, BR_PROCID_RECORD );
          *p_seq  = SEQ_FADEOUT_START;
          break;
        }
      }
    }
    break;

  case SEQ_FADEOUT_START:
    BR_FADE_StartFade( p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_OUT );
    *p_seq  = SEQ_FADEOUT_WAIT;
    break;

  case SEQ_FADEOUT_WAIT:
    if( BR_FADE_IsEnd( p_param->p_fade ) )
    { 
      *p_seq  = SEQ_END;
    }
    break;

  case SEQ_END:
    return GFL_PROC_RES_FINISH;
  }


    

  BR_MSGWIN_PrintMain( p_wk->p_msgwin );
  BR_MSGWIN_PrintMain( p_wk->p_title );
  PRINTSYS_QUE_Main( p_wk->p_que );

	return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *  ランキングワーク
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ランキングを初期化
 *
 *	@param	BR_RANK_WORK *p_wk        ワーク
 *	@param	BR_OUTLINE_DATA *cp_data  リストに表示するデータ
 *	@param	*p_res  リソース
 *	@param	*p_unit ユニット
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
static void BR_RANK_Init( BR_RANK_WORK *p_wk, const BR_OUTLINE_DATA *cp_data, BR_RES_WORK *p_res, GFL_CLUNIT *p_unit, HEAPID heapID )
{ 
  GFL_FONT    *p_font;
  GFL_MSGDATA *p_msg;
  p_font  = BR_RES_GetFont( p_res );
  p_msg   = BR_RES_GetMsgData( p_res );

  //ワーク初期化
  GFL_STD_MemClear( p_wk, sizeof(BR_RANK_WORK) );
  p_wk->cp_data   = cp_data;
  p_wk->p_word    = WORDSET_Create( heapID );

  //リストデータ作成
  { 
    int i;
    p_wk->p_list_data = BmpMenuWork_ListCreate( cp_data->data_num, heapID );

    for( i = 0; i < cp_data->data_num; i++ )
    { 
      p_wk->p_list_data[i].param  = i;
    }
  }
  { 
    static const BR_LIST_PARAM sc_list_param =
    {
      NULL,
      0,
      2,
      8,
      9,
      15,
      PLT_BG_M_FONT,
      BG_FRAME_M_FONT,
      3,
      BR_LIST_TYPE_CURSOR,
      NULL,
      NULL, 
    };
    int i;
    STRBUF  *p_colon;
    STRBUF  *p_str1;
    STRBUF  *p_str2;
    STRBUF  *p_str_name;
    int     w;

    //リスト設定
    BR_LIST_PARAM list_param  = sc_list_param;
    list_param.cp_list  = p_wk->p_list_data;
    list_param.list_max = cp_data->data_num;
    list_param.p_res    = p_res;
    list_param.p_unit   = p_unit;
    //リスト作成
    p_wk->p_list  = BR_LIST_Init( &list_param, heapID );

    //リストを拡張  BMPを外部から設定する
    p_colon     = GFL_MSG_CreateString( p_msg, msg_607 );
    p_str1      = GFL_STR_CreateBuffer( 255, heapID );
    p_str2      = GFL_MSG_CreateString( p_msg, msg_727 );
    p_str_name  = GFL_STR_CreateBuffer( 255, heapID );
    for( i = 0; i < cp_data->data_num; i++ )
    { 
      GFL_BMP_DATA  *p_bmp;
      //作成
      p_bmp = GFL_BMP_Create( list_param.w, list_param.str_line, GFL_BMP_16_COLOR, heapID );
      GFL_BMP_Clear( p_bmp, 0 );

      //文字列
      WORDSET_RegisterNumber( p_wk->p_word, 0, i+1, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
      WORDSET_ExpandStr( p_wk->p_word, p_str1, p_str2 );
      GFL_STR_SetStringCode( p_str_name, cp_data->data[ i ].profile.name );

      w = PRINTSYS_GetStrWidth( p_str1, p_font, 0 );

      //書き込み
      PRINTSYS_PrintColor( p_bmp, 16-w, 4, p_str1, p_font, BR_PRINT_COL_NORMAL );
      PRINTSYS_PrintColor( p_bmp, 16, 4, p_colon, p_font, BR_PRINT_COL_NORMAL );
      PRINTSYS_PrintColor( p_bmp, 22, 4, p_str_name, p_font, BR_PRINT_COL_NORMAL );

      //リストに設定
      BR_LIST_SetBmp( p_wk->p_list, i, p_bmp );
    }

    GFL_STR_DeleteBuffer( p_str_name );
    GFL_STR_DeleteBuffer( p_str1 );
    GFL_STR_DeleteBuffer( p_str2 );
    GFL_STR_DeleteBuffer( p_colon );
  }

  BR_LIST_Write( p_wk->p_list );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ランキングを破棄
 *
 *	@param	BR_RANK_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BR_RANK_Exit( BR_RANK_WORK *p_wk )
{ 
  int i;
  for( i = 0; i < BR_LIST_GetParam( p_wk->p_list,BR_LIST_PARAM_IDX_LIST_MAX); i++ )
  { 
    GFL_BMP_DATA *p_bmp = BR_LIST_GetBmp( p_wk->p_list, i );
    GFL_BMP_Delete( p_bmp );
  }
  WORDSET_Delete( p_wk->p_word );
  BR_LIST_Exit( p_wk->p_list );
  BmpMenuWork_ListDelete( p_wk->p_list_data );
  GFL_STD_MemClear( p_wk, sizeof(BR_RANK_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ランキングメイン処理
 *
 *	@param	BR_RANK_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BR_RANK_Main( BR_RANK_WORK *p_wk, HEAPID heapID )
{ 
  u32 pre_list, now_list;
  s8  dir;

  //動く前の位置が欲しいので、Mainの前
  pre_list  = BR_LIST_GetParam( p_wk->p_list, BR_LIST_PARAM_IDX_LIST_POS );

  BR_LIST_Main( p_wk->p_list );

  now_list  = BR_LIST_GetParam( p_wk->p_list, BR_LIST_PARAM_IDX_LIST_POS );

  dir   = BR_LIST_GetParam( p_wk->p_list, BR_LIST_PARAM_IDX_MOVE_TIMING );
  if( dir != 0 && pre_list != now_list )
  {
    NAGI_Printf( "timing %d\n", dir );
    Br_Rank_ScrollPokeIcon( p_wk, now_list, dir, heapID );
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  ランキングで現在選択中のものを取得
 *
 *	@param	BR_RANK_WORK *p_wk  ワーク
 *
 *	@return 選択したもの
 */
//-----------------------------------------------------------------------------
static u32  BR_RANK_GetSelect( const BR_RANK_WORK *cp_wk )
{ 
  return BR_LIST_GetSelect( cp_wk->p_list );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンアイコンを作成
 *
 *	@param	BR_RANK_WORK *p_wk  ワーク
 *	@param	*p_unit             ユニット
 *	@param	heapID              ヒープID
 */
//-----------------------------------------------------------------------------
static void Br_Rank_CreatePokeIcon( BR_RANK_WORK *p_wk, GFL_CLUNIT *p_unit, HEAPID heapID )
{ 
  //リストの最大値がラインより少ないならば、それ以降は最大値はリストの最大値
  const int c_max = p_wk->cp_data->data_num < BR_RANK_LIST_LINE ? 
    p_wk->cp_data->data_num: BR_RANK_LIST_LINE;

  int i, j;
  GFL_CLWK_DATA cldata;
  ARCHANDLE *p_handle;
  u32 mons_tbl[ TEMOTI_POKEMAX ];
  u32 form_tbl[ TEMOTI_POKEMAX ];

  //共通リソース読み込み
  p_handle  = GFL_ARC_OpenDataHandle( ARCID_POKEICON, GFL_HEAP_LOWID(heapID) );
  p_wk->plt = GFL_CLGRP_PLTT_RegisterComp( p_handle, 
          POKEICON_GetPalArcIndex(),
          CLSYS_DRAW_MAIN, PLT_OBJ_M_POKEICON*0x20, heapID );
  p_wk->cel = GFL_CLGRP_CELLANIM_Register( p_handle,
					POKEICON_GetCellArcIndex(), 
          POKEICON_GetAnmArcIndex(), heapID );

  //CLDATA設定
  GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
  cldata.anmseq = POKEICON_ANM_HPMAX;
  cldata.softpri  = 0;
  cldata.bgpri    = 0;

  //CLWk作成
  for( i = 0; i < c_max; i++ )
  { 
    //卵抜きソート
    { 
      int k;
      int idx;
      int monsno;
      int formno;
      GFL_STD_MemClear( mons_tbl, sizeof(u32) * TEMOTI_POKEMAX );
      GFL_STD_MemClear( form_tbl, sizeof(u32) * TEMOTI_POKEMAX );
      idx = 0;
      //@todo ダブルとシングルで詰め方が違う　
      for( k = 0; k < TEMOTI_POKEMAX; k++ )
      { 
        monsno  = p_wk->cp_data->data[ i ].head.monsno[ k ];
        formno  = p_wk->cp_data->data[ i ].head.form_no[ k ];
        if( monsno == 0 )
        { 
          continue;
        }
        mons_tbl[idx]  = monsno;
        form_tbl[idx]  = formno;
        idx++;
      }
    }
    
    for( j = 0; j < TEMOTI_POKEMAX; j++ )
    { 
      NAGI_Printf( "i %d j %d\n", i, j );
      //リソースをBR_RANK_LIST_LINE * TEMOTI_POKEMAX分読み込む
      p_wk->cgr[i][j] = GFL_CLGRP_CGR_Register( p_handle,
					POKEICON_GetCgxArcIndexByMonsNumber( mons_tbl[j] == 0 ? 1: mons_tbl[j], form_tbl[i], FALSE ),
          FALSE, CLSYS_DRAW_MAIN, heapID );

      //CLWK作成
      cldata.pos_x  = 106 + j * 24;
      cldata.pos_y  =  76 + i * 24;
      p_wk->p_poke[i][j]  = GFL_CLACT_WK_Create( p_unit,
            p_wk->cgr[i][j], p_wk->plt, p_wk->cel,
            &cldata, CLSYS_DRAW_MAIN, heapID );
      GFL_CLACT_WK_SetPlttOffs( p_wk->p_poke[i][j], POKEICON_GetPalNum(mons_tbl[j] == 0 ? 1: mons_tbl[j], form_tbl[j], FALSE) , CLWK_PLTTOFFS_MODE_OAM_COLOR );
      GFL_CLACT_WK_SetObjMode( p_wk->p_poke[i][j], GX_OAM_MODE_XLU );
    }
  }

  GFL_ARC_CloseDataHandle( p_handle );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンアイコン消去
 *
 *	@param	BR_RANK_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Br_Rank_DeletePokeIcon( BR_RANK_WORK *p_wk )
{  
  int i, j;
  GFL_CLGRP_CELLANIM_Release( p_wk->cel );
  GFL_CLGRP_PLTT_Release( p_wk->plt );

  for( i = 0; i < BR_RANK_LIST_LINE; i++ )
  { 
    for( j = 0; j < TEMOTI_POKEMAX; j++ )
    { 
      if( p_wk->p_poke[i][j] )
      { 
        //リソースリリース
        GFL_CLGRP_CGR_Release( p_wk->cgr[i][j] );

        //CLWKリリース
        GFL_CLACT_WK_Remove( p_wk->p_poke[i][j] );
        p_wk->p_poke[i][j]  = NULL;
      }
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ポケアイコンを再読み込み
 *
 *	@param	BR_RANK_WORK *p_wk  ワーク
 *	@param  now                 移動前のリストの位置
 *	@param	dir                 方向
 */
//-----------------------------------------------------------------------------
static void Br_Rank_ScrollPokeIcon( BR_RANK_WORK *p_wk, u32 list, s8 dir, HEAPID heapID )
{ 
  int i, j;
  ARCHANDLE *p_handle;
  u32 mons_tbl[ TEMOTI_POKEMAX ];
  u32 form_tbl[ TEMOTI_POKEMAX ];
  NNSG2dCharacterData *p_chr_data;
  void *p_chr_buff;

  //CLWk作成
  for( i = 0; i < BR_RANK_LIST_LINE; i++ )
  { 
    //卵抜きソート
    { 
      int k;
      int idx;
      int monsno;
      int formno;
      GFL_STD_MemClear( mons_tbl, sizeof(u32) * TEMOTI_POKEMAX );
      GFL_STD_MemClear( form_tbl, sizeof(u32) * TEMOTI_POKEMAX );
      idx = 0;
      //@todo ダブルとシングルで詰め方が違う　
      for( k = 0; k < TEMOTI_POKEMAX; k++ )
      { 
        monsno  = p_wk->cp_data->data[ i + list ].head.monsno[ k ];
        formno  = p_wk->cp_data->data[ i + list ].head.form_no[ k ];
        if( monsno == 0 )
        { 
          continue;
        }
        mons_tbl[idx]  = monsno;
        form_tbl[idx]  = formno;
        idx++;
      }
    }
    
    for( j = 0; j < TEMOTI_POKEMAX; j++ )
    { 

      //先頭と後尾は行わない
      //方向によって読み替え順番が違う
      if( dir == 1 && i != BR_RANK_LIST_LINE-1 )
      { 
        //CLWK素材読み替え
        NNSG2dImageProxy  img;
        u8                plt_ofs;
        GFL_CLACT_WK_GetImgProxy( p_wk->p_poke[i + 1 ][j], &img );
        plt_ofs = GFL_CLACT_WK_GetPlttOffs( p_wk->p_poke[i+ 1 ][j] );
        GFL_CLACT_WK_SetImgProxy( p_wk->p_poke[i ][j], &img );
        GFL_CLACT_WK_SetPlttOffs( p_wk->p_poke[i ][j], plt_ofs, CLWK_PLTTOFFS_MODE_OAM_COLOR );
      }
      else if( dir == -1 && i != BR_RANK_LIST_LINE-1 )
      { 
        //CLWK素材読み替え
        //逆から読み替える
        NNSG2dImageProxy  img;
        u8                plt_ofs;
        GFL_CLACT_WK_GetImgProxy( p_wk->p_poke[BR_RANK_LIST_LINE - 2 -i][j], &img );
        plt_ofs = GFL_CLACT_WK_GetPlttOffs( p_wk->p_poke[BR_RANK_LIST_LINE - 2 -i][j] );
        GFL_CLACT_WK_SetImgProxy( p_wk->p_poke[BR_RANK_LIST_LINE - 1 -i][j], &img );
        GFL_CLACT_WK_SetPlttOffs( p_wk->p_poke[BR_RANK_LIST_LINE - 1 -i][j], plt_ofs, CLWK_PLTTOFFS_MODE_OAM_COLOR );
      }
    }
  }

  p_handle  = GFL_ARC_OpenDataHandle( ARCID_POKEICON, GFL_HEAP_LOWID(heapID) );

  //↑or↓分のリソースを TEMOTI_POKEMAX分読み込む

  if( dir == -1 )
  { 
    p_wk->cgr_idx--;
    if( p_wk->cgr_idx  < 0 )
    { 
      p_wk->cgr_idx = BR_RANK_LIST_LINE - 1;
    }
  }

  for( j = 0; j < TEMOTI_POKEMAX; j++ )
  { 
    if( dir == 1 )
    { 
      i = BR_RANK_LIST_LINE - 1;
    }
    else if( dir == -1 )
    { 
      i = 0;
    }

    //卵抜きソート
    { 
      int k;
      int idx;
      int monsno;
      int formno;
      GFL_STD_MemClear( mons_tbl, sizeof(u32) * TEMOTI_POKEMAX );
      GFL_STD_MemClear( form_tbl, sizeof(u32) * TEMOTI_POKEMAX );
      idx = 0;
      //@todo ダブルとシングルで詰め方が違う　
      for( k = 0; k < TEMOTI_POKEMAX; k++ )
      { 
        monsno  = p_wk->cp_data->data[i + list ].head.monsno[ k ];
        formno  = p_wk->cp_data->data[i + list ].head.form_no[ k ];
        if( monsno == 0 )
        { 
          continue;
        }
        mons_tbl[idx]  = monsno;
        form_tbl[idx]  = formno;
        idx++;
      }
    }

    {
      NNSG2dImageProxy  img;
      p_chr_buff  = GFL_ARCHDL_UTIL_LoadOBJCharacter( p_handle, POKEICON_GetCgxArcIndexByMonsNumber( mons_tbl[j] == 0 ? 1: mons_tbl[j], form_tbl[j], FALSE ), FALSE, &p_chr_data, GFL_HEAP_LOWID(heapID) );
      GFL_CLGRP_CGR_Replace( p_wk->cgr[ p_wk->cgr_idx ][j], p_chr_data );
      GFL_CLGRP_CGR_GetProxy( p_wk->cgr[ p_wk->cgr_idx ][j], &img );
      GFL_CLACT_WK_SetImgProxy( p_wk->p_poke[i][j], &img );
      GFL_CLACT_WK_SetPlttOffs( p_wk->p_poke[i][j], POKEICON_GetPalNum(mons_tbl[j] == 0 ? 1: mons_tbl[j], form_tbl[j], FALSE) , CLWK_PLTTOFFS_MODE_OAM_COLOR );
      GFL_HEAP_FreeMemory( p_chr_buff );
    }
  }

  //キャラ置き場をリングバッファのように使うため
  //インデックスを計算
  if( dir == 1 )
  { 
    p_wk->cgr_idx++;
    if( BR_RANK_LIST_LINE - 1 < p_wk->cgr_idx )
    { 
      p_wk->cgr_idx = 0;
    }
  }

  NAGI_Printf( "dir %d cgr%d\n", dir, p_wk->cgr_idx );

  GFL_ARC_CloseDataHandle( p_handle );
}

//----------------------------------------------------------------------------
/**
 *	@brief  バトルビデオをみる  を押した
 *
 *	@param	u32 x X
 *	@param	y     Y
 *
 *	@return TRUEならば押した  FALSEならば押していない
 */
//-----------------------------------------------------------------------------
static BOOL Br_Rank_IsPushBattleBV( u32 x, u32 y )
{ 
	GFL_RECT rect;

	rect.left		= (10)*8;
	rect.right	= (21)*8;
	rect.top		= (4)*8;
	rect.bottom	= (11)*8;

  return ( ((u32)( x - rect.left) <= (u32)(rect.right - rect.left))
            & ((u32)( y - rect.top) <= (u32)(rect.bottom - rect.top)));
}
