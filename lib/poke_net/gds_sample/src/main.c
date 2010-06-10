/*---------------------------------------------------------------------------*
  Pokemon Network Library Test Client

  --履歴--
  [2010/04/15] Siota
   ・バトルデータのアップロード機能をメニューから削除

  [2010/03/29] Shinichiro Yamashita
・poke_net機能(バトルビデオアップデート)のインタフェース変更に伴い修正
  
  [2010/03/17] Shinichiro Yamashita
  ・履歴コメントを新規追加
  ・gds_header修正に伴い、サンプルも修正・更新
  ・crc値を計算するように修正
 *---------------------------------------------------------------------------*/

#include <dwc.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "main.h"
#include "init.h"
#include "utility.h"
#include "screen.h"

#include "dbs.h"
#include "crc.h"

// poke_net 関連
#include "../../poke_net/poke_net_gds.h"	// POKE_NETライブラリのヘッダ
#include "../../poke_net/poke_net_opt.h"	// バトルビデオIDの変換メソッドが宣言してあります。

/** --------------------------------------------------------------------
  defines
  ----------------------------------------------------------------------*/
#define INITIAL_CODE    'NTRJ'     // このサンプルが仕様するイニシャルコード
#define GAME_NAME        "poke_net_test" // このサンプルが使用するゲーム名
#define GAME_FRAME       1

// サーバのURLまたはIPアドレスとポート番号を定義
#define SYACHI_SERVER_URL	("pkgdstest.nintendo.co.jp")
#define SYACHI_SERVER_PORT	(12401)
//#define SYACHI_SERVER_URL	("192.168.0.133")
//#define SYACHI_SERVER_PORT	(23457)

// Auth構造体に設定する値を定義
#define	AUTH_PID				(2345678)	// PID
#define AUTH_ROMCODE			(20)		// ROM コード
#define AUTH_LANGCODE			(1)			// 言語コード
//#define BATTLEID_AUTO_ENCODE

// この値をPIDとして送信すると、色々なチェックが無視されるので注意！
#define TRAFFIC_CHECK_PID		(9999999)

// バトルビデオバージョン。新バージョンが出ない限り100。
#define	BATTLEVIDEO_SERVER_VER		(100)

// イベントバトルビデオのIDへ変換する。
//#define TO_EVENT_BATTLEVIDEO_ID(x)	(x + 900000000000ULL)
#define TO_EVENT_BATTLEVIDEO_ID(x)	(x)

// *
// * エラーコード⇔メッセージ定義
// *
const char* sErrorMessageList[] = {
	"NONE",							// [ 0] POKE_NET_GDS_LASTERROR_NONE
	"NOTINITIALIZED",				// [ 1] POKE_NET_GDS_LASTERROR_NOTINITIALIZED
	"ILLEGALREQUEST",				// [ 2] POKE_NET_GDS_LASTERROR_ILLEGALREQUEST
	"CREATESOCKET",					// [ 3] POKE_NET_GDS_LASTERROR_CREATESOCKET
	"IOCTRLSOCKET",	                // [ 4] POKE_NET_GDS_LASTERROR_IOCTRLSOCKET
	"NETWORKSETTING",				// [ 5] POKE_NET_GDS_LASTERROR_NETWORKSETTING
	"CREATETHREAD",					// [ 6] POKE_NET_GDS_LASTERROR_CREATETHREAD
	"CONNECT",						// [ 7] POKE_NET_GDS_LASTERROR_CONNECT
	"SENDREQUEST",					// [ 8] POKE_NET_GDS_LASTERROR_SENDREQUEST
	"RECVRESPONSE",					// [ 9] POKE_NET_GDS_LASTERROR_RECVRESPONSE
	"CONNECTTIMEOUT",				// [10] POKE_NET_GDS_LASTERROR_CONNECTTIMEOUT
	"SENDTIMEOUT",					// [11] POKE_NET_GDS_LASTERROR_SENDTIMEOUT
	"RECVTIMEOUT",					// [12] POKE_NET_GDS_LASTERROR_RECVTIMEOUT
	"ABORTED"						// [13] POKE_NET_GDS_LASTERROR_ABORTED
};

/** --------------------------------------------------------------------
  globals
  ----------------------------------------------------------------------*/
KeyControl		g_KeyCtrl;

/** --------------------------------------------------------------------
  statics
  ----------------------------------------------------------------------*/
static u8 s_Work[ DWC_INIT_WORK_SIZE ] ATTRIBUTE_ALIGN( 32 );
static DWCInetControl			s_ConnCtrl;
static int  s_state  = state_init;
static POKE_NET_REQUESTCOMMON_AUTH sReqAuth;
#define BUFF_SIZE (32 * 1024)
static u8 sReqBuff[BUFF_SIZE] = "";
static u8 sResBuff[BUFF_SIZE] = "";
static DWCApInfoType sApInfoType;
const wchar_t g_TrainerName[]	= L"でぃーえす";				// プロフィールとミュージカルデータに使用
const wchar_t g_TrainerName2[]  = L"ミニスカートが愛を叫ぶ";	// バトル本体のトレーナー情報に使用(現ソースでは未使用)
const wchar_t g_MusicalTitle[]	= L"クール曲";	// ミュージカルタイトル

/** --------------------------------------------------------------------
  static functions
  ----------------------------------------------------------------------*/
static void initFunctions(void);
static void update( void );
static int update_select_connect(GameSequence* seq);
static int update_init(GameSequence*);
static int update_connect(GameSequence*);
static int update_select_pid(GameSequence*);
static int update_input_pid(GameSequence*);
static int update_gds_init(GameSequence*);
static int update_menu(GameSequence*);
static int update_gds_response(int seqNo);
// メニューアイテム
static int update_debug_message(GameSequence*);
static int update_musical_update(GameSequence*);
static int update_musical_download(GameSequence*);
static int update_battle_upload(GameSequence*);
static int update_battle_search(GameSequence*);
static int update_battle_rank_search(GameSequence*);
static int update_battle_download(GameSequence*);
static int update_event_battle_download(GameSequence*);
static int update_event_battle_bookmark(GameSequence*);
static int update_error(GameSequence*);
static int update_query_disconnect(GameSequence*);
static int update_disconnect(GameSequence*);
// 入力
static void initInput(InputElement* input, u64 min, u64 max, u64 first);
static int  processInput(GameSequence* seq, s16 x, s16 y);
static void initInputSequence(InputSequence* input);
static int processSceneSequence(GameSequence* input);
static s16 dispInput(InputElement* input, s16 x, s16 y);
static u64 getResultValue(GameSequence* seq, int no);
// ユーティリティ
static void nextGameSequence(int seqNo);
static BOOL processMenuSequence(GameSequence* seq, s16 x, s16 y);
static int  processSceneElement(GameSequence* seq, s16 x, s16 y);
static s16  DispTitle( const char* title, s16 x, s16 y);
static s16  DispSubTitle( const char* title, s16 x, s16 y);
static void GameWaitVBlankIntr(void);

