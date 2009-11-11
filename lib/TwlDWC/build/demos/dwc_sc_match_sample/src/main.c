#include <nitro.h>
#include <dwc.h>
#include  <cstring>

#include "main.h"
#include "init.h"
#include "utility.h"
#include "userdata.h"
#include "screen.h"

#include "dbs.h"

/** --------------------------------------------------------------------
  defines
  ----------------------------------------------------------------------*/
#define INITIAL_CODE     'NTRJ'    // このサンプルが仕様するイニシャルコード
#define GAME_NAME        "dwctest" // このサンプルが使用するゲーム名
#define GAME_SECRET_KEY  "d4q9GZ"  // このサンプルが使用するシークレットキー
#define GAME_PRODUCTID   10824     // このサンプルが使用するプロダクトID

#define	MAX_PLAYERS		 4	       // 自分も含めた接続人数
#define CB_CONNECT_SERVER 0        // サーバクライアントマッチメイクのサーバDS
#define CB_CONNECT_CLIENT 1        // サーバクライアントマッチメイクのクライアントDS
#define APP_CONNECTION_KEEPALIVE_TIME 30000 // キープアライブ時間
#define KEEPALIVE_INTERVAL (APP_CONNECTION_KEEPALIVE_TIME/5) // キー入力を待たずデータを転送する時間
#define FILTER_STRING "sample_filter"	// フィルタ文字列. 適宜変更してください
//#define USE_RELIABLE	// Reliable 通信を行う場合は定義しておいてください
//#define USE_AUTHSERVER_PRODUCTION // 製品向け認証サーバを使用する場合有効にする

/** --------------------------------------------------------------------
  globals
  ----------------------------------------------------------------------*/
KeyControl		g_KeyCtrl;

/** --------------------------------------------------------------------
  statics
  ----------------------------------------------------------------------*/
static DWCInetControl			s_ConnCtrl;
static int  s_state;
static BOOL s_logined;
static BOOL s_keepmatch;
static BOOL s_matched;
static BOOL s_canceld;
static BOOL s_closed;
static int  s_matched_frame;
static int  s_profileID = 0;
static u8   s_entry = 0;
static DWCTopologyType s_topologyType = DWC_TOPOLOGY_TYPE_HYBRID;
static u32  s_groupID = 0;

const int	SIZE_SEND_BUFFER = 512;
const int	SIZE_RECV_BUFFER = 4 * 1024;

static u8	s_send_buf[ SIZE_SEND_BUFFER ];
static u8	s_recv_buf[ DWC_MAX_CONNECTIONS ][ SIZE_RECV_BUFFER ];

// 通信でやり取りする位置データ構造
static struct Position
{
    s32  xpos;
    s32  ypos;
}
s_position[ MAX_PLAYERS ];  // ローカルで保持する位置データ

// 通信コールバックに渡すアプリケーション固有データサンプル
typedef struct tagCommunicationData
{
    s32 count;
}CommunicationData;
CommunicationData s_commData;

/** --------------------------------------------------------------------
  static functions
  ----------------------------------------------------------------------*/
static int update_init( void );
static int update_connect( void );
static int update_login( void );
static int update_online( void );
static int update_server( void );
static int update_client( void );
static int update_matching( void );
static int update_matched( void );
static int update_close( void );
static int update_disconnect( void );
static int update_save( void );
static int update_error( void );

static BOOL InputNumber( u8* number );
static void DispMenuMsg( const u8 figure, const u8 number );

static void cb_login(DWCError error, int profileID, void* param);
static void cb_updateServers(DWCError error, BOOL isChanged, void* param);
static void cb_newclient(int index, void* param );
static void cb_connect(DWCError error, BOOL cancel, BOOL self, BOOL isServer, int index, void* param);
static void cb_close(DWCError error, BOOL isLocal, BOOL isServer, u8  aid, int index, void* param);
static void cb_send( int size, u8 aid, void* param );
static void cb_recv( u8 aid, u8* buffer, int size, void* param );
static void cb_suspend( DWCSuspendResult result, BOOL suspend, void* data );

static void DispMessage( const char* main_msg, const char* msg_a, const char* msg_b, const char* msg_x, const char* msg_y,  const char* msg_l, const char* msg_r );

/*---------------------------------------------------------------------------*
  Vブランク割り込み関数
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    //---- upload pseudo screen to VRAM
    DC_FlushRange(gScreen, sizeof(gScreen));
    GX_LoadBG0Scr(gScreen, 0, sizeof(gScreen));

    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/** --------------------------------------------------------------------
  初期化処理
  ----------------------------------------------------------------------*/
