//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_bvsave_proc.c
 *	@brief	バトルビデオ保存プロセス
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

//アーカイブ
#include "msg/msg_battle_rec.h"

//内部モジュール
#include "br_util.h"
#include "br_snd.h"
#include "br_btn.h"
#include "br_res.h"

//外部参照
#include "br_bvsave_proc.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	MSGWIN個数
//=====================================
enum
{
  BR_BVSAVE_MSGWINID_S_YES,
  BR_BVSAVE_MSGWINID_S_NO,
  BR_BVSAVE_MSGWINID_S_MAX,
};

//-------------------------------------
///	ボタンインデックス
//=====================================
typedef enum
{
	BR_BVSAVE_SAVE_BTNID_START,
	BR_BVSAVE_SAVE_BTNID_SAVE_01 = BR_BVSAVE_SAVE_BTNID_START,
	BR_BVSAVE_SAVE_BTNID_SAVE_02,
	BR_BVSAVE_SAVE_BTNID_SAVE_03,
	BR_BVSAVE_SAVE_BTNID_RETURN,
	BR_BVSAVE_SAVE_BTNID_MAX,

	BR_BVSAVE_BTNID_MAX,
} BR_BVSAVE_MAIN_BTNID;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	バトルビデオ消去メインワーク  定義
//=====================================
typedef struct _BR_BVSAVE_WORK BR_BVSAVE_WORK;

//-------------------------------------
///	画面生成インターフェイス
//=====================================
typedef struct 
{
  void (*Br_BvSave_Create)(BR_BVSAVE_WORK *p_wk, BR_BVSAVE_PROC_PARAM *p_param );
  void (*Br_BvSave_Delete)(BR_BVSAVE_WORK *p_wk, BR_BVSAVE_PROC_PARAM *p_param );
} BR_BVSAVE_DISPLAY_INTERFACE;

//-------------------------------------
///	バトルビデオ消去メインワーク  実装
//=====================================
struct _BR_BVSAVE_WORK
{
  BMPOAM_SYS_PTR		    p_bmpoam;	//BMPOAMシステム
  PRINT_QUE             *p_que;   //プリントキュー
  BR_TEXT_WORK          *p_text;  //テキスト
  BR_SEQ_WORK           *p_seq;
  u32                   save_btn_idx;

  BR_BTN_WORK	          *p_btn[ BR_BVSAVE_BTNID_MAX ];
  BR_MSGWIN_WORK        *p_msgwin_s[ BR_BVSAVE_MSGWINID_S_MAX ];
  BR_BALLEFF_WORK       *p_balleff[ CLSYS_DRAW_MAX ];
	HEAPID                heapID;
  BR_BVSAVE_PROC_PARAM	*p_param;

  u16                   work0;
  u16                   work1;
  u32                   cnt;
  u32                   text_cnt;

