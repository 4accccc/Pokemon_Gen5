/*===========================================================================*


   This demo program uses following database settings.

   ■table setting
   　　table name　　　：　demotable
   　  table type    　：　Profile type
   　　permission    　：　all checked (all permitted)
   　　limit per owner ：  10

   ■field setting：
   　　recordid     [Int]
       ownerid      [Int]
       demoscore    [Int]
       demostage    [Short]
       demodate     [DateAndTime]
       demofile     [FileID]


 *===========================================================================*/


#include <nitro.h>
#include <dwc.h>
#include <gdb/dwc_gdb.h>
#include  <cstring>


// ヘッダファイルインクルード
//-------------------------------------
#include "main.h"
#include "init.h"
#include "utility.h"

// 定数宣言
//---------------------------------------------------------
#define FRIEND_LIST_LEN  64
#define TABLE_NAME "demotable"
#define TIMEOUT_MSEC (20*1000)

// 簡易データベースライブラリテスト用アカウント（本番では変える必要があります）
#define GAME_NAME        "dwctest"
#define SECRET_KEY       "d4q9GZ"	// このサンプルが使用するシークレットキー
#define GAME_PRODUCTID   10824		// このサンプルが使用するプロダクトID
#define GAME_ID           1408       // このサンプルが使用するゲームID
#define INITIAL_CODE     'NTRJ'     // イニシャルコード

#define CREATE_NUM  4

//#define USE_AUTHSERVER_PRODUCTION // 製品向け認証サーバを使用する場合有効にする


// グローバル変数宣言
//---------------------------------------------------------
DWCInetControl stConnCtrl;
static int gprofile_id;                       // profileid = ownerid


// 関数プロトタイプ
//---------------------------------------------------------
static BOOL StartIPMain(void);
static void VBlankIntr(void);
static void login_callback(DWCError error, int profileid, void *param);
static DWCError wait_async_end(void);
static void gdb_demo_exit(void);
static void get_records_callback(int record_num, DWCGdbField** records, void* user_param);
static void print_field(DWCGdbField* field);
static void download_file_callback(const void* data, int size, void* user_param);
static void file_progress_callback(int bytesDone, int totalBytes, void* user_param);


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
 *    @brief    IPの取得
 *
 *    @retval    TRUE        成功
 *    @retval    FALSE       失敗
 */