static void
initFunctions( void )
{
	OS_Init();
    CARD_Init();
    OS_SetThreadStackWarningOffset(OS_GetCurrentThread(), 0x100);
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

    Heap_Init();
    Heap_SetDebug( TRUE );

    // 出力初期化
    InitScreen();
    dbs_DemoInit();
    DTDB_SetPrintOverride( TRUE );

    // キー入力初期化
    InitKeyData( &g_KeyCtrl );

    // 表示開始
    OS_WaitVBlankIntr();              // Waiting the end of VBlank interrupt
    GX_DispOn();
    GXS_DispOn();

}

/** --------------------------------------------------------------------
  1フレーム1回呼ばれる関数
  ----------------------------------------------------------------------*/
static void
update( void )
{
    int errcode;
    DWCError err;
    DWCErrorType errtype;

    switch ( s_state )
    {
    case state_init:			s_state = update_init();			break;
    case state_connect:			s_state = update_connect();			break;
    case state_login:			s_state = update_login();			break;
    case state_online:			s_state = update_online();			break;
    case state_server:          s_state = update_server();          break;
    case state_client:          s_state = update_client();          break;
    case state_matching:		s_state = update_matching();		break;
    case state_matched:			s_state = update_matched();			break;
    case state_close:			s_state = update_close();			break;
    case state_disconnect:		s_state = update_disconnect();		break;
    case state_save:            s_state = update_save();            break;
    case state_error:			s_state = update_error();			break;
    }

#if 1
    err = DWC_GetLastErrorEx(&errcode, &errtype);

    if ( err != DWC_ERROR_NONE && s_state != state_error )
    {
        // エラーが発生した場合、ここでは状態をエラー解除待ちにするだけ
        OS_TPrintf( "\n*******************************\n" );
        OS_TPrintf( "ERROR : %d : %d\n", err, errcode );
        OS_TPrintf( "Push START BUTTON to continue\n" );
        OS_TPrintf( "*******************************\n\n" );
        s_state = state_error;
    }

#else
    err = DWC_GetLastErrorEx(&errcode, &errtype);
    if ( err != DWC_ERROR_NONE )
    {
        OS_TPrintf("ERROR: %d:%d\n", err, errcode);
        DWC_ClearError();
        
        if ( errtype == DWC_ETYPE_DISCONNECT )
        {
            // 通信切断時はFriendsMatch処理の終了とインターネット接続の
            // Cleanupを行なう
            DWC_ShutdownFriendsMatch();
            s_state = state_disconnect;
        }
        else if ( errtype == DWC_ETYPE_FATAL )
        {
            // Fatal Errorなので終了
            OS_Panic("++FATAL ERROR\n");
        }
        else if ( errtype == DWC_ETYPE_LIGHT )
        {
            // 軽いエラーは何もせずにオンライン状態に戻る
            DTUD_DispFriendList();
            s_state = state_online;
        }
        else {
            // FriendsMatch処理のみ終了したら、再びFrientsMatch初期化を行なう
            pushResult( err );
            DWC_ShutdownFriendsMatch();
            s_state = state_connect;
        }
    }
#endif
}

/** --------------------------------------------------------------------
  インターネットに接続
  ----------------------------------------------------------------------*/
static int
update_init( void )
{
    // DS本体とDSカードのユーザデータが一致するか
    if ( DWC_CheckValidConsole( DTUD_GetUserData() ) )
    {
        DispMessage( "STATE : CONNECTING", NULL, NULL, NULL, NULL, NULL, NULL);
        
        DWC_InitInet( &s_ConnCtrl );
        
        DWC_ConnectInetAsync();
        
        return state_connect;
    }
    else
    {
        // 一致していない場合はインターネットに接続できない
        OS_TPrintf( "UserData is not Valid Console.\n" );
        return state_init;
    }

    return state_init;
}

/** --------------------------------------------------------------------
  接続中
  ----------------------------------------------------------------------*/
static int
update_connect( void )
{
    DWC_ProcessInet();

    if ( DWC_CheckInet() )
    {
        int status;

        status = DWC_GetLastError(NULL);

        if ( status == DWC_ERROR_NONE )
        {

            OS_TPrintf("Connected!..start FriendsMatch.\n");
            
            s_logined = FALSE;
            
            DWC_InitFriendsMatch(DTUD_GetUserData(),
                                 GAME_PRODUCTID, GAME_SECRET_KEY,
                                 0, 0,
                                 DTUD_GetFriendList(), FRIEND_LIST_LEN );
            
            // 認証用関数を使ってログイン
            DWC_LoginAsync(L"なまえ", NULL, cb_login, NULL);
            
            return state_login;
            
        }
        else
        {
            OS_TPrintf("Error\n");
            return state_disconnect;
        }
    }

    return state_connect;
}

/** --------------------------------------------------------------------
  ログイン中
  ----------------------------------------------------------------------*/
