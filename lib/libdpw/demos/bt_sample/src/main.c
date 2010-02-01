/*---------------------------------------------------------------------------*
  Project:  DP WiFi Library
  File:     main.c

  Copyright 2003-2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*! @file
	@brief	DPW BT サンプル
	
	@author	kitayama(kitayama_shigetoshi@nintendo.co.jp)
	
	@version beta10	(2006/5/23)
		@li Dpw_Bt_Player::friend_key, Dpw_Bt_Leader::friend_keyの削除に対応しました。
	@version beta9	(2006/5/22)
		@li Dpw_Bt_Player::friend_key, Dpw_Bt_Leader::friend_keyの追加に対応しました。
	@version beta4	(2006/4/17)
		@li DPWライブラリを使用する前に、WiFiコネクションへのログインを行い、すぐ切断するようにしました。
		@li 時刻を取得する関数 DWC_GetDateTime() の使用例を追加しました。
	@version beta3	(2006/4/10)
		@li Dpw_Bt_Player, Dpw_Bt_Leader, Dpw_Bt_Room の変更に対応しました。
	@version beta1	(2006/3/17)
		@li 初版をリリースしました。
*/

#include <nitro.h>
#include <dwc.h>

#include <wchar.h>

#include "dbs.h"
#include "init.h"
#include "userdata.h"
#include "dpw_bt.h"

/*-----------------------------------------------------------------------*
					型・定数宣言
 *-----------------------------------------------------------------------*/

#define GAME_PRODUCTID   0			// このサンプルが使用するプロダクトID
#define GAME_NAME        "gmtest"	// このサンプルが使用するゲーム名
#define GAME_SECRET_KEY  "HA6zkS"	// このサンプルが使用するシークレットキー

#define GAME_FRAME		2		// 想定するゲームフレーム（1/60を1とする）

#define BT_ROOM_RANK	1		// アクセスするランク
#define MAIL_ADDR       "wfc-mail@nintendo.com" // メール送信先

typedef enum
{
	SAMPLE_START,
	SAMPLE_CONNECT_INET,
	SAMPLE_LOGIN,
	SAMPLE_LOGIN_WAIT,
	SAMPLE_BT_INIT,
    SAMPLE_BT_SETPROFILE,
    SAMPLE_BT_SETPROFILE_WAIT,
	SAMPLE_BT_GET_SERVER_STATE,
	SAMPLE_BT_GET_SERVER_STATE_WAIT,
	//SAMPLE_BT_INIT_SERVER,
	SAMPLE_BT_GET_ROOM_NUM,
	SAMPLE_BT_GET_ROOM_NUM_WAIT,
	SAMPLE_BT_UPLOAD_PLAYER_FOR_CANCEL,
	SAMPLE_BT_CANCEL,
	SAMPLE_BT_CANCEL_WAIT,
	SAMPLE_BT_UPLOAD_PLAYER,
	SAMPLE_BT_UPLOAD_PLAYER_WAIT,
	//SAMPLE_BT_UPDATE_SERVER,
	SAMPLE_BT_DOWNLOAD_ROOM,
	SAMPLE_BT_DOWNLOAD_ROOM_WAIT,
	SAMPLE_BT_END,
	SAMPLE_ERROR_OCCURED,
	SAMPLE_END
} DpwBtSampleState;

/*-----------------------------------------------------------------------*
					関数プロトタイプ宣言
 *-----------------------------------------------------------------------*/

//--- Auto Function Prototype --- Don't comment here.
void NitroMain();
static void GameWaitVBlankIntr(void);
static void VBlankIntr(void);
static void set_upload_player(void);
static void fill_random_data(void *addr, u32 size);
static void login_callback(DWCError error, int profileID, void* param);
//--- End of Auto Function Prototype

/*-----------------------------------------------------------------------*
					グローバル変数定義
 *-----------------------------------------------------------------------*/

static u8 s_Work[DWC_INIT_WORK_SIZE] ATTRIBUTE_ALIGN(32);
static DWCFriendsMatchControl stFMCtrl;
static DWCInetControl stConnCtrl;

// キー情報
static u16 Trg;
static u16 Cont;

static BOOL is_logined;						// ログインしているか
static int pid;								// プロファイルID

