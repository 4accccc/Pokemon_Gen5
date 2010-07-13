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
#include "br_util.h"
#include "br_btn_data.h"
#include "br_snd.h"

//外部公開
#include "br_btn.h"

//-------------------------------------
///	デバッグ
//=====================================
#ifdef PM_DEBUG
//#define DEBUG_MOVESYNC_MODIFY
#endif //PM_DEBUG

//定数をデバッグ操作で変更可能にするためのマクロ
#ifdef DEBUG_MOVESYNC_MODIFY
#define VALUE_CONST( name, value )  static int name = value;
#else
#define VALUE_CONST( name, value )  static const int name = value;
#endif //DEBUG_MOVESYNC_MODIFY

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

VALUE_CONST( BR_BTN_MOVE_DEFAULT_SYNC, 26 )
VALUE_CONST( BR_BTN_MOVE_HIDE_SYNC, 10 )
VALUE_CONST( BR_BTN_MOVE_APPEAR_SYNC, 16 )
VALUE_CONST( BR_BTN_MOVE_UPTAG_M_SYNC, 14 )
VALUE_CONST( BR_BTN_MOVE_UPTAG_S_SYNC, 14 )

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
  BR_BTN_PARAM_NONE_PROC,   //押せないときの動作 BR_BTN_NONEPROC列挙
  BR_BTN_PARAM_NONE_DATA,   //押せないときの動作のデータ（BR_BTN_NONEPROC列挙のコメント参照）
  BR_BTN_PARAM_UNIQUE,      //固有ボタン

  BR_BTN_PARAM_X,
  BR_BTN_PARAM_Y,
} BR_BTN_PARAM;

//-------------------------------------
///	
//=====================================
#define BR_BTN_OAMFONT_OFS_X	(4*8)
#define BR_BTN_OAMFONT_OFS_Y	(1*8)

//-------------------------------------
///	ボタン管理システム
//=====================================
//上画面にボタンがいったときの座標
//#define TAG_INIT_POS_UP( n )		( 25 + ( ( 36 ) * ( 4-n ) ) )
#define TAG_INIT_POS_UP( n )		( 169 - n * 16 )

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
  BR_RES_OBJID        use_resID;

  GFL_POINT           *p_follow_pos;

  //画面切り替え用に作りなおす際必要なメンバ
  const BR_RES_WORK   *cp_res;
  HEAPID              heapID;
  GFL_CLUNIT          *p_unit;
  BMPOAM_SYS_PTR		  p_bmpoam;
  STRBUF              *p_strbuf;
};

//-------------------------------------
///	バトルレコーダー　ボタン管理
//=====================================
struct _BR_BTN_SYS_WORK
{
	HEAPID						heapID;       //ヒープID
	BR_RES_WORK				*p_res;		    //リソース
	GFL_CLUNIT				*p_unit;	    //セルユニット
  BR_SEQ_WORK       *p_seq;       //シーケンス
  BR_RES_OBJID      use_resID;    //ボタンに使用するリソースID

	BR_BTN_SYS_STATE	state;	      //ボタン管理の状態
	BR_BTN_SYS_INPUT	input;	      //ボタン入力状態
	BR_BTN_TYPE				next_type;    //次の動作
	BOOL							next_valid;   //次のボタンが押せるかどうか
	u32								next_proc;    //次のプロセス
	u32								next_mode;    //次のプロセスモード

	u32								trg_btn;			//押したボタン
  GFL_POINT         ball_folow_pos; //ボールが追う座標

	u8								btn_now_max;	//現在のボタンバッファ最大
	u8								btn_now_num;	//現在のボタン数
	u8								btn_stack_max;//ボタンスタックバッファ最大
	u8								btn_stack_num;//ボタンスタック現在数
	BR_BTNEX_WORK			*p_btn_stack;	//スタックに積むボタンバッファ
	BR_BTNEX_WORK			*p_btn_now;		//現在のボタンバッファ

	BR_BTN_DATA_SYS		*p_btn_data;	//ボタンデータバッファ
	BMPOAM_SYS_PTR		p_bmpoam;	    //BMPOAMシステム
  PRINT_QUE         *p_que;       //プリントキュー 

  BR_TEXT_WORK      *p_text;      //文字表示

  BR_EXIT_TYPE      exit_type;    //文字列
  BR_BALLEFF_WORK   *p_balleff;

