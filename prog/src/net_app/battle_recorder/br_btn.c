//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_btn.c
 *	@brief	バトルレコーダーボタン管理
 *	@author	Toru=Nagihashi
 *	@data		2009.11.10
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/gfl_use.h"
#include "system/main.h"  //HEAPID

//文字表示
#include "print/printsys.h"

//自分のモジュール
#include "br_btn_data.h"
#include "br_btn.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	ボタン移動タイプ
//=====================================
typedef enum
{
	BR_BTN_MOVE_HIDE,		//押されたボタンの格納する処理	target= need
	BR_BTN_MOVE_APPEAR,	//ボタン出現処理								target= NULL
	BR_BTN_MOVE_UP_TAG,		  //タグ位置へ登る                target= need
  BR_BTN_MOVE_DOWN_TAG,     //タグ位置からボタン位置へ下がるtarget= need
  BR_BTN_MOVE_NEXT_TARGET,  //ターゲットの座標へいく  
} BR_BTN_MOVE;
#define BR_BTN_MOVE_DEFAULT_SYNC  (16)
#define BR_BTN_MOVE_HIDE_SYNC     (BR_BTN_MOVE_DEFAULT_SYNC)
#define BR_BTN_MOVE_APPEAR_SYNC   (BR_BTN_MOVE_DEFAULT_SYNC)
#define BR_BTN_MOVE_UPTAG_M_SYNC   (BR_BTN_MOVE_DEFAULT_SYNC/2)
#define BR_BTN_MOVE_UPTAG_S_SYNC   (BR_BTN_MOVE_DEFAULT_SYNC/2)


//-------------------------------------
///	ボタン公開情報
//=====================================
typedef enum
{
	BR_BTN_PARAM_TYPE,				//ボタンのタイプ	（BR_BTN_TYPE）
	BR_BTN_PARAM_DATA1,				//タイプによったデータ
	BR_BTN_PARAM_DATA2,				//タイプによったデータ
	BR_BTN_PARAM_VALID,				//おせるかどうか
  BR_BTN_PARAM_MENUID,      //自分のメニューID
} BR_BTN_PARAM;

//-------------------------------------
///	
//=====================================
#define BR_BTN_OAMFONT_OFS_X	(4*8)
#define BR_BTN_OAMFONT_OFS_Y	(1*8)

//-------------------------------------
///	ボタン管理システム
//=====================================
//管理スタック数
#define BR_BTN_SYS_STACK_MAX	(3)
//上画面にボタンがいったときの座標
#define TAG_INIT_POS_UP( n )		( 25 + ( ( 36 ) * ( 4-n ) ) )

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	汎用ボタンワーク
//=====================================
struct _BR_BTN_WORK
{
	GFL_CLWK					*p_clwk;		//CLWK
	BMPOAM_ACT_PTR		p_oamfont;	//OAMフォント
	GFL_BMP_DATA			*p_bmp;			//フォント
	const BR_BTN_DATA *cp_data;		//ボタンのデータ
	CLSYS_DRAW_TYPE		display;		//どちらに描画するか
  u32               w;          //幅
};

//-------------------------------------
///	ボタンシステムで管理する専用のボタンワーク
//=====================================
typedef struct _BR_BTNEX_WORK BR_BTNEX_WORK;
typedef BOOL (*BR_BTNEX_MOVEFUNCTION)( BR_BTNEX_WORK *p_wk );
struct _BR_BTNEX_WORK
{
	BOOL							is_use;			//使用フラグ
  CLSYS_DRAW_TYPE   display;    //どちらの画面か
	BR_BTN_WORK				*p_btn;			//ボタンワーク
	const BR_BTN_DATA *cp_data;		//ボタンのデータ
  BR_BTNEX_MOVEFUNCTION move_function; //移動関数
  GFL_POINT           target; //移動のターゲット
  u32                 move_seq;   //移動のシーケンス
  s16                 start;
  s16                 end;
  s16                 sync_now;
  s16                 sync_max;

  //画面切り替え用に作りなおす際必要なメンバ
  const BR_RES_WORK   *cp_res;
  HEAPID              heapID;
  GFL_CLUNIT          *p_unit;
  BMPOAM_SYS_PTR		  p_bmpoam;
};

//-------------------------------------
///	シーケンス管理
//=====================================
typedef struct _SEQ_WORK SEQ_WORK;	//関数型作るため前方宣言
typedef void (*SEQ_FUNCTION)( SEQ_WORK *p_wk, int *p_seq, void *p_wk_adrs );
struct _SEQ_WORK
{
	SEQ_FUNCTION	seq_function;		//実行中のシーケンス関数
	BOOL is_end;									//シーケンスシステム終了フラグ
	int seq;											//実行中のシーケンス関数の中のシーケンス
	void *p_wk_adrs;							//実行中のシーケンス関数に渡すワーク
};

//-------------------------------------
///	バトルレコーダー　ボタン管理
//=====================================
struct _BR_BTN_SYS_WORK
{
	HEAPID						heapID;
	BR_RES_WORK				*p_res;		//リソース
	GFL_CLUNIT				*p_unit;	//セルユニット
  SEQ_WORK          seq;

	BR_BTN_SYS_STATE	state;	//ボタン管理の状態
	BR_BTN_SYS_INPUT	input;	//ボタン入力状態
	BR_BTN_TYPE				next_type;//次の動作
	BOOL							next_valid;//次のボタンが押せるかどうか
	u32								next_proc;//次のプロセス
	u32								next_mode;//次のプロセスモード

	u32								trg_btn;			//押したボタン

	u8								btn_now_max;	//現在のボタンバッファ最大
	u8								btn_now_num;	//現在のボタン数
	u8								btn_stack_max;//ボタンスタックバッファ最大
	u8								btn_stack_num;//ボタンスタック現在数
	BR_BTNEX_WORK			*p_btn_stack;	//スタックに積むボタンバッファ
	BR_BTNEX_WORK			*p_btn_now;		//現在のボタンバッファ

	BR_BTN_DATA_SYS		*p_btn_data;	//ボタンデータバッファ
	BMPOAM_SYS_PTR		p_bmpoam;	//BMPOAMシステム
};
//=============================================================================
/**
 *					PRIVATE関数
*/
//=============================================================================
//-------------------------------------
///	ボタンシステムのプライベート
//=====================================
static void Br_Btn_Sys_PushStack( BR_BTN_SYS_WORK *p_wk, BR_BTNEX_WORK *p_btn, CLSYS_DRAW_TYPE display );
static BOOL Br_Btn_Sys_PopStack( BR_BTN_SYS_WORK *p_wk, BR_BTNEX_WORK *p_btn );
static void Br_Btn_Sys_ReLoadBtn( BR_BTN_SYS_WORK *p_wk, BR_MENUID menuID, const GFL_POINT *cp_init_pos );

//-------------------------------------
///	SEQ
//=====================================
static void SEQ_Init( SEQ_WORK *p_wk, void *p_wk_adrs, SEQ_FUNCTION seq_function );
static void SEQ_Exit( SEQ_WORK *p_wk );
static void SEQ_Main( SEQ_WORK *p_wk );
static BOOL SEQ_IsEnd( const SEQ_WORK *cp_wk );
static void SEQ_SetNext( SEQ_WORK *p_wk, SEQ_FUNCTION seq_function );
static void SEQ_End( SEQ_WORK *p_wk );

