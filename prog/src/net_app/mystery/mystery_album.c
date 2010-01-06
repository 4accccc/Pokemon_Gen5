//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		mystery_album.c
 *	@brief
 *	@author Toru=Nagihashi
 *	@data		2009.12.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/main.h"
#include "system/gfl_use.h"
#include "sound/pm_sndsys.h"
#include "system/bmp_winframe.h"
#include "pokeicon/pokeicon.h"
#include "item/item.h"
#include "system/poke2dgra.h"

//アーカイブ
#include "arc_def.h"
#include "item_icon.naix"
#include "arc\mystery.naix"
#include "msg/msg_mystery.h"

//自分のモジュール
#include "mystery_util.h"

//外部公開
#include "mystery_album.h"
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  アルバム
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//=============================================================================
/**
 *          定数
 */
//=============================================================================
#define MYSTERY_ALBUM_CARD_MAX         GIFT_DATA_MAX 
#define MYSTERY_ALBUM_PAGE_MAX          3
#define MYSTERY_ALBUM_PAGE_IN_CARD_NUM  (MYSTERY_ALBUM_CARD_MAX/MYSTERY_ALBUM_PAGE_MAX)


#define MYSTERY_ALBUM_LIST_FRM        GFL_BG_FRAME0_M
#define MYSTERY_ALBUM_FONT_FRM        GFL_BG_FRAME1_M
#define MYSTERY_ALBUM_CARD_FRM        GFL_BG_FRAME2_M
#define MYSTERY_ALBUM_BACK_FRM        GFL_BG_FRAME3_M


#define MYSTERY_ALBUM_CARD_FONT_S       GFL_BG_FRAME0_S
#define MYSTERY_ALBUM_CARD_FRM_S        GFL_BG_FRAME2_S

#define MYSTERY_ALBUM_BG_FRM_S_PLT          13
#define MYSTERY_ALBUM_BG_BACK_S_PLT         14
#define MYSTERY_ALBUM_BG_FONT_S_PLT         15
#define MYSTERY_ALBUM_OBJ_ICON_S_PLT        11
#define MYSTERY_ALBUM_OBJ_SILHOUETTE_S_PLT  14

#define MYSTERY_ALBUM_BG_BACK_PLT         4
#define MYSTERY_ALBUM_FONT_PLT            15

#define MYSTERY_ALBUM_OBJ_CURSOR_PLT      3
#define MYSTERY_ALBUM_OBJ_ICON_PLT        4
#define MYSTERY_ALBUM_OBJ_POKEICON_PLT    12

//-------------------------------------
///	カーソル
//=====================================
typedef enum
{
  MYSTERY_CURSOR_LEFT_UP,
  MYSTERY_CURSOR_RIGHT_UP,
  MYSTERY_CURSOR_LEFT_DOWN,
  MYSTERY_CURSOR_RIGHT_DOWN,

  MYSTERY_CURSOR_MAX,
  MYSTERY_CURSOR_RETURN = MYSTERY_CURSOR_MAX,
} MYSTERY_CURSOR_POS;

//ｘがminより小さいならばmax ｘがmaxより大きいならばmin　さもなくばx
#define MATH_ROUND(x,min,max) (x>max?min:x<min?max:x)

//-------------------------------------
///	移動
//=====================================
#define MYSTERY_ALBUM_MOVE_SCROLL_DISTANCE  256
#define MYSTERY_ALBUM_MOVE_SCROLL_SYNC      16


//=============================================================================
/**
 *          構造体
 */
//=============================================================================
//-------------------------------------
///	カード情報
//=====================================
typedef struct 
{
  BOOL            is_exist;
  GIFT_PACK_DATA  *p_data;
  GFL_BMP_DATA    *p_bmp;
  u32             res_cgx;
  u32             res_cel;
} MYSTERY_CARD_DATA;

//-------------------------------------
///	アルバム構造体
//=====================================
struct _MYSTERY_ALBUM_WORK
{ 
  MYSTERY_ALBUM_SETUP setup;
  HEAPID              heapID;
  u16                 dummy;
  MYSTERY_CARD_DATA   data[ MYSTERY_ALBUM_CARD_MAX ];

  BOOL                    is_card_update;
  MYSTERY_CARD_WORK       *p_card;
  MYSTERY_MSGWIN_WORK     *p_page_win[2];
  MYSTERY_MSGWINSET_WORK  *p_winset;

  //テキスト
  MYSTERY_TEXT_WORK         *p_text; 

  //選択肢
  MYSTERY_LIST_WORK         *p_list;

  MYSTERY_SEQ_WORK    *p_seq;        //シーケンス管理
  s32                 now_page;   //現在のページ
  MYSTERY_CURSOR_POS  cursor;     //カーソル
  BOOL                is_left;    //ページ書き込み場所
  BOOL                is_change;  //変更フラグ
  u32                 scroll_cnt;
  s8                  scroll_req; //1だと右-1だと左0だと動かない
  u8                  dummy2;
  u16                 swap_card_index;
  BOOL                is_swap;
  BOOL                key_btn_save; //もどるボタンにきたときの前のボタンを記憶  TRUEなら左 FALSEなら右

  GFL_CLWK            *p_cursor;  //カーソル
  GFL_CLWK            *p_swap_cursor;  //カーソル(swap)
  u32                 res_cursor_cgx;
  u32                 res_cursor_cel;
  u32                 res_cursor_plt;

  GFL_BMPWIN          *p_bmpwin[MYSTERY_CURSOR_MAX*2];  //表と裏
  GFL_CLWK            *p_icon[ MYSTERY_CURSOR_MAX*2 ];  //コピーされたポインタ
  u32                 res_cgx[MYSTERY_CURSOR_MAX*2];
  u32                 res_cel[MYSTERY_CURSOR_MAX*2];
  u32                 res_icon_plt[MYSTERY_CURSOR_MAX*2];//8本
  u32                 res_poke_icon_plt;    //3本
};

//=============================================================================
/**
 *          プロトタイプ
 */
//=============================================================================
//-------------------------------------
///	アルバムプライベート
//=====================================
static void Mystery_Album_InitDisplay( MYSTERY_ALBUM_WORK *p_wk, HEAPID heapID );
static void Mystery_Album_ExitDisplay( MYSTERY_ALBUM_WORK *p_wk );
static void Mystery_Album_CreateDisplay( MYSTERY_ALBUM_WORK *p_wk, BOOL is_front, BOOL is_left, u32 page, HEAPID heapID );
static void Mystery_Album_DeleteDisplay( MYSTERY_ALBUM_WORK *p_wk, BOOL is_front );
static void Mystery_Album_StartScrollPage( MYSTERY_ALBUM_WORK *p_wk, BOOL is_left );
static BOOL Mystery_Album_ScrollPage( MYSTERY_ALBUM_WORK *p_wk );
static void Mystery_Album_RemoveCard( MYSTERY_ALBUM_WORK *p_wk, u32 card_index );
static void Mystery_Album_SwapCard( MYSTERY_ALBUM_WORK *p_wk, u32 card_index1, u32 card_index2 );
static u32 Mystery_Album_GetPageMax( const MYSTERY_ALBUM_WORK *cp_wk );
static u32 Mystery_Album_GetDataNum( const MYSTERY_ALBUM_WORK *cp_wk );
static inline u32 Mystery_Album_GetCardIndex( u32 page, u32 cursor )
{ 
  return page * MYSTERY_CURSOR_MAX + cursor;
}
static inline u32 Mystery_Album_GetNowCardIndex( const MYSTERY_ALBUM_WORK *cp_wk )
{ 
  return Mystery_Album_GetCardIndex( cp_wk->now_page, cp_wk->cursor );
}

//-------------------------------------
///	カードデータ
//=====================================
static void MYSTERY_CARD_DATA_Init( MYSTERY_CARD_DATA *p_wk, GIFT_PACK_DATA *p_data, const MYSTERY_ALBUM_SETUP *cp_setup, HEAPID heapID );
static void MYSTERY_CARD_DATA_Exit( MYSTERY_CARD_DATA *p_wk );
static BOOL MYSTERY_CARD_DATA_IsExist( const MYSTERY_CARD_DATA *cp_wk );
static const GFL_BMP_DATA *MYSTERY_CARD_DATA_GetBmp( const MYSTERY_CARD_DATA *cp_wk );
static ARCHANDLE * MYSTERY_CARD_DATA_GetArcHandle( const MYSTERY_CARD_DATA *cp_wk, HEAPID heapID );
static u32 MYSTERY_CARD_DATA_GetResPlt( const MYSTERY_CARD_DATA *cp_wk );
static u32 MYSTERY_CARD_DATA_GetPltOfs( const MYSTERY_CARD_DATA *cp_wk );
static u32 MYSTERY_CARD_DATA_GetResCgx( const MYSTERY_CARD_DATA *cp_wk );
static u32 MYSTERY_CARD_DATA_GetResCel( const MYSTERY_CARD_DATA *cp_wk );
static u32 MYSTERY_CARD_DATA_GetResAnm( const MYSTERY_CARD_DATA *cp_wk );
static u32 MYSTERY_CARD_DATA_GetType( const MYSTERY_CARD_DATA *cp_wk );
static GIFT_PACK_DATA *MYSTERY_CARD_DATA_GetGiftBackData( const MYSTERY_CARD_DATA *cp_wk );

