
//============================================================================================
/**
 * @file	gf_mcs.c
 * @brief	MCS関連関数
 * @author	soga
 * @date	2009.03.06
 */
//============================================================================================

#include <gflib.h>
#include <nnsys/mcs.h>

#include "system\gfl_use.h"
#include "debug/gf_mcs.h"

//============================================================================================
/**
 *	構造体宣言
 */
//============================================================================================

typedef struct
{
	HEAPID						heapID;
	u8*								mcsWorkMem;
	NNSMcsDeviceCaps	deviceCaps;
	NNSMcsRecvCBInfo	recvCBInfo;
	void*							printBuffer;
	void*							recvBuf;
	void*							recvBuf2;
	GFL_TCB*					vBlankTask;
}MCS_WORK;

static	MCS_WORK	*mw = NULL;

#define	MCS_SEND_IDLE	( 1 )

//============================================================================================
/**
 *	プロトタイプ宣言
 */
//============================================================================================

BOOL	MCS_Init( HEAPID heapID );
void	MCS_Exit( void );
void	MCS_Main( void );
u32		MCS_Read( void *buf, int size );
BOOL	MCS_Write( int ch, const void *buf, int size );
u32		MCS_CheckRead( void );
BOOL	MCS_CheckEnable( void );

static	void	MCS_DataRecvCallback( const void* pRecv, u32 recvSize, u32 userData, u32 offset, u32 totalSize );
static	void	MCS_VBlankIntr( GFL_TCB *tcb, void *work );
//static void MCS_CartIntrFunc( GFL_TCB *tcb, void *work );
static	void	MCS_CartIntrFunc( void );

//============================================================================================
/**
 *	システム初期化
 *
 * @param[in]	heapID	ヒープID
 *
 * @retval	FALSE:正常初期化　TRUE:失敗（すでに初期化済みなど）
 */
//============================================================================================
BOOL	MCS_Init( HEAPID heapID )
{
	if(	mw )
	{	
		return TRUE;
	}

	//管理構造体メモリを確保
	mw = GFL_HEAP_AllocMemory( heapID, sizeof( MCS_WORK ) );

	mw->heapID = heapID;

	// mcsの初期化
	mw->mcsWorkMem = GFL_HEAP_AllocMemory( mw->heapID, NNS_MCS_WORKMEM_SIZE ); // MCSのワーク用メモリを確保
	NNS_McsInit( mw->mcsWorkMem );

	mw->vBlankTask = GFUser_VIntr_CreateTCB( MCS_VBlankIntr, NULL, 0 );

	// カートリッジ割り込みを有効にし、カートリッジ割り込み内で
    // NNS_McsCartridgeInterrupt()が呼ばれるようにする
    OS_SetIrqFunction(OS_IE_CARTRIDGE, MCS_CartIntrFunc);
	(void)OS_EnableIrqMask(OS_IE_CARTRIDGE);

	// デバイスのオープン
	if( NNS_McsGetMaxCaps() > 0 && NNS_McsOpen( &mw->deviceCaps ) )
	{
		mw->printBuffer = GFL_HEAP_AllocMemory( mw->heapID, 1024 );        // プリント用のバッファの確保
		mw->recvBuf = GFL_HEAP_AllocMemory( mw->heapID, 0x4000 );       // 受信用バッファの確保
		mw->recvBuf2 = GFL_HEAP_AllocMemory( mw->heapID, 0x4000 );       // 受信用バッファの確保

		NNS_NULL_ASSERT( mw->printBuffer );
		NNS_NULL_ASSERT( mw->recvBuf );
		NNS_NULL_ASSERT( mw->recvBuf2 );

		// OS_Printfによる出力
		OS_Printf("device open\n");

		// mcs文字列出力の初期化
		NNS_McsInitPrint( mw->printBuffer, 1024 );

		// NNS_McsPrintfによる出力
		// このタイミングでmcsサーバが接続していれば、コンソールに表示されます。
		(void)NNS_McsPrintf("device ID %08X\n", mw->deviceCaps.deviceID );

		 // 読み取り用バッファの登録
		NNS_McsRegisterStreamRecvBuffer( MCS_CHANNEL0, mw->recvBuf, 0x4000 );
		NNS_McsRegisterStreamRecvBuffer( MCS_CHANNEL1, mw->recvBuf2, 0x4000 );

        // 受信コールバック関数の登録
//		NNS_McsRegisterRecvCallback( &mw->recvCBInfo, MCS_CHANNEL1, MCS_DataRecvCallback, (u32)&mw->mw );

		if( NNS_McsIsServerConnect() == FALSE )
		{	
			OS_TPrintf("接続に失敗しています\n");
			OS_TPrintf("接続するときは、[デバイス]-[接続]を選んでください\n");
			MCS_Exit();
			return TRUE;
		}

		return FALSE;
	}
	OS_Printf("device open fail.\n");
	return TRUE;
}

//============================================================================================
/**
 *	システム終了
 */
//============================================================================================
void	MCS_Exit( void )
{
	NNS_McsUnregisterRecvResource( MCS_CHANNEL0 );
	NNS_McsUnregisterRecvResource( MCS_CHANNEL1 );

	GFL_HEAP_FreeMemory( mw->recvBuf2 );
	GFL_HEAP_FreeMemory( mw->recvBuf );
	GFL_HEAP_FreeMemory( mw->printBuffer );

	GFL_TCB_DeleteTask( mw->vBlankTask );

	// NNS_McsPutStringによる出力
	if( NNS_McsIsServerConnect() == TRUE )
	{	
		(void)NNS_McsPutString("device close\n");
		(void)NNS_McsPutString("再接続するときは、[デバイス]-[切断]を選んで切断した後\n");
		(void)NNS_McsPutString("[デバイス]-[接続]を選んでください\n");
	}

	// デバイスをクローズ
	(void)NNS_McsClose();
	
	GFL_HEAP_FreeMemory( mw->mcsWorkMem );

	GFL_HEAP_FreeMemory( mw );
	mw = NULL;
}

