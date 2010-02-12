//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		zknd_tbar.c
 *	@brief	アプリ用下画面タッチバー
 *	@author	Koji Kawada
 *	@data		2010.02.04
 *
 *  @note   touchbar.cをコピペして、
 *          (1) TOUCHBARをZKND_TBARに置換し、
 *          (2) TouchbarをZknd_Tbarに置換し、
 *          (3) ui/touchbarをzknd_tbarに置換し、
 *          必要な機能を追加しただけです。
 *          Revision 14819 Date 2009.12.16が元。
 *          変更箇所にはZKND_Modifiedという印を付けておきました。
 *
 *  モジュール名：ZKND_TBAR
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>

//sound
#include "sound/pm_sndsys.h"

//mine
#include "zknd_tbar.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	リソース
//=====================================
//リソース
typedef enum
{	
	ZKND_TBAR_RES_COMMON_PLT,
	ZKND_TBAR_RES_COMMON_CHR,
	ZKND_TBAR_RES_COMMON_CEL,

	ZKND_TBAR_RES_MAX
}RES_TYPE;

//-------------------------------------
///	シーケンス
//=====================================
enum
{
	ZKND_TBAR_SEQ_MAIN,	//タッチ待ち
	ZKND_TBAR_SEQ_ANM_TRG,		//ボタンアニメ
	ZKND_TBAR_SEQ_ANM,		//ボタンアニメ
	ZKND_TBAR_SEQ_TRG,		//トリガ
} ;

//-------------------------------------
///	動作関数タイプ
//=====================================
typedef enum
{
	ZKND_TBAR_ICON_MOVETYPE_PUSH,	//押されたら、押されるアニメになるボタン
	ZKND_TBAR_ICON_MOVETYPE_FLIP,	//押されたら、ONとOFFが切り替るボタン
} ZKND_TBAR_ICON_MOVETYPE;


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	アイコンワーク
//=====================================
typedef struct _ICON_WORK ICON_WORK;
typedef void (*PUSH_FUNC)( ICON_WORK *p_wk );
struct _ICON_WORK
{
	GFL_CLWK						*p_clwk;		
	BOOL								is_active;		//アクティブか
	u32									now_anmseq;		//現在のアニメ
	PUSH_FUNC						push_func;		//押された時の動作
	ZKND_TBAR_ICON_MOVETYPE	movetype;	//動作タイプ
	ZKND_TBAR_ITEM_ICON			data;			//ボタン情報
} ;
//-------------------------------------
///	共通リソース
//=====================================
typedef struct 
{
	u32				res[ZKND_TBAR_RES_MAX];
	u32				bg_frm;
	GFL_ARCUTIL_TRANSINFO				chr_pos;
} RES_WORK;

//-------------------------------------
///	下画面バーメインワーク
//=====================================
struct _ZKND_TBAR_WORK
{
	s32				trg;			//押したアイコンのインデックス
	u32				tbl_len;	//最大アイコンのテーブル長
	u32				seq;			//シーケンス
  
  BOOL      whole_is_active;  // 全体専用のアクティブフラグ  // ZKND_Modified 全体専用の設定を追加。
  BOOL      whole_is_lock;    // 全体専用のロック            // ZKND_Modified 全体専用の設定を追加。