// プロフィール作成
void MakeProfileData(GDS_PROFILE* _pProfile);
// PMSデータ作成
void MakePMSData(PMS_DATA* _pPMS);
// ミュージカルショット作成
void MakeMusicalshotData(MUSICAL_SHOT_DATA *_pMusicalData, int _MonsNoInc);
// バトルビデオデータ作成
void MakeBattleVideoHeader(BATTLE_REC_HEADER *_bhead, BOOL bSubway, int _MonsNoInc);	// ヘッダ
void MakeBattleVideoData(BATTLE_REC_WORK *_brec, int _MonsNoInc);						// 本体
// 終端文字(0xFFFF)を代入します。
// バッファオーバーフローに注意してください。
// (必ずバッファの最後の要素には0が入るようにしてください。)
void SetGDS_EOM(STRCODE* _pStr, int _Size);

// メニューアイテム
GameSequence sGameSeqs[] = {
    // select_connect
    {
        "SELECT AP",
        1,
        update_select_connect,
        {
            {   TYPE_SELECT, 4,
                {
                    { "Connection1",},
                    { "Connection2",},
                    { "Connection3",},
                    { "Any",},
                },
            },
        },
    },
    // init
    {
        "INIT",
        1,
        update_init,
        {
            { TYPE_NORMAL, 1, },
        },
    },
    // connect
    {
        "CONNECTING",
        1,
        update_connect,
        {
            { TYPE_NORMAL, 1, },
        },
    },
    // select_pid
    {
        "SELECT USE PID",
        1,
        update_select_pid,
        {
            {   TYPE_SELECT, 3,
                {
                    {"USE DEFAULT PID",},
                    {"INPUT PID",},
                    {"USE DEBUG PID",},
                },
            },
        },
    },
    // input pid
    {
        "INPUT PID",
        1,
        update_input_pid,
        {
            {   TYPE_INPUT, 1,
                {
                    {"INPUT PID",},
                },
            },
        },
    },
    // gds_init
    {
        "GDS INIT",
        1,
        update_gds_init,
        {
            { TYPE_NORMAL, 1, },
        },
    },
    // menu
    {
        "MENU",
        1,
        update_menu,
        {
            {   TYPE_SELECT, 
#ifdef BATTLEVIDEO_UPLOAD_ENABLE
                9,
#else
                8,
#endif
                {
                    {"test request",},
                    {"musical update",},
                    {"musical download",},
#ifdef BATTLEVIDEO_UPLOAD_ENABLE
                    {"battle upload",},
#endif
                    {"battle search",},
                    {"battle rank search",},
                    {"battle download",},
                    {"event battle download",},
                    {"battle bookmark",},
                },
            },
        },
    },
    // debug_message
    {
        "DEBUG MESSAGE",
        1,
        update_debug_message,
        {
            { TYPE_NORMAL, 1, }, 
        },
    },
    // musical_update
    {
        "MUSICAL UPDATE",
        1,
        update_musical_update,
        {
            {   TYPE_INPUT, 1, 
                {
                    {"INPUT NO",},
                }
            },
        },
    },
    
    // musical_download
    {
        "MUSICAL DOWNLOAD",
        1,
        update_musical_download,
        {
            {   TYPE_INPUT, 1,
                {
                    {"INPUT NO",},
                },
            },
        },
    },
#ifdef BATTLEVIDEO_UPLOAD_ENABLE
    // battle_upload
    {
        "BATTLE UPLOAD",
        2,
        update_battle_upload,
        {
            {   TYPE_INPUT, 1,
                {
                    {"INPUT NO",},
                },
            },
            {   TYPE_SELECT, 2,
                {
                    {"BATTLE SUBWAY",},
                    {"OTHER",},
                },
            },
        },
    },
#endif
    // battle_search
    {
        "BATTLE SEARCH",
        2,
        update_battle_search,
        {
            {   TYPE_INPUT, 1,
                {
                    {"INPUT NO",},
                },
            },
            {
                TYPE_SELECT, 3,
                {
                    {"NO TARGET"},
                    {"BATTLE SUBWAY",},
                    {"REMOTE BATTLE",},
                },
            },
        },
    },
    // battle_rank_search
    {
        "BATTLE RANK SEARCH",
        1,
        update_battle_rank_search,
        {
            {   TYPE_SELECT, 3,
                {
                    {"LATEST 30",},
                    {"SUBWAY",},
                    {"COMM",},
                },
            },
        },
    },
    // battle_download
    {
        "BATTLE DOWNLOAD",
        1,
        update_battle_download,
        {
            {   TYPE_INPUT, 1,
                {
                    {"INPUT NO",},
                },
            },
        },
    },
    // event_battle_download
    {
        "EVENT BATTLE DOWNLOAD",
        1,
        update_event_battle_download,
        {
            {   TYPE_INPUT, 1,
                {
                    {"INPUT NO",},
                },
            },
        },
    },
    // event_battle_bookmark
    {
        "EVENT BATTLE BOOKMARK",
        1,
        update_event_battle_bookmark,
        {
            {   TYPE_INPUT, 1,
                {
                    {"INPUT NO",},
                },
            },
        },
    },
    // error
    {
        "ERROR",
        1,
        update_error,
        {
            { TYPE_NORMAL, 1, },
        },
    },
    // query disconnect
    {
        "DISCONNECT?",
        1,
        update_query_disconnect,
        {
            {   TYPE_SELECT, 2,
                {
                    {"YES",},
                    {"NO",},
                },
            },
        },
    },
    // disconnect
    {
        "DISCONNECT",
        1,
        update_disconnect,
        {
            { TYPE_NORMAL, 1, },
        },
    },
};
GameSequence* sCurSeq = NULL;

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
  
  ----------------------------------------------------------------------*/
/** --------------------------------------------------------------------
  接続先 AP 選択
  ----------------------------------------------------------------------*/
static int update_select_connect(GameSequence* seq)
{
    if (seq->result == INPUT_CONFIRM) {
        int ret = (int)getResultValue(seq, 0);

        switch (ret) {
            case 0:  sApInfoType = DWC_APINFO_TYPE_USER0; break;    
            case 1:  sApInfoType = DWC_APINFO_TYPE_USER1; break;    
            case 2:  sApInfoType = DWC_APINFO_TYPE_USER2; break;    
            default: sApInfoType = DWC_APINFO_TYPE_FREESPOT; break;    
        }

        // ついでに初期化
        DWC_InitInet( &s_ConnCtrl );
        
        //DWC_SetAuthServer( DWC_CONNECTINET_AUTH_RELEASE );
        // 接続先を設定
        if (sApInfoType != DWC_APINFO_TYPE_FREESPOT) {
            DWC_SetConnectApType(sApInfoType);
        }

        DWC_ConnectInetAsync();

        return state_connect;
    }

    return state_select_connect;
}

/** --------------------------------------------------------------------
  初期状態
  ----------------------------------------------------------------------*/
static int update_init(GameSequence* seq)
{
#pragma unused(seq)
    DWC_InitInet( &s_ConnCtrl );
        
    //DWC_SetAuthServer( DWC_CONNECTINET_AUTH_RELEASE );
    // 接続先を設定
    DWC_SetConnectApType(DWC_APINFO_TYPE_USER0);
        
    DWC_ConnectInetAsync();

    return state_connect;
}

/** --------------------------------------------------------------------
  接続中
  ----------------------------------------------------------------------*/