int
update_login( void )
{
    u8 i;
    
    DWC_ProcessFriendsMatch();  // DWC通信処理更新

    if ( s_logined )
    {
        // ingamesnチェックの結果を取得する
	    if( DWC_GetIngamesnCheckResult() == DWC_INGAMESN_INVALID )
        {
            // 不適切な名前と判断された場合は特別な処理が必要
		    OS_TPrintf("BAD ingamesn is detected by NAS.\n");
        }
        
        OS_TPrintf("================================\n");

        s_keepmatch = FALSE;
        s_matched   = FALSE;
        s_canceld   = FALSE;
        s_closed    = FALSE;

        DTUD_DispFriendList();

        // この段階でコネクションクローズコールバックを設定
        DWC_SetConnectionClosedCallback(cb_close, NULL);

        // 受信バッファも設定
        for ( i = 0; i < DWC_MAX_CONNECTIONS; ++i )
        {
            DWC_SetRecvBuffer( i, &s_recv_buf[i], SIZE_RECV_BUFFER );
        }

        // ログイン成功
        DispMessage( "STATE : ONLINE", "connect to server", "shutdown", "setup game server", "friend state", "save firned data",
                     s_groupID ? "groupID reconnect" : NULL );

        return state_online;
    }

    return state_login;
}

/** --------------------------------------------------------------------
  オンライン中
  ----------------------------------------------------------------------*/
int
update_online( void )
{
    BOOL changed = FALSE;
    
    DWC_ProcessFriendsMatch();  // DWC通信処理更新

    if ( g_KeyCtrl.trg & PAD_BUTTON_B )
    {
        // Bボタンで通信を切断し、初期状態に戻る
        DWC_ShutdownFriendsMatch();
        return state_disconnect;
    }
    else if ( g_KeyCtrl.trg & PAD_BUTTON_A )
    {
        // Aボタンでクライアントモードへ
        DTUD_DispFriendList();
        return state_client;
    }
    else if ( g_KeyCtrl.trg & PAD_BUTTON_X )
    {
        // Xボタンでサーバーモードへ
        DTUD_DispFriendList();
        return state_server;
    }
    else if ( g_KeyCtrl.trg & PAD_BUTTON_Y )
    {
        // Yボタンで友達の情報を表示する
        DTUD_DispFriendList();
    }
    else if ( g_KeyCtrl.trg & PAD_BUTTON_START )
    {
        // スタートボタンで通信を切断し、初期状態に戻る。
        DWC_ShutdownFriendsMatch();
        return state_disconnect;
    }
    else if ( g_KeyCtrl.trg & PAD_BUTTON_L )
    {
        DispMessage( "SAVE DATA?", "save", "cancel", NULL, NULL, NULL, NULL );
        return state_save;
    }
    else if ( g_KeyCtrl.trg & PAD_BUTTON_R )
    {
    	// R ボタンでグループID 接続を試す
    	if(s_groupID)
		{
            if( DWC_ConnectToGameServerByGroupID( s_topologyType,    // 接続形態
                                                s_groupID,        // グループID
                                                cb_connect,
                                                NULL,
                                                cb_newclient,
                                                NULL,
                                                NULL,
                                                NULL,
                                                NULL ))
            {
                DispMessage( "STATE : MATCHING", NULL, "cancel", NULL, NULL, NULL, NULL );
                return state_matching;
            }
		}
    }

    return state_online;
}

/** --------------------------------------------------------------------
  サーバDSの立ち上げ
  ----------------------------------------------------------------------*/
static int
update_server( void )
{
    static u8 s_entry;
    
    DWC_ProcessFriendsMatch();  // DWC通信処理更新

    DispMessage( "SETUP GAME SERVER", "set", "cancel", NULL, NULL, NULL, NULL );
    PrintString( 1, 14, 0xf, "MAX MATCHING NUMBER:");
    PrintString( 1, 15, 0xf, "Please input 2 or more.");
    
    if ( g_KeyCtrl.trg & PAD_BUTTON_B )
    {
        DispMessage( "STATE : ONLINE", "connect to server", "shutdown", "setup game server", "friend state", "save firned data", NULL );
        // Bボタンで通信をキャンセルする
        return state_online;
    }
    else if( InputNumber( &s_entry ) )
    {
        // 最大接続人数は必ず2人以上でなければならない
        if( s_entry > 1 ){
            DispMessage( "STATE : MATCHING", NULL, "cancel", NULL, NULL, NULL, NULL );
            
            DWC_SetupGameServer(	s_topologyType,		// 接続形態
                                    (u8)s_entry,	// マッチングする人数
                                    cb_connect,
                                    (void *)CB_CONNECT_SERVER,
                                    cb_newclient,	//
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL );				//
            s_entry = 0;
            
            return state_matching;
        }
    }
    return state_server;
}

