#include <nitro.h>
#include <dwc.h>
#include  <cstring>


// ヘッダファイルインクルード
//-------------------------------------
#include "main.h"
#include "init.h"
#include "utility.h"

// 定数宣言
//---------------------------------------------------------
#define INITIAL_CODE     'NTRJ'     // このサンプルが仕様するイニシャルコード
#define GAME_NAME        "dwctest" // このサンプルが使用するゲーム名
#define GAME_SECRET_KEY  "d4q9GZ"  // このサンプルが使用するシークレットキー
#define GAME_PRODUCTID   10824     // このサンプルが使用するプロダクトID
#define RANKING_INITDATA "hxBfMOkOvlOHpUGfQlOb0001ed350000685d0040000037012e6bdwctest"
#define CATEGORY         10
//#define USE_AUTHSERVER_PRODUCTION // 製品向け認証サーバを使用する場合有効にする


// 構造体型宣言
//---------------------------------------------------------

// ユーザー定義データ
typedef struct _tagMYUSERDEFINEDDATA{
	u32		checksum;		// checksum
	u32		size;		// サイズ
	u8		data[32];	// データ(32bytes)
}MYUSERDEFINEDDATA;


// グローバル変数宣言
//---------------------------------------------------------
DWCInetControl stConnCtrl;


// 関数プロトタイプ
//---------------------------------------------------------
static BOOL StartIPMain(void);
static void VBlankIntr(void);

/*---------------------------------------------------------------------------*
  メモリ確保関数
 *---------------------------------------------------------------------------*/
static void*
AllocFunc( DWCAllocType name, u32   size, int align )
{
	void * ptr;
	OSIntrMode old;
	(void)name;
	(void)align;

	old = OS_DisableInterrupts();

	ptr = OS_AllocFromMain( size );

	OS_RestoreInterrupts( old );

	return ptr;
}

/*---------------------------------------------------------------------------*
  メモリ開放関数
 *---------------------------------------------------------------------------*/
static void
FreeFunc( DWCAllocType name, void* ptr,  u32 size  )
{
	OSIntrMode old;
	(void)name;
	(void)size;

	if ( !ptr ) return;

	old = OS_DisableInterrupts();

	OS_FreeToMain( ptr );

	OS_RestoreInterrupts( old );
}


//=============================================================================
/*!
 *	@brief	IPの取得
 *
 *	@retval	TRUE		成功
 *	@retval	FALSE   	失敗
 */
//=============================================================================
static BOOL StartIPMain(void)
{
	DWC_InitInet( &stConnCtrl );

	DWC_ConnectInetAsync();

	// 安定なステートまで待つ。
	while ( !DWC_CheckInet() )
	{
		DWC_ProcessInet();

		OS_WaitVBlankIntr();

	}

	switch ( DWC_GetInetStatus() )
	{
	case DWC_CONNECTINET_STATE_ERROR:
		{
			// エラー表示
			DWCError err;
			int errcode;
			err = DWC_GetLastError( &errcode );

			OS_TPrintf("   Error occurred %d %d.\n", err, errcode );
		}
		DWC_ClearError();

		DWC_CleanupInet();
		break;
	case DWC_CONNECTINET_STATE_FATAL_ERROR:
		{
			// エラー表示
			DWCError err;
			int errcode;
			err = DWC_GetLastError( &errcode );

			OS_Panic("   Error occurred %d %d.\n", err, errcode );
		}
		break;

	case DWC_CONNECTINET_STATE_CONNECTED:
		OS_TPrintf("   CONNECTED!!!.\n");
		return TRUE;
	}
	return FALSE;
}

/*---------------------------------------------------------------------------*
  Vブランク割り込み関数
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

//=============================================================================
/*!
 *	@brief	メイン
 */
