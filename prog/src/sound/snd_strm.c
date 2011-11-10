//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		snd_strm.c
 *	@brief	サウンド	ストリーミング再生
 *	@author	Toru=Nagihashi
 *	@data		20101130
 *	@note   神王蟲プロジェクトで使用していたソースを移植したが、
 *	        StrmをSndArcStrmに変更したため、ほぼ書き直し
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include "gflib.h"
#include "system/gfl_use.h"
#include "arc_def.h"

#include "sound/pm_sndsys.h"

#include "sound/snd_strm.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	ストリームスレッドのプライオリティ
//=====================================
#define STREAM_THREAD_PRIO 17

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
typedef struct
{
  NNSSndStrmHandle p_strm_handle;
  BOOL  isPrepared;
  s32   strmNo;
}STRM_WORK;

//ワーク
static STRM_WORK*	sp_STRM_WORK = NULL;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
static void Snd_Strm_ResetHandle( void );

//----------------------------------------------------------------------------
/**
 *	@brief	ストリーミング再生システム	初期化
 *	
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
void SND_STRM_Init( HEAPID heapID )
{
	GF_ASSERT( sp_STRM_WORK == NULL );

	sp_STRM_WORK = GFL_HEAP_AllocMemory( heapID, sizeof(STRM_WORK) );
	GFL_STD_MemClear( sp_STRM_WORK, sizeof(STRM_WORK) );

	// ストリーミングチャンネル設定
  NNS_SndArcStrmInit( STREAM_THREAD_PRIO, PMSND_GetSndHeap() );
  NNS_SndStrmHandleInit( &sp_STRM_WORK->p_strm_handle );

  //チャンネルをアロケート
  NNS_SndArcStrmAllocChannel( PLAYER_STRM );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ストリーミング再生システム	破棄
 */
//-----------------------------------------------------------------------------
void SND_STRM_Exit( void )
{
	GF_ASSERT( sp_STRM_WORK );

	if(sp_STRM_WORK){
    //チャンネルを開放
    NNS_SndArcStrmFreeChannel( PLAYER_STRM );

    if( !NNS_SndStrmHandleIsValid( &sp_STRM_WORK->p_strm_handle ) )
    {
      NNS_SndStrmHandleRelease( &sp_STRM_WORK->p_strm_handle );
    }

		// メモリ破棄
		GFL_HEAP_FreeMemory( sp_STRM_WORK );
		sp_STRM_WORK = NULL;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ストリーミング再生システム	メイン
 */
//-----------------------------------------------------------------------------
void SND_STRM_Main( void )
{
	GF_ASSERT( sp_STRM_WORK );
	
  //NNS_SndMain();  //->PMSND_Mainで呼んでいる
}

//----------------------------------------------------------------------------
/**
 *	@brief  ストリームの読み込みを行います
 *
 *	@param	s32 strmNo  ストリーム番号
 */
//-----------------------------------------------------------------------------
void SND_STRM_LoadEx( s32 strmNo, u32 offset )
{
  BOOL ret;
  
  //ハンドル再初期化
  Snd_Strm_ResetHandle();
  
  //読み込み開始
  ret = NNS_SndArcStrmPrepare(
        &sp_STRM_WORK->p_strm_handle,
        strmNo, offset );

  GF_ASSERT_MSG( ret, "ストリームの準備ができませんでした" );

  sp_STRM_WORK->isPrepared  = TRUE;
  sp_STRM_WORK->strmNo  = strmNo;
}

//----------------------------------------------------------------------------
/**
 *	@brief	再生
 */
//-----------------------------------------------------------------------------
void SND_STRM_Play( s32 strmNo )
{
  //事前読み込み済みを再生しようとした
  if( sp_STRM_WORK->isPrepared && sp_STRM_WORK->strmNo == strmNo )
  {
    NNS_SndArcStrmStartPrepared( &sp_STRM_WORK->p_strm_handle );
  }
  else
  {
    BOOL ret;

    //ハンドル再初期化
    Snd_Strm_ResetHandle();
    
    //読み込みしてから再生
    ret  = NNS_SndArcStrmStart( &sp_STRM_WORK->p_strm_handle, strmNo, 0 );
    GF_ASSERT_MSG( ret, "ストリームを鳴らすことができなかった\n" );

    //読み込んだので、事前読み込み解除
    sp_STRM_WORK->isPrepared  = FALSE;
    sp_STRM_WORK->strmNo      = strmNo;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	停止
 */
//-----------------------------------------------------------------------------
void SND_STRM_Stop( void )
{
	GF_ASSERT( sp_STRM_WORK );
  NNS_SndArcStrmStop( &sp_STRM_WORK->p_strm_handle, 0 );
}

//----------------------------------------------------------------------------
/**
 *	@brief	再生中かチェックする
 *
 *	@retval	TRUE	再生中
 *	@retval	FALSE	停止中
 */
//-----------------------------------------------------------------------------
BOOL SND_STRM_CheckPlay( void )
{
	GF_ASSERT( sp_STRM_WORK );
  return NNS_SndArcStrmGetCurrentPlayingPos( &sp_STRM_WORK->p_strm_handle ) != 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ボリューム設定
 *
 *	@param	volume	ボリューム
 */
//-----------------------------------------------------------------------------
void SND_STRM_Volume( int volume )
{
	GF_ASSERT( sp_STRM_WORK );
  NNS_SndArcStrmSetVolume(
        &sp_STRM_WORK->p_strm_handle, volume );

}

//----------------------------------------------------------------------------
/**
 *	@brief  フェードアウト
 *
 *	@param	int fade_frame  フェードアウトするフレーム
 */
//-----------------------------------------------------------------------------
void SND_STRM_FadeOut( int fade_frame )
{
	GF_ASSERT( sp_STRM_WORK );
  NNS_SndArcStrmStop( &sp_STRM_WORK->p_strm_handle, fade_frame );
}
//----------------------------------------------------------------------------
/**
 *	@brief  曲の長さを取得
 *
 *	@param	void 
 *
 *	@return 曲のマイクロ秒
 */
//-----------------------------------------------------------------------------
u32 SND_STRM_GetMicroSeconds( void )
{
	GF_ASSERT( sp_STRM_WORK );
  return NNS_SndArcStrmGetTimeLength( &sp_STRM_WORK->p_strm_handle );
}

//-----------------------------------------------------------------------------
/**
 *			プライベート関数
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  ハンドルを再初期化します
 */
//-----------------------------------------------------------------------------
static void Snd_Strm_ResetHandle( void )
{
  if( !NNS_SndStrmHandleIsValid( &sp_STRM_WORK->p_strm_handle ) )
  {
    NNS_SndStrmHandleRelease( &sp_STRM_WORK->p_strm_handle );
    NNS_SndStrmHandleInit( &sp_STRM_WORK->p_strm_handle );
  }
}
