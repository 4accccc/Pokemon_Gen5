#include <nitro.h>
#include <dwc.h>
#include <string.h>

#include "main.h"
#include "init.h"
#include "utility.h"
#include "registerfriend.h"
#include "transport.h"
#include "dbs.h"

#include "logon.h"

//----------------------------------------------------------------------------
// define
//----------------------------------------------------------------------------
#define GAME_NUM_MATCH_KEYS 3      // マッチメイク用追加キー個数       // 
#define ConnectMaxVCT	8          //
#define FILTER_STRING "sample_filter"	// フィルタ文字列. 適宜変更してください
#define FILTER_KEY "str_key"
//#define GAME_USE_STORAGE_SERVER
//----------------------------------------------------------------------------
// variable
//----------------------------------------------------------------------------
extern GameSequence gameSeqList[GAME_MODE_NUM];
extern GameControl stGameCnt;  // ゲーム制御情報構造体  
extern KeyControl stKeyCnt;    // キー入力制御構造体  
u32  s_groupID = 0;
// 友達無指定ピアマッチメイク用追加キーデータ構造体 
static GameMatchExtKeys stMatchKeys[GAME_NUM_MATCH_KEYS] = { 0, };


// GameSpy Statusデータとしてセットするサンプルデータ
static const char gsStatusSample[16] = { 10, 9, 8, 0, 0, 1, 2, 3, };

static int stNumEntry  = 2;    // ネットワーク構成人数指定 
static int stServerIdx = 0;    // 接続したいゲームサーバの友達リストインデックス 

char addFilter[128] = "";
static u8   userData[4]={ 0,5,120,254 };
extern DWCTopologyType s_topologyType;

static void SetGameMatchKeys(void);

//callbackプロトタイプ 
static void ConnectToAnybodyCallback(DWCError error, 
                                     BOOL cancel, 
                                     BOOL self,
                                     BOOL isServer,
                                     int  index,
                                     void* param);

static void ConnectToFriendsCallback(DWCError error, 
                                     BOOL cancel, 
                                     BOOL self,
                                     BOOL isServer,
                                     int  index,
                                     void* param);

static void SetupGameServerCallback(DWCError error,
                                    BOOL cancel,
                                    BOOL self,
                                    BOOL isServer,
                                    int index,
                                    void* param);
static void ConnectToGameServerCallback(DWCError error,
                                        BOOL cancel,
                                        BOOL self,
                                        BOOL isServer,
                                        int index,
                                        void* param);
static void NewClientCallback(int index, void* param);
static int  EvaluateAnybodyCallback(int index, void* param);
static int  EvaluateFriendCallback(int index, void* param);
static void GenericNewClientCallback(int index, void* param);
static void LoginToStorageCallback(DWCError error, void* param);
/*---------------------------------------------------------------------------*
  ゲーム定義のマッチメイク用追加キーセット関数
 *---------------------------------------------------------------------------*/
static void SetGameMatchKeys(void)
{
    int ranking = (int)((OS_GetTick()*1592653589UL+453816691UL) & 0xff);

    // ランキングを整数キーとして設定
    stMatchKeys[0].isStr  = 0;
    strcpy(stMatchKeys[0].keyStr, "game_rank");
    stMatchKeys[0].ivalue = ranking;
    stMatchKeys[0].keyID  =
        DWC_AddMatchKeyInt(stMatchKeys[0].keyID,
                           stMatchKeys[0].keyStr,
                           &stMatchKeys[0].ivalue);
    if (stMatchKeys[0].keyID == 0) OS_TPrintf("AddMatchKey failed 0.\n");

    // マッチメイクに用いる評価タイプを文字列キーとして設定
    stMatchKeys[1].isStr  = 1;
    strcpy(stMatchKeys[1].keyStr, FILTER_KEY);
    strcpy(stMatchKeys[1].svalue, FILTER_STRING);
    stMatchKeys[1].keyID  =
        DWC_AddMatchKeyString(stMatchKeys[1].keyID,
                              stMatchKeys[1].keyStr,
                              stMatchKeys[1].svalue);
    if (stMatchKeys[1].keyID == 0) OS_TPrintf("AddMatchKey failed 1.\n");

    // とりあえずもう一つぐらい整数キーを設定
    stMatchKeys[2].isStr  = 0;
    strcpy(stMatchKeys[2].keyStr, "game_manner");
    stMatchKeys[2].ivalue = ranking%11;
    stMatchKeys[2].keyID  =
        DWC_AddMatchKeyInt(stMatchKeys[2].keyID,
                           stMatchKeys[2].keyStr,
                           &stMatchKeys[2].ivalue);
    if (stMatchKeys[2].keyID == 0) OS_TPrintf("AddMatchKey failed 2.\n");
}