static int update_connect(GameSequence* seq)
{
#pragma unused(seq)
    DWC_ProcessInet();

    if ( DWC_CheckInet() )
    {
        int status;

        status = DWC_GetLastError(NULL);

        if ( status == DWC_ERROR_NONE )
        {
        	switch ( DWC_GetInetStatus() )
	        {
		        case DWC_CONNECTINET_STATE_ERROR:{
			    // エラー表示
			    DWCError err;
			    int errcode;

			    err = DWC_GetLastError( &errcode );
			    OS_TPrintf("   Error occurred %d %d.\n", err, errcode );

			    DWC_ClearError();
			    DWC_CleanupInet();
		    }
            break;
		    case DWC_CONNECTINET_STATE_FATAL_ERROR:{
			    // エラー表示
			    DWCError err;
			    int errcode;
			    err = DWC_GetLastError( &errcode );

			    OS_Panic("   Error occurred %d %d.\n", err, errcode );
		    }
            break;

		    case DWC_CONNECTINET_STATE_CONNECTED:
			    OS_TPrintf("   CONNECTED!!!.\n");
			    return state_select_pid;
	        }
        }
    }

    return state_connect;
}

/** --------------------------------------------------------------------
  PID 使用方法選択
  ---------------------------------------------------------------------*/
static int update_select_pid(GameSequence* seq)
{
    if (seq->result == INPUT_CONFIRM) {
        int no = (int)getResultValue(seq, 0);
        int next = state_gds_init;
        if (no == 0) sReqAuth.PID = AUTH_PID;
        else if( no == 2) sReqAuth.PID = TRAFFIC_CHECK_PID;
        else {
            next = state_input_pid;
        }
        return next;
    }

    return state_select_pid;
}

/** --------------------------------------------------------------------
  PID 入力
  ---------------------------------------------------------------------*/
static int update_input_pid(GameSequence* seq)
{
    if (seq->result == INPUT_CONFIRM) {
        sReqAuth.PID = (int)getResultValue(seq, 0);
        return state_gds_init;
    }
    return state_select_pid;
}

/** --------------------------------------------------------------------
  GDS ライブラリ初期化
  ---------------------------------------------------------------------*/
static int update_gds_init(GameSequence* seq)
{
#pragma unused(seq)
    OS_Printf("GDS ライブラリ初期化中 ... ");

	sReqAuth.ROMCode	= AUTH_ROMCODE;
	sReqAuth.LangCode	= AUTH_LANGCODE;

    if (! POKE_NET_GDS_Initialize(&sReqAuth, SYACHI_SERVER_URL, SYACHI_SERVER_PORT)) {
        OS_Printf("GDS ライブラリの初期化に失敗 !\n");
        OS_Halt();
    }

    OS_Printf("GDS ライブラリ初期化正常終了!\n");

    return state_menu;
}

/** --------------------------------------------------------------------
  メニュー
  ----------------------------------------------------------------------*/
static int update_menu(GameSequence* seq)
{
    if (seq->result == INPUT_CANCEL) return state_query_disconnect;

    return (int)((getResultValue(seq, 0) + state_menu + 1));
}

/** --------------------------------------------------------------------
  次のメニュー
  ----------------------------------------------------------------------*/
static void nextGameSequence(int seqNo)
{
    sCurSeq = &sGameSeqs[seqNo];
}

/** --------------------------------------------------------------------
  メニュー処理
  ----------------------------------------------------------------------*/
static BOOL processMenuSequence(GameSequence* seq, s16 x, s16 y)
{
    InputSequence* input = &seq->inputList[seq->currentElement];
    
    PrintString( x,y++, 0xf, "=============================");
    {
        int i;
        for (i = 0; i < input->numElements; ++i) {
            if (input->currentElement == i) PrintString(x, y, 0xf, ">");
            else              PrintString(x, y, 0xf, " ");
            PrintString((s16)(x+1), y++, 0xf, "%s", input->elements[i].head);
        }
    }
    PrintString( x,y++, 0xf, "=============================");

    // キー操作
    if (g_KeyCtrl.trg & PAD_KEY_DOWN) {
        if (input->currentElement == (input->numElements - 1)) {
            input->currentElement = 0;
        } else {
            ++input->currentElement;
        }
    }
    else if (g_KeyCtrl.trg & PAD_KEY_UP) {
        if (input->currentElement == 0) {
            input->currentElement = input->numElements - 1;
        } else {
            --input->currentElement;
        }
    }
    if (g_KeyCtrl.trg & PAD_BUTTON_A) {
        input->result =input->currentElement;
        return INPUT_CONFIRM; 
    }
    else if (g_KeyCtrl.trg & PAD_BUTTON_B) {
        return INPUT_CANCEL; 
    }

    return INPUT_NOW;
}

/** --------------------------------------------------------------------
  デバッグメッセージ
  ----------------------------------------------------------------------*/
static int update_debug_message(GameSequence* seq)
{
#pragma unused(seq)
    char str[] = "TestMessage for GDS";
    
    OS_TPrintf("test message send to GDS... > %s\n", str);

    POKE_NET_GDS_DebugMessageRequest(str, sResBuff);

    OS_TPrintf("test message send done.\n");

    return update_gds_response(state_menu);
}

/** --------------------------------------------------------------------
  ミュージカルショットアップロード
  ----------------------------------------------------------------------*/
static int update_musical_update(GameSequence* seq)
{
    if (seq->result == INPUT_CONFIRM) {
        GDS_PROFILE profile = {0};
        int no = (int)getResultValue(seq, 0);
        MakeProfileData(&profile);

        MI_CpuClear8(sReqBuff, sizeof(sReqBuff));
        MI_CpuClear8(sResBuff, sizeof(sResBuff));

        // プロフィールをコピー
	    memcpy(&((MUSICAL_SHOT_SEND*)sReqBuff)->profile, &profile, sizeof(GDS_PROFILE));

	    // ミュージカルショットデータを作成
	    MakeMusicalshotData(&((MUSICAL_SHOT_SEND*)sReqBuff)->mus_shot, (int)no);

	    // 通信
	    POKE_NET_GDS_MusicalShotRegist((MUSICAL_SHOT_SEND*)sReqBuff ,sResBuff);

        return update_gds_response(state_menu);
    }

    return state_menu;
}

/** --------------------------------------------------------------------
  ミュージカルショットダウンロード
  ----------------------------------------------------------------------*/
static int update_musical_download(GameSequence* seq)
{
    // 入力処理
    if (seq->result == INPUT_CONFIRM) {
        int no = (int)getResultValue(seq, 0);
        GDS_PROFILE profile = {0};
        MakeProfileData(&profile);
        
        MI_CpuClear8(sReqBuff, sizeof(sReqBuff));
        MI_CpuClear8(sResBuff, sizeof(sResBuff));
        
        POKE_NET_GDS_MusicalShotGet(no, sResBuff); 
        return update_gds_response(state_menu);
    }

    return state_menu;
}

/** --------------------------------------------------------------------
  バトルビデオアップロード
  ----------------------------------------------------------------------*/