/** --------------------------------------------------------------------
  クライアントDSの立ち上げ
  ----------------------------------------------------------------------*/
static int
update_client( void )
{
    static u8 s_entry;
    u8   maxEntry, numEntry;
    DWCFriendData* s_FriendList;

    DWC_ProcessFriendsMatch();  // DWC通信処理更新

    DispMessage( "CONNECT TO SERVER", "set", "cancel", NULL, NULL, NULL, NULL );
    PrintString( 1, 14, 0xf, "CONNECT FRIEND INDEX:");

    // 自動マッチメイク再開待ちの時はBボタンで解除する
    if ( g_KeyCtrl.trg & PAD_BUTTON_B )
    {
        // Bボタンで通信をキャンセルする
        DispMessage( "STATE : ONLINE", "connect to server", "shutdown", "setup game server", "friend state", "save firned data", NULL );
        return state_online;
    }
    else if( InputNumber( &s_entry ) )
    {
        s_FriendList = DTUD_GetFriendList();

        DWC_GetFriendStatusSC( &s_FriendList[s_entry], &maxEntry, &numEntry, NULL );
        if ( numEntry >= maxEntry )
        {
            // 既にサーバDSが定員オーバー
            OS_TPrintf( "GameServer is full!\n" );
            OS_TPrintf( "Do over again.\n" );
        }
        else
        {
            DispMessage( "STATE : MATCHING", NULL, "cancel", NULL, NULL, NULL, NULL );
        
            // サーバへ接続する
           DWC_ConnectToGameServerAsync(	s_topologyType,		// 接続形態
                                            s_entry,
                                            cb_connect,
                                            (void *)CB_CONNECT_CLIENT,
                                            cb_newclient,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL );				//
            s_entry = 0;
            
            return state_matching;
        }
    }
    return state_client;
}
        
/** --------------------------------------------------------------------
  マッチメイク中
  ----------------------------------------------------------------------*/
static int
update_matching( void )
{
    DWC_ProcessFriendsMatch();  // DWC通信処理更新

    if ( g_KeyCtrl.trg & PAD_BUTTON_B )
    {
        // Bボタンでマッチメイクをキャンセルする            
        s_canceld = TRUE;
    }

    if ( s_matched )
    {
        DispMessage( "STATE : MATCHED\nCrossKey : send position", 
                     "send all", "shutdown", "my state", "friend state", 
                     "suspend ON", "suspend OFF" );

        s_matched_frame = 0;

        return state_matched;
    }
    else if ( s_canceld )
    {
        // キャンセルされたらオンライン状態に戻る
        DispMessage( "STATE : ONLINE", "connect to server", "shutdown", "setup game server", "friend state", "save firned data",
                     s_groupID ? "groupID reconnect" : NULL );

        OS_TPrintf("Canceled connection ... retry\n");

        DWC_CloseAllConnectionsHard();

        s_canceld = FALSE;

        DTUD_DispFriendList();

        return state_online;
    }

    return state_matching;
}

/** --------------------------------------------------------------------
  マッチメイク完了
  ----------------------------------------------------------------------*/