/*---------------------------------------------------------------------------*
  ログイン後モードの追加表示用コールバック関数
 *---------------------------------------------------------------------------*/
void LogonModeDispCallback(int curIdx, void* param)
{
#pragma unused(param)

    // 自分以外のエントリー人数へのポインタを表示する
    OS_TPrintf("\nnumber of entry = ");
    if (stNumEntry == 2){
        OS_TPrintf("   %d >>\n", stNumEntry);
    }
    else if (stNumEntry == DWC_MAX_CONNECTIONS){
        OS_TPrintf("<< %d\n", stNumEntry);
    }
    else {
        OS_TPrintf("<< %d >>\n", stNumEntry);
    }

    if (curIdx == 3){
        // ゲームサーバへの接続時は、友達リストインデックスも表示する
        OS_TPrintf("GameServerIndex = ");
        if (DWC_GetNumFriend(stGameCnt.friendList.keyList, GAME_MAX_FRIEND_LIST) == 1){
            OS_TPrintf("   %d\n", stServerIdx);
        }
        else if (stServerIdx == 0){
            OS_TPrintf("   %d >>\n", stServerIdx);
        }
        else if (stServerIdx == GAME_MAX_FRIEND_LIST-1){
            OS_TPrintf("<< %d\n", stServerIdx);
        }
        else {
            OS_TPrintf("<< %d >>\n", stServerIdx);
        }
    }
}

/*---------------------------------------------------------------------------*
  サーバクライアントマッチメイク時の新規接続クライアント通知コールバック関数
 *---------------------------------------------------------------------------*/
static void NewClientCallback(int index, void* param)
{
#pragma unused(param)

    // 新規接続クライアントの接続処理が終わるまでは、
    // コントローラ操作を禁止する
    stGameCnt.blocking = TRUE;
    
    OS_TPrintf("New client started to connect.\n");
    if (index != -1){
        OS_TPrintf("He is my friend [%d].\n", index);
    }
    else {
        OS_TPrintf("He is not my friend.\n");
    }
}


/*---------------------------------------------------------------------------*
  友達無指定ピアマッチメイク時プレイヤー評価コールバック関数
 *---------------------------------------------------------------------------*/
static int  EvaluateAnybodyCallback(int index, void* param)
{
#pragma unused(param)
    const char* matchType = DWC_GetMatchStringValue(index, stMatchKeys[1].keyStr, "NONE");
    int rank;
    int diff;
    
    if (!strncmp(matchType, "eval_rank", strlen("eval_rank"))){
        // 相手もマッチメイクタイプとしてFILTER_STRINGをセットしている場合
        rank = DWC_GetMatchIntValue(index, stMatchKeys[0].keyStr, -1);
        if (rank == -1){
            return 0;  // "game_rank"キーがなければマッチメイクしない 
        }

        diff = (stMatchKeys[0].ivalue > rank) ? stMatchKeys[0].ivalue-rank : rank-stMatchKeys[0].ivalue;

        OS_TPrintf("Player[%d]'s rank is %d (mine %d). %d point.\n",
                   index, rank, stMatchKeys[0].ivalue, 200-diff);

#if 0   // 評価のテストを行いたい場合はここを1にする
        // 自分との順位差が近いほど高得点になるように、200点満点で点を付ける。
        // 200位差以上ならマッチメイクしない（0以下の値を返す）
        return 200-diff;

#else   // 普段はマッチメイクしないことがあると面倒なのでこちらを通す 
		return 300-diff;
#endif
    }
    else if (!strncmp(matchType, "NONE", strlen("NONE"))){
        return 1;	// "game_mtype"キーがなければとりあえず平等にマッチメイクする 

    }

    return 1;
}


