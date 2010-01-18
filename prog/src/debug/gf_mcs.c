
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
 *	定数
 */
//============================================================================================
#define	MCS_CH_MAX			( MCS_CHANNEL_END - 1 )
#define	MCS_CH_NUM			( MCS_CHANNEL_END )
#define	MCS_SEND_IDLE		( 1 )
#define	MCS_TIME_OUT	  ( 500 )

#define MCS_PRINTBUF_SIZE (1024)
#define MCS_RECVBUF_SIZE	(0x2000)
#define MCS_SPLIT_SIZE		(MCS_RECVBUF_SIZE	- 0x0800)

#define POKEMON_MCS_ID			(0x58976557)
#define	MCS_SEND_SIZE	( 0x1800 )

#define	TWL_MCS_WAIT	( 120 )   //TWLとの接続では遅延が発生するので、ある程度待つ処理が必要

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

    //TWLは接続の移行に遅延があるためある程度ウェイトをもたせる
    if( OS_GetConsoleType() & OS_CONSOLE_TWLDEBUGGER )
    {
      int wait = TWL_MCS_WAIT;
      OS_Printf("Waiting server connect.\n");
      do
      {
        SVC_WaitVBlankIntr();
        wait--;
      }while (! NNS_McsIsServerConnect() && ( wait ) );
    }

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

    if( OS_GetConsoleType() & OS_CONSOLE_NITRO ){
      return FALSE;
    }
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
	GF_ASSERT( ch < MCS_CH_NUM );
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

//============================================================================================
/**
 *	MCSが起動していてデバイスがオープンしているかチェック
 *
 *	@retval TRUE:起動中　FALSE:停止中
 */
//============================================================================================
//============================================================================================
/**
 *	ローカル関数
 */
//============================================================================================
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











//============================================================================================
/**
 *	
 *
 *
 *
 *
 *
 *
 *
 */
//============================================================================================
//============================================================================================
/**
 *	定数定義
 */
//============================================================================================
#define MCS_LINK_MAX	(7)	// 1リンクにつき2チャンネル使う

enum{
	GFL_MCSCOMM_NULL = 0,
	GFL_MCSCOMM_LINK,
	GFL_MCSCOMM_ANNUL,
};

#define MCS_COMMBUF_SIZE	(0x80)	// コマンドリクエスト格納バッファサイズ
//============================================================================================
/**
 *	構造体定義
 */
//============================================================================================
// コマンドヘッダ構造体
typedef struct {
	u32 projectID;		// プロジェクト識別ID
	u32 commID;		    // コマンドID
  u32 categoryID;		// カテゴリー識別ID
}GFL_MCS_HEADER;

// PC → DS への接続リクエスト内容構造体
typedef struct {
	u32 channelID1;		// PC側で確保されたチャンネル1(通信バッファより小さいデータ通信用)
	u32 channelID2;		// PC側で確保されたチャンネル2(通信バッファより大きいデータ通信用)
}GFL_MCS_LINKPARAM;

// PC → DS へのリンクリクエスト内容構造体
typedef struct {
	u32 projectID;		// プロジェクト識別ID
	u32 categoryID;		// カテゴリー識別ID
	u32 channelID1;		// PC側で確保されたチャンネル1(通信バッファより小さいデータ通信用)
	u32 channelID2;		// PC側で確保されたチャンネル2(通信バッファより大きいデータ通信用)
}GFL_MCS_LINKREQ;

// PC ←→ DS リンク情報構造体
typedef struct {
	u32 categoryID;		// カテゴリー識別ID
	u32 channelID1;		// 使用チャンネル1(通信バッファより小さいデータ通信用)
	u32 channelID2;		// 使用チャンネル2(通信バッファより大きいデータ通信用)
}GFL_MCS_LINKKEY;

// 受信情報格納構造体
typedef struct {
	u32		size;				// 受信データ総サイズ
	BOOL	split;			// 分割受信フラグ
	BOOL	getInfo;		// 事前情報取得フラグ
}GFL_MCS_READPARAM;

// PC ←→ DS リンクステータス
typedef struct {
	GFL_MCS_LINKKEY			key;
	GFL_MCS_READPARAM		rParam;
}GFL_MCS_LINKSTATUS;

// チャンネル（受信用）
typedef struct {
	u8		recvBuf[MCS_RECVBUF_SIZE];
	BOOL	resist;
}GFL_MCS_CHANNEL;