static int
update_matched( void )
{
    struct Position sendPos;
    u8 me = DWC_GetMyAID();
    
    s_matched_frame++;

    DWC_ProcessFriendsMatch();  // DWC通信処理更新

    {
        // キープアライブでのタイムアウトがあるため、
        // 受信タイムアウト時間より短い間隔で、強制的にデータ送信する。
        static int lastKeepAliveSent = 0;
        int now = (int)DWCi_Np_TicksToMilliSeconds(DWCi_Np_GetTick());
        if (lastKeepAliveSent == 0 || (now - lastKeepAliveSent) > KEEPALIVE_INTERVAL)
        {
            g_KeyCtrl.trg |= PAD_BUTTON_A;
            lastKeepAliveSent = now;
        }
    }
    
    if ( g_KeyCtrl.trg & (PAD_KEY_UP | PAD_BUTTON_A))	s_position[ me ].ypos--;
    if ( g_KeyCtrl.trg & PAD_KEY_DOWN )    				s_position[ me ].ypos++;
    if ( g_KeyCtrl.trg & PAD_KEY_LEFT )    				s_position[ me ].xpos--;
    if ( g_KeyCtrl.trg & PAD_KEY_RIGHT )   				s_position[ me ].xpos++;
    
    // サーバDSがクローズしてstateが変わった場合はすぐに抜ける
    if ( s_state != state_matched )
    {
        DTUD_DispFriendList();
        
        return s_state;
    }
    
    if ( g_KeyCtrl.trg & (PAD_BUTTON_A |
                          PAD_KEY_UP | PAD_KEY_DOWN | PAD_KEY_LEFT | PAD_KEY_RIGHT))
    {
        sendPos.xpos = (s32)DWCi_HtoLEl((u32)s_position[ me ].xpos);
        sendPos.ypos = (s32)DWCi_HtoLEl((u32)s_position[ me ].ypos);
        // すべての相手に自分の位置を送信
#ifdef USE_RELIABLE
        DWC_SendReliableBitmap( DWC_GetAIDBitmap(),
                                (const void*)&sendPos,
                                sizeof( s32 ) * 2 );
#else
        DWC_SendUnreliableBitmap( DWC_GetAIDBitmap(),
                                  (const void*)&sendPos,
                                  sizeof( s32 ) * 2 );
#endif
    }
    else if ( g_KeyCtrl.trg & PAD_BUTTON_B )
    {
        // Bボタンでコネクションをクローズする
        int result = DWC_CloseAllConnectionsHard();
        
        OS_TPrintf( "CLOSE! result %d.\n", result );
        
        DTUD_DispFriendList();
        DispMessage( "STATE : ONLINE", "connect to server", "shutdown", "setup game server", "friend state", "save firned data",
                     s_groupID ? "groupID reconnect" : NULL );
        s_matched  = FALSE;
        s_canceld  = FALSE;
        return state_online;
    }
    else if ( g_KeyCtrl.trg & PAD_BUTTON_X )
    {
        // Xボタンで自分の情報を表示する
        u8* pAidList;
        int num;
        int i;

        num = DWC_GetAIDList(&pAidList);
        for ( i = 0; i < num; i++ )
        {
            if ( pAidList[i] == DWC_GetMyAID() ) break;
        }
        OS_TPrintf( "[%d] aid:%d, num:%d\n", i, DWC_GetMyAID(), num );
    }
    else if ( g_KeyCtrl.trg & PAD_BUTTON_Y )
    {
        // Yボタンで友達の情報を表示する
        DTUD_DispFriendList();
    }
    else if ( g_KeyCtrl.trg & (PAD_BUTTON_L | PAD_BUTTON_R))
    {
    	BOOL suspend = g_KeyCtrl.trg & PAD_BUTTON_L ? TRUE : FALSE;
    	
    	// X, Yボタンで締め切り処理開始
    	if(DWC_RequestSuspendMatchAsync(suspend, cb_suspend, NULL))
    	{
    		OS_TPrintf( "Suspend %s Start.\n", suspend ? "TRUE" : "FALSE" );
    	}
    }

    return state_matched;
}

/** --------------------------------------------------------------------
  クローズ中
  ----------------------------------------------------------------------*/
static int
update_close( void )
{
    s_logined = FALSE;

    return state_login;
}

/** --------------------------------------------------------------------
  切断された。
  ----------------------------------------------------------------------*/
static int
update_disconnect( void )
{
    if ( DWC_CleanupInetAsync() )
    {
        return state_init;
    }

    return state_disconnect;
}

/** --------------------------------------------------------------------
  データをセーブする。
  ----------------------------------------------------------------------*/
static int
update_save( void )
{
    DWC_ProcessFriendsMatch();  // DWC通信処理更新
    
    if ( g_KeyCtrl.trg & PAD_BUTTON_A )
    {
        if( DWC_CheckDirtyFlag( DTUD_GetUserData() ) ){
            DTUD_SaveUserData();
            OS_TPrintf( "Saved User Data.\n" );
        }
        
        DTUD_SaveFriendList();
        OS_TPrintf( "Saved Friend Data.\n" );

        DispMessage( "STATE : ONLINE", "connect to server", "shutdown", "setup game server", "friend state", "save firned data",
                    s_groupID ? "groupID reconnect" : NULL );
        return state_online;
    }
    else if ( g_KeyCtrl.trg & PAD_BUTTON_B )
    {
        DispMessage( "STATE : ONLINE", "connect to server", "shutdown", "setup game server", "friend state", "save firned data",
                    s_groupID ? "groupID reconnect" : NULL );
        return state_online;
    }
    return state_save;
}

/** --------------------------------------------------------------------
  エラー解除待ち
  ----------------------------------------------------------------------*/
static int update_error( void )
{
    int errcode;
    DWCError err;
    DWCErrorType errtype;
    
    if ( g_KeyCtrl.trg & PAD_BUTTON_START )
    {
        // スタートボタンでエラー解除
        err = DWC_GetLastErrorEx(&errcode, &errtype);

        DWC_ClearError();
        
        if ( errtype == DWC_ETYPE_DISCONNECT )
        {
            // 通信切断時はFriendsMatch処理の終了とインターネット接続の
            // Clearupを行なう
            DWC_ShutdownFriendsMatch();
            return state_disconnect;
        }
        else if ( errtype == DWC_ETYPE_FATAL )
        {
            // Fatal Errorなので終了
            OS_Panic("++FATAL ERROR\n");
            return state_init;  // not reached;
        }
        else if ( errtype == DWC_ETYPE_LIGHT )
        {
            // 軽いエラーは何もせずにオンライン状態に戻る
            DTUD_DispFriendList();
            DispMessage( "STATE : ONLINE", "connect to server", "shutdown", "setup game server", "friend state", "save firned data",
                         s_groupID ? "groupID reconnect" : NULL );
            return state_online;
        }
        else {
            // FriendsMatch処理のみ終了したら、再びFrientsMatch初期化を行なう
            DWC_ShutdownFriendsMatch();
            return state_connect;
        }
        
        s_matched   = FALSE;
        s_canceld   = FALSE;
    }

    return state_error;
}