  BR_BVSAVE_DISPLAY_INTERFACE display;
};


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	BR保存プロセス
//=====================================
static GFL_PROC_RESULT BR_BVSAVE_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BR_BVSAVE_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BR_BVSAVE_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
//-------------------------------------
///	シーケンス
//=====================================
static void Br_BvSave_Seq_VideoDownloadSave( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_BvSave_Seq_SaveSelect( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_BvSave_Seq_OverWrite( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_BvSave_Seq_Save( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
///	画面生成
//=====================================
static void BR_BVSAVE_CreateDisplay( BR_BVSAVE_DISPLAY_INTERFACE *p_diplay, BR_BVSAVE_WORK *p_wk, BR_BVSAVE_PROC_PARAM *p_param );
static void BR_BVSAVE_DeleteDisplay( BR_BVSAVE_DISPLAY_INTERFACE *p_diplay, BR_BVSAVE_WORK *p_wk, BR_BVSAVE_PROC_PARAM *p_param );

//-------------------------------------
///	画面作成
//=====================================
//上書き確認
static BR_BVSAVE_DISPLAY_INTERFACE Br_BvSave_OverWrite_Create( void );

//ダウンロード画面
static BR_BVSAVE_DISPLAY_INTERFACE Br_BvSave_Download_Create( void );

//保存画面
static BR_BVSAVE_DISPLAY_INTERFACE Br_BvSave_Save_Create( void );

//-------------------------------------
///	その他
//=====================================
static BOOL Br_BvSave_GetTrgYes( BR_BVSAVE_WORK * p_wk, u32 x, u32 y );
static BOOL Br_BvSave_GetTrgNo( BR_BVSAVE_WORK * p_wk, u32 x, u32 y );

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//-------------------------------------
///	バトルビデオ保存プロセス
//=====================================
const GFL_PROC_DATA BR_BVSAVE_ProcData =
{	
	BR_BVSAVE_PROC_Init,
	BR_BVSAVE_PROC_Main,
	BR_BVSAVE_PROC_Exit,
};

//=============================================================================
/**
 *					データ
 */
//=============================================================================

//=============================================================================
/**
 *					バトルビデオ保存プロセス
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	バトルビデオ保存プロセス	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_BVSAVE_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	BR_BVSAVE_WORK				*p_wk;
	BR_BVSAVE_PROC_PARAM	*p_param	= p_param_adrs;

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(BR_BVSAVE_WORK), BR_PROC_SYS_GetHeapID( p_param->p_procsys ) );
	GFL_STD_MemClear( p_wk, sizeof(BR_BVSAVE_WORK) );	
  p_wk->p_param = p_param;
	p_wk->heapID	= BR_PROC_SYS_GetHeapID( p_param->p_procsys );

  //モジュール初期化
  p_wk->p_bmpoam		= BmpOam_Init( p_wk->heapID, p_param->p_unit);
  p_wk->p_seq = BR_SEQ_Init( p_wk, Br_BvSave_Seq_VideoDownloadSave, p_wk->heapID );
  p_wk->p_que = PRINTSYS_QUE_Create( p_wk->heapID );

  { 
    int i;
    for( i = 0; i < CLSYS_DRAW_MAX; i++ )
    { 
      p_wk->p_balleff[i] = BR_BALLEFF_Init( p_param->p_unit, p_param->p_res, i, p_wk->heapID );
    }
  }

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルビデオ保存プロセス	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_BVSAVE_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	BR_BVSAVE_WORK				*p_wk	= p_wk_adrs;
	BR_BVSAVE_PROC_PARAM	*p_param	= p_param_adrs;


  BR_BVSAVE_DeleteDisplay( &p_wk->display, p_wk, p_wk->p_param );

  { 
    int i;
    for( i = 0; i < CLSYS_DRAW_MAX; i++ )
    { 
      BR_BALLEFF_Exit( p_wk->p_balleff[i] );
    }
  }

#ifdef BUGFIX_GFBTS1996_20100719
  if( p_wk->p_text )
  { 
    BR_TEXT_Exit( p_wk->p_text, p_wk->p_param->p_res );
    p_wk->p_text  = NULL;
  }
#endif //BUGFIX_GFBTS1996_20100719

	//モジュール破棄
  PRINTSYS_QUE_Delete( p_wk->p_que );
  BR_SEQ_Exit( p_wk->p_seq );
  BmpOam_Exit( p_wk->p_bmpoam );


  BR_PROC_SYS_Pop( p_wk->p_param->p_procsys );

	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルビデオ保存プロセス	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_BVSAVE_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	

	BR_BVSAVE_WORK	*p_wk	= p_wk_adrs;

  //エラーチェック
#ifdef BUGFIX_GFBTS1996_20100719
  if( GAMEDATA_GetIsSave( p_wk->p_param->p_gamedata ) == FALSE )
#endif //BUGFIX_GFBTS1996_20100719
  {
    if( BR_NET_SYSERR_RETURN_DISCONNECT == BR_NET_GetSysError( p_wk->p_param->p_net ) )
    { 
      BR_PROC_SYS_Abort( p_wk->p_param->p_procsys );
      return GFL_PROC_RES_FINISH;
    }
  }

  //シーケンス管理
  BR_SEQ_Main( p_wk->p_seq );
  if( BR_SEQ_IsEnd( p_wk->p_seq ) )
  { 
    return GFL_PROC_RES_FINISH;
  }

  //ボール処理
  { 
    int i;
    for( i = 0; i < CLSYS_DRAW_MAX; i++ )
    { 
      BR_BALLEFF_Main( p_wk->p_balleff[i] );
    }
  }

  //表示
  PRINTSYS_QUE_Main( p_wk->p_que );
  if( p_wk->p_text )
  { 
    BR_TEXT_PrintMain( p_wk->p_text );
  }

  //メッセージウィンドウ
  {
    int i;
    for( i = 0; i < BR_BVSAVE_MSGWINID_S_MAX; i++ )
    {
      if( p_wk->p_msgwin_s[i] )
      { 
        BR_MSGWIN_PrintMain( p_wk->p_msgwin_s[i] );
      }
    }
  }

	return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *  シーケンス
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  録画データ本体をダウンロードをして保存画面へ行く
 *
 *	@param	BR_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                シーケンス変数
 *	@param	*p_wk_adrs            ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Br_BvSave_Seq_VideoDownloadSave( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_CHANGE_MAIN,
    SEQ_CHANGE_FADEIN_START,
    SEQ_CHANGE_FADEIN_WAIT,

    SEQ_DOWNLOAD_START,
    SEQ_DOWNLOAD_WAIT,
    SEQ_DOWNLOAD_END,

    SEQ_FADEOUT_START,
    SEQ_FADEOUT_WAIT,
    SEQ_FADEOUT_END,

    SEQ_MSG_WAIT,

    SEQ_FADEOUT_START_RET,
    SEQ_FADEOUT_WAIT_RET,
    SEQ_FADEOUT_END_RET,

  };

  BR_BVSAVE_WORK	*p_wk	= p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_CHANGE_MAIN:
    //作成
    p_wk->display = Br_BvSave_Download_Create();
    BR_BVSAVE_CreateDisplay( &p_wk->display, p_wk, p_wk->p_param );

    *p_seq  = SEQ_CHANGE_FADEIN_START;
    break;

  case SEQ_CHANGE_FADEIN_START:
    BR_FADE_StartFade( p_wk->p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_IN );
    *p_seq  = SEQ_CHANGE_FADEIN_WAIT;
    break;

  case SEQ_CHANGE_FADEIN_WAIT:
    if( BR_FADE_IsEnd( p_wk->p_param->p_fade ) )
    {
      *p_seq  = SEQ_DOWNLOAD_START;
    }
    break;

  case SEQ_DOWNLOAD_START:
    {
      BR_NET_REQUEST_PARAM  req_param;
      GFL_STD_MemClear( &req_param, sizeof(BR_NET_REQUEST_PARAM) );
      req_param.download_video_number = p_wk->p_param->video_number;
      BR_NET_StartRequest( p_wk->p_param->p_net, BR_NET_REQUEST_BATTLE_VIDEO_DOWNLOAD, &req_param );
      p_wk->cnt = 0;

      *p_seq  = SEQ_DOWNLOAD_WAIT;
    }
    break;
  case SEQ_DOWNLOAD_WAIT:
    p_wk->cnt++;
    if( BR_NET_WaitRequest( p_wk->p_param->p_net ) )
    { 
      if( p_wk->cnt > RR_SEARCH_SE_FRAME )
      { 
        BR_BALLEFF_StartMove( p_wk->p_balleff[ CLSYS_DRAW_MAIN ], BR_BALLEFF_MOVE_NOP, NULL );
        *p_seq  = SEQ_DOWNLOAD_END;
      }
    }
    break;
  case SEQ_DOWNLOAD_END:
    { 
      BR_NET_ERR_RETURN err;
      int msg;

      int video_number;
      BATTLE_REC_RECV       *p_btl_rec;

      err = BR_NET_GetError( p_wk->p_param->p_net, &msg );

      if( err == BR_NET_ERR_RETURN_NONE )
      { 
        if( BR_NET_GetDownloadBattleVideo( p_wk->p_param->p_net, &p_btl_rec, &video_number ) )
        {   
          //受信したので、battle_recを設定
          BattleRec_DataSet( &p_btl_rec->profile, &p_btl_rec->head,
              &p_btl_rec->rec, GAMEDATA_GetSaveControlWork( p_wk->p_param->p_gamedata ) );

          { 
            BOOL is_have  = FALSE;
            int i;
            u64 now_number  = RecHeader_ParamGet( BattleRec_HeaderPtrGet(), RECHEAD_IDX_DATA_NUMBER, 0 );
            //すでに受信しているものかチェック
            for( i = BR_SAVEDATA_OTHER_00; i <= BR_SAVEDATA_OTHER_02; i++ )
            { 
              OS_TPrintf("比較 save[%d]%d%d download%d%d\n", i,
                  (p_wk->p_param->cp_rec_saveinfo->video_number[i] >> 16) & 0xFFFFFFFF, 
                  (p_wk->p_param->cp_rec_saveinfo->video_number[i]) & 0xFFFFFFFF, 
                  (now_number >> 16) & 0xFFFFFFFF, now_number & 0xFFFFFFFF );
              if( p_wk->p_param->cp_rec_saveinfo->video_number[i] == now_number )
              { 
                is_have = TRUE;
                break;
              }
            }

            if( is_have == TRUE )
            { 
              //すでに受信していたのでメッセージをだして、戻る
              BR_TEXT_Print( p_wk->p_text, p_wk->p_param->p_res, msg_info_008 );
              *p_seq  = SEQ_MSG_WAIT;
            }
            else
            { 
              //次へ
              *p_seq  = SEQ_FADEOUT_START;
            }
          }
        }
        else
        { 
          //受信に失敗したので、戻る
          BR_TEXT_Print( p_wk->p_text, p_wk->p_param->p_res, msg_info_031 );
          *p_seq  = SEQ_MSG_WAIT;
        }
      }
      else
      { 
        //エラー
        BR_TEXT_Print( p_wk->p_text, p_wk->p_param->p_res, msg );
        *p_seq  = SEQ_MSG_WAIT;
      }
    }
    break;

  case SEQ_MSG_WAIT:
    if( GFL_UI_TP_GetTrg() )
    {

      { 
        u32 x, y;
        GFL_POINT pos;

        GFL_UI_TP_GetPointTrg( &x, &y );
        pos.x = x;
        pos.y = y;
        BR_BALLEFF_StartMove( p_wk->p_balleff[ CLSYS_DRAW_SUB ], BR_BALLEFF_MOVE_EMIT, &pos );
        PMSND_PlaySE( BR_SND_SE_OK );
      }

      *p_seq  = SEQ_FADEOUT_START_RET;
    }
    break;

    //-------------------------------------
    ///	次へ
    //=====================================
  case SEQ_FADEOUT_START:
    BR_FADE_StartFade( p_wk->p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_OUT );
    *p_seq  = SEQ_FADEOUT_WAIT;
    break;
  case SEQ_FADEOUT_WAIT:
    if( BR_FADE_IsEnd( p_wk->p_param->p_fade ) )
    { 
      *p_seq  = SEQ_FADEOUT_END;
    }
    break;
  case SEQ_FADEOUT_END:
    BR_BVSAVE_DeleteDisplay( &p_wk->display, p_wk, p_wk->p_param );

    BR_SEQ_SetNext( p_seqwk, Br_BvSave_Seq_SaveSelect );
    break;

    //-------------------------------------
    ///	もどる
    //=====================================
  case SEQ_FADEOUT_START_RET:
    BR_FADE_StartFade( p_wk->p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_OUT );
    *p_seq  = SEQ_FADEOUT_WAIT_RET;
    break;
  case SEQ_FADEOUT_WAIT_RET:
    if( BR_FADE_IsEnd( p_wk->p_param->p_fade ) )
    { 
      *p_seq  = SEQ_FADEOUT_END_RET;
    }
    break;
  case SEQ_FADEOUT_END_RET:
    //破棄
    BR_BVSAVE_DeleteDisplay( &p_wk->display, p_wk, p_wk->p_param );
    BR_SEQ_End( p_seqwk );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  保存選択シーケンス
 *
 *	@param	BR_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                シーケンス変数
 *	@param	*p_wk_adrs            ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Br_BvSave_Seq_SaveSelect( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_CHANGE_MAIN,
    SEQ_CHANGE_FADEIN_START,
    SEQ_CHANGE_FADEIN_WAIT,

    //メイン
    SEQ_SAVE_MAIN,

    SEQ_FADEOUT_START,
    SEQ_FADEOUT_WAIT,
    SEQ_FADEOUT_END,

    //上書き
    SEQ_FADEOUT_START_OV,
    SEQ_FADEOUT_WAIT_OV,
    SEQ_FADEOUT_END_OV,

    //戻る
    SEQ_FADEOUT_START_RET,
    SEQ_FADEOUT_WAIT_RET,
    SEQ_FADEOUT_MAIN_RET,
  };

  BR_BVSAVE_WORK	*p_wk	= p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_CHANGE_MAIN:
    //作成
    p_wk->display = Br_BvSave_Save_Create();
    BR_BVSAVE_CreateDisplay( &p_wk->display, p_wk, p_wk->p_param );
    *p_seq  = SEQ_CHANGE_FADEIN_START;
    break;

  case SEQ_CHANGE_FADEIN_START:
    BR_FADE_StartFade( p_wk->p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_IN );
    *p_seq  = SEQ_CHANGE_FADEIN_WAIT;
    break;

  case SEQ_CHANGE_FADEIN_WAIT:
    if( BR_FADE_IsEnd( p_wk->p_param->p_fade ) )
    {
      *p_seq  = SEQ_SAVE_MAIN;
    }
    break;

    //-------------------------------------
    /// ボタン
    //=====================================
  case SEQ_SAVE_MAIN:
    {
      int i;
      u32 idx;
      u32 x, y;
      if( GFL_UI_TP_GetPointTrg( &x, &y ) )
      {
        for( i = BR_BVSAVE_SAVE_BTNID_SAVE_01; i <= BR_BVSAVE_SAVE_BTNID_SAVE_03; i++ )
        { 
          idx = i - BR_BVSAVE_SAVE_BTNID_SAVE_01;

          //保存場所ボタンを押した
          if( BR_BTN_GetTrg( p_wk->p_btn[i], x, y ) )
          {	
            GFL_POINT pos;
            pos.x = x;
            pos.y = y;
            BR_BALLEFF_StartMove( p_wk->p_balleff[ CLSYS_DRAW_SUB ], BR_BALLEFF_MOVE_EMIT, &pos );
            p_wk->save_btn_idx  = idx;
            if( p_wk->p_param->cp_rec_saveinfo->is_valid[BR_SAVEDATA_OTHER_00+idx] )
            { 
              *p_seq  = SEQ_FADEOUT_START_OV;
            }
            else
            { 
              *p_seq  = SEQ_FADEOUT_START;
            }
          }
        }

        //戻るボタンを押した
        if( BR_BTN_GetTrg( p_wk->p_btn[BR_BVSAVE_SAVE_BTNID_RETURN], x, y ) )
        {
          GFL_POINT pos;
          pos.x = x;
          pos.y = y;
          BR_BALLEFF_StartMove( p_wk->p_balleff[ CLSYS_DRAW_SUB ], BR_BALLEFF_MOVE_EMIT, &pos );
          *p_seq  = SEQ_FADEOUT_START_RET;
        }
      }
    }
    break;

    //-------------------------------------
    /// 書き込み
    //=====================================
  case SEQ_FADEOUT_START:
    BR_FADE_StartFade( p_wk->p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_OUT );
    *p_seq  = SEQ_FADEOUT_WAIT;
    break;

  case SEQ_FADEOUT_WAIT:
    if( BR_FADE_IsEnd( p_wk->p_param->p_fade ) )
    { 
      *p_seq  = SEQ_FADEOUT_END;
    }
    break;

  case SEQ_FADEOUT_END:
    //破棄
    BR_BVSAVE_DeleteDisplay( &p_wk->display, p_wk, p_wk->p_param );
    BR_SEQ_SetNext( p_wk->p_seq, Br_BvSave_Seq_Save );
    break;

    //-------------------------------------
    /// オーバーライト
    //=====================================
  case SEQ_FADEOUT_START_OV:
    BR_FADE_StartFade( p_wk->p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_OUT );
    (*p_seq)++;
    break;

  case SEQ_FADEOUT_WAIT_OV:
    if( BR_FADE_IsEnd( p_wk->p_param->p_fade ) )
    { 
      (*p_seq)++;
    }
    break;

  case SEQ_FADEOUT_END_OV:
    BR_BVSAVE_DeleteDisplay( &p_wk->display, p_wk, p_wk->p_param );

    BR_SEQ_SetNext( p_wk->p_seq, Br_BvSave_Seq_OverWrite );
    break;

    //-------------------------------------
    ///	戻るボタン
    //=====================================
  case SEQ_FADEOUT_START_RET:
    BR_FADE_StartFade( p_wk->p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_OUT );
    *p_seq  = SEQ_FADEOUT_WAIT_RET;
    break;

  case SEQ_FADEOUT_WAIT_RET:
    if( BR_FADE_IsEnd( p_wk->p_param->p_fade ) )
    { 
      *p_seq  = SEQ_FADEOUT_MAIN_RET;
    }
    break;

  case SEQ_FADEOUT_MAIN_RET:
    //再構築
    BR_BVSAVE_DeleteDisplay( &p_wk->display, p_wk, p_wk->p_param );
    BR_SEQ_End( p_seqwk );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  上書きシーケンス
 *
 *	@param	BR_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                シーケンス変数
 *	@param	*p_wk_adrs            ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Br_BvSave_Seq_OverWrite( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_CREATE_DISPLAY,
    SEQ_FADEIN_START,
    SEQ_FADEIN_WAIT,

    //メイン
    SEQ_SAVE_MAIN,

    SEQ_FADEOUT_START,
    SEQ_FADEOUT_WAIT,
    SEQ_FADEOUT_END,

    SEQ_FADEOUT_START_RET,
    SEQ_FADEOUT_WAIT_RET,
    SEQ_FADEOUT_END_RET,
  };

  BR_BVSAVE_WORK	*p_wk	= p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_CREATE_DISPLAY:
    p_wk->display = Br_BvSave_OverWrite_Create();
    BR_BVSAVE_CreateDisplay( &p_wk->display, p_wk, p_wk->p_param );

    (*p_seq)++;
    break;
  case SEQ_FADEIN_START:
    BR_FADE_StartFade( p_wk->p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_IN );
    (*p_seq)++;
    break;
  case SEQ_FADEIN_WAIT:
    if( BR_FADE_IsEnd( p_wk->p_param->p_fade ) )
    {
      (*p_seq)++;
    }
    break;

    //メイン
  case SEQ_SAVE_MAIN:
    { 
      u32 x, y;
      if( GFL_UI_TP_GetPointTrg( &x, &y ) )
      {
        if( Br_BvSave_GetTrgYes( p_wk, x, y ) )
        { 
          *p_seq  = SEQ_FADEOUT_START;
        }

        if( Br_BvSave_GetTrgNo( p_wk, x, y ) )
        { 
          *p_seq  = SEQ_FADEOUT_START_RET;
        }
      }
    }
    break;

    //-------------------------------------
    ///	セーブへ
    //=====================================
  case SEQ_FADEOUT_START:
    BR_FADE_StartFade( p_wk->p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_OUT );
    (*p_seq)++;
    break;
  case SEQ_FADEOUT_WAIT:
    if( BR_FADE_IsEnd( p_wk->p_param->p_fade ) )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_FADEOUT_END:
    BR_BVSAVE_DeleteDisplay( &p_wk->display, p_wk, p_wk->p_param );
    BR_SEQ_SetNext( p_seqwk, Br_BvSave_Seq_Save );
    break;

    //-------------------------------------
    ///	戻る
    //=====================================
  case SEQ_FADEOUT_START_RET:
    BR_FADE_StartFade( p_wk->p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_OUT );
    (*p_seq)++;
    break;
  case SEQ_FADEOUT_WAIT_RET:
    if( BR_FADE_IsEnd( p_wk->p_param->p_fade ) )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_FADEOUT_END_RET:
    BR_BVSAVE_DeleteDisplay( &p_wk->display, p_wk, p_wk->p_param );
    BR_SEQ_SetNext( p_seqwk, Br_BvSave_Seq_SaveSelect );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  保存
 *
 *	@param	BR_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                シーケンス変数
 *	@param	*p_wk_adrs            ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Br_BvSave_Seq_Save( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_FADEIN_START,
    SEQ_FADEIN_WAIT,

    //保存処理
    SEQ_SAVE_INIT,
    SEQ_SAVE_WAIT,

    //お気に入り送信処理
    SEQ_SEND_INIT,
    SEQ_SEND_WAIT,
    SEQ_TRG,

    //フェードアウト
    SEQ_FADEOUT_START,
    SEQ_FADEOUT_WAIT,
    SEQ_FADEOUT_MAIN,
  };

  BR_BVSAVE_WORK	*p_wk	= p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_FADEIN_START:
    if( p_wk->p_text == NULL )
    { 
      p_wk->p_text    = BR_TEXT_Init( p_wk->p_param->p_res, p_wk->p_que, p_wk->heapID );
    }
    BR_TEXT_Print( p_wk->p_text, p_wk->p_param->p_res, msg_info_009 );

    {
      GFL_POINT pos;
      pos.x = 256/2;
      pos.y = 192/2;
      BR_BALLEFF_StartMove( p_wk->p_balleff[ CLSYS_DRAW_MAIN ], BR_BALLEFF_MOVE_BIG_CIRCLE, &pos );
    }

    BR_FADE_StartFade( p_wk->p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_IN );
    (*p_seq)++;
    break;

  case SEQ_FADEIN_WAIT:
    if( BR_FADE_IsEnd( p_wk->p_param->p_fade ) )
    {
      (*p_seq)++;
    }
    break;

    //保存処理
  case SEQ_SAVE_INIT:
    p_wk->cnt = 0;
    p_wk->work0 = 0;
    p_wk->work1 = 0;
    (*p_seq)++;
    break;

  case SEQ_SAVE_WAIT:
    { 
      SAVE_RESULT result;

      int save_idx  = BR_SAVEDATA_OTHER_00+p_wk->save_btn_idx;

      result  = BattleRec_GDS_RecvData_Save(p_wk->p_param->p_gamedata, save_idx, p_wk->p_param->is_secure, &p_wk->work0, &p_wk->work1, p_wk->heapID );
      if( result == SAVE_RESULT_OK || result == SAVE_RESULT_NG )
      { 
        p_wk->p_param->is_save  = TRUE; 
        p_wk->p_param->save_idx = BR_SAVEDATA_OTHER_00+p_wk->save_btn_idx;
        (*p_seq)++;
      }
      p_wk->cnt++;
    }
    break;

    //お気に入り送信処理　&& メッセージ表示
  case SEQ_SEND_INIT:
    {
      BR_NET_REQUEST_PARAM  req_param;
      GFL_STD_MemClear( &req_param, sizeof(BR_NET_REQUEST_PARAM) );
      req_param.upload_favorite_video_number  = RecHeader_ParamGet( BattleRec_HeaderPtrGet(), RECHEAD_IDX_DATA_NUMBER, 0 );
      BR_NET_StartRequest( p_wk->p_param->p_net, BR_NET_REQUEST_FAVORITE_VIDEO_UPLOAD, &req_param );
      BR_TEXT_Print( p_wk->p_text, p_wk->p_param->p_res, msg_info_011 );
    }
    p_wk->cnt++;
    p_wk->text_cnt  = 0;
    (*p_seq)++;
    break;

  case SEQ_SEND_WAIT:
    p_wk->cnt++;
    p_wk->text_cnt++;
    if( BR_NET_WaitRequest( p_wk->p_param->p_net ) )
    {
      if( p_wk->cnt > RR_SEARCH_SE_FRAME ) 
      { 
        if( p_wk->text_cnt > 60 ) //msg_info_011のメッセージが一瞬でよくわからないので、最低60フレーム待つ
        {
          BR_BALLEFF_StartMove( p_wk->p_balleff[ CLSYS_DRAW_MAIN ], BR_BALLEFF_MOVE_NOP, NULL );

          BR_TEXT_Print( p_wk->p_text, p_wk->p_param->p_res, msg_info_012 );
          (*p_seq)++;
        }
      }
    }
    break;
  
  case SEQ_TRG:
    if( GFL_UI_TP_GetTrg() )
    { 
      { 
        u32 x, y;
        GFL_POINT pos;

        GFL_UI_TP_GetPointTrg( &x, &y );
        pos.x = x;
        pos.y = y;
        BR_BALLEFF_StartMove( p_wk->p_balleff[ CLSYS_DRAW_SUB ], BR_BALLEFF_MOVE_EMIT, &pos );
        PMSND_PlaySE( BR_SND_SE_OK );
      }

      (*p_seq)++;
    }
    break;

  case SEQ_FADEOUT_START:
    BR_FADE_StartFade( p_wk->p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_OUT );
    (*p_seq)++;
    break;

  case SEQ_FADEOUT_WAIT:
    if( BR_FADE_IsEnd( p_wk->p_param->p_fade ) )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_FADEOUT_MAIN:
    if( p_wk->p_text )
    { 
      BR_TEXT_Exit( p_wk->p_text, p_wk->p_param->p_res );
      p_wk->p_text  = NULL;
    }
    BR_SEQ_End( p_seqwk );
    break;
  }
}


//=============================================================================
/**
 *    画面作成
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  画面生成
 *
 *	@param	BR_BVSAVE_DISPLAY_INTERFACE *p_diplay 画面インターフェイス
 *	@param	*p_wk      ワーク
 *	@param	*p_param  引数
 */
//-----------------------------------------------------------------------------
static void BR_BVSAVE_CreateDisplay( BR_BVSAVE_DISPLAY_INTERFACE *p_diplay, BR_BVSAVE_WORK *p_wk, BR_BVSAVE_PROC_PARAM *p_param )
{
  if( p_diplay->Br_BvSave_Create )
  {
    p_diplay->Br_BvSave_Create( p_wk, p_param );
    p_diplay->Br_BvSave_Create  = NULL;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  画面破棄
 *
 *	@param	BR_BVSAVE_DISPLAY_INTERFACE *p_diplay 画面インターフェイス
 *	@param	*p_wk      ワーク
 *	@param	*p_param  引数
 */
//-----------------------------------------------------------------------------
static void BR_BVSAVE_DeleteDisplay( BR_BVSAVE_DISPLAY_INTERFACE *p_diplay, BR_BVSAVE_WORK *p_wk, BR_BVSAVE_PROC_PARAM *p_param )
{
  if( p_diplay->Br_BvSave_Delete )
  {
    p_diplay->Br_BvSave_Delete( p_wk, p_param );
    p_diplay->Br_BvSave_Delete  = NULL;
  }
}



//----------------------------------------------------------------------------
/**
 *	@brief  上書き画面用インターフェイス作成
 *
 *	@return 上書き画面
 */
//-----------------------------------------------------------------------------
static void Br_BvSave_OverWrite_CreateDisplay( BR_BVSAVE_WORK *p_wk, BR_BVSAVE_PROC_PARAM *p_param );
static void Br_BvSave_OverWrite_DeleteDisplay( BR_BVSAVE_WORK *p_wk, BR_BVSAVE_PROC_PARAM *p_param );
static BR_BVSAVE_DISPLAY_INTERFACE Br_BvSave_OverWrite_Create( void )
{
  static const BR_BVSAVE_DISPLAY_INTERFACE  overwrite =
  {
    Br_BvSave_OverWrite_CreateDisplay,
    Br_BvSave_OverWrite_DeleteDisplay
  };

  return overwrite;
}

//----------------------------------------------------------------------------
/**
 *	@brief  上書き確認画面作成
 *
 *	@param	BR_BVSAVE_WORK *p_wk  ワーク
 *	@param	*p_param                引数
 */
//-----------------------------------------------------------------------------
static void Br_BvSave_OverWrite_CreateDisplay( BR_BVSAVE_WORK *p_wk, BR_BVSAVE_PROC_PARAM *p_param )
{
  //上画面はテキストだけ
  if( p_wk->p_text == NULL )
  { 
    p_wk->p_text  = BR_TEXT_Init( p_param->p_res, p_wk->p_que, p_wk->heapID );
  }

  { 
    WORDSET *p_word     = BR_RES_GetWordSet( p_param->p_res );
    GFL_MSGDATA *p_msg  = BR_RES_GetMsgData( p_param->p_res );
    STRBUF  *p_src  = GFL_MSG_CreateString( p_msg, msg_info_013 );
    STRBUF  *p_dst  = GFL_STR_CreateBuffer( 128, p_wk->heapID );

    WORDSET_RegisterWord( p_word, 0,
        p_param->cp_rec_saveinfo->p_name[ BR_SAVEDATA_OTHER_00 + p_wk->save_btn_idx],
        p_param->cp_rec_saveinfo->sex[ BR_SAVEDATA_OTHER_00 + p_wk->save_btn_idx], 
        TRUE, PM_LANG );

    WORDSET_ExpandStr( p_word, p_dst, p_src );

    BR_TEXT_PrintBuff( p_wk->p_text, p_param->p_res, p_dst );

    GFL_STR_DeleteBuffer( p_dst );
    GFL_STR_DeleteBuffer( p_src );
  }

  //下画面は専用BG
  BR_RES_LoadBG( p_param->p_res, BR_RES_BG_BVDELETE_S, p_wk->heapID );

  { 
    static const struct 
    { 
      u8 x;
      u8 y;
      u8 w;
      u8 h;
      u32 msgID;
    } sc_msgwin_data[BR_BVSAVE_MSGWINID_S_MAX]  =
    { 
      {
        4,
        6,
        9,
        2,
        msg_1000,
      },
      { 
        18,
        6,
        9,
        2,
        msg_1001,
      },
    };
    int i;

    GFL_FONT    *p_font  = BR_RES_GetFont( p_param->p_res );
    GFL_MSGDATA *p_msg   = BR_RES_GetMsgData( p_param->p_res );
    for( i = 0; i < BR_BVSAVE_MSGWINID_S_MAX; i++ )
    { 
      p_wk->p_msgwin_s[i]  = BR_MSGWIN_Init( BG_FRAME_S_FONT, sc_msgwin_data[i].x, sc_msgwin_data[i].y, sc_msgwin_data[i].w, sc_msgwin_data[i].h, PLT_BG_S_FONT, p_wk->p_que, p_wk->heapID );
      BR_MSGWIN_PrintColor( p_wk->p_msgwin_s[i], p_msg, sc_msgwin_data[i].msgID, p_font, BR_PRINT_COL_NORMAL );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  上書き確認画面破棄
 *
 *	@param	BR_BVSAVE_WORK *p_wk  ワーク
 *	@param	*p_param                引数
 */
//-----------------------------------------------------------------------------
static void Br_BvSave_OverWrite_DeleteDisplay( BR_BVSAVE_WORK *p_wk, BR_BVSAVE_PROC_PARAM *p_param )
{ 
  //メッセージウィンドウ
  {
    int i;
    for( i = 0; i < BR_BVSAVE_MSGWINID_S_MAX; i++ )
    {
      if( p_wk->p_msgwin_s[i] )
      { 
        BR_MSGWIN_Exit( p_wk->p_msgwin_s[i] );
        p_wk->p_msgwin_s[i] = NULL;
      }
    }
  }
  GFL_BG_LoadScreenReq( BG_FRAME_S_FONT );

  //下画面破棄
  BR_RES_UnLoadBG( p_param->p_res, BR_RES_BG_BVDELETE_S );

  //上画面破棄
  if( p_wk->p_text )
  { 
    BR_TEXT_Exit( p_wk->p_text, p_param->p_res );
    p_wk->p_text  = NULL;
  }
}



//----------------------------------------------------------------------------
/**
 *	@brief  ダウンロード用インターフェイス作成
 *
 *	@return ダウンロード要
 */
//-----------------------------------------------------------------------------
static void Br_BvSave_Download_CreateDisplay( BR_BVSAVE_WORK * p_wk, BR_BVSAVE_PROC_PARAM	*p_param );
static void Br_BvSave_Download_DeleteDisplay( BR_BVSAVE_WORK * p_wk, BR_BVSAVE_PROC_PARAM	*p_param );
static BR_BVSAVE_DISPLAY_INTERFACE Br_BvSave_Download_Create( void )
{
  static const BR_BVSAVE_DISPLAY_INTERFACE  download =
  {
    Br_BvSave_Download_CreateDisplay,
    Br_BvSave_Download_DeleteDisplay
  };

  return download;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ダウンロード用の絵を作成
 *
 *	@param	BR_BVSAVE_WORK * p_wk ワーク
 *	@param	                      引数
 */
//-----------------------------------------------------------------------------
static void Br_BvSave_Download_CreateDisplay( BR_BVSAVE_WORK * p_wk, BR_BVSAVE_PROC_PARAM	*p_param )
{ 
  //ダウンロード用の絵を作成
  if( p_wk->p_text == NULL )
  { 
    p_wk->p_text    = BR_TEXT_Init( p_wk->p_param->p_res, p_wk->p_que, p_wk->heapID );
  }
  BR_TEXT_Print( p_wk->p_text, p_wk->p_param->p_res, msg_info_023 );
  {
    GFL_POINT pos;
    pos.x = 256/2;
    pos.y = 192/2;
    BR_BALLEFF_StartMove( p_wk->p_balleff[ CLSYS_DRAW_MAIN ], BR_BALLEFF_MOVE_BIG_CIRCLE, &pos );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ダウンロード用の絵を破棄
 *
 *	@param	BR_BVSAVE_WORK * p_wk ワーク
 *	@param	                      引数
 */
//-----------------------------------------------------------------------------
static void Br_BvSave_Download_DeleteDisplay( BR_BVSAVE_WORK * p_wk, BR_BVSAVE_PROC_PARAM	*p_param )
{ 
  BR_BALLEFF_StartMove( p_wk->p_balleff[ CLSYS_DRAW_MAIN ], BR_BALLEFF_MOVE_NOP, NULL );
  if( p_wk->p_text )
  { 
    BR_TEXT_Exit( p_wk->p_text, p_wk->p_param->p_res );
    p_wk->p_text  = NULL;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  保存画面インターフェイス作成
 *
 *	@return 保存画面
 */
//-----------------------------------------------------------------------------
static void Br_BvSave_Save_CreateDisplay( BR_BVSAVE_WORK * p_wk, BR_BVSAVE_PROC_PARAM	*p_param );

static void Br_BvSave_Save_DeleteDisplay( BR_BVSAVE_WORK * p_wk, BR_BVSAVE_PROC_PARAM	*p_param );
static BR_BVSAVE_DISPLAY_INTERFACE Br_BvSave_Save_Create( void )
{
  static const BR_BVSAVE_DISPLAY_INTERFACE  overwrite =
  {
    Br_BvSave_Save_CreateDisplay,
    Br_BvSave_Save_DeleteDisplay
  };

  return overwrite;
}
//----------------------------------------------------------------------------
/**
 *	@brief  保存選択画面作成
 *
 *	@param	BR_BVSAVE_WORK * p_wk ワーク
 *	@param	BR_BVSAVE_PROC_PARAM  引数
 */
//-----------------------------------------------------------------------------
static void Br_BvSave_Save_CreateDisplay( BR_BVSAVE_WORK * p_wk, BR_BVSAVE_PROC_PARAM	*p_param )
{ 
  enum
  { 
    OAM_POS_INIT_Y  = 40,
    OAM_POS_OFS_Y   = 8+32,
  };

  GFL_FONT    *p_font  = BR_RES_GetFont( p_param->p_res );
  GFL_MSGDATA *p_msg   = BR_RES_GetMsgData( p_param->p_res );
  WORDSET     *p_word  = BR_RES_GetWordSet( p_param->p_res );
  STRBUF      *p_strbuf= GFL_STR_CreateBuffer( 128, p_wk->heapID );    
  STRBUF      *p_src   = GFL_STR_CreateBuffer( 128, p_wk->heapID );    

  int i;
  u32 idx;
  GFL_CLWK_DATA cldata;
  BR_RES_OBJ_DATA res;

  GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

  cldata.pos_x    = 48;
  cldata.pos_y    = OAM_POS_INIT_Y;
  cldata.anmseq   = 11;
  cldata.softpri  = 1;

	BR_RES_LoadOBJ( p_param->p_res, BR_RES_OBJ_BROWSE_BTN_S, p_wk->heapID );
  BR_RES_GetOBJRes( p_param->p_res, BR_RES_OBJ_BROWSE_BTN_S, &res );

  //セーブ箇所ボタン
  for( i = BR_BVSAVE_SAVE_BTNID_START; i < BR_BVSAVE_SAVE_BTNID_RETURN; i++ )
  { 
    idx = BR_SAVEDATA_OTHER_00 + (i - BR_BVSAVE_SAVE_BTNID_START);

    //録画の保存状況によりボタンの文字変更
    if( p_param->cp_rec_saveinfo->is_valid[idx] )
    { 
      //○○の記録
      GFL_MSG_GetString( p_msg, msg_25, p_src );

      //単語登録
      WORDSET_RegisterWord( p_word, 0, 
          p_param->cp_rec_saveinfo->p_name[idx],
          p_param->cp_rec_saveinfo->sex[idx], 
          TRUE, PM_LANG );
        
      WORDSET_ExpandStr( p_word, p_strbuf, p_src );
    }
    else
    { 
      //空き
      GFL_MSG_GetString( p_msg, msg_10, p_strbuf );
    }

    p_wk->p_btn[ i ]  = BR_BTN_InitEx( &cldata, p_strbuf, BR_BTN_DATA_WIDTH, CLSYS_DRAW_SUB, p_param->p_unit, p_wk->p_bmpoam, p_font, &res, p_wk->heapID );


    cldata.pos_y    += OAM_POS_OFS_Y;
  }

  //戻るボタン
  cldata.anmseq   = 1;
  p_wk->p_btn[ BR_BVSAVE_SAVE_BTNID_RETURN ]  = BR_BTN_Init( &cldata, msg_05, BR_BTN_DATA_WIDTH, CLSYS_DRAW_SUB, p_param->p_unit, p_wk->p_bmpoam, p_font, p_msg, &res, p_wk->heapID );

  //上画面
  { 
    GDS_PROFILE_PTR p_profile = BattleRec_GDSProfilePtrGet();
    STRBUF  *p_profile_name = GDS_Profile_CreateNameString( p_profile, GFL_HEAP_LOWID(p_wk->heapID) );

    if( p_wk->p_text == NULL )
    { 
      p_wk->p_text    = BR_TEXT_Init( p_param->p_res, p_wk->p_que, p_wk->heapID );
    }

    //○○の記録
    GFL_MSG_GetString( p_msg, msg_608, p_src );

    //単語登録
    WORDSET_RegisterWord( p_word, 0, 
        p_profile_name,
        GDS_Profile_GetSex( p_profile ), 
        TRUE, PM_LANG );

    WORDSET_ExpandStr( p_word, p_strbuf, p_src );

    BR_TEXT_PrintBuff( p_wk->p_text, p_param->p_res, p_strbuf );

    GFL_STR_DeleteBuffer( p_profile_name );
  }

  GFL_STR_DeleteBuffer( p_src );
  GFL_STR_DeleteBuffer( p_strbuf );
}
//----------------------------------------------------------------------------
/**
 *	@brief  保存選択画面破棄
 *
 *	@param	BR_BVSAVE_WORK * p_wk ワーク
 *	@param	BR_BVSAVE_PROC_PARAM  引数
 */
//-----------------------------------------------------------------------------
static void Br_BvSave_Save_DeleteDisplay( BR_BVSAVE_WORK * p_wk, BR_BVSAVE_PROC_PARAM	*p_param )
{ 
  //上画面破棄
  if( p_wk->p_text )
  { 
    BR_TEXT_Exit( p_wk->p_text, p_param->p_res );
    p_wk->p_text  = NULL;
  }

  //ボタン破棄
  { 
    int i;
    for( i = BR_BVSAVE_SAVE_BTNID_START; i < BR_BVSAVE_SAVE_BTNID_MAX; i++ )
    { 
      if( p_wk->p_btn[i] )
      { 
        BR_BTN_Exit( p_wk->p_btn[i] );
        p_wk->p_btn[i]  = NULL;
      }
    }
  }

  BR_RES_UnLoadOBJ( p_param->p_res, BR_RES_OBJ_BROWSE_BTN_S ); 
}



//----------------------------------------------------------------------------
/**
 *	@brief  はいを選択
 *
 *	@param	x     X座標
 *	@param	y     Y座標
 *
 *	@return TRUE入力  FALSE何もしない
 */
//-----------------------------------------------------------------------------
static BOOL Br_BvSave_GetTrgYes( BR_BVSAVE_WORK * p_wk, u32 x, u32 y )
{ 
	GFL_RECT rect;
  BOOL ret;

	rect.left		= (4)*8;
	rect.right	= (4 + 9)*8;
	rect.top		= (6)*8;
	rect.bottom	= (6 + 2)*8;

  ret = ( ((u32)( x - rect.left) <= (u32)(rect.right - rect.left))
            & ((u32)( y - rect.top) <= (u32)(rect.bottom - rect.top)));

  if( ret )
  { 
    GFL_POINT pos;
    pos.x = x;
    pos.y = y;
    BR_BALLEFF_StartMove( p_wk->p_balleff[ CLSYS_DRAW_SUB ], BR_BALLEFF_MOVE_EMIT, &pos );
    PMSND_PlaySE( BR_SND_SE_OK );
  }

  return ret;
}
//----------------------------------------------------------------------------
/**
 *	@brief  いいえを選択
 *
 *	@param	x     X座標
 *	@param	y     Y座標
 *
 *	@return TRUE入力  FALSE何もしない
 */
//-----------------------------------------------------------------------------
static BOOL Br_BvSave_GetTrgNo( BR_BVSAVE_WORK * p_wk, u32 x, u32 y )
{ 
	GFL_RECT rect;
  BOOL ret;

	rect.left		= (18)*8;
	rect.right	= (18 + 9)*8;
	rect.top		= (6)*8;
	rect.bottom	= (6 + 2)*8;


  ret = ( ((u32)( x - rect.left) <= (u32)(rect.right - rect.left))
            & ((u32)( y - rect.top) <= (u32)(rect.bottom - rect.top)));

  if( ret )
  { 
    GFL_POINT pos;
    pos.x = x;
    pos.y = y;
    BR_BALLEFF_StartMove( p_wk->p_balleff[ CLSYS_DRAW_SUB ], BR_BALLEFF_MOVE_EMIT, &pos );
    PMSND_PlaySE( BR_SND_SE_OK );
  }
  return ret;
}
