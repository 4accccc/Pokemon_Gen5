//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		mb_data_convert_sys.c
 *	@brief  データコンバートメイン処理
 *	@author	Toru=Nagihashi
 *	@data		2011.07.05
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "system/wipe.h"
#include "system/net_err.h"

#include "arc_def.h"
#include "mb_child_gra.naix"
#include "mystery.naix"
#include "multiboot/swan_sound_palpark.sadl"
#include "sound/pm_sndsys.h"

#include "multiboot/mb_util.h"
#include "multiboot/mb_util_msg.h"
#include "multiboot/mb_data_main.h"
#include "multiboot/mb_comm_sys.h"
#include "multiboot/mb_local_def.h"

#include "multiboot/mb_data_convert_sys.h"
#include "mb_data_convert_graphic.h"
#include "arc/arc_extern.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
enum
{
  MB_DATACONV_FRAME_MSG = GFL_BG_FRAME1_M,
  MB_DATACONV_FRAME_BG  = GFL_BG_FRAME2_M,
};

#define MB_DATACONV_FIRST_TIMEOUT (60*30) //通常5秒以内で接続するのができなかった。

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	メインワーク
//=====================================
typedef struct _MB_DATA_CONVERT_WORK  MB_DATA_CONVERT_WORK;

typedef BOOL (*MB_DATACONV_SEQ_FUNC)( MB_DATA_CONVERT_WORK *p_wk, int *p_seq );

struct _MB_DATA_CONVERT_WORK
{
  MB_DATA_CONVERT_GRAPHIC_WORK *p_graphic;
  MB_COMM_INIT_DATA *p_comm_init;
  MB_MSG_WORK   *p_msg;
  MB_COMM_WORK  *p_comm;
  MB_DATA_WORK  *p_data;
  void          *p_snd_buff;
  u32           timeout_cnt;

  MB_DATACONV_SEQ_FUNC seq;
  DLPLAY_CARD_TYPE card_type;

  BOOL          is_net_err;
  HEAPID        heapID;
} ;


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
/// プロセス
//=====================================
static GFL_PROC_RESULT Mb_Data_Convert_Proc_Init
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT Mb_Data_Convert_Proc_Exit
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT Mb_Data_Convert_Proc_Main
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );

//-------------------------------------
/// シーケンス管理
//=====================================
static BOOL Mb_DataConv_Seq_Main( MB_DATA_CONVERT_WORK *p_wk, int *p_seq );
static void Mb_DataConv_Seq_Set( MB_DATA_CONVERT_WORK *p_wk, MB_DATACONV_SEQ_FUNC seq, int *p_seq );

//-------------------------------------
/// シーケンス
//=====================================
static BOOL Mb_DataConv_SeqFunc_FadeIn( MB_DATA_CONVERT_WORK *p_wk, int *p_seq );
static BOOL Mb_DataConv_SeqFunc_Main( MB_DATA_CONVERT_WORK *p_wk, int *p_seq );
static BOOL Mb_DataConv_SeqFunc_FadeOut( MB_DATA_CONVERT_WORK *p_wk, int *p_seq );


