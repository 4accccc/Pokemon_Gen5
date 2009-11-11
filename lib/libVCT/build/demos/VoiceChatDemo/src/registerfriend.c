#include <nitro.h>
#include <dwc.h>

#include "main.h"
#include "init.h"
#include "utility.h"
#include "wc.h"
#include "screen.h"
#include "dbs.h"
#include "mp.h"

#include "registerfriend.h"

extern DWCUserData stUserData; //  ユーザデータを格納する構造体  
extern GameSequence gameSeqList[GAME_MODE_NUM];
extern GameControl stGameCnt;  // ゲーム制御情報構造体 
extern KeyControl stKeyCnt;    // キー入力制御構造体  
extern u64  friend_key;

// 友達登録鍵 
static char stFriendKey[sizeof(DWCFriendData)];

// サンプルMP交換用友達データ 
static DWCFriendData stSampleFriendData ATTRIBUTE_ALIGN(32);

// データ通信用バッファ 
static u16* gSendBuf ATTRIBUTE_ALIGN(32);//送信用バッファ 

/*---------------------------------------------------------------------------*
  友達登録メイン関数
  *---------------------------------------------------------------------------*/
GameMode GameRegisterFriendMain(void)
{
    GameSequence *gameSeq = &gameSeqList[GAME_MODE_REG_FRIEND];
    GameMode returnSeq = GAME_MODE_NUM;
    RegFriendParam cntParam;
    int curIdx = 0;
    int friendIdx;
    int maxFigure;
    int validFlg;

    int i, j;

    cntParam.step   = 0;
    cntParam.figure = 0;

    MI_CpuFill8( stFriendKey, '0', sizeof(stFriendKey));
    
    while (1){
		{
			char friendKeyString[DWC_FRIENDKEY_STRING_BUFSIZE];
			int i, x;
			DWC_FriendKeyToString(friendKeyString, friend_key);
	        dbs_Print( 0, 0, "friendkey:xxxx-xxxx-xxxx");
	        x = 10;
	        for(i = 0; i < 12; i++) {
				dbs_Print(x, 0, "%c", friendKeyString[i]);
				x++;
				if(i == 3 || i == 7) {
			        dbs_Print(x, 0, "-");
					x++;
				}
			}
		}
        ReadKeyData();// キーデータ取得 

        // 次に進むべきモードがセットされていたら抜ける 
        if (returnSeq != GAME_MODE_NUM){
            break;
        }
        
        ////////// 以下キー操作処理 
        if (cntParam.step == 0){
            // コマンド選択中 
            if (stKeyCnt.trg & PAD_BUTTON_A){
	            // Aボタンでメニュー決定 
                returnSeq = gameSeq->menuList[curIdx].mode;
                stGameCnt.blocking = gameSeq->menuList[curIdx].blocking;

                if ((curIdx == 0) || (curIdx == 1)){
                    // 友達登録鍵入力による友達登録
                    cntParam.step   = 1;
                    cntParam.figure = 0;
                    cntParam.value  = 0;
                    
                    // メニューリスト再表示 
                    DispMenuMsgWithCursor(gameSeq, curIdx, &cntParam);
                    
                    if (curIdx == 0){
                        maxFigure = 12;  // 友達鍵は最高１２桁 
                    }
                    else {
                        maxFigure = 2;   // 友達リストインデックスは最高２桁 
                    }
                }
                else if ( curIdx == 2 )
                {
					// MP通信による友達情報交換
					OS_TPrintf( "MP\n" );

		            // 無線初期化 
                    DWC_CleanupInet();
                    
		            // MP通信用データ作成 
                    DWC_CreateExchangeToken(&stUserData, &stSampleFriendData);
                    gSendBuf = (u16*)(&stSampleFriendData);
                    
                    mp_match(gSendBuf, stGameCnt.friendList.keyList);//MP communications

                    friendIdx = GetAvailableFriendListIndex();// 同一のMPデータを省く 
                    for(i = 0; i < friendIdx; i++){
                        for(j = i+1; j < friendIdx; j++){
                            if(DWC_IsEqualFriendData( (const DWCFriendData*)&stGameCnt.friendList.keyList[i], (const DWCFriendData*)&stGameCnt.friendList.keyList[j])){
                                MI_CpuClear8(&stGameCnt.friendList.keyList[j], sizeof(DWCFriendData));
                            }
                        }
                    }
                    // メニューリスト再表示 
                    DispMenuMsgWithCursor(gameSeq, curIdx, &cntParam);
                }
            }
            else if (stKeyCnt.trg & PAD_KEY_UP){
	            // 十字キー上でカーソル移動 
                curIdx--;
                if (curIdx < 0) curIdx = gameSeq->numMenu-1;
	            // メニューリスト再表示 
                DispMenuMsgWithCursor(gameSeq, curIdx, &cntParam);
            }
            else if (stKeyCnt.trg & PAD_KEY_DOWN){
	            // 十字キー下でカーソル移動 
                curIdx++;
                if (curIdx >= gameSeq->numMenu) curIdx = 0;
	            // メニューリスト再表示 
                DispMenuMsgWithCursor(gameSeq, curIdx, &cntParam);
            }
        }
        else {
	        // 数値入力中 
            if (stKeyCnt.trg & PAD_BUTTON_A){
                // Aボタンで友達の登録・削除を行う 
                if (curIdx == 0){
                    // 友達登録 
                    if( DWC_CheckFriendKey( &stUserData, DWC_StringToFriendKey(stFriendKey)) )
                    {
//                        DWCFriendData* s_FriendList;
                        int friendIdx = -1;

                        // 友達登録鍵による友達登録 
                        friendIdx = GetAvailableFriendListIndex();
                        DWC_CreateFriendKeyToken(&stGameCnt.friendList.keyList[friendIdx], 
                                		DWC_StringToFriendKey(stFriendKey));
                        validFlg = 1;
                    }
                    else {
                        OS_TPrintf( "Invalid Friend Code!\n" );
                        validFlg = 0;  // プロファイルID=0は無効 
                    }
                }
                else {
                    // 友達削除
                    int friendIdx = (int)((stFriendKey[0]-48)*10+(stFriendKey[1]-48));
                    if ((friendIdx < GAME_MAX_FRIEND_LIST)&&
                        DWC_CanChangeFriendList()){
                        // 友達リストから友達を削除 
                        DWC_DeleteBuddyFriendData(&stGameCnt.friendList.keyList[friendIdx]);
                        validFlg = 1;
                    }
                    else {
                        validFlg = 0;  // 友達リスト範囲外 
                    }
                }

                if (validFlg){
                    // 有効な入力であればコマンド選択に戻る 
                    cntParam.step = 0;
                    // メニューリスト再表示 
                    DispMenuMsgWithCursor(gameSeq, curIdx, &cntParam);
                }
            }
            else if (stKeyCnt.trg & PAD_BUTTON_B){
                MI_CpuFill8( stFriendKey, '0', sizeof(stFriendKey));
                // Bボタンでコマンド選択に戻る 
                cntParam.step = 0;
                // メニューリスト再表示 
                DispMenuMsgWithCursor(gameSeq, curIdx, &cntParam);
            }
            else if (stKeyCnt.trg & PAD_KEY_UP){
                // 十字キー上下で選択中の数値の増減 
                stFriendKey[cntParam.figure]++;
                if(stFriendKey[cntParam.figure] > '9'){
                    stFriendKey[cntParam.figure] = '0';
                }
                // メニューリスト再表示 
                DispMenuMsgWithCursor(gameSeq, curIdx, &cntParam);
            }
            else if (stKeyCnt.trg & PAD_KEY_DOWN){
                // 十字キー上下で選択中の数値の増減 
                stFriendKey[cntParam.figure]--;
                if(stFriendKey[cntParam.figure] < '0'){
                    stFriendKey[cntParam.figure] = '9';
                }
                // メニューリスト再表示 
                DispMenuMsgWithCursor(gameSeq, curIdx, &cntParam);
            }
            else if (stKeyCnt.trg & PAD_KEY_LEFT){
                // 十字キー左右で選択桁数の移動 
                if (cntParam.figure > 0 && cntParam.figure <= maxFigure-1 ) cntParam.figure--;
                // メニューリスト再表示 
                DispMenuMsgWithCursor(gameSeq, curIdx, &cntParam);
            }
            else if (stKeyCnt.trg & PAD_KEY_RIGHT){
                // 十字キー左右で選択桁数の移動 
                if (cntParam.figure >= 0 && cntParam.figure < maxFigure-1 ) cntParam.figure++;
                // メニューリスト再表示 
                DispMenuMsgWithCursor(gameSeq, curIdx, &cntParam);
            }
        }
        ////////// キー操作処理ここまで 

        // Vブランク待ち処理
        GameWaitVBlankIntr();
    }

    return returnSeq;
}