//-------------------------------------
///	SEQFUNC
//=====================================
static void SEQFUNC_MoveCursor( MYSTERY_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_SelectList( MYSTERY_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_DeleteCard( MYSTERY_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_SwapCard( MYSTERY_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_End( MYSTERY_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//=============================================================================
/**
 *          データ
 */
//=============================================================================
//-------------------------------------
///	上画面サムネイルカード位置データ
//=====================================
typedef struct
{ 
  u8 card_x;  //カード開始座標Xキャラ単位
  u8 card_y;  //カード開始座標Yキャラ単位 
  u8 date_x;  //日付用BMPWIN開始Xキャラ単位
  u8 date_y;  //日付用BMPWIN開始Yキャラ単位
  s16 icon_x; //アイコン座標X
  s16 icon_y; //アイコン座標Y
} MYSTERY_ALBUM_THUMBNAIL_DATA;
static const MYSTERY_ALBUM_THUMBNAIL_DATA sc_thumbnail_data[MYSTERY_CURSOR_MAX] =
{ 
  //左上
  { 
    3,
    4,
    4,
    8,
    96,
    48,
  },
  //右上
  { 
    18,
    4,
    19,
    8,
    216,
    48,
  },
  //左下
  { 
    3,
    12,
    4,
    16,
    96,
    112,
  },
  //右下
  { 
    18,
    12,
    19,
    16,
    216,
    112,
  },
};
#define MYSTERY_ALBUM_CARD_BACKSURFACE_OFS  256
#define MYSTERY_ALBUM_THUMBNAIL_CARD_W      12
#define MYSTERY_ALBUM_THUMBNAIL_CARD_H      7
#define MYSTERY_ALBUM_THUMBNAIL_DATE_W      10
#define MYSTERY_ALBUM_THUMBNAIL_DATE_H      2

#define MYSTERY_ALBUM_CARD_WRITE_SRC_START_X      1
#define MYSTERY_ALBUM_CARD_WRITE_SRC_START_Y      1
#define MYSTERY_ALBUM_CARD_WRITE_SRC_W            32


//=============================================================================
/**
 *          パブリック
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  アルバム作成
 *
 *	@param	const MYSTERY_ALBUM_SETUP *cp_setup 設定構造体
 *	@param	heapID                              ヒープID 
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
MYSTERY_ALBUM_WORK * MYSTERY_ALBUM_Init( const MYSTERY_ALBUM_SETUP *cp_setup, HEAPID heapID )
{ 
  MYSTERY_ALBUM_WORK *  p_wk;

  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(MYSTERY_ALBUM_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(MYSTERY_ALBUM_WORK) );
  p_wk->is_left = TRUE;
  p_wk->setup   = *cp_setup;
  p_wk->heapID  = heapID;

  GFL_STD_MemFill32( p_wk->res_cgx, GFL_CLGRP_REGISTER_FAILED, MYSTERY_CURSOR_MAX*2*sizeof(u32) );
  GFL_STD_MemFill32( p_wk->res_icon_plt, GFL_CLGRP_REGISTER_FAILED, MYSTERY_CURSOR_MAX*2*sizeof(u32) );
  GFL_STD_MemFill32( p_wk->res_cel, GFL_CLGRP_REGISTER_FAILED, MYSTERY_CURSOR_MAX*2*sizeof(u32) );

  //カード初期化
  { 
    int i;
    GIFT_PACK_DATA *p_data;
    for( i = 0; i < MYSTERY_ALBUM_CARD_MAX; i++ )
    {
      if( MYSTERYDATA_IsExistsCard( p_wk->setup.p_sv, i) )
      { 
        p_data  = MYSTERYDATA_GetCardData( p_wk->setup.p_sv, i );
        MYSTERY_CARD_DATA_Init( &p_wk->data[i], p_data, &p_wk->setup, heapID );
      }
    }   
  }

  p_wk->p_seq = MYSTERY_SEQ_Init( p_wk, SEQFUNC_MoveCursor, heapID );

  Mystery_Album_InitDisplay( p_wk, heapID );
  Mystery_Album_CreateDisplay( p_wk, TRUE, 0, p_wk->now_page, heapID );


  //WINSET
  { 
    const MYSTERY_MSGWINSET_SETUP_TBL c_setup_tbl[] =
    { 
      { 
        1,
        1,
        30,
        2,
        syachi_mystery_album_001
      },
      {
        1,
        0x15,
        11,
        2,
        syachi_mystery_album_002,
      },
      { 
        0x18,
        0x15,
        8,
        4,
        syachi_mystery_album_003,
      },
    };
    p_wk->p_winset  = MYSTERY_MSGWINSET_Init( c_setup_tbl, NELEMS(c_setup_tbl), MYSTERY_ALBUM_FONT_FRM, MYSTERY_ALBUM_FONT_PLT, p_wk->setup.p_que, p_wk->setup.p_msg, p_wk->setup.p_font, heapID );
  }

  //下画面カード
  if( Mystery_Album_GetDataNum(p_wk) != 0 )
  {
    const u32 card_index = Mystery_Album_GetNowCardIndex( p_wk );
    const MYSTERY_CARD_DATA *p_data = &p_wk->data[ card_index ];

    { 
      MYSTERY_CARD_SETUP setup;
      GFL_STD_MemClear( &setup, sizeof(MYSTERY_CARD_SETUP) );

      setup.p_data            = MYSTERY_CARD_DATA_GetGiftBackData( p_data );
      setup.back_frm          = MYSTERY_ALBUM_CARD_FRM_S;
      setup.font_frm          = MYSTERY_ALBUM_CARD_FONT_S;
      setup.back_plt_num      = MYSTERY_ALBUM_BG_BACK_S_PLT;
      setup.font_plt_num      = MYSTERY_ALBUM_BG_FONT_S_PLT;
      setup.icon_obj_plt_num  = MYSTERY_ALBUM_OBJ_ICON_S_PLT; 
      setup.silhouette_obj_plt_num  = MYSTERY_ALBUM_OBJ_SILHOUETTE_S_PLT;
      setup.p_clunit          = p_wk->setup.p_clunit;
      setup.p_sv              = p_wk->setup.p_sv;
      setup.p_msg             = p_wk->setup.p_msg; 
      setup.p_font            = p_wk->setup.p_font; 
      setup.p_que             = p_wk->setup.p_que; 
      setup.p_word            = p_wk->setup.p_word;
      p_wk->p_card  = MYSTERY_CARD_Init( &setup, heapID );
    }
  }

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  アルバム破棄
 *
 *	@param	MYSTERY_ALBUM_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
void MYSTERY_ALBUM_Exit( MYSTERY_ALBUM_WORK *p_wk )
{ 
  if( p_wk->p_card )
  { 
    MYSTERY_CARD_Exit( p_wk->p_card );
      
    GFL_BG_ClearScreen( MYSTERY_ALBUM_CARD_FONT_S );
    GFL_BG_ClearScreen( MYSTERY_ALBUM_CARD_FRM_S );
    GFL_BG_LoadScreenReq( MYSTERY_ALBUM_CARD_FONT_S );
    GFL_BG_LoadScreenReq( MYSTERY_ALBUM_CARD_FRM_S );
    GFL_BG_SetVisible( MYSTERY_ALBUM_CARD_FONT_S, TRUE );
    GFL_BG_SetVisible( MYSTERY_ALBUM_CARD_FRM_S, TRUE );
    
    p_wk->p_card  = NULL;
  }

  if( p_wk->p_text )
  { 
    MYSTERY_TEXT_Exit( p_wk->p_text );
    p_wk->p_text  = NULL;
  }

  GFL_BG_SetScroll( MYSTERY_ALBUM_BACK_FRM, GFL_BG_SCROLL_X_SET, 0 );
  GFL_BG_SetScroll( MYSTERY_ALBUM_CARD_FRM, GFL_BG_SCROLL_X_SET, 0 );

  MYSTERY_MSGWINSET_Exit( p_wk->p_winset );

  Mystery_Album_DeleteDisplay( p_wk, TRUE );
  Mystery_Album_DeleteDisplay( p_wk, FALSE );
  Mystery_Album_ExitDisplay( p_wk );

  //カード破棄
  { 
    int i;
    for( i = 0; i < MYSTERY_ALBUM_CARD_MAX; i++ )
    {
      MYSTERY_CARD_DATA_Exit( &p_wk->data[i]);
    }   
  }

  MYSTERY_SEQ_Exit( p_wk->p_seq );
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  アルバム  メイン処理
 *
 *	@param	MYSTERY_ALBUM_WORK *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void MYSTERY_ALBUM_Main( MYSTERY_ALBUM_WORK *p_wk )
{ 
  MYSTERY_SEQ_Main( p_wk->p_seq );
  MYSTERY_MSGWINSET_PrintMain( p_wk->p_winset );
  if( p_wk->p_page_win[0] )
  { 
    MYSTERY_MSGWIN_PrintMain( p_wk->p_page_win[0] );
  }
  if( p_wk->p_page_win[1] )
  { 
    MYSTERY_MSGWIN_PrintMain( p_wk->p_page_win[1] );
  }


  if( p_wk->is_card_update )
  { 
    MYSTERY_CARD_DATA *p_data;

    if( p_wk->p_card )
    { 
      GFL_BG_SetVisible( MYSTERY_ALBUM_CARD_FRM_S, FALSE );
      GFL_BG_SetVisible( MYSTERY_ALBUM_CARD_FONT_S, FALSE );
      MYSTERY_CARD_Exit( p_wk->p_card );
      p_wk->p_card  = NULL;
    }

    if( p_wk->cursor < MYSTERY_CURSOR_MAX )
    { 
      p_data = &p_wk->data[ Mystery_Album_GetNowCardIndex( p_wk ) ];
      if( MYSTERY_CARD_DATA_IsExist( p_data ) )
      { 
        { 
          MYSTERY_CARD_SETUP setup;
          GFL_STD_MemClear( &setup, sizeof(MYSTERY_CARD_SETUP) );

          setup.p_data            = MYSTERY_CARD_DATA_GetGiftBackData( p_data );
          setup.back_frm          = MYSTERY_ALBUM_CARD_FRM_S;
          setup.font_frm          = MYSTERY_ALBUM_CARD_FONT_S;
          setup.back_plt_num      = MYSTERY_ALBUM_BG_BACK_S_PLT;
          setup.font_plt_num      = MYSTERY_ALBUM_BG_FONT_S_PLT;
          setup.icon_obj_plt_num  = MYSTERY_ALBUM_OBJ_ICON_S_PLT; 
          setup.silhouette_obj_plt_num  = MYSTERY_ALBUM_OBJ_SILHOUETTE_S_PLT;
          setup.p_clunit          = p_wk->setup.p_clunit;
          setup.p_sv              = p_wk->setup.p_sv;
          setup.p_msg             = p_wk->setup.p_msg; 
          setup.p_font            = p_wk->setup.p_font; 
          setup.p_que             = p_wk->setup.p_que; 
          setup.p_word            = p_wk->setup.p_word;
          p_wk->p_card  = MYSTERY_CARD_Init( &setup, p_wk->heapID );
        }
      }
    }
    p_wk->is_card_update  = FALSE;
  }
  if( p_wk->p_card )
  { 
    MYSTERY_CARD_Main( p_wk->p_card );
  }


}
//----------------------------------------------------------------------------
/**
 *	@brief  アルバム  終了取得
 *
 *	@param	const MYSTERY_ALBUM_WORK *cp_wk   ワーク
 *
 *	@return TRUEでアルバム終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
BOOL MYSTERY_ALBUM_IsEnd( const MYSTERY_ALBUM_WORK *cp_wk )
{ 
  return MYSTERY_SEQ_IsEnd( cp_wk->p_seq );
}
//=============================================================================
/**
 *  アルバムプライベート
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  画面初期化
 *
 *	@param	MYSTERY_ALBUM_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Mystery_Album_InitDisplay( MYSTERY_ALBUM_WORK *p_wk, HEAPID heapID )
{ 
  int i;
  MYSTERY_ALBUM_THUMBNAIL_DATA  data;


  //カード作成
  for( i = 0; i < MYSTERY_CURSOR_MAX*2; i++ )
  { 

    //貼る面が右側だったらオフセットでずらす
    if( i >= MYSTERY_CURSOR_MAX )
    { 
      //データ取得
      data  = sc_thumbnail_data[i-MYSTERY_CURSOR_MAX];

      data.card_x += MYSTERY_ALBUM_CARD_BACKSURFACE_OFS/8;
      data.date_x += MYSTERY_ALBUM_CARD_BACKSURFACE_OFS/8;
    }
    else
    { 
      //データ取得
      data  = sc_thumbnail_data[i];
    }

    //作成
    p_wk->p_bmpwin[i] = GFL_BMPWIN_Create( MYSTERY_ALBUM_CARD_FRM, 
        data.date_x, data.date_y, MYSTERY_ALBUM_THUMBNAIL_DATE_W, MYSTERY_ALBUM_THUMBNAIL_DATE_H,
        MYSTERY_ALBUM_FONT_PLT, GFL_BMP_CHRAREA_GET_B );
    GFL_BMPWIN_MakeScreen( p_wk->p_bmpwin[i] );
  }

  { 

    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_MYSTERY, heapID );
    //BG
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_mystery_fushigi_album_NCLR,
				PALTYPE_MAIN_BG, MYSTERY_ALBUM_BG_BACK_PLT*0x20, 0x20, heapID );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_mystery_fushigi_album_NCGR, 
				MYSTERY_ALBUM_BACK_FRM, 0, 0, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_mystery_fushigi_album_NSCR,
				MYSTERY_ALBUM_BACK_FRM, 0, 0, FALSE, heapID );
    GFL_BG_ChangeScreenPalette( MYSTERY_ALBUM_BACK_FRM, 0, 0,64,24, MYSTERY_ALBUM_BG_BACK_PLT );
    GFL_BG_LoadScreenReq( MYSTERY_ALBUM_BACK_FRM );

    GFL_ARC_CloseDataHandle( p_handle );

  }
  { 
    //キャラを単位で読み込み
    //BG_FRAME_M_TEXT
   // GFL_BG_FillCharacter( MYSTERY_ALBUM_LIST_FRM, 0, 1, 0 );
    BmpWinFrame_GraphicSet( MYSTERY_ALBUM_LIST_FRM, 1, MYSTERY_ALBUM_BG_FRM_S_PLT, MENU_TYPE_SYSTEM, heapID );
  }

  { 
    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_POKEICON, heapID );
    //共通ポケアイコン用パレット
    p_wk->res_poke_icon_plt = GFL_CLGRP_PLTT_RegisterComp( p_handle, 
				POKEICON_GetPalArcIndex(), CLSYS_DRAW_MAIN, MYSTERY_ALBUM_OBJ_POKEICON_PLT*0x20, heapID );
    GFL_ARC_CloseDataHandle( p_handle );
  }

  //ページ作成( 0/3 )
  p_wk->p_page_win[0]  = MYSTERY_MSGWIN_Init( MYSTERY_ALBUM_CARD_FRM, 0xD, 0x15, 6, 2, MYSTERY_ALBUM_FONT_PLT, p_wk->setup.p_que, heapID );
  p_wk->p_page_win[1]  = MYSTERY_MSGWIN_Init( MYSTERY_ALBUM_CARD_FRM, 0xD+MYSTERY_ALBUM_CARD_BACKSURFACE_OFS/8, 0x15, 6, 2, MYSTERY_ALBUM_FONT_PLT, p_wk->setup.p_que, heapID );

  //カーソル作成
  { 
    { 
      ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_MYSTERY, heapID );
      p_wk->res_cursor_plt  = GFL_CLGRP_PLTT_RegisterEx( p_handle, 
        NARC_mystery_album_cursor_NCLR, CLSYS_DRAW_MAIN, MYSTERY_ALBUM_OBJ_CURSOR_PLT*0x20, 0, 1, heapID );
      p_wk->res_cursor_cel	= GFL_CLGRP_CELLANIM_Register( p_handle,
          NARC_mystery_album_cursor_NCER, NARC_mystery_album_cursor_NANR, heapID );
      p_wk->res_cursor_cgx	= GFL_CLGRP_CGR_Register( p_handle,
              NARC_mystery_album_cursor_NCGR, FALSE, CLSYS_DRAW_MAIN, heapID );
      GFL_ARC_CloseDataHandle( p_handle );
    }
    { 
      GFL_CLWK_DATA cldata;
      GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
      cldata.pos_x  = sc_thumbnail_data[0].card_x*8;
      cldata.pos_y  = sc_thumbnail_data[0].card_y*8;
      cldata.bgpri  = MYSTERY_ALBUM_FONT_FRM;

      p_wk->p_cursor    = GFL_CLACT_WK_Create( p_wk->setup.p_clunit, 
          p_wk->res_cursor_cgx, p_wk->res_cursor_plt, p_wk->res_cursor_cel,
          &cldata, CLSYS_DEFREND_MAIN, heapID );

      p_wk->p_swap_cursor    = GFL_CLACT_WK_Create( p_wk->setup.p_clunit, 
          p_wk->res_cursor_cgx, p_wk->res_cursor_plt, p_wk->res_cursor_cel,
          &cldata, CLSYS_DEFREND_MAIN, heapID );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_swap_cursor, FALSE );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  画面破棄
 *
 *	@param	MYSTERY_ALBUM_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Mystery_Album_ExitDisplay( MYSTERY_ALBUM_WORK *p_wk )
{ 
  //GFL_BG_FillCharacterRelease( MYSTERY_ALBUM_LIST_FRM, 1, 0 );

  //カーソル破棄
  { 
    GFL_CLACT_WK_Remove( p_wk->p_swap_cursor );
    GFL_CLACT_WK_Remove( p_wk->p_cursor );
    GFL_CLGRP_CGR_Release( p_wk->res_cursor_cgx );
    GFL_CLGRP_CELLANIM_Release( p_wk->res_cursor_cel );
    GFL_CLGRP_PLTT_Release( p_wk->res_cursor_plt );
  }

  MYSTERY_MSGWIN_Exit( p_wk->p_page_win[0] );
  p_wk->p_page_win[0]  = NULL;
  MYSTERY_MSGWIN_Exit( p_wk->p_page_win[1] );
  p_wk->p_page_win[1]  = NULL;
  GFL_CLGRP_PLTT_Release( p_wk->res_poke_icon_plt );

  { 
    int i;
    for( i = 0; i < MYSTERY_CURSOR_MAX*2; i++ )
    {
      GFL_BMPWIN_Delete( p_wk->p_bmpwin[i] );
    }
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  画面作成  
 *
 *	@param	MYSTERY_ALBUM_WORK *p_wk  ワーク
 *	@param	is_front                  表につくるか、裏に作るか
 *	@param	page                      作成するページ
 */
//-----------------------------------------------------------------------------
static void Mystery_Album_CreateDisplay( MYSTERY_ALBUM_WORK *p_wk, BOOL is_front, BOOL is_left, u32 page, HEAPID heapID )
{
  int i;
  MYSTERY_CARD_DATA *p_src;
  MYSTERY_ALBUM_THUMBNAIL_DATA  data;
  u32 dst_idx;

  for( i = 0; i < MYSTERY_CURSOR_MAX; i++ )
  { 
    //カードデータ取得
    p_src = &p_wk->data[ Mystery_Album_GetCardIndex( page, i ) ];

    //カードの中身があったら貼付け
    if( MYSTERY_CARD_DATA_IsExist( p_src ) )
    { 
      //貼り付ける位置を取得
      dst_idx = i;

      //座標データ取得
      data  = sc_thumbnail_data[i];

      //貼る面が右側だったらオフセットでずらす
      if( (is_front && !p_wk->is_left)
          || (!is_front && p_wk->is_left) )
      { 
        dst_idx += MYSTERY_CURSOR_MAX;

        data.card_x += MYSTERY_ALBUM_CARD_BACKSURFACE_OFS/8;
        data.date_x += MYSTERY_ALBUM_CARD_BACKSURFACE_OFS/8;
      }

      //カードのキャラをスクリーンに貼り付ける
      { 
        int i,j;
        u16 chr_num;
        for( i = 0; i < MYSTERY_ALBUM_THUMBNAIL_CARD_H; i++ )
        { 
          for( j = 0; j < MYSTERY_ALBUM_THUMBNAIL_CARD_W; j++ )
          { 
            chr_num = MYSTERY_ALBUM_CARD_WRITE_SRC_START_X + 
              MYSTERY_ALBUM_CARD_WRITE_SRC_START_Y * MYSTERY_ALBUM_CARD_WRITE_SRC_W
              + i * MYSTERY_ALBUM_CARD_WRITE_SRC_W + j;

            GFL_BG_FillScreen( MYSTERY_ALBUM_BACK_FRM, chr_num,
                data.card_x + j, data.card_y + i, 1, 1, GFL_BG_SCRWRT_PALNL );
          }
        }
      }

      //日付を貼り付ける
      { 
        const GFL_BMP_DATA  *cp_src  = MYSTERY_CARD_DATA_GetBmp( p_src );
        GFL_BMP_DATA  *p_dst  = GFL_BMPWIN_GetBmp( p_wk->p_bmpwin[dst_idx] );
        GFL_BMP_Copy( cp_src, p_dst );
        GFL_BMPWIN_TransVramCharacter( p_wk->p_bmpwin[dst_idx] );
      }

      //アイコンを作成する
      //  日付のように、作成しておいて、設定するだけにしたかったのだが、
      //  リソースの関係でやむをえずここで作成する
      { 
        //リソース読みこみ
        {	
          ARCHANDLE	*	p_handle	= MYSTERY_CARD_DATA_GetArcHandle( p_src, heapID );

          if( MYSTERY_CARD_DATA_GetType(p_src) != MYSTERYGIFT_TYPE_POKEMON )
          { 
            p_wk->res_icon_plt[ dst_idx ]	= GFL_CLGRP_PLTT_RegisterEx( p_handle, 
                MYSTERY_CARD_DATA_GetResPlt(p_src), CLSYS_DRAW_MAIN, (MYSTERY_ALBUM_OBJ_ICON_PLT+dst_idx)*0x20, 0,1,heapID );
          }
          p_wk->res_cel[ dst_idx ]	= GFL_CLGRP_CELLANIM_Register( p_handle,
              MYSTERY_CARD_DATA_GetResCel(p_src), MYSTERY_CARD_DATA_GetResAnm(p_src), heapID );
          p_wk->res_cgx[ dst_idx ]	= GFL_CLGRP_CGR_Register( p_handle,
              MYSTERY_CARD_DATA_GetResCgx(p_src), FALSE, CLSYS_DRAW_MAIN, heapID );

          GFL_ARC_CloseDataHandle( p_handle );
        }
        //アイコン作成
        { 
          u32 plt;
          GFL_CLWK_DATA cldata;
          GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
          cldata.pos_y  = data.icon_y;
          cldata.bgpri  = MYSTERY_ALBUM_FONT_FRM;

          if( is_front )
          { 
            cldata.pos_x = data.icon_x;
          }
          else
          { 
            if( is_left )
            { 
              cldata.pos_x = data.icon_x - MYSTERY_ALBUM_CARD_BACKSURFACE_OFS;
            }
            else if( !is_left )
            { 
              cldata.pos_x = data.icon_x + MYSTERY_ALBUM_CARD_BACKSURFACE_OFS;
            }
            else
            { 
              cldata.pos_x = data.icon_x;
            }
          }

          plt = ( MYSTERY_CARD_DATA_GetType(p_src) == MYSTERYGIFT_TYPE_POKEMON ) ?
            p_wk->res_poke_icon_plt: p_wk->res_icon_plt[ dst_idx ];
        
          p_wk->p_icon[ dst_idx ] = GFL_CLACT_WK_Create( p_wk->setup.p_clunit,
              p_wk->res_cgx[ dst_idx ], plt, p_wk->res_cel[ dst_idx ],
              &cldata, CLSYS_DEFREND_MAIN, p_wk->heapID );
          GFL_CLACT_WK_SetPlttOffs( p_wk->p_icon[ dst_idx ],
              MYSTERY_CARD_DATA_GetPltOfs( p_src ), CLWK_PLTTOFFS_MODE_OAM_COLOR );
          GFL_CLACT_WK_SetDrawEnable( p_wk->p_icon[ dst_idx ], TRUE );
        }

        //入れ替えカーソル
        if( p_wk->is_swap && Mystery_Album_GetCardIndex( page, i ) == p_wk->swap_card_index )
        { 
          GFL_CLACTPOS  pos;
          pos.x = sc_thumbnail_data[i].card_x*8;
          pos.y = sc_thumbnail_data[i].card_y*8;
          if( is_front )
          { 
          }
          else
          { 
            if( is_left )
            { 
              pos.x -= MYSTERY_ALBUM_CARD_BACKSURFACE_OFS;
            }
            else if( !is_left )
            { 
              pos.x += MYSTERY_ALBUM_CARD_BACKSURFACE_OFS;
            }
          }
          GFL_CLACT_WK_SetPos( p_wk->p_swap_cursor, &pos, CLSYS_DEFREND_MAIN );
        }
      }
    }
  }

  { 
    int page_max;
    STRBUF *p_strbuf_src;
    STRBUF *p_strbuf_dst;

    p_strbuf_src  = GFL_MSG_CreateString( p_wk->setup.p_msg, syachi_mystery_album_004 );
    p_strbuf_dst  = GFL_MSG_CreateString( p_wk->setup.p_msg, syachi_mystery_album_004 );
    WORDSET_RegisterNumber( p_wk->setup.p_word, 0, page+1, 1, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
    page_max =  Mystery_Album_GetPageMax( p_wk );
    WORDSET_RegisterNumber( p_wk->setup.p_word, 1, page_max, 1, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
    WORDSET_ExpandStr( p_wk->setup.p_word, p_strbuf_src, p_strbuf_dst );

    if( (is_front && !p_wk->is_left)
        || (!is_front && p_wk->is_left) )
    { 
      MYSTERY_MSGWIN_PrintBuf( p_wk->p_page_win[1], p_strbuf_src, p_wk->setup.p_font );
    }
    else
    { 
      MYSTERY_MSGWIN_PrintBuf( p_wk->p_page_win[0], p_strbuf_src, p_wk->setup.p_font );
    }

    GFL_STR_DeleteBuffer( p_strbuf_src );
    GFL_STR_DeleteBuffer( p_strbuf_dst );

  }


  GFL_BG_LoadScreenReq( MYSTERY_ALBUM_BACK_FRM );
}
//----------------------------------------------------------------------------
/**
 *	@brief  画面破棄
 *
 *	@param	MYSTERY_ALBUM_WORK *p_wk ワーク
 *	@param  is_front  表を破棄するか、裏を破棄するか
 */
//-----------------------------------------------------------------------------
static void Mystery_Album_DeleteDisplay( MYSTERY_ALBUM_WORK *p_wk, BOOL is_front )
{ 
  int i;
  MYSTERY_ALBUM_THUMBNAIL_DATA  data;
  u32 dst_idx;

  for( i = 0; i < MYSTERY_CURSOR_MAX; i++ )
  { 
    //貼り付ける位置を取得
    dst_idx = i;

    //座標データ取得
    data  = sc_thumbnail_data[i];

    //貼る面が右側だったらオフセットでずらす
    if( (is_front && !p_wk->is_left)
        || (!is_front && p_wk->is_left) )
    { 
      dst_idx += MYSTERY_CURSOR_MAX;

      data.card_x += MYSTERY_ALBUM_CARD_BACKSURFACE_OFS/8;
      data.date_x += MYSTERY_ALBUM_CARD_BACKSURFACE_OFS/8;
    }


    //カードのキャラを消す
    { 
      int i,j;
      u16 chr_num;
      for( i = 0; i < MYSTERY_ALBUM_THUMBNAIL_CARD_H; i++ )
      { 
        for( j = 0; j < MYSTERY_ALBUM_THUMBNAIL_CARD_W; j++ )
        { 
          GFL_BG_FillScreen( MYSTERY_ALBUM_BACK_FRM, 3,
              data.card_x + j, data.card_y + i, 1, 1, GFL_BG_SCRWRT_PALNL );
        }
      }
    }

    //日付を消す
    { 
      GFL_BMP_DATA  *p_dst  = GFL_BMPWIN_GetBmp( p_wk->p_bmpwin[dst_idx] );
      GFL_BMP_Clear( p_dst, 0 );
      GFL_BMPWIN_TransVramCharacter( p_wk->p_bmpwin[dst_idx] );
    }

    //アイコンを消す
    if( p_wk->p_icon[ dst_idx ] != NULL )
    { 
      GFL_CLACT_WK_Remove( p_wk->p_icon[ dst_idx ] );
      p_wk->p_icon[ dst_idx ] = NULL;

      //リソースを消す
      if( p_wk->res_icon_plt[dst_idx] != GFL_CLGRP_REGISTER_FAILED )
      { 
        GFL_CLGRP_PLTT_Release( p_wk->res_icon_plt[dst_idx] );
        p_wk->res_icon_plt[dst_idx] = GFL_CLGRP_REGISTER_FAILED;
      }
      if( p_wk->res_cgx[dst_idx] != GFL_CLGRP_REGISTER_FAILED )
      { 
        GFL_CLGRP_CGR_Release( p_wk->res_cgx[dst_idx] );
        p_wk->res_cgx[dst_idx] = GFL_CLGRP_REGISTER_FAILED;
      }
      if( p_wk->res_cel[dst_idx] != GFL_CLGRP_REGISTER_FAILED )
      { 
        GFL_CLGRP_CELLANIM_Release( p_wk->res_cel[dst_idx] );
        p_wk->res_cel[dst_idx] = GFL_CLGRP_REGISTER_FAILED;
      }
    }
  }
  GFL_BG_LoadScreenReq( MYSTERY_ALBUM_BACK_FRM );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ページ動作開始
 *
 *	@param	MYSTERY_ALBUM_WORK *p_wk  ワーク
 *	@param	is_left                   TRUEで左に動作  FALSEで右に動作
 */
//-----------------------------------------------------------------------------
static void Mystery_Album_StartScrollPage( MYSTERY_ALBUM_WORK *p_wk, BOOL is_left )
{ 
  p_wk->scroll_req   = is_left? -1: 1;
  p_wk->scroll_cnt    = 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ページ動作
 *
 *	@param	MYSTERY_ALBUM_WORK *p_wk  ワーク
 *
 *	@return TRUEで動作終了  FALSEで動作中
 */
//-----------------------------------------------------------------------------
static BOOL Mystery_Album_ScrollPage( MYSTERY_ALBUM_WORK *p_wk )
{ 
  if( p_wk->scroll_req != 0 )
  { 
    int i;
    GFL_CLACTPOS  pos;
    s32 value = p_wk->scroll_req * MYSTERY_ALBUM_MOVE_SCROLL_DISTANCE / MYSTERY_ALBUM_MOVE_SCROLL_SYNC;

    //CLWK
    for( i = 0; i < MYSTERY_CURSOR_MAX*2; i++ )
    { 
      if( p_wk->p_icon[i])
      { 
        //加算
        GFL_CLACT_WK_GetPos( p_wk->p_icon[i], &pos, CLSYS_DEFREND_MAIN );
        pos.x -= value;
        GFL_CLACT_WK_SetPos( p_wk->p_icon[i], &pos, CLSYS_DEFREND_MAIN );

        //カリング
        { 
          GFL_CLACTPOS  pos;
          GFL_CLACT_WK_GetPos( p_wk->p_icon[i], &pos, CLSYS_DEFREND_MAIN );
          if( -16 <= pos.x && pos.x <= 256+16
            && -16 <= pos.y && pos.y <= 192+16)
          { 
            GFL_CLACT_WK_SetDrawEnable( p_wk->p_icon[i], TRUE );
          }
          else
          { 
            GFL_CLACT_WK_SetDrawEnable( p_wk->p_icon[i], FALSE );
          }
        }
      }
    }
    //加算
    if( p_wk->is_swap )
    { 
      GFL_CLACT_WK_GetPos( p_wk->p_swap_cursor, &pos, CLSYS_DEFREND_MAIN );
      pos.x -= value;
      GFL_CLACT_WK_SetPos( p_wk->p_swap_cursor, &pos, CLSYS_DEFREND_MAIN );
      if( -12*8 <= pos.x && pos.x <= 256+12*8
          && -12*8 <= pos.y && pos.y <= 192+12*8)
      { 
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_swap_cursor, TRUE );
      }
      else
      { 
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_swap_cursor, FALSE );
      }
    }

    //BG
    GFL_BG_SetScroll( MYSTERY_ALBUM_BACK_FRM, GFL_BG_SCROLL_X_INC, value );
    GFL_BG_SetScroll( MYSTERY_ALBUM_CARD_FRM, GFL_BG_SCROLL_X_INC, value );

    //終了
    if( p_wk->scroll_cnt++ >= MYSTERY_ALBUM_MOVE_SCROLL_SYNC-1 )
    { 
      //終了時、どちらの面か決定する
      p_wk->is_left ^= 1;
      p_wk->scroll_req = 0;
    }

    return FALSE;
  }
  else
  { 
    return TRUE;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  カードを消去
 *  
 *	@param	MYSTERY_ALBUM_WORK *p_wk  ワーク
 *	@param	card_index                消去するカードインデックス
 */
//-----------------------------------------------------------------------------
static void Mystery_Album_RemoveCard( MYSTERY_ALBUM_WORK *p_wk, u32 card_index )
{ 
  //カード消去
  MYSTERYDATA_RemoveCardData( p_wk->setup.p_sv, card_index);

  //カード作り直し
  { 
    int i;
    for( i = 0; i < MYSTERY_ALBUM_CARD_MAX; i++ )
    {
      MYSTERY_CARD_DATA_Exit( &p_wk->data[i]);
    }

    for( i = 0; i < MYSTERY_ALBUM_CARD_MAX; i++ )
    {
      if( MYSTERYDATA_IsExistsCard( p_wk->setup.p_sv, i) )
      { 
        GIFT_PACK_DATA *p_data;
        p_data  = MYSTERYDATA_GetCardData( p_wk->setup.p_sv, i );
        MYSTERY_CARD_DATA_Init( &p_wk->data[i], p_data, &p_wk->setup, p_wk->heapID );
      }
    }   
  }



  p_wk->is_change = TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  カードを入れ替え
 *
 *	@param	MYSTERY_ALBUM_WORK *p_wk  ワーク
 *	@param	card_index1               入れ替えるカード１
 *	@param	card_index2               入れ替えるカード２
 */
//-----------------------------------------------------------------------------
static void Mystery_Album_SwapCard( MYSTERY_ALBUM_WORK *p_wk, u32 card_index1, u32 card_index2 )
{ 
  MYSTERY_CARD_DATA temp;
  GIFT_PACK_DATA*   p_gift;

  //スワップ
  MYSTERYDATA_SwapCard( p_wk->setup.p_sv, card_index1, card_index2 );

  //データ作り直し
/*
  MYSTERY_CARD_DATA_Exit( &p_wk->data[ card_index1] );
  p_gift  = MYSTERYDATA_GetCardData( p_wk->setup.p_sv, card_index1 );
  MYSTERY_CARD_DATA_Init( &p_wk->data[ card_index1], p_gift, &p_wk->setup, p_wk->heapID );

  MYSTERY_CARD_DATA_Exit( &p_wk->data[ card_index2] );
  p_gift  = MYSTERYDATA_GetCardData( p_wk->setup.p_sv, card_index2 );
  MYSTERY_CARD_DATA_Init( &p_wk->data[ card_index2], p_gift, &p_wk->setup, p_wk->heapID );
*/
  //カード作り直し
  { 
    int i;
    for( i = 0; i < MYSTERY_ALBUM_CARD_MAX; i++ )
    {
        MYSTERY_CARD_DATA_Exit( &p_wk->data[i]);
    }

    for( i = 0; i < MYSTERY_ALBUM_CARD_MAX; i++ )
    {
      if( MYSTERYDATA_IsExistsCard( p_wk->setup.p_sv, i) )
      { 
        GIFT_PACK_DATA *p_data;
        p_data  = MYSTERYDATA_GetCardData( p_wk->setup.p_sv, i );
        MYSTERY_CARD_DATA_Init( &p_wk->data[i], p_data, &p_wk->setup, p_wk->heapID );
      }
    }   
  }

  p_wk->is_change = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ページ最大数を取得
 *
 *	@param	const MYSTERY_ALBUM_WORK *cp_wk   ワーク
 *
 *	@return ページ最大数
 */
//-----------------------------------------------------------------------------
static u32 Mystery_Album_GetPageMax( const MYSTERY_ALBUM_WORK *cp_wk )
{ 
  int cnt;
  u32 max;

  cnt = Mystery_Album_GetDataNum( cp_wk );

  max = 1 + cnt / MYSTERY_ALBUM_PAGE_IN_CARD_NUM;
  max = MATH_IMin( max, MYSTERY_ALBUM_PAGE_MAX );

  return max;
}
//----------------------------------------------------------------------------
/**
 *	@brief  データ数を取得
 *
 *	@param	const MYSTERY_ALBUM_WORK *cp_wk   ワーク
 *
 *	@return データ数を取得
 */
//-----------------------------------------------------------------------------
static u32 Mystery_Album_GetDataNum( const MYSTERY_ALBUM_WORK *cp_wk )
{ 

  int i;
  int cnt;

  cnt = 0;
  for( i = 0; i < MYSTERY_ALBUM_CARD_MAX; i++ )
  { 
    if( MYSTERY_CARD_DATA_IsExist(&cp_wk->data[i]))
    { 
      cnt++;
    }
  }
  return cnt;
}
//=============================================================================
/**
 *    カードデータ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  カードデータを作成
 *
 *	@param	MYSTERY_CARD_DATA *p_wk ワーク
 *	@param	*p_data                 データ
 *	@param  p_unit                  ユニット
 *	@param	heapID                  ヒープID
 */
//-----------------------------------------------------------------------------
static void MYSTERY_CARD_DATA_Init( MYSTERY_CARD_DATA *p_wk, GIFT_PACK_DATA *p_data, const MYSTERY_ALBUM_SETUP *cp_setup, HEAPID heapID )
{ 
  GFL_STD_MemClear( p_wk, sizeof(MYSTERY_CARD_DATA) );

  if( p_data )
  { 
    p_wk->is_exist  = TRUE;
    p_wk->p_data    = p_data;


    p_wk->p_bmp     = GFL_BMP_Create( MYSTERY_ALBUM_THUMBNAIL_DATE_W, MYSTERY_ALBUM_THUMBNAIL_DATE_H,
                      GFL_BMP_16_COLOR, heapID);
    //BMPに文字描画しておく
    { 
      STRBUF *p_strbuf_src;
      STRBUF *p_strbuf_dst;
      s32 y,m,d;

      y = MYSTERYDATA_GetYear( p_data->recv_date );
      m = MYSTERYDATA_GetMonth( p_data->recv_date );
      d = MYSTERYDATA_GetDay( p_data->recv_date );

      p_strbuf_src  = GFL_MSG_CreateString( cp_setup->p_msg, syachi_mystery_album_005 );
      p_strbuf_dst  = GFL_MSG_CreateString( cp_setup->p_msg, syachi_mystery_album_005 );
      WORDSET_RegisterNumber( cp_setup->p_word, 0, y, 4, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
      WORDSET_RegisterNumber( cp_setup->p_word, 1, m, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
      WORDSET_RegisterNumber( cp_setup->p_word, 2, d, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
      WORDSET_ExpandStr( cp_setup->p_word, p_strbuf_src, p_strbuf_dst );

      PRINTSYS_Print( p_wk->p_bmp, 0, 0, p_strbuf_src, cp_setup->p_font );

      GFL_STR_DeleteBuffer( p_strbuf_src );
      GFL_STR_DeleteBuffer( p_strbuf_dst );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  カードデータを破棄
 *
 *	@param	MYSTERY_CARD_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void MYSTERY_CARD_DATA_Exit( MYSTERY_CARD_DATA *p_wk )
{ 
  if( p_wk->is_exist )
  { 
    GFL_BMP_Delete( p_wk->p_bmp );
    GFL_STD_MemClear( p_wk, sizeof(MYSTERY_CARD_DATA) );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  カードデータが存在するか
 *
 *	@param	const MYSTERY_CARD_DATA *cp_wk  ワーク
 *
 *	@return TRUEならば存在  FALSEならばなし
 */
//-----------------------------------------------------------------------------
static BOOL MYSTERY_CARD_DATA_IsExist( const MYSTERY_CARD_DATA *cp_wk )
{ 
  return cp_wk->is_exist;
}
//----------------------------------------------------------------------------
/**
 *	@brief  BMPWINデータを取得
 *
 *	@param	const MYSTERY_CARD_DATA *cp_wk  ワーク
 *
 *	@return GFL_BMP_DATA  BMP
 */
//-----------------------------------------------------------------------------
static const GFL_BMP_DATA *MYSTERY_CARD_DATA_GetBmp( const MYSTERY_CARD_DATA *cp_wk )
{ 
  return cp_wk->p_bmp;
}
//----------------------------------------------------------------------------
/**
 *	@brief  アーカイブ作成
 *
 *	@param	const MYSTERY_CARD_DATA *cp_wk  ワーク
 *	@param	heapID  ヒープID
 *
 *	@return アーカイブ
 */
//-----------------------------------------------------------------------------
static ARCHANDLE * MYSTERY_CARD_DATA_GetArcHandle( const MYSTERY_CARD_DATA *cp_wk, HEAPID heapID )
{ 
  switch( MYSTERY_CARD_DATA_GetType(cp_wk) )
  { 
  case MYSTERYGIFT_TYPE_POKEMON:
    return GFL_ARC_OpenDataHandle( ARCID_POKEICON, heapID );

  case MYSTERYGIFT_TYPE_ITEM:
    return GFL_ARC_OpenDataHandle( ARCID_ITEMICON, heapID );

  case MYSTERYGIFT_TYPE_RULE:
    /* fallthrow */
  case MYSTERYGIFT_TYPE_NUTSET:
    return GFL_ARC_OpenDataHandle( ARCID_ITEMICON, heapID );
  }

  GF_ASSERT(0);
  return GFL_ARC_OpenDataHandle( ARCID_ITEMICON, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief  パレットを取得
 *
 *	@param	const MYSTERY_CARD_DATA *cp_wk  ワーク
 *
 *	@return パレット
 */
//-----------------------------------------------------------------------------
static u32 MYSTERY_CARD_DATA_GetResPlt( const MYSTERY_CARD_DATA *cp_wk )
{ 
  switch( MYSTERY_CARD_DATA_GetType(cp_wk) )
  { 
  case MYSTERYGIFT_TYPE_POKEMON:
    return POKEICON_GetPalArcIndex();

  case MYSTERYGIFT_TYPE_ITEM:
    { 
      const GIFT_PACK_DATA *cp_data = MYSTERY_CARD_DATA_GetGiftBackData(cp_wk );
      const GIFT_PRESENT_ITEM *cp_item = &cp_data->data.item;
      return ITEM_GetIndex( cp_item->itemNo, ITEM_GET_ICON_PAL );
    }

  case MYSTERYGIFT_TYPE_RULE:
    /* fallthrow */
  case MYSTERYGIFT_TYPE_NUTSET:
    return ITEM_GetIndex( 1, ITEM_GET_ICON_PAL );
  }

  GF_ASSERT(0);
  return ITEM_GetIndex( 1, ITEM_GET_ICON_PAL );
}
//----------------------------------------------------------------------------
/**
 *	@brief  パレットオフセット
 *
 *	@param	const MYSTERY_CARD_DATA *cp_wk  ワーク
 *
 *	@return パレットオフセット
 */
//-----------------------------------------------------------------------------
static u32 MYSTERY_CARD_DATA_GetPltOfs( const MYSTERY_CARD_DATA *cp_wk )
{ 
  switch( MYSTERY_CARD_DATA_GetType(cp_wk) )
  { 
  case MYSTERYGIFT_TYPE_POKEMON:
    { 
      const GIFT_PACK_DATA *cp_data = MYSTERY_CARD_DATA_GetGiftBackData(cp_wk );
      const GIFT_PRESENT_POKEMON *cp_pokemon = &cp_data->data.pokemon;
      return POKEICON_GetPalNum( cp_pokemon->mons_no, cp_pokemon->form_no, cp_pokemon->egg );
    }

  default:
    return 0;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  キャラクターを取得
 *
 *	@param	const MYSTERY_CARD_DATA *cp_wk  ワーク
 *
 *	@return キャラクターを取得
 */
//-----------------------------------------------------------------------------
static u32 MYSTERY_CARD_DATA_GetResCgx( const MYSTERY_CARD_DATA *cp_wk )
{ 
  switch( MYSTERY_CARD_DATA_GetType(cp_wk) )
  { 
  case MYSTERYGIFT_TYPE_POKEMON:
    { 
      const GIFT_PACK_DATA *cp_data = MYSTERY_CARD_DATA_GetGiftBackData(cp_wk );
      const GIFT_PRESENT_POKEMON *cp_pokemon = &cp_data->data.pokemon;
      return POKEICON_GetCgxArcIndexByMonsNumber( cp_pokemon->mons_no, cp_pokemon->form_no, cp_pokemon->egg );
    }

  case MYSTERYGIFT_TYPE_ITEM:
    { 
      const GIFT_PACK_DATA *cp_data = MYSTERY_CARD_DATA_GetGiftBackData(cp_wk );
      const GIFT_PRESENT_ITEM *cp_item = &cp_data->data.item;
      return ITEM_GetIndex( cp_item->itemNo, ITEM_GET_ICON_CGX );
    }

  case MYSTERYGIFT_TYPE_RULE:
    /*  fallthrow */
  case MYSTERYGIFT_TYPE_NUTSET:
    return ITEM_GetIndex( 0, ITEM_GET_ICON_CGX );
  }

  GF_ASSERT(0);
  return ITEM_GetIndex( 0, ITEM_GET_ICON_CGX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  セルデータを取得
 *
 *	@param	const MYSTERY_CARD_DATA *cp_wk  ワーク
 *
 *	@return セルデータ
 */
//-----------------------------------------------------------------------------}
static u32 MYSTERY_CARD_DATA_GetResCel( const MYSTERY_CARD_DATA *cp_wk )
{ 
  switch( MYSTERY_CARD_DATA_GetType(cp_wk) )
  { 
  case MYSTERYGIFT_TYPE_POKEMON:
    return POKEICON_GetCellArcIndex();

  case MYSTERYGIFT_TYPE_ITEM:
    return NARC_item_icon_itemicon_NCER;

  case MYSTERYGIFT_TYPE_RULE:
    /* fallthrow */
  case MYSTERYGIFT_TYPE_NUTSET:
    return NARC_item_icon_itemicon_NCER;
  }

  GF_ASSERT(0);
  return NARC_item_icon_itemicon_NCER;
}
//----------------------------------------------------------------------------
/**
 *	@brief  アニメデータを取得
 *
 *	@param	const MYSTERY_CARD_DATA *cp_wk ワーク
 *
 *	@return アニメデータ
 */
//-----------------------------------------------------------------------------
static u32 MYSTERY_CARD_DATA_GetResAnm( const MYSTERY_CARD_DATA *cp_wk )
{ 
  switch( MYSTERY_CARD_DATA_GetType(cp_wk) )
  { 
  case MYSTERYGIFT_TYPE_POKEMON:
    return POKEICON_GetAnmArcIndex();

  case MYSTERYGIFT_TYPE_ITEM:
    return NARC_item_icon_itemicon_NANR;

  case MYSTERYGIFT_TYPE_RULE:
    /* fallthrow */
  case MYSTERYGIFT_TYPE_NUTSET:
    return NARC_item_icon_itemicon_NANR;
  }

  GF_ASSERT(0);
  return NARC_item_icon_itemicon_NANR;
}
//----------------------------------------------------------------------------
/**
 *	@brief  カードの種類取得
 *
 *	@param	const MYSTERY_CARD_DATA *cp_wk  ワーク
 *
 *	@return タイプ取得
 */
//-----------------------------------------------------------------------------
static u32 MYSTERY_CARD_DATA_GetType( const MYSTERY_CARD_DATA *cp_wk )
{ 
  return cp_wk->p_data->gift_type;
}
//----------------------------------------------------------------------------
/**
 *	@brief  カードのデータを取得
 *
 *	@param	const MYSTERY_CARD_DATA *cp_wk  ワーク
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
static GIFT_PACK_DATA *MYSTERY_CARD_DATA_GetGiftBackData( const MYSTERY_CARD_DATA *cp_wk )
{ 
  return cp_wk->p_data;
}

//=============================================================================
/**
 *    シーケンス
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  カーソル移動
 *
 *	@param	MYSTERY_SEQ_WORK *p_seqwk   シーケンス管理
 *	@param	*p_seq              シーケンス
 *	@param	*p_wk_adrs          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_MoveCursor( MYSTERY_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_MOVE,
    SEQ_SCROLL,
  };


  MYSTERY_ALBUM_WORK *p_wk  = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_MOVE:
    { 
      const int key = GFL_UI_KEY_GetRepeat();
      BOOL is_update  = FALSE;


      //キー入力
      if( key & PAD_KEY_UP )
      { 
        switch( p_wk->cursor )
        { 
        case MYSTERY_CURSOR_LEFT_UP:
          if( p_wk->is_swap )
          {
            p_wk->cursor  = MYSTERY_CURSOR_LEFT_DOWN;
          }
          else
          { 
            p_wk->key_btn_save  = TRUE;
            p_wk->cursor  = MYSTERY_CURSOR_RETURN;
          }
          break;

        case MYSTERY_CURSOR_RIGHT_UP:
          if( p_wk->is_swap )
          { 
            p_wk->cursor  = MYSTERY_CURSOR_RIGHT_DOWN;
          }
          else
          { 
            p_wk->key_btn_save  = FALSE;
            p_wk->cursor  = MYSTERY_CURSOR_RETURN;
          }
          break;

        case MYSTERY_CURSOR_LEFT_DOWN:
          p_wk->cursor  = MYSTERY_CURSOR_LEFT_UP;
          break;

        case MYSTERY_CURSOR_RIGHT_DOWN:
          p_wk->cursor  = MYSTERY_CURSOR_RIGHT_UP;
          break;
            
        case MYSTERY_CURSOR_RETURN:
          if( p_wk->key_btn_save )
          { 
            p_wk->cursor  = MYSTERY_CURSOR_LEFT_DOWN;
          }
          else
          { 
            p_wk->cursor  = MYSTERY_CURSOR_RIGHT_DOWN;
          }
          break;
        }
        is_update     = TRUE;
      }
      else if( key & PAD_KEY_DOWN )
      { 
        switch( p_wk->cursor )
        { 
        case MYSTERY_CURSOR_LEFT_UP:
          p_wk->cursor  = MYSTERY_CURSOR_LEFT_DOWN;
          break;

        case MYSTERY_CURSOR_RIGHT_UP:
          p_wk->cursor  = MYSTERY_CURSOR_RIGHT_DOWN;
          break;

        case MYSTERY_CURSOR_LEFT_DOWN:
          if( p_wk->is_swap )
          { 
            p_wk->cursor  = MYSTERY_CURSOR_LEFT_UP;
          }
          else
          { 
            p_wk->key_btn_save  = TRUE;
            p_wk->cursor  = MYSTERY_CURSOR_RETURN;
          }
          break;
        case MYSTERY_CURSOR_RIGHT_DOWN:
          if( p_wk->is_swap )
          { 
            p_wk->cursor  = MYSTERY_CURSOR_RIGHT_UP;
          }
          else
          { 
            p_wk->key_btn_save  = FALSE;
            p_wk->cursor  = MYSTERY_CURSOR_RETURN;
          }
          break;

        case MYSTERY_CURSOR_RETURN:
          if( p_wk->key_btn_save )
          { 
            p_wk->cursor  = MYSTERY_CURSOR_LEFT_UP;
          }
          else
          { 
            p_wk->cursor  = MYSTERY_CURSOR_RIGHT_UP;
          }
          break;
        }
        is_update     = TRUE;
      }
      else if( key & PAD_KEY_LEFT )
      {
        const int page_max  = Mystery_Album_GetPageMax( p_wk );

        if( (p_wk->cursor == MYSTERY_CURSOR_LEFT_UP
          || p_wk->cursor == MYSTERY_CURSOR_LEFT_DOWN) )
        { 
          u32 next_page;

          if( p_wk->now_page != 0 )
          { 
            next_page = p_wk->now_page-1;
          }
          else
          { 
            next_page  = page_max-1;
          }

          Mystery_Album_StartScrollPage( p_wk, TRUE );
          Mystery_Album_DeleteDisplay( p_wk, FALSE );
          Mystery_Album_CreateDisplay( p_wk, FALSE, TRUE, next_page, p_wk->heapID );

          p_wk->now_page  = next_page;

          *p_seq  = SEQ_SCROLL;
        }
        else if(p_wk->cursor != MYSTERY_CURSOR_RETURN )
        { 
          if( p_wk->cursor == MYSTERY_CURSOR_LEFT_UP || p_wk->cursor == MYSTERY_CURSOR_RIGHT_UP )
          { 
            p_wk->cursor  -=1;
            p_wk->cursor  = MATH_ROUND(p_wk->cursor,MYSTERY_CURSOR_LEFT_UP ,MYSTERY_CURSOR_RIGHT_UP);
          }else
          { 
            p_wk->cursor  -=1;
            p_wk->cursor  = MATH_ROUND(p_wk->cursor,MYSTERY_CURSOR_LEFT_DOWN, MYSTERY_CURSOR_RIGHT_DOWN);
          }
          is_update     = TRUE;
        }
      }
      else if( key & PAD_KEY_RIGHT )
      { 
        const int page_max  = Mystery_Album_GetPageMax( p_wk );

        if( (p_wk->cursor == MYSTERY_CURSOR_RIGHT_UP
          || p_wk->cursor == MYSTERY_CURSOR_RIGHT_DOWN) )
        { 
          u32 next_page;

          if( p_wk->now_page < page_max-1 )
          { 
            next_page = p_wk->now_page+1;
          }
          else
          { 
            next_page  = 0;
          }

          Mystery_Album_StartScrollPage( p_wk, FALSE );
          Mystery_Album_DeleteDisplay( p_wk, FALSE );
          Mystery_Album_CreateDisplay( p_wk, FALSE, FALSE, next_page, p_wk->heapID );

          p_wk->now_page  = next_page;
          *p_seq  = SEQ_SCROLL;
        }
        else if( p_wk->cursor != MYSTERY_CURSOR_RETURN )
        {
          if( p_wk->cursor == MYSTERY_CURSOR_LEFT_UP || p_wk->cursor == MYSTERY_CURSOR_RIGHT_UP )
          { 
            p_wk->cursor  +=1;
            p_wk->cursor  = MATH_ROUND(p_wk->cursor,MYSTERY_CURSOR_LEFT_UP ,MYSTERY_CURSOR_RIGHT_UP);
          }else
          { 
            p_wk->cursor  +=1;
            p_wk->cursor  = MATH_ROUND(p_wk->cursor,MYSTERY_CURSOR_LEFT_DOWN, MYSTERY_CURSOR_RIGHT_DOWN);
          }
          is_update = TRUE;
        }
      }
      else if( key & PAD_BUTTON_DECIDE )
      {  
        if( p_wk->cursor == MYSTERY_CURSOR_RETURN )
        { 
          if( !p_wk->is_swap )
          { 
            MYSTERY_SEQ_SetNext( p_seqwk, SEQFUNC_End );
          }
        }
        else
        { 
          const u32 card_index  = Mystery_Album_GetNowCardIndex( p_wk );
          const MYSTERY_CARD_DATA *cp_data  = &p_wk->data[ card_index ];
          if( MYSTERY_CARD_DATA_IsExist(cp_data) )
          { 
            //スワップモードならば入れ替えへ
            if( p_wk->is_swap )
            { 
              MYSTERY_SEQ_SetNext( p_seqwk, SEQFUNC_SwapCard );
            }
            else
            { 
              MYSTERY_SEQ_SetNext( p_seqwk, SEQFUNC_SelectList );
            }
          }
        }
      }
      else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL )
      { 
        //スワップモードならば、キャンセル
        if( p_wk->is_swap )
        { 
          if( p_wk->p_text )
          { 
            MYSTERY_TEXT_Exit( p_wk->p_text );
            p_wk->p_text  = NULL;
          }

          GFL_CLACT_WK_SetDrawEnable( p_wk->p_swap_cursor, FALSE );
          p_wk->is_swap = FALSE;
        }
        else
        { 
          MYSTERY_SEQ_SetNext( p_seqwk, SEQFUNC_End );
        }
      }

      //カーソルの移動
      if( is_update )
      { 
        GFL_CLACTPOS  pos;

        if( p_wk->cursor < MYSTERY_CURSOR_MAX )
        { 
          pos.x  = sc_thumbnail_data[ p_wk->cursor ].card_x*8;
          pos.y  = sc_thumbnail_data[ p_wk->cursor ].card_y*8;

          GFL_CLACT_WK_SetAnmSeq( p_wk->p_cursor, 0);
        }
        else
        { 
          pos.x = 192;
          pos.y = 160;
          GFL_CLACT_WK_SetAnmSeq( p_wk->p_cursor, 1);
        }

        GFL_CLACT_WK_SetPos( p_wk->p_cursor, &pos, CLSYS_DEFREND_MAIN );

        p_wk->is_card_update  = TRUE;
      }
    }
    break;

  case SEQ_SCROLL:
    if( Mystery_Album_ScrollPage( p_wk ) )
    { 
      p_wk->is_card_update  = TRUE;
      *p_seq  = SEQ_MOVE;
    }
    break;
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  リスト選択
 *
 *	@param	MYSTERY_SEQ_WORK *p_seqwk   シーケンス管理
 *	@param	*p_seq              シーケンス
 *	@param	*p_wk_adrs          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_SelectList( MYSTERY_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  {   
    SEQ_INIT,
    SEQ_SELECT_INIT,
    SEQ_SELECT_WAIT,
    SEQ_NEXT_MOVE,
    SEQ_SWAPMSG_WAIT,
    SEQ_NEXT_SWAP,
    SEQ_NEXT_DELETE,
  };
  MYSTERY_ALBUM_WORK *p_wk  = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_INIT:
    if( p_wk->p_text == NULL )
    { 
      p_wk->p_text  = MYSTERY_TEXT_Init( MYSTERY_ALBUM_LIST_FRM, MYSTERY_ALBUM_FONT_PLT, p_wk->setup.p_que, p_wk->setup.p_font, HEAPID_MYSTERYGIFT );
      MYSTERY_TEXT_WriteWindowFrame( p_wk->p_text, 1, MYSTERY_ALBUM_BG_FRM_S_PLT );
    }
    MYSTERY_TEXT_Print( p_wk->p_text, p_wk->setup.p_msg, syachi_mystery_01_017, MYSTERY_TEXT_TYPE_QUE );

    *p_seq  = SEQ_SELECT_INIT;
    break;

  case SEQ_SELECT_INIT:
    { 
      MYSTERY_LIST_SETUP setup;
      GFL_STD_MemClear( &setup, sizeof(MYSTERY_LIST_SETUP) );
      setup.p_msg   = p_wk->setup.p_msg;
      setup.p_font  = p_wk->setup.p_font;
      setup.p_que   = p_wk->setup.p_que;
      setup.strID[0]= syachi_mystery_menu_008;
      setup.strID[1]= syachi_mystery_menu_009;
      setup.strID[2]= syachi_mystery_menu_010;
      setup.list_max= 3;
      setup.frm     = MYSTERY_ALBUM_LIST_FRM;
      setup.font_plt= MYSTERY_ALBUM_FONT_PLT;
      setup.frm_plt = MYSTERY_ALBUM_BG_FRM_S_PLT;
      setup.frm_chr = 1;
      p_wk->p_list  = MYSTERY_LIST_Init( &setup, p_wk->heapID ); 
      *p_seq  = SEQ_SELECT_WAIT;
    }
    break;

  case SEQ_SELECT_WAIT:
    { 
      u32 ret;
      ret = MYSTERY_LIST_Main( p_wk->p_list );
      if( ret != MYSTERY_LIST_SELECT_NULL )
      { 
        if( ret == 0 )
        { 
          //いれかえ
          p_wk->is_swap = TRUE;

          { 
            GFL_CLACTPOS  pos;
            pos.x  = sc_thumbnail_data[ p_wk->cursor ].card_x*8;
            pos.y  = sc_thumbnail_data[ p_wk->cursor ].card_y*8;

            GFL_CLACT_WK_SetPos( p_wk->p_swap_cursor, &pos, CLSYS_DEFREND_MAIN );
            GFL_CLACT_WK_SetDrawEnable( p_wk->p_swap_cursor, TRUE );
          }

          p_wk->swap_card_index = Mystery_Album_GetNowCardIndex( p_wk );
          NAGI_Printf( "入れ替えるカード %d\n", p_wk->swap_card_index );
          if( p_wk->p_list )
          { 
            MYSTERY_LIST_Exit( p_wk->p_list );
            p_wk->p_list  = NULL;
          }

          if( p_wk->p_text )
          { 
            MYSTERY_TEXT_Exit( p_wk->p_text );
            p_wk->p_text  = NULL;
          }

          if( p_wk->p_text == NULL )
          { 
            p_wk->p_text  = MYSTERY_TEXT_InitOneLine( MYSTERY_ALBUM_LIST_FRM, MYSTERY_ALBUM_FONT_PLT, p_wk->setup.p_que, p_wk->setup.p_font, HEAPID_MYSTERYGIFT );
            MYSTERY_TEXT_WriteWindowFrame( p_wk->p_text, 1, MYSTERY_ALBUM_BG_FRM_S_PLT );
          }
          MYSTERY_TEXT_Print( p_wk->p_text, p_wk->setup.p_msg, syachi_mystery_01_020, MYSTERY_TEXT_TYPE_QUE );
          *p_seq        = SEQ_NEXT_SWAP;
        }
        else if( ret == 1 )
        { 
          //すてる
          *p_seq  = SEQ_NEXT_DELETE;
        }
        else if( ret == 2 )
        { 
          //もどる
          *p_seq  = SEQ_NEXT_MOVE;
        }
      }
    }
    break;
    
  case SEQ_NEXT_MOVE:
    if( p_wk->p_text )
    { 
      MYSTERY_TEXT_Exit( p_wk->p_text );
      p_wk->p_text  = NULL;
    }
    MYSTERY_LIST_Exit( p_wk->p_list );
    p_wk->p_list  = NULL;
    MYSTERY_SEQ_SetNext( p_seqwk, SEQFUNC_MoveCursor );
    break;

  case SEQ_NEXT_SWAP:
    MYSTERY_SEQ_SetNext( p_seqwk, SEQFUNC_MoveCursor );
    break;

  case SEQ_NEXT_DELETE:
    if( p_wk->p_text )
    { 
      MYSTERY_TEXT_Exit( p_wk->p_text );
      p_wk->p_text  = NULL;
    }
    MYSTERY_LIST_Exit( p_wk->p_list );
    p_wk->p_list  = NULL;
    MYSTERY_SEQ_SetNext( p_seqwk, SEQFUNC_DeleteCard );
    break;
  }

  if( p_wk->p_text )
  { 
    MYSTERY_TEXT_Main( p_wk->p_text );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  カード消去
 *
 *	@param	MYSTERY_SEQ_WORK *p_seqwk   シーケンス管理
 *	@param	*p_seq              シーケンス
 *	@param	*p_wk_adrs          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_DeleteCard( MYSTERY_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_YESNO_MSG,
    SEQ_YESNO_INIT,
    SEQ_YESNO_MAIN,
    SEQ_INIT,
    SEQ_EFFECT_WAIT,
    SEQ_DELETE,
    SEQ_MSG_INIT,
    SEQ_MSG_WAIT,
    SEQ_EXIT,
  };

  MYSTERY_ALBUM_WORK *p_wk  = p_wk_adrs;
  const u32 card_index  = Mystery_Album_GetNowCardIndex( p_wk );

  switch( *p_seq )
  { 
  case SEQ_YESNO_MSG:
    if( p_wk->p_text == NULL )
    { 
      p_wk->p_text  = MYSTERY_TEXT_Init( MYSTERY_ALBUM_LIST_FRM, MYSTERY_ALBUM_FONT_PLT, p_wk->setup.p_que, p_wk->setup.p_font, HEAPID_MYSTERYGIFT );
      MYSTERY_TEXT_WriteWindowFrame( p_wk->p_text, 1, MYSTERY_ALBUM_BG_FRM_S_PLT );
    }
    MYSTERY_TEXT_Print( p_wk->p_text, p_wk->setup.p_msg, syachi_mystery_01_021, MYSTERY_TEXT_TYPE_STREAM ); 

    *p_seq  = SEQ_YESNO_INIT;
    break;

  case SEQ_YESNO_INIT:
    if( MYSTERY_TEXT_IsEndPrint( p_wk->p_text ) )
    { 
      MYSTERY_LIST_SETUP setup;
      GFL_STD_MemClear( &setup, sizeof(MYSTERY_LIST_SETUP) );
      setup.p_msg   = p_wk->setup.p_msg;
      setup.p_font  = p_wk->setup.p_font;
      setup.p_que   = p_wk->setup.p_que;
      setup.strID[0]= syachi_mystery_01_021_yes;
      setup.strID[1]= syachi_mystery_01_021_no;
      setup.list_max= 2;
      setup.frm     = MYSTERY_ALBUM_LIST_FRM;
      setup.font_plt= MYSTERY_ALBUM_FONT_PLT;
      setup.frm_plt = MYSTERY_ALBUM_BG_FRM_S_PLT;
      setup.frm_chr = 1;
      p_wk->p_list  = MYSTERY_LIST_Init( &setup, p_wk->heapID ); 

      *p_seq  = SEQ_YESNO_MAIN;
    }
    break;

  case SEQ_YESNO_MAIN:
    { 
      u32 ret;
      ret = MYSTERY_LIST_Main( p_wk->p_list );
      if( ret != MYSTERY_LIST_SELECT_NULL )
      { 
        MYSTERY_LIST_Exit( p_wk->p_list );
        p_wk->p_list  = NULL;

        if( ret == 0 )
        { 
          //はい
          *p_seq  = SEQ_INIT;
        }
        else
        { 
          //いいえ
          if( p_wk->p_text )
          { 
            MYSTERY_TEXT_Exit( p_wk->p_text );
            p_wk->p_text  = NULL;
          }
          MYSTERY_SEQ_SetNext( p_seqwk, SEQFUNC_MoveCursor );
        }
      }
    }
    break;

  case SEQ_INIT:
    *p_seq  = SEQ_EFFECT_WAIT;
    break;

  case SEQ_EFFECT_WAIT:
    *p_seq  = SEQ_DELETE;
    break;

  case SEQ_DELETE:
    //削除
    Mystery_Album_RemoveCard( p_wk, card_index );

    //画面作り直し
    Mystery_Album_DeleteDisplay( p_wk, TRUE );
    Mystery_Album_CreateDisplay( p_wk, TRUE, 0, p_wk->now_page, p_wk->heapID );

    p_wk->is_card_update  = TRUE;
    *p_seq  = SEQ_MSG_INIT;
    break;

  case SEQ_MSG_INIT:
    MYSTERY_TEXT_Print( p_wk->p_text, p_wk->setup.p_msg, syachi_mystery_01_019, MYSTERY_TEXT_TYPE_STREAM );
    *p_seq  = SEQ_MSG_WAIT;
    break;

  case SEQ_MSG_WAIT:
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
    { 
      *p_seq  = SEQ_EXIT;
    }
    break;

  case SEQ_EXIT:
    if( p_wk->p_text )
    { 
      MYSTERY_TEXT_Exit( p_wk->p_text );
      p_wk->p_text  = NULL;
    }
    MYSTERY_SEQ_SetNext( p_seqwk, SEQFUNC_MoveCursor );
    break;
  }


  if( p_wk->p_text )
  { 
    MYSTERY_TEXT_Main( p_wk->p_text );
  }  
  
}
//----------------------------------------------------------------------------
/**
 *	@brief  カード入れ替え
 *
 *	@param	MYSTERY_SEQ_WORK *p_seqwk   シーケンス管理
 *	@param	*p_seq              シーケンス
 *	@param	*p_wk_adrs          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_SwapCard( MYSTERY_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_EFFECT_WAIT,
    SEQ_DELETE,
    SEQ_EXIT,
  };
  MYSTERY_ALBUM_WORK *p_wk  = p_wk_adrs;
  const u32 card_index  = Mystery_Album_GetNowCardIndex( p_wk );

  switch( *p_seq )
  { 
  case SEQ_INIT:
    //スワップ元と先が同じならば終了
    if( card_index == p_wk->swap_card_index )
    { 
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_swap_cursor, FALSE );

      if( p_wk->p_text )
      { 
        MYSTERY_TEXT_Exit( p_wk->p_text );
        p_wk->p_text  = NULL;
      }
      p_wk->is_swap = FALSE;

      MYSTERY_SEQ_SetNext( p_seqwk, SEQFUNC_MoveCursor );
      return ;
    }

    *p_seq  = SEQ_EFFECT_WAIT;
    break;

  case SEQ_EFFECT_WAIT:
    *p_seq  = SEQ_DELETE;
    break;

  case SEQ_DELETE:
    //スワップ
    Mystery_Album_SwapCard( p_wk, card_index, p_wk->swap_card_index );
    NAGI_Printf( "入れ替えます 今の%d 前の%d\n", card_index, p_wk->swap_card_index );

    //画面作り直し
    Mystery_Album_DeleteDisplay( p_wk, TRUE );
    Mystery_Album_CreateDisplay( p_wk, TRUE, 0, p_wk->now_page, p_wk->heapID );

    p_wk->is_card_update  = TRUE;
    *p_seq  = SEQ_EXIT;
    break;

  case SEQ_EXIT:
    p_wk->is_swap = FALSE;
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_swap_cursor, FALSE );
    if( p_wk->p_text )
    { 
      MYSTERY_TEXT_Exit( p_wk->p_text );
      p_wk->p_text  = NULL;
    }
    MYSTERY_SEQ_SetNext( p_seqwk, SEQFUNC_MoveCursor );
    break;
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  終了処理orセーブ
 *
 *	@param	MYSTERY_SEQ_WORK *p_seqwk   シーケンス管理
 *	@param	*p_seq              シーケンス
 *	@param	*p_wk_adrs          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_End( MYSTERY_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_CHECK,
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_EXIT,
  };

  MYSTERY_ALBUM_WORK *p_wk  = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_CHECK:
    if( p_wk->is_change )
    { 
      *p_seq  = SEQ_INIT;
    }
    else
    { 
      *p_seq  = SEQ_EXIT;
    }
    break;

  case SEQ_INIT:
    p_wk->p_text  = MYSTERY_TEXT_Init( MYSTERY_ALBUM_LIST_FRM, MYSTERY_ALBUM_FONT_PLT, p_wk->setup.p_que, p_wk->setup.p_font, HEAPID_MYSTERYGIFT );
    MYSTERY_TEXT_WriteWindowFrame( p_wk->p_text, 1, MYSTERY_ALBUM_BG_FRM_S_PLT );
    MYSTERY_TEXT_Print( p_wk->p_text, p_wk->setup.p_msg, syachi_mystery_01_016, MYSTERY_TEXT_TYPE_STREAM );
    *p_seq  = SEQ_MAIN;
    break;

  case SEQ_MAIN:
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
    { 
      if( p_wk->p_text )
      { 
        MYSTERY_TEXT_Exit( p_wk->p_text );
        p_wk->p_text  = NULL;
      }
      *p_seq  = SEQ_EXIT;
    }
    break;

  case SEQ_EXIT:
    MYSTERY_SEQ_End( p_seqwk );
    break;
  }

  if( p_wk->p_text )
  { 
    MYSTERY_TEXT_Main( p_wk->p_text );
  }  
}


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  カード
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//=============================================================================
/**
 *          定数
 */
//=============================================================================
//=============================================================================
/**
 *          構造体
 */
//=============================================================================
//-------------------------------------
///	カードワーク
//=====================================
struct _MYSTERY_CARD_WORK
{ 
  MYSTERY_CARD_DATA       data;
  MYSTERY_MSGWINSET_WORK  *p_winset;
  u32 res_icon_plt;
  u32 res_icon_cgx;
  u32 res_icon_cel;
  u32 res_silhouette_plt;
  u32 res_silhouette_cgx;
  u32 res_silhouette_cel;
  GFL_CLWK  *p_icon;
  GFL_CLWK  *p_silhouette;
};

//=============================================================================
/**
 *          プロトタイプ
 */
//=============================================================================
//=============================================================================
/**
 *          データ
 */
//=============================================================================
//=============================================================================
/**
 *          パブリック
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  カード  作成
 *
 *	@param	const GIFT_PACK_DATA *cp_data カードに表示するデータ
 *	@param	heapID                        ヒープID 
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
MYSTERY_CARD_WORK * MYSTERY_CARD_Init( const MYSTERY_CARD_SETUP *cp_setup, HEAPID heapID )
{ 
  CLSYS_DRAW_TYPE draw_type;
  PALTYPE         paltype;

  MYSTERY_CARD_WORK * p_wk;
  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(MYSTERY_CARD_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(MYSTERY_CARD_WORK) );
  
  //描画先をチェック
  if( cp_setup->back_frm < GFL_BG_FRAME0_S )
  { 
    draw_type = CLSYS_DRAW_MAIN;
    paltype   = PALTYPE_MAIN_BG;
  }
  else
  { 
    draw_type = CLSYS_DRAW_SUB;
    paltype   = PALTYPE_SUB_BG;
  }


  //リソース取得のためにデータ作成
  { 
    MYSTERY_ALBUM_SETUP setup;
    GFL_STD_MemClear( &setup, sizeof(MYSTERY_LIST_SETUP) );
    setup.mode  = MYSTERY_ALBUM_MODE_VIEW;
    setup.p_clunit  = cp_setup->p_clunit;
    setup.p_sv      = cp_setup->p_sv;
    setup.p_font    = cp_setup->p_font;
    setup.p_que     = cp_setup->p_que;
    setup.p_word    = cp_setup->p_word;
    setup.p_msg     = cp_setup->p_msg;
 
    MYSTERY_CARD_DATA_Init( &p_wk->data, cp_setup->p_data, &setup, heapID );
  }

  //BG読み込み追加
  { 
    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_MYSTERY, heapID );
    //BG
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_mystery_fusigi_card_NCLR,
				paltype, cp_setup->back_plt_num*0x20, 0x20, heapID );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_mystery_fusigi_card_NCGR, 
				cp_setup->back_frm, 0, 0, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_mystery_fusigi_card01_NSCR,
				cp_setup->back_frm, 0, 0, FALSE, heapID );
    GFL_BG_ChangeScreenPalette( cp_setup->back_frm, 0, 0,34,24, cp_setup->back_plt_num );
    GFL_BG_LoadScreenReq( cp_setup->back_frm );

    GFL_ARC_CloseDataHandle( p_handle ); 
  }

  //文章作成
  {
    MYSTERY_MSGWINSET_SETUP_TBL tbl[] =
    { 
      //不思議カード
      { 
        2,
        3,
        10,
        2,
        syachi_mystery_album_006,
      },
      //イベント名
      { 
        2,
        6,
        28,
        2,
      },
      //本文
      { 
        2,
        9,
        28,
        10,
      },
      //もらったひづけ
      { 
        2,
        20,
        14,
        2,
        syachi_mystery_album_007,
      },
      //日付
      { 
        21,
        20,
        9,
        2
      }
    };
    STRBUF  *p_wordbuf;
    s32 y,m,d;

    tbl[1].p_strbuf = GFL_STR_CreateBuffer( GIFT_DATA_CARD_TITLE_MAX+1, heapID );
    GFL_STR_SetStringCodeOrderLength( tbl[1].p_strbuf, cp_setup->p_data->event_name, GIFT_DATA_CARD_TITLE_MAX );
    tbl[2].p_strbuf = GFL_MSG_CreateString( cp_setup->p_msg, syachi_mystery_card_txt_00_01 + cp_setup->p_data->card_message*2 + cp_setup->p_data->have );
    tbl[4].p_strbuf = GFL_MSG_CreateString( cp_setup->p_msg, syachi_mystery_album_008 );

    y = MYSTERYDATA_GetYear( cp_setup->p_data->recv_date );
    m = MYSTERYDATA_GetMonth( cp_setup->p_data->recv_date );
    d = MYSTERYDATA_GetDay( cp_setup->p_data->recv_date );
    p_wordbuf = GFL_MSG_CreateString( cp_setup->p_msg, syachi_mystery_album_008 );
    WORDSET_RegisterNumber( cp_setup->p_word, 0, y, 4, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterNumber( cp_setup->p_word, 1, m, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterNumber( cp_setup->p_word, 2, d, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
    WORDSET_ExpandStr( cp_setup->p_word, tbl[4].p_strbuf, p_wordbuf );

    p_wk->p_winset  = MYSTERY_MSGWINSET_Init( tbl, NELEMS(tbl), cp_setup->font_frm, cp_setup->font_plt_num, cp_setup->p_que, cp_setup->p_msg, cp_setup->p_font, heapID );

    GFL_STR_DeleteBuffer( tbl[1].p_strbuf );
    GFL_STR_DeleteBuffer( tbl[2].p_strbuf );
    GFL_STR_DeleteBuffer( tbl[4].p_strbuf );
    GFL_STR_DeleteBuffer( p_wordbuf );
  }


  //アイコン作成
  { 
    { 
      ARCHANDLE	*	p_handle	= MYSTERY_CARD_DATA_GetArcHandle( &p_wk->data, heapID );

      if( MYSTERY_CARD_DATA_GetType(&p_wk->data) != MYSTERYGIFT_TYPE_POKEMON )
      { 
        p_wk->res_icon_plt	= GFL_CLGRP_PLTT_RegisterEx( p_handle, 
            MYSTERY_CARD_DATA_GetResPlt(&p_wk->data), draw_type, cp_setup->icon_obj_plt_num*0x20, 0,3,heapID );
      }
      else
      { 

        p_wk->res_icon_plt = GFL_CLGRP_PLTT_RegisterComp( p_handle, 
            MYSTERY_CARD_DATA_GetResPlt(&p_wk->data), draw_type, cp_setup->icon_obj_plt_num*0x20, heapID );
      }
      p_wk->res_icon_cel	= GFL_CLGRP_CELLANIM_Register( p_handle,
          MYSTERY_CARD_DATA_GetResCel(&p_wk->data), MYSTERY_CARD_DATA_GetResAnm(&p_wk->data), heapID );
      p_wk->res_icon_cgx	= GFL_CLGRP_CGR_Register( p_handle,
          MYSTERY_CARD_DATA_GetResCgx(&p_wk->data), FALSE, draw_type, heapID );

      GFL_ARC_CloseDataHandle( p_handle );
    }
    { 
      GFL_CLWK_DATA cldata;
      GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
      cldata.pos_x  = 212;
      cldata.pos_y  = 33;

      p_wk->p_icon    = GFL_CLACT_WK_Create( cp_setup->p_clunit, 
          p_wk->res_icon_cgx, p_wk->res_icon_plt, p_wk->res_icon_cel,
          &cldata, draw_type, heapID );
      GFL_CLACT_WK_SetPlttOffs( p_wk->p_icon,
              MYSTERY_CARD_DATA_GetPltOfs( &p_wk->data ), CLWK_PLTTOFFS_MODE_OAM_COLOR );

    }
  }

  //シルエット作成
  if( MYSTERY_CARD_DATA_GetType(&p_wk->data) == MYSTERYGIFT_TYPE_POKEMON )
  { 
    { 
      const GIFT_PRESENT_POKEMON  *cp_pokemon = &cp_setup->p_data->data.pokemon;
      ARCHANDLE *p_handle = POKE2DGRA_OpenHandle( heapID );

      p_wk->res_silhouette_plt  = POKE2DGRA_OBJ_PLTT_Register( p_handle, cp_pokemon->mons_no, cp_pokemon->form_no, cp_pokemon->sex, cp_pokemon->rare, POKEGRA_DIR_FRONT, cp_pokemon->egg, draw_type, cp_setup->silhouette_obj_plt_num*0x20, heapID );
      p_wk->res_silhouette_cel	= POKE2DGRA_OBJ_CELLANM_Register( cp_pokemon->mons_no, cp_pokemon->form_no, cp_pokemon->sex, cp_pokemon->rare, POKEGRA_DIR_FRONT, cp_pokemon->egg, APP_COMMON_MAPPING_128K, draw_type, heapID );
      p_wk->res_silhouette_cgx	= POKE2DGRA_OBJ_CGR_Register( p_handle, cp_pokemon->mons_no, cp_pokemon->form_no, cp_pokemon->sex, cp_pokemon->rare, POKEGRA_DIR_FRONT, cp_pokemon->egg, draw_type, heapID );

      GFL_ARC_CloseDataHandle( p_handle );
    }
    { 
      GFL_CLWK_DATA cldata;
      GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
      cldata.pos_x  = 184;
      cldata.pos_y  = 112;

      p_wk->p_silhouette   = GFL_CLACT_WK_Create( cp_setup->p_clunit, 
          p_wk->res_silhouette_cgx, p_wk->res_silhouette_plt, p_wk->res_silhouette_cel,
          &cldata, draw_type, heapID );
    }
  }

  GFL_BG_SetVisible( cp_setup->back_frm, TRUE );
  GFL_BG_SetVisible( cp_setup->font_frm, TRUE );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  カード  破棄
 *
 *	@param	MYSTERY_CARD_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void MYSTERY_CARD_Exit( MYSTERY_CARD_WORK *p_wk )
{ 
  //シルエット破棄
  if( p_wk->p_silhouette )
  { 
    { 
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_silhouette, FALSE );
      GFL_CLACT_WK_Remove( p_wk->p_silhouette );
      GFL_CLGRP_CGR_Release( p_wk->res_silhouette_cgx );
      GFL_CLGRP_CELLANIM_Release( p_wk->res_silhouette_cel );
      GFL_CLGRP_PLTT_Release( p_wk->res_silhouette_plt );
    }
  }

  //アイコン破棄
  { 
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_icon, FALSE );
    GFL_CLACT_WK_Remove( p_wk->p_icon );
    GFL_CLGRP_CGR_Release( p_wk->res_icon_cgx );
    GFL_CLGRP_CELLANIM_Release( p_wk->res_icon_cel );
    GFL_CLGRP_PLTT_Release( p_wk->res_icon_plt ); 
  }

  MYSTERY_MSGWINSET_Exit( p_wk->p_winset );

  MYSTERY_CARD_DATA_Exit( &p_wk->data );
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  カード  メイン処理
 *
 *	@param	MYSTERY_CARD_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void MYSTERY_CARD_Main( MYSTERY_CARD_WORK *p_wk )
{ 
  MYSTERY_MSGWINSET_PrintMain( p_wk->p_winset );
}
