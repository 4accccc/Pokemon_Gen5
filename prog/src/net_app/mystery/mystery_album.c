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
#include "sound/pm_wb_voice.h"
#include "system/bmp_winframe.h"
#include "pokeicon/pokeicon.h"
#include "item/item.h"
#include "system/poke2dgra.h"

//アーカイブ
#include "arc_def.h"
#include "item_icon.naix"
#include "arc\mystery.naix"
#include "msg/msg_mystery.h"
#include "font/font.naix"

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

//メインBG面
#define MYSTERY_ALBUM_LIST_FRM        GFL_BG_FRAME0_M
#define MYSTERY_ALBUM_FONT_FRM        GFL_BG_FRAME1_M
#define MYSTERY_ALBUM_CARD_FRM        GFL_BG_FRAME2_M
#define MYSTERY_ALBUM_BACK_FRM        GFL_BG_FRAME3_M

//サブBG面
#define MYSTERY_ALBUM_CARD_FONT_S       GFL_BG_FRAME0_S
#define MYSTERY_ALBUM_CARD_FRM_S        GFL_BG_FRAME2_S
#define MYSTERY_ALBUM_CARD_BACK_S       GFL_BG_FRAME3_S

//メインBGパレット
#define MYSTERY_ALBUM_FONT_PLT              15
#define MYSTERY_ALBUM_TOP_FONT_PLT          3
#define MYSTERY_ALBUM_BG_BACK_PLT_M         0

//サブBGパレット
#define MYSTERY_ALBUM_BG_FRM_S_PLT          13
#define MYSTERY_ALBUM_BG_BACK_S_PLT         14
#define MYSTERY_ALBUM_BG_FONT_S_PLT         15
#define MYSTERY_ALBUM_BG_BACK_PLT_S         0

//メインOBJパレット
#define MYSTERY_ALBUM_OBJ_CURSOR_PLT        0
#define MYSTERY_ALBUM_OBJ_ICON_PLT          9
#define MYSTERY_ALBUM_OBJ_POKEICON_PLT      12

//サブOBJパレット
#define MYSTERY_ALBUM_OBJ_ICON_S_PLT        11
#define MYSTERY_ALBUM_OBJ_SILHOUETTE_S_PLT  14

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

#define MYSTERY_ALBUM_SWAP_SYNC             30
#define MYSTERY_ALBUM_DELETE_SYNC             30

#define MYSTERY_ALBUM_SWAP_BRIGHTNESS_MASK ( \
                                            GX_WND_PLANEMASK_BG1 | \
                                            GX_WND_PLANEMASK_BG2 | \
                                            GX_WND_PLANEMASK_BG3 | \
                                            GX_WND_PLANEMASK_OBJ )

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
  MYSTERY_MSGWIN_WORK     *p_page_win;
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
  u32                 cnt;

  GFL_CLWK            *p_cursor;  //カーソル
  GFL_CLWK            *p_swap_cursor;  //カーソル(swap)
  GFL_CLWK            *p_allow;
  u32                 res_cursor_cgx;
  u32                 res_cursor_cel;
  u32                 res_cursor_plt;

  GFL_BMPWIN          *p_bmpwin[MYSTERY_CURSOR_MAX*2];  //表と裏
  GFL_CLWK            *p_icon[ MYSTERY_CURSOR_MAX*2 ];  //コピーされたポインタ
  u32                 res_cgx[MYSTERY_CURSOR_MAX*2];
  u32                 res_cel[MYSTERY_CURSOR_MAX*2];
  u32                 res_icon_plt[MYSTERY_CURSOR_MAX*2];//8本
  u32                 res_poke_icon_plt;    //3本

  //カーソルパレットフェード用
  u16 plt_cnt;
  u16 plt[0x10];
  u16 plt_src[0x10];
  u16 plt_dst[0x10];
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
static inline u32 Mystery_Album_GetIndexToPage( u32 cursor )
{ 
  return cursor / MYSTERY_CURSOR_MAX;
}
static void Mystery_MainPltAnm( NNS_GFD_DST_TYPE type, u16 *p_buff, u16 cnt, u8 plt_num, u8 plt_col, GXRgb start, GXRgb end );

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
static BOOL MYSTERY_CARD_DATA_IsHave( const MYSTERY_CARD_DATA *cp_wk );