  BR_BTN_SYS_RECOVERY_DATA  *p_recovery;  //復帰データ
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

static void Br_Btn_Sys_PushRecoveryData( BR_BTN_SYS_RECOVERY_DATA *p_wk, u16 menuID, u16 btnID );
static void Br_Btn_Sys_PopRecoveryData( BR_BTN_SYS_RECOVERY_DATA *p_wk );

typedef enum
{ 
  BR_BTN_SYS_CHANGE_PALLETE_OFFSET_STACK_MODE_UP,
  BR_BTN_SYS_CHANGE_PALLETE_OFFSET_STACK_MODE_DOWN,
}BR_BTN_SYS_CHANGE_PALLETE_OFFSET_STACK_MODE;
static void Br_Btn_Sys_ChangePalleteOffsetStack( BR_BTN_SYS_WORK *p_wk, BR_BTN_SYS_CHANGE_PALLETE_OFFSET_STACK_MODE mode );

//-------------------------------------
///	SEQFUNC
//    次のボタンへいくとき
//      Touch -> HideBtn -> ChangePage -> UpTag -> AppearBtn
//
//    戻るとき
//      Touch -> LeaveBtn -> DownTag -> ChangePage -> AppearBtn
//
//    次のPROCへいくとき
//      Touch -> HideBtn -> ChangePage -> UpTag
//
//=====================================
static void SEQFUNC_Start( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_Touch( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_HideBtn( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_HideBtn2( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_ChangePage( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_AppearBtn( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_LeaveBtn( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_UpTag( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_DownTag( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_Decide( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_NotPushMessage( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

static void SEQFUNC_Open( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_Close( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
///	BTNの処理
//=====================================
static void BR_BTNEX_Init( BR_BTNEX_WORK *p_wk, const BR_BTN_DATA *cp_data, GFL_CLUNIT *p_unit, BMPOAM_SYS_PTR p_bmpoam, const BR_RES_WORK *cp_res, const STRBUF *cp_strbuf, BR_RES_OBJID use_resID, HEAPID heapID );
static void BR_BTNEX_Exit( BR_BTNEX_WORK *p_wk );
static BOOL BR_BTNEX_GetTrg( const BR_BTNEX_WORK *cp_wk, u32 x, u32 y );
static void BR_BTNEX_StartMove( BR_BTNEX_WORK *p_wk, BR_BTN_MOVE move, const GFL_POINT *cp_target );
static BOOL BR_BTNEX_MainMove( BR_BTNEX_WORK *p_wk );
static void BR_BTNEX_ChangeDisplay( BR_BTNEX_WORK *p_wk, CLSYS_DRAW_TYPE display );
static void BR_BTNEX_Copy( const BR_BTN_SYS_WORK *cp_wk, BR_BTNEX_WORK *p_dst, const BR_BTNEX_WORK *cp_src, CLSYS_DRAW_TYPE	display );
static void BR_BTNEX_SetStackPos( BR_BTNEX_WORK *p_wk, u16 stack_num );
static void BR_BTNEX_GetPos( const BR_BTNEX_WORK *cp_wk, GFL_POINT *p_pos );
static void BR_BTNEX_SetPosY( BR_BTNEX_WORK *p_wk, s16 y );
static u32 BR_BTNEX_GetParam( const BR_BTNEX_WORK *cp_wk, BR_BTN_PARAM param );
static void BR_BTNEX_SetSoftPriority( BR_BTNEX_WORK *p_wk, u16 soft_pri );
static void BR_BTNEX_SetBgPriority( BR_BTNEX_WORK *p_wk, u16 bg_pri );
static void BR_BTNEX_SetPalletOffset( BR_BTNEX_WORK *p_wk, u16 ofs );
static void BR_BTNEX_SetFollowPtr( BR_BTNEX_WORK *p_wk, GFL_POINT *p_follow_pos );

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
  //一番上
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
 *  @param	BR_SAVE_INFO		バトル録画情報
 *  @param  BR_BTN_SYS_RECOVERY_DATA  復帰データ
 *	@param	HEAPID heapID		ヒープID
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
BR_BTN_SYS_WORK *BR_BTN_SYS_Init( BR_MENUID menuID, GFL_CLUNIT *p_unit, BR_RES_WORK *p_res, const BR_SAVE_INFO  *cp_saveinfo, BR_BTN_SYS_RECOVERY_DATA *p_recovery, BR_BALLEFF_WORK *p_balleff, HEAPID heapID )
{	
	BR_BTN_SYS_WORK *p_wk;

	GF_ASSERT_HEAVY( menuID < BR_MENUID_MAX );

	//ワーク作成
	{	
		p_wk	= GFL_HEAP_AllocMemory( heapID, sizeof(BR_BTN_SYS_WORK) );
		GFL_STD_MemClear( p_wk, sizeof(BR_BTN_SYS_WORK) );
		p_wk->p_res				= p_res;
		p_wk->p_unit			= p_unit;
		p_wk->heapID			= heapID;
    p_wk->p_recovery  = p_recovery;
		p_wk->p_bmpoam		= BmpOam_Init( heapID, p_unit);
    p_wk->p_que       = PRINTSYS_QUE_Create( heapID );
    p_wk->p_balleff   = p_balleff;
    p_wk->state = BR_BTN_SYS_STATE_BTN_MOVE;

    {
      BR_BTN_DATA_SETUP setup;
      GFL_STD_MemClear( &setup, sizeof(BR_BTN_DATA_SETUP) );
      setup.cp_saveinfo = cp_saveinfo;
      p_wk->p_btn_data	= BR_BTN_DATA_SYS_Init( &setup, heapID );
    }
	}

	//リソース読み込み
	{	
    if( menuID >= BR_MUSICAL_MENUID_TOP )
    {
      p_wk->use_resID = BR_RES_OBJ_MUSICAL_BTN_M;
    }
    else
    { 
      p_wk->use_resID = BR_RES_OBJ_BROWSE_BTN_M;
    }

    BR_RES_LoadOBJ( p_wk->p_res, p_wk->use_resID + CLSYS_DRAW_MAIN, heapID );
    BR_RES_LoadOBJ( p_wk->p_res, p_wk->use_resID + CLSYS_DRAW_SUB, heapID );
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

  //モジュール作成
  p_wk->p_seq = BR_SEQ_Init( p_wk, SEQFUNC_Start, heapID );


	//最初に読み込むボタンを初期化
  if( p_wk->p_recovery->stack_num != 0 )
  { 
    //戻りの場合

    GFL_MSGDATA *p_msg		= BR_RES_GetMsgData( p_res );

    STRBUF  *p_strbuf;
    const BR_BTN_DATA *	cp_data;
    BR_BTNEX_WORK	btn;

    int i;
    //スタック
    for( i = 0; i < p_wk->p_recovery->stack_num; i++ )
    { 
      cp_data	= BR_BTN_DATA_SYS_GetData( p_wk->p_btn_data, 
          p_wk->p_recovery->stack[ i ].menuID, 
          p_wk->p_recovery->stack[ i ].btnID );

      p_strbuf  = BR_BTN_DATA_CreateString( p_wk->p_btn_data, cp_data, p_msg, GFL_HEAP_LOWID(heapID) );

      GFL_STD_MemClear( &btn, sizeof(BR_BTNEX_WORK) );
      BR_BTNEX_Init( &btn, cp_data, p_unit, p_wk->p_bmpoam, p_res, p_strbuf, p_wk->use_resID, heapID );
      Br_Btn_Sys_PushStack( p_wk, &btn, CLSYS_DRAW_MAIN );

      GFL_STR_DeleteBuffer( p_strbuf );
    }
    //座標を設定
    for( i = 0; i < p_wk->btn_stack_num; i++ )
    {	
      BR_BTNEX_SetStackPos( &p_wk->p_btn_stack[p_wk->btn_stack_num - i - 1], i );
    }

    //上部にあるタグが落ちる演出
    BR_SEQ_SetNext( p_wk->p_seq, SEQFUNC_DownTag );
    p_wk->next_type = BR_BTN_TYPE_RETURN;
  }
  else
  { 
    //開始の場合
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

      BR_BTNEX_Init( &p_wk->p_btn_now[i], cp_data, p_unit, p_wk->p_bmpoam, p_res, p_strbuf, p_wk->use_resID, heapID );
      GFL_STR_DeleteBuffer( p_strbuf );
    }
    //座標を設定
    for( i = 0; i < p_wk->btn_now_num; i++ )
    {	
      BR_BTNEX_SetPosY( &p_wk->p_btn_now[i], 192+32 );
    }

    BR_SEQ_SetNext( p_wk->p_seq, SEQFUNC_Open );
  }

#if 0
	{	
		//読み込むボタンのリンク情報を検知し、途中のものであればスタックに積む
		{	
			u32 num	= BR_BTN_DATA_SYS_GetLinkMax( p_wk->p_btn_data, menuID );

			if( num != 0 )
			{	
        //繋がりがあるとき

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
					BR_BTNEX_Init( &btn, cp_data, p_unit, p_wk->p_bmpoam, p_res, p_strbuf, p_wk->use_resID, heapID );
					Br_Btn_Sys_PushStack( p_wk, &btn, CLSYS_DRAW_MAIN );

          GFL_STR_DeleteBuffer( p_strbuf );
				}
        //座標を設定
        for( i = 0; i < p_wk->btn_stack_num; i++ )
        {	
          BR_BTNEX_SetStackPos( &p_wk->p_btn_stack[p_wk->btn_stack_num - i - 1], i );
        }
      }
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

          BR_BTNEX_Init( &p_wk->p_btn_now[i], cp_data, p_unit, p_wk->p_bmpoam, p_res, p_strbuf, p_wk->use_resID, heapID );
          GFL_STR_DeleteBuffer( p_strbuf );
        }

        
      }
		}
	}
#endif

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
  BR_SEQ_Exit( p_wk->p_seq );

  //テキスト面破棄
  if( p_wk->p_text )
  { 
    BR_TEXT_Exit( p_wk->p_text, p_wk->p_res );
    p_wk->p_text  = NULL;
  }

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
    BR_RES_UnLoadOBJ( p_wk->p_res, p_wk->use_resID + CLSYS_DRAW_MAIN );
    BR_RES_UnLoadOBJ( p_wk->p_res, p_wk->use_resID + CLSYS_DRAW_SUB );
	}

	BR_BTN_DATA_SYS_Exit( p_wk->p_btn_data );
	BmpOam_Exit( p_wk->p_bmpoam );
  PRINTSYS_QUE_Delete( p_wk->p_que );

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
	BR_SEQ_Main( p_wk->p_seq );

  //プリント
  PRINTSYS_QUE_Main( p_wk->p_que );

  if( p_wk->p_text )
  { 
    BR_TEXT_PrintMain( p_wk->p_text );
  }


#ifdef DEBUG_MOVESYNC_MODIFY
  {
    static int s_modify_mode  = 0;
    static const char * scp_name_tbl[]  =
    {
      "デフォルト",
      "隠れるとき",
      "出現時",
      "上昇時上",
      "上昇時↓",
    };
    int *p_value_tbl[]  =
    {
      &BR_BTN_MOVE_DEFAULT_SYNC,
      &BR_BTN_MOVE_HIDE_SYNC,
      &BR_BTN_MOVE_APPEAR_SYNC,
      &BR_BTN_MOVE_UPTAG_M_SYNC,
      &BR_BTN_MOVE_UPTAG_S_SYNC,
    };
    const int trg = GFL_UI_KEY_GetTrg();
    BOOL is_update  = FALSE;

    if( trg & PAD_BUTTON_L )
    {
      if( s_modify_mode > 0 )
      {
        s_modify_mode--;
      }
      is_update = TRUE;
    }
    else if( trg & PAD_BUTTON_R )
    {
      if( s_modify_mode < NELEMS(scp_name_tbl )-1 )
      {
        s_modify_mode++;
      }
      is_update = TRUE;
    }
    else if( trg & PAD_KEY_UP )
    {
      (*p_value_tbl[ s_modify_mode])++;
      is_update = TRUE;
    }
    else if( trg & PAD_KEY_DOWN )
    {
      (*p_value_tbl[ s_modify_mode])--;
      is_update = TRUE;
    }

    if( is_update )
    {
      OS_TPrintf( "%s=%d\n", scp_name_tbl[ s_modify_mode ], *p_value_tbl[ s_modify_mode] );
    }
  }
#endif
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
//----------------------------------------------------------------------------
/**
 *	@brief	バトルレコーダー	終了タイプを取得
 *
 *	@param	const BR_BTN_SYS_WORK *cp_wk  ワーク
 *
 *	@return 終了タイプを取得
 */
//-----------------------------------------------------------------------------
BR_EXIT_TYPE BR_BTN_SYS_GetExitType( const BR_BTN_SYS_WORK *cp_wk )
{ 
  return cp_wk->exit_type;
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
	GF_ASSERT_HEAVY( p_wk->btn_stack_num < p_wk->btn_stack_max );

	//積む
	BR_BTNEX_Copy( p_wk, &p_wk->p_btn_stack[p_wk->btn_stack_num], p_btn, display );
	p_wk->btn_stack_num++;

  //優先度を設定
  {
    int i;
    for( i = 0; i < p_wk->btn_stack_num; i++ )
    { 
      //内部でOBJと文字OAMの両方に設定しているので i*2+1
      BR_BTNEX_SetSoftPriority( &p_wk->p_btn_stack[i], (p_wk->btn_stack_num-i)*2+1 );

      //メッセージ表示するため
      BR_BTNEX_SetBgPriority( &p_wk->p_btn_stack[i], 1 );
    }
  }
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

			BR_BTNEX_Init( &p_wk->p_btn_now[i], cp_data, p_wk->p_unit, p_wk->p_bmpoam, p_wk->p_res, p_strbuf, p_wk->use_resID, p_wk->heapID );

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

//----------------------------------------------------------------------------
/**
 *	@brief  復帰データに設定
 *
 *	@param	BR_BTN_SYS_RECOVERY_DATA *p_wk  復帰データ
 *	@param	menuID  メニューID
 *	@param	btnID   ボタンID
 */
//-----------------------------------------------------------------------------
static void Br_Btn_Sys_PushRecoveryData( BR_BTN_SYS_RECOVERY_DATA *p_wk, u16 menuID, u16 btnID )
{ 
  GF_ASSERT_HEAVY( p_wk->stack_num < BR_BTN_SYS_STACK_MAX );

  p_wk->stack[ p_wk->stack_num ].menuID = menuID;
  p_wk->stack[ p_wk->stack_num ].btnID = btnID;
  p_wk->stack_num++;
}
//----------------------------------------------------------------------------
/**
 *	@brief  復帰データから除去
 *
 *	@param	BR_BTN_SYS_RECOVERY_DATA *p_wk 復帰データ
 */
//-----------------------------------------------------------------------------
static void Br_Btn_Sys_PopRecoveryData( BR_BTN_SYS_RECOVERY_DATA *p_wk )
{ 
	if( p_wk->stack_num != 0 )
	{	
    p_wk->stack_num--;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	スタックのボタンを位置によりパレットオフセットを変える
 *
 *	@param	BR_BTN_SYS_WORK *p_wk ワーク
 *	@param  モード
 */
//-----------------------------------------------------------------------------
static void Br_Btn_Sys_ChangePalleteOffsetStack( BR_BTN_SYS_WORK *p_wk, BR_BTN_SYS_CHANGE_PALLETE_OFFSET_STACK_MODE mode )
{
  int i;
  BOOL is_on;
  for( i = 0; i < p_wk->btn_stack_num; i++ )
  {
    switch( mode )
    {
    case BR_BTN_SYS_CHANGE_PALLETE_OFFSET_STACK_MODE_UP:
      is_on = (i == p_wk->btn_stack_num - 1);
      break;
    case BR_BTN_SYS_CHANGE_PALLETE_OFFSET_STACK_MODE_DOWN:
      is_on = (i >= p_wk->btn_stack_num - 2);
      break;
    }

    if( is_on )
    { 
      BR_BTNEX_SetPalletOffset( &p_wk->p_btn_stack[i], 0 );
    }
    else
    { 
      BR_BTNEX_SetPalletOffset( &p_wk->p_btn_stack[i], 1 );
    }
  }
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
 *	@param	BR_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Start( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 

  BR_BTN_SYS_WORK *p_wk = p_wk_adrs;
  BR_SEQ_SetNext( p_seqwk, SEQFUNC_Touch );
}
//----------------------------------------------------------------------------
/**
 *	@brief	タッチメイン
 *
 *	@param	BR_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Touch( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  int i;
  u32 x, y;
  BOOL is_trg	= FALSE;

  BR_BTN_SYS_WORK *p_wk = p_wk_adrs;

  p_wk->input	= BR_BTN_SYS_INPUT_NONE;
  p_wk->state = BR_BTN_SYS_STATE_WAIT;

  //ボタン押し検知
  if( GFL_UI_TP_GetPointTrg( &x, &y ) )
  {	
    for( i = 0; i < p_wk->btn_now_max; i++ )
    {	
      if( BR_BTNEX_GetTrg( &p_wk->p_btn_now[i], x, y ) )
      {	
        //押せるかどうか
        BOOL is_valid = BR_BTNEX_GetParam(&p_wk->p_btn_now[i],BR_BTN_PARAM_VALID );
        if( is_valid )
        { 
          if( p_wk->p_text )
          { 
            BR_TEXT_Exit( p_wk->p_text, p_wk->p_res );
            p_wk->p_text  = NULL;
          }
          PMSND_PlaySE( BR_SND_SE_OK );
          //押せるのでおした
          p_wk->trg_btn	= i;
          is_trg				= TRUE;
          {
            p_wk->ball_folow_pos.x  = x;
            p_wk->ball_folow_pos.y  = y;
            BR_BALLEFF_StartMove( p_wk->p_balleff, BR_BALLEFF_MOVE_EMIT_FOLLOW, &p_wk->ball_folow_pos );

            BR_BTNEX_SetFollowPtr( &p_wk->p_btn_now[p_wk->trg_btn], &p_wk->ball_folow_pos );
          }
          p_wk->exit_type = BR_EXIT_TYPE_RETURN;
          break;
        }
        else
        { 
          //押せないときの動作
          BR_BTN_NONEPROC proc  = BR_BTNEX_GetParam(&p_wk->p_btn_now[i],BR_BTN_PARAM_NONE_PROC );
          u16 proc_data  = BR_BTNEX_GetParam(&p_wk->p_btn_now[i],BR_BTN_PARAM_NONE_DATA );
          PMSND_PlaySE( BR_SND_SE_NG );
          switch( proc )
          { 
          case BR_BTN_NONEPROC_NOPUSH:
            p_wk->trg_btn	= i;
            BR_SEQ_SetNext( p_seqwk, SEQFUNC_NotPushMessage );
            break;
          }
          break;
        }
      }
    }
  }

  //ボタン動作開始
  if( is_trg )
  {	

    p_wk->state = BR_BTN_SYS_STATE_BTN_MOVE;

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
    case BR_BTN_TYPE_RETURN:
      BR_SEQ_SetNext( p_seqwk, SEQFUNC_LeaveBtn );
      break;

    case BR_BTN_TYPE_SELECT_MSG:
      /* fallthrough */
    case BR_BTN_TYPE_SELECT:
      /* fallthrough */
    case BR_BTN_TYPE_CHANGESEQ:
      BR_SEQ_SetNext( p_seqwk, SEQFUNC_HideBtn );
      break;

    case BR_BTN_TYPE_EXIT:
      BR_SEQ_SetNext( p_seqwk, SEQFUNC_Close );
      break;

    default:
      BR_SEQ_SetNext( p_seqwk, SEQFUNC_Decide );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	ボタンが隠れる
 *
 *	@param	BR_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_HideBtn( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
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
        if( p_wk->trg_btn != i )
        {	
          BR_BTNEX_GetPos( &p_wk->p_btn_now[p_wk->trg_btn], &pos );
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
      BR_SEQ_SetNext( p_seqwk, SEQFUNC_ChangePage );
      break;

    default:
      BR_SEQ_SetNext( p_seqwk, SEQFUNC_ChangePage );
      break;
    }
    break;
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief	ボタンが隠れる  ターゲットは降りてきたもの
 *
 *	@param	BR_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_HideBtn2( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
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
      BR_SEQ_SetNext( p_seqwk, SEQFUNC_ChangePage );
      break;

    default:
      BR_SEQ_SetNext( p_seqwk, SEQFUNC_ChangePage );
      break;
    }
    break;
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief	ページ切り替え演出
 *
 *	@param	BR_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_ChangePage( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  BR_BTN_SYS_WORK *p_wk = p_wk_adrs;

  //読み替え
  if( p_wk->next_type == BR_BTN_TYPE_SELECT || p_wk->next_type == BR_BTN_TYPE_SELECT_MSG )
  {
    //次へ行く場合、押したボタンから次への情報を得る
    u32 nextID					= p_wk->next_proc; 
    GFL_POINT pos;

    //消す前に座標をもらう
    BR_BTNEX_GetPos( &p_wk->p_btn_now[p_wk->trg_btn], &pos );

    //決定したボタンをスタックに積む
    Br_Btn_Sys_PushStack( p_wk, &p_wk->p_btn_now[p_wk->trg_btn], CLSYS_DRAW_SUB );
    Br_Btn_Sys_PushRecoveryData( p_wk->p_recovery, 
        BR_BTNEX_GetParam( &p_wk->p_btn_now[p_wk->trg_btn], BR_BTN_PARAM_MENUID ),
        p_wk->trg_btn );

    //積んだので、元の場所の情報を消す(コピーしてるので開放はしない)
    GFL_STD_MemClear( &p_wk->p_btn_now[p_wk->trg_btn], sizeof(BR_BTNEX_WORK) );

    //他のボタンを読み変える
    pos.y = -32;
    Br_Btn_Sys_ReLoadBtn( p_wk, nextID, &pos );

    BR_SEQ_SetNext( p_seqwk, SEQFUNC_UpTag );
  }
  else if( p_wk->next_type == BR_BTN_TYPE_RETURN )
  { 
    //戻る場合、タグから次への情報を得る
    BR_BTNEX_WORK	btn;

    Br_Btn_Sys_PopRecoveryData( p_wk->p_recovery );
    if( Br_Btn_Sys_PopStack( p_wk, &btn ) )
    {	
      u32 menuID  = BR_BTNEX_GetParam( &btn, BR_BTN_PARAM_MENUID );

      GFL_POINT pos;
      GF_ASSERT_HEAVY( btn.is_use );
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

      BR_SEQ_SetNext( p_seqwk, SEQFUNC_AppearBtn );
    }
  } 
  else
  { 
    //決定したボタンをスタックに積む
    Br_Btn_Sys_PushStack( p_wk, &p_wk->p_btn_now[p_wk->trg_btn], CLSYS_DRAW_SUB );
    Br_Btn_Sys_PushRecoveryData( p_wk->p_recovery, 
        BR_BTNEX_GetParam( &p_wk->p_btn_now[p_wk->trg_btn], BR_BTN_PARAM_MENUID ),
        p_wk->trg_btn );

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


    BR_SEQ_SetNext( p_seqwk, SEQFUNC_UpTag );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	出現
 *
 *	@param	BR_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_AppearBtn( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
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
    case BR_BTN_TYPE_SELECT_MSG:
      /* fallthrough */
    case BR_BTN_TYPE_SELECT:
      BR_SEQ_SetNext( p_seqwk, SEQFUNC_Decide );
      break;

    case BR_BTN_TYPE_CHANGESEQ:
      BR_SEQ_SetNext( p_seqwk, SEQFUNC_UpTag ); 
      break;

    default:
      BR_SEQ_SetNext( p_seqwk, SEQFUNC_Decide );
      break;
    }
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンが下へ去る
 *
 *	@param	BR_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_LeaveBtn( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_LEAVE_START,   //ボタンがさっていく開始
    SEQ_LEAVE_WAIT,    //ボタンがさっていく待機
    SEQ_END,
  };
  BR_BTN_SYS_WORK *p_wk = p_wk_adrs;

 switch( *p_seq )
  { 
  case SEQ_LEAVE_START:
    //ボタンが去る動作開始
    { 
      int i;
      GFL_POINT pos;
      for( i = 0; i < p_wk->btn_now_num; i++ )
      {	
        BR_BTNEX_GetPos( &p_wk->p_btn_now[i], &pos );
        pos.y = 192 + 32;
        BR_BTNEX_StartMove( &p_wk->p_btn_now[i], BR_BTN_MOVE_NEXT_TARGET, &pos );
      }
    }
    *p_seq  = SEQ_LEAVE_WAIT;
    break;
  
  case SEQ_LEAVE_WAIT:
    //ボタンが去るまでの動作
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

    BR_SEQ_SetNext( p_seqwk, SEQFUNC_DownTag );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	タグが上に
 *
 *	@param	BR_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_UpTag( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_UP_START, //ボタン出現処理
    SEQ_UP_WAIT,  //ボタン出現待機
    SEQ_MSG_PRINT,  //テキスト表示（ボタンによる）
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
        Br_Btn_Sys_ChangePalleteOffsetStack( p_wk, BR_BTN_SYS_CHANGE_PALLETE_OFFSET_STACK_MODE_UP );
        *p_seq  = SEQ_MSG_PRINT;
			}
		}
    break;
    
  case SEQ_MSG_PRINT:  //テキスト表示（ボタンによる）
    { 
      const BR_BTNEX_WORK *cp_target_btn  = &p_wk->p_btn_stack[ p_wk->btn_stack_num - 1 ];
      BR_BTN_TYPE  type = BR_BTNEX_GetParam( cp_target_btn, BR_BTN_PARAM_TYPE );
      u16   strID       = BR_BTNEX_GetParam( cp_target_btn, BR_BTN_PARAM_DATA2 );
      if( type == BR_BTN_TYPE_SELECT_MSG )
      { 
        if( p_wk->p_text == NULL )
        { 
          p_wk->p_text  = BR_TEXT_Init( p_wk->p_res, p_wk->p_que, p_wk->heapID );
        }
        BR_TEXT_Print( p_wk->p_text, p_wk->p_res, strID );
      }
    }

    *p_seq  = SEQ_END;
    break;

  case SEQ_END:
    switch( p_wk->next_type )
    {
    case BR_BTN_TYPE_SELECT_MSG:
      /* fallthrough */
    case BR_BTN_TYPE_SELECT:
      BR_SEQ_SetNext( p_seqwk, SEQFUNC_AppearBtn ); 
      break;
    default:
      BR_SEQ_SetNext( p_seqwk, SEQFUNC_Decide ); 
    }
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	タグを下に移動
 *
 *	@param	BR_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_DownTag( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_DOWN_START, //ボタン下降開始
    SEQ_DOWN_WAIT,  //ボタン下降待機
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
      //スタックに積んだ一番まえのボタンは下部へ移動
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
        Br_Btn_Sys_ChangePalleteOffsetStack( p_wk, BR_BTN_SYS_CHANGE_PALLETE_OFFSET_STACK_MODE_DOWN );
        *p_seq  = SEQ_END;
			}
		}
    break;
    
  case SEQ_END:
    BR_SEQ_SetNext( p_seqwk, SEQFUNC_ChangePage ); 
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	入力決定
 *
 *	@param	BR_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Decide( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
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
      case BR_BTN_TYPE_SELECT_MSG:
      /* fallthrough */
      case BR_BTN_TYPE_SELECT:				//選択用ボタン
        /* fallthrough */
      case BR_BTN_TYPE_RETURN:				//1つ前のメニューへ戻る用ボタン
        break;

      case BR_BTN_TYPE_EXIT:					//バトルレコーダー終了用ボタン
        p_wk->input	= BR_BTN_SYS_INPUT_EXIT;
        break;

			case BR_BTN_TYPE_CHANGESEQ:		//シーケンス変更ボタン			
        p_wk->input	= BR_BTN_SYS_INPUT_CHANGESEQ;
        break;
      }
    }

    BR_SEQ_SetNext( p_seqwk, SEQFUNC_Touch ); 
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	押せないメッセージ表示
 *
 *	@param	BR_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					  シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_NotPushMessage( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_TOUCH,
    SEQ_END,
  };

  BR_BTN_SYS_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_INIT:
    { 
      u32 strID = BR_BTNEX_GetParam(&p_wk->p_btn_now[p_wk->trg_btn],BR_BTN_PARAM_NONE_DATA );

      if( p_wk->p_text == NULL )
      { 
          p_wk->p_text  = BR_TEXT_Init( p_wk->p_res, p_wk->p_que, p_wk->heapID );
      }
      BR_TEXT_Print( p_wk->p_text, p_wk->p_res, strID );
    }
    *p_seq  = SEQ_TOUCH;
    break;

  case SEQ_TOUCH:
    *p_seq  = SEQ_END;
    break;

  case SEQ_END:
    BR_SEQ_SetNext( p_seqwk, SEQFUNC_Touch );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	開く
 *
 *	@param	BR_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					  シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Open( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_OPEN_START,   //ボタンがさっていく開始
    SEQ_OPEN_WAIT,    //ボタンがさっていく待機
    SEQ_END,
  };
  BR_BTN_SYS_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_OPEN_START:
    //ボタンが登る動作開始
    { 
      int i;
      GFL_POINT pos;
      for( i = 0; i < p_wk->btn_now_num; i++ )
      {	
        pos.x    = BR_BTNEX_GetParam( &p_wk->p_btn_now[i], BR_BTN_PARAM_X );
        pos.y    = BR_BTNEX_GetParam( &p_wk->p_btn_now[i], BR_BTN_PARAM_Y );
        BR_BTNEX_StartMove( &p_wk->p_btn_now[i], BR_BTN_MOVE_NEXT_TARGET, &pos );
      }
    }
    *p_seq  = SEQ_OPEN_WAIT;
    break;
  
  case SEQ_OPEN_WAIT:
    //ボタンが登るまでの動作
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
    BR_SEQ_SetNext( p_seqwk, SEQFUNC_Start );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	閉じる
 *
 *	@param	BR_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					  シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Close( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_CLOSE_START,   //ボタンがさっていく開始
    SEQ_CLOSE_WAIT,    //ボタンがさっていく待機
    SEQ_END,
  };
  BR_BTN_SYS_WORK *p_wk = p_wk_adrs;

 switch( *p_seq )
  { 
  case SEQ_CLOSE_START:
    //ボタンが去る動作開始
    { 
      int i;
      GFL_POINT pos;
      for( i = 0; i < p_wk->btn_now_num; i++ )
      {	
        BR_BTNEX_GetPos( &p_wk->p_btn_now[i], &pos );
        pos.y = 192 + 32;
        BR_BTNEX_StartMove( &p_wk->p_btn_now[i], BR_BTN_MOVE_NEXT_TARGET, &pos );
      }
    }
    *p_seq  = SEQ_CLOSE_WAIT;
    break;
  
  case SEQ_CLOSE_WAIT:
    //ボタンが去るまでの動作
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
    BR_SEQ_SetNext( p_seqwk, SEQFUNC_Decide );
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
 *	@param  use_resID             使用OBJ番号
 *	@param	heapID								ヒープID
 */
//-----------------------------------------------------------------------------
static void BR_BTNEX_Init( BR_BTNEX_WORK *p_wk, const BR_BTN_DATA *cp_data, GFL_CLUNIT *p_unit, BMPOAM_SYS_PTR p_bmpoam, const BR_RES_WORK *cp_res, const STRBUF *cp_strbuf, BR_RES_OBJID use_resID, HEAPID heapID )
{	
	u32 plt;

	GF_ASSERT_HEAVY( p_wk->is_use == FALSE );

	GFL_STD_MemClear( p_wk, sizeof(BR_BTNEX_WORK) );
	p_wk->is_use	= TRUE;
	p_wk->cp_data	= cp_data;
  p_wk->display = CLSYS_DRAW_SUB;

  p_wk->use_resID = use_resID;
  p_wk->cp_res  = cp_res;
  p_wk->heapID  = heapID;
  p_wk->p_unit  = p_unit;
  p_wk->p_bmpoam  = p_bmpoam;
  p_wk->p_strbuf = GFL_STR_CreateCopyBuffer( cp_strbuf, heapID );

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

		//リソース読み込み()
		BR_RES_GetOBJRes( cp_res, p_wk->use_resID + CLSYS_DRAW_SUB, &res );
		p_font	= BR_RES_GetFont( cp_res );

		//作成
		p_wk->p_btn		= BR_BTN_InitEx( &cldata, p_wk->p_strbuf, BR_BTN_DATA_WIDTH, p_wk->display, p_unit, 
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
    GFL_STR_DeleteBuffer( p_wk->p_strbuf );
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

  return BR_BTN_GetHit( cp_wk->p_btn, x, y );
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

    //追尾用の変数へ座標を設定する
    if( p_wk->p_follow_pos )
    { 
      s16 x, y;
      BR_BTN_GetPos( p_wk->p_btn, &x, &y );

      p_wk->p_follow_pos->x = x + BR_BTN_DATA_WIDTH/2;
      p_wk->p_follow_pos->y = y;
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
    u16 soft_pri, bg_pri;

    p_wk->display = display;

    //新しく作り直すため、消去
		{	
      BR_BTN_GetPos( p_wk->p_btn, &x, &y );
      soft_pri  = BR_BTN_GetSoftPriority( p_wk->p_btn );
      bg_pri    = BR_BTN_GetBgPriority( p_wk->p_btn );
			BR_BTN_Exit( p_wk->p_btn );
		}

    //いらないデータを破棄
    {
      p_wk->p_follow_pos  = NULL;
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
      cldata.softpri  = soft_pri;
      cldata.bgpri    = bg_pri;

      //リソース読み込み
      BR_RES_GetOBJRes( p_wk->cp_res, p_wk->use_resID + display, &res );
      p_font	= BR_RES_GetFont( p_wk->cp_res );
      p_msg		= BR_RES_GetMsgData( p_wk->cp_res );
      msgID		= BR_BTN_DATA_GetParam( p_wk->cp_data, BR_BTN_DATA_PARAM_MSGID );


      //作成
      p_wk->p_btn		= BR_BTN_InitEx( &cldata, p_wk->p_strbuf, BR_BTN_DATA_WIDTH, display, p_wk->p_unit, 
          p_wk->p_bmpoam, p_font, &res, p_wk->heapID );
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

	GF_ASSERT_HEAVY( p_dst->is_use == FALSE );
	GF_ASSERT_HEAVY( cp_src->is_use == TRUE );
		
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
static void BR_BTNEX_GetPos( const BR_BTNEX_WORK *cp_wk, GFL_POINT *p_pos )
{ 
  s16 x, y;
  BR_BTN_GetPos( cp_wk->p_btn, &x, &y );
  p_pos->x  = x;
  p_pos->y  = y;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ボタンのY位置を設定
 *
 *	@param	const BR_BTNEX_WORK *cp_wk  ワーク
 *	@param	y Y座標
 */
//-----------------------------------------------------------------------------
static void BR_BTNEX_SetPosY( BR_BTNEX_WORK *p_wk, s16 y )
{ 
  s16 x;
	x	= BR_BTN_DATA_GetParam( p_wk->cp_data, BR_BTN_DATA_PARAM_X );
  BR_BTN_SetPos( p_wk->p_btn, x, y );
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


  case BR_BTN_PARAM_NONE_PROC:
		ret = BR_BTN_DATA_GetParam( cp_wk->cp_data, BR_BTN_DATA_PARAM_NONE_PROC );
    break;

  case BR_BTN_PARAM_NONE_DATA:
		ret = BR_BTN_DATA_GetParam( cp_wk->cp_data, BR_BTN_DATA_PARAM_NONE_DATA );
    break;
	
	case BR_BTN_PARAM_VALID:
		ret = BR_BTN_DATA_GetParam( cp_wk->cp_data, BR_BTN_DATA_PARAM_VALID );
		break;

	case BR_BTN_PARAM_MENUID:
		ret = BR_BTN_DATA_GetParam( cp_wk->cp_data, BR_BTN_DATA_PARAM_MENUID );
		break;

  case BR_BTN_PARAM_UNIQUE:
		ret = BR_BTN_DATA_GetParam( cp_wk->cp_data, BR_BTN_DATA_PARAM_UNIQUE );
    break;

  case BR_BTN_PARAM_X:
		ret = BR_BTN_DATA_GetParam( cp_wk->cp_data, BR_BTN_DATA_PARAM_X );
    break;

  case BR_BTN_PARAM_Y:
		ret = BR_BTN_DATA_GetParam( cp_wk->cp_data, BR_BTN_DATA_PARAM_Y );
    break;

	default:
		ret	= 0;
		GF_ASSERT_HEAVY(0);
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
 *	@brief  BGプライオリティ設定
 *
 *	@param	BR_BTNEX_WORK *p_wk   ワーク
 *	@param	bg_pri                BG優先度
 */
//-----------------------------------------------------------------------------
static void BR_BTNEX_SetBgPriority( BR_BTNEX_WORK *p_wk, u16 bg_pri )
{ 
  BR_BTN_SetBgPriority( p_wk->p_btn, bg_pri );
}
//----------------------------------------------------------------------------
/**
 *	@brief  パレット設定
 *
 *	@param	BR_BTNEX_WORK *p_wk ワーク  
 *	@param	ofs                 パレットオフセット
 */
//-----------------------------------------------------------------------------
static void BR_BTNEX_SetPalletOffset( BR_BTNEX_WORK *p_wk, u16 ofs )
{ 
  BR_BTN_SetPalleteOffset( p_wk->p_btn, ofs );
}
//----------------------------------------------------------------------------
/**
 *	@brief  追尾するための座標を格納する変数を設定する
 *
 *	@param	BR_BTNEX_WORK *p_wk ワーク
 *	@param	*p_follow_pos       追尾座標格納変数
 */
//-----------------------------------------------------------------------------
static void BR_BTNEX_SetFollowPtr( BR_BTNEX_WORK *p_wk, GFL_POINT *p_follow_pos )
{ 
  p_wk->p_follow_pos  = p_follow_pos;
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
      GFL_POINT pos;

      BR_BTNEX_GetPos( p_wk, &pos );
      p_wk->start       = pos.y;
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
      GFL_POINT pos;
      u16 end_y;

      BR_BTNEX_GetPos( p_wk, &pos );
      end_y  = BR_BTN_DATA_GetParam( p_wk->cp_data, BR_BTN_DATA_PARAM_Y ); 

      p_wk->start       = pos.y;
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
      GFL_POINT pos;

      BR_BTNEX_GetPos( p_wk, &pos );
      //end_y  = p_wk->target.y;

      p_wk->start       = pos.y;
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
      GFL_POINT pos;
      BR_BTNEX_GetPos( p_wk, &pos );

      p_wk->start       = pos.y;
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
      GFL_POINT pos;
      BR_BTNEX_GetPos( p_wk, &pos );

      p_wk->start       = pos.y;
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
      GFL_POINT pos;

      BR_BTNEX_GetPos( p_wk, &pos );

      p_wk->start       = pos.y;
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
      GFL_POINT pos;

      BR_BTNEX_GetPos( p_wk, &pos );
      p_wk->start       = pos.y;
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
		PRINTSYS_PrintColor( p_wk->p_bmp, 0, 0, cp_strbuf, p_font, PRINTSYS_LSB_Make( 0xF, 0xE, 0 ) );
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
    actdata.pal_offset  = 1;
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
  BOOL ret;
  ret = BR_BTN_GetHit( cp_wk, x, y );

  if( ret )
  { 
    PMSND_PlaySE( BR_SND_SE_OK );
  }

  return ret;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンと点の衝突判定  (GetTrgとの違いはSEがなるかならないかです)
 *
 *	@param	const BR_BTN_WORK *cp_wk	ワーク
 *	@param	x													タッチ座標X
 *	@param	y													タッチ座標Y
 *
 *	@return	TRUEで衝突	FALSEで衝突していない
 */
//-----------------------------------------------------------------------------
BOOL BR_BTN_GetHit( const BR_BTN_WORK *cp_wk, u32 x, u32 y )
{	
	GFL_RECT rect;
	GFL_CLACTPOS	pos;
  BOOL ret;

	GFL_CLACT_WK_GetPos( cp_wk->p_clwk, &pos, cp_wk->display );

	rect.left		= pos.x;
	rect.right	= pos.x + cp_wk->w;
	rect.top		= pos.y - BR_BTN_DATA_HEIGHT/2;
	rect.bottom	= pos.y + BR_BTN_DATA_HEIGHT/2;

  ret = ( ((u32)( x - rect.left) <= (u32)(rect.right - rect.left))
            & ((u32)( y - rect.top) <= (u32)(rect.bottom - rect.top)));

  return ret;
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

//----------------------------------------------------------------------------
/**
 *	@brief  BGとのプライオリティを設定
 *
 *	@param	BR_BTN_WORK *p_wk ワーク
 *	@param	bg_pri          プライオリティ
 */
//-----------------------------------------------------------------------------
void BR_BTN_SetBgPriority( BR_BTN_WORK *p_wk, u8 bg_pri )
{ 
  GFL_CLACT_WK_SetBgPri( p_wk->p_clwk, bg_pri );
  BmpOam_ActorSetBgPriority( p_wk->p_oamfont, bg_pri );

}

//----------------------------------------------------------------------------
/**
 *	@brief  BGとのプライオリティ取得
 *
 *	@param	const BR_BTN_WORK *cp_wk  ワーク
 *
 *	@return プライオリティ
 */
//-----------------------------------------------------------------------------
u8 BR_BTN_GetBgPriority( const BR_BTN_WORK *cp_wk )
{ 
  return GFL_CLACT_WK_GetBgPri( cp_wk->p_clwk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  パレットチェンジ
 *
 *	@param	BR_BTN_WORK *p_wk ワーク
 *	@param	ofs               オフセット
 */
//-----------------------------------------------------------------------------
void BR_BTN_SetPalleteOffset( BR_BTN_WORK *p_wk, u16 ofs )
{ 
  GFL_CLACT_WK_SetPlttOffs( p_wk->p_clwk, ofs, CLWK_PLTTOFFS_MODE_OAM_COLOR );
  BmpOam_ActorSetPaletteOffset( p_wk->p_oamfont, ofs + 1 );
}
