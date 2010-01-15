//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		snd_strm.c
 *	@brief		サウンド	ストリーミング再生
 *	@author		tomoya takahashi
 *	@data		2008.12.02
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "sound/snd_strm.h"

//-----------------------------------------------------------------------------
/**
 *					コーディング規約
 *		●関数名
 *				１文字目は大文字それ以降は小文字にする
 *		●変数名
 *				・変数共通
 *						constには c_ を付ける
 *						staticには s_ を付ける
 *						ポインタには p_ を付ける
 *						全て合わさると csp_ となる
 *				・グローバル変数
 *						１文字目は大文字
 *				・関数内変数
 *						小文字と”＿”と数字を使用する 関数の引数もこれと同じ
 */
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *					定数宣言
 */
//-----------------------------------------------------------------------------

//-------------------------------------
///	チャンネル
//=====================================
#define SND_STRM_CHANNEL_MAX		(1)
static const u8 sc_SND_STRM_CHANNEL_TBL[ SND_STRM_CHANNEL_MAX ] = {
	0,
};

//-------------------------------------
///	周波数
//=====================================
static const int sc_SND_STRM_HZ_TBL[ SND_STRM_HZMAX ] = {
	NNS_SND_STRM_TIMER_CLOCK/8000,
	NNS_SND_STRM_TIMER_CLOCK/11025,
	NNS_SND_STRM_TIMER_CLOCK/16000,
	NNS_SND_STRM_TIMER_CLOCK/22050,
	NNS_SND_STRM_TIMER_CLOCK/32000,
};

//-------------------------------------
/// サウンドデータタイプ
//=====================================
static const u32 sc_SND_STRM_DATATYPE[ SND_STRM_TYPE_MAX ] = {
	NNS_SND_STRM_FORMAT_PCM8,
	NNS_SND_STRM_FORMAT_PCM16,
};

//-------------------------------------
///	ストリーミング読み込み１サイズ
//=====================================
#define SND_STRM_ONELOAD_DATASIZ	(0)



//-------------------------------------
///	その他情報
//=====================================
#define	INTERVAL		(16)
#define	STRM_BUF_SIZE	( INTERVAL * 2 * SND_STRM_CHANNEL_MAX * 32 )
#define	SWAV_HEAD_SIZE	(18)

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
 */
//-----------------------------------------------------------------------------
typedef struct
{
  void* pStreamBuff32;
	NNSSndStrm			strm;
	u8					FS_strmBuffer[STRM_BUF_SIZE];
	int					FSReadPos;
	u32					strmReadPos;
	u32					strmWritePos;
	BOOL				playing;

	// 音楽情報
	BOOL				snddata_in;
	u32					arcid;
	u32					dataid;
	ARCHANDLE*			p_handle;

	u8* pStraightData;

	u32					data_siz;
	u32					seek_top;
	u32					type;
	u32					hz;
	
	BOOL        isLoop;
	BOOL        isFinishData;
}STRM_WORK;

//ワーク
static STRM_WORK*	sp_STRM_WORK = NULL;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
 */
//-----------------------------------------------------------------------------
static void SND_STRM_StockWaveData( NNSSndStrmCallbackStatus status, int numChannels, void *buffer[], u32	len, NNSSndStrmFormat format, void *arg );
static void SND_STRM_CopyBuffer( STRM_WORK* p_wk,int size );