static MATHRandContext16 rand_cont16;		// 乱数の状態

static DpwBtSampleState app_state;			// アプリケーションのステート
static s32 room_num;						// BT_ROOM_RANKの部屋の数
static Dpw_Bt_Player upload_player;			// アップロードするプレイヤーデータ
static Dpw_Bt_Room download_room;			// ダウンロードするルームのデータ
static Dpw_Common_Profile profile;          // 登録する個人情報
static Dpw_Common_ProfileResult profile_result; // 個人情報登録APIの結果

static DPW_SERVER_TYPE server_type = DPW_SERVER_DEBUG; // アクセスするサーバ

/*-----------------------------------------------------------------------*
					グローバル関数定義
 *-----------------------------------------------------------------------*/

void NitroMain()
{
	// 認証サーバから送られてきたトークン
	u8 token[TOKEN_LEN] = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcde";
	
	//================ 初期化
	//---- OS 初期化
	OS_Init();
	OS_InitTick();
	OS_InitAlarm();
	RTC_Init();
	Heap_Init();
	MATH_InitRand16(&rand_cont16, 0x12345678);

	// スタック溢れチェック初期設定
	OS_SetThreadStackWarningOffset(OS_GetCurrentThread(), 0x100);

	//---- GX 初期化
	GX_Init();
	GX_DispOff();
	GXS_DispOff();

	//================ 設定
	//---- 電源全てオン
	GX_SetPower(GX_POWER_ALL);

	Cont = PAD_Read();

	//---- VRAM クリア
	GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
	MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
	(void)GX_DisableBankForLCDC();

	//---- OAMとパレットクリア
	MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
	MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);

	// 出力初期化
	dbs_DemoInit();
	DTDB_SetPrintOverride(TRUE);

	//----  Vブランク割込許可
	OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
	(void)OS_EnableIrqMask(OS_IE_V_BLANK);
	(void)OS_EnableIrq();
	(void)OS_EnableInterrupts();
	(void)GX_VBlankIntr(TRUE);
	GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);

	// DWCの初期化
	DWC_SetReportLevel(DWC_REPORTFLAG_ALL);
#if defined(_NITRO) && SDK_VERSION_MAJOR > 4
    DWC_InitForDevelopment("syachi2ds", 'IRAJ', DTDB_GetAllocFunc(), DTDB_GetFreeFunc());
#else
    DWC_Init(s_Work);
    DWC_SetMemFunc(DTDB_GetAllocFunc(), DTDB_GetFreeFunc());