//=============================================================================
void NitroMain()
{

	DWCUserData	    userdata;	// DWCのユーザーデータ
	DWCRnkError		res;		// ランキングライブラリのエラー取得用
	DWCRnkGetParam	param;		// ランキング取得時のパラメータ

	MYUSERDEFINEDDATA	userdefineddata;	// ユーザー定義データ

	u32 order;
	u32 count;
	int i;
	int errorCode;
	int ret;

	// initialize
	//-------------------------------------------
	OS_Init();

	// スタック溢れチェック初期設定
	OS_SetThreadStackWarningOffset(OS_GetCurrentThread(), 0x100);
	
	Heap_Init();

	GX_Init();

	GX_DispOff();
	GXS_DispOff();

	// Vブランク割り込み許可
	OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
	(void)OS_EnableIrqMask(OS_IE_V_BLANK);
	(void)OS_EnableIrq();
	(void)OS_EnableInterrupts();
	(void)GX_VBlankIntr(TRUE);
	GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);

	FS_Init( MI_DMA_MAX_NUM ); 

	// 表示開始
	OS_WaitVBlankIntr();			  // Waiting the end of VBlank interrupt
	GX_DispOn();
	GXS_DispOn();

    // DWCライブラリ初期化
#if defined( USE_AUTHSERVER_PRODUCTION )
    ret = DWC_InitForProduction( GAME_NAME, INITIAL_CODE, AllocFunc, FreeFunc );
#else
    ret = DWC_InitForDevelopment( GAME_NAME, INITIAL_CODE, AllocFunc, FreeFunc );