//-------------------------------------
/// エラー
//=====================================
static BOOL Mb_DataConv_Err_Check( void );

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	プロセス
//=====================================
const GFL_PROC_DATA MULTIBOOT_DATA_CONVERT_ProcData =
{
  Mb_Data_Convert_Proc_Init,
  Mb_Data_Convert_Proc_Main,
  Mb_Data_Convert_Proc_Exit,
};
//=============================================================================
/**
 *					プロセス
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  SYSプロセス初期化
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Mb_Data_Convert_Proc_Init
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  MB_DATA_CONVERT_WORK *p_wk;

  OS_Printf( "heapID=%d FreeSize=%d\n", GFL_HEAPID_APP, GFL_HEAP_GetHeapFreeSize( GFL_HEAPID_APP ) );
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MULTIBOOT, 0xF8000 );
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MULTIBOOT_DATA, 0x70000 );

  {
    OS_Printf( "messagedl_movie %d\n", _start_message_dl_movie );
  }

#if 0
  {
    int i, j;
    for( i = 0; i < ARCID_TABLE_MAX; i++ )
    {
      OS_Printf( "ARCID %d\n", i );
      for( j = 0; j < GFL_ARC_GetDataFileCnt(i); j++ )
      {
        OS_Printf( "DATID%d FreeSize=%d\n", j, GFL_ARC_GetDataSize(i,j) );
      }
    }
  }
#endif
  

  OS_Printf( "heapID=%d FreeSize=%d\n", GFL_HEAPID_APP, GFL_HEAP_GetHeapFreeSize( GFL_HEAPID_APP ) );

  p_wk = GFL_PROC_AllocWork( p_proc, sizeof(MB_DATA_CONVERT_WORK), HEAPID_MULTIBOOT );
  GFL_STD_MemClear( p_wk, sizeof(MB_DATA_CONVERT_WORK) );
  p_wk->heapID  = HEAPID_MULTIBOOT;
  p_wk->p_graphic = MB_DATA_CONVERT_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, HEAPID_MULTIBOOT );
  p_wk->p_comm  = MB_COMM_CreateSystem( p_wk->heapID );
  p_wk->p_msg   = MB_MSG_MessageInit( p_wk->heapID, MB_DATACONV_FRAME_MSG, MB_DATACONV_FRAME_MSG, FILE_MSGID_MB , TRUE , TRUE );

  MB_MSG_MessageCreateWindow( p_wk->p_msg, MMWT_NORMAL );
  p_wk->p_snd_buff = GFL_ARC_UTIL_Load( ARCID_MB_CHILD ,
      NARC_mb_child_gra_swan_sound_palpark_sdat ,
      FALSE ,
      p_wk->heapID );

  PMSND_InitMultiBoot( p_wk->p_snd_buff );

  Mb_DataConv_Seq_Set( p_wk, Mb_DataConv_SeqFunc_FadeIn, p_seq );

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *  @brief  SYSプロセス破棄
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Mb_Data_Convert_Proc_Exit
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  MB_DATA_CONVERT_WORK *p_wk = p_wk_adrs;

  PMSND_Exit();
  GFL_HEAP_FreeMemory( p_wk->p_snd_buff );
  MB_COMM_DeleteSystem( p_wk->p_comm );

  if( p_wk->p_data != NULL )
  {
    MB_DATA_TermSystem( p_wk->p_data );
  }

  if( p_wk->p_msg != NULL )
  {
    MB_MSG_MessageTerm( p_wk->p_msg );
  }

  MB_DATA_CONVERT_GRAPHIC_Exit( p_wk->p_graphic );

  GFL_PROC_FreeWork( p_proc );
  GFL_HEAP_DeleteHeap( p_wk->heapID );

  NetErr_DispCall(FALSE);

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *  @brief  SYSプロセスメイン{処理
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Mb_Data_Convert_Proc_Main
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  MB_DATA_CONVERT_WORK *p_wk = p_wk_adrs;

  //通信アップデート
  MB_COMM_UpdateSystem( p_wk->p_comm );

  //エラーを感知したらフェードアウトして終了
  if( Mb_DataConv_Err_Check() )
  {
    Mb_DataConv_Seq_Set( p_wk, Mb_DataConv_SeqFunc_FadeOut, p_seq );
  }

  //描画
  MB_DATA_CONVERT_GRAPHIC_2D_Draw( p_wk->p_graphic );

  //サウンドメイン
  PMSND_Main();

  if( p_wk->p_msg != NULL )
  {
    MB_MSG_MessageMain( p_wk->p_msg );
  }

  //シーケンス処理
  if( Mb_DataConv_Seq_Main( p_wk, p_seq ) )
  {
    return GFL_PROC_RES_FINISH;
  }
  else
  {
    return GFL_PROC_RES_CONTINUE;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  シーケンスメイン処理
 *
 *	@param	MB_DATA_CONVERT_WORK *p_wkワーク
 *	@param	*p_seq  シーケンス
 *
 *	@return TRUEで終了  FALSEでまだ
 */
//-----------------------------------------------------------------------------
static BOOL Mb_DataConv_Seq_Main( MB_DATA_CONVERT_WORK *p_wk, int *p_seq )
{
  return p_wk->seq( p_wk, p_seq );
}
//----------------------------------------------------------------------------
/**
 *	@brief  シーケンスをセット
 *
 *	@param	MB_DATA_CONVERT_WORK *p_wk  ワーク
 *	@param	seq 関数
 *	@param	*p_seq  シーケンス
 */