/** --------------------------------------------------------------------
  ----------------------------------------------------------------------*/
/** --------------------------------------------------------------------
  コールバック関数
  ----------------------------------------------------------------------*/
static void
cb_login(DWCError error, int profileID, void* param)
{
    (void)param;
    if (error == DWC_ERROR_NONE)
    {
        // 認証成功、プロファイルID取得
        OS_TPrintf("Login succeeded. profileID = %u\n\n", profileID);

        s_profileID = profileID;  // プロファイルIDを記録

        // stUserDataが更新されているかどうかを確認。
        if ( DTUD_CheckUpdateUserData() )
        {
            DTUD_SaveUserData();
        }

        // 友達リストとGameSpyサーバDS上のバディリストの同期処理
        DWC_UpdateServersAsync(NULL,
                               cb_updateServers, NULL,
                               NULL, NULL,
                               NULL, NULL);
                               
		// キープアライブ時間の設定        
        DWC_SetConnectionKeepAliveTime(APP_CONNECTION_KEEPALIVE_TIME);
		
        // コネクションクローズコールバックを設定
        DWC_SetConnectionClosedCallback(cb_close, NULL);

        // 送信コールバックを設定
        DWC_SetUserSendCallback( cb_send, &s_commData );

        // 受信コールバックを設定
        DWC_SetUserRecvCallback( cb_recv, &s_commData );
    }
    else
    {
        OS_TPrintf("Error\n");
    }
}

static void
cb_updateServers(DWCError error, BOOL isChanged, void* param)
{
    (void)param;
    (void)isChanged;

    if ( error == DWC_ERROR_NONE )
    {
        // 友達リスト同期処理成功
        OS_TPrintf( "Update Servers succeeded.\n" );
        
        s_logined = TRUE;
    }
    else
    {
        OS_TPrintf("Error\n");
    }
}

static void
cb_newclient(int index, void* param )
{
    (void)param;

    OS_TPrintf( "New Client is connecting(idx %d)...\n", index);
}

static void
cb_connect(DWCError error, BOOL cancel, BOOL self, BOOL isServer, int index, void* param)
{

    if ( (int)param == CB_CONNECT_SERVER )
    {
        // サーバDS
        OS_TPrintf( "connected callback : SERVER\n" );
    }
    else if ( (int)param == CB_CONNECT_CLIENT )
    {
        // クライアントDS
        OS_TPrintf( "connected callback : CLIENT\n" );
    }
    OS_TPrintf( "err:%d, cancel:%d, self:%d, srv:%d, idx:%d\n",
                error, cancel, self, isServer, index );

    if ( error == DWC_ERROR_NONE )
    {
        if ( !cancel )
        {
            int i, num;
            u8* pAidList;

            num = DWC_GetAIDList(&pAidList);
            for (i = 0; i < num; i++)
            {
                if (pAidList[i] == DWC_GetMyAID())
                {
                    continue;
                }

                // 自分のAID以外の受信バッファをセットする
                DWC_SetRecvBuffer( pAidList[i], &s_recv_buf[pAidList[i]], SIZE_RECV_BUFFER );
            }
            
            // 接続成功
            s_matched = TRUE;
            
            // グループID を保存
            s_groupID = DWC_GetGroupID();
        }
        else if ( self || isServer )
        {
            // 自分がマッチメイクをキャンセルした、もしくは自分がクライアントで
            // サーバDSがマッチメイクをキャンセルした
            s_canceld = TRUE;
        }
        // 新規接続クライアントがマッチメイクをキャンセルしても何もしない
    }
    else
    {
        OS_TPrintf("Error\n");
    }

    s_keepmatch = FALSE;
}

static void
cb_close(DWCError error, BOOL isLocal, BOOL isServer, u8  aid, int index, void* param)
{
    (void)param;

    OS_TPrintf( "Closed Callback err:%d, local:%d, srv:%d, aid:%d, idx:%d\n",
               error, isLocal, isServer, aid, index );

    if ( error == DWC_ERROR_NONE )
    {
        if(isLocal)
        {
        	// 自分でクローズした場合は online に戻す
            s_closed = FALSE;
            s_state  = state_online;
            DispMessage( "STATE : ONLINE", "connect to server", "shutdown", "setup game server", "friend state", "save firned data", NULL );
        }
    }
    else
    {
        OS_TPrintf("Error\n");
    }
}