/*---------------------------------------------------------------------------*
  友達指定ピアマッチメイク時プレイヤー評価コールバック関数
 *---------------------------------------------------------------------------*/
static int  EvaluateFriendCallback(int index, void* param)
{
#pragma unused(param)
    const char* matchType = DWC_GetMatchStringValue(index, stMatchKeys[1].keyStr, "NONE");
    
    if (!strncmp(matchType, FILTER_STRING, strlen(FILTER_STRING))){
        // 相手もマッチメイクタイプとしてFILTER_STRINGをセットしている場合
        return 1;  // マッチメイクOK 
    }
    else {
        return 0;  // "game_mtype"キーがなければマッチメイクしない 
    }
}

/*---------------------------------------------------------------------------*
  ログイン後メイン関数
 *---------------------------------------------------------------------------*/
GameMode GameLogonMain(void)
{
    GameSequence *gameSeq = &gameSeqList[GAME_MODE_LOGIN];
    GameMode returnSeq = GAME_MODE_NUM;
    int curIdx = 0;
    BOOL boolRet;

    stNumEntry  = 2;
    stServerIdx = 0;

    // デバッグ表示レベル指定
    DWC_SetReportLevel(DWC_REPORTFLAG_ALL);

#if 0
// サンプルの友達登録鍵表示テスト
{
    char tmpstr[DWC_FRIENDKEY_STRING_BUFSIZE];
    
    DWC_FriendKeyToString(tmpstr, DWC_CreateFriendKey(&stUserData));
    OS_TPrintf("friendKeyToken '%s'\n", tmpstr);
}
#endif

#ifdef GAME_USE_STORAGE_SERVER
// GameSpyストレージサーバへのセーブ・ロードを行なうテストのためのログイン処理
{
    if (!DWC_LoginToStorageServerAsync(LoginToStorageCallback, NULL)){
        OS_TPrintf("DWC_LoginToStorageServerAsync() failed.\n");
    }
}
#endif

    while (1){
        dbs_Print( 0, 0, "s:%d", DWC_GetMatchState() );
        dbs_Print( 7, 0, "n:%d", DWC_GetNumConnectionHost() );
        dbs_Print( 0, 1, "w:%d", DWC_GetLinkLevel() );
        dbs_Print( 10,1, "p:%d", stGameCnt.userData.profileID );
	    dbs_Print( 30, 0, "%c",  LoopChar());
        dbs_Print( 0,2, "AID:%d(0x%08X)", DWC_GetMyAID(), DWC_GetAIDBitmap() );
        dbs_Print( 20, 2, "g:%d", s_groupID );

        ReadKeyData();  // キーデータ取得 

        DWC_ProcessFriendsMatch();  // DWC通信処理更新 

        HandleDWCError(&returnSeq);  // エラー処理

        if (stGameCnt.blocking){
            // DWC処理中はキー操作を禁止する（キャンセルのみ可）
            if (stKeyCnt.trg & PAD_BUTTON_B){
                // Bボタンでマッチメイクをキャンセルする
                DWC_CloseAllConnectionsHard();
                stGameCnt.blocking = FALSE;
                returnSeq = GAME_MODE_LOGIN;
            }

            // Vブランク待ち処理
            GameWaitVBlankIntr();
            continue;
        }

        // 次に進むべきモードがセットされていたらループを抜ける
        if (returnSeq != GAME_MODE_NUM) break;

        ////////// 以下キー操作処理
        if (stKeyCnt.trg & PAD_BUTTON_A){
            // Aボタンでメニュー決定
            returnSeq = gameSeq->menuList[curIdx].mode;
            stGameCnt.blocking = gameSeq->menuList[curIdx].blocking;
            boolRet   = TRUE;

            switch (curIdx){
            case 0:  // 友達無指定ピアマッチメイク呼び出し 
                if (!(stKeyCnt.cont & PAD_BUTTON_X)){
                    // Xボタンが押されていなければ、マッチメイク用追加キーデータ設定
                    SetGameMatchKeys();
                }
                boolRet = 
                DWC_ConnectToAnybodyAsync(s_topologyType,
                                          (u8)stNumEntry,
                                          addFilter,
                                          ConnectToAnybodyCallback,
                                          &returnSeq,
                                          GenericNewClientCallback,
                                          NULL,
                                          EvaluateAnybodyCallback,
                                          NULL,
                                          NULL,
                                          userData,
                                          NULL);
                break;
            case 1:  // 友達指定ピアマッチメイク呼び出し 
                if (!(stKeyCnt.cont & PAD_BUTTON_X)){
                    // Xボタンが押されていなければ、マッチメイク用追加キーデータ設定
                    SetGameMatchKeys();
                }
                // 引数friendIdxListにNULLを指定すると友達リスト内の全員から検索する
                boolRet =
                DWC_ConnectToFriendsAsync(s_topologyType,
                                          NULL,
                                          ConnectMaxVCT,
                                          (u8)stNumEntry,
                                          ConnectToFriendsCallback,
                                          &returnSeq,
                                          GenericNewClientCallback,
                                          NULL,
                                          EvaluateFriendCallback,
                                          NULL,
                                          NULL,
                                          userData,
                                          NULL);
                break;
            case 2:  // サーバDSとしてサーバクライアントマッチメイク開始 
                boolRet =
                DWC_SetupGameServer(s_topologyType,
                                    (u8)stNumEntry,
                                    SetupGameServerCallback,
                                    &returnSeq,
                                    NewClientCallback,
                                    NULL,
                                    NULL,
                                    userData,
                                    NULL);
                break;
            case 3:  // クライアントDSとしてサーバクライアントマッチメイク開始 
                boolRet =
                DWC_ConnectToGameServerAsync(s_topologyType,
                                             stServerIdx,
                                             ConnectToGameServerCallback,
                                             &returnSeq,
                                             NewClientCallback,
                                             NULL,
                                             NULL,
                                             userData,
                                             NULL);
                break;
            case 4:
                if(s_groupID)
                {
                     boolRet =
                     DWC_ConnectToGameServerByGroupID( s_topologyType,  // 接続形態
                                                       s_groupID,		// グループID
                                                       ConnectToGameServerCallback,
                                                       &returnSeq,
                                                       NewClientCallback,
                                                       NULL,
                                                       NULL,
                                                       userData,
                                                       NULL );
                }
                else
                {
                    boolRet = FALSE;
                }
                break;
            case 5:
                break;
            case 6:  // ログアウト 
                ShutdownInet();  // ネットワーク切断 
                break;
            default:
                break;
            }
            if (!boolRet){
                // マッチメイクを開始できなかった場合はブロッキング状態にしない。
                // エラー発生中かログイン前、接続後、二重呼び出し時しか
                // FALSEにはならない
                OS_TPrintf("Can't call matching function.\n");
                returnSeq          = GAME_MODE_NUM;
                stGameCnt.blocking = 0;
            }
        }
        else if (stKeyCnt.trg & PAD_BUTTON_Y){
            if (!(stKeyCnt.cont & PAD_BUTTON_X)){
                // Xボタンが押されていなければ、Yボタンで現在の友達の状態を取得表示（statusString）
                DispFriendList(TRUE);
                OS_TPrintf("\n");
            }
            else {
                // Xボタンが押されていれば、Yボタンで現在の友達の状態を取得表示（statusData）
                DispFriendListData();
                OS_TPrintf("\n");
            }
        }
        else if (stKeyCnt.trg & PAD_BUTTON_SELECT){
#ifndef GAME_USE_STORAGE_SERVER  // GameSpyのStatusセットを試す場合
            if (!(stKeyCnt.cont & PAD_BUTTON_X)){
                // Xボタンが押されていなければ、セレクトボタンでGameSpy Status文字列をセット
                if (DWC_SetOwnStatusString("SampleString-00")){
                    OS_TPrintf("Set GsStatusString.\n");
                }
                else {
                    OS_TPrintf("Failed to set GsStatusString.\n");
                }
            }
            else {
                // Xボタンが押されていれば、セレクトボタンでGameSpy Statusデータをセット
                if (DWC_SetOwnStatusData(gsStatusSample, sizeof(gsStatusSample))){
                    OS_TPrintf("Set GsStatusData.\n");
                }
                else {
                    OS_TPrintf("Failed to set GsStatusData.\n");
                }
            }

#else  // ストレージサーバへのセーブを試す場合
            // セレクトボタンでストレージサーバにテストキーをセーブ
            char keyvalues[30];
            BOOL boolResult;

            if (stKeyCnt.cont & PAD_BUTTON_X){
                // Xボタンが押されていればPrivateデータをセーブ
                snprintf(keyvalues, sizeof(keyvalues), "\\test_key_prv\\%lld", OS_TicksToSeconds(OS_GetTick()));
                boolResult = DWC_SavePrivateDataAsync(keyvalues, NULL);
            }
            else {
                // Xボタンが押されていなければPublicデータをセーブ
                snprintf(keyvalues, sizeof(keyvalues), "\\test_key_pub\\%lld", OS_TicksToSeconds(OS_GetTick()));
                boolResult = DWC_SavePublicDataAsync(keyvalues, NULL);
            }

            if (boolResult){
                OS_TPrintf("Saved '%s'.\n", keyvalues);
            }
            else {
                OS_TPrintf("Can't save to storage server.\n");
            }
#endif
        }
        else if (stKeyCnt.trg & PAD_BUTTON_START){
#ifndef GAME_USE_STORAGE_SERVER  // GameSpyのStatusセットを試す場合
            char statusData[DWC_FRIEND_STATUS_STRING_LEN];
            
            if (!(stKeyCnt.cont & PAD_BUTTON_X)){
                // Xボタンが押されていなければ、
                // スタートボタンでGameSpy Status文字列を取得
                if (DWC_GetOwnStatusString(statusData)){
                    OS_TPrintf("Got my GsStatusString : %s\n", statusData);
                }
                else {
                    OS_TPrintf("Failed to get GsStatusString.\n");
                }
            }
            else {
                // Xボタンが押されていれば、
                // スタートボタンでGameSpy Statusデータを取得
                int size, i;

                size = DWC_GetOwnStatusData(statusData);
                if (size != -1){
                    OS_TPrintf("Got my GsStatusData\n");
                    for (i = 0; i < size; i++){
                        OS_TPrintf("%02d, ", statusData[i]);
                    }
                    OS_TPrintf("\n");
                }
                else {
                    OS_TPrintf("Failed to get GsStatusData.\n");
                }
            }

#else  // ストレージサーバからのロードを試す場合 
            // スタートボタンでストレージサーバからテストデータをロード 
            BOOL boolResult;

#if 1
            if (stKeyCnt.cont & PAD_BUTTON_X){
                // Xボタンが押されていれば自分のPrivateデータをロード
                boolResult = DWC_LoadOwnPrivateDataAsync("\\test_key_prv\\test_key_pub", NULL);
            }
            else {
                // Xボタンが押されていなければ自分のPublicデータをロード
                boolResult = DWC_LoadOwnPublicDataAsync("\\test_key_prv\\test_key_pub", NULL);
            }
#else
            // 友達リスト中のホストのデータをロード
            boolResult = DWC_LoadOthersDataAsync("\\test_key", stServerIdx, NULL);
#endif
            if (!boolResult) OS_TPrintf("Can't load from storage server.\n");
#endif
        }
        else if (stKeyCnt.trg & PAD_KEY_UP){
            // 十字キー上でカーソル移動 
            curIdx--;
            if (curIdx < 0) curIdx = gameSeq->numMenu-1;
            // メニューリスト再表示 
            DispMenuMsgWithCursor(gameSeq, curIdx, NULL);
        }
        else if (stKeyCnt.trg & PAD_KEY_DOWN){
            // 十字キー下でカーソル移動 
            curIdx++;
            if (curIdx >= gameSeq->numMenu) curIdx = 0;
    	        // メニューリスト再表示 
            DispMenuMsgWithCursor(gameSeq, curIdx, NULL);
        }
        else if (stKeyCnt.trg & PAD_KEY_LEFT){
            // 十字キー左でエントリー人数減少 
            if (stNumEntry > 2){
                stNumEntry--;
	            // メニューリスト再表示 
                DispMenuMsgWithCursor(gameSeq, curIdx, NULL);
            }
        }
        else if (stKeyCnt.trg & PAD_KEY_RIGHT){
            // 十字キー右でエントリー人数増加 
            if (stNumEntry < DWC_MAX_CONNECTIONS){
                stNumEntry++;
	            // メニューリスト再表示 
                DispMenuMsgWithCursor(gameSeq, curIdx, NULL);
            }
        }
        else if (stKeyCnt.trg & PAD_BUTTON_L){
            // LRボタンでサーバクライアントマッチメイクのサーバDSの友達リストインデックスを選択
            if (curIdx == 3){
                if (stServerIdx > 0){
                    stServerIdx--;
		            // メニューリスト再表示 
                    DispMenuMsgWithCursor(gameSeq, curIdx, NULL);
                }
            }
        }
        else if (stKeyCnt.trg & PAD_BUTTON_R){
            // LRボタンでサーバクライアントマッチメイクのサーバDSの友達リストインデックスを選択
            if (curIdx == 3){
                if (stServerIdx < GAME_MAX_FRIEND_LIST-1){
                    stServerIdx++;
		            // メニューリスト再表示 
                    DispMenuMsgWithCursor(gameSeq, curIdx, NULL);
                }
            }
        }
        ////////// キー操作処理ここまで

        // Vブランク待ち処理
        GameWaitVBlankIntr();
    }

    return returnSeq;
}