//-----------------------------------------------------------------------------
static void Mb_DataConv_Seq_Set( MB_DATA_CONVERT_WORK *p_wk, MB_DATACONV_SEQ_FUNC seq, int *p_seq )
{
  p_wk->seq = seq;
  *p_seq  = 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  フェードイン
 *
 *	@param	MB_DATA_CONVERT_WORK *p_wk  ワーク
 *	@param	*p_seq  ローカルシーケンス
 *
 *	@return TRUEでPROC終了  FALSEで続行
 */
//-----------------------------------------------------------------------------
static BOOL Mb_DataConv_SeqFunc_FadeIn( MB_DATA_CONVERT_WORK *p_wk, int *p_seq )
{
  enum
  {
    SEQ_START,
    SEQ_WAIT,
    SEQ_NEXT,
  };

  switch( *p_seq )
  {
  case SEQ_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
    (*p_seq)++;
    break;
  case SEQ_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
		  (*p_seq)++;
		}
    break;
  case SEQ_NEXT:
    Mb_DataConv_Seq_Set( p_wk, Mb_DataConv_SeqFunc_Main, p_seq );
    break;
  default:
    GF_ASSERT_HEAVY(0);
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  メイン
 *
 *	@param	MB_DATA_CONVERT_WORK *p_wk  ワーク
 *	@param	*p_seq  ローカルシーケンス
 *
 *	@return TRUEでPROC終了  FALSEで続行
 */
//-----------------------------------------------------------------------------
static BOOL Mb_DataConv_SeqFunc_Main( MB_DATA_CONVERT_WORK *p_wk, int *p_seq )
{
  enum
  {
    SEQ_COMM_INIT,
    SEQ_COMM_WAIT,
    SEQ_CONNECT_WAIT,
    SEQ_CHECK_ROM,
    SEQ_LOAD_DATA,
    SEQ_LOAD_ERR,
    SEQ_DATA_CONV,

    SEQ_TRANS_SAVE_SEND,
    SEQ_TRANS_SAVE_SEND_WAIT,

    SEQ_TRANS_SAVE_FINISH_WAIT,

    SEQ_WAIT_NEXT_GAME_ERROR_MSG,
    SEQ_WAIT_EXIT_COMM,
    SEQ_EXIT_GAME,
  };

  switch( *p_seq )
  {
  case SEQ_COMM_INIT:
    MB_COMM_InitComm( p_wk->p_comm );
    MB_MSG_MessageDispNoWait( p_wk->p_msg, MSG_MB_MOVIE_01 );
    MB_MSG_SetDispTimeIcon( p_wk->p_msg, TRUE );
    (*p_seq)++;
    break;
  case SEQ_COMM_WAIT:
    if( MB_COMM_IsInitComm(p_wk->p_comm) )
    {
      GF_ASSERT_MSG_HEAVY( MB_IsMultiBootChild(),"This DS is not multiboot child!!\n");
      GFL_NET_ReloadIconTopOrBottom(TRUE, p_wk->heapID );

      {
        const MBParentBssDesc *desc = MB_GetMultiBootParentBssDesc();
        MB_COMM_InitChild( p_wk->p_comm, (u8*)desc->bssid );
      }
      
      MB_MSG_MessageDispNoWait( p_wk->p_msg, MSG_MB_MOVIE_01 );
      MB_MSG_SetDispTimeIcon( p_wk->p_msg , TRUE );
      (*p_seq)++;
    }
    break;
  case SEQ_CONNECT_WAIT:
    if( MB_COMM_IsSendEnable( p_wk->p_comm ) == FALSE )
    {
      p_wk->timeout_cnt++;
    }
    if( p_wk->timeout_cnt >= MB_DATACONV_FIRST_TIMEOUT )
    {
      //TODO
      GF_ASSERT( 0 );
    }
    else if( MB_COMM_IsPostInitData( p_wk->p_comm ) )
    {
      p_wk->p_comm_init = MB_COMM_GetInitData( p_wk->p_comm );
      GFL_MSGSYS_SetLangID( p_wk->p_comm_init->strMode );
      (*p_seq)++;
    }
      
    break;

  case SEQ_CHECK_ROM:
    p_wk->p_data = MB_DATA_InitSystem( p_wk->heapID );
    p_wk->card_type = MB_DATA_GetCardType( p_wk->p_data );

    MB_DATA_ResetSaveLoad( p_wk->p_data );

#if PM_DEBUG
    if( p_wk->card_type == CARD_TYPE_DUMMY )
    {
      MB_MSG_MessageDispNoWait( p_wk->p_msg, MSG_MB_MOVIE_DEB_01 );
      MB_DATA_SetCardType( p_wk->p_data, CARD_TYPE_WB );
      (*p_seq)++;
    }
    else
#endif
    if( p_wk->card_type == CARD_TYPE_INVALID )
    {
      //ROM違う！

      MB_MSG_MessageDisp( p_wk->p_msg, MSG_MB_MOVIE_02 , p_wk->p_comm_init->msgSpeed );
      MB_MSG_SetDispKeyCursor( p_wk->p_msg, TRUE );

      MB_COMM_SetChildState( p_wk->p_comm, MCCS_END_GAME_ERROR );
      (*p_seq) = SEQ_WAIT_NEXT_GAME_ERROR_MSG;
    }
    else
    {
      (*p_seq)++;
    }
    break;
  case SEQ_LOAD_DATA:
    if( MB_DATA_LoadDataFirst( p_wk->p_data ) == TRUE )
    {
      if( MB_DATA_GetErrorState( p_wk->p_data ) == DES_NONE )
      {
        (*p_seq) = SEQ_DATA_CONV;
        MB_MSG_MessageCreateWindow( p_wk->p_msg, MMWT_NORMAL );
        MB_MSG_MessageDisp( p_wk->p_msg, MSG_MB_MOVIE_09 , p_wk->p_comm_init->msgSpeed );
      }
      else
      {
        *p_seq = SEQ_LOAD_ERR;
      }
    }
    break;
  case SEQ_LOAD_ERR:
    {
      switch( MB_DATA_GetErrorState( p_wk->p_data ) )
      {
      case DES_MISS_LOAD_BACKUP:
        MB_COMM_SetChildState( p_wk->p_comm, MCCS_END_GAME_ERROR_BACKUP_LOAD );
        break;
      case DES_ANOTHER_COUNTRY:
        MB_COMM_SetChildState( p_wk->p_comm, MCCS_END_GAME_ERROR_ANOTHER_COUNTRY );
        break;
      default:
        MB_COMM_SetChildState( p_wk->p_comm, MCCS_END_GAME_ERROR );
        break;
      }
      (*p_seq) = SEQ_WAIT_NEXT_GAME_ERROR_MSG;
    }
    break;
  case SEQ_DATA_CONV:
    {
      //TODO この当たりでセーブデータをコンバートデータに変換
      (*p_seq)++;
    }
    break;

  case SEQ_TRANS_SAVE_SEND:
    //TODO データとサイズ
    {
      u8 a = 0;
    if( MB_COMM_Send_CommonData( p_wk->p_comm, &a, 1 ) == TRUE )
    {
      (*p_seq)++;
    }
    }
    break;
  case SEQ_TRANS_SAVE_SEND_WAIT:
    if( MB_COMM_IsPost_PostCommon( p_wk->p_comm ) )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_TRANS_SAVE_FINISH_WAIT:
    if( MB_COMM_IsPostMovieFinishMachine( p_wk->p_comm ) )
    {
      MB_MSG_MessageHide( p_wk->p_msg );
      MB_MSG_MessageCreateWindow( p_wk->p_msg, MMWT_NORMAL );
      MB_MSG_MessageDisp( p_wk->p_msg, MSG_MB_MOVIE_05 , p_wk->p_comm_init->msgSpeed );
      MB_COMM_ReqDisconnect( p_wk->p_comm );
      (*p_seq) = SEQ_WAIT_EXIT_COMM;
    }
    break;
    
  case SEQ_WAIT_NEXT_GAME_ERROR_MSG:
    if( MB_MSG_CheckPrintStreamIsFinish(p_wk->p_msg) )
    {
      //初期タイムアウトから来るとmsgSpdもらってない。
      const int msgSpd = ( p_wk->p_comm_init != NULL ? p_wk->p_comm_init->msgSpeed : 1 );
      MB_MSG_MessageDisp( p_wk->p_msg, MSG_MB_MOVIE_05 , msgSpd );
      MB_COMM_ReqDisconnect( p_wk->p_comm );
      (*p_seq)++;
    }
    break;
  case SEQ_WAIT_EXIT_COMM:
    if( MB_COMM_IsDisconnect( p_wk->p_comm ) )
    {
      MB_COMM_ExitComm( p_wk->p_comm );
      (*p_seq)++;
    }
    break;
  case SEQ_EXIT_GAME:
    /* 無限ループ */
    break;
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フェード・アウト
 *
 *	@param	MB_DATA_CONVERT_WORK *p_wk  ワーク
 *	@param	*p_seq  ローカルシーケンス
 *
 *	@return TRUEでPROC終了  FALSEで続行
 */
//-----------------------------------------------------------------------------
static BOOL Mb_DataConv_SeqFunc_FadeOut( MB_DATA_CONVERT_WORK *p_wk, int *p_seq )
{
  enum
  {
    SEQ_START,
    SEQ_WAIT,
    SEQ_NEXT,
  };

  switch( *p_seq )
  {
  case SEQ_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
    (*p_seq)++;
    break;
  case SEQ_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
		  (*p_seq)++;
		}
    break;
  case SEQ_NEXT:
    return TRUE;
  default:
    GF_ASSERT_HEAVY(0);
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  通信エラーをチェック  （一度だけTRUEを返す）
 *
 *	@return　TRUEでエラー FALSEでエラーじゃない
 */
//-----------------------------------------------------------------------------
static BOOL Mb_DataConv_Err_Check( void )
{
  static BOOL is_net_err = FALSE;
  if( NetErr_App_CheckError() != NET_ERR_CHECK_NONE && is_net_err == FALSE )
  {
    NetErr_App_ReqErrorDisp();
    is_net_err = TRUE;
    return TRUE;
  }
  return FALSE;
}