static int update_battle_upload(GameSequence* seq)
{
	const int SIGNATURE_SIZE = 128;					// 署名サイズ
	unsigned char pModule[SIGNATURE_SIZE] = {0};	// 署名データ

    if (seq->result == INPUT_CONFIRM) {
        int no   = (int)getResultValue(seq, 0),
            type = (int)getResultValue(seq, 1);
        GDS_PROFILE profile = {0};
        MakeProfileData(&profile);

        MI_CpuClear8(sReqBuff, sizeof(sReqBuff));
        MI_CpuClear8(sResBuff, sizeof(sResBuff));

        // プロフィールをコピー
	    memcpy(&((BATTLE_REC_SEND*)sReqBuff)->profile, &profile, sizeof(GDS_PROFILE));

	    // バトルビデオヘッダを作成
	    MakeBattleVideoHeader(&((BATTLE_REC_SEND*)sReqBuff)->head, (type == 0), no);

	    // バトルビデオ本体を作成
	    MakeBattleVideoData(&((BATTLE_REC_SEND*)sReqBuff)->rec, 1);

	    // 通信
	    POKE_NET_GDS_BattleDataRegist((BATTLE_REC_SEND*)sReqBuff, pModule, SIGNATURE_SIZE ,sResBuff);

        return update_gds_response(state_menu);
    }

    return state_menu;
}

/** --------------------------------------------------------------------
  バトルビデオくわしく検索
  ----------------------------------------------------------------------*/
static int update_battle_search(GameSequence* seq)
{
    if (seq->result == INPUT_CONFIRM) {
    	int battleMode;

	    // 検索条件用の構造体
	    BATTLE_REC_SEARCH_SEND Serch = {0};

        memset(sReqBuff ,0x00 ,sizeof(sReqBuff));
	    memset(sResBuff ,0x00 ,sizeof(sResBuff));

        Serch.monsno = (u16)getResultValue(seq, 0);

	    if(Serch.monsno == 0)
	    {
	        Serch.monsno = BATTLE_REC_SEARCH_MONSNO_NONE;
	    }

	    // 国／地方コード
	    Serch.country_code = BATTLE_REC_SEARCH_COUNTRY_CODE_NONE;
	    Serch.local_code = BATTLE_REC_SEARCH_LOCAL_CODE_NONE;

	    // バトルビデオのバージョン
	    Serch.server_version = BATTLEVIDEO_SERVER_VER;

	    // 戦闘モード検索
        // 以下を変えると結果が変わります
        battleMode = (int)getResultValue(seq, 1); 
	    if(battleMode == 0)
	    {
		    Serch.battle_mode = 0;
		    Serch.battle_mode_bit_mask = 0;
	    }
	    else if(battleMode == 1)
	    {
		    // バトルサブウェイを指定
		    Serch.battle_mode = BATTLEMODE_COMMUNICATION_SUBWAY << BATTLEMODE_COMMUNICATION_BIT;
		    Serch.battle_mode_bit_mask = BATTLEMODE_COMMUNICATION_MASK;
	    }
	    else
	    {
		    // 通信対戦を指定
		    Serch.battle_mode = BATTLEMODE_COMMUNICATION_COMMUNICATION << BATTLEMODE_COMMUNICATION_BIT;
		    Serch.battle_mode_bit_mask = BATTLEMODE_COMMUNICATION_MASK;
	    }

	    // 通信
	    POKE_NET_GDS_BattleDataSearchCondition(&Serch, sResBuff);

        return update_gds_response(state_menu);
    }

    return state_menu;
}

/** --------------------------------------------------------------------
  バトルビデオランキング検索 
  ----------------------------------------------------------------------*/
static int update_battle_rank_search(GameSequence* seq)
{
    if (seq->result == INPUT_CONFIRM) {
        u64 no = getResultValue(seq, 0);

	    // 検索条件用の構造体
	    BATTLE_REC_RANKING_SEARCH_SEND Serch = {0};

	    memset(sReqBuff ,0x00 ,sizeof(sReqBuff));
	    memset(sResBuff ,0x00 ,sizeof(sResBuff));

	    if(no == 0)
	    {
		    // 最新30件
		    POKE_NET_GDS_BattleDataSearchRankingLatest(BATTLEVIDEO_SERVER_VER, sResBuff);
	    }
	    else if(no == 1)
	    {
		    // バトルサブウェイランキング
		    POKE_NET_GDS_BattleDataSearchRankingSubway(BATTLEVIDEO_SERVER_VER, sResBuff);
	    }
	    else
	    {
		    // 通信対戦ランキング
		    POKE_NET_GDS_BattleDataSearchRankingComm(BATTLEVIDEO_SERVER_VER, sResBuff);
    	}

        return update_gds_response(state_menu);
    }

    return state_menu;
}

/** --------------------------------------------------------------------
  バトルビデオダウンロード
  ----------------------------------------------------------------------*/
static int update_battle_download(GameSequence* seq)
{
    if (seq->result == INPUT_CONFIRM) {
        u64 no = getResultValue(seq, 0);

        memset(sReqBuff ,0x00 ,sizeof(sReqBuff));
	    memset(sResBuff ,0x00 ,sizeof(sResBuff));

#ifdef BATTLEID_AUTO_ENCODE
    	// 暗号化
	    no = POKE_NET_EncodePlayDataID(no);
#endif

	    // 通信
	    POKE_NET_GDS_BattleDataGet(no, BATTLEVIDEO_SERVER_VER, sResBuff);

        return update_gds_response(state_menu);
    }

    return state_menu;
}

/** --------------------------------------------------------------------
  イベントバトルビデオダウンロード
  ----------------------------------------------------------------------*/
static int update_event_battle_download(GameSequence* seq)
{
    if (seq->result == INPUT_CONFIRM) {
        u64 no = getResultValue(seq, 0);

    	memset(sReqBuff ,0x00 ,sizeof(sReqBuff));
	    memset(sResBuff ,0x00 ,sizeof(sResBuff));

	    // イベントビデオ用にID変換
	    no = TO_EVENT_BATTLEVIDEO_ID(no);

#ifdef BATTLEID_AUTO_ENCODE
	    // 暗号化
	    no = POKE_NET_EncodePlayDataID(no);
#endif
	    // 通信
	    POKE_NET_GDS_BattleDataGet(no, BATTLEVIDEO_SERVER_VER, sResBuff);

        return update_gds_response(state_menu);
    }

    return state_menu;
}

/** --------------------------------------------------------------------
  バトルビデオお気に入り登録
  ----------------------------------------------------------------------*/
static int update_event_battle_bookmark(GameSequence* seq)
{
    if (seq->result == INPUT_CONFIRM) {
        u64 no = getResultValue(seq, 0);

        memset(sReqBuff ,0x00 ,sizeof(sReqBuff));
	    memset(sResBuff ,0x00 ,sizeof(sResBuff));

#ifdef BATTLEID_AUTO_ENCODE
	    // 暗号化
	    no = POKE_NET_EncodePlayDataID(no);
#endif

	    // 通信
	    POKE_NET_GDS_BattleDataFavorite((u64)no, sResBuff);

        return update_gds_response(state_menu);
    }

    return state_menu;
}

/** --------------------------------------------------------------------
  エラー解除待ち
  ----------------------------------------------------------------------*/
static int update_error(GameSequence* seq )
{
#pragma unused(seq)
    int errcode;
    DWCError err;
    DWCErrorType errtype;
    
    if ( g_KeyCtrl.trg & PAD_BUTTON_START )
    {
        // スタートボタンでエラー解除
        err = DWC_GetLastErrorEx(&errcode, &errtype);

        DWC_ClearError();
        
        // 本サンプルではログインやその他マッチメイク等は行わないので
        // 通信から切断するだけ
        return state_disconnect;
    }

    return state_error;
}

/** --------------------------------------------------------------------
  切断問い合わせ
  ----------------------------------------------------------------------*/