/*---------------------------------------------------------------------------*
  友達無指定接続コールバック関数
 *---------------------------------------------------------------------------*/
static void ConnectToAnybodyCallback(DWCError error, BOOL cancel, BOOL self, BOOL isServer,
                                     int index, void* param)
{
    (void)self;
    (void)isServer;
    (void)index;
    (void)param;
    
    if (error == DWC_ERROR_NONE){
        if (!cancel){
            // 見知らぬ人たちとのコネクション設立完了
            OS_TPrintf("Succeeded to connect to anybody\n\n");

            // 受信バッファセット
            SetRecvBuffer();
        }
        else {
            OS_TPrintf("Canceled matching.\n");
        }
    }
    else {
        OS_TPrintf("Failed to connect to anybody. %d\n\n", error);
    }

    stGameCnt.blocking = FALSE;  // ブロッキング解除 
}


/*---------------------------------------------------------------------------*
  友達指定接続コールバック関数
 *---------------------------------------------------------------------------*/
static void ConnectToFriendsCallback(DWCError error, BOOL cancel, BOOL self, BOOL isServer,
                                     int index, void* param)
{
    (void)self;
    (void)isServer;
    (void)index;
    (void)param;

    if (error == DWC_ERROR_NONE){
        if (!cancel){
            // 友達とのコネクション設立完了
            OS_TPrintf("Succeeded to connect to friends\n\n");

            // 受信バッファセット
            SetRecvBuffer();
        }
        else {
            OS_TPrintf("Canceled matching.\n");
        }
    }
    else {
        OS_TPrintf("Failed to connect to friends. %d\n\n", error);
    }

    stGameCnt.blocking = FALSE;  // ブロッキング解除 
}


