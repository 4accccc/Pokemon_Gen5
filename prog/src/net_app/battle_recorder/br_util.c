//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_util.c
 *	@brief  各PROCで使う共通モジュール系
 *	@author	Toru=Nagihashi
 *	@data		2009.11.17
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>

//システム
#include "system/pms_draw.h"
#include "app/app_menu_common.h"
#include "pokeicon/pokeicon.h"
#include "system/nsbtx_to_clwk.h"

//自分のモジュール
#include "br_inner.h"
#include "br_snd.h"

//アーカイブ
#include "arc_def.h"
#include "msg/msg_battle_rec.h"
#include "wifi_unionobj_plt.cdat" //ユニオンOBJのパレット位置
#include "wifi_unionobj.naix"
#include "wifileadingchar.naix"

//外部公開
#include "br_util.h"
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					BMPWINメッセージ描画構造体
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	メッセージウィンドウ
//=====================================
struct _BR_MSGWIN_WORK
{
	PRINT_UTIL				util;				//文字描画
	PRINT_QUE					*p_que;			//文字キュー
	GFL_BMPWIN*				p_bmpwin;		//BMPWIN
	STRBUF*						p_strbuf;		//文字バッファ
	u16								clear_chr;	//クリアキャラ
	u16								frm;        //BG面
  BR_MSGWIN_POS     pos_type;   //位置の種類
  GFL_POINT         pos;        //位置
};
//-------------------------------------
///	プライベート
//=====================================
static void Br_MsgWin_CalcPos( BR_MSGWIN_POS  pos_type, const GFL_POINT *cp_ofs, GFL_BMP_DATA *p_bmp, STRBUF *p_strbuf, GFL_FONT *p_font, GFL_POINT *p_pos );

//-------------------------------------
///	パブリック
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  BMPWINメッセージ  初期化
 *
 *	@param	u16 frm BG面
 *	@param	x       X座標
 *	@param	y       Y座標
 *	@param	w       幅
 *	@param	h       高さ
 *	@param	plt     フォントパレット
 *	@param	*p_que  キュー
 *	@param	heapID  ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
BR_MSGWIN_WORK * BR_MSGWIN_Init( u16 frm, u8 x, u8 y, u8 w, u8 h, u8 plt, PRINT_QUE *p_que, HEAPID heapID )
{ 
  BR_MSGWIN_WORK *p_wk;

  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(BR_MSGWIN_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(BR_MSGWIN_WORK) );
  p_wk->clear_chr = 0;
  p_wk->p_que			= p_que;
  p_wk->frm       = frm;

  //バッファ作成
	p_wk->p_strbuf	= GFL_STR_CreateBuffer( 255, heapID );

	//BMPWIN作成
	p_wk->p_bmpwin	= GFL_BMPWIN_Create( frm, x, y, w, h, plt, GFL_BMP_CHRAREA_GET_B );

	//プリントキュー設定
	PRINT_UTIL_Setup( &p_wk->util, p_wk->p_bmpwin );

	//転送
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );
	GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  BMPWINメッセージ  破棄
 *
 *	@param	BR_MSGWIN_WORK* p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_MSGWIN_Exit( BR_MSGWIN_WORK* p_wk )
{ 
  //BMPWIN破棄
  GFL_BMPWIN_ClearScreen( p_wk->p_bmpwin );
	GFL_BMPWIN_Delete( p_wk->p_bmpwin );

	//バッファ破棄
	GFL_STR_DeleteBuffer( p_wk->p_strbuf );

  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  BMPWINメッセージ  プリント開始
 *
 *	@param	BR_MSGWIN_WORK* p_wk ワーク
 *	@param	*p_msg            メッセージ
 *	@param	strID             メッセージID
 *	@param	*p_font           フォント
 */
//-----------------------------------------------------------------------------
void BR_MSGWIN_Print( BR_MSGWIN_WORK* p_wk, GFL_MSGDATA *p_msg, u32 strID, GFL_FONT *p_font )
{ 
  GFL_POINT pos;

	//一端消去
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );	

	//文字列作成
	GFL_MSG_GetString( p_msg, strID, p_wk->p_strbuf );

  //描画位置計算
  Br_MsgWin_CalcPos( p_wk->pos_type, &p_wk->pos, GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->p_strbuf, p_font, &pos );

	//文字列描画
	PRINT_UTIL_Print( &p_wk->util, p_wk->p_que, pos.x, pos.y, p_wk->p_strbuf, p_font );
}
//----------------------------------------------------------------------------
/**
 *	@brief  BMPWINメッセージ  STRBUFプリント開始
 *
 *	@param	BR_MSGWIN_WORK* p_wk ワーク
 *	@param	*p_strbuf         STRBUF  すぐ破棄してもかまいません
 *	@param	*p_font           フォント
 */
//-----------------------------------------------------------------------------
void BR_MSGWIN_PrintBuf( BR_MSGWIN_WORK* p_wk, const STRBUF *cp_strbuf, GFL_FONT *p_font )
{ 
  GFL_POINT pos;

	//一端消去
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );	

	//文字列作成
  GFL_STR_CopyBuffer( p_wk->p_strbuf, cp_strbuf );

  //描画位置計算
  Br_MsgWin_CalcPos( p_wk->pos_type, &p_wk->pos, GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->p_strbuf, p_font, &pos );

	//文字列描画
	PRINT_UTIL_Print( &p_wk->util, p_wk->p_que, pos.x, pos.y, p_wk->p_strbuf, p_font );
}
//----------------------------------------------------------------------------
/**
 *	@brief  BMPWINメッセージ  プリント開始  色指定版
 *
 *	@param	BR_MSGWIN_WORK* p_wk ワーク
 *	@param	*p_msg            メッセージ
 *	@param	strID             メッセージID
 *	@param	*p_font           フォント
 *	@param  lsb               色指定
 */
//-----------------------------------------------------------------------------
void BR_MSGWIN_PrintColor( BR_MSGWIN_WORK* p_wk, GFL_MSGDATA *p_msg, u32 strID, GFL_FONT *p_font, PRINTSYS_LSB lsb )
{ 
  GFL_POINT pos;
  
	//一端消去
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );	

	//文字列作成
	GFL_MSG_GetString( p_msg, strID, p_wk->p_strbuf );

  //描画位置計算
  Br_MsgWin_CalcPos( p_wk->pos_type, &p_wk->pos, GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->p_strbuf, p_font, &pos );

	//文字列描画
	PRINT_UTIL_PrintColor( &p_wk->util, p_wk->p_que, pos.x, pos.y, p_wk->p_strbuf, p_font, lsb );
}
//----------------------------------------------------------------------------
/**
 *	@brief  BMPWINメッセージ  STRBUFプリント開始
 *
 *	@param	BR_MSGWIN_WORK* p_wk ワーク
 *	@param	*p_strbuf         STRBUF  すぐ破棄してもかまいません
 *	@param	*p_font           フォント
 *	@param  lsb               色指定
 */
//-----------------------------------------------------------------------------
void BR_MSGWIN_PrintBufColor( BR_MSGWIN_WORK* p_wk, const STRBUF *cp_strbuf, GFL_FONT *p_font, PRINTSYS_LSB lsb )
{ 
  GFL_POINT pos;

	//一端消去
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );	

	//文字列作成
  GFL_STR_CopyBuffer( p_wk->p_strbuf, cp_strbuf );

  //描画位置計算
  Br_MsgWin_CalcPos( p_wk->pos_type, &p_wk->pos, GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->p_strbuf, p_font, &pos );

	//文字列描画
	PRINT_UTIL_PrintColor( &p_wk->util, p_wk->p_que, pos.x, pos.y, p_wk->p_strbuf, p_font, lsb );
}
//----------------------------------------------------------------------------
/**
 *	@brief  BMPWINメッセージ  プリント書き込み処理
 *
 *	@param	BR_MSGWIN_WORK* p_wk   ワーク
 *
 *	@return TRUEならば書き込んだ  FALSEならばまだ
 */
//-----------------------------------------------------------------------------
BOOL BR_MSGWIN_PrintMain( BR_MSGWIN_WORK* p_wk )
{ 
	return PRINT_UTIL_Trans( &p_wk->util, p_wk->p_que );
}

//----------------------------------------------------------------------------
/**
 *	@brief  BMPWINメッセージ  位置設定
 *
 *	@param	BR_MSGWIN_WORK* p_wk  ワーク
 *	@param	x                  位置
 *	@param	y                  位置
 *	@param	type                位置の種類
 */
//-----------------------------------------------------------------------------
void BR_MSGWIN_SetPos( BR_MSGWIN_WORK* p_wk, s16 x, s16 y, BR_MSGWIN_POS type )
{ 
  p_wk->pos_type  = type;
  p_wk->pos.x       = x;
  p_wk->pos.y       = y;
}

//----------------------------------------------------------------------------
/**
 *	@brief  描画位置計算
 *
 *	@param	BR_MSGWIN_POS  pos_type 位置の種類
 *	@param	GFL_POINT *cp_ofs       位置
 *	@param	*p_bmp                  書き込みBMP
 *	@param	*p_strbuf               書き込むSTR
 *	@param	*p_font                 使用するFONT
 *	@param	*p_pos                  位置受け取り
 */