static int update_query_disconnect(GameSequence* seq )
{
    if (seq->result == INPUT_CANCEL || getResultValue(seq, 0) == 1) 
        return state_menu;

    return state_disconnect;
}

/** --------------------------------------------------------------------
  切断
  ----------------------------------------------------------------------*/
static int update_disconnect(GameSequence* seq )
{
#pragma unused(seq)
    if ( DWC_CleanupInetAsync() )
    {
        return state_select_connect;
    }

    return state_disconnect;
}

/** --------------------------------------------------------------------
  GDS レスポンス処理
  ----------------------------------------------------------------------*/
static int update_gds_response(int seqNo)
{
	// 出力メッセージ
	const char* messageList[] = {
		"-inactive\n",		// [0] POKE_NET_GDS_STATUS_INACTIVE		// 非稼動中
		"-initialized\n",	// [1] POKE_NET_GDS_STATUS_INITIALIZED	// 初期化済み
		"-request\n",		// [2] POKE_NET_GDS_STATUS_REQUEST		// リクエスト発行
		"-netsetting\n",	// [3] POKE_NET_GDS_STATUS_NETSETTING	// ネットワーク設定中
		"-connecting\n",	// [4] POKE_NET_GDS_STATUS_CONNECTING	// 接続中
		"-sending\n",		// [5] POKE_NET_GDS_STATUS_SENDING		// 送信中
		"-receiving\n",		// [6] POKE_NET_GDS_STATUS_RECEIVING	// 受信中
		"-aborted\n",		// [7] POKE_NET_GDS_STATUS_ABORTED		// 中断
		"-finished\n",		// [8] POKE_NET_GDS_STATUS_FINISHED		// 正常終了
		"-error"			// [9] POKE_NET_GDS_STATUS_ERROR		// エラー終了
	};

    int lastState = POKE_NET_GDS_GetStatus();
    BOOL loop = TRUE;
    while (loop) {
        int state = POKE_NET_GDS_GetStatus();
        if (lastState == state) {
            OS_Sleep(10);
            continue;
        }

        OS_TPrintf("state changed.\n");
        OS_TPrintf("status: %s\n", messageList[state]);
        
        if (state == POKE_NET_GDS_STATUS_INACTIVE ||
            state == POKE_NET_GDS_STATUS_INITIALIZED ||
            state == POKE_NET_GDS_STATUS_ABORTED ||
            state == POKE_NET_GDS_STATUS_FINISHED ||
            state == POKE_NET_GDS_STATUS_ERROR )
        {
            loop = FALSE;
        }

        // 通信完了
        if (state == POKE_NET_GDS_STATUS_FINISHED) {
            POKE_NET_GDS_DEBUG_PrintResponse((POKE_NET_RESPONSE*)POKE_NET_GDS_GetResponse(), POKE_NET_GDS_GetResponseSize());
        }
        else if (state == POKE_NET_GDS_STATUS_ERROR) {
            OS_TPrintf("GDS ERROR OCCURED!!!\n");
            OS_TPrintf("errorCode:%d\n", POKE_NET_GDS_GetLastErrorCode());
            OS_TPrintf("lastErrorMsg:%s\n", sErrorMessageList[POKE_NET_GDS_GetLastErrorCode()]);
        }
        
        lastState = state;
        
        GameWaitVBlankIntr();
    }

    sCurSeq->currentElement = 0;

    return seqNo;
}

/** -------------------------------------------------------------------------
  ---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
 * タイトル表示
 *---------------------------------------------------------------------------*/
static s16 DispTitle( const char* title, s16 x, s16 y)
{
    PrintString( x,y++, 0xf, "=============================");
    PrintString((s16)(x+1), y++, 0xf, "%s", title);
    PrintString( x,y++, 0xf, "=============================");

    return y;
}

/*---------------------------------------------------------------------------*
 * サブタイトル表示
 *---------------------------------------------------------------------------*/
static s16 DispSubTitle( const char* title, s16 x, s16 y)
{
    PrintString( x,y++, 0xf, "-----------------");
    PrintString((s16)(x+1), y++, 0xf, "%s", title);
    PrintString( x,y++, 0xf, "-----------------");

    return y;
}

/** --------------------------------------------------------------------
  毎フレーム呼び出す、状態表示関数
  ----------------------------------------------------------------------*/
void
drawConsole( void )
{
    static int i = 0;
    char loop[] = "/|\\-";

    dbs_Print( 30,0, "%c", loop[((i)>>2)&3] );

    i++;
}

/** --------------------------------------------------------------------
  エントリポイント
  ----------------------------------------------------------------------*/