static void
cb_send( int size, u8 aid, void* param )
{
    CommunicationData* commData = (CommunicationData*)param;
    commData->count++;  // 送信したら加算
    OS_TPrintf( "Send Callback aid:%d, size:%d mycount:%d\n", aid, size, commData->count );
}

static void
cb_recv( u8 aid, u8* buffer, int size, void* param )
{
    if ( size == 8 )
    {
        CommunicationData* commData = (CommunicationData*)param;
        commData->count--;	// 受信したら減算
        s_position[ aid ].xpos = (s32)DWCi_LEtoHl(((u32*)buffer)[0]);
        s_position[ aid ].ypos = (s32)DWCi_LEtoHl(((u32*)buffer)[1]);

        OS_TPrintf( "[aid:%d] x:% 8d y:% 8d mycount:%d\n",
                       aid,
                       s_position[ aid ].xpos,
                       s_position[ aid ].ypos,
                       commData->count);
    }
    else
    {
        OS_TPrintf( "invalid recv data size\n" );
    }
}

static void
cb_suspend( DWCSuspendResult result, BOOL suspend, void* data )
{
	(void)data;
	OS_TPrintf( "Suspend Callback result:%d suspend:%s\n",
	            result, suspend ? "TRUE" : "FALSE" );
}

/** -------------------------------------------------------------------------
  ---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         InputNumber

  Description:  友達登録鍵の数値入力を行う。

  Arguments:    maxFigure - 入力する最大の桁数

  Returns:      入力終了ならTRUE
                入力中ならFALSEを返す。
 *---------------------------------------------------------------------------*/