//-----------------------------------------------------------------------------
static void Br_MsgWin_CalcPos( BR_MSGWIN_POS  pos_type, const GFL_POINT *cp_ofs, GFL_BMP_DATA *p_bmp, STRBUF *p_strbuf, GFL_FONT *p_font, GFL_POINT *p_pos )
{ 
  switch( pos_type )
  { 
  case BR_MSGWIN_POS_ABSOLUTE:
    *p_pos  = *cp_ofs;
    break;
  case BR_MSGWIN_POS_WH_CENTER:
    { 
      u32 x, y;
      x = GFL_BMP_GetSizeX( p_bmp ) / 2;
      y = GFL_BMP_GetSizeY( p_bmp ) / 2;
      x -= PRINTSYS_GetStrWidth( p_strbuf, p_font, 0 ) / 2;
      y -= PRINTSYS_GetStrHeight( p_strbuf, p_font ) / 2;
      p_pos->x  = x + cp_ofs->x;
      p_pos->y  = y + cp_ofs->y;
    }
    break;
  }
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					  リスト表示構造体
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	CLWKの数
//=====================================
enum
{
  BR_LIST_CLWK_ALLOW_U,
  BR_LIST_CLWK_ALLOW_D,
  BR_LIST_CLWK_ALLOW_MAX,
  BR_LIST_CLWK_BAR_L  = BR_LIST_CLWK_ALLOW_MAX,
  BR_LIST_CLWK_BAR_R,
  BR_LIST_CLWK_BAR_MAX,
  BR_LIST_CLWK_MAX  = BR_LIST_CLWK_BAR_MAX,
};

//-------------------------------------
///	スライダーの入力開始距離
//=====================================
#define BR_LIST_SLIDE_DISTANCE  (8)

#define BR_LIST_HITTBL_MAX  (10)

typedef BOOL (*BR_LIST_ENABLE_FUNC)( const BR_LIST_WORK *cp_wk );
typedef BOOL (*BR_LIST_MOVE_FUNC)( BR_LIST_WORK *p_wk, s8 value );
typedef void (*BR_LIST_SELECT_FUNC)( BR_LIST_WORK *p_wk );
//-------------------------------------
///	リスト
//=====================================
struct _BR_LIST_WORK
{ 
  BR_LIST_PARAM       param;
  GFL_CLWK            *p_clwk[BR_LIST_CLWK_MAX];
  GFL_BMPWIN          *p_bmpwin;

  u16   min;
  u16   max;
  u16   cursor;   //ページ中のどこにいるか
  u16   list;     //何項目目にいるか 
  u16   line_max; //1ページ何ラインか
  u32   select_param; //選択したもの
  s8    value;
  u8    is_allow_visible;
  s8    dumy[2];
  
  BR_LIST_MOVE_FUNC   move_callback;
  BR_LIST_ENABLE_FUNC enable_callback;
  BR_LIST_SELECT_FUNC select_callback;

  GFL_UI_TP_HITTBL    hittbl[BR_LIST_HITTBL_MAX];

  BOOL      is_hit;   //スライダー
  u16       trg_y;    //スライダートリガ座標
  GFL_BMP_DATA        *p_bmp[0];

};
//-------------------------------------
///	プロトタイプ
//=====================================
static void Br_List_Write( BR_LIST_WORK *p_wk, u16 now );
static s8 Br_List_Slide( BR_LIST_WORK* p_wk );
static void BMP_Copy( const GFL_BMP_DATA *cp_src, GFL_BMP_DATA *p_dst, u16 src_x, u16 src_y, u16 dst_x, u16 dst_y, u16 w, u16 h );

static BOOL Br_TouchList_MoveCallback( BR_LIST_WORK *p_wk, s8 value );
static BOOL Br_CursorList_MoveCallback( BR_LIST_WORK *p_wk, s8 value );

static BOOL Br_TouchList_EnableCallback( const BR_LIST_WORK *cp_wk );
static BOOL Br_CursorList_EnableCallback( const BR_LIST_WORK *cp_wk );

static void Br_TouchList_SelectCallback( BR_LIST_WORK *p_wk );
static void Br_CursorList_SelectCallback( BR_LIST_WORK *p_wk );
//-------------------------------------
///	データ
//=====================================
static const GFL_POINT sc_cursor_pos[]  =
{ 
  { 
    24,76
  },
  { 
    24,100
  },
  { 
    24,124
  },
  { 
    24,148
  },
  { 
    24,172
  },
};

//----------------------------------------------------------------------------
/**
 *	@brief  リスト初期化
 *
 *	@param	const BR_LIST_PARAM *cp_param リスト引数
 *	@param	heapID                        ヒープ
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
BR_LIST_WORK * BR_LIST_Init( const BR_LIST_PARAM *cp_param, HEAPID heapID )
{ 
  BR_LIST_WORK  *p_wk;

  //ワーク確保、初期化
  { 
    u32 size;

    size  = sizeof(BR_LIST_WORK) + sizeof(GFL_BMP_DATA*) * cp_param->list_max;
    p_wk  = GFL_HEAP_AllocMemory( heapID, size );
    GFL_STD_MemClear( p_wk, size );
    p_wk->param     = *cp_param;
    p_wk->min       = 0;
    p_wk->max       = cp_param->list_max;
    p_wk->line_max  = cp_param->h / cp_param->str_line;

    p_wk->line_max  = MATH_IMin( p_wk->line_max, cp_param->list_max );

    if( cp_param->p_pos )
    { 
      p_wk->list      = cp_param->p_pos->list;
      p_wk->cursor    = cp_param->p_pos->cursor;
    }
  }

  //タイプ別設定
  if( p_wk->param.type == BR_LIST_TYPE_TOUCH )
  { 
    int i;
    for( i = 0; i < p_wk->line_max; i++ )
    {
      GF_ASSERT( i < BR_LIST_HITTBL_MAX )
      p_wk->hittbl[ i ].rect.left    = (p_wk->param.x)*8;
      p_wk->hittbl[ i ].rect.top     = (p_wk->param.y + i * p_wk->param.str_line)*8;
      p_wk->hittbl[ i ].rect.right   = (p_wk->param.x + p_wk->param.w)*8;
      p_wk->hittbl[ i ].rect.bottom  = (p_wk->param.y + (i+1) * (p_wk->param.str_line))*8;
    }
    GF_ASSERT( i < BR_LIST_HITTBL_MAX )
    p_wk->hittbl[ i ].rect.left    = GFL_UI_TP_HIT_NONE;
    p_wk->hittbl[ i ].rect.top     = GFL_UI_TP_HIT_NONE;
    p_wk->hittbl[ i ].rect.right   = GFL_UI_TP_HIT_NONE;
    p_wk->hittbl[ i ].rect.bottom  = GFL_UI_TP_HIT_NONE;

    p_wk->move_callback   = Br_TouchList_MoveCallback;
    p_wk->enable_callback = Br_TouchList_EnableCallback;
    p_wk->select_callback = Br_TouchList_SelectCallback;
  }
  else if( p_wk->param.type == BR_LIST_TYPE_CURSOR )
  { 
    p_wk->move_callback   = Br_CursorList_MoveCallback;
    p_wk->enable_callback = Br_CursorList_EnableCallback;
    p_wk->select_callback = Br_CursorList_SelectCallback;

    if( p_wk->param.p_balleff_main )
    { 
      BR_BALLEFF_StartMove( p_wk->param.p_balleff_main, BR_BALLEFF_MOVE_CIRCLE, &sc_cursor_pos[ p_wk->cursor ] );
    }
  }

  //BMPWIN作成
  {
    p_wk->p_bmpwin  = GFL_BMPWIN_Create( cp_param->frm, cp_param->x, cp_param->y, cp_param->w, cp_param->h, cp_param->plt, GFL_BMP_CHRAREA_GET_B );
    GFL_BMPWIN_MakeScreen( p_wk->p_bmpwin );
    GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(p_wk->p_bmpwin) );
  }

  //矢印作成
  { 
    int i;
    GFL_CLWK_DATA cldata;
    BR_RES_OBJ_DATA res;
    CLSYS_DRAW_TYPE type;
    BOOL ret;

    GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

    if( cp_param->frm < GFL_BG_FRAME0_S )
    { 
      BR_RES_LoadOBJ( p_wk->param.p_res, BR_RES_OBJ_ALLOW_M, heapID );
      ret = BR_RES_GetOBJRes( p_wk->param.p_res, BR_RES_OBJ_ALLOW_M, &res );
      GF_ASSERT( ret );
      type  = CLSYS_DRAW_MAIN;
    }
    else
    { 
      BR_RES_LoadOBJ( p_wk->param.p_res, BR_RES_OBJ_ALLOW_S, heapID );
      ret = BR_RES_GetOBJRes( p_wk->param.p_res, BR_RES_OBJ_ALLOW_S, &res );
      GF_ASSERT( ret );
      type  = CLSYS_DRAW_SUB;
    }

    cldata.pos_x    = 128;
    cldata.softpri  = 0;
    for( i = 0; i < BR_LIST_CLWK_ALLOW_MAX; i++ )
    { 
      if( cp_param->type == BR_LIST_TYPE_TOUCH )
      { 
        cldata.pos_y    = i == 0 ? 16: 128;
      }
      else
      { 
        cldata.pos_y    = i == 0 ? 64: 184;
      }
      cldata.anmseq   = i;

      p_wk->p_clwk[i] = GFL_CLACT_WK_Create( cp_param->p_unit, 
				res.ncg, res.ncl, res.nce, 
				&cldata, type, heapID );
      GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk[i], TRUE );
      GFL_CLACT_WK_SetObjMode( p_wk->p_clwk[i], GX_OAM_MODE_XLU );
    }
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[BR_LIST_CLWK_ALLOW_U], FALSE );
  }

  //バー作成
  { 
    int i;
    GFL_CLWK_DATA cldata;
    BR_RES_OBJ_DATA res;
    BOOL ret;

    GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

    cldata.pos_y    = 72;
    cldata.anmseq   = 5;
    cldata.softpri  = 0;

    ret = BR_RES_GetOBJRes( p_wk->param.p_res, BR_RES_OBJ_SHORT_BTN_S, &res );
    GF_ASSERT( ret );

    for( i = BR_LIST_CLWK_BAR_L; i < BR_LIST_CLWK_BAR_MAX; i++ )
    { 
      cldata.pos_x    = i == BR_LIST_CLWK_BAR_L? 24: 232;
      p_wk->p_clwk[i]  = GFL_CLACT_WK_Create( p_wk->param.p_unit, 
				res.ncg, res.ncl, res.nce, 
				&cldata, CLSYS_DRAW_SUB, heapID );
      GFL_CLACT_WK_SetObjMode( p_wk->p_clwk[i], GX_OAM_MODE_XLU );
    }
  }

  //文字をBMP化しておく
  { 
    int i;
    GFL_FONT  *p_font;
    const STRBUF *cp_strbuf;

    p_font  = BR_RES_GetFont( p_wk->param.p_res );

    for( i = 0; i < p_wk->param.list_max; i++ )
    { 
      cp_strbuf = p_wk->param.cp_list[i].str;

      if( cp_strbuf != NULL )
      { 
        p_wk->p_bmp[i]  = GFL_BMP_Create( p_wk->param.w, p_wk->param.str_line, GFL_BMP_16_COLOR, heapID );
        GFL_BMP_Clear( p_wk->p_bmp[i], 0 );
        PRINTSYS_PrintColor( p_wk->p_bmp[i], 0, 0, cp_strbuf, p_font, BR_PRINT_COL_NORMAL );
      }
    }
  }

  //初期表示をプリント
  { 
    Br_List_Write( p_wk, p_wk->list );
  }

  //リストが表示より少ないならば、矢印を消す
  { 
    if(  cp_param->list_max < p_wk->line_max )
    { 
      p_wk->is_allow_visible  =   FALSE;
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[BR_LIST_CLWK_ALLOW_U], FALSE );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[BR_LIST_CLWK_ALLOW_D], FALSE );
    }
    else
    { 
      p_wk->is_allow_visible  =   TRUE;

      //一番上まできたら、↑の矢印を
      //一番下まできたら、↓の矢印を消す
      if( p_wk->is_allow_visible )
      { 
        if( 0 == p_wk->list )
        { 
          GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[BR_LIST_CLWK_ALLOW_U], FALSE );
        }
        else
        { 
          GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[BR_LIST_CLWK_ALLOW_U], TRUE );
        }

        if( p_wk->list + p_wk->line_max == p_wk->param.list_max )
        { 
          GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[BR_LIST_CLWK_ALLOW_D], FALSE );
        }
        else
        { 
          GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[BR_LIST_CLWK_ALLOW_D], TRUE );
        }
      }
    }
  }

  //リストが動作するならば、バーをアニメON
  { 
    if( BR_LIST_IsMoveEnable( p_wk ) )
    { 
      int i;
      for( i = BR_LIST_CLWK_BAR_L; i < BR_LIST_CLWK_BAR_MAX; i++ )
      { 
        GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk[i], TRUE );
      }
    }
  }

  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  リスト破棄
 *
 *	@param	BR_LIST_WORK* p_wk 破棄するワーク
 */
//-----------------------------------------------------------------------------
void BR_LIST_Exit( BR_LIST_WORK* p_wk )
{ 
  int i;

  if( p_wk->param.p_balleff_main )
  { 
    BR_BALLEFF_StartMove( p_wk->param.p_balleff_main, BR_BALLEFF_MOVE_NOP, NULL );
  }

  for( i = 0; i < BR_LIST_CLWK_MAX; i++ )
  { 
    GFL_CLACT_WK_Remove( p_wk->p_clwk[i] );
  }

  for( i = 0; i < p_wk->param.list_max; i++ )
  { 
    if( p_wk->p_bmp[i] && p_wk->param.cp_list[i].str )
    { 
      GFL_BMP_Delete( p_wk->p_bmp[i] );
    }
  }
  GFL_BMPWIN_ClearScreen( p_wk->p_bmpwin );
  GFL_BMPWIN_Delete( p_wk->p_bmpwin );

  if( p_wk->param.frm < GFL_BG_FRAME0_S )
  { 
    BR_RES_UnLoadOBJ( p_wk->param.p_res, BR_RES_OBJ_ALLOW_M );
  }
  else
  { 
    BR_RES_UnLoadOBJ( p_wk->param.p_res, BR_RES_OBJ_ALLOW_S );
  }


  if( p_wk->param.p_pos )
  { 
    p_wk->param.p_pos->list   = p_wk->list;
    p_wk->param.p_pos->cursor = p_wk->cursor;
  }

  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  メイン処理
 *
 *	@param	BR_LIST_WORK* p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_LIST_Main( BR_LIST_WORK* p_wk )
{ 
  //スクロール
  if( BR_LIST_IsMoveEnable( p_wk ) )
  { 
    //スライダー操作
    p_wk->value = Br_List_Slide( p_wk );
    if( p_wk->value != 0 )
    {   
      //矢印モードならば、矢印の場所にって変動
      //タッチモードならばリストが直接動く
      if( p_wk->move_callback( p_wk, p_wk->value ) )
      { 
        if( !PMSND_CheckPlayingSEIdx(BR_SND_SE_SLIDE) )
        {
          PMSND_PlaySE( BR_SND_SE_SLIDE );
        }
        Br_List_Write( p_wk, p_wk->list );
      }

      //一番上まできたら、↑の矢印を
      //一番下まできたら、↓の矢印を消す
      if( p_wk->is_allow_visible )
      { 
        if( 0 == p_wk->list )
        { 
          GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[BR_LIST_CLWK_ALLOW_U], FALSE );
        }
        else
        { 
          GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[BR_LIST_CLWK_ALLOW_U], TRUE );
        }

        if( p_wk->list + p_wk->line_max == p_wk->param.list_max )
        { 
          GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[BR_LIST_CLWK_ALLOW_D], FALSE );
        }
        else
        { 
          GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[BR_LIST_CLWK_ALLOW_D], TRUE );
        }
      }
    }
  }

  //リスト選択
  {
    p_wk->select_callback( p_wk );
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  リストが動けるかどうか
 *
 *	@param	const BR_LIST_WORK* cp_wk   ワーク
 *
 *	@return TRUEならばリスト動作可能  FALSEならば不可能
 */
//-----------------------------------------------------------------------------
BOOL BR_LIST_IsMoveEnable( const BR_LIST_WORK* cp_wk )
{ 
  return cp_wk->enable_callback( cp_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  選択を取得
 *
 *	@param	const BR_LIST_WORK* cp_wk リスト
 *
 *	@return BMP_MENULIST_DATAで設定したパラメータ 押していない場合はBR_LIST_SELECT_NONE
 */
//-----------------------------------------------------------------------------
u32 BR_LIST_GetSelect( const BR_LIST_WORK* cp_wk )
{ 
  return cp_wk->select_param;
}

//----------------------------------------------------------------------------
/**
 *	@brief  BMPを設定
 *
 *	@param	BR_LIST_WORK* p_wk  ワーク
 *	@param	idx                 インデックス
 *	@param	*p_src              p_src
 */
//-----------------------------------------------------------------------------
void BR_LIST_SetBmp( BR_LIST_WORK* p_wk, u16 idx, GFL_BMP_DATA *p_src )
{ 
  GF_ASSERT( idx < p_wk->param.list_max );
  GF_ASSERT( p_wk->param.cp_list[idx].str == NULL );
  p_wk->p_bmp[ idx ]  = p_src; 
}

//----------------------------------------------------------------------------
/**
 *	@brief  BMP取得
 *
 *	@param	BR_LIST_WORK* p_wk  ワーク
 *	@param	idx                 インデックス
 *
 *	@retval   BMP
 */
//-----------------------------------------------------------------------------
GFL_BMP_DATA *BR_LIST_GetBmp( const BR_LIST_WORK* cp_wk, u16 idx )
{ 
  GF_ASSERT( idx < cp_wk->param.list_max );
  return cp_wk->p_bmp[ idx ];
}

//----------------------------------------------------------------------------
/**
 *	@brief  データ取得
 *
 *	@param	BR_LIST_WORK* p_wk    ワーク
 *	@param	param                 取得インデックス
 *
 *	@return 取得したもの
 */
//-----------------------------------------------------------------------------
u32 BR_LIST_GetParam( const BR_LIST_WORK* cp_wk, BR_LIST_PARAM_IDX param )
{ 
  switch( param )
  { 
  case BR_LIST_PARAM_IDX_CURSOR_POS:
    return cp_wk->cursor;

  case BR_LIST_PARAM_IDX_LIST_POS:
    return cp_wk->list;

  case BR_LIST_PARAM_IDX_MOVE_TIMING:
    return cp_wk->value;

  case BR_LIST_PARAM_IDX_LIST_MAX:
    return cp_wk->param.list_max;

  default:
    GF_ASSERT(0);
    return 0;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  表示
 *
 *	@param	BR_LIST_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
extern void BR_LIST_Write( BR_LIST_WORK *p_wk )
{ 
  Br_List_Write( p_wk, p_wk->list );
}
//----------------------------------------------------------------------------
/**
 *	@brief  表示
 *
 *	@param	BR_LIST_WORK *p_wk ワーク
 *	@param	now                 現在のリストの位置
 */
//-----------------------------------------------------------------------------
static void Br_List_Write( BR_LIST_WORK *p_wk, u16 now )
{ 
  int i;


  NAGI_Printf( "list %d cursor %d line %d max %d \n", p_wk->list, p_wk->cursor, p_wk->line_max, p_wk->param.list_max );

  //初期表示をプリント
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), 0 );
  for( i = now; i < now + p_wk->line_max; i++ )
  { 
    //GFL_BMP_Printを使うと文字書き込みとスピードがほぼ変わりないので、
    //自前のキャラ単位コピー関数をつかい高速化を図る
    if( i < p_wk->param.list_max )
    { 
      if( p_wk->p_bmp[i] )
      { 
        BMP_Copy( p_wk->p_bmp[i], GFL_BMPWIN_GetBmp(p_wk->p_bmpwin),
          0, 0, 0, (i - now) * p_wk->param.str_line, p_wk->param.w, p_wk->param.str_line );
      }
    }
  }
  GFL_BMPWIN_TransVramCharacter( p_wk->p_bmpwin );


}

//----------------------------------------------------------------------------
/**
 *	@brief  スライド操作
 *
 *	@param	BR_LIST_WORK* p_wk  ワーク
 *
 *	@return 1ならば↓に１つ動く -ならば↑に１つ動く 0ならば動作なし
 */
//-----------------------------------------------------------------------------
static s8 Br_List_Slide( BR_LIST_WORK* p_wk )
{ 
  ///< スライダーの当たり判定
  static const GFL_UI_TP_HITTBL sc_slider_hit_tbl[] = {
    { 8, 128,   0 +  8,   0 + 32 },
    { 8, 128, 256 - 32, 256 -  8 },
    { GFL_UI_TP_HIT_END, GFL_UI_TP_HIT_END, GFL_UI_TP_HIT_END, GFL_UI_TP_HIT_END }
  };

  s8 value  = 0;

  if( GFL_UI_TP_HitCont( sc_slider_hit_tbl ) != GFL_UI_TP_HIT_NONE )
  { 
    u32   x, y;

    GFL_UI_TP_GetPointCont( &x, &y );

    if( !p_wk->is_hit )
    { 
      //スライダートリガー
      p_wk->is_hit  = TRUE;
      p_wk->trg_y   = y;
    }
    else
    { 
      //スライダー中
      s32 dif_y = y - p_wk->trg_y;
      if( MATH_IAbs( dif_y ) > BR_LIST_SLIDE_DISTANCE )
      { 
        if( dif_y < 0 )
        { 
          value = -1;
        }
        else
        { 
          value = 1;
        }
        p_wk->trg_y = y;
      }
    }
  }
  else
  { 
    //スライダー離し
    if( p_wk->is_hit )
    {
      p_wk->is_hit  = FALSE;
    }
  }

  return value;
}

//----------------------------------------------------------------------------
/**
 *	@brief	BMPのキャラ単位コピー処理（PRINTはドット単位でコピーするのでより早い）
 *
 *	@param	const GFL_BMP_DATA *cp_src	転送元
 *	@param	*p_dst	転送先
 *	@param	src_x		転送元X座標（以下すべてキャラ単位）
 *	@param	src_y		転送元Y座標
 *	@param	dst_x		転送先X座標	
 *	@param	dst_y		転送先Y座標
 *	@param	w				転送幅
 *	@param	h				転送高さ
 *
 */
//-----------------------------------------------------------------------------
static void BMP_Copy( const GFL_BMP_DATA *cp_src, GFL_BMP_DATA *p_dst, u16 src_x, u16 src_y, u16 dst_x, u16 dst_y, u16 w, u16 h )
{	
	const u8	*cp_src_chr		= (const u8 *)GFL_BMP_GetCharacterAdrs( (GFL_BMP_DATA *)(cp_src) );
	u8	*p_dst_chr					= (u8 *)GFL_BMP_GetCharacterAdrs( p_dst );
	const u8 *cp_src_chr_ofs;
	u8 *p_dst_chr_ofs;

	cp_src_chr_ofs	= cp_src_chr + ((w * src_y) + src_x)*GFL_BG_1CHRDATASIZ;
	p_dst_chr_ofs		= p_dst_chr	+ ((w * dst_y) + dst_x)*GFL_BG_1CHRDATASIZ;

	GFL_STD_MemCopy32( cp_src_chr_ofs, p_dst_chr_ofs, w * h * GFL_BG_1CHRDATASIZ );
}

//----------------------------------------------------------------------------
/**
 *	@brief  タッチ式リストのときの移動
 *
 *	@param	BR_LIST_WORK *p_wk   ワーク
 *	@param	value                 1ならば↑へ-1ならば↓へ
 *
 *	@return TRUEで変更  FALSEで変更していない
 */
//-----------------------------------------------------------------------------
static BOOL Br_TouchList_MoveCallback( BR_LIST_WORK *p_wk, s8 value )
{ 
  if( 0 < p_wk->list && value == -1 )
  { 
    p_wk->list  += value;
    return TRUE;
  }

  if( p_wk->list + p_wk->line_max < p_wk->param.list_max && value == 1 )
  { 
    p_wk->list  += value;
    return TRUE;
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カーソル式リストのときの移動
 *
 *	@param	BR_LIST_WORK *p_wk   ワーク
 *	@param	value                 1ならば↑へ-1ならば↓へ
 *
 *	@return TRUEで変更  FALSEで変更していない
 */
//-----------------------------------------------------------------------------
static BOOL Br_CursorList_MoveCallback( BR_LIST_WORK *p_wk, s8 value )
{ 
  BOOL ret  = FALSE;

  //リストの半分の位置
  const int check_p = (p_wk->line_max-1)/2 + (p_wk->line_max-1)%2;

  if( value > 0 )
  {
    if( check_p == p_wk->cursor && p_wk->list + p_wk->line_max < p_wk->param.list_max )
    {
      //リストが動くのはチェックポイントまできていて、
      //下部が見えるまで
      p_wk->list += value;
      ret = TRUE;
    }
    else if( p_wk->list + p_wk->cursor < p_wk->param.list_max-1 )
    {
      //カーソルが動けるのはリストの範囲内のみ
      p_wk->cursor += value;
      ret = TRUE;
    }
  }
  else if( value < 0 )
  {
    if( check_p == p_wk->cursor && 0 < p_wk->list  )
    {
      //リストが動くのはチェックポイントまできていて、
      //範囲内のとき
      p_wk->list += value;
      ret = TRUE;
    }
    else if( 0 < p_wk->cursor )
    {
      //カーソルが動けるのはリストの範囲内のみ
      p_wk->cursor += value;
      ret = TRUE;
    }
  }

  //動くときカーソルが移動する
  if( ret )
  { 
    OS_TFPrintf( 3, "cur=%d lis=%d max=%d line=%d v=%d\n", p_wk->cursor, p_wk->list, p_wk->param.list_max, p_wk->line_max,value );

    if( p_wk->param.p_balleff_main )
    { 
      BR_BALLEFF_StartMove( p_wk->param.p_balleff_main, BR_BALLEFF_MOVE_CIRCLE, &sc_cursor_pos[ p_wk->cursor ] );
    }
  }

  return ret;
}
//----------------------------------------------------------------------------
/**
 *	@brief  タッチリストが動作可能か
 *	        （タッチリストの動作可能かは、項目数がウィンドウ以上かどうか）
 *
 *	@param	const BR_LIST_WORK *cp_wk   ワーク
 *
 *	@return TRUEで動作  FALSEで動作しない
 */
//-----------------------------------------------------------------------------
static BOOL Br_TouchList_EnableCallback( const BR_LIST_WORK *cp_wk )
{ 
  return cp_wk->param.list_max >= cp_wk->line_max;
}
//----------------------------------------------------------------------------
/**
 *	@brief  カーソルリストが動作可能か
 *	        （カーソルリストの動作可能かは、１つより大きいか）
 *
 *	@param	const BR_LIST_WORK *cp_wk   ワーク
 *
 *	@return TRUEで動作  FALSEで動作しない
 */
//-----------------------------------------------------------------------------
static BOOL Br_CursorList_EnableCallback( const BR_LIST_WORK *cp_wk )
{ 
  return cp_wk->param.list_max > 1;
}
//----------------------------------------------------------------------------
/**
 *	@brief  タッチリストの選択関数
 *
 *	@param	BR_LIST_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Br_TouchList_SelectCallback( BR_LIST_WORK *p_wk )
{ 
  int ret;
  p_wk->select_param  = BR_LIST_SELECT_NONE;

  ret = GFL_UI_TP_HitTrg( p_wk->hittbl );
  if( ret != GFL_UI_TP_HIT_NONE )
  {
    if( p_wk->param.p_balleff_sub )
    { 
      u32 x, y;
      GFL_POINT pos;

      GFL_UI_TP_GetPointTrg( &x, &y );
      pos.x = x;
      pos.y = y;
      BR_BALLEFF_StartMove( p_wk->param.p_balleff_sub, BR_BALLEFF_MOVE_EMIT, &pos );
    }

    PMSND_PlaySE( BR_SND_SE_OK );
    p_wk->select_param  = p_wk->param.cp_list[ ret + p_wk->list ].param;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  カーソルリストの選択関数
 *
 *	@param	BR_LIST_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Br_CursorList_SelectCallback( BR_LIST_WORK *p_wk )
{ 
  GF_ASSERT_MSG( p_wk->cursor + p_wk->list < p_wk->param.list_max, "cur=%d lis=%d max=%d\n", p_wk->cursor, p_wk->list, p_wk->param.list_max );
  p_wk->select_param  = p_wk->param.cp_list[ p_wk->cursor + p_wk->list ].param;
}
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					  上画面テキスト
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	テキストワーク
//    実はMSGWINと同じ
//=====================================
//typedef BR_MSGWIN_WORK  ;

//-------------------------------------
///	パブリック
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  上画面テキスト表示
 *
 *	@param	PRINT_QUE *p_que  キュー
 *	@param	heapID            ヒープID 
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
BR_TEXT_WORK * BR_TEXT_Init( BR_RES_WORK *p_res, PRINT_QUE *p_que, HEAPID heapID )
{ 
  BR_TEXT_WORK *p_wk;

  //BG読み込み
  BR_RES_LoadBG( p_res, BR_RES_BG_TEXT_M, heapID );

  //ワーク作成
  p_wk  = BR_MSGWIN_Init( BG_FRAME_M_TEXT, 1, 19, 30, 4, PLT_BG_M_COMMON+2, p_que, heapID );
  p_wk->clear_chr = 0xC;
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  破棄
 *
 *	@param	BR_TEXT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_TEXT_Exit( BR_TEXT_WORK *p_wk, BR_RES_WORK *p_res )
{ 
  //ワーク破棄
  BR_MSGWIN_Exit( p_wk );

  //BG破棄
  BR_RES_UnLoadBG( p_res, BR_RES_BG_TEXT_M );
}
//----------------------------------------------------------------------------
/**
 *	@brief  文字表示
 *
 *	@param	BR_TEXT_WORK* p_wk  ワーク
 *	@param	*p_res              リソース
 *	@param	strID               文字ID
 */
//-----------------------------------------------------------------------------
void BR_TEXT_Print( BR_TEXT_WORK* p_wk, const BR_RES_WORK *cp_res, u32 strID )
{ 
  GFL_FONT    *p_font;
  GFL_MSGDATA *p_msg;

  p_font  = BR_RES_GetFont( cp_res );
  p_msg   = BR_RES_GetMsgData( cp_res );
  BR_MSGWIN_PrintColor( p_wk, p_msg, strID, p_font, BR_PRINT_COL_INFO );
}
//----------------------------------------------------------------------------
/**
 *	@brief  文字表示  バッファ
 *
 *	@param	BR_TEXT_WORK* p_wk  ワーク
 *	@param	*p_res              リソース
 *	@param	strbuf              文字バッファ
 */
//-----------------------------------------------------------------------------
void BR_TEXT_PrintBuff( BR_TEXT_WORK* p_wk, const BR_RES_WORK *cp_res, const STRBUF *cp_strbuf )
{ 
  GFL_FONT    *p_font;

  p_font  = BR_RES_GetFont( cp_res );
  BR_MSGWIN_PrintBufColor( p_wk, cp_strbuf, p_font, BR_PRINT_COL_INFO );
}
//----------------------------------------------------------------------------
/**
 *	@brief  表示処理
 *
 *	@param	BR_TEXT_WORK* p_wk  ワーク
 *
 *	@return TRUEで表示  FALSEで表示していない
 */
//-----------------------------------------------------------------------------
BOOL BR_TEXT_PrintMain( BR_TEXT_WORK* p_wk )
{ 
  return BR_MSGWIN_PrintMain( p_wk );
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					  プロフィール画面作成
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	プロフィールMSGWIN
//=====================================
typedef enum
{
  BR_PROFILE_MSGWINID_M_NAME = 0,   //●●●のプロフィール
  BR_PROFILE_MSGWINID_M_BIRTH,      //●がつうまれ
  BR_PROFILE_MSGWINID_M_PLACE,      //すんでいるばしょ
  BR_PROFILE_MSGWINID_M_COUNTRY,    //住んでいる場所国名
  BR_PROFILE_MSGWINID_M_AREA,       //住んでいる地域
  BR_PROFILE_MSGWINID_M_INTRO,      //じこしょうかい
  BR_PROFILE_MSGWINID_M_WORD,       //自己紹介文

  BR_PROFILE_MSGWINID_M_MAX,

} BR_PROFILE_MSGWINID_M;
//-------------------------------------
///	プロフィールワーク
//=====================================
struct _BR_PROFILE_WORK
{ 
  BR_MSGWIN_WORK    *p_msgwin[ BR_PROFILE_MSGWINID_M_MAX ];
  BR_RES_WORK       *p_res;
  PMS_DRAW_WORK     *p_pms;


  GFL_CLWK          *p_pokeicon;
  GFL_CLWK          *p_selficon;
  u32               res_icon_plt;
  u32               res_icon_chr;
  u32               res_icon_cel;
  u32               res_self_plt;
  u32               res_self_chr;
  u32               res_self_cel;
};

//-------------------------------------
///	パブリック
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  プロフィール上画面を作成
 *
 *	@param	GDS_PROFILE_PTR cp_profile  表示用データ
 *	@param	*p_res                      リソース
 *	@param	*p_unit                     ユニット
 *	@param	*p_que                      キュー
 *	@param  type                        種類
 *	@param	heapID                      ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
BR_PROFILE_WORK * BR_PROFILE_CreateMainDisplay( const GDS_PROFILE_PTR cp_profile, BR_RES_WORK *p_res, GFL_CLUNIT *p_unit, PRINT_QUE *p_que, BR_PROFILE_TYPE type, HEAPID heapID )
{ 
  static const struct 
  { 
    u8 x;
    u8 y;
    u8 w;
    u8 h;
    u32 msgID;
  } sc_msgwin_data[BR_PROFILE_MSGWINID_M_MAX]  =
  { 
    {
      4,
      4,
      24,
      2,
      msg_13,
    },
    {
      17,
      8,
      11,
      4,
      msg_15,
    },
    {
      4,
      11,
      12,
      2,
      msg_16,
    },
    {
      4,
      13,
      24,
      2,
      msg_22,
    },
    {
      4,
      15,
      24,
      2,
      msg_23,
    },
    {
      4,
      17,
      11,
      2,
      msg_17,
    },
    { 
      4,
      19,
      24,
      4,
      0,
    }
  };


  GFL_FONT *p_font;
  BR_PROFILE_WORK * p_wk;
  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(BR_PROFILE_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(BR_PROFILE_WORK) );
  p_wk->p_res = p_res;


  p_font  = BR_RES_GetFont( p_res );

  { 
    p_wk->p_pms = PMS_DRAW_Init( p_unit, CLSYS_DRAW_MAIN, p_que, p_font, PLT_OBJ_M_PMS, 1, heapID );
    PMS_DRAW_SetPmsObjMode( p_wk->p_pms, 0, GX_OAM_MODE_XLU );
  }

	BR_RES_LoadBG( p_wk->p_res, BR_RES_BG_RECORD_M_PROFILE, heapID );

 //メッセージWIN作成
  {
    int i;
    GFL_MSGDATA *p_msg;
    WORDSET *p_word;
    STRBUF  *p_strbuf;
    STRBUF  *p_src;

    BOOL is_msg   = TRUE;

    p_msg   = BR_RES_GetMsgData( p_res );
    p_word  = BR_RES_GetWordSet( p_res );

    for( i = 0; i < BR_PROFILE_MSGWINID_M_MAX; i++ )
    { 
      p_wk->p_msgwin[i]  = BR_MSGWIN_Init( BG_FRAME_M_FONT, sc_msgwin_data[i].x, sc_msgwin_data[i].y, sc_msgwin_data[i].w, sc_msgwin_data[i].h, PLT_BG_M_FONT, p_que, heapID );

      BR_MSGWIN_SetPos( p_wk->p_msgwin[i], 0, 0, BR_MSGWIN_POS_ABSOLUTE );

      is_msg   = TRUE;
      switch(i)
      { 
      case BR_PROFILE_MSGWINID_M_NAME:   //●●●のプロフィール
        { 
          STRBUF  *p_name;
          p_strbuf  = GFL_STR_CreateBuffer( 128, heapID );
          p_name    = GDS_Profile_CreateNameString( cp_profile, heapID );
          p_src     = GFL_MSG_CreateString( p_msg, sc_msgwin_data[i].msgID );
          WORDSET_RegisterWord( p_word, 0, p_name, GDS_Profile_GetSex(cp_profile), TRUE, PM_LANG );
          WORDSET_ExpandStr( p_word, p_strbuf, p_src );
          GFL_STR_DeleteBuffer( p_name );
          GFL_STR_DeleteBuffer( p_src );
        }
        break;
      case BR_PROFILE_MSGWINID_M_BIRTH:      //●がつうまれ
        { 
          const u32 month = GDS_Profile_GetMonthBirthday( cp_profile );
          p_strbuf  = GFL_STR_CreateBuffer( 128, heapID );
          p_src     = GFL_MSG_CreateString( p_msg, sc_msgwin_data[i].msgID );
          WORDSET_RegisterNumber( p_word, 0, month, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
          WORDSET_ExpandStr( p_word, p_strbuf, p_src );
          GFL_STR_DeleteBuffer( p_src );
        }
        break;
      case BR_PROFILE_MSGWINID_M_COUNTRY:    //住んでいる場所国名
        { 
          const u32 nation = GDS_Profile_GetNation( cp_profile );
          p_strbuf  = GFL_STR_CreateBuffer( 128, heapID );
          p_src     = GFL_MSG_CreateString( p_msg, sc_msgwin_data[i].msgID );
          WORDSET_RegisterCountryName( p_word, 0, nation );
          WORDSET_ExpandStr( p_word, p_strbuf, p_src );
          GFL_STR_DeleteBuffer( p_src );
        }
        break;
      case BR_PROFILE_MSGWINID_M_AREA:       //住んでいる地域
        { 
          const u32 nation  = GDS_Profile_GetNation( cp_profile );
          const u32 area    = GDS_Profile_GetArea( cp_profile );
          p_strbuf  = GFL_STR_CreateBuffer( 128, heapID );
          p_src     = GFL_MSG_CreateString( p_msg, sc_msgwin_data[i].msgID );
          WORDSET_RegisterLocalPlaceName( p_word, 0, nation, area );
          WORDSET_ExpandStr( p_word, p_strbuf, p_src );

          GFL_STR_DeleteBuffer( p_src );
        }
        break;

      case BR_PROFILE_MSGWINID_M_WORD:    //自己紹介文
        { 
          STRBUF  *p_self;
          PMS_DATA pms;
          p_self    = GDS_Profile_GetSelfIntroduction( cp_profile, &pms, heapID );
          if( p_self )
          { 
            p_strbuf  = GFL_STR_CreateCopyBuffer( p_self, heapID );
            GFL_STR_DeleteBuffer( p_self );
          }
          else
          { 
            PMS_DRAW_SetNullColorPallet( p_wk->p_pms, 0 );
            PMS_DRAW_SetPrintColor( p_wk->p_pms, BR_PRINT_COL_NORMAL );
            PMS_DRAW_Print( p_wk->p_pms, p_wk->p_msgwin[i]->p_bmpwin, &pms, 0 ); 
            is_msg  = FALSE;
          }
        }
        break;

      default:
        p_strbuf  = GFL_MSG_CreateString( p_msg, sc_msgwin_data[i].msgID );
      }

      if( is_msg )
      { 
        BR_MSGWIN_PrintBufColor( p_wk->p_msgwin[i], p_strbuf, p_font, BR_PRINT_COL_NORMAL );

        GFL_STR_DeleteBuffer( p_strbuf );
      }
    }
  }


  //ポケモン
  { 
    const int egg   = GDS_Profile_GetEggFlag(cp_profile);
    const int mons   = GDS_Profile_GetMonsNo(cp_profile);
    const int form   = GDS_Profile_GetFormNo(cp_profile);
    const int sex   = GDS_Profile_GetPokeSex(cp_profile);

    if( mons != 0 )
    { 
      { 
        ARCHANDLE *p_handle;
        p_handle  = GFL_ARC_OpenDataHandle( ARCID_POKEICON, GFL_HEAP_LOWID(heapID) );
        p_wk->res_icon_plt  = GFL_CLGRP_PLTT_RegisterComp( p_handle, 
            POKEICON_GetPalArcIndex(),
            CLSYS_DRAW_MAIN, PLT_OBJ_M_POKEICON*0x20, GFL_HEAP_LOWID(heapID) );
        p_wk->res_icon_cel = GFL_CLGRP_CELLANIM_Register( p_handle,
            POKEICON_GetCellArcIndex(), POKEICON_GetAnmArcIndex(), GFL_HEAP_LOWID(heapID) );
        p_wk->res_icon_chr  = GFL_CLGRP_CGR_Register( p_handle,
            POKEICON_GetCgxArcIndexByMonsNumber( mons, form, sex, egg ),
            FALSE, CLSYS_DRAW_MAIN, GFL_HEAP_LOWID(heapID) );
        GFL_ARC_CloseDataHandle( p_handle );
      }
      { 
        GFL_CLWK_DATA data;
        GFL_STD_MemClear( &data, sizeof(GFL_CLWK_DATA) );
        data.pos_x  = 75;
        data.pos_y  = 64;
        data.anmseq = POKEICON_ANM_HPMAX;
        p_wk->p_pokeicon  = GFL_CLACT_WK_Create( p_unit,
            p_wk->res_icon_chr,p_wk->res_icon_plt,p_wk->res_icon_cel,
            &data, CLSYS_DEFREND_MAIN, heapID );
        GFL_CLACT_WK_SetPlttOffs( p_wk->p_pokeicon, POKEICON_GetPalNum( mons, form, sex, egg ), CLWK_PLTTOFFS_MODE_OAM_COLOR );
        GFL_CLACT_WK_SetObjMode( p_wk->p_pokeicon, GX_OAM_MODE_XLU );
      }
    }
  }

  //自分のみため
  { 
    switch( type )
    {
    case BR_PROFILE_TYPE_MY:   //自分
      { 
        ARCHANDLE *p_handle;
        u32 plt, cel, chr, anm;

        if( GDS_Profile_GetSex(cp_profile) == PM_MALE )
        {
          plt = NARC_wifileadingchar_hero_NCLR;
          chr = NARC_wifileadingchar_hero_simple_NCGR;
          cel = NARC_wifileadingchar_hero_simple_NCER;
          anm = NARC_wifileadingchar_hero_simple_NANR;
        }
        else
        {
          plt = NARC_wifileadingchar_heroine_NCLR;
          chr = NARC_wifileadingchar_heroine_simple_NCGR;
          cel = NARC_wifileadingchar_heroine_simple_NCER;
          anm = NARC_wifileadingchar_heroine_simple_NANR;
        }

        p_handle  = GFL_ARC_OpenDataHandle( ARCID_WIFILEADING, GFL_HEAP_LOWID(heapID) );
        p_wk->res_self_plt  = GFL_CLGRP_PLTT_RegisterEx( p_handle, 
            plt,
            CLSYS_DRAW_MAIN, PLT_OBJ_M_TRAINER*0x20, 
            0, 1, heapID );
        p_wk->res_self_cel = GFL_CLGRP_CELLANIM_Register( p_handle,
            cel, 
            anm, heapID );
        p_wk->res_self_chr  = GFL_CLGRP_CGR_Register( p_handle,
            chr,
            FALSE, CLSYS_DRAW_MAIN, heapID );
        GFL_ARC_CloseDataHandle( p_handle );
      }
      break;

    case BR_PROFILE_TYPE_OTHER:   //他人
      { 
        const int self  = GDS_Profile_GetTrainerView(cp_profile);
        ARCHANDLE *p_handle;
        p_handle  = GFL_ARC_OpenDataHandle( ARCID_WIFIUNIONCHAR, GFL_HEAP_LOWID(heapID) );
        p_wk->res_self_plt  = GFL_CLGRP_PLTT_RegisterEx( p_handle, 
            NARC_wifi_unionobj_wifi_union_obj_NCLR,
            CLSYS_DRAW_MAIN, PLT_OBJ_M_TRAINER*0x20, 
            sc_wifi_unionobj_plt[self], 1, heapID );
        p_wk->res_self_cel = GFL_CLGRP_CELLANIM_Register( p_handle,
            NARC_wifi_unionobj_front00_NCER, 
            NARC_wifi_unionobj_front00_NANR, heapID );
        p_wk->res_self_chr  = GFL_CLGRP_CGR_Register( p_handle,
            NARC_wifi_unionobj_front00_NCGR + self,
            FALSE, CLSYS_DRAW_MAIN, heapID );
        GFL_ARC_CloseDataHandle( p_handle );
      }
      break;
    }

    { 
      GFL_CLWK_DATA data;
      GFL_STD_MemClear( &data, sizeof(GFL_CLWK_DATA) );
      data.pos_x  = 48;
      data.pos_y  = 64;
      p_wk->p_selficon  = GFL_CLACT_WK_Create( p_unit,
          p_wk->res_self_chr,p_wk->res_self_plt,p_wk->res_self_cel,
            &data, CLSYS_DEFREND_MAIN, heapID );
      GFL_CLACT_WK_SetPlttOffs( p_wk->p_selficon, 0, CLWK_PLTTOFFS_MODE_PLTT_TOP );
      GFL_CLACT_WK_SetObjMode( p_wk->p_selficon, GX_OAM_MODE_XLU );
    }
  }


  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  プロフィール上画面を破棄
 *
 *	@param	BR_PROFILE_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_PROFILE_DeleteMainDisplay( BR_PROFILE_WORK *p_wk )
{ 
  //OBJ開放
  { 
    if( p_wk->p_selficon )
    { 
      GFL_CLACT_WK_Remove( p_wk->p_selficon );
      GFL_CLGRP_PLTT_Release(p_wk->res_self_plt);
      GFL_CLGRP_CGR_Release(p_wk->res_self_chr);
      GFL_CLGRP_CELLANIM_Release(p_wk->res_self_cel);
    }

    if( p_wk->p_pokeicon )
    { 
      GFL_CLACT_WK_Remove( p_wk->p_pokeicon );
      GFL_CLGRP_PLTT_Release(p_wk->res_icon_plt);
      GFL_CLGRP_CGR_Release(p_wk->res_icon_chr);
      GFL_CLGRP_CELLANIM_Release(p_wk->res_icon_cel);
    }
  }

  //BG開放
  { 
    int i;
    for( i = 0; i < BR_PROFILE_MSGWINID_M_MAX; i++ )
    { 
      if( p_wk->p_msgwin[i] )
      { 
        BR_MSGWIN_Exit( p_wk->p_msgwin[i] );
        p_wk->p_msgwin[i] = NULL;
      }
    }
  }

  PMS_DRAW_Exit( p_wk->p_pms );

  BR_RES_UnLoadBG( p_wk->p_res, BR_RES_BG_RECORD_M_PROFILE );

  GFL_HEAP_FreeMemory( p_wk );

  GFL_BG_LoadScreenReq( BG_FRAME_M_FONT );
}
//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	BR_PROFILE_WORK *p_wk 
 *
 *	@return TRUEで表示  FALSEでまだ
 */
//-----------------------------------------------------------------------------
BOOL BR_PROFILE_PrintMain( BR_PROFILE_WORK *p_wk )
{ 
  BOOL  ret = TRUE;
  int i;

  for( i = 0; i < BR_PROFILE_MSGWINID_M_MAX; i++ )
  { 
    if( p_wk->p_msgwin[i] )
    { 
      ret &= BR_MSGWIN_PrintMain( p_wk->p_msgwin[i] );
    }
  }

  PMS_DRAW_Main( p_wk->p_pms );

  return ret;
}

//=============================================================================
/**
 *  シーケンスシステム
 */
//=============================================================================
//-------------------------------------
///	シーケンスワーク
//=====================================
struct _BR_SEQ_WORK
{
	BR_SEQ_FUNCTION	seq_function;		//実行中のシーケンス関数
	int seq;											//実行中のシーケンス関数の中のシーケンス
	void *p_wk_adrs;							//実行中のシーケンス関数に渡すワーク
  int reserv_seq;               //予約シーケンス
};

//-------------------------------------
///	パブリック
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	初期化
 *
 *	@param	BR_SEQ_WORK *p_wk	ワーク
 *	@param	*p_param				パラメータ
 *	@param	seq_function		シーケンス
 *
 */
//-----------------------------------------------------------------------------
BR_SEQ_WORK * BR_SEQ_Init( void *p_wk_adrs, BR_SEQ_FUNCTION seq_function, HEAPID heapID )
{	
  BR_SEQ_WORK *p_wk;

	//作成
  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(BR_SEQ_WORK) );
	GFL_STD_MemClear( p_wk, sizeof(BR_SEQ_WORK) );

	//初期化
	p_wk->p_wk_adrs	= p_wk_adrs;

	//セット
	BR_SEQ_SetNext( p_wk, seq_function );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	破棄
 *
 *	@param	BR_SEQ_WORK *p_wk	ワーク
 */
//-----------------------------------------------------------------------------
void BR_SEQ_Exit( BR_SEQ_WORK *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	メイン処理
 *
 *	@param	BR_SEQ_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
void BR_SEQ_Main( BR_SEQ_WORK *p_wk )
{	
	if( p_wk->seq_function )
	{	
		p_wk->seq_function( p_wk, &p_wk->seq, p_wk->p_wk_adrs );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	終了取得
 *
 *	@param	const BR_SEQ_WORK *cp_wk		ワーク
 *
 *	@return	TRUEならば終了	FALSEならば処理中
 */	
//-----------------------------------------------------------------------------
BOOL BR_SEQ_IsEnd( const BR_SEQ_WORK *cp_wk )
{	
	return cp_wk->seq_function == NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	次のシーケンスを設定
 *
 *	@param	BR_SEQ_WORK *p_wk	ワーク
 *	@param	seq_function		シーケンス
 *
 */
//-----------------------------------------------------------------------------
void BR_SEQ_SetNext( BR_SEQ_WORK *p_wk, BR_SEQ_FUNCTION seq_function )
{	
	p_wk->seq_function	= seq_function;
	p_wk->seq	= 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	終了
 *
 *	@param	BR_SEQ_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
void BR_SEQ_End( BR_SEQ_WORK *p_wk )
{	
  BR_SEQ_SetNext( p_wk, NULL );
}
//----------------------------------------------------------------------------
/**
 *	@brief  SEQ 次のシーケンスを予約
 *
 *	@param	BR_SEQ_WORK *p_wk  ワーク
 *	@param	seq             次のシーケンス
 */
//-----------------------------------------------------------------------------
void BR_SEQ_SetReservSeq( BR_SEQ_WORK *p_wk, int seq )
{ 
  p_wk->reserv_seq  = seq;
}
//----------------------------------------------------------------------------
/**
 *	@brief  SEQ 予約されたシーケンスへ飛ぶ
 *
 *	@param	BR_SEQ_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_SEQ_NextReservSeq( BR_SEQ_WORK *p_wk )
{ 
  p_wk->seq = p_wk->reserv_seq;
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のシーケンスと同じかどうか
 *
 *	@param	const BR_SEQ_WORK *cp_wk  ワーク
 *	@param	seq_function              シーケンス
 *
 *	@return TRUE同じ FALSE異なる
 */
//-----------------------------------------------------------------------------
BOOL BR_SEQ_IsComp( const BR_SEQ_WORK *cp_wk, BR_SEQ_FUNCTION seq_function )
{ 
  return cp_wk->seq_function  == seq_function;
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					  小さいボール演出
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	定義
//=====================================
#define BR_BALLEFF_CLWK_MAX (12)

//-------------------------------------
///	動作用ワーク
//=====================================
typedef struct 
{
  GFL_POINT center_pos;
  GFL_POINT now_pos;
  u16 r;
  u16 max_r;
  u16 init_angle;
  u16 end_angle;
  int cnt_max;
} MOVE_EMIT;

typedef struct 
{
  GFL_POINT center_pos;
  GFL_POINT now_pos;
  u16 r;
  u16 init_angle;
  int cnt_max;
} MOVE_CIRCLE;

typedef struct 
{
  GFL_POINT start_pos;
  GFL_POINT end_pos;
  GFL_POINT now_pos;

  int wait_cnt;
  int cnt_max;
} MOVE_LINE;

typedef struct 
{
  const GFL_POINT *cp_homing_pos;
  VecFx32 now_pos;
  fx32 speed;
} MOVE_HOMING;

typedef struct
{ 
  GFL_POINT center_pos;
  GFL_POINT now_pos;
  u16 r_x;
  u16 r_y;
  u16 init_angle;
  u16 ellipse_angle;
  int cnt_max;
} MOVE_ELLIPSE;

//-------------------------------------
///	カーソルワーク
//=====================================
struct _BR_BALLEFF_WORK
{ 
  GFL_POINT   init_pos;
  const GFL_POINT   *cp_folow_pos;
  CLSYS_DRAW_TYPE draw;
  BR_SEQ_WORK *p_seq;
  BR_RES_WORK *p_res;

  union
  { 
    MOVE_LINE     line;
    MOVE_CIRCLE   circle;
    MOVE_HOMING   homing;
    MOVE_EMIT     emit;
    MOVE_ELLIPSE  ellipse;
  }move[BR_BALLEFF_CLWK_MAX];
  GFL_POINT   now_pos[ BR_BALLEFF_CLWK_MAX ];
  GFL_CLWK    *p_clwk[ BR_BALLEFF_CLWK_MAX ];
  int         cnt;
  BOOL        is_end;
  BOOL        is_homing;

  BOOL        is_end_one[ BR_BALLEFF_CLWK_MAX ];
};

//-------------------------------------
///	プロトタイプ
//=====================================
//シーケンス
static void Br_BallEff_Seq_Nop( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_BallEff_Seq_Emit( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_BallEff_Seq_Line( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_BallEff_Seq_Opening( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_BallEff_Seq_BigCircle( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_BallEff_Seq_Circle( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_BallEff_Seq_CircleCont( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_BallEff_Seq_EmitFollow( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_BallEff_Seq_OpeningTouch( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
//動き
static void MOVE_EMIT_Init( MOVE_EMIT *p_wk, const GFL_POINT *cp_center_pos, u16 r, u16 max_r, u16 init_angle, u16 end_angle, int cnt_max );
static BOOL MOVE_EMIT_Main( MOVE_EMIT *p_wk, GFL_POINT *p_now_pos, int cnt );
static void MOVE_CIRCLE_Init( MOVE_CIRCLE *p_wk, const GFL_POINT *cp_center_pos, u16 r, u16 init_angle, int cnt_max );
static BOOL MOVE_CIRCLE_Main( MOVE_CIRCLE *p_wk, GFL_POINT *p_now_pos, int cnt );
static void MOVE_LINE_Init( MOVE_LINE *p_wk, const GFL_POINT *cp_start_pos, const GFL_POINT *cp_end_pos, int wait_cnt, int cnt_max );
static BOOL MOVE_LINE_Main( MOVE_LINE *p_wk, GFL_POINT *p_now_pos, int cnt );
//ホーミング
static void MOVE_HOMING_Init( MOVE_HOMING *p_wk, const GFL_POINT *cp_homing_pos,const GFL_POINT *cp_init_pos, fx32 speed );
static BOOL MOVE_HOMING_Main( MOVE_HOMING *p_wk, GFL_POINT *p_now_pos );
//楕円
static void MOVE_ELLIPSE_Init( MOVE_ELLIPSE *p_wk, const GFL_POINT *cp_center_pos, u16 r_x, u16 r_y, u16 init_angle, u16 ellipse_angle, int cnt_max );
static BOOL MOVE_ELLIPSE_Main( MOVE_ELLIPSE *p_wk, GFL_POINT *p_now_pos, int cnt );

//-------------------------------------
///	データ
//=====================================
//直線動作のデータ
static const GFL_POINT sc_line_start_pos  =
{ 
  256/2,
  16
};
static const GFL_POINT sc_line_end_pos  =
{ 
  256/2,
  192+24
};
#define BR_BALLEFF_LINE_SYNC  (25*2)

//オープニングのデータ
static const GFL_POINT sc_opening_start_pos  =
{ 
  256/2,
  -16
};
static const GFL_POINT sc_opening_end_pos  =
{ 
  256/2,
  192/2
};
#define BR_BALLEFF_OPENING_SYNC  (18*2)

//追尾データ
#define BR_BALLEFF_HOMING_SPEED_FIX  (FX32_CONST( 8.0f ) / 2)
#define BR_BALLEFF_HOMING_SPEED(x)  (BR_BALLEFF_HOMING_SPEED_FIX  - (x * FX32_CONST( 0.4f ) / 2))
#define BR_BALLEFF_HOMING_SPEED_OP(x)  (FX32_CONST( 7.0f ) / 2  - (x * FX32_CONST( 0.7f ) / 2))
#define BR_BALLEFF_HOMING_SPEED_MENU(x)  (FX32_CONST( 8.0f ) / 2  - (x * FX32_CONST( 0.6f ) / 2))

//円をかく
#define BR_BALLEFF_CIRCLE_R (15)
#define BR_BALLEFF_CIRCLE_SYNC (90)
#define BR_BALLEFF_CIRCLE_USE_MAX (6)

//放射状に動く
#define BR_BALLEFF_EMIT_SYNC (15)
#define BR_BALLEFF_EMIT_R_MIN (8)
#define BR_BALLEFF_EMIT_R_MAX (20)
#define BR_BALLEFF_EMIT_ANGLE_MIN(x,max) (0xFFFF*x/max)
#define BR_BALLEFF_EMIT_ANGLE_MAX(x,max) (BR_BALLEFF_EMIT_ANGLE_MIN(x,max)+0xFFFF/(max-1))
#define BR_BALLEFF_EMIT_FOLLOW_SYNC (16)

//-------------------------------------
///	外部公開
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  ボールエフェクト作成
 *
 *  @param  GFL_CLUNIT          OAM登録ユニット
 *	@param	BR_RES_WORK *p_res  リソース
 *	@param	draw    描画先
 *	@param	heapID  ヒープID 
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
BR_BALLEFF_WORK *BR_BALLEFF_Init( GFL_CLUNIT *p_unit, BR_RES_WORK *p_res, CLSYS_DRAW_TYPE draw, HEAPID heapID )
{ 
  BR_BALLEFF_WORK *p_wk = GFL_HEAP_AllocMemory( heapID, sizeof(BR_BALLEFF_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(BR_BALLEFF_WORK) );
  p_wk->p_res = p_res;
  p_wk->draw  = draw;

  //シーケンスシステム作成
  p_wk->p_seq = BR_SEQ_Init( p_wk, Br_BallEff_Seq_Nop, heapID );

  //リソース読み込み
  BR_RES_LoadOBJ( p_wk->p_res, BR_RES_OBJ_BALL_M + draw, heapID );

  //CLWK作成
  { 
    int i;
    BOOL ret;
    GFL_CLWK_DATA cldata;
    BR_RES_OBJ_DATA resdata;
    GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

    ret = BR_RES_GetOBJRes( p_wk->p_res, BR_RES_OBJ_BALL_M + draw, &resdata );
    GF_ASSERT( ret );
    for( i = 0; i < BR_BALLEFF_CLWK_MAX; i++ )
    { 
      p_wk->p_clwk[i] = GFL_CLACT_WK_Create( 
          p_unit, resdata.ncg, resdata.ncl, resdata.nce, &cldata, draw, heapID
          );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], FALSE );
      GFL_CLACT_WK_SetObjMode( p_wk->p_clwk[i], GX_OAM_MODE_XLU );
    }
  }

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ボールエフェクト破棄
 *
 *	@param	BR_BALLEFF_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_BALLEFF_Exit( BR_BALLEFF_WORK *p_wk )
{ 
  //CLWK破棄
  { 
    int i;
    for( i = 0; i < BR_BALLEFF_CLWK_MAX; i++ )
    { 
      GFL_CLACT_WK_Remove( p_wk->p_clwk[i] );
    }
  }

  //リソース破棄
  BR_RES_UnLoadOBJ( p_wk->p_res, BR_RES_OBJ_BALL_M + p_wk->draw );

  //シーケンス破棄
  BR_SEQ_Exit( p_wk->p_seq );

  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ボールエフェクトメイン処理
 *
 *	@param	BR_BALLEFF_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_BALLEFF_Main( BR_BALLEFF_WORK *p_wk )
{ 
  BR_SEQ_Main( p_wk->p_seq );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ボールエフェクト  エフェクト動作開始
 *
 *	@param	BR_BALLEFF_WORK *p_wk ワーク
 *	@param	move                  動作の種類（列挙参照）
 *	@param	GFL_POINT *cp_pos     動作開始位置 or ついて行く座標のポインタ
 */
//-----------------------------------------------------------------------------
void BR_BALLEFF_StartMove( BR_BALLEFF_WORK *p_wk, BR_BALLEFF_MOVE move, const GFL_POINT *cp_pos )
{ 
  if( cp_pos )
  { 
    p_wk->init_pos  = *cp_pos;
    p_wk->cp_folow_pos  = cp_pos;
  }
  p_wk->is_end      = FALSE;

  if( move == BR_BALLEFF_MOVE_NOP && BR_SEQ_IsComp( p_wk->p_seq, Br_BallEff_Seq_BigCircle ) )
  { 
    PMSND_PlaySE( BR_SND_SE_SEARCH_OK );
  }

  OS_TPrintf( "X%d Y%d\n", p_wk->init_pos.x, p_wk->init_pos.y );

  switch( move )
  { 
  case BR_BALLEFF_MOVE_NOP:           //動かない(表示されない)
    BR_SEQ_SetNext( p_wk->p_seq, Br_BallEff_Seq_Nop );
    break;
  case BR_BALLEFF_MOVE_EMIT:          //放射に動く          STOP
    BR_SEQ_SetNext( p_wk->p_seq, Br_BallEff_Seq_Emit );
    break;
  case BR_BALLEFF_MOVE_LINE:          //線の動き            STOP
    BR_SEQ_SetNext( p_wk->p_seq, Br_BallEff_Seq_Line );
    break;
  case BR_BALLEFF_MOVE_OPENING:       //開始の動き          STOP
    BR_SEQ_SetNext( p_wk->p_seq, Br_BallEff_Seq_Opening );
    break;
  case BR_BALLEFF_MOVE_BIG_CIRCLE:    //大きい円を描く      LOOP
    BR_SEQ_SetNext( p_wk->p_seq, Br_BallEff_Seq_BigCircle );
    break;
  case BR_BALLEFF_MOVE_CIRCLE:        //円を描く            LOOP
    BR_SEQ_SetNext( p_wk->p_seq, Br_BallEff_Seq_Circle );
    break;
  case BR_BALLEFF_MOVE_CIRCLE_CONT:   //場所を変えて円を描くLOOP
    BR_SEQ_SetNext( p_wk->p_seq, Br_BallEff_Seq_CircleCont );
    break;
  case BR_BALLEFF_MOVE_EMIT_FOLLOW:   //放射に動き、ついて行く STOP
    BR_SEQ_SetNext( p_wk->p_seq, Br_BallEff_Seq_EmitFollow );
    break;
  case BR_BALLEFF_MOVE_OPENING_TOUCH: //開始時のタッチ     　STOP
    BR_SEQ_SetNext( p_wk->p_seq, Br_BallEff_Seq_OpeningTouch );
    break;

  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  ボールエフェクト  エフェクト動作終了確認  （動作によってはLOOPするので注意）
 *
 *	@param	const BR_BALLEFF_WORK *cp_wk ワーク
 *
 *	@return TRUE動作終了  FALSE動作中
 */
//-----------------------------------------------------------------------------
BOOL BR_BALLEFF_IsMoveEnd( const BR_BALLEFF_WORK *cp_wk )
{ 
  return cp_wk->is_end;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ボールエフェクト  アニメ変更
 *
 *	@param	BR_BALLEFF_WORK *p_wk ワーク
 *	@param	seq                   アニメシーケンス
 */
//-----------------------------------------------------------------------------
void BR_BALLEFF_SetAnmSeq( BR_BALLEFF_WORK *p_wk, int seq )
{ 
  int i;
  for( i = 0; i < BR_BALLEFF_CLWK_MAX; i++ )
  { 
    GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[i], seq );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  何もしない
 *
 *	@param	BR_SEQ_WORK *p_seqwk  シーケンス
 *	@param	*p_seq                シーケンス変数
 *	@param	*p_wk_adrs            ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Br_BallEff_Seq_Nop( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_MAIN,
  };

  BR_BALLEFF_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_INIT:
    { 
      int i;
      for( i = 0; i < BR_BALLEFF_CLWK_MAX; i++ )
      { 
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], FALSE );
      }
    }
    p_wk->is_end  = TRUE;
    *p_seq        = SEQ_MAIN;
    break;

  case SEQ_MAIN:
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  放射状に出る  STOP
 *
 *	@param	BR_SEQ_WORK *p_seqwk  シーケンス
 *	@param	*p_seq                シーケンス変数
 *	@param	*p_wk_adrs            ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Br_BallEff_Seq_Emit( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_EXIT,
  };

  BR_BALLEFF_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_INIT:
    { 
      int i;
      u16 angle;
      u16 angle_max;


      p_wk->cnt = 0;
      for( i = 0; i < BR_BALLEFF_CIRCLE_USE_MAX; i++ )
      { 
        angle = BR_BALLEFF_EMIT_ANGLE_MIN(i,BR_BALLEFF_CIRCLE_USE_MAX);
        angle_max = BR_BALLEFF_EMIT_ANGLE_MAX(i,BR_BALLEFF_CIRCLE_USE_MAX);

        MOVE_EMIT_Init( &p_wk->move[i].emit, &p_wk->init_pos, 
            BR_BALLEFF_EMIT_R_MIN, BR_BALLEFF_EMIT_R_MAX, angle, angle_max, BR_BALLEFF_EMIT_SYNC );

        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
      }
    }
    *p_seq  = SEQ_MAIN;
    /* fallthr  */

  case SEQ_MAIN:
    { 
      int i;
      GFL_POINT pos;
      GFL_CLACTPOS  clpos;
      for( i = 0; i < BR_BALLEFF_CIRCLE_USE_MAX; i++ )
      { 
        MOVE_EMIT_Main( &p_wk->move[i].emit, &pos, p_wk->cnt );
        clpos.x = pos.x;
        clpos.y = pos.y;
        GFL_CLACT_WK_SetPos( p_wk->p_clwk[i], &clpos, p_wk->draw );
      }

      if( p_wk->cnt++ > BR_BALLEFF_EMIT_SYNC )
      { 
        *p_seq  = SEQ_EXIT;
      }
    }
    break;

  case SEQ_EXIT:
    BR_SEQ_SetNext( p_wk->p_seq, Br_BallEff_Seq_Nop );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  直線に動く  STOP
 *
 *	@param	BR_SEQ_WORK *p_seqwk  シーケンス
 *	@param	*p_seq                シーケンス変数
 *	@param	*p_wk_adrs            ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Br_BallEff_Seq_Line( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_END
  };

  BR_BALLEFF_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_INIT:
    { 
      int i;
      p_wk->cnt   = 0;

      //初期座標
      for(i = 0; i < BR_BALLEFF_CLWK_MAX; i++ )
      { 
        p_wk->now_pos[i]  = sc_line_start_pos;
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
      }

      //先頭
      MOVE_LINE_Init( &p_wk->move[0].line, &p_wk->now_pos[0], &sc_line_end_pos, 0, BR_BALLEFF_LINE_SYNC );

      //追尾
      for( i = 1; i < BR_BALLEFF_CLWK_MAX; i++ )
      { 
        MOVE_HOMING_Init( &p_wk->move[i].homing, &p_wk->now_pos[i-1], &p_wk->now_pos[i], BR_BALLEFF_HOMING_SPEED(i) );
      }
    }
    *p_seq  = SEQ_MAIN;
  /* fallthr */

  case SEQ_MAIN:
    { 
      int i;
      GFL_CLACTPOS  clpos;
      BOOL is_end = TRUE;

      //先頭動作
      p_wk->is_end  = MOVE_LINE_Main( &p_wk->move[0].line, &p_wk->now_pos[0], p_wk->cnt );
      is_end  &= p_wk->is_end;

      //追尾
      for( i = 1; i < BR_BALLEFF_CLWK_MAX; i++ )
      { 
        is_end  &= MOVE_HOMING_Main( &p_wk->move[i].homing, &p_wk->now_pos[i] );
      }

      //設定
      for(i = 0; i < BR_BALLEFF_CLWK_MAX; i++ )
      { 
        clpos.x = p_wk->now_pos[i].x;
        clpos.y = p_wk->now_pos[i].y;
        GFL_CLACT_WK_SetPos( p_wk->p_clwk[i], &clpos, p_wk->draw );
      }

      p_wk->cnt++;

      if( is_end )
      { 
        *p_seq  = SEQ_END;
      }
    }
    break;
  case SEQ_END:
    BR_SEQ_SetNext( p_wk->p_seq, Br_BallEff_Seq_Nop );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  最初の演出  STOP
 *
 *	@param	BR_SEQ_WORK *p_seqwk  シーケンス
 *	@param	*p_seq                シーケンス変数
 *	@param	*p_wk_adrs            ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Br_BallEff_Seq_Opening( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    OPENING_LINE_USE_MAX  = BR_BALLEFF_CIRCLE_USE_MAX,//半分は降りてくる
    //半分は円を作る
  };

  enum
  { 
    SEQ_LINE_INIT,
    SEQ_LINE_MAIN,

    SEQ_CIRCLE_INIT,
    SEQ_CIRCLE_MAIN,

    SEQ_END
  };

  BR_BALLEFF_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_LINE_INIT:
    { 
      int i;
      p_wk->cnt   = 0;

      //初期座標
      for(i = 0; i < OPENING_LINE_USE_MAX; i++ )
      { 
        p_wk->now_pos[i]  = sc_opening_start_pos;
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
      }

      //先頭
      MOVE_LINE_Init( &p_wk->move[0].line, &p_wk->now_pos[0], &sc_opening_end_pos, 0, BR_BALLEFF_OPENING_SYNC );

      //追尾
      for( i = 1; i < OPENING_LINE_USE_MAX; i++ )
      { 
        MOVE_HOMING_Init( &p_wk->move[i].homing, &p_wk->now_pos[i-1], &p_wk->now_pos[i], BR_BALLEFF_HOMING_SPEED_OP(i) );
      }
    }
    *p_seq  = SEQ_LINE_MAIN;
  /* fallthr */

  case SEQ_LINE_MAIN:
    { 
      int i;
      GFL_CLACTPOS  clpos;

      //先頭動作
      p_wk->is_end_one[0]  = MOVE_LINE_Main( &p_wk->move[0].line, &p_wk->now_pos[0], p_wk->cnt );

      //追尾
      for( i = 1; i < OPENING_LINE_USE_MAX; i++ )
      { 
        p_wk->is_end_one[i] = MOVE_HOMING_Main( &p_wk->move[i].homing, &p_wk->now_pos[i] );
      }

      //設定
      for(i = 0; i < OPENING_LINE_USE_MAX; i++ )
      { 
        clpos.x = p_wk->now_pos[i].x;
        clpos.y = p_wk->now_pos[i].y;
        GFL_CLACT_WK_SetPos( p_wk->p_clwk[i], &clpos, p_wk->draw );
      }

      p_wk->cnt++;

      if( p_wk->is_end_one[0] )
      { 
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[0], FALSE );
        *p_seq  = SEQ_CIRCLE_INIT;
      }
    }
    break;

  case SEQ_CIRCLE_INIT:
    {
      int i = 0;
      p_wk->cnt = 0;
      
      //円を表示
      for( i = OPENING_LINE_USE_MAX; i < BR_BALLEFF_CLWK_MAX; i++ )
      { 
        MOVE_CIRCLE_Init( &p_wk->move[i].circle, &sc_opening_end_pos, BR_BALLEFF_CIRCLE_R, 0xFFFF*(i-OPENING_LINE_USE_MAX)/(BR_BALLEFF_CLWK_MAX - OPENING_LINE_USE_MAX ), BR_BALLEFF_CIRCLE_SYNC );
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
      }
    }
    *p_seq  = SEQ_CIRCLE_MAIN;
    /* fallthr */

  case SEQ_CIRCLE_MAIN:
    { 
      int i;
      GFL_CLACTPOS  clpos;

      //円を表示
      for( i = OPENING_LINE_USE_MAX; i < BR_BALLEFF_CLWK_MAX; i++ )
      { 
        MOVE_CIRCLE_Main( &p_wk->move[i].circle, &p_wk->now_pos[i], p_wk->cnt );
      }

      //追尾し続けている奴は終わったら消す
      for( i = 1; i < OPENING_LINE_USE_MAX; i++ )
      { 
        p_wk->is_end_one[i] = MOVE_HOMING_Main( &p_wk->move[i].homing, &p_wk->now_pos[i] );
        if( p_wk->is_end_one[i] )
        { 
          GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], FALSE );
        }
      }

      //設定
      for(i = 0; i < BR_BALLEFF_CLWK_MAX; i++ )
      { 
        clpos.x = p_wk->now_pos[i].x;
        clpos.y = p_wk->now_pos[i].y;
        GFL_CLACT_WK_SetPos( p_wk->p_clwk[i], &clpos, p_wk->draw );
      }

      p_wk->is_end  = TRUE;
      p_wk->cnt++;
    }
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  大きい円を描く  LOOP
 *
 *	@param	BR_SEQ_WORK *p_seqwk  シーケンス
 *	@param	*p_seq                シーケンス変数
 *	@param	*p_wk_adrs            ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Br_BallEff_Seq_BigCircle( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_MAIN,
  };

  BR_BALLEFF_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_INIT:
    { 
      int i;
      p_wk->cnt = 0;
      for( i = 0; i < BR_BALLEFF_CIRCLE_USE_MAX; i++ )
      { 
        MOVE_CIRCLE_Init( &p_wk->move[i].circle, &p_wk->init_pos, BR_BALLEFF_CIRCLE_R, 0xFFFF*i/BR_BALLEFF_CIRCLE_USE_MAX, BR_BALLEFF_CIRCLE_SYNC );
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
      }

      PMSND_PlaySE( BR_SND_SE_SEARCH );
    }
    *p_seq  = SEQ_MAIN;
    /* fallthr  */

  case SEQ_MAIN:
    { 
      int i;
      GFL_POINT pos;
      GFL_CLACTPOS  clpos;
      for( i = 0; i < BR_BALLEFF_CIRCLE_USE_MAX; i++ )
      { 
        MOVE_CIRCLE_Main( &p_wk->move[i].circle, &pos, p_wk->cnt );
        clpos.x = pos.x;
        clpos.y = pos.y;
        GFL_CLACT_WK_SetPos( p_wk->p_clwk[i], &clpos, p_wk->draw );
      }

      p_wk->cnt++;
    }
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  円を描く  LOOP
 *
 *	@param	BR_SEQ_WORK *p_seqwk  シーケンス
 *	@param	*p_seq                シーケンス変数
 *	@param	*p_wk_adrs            ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Br_BallEff_Seq_Circle( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_MAIN,
  };

  BR_BALLEFF_WORK *p_wk = p_wk_adrs;

#if 0
  switch( *p_seq )
  { 
  case SEQ_INIT:
    { 
      int i;
      int idx;
      p_wk->cnt = 0;
      for( i = 0; i < BR_BALLEFF_CIRCLE_USE_MAX; i++ )
      { 
        MOVE_ELLIPSE_Init( &p_wk->move[i].ellipse, &p_wk->init_pos, 10, 5, 0xFFFF*i/BR_BALLEFF_CIRCLE_USE_MAX, 0xFFFF * 45 /360, BR_BALLEFF_CIRCLE_SYNC );
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
      }
      for( i = BR_BALLEFF_CIRCLE_USE_MAX; i < BR_BALLEFF_CLWK_MAX; i++ )
      { 
        idx = i - BR_BALLEFF_CIRCLE_USE_MAX;
        MOVE_ELLIPSE_Init( &p_wk->move[i].ellipse, &p_wk->init_pos, 10, 5, 0xFFFF*idx/BR_BALLEFF_CIRCLE_USE_MAX, 0xFFFF * (45+90) /360, BR_BALLEFF_CIRCLE_SYNC );
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
      }
    }
    *p_seq  = SEQ_MAIN;
    /* fallthr  */

  case SEQ_MAIN:
    { 
      int i;
      GFL_POINT pos;
      GFL_CLACTPOS  clpos;
      for( i = 0; i < BR_BALLEFF_CIRCLE_USE_MAX; i++ )
      { 
        MOVE_ELLIPSE_Main( &p_wk->move[i].ellipse, &pos, p_wk->cnt );
        clpos.x = pos.x;
        clpos.y = pos.y;
        GFL_CLACT_WK_SetPos( p_wk->p_clwk[i], &clpos, p_wk->draw );
      }

      p_wk->cnt++;
    }
    break;
  }
#else
  switch( *p_seq )
  { 
  case SEQ_INIT:
    { 
      int i;
      p_wk->cnt = 0;
      for( i = 0; i < BR_BALLEFF_CIRCLE_USE_MAX; i++ )
      { 
        MOVE_CIRCLE_Init( &p_wk->move[i].circle, &p_wk->init_pos, 10, 0xFFFF*i/BR_BALLEFF_CIRCLE_USE_MAX, BR_BALLEFF_CIRCLE_SYNC );
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
      }
    }
    *p_seq  = SEQ_MAIN;
    /* fallthr  */

  case SEQ_MAIN:
    { 
      int i;
      GFL_POINT pos;
      GFL_CLACTPOS  clpos;
      for( i = 0; i < BR_BALLEFF_CIRCLE_USE_MAX; i++ )
      { 
        MOVE_CIRCLE_Main( &p_wk->move[i].circle, &pos, p_wk->cnt );
        clpos.x = pos.x;
        clpos.y = pos.y;
        GFL_CLACT_WK_SetPos( p_wk->p_clwk[i], &clpos, p_wk->draw );
      }

      p_wk->cnt++;
    }
    break;
  }
#endif
}
//----------------------------------------------------------------------------
/**
 *	@brief  円を描く（移動）  LOOP
 *
 *	@param	BR_SEQ_WORK *p_seqwk  シーケンス
 *	@param	*p_seq                シーケンス変数
 *	@param	*p_wk_adrs            ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Br_BallEff_Seq_CircleCont( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 

  BR_BALLEFF_WORK *p_wk = p_wk_adrs;
}

//----------------------------------------------------------------------------
/**
 *	@brief  放射に動いたあと、ついて行く
 *
 *	@param	BR_SEQ_WORK *p_seqwk  シーケンス
 *	@param	*p_seq                シーケンス変数
 *	@param	*p_wk_adrs            ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Br_BallEff_Seq_EmitFollow( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_INIT_EMIT,
    SEQ_MAIN_EMIT,
    SEQ_EXIT_EMIT,

    SEQ_INIT_FOLLOW,
    SEQ_MAIN_FOLLOW,
    SEQ_EXIT_FOLLOW,
  };

  BR_BALLEFF_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_INIT_EMIT:
    { 
      int i;
      u16 angle;
      u16 angle_max;


      p_wk->cnt = 0;
      for( i = 0; i < BR_BALLEFF_CIRCLE_USE_MAX; i++ )
      { 
        angle = BR_BALLEFF_EMIT_ANGLE_MIN(i,BR_BALLEFF_CIRCLE_USE_MAX);
        angle_max = BR_BALLEFF_EMIT_ANGLE_MAX(i,BR_BALLEFF_CIRCLE_USE_MAX);

        MOVE_EMIT_Init( &p_wk->move[i].emit, &p_wk->init_pos, 
            BR_BALLEFF_EMIT_R_MIN, BR_BALLEFF_EMIT_R_MAX, angle, angle_max, BR_BALLEFF_EMIT_FOLLOW_SYNC );

        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
      }
    }
    *p_seq  = SEQ_MAIN_EMIT;
    /* fallthr  */

  case SEQ_MAIN_EMIT:
    { 
      int i;
      GFL_CLACTPOS  clpos;
      for( i = 0; i < BR_BALLEFF_CIRCLE_USE_MAX; i++ )
      { 
        MOVE_EMIT_Main( &p_wk->move[i].emit, &p_wk->now_pos[i], p_wk->cnt );
        clpos.x = p_wk->now_pos[i].x;
        clpos.y = p_wk->now_pos[i].y;
        GFL_CLACT_WK_SetPos( p_wk->p_clwk[i], &clpos, p_wk->draw );
      }

      if( p_wk->cnt++ > BR_BALLEFF_EMIT_FOLLOW_SYNC )
      { 
        *p_seq  = SEQ_EXIT_EMIT;
      }
    }
    break;

  case SEQ_EXIT_EMIT:
    { 
      *p_seq  = SEQ_INIT_FOLLOW;
    }
    break;

  case SEQ_INIT_FOLLOW:
    { 
      int i;
      for( i = 0; i < BR_BALLEFF_CIRCLE_USE_MAX; i++ )
      { 
        MOVE_HOMING_Init( &p_wk->move[i].homing, p_wk->cp_folow_pos, &p_wk->now_pos[i], BR_BALLEFF_HOMING_SPEED_MENU(i) );
      }
    }
    *p_seq  = SEQ_MAIN_FOLLOW;
    /* fallthrough */

  case SEQ_MAIN_FOLLOW:
    { 
      int i;
      BOOL is_end = TRUE;
      GFL_CLACTPOS  clpos;
      for( i = 0; i < BR_BALLEFF_CIRCLE_USE_MAX; i++ )
      { 
        is_end  &= MOVE_HOMING_Main( &p_wk->move[i].homing, &p_wk->now_pos[i] );
        clpos.x = p_wk->now_pos[i].x;
        clpos.y = p_wk->now_pos[i].y;
        GFL_CLACT_WK_SetPos( p_wk->p_clwk[i], &clpos, p_wk->draw );
      }
      if( is_end )
      { 
        *p_seq  = SEQ_EXIT_FOLLOW;
      }
    }
    break;

  case SEQ_EXIT_FOLLOW:
    BR_SEQ_SetNext( p_seqwk, Br_BallEff_Seq_Nop );
    break;

  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  オープニングのタッチの動き
 *
 *	@param	BR_SEQ_WORK *p_seqwk  シーケンス
 *	@param	*p_seq                シーケンス変数
 *	@param	*p_wk_adrs            ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Br_BallEff_Seq_OpeningTouch( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_CIRCLE,
    SEQ_WAIT_CIRCLE,
    SEQ_END,
  };
   
  BR_BALLEFF_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_START_CIRCLE:
    {
      int i;
      GFL_CLACTPOS  clpos;
      u16 now_angle;

      //ボール変更
      BR_BALLEFF_SetAnmSeq( p_wk, 0 );

      //円を表示  オープニングに続き
      for( i = BR_BALLEFF_CIRCLE_USE_MAX; i < BR_BALLEFF_CLWK_MAX; i++ )
      { 
        now_angle = p_wk->move[i].circle.init_angle + p_wk->cnt / p_wk->move[i].circle.cnt_max;
        MOVE_EMIT_Init( &p_wk->move[i].emit, &sc_opening_end_pos, BR_BALLEFF_CIRCLE_R, 1, now_angle, now_angle + 0xF000, 30 );
        GFL_CLACT_WK_SetObjMode( p_wk->p_clwk[i], GX_OAM_MODE_NORMAL );
      }
      p_wk->cnt = 0;

      *p_seq  = SEQ_WAIT_CIRCLE;
    }
    /* fallthrough */

  case SEQ_WAIT_CIRCLE:
    { 
      int i;
      GFL_POINT pos;
      GFL_CLACTPOS  clpos;
      for( i = BR_BALLEFF_CIRCLE_USE_MAX; i < BR_BALLEFF_CLWK_MAX; i++ )
      { 
        MOVE_EMIT_Main( &p_wk->move[i].emit, &pos, p_wk->cnt );
        clpos.x = pos.x;
        clpos.y = pos.y;
        GFL_CLACT_WK_SetPos( p_wk->p_clwk[i], &clpos, p_wk->draw );
      }

      if( p_wk->cnt++ > 60 )
      { 
        *p_seq  = SEQ_END;
      }
    }
    break;

  case SEQ_END:
    BR_SEQ_SetNext( p_seqwk, Br_BallEff_Seq_Nop );
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  放射の動き  初期化
 *
 *	@param	MOVE_EMIT *p_wk           ワーク
 *	@param	GFL_POINT *cp_center_pos  中心座標
 *	@param	r                         半径
 *	@param  max_r                     動き終了時の半径
 *	@param	init_angle                最初の角度
 *	@param  end_angle                 動き終了時の角度;
 *	@param	cnt_max                   動き
 */
//-----------------------------------------------------------------------------
static void MOVE_EMIT_Init( MOVE_EMIT *p_wk, const GFL_POINT *cp_center_pos, u16 r, u16 max_r, u16 init_angle, u16 end_angle, int cnt_max )
{ 
  GFL_STD_MemClear( p_wk, sizeof(MOVE_EMIT) );
  p_wk->r           = r;
  p_wk->max_r       = max_r;
  p_wk->init_angle  = init_angle;
  p_wk->end_angle   = end_angle;
  p_wk->cnt_max     = cnt_max;
  p_wk->center_pos  = *cp_center_pos;

  MOVE_EMIT_Main( p_wk, &p_wk->now_pos, 0 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  放射の動き  メイン処理
 *
 *	@param	MOVE_CIRCLE *p_wk ワーク
 *	@param	*p_now_pos        座標
 *	@param	cnt               シンク
 *
 *	@return TREUで動き終了  FALSEで動作中
 */
//-----------------------------------------------------------------------------
static BOOL MOVE_EMIT_Main( MOVE_EMIT *p_wk, GFL_POINT *p_now_pos, int cnt )
{ 

  const u16 diff_angle  = p_wk->end_angle - p_wk->init_angle;
  const s16 diff_r      = (s16)p_wk->max_r - (s16)p_wk->r;
  const s8  dir_r       = diff_r / MATH_IAbs(diff_r);

  u16 angle;
  s32 r;

  //現在のアングルを取得
  angle = p_wk->init_angle + (diff_angle * cnt / p_wk->cnt_max);
  r     = p_wk->r + dir_r * ( MATH_IAbs(diff_r) * cnt / p_wk->cnt_max );
  r     = MATH_IMax( r, 0 );

  //NAGI_Printf( "ang%d r%d\n", angle, r );

  ///角度から座標を求める
  p_wk->now_pos.x = (FX_CosIdx( angle ) * r) >> FX32_SHIFT;
  p_wk->now_pos.y = (FX_SinIdx( angle ) * r) >> FX32_SHIFT;

  //中心座標を足す
  p_wk->now_pos.x += p_wk->center_pos.x;
  p_wk->now_pos.y += p_wk->center_pos.y;

  //受け取りバッファに格納
  if( p_now_pos )
  { 
    *p_now_pos  = p_wk->now_pos;
  }

  return cnt == p_wk->cnt_max;
}
//----------------------------------------------------------------------------
/**
 *	@brief  円運動  初期化
 *
 *	@param	MOVE_CIRCLE *p_wk         ワーク
 *	@param	GFL_POINT *cp_center_pos  中心座標
 *	@param	r                         半径
 *	@param	init_angle                初期角度
 *	@param	cnt_max                   動作するシンク
 */
//-----------------------------------------------------------------------------
static void MOVE_CIRCLE_Init( MOVE_CIRCLE *p_wk, const GFL_POINT *cp_center_pos, u16 r, u16 init_angle, int cnt_max )
{ 
  GFL_STD_MemClear( p_wk, sizeof(MOVE_CIRCLE) );
  p_wk->center_pos  = *cp_center_pos;
  p_wk->r           = r;
  p_wk->cnt_max     = cnt_max;
  p_wk->init_angle  = init_angle;

  MOVE_CIRCLE_Main( p_wk, NULL, 0 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  円運動  メイン処理
 *
 *	@param	MOVE_CIRCLE *p_wk ワーク
 *	@param	*p_now_pos        現在座標受け取り
 *	@param	cnt               カウンタ
 *
 *	@return TRUE maxシンクを超えた  FALSE maxシンクを超えてない
 */
//-----------------------------------------------------------------------------
static BOOL MOVE_CIRCLE_Main( MOVE_CIRCLE *p_wk, GFL_POINT *p_now_pos, int cnt )
{ 
  u16 angle;

  //現在のアングルを取得
  angle = p_wk->init_angle + (0xFFFF * cnt / p_wk->cnt_max);

  ///角度から座標を求める
  p_wk->now_pos.x = (FX_CosIdx( angle ) * p_wk->r) >> FX32_SHIFT;
  p_wk->now_pos.y = (FX_SinIdx( angle ) * p_wk->r) >> FX32_SHIFT;

  //中心座標を足す
  p_wk->now_pos.x += p_wk->center_pos.x;
  p_wk->now_pos.y += p_wk->center_pos.y;


  //受け取りバッファに格納
  if( p_now_pos )
  { 
    *p_now_pos  = p_wk->now_pos;
  }

  return cnt == p_wk->cnt_max;
}
//----------------------------------------------------------------------------
/**
 *	@brief  直線運動  初期化
 *
 *	@param	MOVE_LINE *p_wk           ワーク
 *	@param	GFL_POINT *cp_start_pos   開始点
 *	@param	GFL_POINT *cp_end_pos     終了点
 *	@param	wait_cnt                  待機時間
 *	@param	cnt_max                   動作時間
 */
//-----------------------------------------------------------------------------
static void MOVE_LINE_Init( MOVE_LINE *p_wk, const GFL_POINT *cp_start_pos, const GFL_POINT *cp_end_pos, int wait_cnt, int cnt_max )
{ 
  GFL_STD_MemClear( p_wk, sizeof(MOVE_LINE) );
  p_wk->start_pos = *cp_start_pos;
  p_wk->end_pos   = *cp_end_pos;
  p_wk->wait_cnt  = wait_cnt;
  p_wk->cnt_max   = cnt_max;

  MOVE_LINE_Main( p_wk, NULL, 0 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  直線運動  メイン処理
 *
 *	@param	MOVE_LINE *p_wk ワーク
 *	@param	*p_now_pos      現在座標の受け取り
 *	@param	cnt             カウンタ
 *
 *	@return TRUE maxシンクを超えた  FALSE maxシンクを超えてない
 */
//-----------------------------------------------------------------------------
static BOOL MOVE_LINE_Main( MOVE_LINE *p_wk, GFL_POINT *p_now_pos, int cnt )
{ 
  BOOL ret  = FALSE;
  cnt -= p_wk->wait_cnt;
  cnt = MATH_IMax( cnt, 0 );

  p_wk->now_pos.x = p_wk->start_pos.x + (p_wk->end_pos.x - p_wk->start_pos.x) * cnt / p_wk->cnt_max;
  p_wk->now_pos.y = p_wk->start_pos.y + (p_wk->end_pos.y - p_wk->start_pos.y) * cnt / p_wk->cnt_max;

  if( cnt >= p_wk->cnt_max )
  {
    p_wk->now_pos = p_wk->end_pos;
    ret = TRUE;
  }

  if( p_now_pos )
  { 
    *p_now_pos  = p_wk->now_pos;
  }

  return ret;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ホーミング  初期化
 *
 *	@param	MOVE_HOMING *p_wk         ワーク
 *	@param	GFL_POINT *cp_homing_pos  追尾する座標
 *	@param	speed                     スピード
 */
//-----------------------------------------------------------------------------
static void MOVE_HOMING_Init( MOVE_HOMING *p_wk, const GFL_POINT *cp_homing_pos,const GFL_POINT *cp_init_pos, fx32 speed )
{ 
  GFL_STD_MemClear( p_wk, sizeof(MOVE_HOMING) );
  p_wk->cp_homing_pos = cp_homing_pos;
  p_wk->now_pos.x       = FX32_CONST(cp_init_pos->x);
  p_wk->now_pos.y       = FX32_CONST(cp_init_pos->y);
  p_wk->now_pos.z       = 0;
  p_wk->speed           = speed;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ホーミング  メイン処理
 *
 *	@param	MOVE_HOMING *p_wk   ワーク
 *	@param	*p_now_pos          座標
 *
 *	@return TRUEならば追尾座標と同じ場所  FALSEならば追尾中
 */
//-----------------------------------------------------------------------------
static BOOL MOVE_HOMING_Main( MOVE_HOMING *p_wk, GFL_POINT *p_now_pos )
{
  //自分と相手の差分
  VecFx32 diff;
  VecFx32 add;
  fx32  distance;

  diff.x  = FX32_CONST(p_wk->cp_homing_pos->x) - p_wk->now_pos.x;
  diff.y  = FX32_CONST(p_wk->cp_homing_pos->y) - p_wk->now_pos.y;
  diff.z  = 0;

  distance  = VEC_Mag( &diff );

  if( distance != 0 )
  { 
    add.x = FX_Div( FX_Mul( diff.x, p_wk->speed ), distance );
    add.y = FX_Div( FX_Mul( diff.y, p_wk->speed ), distance );

    if( MATH_IAbs(diff.x) > MATH_IAbs(add.x) )
    { 
      p_wk->now_pos.x += add.x;
    }
    else
    { 
      p_wk->now_pos.x = FX32_CONST(p_wk->cp_homing_pos->x);
    }

    if( MATH_IAbs(diff.y) > MATH_IAbs(add.y) )
    { 
      p_wk->now_pos.y += add.y;
    }
    else
    { 
      p_wk->now_pos.y = FX32_CONST(p_wk->cp_homing_pos->y);
    }

    //OS_Printf( "X%.3f Y%.3f S%.3f calc%.3f dist%.3f \n", FX_FX32_TO_F32(diff.x), FX_FX32_TO_F32(diff.y), FX_FX32_TO_F32(distance) , FX_FX32_TO_F32( FX_Div( FX_Mul( diff.y, p_wk->speed ), distance ) ));
  }

  if( p_now_pos )
  { 
    p_now_pos->x  = p_wk->now_pos.x >> FX32_SHIFT;
    p_now_pos->y  = p_wk->now_pos.y >> FX32_SHIFT;
  }

  return distance == 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  楕円軌道  初期化
 *
 *	@param	MOVE_ELLIPSE *p_wk        ワーク
 *	@param	GFL_POINT *cp_center_pos  中心座標
 *	@param	r_x                       半径（X方向）
 *	@param	r_y                       半径（Y方向）
 *	@param	init_angle                初期角度
 *  @param  ellipse_angle             楕円の角度
 *	@param	cnt_max                   何シンクで１周するか
 *
 */
//-----------------------------------------------------------------------------
static void MOVE_ELLIPSE_Init( MOVE_ELLIPSE *p_wk, const GFL_POINT *cp_center_pos, u16 r_x, u16 r_y, u16 init_angle, u16 ellipse_angle, int cnt_max )
{ 
  GFL_STD_MemClear( p_wk, sizeof(MOVE_ELLIPSE) );
  p_wk->center_pos  = *cp_center_pos;
  p_wk->r_x         = r_x;
  p_wk->r_y         = r_y;
  p_wk->init_angle  = init_angle;
  p_wk->ellipse_angle = ellipse_angle;
  p_wk->cnt_max     = cnt_max;

  MOVE_ELLIPSE_Main( p_wk, &p_wk->now_pos, 0 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  楕円軌道  メイン処理
 *
 *	@param	MOVE_ELLIPSE *p_wk  ワーク
 *	@param	*p_now_pos          現在の位置（受け取り）
 *	@param	cnt                 現在のシンク
 */
//-----------------------------------------------------------------------------
static BOOL MOVE_ELLIPSE_Main( MOVE_ELLIPSE *p_wk, GFL_POINT *p_now_pos, int cnt )
{ 
  u16 angle;

  //現在のアングルを取得
  angle = p_wk->init_angle + (0xFFFF * cnt / p_wk->cnt_max);

  ///角度から座標を求める
  p_wk->now_pos.x = (FX_Mul( p_wk->r_x * FX_CosIdx( angle ), FX_CosIdx( p_wk->ellipse_angle )) -
                    FX_Mul( p_wk->r_y * FX_SinIdx( angle ), FX_SinIdx( p_wk->ellipse_angle ) ))
                    >> FX32_SHIFT;

  p_wk->now_pos.y = (FX_Mul( p_wk->r_x * FX_CosIdx( angle ), FX_SinIdx( p_wk->ellipse_angle )) +
                    FX_Mul( p_wk->r_y * FX_SinIdx( angle ), FX_CosIdx( p_wk->ellipse_angle ) ))
                    >> FX32_SHIFT;

  //中心座標を足す
  p_wk->now_pos.x += p_wk->center_pos.x;
  p_wk->now_pos.y += p_wk->center_pos.y;


  //受け取りバッファに格納
  if( p_now_pos )
  { 
    *p_now_pos  = p_wk->now_pos;
  }

  return cnt == p_wk->cnt_max;
}
