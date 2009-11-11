#include <nitro.h>
#include <dwc.h>

#include "main.h"
#include "init.h"
#include "utility.h"
#include "dbs.h"

#include "gamemain.h"

//----------------------------------------------------------------------------
// define
//----------------------------------------------------------------------------
#define FILTER_STRING "sample_filter"	// フィルタ文字列. 適宜変更してください
#define FILTER_KEY "str_key"
#define APP_CONNECTION_KEEPALIVE_TIME 300000 // キープアライブ時間
#define KEEPALIVE_INTERVAL (APP_CONNECTION_KEEPALIVE_TIME/5) // キー入力を待たずデータを転送する時間

//----------------------------------------------------------------------------
// variable
//----------------------------------------------------------------------------
extern GameSequence gameSeqList[GAME_MODE_NUM];
extern GameControl stGameCnt;  // ゲーム制御情報構造体 
extern KeyControl stKeyCnt;    // キー入力制御構造体 

extern DWCUserData stUserData;  // 本体固有のユーザID(本来はユーザの入力であったりゲームごとに割り当てられるものであったりする値)
extern GameSaveData saveData;  
extern char addFilter[128];
extern u32  s_groupID;
u64 friend_key;

//----------------------------------------------------------------------------
// prototype
//----------------------------------------------------------------------------
static void LoginCallback(DWCError error, int profileID, void* param);
static void UpdateServersCallback(DWCError error, BOOL isChanged, void* param);
static void FriendStatusCallback(int index, u8 status, const char* statusString, void* param);
static void DeleteFriendListCallback(int deletedIndex, int srcIndex,void* param);
static void SaveToServerCallback(BOOL success, BOOL isPublic, void* param);
static void LoadFromServerCallback(BOOL success, int profileID, char* data, int len, void* param);
static void BuddyFriendCallback(int index, void* param);
/*---------------------------------------------------------------------------*
  オフライン時メイン関数
 *---------------------------------------------------------------------------*/