//-------------------------------------
///	SEQFUNC
//=====================================
static void SEQFUNC_MoveCursor( MYSTERY_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_SelectList( MYSTERY_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_DeleteCard( MYSTERY_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_SwapCard( MYSTERY_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_End( MYSTERY_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

static void SEQFUNC_DeleteMsg( MYSTERY_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

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
    48 -2,
  },
  //右上
  { 
    18,
    4,
    19,
    8,
    216,
    48 -2,
  },
  //左下
  { 
    3,
    12,
    4,
    16,
    96,
    112 -2,
  },
  //右下
  { 
    18,
    12,
    19,
    16,
    216,
    112-2,
  },
};
#define MYSTERY_ALBUM_CARD_BACKSURFACE_OFS  256
#define MYSTERY_ALBUM_THUMBNAIL_CARD_W      12
#define MYSTERY_ALBUM_THUMBNAIL_CARD_H      7
#define MYSTERY_ALBUM_THUMBNAIL_DATE_W      10
#define MYSTERY_ALBUM_THUMBNAIL_DATE_H      2
#define MYSTERY_ALBUM_THUMBNAIL_GETPOKE_PLT       0
#define MYSTERY_ALBUM_THUMBNAIL_NOGETPOKE_PLT     5
#define MYSTERY_ALBUM_THUMBNAIL_NONE_PLT     3

#define MYSTERY_ALBUM_CARD_WRITE_SRC_START_X      0
#define MYSTERY_ALBUM_CARD_WRITE_SRC_START_Y      2
#define MYSTERY_ALBUM_CARD_WRITE_SRC_W            32

//書き込みキャラ取得
#define MYSTERY_ALBUM_CARD_WRITE_SRC_GET_CHARA(x,y)    (MYSTERY_ALBUM_CARD_WRITE_SRC_START_X +  \
                                                    MYSTERY_ALBUM_CARD_WRITE_SRC_START_Y * \
                                                    MYSTERY_ALBUM_CARD_WRITE_SRC_W + \
                                                    y * MYSTERY_ALBUM_CARD_WRITE_SRC_W + x)

#define MYSTERY_ALBUM_CARD_WRITE_BLANK_SRC_START_X     0
#define MYSTERY_ALBUM_CARD_WRITE_BLANK_SRC_START_Y      12

//背面書き込みキャラ取得
#define MYSTERY_ALBUM_CARD_WRITE_BLANK_SRC_GET_CHARA(x,y) ( MYSTERY_ALBUM_CARD_WRITE_BLANK_SRC_START_X +  \
                                                    MYSTERY_ALBUM_CARD_WRITE_BLANK_SRC_START_Y * \
                                                    MYSTERY_ALBUM_CARD_WRITE_SRC_W + \
                                                    y * MYSTERY_ALBUM_CARD_WRITE_SRC_W + x)

#define MYSTERY_ALBUM_CARD_NUMBER_SRC_START_X     24
#define MYSTERY_ALBUM_CARD_NUMBER_SRC_START_Y     2
#define MYSTERY_ALBUM_CARD_NUMBER_W               2
#define MYSTERY_ALBUM_CARD_NUMBER_H               2

//数字キャラのトップ取得
#define MYSTERY_ALBUM_CARD_NUMBER_GET_TOP(i)  (MYSTERY_ALBUM_CARD_NUMBER_SRC_START_X + \
  ( MYSTERY_ALBUM_CARD_WRITE_SRC_W * MYSTERY_ALBUM_CARD_NUMBER_H * (i % 3)) + \
  + (MYSTERY_ALBUM_CARD_NUMBER_W * (i / 3)))

//数字書き込みキャラ取得
#define MYSTERY_ALBUM_CARD_NUMBER_GET_CHARA(i,x,y)    (MYSTERY_ALBUM_CARD_NUMBER_GET_TOP(i) + \
                                                      MYSTERY_ALBUM_CARD_NUMBER_SRC_START_Y * \
                                                      MYSTERY_ALBUM_CARD_WRITE_SRC_W + \
                                                      y * MYSTERY_ALBUM_CARD_WRITE_SRC_W + x)


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

  { 
    MYSTERY_SEQ_FUNCTION  start_seq;
    if( p_wk->setup.mode == MYSTERY_ALBUM_MODE_VIEW )
    { 
      start_seq = SEQFUNC_MoveCursor;
    }
    else
    { 
      start_seq = SEQFUNC_DeleteMsg;
    }
    p_wk->p_seq = MYSTERY_SEQ_Init( p_wk, start_seq, heapID );
  }

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
        syachi_mystery_album_001,
        NULL,
        MYSTERY_MSGWIN_POS_WH_CENTER,
        0, 0,
        PRINTSYS_MACRO_LSB( 0xE, 0xF, 0 )
      },
      {
        1,
        0x16,
        11,
        2,
        syachi_mystery_album_002,
        NULL,
        MYSTERY_MSGWIN_POS_ABSOLUTE,
        0, 0,
        PRINTSYS_MACRO_LSB( 0xE, 0xF, 0xA )
      },
      { 
        0x17,
        0x14,
        7,
        2,
        syachi_mystery_album_003,
        NULL,
        MYSTERY_MSGWIN_POS_WH_CENTER,
        0, 0,
        PRINTSYS_MACRO_LSB( 0xE, 0xF, 2 )
      },
    };
    p_wk->p_winset  = MYSTERY_MSGWINSET_Init( c_setup_tbl, NELEMS(c_setup_tbl), MYSTERY_ALBUM_FONT_FRM, MYSTERY_ALBUM_TOP_FONT_PLT, p_wk->setup.p_que, p_wk->setup.p_msg, p_wk->setup.p_font, heapID );
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

  GFL_BG_SetScroll( GFL_BG_FRAME3_M, GFL_BG_SCROLL_Y_SET, 0 );
  GFL_BG_SetScroll( GFL_BG_FRAME3_S, GFL_BG_SCROLL_Y_SET, 0 );
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
  GFL_BG_ClearScreen( MYSTERY_ALBUM_CARD_FONT_S );
  GFL_BG_ClearScreen( MYSTERY_ALBUM_CARD_FRM_S );
  GFL_BG_LoadScreenReq( MYSTERY_ALBUM_CARD_FONT_S );
  GFL_BG_LoadScreenReq( MYSTERY_ALBUM_CARD_FRM_S );

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
  //メイン処理
  MYSTERY_SEQ_Main( p_wk->p_seq );

  //下画面カード読み替え処理
  if( p_wk->is_card_update )
  { 
    MYSTERY_CARD_DATA *p_data;

    if( p_wk->p_card )
    { 
      MYSTERY_CARD_Exit( p_wk->p_card );
      p_wk->p_card  = NULL;
    }

    if( p_wk->cursor < MYSTERY_CURSOR_MAX )
    { 
      p_data = &p_wk->data[ Mystery_Album_GetNowCardIndex( p_wk ) ];
      if( MYSTERY_CARD_DATA_IsExist( p_data ) )
      { 
        if( p_wk->p_card == NULL )
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
          GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG2 | GX_PLANEMASK_OBJ, TRUE );
        }
      }
      else
      { 
        GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG2 | GX_PLANEMASK_OBJ, FALSE );
      }
    }
    else
    { 
      GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG2 | GX_PLANEMASK_OBJ, FALSE );
    }
    p_wk->is_card_update  = FALSE;
  }

  //パレットフェード
  { 
    int i;
    const u16 add = 0x400;

    //パレットアニメ
    if( p_wk->plt_cnt + add >= 0x10000 )
    {
      p_wk->plt_cnt = p_wk->plt_cnt+add-0x10000;
    }
    else
    {
      p_wk->plt_cnt += add;
    }

    for( i = 0; i < 0x10; i++ )
    { 
      Mystery_MainPltAnm( NNS_GFD_DST_2D_OBJ_PLTT_MAIN, &p_wk->plt[i], p_wk->plt_cnt, MYSTERY_ALBUM_OBJ_CURSOR_PLT, i, p_wk->plt_src[i], p_wk->plt_dst[i] );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  文字描画メイン
 *
 *	@param	MYSTERY_ALBUM_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void MYSTERY_ALBUM_PrintMain( MYSTERY_ALBUM_WORK *p_wk )
{ 
  MYSTERY_MSGWINSET_PrintMain( p_wk->p_winset );
  if( p_wk->p_page_win )
  { 
    MYSTERY_MSGWIN_PrintMain( p_wk->p_page_win );
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

  { 
    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_MYSTERY, heapID );
    //BG
    //上画面
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_mystery_fushigi_album_NCLR,
				PALTYPE_MAIN_BG, MYSTERY_ALBUM_BG_BACK_PLT_M*0x20, 10*0x20, heapID );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_mystery_fushigi_album_NCGR, 
				MYSTERY_ALBUM_BACK_FRM, 0, 0, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_mystery_fushigi_album_NSCR,
				MYSTERY_ALBUM_BACK_FRM, 0, 0, FALSE, heapID );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_mystery_fushigi_album_NCGR, 
				MYSTERY_ALBUM_FONT_FRM, 0, 0, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_mystery_fushigi_album03_NSCR,
				MYSTERY_ALBUM_FONT_FRM, 0, 0, FALSE, heapID );
    GFL_BG_LoadScreenReq( MYSTERY_ALBUM_BACK_FRM );

    //下画面
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_mystery_fushigi_album_NCLR,
				PALTYPE_SUB_BG, MYSTERY_ALBUM_BG_BACK_PLT_S*0x20, 10*0x20, heapID );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_mystery_fushigi_album_NCGR, 
				MYSTERY_ALBUM_CARD_BACK_S, 0, 0, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_mystery_fushigi_album02_NSCR,
				MYSTERY_ALBUM_CARD_BACK_S, 0, 0, FALSE, heapID );

    GFL_ARC_CloseDataHandle( p_handle );

  }
  //フォント
  {	
    ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_FONT, heapID );

    //上下画面フォントパレット
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_default_nclr,
        PALTYPE_MAIN_BG, MYSTERY_ALBUM_FONT_PLT*0x20, 0x20, heapID );
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_default_nclr,
        PALTYPE_SUB_BG, MYSTERY_ALBUM_BG_FONT_S_PLT*0x20, 0x20, heapID );

    GFL_ARC_CloseDataHandle( p_handle );
  }
  { 
    //キャラを単位で読み込み
    //BG_FRAME_M_TEXT
    { 
      GFL_BG_FillCharacter( MYSTERY_ALBUM_FONT_FRM, 0, 1, 0 );
      GFL_BG_FillCharacter( MYSTERY_ALBUM_CARD_FRM, 0, 1, 0 );
    }
    BmpWinFrame_GraphicSet( MYSTERY_ALBUM_LIST_FRM, 1, MYSTERY_ALBUM_BG_FRM_S_PLT, MENU_TYPE_SYSTEM, heapID );
  }

  { 
    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_POKEICON, heapID );
    //共通ポケアイコン用パレット
    p_wk->res_poke_icon_plt = GFL_CLGRP_PLTT_RegisterComp( p_handle, 
				POKEICON_GetPalArcIndex(), CLSYS_DRAW_MAIN, MYSTERY_ALBUM_OBJ_POKEICON_PLT*0x20, heapID );
    GFL_ARC_CloseDataHandle( p_handle );
  }

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
        MYSTERY_ALBUM_FONT_PLT, GFL_BMP_CHRAREA_GET_F );
    GFL_BMPWIN_MakeScreen( p_wk->p_bmpwin[i] );
    GFL_BG_LoadScreenReq( MYSTERY_ALBUM_CARD_FRM );
  }

  //ページ作成( 0/3 )
  p_wk->p_page_win  = MYSTERY_MSGWIN_Init( MYSTERY_ALBUM_FONT_FRM, 13, 22, 6, 2, MYSTERY_ALBUM_TOP_FONT_PLT, p_wk->setup.p_que, heapID );

  //カーソル作成
  { 
    { 
      ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_MYSTERY, heapID );
      p_wk->res_cursor_plt  = GFL_CLGRP_PLTT_RegisterEx( p_handle, 
        NARC_mystery_album_cursor_NCLR, CLSYS_DRAW_MAIN, MYSTERY_ALBUM_OBJ_CURSOR_PLT*0x20, 0, 6, heapID );
      p_wk->res_cursor_cel	= GFL_CLGRP_CELLANIM_Register( p_handle,
          NARC_mystery_album_cursor_NCER, NARC_mystery_album_cursor_NANR, heapID );
      p_wk->res_cursor_cgx	= GFL_CLGRP_CGR_Register( p_handle,
              NARC_mystery_album_cursor_NCGR, FALSE, CLSYS_DRAW_MAIN, heapID );

      //カーソルのパレットフェード用にパレットをメモリ読み込み
      { 
        void *p_buff;
        NNSG2dPaletteData *p_plt;
        const u16 *cp_plt_adrs;

        //もとのパレットから色情報を保存
        p_buff  = GFL_ARCHDL_UTIL_LoadPalette( p_handle, NARC_mystery_album_cursor_NCLR, &p_plt, heapID );
        cp_plt_adrs = p_plt->pRawData;
        GFL_STD_MemCopy( cp_plt_adrs, p_wk->plt_dst, sizeof(u16) * 0x10 );
        GFL_STD_MemCopy( (u8*)cp_plt_adrs + 1 * 0x20, p_wk->plt_src, sizeof(u16) * 0x10 );

        //パレット破棄
        GFL_HEAP_FreeMemory( p_buff );
      }

      GFL_ARC_CloseDataHandle( p_handle );
    }
    { 
      GFL_CLWK_DATA cldata;
      GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
      cldata.pos_x  = sc_thumbnail_data[0].card_x*8 - 24;
      cldata.pos_y  = sc_thumbnail_data[0].card_y*8 - 8;
      cldata.bgpri  = MYSTERY_ALBUM_FONT_FRM;
      cldata.softpri  = 1;

      p_wk->p_cursor    = GFL_CLACT_WK_Create( p_wk->setup.p_clunit, 
          p_wk->res_cursor_cgx, p_wk->res_cursor_plt, p_wk->res_cursor_cel,
          &cldata, CLSYS_DEFREND_MAIN, heapID );

      cldata.anmseq = 2;
      p_wk->p_swap_cursor    = GFL_CLACT_WK_Create( p_wk->setup.p_clunit, 
          p_wk->res_cursor_cgx, p_wk->res_cursor_plt, p_wk->res_cursor_cel,
          &cldata, CLSYS_DEFREND_MAIN, heapID );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_swap_cursor, FALSE );

      cldata.pos_x  = 240;
      cldata.pos_y  = 16;
      cldata.anmseq = 9;
      cldata.softpri  = 0;
      p_wk->p_allow          = GFL_CLACT_WK_Create( p_wk->setup.p_clunit, 
          p_wk->res_cursor_cgx, p_wk->res_cursor_plt, p_wk->res_cursor_cel,
          &cldata, CLSYS_DEFREND_MAIN, heapID );
      GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_allow, TRUE );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_allow, FALSE );
    }
  }

  GFL_BG_SetVisible( MYSTERY_ALBUM_CARD_FRM, TRUE );
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

  GFL_BG_FillCharacterRelease( MYSTERY_ALBUM_CARD_FRM, 1, 0 );
  GFL_BG_FillCharacterRelease( MYSTERY_ALBUM_FONT_FRM, 1, 0 );

  //カーソル破棄
  { 
    GFL_CLACT_WK_Remove( p_wk->p_allow );
    GFL_CLACT_WK_Remove( p_wk->p_swap_cursor );
    GFL_CLACT_WK_Remove( p_wk->p_cursor );
    GFL_CLGRP_CGR_Release( p_wk->res_cursor_cgx );
    GFL_CLGRP_CELLANIM_Release( p_wk->res_cursor_cel );
    GFL_CLGRP_PLTT_Release( p_wk->res_cursor_plt );
  }

  MYSTERY_MSGWIN_Exit( p_wk->p_page_win );
  p_wk->p_page_win  = NULL;
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

    //カードの中身があったら貼付け---
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

    //数字キャラ貼付け
    //カードのキャラをスクリーンに貼り付ける
    { 
      int j, k;
      u16 chr_num;
      u8 card_index;
      for( j = 0; j < MYSTERY_ALBUM_CARD_NUMBER_H; j++ )
      { 
        for( k = 0; k < MYSTERY_ALBUM_CARD_NUMBER_W; k++ )
        { 
          card_index  = Mystery_Album_GetCardIndex( page, i );
          chr_num = MYSTERY_ALBUM_CARD_NUMBER_GET_CHARA( card_index, k, j );

          NAGI_Printf( "cardindex %d top %d\n", card_index, MYSTERY_ALBUM_CARD_NUMBER_GET_TOP(card_index) );

          GFL_BG_FillScreen( MYSTERY_ALBUM_BACK_FRM, chr_num,
              data.card_x + k - MYSTERY_ALBUM_CARD_NUMBER_W, 
              data.card_y + j, 1, 1, MYSTERY_ALBUM_THUMBNAIL_NONE_PLT );
        }
      }
    }

    if( !MYSTERY_CARD_DATA_IsExist( p_src ) )
    {
      //カードの中身がなかったら、背面貼付け--
      //カードのキャラをスクリーンに貼り付ける
      { 
        int i,j;
        u16 chr_num;
        for( i = 0; i < MYSTERY_ALBUM_THUMBNAIL_CARD_H; i++ )
        { 
          for( j = 0; j < MYSTERY_ALBUM_THUMBNAIL_CARD_W; j++ )
          { 
            chr_num = MYSTERY_ALBUM_CARD_WRITE_BLANK_SRC_GET_CHARA( j, i );

            GFL_BG_FillScreen( MYSTERY_ALBUM_BACK_FRM, chr_num,
                data.card_x + j, data.card_y + i, 1, 1, MYSTERY_ALBUM_THUMBNAIL_NONE_PLT );
          }
        }
      }
    }
    else
    { 
      //カードのキャラをスクリーンに貼り付ける
      { 
        int i,j;
        u16 chr_num;
        for( i = 0; i < MYSTERY_ALBUM_THUMBNAIL_CARD_H; i++ )
        { 
          for( j = 0; j < MYSTERY_ALBUM_THUMBNAIL_CARD_W; j++ )
          { 
            chr_num = MYSTERY_ALBUM_CARD_WRITE_SRC_GET_CHARA( j, i );

            GFL_BG_FillScreen( MYSTERY_ALBUM_BACK_FRM, chr_num,
                data.card_x + j, data.card_y + i, 1, 1, MYSTERY_ALBUM_THUMBNAIL_NOGETPOKE_PLT );
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
          pos.x = sc_thumbnail_data[i].card_x*8 - 24;
          pos.y = sc_thumbnail_data[i].card_y*8 - 8;
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

    MYSTERY_MSGWIN_SetColor( p_wk->p_page_win, PRINTSYS_MACRO_LSB( 0xE, 0xF, 0xA ) );
    MYSTERY_MSGWIN_SetPos( p_wk->p_page_win, 0, 0, MYSTERY_MSGWIN_POS_WH_CENTER );
    MYSTERY_MSGWIN_PrintBuf( p_wk->p_page_win, p_strbuf_src, p_wk->setup.p_font );

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
      //カードのキャラをスクリーンに貼り付ける
      int i,j;
      u16 chr_num;
      for( i = 0; i < MYSTERY_ALBUM_THUMBNAIL_CARD_H; i++ )
      { 
        for( j = 0; j < MYSTERY_ALBUM_THUMBNAIL_CARD_W; j++ )
        { 
          chr_num = MYSTERY_ALBUM_CARD_WRITE_BLANK_SRC_GET_CHARA( j, i );
          GFL_BG_FillScreen( MYSTERY_ALBUM_BACK_FRM, chr_num,
              data.card_x + j, data.card_y + i, 1, 1, MYSTERY_ALBUM_THUMBNAIL_NONE_PLT );
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
//----------------------------------------------------------------------------
/**
 *	@brief  パレットフェード
 *
 *	@param	NNS_GFD_DST_TYPE type 転送先
 *	@param	*p_buff               バッファ
 *	@param	cnt                   カウント
 *	@param	plt_num               パレット縦
 *	@param	plt_col               パレット横
 *	@param	start                 開始色
 *	@param	end                   終了色
 */
//-----------------------------------------------------------------------------
static void Mystery_MainPltAnm( NNS_GFD_DST_TYPE type, u16 *p_buff, u16 cnt, u8 plt_num, u8 plt_col, GXRgb start, GXRgb end )
{ 
  //1～0に変換
  const fx16 cos = (FX_CosIdx(cnt)+FX16_ONE)/2;
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
    MYSTERYDATA_ModifyGiftData( p_wk->p_data );

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

      {
        s16 x, y;
        x = MYSTERY_ALBUM_THUMBNAIL_DATE_W * 8 / 2;
        y = MYSTERY_ALBUM_THUMBNAIL_DATE_H * 8 / 2;
        x -= PRINTSYS_GetStrWidth( p_strbuf_src, cp_setup->p_font, 0 ) / 2;
        y -= PRINTSYS_GetStrHeight( p_strbuf_src, cp_setup->p_font ) / 2;
        PRINTSYS_Print( p_wk->p_bmp, x, y, p_strbuf_src, cp_setup->p_font );
      }

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

//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	const MYSTERY_CARD_DATA *cp_wk 
 *
 *	@return TRUEで受取済み  FALSEでまだ受け取っていない
 */
//-----------------------------------------------------------------------------
static BOOL MYSTERY_CARD_DATA_IsHave( const MYSTERY_CARD_DATA *cp_wk )
{ 
  return cp_wk->p_data->have;
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
    //カーソル、矢印出現
    { 
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_cursor, TRUE );

      if( p_wk->cursor == MYSTERY_CURSOR_RETURN )
      { 
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_allow, FALSE );
      }
      else
      { 
        if( p_wk->cursor == MYSTERY_CURSOR_LEFT_UP || p_wk->cursor == MYSTERY_CURSOR_LEFT_DOWN )
        { 
          GFL_CLACT_WK_SetTypePos( p_wk->p_allow,  16, CLSYS_DEFREND_MAIN, CLSYS_MAT_X );
          GFL_CLACT_WK_SetFlip( p_wk->p_allow, CLWK_FLIP_H, TRUE );
        }
        else if( p_wk->cursor == MYSTERY_CURSOR_RIGHT_UP || p_wk->cursor == MYSTERY_CURSOR_RIGHT_DOWN )
        { 
          GFL_CLACT_WK_SetTypePos( p_wk->p_allow,  240, CLSYS_DEFREND_MAIN, CLSYS_MAT_X );
          GFL_CLACT_WK_SetFlip( p_wk->p_allow, CLWK_FLIP_H, FALSE );
        }
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_allow, TRUE );
      }
    }

    //入力
    { 
      const int key = GFL_UI_KEY_GetRepeat();
      BOOL is_update  = FALSE;

      //タッチ入力
      if( GFL_UI_TP_GetTrg() )
      { 
        if( p_wk->p_card )
        { 
          MYSTERY_CARD_StartEffect( p_wk->p_card );
        }
      }

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

          GFL_CLACT_WK_SetDrawEnable( p_wk->p_cursor, FALSE );
          GFL_CLACT_WK_SetDrawEnable( p_wk->p_allow, FALSE );
          *p_seq  = SEQ_SCROLL;
        }
        if( p_wk->cursor != MYSTERY_CURSOR_RETURN )
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
          GFL_CLACT_WK_SetDrawEnable( p_wk->p_cursor, FALSE );
          GFL_CLACT_WK_SetDrawEnable( p_wk->p_allow, FALSE );
          *p_seq  = SEQ_SCROLL;
        }
        if( p_wk->cursor != MYSTERY_CURSOR_RETURN )
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
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_allow, FALSE );

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
          GFL_CLACT_WK_SetDrawEnable( p_wk->p_allow, FALSE );
          MYSTERY_SEQ_SetNext( p_seqwk, SEQFUNC_End );
        }
      }

      //カーソルの移動
      if( is_update )
      { 
        GFL_CLACTPOS  pos;
        if( p_wk->cursor < MYSTERY_CURSOR_MAX )
        { 
          pos.x  = sc_thumbnail_data[ p_wk->cursor ].card_x*8 - 24;
          pos.y  = sc_thumbnail_data[ p_wk->cursor ].card_y*8 - 8;

          GFL_CLACT_WK_SetAnmSeq( p_wk->p_cursor, 0);
        }
        else
        { 
          pos.x = 176;
          pos.y = 156;
          GFL_CLACT_WK_SetAnmSeq( p_wk->p_cursor, 12);
        }

        GFL_CLACT_WK_SetPos( p_wk->p_cursor, &pos, CLSYS_DEFREND_MAIN );
        p_wk->plt_cnt = 0;

        p_wk->is_card_update  = TRUE;
      }
    }
    break;

  case SEQ_SCROLL:
    if( Mystery_Album_ScrollPage( p_wk ) )
    { 
      //p_wk->is_card_update  = TRUE;
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
            pos.x  = sc_thumbnail_data[ p_wk->cursor ].card_x*8 - 24;
            pos.y  = sc_thumbnail_data[ p_wk->cursor ].card_y*8 - 8;

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
    SEQ_EFFECT_FADEOUT,
    SEQ_DELETE,
    SEQ_EFFECT_FADEIN,
    SEQ_EFFECT_WAIT,
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
    GX_SetVisibleWnd( GX_WNDMASK_W0 );
    { 
      const u8      src_pos = card_index % MYSTERY_CURSOR_MAX;
      GFL_RECT  src_rect;
      src_rect.left = (sc_thumbnail_data[ src_pos ].card_x)*8;
      src_rect.right = (sc_thumbnail_data[ src_pos ].card_x + MYSTERY_ALBUM_THUMBNAIL_CARD_W)*8;
      src_rect.top = (sc_thumbnail_data[ src_pos ].card_y)*8;
      src_rect.bottom = (sc_thumbnail_data[ src_pos ].card_y + MYSTERY_ALBUM_THUMBNAIL_CARD_H)*8;
      G2_SetWnd0InsidePlane( MYSTERY_ALBUM_SWAP_BRIGHTNESS_MASK, TRUE );
      G2_SetWnd0Position( src_rect.left, src_rect.top, src_rect.right, src_rect.bottom );
    }
    G2_SetWndOutsidePlane( MYSTERY_ALBUM_SWAP_BRIGHTNESS_MASK, FALSE );
    *p_seq  = SEQ_EFFECT_FADEOUT;
    break;

  case SEQ_EFFECT_FADEOUT:
    {
      int fade  = 0 + 16 * p_wk->cnt / MYSTERY_ALBUM_DELETE_SYNC;
      G2_SetBlendBrightness( MYSTERY_ALBUM_SWAP_BRIGHTNESS_MASK, fade );
      if( p_wk->cnt++ > MYSTERY_ALBUM_DELETE_SYNC )
      { 
        p_wk->cnt = 0;
        *p_seq  = SEQ_DELETE;
      }
    }
    break;

  case SEQ_DELETE:
    //削除
    Mystery_Album_RemoveCard( p_wk, card_index );

    //画面作り直し
    Mystery_Album_DeleteDisplay( p_wk, TRUE );
    Mystery_Album_CreateDisplay( p_wk, TRUE, 0, p_wk->now_page, p_wk->heapID );

    p_wk->is_card_update  = TRUE;
    *p_seq  = SEQ_EFFECT_FADEIN;
    break;

  case SEQ_EFFECT_FADEIN:
    {
      int fade  = 16 - 16 * p_wk->cnt / MYSTERY_ALBUM_DELETE_SYNC;
      G2_SetBlendBrightness( MYSTERY_ALBUM_SWAP_BRIGHTNESS_MASK, fade );
      if( p_wk->cnt++ > MYSTERY_ALBUM_SWAP_SYNC )
      { 
        p_wk->cnt = 0;
        *p_seq  = SEQ_EFFECT_WAIT;
      }
    }
    break;

  case SEQ_EFFECT_WAIT:
    GX_SetVisibleWnd( GX_WNDMASK_NONE );
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
    SEQ_EFFECT_INIT,
    SEQ_EFFECT_FADEOUT,
    SEQ_DELETE,
    SEQ_EFFECT_FADEIN,
    SEQ_EFFECT_WAIT,
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

    *p_seq  = SEQ_EFFECT_INIT;
    break;

  case SEQ_EFFECT_INIT:
    { 
      const u8      src_pos = card_index % MYSTERY_CURSOR_MAX;
      const u8      dst_pos = p_wk->swap_card_index % MYSTERY_CURSOR_MAX;
      const BOOL    is_wnd0 = Mystery_Album_GetIndexToPage( card_index ) == p_wk->now_page;
      const BOOL    is_wnd1 = Mystery_Album_GetIndexToPage( p_wk->swap_card_index ) == p_wk->now_page;

      NAGI_Printf( "入れ替え %d %d \n", card_index, p_wk->swap_card_index );

      {
        int wnd = 0;
        if( is_wnd0 )
        { 
          wnd |= GX_WNDMASK_W0;
        }
        if( is_wnd1 )
        { 
          wnd |= GX_WNDMASK_W1;
        }
        GX_SetVisibleWnd( wnd );
      }

      if( is_wnd0 )
      { 
        GFL_RECT  src_rect;
        src_rect.left = (sc_thumbnail_data[ src_pos ].card_x)*8;
        src_rect.right = (sc_thumbnail_data[ src_pos ].card_x + MYSTERY_ALBUM_THUMBNAIL_CARD_W)*8;
        src_rect.top = (sc_thumbnail_data[ src_pos ].card_y)*8;
        src_rect.bottom = (sc_thumbnail_data[ src_pos ].card_y + MYSTERY_ALBUM_THUMBNAIL_CARD_H)*8;
        G2_SetWnd0InsidePlane( MYSTERY_ALBUM_SWAP_BRIGHTNESS_MASK, TRUE );
        G2_SetWnd0Position( src_rect.left, src_rect.top, src_rect.right, src_rect.bottom );
      }
      if( is_wnd1 )
      { 
        GFL_RECT  dst_rect;
        dst_rect.left = (sc_thumbnail_data[ dst_pos ].card_x)*8;
        dst_rect.right = (sc_thumbnail_data[ dst_pos ].card_x + MYSTERY_ALBUM_THUMBNAIL_CARD_W)*8;
        dst_rect.top = (sc_thumbnail_data[ dst_pos ].card_y)*8;
        dst_rect.bottom = (sc_thumbnail_data[ dst_pos ].card_y + MYSTERY_ALBUM_THUMBNAIL_CARD_H)*8;
        G2_SetWnd1InsidePlane( MYSTERY_ALBUM_SWAP_BRIGHTNESS_MASK, TRUE );
        G2_SetWnd1Position( dst_rect.left, dst_rect.top, dst_rect.right, dst_rect.bottom );
      }
      G2_SetWndOutsidePlane( MYSTERY_ALBUM_SWAP_BRIGHTNESS_MASK, FALSE );
    }
    *p_seq  = SEQ_EFFECT_FADEOUT;
    break;

  case SEQ_EFFECT_FADEOUT:
    {
      int fade  = 0 + 16 * p_wk->cnt / MYSTERY_ALBUM_SWAP_SYNC;
      G2_SetBlendBrightness( MYSTERY_ALBUM_SWAP_BRIGHTNESS_MASK, fade );
      if( p_wk->cnt++ > MYSTERY_ALBUM_SWAP_SYNC )
      { 
        p_wk->cnt = 0;
        *p_seq  = SEQ_DELETE;
      }
    }
    break;

  case SEQ_DELETE:
    //スワップ
    Mystery_Album_SwapCard( p_wk, card_index, p_wk->swap_card_index );
    NAGI_Printf( "入れ替えます 今の%d 前の%d\n", card_index, p_wk->swap_card_index );

    //画面作り直し
    Mystery_Album_DeleteDisplay( p_wk, TRUE );
    Mystery_Album_CreateDisplay( p_wk, TRUE, 0, p_wk->now_page, p_wk->heapID );

    p_wk->is_card_update  = TRUE;
    *p_seq  = SEQ_EFFECT_FADEIN;
    break;

  case SEQ_EFFECT_FADEIN:
    {
      int fade  = 16 - 16 * p_wk->cnt / MYSTERY_ALBUM_SWAP_SYNC;
      G2_SetBlendBrightness( MYSTERY_ALBUM_SWAP_BRIGHTNESS_MASK, fade );
      if( p_wk->cnt++ > MYSTERY_ALBUM_SWAP_SYNC )
      { 
        p_wk->cnt = 0;
        *p_seq  = SEQ_EFFECT_WAIT;
      }
    }
    break;

  case SEQ_EFFECT_WAIT:
    GX_SetVisibleWnd( GX_WNDMASK_NONE );
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
    SEQ_INIT,
    SEQ_CHECK_MODE,

    SEQ_DELETE_MSG_INIT,
    SEQ_DELETE_YESNO_INIT,
    SEQ_DELETE_YESNO_MAIN,

    SEQ_CHECK_CHANGE,
    SEQ_SAVE_MSG,
    SEQ_SAVE_MSG_WAIT,
    SEQ_SAVE_INIT,
    SEQ_SAVE_MAIN,
    SEQ_EXIT,
  };

  MYSTERY_ALBUM_WORK *p_wk  = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_INIT:
    *p_seq  = SEQ_CHECK_MODE;
    break;

  case SEQ_CHECK_MODE:
    if( p_wk->setup.mode == MYSTERY_ALBUM_MODE_VIEW )
    { 
      *p_seq  = SEQ_CHECK_CHANGE;
    }
    else
    { 
      if( MYSTERYDATA_CheckCardDataSpace( p_wk->setup.p_sv ) )
      { 
        *p_seq  = SEQ_CHECK_CHANGE;
      }
      else
      { 
        *p_seq  = SEQ_DELETE_MSG_INIT;
      }
    }
    break;


  case SEQ_DELETE_MSG_INIT:
    if( p_wk->p_text == NULL )
    { 
      p_wk->p_text  = MYSTERY_TEXT_Init( MYSTERY_ALBUM_LIST_FRM, MYSTERY_ALBUM_FONT_PLT, p_wk->setup.p_que, p_wk->setup.p_font, HEAPID_MYSTERYGIFT );
      MYSTERY_TEXT_WriteWindowFrame( p_wk->p_text, 1, MYSTERY_ALBUM_BG_FRM_S_PLT ); 
    }
    MYSTERY_TEXT_Print( p_wk->p_text, p_wk->setup.p_msg, syachi_mystery_01_006, MYSTERY_TEXT_TYPE_STREAM ); 
    *p_seq  = SEQ_DELETE_YESNO_INIT;
    break;

  case SEQ_DELETE_YESNO_INIT:
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

      *p_seq  = SEQ_DELETE_YESNO_MAIN;
    }
    break;

  case SEQ_DELETE_YESNO_MAIN:
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
          *p_seq  = SEQ_CHECK_CHANGE;
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

  case SEQ_CHECK_CHANGE:
    if( p_wk->is_change )
    { 
      *p_seq  = SEQ_SAVE_MSG;
    }
    else
    { 
      *p_seq  = SEQ_EXIT;
    }
    break;

  case SEQ_SAVE_MSG:
    if( p_wk->p_text == NULL )
    { 
      p_wk->p_text  = MYSTERY_TEXT_Init( MYSTERY_ALBUM_LIST_FRM, MYSTERY_ALBUM_FONT_PLT, p_wk->setup.p_que, p_wk->setup.p_font, HEAPID_MYSTERYGIFT );
      MYSTERY_TEXT_WriteWindowFrame( p_wk->p_text, 1, MYSTERY_ALBUM_BG_FRM_S_PLT ); 
    }
    MYSTERY_TEXT_Print( p_wk->p_text, p_wk->setup.p_msg, syachi_mystery_01_016, MYSTERY_TEXT_TYPE_QUE );
    *p_seq  = SEQ_SAVE_MSG_WAIT;
    break;

  case SEQ_SAVE_MSG_WAIT:
    if( MYSTERY_TEXT_IsEndPrint( p_wk->p_text ) )
    { 
      *p_seq  = SEQ_SAVE_INIT;
    }
    break;

  case SEQ_SAVE_INIT:
    SaveControl_SaveAsyncInit( GAMEDATA_GetSaveControlWork(p_wk->setup.p_gamedata) );
    *p_seq  = SEQ_SAVE_MAIN;
    break;

  case SEQ_SAVE_MAIN:
    { 
      SAVE_RESULT result  = SaveControl_SaveAsyncMain(GAMEDATA_GetSaveControlWork(p_wk->setup.p_gamedata) );
      if( result == SAVE_RESULT_OK )
      { 
        *p_seq  = SEQ_EXIT;
      }
    }
    break;

  case SEQ_EXIT:

    if( p_wk->p_text )
    { 
      MYSTERY_TEXT_Exit( p_wk->p_text );
      p_wk->p_text  = NULL;
    }

    MYSTERY_SEQ_End( p_seqwk );
    break;
  }

  if( p_wk->p_text )
  { 
    MYSTERY_TEXT_Main( p_wk->p_text );
  }  
}
//----------------------------------------------------------------------------
/**
 *	@brief  メッセージ消去
 *
 *	@param	MYSTERY_SEQ_WORK *p_seqwk   シーケンス管理
 *	@param	*p_seq              シーケンス
 *	@param	*p_wk_adrs          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_DeleteMsg( MYSTERY_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  { 
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_EXIT,
  };

  MYSTERY_ALBUM_WORK *p_wk  = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_INIT:
    p_wk->p_text  = MYSTERY_TEXT_Init( MYSTERY_ALBUM_LIST_FRM, MYSTERY_ALBUM_FONT_PLT, p_wk->setup.p_que, p_wk->setup.p_font, HEAPID_MYSTERYGIFT );
    MYSTERY_TEXT_WriteWindowFrame( p_wk->p_text, 1, MYSTERY_ALBUM_BG_FRM_S_PLT );
    MYSTERY_TEXT_Print( p_wk->p_text, p_wk->setup.p_msg, syachi_mystery_01_005, MYSTERY_TEXT_TYPE_STREAM );
    break;

  case SEQ_MAIN:
    if( MYSTERY_TEXT_IsEndPrint( p_wk->p_text ) )
    { 
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
      {
        *p_seq  = SEQ_EXIT;
      }
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
#define MYSTERY_CARD_SILHOUETTE_EV1 (2)
#define MYSTERY_CARD_SILHOUETTE_EV2 (12)

#define MYSTERY_CARD_EFFECT_SYNC    (60)

#define MYSTERY_CARD_EFFECT_START_SEQ    (1)

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

  u16 seq;
  u16 cnt;

  u16 mons_no;
  u16 form_no;
  u32 voice_player;

  //シルエット用ポケモンパレット
  u16 plt[0x10];
  u16 plt_silhouette[0x10];
  u16 plt_pokemon[0x10];
  BOOL is_main;
  u32 plt_num;
  u32 silhouette_bg_pri;
  u32 font_bg_pri;
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
  p_wk->plt_num = cp_setup->silhouette_obj_plt_num;

  //描画先をチェック
  if( cp_setup->back_frm < GFL_BG_FRAME0_S )
  { 
    draw_type = CLSYS_DRAW_MAIN;
    paltype   = PALTYPE_MAIN_BG;
    p_wk->is_main = TRUE;
  }
  else
  { 
    draw_type = CLSYS_DRAW_SUB;
    paltype   = PALTYPE_SUB_BG;
    p_wk->is_main = FALSE;
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
    setup.p_gamedata  = NULL;
 
    MYSTERY_CARD_DATA_Init( &p_wk->data, cp_setup->p_data, &setup, heapID );
  }

  

  //BG読み込み追加
  {
    //上画面ならばもらったとき。下画面ならばアルバムの中
    const u16 load_scr  = cp_setup->back_frm > GFL_BG_FRAME3_M ?
      NARC_mystery_fushigi_card_album_NSCR: NARC_mystery_fushigi_card_view_NSCR;

    //もらったものならば、ハンコ付き。もらっていないならばハンコなし
    const u16 src_ofs = MYSTERY_CARD_DATA_IsHave( &p_wk->data ) ? 0x20: 0;

    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_MYSTERY, heapID );
    //BG
		GFL_ARCHDL_UTIL_TransVramPaletteEx( p_handle, NARC_mystery_fushigi_card_NCLR,
				paltype, src_ofs, cp_setup->back_plt_num*0x20, 0x20, heapID );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_mystery_fushigi_card_NCGR, 
				cp_setup->back_frm, 0, 0, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, load_scr,
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
        6,
        2,
        10,
        2,
        syachi_mystery_album_006,
        NULL,
        MYSTERY_MSGWIN_POS_ABSOLUTE,
        0, 0,
        MYSTERY_MSGWIN_WHITE_COLOR
      },
      //イベント名
      { 
        2,
        6,
        28,
        2,
        0,
        NULL,
        MYSTERY_MSGWIN_POS_ABSOLUTE,
        0, 0,
        MYSTERY_MSGWIN_DEFAULT_COLOR
      },
      //本文
      { 
        2,
        9,
        28,
        10,
        0,
        NULL,
        MYSTERY_MSGWIN_POS_ABSOLUTE,
        0, 0,
        MYSTERY_MSGWIN_DEFAULT_COLOR
      },
      //もらったひづけ
      { 
        9,
        20,
        11,
        2,
        syachi_mystery_album_007,
        NULL,
        MYSTERY_MSGWIN_POS_ABSOLUTE,
        0, 0,
        MYSTERY_MSGWIN_WHITE_COLOR
      },
      //日付
      { 
        20,
        20,
        9,
        2,
        0,
        NULL,
        MYSTERY_MSGWIN_POS_ABSOLUTE,
        0, 0,
        MYSTERY_MSGWIN_WHITE_COLOR
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
      cldata.pos_x  = 216;
      cldata.pos_y  = 26;

      p_wk->p_icon    = GFL_CLACT_WK_Create( cp_setup->p_clunit, 
          p_wk->res_icon_cgx, p_wk->res_icon_plt, p_wk->res_icon_cel,
          &cldata, draw_type, heapID );
      GFL_CLACT_WK_SetPlttOffs( p_wk->p_icon,
              MYSTERY_CARD_DATA_GetPltOfs( &p_wk->data ), CLWK_PLTTOFFS_MODE_OAM_COLOR );
      GFL_CLACT_WK_SetObjMode( p_wk->p_icon, GX_OAM_MODE_NORMAL );

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

      p_wk->mons_no = cp_pokemon->mons_no;
      p_wk->form_no = cp_pokemon->form_no;
      
      //カーソルのパレットフェード用にパレットをメモリ読み込み
      { 
        u16 plt_arcID;
        void *p_buff;
        NNSG2dPaletteData *p_plt;
        const u16 *cp_plt_adrs;

        plt_arcID	= POKEGRA_GetPalArcIndex( cp_pokemon->mons_no, cp_pokemon->form_no, cp_pokemon->sex, cp_pokemon->rare, POKEGRA_DIR_FRONT, cp_pokemon->egg );

        //もとのパレットから色情報を保存
        p_buff  = GFL_ARCHDL_UTIL_LoadPalette( p_handle, plt_arcID, &p_plt, heapID );
        cp_plt_adrs = p_plt->pRawData;
        GFL_STD_MemCopy( cp_plt_adrs, p_wk->plt_pokemon, sizeof(u16) * 0x10 );
        GFL_STD_MemFill16( p_wk->plt_silhouette, GX_RGB( 0, 0, 0), 0x10 );
        //パレット破棄
        GFL_HEAP_FreeMemory( p_buff );
      }

      GFL_ARC_CloseDataHandle( p_handle );
    }
    { 
      GFL_CLWK_DATA cldata;
      GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
      cldata.pos_x  = 184;
      cldata.pos_y  = 112;
      cldata.bgpri  = cp_setup->back_frm % GFL_BG_FRAME0_S;
      p_wk->silhouette_bg_pri = cldata.bgpri;
      p_wk->font_bg_pri = cp_setup->font_frm % GFL_BG_FRAME0_S;
      p_wk->p_silhouette   = GFL_CLACT_WK_Create( cp_setup->p_clunit, 
          p_wk->res_silhouette_cgx, p_wk->res_silhouette_plt, p_wk->res_silhouette_cel,
          &cldata, draw_type, heapID );
      GFL_CLACT_WK_SetObjMode( p_wk->p_silhouette, GX_OAM_MODE_XLU );
    }


    { 
      int i;
      u16 *p_plt_addr;
      
      if( cp_setup->back_frm < GFL_BG_FRAME0_S )
      { 
        p_plt_addr  = (u16*)(HW_OBJ_PLTT + GFL_CLGRP_PLTT_GetAddr( p_wk->res_silhouette_plt, draw_type ));
      }
      else
      { 
        p_plt_addr  = (u16*)(HW_DB_OBJ_PLTT + GFL_CLGRP_PLTT_GetAddr( p_wk->res_silhouette_plt, draw_type ));
      }

      for( i = 0; i < 0x10; i++ )
      { 
        p_plt_addr[i] = p_wk->plt_silhouette[i];
      }
    }

    if( cp_setup->back_frm < GFL_BG_FRAME0_S )
    { 
      u8 back_frm = cp_setup->back_frm - GFL_BG_FRAME0_M;
      u8 font_frm = cp_setup->font_frm - GFL_BG_FRAME0_M;
      G2_SetBlendAlpha(
          GX_BLEND_PLANEMASK_NONE,
          (1 << back_frm) | (1 << font_frm),
          MYSTERY_CARD_SILHOUETTE_EV1,
          MYSTERY_CARD_SILHOUETTE_EV2
          );
    }
    else
    { 
      u8 back_frm = cp_setup->back_frm - GFL_BG_FRAME0_S;
      u8 font_frm = cp_setup->font_frm - GFL_BG_FRAME0_S;
      G2S_SetBlendAlpha(
          GX_BLEND_PLANEMASK_NONE,
          (1 << back_frm) | (1 << font_frm),
          MYSTERY_CARD_SILHOUETTE_EV1,
          MYSTERY_CARD_SILHOUETTE_EV2
          );
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

  //シルエット
  if( p_wk->p_silhouette )
  { 
    //アルファ無効
    if( p_wk->is_main )
    { 
      G2_BlendNone();
    }
    else
    { 
      G2S_BlendNone();
    }

    //シルエット破棄
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
  enum
  { 
    SEQ_INIT = MYSTERY_CARD_EFFECT_START_SEQ,
    SEQ_START_FADEIN,
    SEQ_WAIT_FADEIN,
    SEQ_START_VOICE,
    SEQ_WAIT_VOICE,
    SEQ_START_FADEOUT,
    SEQ_WAIT_FADEOUT,
    SEQ_EXIT,
  };

  MYSTERY_MSGWINSET_PrintMain( p_wk->p_winset );


  { 
    NNS_GFD_DST_TYPE  gfd_type;
    void (*ChangeBlendAlpha)( int, int );
    if( p_wk->is_main )
    { 
      gfd_type  = NNS_GFD_DST_2D_OBJ_PLTT_MAIN;
      ChangeBlendAlpha  = G2_ChangeBlendAlpha;
    }
    else
    { 
      gfd_type  = NNS_GFD_DST_2D_OBJ_PLTT_SUB;
      ChangeBlendAlpha  = G2S_ChangeBlendAlpha;
    }
	
	  switch( p_wk->seq )
	  { 
	  case SEQ_INIT:
	    GFL_CLACT_WK_SetBgPri( p_wk->p_silhouette, p_wk->font_bg_pri );
	    p_wk->seq = SEQ_START_FADEIN;
	    break;
	  case SEQ_START_FADEIN:
	    //パレットフェード
	    { 
	      int i;
	      u16 fade = 0 + 0xFFFF/2 * p_wk->cnt / MYSTERY_CARD_EFFECT_SYNC;
	      for( i = 0; i < 0x10; i++ )
	      { 
	        Mystery_MainPltAnm( gfd_type, &p_wk->plt[i], fade, p_wk->plt_num, i, p_wk->plt_pokemon[i], p_wk->plt_silhouette[i] );
	      }
	
	    }
	    //アルファフェード
	    { 
	      int ev1, ev2;
	      ev1 = MYSTERY_CARD_SILHOUETTE_EV1 + (16-MYSTERY_CARD_SILHOUETTE_EV1) * p_wk->cnt / MYSTERY_CARD_EFFECT_SYNC;
	      ev2 = MYSTERY_CARD_SILHOUETTE_EV2 + (0-MYSTERY_CARD_SILHOUETTE_EV2) * p_wk->cnt / MYSTERY_CARD_EFFECT_SYNC;
	      ChangeBlendAlpha( ev1, ev2 );
	    }
	    //カウンタ
	    if( p_wk->cnt++ > MYSTERY_CARD_EFFECT_SYNC )
	    { 
	      p_wk->cnt = 0;
	      p_wk->seq = SEQ_WAIT_FADEIN;
	    }
	    break;
	  case SEQ_WAIT_FADEIN:
	    p_wk->seq = SEQ_START_VOICE;
	    break;
	  case SEQ_START_VOICE:
	    p_wk->voice_player  = PMV_PlayVoice( p_wk->mons_no, p_wk->form_no );
	    p_wk->seq = SEQ_WAIT_VOICE;
	    break;
	  case SEQ_WAIT_VOICE:
	    if( !PMVOICE_CheckPlay( p_wk->voice_player ) )
	    { 
	      p_wk->seq = SEQ_START_FADEOUT;
	    }
	    break;
	  case SEQ_START_FADEOUT:
	    //パレットフェード
	    { 
	      int i;
	      u16 fade = 0xFFFF/2 - 0xFFFF/2 * p_wk->cnt / MYSTERY_CARD_EFFECT_SYNC;
	      for( i = 0; i < 0x10; i++ )
	      { 
	        Mystery_MainPltAnm( gfd_type, &p_wk->plt[i], fade, p_wk->plt_num, i, p_wk->plt_pokemon[i], p_wk->plt_silhouette[i] );
	      }
	
	    }
	    //アルファフェード
	    { 
	      int ev1, ev2;
	      ev1 = 16 + (MYSTERY_CARD_SILHOUETTE_EV1-16) * p_wk->cnt / MYSTERY_CARD_EFFECT_SYNC;
	      ev2 = 0 + (MYSTERY_CARD_SILHOUETTE_EV2-0) * p_wk->cnt / MYSTERY_CARD_EFFECT_SYNC;
	      ChangeBlendAlpha( ev1, ev2 );
	    }
	    //カウンタ
	    if( p_wk->cnt++ > MYSTERY_CARD_EFFECT_SYNC )
	    { 
	      p_wk->cnt = 0;
	      p_wk->seq = SEQ_WAIT_FADEOUT;
	    }
	    break;
	  case SEQ_WAIT_FADEOUT:
	    p_wk->seq = SEQ_EXIT;
	    break;
	  case SEQ_EXIT:
	    GFL_CLACT_WK_SetBgPri( p_wk->p_silhouette, p_wk->silhouette_bg_pri );
	    p_wk->seq = 0;
	    break;
	  }
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  エフェクト開始
 *
 *	@param	MYSTERY_CARD_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void MYSTERY_CARD_StartEffect( MYSTERY_CARD_WORK *p_wk )
{ 
  if( p_wk->seq == 0 )
  { 
    p_wk->seq     = MYSTERY_CARD_EFFECT_START_SEQ;
    p_wk->cnt     = 0;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  エフェクト終了街
 *
 *	@param	const MYSTERY_CARD_WORK *cp_wk ワーク
 *	@retval TRUEエフェクト終了  FALSEエフェクト動作中
 */
//-----------------------------------------------------------------------------
BOOL MYSTERY_CARD_IsEndEffect( const MYSTERY_CARD_WORK *cp_wk )
{ 
  return cp_wk->seq == 0;
}
