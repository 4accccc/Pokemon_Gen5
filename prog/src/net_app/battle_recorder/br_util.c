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

//アーカイブ
#include "arc_def.h"
#include "msg/msg_battle_rec.h"

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
};
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
	//一端消去
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );	

	//文字列作成
	GFL_MSG_GetString( p_msg, strID, p_wk->p_strbuf );

	//文字列描画
	PRINT_UTIL_Print( &p_wk->util, p_wk->p_que, 0, 0, p_wk->p_strbuf, p_font );
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
	//一端消去
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );	

	//文字列作成
  GFL_STR_CopyBuffer( p_wk->p_strbuf, cp_strbuf );

	//文字列描画
	PRINT_UTIL_Print( &p_wk->util, p_wk->p_que, 0, 0, p_wk->p_strbuf, p_font );
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
	//一端消去
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );	

	//文字列作成
	GFL_MSG_GetString( p_msg, strID, p_wk->p_strbuf );

	//文字列描画
	PRINT_UTIL_PrintColor( &p_wk->util, p_wk->p_que, 0, 0, p_wk->p_strbuf, p_font, lsb );
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
	//一端消去
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );	

	//文字列作成
  GFL_STR_CopyBuffer( p_wk->p_strbuf, cp_strbuf );

	//文字列描画
	PRINT_UTIL_PrintColor( &p_wk->util, p_wk->p_que, 0, 0, p_wk->p_strbuf, p_font, lsb );
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