	RES_WORK	res;			//共通リソース
	ICON_WORK	icon[0];	//アイコン
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	アプリケーション
//=====================================
static ICON_WORK * Zknd_Tbar_Search( ZKND_TBAR_WORK *p_wk, ZKND_TBAR_ICON type );
static const ICON_WORK * Zknd_Tbar_SearchConst( const ZKND_TBAR_WORK *cp_wk, ZKND_TBAR_ICON type );

//-------------------------------------
///	リソース
//=====================================
static void RES_Init( RES_WORK *p_wk, u8 frm, CLSYS_DRAW_TYPE	clsys_draw_type, PALTYPE bgplttype, APP_COMMON_MAPPING mapping, u8 bg_plt, u8 obj_plt, BOOL is_notload_bg, HEAPID heapID );
static void RES_Exit( RES_WORK *p_wk );
static u32 RES_GetResource( const RES_WORK *cp_wk, RES_TYPE type );
static void RES_UTIL_TransVramScreenEx( ARCHANDLE *handle, ARCDATID datID, u32 frm, u32 chr_ofs, u8 src_x, u8 src_y, u8 src_w, u8 src_h, u8 dst_x, u8 dst_y, u8 dst_w, u8 dst_h,  u8 plt, BOOL compressedFlag, HEAPID heapID );
//-------------------------------------
///	ICON
//=====================================
static void ICON_Init( ICON_WORK *p_wk, GFL_CLUNIT* p_unit, const RES_WORK *cp_res, const ZKND_TBAR_ITEM_ICON *cp_setup,CLSYS_DEFREND_TYPE	clsys_def_type, HEAPID heapID );
static void ICON_Exit( ICON_WORK *p_wk );
static BOOL ICON_Main( ICON_WORK *p_wk );
static BOOL ICON_IsMoveEnd( const ICON_WORK *cp_wk );
static ZKND_TBAR_ICON ICON_GetType( const ICON_WORK *cp_wk );
static void ICON_SetVisible( ICON_WORK *p_wk, BOOL is_visible );
static BOOL ICON_GetVisible( const ICON_WORK *cp_wk );
static void ICON_SetActive( ICON_WORK *p_wk, BOOL is_active );
static BOOL ICON_GetActive( const ICON_WORK *cp_wk );
static void ICON_SetSE( ICON_WORK *p_wk, u32 se );
static u32 ICON_GetSE( const ICON_WORK *cp_wk );
static void ICON_SetKey( ICON_WORK *p_wk, u32 key );
static u32 ICON_GetKey( const ICON_WORK *cp_wk );
static void ICON_SetFlip( ICON_WORK *p_wk, BOOL is_flip );
static BOOL ICON_GetFlip( const ICON_WORK *cp_wk );
static void Icon_PushFuncNormal( ICON_WORK *p_wk );
static void Icon_PushFuncFlip( ICON_WORK *p_wk );

//=============================================================================
/**
 *					データ
 */
//=============================================================================
//-------------------------------------
///	アイコン情報
//	ZKND_TBAR_ICONと対応したアクティブテーブル
//=====================================
static const struct
{	
	u16 active_anmseq;
	u16 noactive_anmseq;
	u16 push_anmseq;
	u16 se;
	u32	key;
	ZKND_TBAR_ICON_MOVETYPE	movetype;
} sc_common_icon_data[ZKND_TBAR_ICON_MAX]	=
{	
	//ZKND_TBAR_ICON_CLOSE,	//×ボタン
	{	
		APP_COMMON_BARICON_EXIT,
		APP_COMMON_BARICON_EXIT_OFF,
		APP_COMMON_BARICON_EXIT_ON,
		ZKND_TBAR_SE_CLOSE,
		PAD_BUTTON_X,
		ZKND_TBAR_ICON_MOVETYPE_PUSH,
	},
	//ZKND_TBAR_ICON_RETURN,	//←┘ボタン
	{	
		APP_COMMON_BARICON_RETURN,
		APP_COMMON_BARICON_RETURN_OFF,
		APP_COMMON_BARICON_RETURN_ON,
		ZKND_TBAR_SE_CANCEL,
		PAD_BUTTON_B,
		ZKND_TBAR_ICON_MOVETYPE_PUSH,
	},
	//ZKND_TBAR_ICON_CUR_D,	//↓ボタン
	{	
		APP_COMMON_BARICON_CURSOR_DOWN,
		APP_COMMON_BARICON_CURSOR_DOWN_OFF,
		APP_COMMON_BARICON_CURSOR_DOWN_ON,
		ZKND_TBAR_SE_DECIDE,
		PAD_KEY_DOWN,
		ZKND_TBAR_ICON_MOVETYPE_PUSH,
	},
	//ZKND_TBAR_ICON_CUR_U,	//↑ボタン
	{	
		APP_COMMON_BARICON_CURSOR_UP,
		APP_COMMON_BARICON_CURSOR_UP_OFF,
		APP_COMMON_BARICON_CURSOR_UP_ON,
		ZKND_TBAR_SE_DECIDE,
		PAD_KEY_UP,
		ZKND_TBAR_ICON_MOVETYPE_PUSH,
	},
	//ZKND_TBAR_ICON_CUR_L,	//←ボタン
	{	
		APP_COMMON_BARICON_CURSOR_LEFT,
		APP_COMMON_BARICON_CURSOR_LEFT_OFF,
		APP_COMMON_BARICON_CURSOR_LEFT_ON,
		ZKND_TBAR_SE_DECIDE,
		PAD_KEY_LEFT,
		ZKND_TBAR_ICON_MOVETYPE_PUSH,
	},
	//ZKND_TBAR_ICON_CUR_R,	//→ボタ
	{	
		APP_COMMON_BARICON_CURSOR_RIGHT,
		APP_COMMON_BARICON_CURSOR_RIGHT_OFF,
		APP_COMMON_BARICON_CURSOR_RIGHT_ON,
		ZKND_TBAR_SE_DECIDE,
		PAD_KEY_RIGHT,
		ZKND_TBAR_ICON_MOVETYPE_PUSH,
	},
	//ZKND_TBAR_ICON_CHECK,	//ちぇっくボタン
	{	
		APP_COMMON_BARICON_CHECK_OFF,
		APP_COMMON_BARICON_CHECK_NONE,
		APP_COMMON_BARICON_CHECK_ON,
		ZKND_TBAR_SE_Y_REG,
		PAD_BUTTON_Y,
		ZKND_TBAR_ICON_MOVETYPE_FLIP,
	},
};


//=============================================================================
/**
 *					GLOBAL
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	初期化拡張版（アイコン位置指定版）
 *
 *	@param	const ZKND_TBAR_SETUP *cp_setup	設定情報
 *	@param	heapID	ヒープID
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
extern ZKND_TBAR_WORK * ZKND_TBAR_Init( ZKND_TBAR_SETUP *p_setup, HEAPID heapID )
{	
	ZKND_TBAR_WORK	*p_wk;


	//ワーク作成、初期化
	{	
		u32 size;

		size	= sizeof(ZKND_TBAR_WORK) + sizeof(ICON_WORK) * p_setup->item_num;
		p_wk	= GFL_HEAP_AllocMemory( heapID, size );
		GFL_STD_MemClear( p_wk, size );

		p_wk->tbl_len	= p_setup->item_num;
		p_wk->seq			= ZKND_TBAR_SEQ_MAIN;
    
    p_wk->whole_is_active = TRUE;   // ZKND_Modified 全体専用の設定を追加。
    p_wk->whole_is_lock   = FALSE;  // ZKND_Modified 全体専用の設定を追加。
	}

	//リソース読みこみ
	{	
		CLSYS_DRAW_TYPE			clsys_draw_type;
		CLSYS_DEFREND_TYPE	clsys_def_type;
		PALTYPE							bgplttype;

		//OBJ読み込む場所をチェック
		if( p_setup->bar_frm >= GFL_BG_FRAME0_S )
		{	
			clsys_draw_type	= CLSYS_DRAW_SUB;
			clsys_def_type	= CLSYS_DEFREND_SUB;
			bgplttype				= PALTYPE_SUB_BG;
		}
		else
		{	
			clsys_draw_type	= CLSYS_DRAW_MAIN;
			clsys_def_type	= CLSYS_DEFREND_MAIN;
			bgplttype				= PALTYPE_MAIN_BG;
		}

		//リソース読み込み
		RES_Init( &p_wk->res, p_setup->bar_frm, clsys_draw_type, bgplttype, 
				p_setup->mapping, p_setup->bg_plt, p_setup->obj_plt, p_setup->is_notload_bg, heapID );
	
		//ICON設定
		{	
			int i;
			for( i = 0; i < p_wk->tbl_len; i++ )
			{	
				ICON_Init( &p_wk->icon[i], p_setup->p_unit, &p_wk->res, &p_setup->p_item[i], clsys_draw_type, heapID );
			}
		}
	}

	return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ZKND_TBAR	破棄
 *
 *	@param	ZKND_TBAR_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
void ZKND_TBAR_Exit( ZKND_TBAR_WORK *p_wk )
{	
	//ICON破棄
	{	
		int i;
		for( i = 0; i < p_wk->tbl_len; i++ )
		{	
			ICON_Exit( &p_wk->icon[i] );
		}
	}

	//リソース破棄
	RES_Exit( &p_wk->res );
	
	//破棄
	GFL_HEAP_FreeMemory( p_wk );

}
//----------------------------------------------------------------------------
/**
 *	@brief	ZKND_TBAR	メイン処理
 *
 *	@param	ZKND_TBAR_WORK *p_wk			ワーク
 *	@param	CURSOR_WORK	cp_cursor	カーソルでボタンを押すのを実現させるため
 *
 */
//-----------------------------------------------------------------------------
void ZKND_TBAR_Main( ZKND_TBAR_WORK *p_wk )
{	
	switch( p_wk->seq )
	{	
	case ZKND_TBAR_SEQ_MAIN:
		//タッチ検出
		{	
			int i;
			p_wk->trg		= ZKND_TBAR_SELECT_NONE;

      if( (p_wk->whole_is_active) && (!p_wk->whole_is_lock) )  // ZKND_Modified 全体専用の設定を追加。 
      {
			  for( i = 0; i < p_wk->tbl_len; i++ )
			  {
				  if( ICON_Main( &p_wk->icon[ i ] ) )
				  {	
					  p_wk->trg	= i;
					  p_wk->seq	= ZKND_TBAR_SEQ_ANM_TRG;
            p_wk->whole_is_lock = TRUE;  // ZKND_Modified 全体専用の設定を追加。
					  break;
				  }
			  }
      }
		}
		break;

	case ZKND_TBAR_SEQ_ANM_TRG:	//アニメ前の１Fを得るため
			p_wk->seq	 = ZKND_TBAR_SEQ_ANM;
		/* fallthrough */
	
	case ZKND_TBAR_SEQ_ANM:
		//動作
		if( ICON_IsMoveEnd( &p_wk->icon[ p_wk->trg ] ) )
		{
			p_wk->seq	 = ZKND_TBAR_SEQ_TRG;
		}
		break;

	case ZKND_TBAR_SEQ_TRG:
		//トリガを1フレームだけ伝える
		p_wk->seq	 = ZKND_TBAR_SEQ_MAIN;
		break;
	}
	
	}
//----------------------------------------------------------------------------
/**
 *	@brief	ZKND_TBAR	選択されたものを取得	アニメ後の１F
 *
 *	@param	const ZKND_TBAR_WORK *cp_wk ワーク
 *
 *	@return	ZKND_TBAR_SELECT列挙
 */
//-----------------------------------------------------------------------------
ZKND_TBAR_ICON ZKND_TBAR_GetTrg( const ZKND_TBAR_WORK *cp_wk )
{	

	if( cp_wk->seq	== ZKND_TBAR_SEQ_TRG && cp_wk->trg != ZKND_TBAR_SELECT_NONE )
	{	
		return ICON_GetType( &cp_wk->icon[ cp_wk->trg ] );
	}

	return ZKND_TBAR_SELECT_NONE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ZKND_TBAR	選択されたものを取得	アニメ前の１F
 *
 *	@param	const ZKND_TBAR_WORK *cp_wk ワーク
 *
 *	@return	ZKND_TBAR_SELECT列挙
 */
//-----------------------------------------------------------------------------
ZKND_TBAR_ICON ZKND_TBAR_GetTouch( const ZKND_TBAR_WORK *cp_wk )
{	
	if( cp_wk->seq	== ZKND_TBAR_SEQ_ANM_TRG && cp_wk->trg != ZKND_TBAR_SELECT_NONE )
	{	
		return ICON_GetType( &cp_wk->icon[ cp_wk->trg ] );
	}

	return ZKND_TBAR_SELECT_NONE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ZKND_TBAR	全てのアイコンを設定
 *
 *	@param	ZKND_TBAR_WORK *p_wk	ワーク
 *	@param	is_active						アクティブ
 */
//-----------------------------------------------------------------------------
void ZKND_TBAR_SetActiveAll( ZKND_TBAR_WORK *p_wk, BOOL is_active )
{	
	int i;
	for( i = 0; i < p_wk->tbl_len; i++ )
	{	
		ICON_SetActive( &p_wk->icon[ i ], is_active );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	ZKND_TBAR	全てのアイコンを表示設定
 *
 *	@param	ZKND_TBAR_WORK *p_wk	ワーク
 *	@param	is_active						表示
 */
//-----------------------------------------------------------------------------
void ZKND_TBAR_SetVisibleAll( ZKND_TBAR_WORK *p_wk, BOOL is_visible )
{	
	int i;
	for( i = 0; i < p_wk->tbl_len; i++ )
	{	
		ICON_SetVisible( &p_wk->icon[ i ], is_visible );
	}
}

// ZKND_Modified 全体専用の設定を追加。↓
//-------------------------------------
///	全体のアクティブを切り替える
// 見た目をアクティブ状態にしたまま、全体専用のアクティブフラグの切り替えを行う
//=====================================
void ZKND_TBAR_SetActiveWhole( ZKND_TBAR_WORK *p_wk, BOOL is_active )
{
  p_wk->whole_is_active = is_active;
}

//-------------------------------------
///	全体のロックを解除する
// 見た目をアクティブ状態にしたままかかっている全体専用のロックを解除する
// ２度押しがないように、１度押したら押せないようにロックをかけることにする
//=====================================
void ZKND_TBAR_UnlockWhole( ZKND_TBAR_WORK *p_wk )
{
  p_wk->whole_is_lock = FALSE;
}
// ZKND_Modified 全体専用の設定を追加。↑


//----------------------------------------------------------------------------
/**
 *	@brief	ZKND_TBAR	アイコンのアクティブ設定
 *
 *	@param	ZKND_TBAR_WORK *p_wk	ワーク
 *	@param	icon								設定するアイコン
 *	@param	is_active						TRUEでアクティブ	FALSEでノンアクティブ
 */
//-----------------------------------------------------------------------------
void ZKND_TBAR_SetActive( ZKND_TBAR_WORK *p_wk,  ZKND_TBAR_ICON icon, BOOL is_active )
{	
	ICON_WORK * p_icon;
		
	//指定アイコンを検索
	p_icon	= Zknd_Tbar_Search( p_wk, icon );

	//設定
	ICON_SetActive( p_icon, is_active );
}
//----------------------------------------------------------------------------
/**	
 *	@brief	ZKND_TBAR	アイコンのアクティブを取得
 *
 *	@param	const ZKND_TBAR_WORK *cp_wk	ワーク
 *	@param	icon												取得するアイコン
 *
 *	@return	TRUEでアクティブ	FALSEでノンアクティブ
 */
//-----------------------------------------------------------------------------
BOOL ZKND_TBAR_GetActive( const ZKND_TBAR_WORK *cp_wk, ZKND_TBAR_ICON icon )
{	
	const ICON_WORK * cp_icon;
		
	//指定アイコンを検索
	cp_icon	= Zknd_Tbar_SearchConst( cp_wk, icon );

	//取得
	return ICON_GetActive( cp_icon );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ZKND_TBAR	アイコンの表示設定
 *
 *	@param	ZKND_TBAR_WORK *p_wk	ワーク
 *	@param	icon								設定するアイコン
 *	@param	is_visible					TRUEで表示	FALSEで非表示
 */
//-----------------------------------------------------------------------------
void ZKND_TBAR_SetVisible( ZKND_TBAR_WORK *p_wk,  ZKND_TBAR_ICON icon, BOOL is_visible )
{	
	ICON_WORK * p_icon;
		
	//指定アイコンを検索
	p_icon	= Zknd_Tbar_Search( p_wk, icon );

	//設定
	ICON_SetVisible( p_icon, is_visible );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ZKND_TBAR	アイコンの表示取得
 *
 *	@param	const ZKND_TBAR_WORK *cp_wk	ワーク
 *	@param	icon												取得するアイコン
 *
 *	@return	TRUEで表示	FALSEで非表示
 */
//-----------------------------------------------------------------------------
BOOL ZKND_TBAR_GetVisible( const ZKND_TBAR_WORK *cp_wk, ZKND_TBAR_ICON icon )
{	
	const ICON_WORK * cp_icon;
		
	//指定アイコンを検索
	cp_icon	= Zknd_Tbar_SearchConst( cp_wk, icon );

	//取得
	return ICON_GetVisible( cp_icon );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ZKND_TBAR	アイコンにSEを設定
 *
 *	@param	ZKND_TBAR_WORK *p_wk	ワーク
 *	@param	icon								アイコン
 *	@param	se									SE
 */
//-----------------------------------------------------------------------------
void ZKND_TBAR_SetSE( ZKND_TBAR_WORK *p_wk, ZKND_TBAR_ICON icon, u32 se )
{	

	ICON_WORK * p_icon;
		
	//指定アイコンを検索
	p_icon	= Zknd_Tbar_Search( p_wk, icon );

	//設定
	ICON_SetSE( p_icon, se );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ZKND_TBAR	アイコンのSEを取得
 *
 *	@param	const ZKND_TBAR_WORK *cp_wk	ワーク
 *	@param	icon	アイコン
 *
 *	@return	SE
 */
//-----------------------------------------------------------------------------
u32 ZKND_TBAR_GetSE( const ZKND_TBAR_WORK *cp_wk, ZKND_TBAR_ICON icon )
{	

	const ICON_WORK * cp_icon;
		
	//指定アイコンを検索
	cp_icon	= Zknd_Tbar_SearchConst( cp_wk, icon );

	//取得
	return ICON_GetSE( cp_icon );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ZKND_TBAR	アイコンにキー入力を設定
 *
 *	@param	ZKND_TBAR_WORK *p_wk	ワーク
 *	@param	icon								アイコン
 *	@param	key									キー入力	PAD_KEY_UPやPAD_BUTTON_A等
 */
//-----------------------------------------------------------------------------
void ZKND_TBAR_SetUseKey( ZKND_TBAR_WORK *p_wk, ZKND_TBAR_ICON icon, u32 key )
{	

	ICON_WORK * p_icon;
		
	//指定アイコンを検索
	p_icon	= Zknd_Tbar_Search( p_wk, icon );

	//設定
	ICON_SetKey( p_icon, key );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ZKND_TBAR	アイコンからキー入力を取得
 *
 *	@param	const ZKND_TBAR_WORK *cp_wk	ワーク
 *	@param	icon												アイコン
 *
 *	@return	キー入力
 */
//-----------------------------------------------------------------------------
u32 ZKND_TBAR_GetUseKey( const ZKND_TBAR_WORK *cp_wk, ZKND_TBAR_ICON icon )
{	

	const ICON_WORK * cp_icon;
		
	//指定アイコンを検索
	cp_icon	= Zknd_Tbar_SearchConst( cp_wk, icon );

	//取得
	return ICON_GetKey( cp_icon );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ZKND_TBAR	フリップ式ボタンへの設定
 *
 *	@param	ZKND_TBAR_WORK *p_wk	ワーク
 *	@param	icon			アイコン
 *	@param	is_flip		TRUEでフリップON（チェックされている）FALSEでOFF
 */
//-----------------------------------------------------------------------------
void ZKND_TBAR_SetFlip( ZKND_TBAR_WORK *p_wk, ZKND_TBAR_ICON icon, BOOL is_flip )
{	
	ICON_WORK * p_icon;
		
	//指定アイコンを検索
	p_icon	= Zknd_Tbar_Search( p_wk, icon );

	//設定
	ICON_SetFlip( p_icon, is_flip );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ZKND_TBAR	フリップ式ボタンの取得
 *
 *	@param	ZKND_TBAR_WORK *p_wk	ワーク
 *	@param	icon			アイコン
 *	@retval	is_flip		TRUEでフリップON（チェックされている）FALSEでOFF
 */
//-----------------------------------------------------------------------------
BOOL ZKND_TBAR_GetFlip( const ZKND_TBAR_WORK *cp_wk, ZKND_TBAR_ICON icon )
{	
	const ICON_WORK * cp_icon;
		
	//指定アイコンを検索
	cp_icon	= Zknd_Tbar_SearchConst( cp_wk, icon );

	//取得
	return ICON_GetFlip( cp_icon );
}

// ZKND_Modified アイコン個別操作を追加。↓
//-------------------------------------
///	アイコンのGFL_CLWKを取得する
//=====================================
GFL_CLWK* ZKND_TBAR_GetClwk( ZKND_TBAR_WORK *p_wk, ZKND_TBAR_ICON icon )
{
	ICON_WORK* p_icon	= Zknd_Tbar_Search( p_wk, icon );
  return  p_icon->p_clwk;
}
// ZKND_Modified アイコン個別操作を追加。↑


//=============================================================================
/**
 *	ZKND_TBAR private
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	アイコンの種類からオブジェクトをSearch
 *
 *	@param	ZKND_TBAR_WORK *p_wk	ワーク
 *	@param	type								探すアイコンのタイプ
 *
 *	@return	見つかったICON
 */
//-----------------------------------------------------------------------------
static ICON_WORK * Zknd_Tbar_Search( ZKND_TBAR_WORK *p_wk, ZKND_TBAR_ICON type )
{	
	int i;
	for( i = 0; i < p_wk->tbl_len; i++ )
	{	
		if( ICON_GetType( &p_wk->icon[ i ] ) == type )
		{	
			return &p_wk->icon[ i ];
		}
	}

	GF_ASSERT_MSG( 0,"ZKND_TBAR:指定したアイコンが見つかりませんでした%d", type );
	return NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief	アイコンの種類からオブジェクトをSearch	CONST版
 *
 *	@param	ZKND_TBAR_WORK *p_wk	ワーク
 *	@param	type								探すアイコンのタイプ
 *
 *	@return	見つかったICON
 */
//-----------------------------------------------------------------------------
static const ICON_WORK * Zknd_Tbar_SearchConst( const ZKND_TBAR_WORK *cp_wk, ZKND_TBAR_ICON type )
{	
	return Zknd_Tbar_Search( (ZKND_TBAR_WORK*)cp_wk, type );
}
//=============================================================================
/**
 *			RES
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	リソース読みこみ
 *
 *	@param	ZKND_TBAR_WORK *p_wk	ワーク
 *	@param	frm								フレーム
 *	@param	clsys_draw_type		セル読み込みタイプ
 *	@param	PALTYPE bgplttype	パレット読み込み場所
 *	@param	mapping						セルマッピングモード
 *	@param	bg_plt						BGのパレット番号
 *	@patam	obj_plt						OBJのパレット番号
 *	@param	is_notload_bg			TRUでBG読み込まないフラグ
 *	@param	heapID						読み込みテンポラリ用ヒープID
 */
//-----------------------------------------------------------------------------
static void RES_Init( RES_WORK *p_wk, u8 frm, CLSYS_DRAW_TYPE	clsys_draw_type, PALTYPE bgplttype, APP_COMMON_MAPPING mapping, u8 bg_plt, u8 obj_plt, BOOL is_notload_bg, HEAPID heapID )
{	
	//クリアー
	GFL_STD_MemClear( p_wk, sizeof(RES_WORK) );
	p_wk->bg_frm	= frm;

	{	
		ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), heapID );

		//BG
		if( !is_notload_bg )
		{	
			//領域の確保
			GFL_ARCHDL_UTIL_TransVramPalette( p_handle, APP_COMMON_GetBarPltArcIdx(),
					bgplttype, bg_plt*0x20, ZKND_TBAR_BG_PLT_NUM*0x20, heapID );
			//確保した領域に読み込み
			p_wk->chr_pos	= GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, APP_COMMON_GetBarCharArcIdx(), p_wk->bg_frm, ZKND_TBAR_BG_CHARAAREA_SIZE, FALSE, heapID );
			GF_ASSERT_MSG( p_wk->chr_pos != GFL_ARCUTIL_TRANSINFO_FAIL, "ZKND_TBAR:BGキャラ領域が足りませんでした\n" );
			//スクリーンはメモリ上に置いて、下部32*3だけ書き込み
			RES_UTIL_TransVramScreenEx( p_handle, APP_COMMON_GetBarScrnArcIdx(), p_wk->bg_frm,
					GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->chr_pos), ZKND_TBAR_MENUBAR_X, ZKND_TBAR_MENUBAR_Y, 32, 24, 
					ZKND_TBAR_MENUBAR_X, ZKND_TBAR_MENUBAR_Y, ZKND_TBAR_MENUBAR_W, ZKND_TBAR_MENUBAR_H,
					bg_plt, FALSE, heapID );
		}


		//OBJ
		//共通アイコンリソース	
		p_wk->res[ZKND_TBAR_RES_COMMON_PLT]	= GFL_CLGRP_PLTT_RegisterEx( p_handle,
				APP_COMMON_GetBarIconPltArcIdx(), clsys_draw_type, obj_plt*0x20, 0, ZKND_TBAR_OBJ_PLT_NUM, heapID );	
		p_wk->res[ZKND_TBAR_RES_COMMON_CHR]	= GFL_CLGRP_CGR_Register( p_handle,
				APP_COMMON_GetBarIconCharArcIdx(), FALSE, clsys_draw_type, heapID );

		p_wk->res[ZKND_TBAR_RES_COMMON_CEL]	= GFL_CLGRP_CELLANIM_Register( p_handle,
				APP_COMMON_GetBarIconCellArcIdx(mapping),
				APP_COMMON_GetBarIconAnimeArcIdx(mapping), heapID );

		GFL_ARC_CloseDataHandle( p_handle );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	リソース破棄
 *
 *	@param	RES_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RES_Exit( RES_WORK *p_wk )
{	
	//OBJ
	GFL_CLGRP_CELLANIM_Release( p_wk->res[ZKND_TBAR_RES_COMMON_CEL] );
	GFL_CLGRP_CGR_Release( p_wk->res[ZKND_TBAR_RES_COMMON_CHR] );
	GFL_CLGRP_PLTT_Release( p_wk->res[ZKND_TBAR_RES_COMMON_PLT] );

	//BG
	GFL_BG_FreeCharacterArea(p_wk->bg_frm,
			GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->chr_pos),
			GFL_ARCUTIL_TRANSINFO_GetSize(p_wk->chr_pos));


	//クリアー
	GFL_STD_MemClear( p_wk, sizeof(RES_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	リソース取得
 *
 *	@param	const RES_WORK *cp_wk	ワーク
 *	@param	type									タイプ
 *
 *	@return	リソース取得
 */
//-----------------------------------------------------------------------------
static u32 RES_GetResource( const RES_WORK *cp_wk, RES_TYPE type )
{	
	return cp_wk->res[ type ];
}
//----------------------------------------------------------------------------
/**
 *	@brief	Screenデータの　VRAM転送拡張版（読み込んだスクリーンの一部範囲をバッファに張りつける）
 *
 *	@param	ARCHANDLE *handle	ハンドル
 *	@param	datID							データID
 *	@param	frm								フレーム
 *	@param	chr_ofs						キャラオフセット
 *	@param	src_x							転送元ｘ座標
 *	@param	src_y							転送元Y座標
 *	@param	src_w							転送元幅			//データの全体の大きさ
 *	@param	src_h							転送元高さ		//データの全体の大きさ
 *	@param	dst_x							転送先X座標
 *	@param	dst_y							転送先Y座標
 *	@param	dst_w							転送先幅
 *	@param	dst_h							転送先高さ
 *	@param	plt								パレット番号
 *	@param	compressedFlag		圧縮フラグ
 *	@param	heapID						一時バッファ用ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void RES_UTIL_TransVramScreenEx( ARCHANDLE *handle, ARCDATID datID, u32 frm, u32 chr_ofs, u8 src_x, u8 src_y, u8 src_w, u8 src_h, u8 dst_x, u8 dst_y, u8 dst_w, u8 dst_h, u8 plt, BOOL compressedFlag, HEAPID heapID )
{	
	void *p_src_arc;
	NNSG2dScreenData* p_scr_data;

	//読み込み
	p_src_arc = GFL_ARCHDL_UTIL_Load( handle, datID, compressedFlag, GetHeapLowID(heapID) );

	//アンパック
	if(!NNS_G2dGetUnpackedScreenData( p_src_arc, &p_scr_data ) )
	{	
		GF_ASSERT(0);	//スクリーンデータじゃないよ
	}

	//エラー
	GF_ASSERT( src_w * src_h * 2 <= p_scr_data->szByte );

	//キャラオフセット計算
	if( chr_ofs )
	{	
		int i;
		if( GFL_BG_GetScreenColorMode( frm ) == GX_BG_COLORMODE_16 )
		{
			u16 *p_scr16;
			
			p_scr16	=	(u16 *)&p_scr_data->rawData;
			for( i = 0; i < src_w * src_h ; i++ )
			{
				p_scr16[i]	+= chr_ofs;
			}	
		}
		else
		{	
			GF_ASSERT(0);	//256版は作ってない
		}
	}

	//書き込み
	if( GFL_BG_GetScreenBufferAdrs( frm ) != NULL )
	{	
		GFL_BG_WriteScreenExpand( frm, dst_x, dst_y, dst_w, dst_h,
				&p_scr_data->rawData, src_x, src_y, src_w, src_h );	
		GFL_BG_ChangeScreenPalette( frm, dst_x, dst_y, dst_w, dst_h, plt );
		GFL_BG_LoadScreenReq( frm );
	}
	else
	{	
		GF_ASSERT(0);	//バッファがない
	}

	GFL_HEAP_FreeMemory( p_src_arc );
}	

//=============================================================================
/**
 *			ICON
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ICON初期化
 *
 *	@param	ICON_WORK *p_wk								ワーク
 *	@param	p_unit												CLWK作成用UNIT
 *	@param	RES_WORK											共通リソース
 *	@param	ZKND_TBAR_ITEM_ICON *cp_setup	セットアップデータ
 *	@param	HEAPID heapID									ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void ICON_Init( ICON_WORK *p_wk, GFL_CLUNIT* p_unit, const RES_WORK *cp_res, const ZKND_TBAR_ITEM_ICON *cp_setup, CLSYS_DEFREND_TYPE	clsys_def_type, HEAPID heapID )
{	
	//クリア
	GFL_STD_MemClear( p_wk ,sizeof(ICON_WORK) );
	p_wk->data	= *cp_setup;
	p_wk->is_active	= TRUE;
	
	//CLWK作成
	{	
		u32 cg, plt, cell;
		GFL_CLWK_DATA				cldata;
		GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
		cldata.pos_x	= cp_setup->pos.x;
		cldata.pos_y	= cp_setup->pos.y;

		//タイプによってリソースやデータが違う
		if( cp_setup->icon >= ZKND_TBAR_ICON_CUSTOM )  // ZKND_Modified カスタムの個数制限なしにした。
		{	
			//カスタムアイコン
			cg		= cp_setup->cg_idx;
			plt		= cp_setup->plt_idx;
			cell	= cp_setup->cell_idx;
			p_wk->movetype	= ZKND_TBAR_ICON_MOVETYPE_PUSH;
		}
		else
		{	
			//共通アイコン
			cg		= RES_GetResource( cp_res, ZKND_TBAR_RES_COMMON_CHR );
			plt		= RES_GetResource( cp_res, ZKND_TBAR_RES_COMMON_PLT );
			cell	= RES_GetResource( cp_res, ZKND_TBAR_RES_COMMON_CEL );

			p_wk->data.active_anmseq		= sc_common_icon_data[ cp_setup->icon ].active_anmseq;
			p_wk->data.noactive_anmseq	= sc_common_icon_data[ cp_setup->icon ].noactive_anmseq;
			p_wk->data.push_anmseq			= sc_common_icon_data[ cp_setup->icon ].push_anmseq;
			p_wk->data.key							= sc_common_icon_data[ cp_setup->icon ].key;
			p_wk->data.se								= sc_common_icon_data[ cp_setup->icon ].se;
			p_wk->movetype							= sc_common_icon_data[ cp_setup->icon ].movetype;
		}
		p_wk->now_anmseq	= p_wk->data.active_anmseq;
		cldata.anmseq	= p_wk->now_anmseq;

		//作成
		p_wk->p_clwk	= GFL_CLACT_WK_Create( p_unit, cg, plt, cell, &cldata, clsys_def_type, heapID );
		GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk, TRUE );

	}

		//動作タイプによって実行関数切り替え
	{	
		switch( p_wk->movetype )
		{	
		case ZKND_TBAR_ICON_MOVETYPE_PUSH:	//押されたら、押されるアニメになるボタン
			p_wk->push_func	= Icon_PushFuncNormal;
			break;
		case ZKND_TBAR_ICON_MOVETYPE_FLIP:	//押されたら、ONとOFFが切り替るボタン
			p_wk->push_func	= Icon_PushFuncFlip;
			break;
		default:
			GF_ASSERT_MSG( 0, "ZKND_TBAR:動作タイプが不適切です%d", p_wk->movetype );
		}

	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	ICON破棄
 *
 *	@param	ICON_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void ICON_Exit( ICON_WORK *p_wk )
{	
	//CLWK破棄
	GFL_CLACT_WK_Remove( p_wk->p_clwk );
	//クリア
	GFL_STD_MemClear( p_wk ,sizeof(ICON_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ICON	メイン処理
 *
 *	@param	ICON_WORK *p_wk ワーク
 *
 *	@return	TRUE入力ON　FALSE入力OFF
 */
//-----------------------------------------------------------------------------
static BOOL ICON_Main( ICON_WORK *p_wk )
{	
				
	BOOL	is_visible;
	BOOL	is_update;
	u32 x, y;

	is_update		= FALSE;
	is_visible	= GFL_CLACT_WK_GetDrawEnable( p_wk->p_clwk );

	//判定可能なときのみ
	if( p_wk->is_active & is_visible )
	{	
		//タッチ判定
		if( GFL_UI_TP_GetPointTrg( &x, &y ) )
		{	
			if( ((u32)( x - p_wk->data.pos.x) <= (u32)(p_wk->data.width))  // ZKND_Modified 幅を指定できるようにした。
					&	((u32)( y - p_wk->data.pos.y) <= (u32)(ZKND_TBAR_ICON_HEIGHT)))
			{
				GFL_UI_SetTouchOrKey(GFL_APP_KTST_TOUCH);
				is_update	= TRUE;
			}
		}

		//キー判定
		if( p_wk->data.key != 0 )
		{	
			if( GFL_UI_KEY_GetTrg() & p_wk->data.key )
			{	
				GFL_UI_SetTouchOrKey(GFL_APP_KTST_KEY);
				is_update	= TRUE;
			}
		}

		//動作関数
		if( is_update )
		{	
			p_wk->push_func( p_wk );
			return TRUE;
		}
	}

	return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ICON	アニメの動作をチェック
 *
 *	@param	const ICON_WORK *cp_wk	ワーク
 *
 *	@return	TRUEで動作していない	FALSEで動作中
 */
//-----------------------------------------------------------------------------
static BOOL ICON_IsMoveEnd( const ICON_WORK *cp_wk )
{	
	return !GFL_CLACT_WK_CheckAnmActive( cp_wk->p_clwk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ICON	アイコンの種類
 *
 *	@param	const ICON_WORK *cp_wk ワーク
 *
 *	@return	アイコンの種類
 */
//-----------------------------------------------------------------------------
static ZKND_TBAR_ICON ICON_GetType( const ICON_WORK *cp_wk )
{	
	return	cp_wk->data.icon;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ICON	表示設定
 *
 *	@param	ICON_WORK *p_wk	ワーク
 *	@param	is_visible			TRUEで表示	FALSEで非表示
 */
//-----------------------------------------------------------------------------
static void ICON_SetVisible( ICON_WORK *p_wk, BOOL is_visible )
{	
	GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, is_visible );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ICON	表示取得
 *
 *	@param	const ICON_WORK *cp_wk	ワーク
 *
 *	@return	TRUE表示	FALSE非表示
 */
//-----------------------------------------------------------------------------
static BOOL ICON_GetVisible( const ICON_WORK *cp_wk )
{	
	return GFL_CLACT_WK_GetDrawEnable( cp_wk->p_clwk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ICON	アクティブ状態設定
 *
 *	@param	ICON_WORK *p_wk		ワーク
 *	@param	is_active					TRUEでアクティブ状態	FALSEでノンアクティブ状態
 */
//-----------------------------------------------------------------------------
static void ICON_SetActive( ICON_WORK *p_wk, BOOL is_active )
{	
	p_wk->is_active	= is_active;

	if( is_active )
	{	
		//アクティブになった瞬間にアニメ開始にならないようにする
		if( p_wk->now_anmseq == p_wk->data.push_anmseq )
		{	
			p_wk->now_anmseq	= p_wk->data.active_anmseq;
		}	
		GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk, p_wk->now_anmseq );
	}
	else
	{	
		GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk, p_wk->data.noactive_anmseq );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	ICON	アクティブ判定取得
 *
 *	@param	const ICON_WORK *cp_wk	ワーク
 *
 *	@return	TRUEでアクティブ状態	FALSEでノンアクティブ状態
 */
//-----------------------------------------------------------------------------
static BOOL ICON_GetActive( const ICON_WORK *cp_wk )
{	
	return cp_wk->is_active;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ICON	SEを設定
 *
 *	@param	ICON_WORK *p_wk	ワーク
 *	@param	se							SE
 */
//-----------------------------------------------------------------------------
static void ICON_SetSE( ICON_WORK *p_wk, u32 se )
{	
	p_wk->data.se	= se;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ICON	SEを取得
 *
 *	@param	const ICON_WORK *cp_wk	ワーク
 *
 *	@return	SE
 */
//-----------------------------------------------------------------------------
static u32 ICON_GetSE( const ICON_WORK *cp_wk )
{	
	return cp_wk->data.se;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ICON	キー入力をセット
 *
 *	@param	ICON_WORK *p_wk	ワーク
 *	@param	key							キー入力	PAD_KEY_UP等
 */
//-----------------------------------------------------------------------------
static void ICON_SetKey( ICON_WORK *p_wk, u32 key )
{	
	p_wk->data.key	= key;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ICON	キー入力を取得
 *
 *	@param	const ICON_WORK *cp_wk	ワーク
 *
 *	@return	キー入力
 */
//-----------------------------------------------------------------------------
static u32 ICON_GetKey( const ICON_WORK *cp_wk )
{	
	return cp_wk->data.key;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ICON	フリップ設定
 *
 *	@param	ICON_WORK *p_wk	ワーク
 *	@param	is_flip					TRUEでON	FALSEでOFF
 *
 */
//-----------------------------------------------------------------------------
static void ICON_SetFlip( ICON_WORK *p_wk, BOOL is_flip )
{	
	if( is_flip )
	{	
		p_wk->now_anmseq	= p_wk->data.push_anmseq;
	}
	else
	{	
		p_wk->now_anmseq	= p_wk->data.active_anmseq;
	}	
	GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk, p_wk->now_anmseq );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ICON	フリップ取得
 *
 *	@param	const ICON_WORK *cp_wk ワーク
 *
 *	@return	TRUEでON	FALSEでOFF
 */
//-----------------------------------------------------------------------------
static BOOL ICON_GetFlip( const ICON_WORK *cp_wk )
{	
	return cp_wk->now_anmseq	== cp_wk->data.push_anmseq;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンが押された時の動作関数	通常版
 *
 *	@param	ICON_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Icon_PushFuncNormal( ICON_WORK *p_wk )
{	
	//アニメを変え、音を鳴らす
	p_wk->now_anmseq	= p_wk->data.push_anmseq;

	GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk, p_wk->now_anmseq );

	if( p_wk->data.se != 0 )
	{	
		PMSND_PlaySE( p_wk->data.se );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンが押された時の動作関数	フリップ版
 *
 *	@param	ICON_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Icon_PushFuncFlip( ICON_WORK *p_wk )
{	
	BOOL now_flip	= ICON_GetFlip( p_wk );

	ICON_SetFlip( p_wk, !now_flip );

	if( p_wk->data.se != 0 )
	{	
		PMSND_PlaySE( p_wk->data.se );
	}
}