void NitroMain ()
{
    initFunctions();

    // デバッグ表示レベル指定
    DWC_SetReportLevel((u32)( DWC_REPORTFLAG_ALL &
                              ~DWC_REPORTFLAG_QR2_REQ ));

    DWC_InitForDevelopment( GAME_NAME, INITIAL_CODE, DTDB_GetAllocFunc(), DTDB_GetFreeFunc() );

    // 最初の処理の設定
    nextGameSequence(state_select_connect);

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

/** --------------------------------------------------------------------
  初期化処理
  ----------------------------------------------------------------------*/
static void
initFunctions( void )
{
	OS_Init();
    OS_InitTick();
    OS_InitAlarm();
    RTC_Init();
    CARD_Init();
    OS_SetThreadStackWarningOffset(OS_GetCurrentThread(), 0x100);
    PXI_Init();
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
static void update( void )
{
    int errcode;
    DWCError err;
    DWCErrorType errtype;
    
    sCurSeq->result = processSceneSequence(sCurSeq);
    nextGameSequence(sCurSeq->fpResult(sCurSeq));

    err = DWC_GetLastErrorEx(&errcode, &errtype);

    if ( err != DWC_ERROR_NONE && s_state != state_error )
    {
        // エラーが発生した場合、ここでは状態をエラー解除待ちにするだけ
        OS_TPrintf( "\n*******************************\n" );
        OS_TPrintf( "ERROR : %d : %d\n", err, errcode );
        OS_TPrintf( "*******************************\n\n" );
        DWC_ClearError();
        nextGameSequence(state_disconnect);
    }
}

/*---------------------------------------------------------------------------*
  Vブランク待ち関数
 *---------------------------------------------------------------------------*/
static void GameWaitVBlankIntr(void)
{
    int i;

    dbs_DemoUpdate();

    // 想定するゲームフレームに合わせてVブランク待ちの回数を変える
    for (i = 0; i < GAME_FRAME; i++){
        OS_WaitIrq(TRUE, OS_IE_V_BLANK);
    }

    Heap_Debug();
    dbs_DemoLoad();

    // スタック溢れチェック
    OS_CheckStack(OS_GetCurrentThread());
}

// **                 
// ** プロフィール作成
// **                 
void MakeProfileData(GDS_PROFILE* _pProfile)
{
	MATHCRC16Table crctable;
	
	MI_CpuClear8(_pProfile, sizeof(GDS_PROFILE));

	// 7文字まで(8文字目はNULL)
	memcpy(
		&_pProfile->name, 
		g_TrainerName,
		sizeof(g_TrainerName)
	);										// プレイヤー名称		[置換] SOAPを利用する
	SetGDS_EOM(_pProfile->name, PLW_TRAINER_NAME_SIZE);

	_pProfile->player_id = 0x1234;			// プレイヤーID			[ － ] 0 - 0xffffffff
	_pProfile->player_sex = 0;				// プレイヤー性別		[拒否] 0 - 1
	_pProfile->birthday_month = 4;			// 誕生月				[拒否] 1 - 12
	_pProfile->trainer_view = 0;			// プレイヤーの見た目	[拒否] 0 - 15

	_pProfile->country_code = 0;			// 住んでいる国コード	[無視] 0 - 233
	_pProfile->local_code = 0;				// 住んでいる地方コード	[無視] * - *

	_pProfile->version_code = 20;			// バージョンコード		[拒否] 20, 21	(WHITE=20, BLACK=21)
	_pProfile->language = 1;				// 言語コード			[拒否] 1 - 8

	_pProfile->egg_flag = 0;				// タマゴフラグ			[無視] 0 - 1
	_pProfile->form_no = 0;					// フォルムNo			[無視] * - *
	_pProfile->mons_sex = 0;				// モンスター性別		[無視] 0 - 1
	_pProfile->monsno = 25;					// モンスターNo			[拒否] 1 - 可変

	// MESSAGE_FLAG_NORMAL(簡易会話) / MESSAGE_FLAG_EVENT(フリーワード)
	_pProfile->message_flag = MESSAGE_FLAG_NORMAL;	//				[拒否] 0 - 0

	// 簡易会話設定
	MakePMSData(&_pProfile->self_introduction);

	// MATH_CalcCRC16CCITT											[拒否] 
	MATH_CRC16CCITTInitTable(&crctable);
	_pProfile->crc.crc16ccitt_hash = MATH_CalcCRC16CCITT(&crctable, _pProfile, sizeof(GDS_PROFILE) - GDS_CRC_SIZE);
}


// **                 
// ** PMSデータ作成
// **                 
void MakePMSData(PMS_DATA* _pPMS)
{
	_pPMS->sentence_type = 0;	// 文章タイプ		[拒否] 0 - 4
	_pPMS->sentence_id = 0;		// タイプ内ID		[拒否] 0 - 19
	_pPMS->word[0] = 0;			// 単語ID			[拒否] 0 - 1735
	_pPMS->word[1] = 0;
}


// **                 
// ** ミュージカルショット作成
// **                 
void MakeMusicalshotData(MUSICAL_SHOT_DATA *_pMusicalData, int _MonsNoInc)
{
    int i;
	MI_CpuClear8(_pMusicalData, sizeof(MUSICAL_SHOT_DATA));

	//ミュージカルタイトル(日本18・海外36＋EOM
	memcpy(&_pMusicalData->title, g_MusicalTitle, sizeof(g_MusicalTitle));
	SetGDS_EOM(_pMusicalData->title, MUSICAL_PROGRAM_NAME_MAX);
	
	_pMusicalData->bgNo		= 1;			    // 背景番号					[拒否] 0 - 4
	_pMusicalData->spotBit	= 0x2;			    // スポットライト対象(bit)(トップだったポケモン)
											    //							[拒否] 0 - 15
	_pMusicalData->year		= 2010 - 2000;	    // 年						[拒否] 0 - 99
	_pMusicalData->month	= 4;			    // 月						[拒否] 1 - 12
	_pMusicalData->day		= 19;			    // 日						[拒否] 1 - 31
	_pMusicalData->player	= 1;			    // 自分の番号				[拒否] 0 - 3
	_pMusicalData->musVer	= 0;			    // ミュージカルバージョン	[拒否] 0 - 0
	_pMusicalData->pmVersion= AUTH_ROMCODE;	    // PM_VERSION				[拒否] 20, 21	(WHITE=20, BLACK=21)
	_pMusicalData->pmLang	= AUTH_LANGCODE;    // PM_LANG					[拒否] 1 - 8

	for(i = 0;i < MUSICAL_POKE_MAX; i++)
	{
		_pMusicalData->shotPoke[i].monsno	= (u16)(_MonsNoInc);    // ポケモン番号	[ － ] 別サーバにてチェック	// ポケモン番号
		_pMusicalData->shotPoke[i].sex		= 0;					// 性別			[ － ]
		_pMusicalData->shotPoke[i].rare		= 0;					// レアフラグ	[ － ]
		_pMusicalData->shotPoke[i].form		= 0;					// フォルム番号	[ － ]
		_pMusicalData->shotPoke[i].perRand	= 0;					//				[ － ]

		// トレーナー名
		// 7文字まで(8文字目はNULL)
		memcpy(
			&_pMusicalData->shotPoke[i].trainerName,
			g_TrainerName,
			sizeof(g_TrainerName)
		);
		SetGDS_EOM(_pMusicalData->shotPoke[i].trainerName, MUSICAL_TRAINER_NAME_LEN);

		// 装備グッズ
		// 最大８つまで装備可能(ここでは１つしか設定しない)
		_pMusicalData->shotPoke[i].equip[0].itemNo	= (u16)i;		    // グッズ番号	[拒否] 0 - 99, 65535	(65535はデータ無しの場合)
		_pMusicalData->shotPoke[i].equip[0].angle	= (s16)(i * 10);	// 角度			[無視] -32767 - 32768
		_pMusicalData->shotPoke[i].equip[0].equipPos= (u8)i;		    // 装備箇所		[拒否] 0 - 8, 10		(10はデータ無しの場合)
	}
}


// **                 
// ** バトルビデオデータ作成
// ** (ヘッダ)
// **                 
void MakeBattleVideoHeader(BATTLE_REC_HEADER* _pHeader, BOOL bSubway, int _MonsNoInc)
{
    int i;
	MATHCRC16Table crctable;

	// 初期化
	MI_CpuClear8(_pHeader, sizeof(BATTLE_REC_HEADER));

	for(i = 0; i < HEADER_MONSNO_MAX; i++)
	{
		_pHeader->monsno[i]	        	= (u16)(_MonsNoInc + i);	// ポケモン番号			[拒否] 1 - 可変
		_pHeader->form_no_and_sex[i]	= 0;				        // ポケモンフォルム番号	[無視]
	}
	
	_pHeader->battle_counter	= 0;								// 連勝数				[拒否] 0 - 9999

	// 戦闘モード													// 戦闘モード			[拒否] 十数種類の固定値
	if(bSubway)
	{
		// ランキング種別はバトルサブウェイ
		_pHeader->mode = 0x1234;//BATTLEMODE_COMMUNICATION_SUBWAY << BATTLEMODE_COMMUNICATION_BIT;
	}
	else
	{
		// ランキング種別は通信対戦
		_pHeader->mode = 0x1234;//BATTLEMODE_COMMUNICATION_COMMUNICATION << BATTLEMODE_COMMUNICATION_BIT;
	}
	
	_pHeader->secure		= 0;		// (TRUE:安全保障, FALSE:未再生)[拒否] 0 - 0
	_pHeader->magic_key		= REC_OCC_MAGIC_KEY;	// マジックキー		[拒否] 0xe281 - 0xe281
	_pHeader->server_vesion	= 100;		// サーババージョン				[拒否] 100 - 100
	_pHeader->data_number	= 0;		// サーバ側でセットされる		[ － ] 
	//_pHeader->work[]			= 0;	// 予備							[ － ] 

	// MATH_CalcCRC16CCITT
	MATH_CRC16CCITTInitTable(&crctable);
	_pHeader->crc.crc16ccitt_hash = MATH_CalcCRC16CCITT(&crctable, _pHeader, sizeof(BATTLE_REC_HEADER) - GDS_CRC_SIZE);
}


// **                 
// ** バトルビデオデータ作成
// ** (本体)
// **                 
void MakeBattleVideoData(BATTLE_REC_WORK* _pData, int _MonsNoInc)
{
    int i;
	MATHCRC16Table crctable;

	// 初期化
	MI_CpuClear8(_pData, sizeof(BATTLE_REC_WORK));

	// バトル画面セットアップパラメータの録画データ用サブセット

	// 乱数の種
	// BTLREC_SETUP_SUBSET 構造体
	_pData->setupSubset.randomContext.x		= 0;	// [無視]
	_pData->setupSubset.randomContext.mul	= 0;	// [無視]
	_pData->setupSubset.randomContext.add	= 0;	// [無視]

	// バトルシチュエーション
	// BTLREC_SETUP_SUBSET :: BTL_FIELD_SITUATION 構造体
	_pData->setupSubset.fieldSituation.bgType	= BATTLE_BG_TYPE_GRASS;			// 戦闘背景						[拒否] 0 - 10
	_pData->setupSubset.fieldSituation.bgAttr	= BATTLE_BG_ATTR_NORMAL_GROUND;	// 戦闘背景指定アトリビュート	[拒否] 0 - 12
	_pData->setupSubset.fieldSituation.weather	= BTL_WEATHER_NONE;				// 天候							[拒否] 0 - 5
	_pData->setupSubset.fieldSituation.season	= 0;							// 四季							[拒否] 0 - 3
	_pData->setupSubset.fieldSituation.zoneID	= (ZONEID)0;					// コメント無し					[無視] - - -
	_pData->setupSubset.fieldSituation.hour		= 0;							// コメント無し					[無視] 0 - 23
	_pData->setupSubset.fieldSituation.minute	= 0;							// コメント無し					[無視] 0 - 59

	// 設定データ
	// BTLREC_SETUP_SUBSET :: CONFIG 構造体
	_pData->setupSubset.config.msg_speed		= 0;// MSG送りの速度						[拒否] 0 - 2
	_pData->setupSubset.config.sound_mode		= 0;// サウンド出力							[拒否] 0 - 1
	_pData->setupSubset.config.battle_rule		= 0;// 戦闘ルール							[拒否] 0 - 1
	_pData->setupSubset.config.wazaeff_mode		= 0;// わざエフェクト						[拒否] 0 - 1
	_pData->setupSubset.config.moji_mode		= 0;// ひらがな／漢字						[拒否] 0 - 1
	_pData->setupSubset.config.wirelesssave_mode= 0;// ワイヤレスでセーブをはさむかどうか	[拒否] 0 - 1
	_pData->setupSubset.config.network_search	= 0;// ゲーム中にビーコンサーチするかどうか	[拒否] 0 - 1

	_pData->setupSubset.musicDefault= 0;	// デフォルトBGM								[拒否] 0 - 65535
	_pData->setupSubset.musicPinch	= 0;	// ピンチ時BGM									[拒否] 0 - 65535
	_pData->setupSubset.debugFlagBit= 0;	// デバッグ機能Bitフラグ -> enum BtlDebugFlag @ battle/battle.h								[拒否] 0 - 0
	_pData->setupSubset.competitor	= 0;	// 対戦者タイプ（ゲーム内トレーナー、通信対戦）-> enum BtlCompetitor @ battle/battle.h		[拒否] 2 - 3
	_pData->setupSubset.myCommPos	= 0;	// 通信対戦時の自分の立ち位置（マルチの時、0,2 vs 1,3 になり、0,1が左側／2,3が右側になる）	[拒否] 0 - 3
	_pData->setupSubset.rule		= 0;	// ルール（シングル・ダブル・トリプル・ローテ）-> enum BtlRule @ battle/battle.h			[拒否] 0 - 3
	_pData->setupSubset.MultiMode	= 0;	// マルチバトルフラグ（ルールは必ずダブル）		[拒否] 0 - 1
	_pData->setupSubset.shooterBit	= 0;

	// クライアント操作内容の保存バッファ
	// BTLREC_OPERATION_BUFFER 構造体
	_pData->opBuffer.size		= 0;		// サイズ：BTLREC_OPERATION_BUFFER_SIZE			[拒否] 0 - 0xc00
	_pData->opBuffer.buffer[0]	= 0;		// バッファ										[無視] 

	// バトル参加プレイヤー毎の情報
	for(i = 0; i < BTL_CLIENT_MAX; i++)
	{
        int j = 0;

		// ポケモンパーティデータ
		// REC_POKEPARTY 構造体
		_pData->rec_party[i].PokeCountMax = 6;	// 保持出来るポケモン数の最大				[拒否] 6 - 6
		_pData->rec_party[i].PokeCount = 1;		// 現在保持しているポケモン数				[拒否] 1 - 6
		
		// ポケモンデータ
		// REC_POKEPARTY :: REC_POKEPARA 構造体												[ － ] 別サーバにてチェック
		for(j = 0; j < TEMOTI_POKEMAX; j++)
		{
			// ポケモンのステータス情報
			_pData->rec_party[i].member[j].monsno = (u16)(_MonsNoInc + j);
		}

		// プレイヤー状態
		// BTLREC_CLIENT_STATUS 構造体
		_pData->clientStatus[i].type = BTLREC_CLIENTSTATUS_PLAYER;	// クライアントタイプ	[拒否] 0 - 2 

		if(_pData->clientStatus[i].type == BTLREC_CLIENTSTATUS_PLAYER)
		{
			// MYSTATUS 構造体
			memcpy(
				&_pData->clientStatus[i].player.name,
				g_TrainerName,
				sizeof(g_TrainerName)
			);													// プレイヤー名
			SetGDS_EOM(_pData->clientStatus[i].player.name, PERSON_NAME_SIZE + EOM_SIZE);

			_pData->clientStatus[i].player.id = 0;				// トレーナーID				[置換] SOAPを利用する
			_pData->clientStatus[i].player.profileID = 0;		// プロファイルID			[無視] 
			_pData->clientStatus[i].player.nation = 0;			// 住んでいる国コード		[無視] 
			_pData->clientStatus[i].player.area = 0;			// 住んでいる地方コード		[無視] 
			_pData->clientStatus[i].player.sex = 0;				// 性別						[拒否] 0 - 1
			_pData->clientStatus[i].player.region_code = 1;		// 言語コード				[拒否] 1 - 8
			_pData->clientStatus[i].player.trainer_view = 0;	// 見た目					[拒否] 0 - 15
			_pData->clientStatus[i].player.rom_code = 20;		// バージョンコード			[拒否] 20, 21	(WHITE=20, BLACK=21)
		}
		else if(_pData->clientStatus[i].type == BTLREC_CLIENTSTATUS_TRAINER)
		{
			// BTLREC_TRAINER_STATUS 構造体
			_pData->clientStatus[i].trainer.ai_bit = 0;					//					[拒否] 0 - 0x3fff	(増減の可能性あり)
			_pData->clientStatus[i].trainer.tr_id = 0;					//					[拒否] 0 - 304		(増減の可能性あり)
			_pData->clientStatus[i].trainer.tr_type = 0;				//					[ － ] 0 - 82		(増減の可能性あり)
			memset(
				&_pData->clientStatus[i].trainer.use_item,
				0,
				BSP_TRAINERDATA_ITEM_MAX * sizeof(u16)
			);															// 使用するアイテム [拒否] 0 - 0

			memcpy(
				&_pData->clientStatus[i].trainer.name,
				g_TrainerName2,
				sizeof(g_TrainerName2)
			);													// プレイヤー名
			SetGDS_EOM(_pData->clientStatus[i].trainer.name, BUFLEN_PERSON_NAME);

			MakePMSData(&_pData->clientStatus[i].trainer.win_word);		// 戦闘終了時勝利メッセージ <-8byte
			MakePMSData(&_pData->clientStatus[i].trainer.lose_word);	// 戦闘終了時負けメッセージ <-8byte
		}
	}

	_pData->magic_key			= 0;	// マジックキー										[拒否] 0xe281 - 0xe281
	//_pData->padding = 0;				// 													[無視]
	
	// MATH_CalcCRC16CCITT
	MATH_CRC16CCITTInitTable(&crctable);
	_pData->crc.crc16ccitt_hash = MATH_CalcCRC16CCITT(&crctable, _pData, sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE);
}


// **                 
// ** 終端文字(0xFFFF)を代入します。
// ** バッファオーバーフローに注意してください。
// ** (必ずバッファの最後の要素には0が入るようにしてください。)
// **                 
void SetGDS_EOM(STRCODE* _pStr, int _Size)
{
	int i = 0;
	for(i = 0; i < _Size; i++)
	{
		if(_pStr[i] == 0)
		{
			_pStr[i] = 0xFFFF;
			break;
		}
	}
}


/** --------------------------------------------------------------------
  シーン処理
  ----------------------------------------------------------------------*/
static int processSceneElement(GameSequence* seq, s16 x, s16 y)
{
    InputSequence* input = &seq->inputList[seq->currentElement];

    if (input->type == TYPE_SELECT) {
        return processMenuSequence(seq, x, y);
    }
    else if (input->type == TYPE_INPUT) {
        //processInput(&input->elements[seq->currentElement], x, y);
        return processInput(seq, x, y);
    }
    else if (input->type == TYPE_NORMAL) {
        return INPUT_CONFIRM;    
    }

    // ここにはこない
    return INPUT_NOW;
}

/** --------------------------------------------------------------------
  入力関連
  ----------------------------------------------------------------------*/
const int MAX_KETA = 12;

/** --------------------------------------------------------------------
  入力処理初期化
  ----------------------------------------------------------------------*/
static void initInput(InputElement* input, u64 min, u64 max, u64 first)
{
#pragma unused(min)
#pragma unused(max)
    MI_CpuClear8(input->buf, sizeof(input->buf));
    input->inputKeta = 0;
    input->inputValue = first;
    snprintf(input->buf, 20, "%012d", first);
}

/** --------------------------------------------------------------------
  入力処理
  ----------------------------------------------------------------------*/
static int processInput(GameSequence* seq, s16 x, s16 y)
{
    InputSequence* inputSeq = &seq->inputList[seq->currentElement];
    InputElement* input = &inputSeq->elements[0];

    // 状態表示
    (void)dispInput(input, x, y);

    // 操作
    if (g_KeyCtrl.trg & PAD_KEY_UP) {
        if (input->buf[input->inputKeta] == '9') {
            input->buf[input->inputKeta] = '0'; 
        } else {
            input->buf[input->inputKeta]++; 
        }
    }
    else if (g_KeyCtrl.trg & PAD_KEY_DOWN) {
        if (input->buf[input->inputKeta] == '0') {
            input->buf[input->inputKeta] = '9'; 
        } else {
            input->buf[input->inputKeta]--; 
        }
    }
    else if (g_KeyCtrl.trg & PAD_KEY_LEFT) {
        if (input->inputKeta == 0) {
            input->inputKeta = MAX_KETA - 1;
        } else {
            input->inputKeta--;
        }
    }
    else if (g_KeyCtrl.trg & PAD_KEY_RIGHT) {
        if (input->inputKeta == (MAX_KETA - 1)) {
            input->inputKeta = 0;
        } else {
            input->inputKeta++;
        }
    }
    if (g_KeyCtrl.trg & PAD_BUTTON_A) {
        input->inputValue = (u64)atoll(input->buf);
        inputSeq->result = input->inputValue;
        return INPUT_CONFIRM;
    }
    else if (g_KeyCtrl.trg & PAD_BUTTON_B) {
        return INPUT_CANCEL;
    }
    return INPUT_NOW; 
}

/** --------------------------------------------------------------------
  入力シーケンス初期化
  ----------------------------------------------------------------------*/
static void initInputSequence(InputSequence* input)
{
    input->currentElement = 0;
    {
        int i;
        for (i = 0; i < MAX_INPUT_ELEMENTS; ++i) {
            initInput(&input->elements[i], 0, 0, 0);
        }
    }
}

/** --------------------------------------------------------------------
  入力シーケンス処理
  ----------------------------------------------------------------------*/
static int processSceneSequence(GameSequence* seq)
{
    InputSequence* input = &seq->inputList[seq->currentElement];
    initInputSequence(input);

    while (1) {
    	int ret;
        s16 x = 0, y = 0;

        drawConsole();
        ReadKeyData();

        ClearScreen();

        y = (s16)(DispTitle(seq->modeName, x, y) + 1);
        ret = processSceneElement(seq, x, y);
	    switch (ret) {
	        case INPUT_CONFIRM:
	            {
                    // TYPE_NORMAL の場合はすぐ終わり
	                if (input->type == TYPE_NORMAL ||
                        seq->currentElement >= (seq->numElements - 1)) {
                        return ret;
	                } else {
	                    // 次の入力へ
	                    seq->currentElement++;
	                }
	            }
	            break;

	        case INPUT_CANCEL:
	            {
	                if (seq->currentElement == 0) {
                        return ret;
	                } else {
	                    // ひとつ戻る
	                    InputElement* elem;
	                    seq->currentElement--;
	                    elem = &input->elements[seq->currentElement];
	                    initInput(elem, 0, 0, elem->inputValue);
	                }
	            }
	            break;

            // INPUT_NOW は処理続行
	    }
    	
        dbs_DemoUpdate();
        OS_WaitIrq(TRUE, OS_IE_V_BLANK);
        Heap_Debug();
        dbs_DemoLoad();
    }

    // ここにはきません
    return INPUT_CONFIRM;
}

/** --------------------------------------------------------------------
  入力状態表示
  ----------------------------------------------------------------------*/
static s16 dispInput(InputElement* input, s16 x, s16 y)
{
    int i = 0;

    // タイトル表示
    y = (s16)(DispSubTitle(input->head, x, y) + 1);

    // 項目表示
    x = 3;
    for (i = 0; i < MAX_KETA; ++i) {
        PrintCharacter(x, y, 0xf, input->buf[i]);
        if (i == input->inputKeta) {
            PrintString(x, (s16)(y+1), 0xf, "^");
        }
        x++;
    }

    return (s16)(y + 2);
}

/** --------------------------------------------------------------------
  入力結果取得
  ----------------------------------------------------------------------*/
static u64 getResultValue(GameSequence* seq, int no)
{
    if (no < 0 || no >= seq->numElements) return -1;

    return (u64)(seq->inputList[no].result);
}