GameMode GameMain(void)
{
    GameSequence *gameSeq = &gameSeqList[GAME_MODE_MAIN];
    GameMode returnSeq = GAME_MODE_NUM;
    int curIdx = 0;

    while (1){
        dbs_Print( 0, 0, "s:%d", DWC_GetMatchState() );
        dbs_Print( 7, 0, "n:%d", DWC_GetNumConnectionHost() );
        dbs_Print( 0, 1, "w:%d", DWC_GetLinkLevel() );
        dbs_Print( 10,1, "p:%d", stGameCnt.userData.profileID );
	    dbs_Print( 30, 0, "%c",  LoopChar());
        dbs_Print( 20, 2, "g:%d", s_groupID );

        ReadKeyData();  // キーデータ取得 

        DWC_ProcessFriendsMatch();  // DWC通信処理更新 

        HandleDWCError(&returnSeq);  // エラー処理

        if (stGameCnt.blocking){
            // DWC処理中はキー操作を禁止する

            // Vブランク待ち処理
            GameWaitVBlankIntr();
            continue;
        }

        // 次に進むべきモードがセットされていたら認証完了
        if (returnSeq == GAME_MODE_LOGIN){
            // ログインが選択されていた場合
            if (stGameCnt.userData.isValidProfile){
               // 認証成功でプロファイルIDを取得できた場合はループを抜ける
                 break;
            }
            else {
                // 認証失敗の場合はモードを進めないようにする
                returnSeq = GAME_MODE_NUM;
            }
        }
        else if (returnSeq != GAME_MODE_NUM){
            // それ以外の場合は必ず抜ける
            break;
        }
            
        ////////// 以下キー操作処理
        if (stKeyCnt.trg & PAD_BUTTON_A){
           // Aボタンでメニュー決定
            returnSeq = gameSeq->menuList[curIdx].mode;
            stGameCnt.blocking = gameSeq->menuList[curIdx].blocking;

            switch (returnSeq){
            case GAME_MODE_LOGIN:  // ログイン 
                if (DWC_CheckHasProfile(&stUserData))
                {
                    OS_TPrintf("DWC_CheckHasProfile:TRUE\n");
                    if (DWC_CheckValidConsole(&stUserData))
                    {
                        OS_TPrintf("DWC_CheckValidConsole:TRUE\n");
                    }
                    else
                    {
                        OS_TPrintf("DWC_CheckValidConsole:FALSE\n");
                        // このままログイン処理に進めても認証エラーになるため、
                        // 通常はこのまま先に進めてはいけません。
                    }
                }
                else
                {
                    OS_TPrintf("DWC_CheckHasProfile:FALSE\n");
                }
                DWC_InitFriendsMatch(&stUserData, GAME_PRODUCTID,
                                     GAME_SECRET_KEY, 0, 0,
                                     stGameCnt.friendList.keyList, GAME_MAX_FRIEND_LIST);

                // 認証用関数を使ってログイン
                if (!DWC_LoginAsync(L"Name", NULL, LoginCallback, &returnSeq)){
                    // ログインを開始できなかった場合はブロッキング状態にしない。
                    // エラー発生中か二重ログインでなければFALSEにはならない。
                    OS_TPrintf("Can't call DWC_LoginAsync().\n");
                    returnSeq          = GAME_MODE_NUM;
                    stGameCnt.blocking = 0;
                }
                break;
            default:
                break;
            }
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
        else if ( stKeyCnt.trg & PAD_BUTTON_Y){
            Heap_Dump();
        }
        ////////// キー操作処理ここまで

        // Vブランク待ち処理
        GameWaitVBlankIntr();
    }

    return returnSeq;
}

/*---------------------------------------------------------------------------*
  ログインコールバック関数
 *---------------------------------------------------------------------------*/
static void LoginCallback(DWCError error, int profileID, void *param)
{
    BOOL result;
	RTCTime time;
	RTCDate date;
    
    if (error == DWC_ERROR_NONE){
        // フィルタ文字列を作成する
        char* filterString = FILTER_STRING;
		OS_SNPrintf(addFilter, sizeof(addFilter), "%s='%s'", FILTER_KEY, filterString);

        // 認証成功、プロファイルID取得
        OS_TPrintf("Login succeeded. profileID = %u\n\n", profileID);

        // ingamesnチェックの結果を取得する
		if(DWC_GetIngamesnCheckResult() == DWC_INGAMESN_INVALID)
			OS_TPrintf("BAD ingamesn is detected by NAS.\n");
		else if(DWC_GetIngamesnCheckResult() == DWC_INGAMESN_VALID)
			OS_TPrintf("GOOD ingamesn is detected by NAS.\n");
		else
			OS_TPrintf("ingamesn is not checked yet.\n");
		
		// 認証サーバの時刻を表示する
		if(DWC_GetDateTime(&date, &time) == TRUE)
			OS_TPrintf("NasTime = %02d/%02d/%02d %02d:%02d:%02d\n", date.year+2000, date.month, date.day, time.hour, time.minute, time.second);
		else
			DWC_Printf(DWC_REPORTFLAG_AUTH, "DWC_GetNasTime failed\n");


        // stUserDataが更新されているかどうかを確認。
        if ( DWC_CheckDirtyFlag( &stUserData ) )
        {
            DWC_ClearDirtyFlag( &stUserData );

            OS_TPrintf("*******************************\n");
            OS_TPrintf("You must save the DWCUserData!!\n");
            OS_TPrintf("*******************************\n");

            // 必ずこのタイミングでチェックして、dirty flagが有効になっていたら、
            // DWCUserDataをDSカードのバックアップに保存するようにしてください。
            // saveUserDataToDSCard( &stUserData );
        }
        
        stGameCnt.userData.profileID = profileID;
        stGameCnt.userData.isValidProfile = TRUE;


        // 友達登録鍵作成
        friend_key = DWC_CreateFriendKey( &stUserData );
		if( friend_key ){
            OS_TPrintf("friend_key = %lld\n", friend_key);
        }
        
        // 友達リスト同期処理開始
        result = DWC_UpdateServersAsync(NULL,
                                        UpdateServersCallback, param,
                                        FriendStatusCallback, param,
                                        DeleteFriendListCallback, param);
        if (result == FALSE){
           // 呼んでもいい状態（ログインが完了していない状態）で呼んだ時のみ
            // FALSEが返ってくるので、普通はTRUE
            OS_Panic("--- DWC_UpdateServersAsync error teminated.\n");
            *(GameMode *)param = GAME_MODE_NUM;
            stGameCnt.blocking  = 0;
        }
        else {
            // GameSpyサーバ上バディ成立コールバックを登録する
            DWC_SetBuddyFriendCallback(BuddyFriendCallback, NULL);

            // ストレージサーバセーブ・ロード完了通知コールバックを登録する
            DWC_SetStorageServerCallback(SaveToServerCallback,
                                         LoadFromServerCallback);
        }

		// キープアライブ時間の設定        
        DWC_SetConnectionKeepAliveTime(APP_CONNECTION_KEEPALIVE_TIME);

    }
    else {
        // 認証失敗
        OS_TPrintf("Login failed. %d\n\n", error);
        stGameCnt.blocking = FALSE;  // ブロッキング解除
    }
}

/*---------------------------------------------------------------------------*
  GameSpyバディ成立コールバック関数
 *---------------------------------------------------------------------------*/
static void BuddyFriendCallback(int index, void* param)
{
#pragma unused(param)

    OS_TPrintf("I was authorized by friend[%d].\n", index);
}


/*---------------------------------------------------------------------------*
  友達リスト同期処理完了コールバック関数
 *---------------------------------------------------------------------------*/
static void UpdateServersCallback(DWCError error, BOOL isChanged, void* param)
{
#pragma unused(param)

    if (error == DWC_ERROR_NONE){
        // 友達リスト同期処理完了
        OS_TPrintf("Updating servers succeeded.\n");

        if (isChanged){
            // 友達リストが変更されていたらセーブする
            // （セーブのつもり）
            MI_CpuCopy32(&stGameCnt.friendList, &saveData.friendList, sizeof(GameFriendList));
        }

        // 友達リストを表示する
        DispFriendList(TRUE);
    }
    else {
        // 失敗しても特に何もしない
        OS_TPrintf("Failed to update GameSpy servers. %d\n\n", error);
    }

    stGameCnt.blocking = FALSE;  // ブロッキング解除 
}

/*---------------------------------------------------------------------------*
   友達状態変化通知コールバック関数
 *---------------------------------------------------------------------------*/
static void FriendStatusCallback(int index, u8 status, const char* statusString, void* param)
{
#pragma unused(param)
#ifdef SDK_FINALROM
#pragma unused(index, status, statusString)
#endif

    OS_TPrintf("Friend[%d] changed status -> %d (statusString : %s).\n",
               index, status, statusString);
}


/*---------------------------------------------------------------------------*
  友達リスト削除コールバック関数
 *---------------------------------------------------------------------------*/
static void DeleteFriendListCallback(int deletedIndex, int srcIndex, void* param)
{
#pragma unused(param)
#ifdef SDK_FINALROM
#pragma unused(deletedIndex, srcIndex)
#endif
#if 0
     // 友達データの削除・前詰めに合わせて、それに対応するデータも前に詰める
    if (index < GAME_MAX_FRIEND_LIST-1){
        // 友達の通信状態を前に詰める
        MI_CpuCopy8(&stGameCnt.friendStatus[index+1], &stGameCnt.friendStatus[index],
                    (u32)(GAME_MAX_FRIEND_LIST-index-1));

        // 友達の通信状態を前に詰める
        MI_CpuCopy8(&stGameCnt.friendList.playerName[index+1],
                    &stGameCnt.friendList.playerName[index],
                    (u32)(GAME_MAX_PLAYER_NAME*(GAME_MAX_FRIEND_LIST-index-1)));
    }

    stGameCnt.friendStatus[GAME_MAX_FRIEND_LIST-1] = DWC_STATUS_OFFLINE;
    MI_CpuClear32(&stGameCnt.friendList.playerName[GAME_MAX_FRIEND_LIST-1],
                  GAME_MAX_PLAYER_NAME);

#else
    OS_TPrintf("friend[%d] was deleted (equal friend[%d]).\n",
               deletedIndex, srcIndex);
#endif
}


/*---------------------------------------------------------------------------*
  ストレージサーバへのデータセーブ完了コールバック関数
 *---------------------------------------------------------------------------*/
static void SaveToServerCallback(BOOL success, BOOL isPublic, void* param)
{
#pragma unused(param)

    OS_TPrintf("Saved data to server.\n");
    OS_TPrintf("result %d, isPublic %d.\n", success, isPublic);
}


/*---------------------------------------------------------------------------*
  ストレージサーバからのデータロード完了コールバック関数
 *---------------------------------------------------------------------------*/
static void LoadFromServerCallback(BOOL success, int profileID, char* data, int len, void* param)
{
#pragma unused(param)

    OS_TPrintf("Saved data to server.\n");
    OS_TPrintf("result %d, index %d, data '%s', len %d\n",
               success, profileID, data, len);
}