#endif

	// アプリケーションの初期化
	app_state = SAMPLE_START;
	
	// アップロードするプレイヤーの仮データを設定する。
	set_upload_player();

	//---- 表示開始
	OS_WaitIrq(TRUE, OS_IE_V_BLANK);
	GX_DispOn();
	GXS_DispOn();

	while (1)
	{
		u16 keyData;

		{	// カウントアップ
			static u8 count;
			dbs_Print(0, 0, "DPW BT SAMPLE " __DATE__ "     %02x", count++);
		}

		GameWaitVBlankIntr();

		//---- パッドデータ読み込み
		keyData = PAD_Read();
		Trg = (u16) (keyData & (keyData ^ Cont));
		Cont = keyData;

		// Dpw_Bt_Main() だけは例外的にいつでも呼べる
		Dpw_Bt_Main();

		switch (app_state)
		{
		case SAMPLE_START:
			DWC_InitInet(&stConnCtrl);
			DWC_ConnectInetAsync();

			app_state = SAMPLE_CONNECT_INET;
			break;
		case SAMPLE_CONNECT_INET:
			DWC_ProcessInet();

			if (DWC_CheckInet())
			{
				switch (DWC_GetInetStatus())
				{
				case DWC_CONNECTINET_STATE_ERROR:
					{
						// エラー表示
						DWCError err;
						int errcode;
						err = DWC_GetLastError(&errcode);

						OS_TPrintf("   Error occurred %d %d.\n", err, errcode);
					}
					DWC_ClearError();
					DWC_CleanupInet();
					app_state = SAMPLE_START;	// 接続し直す
					break;
				case DWC_CONNECTINET_STATE_FATAL_ERROR:
					{
						// エラー表示
						DWCError err;
						int errcode;
						err = DWC_GetLastError(&errcode);

						OS_Panic("   Error occurred %d %d.\n", err, errcode);
					}
					break;
				case DWC_CONNECTINET_STATE_CONNECTED:
			        {	// 接続先を表示する。店舗の場合は店舗情報も表示する。
						DWCApInfo apinfo;
			
						DWC_GetApInfo(&apinfo);
			
			            OS_TPrintf("   Connected to AP type %d.\n", apinfo.aptype);

			            if (apinfo.aptype == DWC_APINFO_TYPE_SHOP)
			            {
							OS_TPrintf("<Shop AP Data>\n");
			                OS_TPrintf("area code: %d.\n", apinfo.area);
			                OS_TPrintf("spotinfo : %s.\n", apinfo.spotinfo);
			            }
			        }
					app_state = SAMPLE_LOGIN;
					break;
				}
			}
			break;
		case SAMPLE_LOGIN:
		    // ユーザデータ読み込み。ゲームに組み込む際は、ゲーム固有の処理を行ってください。
		    DTUD_Init();
			
		    // フレンドマッチライブラリ初期化。ゲームに組み込む際は、ゲーム固有の処理を行ってください。
#if defined(_NITRO) && SDK_VERSION_MAJOR > 4
            DWC_InitFriendsMatch(DTUD_GetUserData(),
                                 GAME_PRODUCTID, GAME_SECRET_KEY,
                                 0, 0,
                                 NULL, 0 );
#else
            DWC_InitFriendsMatch(&stFMCtrl, DTUD_GetUserData(),
                                 GAME_PRODUCTID, GAME_NAME, GAME_SECRET_KEY,
                                 0, 0,
                                 NULL, 0 );
#endif
			
			is_logined = FALSE;
			
            // 認証用関数を使ってログイン
            DWC_LoginAsync(L"サトシ", NULL, login_callback, NULL);
			
			app_state = SAMPLE_LOGIN_WAIT;
			break;
		case SAMPLE_LOGIN_WAIT:
		    DWC_ProcessFriendsMatch();  // DWC通信処理更新
			
			if (is_logined)
			{
		        // ingamesnチェックの結果を取得する
			    if( DWC_GetIngamesnCheckResult() == DWC_INGAMESN_INVALID )
		        {
		            // 不適切な名前と判断された場合は特別な処理が必要
				    OS_TPrintf(" BAD ingamesn is detected by NAS.\n");
		        }
				
				// すぐにログオフ
	            DWC_ShutdownFriendsMatch();
				
				app_state = SAMPLE_BT_INIT;
			}
			break;
		case SAMPLE_BT_INIT:
			// Dpw_Btの初期化を行う。
			Dpw_Bt_Init(pid, DWC_CreateFriendKey(DTUD_GetUserData()), server_type);
			app_state = SAMPLE_BT_GET_SERVER_STATE;
			break;
		case SAMPLE_BT_GET_SERVER_STATE:
			// サーバーの状態を取得する
			Dpw_Bt_GetServerStateAsync();
			app_state = SAMPLE_BT_GET_SERVER_STATE_WAIT;
			break;
		case SAMPLE_BT_GET_SERVER_STATE_WAIT:
			if (Dpw_Bt_IsAsyncEnd())
			{
				s32 result = Dpw_Bt_GetAsyncResult();

				if (result < 0)
				{	// エラーのとき
					OS_TPrintf(" error %d", result);
					app_state = SAMPLE_ERROR_OCCURED;
				}
				else
				{
					switch (result)
					{
					case DPW_BT_STATUS_SERVER_OK:		// 正常に動作している
						OS_TPrintf(" server is up!\n");
						app_state = SAMPLE_BT_SETPROFILE;
						break;
					case DPW_BT_STATUS_SERVER_STOP_SERVICE:	// サービス停止中
						OS_TPrintf(" server stop service.\n");
						app_state = SAMPLE_ERROR_OCCURED;
						break;
					case DPW_BT_STATUS_SERVER_FULL:		// サーバーが満杯
						OS_TPrintf(" server full.\n");
						app_state = SAMPLE_ERROR_OCCURED;
						break;
					}
				}
			}
			break;
            /*
		case SAMPLE_BT_INIT_SERVER:
			// サーバーを初期化する。
			// この作業は、本番サーバでは非常事態にしか行われない。
			// テストサーバではデバッグ用の関数を用いて任意のタイミングで行うことができる。
			if (Dpw_Bt_Db_InitServer())
			{
				OS_TPrintf(" server init success.\n");
				app_state = SAMPLE_BT_GET_ROOM_NUM;
			}
			else
			{
				OS_TPrintf(" server update failed.\n");
				app_state = SAMPLE_ERROR_OCCURED;
			}
			break;
                */
		case SAMPLE_BT_SETPROFILE:
            memset(&profile, 0, sizeof(profile));
			profile.version = 10;
            profile.language = 8;
            profile.countryCode = 110;
            profile.localCode = 1;
            profile.playerId = 12345678;
        	profile.playerName[0] = 0x5c;      // カ
        	profile.playerName[1] = 0x6a;      // ス
        	profile.playerName[2] = 0x90;      // ミ
        	profile.playerName[3] = 0xffff;    // 終端
            profile.flag = 0;
            strcpy(profile.mailAddr, MAIL_ADDR);
            profile.mailAddrAuthPass = DPW_MAIL_ADDR_AUTH_DEBUG_PASSWORD;   // デバッグ用パスワードを使用する。
            profile.mailAddrAuthVerification = 123;
            profile.mailRecvFlag = DPW_PROFILE_MAILRECVFLAG_EXCHANGE;
            
			Dpw_Bt_SetProfileAsync(&profile, &profile_result);
			
			app_state = SAMPLE_BT_SETPROFILE_WAIT;
			break;
		case SAMPLE_BT_SETPROFILE_WAIT:
			if (Dpw_Bt_IsAsyncEnd()) {
				s32 result = Dpw_Bt_GetAsyncResult();
				
				if (result < 0) {	// エラーのとき
					OS_TPrintf(" error %d", result);
                    
					app_state = SAMPLE_ERROR_OCCURED;
				} else {
                    OS_TPrintf(" setProfile result. code:%u, mailAddrAuthResult:%u\n", profile_result.code, profile_result.mailAddrAuthResult);
                    if(profile_result.code == DPW_PROFILE_RESULTCODE_SUCCESS && profile_result.mailAddrAuthResult == DPW_PROFILE_AUTHRESULT_SUCCESS)
                    {
                        // デバッグ用パスワードを使っているので必ず成功するはず
    					app_state = SAMPLE_BT_GET_ROOM_NUM;
                    } else {
                        OS_TPrintf(" setProfile failed.");
					    app_state = SAMPLE_ERROR_OCCURED;
                    }
				}
			}
			break;
		case SAMPLE_BT_GET_ROOM_NUM:
			// 指定したランクのルーム数を取得する
			Dpw_Bt_GetRoomNumAsync(BT_ROOM_RANK);
			app_state = SAMPLE_BT_GET_ROOM_NUM_WAIT;
			break;
		case SAMPLE_BT_GET_ROOM_NUM_WAIT:
			if (Dpw_Bt_IsAsyncEnd())
			{
				s32 result = Dpw_Bt_GetAsyncResult();

				if (result < 0)
				{	// エラーのとき
					OS_TPrintf(" error %d", result);
					app_state = SAMPLE_ERROR_OCCURED;
				}
				else
				{
					room_num = result;		// ルーム数を保存する
					OS_TPrintf(" room num %d\n", room_num);
					app_state = SAMPLE_BT_UPLOAD_PLAYER_FOR_CANCEL;
				}
			}
			break;
		case SAMPLE_BT_UPLOAD_PLAYER_FOR_CANCEL:
			// ７勝したプレイヤーのデータをアップロードする。
			// サーバーをアップデートしたときにこの部屋のリーダーになるはず。
			// 番号の一番大きい部屋へアップロードする
			Dpw_Bt_UploadPlayerAsync(BT_ROOM_RANK, room_num, 7, &upload_player, token, TOKEN_LEN);
			app_state = SAMPLE_BT_CANCEL;
			break;
		case SAMPLE_BT_CANCEL:
			// プレイヤーのアップロードをキャンセルしてみる
			Dpw_Bt_CancelAsync();
			app_state = SAMPLE_BT_CANCEL_WAIT;
			break;
		case SAMPLE_BT_CANCEL_WAIT:
			if (Dpw_Bt_IsAsyncEnd())
			{
				s32 result = Dpw_Bt_GetAsyncResult();
				
				if (result == DPW_BT_ERROR_CANCEL)
				{	// キャンセルできたとき
					OS_TPrintf(" cancel upload player sccess.\n");
					app_state = SAMPLE_BT_UPLOAD_PLAYER;
				}
				else if (result == 0)
				{
					// キャンセルできず成功が返ってきた場合は、処理が終了して
					// しまっている。アップロードが終了しているものとして続行。
					OS_TPrintf(" cannot cancel, upload player. ");
					app_state = SAMPLE_BT_DOWNLOAD_ROOM;
				}
				else
				{	// キャンセル以外のエラーのとき。
					OS_TPrintf(" error %d", result);
					app_state = SAMPLE_ERROR_OCCURED;
				}
			}
			break;
		case SAMPLE_BT_UPLOAD_PLAYER:
			// ７勝したプレイヤーのデータをアップロードする。
			// サーバーをアップデートしたときにこの部屋のリーダーになるはず。
			// 番号の一番大きい部屋へアップロードする
			Dpw_Bt_UploadPlayerAsync(BT_ROOM_RANK, room_num, 7, &upload_player, token, TOKEN_LEN);
			app_state = SAMPLE_BT_UPLOAD_PLAYER_WAIT;
			break;
		case SAMPLE_BT_UPLOAD_PLAYER_WAIT:
			if (Dpw_Bt_IsAsyncEnd())
			{
				s32 result = Dpw_Bt_GetAsyncResult();

				if (result < 0)
				{	// エラーのとき
					OS_TPrintf(" error %d", result);
					app_state = SAMPLE_ERROR_OCCURED;
				}
				else
				{
					OS_TPrintf(" upload player.\n");
					app_state = SAMPLE_BT_DOWNLOAD_ROOM;
				}
			}
			break;
            /*
		case SAMPLE_BT_UPDATE_SERVER:
			// サーバーをアップデートする。
			// この作業は、本番サーバでは１日１回定期的に行われるが、
			// テストサーバではデバッグ用の関数を用いて任意のタイミングでアップデートする。
			if (Dpw_Bt_Db_UpdateServer())
			{
				OS_TPrintf(" server update success.\n");
				app_state = SAMPLE_BT_DOWNLOAD_ROOM;
			}
			else
			{
				OS_TPrintf(" server update failed.\n");
				app_state = SAMPLE_ERROR_OCCURED;
			}
			break;
                */
		case SAMPLE_BT_DOWNLOAD_ROOM:
			// ルームをダウンロードする
			Dpw_Bt_DownloadRoomAsync(BT_ROOM_RANK, room_num, &download_room);
			app_state = SAMPLE_BT_DOWNLOAD_ROOM_WAIT;
			break;
		case SAMPLE_BT_DOWNLOAD_ROOM_WAIT:
			if (Dpw_Bt_IsAsyncEnd())
			{
				s32 result = Dpw_Bt_GetAsyncResult();

				if (result < 0)
				{	// エラーのとき
					OS_TPrintf(" error %d", result);
					app_state = SAMPLE_ERROR_OCCURED;
				}
				else
				{
					OS_TPrintf(" download room.\n");

					{
						RTCDate date;
						RTCTime time;
						
						// 時間を取得する
						DWC_GetDateTime(&date, &time);
						
						OS_TPrintf(" current time is %02d/%02d/%02d %02d:%02d:%02d(GMT).\n",
								   date.year, date.month, date.day, time.hour, time.minute, time.second);
					}
					/*
					// ダウンロードしたデータをアップロードしたデータと比較する。
					// ７勝でアップロードしたので、リーダーになっているはず。
					if (memcmp(&download_room.player[6], &upload_player, sizeof(Dpw_Bt_Player)) == 0)
					{
						OS_TPrintf(" download correct leader player data.\n");
					}
					else
					{
						OS_TPrintf(" download incorrect leader player data.\n");
					}
					
					// リーダーの履歴データが更新されているかを調べる。
					if (memcmp(&download_room.leader[29], &upload_player.playerName, sizeof(Dpw_Bt_Leader)) == 0)
					{
						OS_TPrintf(" download correct leader data.\n");
					}
					else
					{
						OS_TPrintf(" download incorrect leader data.\n");
					}
*/
					app_state = SAMPLE_BT_END;
				}
			}
			break;
		case SAMPLE_BT_END:
			// Dpw_Btライブラリを終了する。
			Dpw_Bt_End();
			OS_TPrintf("DPW BT sample end.\n");
			app_state = SAMPLE_END;
			break;
		case SAMPLE_ERROR_OCCURED:
			OS_TPrintf("DPW BT error occured.\n");
			break;
		case SAMPLE_END:
			break;
		}
	}
}