//============================================================================================
/**
 *	システムメイン
 */
//============================================================================================
void	MCS_Main( void )
{
	if( mw == NULL ) return;
	NNS_McsPollingIdle();
}

//============================================================================================
/**
 *	読み込めるデータがあったときにデータを読みとる
 *
 *	@param[in]	buf		読み込んだデータを格納するワークへのポインタ
 *	@param[in]	size	読み込むサイズ
 *
 *	@retval readSize 実際に読み込んだサイズ
 */
//============================================================================================
u32		MCS_Read( void *buf, int size )
{
	int	len;
	u32 readSize = 0;

	// 受信可能なデータサイズの取得
	len = NNS_McsGetStreamReadableSize( MCS_CHANNEL0 );

	if( len > 0 ){
		// データの読み取り
		NNS_McsReadStream( MCS_CHANNEL0, buf, size, &readSize );
	}
	else{
		//分割データを受信
		// 受信可能なデータサイズの取得
		u32	flag = MCS_SEND_IDLE;
		u32 read;

		readSize = size;
		NNS_McsWriteStream( MCS_CHANNEL0, &flag, 4 );

		while( size ){
			MCS_Main();
			len = NNS_McsGetStreamReadableSize( MCS_CHANNEL1 );
			if( len ){
				NNS_McsReadStream( MCS_CHANNEL1, buf, len, &read );
				size -= len;
				(u8 *)buf += len;
				NNS_McsWriteStream( MCS_CHANNEL0, &flag, 4 );
			}
		}
	}

	return readSize;
}

//============================================================================================
/**
 *	データを書き出す
 *
 *  @param[in]	ch		書き出しに使用するチャンネルナンバー
 *	@param[in]	buf		書き出すデータが格納されているワークへのポインタ
 *	@param[in]	size	書き出すサイズ
 *
 *	@retval　TRUE:書き出し成功　FALSE:書き出し失敗
 */
//============================================================================================
BOOL	MCS_Write( int ch, const void *buf, int size )
{
	GF_ASSERT( ch < MCS_CH_MAX );
	GF_ASSERT( size < MCS_SEND_SIZE );
	return NNS_McsWriteStream( MCS_CHANNEL0 + ch, buf, size );
}

//============================================================================================
/**
 *	読み込めるデータがあるかチェック
 *
 *	@retval 読み込めるデータサイズ
 */
//============================================================================================
u32		MCS_CheckRead( void )
{
	u32	size;

	// 受信可能なデータサイズの取得
	size = NNS_McsGetStreamReadableSize( MCS_CHANNEL0 );

	if( size == 0 ){
	//サイズが大きくて分割送信されるデータの取得
		size = NNS_McsGetStreamReadableSize( MCS_CHANNEL1 );
		if( size ){
			u32 readSize = 0;
			// データの読み取り
			NNS_McsReadStream( MCS_CHANNEL1, &size, 4, &readSize );
		}
	}

	return size;
}

//============================================================================================
/**
 *	MCSが起動していてデバイスがオープンしているかチェック
 *
 *	@retval TRUE:起動中　FALSE:停止中
 */
//============================================================================================
BOOL	MCS_CheckEnable( void )
{	
	if( mw == NULL )
	{	
		return FALSE;
	}

	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         DataRecvCallback

  Description:  PC側からデータを受信したときに呼ばれるコールバック関数です。

                登録するコールバック関数内ではデータの送受信を行わないでください。
                また、割り込みが禁止されている場合があるため、
                割り込み待ちループも行わないでください。

  Arguments:    recv:       受信したデータの一部あるいは全部を格納している
                            バッファへのポインタ。
                recvSize:   recvによって示されるバッファに格納されている
                            データのサイズ。
                userData:   NNS_McsRegisterRecvCallback()の引数userDataで
                            指定した値。
                offset:     受信したデータの全部がrecvによって示されるバッファに
                            格納されている場合は0。
                            受信したデータの一部が格納されている場合は、
                            受信したデータ全体に対する0を基準としたオフセット位置。
                totalSize:  受信したデータの全体のサイズ。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void
MCS_DataRecvCallback(
    const void* pRecv,
    u32         recvSize,
    u32         userData,
    u32         offset,
    u32         totalSize
)
{
    MCS_WORK *mw = (MCS_WORK *)userData;

	OS_Printf( " Callback OK!\n");

	// 受信バッファチェック
	if (pRecv != NULL && recvSize == sizeof(MCS_WORK) && offset == 0)
	{
        mw = (MCS_WORK *)pRecv;
    }
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  Vブランク割り込みハンドラです。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void MCS_VBlankIntr( GFL_TCB *tcb, void *work )
{
	OS_SetIrqCheckFlag( OS_IE_V_BLANK );
    NNS_McsVBlankInterrupt();
}

/*---------------------------------------------------------------------------*
  Name:         CartIntrFunc

  Description:  カードリッジ割り込みハンドラです。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
//static void MCS_CartIntrFunc( GFL_TCB *tcb, void *work )
static void MCS_CartIntrFunc( void )
{
    OS_SetIrqCheckFlag(OS_IE_CARTRIDGE);
    NNS_McsCartridgeInterrupt();
}


