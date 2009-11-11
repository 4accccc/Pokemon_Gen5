#include <nitro.h>
#include <nitro/os.h>
#include <dwc.h>

#include "main.h"
#include "init.h"
#include "utility.h"

#include "dbs.h"
#include "screen.h"
#include "sound.h"

#include "transport.h"
#include "demo_util.h"

extern GameSequence gameSeqList[GAME_MODE_NUM];
extern GameControl stGameCnt;  // ゲーム制御情報構造体 
extern KeyControl stKeyCnt;    // キー入力制御構造体 
extern u32  s_groupID;

/** -------------------------------------------------------------------------
  DWC Transport
  ---------------------------------------------------------------------------*/
const int		SIZE_SEND_BUFFER = 512;
const int		SIZE_RECV_BUFFER = 4 * 1024;

static u8		sSendBuffer[ SIZE_SEND_BUFFER ];
static u8		sRecvBuffer[ MAX_PLAYERS ][ SIZE_RECV_BUFFER ];

static int      stExchangeProfile = 0;

#define DEMO_EXCHANGE_MAGIC_TOKEN '_Vdm'

/** -------------------------------------------------------------------------
  DWC Transport用　受信バッファを設定  
  ---------------------------------------------------------------------------*/
void
SetRecvBuffer( void )
{
	int i;
    int j = 0;
	int connectingNum = DWC_GetNumConnectionHost()	;

    OS_TPrintf( "connection host = %d\n", connectingNum );
    
    for ( i = 0; i < connectingNum; ++i )
    {
		if ( i == DWC_GetMyAID() )
        {
            j++;
			continue;
        }

        DWC_SetRecvBuffer( (u8)i, &sRecvBuffer[i-j], SIZE_RECV_BUFFER );
    }
    // グループID も保存
    s_groupID = DWC_GetGroupID();
}

/** -------------------------------------------------------------------------
  送信完了コールバック  
  ---------------------------------------------------------------------------*/
static void
SendDoneCallback( int size, u8 aid, void* param )
{
#pragma unused(size, aid, param)
}

/*---------------------------------------------------------------------------*
  DWC 受信処理
  ---------------------------------------------------------------------------*/
static void 
UserRecvCallback( u8 aid, u8* buffer, int size, void* param )
{
#pragma unused(param)
    BOOL flag;
    
    flag = VCT_HandleData(aid, buffer, size);

    if (flag == FALSE) {
        //
        // 
        // VCT_HandleDataの戻り値がFALSEの場合、VoiceChat以外のデータ、または
        // VoiceChatが初期化されていない状態です。
        //
        // デモではクライアントの状態をやりとりしています
        // 
        //
        client_info *info = (client_info*)buffer;
        if (info->magic != DEMO_EXCHANGE_MAGIC_TOKEN) {
            return;
        }

        UpdateClientInfo(aid, info);
    }
}

/*---------------------------------------------------------------------------*
  接続完了後メイン関数
 *---------------------------------------------------------------------------*/
GameMode GameConnectedMain(void)
{
    GameSequence *gameSeq = &gameSeqList[GAME_MODE_CONNECTED];
    GameMode returnSeq = GAME_MODE_NUM;
    int curIdx = 0;

    // レポートレベルを下げる
    DWC_SetReportLevel(0);
    
    // 送信コールバックの設定	
    DWC_SetUserSendCallback( SendDoneCallback, NULL ); 

    // 受信コールバックの設定	
    DWC_SetUserRecvCallback( UserRecvCallback, NULL ); 
    
    // 受信バッファの設定	
    SetRecvBuffer();

    // コネクションクローズコールバックを設定
    DWC_SetConnectionClosedCallback(ConnectionClosedCallback, &returnSeq);

#if 0
	// パケットロス率を設定
	if ( DWC_GetMyAID() != 0 )
    {
		DWC_SetSendDrop( 30, 0 );
		DWC_SetRecvDrop( 30, 0 );        
    }
#endif
#if 0
    // 遅延を設定（単位: ms）
	if ( DWC_GetMyAID() != 0 )
    {
		DWC_SetSendDelay( 500, 0 );
		DWC_SetRecvDelay( 500, 0 );        
    }
#endif    

    (void)PM_SetBackLight( PM_LCD_ALL, PM_BACKLIGHT_ON );
    OS_TPrintf("AID bitmap = %x\n", DWC_GetAIDBitmap());
    OS_TPrintf( "my aid = %d\n", DWC_GetMyAID() );

    // プロフィール情報を交換する
    //
    BroadCastStatus(DEMO_STATUS_JOIN);
    
    DispMenuMsgWithCursor(gameSeq, curIdx, NULL);
    
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
            // DWC処理中はキー操作を禁止する

            // Vブランク待ち処理
            GameWaitVBlankIntr();
            continue;
	    }

        // 次に進むべきモードがセットされていたらループを抜ける
        if (DWC_GetNumConnectionHost() <= 1){
            DWC_CloseAllConnectionsHard();
            stGameCnt.blocking = 0;
            returnSeq = gameSeq->menuList[3].mode;
        }
        if (returnSeq != GAME_MODE_NUM) break;

        ////////// 以下キー操作処理
        if (stKeyCnt.trg & PAD_BUTTON_A){
            // Aボタンでメニュー決定
            returnSeq = gameSeq->menuList[curIdx].mode;
            stGameCnt.blocking = gameSeq->menuList[curIdx].blocking;

            switch (curIdx) {
            case 3:
                DWC_CloseAllConnectionsHard();
                
                if (DWC_GetNumConnectionHost() <= 1){
                    // 既に接続ホスト数が0ならすぐに終了し、コールバックは呼ばれない
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
        ////////// キー操作処理ここまで

        // Vブランク待ち処理
        GameWaitVBlankIntr();
    }

    return returnSeq;
}