//----------------------------------------------------------------------------
/**
 *	@brief	ストリーミング再生システム	初期化
 *
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
void SND_STRM_Init( u32 heapID )
{
	GF_ASSERT( sp_STRM_WORK == NULL );

	sp_STRM_WORK = GFL_HEAP_AllocMemory( heapID, sizeof(STRM_WORK) );
	GFL_STD_MemClear( sp_STRM_WORK, sizeof(STRM_WORK) );
  sp_STRM_WORK->pStreamBuff32 = GFL_NET_Align32Alloc(heapID,STRM_BUF_SIZE);

	// ストリーミングチャンネル設定
	NNS_SndInit();
	NNS_SndStrmInit( &sp_STRM_WORK->strm );
	NNS_SndStrmAllocChannel( &sp_STRM_WORK->strm, SND_STRM_CHANNEL_MAX, sc_SND_STRM_CHANNEL_TBL);
	
	sp_STRM_WORK->isLoop = FALSE;
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
		// ストリーミングチャンネル破棄
		NNS_SndStrmFreeChannel( &sp_STRM_WORK->strm );

    GFL_NET_Align32Free(sp_STRM_WORK->pStreamBuff32);

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

	if( sp_STRM_WORK->snddata_in ){
		SND_STRM_CopyBuffer( sp_STRM_WORK, SND_STRM_ONELOAD_DATASIZ );
	}
}


//----------------------------------------------------------------------------
/**
 *	@brief	サウンドデータ	設定
 *
 *	@param	arcid		アークID
 *	@param	dataid		データID
 *	@param	type		データタイプ
 *	@param	hz			サンプリング周期
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
void SND_STRM_SetUp( u32 arcid, u32 dataid, SND_STRM_TYPE type, SND_STRM_HZ hz, u32 heapID )
{
	GF_ASSERT( sp_STRM_WORK );


	sp_STRM_WORK->snddata_in = TRUE;
	sp_STRM_WORK->arcid		= arcid;
	sp_STRM_WORK->dataid	= dataid;
	sp_STRM_WORK->p_handle	= GFL_ARC_OpenDataHandle( sp_STRM_WORK->arcid, heapID );
	sp_STRM_WORK->data_siz	= GFL_ARC_GetDataSizeByHandle( sp_STRM_WORK->p_handle, sp_STRM_WORK->dataid );
	sp_STRM_WORK->seek_top	= GFL_ARC_GetDataOfsByHandle( sp_STRM_WORK->p_handle, sp_STRM_WORK->dataid );
	sp_STRM_WORK->seek_top	+= SWAV_HEAD_SIZE;
	sp_STRM_WORK->type		= type;
	sp_STRM_WORK->hz		= hz;
}

//----------------------------------------------------------------------------
/**
 *	@brief	サウンドデータ	設定
 *
 *	@param	type		データタイプ
 *	@param	hz			サンプリング周期
 *	@param	heapID	ヒープID
 *	@param	data		生のサンプリングデータ
 *	@param	size		サイズ
 */
//-----------------------------------------------------------------------------
void SND_STRM_SetUpStraightData( SND_STRM_TYPE type, SND_STRM_HZ hz, u32 heapID, u8* data,u32 size)
{
	GF_ASSERT( sp_STRM_WORK );

	sp_STRM_WORK->snddata_in = TRUE;
	sp_STRM_WORK->p_handle = NULL;
	sp_STRM_WORK->pStraightData = data;
	sp_STRM_WORK->data_siz = size;
	sp_STRM_WORK->type = type;
	sp_STRM_WORK->hz = hz;
}

//----------------------------------------------------------------------------
/**
 *	@brief	サウンドデータの破棄
 */
//-----------------------------------------------------------------------------
void SND_STRM_Release( void )
{
	GF_ASSERT( sp_STRM_WORK );
	GF_ASSERT( sp_STRM_WORK->snddata_in );

	// 停止
	NNS_SndStrmStop( &sp_STRM_WORK->strm );
	sp_STRM_WORK->playing = FALSE;

	// 破棄
	if( sp_STRM_WORK->p_handle != NULL )
	{
  	GFL_ARC_CloseDataHandle( sp_STRM_WORK->p_handle );
  }

	sp_STRM_WORK->snddata_in = FALSE;
}


//----------------------------------------------------------------------------
/**
 *	@brief		サウンドデータの有無を取得
 *
 *	@retval	TRUE	ある
 *	@retval	FALSE	ない
 */
//-----------------------------------------------------------------------------
BOOL SND_STRM_CheckSetUp( void )
{
	if( !sp_STRM_WORK ){
		return FALSE;
	}
	return sp_STRM_WORK->snddata_in;

}