// システムワーク
typedef struct {
	// ライブラリ使用ワーク
	u8										mcsWork[NNS_MCS_WORKMEM_SIZE];
	u8										printBuf[MCS_PRINTBUF_SIZE];
	GFL_MCS_CHANNEL				channel[MCS_CH_NUM];
	NNSMcsDeviceCaps			deviceCaps;
	//GFL_TCB*						vBlankTask;

	// 内部制御ワーク
	BOOL									deviceOpen;
	GFL_MCS_LINKSTATUS		linkStatus[MCS_LINK_MAX];

}GFL_MCS_WORK;

//============================================================================================
/**
 *	変数定義
 */
//============================================================================================
static	GFL_MCS_WORK*		gflMCS = NULL;
static	GFL_MCS_WORK		gflMCSdata;
static	u8							commBuf[MCS_COMMBUF_SIZE];
static	u32							readResult;	// 読み取り関数引数用

static int count;

#include "debug/mcs_readfile.h"
//============================================================================================
/**
 *
 *	システム初期化
 *
 */
//============================================================================================
void	GFL_MCS_Init( void )
{
	int i;

	//管理構造体メモリを設定
	gflMCS = &gflMCSdata;
	GFL_STD_MemClear( gflMCS, sizeof(GFL_MCS_WORK) );

	// mcsの初期化
	NNS_McsInit( gflMCS->mcsWork );

  GF_MCS_FILE_Init();
	// VBlank割り込み
	//gflMCS->vBlankTask = GFUser_VIntr_CreateTCB( MCS_VBlankIntr, NULL, 0 );

	// カートリッジ割り込みを有効にし、カートリッジ割り込み内で
  // NNS_McsCartridgeInterrupt()が呼ばれるようにする
  OS_SetIrqFunction(OS_IE_CARTRIDGE, MCS_CartIntrFunc);
	(void)OS_EnableIrqMask(OS_IE_CARTRIDGE);

	// 内部制御ワークの初期化
	gflMCS->deviceOpen = FALSE;
}

//============================================================================================
/**
 *	システムメイン
 */
//============================================================================================
//--------------------------------------------------------------------------------------------
/**
 *	PCとのリンクを確立するための受信データ取得
 *	 (識別チャンネルはMCS_CHANNEL15固定)
 */
