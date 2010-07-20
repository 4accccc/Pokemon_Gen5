//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_bvsend_proc.c
 *	@brief	バトルビデオ送信プロセス
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
#include "arc_def.h"
#include "msg/msg_battle_rec.h"

//内部モジュール
#include "br_util.h"
#include "br_snd.h"

//外部参照
#include "br_bvsend_proc.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	バトルビデオ送信メインワーク
//=====================================
typedef struct 
{
  PRINT_QUE             *p_que;
  BR_SEQ_WORK           *p_seq;
	HEAPID                heapID;
  BR_TEXT_WORK          *p_text;
  BR_BALLEFF_WORK       *p_balleff[ CLSYS_DRAW_MAX ];
  BR_BVSEND_PROC_PARAM	*p_param;
  u32                   cnt;
} BR_BVSEND_WORK;


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	BRコアプロセス
//=====================================
static GFL_PROC_RESULT BR_BVSEND_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BR_BVSEND_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BR_BVSEND_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
//-------------------------------------
///	シーケンス
//=====================================
static void Br_BvSend_Seq_FadeIn( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_BvSend_Seq_FadeOut( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_BvSend_Seq_Upload( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );


//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//-------------------------------------
///	バトルビデオ送信プロセスプロセス
//=====================================
const GFL_PROC_DATA BR_BVSEND_ProcData =
{	
	BR_BVSEND_PROC_Init,
	BR_BVSEND_PROC_Main,
	BR_BVSEND_PROC_Exit,
};

//=============================================================================
/**
 *					データ
 */
//=============================================================================

//=============================================================================
/**
 *					バトルビデオ送信プロセス
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	バトルビデオ送信プロセス	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_BVSEND_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	BR_BVSEND_WORK				*p_wk;
	BR_BVSEND_PROC_PARAM	*p_param	= p_param_adrs;

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(BR_BVSEND_WORK), BR_PROC_SYS_GetHeapID( p_param->p_procsys ) );
	GFL_STD_MemClear( p_wk, sizeof(BR_BVSEND_WORK) );	
  p_wk->p_param = p_param;
	p_wk->heapID	= BR_PROC_SYS_GetHeapID( p_param->p_procsys );

  //モジュール初期化
  p_wk->p_seq = BR_SEQ_Init( p_wk, Br_BvSend_Seq_FadeIn, p_wk->heapID );
  p_wk->p_que   = PRINTSYS_QUE_Create( p_wk->heapID );
  p_wk->p_text  = BR_TEXT_Init( p_param->p_res, p_wk->p_que, p_wk->heapID );
  BR_TEXT_Print( p_wk->p_text, p_param->p_res, msg_716 );

  {
    int i;
    for( i = 0; i < CLSYS_DRAW_MAX; i++ )
    { 
      p_wk->p_balleff[i] = BR_BALLEFF_Init( p_param->p_unit, p_param->p_res, i, p_wk->heapID );
    }
  }
  { 
    GFL_POINT pos = 
    { 
      256/2,
      192/2,
    };
    BR_BALLEFF_StartMove( p_wk->p_balleff[ CLSYS_DRAW_MAIN ], BR_BALLEFF_MOVE_BIG_CIRCLE, &pos );
  }

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルビデオ送信プロセス	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_BVSEND_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	BR_BVSEND_WORK				*p_wk	= p_wk_adrs;
	BR_BVSEND_PROC_PARAM	*p_param	= p_param_adrs;

	//モジュール破棄
  {
    int i;
    for( i = 0; i < CLSYS_DRAW_MAX; i++ )
    { 
      BR_BALLEFF_Exit( p_wk->p_balleff[i] );
    }
  }
  BR_TEXT_Exit( p_wk->p_text, p_param->p_res );
  PRINTSYS_QUE_Delete( p_wk->p_que );
  BR_SEQ_Exit( p_wk->p_seq );

	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルビデオ送信プロセス	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_BVSEND_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	BR_BVSEND_WORK	*p_wk	= p_wk_adrs;

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

  //シーケンス
  BR_SEQ_Main( p_wk->p_seq );
  if( BR_SEQ_IsEnd( p_wk->p_seq ) )
  { 
    return GFL_PROC_RES_FINISH;
  }

  //ボール演出
  {
    int i;
    for( i = 0; i < CLSYS_DRAW_MAX; i++ )
    { 
      BR_BALLEFF_Main( p_wk->p_balleff[i] );
    }
  }

  //表示
  PRINTSYS_QUE_Main( p_wk->p_que );
  BR_TEXT_PrintMain( p_wk->p_text );

	return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *  シーケンス
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  フェードイン
 *
 *	@param	BR_SEQ_WORK *p_seqwk    シーケンスワーク
 *	@param	*p_seq                  シーケンス変数
 *	@param	*p_wk_adrs              ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Br_BvSend_Seq_FadeIn( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_FADEIN_START,
    SEQ_FADEIN_WAIT,
    SEQ_FADEIN_END,
  };

  BR_BVSEND_WORK	*p_wk	= p_wk_adrs;

	//プロセス処理
  switch( *p_seq )
  { 
  case SEQ_FADEIN_START:
    BR_FADE_StartFade( p_wk->p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_IN );
    *p_seq  = SEQ_FADEIN_WAIT;
    break;

  case SEQ_FADEIN_WAIT:
    if( BR_FADE_IsEnd( p_wk->p_param->p_fade ) )
    { 
      *p_seq  = SEQ_FADEIN_END;
    }
    break;

  case SEQ_FADEIN_END:
    BR_SEQ_SetNext( p_seqwk, Br_BvSend_Seq_Upload );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  フェードアウト
 *
 *	@param	BR_SEQ_WORK *p_seqwk    シーケンスワーク
 *	@param	*p_seq                  シーケンス変数
 *	@param	*p_wk_adrs              ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Br_BvSend_Seq_FadeOut( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_FADEOUT_START,
    SEQ_FADEOUT_WAIT,
    SEQ_FADEOUT_END,
  };

  BR_BVSEND_WORK	*p_wk	= p_wk_adrs;

  //プロセス処理
  switch( *p_seq )
  { 
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
    BR_PROC_SYS_Pop( p_wk->p_param->p_procsys );
    BR_SEQ_End( p_seqwk );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  バトルビデオアップロード
 *
 *	@param	BR_SEQ_WORK *p_seqwk    シーケンスワーク
 *	@param	*p_seq                  シーケンス変数
 *	@param	*p_wk_adrs              ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Br_BvSend_Seq_Upload( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_LOAD_BTLVIDEO,
    SEQ_UPLOAD_START,
    SEQ_UPLOAD_WAIT,
    SEQ_UPLOAD_END,

    SEQ_MSG_WAIT,
  };

  BR_BVSEND_WORK  *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_LOAD_BTLVIDEO:
    { 
      GAMEDATA  *p_gamedata = p_wk->p_param->p_gamedata;
      SAVE_CONTROL_WORK *p_sv_ctrl  = GAMEDATA_GetSaveControlWork( p_gamedata );

      LOAD_RESULT result;
      BattleRec_Load( p_sv_ctrl, HEAPID_BATTLE_RECORDER_SYS, &result, LOADDATA_MYREC ); 
      if( result == LOAD_RESULT_OK )
      { 
        *p_seq  = SEQ_UPLOAD_START;
      }
      else
      { 
        GF_ASSERT(0);
        BR_SEQ_SetNext( p_seqwk, Br_BvSend_Seq_FadeOut );
      }
    }
    break;

  case SEQ_UPLOAD_START:
    p_wk->cnt = 0;
    BR_NET_StartRequest( p_wk->p_param->p_net, BR_NET_REQUEST_BATTLE_VIDEO_UPLOAD, NULL );
    *p_seq  = SEQ_UPLOAD_WAIT;
    break;

  case SEQ_UPLOAD_WAIT:
    p_wk->cnt++;
    if( BR_NET_WaitRequest( p_wk->p_param->p_net ) )
    { 
      u64 number  = 0;
      BR_NET_ERR_RETURN err;
      int msg;

      if( p_wk->cnt < RR_SEARCH_SE_FRAME )
      {
        break;
      }

      err = BR_NET_GetError( p_wk->p_param->p_net, &msg );


      if( err == BR_NET_ERR_RETURN_NONE )
      { 
        if( BR_NET_GetUploadBattleVideoNumber( p_wk->p_param->p_net, &number ) )
        { 
          OS_TPrintf( "送信したビデオナンバー%d\n", number );

          //おくりました
          {
            GFL_MSGDATA *p_msg  = BR_RES_GetMsgData( p_wk->p_param->p_res );
            WORDSET     *p_word = BR_RES_GetWordSet( p_wk->p_param->p_res );
            STRBUF  *p_src    = GFL_MSG_CreateString( p_msg, msg_info_026 );
            STRBUF  *p_strbuf = GFL_STR_CreateBuffer( 128, GFL_HEAP_LOWID(HEAPID_BATTLE_RECORDER_SYS) );

            u32 block[3];

            BR_TOOL_GetVideoNumberToBlock( number, block, 3 );

            WORDSET_RegisterNumber( p_word, 2, block[0], 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
            WORDSET_RegisterNumber( p_word, 1, block[1], 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
            WORDSET_RegisterNumber( p_word, 0, block[2], 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
            WORDSET_ExpandStr( p_word, p_strbuf, p_src );
            BR_TEXT_PrintBuff( p_wk->p_text, p_wk->p_param->p_res, p_strbuf );

            GFL_STR_DeleteBuffer( p_strbuf );
            GFL_STR_DeleteBuffer( p_src );
          }
        }
      }
      else if( err == BR_NET_ERR_RETURN_DISCONNECT )
      {
        BR_PROC_SYS_Abort( p_wk->p_param->p_procsys );
        BR_SEQ_End( p_seqwk );
        return;
      }
      else
      { 
        BR_TEXT_Print( p_wk->p_text, p_wk->p_param->p_res, msg );
      }
      BR_BALLEFF_StartMove( p_wk->p_balleff[ CLSYS_DRAW_MAIN ], BR_BALLEFF_MOVE_NOP, NULL );
      *p_seq  = SEQ_MSG_WAIT;
    }
    break;

  case SEQ_UPLOAD_END:
    BR_SEQ_SetNext( p_seqwk, Br_BvSend_Seq_FadeOut );
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

      *p_seq  = SEQ_UPLOAD_END;
    }
    break;
  }
}