static BOOL InputNumber( u8* number )
{
    static u8 figure = 0;
    u8 maxFigure = 2;
    u8 num;

    num = *number;

    DispMenuMsg( figure, num );
    
    if (g_KeyCtrl.trg & PAD_BUTTON_A)
    {
        figure = 0;
        return TRUE;
    }
    else if ( g_KeyCtrl.trg & PAD_KEY_UP )
    {
        // 十字キー上下で選択中の数値の増減
        if ( (num/UtilPow(10, figure) )%10 == 9)
        {
            num -= UtilPow(10, figure)*9;
        }
        else
        {
            num += UtilPow(10, figure);
        }
        // 入力中数値表示
        DispMenuMsg( figure, num );
    }
    else if ( g_KeyCtrl.trg & PAD_KEY_DOWN )
    {
        // 十字キー上下で選択中の数値の増減
        if ( (num/UtilPow(10, figure) )%10 == 0)
        {
            num += UtilPow(10, figure)*9;
        }
        else
        {
            num -= UtilPow(10, figure);
        }
        // 入力中数値表示
        DispMenuMsg( figure, num );
    }
    else if ( g_KeyCtrl.trg & PAD_KEY_LEFT )
    {
        // 十字キー左右で選択桁数の移動
        if (figure < maxFigure-1) figure++;
        // 入力中数値表示
        DispMenuMsg( figure, num );
    }
    else if ( g_KeyCtrl.trg & PAD_KEY_RIGHT )
    {
        // 十字キー左右で選択桁数の移動
        if (figure > 0) figure--;
        // 入力中数値表示
        DispMenuMsg( figure, num );
    }

    *number = num;
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DispMenuMsg

  Description:  友達登録中の数値を表示する。

  Arguments:    figure    - 入力中の桁数
                maxFigure - 最大桁数

  Returns:      NONE.
 *---------------------------------------------------------------------------*/
static void DispMenuMsg( const u8 figure, const u8 number )
{
    PrintString( 1, 17, 0xf, "Input Number:" );
    PrintString( 15, 17, 0xf, "%2.2u", number );

// 選択中の数字を示す為の下線を表示する
    PrintString( (s16)(16-figure), 18, 0xf, "-" );
}

/*---------------------------------------------------------------------------*
  Name:         DispMessage

  Description:  汎用のメッセージを表示する。

  Arguments:    main_msg - 現在のモード
                msg_a    - Aボタンが押された時の処理
                msg_b    - Bボタンが押された時の処理
                msg_x    - Xボタンが押された時の処理
                msg_y    - Yボタンが押された時の処理
                msg_l    - Lボタンが押された時の処理
                msg_r    - Rボタンが押された時の処理

  Returns:      NONE.
 *---------------------------------------------------------------------------*/
static void
DispMessage( const char* main_msg, const char* msg_a, const char* msg_b, const char* msg_x, const char* msg_y,  const char* msg_l, const char* msg_r )
{
    s16 x = 1;
    s16 y = 5;

    ClearScreen();
    
    PrintString( 0, 0, 0xf, "================================" );
    PrintString( 2, 1, 0xf, "*** SC MATCHING SAMPLE ***" );
    PrintString( 0, 2, 0xf, "================================" );
    
    if ( main_msg == NULL ) return;

    PrintString( 0, y, 0xf, "--------------------------------" );
    y++;
    // メインメッセージ表示
    while ( *main_msg != '\0' )
    {
        if ( *main_msg == '\n' )
        {
            x = 1;  y++;
        }
        else
        {
            PrintString( x, y, 0xf, "%c", *main_msg );
            x++;
        }
        main_msg++;
    }
    y++;
    PrintString( 0, y, 0xf, "--------------------------------" );

    y += 2;

    // Aボタン説明、Bボタン、Xボタン、Yボタン説明表示
    if ( msg_a != NULL )
    {
        PrintString( 1, y, 0xf, "A BUTTON:%s", msg_a );
    }
    y++;
    if ( msg_b != NULL )
    {
        PrintString( 1, y, 0xf, "B BUTTON:%s", msg_b );
    }
    y += 2;
    if ( msg_x != NULL )
    {
        PrintString( 1, y, 0xf, "X BUTTON:%s", msg_x );
    }
    y++;
    if ( msg_y != NULL )
    {
        PrintString( 1, y, 0xf, "Y BUTTON:%s", msg_y );
    }
    y += 2;
    if ( msg_l != NULL )
    {
        PrintString( 1, y, 0xf, "L BUTTON:%s", msg_l );
    }
    y++;
    if ( msg_r != NULL )
    {
        PrintString( 1, y, 0xf, "R BUTTON:%s", msg_r );
    }
}

/** --------------------------------------------------------------------
  毎フレーム呼び出す、状態表示関数
  ----------------------------------------------------------------------*/
void
drawConsole( void )
{
    static int i = 0;
    char loop[] = "/|\\-";
    char state[16];
    int  matchType = DWC_GetLastMatchType();

    if ( ( DWC_GetState() != DWC_STATE_CONNECTED ) &&
         ( DWC_GetState() != DWC_STATE_MATCHING ) )
    {
        strcpy( state, "**" );
    }
    else if ( matchType == DWC_MATCH_TYPE_SC_SV )
    {
        strcpy( state, "SV" );
    }
    else if ( matchType == DWC_MATCH_TYPE_SC_CL )
    {
        strcpy( state, "CL" );
        if ( DWC_GetState() == DWC_STATE_CONNECTED )
        {
            strcat( state, "-CONN" );
        }
        else if ( DWC_GetState() == DWC_STATE_MATCHING )
        {
            strcat( state, "-MATCH" );
        }
    }
    else
    {
        strcpy( state, "**" );
    }
    
    dbs_Print( 0, 0, "s:%s", state );
    dbs_Print( 13, 0, "n:%d", DWC_GetNumConnectionHost() );
    dbs_Print( 0, 1, "w:%d", DWC_GetLinkLevel() );
    dbs_Print( 4, 1, "ss:%d", s_state );
    dbs_Print( 13, 1, "p:%d", s_profileID );

    dbs_Print( 30,0, "%c", loop[((i)>>2)&3] );
    dbs_Print( 0, 2, "g:%d", s_groupID );
    i++;
}

/** --------------------------------------------------------------------
  エントリポイント
  ----------------------------------------------------------------------*/
void NitroMain ()
{
    int ret;
    
    initFunctions();

    // デバッグ表示レベル指定
    DWC_SetReportLevel((u32)( DWC_REPORTFLAG_ALL &
                              ~DWC_REPORTFLAG_QR2_REQ ));
    
    // DWCライブラリ初期化
#if defined( USE_AUTHSERVER_PRODUCTION )
    ret = DWC_InitForProduction( GAME_NAME, INITIAL_CODE, DTDB_GetAllocFunc(), DTDB_GetFreeFunc() );
#else
    ret = DWC_InitForDevelopment( GAME_NAME, INITIAL_CODE, DTDB_GetAllocFunc(), DTDB_GetFreeFunc() );
#endif
    
    OS_TPrintf( "DWC_InitFor*() result = %d\n", ret );

    if ( ret == DWC_INIT_RESULT_DESTROY_OTHER_SETTING )
    {
        OS_TPrintf( "Wi-Fi setting might be broken.\n" );
    }

    // ユーザデータ読み込み
    DTUD_Init();

    while (1)
    {
        drawConsole();

        ReadKeyData();

        update();

        dbs_DemoUpdate();
        OS_WaitIrq(TRUE, OS_IE_V_BLANK);
        Heap_Debug();
        dbs_DemoLoad();
    }

    OS_Terminate();
}