//----------------------------------------------------------------------------
/**
 *	@brief	再生
 */
//-----------------------------------------------------------------------------
void SND_STRM_Play( void )
{
	BOOL ret;
	GF_ASSERT( sp_STRM_WORK );
	GF_ASSERT( sp_STRM_WORK->snddata_in );

	// パラメータ初期化
	if(sp_STRM_WORK->pStraightData){
		sp_STRM_WORK->FSReadPos = 0;
	}
	else{
		sp_STRM_WORK->FSReadPos = SWAV_HEAD_SIZE;
	}
	sp_STRM_WORK->strmReadPos=0;
	sp_STRM_WORK->strmWritePos=0;
	sp_STRM_WORK->isFinishData = FALSE;


	// セットアップ
	ret = NNS_SndStrmSetup( &sp_STRM_WORK->strm,
													sc_SND_STRM_DATATYPE[sp_STRM_WORK->type],
													sp_STRM_WORK->pStreamBuff32,
													STRM_BUF_SIZE,
													sc_SND_STRM_HZ_TBL[sp_STRM_WORK->hz],
													INTERVAL,
													SND_STRM_StockWaveData,
													sp_STRM_WORK);
	GF_ASSERT(ret);

	// バッファ状態をクリア
	MI_CpuClearFast( sp_STRM_WORK->pStreamBuff32, STRM_BUF_SIZE );

	// データ補充
	SND_STRM_CopyBuffer( sp_STRM_WORK, STRM_BUF_SIZE );

	NNS_SndStrmStart( &sp_STRM_WORK->strm );

	sp_STRM_WORK->playing = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	停止
 */
//-----------------------------------------------------------------------------
void SND_STRM_Stop( void )
{
	GF_ASSERT( sp_STRM_WORK );
	GF_ASSERT( sp_STRM_WORK->snddata_in );
	if( sp_STRM_WORK->playing ){
		NNS_SndStrmStop( &sp_STRM_WORK->strm );
		sp_STRM_WORK->playing = FALSE;
	}
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
	//GF_ASSERT( sp_STRM_WORK->snddata_in );
	return sp_STRM_WORK->playing;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ストリームが最後まで行ったか？
 *          現在最後のバッファを送る前にフラグが立ちます。
 *          なのでギリギリまでデータが入っていると再生しきれない場合があります。
 *          ループ設定時は必ずFALSEです。
 *
 *	@retval	TRUE	最後まで行った
 *	@retval	FALSE	再生中
 */
//-----------------------------------------------------------------------------
const BOOL SND_STRM_CheckFinish( void )
{
	GF_ASSERT( sp_STRM_WORK );
	GF_ASSERT( sp_STRM_WORK->snddata_in );
	
	if( sp_STRM_WORK->playing == TRUE &&
	    sp_STRM_WORK->isLoop == FALSE &&
	    sp_STRM_WORK->isFinishData == TRUE )
	{
  	return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ループフラグの設定
 *
 *	@param	isLoop	ループ設定
 */
//-----------------------------------------------------------------------------
void SND_STRM_SetLoopFlg( const BOOL isLoop )
{
	GF_ASSERT( sp_STRM_WORK );
	GF_ASSERT( sp_STRM_WORK->snddata_in );
	sp_STRM_WORK->isLoop = isLoop;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ループフラグの取得
 *
 *	@retval	TRUE	ループする
 *	@retval	FALSE	ループしない
 */
//-----------------------------------------------------------------------------
const BOOL SND_STRM_GetLoopFlg( void )
{
	GF_ASSERT( sp_STRM_WORK );
	GF_ASSERT( sp_STRM_WORK->snddata_in );
	return sp_STRM_WORK->isLoop;
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
	GF_ASSERT( sp_STRM_WORK->snddata_in );

	NNS_SndStrmSetVolume( &sp_STRM_WORK->strm, volume );
}






//-----------------------------------------------------------------------------
/**
 *			プライベート関数
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief	波形データを補充するコールバック
 *
 *	@param	status
 *	@param	num_channels
 *	@param	p_buffer[]
 *	@param	len
 *	@param	format
 *	@param	p_arg
 */
//-----------------------------------------------------------------------------
static void SND_STRM_StockWaveData( NNSSndStrmCallbackStatus status, int num_channels, void* p_buffer[], u32 len, NNSSndStrmFormat format, void* p_arg )
{
	STRM_WORK *sw=(STRM_WORK *)p_arg;
	int	i;
	u8	*strBuf;

	strBuf = p_buffer[0];

	for(i=0;i<len;i++){
		strBuf[i]=sw->FS_strmBuffer[i+sw->strmReadPos];
	}

	sw->strmReadPos+=len;
	if(sw->strmReadPos>=STRM_BUF_SIZE){
		sw->strmReadPos=0;
	}
	DC_FlushRange(strBuf,len);
}




//----------------------------------------------------------------------------
/**
 *	@brief	波形データを補充するコールバックが割り込み禁止で呼ばれるので、この関数でリードしておく
 *
 *	@param	p_wk		ワーク
 *	@param	size		サイズ
 */
//-----------------------------------------------------------------------------
static void SND_STRM_CopyBuffer( STRM_WORK* p_wk,int size )
{
	FSFile	file;
	s32		ret;

	GF_ASSERT( p_wk->snddata_in );

	if(p_wk->pStraightData){
		if(size){
			GFL_STD_MemCopy(&p_wk->pStraightData[p_wk->FSReadPos], &p_wk->FS_strmBuffer[0] , size);
			p_wk->FSReadPos+=size;
		}
		else{
			while(p_wk->strmReadPos!=p_wk->strmWritePos){
        if( sp_STRM_WORK->isFinishData == FALSE )
        {
  				GFL_STD_MemCopy(&p_wk->pStraightData[p_wk->FSReadPos], &p_wk->FS_strmBuffer[p_wk->strmWritePos] , 32);
        }
        else
        {
          GFL_STD_MemClear( &p_wk->FS_strmBuffer[p_wk->strmWritePos] , 32 );
        }
				p_wk->FSReadPos+=32;
				p_wk->strmWritePos+=32;
				if(p_wk->strmWritePos>=STRM_BUF_SIZE){
					p_wk->strmWritePos=0;
				}
				if( p_wk->FSReadPos >= sp_STRM_WORK->data_siz ){
  					p_wk->FSReadPos=0;
  					if( p_wk->isLoop == FALSE )
  					{
              p_wk->isFinishData = TRUE;
            }
//					GFL_ARC_SeekDataByHandle( p_wk->p_handle, sp_STRM_WORK->seek_top );
				}
			}
		}
	}
	else{
		if(size){
			GFL_ARC_LoadDataOfsByHandle( p_wk->p_handle, p_wk->dataid, p_wk->FSReadPos, size, &p_wk->FS_strmBuffer[0] );
			p_wk->FSReadPos+=size;
		}
		else{
			while(p_wk->strmReadPos!=p_wk->strmWritePos){

        if( sp_STRM_WORK->isFinishData == FALSE )
        {
  				GFL_ARC_LoadDataByHandleContinue( p_wk->p_handle, 32, &p_wk->FS_strmBuffer[p_wk->strmWritePos] );
        }
        else
        {
          GFL_STD_MemClear( &p_wk->FS_strmBuffer[p_wk->strmWritePos] , 32 );
        }
				p_wk->FSReadPos+=32;
				p_wk->strmWritePos+=32;
				if(p_wk->strmWritePos>=STRM_BUF_SIZE){
					p_wk->strmWritePos=0;
				}
				if( p_wk->FSReadPos >= sp_STRM_WORK->data_siz ){
					p_wk->FSReadPos=SWAV_HEAD_SIZE;
					GFL_ARC_SeekDataByHandle( p_wk->p_handle, sp_STRM_WORK->seek_top );
					if( p_wk->isLoop == FALSE )
					{
            p_wk->isFinishData = TRUE;
          }
				}
			}
		}
	}
}