/*---------------------------------------------------------------------------*
  友達登録・消去モードの追加表示用コールバック関数 
 *---------------------------------------------------------------------------*/
void RegFriendModeDispCallback(int curIdx, void* param)
{
    RegFriendParam cntParam;
    int i;

    if (!param){
        // 初回はparam = NULLで呼ばれる 
        cntParam.step = 0;
    }
    else {
        cntParam = *(RegFriendParam *)param;
    }

    OS_TPrintf("\n");

    if (cntParam.step == 0){
        // コマンド選択中は友達リストを表示 
        DispFriendList(TRUE);
    }
    else {
        // 数値入力中 
        if (curIdx == 0){
            // 友達登録。プロファイルID入力中 
            OS_TPrintf("input> ");
            for(i = 0; i < 12; i++){
                OS_TPrintf("%c", stFriendKey[i]);
                if (i==3 || i==7)
                    OS_TPrintf("-");
            }
            OS_TPrintf("\n");

            // 選択中の数字を示す為の下線を表示する 
            for (i = 0; i < cntParam.figure; i++){
                OS_TPrintf(" ");
            }
            if(cntParam.figure > 3)
            	OS_TPrintf(" ");
            	
            if(cntParam.figure > 7)
            	OS_TPrintf(" ");

            OS_TPrintf("       -\n");
        }
        else {
            // 友達削除。友達リストインデックス入力中 
            DispFriendList(TRUE);  // 友達リストを表示 
            
            OS_TPrintf("Delete Index : ");
            for(i = 0; i < 2; i++){
                OS_TPrintf("%c", stFriendKey[i]);
            }
            OS_TPrintf("\n");

            for (i = 0; i < cntParam.figure; i++){
                OS_TPrintf(" ");
            }
            OS_TPrintf("               -\n");
        }
    }
}