#endif
    
    OS_TPrintf( "DWC_InitFor*() result = %d\n", ret );

    if ( ret == DWC_INIT_RESULT_DESTROY_OTHER_SETTING )
    {
        OS_TPrintf( "Wi-Fi setting might be broken.\n" );
    }
	
	// デバッグ表示レベル指定
	DWC_SetReportLevel(DWC_REPORTFLAG_ALL);

	//IPの取得
	while(!StartIPMain())
	{
		OS_Printf("error at StartIPMain()\n");	
	}

	//---------------------------------
	// ログイン処理
	//---------------------------------
	//
	// ここでユーザーデータの取得処理を行ってください。ランキングライブラリは
	// GameSpyに依存しているため、少なくとも一度はログインをした有効な
	// GameSpyProfileIDを取得しているユーザーデータでなければ初期化に失敗しま
	// す。仮IDは無効です。
	//
	// 既に過去にログインして取得したユーザーデータがバックアップに保存さ
	// れている場合はそれを使用することができます。ただし、ガイドラインに定め
	// られているとおり、インターネットに接続する際にはその都度必ずログインす
	// る必要があることに注意してください
	//
	// 外部認証サーバーなどを使用している場合は有効なGameSpyProfileIDを取得す
	// ることはできませんので、ランキングライブラリは使用できません。そもそも
	// その場合はGameSpyのサーバー資源を使用できません。
	//
	// <注意>
	//
	// このサンプルでは簡単のためにユーザーデータを無理やり生成していますが、
	// 実際にはログインして得られた正式なユーザーデータを使用するようにしてく
	// ださい。それ以外のデータを渡した場合はランキングモジュールの初期化に
	// 成功する可能性はありますが、動作は未定義です。
	//
	{
		// ユーザーデータの設定
		//
		//
		u32 dummyuserdata[DWC_ACC_USERDATA_BUFSIZE] = {
								0x00000040,0xa1b2c3d4,0x5e6f7a8b,0xc9d0e1f2,
								0x00000996,0x3a4b5c6d,0xe7f8a9b0,0x0001e240,
								0x1c2d3e4f,0xa5b6c7d8,0x9e0f1a2b,0xc3d4e5f6,
								0x7a8b9c0d,0xe1f2a3b4,0x5c6d7e8f,0xcfa4db0e };

		memcpy( &userdata, dummyuserdata, DWC_ACC_USERDATA_BUFSIZE );
	}

	//---------------------------------
	// ランキングモジュールの処理
	//---------------------------------

	// ランキングモジュールの初期化
	res = DWC_RnkInitialize(	RANKING_INITDATA,
								&userdata );

	switch( res ){
	case DWC_RNK_SUCCESS:
		OS_Printf("DWC_RnkInitialize success\n");
		break;
	case DWC_RNK_ERROR_INIT_ALREADYINITIALIZED:
		OS_Printf("[err]DWC_RnkInitialize already initialized\n");
		goto exit;
	case DWC_RNK_ERROR_INIT_INVALID_INITDATASIZE:
		OS_Printf("[err]DWC_RnkInitialize invalid init data size\n");
		goto exit;
	case DWC_RNK_ERROR_INIT_INVALID_INITDATA:
		OS_Printf("[err]DWC_RnkInitialize invalid init data\n");
		goto exit;
	case DWC_RNK_ERROR_INIT_INVALID_USERDATA:
		OS_Printf("[err]DWC_RnkInitialize invaild user data\n");
		goto exit;
	}

	//
	// アップロード
	//

	// ユーザー定義データの生成
	//
	// ユーザー定義データには任意のバイナリデータを764バイト指定することができ
	// ます。このデータはランキングリストの取得時に一緒に取得することができま
	// す。
	//
	// このデータは、悪意のあるユーザーによる偽装サーバーなどにより、不正に
	// 書き換えられる可能性があります。不正な書き換えが行われたデータにより、
	// プログラムが暴走したり、悪意のあるコードが実行されることのないように、
	// クライアントでは入力されるデータは信頼されるものではないという前提の上に
	// コーディングを行ってください。
	//
	// 本サンプルでは、ユーザー定義データにサイズとChecksumを記録し、データの
    // 正当性をチェックしています。実際には、NULL終端であると仮定したデータの
    // サーチによるオーバーフロー等にも留意し、少なくともユーザー定義データの
    // サイズ以上のメモリ領域にアクセスしないようにコーディングしてください。
	//
	userdefineddata.size = 32;
	strcpy((char *)userdefineddata.data, "test data");
	userdefineddata.checksum = (u32)MATH_CalcChecksum8(
										(const void*)userdefineddata.data,
										userdefineddata.size );

	// スコアアップロードリクエスト開始
	res = DWC_RnkPutScoreAsync(	CATEGORY,				// category
								DWC_RNK_REGION_JP,		// region
								1234,					// score
								(void*)&userdefineddata,// user defined data
								sizeof( userdefineddata ) );

	switch( res ){
	case DWC_RNK_SUCCESS:
		OS_Printf("DWC_RnkPutScoreAsync success\n");
		break;
	case DWC_RNK_IN_ERROR:
		OS_Printf("[err]DWC_RnkPutScoreAsync in error\n");
		goto exit;
	case DWC_RNK_ERROR_PUT_NOTREADY:
		OS_Printf("[err]DWC_RnkPutScoreAsync not ready\n");
		goto exit;
	case DWC_RNK_ERROR_INVALID_PARAMETER:
		OS_Printf("[err]DWC_RnkPutScoreAsync invalid parameter\n");
		goto exit;
	case DWC_RNK_ERROR_PUT_INVALID_KEY:
		OS_Printf("[err]DWC_RnkPutScoreAsync invalid key\n");
		goto exit;
	case DWC_RNK_ERROR_PUT_NOMEMORY:
		OS_Printf("[err]DWC_RnkPutScoreAsync nomemory\n");
		goto exit;
	}

	// 非同期処理

	while( (res = DWC_RnkProcess()) == DWC_RNK_SUCCESS ){
	
		OS_Sleep(10);

	}

	switch( res ){
    case DWC_RNK_PROCESS_TIMEOUT:
        OS_Printf("DWC_RnkProcess timeout!!\n");
        goto exit;
	case DWC_RNK_IN_ERROR:
		DWC_GetLastError(&errorCode);
		OS_Printf("[err]DWC_RnkProcess in error [code:%d]\n", -errorCode);
		goto exit;
	case DWC_RNK_PROCESS_NOTASK:
		OS_Printf("DWC_RnkProcess done\n");
		break;
	}

	// 通信結果取得
	switch( DWC_RnkGetState() ){
	case DWC_RNK_STATE_COMPLETED:
		OS_Printf("DWC_RnkGetState completed\n");
		break;
	case DWC_RNK_STATE_ERROR:
        DWC_GetLastError(&errorCode);
        OS_Printf("[err]DWC_RnkGetState error [code:%d]\n", -errorCode);
		goto exit;
	}



	//
	// 自分の順位を取得
	//

	// 順位取得リクエスト開始

	param.size = sizeof( param.order );
	param.order.since = 0;
	param.order.sort = DWC_RNK_ORDER_DES;

	res = DWC_RnkGetScoreAsync(	DWC_RNK_GET_MODE_ORDER,		// mode
								CATEGORY,					// category
								DWC_RNK_REGION_JP,			// region
								&param );					// parameter

	switch( res ){
	case DWC_RNK_SUCCESS:
		OS_Printf("DWC_RnkGetScoreAsync success\n");
		break;
	case DWC_RNK_IN_ERROR:
		OS_Printf("[err]DWC_RnkGetScoreAsync in error\n");
		goto exit;
	case DWC_RNK_ERROR_GET_NOTREADY:
		OS_Printf("[err]DWC_RnkGetScoreAsync not ready\n");
		goto exit;
	case DWC_RNK_ERROR_INVALID_PARAMETER:
		OS_Printf("[err]DWC_RnkGetScoreAsync invalid parameter\n");
		goto exit;
	case DWC_RNK_ERROR_GET_INVALID_KEY:
		OS_Printf("[err]DWC_RnkGetScoreAsync invalid key\n");
		goto exit;
	case DWC_RNK_ERROR_GET_NOMEMORY:
		OS_Printf("[err]DWC_RnkGetScoreAsync nomemory\n");
		goto exit;
	}

	// 非同期処理

	while( (res = DWC_RnkProcess()) == DWC_RNK_SUCCESS ){
	
		OS_Sleep(10);

	}

	switch( res ){
    case DWC_RNK_PROCESS_TIMEOUT:
        OS_Printf("DWC_RnkProcess timeout!!\n");
        goto exit;
	case DWC_RNK_IN_ERROR:
		DWC_GetLastError(&errorCode);
		OS_Printf("[err]DWC_RnkProcess in error [code:%d]\n", -errorCode);
		goto exit;
	case DWC_RNK_PROCESS_NOTASK:
		OS_Printf("DWC_RnkProcess done\n");
		break;
	}

	// 通信結果取得
	switch( DWC_RnkGetState() ){
	case DWC_RNK_STATE_COMPLETED:
		OS_Printf("DWC_RnkGetState completed\n");
		break;
	case DWC_RNK_STATE_ERROR:
        DWC_GetLastError(&errorCode);
        OS_Printf("[err]DWC_RnkGetState error [code:%d]\n", -errorCode);
		goto exit;
	}

	// 表示
	res = DWC_RnkResGetOrder( &order );
	switch( res ){
	case DWC_RNK_SUCCESS:
		break;
	case DWC_RNK_IN_ERROR:
		OS_Printf("[err]DWC_RnkResGetOrder in error [code:%d]\n", DWC_GetLastError( NULL ) );
		goto exit;
	case DWC_RNK_ERROR_INVALID_MODE:
		OS_Printf("[err]DWC_RnkResGetOrder invalid mode\n");
		goto exit;
	case DWC_RNK_ERROR_INVALID_PARAMETER:
		OS_Printf("[err]DWC_RnkResGetOrder invalid parameter\n");
		goto exit;
	case DWC_RNK_ERROR_NOTCOMPLETED:
		OS_Printf("[err]DWC_RnkResGetOrder not completed\n");
		goto exit;
	case DWC_RNK_ERROR_EMPTY_RESPONSE:
		OS_Printf("[err]DWC_RnkResGetOrder empty response\n");
		goto exit;
	}

	OS_Printf("order=%d\n", order);


	//
	// ランキングを取得
	//

	// トップランキング取得リクエスト開始
	param.size = sizeof( param.toplist );
	param.toplist.sort = DWC_RNK_ORDER_DES;
	param.toplist.limit = 10;
	param.toplist.since = 0;

	res = DWC_RnkGetScoreAsync(	DWC_RNK_GET_MODE_TOPLIST,	// mode
									CATEGORY,				// category
									DWC_RNK_REGION_JP,		// region
									&param );				// parameter

	switch( res ){
	case DWC_RNK_SUCCESS:
		OS_Printf("DWC_RnkGetScoreAsync success\n");
		break;
	case DWC_RNK_IN_ERROR:
		OS_Printf("[err]DWC_RnkGetScoreAsync in error\n");
		goto exit;
	case DWC_RNK_ERROR_GET_NOTREADY:
		OS_Printf("[err]DWC_RnkGetScoreAsync not ready\n");
		goto exit;
	case DWC_RNK_ERROR_INVALID_PARAMETER:
		OS_Printf("[err]DWC_RnkGetScoreAsync invalid parameter\n");
		goto exit;
	case DWC_RNK_ERROR_GET_INVALID_KEY:
		OS_Printf("[err]DWC_RnkGetScoreAsync invalid key\n");
		goto exit;
	case DWC_RNK_ERROR_GET_NOMEMORY:
		OS_Printf("[err]DWC_RnkGetScoreAsync nomemory\n");
		goto exit;
	}

	// 非同期処理

	while( (res = DWC_RnkProcess()) == DWC_RNK_SUCCESS ){
	
		OS_Sleep(10);

	}

	switch( res ){
    case DWC_RNK_PROCESS_TIMEOUT:
        OS_Printf("DWC_RnkProcess timeout!!\n");
        goto exit;
	case DWC_RNK_IN_ERROR:
		DWC_GetLastError(&errorCode);
		OS_Printf("[err]DWC_RnkProcess in error [code:%d]\n", -errorCode);
		goto exit;
	case DWC_RNK_PROCESS_NOTASK:
		OS_Printf("DWC_RnkProcess done\n");
		break;
	}

	// 通信結果取得
	switch( DWC_RnkGetState() ){
	case DWC_RNK_STATE_COMPLETED:
		OS_Printf("DWC_RnkGetState completed\n");
		break;
	case DWC_RNK_STATE_ERROR:
        DWC_GetLastError(&errorCode);
        OS_Printf("[err]DWC_RnkGetState error [code:%d]\n", -errorCode);
		goto exit;
	}


	// 一覧を取得する

	res = DWC_RnkResGetRowCount( &count );

	switch( res ){
	case DWC_RNK_SUCCESS:
		break;
	case DWC_RNK_IN_ERROR:
		OS_Printf("[err]DWC_RnkResGetRowCount in error [code:%d]\n", DWC_GetLastError( NULL ) );
		goto exit;
	case DWC_RNK_ERROR_INVALID_MODE:
		OS_Printf("[err]DWC_RnkResGetRowCount invalid mode\n");
		goto exit;
	case DWC_RNK_ERROR_INVALID_PARAMETER:
		OS_Printf("[err]DWC_RnkResGetRowCount invalid parameter\n");
		goto exit;
	case DWC_RNK_ERROR_NOTCOMPLETED:
		OS_Printf("[err]DWC_RnkResGetRowCount not completed\n");
		goto exit;
	case DWC_RNK_ERROR_EMPTY_RESPONSE:
		OS_Printf("[err]DWC_RnkResGetRowCount empty response\n");
		goto exit;
	}
	
	for( i=0; i<count; i++ ){

		DWCRnkData data;

		if( DWC_RnkResGetRow( &data, (u32)i ) != DWC_RNK_SUCCESS ){
			OS_Printf("[err]DWC_RnkResGetRow()\n");
			break;
		}

		// 出力
		OS_TPrintf("%dth score=%d pid=%d region=%d update=%dmin before ",
						data.order,
						data.score,
						data.pid,
						data.region,
						data.lastupdate );

		// ユーザー定義データの正当性チェック
		{
			MYUSERDEFINEDDATA*	mydata;
			
			mydata = (MYUSERDEFINEDDATA*)data.userdata;

			if(mydata->size != 32){

				// データサイズが不正
				OS_TPrintf("userdata=[invalid data size]\n");

			}else if(mydata->checksum != (u32)MATH_CalcChecksum8(
										(const void*)mydata->data,
										mydata->size )){

				// Checksumが不正
				OS_TPrintf("userdata=[invalid checksum]\n");
			
			}else{

				// 正しいデータ
				OS_TPrintf("userdata=%s\n", mydata->data);

			}

		}

	}

exit:
	// ランキングモジュールの終了
	DWC_RnkShutdown();
	
	// 終了
	OS_Printf("Terminated.\n");	

	OS_Terminate();

}