/*---------------------------------------------------------------------------*
  サーバクライアントマッチメイク・サーバ起動コールバック関数
 *---------------------------------------------------------------------------*/
static void SetupGameServerCallback(DWCError error,
                                    BOOL cancel,
                                    BOOL self,
                                    BOOL isServer,
                                    int  index,
                                    void* param)
{
#ifdef SDK_FINALROM
#pragma unused(index)
#endif
#pragma unused(isServer)

    if (error == DWC_ERROR_NONE){
        if (!cancel){
            // ネットワークに新規クライアントが加わった
            OS_TPrintf("A friend of mine joined the game.\n");
            OS_TPrintf("friendListIndex = %d.\n\n", index);

            // 受信バッファセット
            SetRecvBuffer();
        }
        else {
            if (self){
                // 自分がマッチメイクをキャンセルした
                OS_TPrintf("Canceled matching.\n\n");
                // ログイン後状態に戻る
                *(GameMode *)param = GAME_MODE_LOGIN;
            }
            else {
                // クライアントDSがマッチメイクをキャンセルした
                OS_TPrintf("Client (friendListIndex = %d) canceled matching.\n\n", index);

                if (DWC_GetNumConnectionHost() == 1){
                    // まだ誰とも接続していなければ、次の状態に進まない
                    return;
                }
            }
        }
    }
    else {
        OS_TPrintf("Game server error occured. %d\n\n", error);
    }

    stGameCnt.blocking = FALSE;  // ブロッキング解除 
}


