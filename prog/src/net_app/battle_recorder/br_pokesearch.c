//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_pokesearch.c
 *	@brief  ポケモン検索
 *	@author	Toru=Nagihashi
 *	@data		2009.11.18
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/gfl_use.h"
#include "system/main.h"  //HEAPID
#include "print/global_msg.h"

//アーカイブ
#include "arc_def.h"
#include "msg/msg_battle_rec.h"
#include "../../../resource/zukan_data/zkn_sort_aiueo_idx.h"
#include "arc\zukan_data.naix"

//自分のモジュール
#include "br_util.h"
#include "br_btn.h"

//外部公開
#include "br_pokesearch.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	デバッグ
//=====================================
#ifdef PM_DEBUG
#define BR_POKESEARCH_ZUKAN_NULL
#endif //PM_DEBUG


//-------------------------------------
///	50音
//=====================================
typedef enum
{
  BR_POKESEARCH_ORDER_A,
  BR_POKESEARCH_ORDER_K,
  BR_POKESEARCH_ORDER_S,
  BR_POKESEARCH_ORDER_T,
  BR_POKESEARCH_ORDER_N,
  BR_POKESEARCH_ORDER_H,
  BR_POKESEARCH_ORDER_M,
  BR_POKESEARCH_ORDER_Y,
  BR_POKESEARCH_ORDER_R,
  BR_POKESEARCH_ORDER_W,

  BR_POKESEARCH_ORDER_MAX,
  BR_POKESEARCH_ORDER_NONE  = BR_POKESEARCH_ORDER_MAX,
} BR_POKESEARCH_ORDER;


//-------------------------------------
///	現在のモード
//=====================================
typedef enum
{
  BR_POKESEARCH_SEQ_HEAD,
  BR_POKESEARCH_SEQ_LIST,
} BR_POKESEARCH_SEQ;


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ポケモン検索ワーク
//=====================================
struct _BR_POKESEARCH_WORK
{
  u32                   seq;
  u32                   next_seq;
  BR_POKESEARCH_SELECT  select;
  BR_POKESEARCH_ORDER   head;
  u32                   select_mons_no;
  u8                    head_order[ BR_POKESEARCH_ORDER_MAX ];
  BOOL                  is_end;

  HEAPID                heapID;
  BR_RES_WORK           *p_res;
  BR_BTN_WORK	          *p_btn;
  BR_FADE_WORK          *p_fade;
  GFL_CLUNIT            *p_unit;
  BMPOAM_SYS_PTR        p_bmpoam;
  PRINT_QUE             *p_que;
  BR_MSGWIN_WORK        *p_msgwin[ BR_POKESEARCH_ORDER_MAX ];

  BMP_MENULIST_DATA     *p_list_data;
  BR_LIST_WORK          *p_list;
  BOOL                  is_start;
  BR_POKESEARCH_SEQ     now_seq;