/*-----------------------------------------------------------------------*
					ローカル関数定義
 *-----------------------------------------------------------------------*/

// Vブランク待ち関数
static void GameWaitVBlankIntr(void)
{
	int i;

	dbs_DemoUpdate();

	// 想定するゲームフレームに合わせてVブランク待ちの回数を変える
	for (i = 0; i < GAME_FRAME; i++)
	{
		OS_WaitIrq(TRUE, OS_IE_V_BLANK);
	}

	Heap_Debug();
	dbs_DemoLoad();

	// スタック溢れチェック
	OS_CheckStack(OS_GetCurrentThread());
}

// Vブランク割り込み関数
static void VBlankIntr(void)
{
	//---- 割り込みチェックフラグ
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

static void set_upload_player(void) {
	
    // コイキング、ポッチャマとかの普通のデータ
    u8 data[] = {129,0,0,0,150,0,0,0,0,0,0,0,54,12,238,45,34,213,25,43,245,97,234,21,0,0,0,0,0,0,0,1,33,70,100,0,85,0,94,0,161,0,97,0,255,255,255,255,255,255,255,255,255,255,255,255,137,1,0,0,250,0,54,0,65,0,56,0,236,18,227,64,163,190,169,16,220,8,120,10,0,0,0,0,0,0,0,1,67,71,142,0,116,0,114,0,148,0,143,0,255,255,0,0,0,0,0,0,0,0,255,255,14,0,0,0,106,0,0,0,0,0,0,0,0,0,0,0,57,148,138,197,0,0,0,0,0,0,0,0,0,0,0,1,61,70,100,0,96,0,241,0,161,0,255,255,0,0,0,0,0,0,0,0,0,0,255,255,25,0,78,0,241,0,255,255,0,0,0,0,0,0,0,0,10,1,0,0,176,238,83,172,1,0,4,0,83,4,255,255,0,24,0,0,0,0,89,4,255,255,1,0,0,0,115,4,255,255,2,0,0,0,123,5,255,255,215,3};
    memcpy(upload_player.pokemon, data, sizeof(data));
    upload_player.countryCode = 110;
    upload_player.langCode = 8;
    upload_player.ngname_f = 0;
	upload_player.playerName[0] = 0x5c;      // カ
	upload_player.playerName[1] = 0x6a;      // ス
	upload_player.playerName[2] = 0x90;      // ミ
	upload_player.playerName[3] = 0xffff;    // 終端
	upload_player.result = (u16) (7 * 1000 + 500);
}

static void fill_random_data(void *addr, u32 size)
{

	u32 i;

	for (i = 0; i < size; i++)
	{
		((u8 *) addr)[i] = (u8) MATH_Rand16(&rand_cont16, 0x100);
	}
}

static void login_callback(DWCError error, int profileID, void* param)
{
#pragma unused(param)
	
	if (error == DWC_ERROR_NONE)
	{
        OS_TPrintf(" Login succeeded. profileID = %u\n", profileID);
		
		// PIDを取得
		pid = profileID;
		
        // stUserDataが更新されているかどうかを確認。
        if ( DTUD_CheckUpdateUserData() )
        {
            DTUD_SaveUserData();
        }
		
		is_logined = TRUE;
	}
	else
	{
        OS_TPrintf(" Login Error\n");
	}
}