/*---------------------------------------------------------------------------*
  サーバクライアントマッチメイク・サーバ接続コールバック関数
 *---------------------------------------------------------------------------*/
static void ConnectToGameServerCallback(DWCError error,
                                        BOOL cancel,
                                        BOOL self,
                                        BOOL isServer,
                                        int  index,
                                        void* param)
{

    if (error == DWC_ERROR_NONE){
        if (!cancel){
            if (self){
                // 自分がサーバDSとそこにできているネットワークへの接続に成功した場合
                OS_TPrintf("Succeeded to join the game.\n\n");
            }
            else {
                // ネットワークに新規クライアントが加わった場合
                OS_TPrintf("New client joined the game.\n");
                if (index != -1){
                    OS_TPrintf("he is my friend [%d].\n\n", index);
                }
                else {
                    OS_TPrintf("he is not my friend.\n\n");
                }
            }

            // 受信バッファセット
            SetRecvBuffer();
        }
        else {
            if (self){
                // 自分がマッチメイクをキャンセルした
                OS_TPrintf("Canceled matching.\n\n");
                // ログイン後状態に戻る
                *(GameMode *)param = GAME_MODE_LOGIN;
            }
            else {
                if (isServer){
                    // サーバDSがマッチメイクをキャンセルした 
                    OS_TPrintf("GameServer canceled matching.\n\n");
                    // マッチメイクを終了してログイン後状態に戻る 
                    *(GameMode *)param = GAME_MODE_LOGIN;
                }
                else {
                    // 他のクライアントDSがマッチメイクをキャンセルした 
                    OS_TPrintf("Host (friendListIndex = %d) canceled matching.\n\n", index);
                }
            }
        }
    }
    else {
        OS_TPrintf("Failed to join the game. %d\n\n", error);
    }

    stGameCnt.blocking = FALSE;  // ブロッキング解除 
}

/*---------------------------------------------------------------------------*
  新規接続クライアント通知コールバック関数
 *---------------------------------------------------------------------------*/
static void GenericNewClientCallback(int index, void* param )
{
    (void)param;

    OS_TPrintf( "New Client is connecting(idx %d)...\n", index);
}

/*---------------------------------------------------------------------------*
  ストレージサーバへのログイン完了コールバック関数
 *---------------------------------------------------------------------------*/
static void LoginToStorageCallback(DWCError error, void* param)
{
#pragma unused(param)

    OS_TPrintf("Login to storage server: result %d\n", error);
}