//=============================================================================
static BOOL StartIPMain(void)
{
    DWC_InitInet( &stConnCtrl );

    DWC_SetRwinSize(65535);

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




/*---------------------------------------------------------------------------*
  ログインが完了したときに呼び出されるコールバック関数
 *---------------------------------------------------------------------------*/
static void
login_callback(DWCError error, int profileid, void *param)
{
    OS_TPrintf("gdb_sample_DEBUG: Login callback : %d\n", error );

    if ( error == DWC_ERROR_NONE )
    {
        *((DWCError*)param) = error;
        OS_TPrintf("gdb_sample_DEBUG: Login Success. ProfileID=%d\n", profileid );
        gprofile_id = profileid;
    }
    else
    {
        *((DWCError*)param) = error;
        OS_TPrintf("gdb_sample_DEBUG: Login Failed\n" );
    }
}


/*---------------------------------------------------------------------------*
  非同期処理待ち関数
 *---------------------------------------------------------------------------*/
static DWCError
wait_async_end()
{
    OSTick start_time = OS_GetTick();
    DWCError err = DWC_ERROR_NONE;
    int cancelflag = 1;
    
    for (;;)
    {
        DWCGdbState state  =  DWC_GdbGetState();
        if(state != DWC_GDB_STATE_IN_ASYNC_PROCESS &&
           state != DWC_GDB_STATE_IN_CANCEL_PROCESS)
        {
            break;
        }
        
        DWC_ProcessFriendsMatch();
        if ((err = DWC_GetLastError(NULL)) != DWC_ERROR_NONE)
	    {
            return err;
	    }
	    
        DWC_GdbProcess();
        
        // タイムアウト確認
        if(OS_TicksToMilliSeconds(OS_GetTick() - start_time) > TIMEOUT_MSEC && cancelflag)
        {
            OS_TPrintf("gdb_sample_DEBUG: wait async timeout.\n");
            DWC_GdbCancelRequest();
            cancelflag = 0;
        }
        OS_WaitVBlankIntr();
    }
    return err;
}


/*---------------------------------------------------------------------------*
  デモプログラム終了処理関数
 *---------------------------------------------------------------------------*/
static void
gdb_demo_exit()
{
    int errorCode;
    DWCErrorType errorType;
    
    // エラーがあればエラーコードとエラータイプを表示
    if( DWC_ERROR_NONE != DWC_GetLastErrorEx( &errorCode, &errorType ) )
    {
        OS_TPrintf("gdb_sample_DEBUG: Error code %d   DWCErrorType[%d].\n", errorCode, errorType );
    }
    DWC_GdbShutdown();           // 簡易データベースライブラリの終了
    DWC_ShutdownFriendsMatch();  // Friendマッチライブラリの終了
    DWC_CleanupInet();
    OS_TPrintf("gdb_sample_DEBUG: OS Terminated.\n");
    Heap_Dump();

}


/*---------------------------------------------------------------------------*
  レコード取得時のコールバック関数　-　簡易データベースライブラリ
 *---------------------------------------------------------------------------*/
static void
get_records_callback(int record_num, DWCGdbField** records, void* user_param)
{
    int i,k;

    for (i = 0; i < record_num; i++)
    {
        OS_TPrintf("gdb_sample_DEBUG:　");
        for (k = 0; k < *(int*)user_param; k++)   // user_param -> field_num
        {
            print_field(&records[i][k]);
            OS_TPrintf(" ");
        }
        OS_TPrintf("\n");
    }
}


static void
print_field(DWCGdbField* field) // レコードをデバッグ出力する。
{
    OS_TPrintf("%s[", field->name);
    switch (field->type)
    {
    case DWC_GDB_FIELD_TYPE_BYTE:
        OS_TPrintf("BYTE]:%d", field->value.int_u8);
        break;
    case DWC_GDB_FIELD_TYPE_SHORT:
        OS_TPrintf("SHORT]:%d", field->value.int_s16);
        break;
    case DWC_GDB_FIELD_TYPE_INT:
        OS_TPrintf("INT]:%d", field->value.int_s32);
        break;
    case DWC_GDB_FIELD_TYPE_FLOAT:
        OS_Printf("FLOAT]:%f", field->value.float_f64);
        break;
    case DWC_GDB_FIELD_TYPE_ASCII_STRING:
        OS_TPrintf("ASCII]:%s", field->value.ascii_string);
        break;
    case DWC_GDB_FIELD_TYPE_UNICODE_STRING:
        OS_Printf("UNICODE]");
        break;
    case DWC_GDB_FIELD_TYPE_BOOLEAN:
        OS_TPrintf("BOOL]:%d", field->value.boolean);
        break;
    case DWC_GDB_FIELD_TYPE_DATE_AND_TIME:
        OS_TPrintf("TIME]:%lld", field->value.datetime);
        break;
    case DWC_GDB_FIELD_TYPE_BINARY_DATA:
        OS_TPrintf("BINARY]:%d", field->value.binary_data.size);
        break;
    }
}


/*---------------------------------------------------------------------------*
  ファイル取得時のコールバック関数　-　簡易データベースライブラリ
 *---------------------------------------------------------------------------*/
static void
download_file_callback(const void* data, int size, void* user_param)
{
#pragma unused(user_param)
#ifdef SDK_FINALROM
#pragma unused(data)
#pragma unused(size)
#endif
    OS_TPrintf("gdb_sample_DEBUG: size: %d data: %lld \n", size, *(s64*)data);

}


/*---------------------------------------------------------------------------*
  ファイル送受信時の進行状況取得コールバック関数　-　簡易データベースライブラリ
 *---------------------------------------------------------------------------*/
static void
file_progress_callback(int bytesDone, int totalBytes, void* user_param)
{
#pragma unused(user_param)

    OS_TPrintf("gdb_sample_DEBUG: bytesDone: %d totalBytes: %d \n", bytesDone, totalBytes);
}


//=============================================================================
/*!
 *    @brief    メイン
 */
//=============================================================================
void NitroMain()
{
    int i;
    int ret = 0;

    DWCUserData     userdata;                    // ユーザーデータ
    DWCFriendData   friendlist[FRIEND_LIST_LEN]; // フレンドリスト

    DWCGdbError          res;   // 簡易データベースライブラリのエラー取得用
    DWCGdbState        state;   // 簡易データベースライブラリの状態取得用
    DWCGdbAsyncResult  result;  // 簡易データベースライブラリ結果取得用
    
    DWCError    err = DWC_ERROR_NONE;   // 非同期通信待ち状態時のエラー取得用

    int record_id[CREATE_NUM];      // レコードの作成デモで作成したレコードID
    int record_id_file;             // ファイルアップロード用のレコードID
    
    DWCGdbField fields_create[2];   // レコードを作成する際に設定するレコード情報
    DWCGdbField fields_update[2];   // レコードを更新する際に設定するレコード情報


    // initialize
    //-----------------------------------------------------------------------
    OS_Init();

    Heap_Init();
    Heap_SetDebug(TRUE);

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
    OS_WaitVBlankIntr();  // Waiting the end of VBlank interrupt
    GX_DispOn();
    GXS_DispOn();

    // デバッグ表示レベル指定
    DWC_SetReportLevel(DWC_REPORTFLAG_ALL);

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
    
    
    //IPの取得
    while (!StartIPMain())
    {
        OS_Printf("error at StartIPMain()\n");
    }

    //-----------------------------------------------------------------------
    // ログイン処理
    //-----------------------------------------------------------------------
    //
    // ここでユーザーデータの取得処理を行ってください。簡易データベース
    // ライブラリはGameSpyに依存しているため、少なくとも一度はログインをした
    // 有効なGameSpyProfileIDを取得しているユーザーデータでなければ初期化に失
    // 敗します。仮IDは無効です。
    //
    // 既に過去にログインして取得したユーザーデータがバックアップに保存さ
    // れている場合はそれを使用することができます。ただし、ガイドラインに定め
    // られているとおり、インターネットに接続する際にはその都度必ずログインす
    // る必要があることに注意してください
    //
    // 外部認証サーバーなどを使用している場合は有効なGameSpyProfileIDを取得す
    // ることはできませんので、簡易データベースライブラリは使用できません。そ
    // もそもその場合はGameSpyのサーバー資源を使用できません。
    //


    // ユーザデータを作成する
    OS_TPrintf("gdb_sample_DEBUG: Creating new UserData\n");
    DWC_CreateUserData( &userdata );

    // 友達リスト初期化
    memset( &friendlist, 0, sizeof( friendlist ) );

    // ユーザデータを表示
    DWC_ReportUserData( &userdata );
    OS_TPrintf("gdb_sample_DEBUG: Initialized\n");

    // フレンドマッチライブラリの初期化
    DWC_InitFriendsMatch( &userdata,
                          GAME_PRODUCTID,
                          SECRET_KEY,
                          0,
                          0,
                          friendlist,
                          FRIEND_LIST_LEN );

    // アプリケーションがGameSpyサーバを利用可能であるか調べ、リモート認証を行い、Wi-Fiコネクションへ接続
    {
        int login_result = -1;

        DWC_LoginAsync((u16*)L"name", NULL, login_callback, &login_result);
        while (login_result == -1)
        {
            OS_WaitVBlankIntr();
            DWC_ProcessFriendsMatch();
            if (DWC_GetLastError(NULL) != DWC_ERROR_NONE) OS_Terminate();
        }

    }


    // 簡易データベースライブラリを初期化
    //-----------------------------------------------------------------------
    res = DWC_GdbInitialize(GAME_ID, &userdata, DWC_SSL_TYPE_SERVER_AUTH);
    
    if (res == DWC_GDB_ERROR_NONE)
    {
        OS_Printf("gdb_sample_DEBUG: DWC_GdbInitialize() success\n");
    }
    else
    {
        OS_Printf("gdb_sample_DEBUG: DWCGdbError[%d] in DWC_GdbInitialize().  %s line[%d]\n",res,__FILE__,__LINE__);
        gdb_demo_exit();
        return;
    }



    // レコードを新規に作成する非同期処理
    //-----------------------------------------------------------------------
    OS_Printf("*********  DEMO_1  *********  create record\n");

    MI_CpuClear8(record_id,sizeof(record_id));

    fields_create[0].name = "demoscore";
    fields_create[0].type = DWC_GDB_FIELD_TYPE_INT;
    fields_create[0].value.int_s32 = 1000;
    fields_create[1].name = "demostage";
    fields_create[1].type = DWC_GDB_FIELD_TYPE_SHORT;
    fields_create[1].value.int_u8 = 5;


    for (i=0; i<CREATE_NUM; i++) // 後に続く検索などのデモのため、レコードを新規に４つ作成します。
    {
        res = DWC_GdbCreateRecordAsync(TABLE_NAME, fields_create, 2, &record_id[i]);

        if (res != DWC_GDB_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbError[%d] in DWC_GdbCreateRecordAsync().  %s line[%d]\n",res,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        if ((err = wait_async_end()) != DWC_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCError[%d]  %s line[%d]\n",err,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        if ((state = DWC_GdbGetState()) != DWC_GDB_STATE_IDLE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbState[%d] is improper state here.  %s line[%d]\n",state,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        result = DWC_GdbGetAsyncResult();
        if (result != DWC_GDB_ASYNC_RESULT_SUCCESS && result != DWC_GDB_ASYNC_RESULT_REQUEST_CANCELLED )
        {
            OS_Printf("error!! DWCGdbAsyncResult[%d].  LINE[%d]\n",result,__LINE__);
            gdb_demo_exit();
            return;
        }
    }



    // レコードを削除する非同期処理
    //-----------------------------------------------------------------------
    OS_Printf("*********  DEMO_2  *********  delete record\n");
    
    if(record_id[0] != 0)
    {
        res = DWC_GdbDeleteRecordAsync(TABLE_NAME, record_id[0]);// 最初に作成されたレコードを削除
    
        if (res != DWC_GDB_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbError[%d] in DWC_GdbDeleteRecordAsync().  %s line[%d]\n",res,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }
    
        if ((err = wait_async_end()) != DWC_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCError[%d]   %s line[%d]\n",err,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }
    
        if ((state = DWC_GdbGetState()) != DWC_GDB_STATE_IDLE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbState[%d] is improper state here.  %s line[%d]\n",state,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }
    
        result = DWC_GdbGetAsyncResult();
        if (result != DWC_GDB_ASYNC_RESULT_SUCCESS && result != DWC_GDB_ASYNC_RESULT_REQUEST_CANCELLED )
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbAsyncResult[%d].  %s line[%d]\n",result,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }
    }
    else
    {
        OS_TPrintf("gdb_sample_DEBUG: Target record is not found. skip.\n");
    }


    // レコードを更新する非同期処理
    //-----------------------------------------------------------------------
    OS_Printf("*********  DEMO_3  *********  update record\n");
    
    if(record_id[1] != 0)
    {
        fields_update[0].name = "demoscore";
        fields_update[0].type = DWC_GDB_FIELD_TYPE_INT;
        fields_update[0].value.int_s32 = 1500;    // 1000 だったものを、1500 に更新する
        fields_update[1].name = "demostage";
        fields_update[1].type = DWC_GDB_FIELD_TYPE_SHORT;
        fields_update[1].value.int_u8 = 10;        // 5 だったものを、10 に更新する
    
        res = DWC_GdbUpdateRecordAsync(TABLE_NAME, record_id[1], fields_update, 2); // 2番目に作成したレコードを更新
    
        if (res != DWC_GDB_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbError[%d] in DWC_GdbUpdateRecordAsync().  %s line[%d]\n",res,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }
    
        if ((err = wait_async_end()) != DWC_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCError[%d]   %s line[%d]\n",err,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }
        
        if ((state = DWC_GdbGetState()) != DWC_GDB_STATE_IDLE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbState[%d] is improper state here.  %s line[%d]\n",state,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }
    
        result = DWC_GdbGetAsyncResult();
        if (result != DWC_GDB_ASYNC_RESULT_SUCCESS && result != DWC_GDB_ASYNC_RESULT_REQUEST_CANCELLED )
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbAsyncResult[%d].  %s line[%d]\n",result,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }
    }
    else
    {
        OS_TPrintf("gdb_sample_DEBUG: Target record is not found. skip.\n");
    }


    // 自分のレコードを全て取得する非同期処理
    //-----------------------------------------------------------------------
    OS_Printf("*********  DEMO_4  *********  get my all records\n");
    {
        const char* field_names[4] = {"recordid","ownerid","demostage","demoscore"};  // 検索で取得するフィールド名
        int field_num = sizeof(field_names)/sizeof(field_names[0]);  // 上記で設定したフィールド名の総数

        res = DWC_GdbGetMyRecordsAsync(TABLE_NAME, field_names, field_num, get_records_callback, &field_num);

        if (res != DWC_GDB_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbError[%d] in DWC_GdbGetMyRecordsAsync().  %s line[%d]\n",res,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        if ((err = wait_async_end()) != DWC_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCError[%d]   %s line[%d]\n",err,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        if ((state = DWC_GdbGetState()) != DWC_GDB_STATE_IDLE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbState[%d] is improper state here.  %s line[%d]\n",state,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        result = DWC_GdbGetAsyncResult();
        if (result != DWC_GDB_ASYNC_RESULT_SUCCESS && result != DWC_GDB_ASYNC_RESULT_REQUEST_CANCELLED )
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbAsyncResult[%d].  %s line[%d]\n",result,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }
    }



    // 指定したレコードIDのレコードを点数付けする非同期処理
    //-----------------------------------------------------------------------
    OS_Printf("*********  DEMO_5  *********  rate record\n");
    
    if(record_id[2] != 0)
    {
        res = DWC_GdbRateRecordAsync(TABLE_NAME, record_id[2], 5); // 3番目に作成したレコードを評価する（5点）
    
        if (res != DWC_GDB_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbError[%d] in DWC_GdbRateRecordAsync().  %s line[%d]\n",res,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }
    
        if ((err = wait_async_end()) != DWC_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCError[%d]   %s line[%d]\n",err,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }
        
        if ((state = DWC_GdbGetState()) != DWC_GDB_STATE_IDLE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbState[%d] is improper state here.  %s line[%d]\n",state,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }
    
        result = DWC_GdbGetAsyncResult();
        if (result != DWC_GDB_ASYNC_RESULT_SUCCESS && result != DWC_GDB_ASYNC_RESULT_REQUEST_CANCELLED )
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbAsyncResult[%d].  %s line[%d]\n",result,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }
    }
    else
    {
        OS_TPrintf("gdb_sample_DEBUG: Target record is not found. skip.\n");
    }


    // 指定したレコードIDのレコードを取得する非同期処理
    //-----------------------------------------------------------------------
    OS_Printf("*********  DEMO_6  *********  ged specified record\n");
    
    if(record_id[1] != 0 && record_id[2] != 0)
    {
        const char* field_names[3] = { "recordid", "num_ratings", "average_rating" }; //　取得したいフィールド名の設定
        int field_num = sizeof(field_names)/sizeof(field_names[0]);    // 上記で設定したフィールド名の総数
        int record_ids[2];

        // 2番目と3番目に作成したレコード
        record_ids[0] = record_id[1];
        record_ids[1] = record_id[2];

        DWC_GdbGetRecordsAsync( TABLE_NAME,
                                record_ids,
                                sizeof(record_ids)/sizeof(record_ids[0]),
                                field_names,
                                field_num,
                                get_records_callback,
                                &field_num );

        if (res != DWC_GDB_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbError[%d] in DWC_GdbGetRecordsAsync().  %s line[%d]\n",res,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        if ((err = wait_async_end()) != DWC_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCError[%d]   %s line[%d]\n",err,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        if ((state = DWC_GdbGetState()) != DWC_GDB_STATE_IDLE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbState[%d] is improper state here.  %s line[%d]\n",state,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        result = DWC_GdbGetAsyncResult();
        if (result != DWC_GDB_ASYNC_RESULT_SUCCESS && result != DWC_GDB_ASYNC_RESULT_REQUEST_CANCELLED )
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbAsyncResult[%d].  %s line[%d]\n",result,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }
    }
    else
    {
        OS_TPrintf("gdb_sample_DEBUG: Target record is not found. skip.\n");
    }



    // 指定した条件でレコードをランダムに１つ取得する非同期処理
    //-----------------------------------------------------------------------
    OS_Printf("*********  DEMO_7  *********  ged record randomly under the specified condition(filter)\n");
    {
        const char* field_names[2] = { "recordid", "demoscore" };    //　取得したいフィールド名の設定
        const char* filter = "demoscore = 1000"; // 条件指定　ランダムに取得する範囲を限定する

        int field_num = sizeof(field_names)/sizeof(field_names[0]);    // 上記で設定したフィールド名の総数

        res = DWC_GdbGetRandomRecordAsync(TABLE_NAME, field_names, field_num, filter, get_records_callback, &field_num);

        if (res != DWC_GDB_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbError[%d] in DWC_GdbGetRandomRecordAsync().  %s line[%d]\n",res,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        if ((err = wait_async_end()) != DWC_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCError[%d]   %s line[%d]\n",err,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        if ((state = DWC_GdbGetState()) != DWC_GDB_STATE_IDLE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbState[%d] is improper state here.  %s line[%d]\n",state,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        result = DWC_GdbGetAsyncResult();
        if (result != DWC_GDB_ASYNC_RESULT_SUCCESS && result != DWC_GDB_ASYNC_RESULT_REQUEST_CANCELLED )
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbAsyncResult[%d].  %s line[%d]\n",result,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }
    }



    //  テーブルに設定された、１ユーザーが作成することのできる
    //  レコードの最大値と、現在作成している数を取得する非同期処理
    //  レコードの最大値は、削除されたレコードについてはカウントされません
    //-----------------------------------------------------------------------
    OS_Printf("*********  DEMO_8  *********  ged the count of record limit and now you have\n");
    {
        int limit_num;
        int owned_num;

        res = DWC_GdbGetRecordLimitAsync(TABLE_NAME, &limit_num, &owned_num);

        if (res != DWC_GDB_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbError[%d] in DWC_GdbGetRecordLimitAsync().  %s line[%d]\n",res,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        if ((err = wait_async_end()) != DWC_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCError[%d]   %s line[%d]\n",err,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        if ((state = DWC_GdbGetState()) != DWC_GDB_STATE_IDLE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbState[%d] is improper state here.  %s line[%d]\n",state,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        result = DWC_GdbGetAsyncResult();
        if (result != DWC_GDB_ASYNC_RESULT_SUCCESS && result != DWC_GDB_ASYNC_RESULT_REQUEST_CANCELLED )
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbAsyncResult[%d].  %s line[%d]\n",result,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        OS_Printf("gdb_sample_DEBUG: limit_num = %d ,  owned_num = %d\n",limit_num,owned_num);

    }



    //  ファイルに関する処理：
    //  ファイルアップロード後、データベースへファイルIDの登録し、
    //  登録したファイルIDを使って、ファイルメタデータに関する情報を取得後、
    //  ファイルをダウンロードします。
    //-----------------------------------------------------------------------
    OS_Printf("*********  DEMO_9  *********  upload a file, register the file_id, get the metadata, download the file.\n");
    {
        s64 data = 83912736414123Ull;            // アップロードデータ
        const char* filename = "sample_file";    // アップロードしたファイルに付けるファイル名
        int file_id;    // ファイルID格納用変数

        DWCGdbField fields[1]; // ファイルIDを、データベースに登録するときに用いるフィールド情報


        // ファイルをアップロードする非同期処理を開始
        //-----------------------------------------------------------------------
        res = DWC_GdbUploadFileAsync(&data, sizeof(s64), filename, &file_id, file_progress_callback);

        if (res != DWC_GDB_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbError[%d] in DWC_GdbUploadFileAsync().  %s line[%d]\n",res,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        if ((err = wait_async_end()) != DWC_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCError[%d]   %s line[%d]\n",err,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        if ((state = DWC_GdbGetState()) != DWC_GDB_STATE_IDLE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbState[%d] is improper state here.  %s line[%d]\n",state,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        result = DWC_GdbGetAsyncResult();
        if (result != DWC_GDB_ASYNC_RESULT_SUCCESS && result != DWC_GDB_ASYNC_RESULT_REQUEST_CANCELLED )
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbAsyncResult[%d].  %s line[%d]\n",result,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }


        record_id_file = 0;
        if(file_id != 0)
        {
            // ファイルIDをデータベースへ登録する非同期処理を開始
            // （レコードを新規に作成する非同期処理）
            //-----------------------------------------------------------------------
    
            fields[0].name = "demofile";                    // <=--------------------------------
            fields[0].type = DWC_GDB_FIELD_TYPE_INT;        // <=  ファイルID登録用レコード情報
            fields[0].value.int_s32 = file_id;              // <=--------------------------------
            
            res = DWC_GdbCreateRecordAsync(TABLE_NAME, fields, 1, &record_id_file);
            OS_TPrintf("gdb_sample_DEBUG: creating a record for fileid = %d\n",file_id);
    
            if (res != DWC_GDB_ERROR_NONE)
            {
                OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbError[%d] in DWC_GdbCreateRecordAsync().  %s line[%d]\n",res,__FILE__,__LINE__);
                gdb_demo_exit();
                return;
            }
    
            if ((err = wait_async_end()) != DWC_ERROR_NONE)
            {
                OS_TPrintf("gdb_sample_DEBUG: error!! DWCError[%d]   %s line[%d]\n",err,__FILE__,__LINE__);
                gdb_demo_exit();
                return;
            }
    
            if ((state = DWC_GdbGetState()) != DWC_GDB_STATE_IDLE)
            {
                OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbState[%d] is improper state here.  %s line[%d]\n",state,__FILE__,__LINE__);
                gdb_demo_exit();
                return;
            }
    
            result = DWC_GdbGetAsyncResult();
            if (result != DWC_GDB_ASYNC_RESULT_SUCCESS && result != DWC_GDB_ASYNC_RESULT_REQUEST_CANCELLED )
            {
                OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbAsyncResult[%d].  %s line[%d]\n",result,__FILE__,__LINE__);
                gdb_demo_exit();
                return;
            }
    
    
    
            // ファイルをダウンロードする非同期処理
            //-----------------------------------------------------------------------
    
            res = DWC_GdbDownloadFileAsync(file_id, 2048, TRUE, file_progress_callback, download_file_callback, NULL);
    
            if (res != DWC_GDB_ERROR_NONE)
            {
                OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbError[%d] in DWC_GdbDownloadFileAsync().  %s line[%d]\n",res,__FILE__,__LINE__);
                gdb_demo_exit();
                return;
            }
    
            if ((err = wait_async_end()) != DWC_ERROR_NONE)
            {
                OS_TPrintf("gdb_sample_DEBUG: error!! DWCError[%d]   %s line[%d]\n",err,__FILE__,__LINE__);
                gdb_demo_exit();
                return;
            }
    
            if ((state = DWC_GdbGetState()) != DWC_GDB_STATE_IDLE)
            {
                OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbState[%d] is improper state here.  %s line[%d]\n",state,__FILE__,__LINE__);
                gdb_demo_exit();
                return;
            }
    
            result = DWC_GdbGetAsyncResult();
            if (result != DWC_GDB_ASYNC_RESULT_SUCCESS && result != DWC_GDB_ASYNC_RESULT_REQUEST_CANCELLED )
            {
                OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbAsyncResult[%d].  %s line[%d]\n",result,__FILE__,__LINE__);
                gdb_demo_exit();
                return;
            }
        }
        else
        {
            OS_TPrintf("gdb_sample_DEBUG: Target record is not found. skip.\n");
        }

    }



    // 指定した条件に当てはまるレコードの数を取得する非同期処理
    //-----------------------------------------------------------------------
    OS_Printf("*********  DEMO_10  *********  get the count of records which can pass the filter\n");
    {
        char filter[30]; // 条件指定
        int count; // 取得したレコード数を格納する変数
        OS_SNPrintf(filter,sizeof(filter),"ownerid = %d",gprofile_id); // gprofile_id はグローバル変数でprofileIDを格納している

        res = DWC_GdbGetRecordCountAsync(TABLE_NAME, filter, FALSE, &count);

        if (res != DWC_GDB_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbError[%d] in DWC_GdbDeleteRecordAsync().  %s line[%d]\n",res,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

         if ((err = wait_async_end()) != DWC_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCError[%d]   %s line[%d]\n",err,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        if ((state = DWC_GdbGetState()) != DWC_GDB_STATE_IDLE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbState[%d] is improper state here.  %s line[%d]\n",state,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        result = DWC_GdbGetAsyncResult();
        if (result != DWC_GDB_ASYNC_RESULT_SUCCESS && result != DWC_GDB_ASYNC_RESULT_REQUEST_CANCELLED )
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbAsyncResult[%d].  %s line[%d]\n",result,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }
    }



    // 指定した検索条件でレコードを取得する非同期処理
    //-----------------------------------------------------------------------
    OS_Printf("*********  DEMO_11  *********  get records under the specified conditions(filter etc.)\n");
    {
        const char* field_names[5] = {  "recordid",
                                        "demofile",
                                        "demofile.size",            //<=-----------------------------
                                        "demofile.name",            //<= ファイルメタデータフィールド
                                        "demofile.downloads"        //<=-----------------------------
                                     };

        const DWCGdbSearchCond cond = { "demofile.downloads = 1 or demoscore = 1000",
                                        "- recordid", // 降順
                                        0,
                                        3,
                                        NULL,
                                        0,
                                        NULL,
                                        0,
                                        FALSE
                                      };

        int field_num = sizeof(field_names)/sizeof(field_names[0]);    // 上記で設定したフィールド名の総数

        res = DWC_GdbSearchRecordsAsync(TABLE_NAME, field_names, field_num, &cond, get_records_callback, &field_num);

        if (res != DWC_GDB_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbError[%d] in DWC_GdbSearchRecordsAsync().  %s line[%d]\n",res,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        if ((err = wait_async_end()) != DWC_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCError[%d]   %s line[%d]\n",err,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        if ((state = DWC_GdbGetState()) != DWC_GDB_STATE_IDLE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbState[%d] is improper state here.  %s line[%d]\n",state,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

        result = DWC_GdbGetAsyncResult();
        if (result != DWC_GDB_ASYNC_RESULT_SUCCESS && result != DWC_GDB_ASYNC_RESULT_REQUEST_CANCELLED )
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbAsyncResult[%d].  %s line[%d]\n",result,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }

    }



    // フィールドの配列から、指定した名前のフィールドを取得する。
    // サーバとの通信は行いません。ローカルなデータに対して行う処理です。
    //-----------------------------------------------------------------------
    OS_Printf("*********  DEMO_12  *********  get a specified field from any field array.\n");
    {
        DWCGdbField fields[5];
        DWCGdbField *field_get;
        int field_num = sizeof(fields)/sizeof(fields[0]);  // 上記で設定したフィールド名の総数

        fields[0].name = "field_1";
        fields[0].type = DWC_GDB_FIELD_TYPE_INT;
        fields[0].value.int_s32 = 1;
        fields[1].name = "field_2";
        fields[1].type = DWC_GDB_FIELD_TYPE_INT;
        fields[1].value.int_s32 = 2;
        fields[2].name = "field_3";
        fields[2].type = DWC_GDB_FIELD_TYPE_INT;
        fields[2].value.int_s32 = 3;
        fields[3].name = "field_4";
        fields[3].type = DWC_GDB_FIELD_TYPE_INT;
        fields[3].value.int_s32 = 4;
        fields[4].name = "field_5";
        fields[4].type = DWC_GDB_FIELD_TYPE_INT;
        fields[4].value.int_s32 = 5;

        field_get = DWC_GdbGetFieldByName(fields[2].name, fields, field_num);

        OS_TPrintf("gdb_sample_DEBUG:  %s = %d\n",fields[2].name,field_get->value);

    }



    // データベースに登録したファイルIDを削除する非同期処理を開始
    // （レコードを削除する非同期処理）
    //  データベースからファイルIDが無くなると、当該ファイルは約24時間で
    //  ファイルサーバーから削除されます。
    //-----------------------------------------------------------------------
    OS_Printf("*********  DEMO_fin  *********  delete file id from the database to finalize this demo..\n");

    if(record_id_file != 0)
    {
        res = DWC_GdbDeleteRecordAsync(TABLE_NAME, record_id_file);
    
        if (res != DWC_GDB_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbError[%d] in DWC_GdbDeleteRecordAsync().  %s line[%d]\n",res,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }
    
        if ((err = wait_async_end()) != DWC_ERROR_NONE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCError[%d]   %s line[%d]\n",err,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }
    
        if ((state = DWC_GdbGetState()) != DWC_GDB_STATE_IDLE)
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbState[%d] is improper state here.  %s line[%d]\n",state,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }
    
        result = DWC_GdbGetAsyncResult();
        if (result != DWC_GDB_ASYNC_RESULT_SUCCESS && result != DWC_GDB_ASYNC_RESULT_REQUEST_CANCELLED )
        {
            OS_TPrintf("gdb_sample_DEBUG: error!! DWCGdbAsyncResult[%d].  %s line[%d]\n",result,__FILE__,__LINE__);
            gdb_demo_exit();
            return;
        }
    }
    else
    {
        OS_TPrintf("gdb_sample_DEBUG: Target record is not found. skip.\n");
    }


//デモ終了
    gdb_demo_exit();
    OS_Terminate();

}