typedef BOOL (*BR_LIST_MOVE_FUNC)( BR_LIST_WORK *p_wk, s8 value );
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
  s8    dumy[3];
  
  BR_LIST_MOVE_FUNC move_callback;
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
    p_wk->param = *cp_param;
    p_wk->min       = 0;
    p_wk->max       = cp_param->list_max;
    p_wk->line_max  = cp_param->h / cp_param->str_line;
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
      p_wk->hittbl[ i ].rect.bottom  = (p_wk->param.y + i * (p_wk->param.str_line+1))*8;
    }
    GF_ASSERT( i < BR_LIST_HITTBL_MAX )
    p_wk->hittbl[ i ].rect.left    = GFL_UI_TP_HIT_NONE;
    p_wk->hittbl[ i ].rect.top     = GFL_UI_TP_HIT_NONE;
    p_wk->hittbl[ i ].rect.right   = GFL_UI_TP_HIT_NONE;
    p_wk->hittbl[ i ].rect.bottom  = GFL_UI_TP_HIT_NONE;

    p_wk->move_callback = Br_TouchList_MoveCallback;
  }
  else if( p_wk->param.type == BR_LIST_TYPE_CURSOR )
  { 
    p_wk->move_callback = Br_CursorList_MoveCallback;
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

  //リストが動作しなかったら、矢印を消す
  { 
    if( !BR_LIST_IsMoveEnable( p_wk ) )
    { 
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[BR_LIST_CLWK_ALLOW_D], FALSE );
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
        Br_List_Write( p_wk, p_wk->list );
      }

      //一番上まできたら、↑の矢印を
      //一番下まできたら、↓の矢印を消す
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

  //リスト選択
  { 
    p_wk->select_param  = BR_LIST_SELECT_NONE;
    if( p_wk->param.type == BR_LIST_TYPE_TOUCH )
    {
      int ret;
      ret = GFL_UI_TP_HitTrg( p_wk->hittbl );
      if( ret != GFL_UI_TP_HIT_NONE )
      { 
        p_wk->select_param  = p_wk->param.cp_list[ ret + p_wk->list ].param;
      }
    }
    else if( p_wk->param.type == BR_LIST_TYPE_CURSOR )
    { 
      GF_ASSERT( p_wk->cursor + p_wk->list < p_wk->param.list_max  );
      p_wk->select_param  = p_wk->param.cp_list[ p_wk->cursor + p_wk->list ].param;
    }
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
  return cp_wk->line_max < cp_wk->param.list_max;
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
  if( p_wk->list == 0  )
  {
    //リスト上部の場合カーソルが中央に動くまでリストは動かない
    if(p_wk->cursor == p_wk->line_max/2 && value == 1 )
    {
      p_wk->list  += 1;
      return TRUE;
    }
    else
    { 
      if( 0 <= p_wk->cursor + value && p_wk->cursor + value <= p_wk->line_max/2 )
      { 
        p_wk->cursor  += value;
        return TRUE;
      }
    }
  }
  else if( p_wk->list + p_wk->line_max ==  p_wk->param.list_max )
  { 
    //リスト下部の場合
    if( p_wk->cursor == p_wk->line_max/2 && value == -1 )
    { 
      p_wk->list  += -1;
      return TRUE;
    }
    else
    {
      if( p_wk->line_max/2 <= p_wk->cursor + value && p_wk->cursor + value < p_wk->line_max )
      { 
        p_wk->cursor  += value;
        return TRUE;
      }
    }
  }
  else
  { 
    //その他はリストが動く
    p_wk->list  += value;
    return TRUE;
  }

  return FALSE;
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
 *	@param	heapID                      ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
BR_PROFILE_WORK * BR_PROFILE_CreateMainDisplay( const GDS_PROFILE_PTR cp_profile, BR_RES_WORK *p_res, GFL_CLUNIT *p_unit, PRINT_QUE *p_que, HEAPID heapID )
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
      switch(i)
      { 
      case BR_PROFILE_MSGWINID_M_NAME:   //●●●のプロフィール
        { 
          STRBUF  *p_name;
          p_strbuf  = GFL_MSG_CreateString( p_msg, sc_msgwin_data[i].msgID );
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
          p_strbuf  = GFL_MSG_CreateString( p_msg, sc_msgwin_data[i].msgID );
          p_src     = GFL_MSG_CreateString( p_msg, sc_msgwin_data[i].msgID );
          WORDSET_RegisterNumber( p_word, 0, month, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
          WORDSET_ExpandStr( p_word, p_strbuf, p_src );
          GFL_STR_DeleteBuffer( p_src );
        }
        break;
      case BR_PROFILE_MSGWINID_M_COUNTRY:    //住んでいる場所国名
        { 
          const u32 nation = GDS_Profile_GetNation( cp_profile );
          p_strbuf  = GFL_MSG_CreateString( p_msg, sc_msgwin_data[i].msgID );
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
          p_strbuf  = GFL_MSG_CreateString( p_msg, sc_msgwin_data[i].msgID );
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
            p_strbuf  = GFL_MSG_CreateString( p_msg, sc_msgwin_data[i].msgID );
            p_src     = GFL_MSG_CreateString( p_msg, sc_msgwin_data[i].msgID );
            WORDSET_RegisterWord( p_word, 0, p_self, GDS_Profile_GetSex(cp_profile), TRUE, PM_LANG );
            WORDSET_ExpandStr( p_word, p_strbuf, p_src );
            GFL_STR_DeleteBuffer( p_self );
            GFL_STR_DeleteBuffer( p_src );
          }
          else
          { 
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

    if( mons != 0 )
    { 
      { 
        ARCHANDLE *p_handle;
        p_handle  = GFL_ARC_OpenDataHandle( ARCID_POKEICON, GFL_HEAP_LOWID(heapID) );
        p_wk->res_icon_plt  = GFL_CLGRP_PLTT_RegisterComp( p_handle, 
            POKEICON_GetPalArcIndex(),
            CLSYS_DRAW_MAIN, PLT_OBJ_M_POKEICON*0x20, heapID );
        p_wk->res_icon_cel = GFL_CLGRP_CELLANIM_Register( p_handle,
            POKEICON_GetCellArcIndex(), POKEICON_GetAnmArcIndex(), heapID );
        p_wk->res_icon_chr  = GFL_CLGRP_CGR_Register( p_handle,
            POKEICON_GetCgxArcIndexByMonsNumber( mons, form, egg ),
            FALSE, CLSYS_DRAW_MAIN, heapID );
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
        GFL_CLACT_WK_SetPlttOffs( p_wk->p_pokeicon, POKEICON_GetPalNum( mons, form, egg ), CLWK_PLTTOFFS_MODE_OAM_COLOR );
      }
    }
  }

  //自分のみため
  { 
    //@todo
    const int self  = 1 + GDS_Profile_GetTrainerView(cp_profile);
    { 
      ARCHANDLE *p_handle;
      p_handle  = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), GFL_HEAP_LOWID(heapID) );
      p_wk->res_self_plt  = GFL_CLGRP_PLTT_Register( p_handle, 
                          APP_COMMON_GetNull4x4PltArcIdx(),
                          CLSYS_DRAW_MAIN, PLT_OBJ_M_POKEICON*0x20, heapID );
      p_wk->res_self_cel = GFL_CLGRP_CELLANIM_Register( p_handle,
          APP_COMMON_GetNull4x4CellArcIdx( APP_COMMON_MAPPING_128K ), 
          APP_COMMON_GetNull4x4AnimeArcIdx( APP_COMMON_MAPPING_128K ), heapID );
      p_wk->res_self_chr  = GFL_CLGRP_CGR_Register( p_handle,
          APP_COMMON_GetNull4x4CharArcIdx(),
          FALSE, CLSYS_DRAW_MAIN, heapID );
      GFL_ARC_CloseDataHandle( p_handle );
    }
    { 
      GFL_CLWK_DATA data;
      GFL_STD_MemClear( &data, sizeof(GFL_CLWK_DATA) );
      data.pos_x  = 48;
      data.pos_y  = 64;
      p_wk->p_selficon  = GFL_CLACT_WK_Create( p_unit,
          p_wk->res_self_chr,p_wk->res_self_plt,p_wk->res_self_cel,
            &data, CLSYS_DEFREND_MAIN, heapID );
      CLWK_TransNSBTX( p_wk->p_selficon, ARCID_MMDL_RES, self, 3, NSBTX_DEF_SX, NSBTX_DEF_SY, 0, CLSYS_DRAW_MAIN, heapID );
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