//--------------------------------------------------------------------------------------------
static void	GFL_MCS_GetComm( void )
{
	u32 channelID = MCS_CHANNEL15;
	int i;

	// 受信可能なデータサイズの取得
	u32	recvSize = NNS_McsGetStreamReadableSize(channelID);

	if((recvSize > 0)&&(recvSize <= MCS_COMMBUF_SIZE)){ 
		GFL_MCS_HEADER* header;
		NNS_McsReadStream(channelID, &commBuf, recvSize, &readResult);
		header = (GFL_MCS_HEADER*)commBuf;
		OS_Printf("コマンド受信... projectID(%x), commID(%x), categoryID(%x)\n", 
							header->projectID, header->commID, header->categoryID);

		if(header->projectID == POKEMON_MCS_ID){
			switch(header->commID){
			case GFL_MCSCOMM_LINK:
				{
					// リンクリクエスト格納
					int pLinkSt = GFL_MCS_LINKIDX_INVALID;
					int appActive = 0;

					for(i=0; i<MCS_LINK_MAX; i++){
						if(gflMCS->linkStatus[i].key.categoryID == 0){
							// リンク情報格納場所保存
							if(pLinkSt == GFL_MCS_LINKIDX_INVALID){ pLinkSt = i; }
						} else if(gflMCS->linkStatus[i].key.categoryID == header->categoryID){
							// 同一アプリ二重起動チェック
							appActive++;
						}
					}
					if(appActive){
						OS_Printf("リンク要求を受信しましたが、既に同一アプリのリンクをしているか\n");
						OS_Printf("二重起動接続をしようとしています。要求を無視します\n");
					} else if(pLinkSt == GFL_MCS_LINKIDX_INVALID){
						OS_Printf("最大接続量を超えるリンク情報を受信しました。要求を無視します\n");
					} else {
						GFL_MCS_LINKSTATUS* linkStatus = &gflMCS->linkStatus[pLinkSt];
						GFL_MCS_LINKPARAM* param = (GFL_MCS_LINKPARAM*)(commBuf + sizeof(GFL_MCS_HEADER));

						OS_Printf("リンク情報受信... categoryID(%x), channelID(%d, %d)\n",
											header->categoryID, param->channelID1, param->channelID2);
						linkStatus->key.categoryID = header->categoryID;
						linkStatus->key.channelID1 = param->channelID1;
						linkStatus->key.channelID2 = param->channelID2;
						// 受信完了信号として、PCへ送り返す
						{
							GFL_MCS_HEADER header;
							header.projectID = POKEMON_MCS_ID;
							header.commID = GFL_MCSCOMM_LINK;
							header.categoryID = linkStatus->key.categoryID;
							NNS_McsWriteStream(MCS_CHANNEL15, &header, sizeof(GFL_MCS_HEADER));
						}
					}
				}
				break;
			case GFL_MCSCOMM_ANNUL:
				// 切断リクエスト（ = リンク情報破棄）
				for(i=0; i<MCS_LINK_MAX; i++){
					GFL_MCS_LINKSTATUS* linkStatus = &gflMCS->linkStatus[i];

					if(linkStatus->key.categoryID == header->categoryID){
						linkStatus->key.categoryID = 0;
						linkStatus->key.channelID1 = MCS_CHANNEL15;
						linkStatus->key.channelID2 = MCS_CHANNEL15;

						linkStatus->rParam.size = 0;
						linkStatus->rParam.split = FALSE;
						linkStatus->rParam.getInfo = FALSE;
						OS_Printf("リンク切断要求を受信しました。categoryID(%x)\n", header->categoryID);
					}
				}
				break;
			}
		}
	} else {
		// コマンド受信バッファより大きい場合は読み捨てる
		while(recvSize){
			u32 tmp;
			if(recvSize <= 4){
				NNS_McsReadStream(channelID, &tmp, recvSize, &readResult);
				break;
			}
			NNS_McsReadStream(channelID, &tmp, 4, &readResult);
			recvSize -= 4;
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 *	PCとのリンク情報INDEXを取得
 */
//--------------------------------------------------------------------------------------------
static u16	GFL_MCS_GetLinkIdx( u32 categoryID )
{
	int i;

	for(i=0; i<MCS_LINK_MAX; i++){
		if(gflMCS->linkStatus[i].key.categoryID == categoryID){
			return i;
		}
	}
	return GFL_MCS_LINKIDX_INVALID;
}

//--------------------------------------------------------------------------------------------
/**
 *	メイン関数
 */
//--------------------------------------------------------------------------------------------
void GFL_MCS_Main( void )
{
	if( !gflMCS ){ return; }
	if( gflMCS->deviceOpen == FALSE ){ return; }

	NNS_McsPollingIdle();

	// 接続確認
	if(NNS_McsIsServerConnect() == FALSE){
		OS_Printf("MCSサーバとの通信が切断されました\n");
		GFL_MCS_Close();
		return;
	}
	GFL_MCS_GetComm();

	//OS_Printf("MCS通信中(%d)\n",count);
	count++;
}

//============================================================================================
/**
 *	システム割り込み処理
 */
//============================================================================================
void	GFL_MCS_VIntrFunc( void )
{
	if( !gflMCS ){ return; }
	if( gflMCS->deviceOpen == FALSE ){ return; }

  NNS_McsVBlankInterrupt();
}

//============================================================================================
/**
 *
 *	システム終了
 *
 */
//============================================================================================
void	GFL_MCS_Exit( void )
{
	if(	!gflMCS ){	return; }

	// デバイスの強制クローズ
	GFL_MCS_Close();

	//GFL_TCB_DeleteTask( gflMCS->vBlankTask );
	gflMCS = NULL;
}

//============================================================================================
/**
 *	デバイスオープン
 *
 * @retval	TRUE:正常初期化　FALSE:失敗
 */
//============================================================================================
BOOL	GFL_MCS_Open( void )
{
	int i;

	// 接続デバイスのサーチ
	if(NNS_McsGetMaxCaps() == 0){
		OS_TPrintf("通信デバイスが見つかりません\n");
		return FALSE;
	}

	// デバイスのオープン
	if(NNS_McsOpen(&gflMCS->deviceCaps) == FALSE){
		OS_TPrintf("デバイスのオープンに失敗しました\n");
		return FALSE;
	}
	OS_Printf("device open\n");

  //TWLは接続の移行に遅延があるためある程度ウェイトをもたせる
  if( OS_GetConsoleType() & OS_CONSOLE_TWLDEBUGGER )
  {
    int wait = TWL_MCS_WAIT;
    OS_Printf("Waiting server connect.\n");
    do
    {
      SVC_WaitVBlankIntr();
      wait--;
    }while (! NNS_McsIsServerConnect() && ( wait ) );
  }

	// mcs文字列出力の初期化
	NNS_McsInitPrint( gflMCS->printBuf, MCS_PRINTBUF_SIZE );
	// NNS_McsPrintfによる出力。mcsサーバが接続していればコンソールに表示される
	(void)NNS_McsPrintf("device ID %08X\n", gflMCS->deviceCaps.deviceID );

	if( NNS_McsIsServerConnect() == FALSE ){	
		OS_TPrintf("接続に失敗しています\n");
		OS_TPrintf("接続するときは、MCSサーバの[デバイス]-[接続]を選んでください\n");
		(void)NNS_McsClose();
		return FALSE;
	}

	// 受信チャンネルをオープン
	for(i=0; i<MCS_CH_NUM; i++){
		GFL_MCS_CHANNEL* mcsCH = &gflMCS->channel[i];
		u32 channelID = MCS_CHANNEL0 + i;

		// 読み取り用バッファの登録
		NNS_McsRegisterStreamRecvBuffer(channelID, &mcsCH->recvBuf, MCS_RECVBUF_SIZE);
		// 受信コールバック関数の登録
		//NNS_McsRegisterRecvCallback(&mchCH->recvCBInfo, channelID, MCS_DataRecvCallback, NULL); 

		mcsCH->resist = TRUE;
	}
	// リンク情報の初期化
	for(i=0; i<MCS_LINK_MAX; i++){
		GFL_MCS_LINKSTATUS* linkStatus = &gflMCS->linkStatus[i];
		linkStatus->key.categoryID = 0;
		linkStatus->key.channelID1 = MCS_CHANNEL15;
		linkStatus->key.channelID2 = MCS_CHANNEL15;

		linkStatus->rParam.size = 0;
		linkStatus->rParam.split = FALSE;
		linkStatus->rParam.getInfo = FALSE;
	}
	gflMCS->deviceOpen = TRUE;

	count = 0;
	return TRUE;
}

//============================================================================================
/**
 *	デバイスクローズ
 */
//============================================================================================
void	GFL_MCS_Close( void )
{
	int i;

	if(	!gflMCS ){	return; }
	if( gflMCS->deviceOpen == FALSE ){ return; }

	// 受信チャンネルをクローズ
	for(i=0; i<MCS_CH_NUM; i++){
		GFL_MCS_CHANNEL* mcsCH = &gflMCS->channel[i];
		u32 channelID = MCS_CHANNEL0 + i;

		if(mcsCH->resist == TRUE){
			// 読み取り用バッファおよびコールバック関数の登録解除
			NNS_McsUnregisterRecvResource( channelID );
			mcsCH->resist = FALSE;
		}
	}
	// デバイスをクローズ
	(void)NNS_McsClose();

	gflMCS->deviceOpen = FALSE;
}

//============================================================================================
/**
 *	PCとのリンクが確立しているかチェックする
 */
//============================================================================================
BOOL	GFL_MCS_CheckLink( u32 categoryID )
{
	int i;

	for(i=0; i<MCS_LINK_MAX; i++){
		GFL_MCS_LINKSTATUS* linkStatus = &gflMCS->linkStatus[i];

		if(linkStatus->key.categoryID == categoryID){
			return TRUE;
		}
	}
	return FALSE;
}

//============================================================================================
/**
 *	アイドリング状態検出＆送信
 */
//============================================================================================
static void sendIdle( GFL_MCS_LINKKEY* pKey )
{
	u32	idleFlag = MCS_SEND_IDLE;

	NNS_McsWriteStream(pKey->channelID1, &idleFlag, 4 );
}

static BOOL checkIdle( GFL_MCS_LINKKEY* pKey )
{
	u32 size = NNS_McsGetStreamReadableSize(pKey->channelID1);
	if(size == 4){
		u32 flag;
		NNS_McsReadStream(pKey->channelID1, &flag, 4, &readResult);
		if(flag == MCS_SEND_IDLE){ return TRUE; }
	}
	return FALSE;
}

//============================================================================================
/**
 *	データ読み込みチェック
 */
//============================================================================================
static int GFL_MCS_CheckReadable_Core( u16 idx )
{
	GFL_MCS_LINKSTATUS* linkStatus = &gflMCS->linkStatus[idx];
	u32 size;

	// リセット
	linkStatus->rParam.size = 0;
	linkStatus->rParam.split = FALSE;
	linkStatus->rParam.getInfo = TRUE;	// 情報取得済みフラグ設定

	// 受信可能なデータサイズの取得(チャンネル1)
	size = NNS_McsGetStreamReadableSize(linkStatus->key.channelID1);
	if( size ){
		//一度の受信で取得できるデータがある
		linkStatus->rParam.size = size;
		linkStatus->rParam.split = FALSE;		// 分割受信なし
		return size;
	}
	// 受信可能なデータサイズの取得(チャンネル2)
	size = NNS_McsGetStreamReadableSize(linkStatus->key.channelID2);
	if( size ){
		// 分割送信されているデータがある
		u32 totalSize;
		// 総サイズ取得
		NNS_McsReadStream(linkStatus->key.channelID2, &totalSize, 4, &readResult);
		linkStatus->rParam.size = totalSize;
		linkStatus->rParam.split = TRUE;		// 分割受信あり
		return totalSize;
	}
	return 0;
}

int		GFL_MCS_CheckReadable( u32 categoryID )
{
	u16 linkIdx;

	if( !gflMCS ){ return 0; }
	if( gflMCS->deviceOpen == FALSE ){ return 0; }

	linkIdx = GFL_MCS_GetLinkIdx(categoryID);
	if(linkIdx == GFL_MCS_LINKIDX_INVALID){ return  0; }

	return GFL_MCS_CheckReadable_Core(linkIdx);
}

//============================================================================================
/**
 *	データ読み込み
 */
//============================================================================================
BOOL	GFL_MCS_Read( u32 categoryID, void* pReadBuf, u32 readBufSize )
{
	GFL_MCS_LINKSTATUS* linkStatus;
	u16		linkIdx;
	u32		readSize;
	BOOL	result = FALSE;

	if( !gflMCS ){ return FALSE; }
	if( gflMCS->deviceOpen == FALSE ){ return FALSE; }

	linkIdx = GFL_MCS_GetLinkIdx(categoryID);
	if(linkIdx == GFL_MCS_LINKIDX_INVALID){ return  FALSE; }
	linkStatus = &gflMCS->linkStatus[linkIdx];

	if(linkStatus->rParam.getInfo == FALSE){ 
		// 読み取り情報が取得されていないのでここで取得
		GFL_MCS_CheckReadable_Core(linkIdx);
	}
	readSize = linkStatus->rParam.size;
	if(readSize){
		if(readSize > readBufSize){
			OS_Printf("読み込みバッファサイズを超えるデータが到着しています\n");
			return FALSE;
		}
		//OS_Printf("読み込みデータ到着(%x)\n", readSize);
		result = TRUE;

		if(linkStatus->rParam.split == FALSE){
			// 一括受信
			NNS_McsReadStream(linkStatus->key.channelID1, pReadBuf, readSize, &readResult);
		} else {
			// 分割受信
			u32 recvSize;
			sendIdle(&linkStatus->key);

			while( readSize ){
				NNS_McsPollingIdle();

				recvSize = NNS_McsGetStreamReadableSize(linkStatus->key.channelID2);
				if( recvSize ){
					NNS_McsReadStream(linkStatus->key.channelID2, pReadBuf, recvSize, &readResult);
					readSize -= recvSize;
					(u8*)pReadBuf += recvSize;
					sendIdle(&linkStatus->key);
				}
			}
		}
	}
	// リセット
	linkStatus->rParam.size = 0;
	linkStatus->rParam.split = FALSE;
	linkStatus->rParam.getInfo = FALSE;

	return result;
}

//============================================================================================
/**
 *	データ書き込み
 */
//============================================================================================
BOOL	GFL_MCS_Write( u32 categoryID, const void* pWriteBuf, u32 writeSize )
{
	GFL_MCS_LINKSTATUS* linkStatus;
	u16		linkIdx;

	if( !gflMCS ){ return FALSE; }
	if( gflMCS->deviceOpen == FALSE ){ return FALSE; }

	linkIdx = GFL_MCS_GetLinkIdx(categoryID);
	if(linkIdx == GFL_MCS_LINKIDX_INVALID){ return  FALSE; }
	linkStatus = &gflMCS->linkStatus[linkIdx];

	if( writeSize <= MCS_SPLIT_SIZE ){
		// 一括送信
		NNS_McsWriteStream(linkStatus->key.channelID1, pWriteBuf, writeSize);
	} else {
		// 分割送信
		NNS_McsWriteStream(linkStatus->key.channelID2, &writeSize, 4);	// 総サイズ送信

		while(writeSize){
			NNS_McsPollingIdle();

			if(checkIdle(&linkStatus->key) == TRUE){
				if(writeSize > MCS_SPLIT_SIZE){
					NNS_McsWriteStream(linkStatus->key.channelID2, pWriteBuf, MCS_SPLIT_SIZE);
					writeSize -= MCS_SPLIT_SIZE;
					(u8*)pWriteBuf += MCS_SPLIT_SIZE;
				} else {
					NNS_McsWriteStream(linkStatus->key.channelID2, pWriteBuf, writeSize);
					writeSize = 0;
				}
			}
		}
		while(checkIdle(&linkStatus->key) == FALSE){ NNS_McsPollingIdle(); }
	}
	return TRUE;
}