//-------------------------------------
///	SEQFUNC
//    次のボタンへいくとき
//      Touch -> HideBtn -> ChangePage -> AppearBtn -> UpTag
//
//    戻るとき
//      Touch -> DownTag -> HideBtn2 -> ChangePage -> AppearBtn 
//
//    次のPROCへいくとき
//      Touch -> HideBtn -> ChangePage -> UpTag
//
//=====================================
static void SEQFUNC_Start( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_Touch( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_HideBtn( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_HideBtn2( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_ChangePage( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_AppearBtn( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_UpTag( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_DownTag( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_Input( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
///	BTNの処理
//=====================================
static void BR_BTNEX_Init( BR_BTNEX_WORK *p_wk, const BR_BTN_DATA *cp_data, GFL_CLUNIT *p_unit, BMPOAM_SYS_PTR p_bmpoam, const BR_RES_WORK *cp_res, const STRBUF *cp_strbuf, HEAPID heapID );
static void BR_BTNEX_Exit( BR_BTNEX_WORK *p_wk );
static BOOL BR_BTNEX_GetTrg( const BR_BTNEX_WORK *cp_wk, u32 x, u32 y );
static void BR_BTNEX_StartMove( BR_BTNEX_WORK *p_wk, BR_BTN_MOVE move, const GFL_POINT *cp_target );
static BOOL BR_BTNEX_MainMove( BR_BTNEX_WORK *p_wk );
static void BR_BTNEX_ChangeDisplay( BR_BTNEX_WORK *p_wk, CLSYS_DRAW_TYPE display );
static void BR_BTNEX_Copy( const BR_BTN_SYS_WORK *cp_wk, BR_BTNEX_WORK *p_dst, const BR_BTNEX_WORK *cp_src, CLSYS_DRAW_TYPE	display );
static void BR_BTNEX_SetStackPos( BR_BTNEX_WORK *p_wk, u16 stack_num );
static void BR_BTNEX_GetPos( const BR_BTNEX_WORK *cp_wk, s16 *p_x, s16 *p_y );
static u32 BR_BTNEX_GetParam( const BR_BTNEX_WORK *cp_wk, BR_BTN_PARAM param );
static void BR_BTNEX_SetSoftPriority( BR_BTNEX_WORK *p_wk, u16 soft_pri );

//typedef BOOL (*BR_BTNEX_MOVEFUNCTION)( BR_BTNEX_WORK *p_wk );
static BOOL Br_BtnEx_Move_Hide( BR_BTNEX_WORK *p_wk );
static BOOL Br_BtnEx_Move_Appear( BR_BTNEX_WORK *p_wk );
static BOOL Br_BtnEx_Move_UpTag( BR_BTNEX_WORK *p_wk );
static BOOL Br_BtnEx_Move_DownTag( BR_BTNEX_WORK *p_wk );
static BOOL Br_BtnEx_Move_NextTarget( BR_BTNEX_WORK *p_wk );

//=============================================================================
/**
 *					データ
*/
//=============================================================================
//-------------------------------------
///	タグの位置
//=====================================
static const GFL_POINT  sc_tag_pos[]  =
{ 
  //一番下
  { 
    BR_BTN_POS_X, TAG_INIT_POS_UP(0)
  },
  { 
    BR_BTN_POS_X, TAG_INIT_POS_UP(1)
  },
  { 
    BR_BTN_POS_X, TAG_INIT_POS_UP(2)
  },
  { 
    BR_BTN_POS_X, TAG_INIT_POS_UP(3)
  },
};


//=============================================================================
/**
 *					PUBILIC関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	バトルレコーダー	ボタン管理初期化
 *
 *  @param	mode						起動モード
 *  @param	GFL_CLUNIT			ユニット
 *  @param	BR_RES_WORK			リソース
 *  @param	BR_BTLREC_SET		バトル録画情報
 *	@param	HEAPID heapID		ヒープID
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
BR_BTN_SYS_WORK *BR_BTN_SYS_Init( BR_MENUID menuID, GFL_CLUNIT *p_unit, BR_RES_WORK *p_res, const BR_BTN_DATA_SETUP *cp_setup, HEAPID heapID )
{	
	BR_BTN_SYS_WORK *p_wk;

	GF_ASSERT( menuID < BR_MENUID_MAX );

	//ワーク作成
	{	
		p_wk	= GFL_HEAP_AllocMemory( heapID, sizeof(BR_BTN_SYS_WORK) );
		GFL_STD_MemClear( p_wk, sizeof(BR_BTN_SYS_WORK) );
		p_wk->p_res				= p_res;
		p_wk->p_unit			= p_unit;
		p_wk->heapID			= heapID;
		p_wk->p_bmpoam		= BmpOam_Init( heapID, p_unit);
		p_wk->p_btn_data	= BR_BTN_DATA_SYS_Init( cp_setup, heapID );
	}

	//リソース読み込み
	{	
		BR_RES_LoadOBJ( p_res, BR_RES_OBJ_BROWSE_BTN_M, heapID );
		BR_RES_LoadOBJ( p_res, BR_RES_OBJ_BROWSE_BTN_S, heapID );
	}

	//ボタンバッファを作成
	{	
		u32 size;
		
		p_wk->btn_now_max	= BR_BTN_DATA_SYS_GetDataMax(p_wk->p_btn_data);
		size = sizeof(BR_BTNEX_WORK) * p_wk->btn_now_max;
		p_wk->p_btn_now	= GFL_HEAP_AllocMemory( heapID, size );
		GFL_STD_MemClear( p_wk->p_btn_now, size );
	}

	//スタックバッファを作成
	{	
		u32 size;

		p_wk->btn_stack_max	= BR_BTN_SYS_STACK_MAX;
		size	= sizeof(BR_BTNEX_WORK) * BR_BTN_SYS_STACK_MAX;
		p_wk->p_btn_stack	= GFL_HEAP_AllocMemory( heapID, size );
		GFL_STD_MemClear( p_wk->p_btn_stack, size );
	}

	//最初に読み込むボタンを初期化
	{	
		//読み込むボタンのリンク情報を検知し、途中のものであればスタックに積む
		{	
			u32 num	= BR_BTN_DATA_SYS_GetLinkMax( p_wk->p_btn_data, menuID );

			if( num != 0 )
			{	
				int i;
				const BR_BTN_DATA *	cp_data;
				BR_MENUID	preID;
				u32 btnID;
				BR_BTNEX_WORK	btn;
        STRBUF  *p_strbuf;
        GFL_MSGDATA *p_msg;

        p_msg		= BR_RES_GetMsgData( p_res );
				for( i = 0; i < num; i++ )
				{	
          GFL_STD_MemClear( &btn, sizeof(BR_BTNEX_WORK) );
					BR_BTN_DATA_SYS_GetLink( p_wk->p_btn_data, menuID, i, &preID, &btnID );

					cp_data	= BR_BTN_DATA_SYS_GetData( p_wk->p_btn_data, preID, btnID );
          p_strbuf  = BR_BTN_DATA_CreateString( p_wk->p_btn_data, cp_data, p_msg, GFL_HEAP_LOWID(heapID) );
					BR_BTNEX_Init( &btn, cp_data, p_unit, p_wk->p_bmpoam, p_res, p_strbuf, heapID );
					Br_Btn_Sys_PushStack( p_wk, &btn, CLSYS_DRAW_MAIN );

          GFL_STR_DeleteBuffer( p_strbuf );
				}
        //座標を設定
        for( i = 0; i < p_wk->btn_stack_num; i++ )
        {	
          BR_BTNEX_SetStackPos( &p_wk->p_btn_stack[p_wk->btn_stack_num - i - 1], i );
        }
      }
		}

		//読み込むもの
		{	
			int i;
      STRBUF  *p_strbuf;
			const BR_BTN_DATA *	cp_data;
      GFL_MSGDATA *p_msg;

      p_msg		= BR_RES_GetMsgData( p_res );
			p_wk->btn_now_num	= BR_BTN_DATA_SYS_GetDataNum( p_wk->p_btn_data, menuID );

			for( i = 0; i < p_wk->btn_now_num; i++ )
			{	
				cp_data	= BR_BTN_DATA_SYS_GetData( p_wk->p_btn_data, menuID, i );
        p_strbuf  = BR_BTN_DATA_CreateString( p_wk->p_btn_data, cp_data, p_msg, GFL_HEAP_LOWID(heapID) );

				BR_BTNEX_Init( &p_wk->p_btn_now[i], cp_data, p_unit, p_wk->p_bmpoam, p_res, p_strbuf, heapID );
        GFL_STR_DeleteBuffer( p_strbuf );
			}
		}
	}

  //モジュール作成
	SEQ_Init( &p_wk->seq, p_wk, SEQFUNC_Start );

	return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルレコーダー	ボタン管理破棄
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
void BR_BTN_SYS_Exit( BR_BTN_SYS_WORK *p_wk )
{	
  //モジュール破棄
  SEQ_Exit( &p_wk->seq );

	//スタックバッファ破棄
	{	
		int i;
		for( i = 0; i < p_wk->btn_stack_num; i++ )
		{	
			BR_BTNEX_Exit( &p_wk->p_btn_stack[i] );
		}
	}
	GFL_HEAP_FreeMemory( p_wk->p_btn_stack );

	//ボタンバッファ破棄
	{	
		int i;
		for( i = 0; i < p_wk->btn_now_num; i++ )
		{	
			BR_BTNEX_Exit( &p_wk->p_btn_now[i] );
		}
	}
	GFL_HEAP_FreeMemory( p_wk->p_btn_now );
	
	//リソース破棄
	{	
		BR_RES_UnLoadOBJ( p_wk->p_res, BR_RES_OBJ_BROWSE_BTN_S );
		BR_RES_UnLoadOBJ( p_wk->p_res, BR_RES_OBJ_BROWSE_BTN_M );
	}

	BR_BTN_DATA_SYS_Exit( p_wk->p_btn_data );
	BmpOam_Exit( p_wk->p_bmpoam );

	//ワーク破棄
	GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルレコーダー	ボタン管理メイン処理
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
void BR_BTN_SYS_Main( BR_BTN_SYS_WORK *p_wk )
{
  //シーケンス
	SEQ_Main( &p_wk->seq );
}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルレコーダー	ボタン管理	入力状態取得
 *
 *	@param	cp_wk	ワーク
 *	@param	p_seq	BR_BTN_SYS_INPUT_CHANGESEQのとき、次のシーケンス
 *	@param	p_param	BR_BTN_SYS_INPUT_CHANGESEQのとき、次のシーケンスのモード
 *
 *	@return	入力の種類
 */
//-----------------------------------------------------------------------------
BR_BTN_SYS_INPUT BR_BTN_SYS_GetInput( const BR_BTN_SYS_WORK *cp_wk, u32 *p_seq, u32 *p_mode )
{	
	if( p_seq )
	{	
		*p_seq	= cp_wk->next_proc;
	}
	if( p_mode )
	{	
		*p_mode	= cp_wk->next_mode;
	}

	return cp_wk->input;
}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルレコーダー	ボタン管理	状態取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	現在の状態
 */
//-----------------------------------------------------------------------------
BR_BTN_SYS_STATE BR_BTN_SYS_GetState( const BR_BTN_SYS_WORK *cp_wk )
{	
	return cp_wk->state;
}
//=============================================================================
/**
 *		ボタン管理システムのプライベート
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ボタン管理システムにボタンを積む
 *
 *	@param	BR_BTN_SYS_WORK *p_wk		ワーク
 *	@param	*p_btn									スタックに積むボタン
 */
//-----------------------------------------------------------------------------
static void Br_Btn_Sys_PushStack( BR_BTN_SYS_WORK *p_wk, BR_BTNEX_WORK *p_btn, CLSYS_DRAW_TYPE display )
{	
	GF_ASSERT( p_wk->btn_stack_num - 1 < p_wk->btn_stack_max );

	//積む
	BR_BTNEX_Copy( p_wk, &p_wk->p_btn_stack[p_wk->btn_stack_num], p_btn, display );
	
	p_wk->btn_stack_num++;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタン管理システムからボタンを破棄する
 *
 *	@param	BR_BTN_SYS_WORK *p_wk	ワーク
 *	@param	*p_btn								スタックから受け取るボタン
 *
 *	@retval	TRUE破棄した　FALSE先頭だったので何もしなかった
 */
//-----------------------------------------------------------------------------
static BOOL Br_Btn_Sys_PopStack( BR_BTN_SYS_WORK *p_wk, BR_BTNEX_WORK *p_btn )
{	
	if( p_wk->btn_stack_num != 0 )
	{	
		p_wk->btn_stack_num--;
		*p_btn	= p_wk->p_btn_stack[p_wk->btn_stack_num];
		GFL_STD_MemClear( &p_wk->p_btn_stack[p_wk->btn_stack_num], sizeof(BR_BTNEX_WORK) );

		return TRUE;
	}

	return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンを破棄し、際読み込みする
 *
 *	@param	BR_BTN_SYS_WORK *p_wk ワーク
 *	@param	BR_MENUID							読み込むボタンのメニューID
 *	@param  cp_init_pos           NULLで初期値、  指定すると、すべてのボタンがその位置にセット
 */
//-----------------------------------------------------------------------------
static void Br_Btn_Sys_ReLoadBtn( BR_BTN_SYS_WORK *p_wk, BR_MENUID menuID, const GFL_POINT *cp_init_pos )
{	
	int i;
	for( i = 0; i < p_wk->btn_now_max; i++ )
	{	
		BR_BTNEX_Exit( &p_wk->p_btn_now[i] );
	}

	{	
		int i;
		const BR_BTN_DATA *	cp_data;
    STRBUF  *p_strbuf;
    GFL_MSGDATA *p_msg;

    p_msg		= BR_RES_GetMsgData( p_wk->p_res );

		p_wk->btn_now_num	= BR_BTN_DATA_SYS_GetDataNum( p_wk->p_btn_data, menuID );

		for( i = 0; i < p_wk->btn_now_num; i++ )
		{	
			cp_data	= BR_BTN_DATA_SYS_GetData( p_wk->p_btn_data, menuID, i );
      p_strbuf  = BR_BTN_DATA_CreateString( p_wk->p_btn_data, cp_data, p_msg, GFL_HEAP_LOWID(p_wk->heapID) );

			BR_BTNEX_Init( &p_wk->p_btn_now[i], cp_data, p_wk->p_unit, p_wk->p_bmpoam, p_wk->p_res, p_strbuf, p_wk->heapID );

      GFL_STR_DeleteBuffer( p_strbuf );

      //優先度設定
      BR_BTNEX_SetSoftPriority( &p_wk->p_btn_now[i], 3 );

      if( cp_init_pos )
      { 
        BR_BTN_SetPos( p_wk->p_btn_now[i].p_btn, cp_init_pos->x, cp_init_pos->y );
      }
		}
	}
}

//=============================================================================
/**
 *						SEQ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	初期化
 *
 *	@param	SEQ_WORK *p_wk	ワーク
 *	@param	*p_param				パラメータ
 *	@param	seq_function		シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Init( SEQ_WORK *p_wk, void *p_wk_adrs, SEQ_FUNCTION seq_function )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(SEQ_WORK) );

	//初期化
	p_wk->p_wk_adrs	= p_wk_adrs;

	//セット
	SEQ_SetNext( p_wk, seq_function );
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	破棄
 *
 *	@param	SEQ_WORK *p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void SEQ_Exit( SEQ_WORK *p_wk )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(SEQ_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	メイン処理
 *
 *	@param	SEQ_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Main( SEQ_WORK *p_wk )
{	
	if( !p_wk->is_end )
	{	
		p_wk->seq_function( p_wk, &p_wk->seq, p_wk->p_wk_adrs );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	終了取得
 *
 *	@param	const SEQ_WORK *cp_wk		ワーク
 *
 *	@return	TRUEならば終了	FALSEならば処理中
 */	
//-----------------------------------------------------------------------------
static BOOL SEQ_IsEnd( const SEQ_WORK *cp_wk )
{	
	return cp_wk->is_end;
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	次のシーケンスを設定
 *
 *	@param	SEQ_WORK *p_wk	ワーク
 *	@param	seq_function		シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_SetNext( SEQ_WORK *p_wk, SEQ_FUNCTION seq_function )
{	
	p_wk->seq_function	= seq_function;
	p_wk->seq	= 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	終了
 *
 *	@param	SEQ_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_End( SEQ_WORK *p_wk )
{	
	p_wk->is_end	= TRUE;
}

//=============================================================================
/**
 *  SEQFUNC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	開始
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Start( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 

  BR_BTN_SYS_WORK *p_wk = p_wk_adrs;
  SEQ_SetNext( p_seqwk, SEQFUNC_Touch );
}
//----------------------------------------------------------------------------
/**
 *	@brief	タッチメイン
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Touch( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  int i;
  u32 x, y;
  BOOL is_trg	= FALSE;

  BR_BTN_SYS_WORK *p_wk = p_wk_adrs;

  p_wk->input	= BR_BTN_SYS_INPUT_NONE;

  //ボタン押し検知
  if( GFL_UI_TP_GetPointTrg( &x, &y ) )
  {	
    for( i = 0; i < p_wk->btn_now_max; i++ )
    {	
      if( BR_BTNEX_GetTrg( &p_wk->p_btn_now[i], x, y ) )
      {	

#ifdef PM_DEBUG
        int j;
        NAGI_Printf("push ! [%d]\n", i );
        for( j = 0; j < BR_BTN_DATA_PARAM_MAX; j++  )
        {	
          NAGI_Printf("param %d\n", BR_BTN_DATA_GetParam( p_wk->p_btn_now[i].cp_data, j) );
        }
#endif	//PM_DEBUG

        p_wk->trg_btn	= i;
        is_trg				= TRUE;
        break;
      }
    }
  }

  //ボタン動作開始
  if( is_trg )
  {	
    //押したボタンの情報
    p_wk->next_type	= BR_BTNEX_GetParam(&p_wk->p_btn_now[p_wk->trg_btn],BR_BTN_PARAM_TYPE );
    p_wk->next_valid= BR_BTNEX_GetParam(&p_wk->p_btn_now[p_wk->trg_btn],BR_BTN_PARAM_VALID );
    p_wk->next_proc	= BR_BTNEX_GetParam(&p_wk->p_btn_now[p_wk->trg_btn],BR_BTN_PARAM_DATA1 );
    p_wk->next_mode	= BR_BTNEX_GetParam(&p_wk->p_btn_now[p_wk->trg_btn],BR_BTN_PARAM_DATA2 );

    //優先度設定
    {
      int i;
      for( i = 0; i < p_wk->btn_now_num; i++ )
      {
        if( i == p_wk->trg_btn )
        { 
          BR_BTNEX_SetSoftPriority( &p_wk->p_btn_now[i], 1 );
        }
        else
        { 
          BR_BTNEX_SetSoftPriority( &p_wk->p_btn_now[i], 3 );
        }
      }
    }

    switch( p_wk->next_type )
    {
    case BR_BTN_TYPE_MYRECORD:
      /* fallthrough */
    case BR_BTN_TYPE_OTHERRECORD:
      if( p_wk->next_valid )
      { 
        SEQ_SetNext( p_seqwk, SEQFUNC_HideBtn );
      }
      break;

    case BR_BTN_TYPE_RETURN:
      SEQ_SetNext( p_seqwk, SEQFUNC_DownTag );
      break;

    case BR_BTN_TYPE_SELECT:
      /* fallthrough */
    case BR_BTN_TYPE_CHANGESEQ:
      SEQ_SetNext( p_seqwk, SEQFUNC_HideBtn );
      break;

    default:
      SEQ_SetNext( p_seqwk, SEQFUNC_Input );
    }
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief	ボタンが隠れる
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_HideBtn( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_HIDE_START,   //ボタンが隠れる処理
    SEQ_HIDE_WAIT,    //ボタンが隠れる待機
    SEQ_END,
  };
  BR_BTN_SYS_WORK *p_wk = p_wk_adrs;

 switch( *p_seq )
  { 
  case SEQ_HIDE_START:
    //ボタンが隠れる動作開始
    { 
      int i;
      GFL_POINT pos;
      s16 x, y;
      for( i = 0; i < p_wk->btn_now_num; i++ )
      {	
        if( p_wk->trg_btn != i )
        {	
          BR_BTNEX_GetPos( &p_wk->p_btn_now[p_wk->trg_btn], &x, &y );
          pos.x = x;
          pos.y = y;
          BR_BTNEX_StartMove( &p_wk->p_btn_now[i], BR_BTN_MOVE_HIDE, &pos );
        }
      }
    }
    *p_seq  = SEQ_HIDE_WAIT;
    break;
  
  case SEQ_HIDE_WAIT:
    //ボタンが隠れるまでの動作
		{	
			int i;
			BOOL is_end	= TRUE;

			for( i = 0; i < p_wk->btn_now_num; i++ )
			{	
				is_end	&= BR_BTNEX_MainMove( &p_wk->p_btn_now[i] );
			}

			if( is_end )
			{
        *p_seq  = SEQ_END;
			}
		}
    break;

  case SEQ_END:

    switch( p_wk->next_type )
    {
    case BR_BTN_TYPE_CHANGESEQ:
      SEQ_SetNext( p_seqwk, SEQFUNC_ChangePage );
      break;

    default:
      SEQ_SetNext( p_seqwk, SEQFUNC_ChangePage );
      break;
    }
    break;
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief	ボタンが隠れる  ターゲットは降りてきたもの
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_HideBtn2( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_HIDE_START,   //ボタンが隠れる処理
    SEQ_HIDE_WAIT,    //ボタンが隠れる待機
    SEQ_END,
  };
  BR_BTN_SYS_WORK *p_wk = p_wk_adrs;

 switch( *p_seq )
  { 
  case SEQ_HIDE_START:
    //ボタンが隠れる動作開始
    { 
      int i;
      GFL_POINT pos;
      for( i = 0; i < p_wk->btn_now_num; i++ )
      {	
        pos.x    = BR_BTN_DATA_GetParam( p_wk->p_btn_stack[p_wk->btn_stack_num - 1].cp_data, BR_BTN_DATA_PARAM_X );
        pos.y    = BR_BTN_DATA_GetParam( p_wk->p_btn_stack[p_wk->btn_stack_num - 1].cp_data, BR_BTN_DATA_PARAM_Y );
        BR_BTNEX_StartMove( &p_wk->p_btn_now[i], BR_BTN_MOVE_HIDE, &pos );
      }
    }
    *p_seq  = SEQ_HIDE_WAIT;
    break;
  
  case SEQ_HIDE_WAIT:
    //ボタンが隠れるまでの動作
		{	
			int i;
			BOOL is_end	= TRUE;

			for( i = 0; i < p_wk->btn_now_num; i++ )
			{	
				is_end	&= BR_BTNEX_MainMove( &p_wk->p_btn_now[i] );
			}

			if( is_end )
			{
        *p_seq  = SEQ_END;
			}
		}
    break;

  case SEQ_END:

    switch( p_wk->next_type )
    {
    case BR_BTN_TYPE_CHANGESEQ:
      SEQ_SetNext( p_seqwk, SEQFUNC_ChangePage );
      break;

    default:
      SEQ_SetNext( p_seqwk, SEQFUNC_ChangePage );
      break;
    }
    break;
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief	ページ切り替え演出
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_ChangePage( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  BR_BTN_SYS_WORK *p_wk = p_wk_adrs;

  //読み替え
  if( p_wk->next_type == BR_BTN_TYPE_SELECT )
  {
    //次へ行く場合、押したボタンから次への情報を得る
    u32 nextID					= p_wk->next_proc; 
    GFL_POINT pos;
    s16 x, y;

    //消す前に座標をもらう
    BR_BTNEX_GetPos( &p_wk->p_btn_now[p_wk->trg_btn], &x, &y );

    //決定したボタンをスタックに積む
    Br_Btn_Sys_PushStack( p_wk, &p_wk->p_btn_now[p_wk->trg_btn], CLSYS_DRAW_SUB );

    //積んだので、元の場所の情報を消す(コピーしてるので開放はしない)
    GFL_STD_MemClear( &p_wk->p_btn_now[p_wk->trg_btn], sizeof(BR_BTNEX_WORK) );

    //他のボタンを読み変える
    pos.x = x;
    pos.y = y;
    Br_Btn_Sys_ReLoadBtn( p_wk, nextID, &pos );

    SEQ_SetNext( p_seqwk, SEQFUNC_AppearBtn );
  }
  else if( p_wk->next_type == BR_BTN_TYPE_RETURN )
  { 
    //戻る場合、タグから次への情報を得る
    BR_BTNEX_WORK	btn;
    if( Br_Btn_Sys_PopStack( p_wk, &btn ) )
    {	
      u32 menuID  = BR_BTNEX_GetParam( &btn, BR_BTN_PARAM_MENUID );

      GFL_POINT pos;
      GF_ASSERT( btn.is_use );
      //
      pos.x    = BR_BTN_DATA_GetParam( btn.cp_data, BR_BTN_DATA_PARAM_X );
      pos.y    = BR_BTN_DATA_GetParam( btn.cp_data, BR_BTN_DATA_PARAM_Y );

      //他のボタンを読み変える
      Br_Btn_Sys_ReLoadBtn( p_wk, menuID, &pos );

      //自分と同じボタンは優先度を高くする
      {
        int i;
        u32 y = BR_BTN_DATA_GetParam( btn.cp_data, BR_BTN_DATA_PARAM_Y );
        for( i = 0; i < p_wk->btn_now_num; i++ )
        { 
          if( y == BR_BTN_DATA_GetParam( p_wk->p_btn_now[i].cp_data, BR_BTN_DATA_PARAM_Y ) )
          { 
            BR_BTNEX_SetSoftPriority( &p_wk->p_btn_now[i], 1 );
          }
        }
      }

      //削除
      BR_BTNEX_Exit( &btn );

      SEQ_SetNext( p_seqwk, SEQFUNC_AppearBtn );
    }
  } 
  else
  { 
    //決定したボタンをスタックに積む
    Br_Btn_Sys_PushStack( p_wk, &p_wk->p_btn_now[p_wk->trg_btn], CLSYS_DRAW_SUB );

    //積んだので、元の場所の情報を消す(コピーしてるので開放はしない)
    GFL_STD_MemClear( &p_wk->p_btn_now[p_wk->trg_btn], sizeof(BR_BTNEX_WORK) );

    //他のボタンを消す
    { 
      int i;
			for( i = 0; i < p_wk->btn_now_num; i++ )
			{	
				BR_BTNEX_Exit( &p_wk->p_btn_now[i] );
			}

    }


    SEQ_SetNext( p_seqwk, SEQFUNC_UpTag );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	出現
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_AppearBtn( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_APPEAR_START, //ボタン出現処理
    SEQ_APPEAR_WAIT,  //ボタン出現待機
    SEQ_END,
  };
  BR_BTN_SYS_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_APPEAR_START:
    //ボタン出現処理
    {	
      int i;
      //ボタン出現処理
      for( i = 0; i < p_wk->btn_now_num; i++ )
      {	
        BR_BTNEX_StartMove( &p_wk->p_btn_now[i], BR_BTN_MOVE_APPEAR, NULL );
      }

      *p_seq  = SEQ_APPEAR_WAIT;
    }
    break;

  case SEQ_APPEAR_WAIT:
    //ボタン出現待機処理
		{	
			int i;
			BOOL is_end	= TRUE;

			for( i = 0; i < p_wk->btn_now_num; i++ )
			{	
				is_end	&= BR_BTNEX_MainMove( &p_wk->p_btn_now[i] );
			}
			if( is_end )
			{
        *p_seq  = SEQ_END;
			}
		}
    break;
    
  case SEQ_END:

    switch( p_wk->next_type )
    {
    case BR_BTN_TYPE_SELECT:
    case BR_BTN_TYPE_CHANGESEQ:
      SEQ_SetNext( p_seqwk, SEQFUNC_UpTag ); 
      break;

    default:
      SEQ_SetNext( p_seqwk, SEQFUNC_Input );
      break;
    }
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	タグが上に
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_UpTag( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_UP_START, //ボタン出現処理
    SEQ_UP_WAIT,  //ボタン出現待機
    SEQ_END,
  };
  BR_BTN_SYS_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_UP_START:
    //ボタン出現処理
    {	
      int i;
      //スタックに積んだ、決定ボタンは上部へ移動
			for( i = 0; i < p_wk->btn_stack_num; i++ )
      { 
        if( i == p_wk->btn_stack_num - 1 )
        {	
          BR_BTNEX_StartMove( &p_wk->p_btn_stack[ i ], BR_BTN_MOVE_UP_TAG, &sc_tag_pos[0] );
        }
        else
        { 
          BR_BTNEX_StartMove( &p_wk->p_btn_stack[ i ], BR_BTN_MOVE_NEXT_TARGET, &sc_tag_pos[p_wk->btn_stack_num-i-1] );
        }
      }

      *p_seq  = SEQ_UP_WAIT;
    }
    break;

  case SEQ_UP_WAIT:
    //ボタン出現待機処理
		{	
			int i;
			BOOL is_end	= TRUE;

			for( i = 0; i < p_wk->btn_stack_num; i++ )
			{	
				is_end	&= BR_BTNEX_MainMove( &p_wk->p_btn_stack[i] );
			}
			if( is_end )
			{
        *p_seq  = SEQ_END;
			}
		}
    break;
    
  case SEQ_END:
    SEQ_SetNext( p_seqwk, SEQFUNC_Input ); 
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	タグを下に移動
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_DownTag( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_DOWN_START, //ボタン上昇開始
    SEQ_DOWN_WAIT,  //ボタン上昇待機
    SEQ_END,
  };
  BR_BTN_SYS_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_DOWN_START:
    //ボタン出現処理
    {	
      int i;
      GFL_POINT pos;
      //スタックに積んだ、決定ボタンは下部へ移動
      for( i = 0; i < p_wk->btn_stack_num; i++ )
      { 
        if( i == p_wk->btn_stack_num - 1  )
        {	
          pos.x = BR_BTN_DATA_GetParam( p_wk->p_btn_stack[i].cp_data, BR_BTN_DATA_PARAM_X );
          pos.y = BR_BTN_DATA_GetParam( p_wk->p_btn_stack[i].cp_data, BR_BTN_DATA_PARAM_Y );
          BR_BTNEX_StartMove( &p_wk->p_btn_stack[i], BR_BTN_MOVE_DOWN_TAG, &pos );
          BR_BTNEX_SetSoftPriority( &p_wk->p_btn_stack[i], 1 );
        }
        else
        { 
          BR_BTNEX_StartMove( &p_wk->p_btn_stack[i], BR_BTN_MOVE_NEXT_TARGET, &sc_tag_pos[p_wk->btn_stack_num-i-2] );
        }
      }

      //優先度設定
      {
        int i;
        for( i = 0; i < p_wk->btn_now_num; i++ )
        {
          BR_BTNEX_SetSoftPriority( &p_wk->p_btn_now[i], 3 );
        }
      }

      *p_seq  = SEQ_DOWN_WAIT;
    }
    break;

  case SEQ_DOWN_WAIT:
    //ボタン出現待機処理
		{	
			int i;
			BOOL is_end	= TRUE;

			for( i = 0; i < p_wk->btn_stack_num; i++ )
			{	
				is_end	&= BR_BTNEX_MainMove( &p_wk->p_btn_stack[i] );
			}
			if( is_end )
			{
        *p_seq  = SEQ_END;
			}
		}
    break;
    
  case SEQ_END:
    SEQ_SetNext( p_seqwk, SEQFUNC_HideBtn2 ); 
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	入力決定
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Input( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_END,
  };
  BR_BTN_SYS_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_END:
    if( p_wk->next_valid )
		{	
      switch( p_wk->next_type )
      {	
      case BR_BTN_TYPE_SELECT:				//選択用ボタン
        /* fallthrough */
      case BR_BTN_TYPE_RETURN:				//1つ前のメニューへ戻る用ボタン
        p_wk->state	= BR_BTN_SYS_STATE_WAIT;
        break;

      case BR_BTN_TYPE_EXIT:					//バトルレコーダー終了用ボタン
        p_wk->input	= BR_BTN_SYS_INPUT_EXIT;
        break;

      case BR_BTN_TYPE_MYRECORD:			//自分の記録ボタン
        p_wk->input	= BR_BTN_SYS_INPUT_CHANGESEQ;
        break;

      case BR_BTN_TYPE_OTHERRECORD:	//誰かの記録ボタン
        p_wk->input	= BR_BTN_SYS_INPUT_CHANGESEQ;
        break;

			case BR_BTN_TYPE_DELETE_MY:		//自分の記録を消すボタン
				break;

			case BR_BTN_TYPE_DELETE_OTHER:	//誰かの記録を消すボタン
				break;

			case BR_BTN_TYPE_CHANGESEQ:		//シーケンス変更ボタン			
        p_wk->input	= BR_BTN_SYS_INPUT_CHANGESEQ;
        break;
      }
    }

    SEQ_SetNext( p_seqwk, SEQFUNC_Touch ); 
    break;
  }
}

//=============================================================================
/**
 *		ボタン１つ１つの処理
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ボタン初期化
 *
 *	@param	BR_BTNEX_WORK *p_wk			ワーク
 *	@param	BR_BTN_DATA *cp_data	ボタン情報
 *	@param	*p_unit								OBJ作成用ユニット
 *	@param	*p_bmpoam							BMPOAMシステム
 *	@param	p_res									リソース
 *	@param	heapID								ヒープID
 */
//-----------------------------------------------------------------------------
static void BR_BTNEX_Init( BR_BTNEX_WORK *p_wk, const BR_BTN_DATA *cp_data, GFL_CLUNIT *p_unit, BMPOAM_SYS_PTR p_bmpoam, const BR_RES_WORK *cp_res, const STRBUF *cp_strbuf, HEAPID heapID )
{	
	u32 plt;

	GF_ASSERT( p_wk->is_use == FALSE );

	GFL_STD_MemClear( p_wk, sizeof(BR_BTNEX_WORK) );
	p_wk->is_use	= TRUE;
	p_wk->cp_data	= cp_data;
  p_wk->display = CLSYS_DRAW_SUB;

  p_wk->cp_res  = cp_res;
  p_wk->heapID  = heapID;
  p_wk->p_unit  = p_unit;
  p_wk->p_bmpoam  = p_bmpoam;

	//ボタン作成
	{	
		BR_RES_OBJ_DATA	res;
		GFL_FONT *p_font;
		GFL_CLWK_DATA	cldata;
    STRBUF  *p_strbuf;

    //データ設定
		GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
    cldata.pos_x    = BR_BTN_DATA_GetParam( p_wk->cp_data, BR_BTN_DATA_PARAM_X );
    cldata.pos_y    = BR_BTN_DATA_GetParam( p_wk->cp_data, BR_BTN_DATA_PARAM_Y );
    cldata.anmseq   = BR_BTN_DATA_GetParam( p_wk->cp_data, BR_BTN_DATA_PARAM_ANMSEQ );
    cldata.softpri  = 1;

		//リソース読み込み
		BR_RES_GetOBJRes( cp_res, BR_RES_OBJ_BROWSE_BTN_S, &res );
		p_font	= BR_RES_GetFont( cp_res );

		//作成
		p_wk->p_btn		= BR_BTN_InitEx( &cldata, cp_strbuf, BR_BTN_DATA_WIDTH, p_wk->display, p_unit, 
				p_bmpoam, p_font, &res, GetHeapLowID( heapID ) );

	}

}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタン破棄
 *
 *	@param	BR_BTNEX_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BR_BTNEX_Exit( BR_BTNEX_WORK *p_wk )
{	
	if( p_wk->is_use )
	{	
    BR_BTN_Exit( p_wk->p_btn );
		GFL_STD_MemClear( p_wk, sizeof(BR_BTNEX_WORK) );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンをタッチしたかどうか
 *
 *	@param	const BR_BTNEX_WORK *cp_wk	ワーク
 *	@param	位置座標
 *
 *	@return	TRUEならば押した	FALSEならば押していない
 */
//-----------------------------------------------------------------------------
static BOOL BR_BTNEX_GetTrg( const BR_BTNEX_WORK *cp_wk, u32 x, u32 y )
{	
	if( !cp_wk->is_use )
	{	
		return FALSE;
	}

  return BR_BTN_GetTrg( cp_wk->p_btn, x, y );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンの動作開始
 *
 *	@param	BR_BTNEX_WORK *p_wk				ワーク
 *	@param	move										動作
 *	@param	BR_BTNEX_WORK *cp_target	動作に必要なターゲっト
 */
//-----------------------------------------------------------------------------
static void BR_BTNEX_StartMove( BR_BTNEX_WORK *p_wk, BR_BTN_MOVE move, const GFL_POINT *cp_target )
{	
  p_wk->move_seq  = 0;

  if( cp_target )
  { 
    p_wk->target = *cp_target;
  }

  switch( move )
  { 
  case BR_BTN_MOVE_HIDE:
    p_wk->move_function = Br_BtnEx_Move_Hide;
    break;

  case BR_BTN_MOVE_APPEAR:
    p_wk->move_function = Br_BtnEx_Move_Appear;
    break;

  case BR_BTN_MOVE_UP_TAG:
    p_wk->move_function = Br_BtnEx_Move_UpTag;
    break;

  case BR_BTN_MOVE_DOWN_TAG:
    p_wk->move_function = Br_BtnEx_Move_DownTag;
    break;
    
  case BR_BTN_MOVE_NEXT_TARGET:
    p_wk->move_function = Br_BtnEx_Move_NextTarget;
    break;
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンのメイン動作
 *
 *	@param	BR_BTNEX_WORK *p_wk		ワーク
 *
 *	@return	TRUEならばボタンの動き終了	FALSEならば動き中
 */
//-----------------------------------------------------------------------------
static BOOL BR_BTNEX_MainMove( BR_BTNEX_WORK *p_wk )
{	
  if( p_wk->move_function )
  { 
    BOOL ret;
    ret = p_wk->move_function( p_wk );

    //終了したら移動関数を空に
    if( ret )
    { 
      p_wk->move_function = NULL;
    }
    return ret;
  }

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ボタンのディスプレイを切り替え
 *
 *	@param	BR_BTNEX_WORK *cp_wk  ワーク
 *	@param	どちらの画面に切り替えるか
 */
//-----------------------------------------------------------------------------
static void BR_BTNEX_ChangeDisplay( BR_BTNEX_WORK *p_wk, CLSYS_DRAW_TYPE display )
{ 
  if( p_wk->display != display )
  { 
    s16 x, y;

    p_wk->display = display;

    //新しく作り直すため、消去
		{	
      BR_BTN_GetPos( p_wk->p_btn, &x, &y );
			BR_BTN_Exit( p_wk->p_btn );
		}
    //ボタン作成
    {	
      BR_RES_OBJ_DATA	res;
      GFL_FONT *p_font;
      GFL_MSGDATA *p_msg;
      GFL_CLWK_DATA	cldata;
      u32 msgID;

      s16 y_ofs;
      if( display == CLSYS_DRAW_MAIN )
      { 
        y_ofs = +512;
      }
      else
      { 

        y_ofs = -512;
      }

      //データ設定
      GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
      cldata.pos_x    = x;
      cldata.pos_y    = y + y_ofs;
      cldata.anmseq   = BR_BTN_DATA_GetParam( p_wk->cp_data, BR_BTN_DATA_PARAM_ANMSEQ );
      cldata.softpri  = 1;

      //リソース読み込み
			if( display == CLSYS_DRAW_MAIN )
			{	
				BR_RES_GetOBJRes( p_wk->cp_res, BR_RES_OBJ_BROWSE_BTN_M, &res );
			}
			else
			{	
				BR_RES_GetOBJRes( p_wk->cp_res, BR_RES_OBJ_BROWSE_BTN_S, &res );
			}
      p_font	= BR_RES_GetFont( p_wk->cp_res );
      p_msg		= BR_RES_GetMsgData( p_wk->cp_res );
      msgID		= BR_BTN_DATA_GetParam( p_wk->cp_data, BR_BTN_DATA_PARAM_MSGID );

      //作成
      p_wk->p_btn		= BR_BTN_Init( &cldata, msgID, BR_BTN_DATA_WIDTH, display, p_wk->p_unit, 
          p_wk->p_bmpoam, p_font, p_msg, &res, p_wk->heapID );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンをコピー
 *
 *	@param	p_wk		ワーク					
 *	@param	p_dst		コピー先　サブ画面のボタン
 *	@param	p_src		コピー元	メイン画面のボタン
 *	@param	display	どちらの画面にコピーするか
 */
//-----------------------------------------------------------------------------
static void BR_BTNEX_Copy( const BR_BTN_SYS_WORK *cp_wk, BR_BTNEX_WORK *p_dst, const BR_BTNEX_WORK *cp_src, CLSYS_DRAW_TYPE display )
{
	u32 plt;	

	GF_ASSERT( p_dst->is_use == FALSE );
	GF_ASSERT( cp_src->is_use == TRUE );
		
	//一旦コピー
	{
    *p_dst  = *cp_src;
	}

	//違うならば作り直し
	if( cp_src->display	!= display )
	{	
    BR_BTNEX_ChangeDisplay( p_dst, display );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	ボタンの座標を設定
 *
 *	@param	BR_BTNEX_WORK *p_wk	ワーク
 *	@param	x									X座標
 *	@param	y									Y座標
 *
 */
//-----------------------------------------------------------------------------
static void BR_BTNEX_SetStackPos( BR_BTNEX_WORK *p_wk, u16 stack_num )
{	
	GFL_CLACTPOS	pos;
	pos.x	= BR_BTN_DATA_GetParam( p_wk->cp_data, BR_BTN_DATA_PARAM_X );
	pos.y	= TAG_INIT_POS_UP( stack_num );

  BR_BTN_SetPos( p_wk->p_btn, pos.x, pos.y );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ボタンの位置を取得
 *
 *	@param	const BR_BTNEX_WORK *cp_wk  ワーク
 *	@param	*p_pos 座標
 */
//-----------------------------------------------------------------------------
static void BR_BTNEX_GetPos( const BR_BTNEX_WORK *cp_wk, s16 *p_x, s16 *p_y )
{ 
  BR_BTN_GetPos( cp_wk->p_btn, p_x, p_y );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ボタンの情報を受け取り
 *
 *	@param	const BR_BTNEX_WORK *cp_wk	ワーク
 *	@param	param		ボタンの情報インデックス
 *
 *	@return	ボタン情報
 */
//-----------------------------------------------------------------------------
static u32 BR_BTNEX_GetParam( const BR_BTNEX_WORK *cp_wk, BR_BTN_PARAM param )
{	
	u32 ret;
	switch( param )
	{	
	case BR_BTN_PARAM_TYPE:
		ret = BR_BTN_DATA_GetParam( cp_wk->cp_data, BR_BTN_DATA_PARAM_TYPE );
		break;

	case BR_BTN_PARAM_DATA1:
		ret = BR_BTN_DATA_GetParam( cp_wk->cp_data, BR_BTN_DATA_PARAM_DATA1 );
		break;

	case BR_BTN_PARAM_DATA2:
		ret = BR_BTN_DATA_GetParam( cp_wk->cp_data, BR_BTN_DATA_PARAM_DATA2 );
		break;
	
	case BR_BTN_PARAM_VALID:
		ret = BR_BTN_DATA_GetParam( cp_wk->cp_data, BR_BTN_DATA_PARAM_VALID );
		break;

	case BR_BTN_PARAM_MENUID:
		ret = BR_BTN_DATA_GetParam( cp_wk->cp_data, BR_BTN_DATA_PARAM_MENUID );
		break;

	default:
		ret	= 0;
		GF_ASSERT(0);
	}

	return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  プライオリティ設定
 *
 *	@param	BR_BTNEX_WORK *p_wk ワーク
 *	@param	soft_pri            優先度
 */
//-----------------------------------------------------------------------------
static void BR_BTNEX_SetSoftPriority( BR_BTNEX_WORK *p_wk, u16 soft_pri )
{ 
  BR_BTN_SetSoftPriority( p_wk->p_btn, soft_pri );
}
//----------------------------------------------------------------------------
/**
 *	@brief  格納する移動
 *
 *	@param	BR_BTNEX_WORK *p_wk   ワーク
 *
 *	@return TRUEで移動終了  FALSEで移動中
 */
//-----------------------------------------------------------------------------
static BOOL Br_BtnEx_Move_Hide( BR_BTNEX_WORK *p_wk )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_EXIT,
  };

  switch( p_wk->move_seq )
  { 
  case SEQ_INIT:
    {
      s16 start_y, end_y;

      BR_BTNEX_GetPos( p_wk, NULL, &start_y );
      p_wk->start       = start_y;
      p_wk->end         = p_wk->target.y;
      p_wk->sync_now    = 0;
      p_wk->sync_max    = BR_BTN_MOVE_HIDE_SYNC;
    }
    p_wk->move_seq  = SEQ_MAIN;
    break;

  case SEQ_MAIN:
    {
      s16 x;
      s16 now_y;
      now_y = p_wk->start + (p_wk->end - p_wk->start) * p_wk->sync_now / p_wk->sync_max;
      
      BR_BTN_GetPos( p_wk->p_btn, &x, NULL);
      BR_BTN_SetPos( p_wk->p_btn, x, now_y );

      if( p_wk->sync_now++ >= p_wk->sync_max )
      { 
        p_wk->move_seq  = SEQ_EXIT;
      }
    }
    break;

  case SEQ_EXIT:
    return TRUE;
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  出現処理
 *
 *	@param	BR_BTNEX_WORK *p_wk   ワーク
 *
 *	@return TRUEで移動終了  FALSEで移動中
 */
//-----------------------------------------------------------------------------
static BOOL Br_BtnEx_Move_Appear( BR_BTNEX_WORK *p_wk )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_EXIT,
  };

  switch( p_wk->move_seq )
  { 
  case SEQ_INIT:
    {
      s16 start_y, end_y;

      BR_BTNEX_GetPos( p_wk, NULL, &start_y );
      end_y  = BR_BTN_DATA_GetParam( p_wk->cp_data, BR_BTN_DATA_PARAM_Y ); 

      p_wk->start       = start_y;
      p_wk->end         = end_y;
      p_wk->sync_now    = 0;
      p_wk->sync_max    = BR_BTN_MOVE_APPEAR_SYNC;
    }
    p_wk->move_seq  = SEQ_MAIN;
    break;

  case SEQ_MAIN:
    {
      s16 x;
      s16 now_y;
      now_y = p_wk->start + (p_wk->end - p_wk->start) * p_wk->sync_now / p_wk->sync_max;
      
      BR_BTN_GetPos( p_wk->p_btn, &x, NULL);
      BR_BTN_SetPos( p_wk->p_btn, x, now_y );

      if( p_wk->sync_now++ >= p_wk->sync_max )
      { 
        p_wk->move_seq  = SEQ_EXIT;
      }
    }
    break;

  case SEQ_EXIT:
    { 
    }
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  タグが上に
 *
 *	@param	BR_BTNEX_WORK *p_wk   ワーク
 *
 *	@return TRUEで移動終了  FALSEで移動中
 */
//-----------------------------------------------------------------------------
static BOOL Br_BtnEx_Move_UpTag( BR_BTNEX_WORK *p_wk )
{ 
  enum
  { 
    SEQ_INIT_S,
    SEQ_MAIN_S,
    SEQ_CHANGE,
    SEQ_INIT_M,
    SEQ_MAIN_M,
    SEQ_EXIT,
  };

  switch( p_wk->move_seq )
  { 
  case SEQ_INIT_S:
    {
      s16 start_y, end_y;

      BR_BTNEX_GetPos( p_wk, NULL, &start_y );
      //end_y  = p_wk->target.y;

      p_wk->start       = start_y;
      p_wk->end         = -BR_BTN_DATA_HEIGHT;
      p_wk->sync_now    = 0;
      p_wk->sync_max    = BR_BTN_MOVE_UPTAG_M_SYNC;

    }
    p_wk->move_seq  = SEQ_MAIN_S;
    break;

  case SEQ_MAIN_S:
    {
      s16 x;
      s16 now_y;
      now_y = p_wk->start + (p_wk->end - p_wk->start) * p_wk->sync_now / p_wk->sync_max;
      
      BR_BTN_GetPos( p_wk->p_btn, &x, NULL);
      BR_BTN_SetPos( p_wk->p_btn, x, now_y );

      if( p_wk->sync_now++ >= p_wk->sync_max )
      { 
        p_wk->move_seq  = SEQ_CHANGE;
      }
    }
    break;

  case SEQ_CHANGE:
    { 
      BR_BTNEX_ChangeDisplay( p_wk, CLSYS_DRAW_MAIN );
      p_wk->move_seq  = SEQ_INIT_M;
    }
    break;

  case SEQ_INIT_M:
    {
      s16 start_y, end_y;

      BR_BTNEX_GetPos( p_wk, NULL, &start_y );

      p_wk->start       = start_y;
      p_wk->end         = p_wk->target.y;
      p_wk->sync_now    = 0;
      p_wk->sync_max    = BR_BTN_MOVE_UPTAG_M_SYNC;

    }
    p_wk->move_seq  = SEQ_MAIN_M;
    break;

  case SEQ_MAIN_M:
    {
      s16 x;
      s16 now_y;
      now_y = p_wk->start + (p_wk->end - p_wk->start) * p_wk->sync_now / p_wk->sync_max;
      
      BR_BTN_GetPos( p_wk->p_btn, &x, NULL);
      BR_BTN_SetPos( p_wk->p_btn, x, now_y );

      if( p_wk->sync_now++ >= p_wk->sync_max )
      { 
        p_wk->move_seq  = SEQ_EXIT;
      }
    }
    break;

  case SEQ_EXIT:
    { 
    }
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  タグが下に
 *
 *	@param	BR_BTNEX_WORK *p_wk   ワーク
 *
 *	@return TRUEで移動終了  FALSEで移動中
 */
//-----------------------------------------------------------------------------
static BOOL Br_BtnEx_Move_DownTag( BR_BTNEX_WORK *p_wk )
{ 
  enum
  { 
    SEQ_INIT_M,
    SEQ_MAIN_M,
    SEQ_CHANGE,
    SEQ_INIT_S,
    SEQ_MAIN_S,
    SEQ_EXIT,
  };

  switch( p_wk->move_seq )
  { 
  case SEQ_INIT_M:
    {
      s16 start_y, end_y;

      BR_BTNEX_GetPos( p_wk, NULL, &start_y );

      p_wk->start       = start_y;
      p_wk->end         = 192+BR_BTN_DATA_HEIGHT;
      p_wk->sync_now    = 0;
      p_wk->sync_max    = BR_BTN_MOVE_UPTAG_M_SYNC;

    }
    p_wk->move_seq  = SEQ_MAIN_M;
    break;

  case SEQ_MAIN_M:
    {
      s16 x;
      s16 now_y;
      now_y = p_wk->start + (p_wk->end - p_wk->start) * p_wk->sync_now / p_wk->sync_max;
      
      BR_BTN_GetPos( p_wk->p_btn, &x, NULL);
      BR_BTN_SetPos( p_wk->p_btn, x, now_y );

      if( p_wk->sync_now++ >= p_wk->sync_max )
      { 
        p_wk->move_seq  = SEQ_CHANGE;
      }
    }
    break;

  case SEQ_CHANGE:
    { 
      BR_BTNEX_ChangeDisplay( p_wk, CLSYS_DRAW_SUB );
      p_wk->move_seq  = SEQ_INIT_S;
    }
    break;

  case SEQ_INIT_S:
    {
      s16 start_y, end_y;

      BR_BTNEX_GetPos( p_wk, NULL, &start_y );

      p_wk->start       = start_y;
      p_wk->end         = p_wk->target.y;
      p_wk->sync_now    = 0;
      p_wk->sync_max    = BR_BTN_MOVE_UPTAG_M_SYNC;

    }
    p_wk->move_seq  = SEQ_MAIN_S;
    break;

  case SEQ_MAIN_S:
    {
      s16 x;
      s16 now_y;
      now_y = p_wk->start + (p_wk->end - p_wk->start) * p_wk->sync_now / p_wk->sync_max;
      
      BR_BTN_GetPos( p_wk->p_btn, &x, NULL);
      BR_BTN_SetPos( p_wk->p_btn, x, now_y );

      if( p_wk->sync_now++ >= p_wk->sync_max )
      { 
        p_wk->move_seq  = SEQ_EXIT;
      }
    }
    break;

  case SEQ_EXIT:
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  目的の座標へいく
 *
 *	@param	BR_BTNEX_WORK *p_wk   ワーク
 *
 *	@return TRUEで移動終了  FALSEで移動中
 */
//-----------------------------------------------------------------------------
static BOOL Br_BtnEx_Move_NextTarget( BR_BTNEX_WORK *p_wk )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_EXIT,
  };

  switch( p_wk->move_seq )
  { 
  case SEQ_INIT:
    {
      s16 start_y, end_y;

      BR_BTNEX_GetPos( p_wk, NULL, &start_y );
      p_wk->start       = start_y;
      p_wk->end         = p_wk->target.y;
      p_wk->sync_now    = 0;
      p_wk->sync_max    = BR_BTN_MOVE_DEFAULT_SYNC;
    }
    p_wk->move_seq  = SEQ_MAIN;
    break;

  case SEQ_MAIN:
    {
      s16 x;
      s16 now_y;
      now_y = p_wk->start + (p_wk->end - p_wk->start) * p_wk->sync_now / p_wk->sync_max;
      
      BR_BTN_GetPos( p_wk->p_btn, &x, NULL);
      BR_BTN_SetPos( p_wk->p_btn, x, now_y );

      if( p_wk->sync_now++ >= p_wk->sync_max )
      { 
        p_wk->move_seq  = SEQ_EXIT;
      }
    }
    break;

  case SEQ_EXIT:
    return TRUE;
  }

  return FALSE;
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					ボタン単体作成関数(各プロックで使用する)
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//----------------------------------------------------------------------------
/**
 *	@brief	ボタン初期化
 *
 *	@param	const GFL_CLWK_DATA *cp_cldata	設定情報
 *	@param	msgID														ボタンに載せる文字列
 *	@param  w                               幅
 *	@param	display													表示面
 *	@param	*p_unit													アクター登録ユニット
 *	@param	p_bmpoam												BMPOAM登録システム
 *	@param	*p_font													フォント
 *	@param	*p_msg													メッセージ
 *	@param	BR_RES_OBJ_DATA *cp_res					リソース
 *	@param	heapID													ヒープID
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
BR_BTN_WORK * BR_BTN_Init( const GFL_CLWK_DATA *cp_cldata, u16 msgID, u16 w, CLSYS_DRAW_TYPE display, GFL_CLUNIT *p_unit, BMPOAM_SYS_PTR p_bmpoam, GFL_FONT *p_font, GFL_MSGDATA *p_msg, const BR_RES_OBJ_DATA *cp_res, HEAPID heapID )
{	
	BR_BTN_WORK *p_wk;
  STRBUF  *p_strbuf;

  //文字列作成
  p_strbuf	= GFL_MSG_CreateString( p_msg, msgID );

  //作成コア
  p_wk  = BR_BTN_InitEx( cp_cldata, p_strbuf, w, display, p_unit, p_bmpoam, p_font, cp_res, heapID );

	//フォント
  GFL_STR_DeleteBuffer( p_strbuf );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタン初期化EX msgIDではなくSTRBUFから文字を作成する版
 *
 *	@param	const GFL_CLWK_DATA *cp_cldata	設定情報
 *	@param	msgID														ボタンに載せる文字列
 *	@param  w                               幅
 *	@param	display													表示面
 *	@param	*p_unit													アクター登録ユニット
 *	@param	p_bmpoam												BMPOAM登録システム
 *	@param	*p_font													フォント
 *	@param	*p_msg													メッセージ
 *	@param	BR_RES_OBJ_DATA *cp_res					リソース
 *	@param	heapID													ヒープID
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
BR_BTN_WORK * BR_BTN_InitEx( const GFL_CLWK_DATA *cp_cldata, const STRBUF *cp_strbuf, u16 w, CLSYS_DRAW_TYPE display, GFL_CLUNIT *p_unit, BMPOAM_SYS_PTR p_bmpoam, GFL_FONT *p_font, const BR_RES_OBJ_DATA *cp_res, HEAPID heapID )
{	
	BR_BTN_WORK *p_wk;

	p_wk	= GFL_HEAP_AllocMemory( heapID, sizeof(BR_BTN_WORK) );
	GFL_STD_MemClear( p_wk, sizeof(BR_BTN_WORK) );
  p_wk->w       = w;
  p_wk->display = display;

	//CLWK作成
	{	
		p_wk->p_clwk	= GFL_CLACT_WK_Create( p_unit, 
				cp_res->ncg, cp_res->ncl, cp_res->nce, 
				cp_cldata, display, heapID );
    GFL_CLACT_WK_SetObjMode( p_wk->p_clwk, GX_OAM_MODE_XLU );
	}

	//フォント
	{	
		STRBUF			*p_str;

		p_wk->p_bmp	= GFL_BMP_Create( p_wk->w/8, 2, GFL_BMP_16_COLOR, heapID );
		PRINTSYS_Print( p_wk->p_bmp, 0, 0, cp_strbuf, p_font );
	}


	//OAMの文字を作成
	{	
		BMPOAM_ACT_DATA	actdata;
		
		GFL_STD_MemClear( &actdata, sizeof(BMPOAM_ACT_DATA) );
		actdata.bmp	= p_wk->p_bmp;
		actdata.x		= cp_cldata->pos_x + BR_BTN_OAMFONT_OFS_X;
		actdata.y		= cp_cldata->pos_y - BR_BTN_DATA_HEIGHT/2 + BR_BTN_OAMFONT_OFS_Y;
		actdata.pltt_index	= cp_res->ncl;
		actdata.soft_pri		= cp_cldata->softpri - 1;
		actdata.setSerface	= display;
		actdata.draw_type		= display;
		actdata.bg_pri			= cp_cldata->bgpri;
		p_wk->p_oamfont	= BmpOam_ActorAdd( p_bmpoam, &actdata );
    BmpOam_ActorBmpTrans(p_wk->p_oamfont);
    BmpOam_ActorSetObjMode( p_wk->p_oamfont, GX_OAM_MODE_XLU );
	}

	return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ボタン破棄
 *
 *	@param	BR_BTN_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_BTN_Exit( BR_BTN_WORK *p_wk )
{	
	BmpOam_ActorDel( p_wk->p_oamfont );
	GFL_BMP_Delete( p_wk->p_bmp );
	GFL_CLACT_WK_Remove( p_wk->p_clwk );

	GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンのタッチを検知
 *
 *	@param	const BR_BTN_WORK *cp_wk	ワーク
 *	@param	x													タッチ座標X
 *	@param	y													タッチ座標Y
 *
 *	@return	TRUEでボタントリガ	FALSEで押していない
 */
//-----------------------------------------------------------------------------
BOOL BR_BTN_GetTrg( const BR_BTN_WORK *cp_wk, u32 x, u32 y )
{	
	GFL_RECT rect;
	GFL_CLACTPOS	pos;

	GFL_CLACT_WK_GetPos( cp_wk->p_clwk, &pos, cp_wk->display );

	rect.left		= pos.x;
	rect.right	= pos.x + cp_wk->w;
	rect.top		= pos.y - BR_BTN_DATA_HEIGHT/2;
	rect.bottom	= pos.y + BR_BTN_DATA_HEIGHT/2;

  return ( ((u32)( x - rect.left) <= (u32)(rect.right - rect.left))
            & ((u32)( y - rect.top) <= (u32)(rect.bottom - rect.top)));
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンの位置を設定
 *
 *	@param	BR_BTN_WORK *p_wk		ワーク
 *	@param	x										座標X
 *	@param	y										座標Y
 */
//-----------------------------------------------------------------------------
void BR_BTN_SetPos( BR_BTN_WORK *p_wk, s16 x, s16 y )
{	
	GFL_CLACTPOS	pos;
	pos.x	= x;
	pos.y	= y;

	GFL_CLACT_WK_SetPos( p_wk->p_clwk, &pos, p_wk->display );
	BmpOam_ActorSetPos( p_wk->p_oamfont,	pos.x + BR_BTN_OAMFONT_OFS_X,
																				pos.y - BR_BTN_DATA_HEIGHT/2 + BR_BTN_OAMFONT_OFS_Y );

}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンの位置を取得
 *
 *	@param	BR_BTN_WORK *p_wk		ワーク
 *	@param	x										座標X受け取り
 *	@param	y										座標Y受け取り
 */
//-----------------------------------------------------------------------------
void BR_BTN_GetPos( const BR_BTN_WORK *cp_wk, s16 *p_x, s16 *p_y )
{ 

	GFL_CLACTPOS	pos;
	GFL_CLACT_WK_GetPos( cp_wk->p_clwk, &pos, cp_wk->display );

  if( p_x )
  { 
    *p_x  = pos.x;
  }
  if( p_y )
  { 
    *p_y  = pos.y;
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  OBJ同士のプライオリティを設定
 *
 *	@param	BR_BTN_WORK *p_wk ワーク
 *	@param	soft_pri          プライオリティ
 */
//-----------------------------------------------------------------------------
void BR_BTN_SetSoftPriority( BR_BTN_WORK *p_wk, u8 soft_pri )
{ 
  GFL_CLACT_WK_SetSoftPri( p_wk->p_clwk, soft_pri );
  BmpOam_ActorSetSoftPriprity( p_wk->p_oamfont, soft_pri-1 );

}

//----------------------------------------------------------------------------
/**
 *	@brief  OBJ同士のプライオリティ取得
 *
 *	@param	const BR_BTN_WORK *cp_wk  ワーク
 *
 *	@return プライオリティ
 */
//-----------------------------------------------------------------------------
u8 BR_BTN_GetSoftPriority( const BR_BTN_WORK *cp_wk )
{ 
  return GFL_CLACT_WK_GetSoftPri( cp_wk->p_clwk );
}