  const ZUKAN_SAVEDATA  *cp_zkn;
  //ソートされたポケモン番号データ
  u16                   *p_sort_data;
  u32                   sort_len;
  u16                   *p_monsno_buff;
  u32                   monsno_len;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	画面構築
//=====================================
static void BR_POKESEARCH_DISPLAY_CreateCommon( BR_POKESEARCH_WORK *p_wk );
static void BR_POKESEARCH_DISPLAY_DeleteCommon( BR_POKESEARCH_WORK *p_wk );
static void BR_POKESEARCH_DISPLAY_CreateHead( BR_POKESEARCH_WORK *p_wk );
static void BR_POKESEARCH_DISPLAY_DeleteHead( BR_POKESEARCH_WORK *p_wk );
static void BR_POKESEARCH_DISPLAY_CreateList( BR_POKESEARCH_WORK *p_wk );
static void BR_POKESEARCH_DISPLAY_DeleteList( BR_POKESEARCH_WORK *p_wk );

//-------------------------------------
///	選択
//=====================================
static BR_POKESEARCH_ORDER BR_POKESEARCH_HEAD_GetSelect( const BR_POKESEARCH_WORK *cp_wk );

//-------------------------------------
/// 図鑑から取得
//=====================================
static BOOL ZUKANDATA_FindPokemonOrder( const ZUKAN_SAVEDATA *cp_zkn, BR_POKESEARCH_ORDER order, const u16 *cp_50data );
static u16 * ZUKANDATA_AllocMonsNoBuffer( const ZUKAN_SAVEDATA *cp_zkn, BR_POKESEARCH_ORDER order, const u16 *cp_50data, u32 *p_arry_num, HEAPID heapID );
static u16 * ZUKANDATA_AllocSort50onData( HEAPID heapID, u32* p_arry_num );

//=============================================================================
/**
 *          データ
 */
//=============================================================================
//-------------------------------------
///	あかさたなはまやらわ順に並んでいるデータ
//=====================================
static const u16 sc_zukansort_data_idx[]=
{
  ZKN_SORT_AIUEO_IDX_START_A,
  ZKN_SORT_AIUEO_IDX_START_KA,
  ZKN_SORT_AIUEO_IDX_START_SA,
  ZKN_SORT_AIUEO_IDX_START_TA,
  ZKN_SORT_AIUEO_IDX_START_NA,
  ZKN_SORT_AIUEO_IDX_START_HA,
  ZKN_SORT_AIUEO_IDX_START_MA,
  ZKN_SORT_AIUEO_IDX_START_YA,
  ZKN_SORT_AIUEO_IDX_START_RA,
  ZKN_SORT_AIUEO_IDX_START_WA,
  ZKN_SORT_AIUEO_IDX_ALL_END,
};

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン検索  初期化
 *
 *	@param	const ZUKAN_SAVEDATA *cp_zkn  図鑑セーブデータ
 *	@param	*p_res                        リソース管理
 *	@param	heapID                        ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
BR_POKESEARCH_WORK *BR_POKESEARCH_Init( const ZUKAN_SAVEDATA *cp_zkn, BR_RES_WORK *p_res, GFL_CLUNIT *p_unit,  BMPOAM_SYS_PTR p_bmpoam, BR_FADE_WORK *p_fade, HEAPID heapID )
{ 
  BR_POKESEARCH_WORK *p_wk;
  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(BR_POKESEARCH_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(BR_POKESEARCH_WORK) );
  p_wk->heapID  = heapID;
  p_wk->cp_zkn  = cp_zkn;
  p_wk->p_res   = p_res;
  p_wk->p_fade  = p_fade;
  p_wk->p_unit    = p_unit;
  p_wk->p_bmpoam  = p_bmpoam;
  p_wk->p_que     = PRINTSYS_QUE_Create( p_wk->heapID );
  p_wk->p_sort_data  = ZUKANDATA_AllocSort50onData( GFL_HEAP_LOWID(heapID), &p_wk->sort_len );

  //図鑑データから見たことのあるポケモンを検索
  { 
    int i;
    u32 size;

    //頭文字を検索
    for( i = 0; i < BR_POKESEARCH_ORDER_MAX; i++ )
    { 
      p_wk->head_order[ i ] = ZUKANDATA_FindPokemonOrder( p_wk->cp_zkn, i, p_wk->p_sort_data );
    }
  }

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン検索  破棄
 *
 *	@param	BR_POKESEARCH_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_POKESEARCH_Exit( BR_POKESEARCH_WORK *p_wk )
{ 
  if( p_wk->p_monsno_buff )
  { 
    GFL_HEAP_FreeMemory( p_wk->p_monsno_buff );
    p_wk->p_monsno_buff = NULL;
  }

  GFL_HEAP_FreeMemory( p_wk->p_sort_data );
  PRINTSYS_QUE_Delete( p_wk->p_que );
  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン検索  メイン処理
 *
 *	@param	BR_POKESEARCH_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_POKESEARCH_Main( BR_POKESEARCH_WORK *p_wk )
{ 
  enum
  { 
    SEQ_FADEIN_START,
    SEQ_FADEIN_WAIT,

    SEQ_HEAD_SELECT,
    SEQ_LIST_SELECT,

    SEQ_CHANGEOUT_START,
    SEQ_CHANGEOUT_WAIT,
    SEQ_CHANGEIN_START,
    SEQ_CHANGEIN_WAIT,

    SEQ_FADEOUT_START,
    SEQ_FADEOUT_WAIT,
    SEQ_END,
  };


  if( p_wk->is_start )
  { 
    switch( p_wk->seq )
    { 
    case SEQ_FADEIN_START:
      p_wk->seq++;
      break;

    case SEQ_FADEIN_WAIT:
      p_wk->seq = SEQ_HEAD_SELECT;
      break;

    case SEQ_HEAD_SELECT:
      { 
        u32 x, y;
        BOOL cancel;
          
        p_wk->head  = BR_POKESEARCH_HEAD_GetSelect( p_wk );

        cancel  = GFL_UI_TP_GetPointTrg( &x, &y );
        cancel  &= BR_BTN_GetTrg( p_wk->p_btn, x, y );
        if( cancel )
        { 
          //やめた
          p_wk->seq       = SEQ_FADEOUT_START;
          p_wk->select    = BR_POKESEARCH_SELECT_CANCEL;
        }
        else if( p_wk->head != BR_POKESEARCH_ORDER_NONE )
        { 
          //選択した
          NAGI_Printf( "選択 %d\n", p_wk->head );
          p_wk->seq       = SEQ_CHANGEOUT_START;
          p_wk->next_seq  = SEQ_LIST_SELECT;
        }
      }
      break;

    case SEQ_LIST_SELECT:
      {
        u32 x, y;
        BOOL cancel;

        BR_LIST_Main( p_wk->p_list );
        p_wk->select_mons_no  = BR_LIST_GetSelect( p_wk->p_list );

        cancel  = GFL_UI_TP_GetPointTrg( &x, &y );
        cancel  &= BR_BTN_GetTrg( p_wk->p_btn, x, y );
        if( cancel )
        { 
          //キャンセルした
          p_wk->seq       = SEQ_CHANGEOUT_START;
          p_wk->next_seq  = SEQ_HEAD_SELECT;
        }
        else if( p_wk->select_mons_no != BR_LIST_SELECT_NONE )
        { 
          //選択した
          p_wk->seq       = SEQ_FADEOUT_START;
          p_wk->select    = BR_POKESEARCH_SELECT_DECIDE;
        }
      }
      break;

    case SEQ_CHANGEOUT_START:
      BR_FADE_StartFade( p_wk->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_SUB, BR_FADE_DIR_OUT );
      p_wk->seq++;
      break;

    case SEQ_CHANGEOUT_WAIT:
      if( BR_FADE_IsEnd( p_wk->p_fade ) )
      { 
        p_wk->seq++;
      }
      break;

    case SEQ_CHANGEIN_START:
      if( p_wk->next_seq == SEQ_LIST_SELECT )
      { 
        BR_POKESEARCH_DISPLAY_DeleteHead( p_wk );
        GF_ASSERT( p_wk->p_monsno_buff == NULL );
        p_wk->p_monsno_buff = ZUKANDATA_AllocMonsNoBuffer( p_wk->cp_zkn, p_wk->head, p_wk->p_sort_data, &p_wk->monsno_len, p_wk->heapID );
        BR_POKESEARCH_DISPLAY_CreateList( p_wk );
      }
      else
      { 
        BR_POKESEARCH_DISPLAY_DeleteList( p_wk );
        if( p_wk->p_monsno_buff )
        { 
          GFL_HEAP_FreeMemory( p_wk->p_monsno_buff );
          p_wk->p_monsno_buff = NULL;
        }
        BR_POKESEARCH_DISPLAY_CreateHead( p_wk );
      }
      BR_FADE_StartFade( p_wk->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_SUB, BR_FADE_DIR_IN );
      p_wk->seq++;
      break;

    case SEQ_CHANGEIN_WAIT:
      if( BR_FADE_IsEnd( p_wk->p_fade ) )
      { 
        p_wk->seq = p_wk->next_seq;
      }
      break;

    case SEQ_FADEOUT_START:
      p_wk->seq++;
      break;

    case SEQ_FADEOUT_WAIT:
      p_wk->seq++;
      break;

    case SEQ_END:
      p_wk->is_end    = TRUE;
      break;
    }
  }

  { 
    int i;
    for( i = 0; i < BR_POKESEARCH_ORDER_MAX; i++ )
    { 
      if( p_wk->p_msgwin[i] )
      { 
        BR_MSGWIN_PrintMain( p_wk->p_msgwin[i] );
      }
    }
  }

  PRINTSYS_QUE_Main( p_wk->p_que );
}
//----------------------------------------------------------------------------
/**
 *	@brief  開始処理を行う
 *
 *	@param	BR_POKESEARCH_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_POKESEARCH_StartUp( BR_POKESEARCH_WORK *p_wk )
{ 
  
  BR_POKESEARCH_DISPLAY_CreateCommon( p_wk );
  BR_POKESEARCH_DISPLAY_CreateHead( p_wk );

  p_wk->is_start  = TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  終了処理を行う
 *
 *	@param	BR_POKESEARCH_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_POKESEARCH_CleanUp( BR_POKESEARCH_WORK *p_wk )
{ 
  p_wk->is_start  = FALSE;

  if( p_wk->now_seq == BR_POKESEARCH_SEQ_LIST )
  { 
    BR_POKESEARCH_DISPLAY_DeleteList( p_wk );
  }
  else
  { 
    BR_POKESEARCH_DISPLAY_DeleteHead( p_wk );
  }

  BR_POKESEARCH_DISPLAY_DeleteCommon( p_wk );

}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン検索  選択結果受け取り
 *
 *	@param	BR_POKESEARCH_WORK *p_wk  ワーク
 *	@param	*p_mons_no                選んだモンスター番号
 *
 *	@return 選択
 */
//-----------------------------------------------------------------------------
BR_POKESEARCH_SELECT BR_POKESEARCH_GetSelect( const BR_POKESEARCH_WORK *cp_wk, u32 *p_mons_no )
{ 
  if( cp_wk->is_end )
  { 
    if( cp_wk->select == BR_POKESEARCH_SELECT_DECIDE )
    { 
      *p_mons_no  = cp_wk->select_mons_no;
    }
    return cp_wk->select;
  }

  return BR_POKESEARCH_SELECT_NONE;
}

//=============================================================================
/**
 *      画面構築
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  共通素材読み込み
 *
 *	@param	BR_POKESEARCH_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BR_POKESEARCH_DISPLAY_CreateCommon( BR_POKESEARCH_WORK *p_wk )
{ 
  GFL_FONT *p_font;
  GFL_MSGDATA *p_msg; 

  p_font  = BR_RES_GetFont( p_wk->p_res );
  p_msg   = BR_RES_GetMsgData( p_wk->p_res );

  //ボタン作成
  { 
    BOOL ret;
    GFL_CLWK_DATA cldata;
    BR_RES_OBJ_DATA res;


    GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

    cldata.pos_x    = 80;
    cldata.pos_y    = 168;
    cldata.anmseq   = 0;
    cldata.softpri  = 1;

    ret = BR_RES_GetOBJRes( p_wk->p_res, BR_RES_OBJ_SHORT_BTN_S, &res );
    GF_ASSERT( ret );

    p_wk->p_btn = BR_BTN_Init( &cldata, msg_05, BR_BTN_DATA_SHORT_WIDTH,CLSYS_DRAW_SUB, p_wk->p_unit, p_wk->p_bmpoam, p_font, p_msg, &res, p_wk->heapID );
;
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  共通素材破棄
 *
 *	@param	BR_POKESEARCH_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BR_POKESEARCH_DISPLAY_DeleteCommon( BR_POKESEARCH_WORK *p_wk )
{ 
  //ボタン破棄
  { 
    BR_BTN_Exit( p_wk->p_btn );
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  あかさたな画面作成
 *
 *	@param	BR_POKESEARCH_WORK *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void BR_POKESEARCH_DISPLAY_CreateHead( BR_POKESEARCH_WORK *p_wk )
{ 
  GFL_FONT *p_font;
  GFL_MSGDATA *p_msg; 

  p_wk->now_seq   = BR_POKESEARCH_SEQ_HEAD;

  //リソース読み込み
  BR_RES_LoadBG( p_wk->p_res, BR_RES_BG_POKESEARCH_S_HEAD, p_wk->heapID );

  p_font  = BR_RES_GetFont( p_wk->p_res );
  p_msg   = BR_RES_GetMsgData( p_wk->p_res );

  //BMPWIN初期化
  { 
    int i;
    u8  x, y, w, h;
    PRINTSYS_LSB color;
    for( i = 0; i < BR_POKESEARCH_ORDER_MAX; i++ )
    { 
      x   = 3 + (i >= 5 ? 6*(i-5): 6*i);
      y   = 4 + (i >= 5 ? 6: 0);
      w   = 2;
      h   = 2;
      p_wk->p_msgwin[ i ] = BR_MSGWIN_Init( BG_FRAME_S_FONT, x, y, w, h, PLT_BG_S_FONT, p_wk->p_que, p_wk->heapID );
      //色変更
      color = p_wk->head_order[ i ] ? BR_PRINT_COL_NORMAL: BR_PRINT_COL_BLACK;
      //文字描画
      BR_MSGWIN_PrintColor( p_wk->p_msgwin[i], p_msg, msg_idx_000 + i, p_font, color );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  あかさたな画面破棄
 *
 *	@param	BR_POKESEARCH_WORK *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void BR_POKESEARCH_DISPLAY_DeleteHead( BR_POKESEARCH_WORK *p_wk )
{ 
  //BMPWIN破棄
  { 
    int i;
    for( i = 0; i < BR_POKESEARCH_ORDER_MAX; i++ )
    { 
      BR_MSGWIN_Exit( p_wk->p_msgwin[ i ] );
      p_wk->p_msgwin[i] = NULL;
    }
  }
  GFL_BG_LoadScreenReq( BG_FRAME_S_FONT );

  //リソース破棄
  BR_RES_UnLoadBG( p_wk->p_res, BR_RES_BG_POKESEARCH_S_HEAD );
}
//----------------------------------------------------------------------------
/**
 *	@brief  リスト画面作成
 *
 *	@param	BR_POKESEARCH_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BR_POKESEARCH_DISPLAY_CreateList( BR_POKESEARCH_WORK *p_wk )
{ 
  p_wk->now_seq   = BR_POKESEARCH_SEQ_LIST;

  //リソース読み込み
  BR_RES_LoadBG( p_wk->p_res, BR_RES_BG_POKESEARCH_S_LIST, p_wk->heapID );

  //リスト作成
  { 

    GF_ASSERT( p_wk->p_list_data == NULL );
    GF_ASSERT( p_wk->p_list == NULL );
    //リストデータ作成
    { 
      int i;
      p_wk->p_list_data = BmpMenuWork_ListCreate( p_wk->monsno_len, p_wk->heapID );
      for( i = 0; i < p_wk->monsno_len; i++ )
      { 
        BmpMenuWork_ListAddArchiveString( &p_wk->p_list_data[i], (GFL_MSGDATA *)GlobalMsg_PokeName,
            p_wk->p_monsno_buff[i], p_wk->p_monsno_buff[i], p_wk->heapID );
      }
    }
    {
      static const BR_LIST_PARAM sc_list_param  =
      { 
        NULL,
        0,
        8,
        3,
        16,
        12,
        PLT_BG_S_FONT,
        BG_FRAME_S_FONT,
        2,
        BR_LIST_TYPE_TOUCH,
        NULL,
        NULL,
      };
      BR_LIST_PARAM list_param  = sc_list_param;
      list_param.cp_list  = p_wk->p_list_data;
      list_param.list_max = p_wk->monsno_len;
      list_param.p_res    = p_wk->p_res;
      list_param.p_unit   = p_wk->p_unit;

      p_wk->p_list  = BR_LIST_Init( &list_param, p_wk->heapID );
    }
  }


}

//----------------------------------------------------------------------------
/**
 *	@brief  リスト画面破棄
 *
 *	@param	BR_POKESEARCH_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BR_POKESEARCH_DISPLAY_DeleteList( BR_POKESEARCH_WORK *p_wk )
{ 
  //リスト破棄
  { 
    BR_LIST_Exit( p_wk->p_list );
    p_wk->p_list  = NULL;
    BmpMenuWork_ListDelete( p_wk->p_list_data );
    p_wk->p_list_data = NULL;
  }

  //リソース破棄
  BR_RES_UnLoadBG( p_wk->p_res, BR_RES_BG_POKESEARCH_S_LIST );
}
//=============================================================================
/**
 *      選択
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  頭文字を選択
 *
 *	@param	const BR_POKESEARCH_WORK *cp_wk   ワーク
 *
 *	@return 選択したもの
 */
//-----------------------------------------------------------------------------
static BR_POKESEARCH_ORDER BR_POKESEARCH_HEAD_GetSelect( const BR_POKESEARCH_WORK *cp_wk )
{ 
  static const GFL_UI_TP_HITTBL sc_hit_tbl[BR_POKESEARCH_ORDER_MAX+1]  =
  { 
    { 
      3*8, 6*8,   2*8,   5*8,
    },         
    {          
      3*8, 6*8,   8*8,  11*8,
    },         
    {          
      3*8, 6*8,   14*8, 17*8,
    },         
    {          
      3*8, 6*8,   20*8, 23*8,
    },         
    {          
      3*8 , 6*8,  26*8, 29*8,
    },         
    //         
    {          
      9*8,12*8,    2*8,  5*8,
    },         
    {          
      9*8,12*8,    8*8, 11*8,
    },         
    {          
      9*8,12*8,   14*8, 17*8,
    },         
    {          
      9*8,12*8,   20*8, 23*8,
    },         
    {          
      9*8,12*8,   26*8, 29*8,
    },
    { 
      GFL_UI_TP_HIT_END,GFL_UI_TP_HIT_END,GFL_UI_TP_HIT_END,GFL_UI_TP_HIT_END,
    }
  };

  int ret;

  ret = GFL_UI_TP_HitTrg( sc_hit_tbl );

  if( ret != GFL_UI_TP_HIT_NONE )
  { 
    if( cp_wk->head_order[ ret ] )
    { 
      return ret;
    }
  }

  return BR_POKESEARCH_ORDER_NONE;
}

//=============================================================================
/**
 *      図鑑データ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  図鑑から50音のポケモンがいるか検知
 *
 *	@param	const ZUKAN_SAVEDATA *cp_zkn  図鑑セーブデータ
 *	@param	order     50音ンデックス
 *
 *	@return TRUEの50音順のポケモンがいる  FALSEでいない
 */
//-----------------------------------------------------------------------------
static BOOL ZUKANDATA_FindPokemonOrder( const ZUKAN_SAVEDATA *cp_zkn, BR_POKESEARCH_ORDER order, const u16 *cp_50data )
{ 
  int i;
  u32 size;
  u32 num, max;
  BOOL ret;
  BOOL is_found;

  num = sc_zukansort_data_idx[ order ];
  max = sc_zukansort_data_idx[ order+1 ];

  //頭文字を検索する
  ret = FALSE;
  for( i = num; i < max; i++ )
  { 
#ifdef BR_POKESEARCH_ZUKAN_NULL
    is_found  = TRUE;
#else
    is_found = ZUKANSAVE_GetPokeSeeFlag( cp_zkn, cp_50data[ i ] );
#endif
    if( is_found)
    { 
      ret = TRUE;
    }
  }

  return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  見たことがあるポケモン番号の配列をアロケート
 *
 *	@param	const ZUKAN_SAVEDATA *cp_zkn    図鑑セーブデータ
 *	@param	order                           50音
 *	@param  *p_arry_num                     配列数
 *	@param	heapID                          ヒープID
 *
 *	@return アロケートされたポケモン番号が入ったバッファ
 */
//-----------------------------------------------------------------------------
static u16 * ZUKANDATA_AllocMonsNoBuffer( const ZUKAN_SAVEDATA *cp_zkn, BR_POKESEARCH_ORDER order, const u16 *cp_50data, u32 *p_arry_num, HEAPID heapID )
{ 
  int i;
  u16 *p_buff;
  u32 size;
  u32 num, max;
  u32 cnt;
  BOOL is_found;

  num = sc_zukansort_data_idx[ order ];
  max = sc_zukansort_data_idx[ order+1 ];
  
  //数を検索
  cnt = 0;
  for( i = num; i < max; i++ )
  { 
#ifdef BR_POKESEARCH_ZUKAN_NULL
    is_found  = TRUE;
#else
    is_found = ZUKANSAVE_GetPokeSeeFlag( cp_zkn, cp_50data[ i ] );
#endif
    if( is_found )
    {
      cnt++;
    }
  }

  //バッファ作成
  p_buff  = GFL_HEAP_AllocMemory( heapID, sizeof(u16) * cnt );
  GFL_STD_MemClear( p_buff, sizeof(u16) * cnt );

  //バッファにデータを設定
  cnt = 0;
  for( i = num; i < max; i++ )
  { 
#ifdef BR_POKESEARCH_ZUKAN_NULL
    is_found  = TRUE;
#else
    is_found = ZUKANSAVE_GetPokeSeeFlag( cp_zkn, cp_50data[ i ] );
#endif
    if( is_found )
    {
      p_buff[ cnt ] = cp_50data[ i ];
      cnt++;
    }
  }

  //配列数を返す
  if( p_arry_num )
  { 
    *p_arry_num = cnt;
  }

  return p_buff;
}

//----------------------------------------------------------------------------
/**
 *	@brief  図鑑の50音順のインデックスデータを取得
 *
 *	@param	HEAPID heapID ヒープID
 *	@param	p_arry_num    配列数
 *
 *	@return アロケートされたバッファ
 */
//-----------------------------------------------------------------------------
static u16 * ZUKANDATA_AllocSort50onData( HEAPID heapID, u32* p_arry_num )
{
	u32 size;
  u16 *p_buff;

  p_buff = GFL_ARC_UTIL_LoadEx( ARCID_ZUKAN_DATA, NARC_zukan_data_zkn_sort_aiueo_dat, FALSE, heapID, &size );
	*p_arry_num = size / sizeof( u16 );

	return p_buff;
}
