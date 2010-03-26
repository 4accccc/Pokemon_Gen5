/*---------------------------------------------------------------------------*
  Pokemon Network Library Test Client

  --履歴--
  [2010/03/25] Shinichiro Yamashita
  ・gds_header修正に伴い、サンプルも修正・更新

  [2010/03/17] Shinichiro Yamashita
  ・gds_header修正に伴い、サンプルも修正・更新
  ・crc値を計算するように修正

  [2010/02/15] Shinichiro Yamashita
  small fix

  [2010/02/08] Shinichiro Yamashita
  新規作成
 *---------------------------------------------------------------------------*/


// ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **
// *                                                                  *
// * ※1.「バトルビデオ」のIDについて                                 *
// *  クライアント側にはDB上のIDをエンコードした値を渡し使用する。    *
// *  リクエストとして渡すビデオIDは全てエンコード済み数値を指定する。*
// *                                                                  *
// * ※2.「イベントビデオ」のIDについて                               *
// *  DB上のIDに+900000000000ULLした値。                              *
// *                                                                  *
// * ※3．バトルビデオのアップロード時にランキングテーブルが          *
// *  埋まっていない場合は、その場で挿入される。                      *
// *  (通常、ランキングテーブルの操作は確定処理時に行われる。)        *
// *                                                                  *
// ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **


//====================================
// インクルード
//====================================

//#define ___POKE_NET_BUILD_WINDOWS___
#include "main.h"

#include "../poke_net/poke_net_gds.h"	// POKE_NETライブラリのヘッダ
#include "../poke_net/poke_net_opt.h"	// バトルビデオIDの変換メソッドが宣言してあります。
#include "Nitro/crc.h"	// CRC計算


//====================================
// マクロ定義
//====================================

// サーバのURLまたはIPアドレスとポート番号を指定してください。
#define SYACHI_SERVER_URL	("pkgdstest.nintendo.co.jp")
#define SYACHI_SERVER_PORT	(12401)
//#define SYACHI_SERVER_URL	("192.168.0.133")
//#define SYACHI_SERVER_PORT	(23457)

// BATTLEID_AUTO_ENCODEマクロが有効の場合、BattleIDはDB上の値を指定してください。
#define BATTLEID_AUTO_ENCODE

// この値をPIDとして送信すると、色々なチェックが無視されるので注意！
#define TRAFFIC_CHECK_PID		(9999999)

// Auth構造体に設定する値を定義
//#define	AUTH_PID				(2345678)	// PID
#define	AUTH_PID				(TRAFFIC_CHECK_PID)	// PID
#define AUTH_ROMCODE			(20)		// ROM コード
#define AUTH_LANGCODE			(1)			// 言語コード

// バトルビデオバージョン。新バージョンが出ない限り100。
#define	BATTLEVIDEO_SERVER_VER		(100)

// イベントバトルビデオのIDへ変換する。
#define TO_EVENT_BATTLEVIDEO_ID(x)	(x + 900000000000ULL)


//====================================
// 変数等定義
//====================================

const wchar_t g_TrainerName[]	= L"やまp";									// プロフィールとミュージカルデータとバトル本体に使用
const wchar_t g_TrainerNmae2[]  = L"ミニスカートより愛をこめて叫ぶ";			// バトル本体のトレーナー情報に使用(現ソースでは未使用)
const wchar_t g_MusicalTitle[]	= L"せくしーこまんどぅ。エリーゼのゆううつ";	// ミュージカルタイトル

// *
// * エラーコード⇔メッセージ定義
// *
const char* g_ErrorMessageList[] = {
	"NONE:エラーなし",									// [ 0] POKE_NET_GDS_LASTERROR_NONE
	"NOTINITIALIZED:初期化されていない",				// [ 1] POKE_NET_GDS_LASTERROR_NOTINITIALIZED
	"ILLEGALREQUEST:リクエストが異常",					// [ 2] POKE_NET_GDS_LASTERROR_ILLEGALREQUEST
	"CREATESOCKET:ソケット生成に失敗",					// [ 3] POKE_NET_GDS_LASTERROR_CREATESOCKET
	"IOCTRLSOCKET:ソケットのノンブロッキング化に失敗",	// [ 4] POKE_NET_GDS_LASTERROR_IOCTRLSOCKET
	"NETWORKSETTING:ネットワーク設定失敗",				// [ 5] POKE_NET_GDS_LASTERROR_NETWORKSETTING
	"CREATETHREAD:スレッド生成失敗",					// [ 6] POKE_NET_GDS_LASTERROR_CREATETHREAD
	"CONNECT:接続失敗",									// [ 7] POKE_NET_GDS_LASTERROR_CONNECT
	"SENDREQUEST:リクエストエラー",						// [ 8] POKE_NET_GDS_LASTERROR_SENDREQUEST
	"RECVRESPONSE:レスポンスエラー",					// [ 9] POKE_NET_GDS_LASTERROR_RECVRESPONSE
	"CONNECTTIMEOUT:接続タイムアウト",					// [10] POKE_NET_GDS_LASTERROR_CONNECTTIMEOUT
	"SENDTIMEOUT:送信タイムアウト",						// [11] POKE_NET_GDS_LASTERROR_SENDTIMEOUT
	"RECVTIMEOUT:受信タイムアウト",						// [12] POKE_NET_GDS_LASTERROR_RECVTIMEOUT
	"ABORTED:中断終了"									// [13] POKE_NET_GDS_LASTERROR_ABORTED
};

// *
// * 実行するタスク 列挙
// *
enum EnumAction
{
	ACTION_HELP							= -2,	// ヘルプ
	ACTION_EXIT							= -1,	// 終了

	ACTION_DEBUG_MSG					= 0,	// 接続テスト用メッセージ送信
	ACTION_MUSICAL_UPLOAD				= 1,	// ミュージカルショットアップロード
	ACTION_MUSICAL_DOWNLOAD				= 2,	// ミュージカルショットダウンロード
	ACTION_BATTLEVIDEO_UPLOAD			= 3,	// バトルビデオアップロード
	ACTION_BATTLEVIDEO_SEARCH			= 4,	// バトルビデオ検索
	ACTION_BATTLEVIDEO_RANKING			= 5,	// ランキング検索
	ACTION_BATTLEVIDEO_DOWNLOAD			= 6,	// バトルビデオダウンロード
	ACTION_BATTLEVIDEO_DOWNLOAD_EVENT	= 7,	// イベントバトルビデオダウンロード
	ACTION_BATTLEVIDEO_FAVORITE			= 8,	// バトルビデオお気に入り登録

	ACTION_END_DUMMY
};


//====================================
// プロトタイプ宣言
//====================================

// プロフィール作成
void MakeProfileData(GDS_PROFILE* _pProfile);

// PMSデータ作成
void MakePMSData(PMS_DATA* _pPMS);

// ミュージカルショット作成
void MakeMusicalshotData(MUSICAL_SHOT_DATA *_pMusicalData, int _MonsNoInc = 1);

// バトルビデオデータ作成
void MakeBattleVideoHeader(BATTLE_REC_HEADER *_bhead, BOOL bSubway, int _MonsNoInc = 1);	// ヘッダ
void MakeBattleVideoData(BATTLE_REC_WORK *_brec, int _MonsNoInc = 1);						// 本体

// 終端文字(0xFFFF)を代入します。
// バッファオーバーフローに注意してください。
// (必ずバッファの最後の要素には0が入るようにしてください。)
void SetGDS_EOM(STRCODE* _pStr);


//====================================
// メイン関数
//====================================
void main(int argc ,char *argv[])
{
	const int BufferSize = 1024 * 64;

	// ライブラリに設定するバッファ
	BYTE ReqBuff[BufferSize];
	BYTE ResBuff[BufferSize];

	// 文字列入力用
	char tmpStr[1024] = {0};

	// 多目的に使用される。
	s64 tmpNo = 0;

	// 出力メッセージ
	const char* MessageList[] = {
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

	// ====================================
	// Winsock初期化
	// ====================================
	WSADATA basedata;
	WSAStartup(MAKEWORD(1, 1) ,&basedata);

	// ====================================
	// 認証構造体
	// ====================================
	POKE_NET_REQUESTCOMMON_AUTH ReqAuth = {0};
	ReqAuth.PID			= AUTH_PID;
	ReqAuth.ROMCode		= AUTH_ROMCODE;
	ReqAuth.LangCode	= AUTH_LANGCODE;

	// ====================================
	// プロフィール作成
	// ====================================
	GDS_PROFILE Profile = {0};
	MakeProfileData(&Profile);

	// ====================================
	// 確認表示
	// ====================================
	printf("To => %s:%d\n", SYACHI_SERVER_URL, SYACHI_SERVER_PORT);
	
	// ====================================
	// メインループ
	// ====================================
	long InputReqNo;
	BOOL MainLoopFlag = TRUE;
	while(MainLoopFlag)
	{
		// リクエストの選択
		printf("?Input send request no(-1:exit -2:help) >");

		// 入力を求める
		scanf("%d" ,&InputReqNo);

		// ***************************************************
		// *                                                 *
		// ** exit                                          **
		// *                                                 *
		// ***************************************************
		if(InputReqNo == ACTION_EXIT)
		{
			break;
		}

		// ***************************************************
		// *                                                 *
		// ** help表示                                      **
		// *                                                 *
		// ***************************************************
		else if(InputReqNo == ACTION_HELP)
		{
			printf("%3d:接続テスト用リクエスト\n",				(int)ACTION_DEBUG_MSG					);

			printf("%3d:ミュージカルショットアップロード\n",	(int)ACTION_MUSICAL_UPLOAD				);
			printf("%3d:ミュージカルショットダウンロード\n",	(int)ACTION_MUSICAL_DOWNLOAD			);

			printf("%3d:バトルビデオアップロード\n",			(int)ACTION_BATTLEVIDEO_UPLOAD			);
			printf("%3d:バトルビデオくわしく検索\n",			(int)ACTION_BATTLEVIDEO_SEARCH			);
			printf("%3d:バトルビデオランキング検索\n",			(int)ACTION_BATTLEVIDEO_RANKING			);
			printf("%3d:バトルビデオダウンロード\n",			(int)ACTION_BATTLEVIDEO_DOWNLOAD		);
			printf("%3d:イベントバトルビデオダウンロード\n",	(int)ACTION_BATTLEVIDEO_DOWNLOAD_EVENT	);
			printf("%3d:バトルビデオお気に入り登録\n",			(int)ACTION_BATTLEVIDEO_FAVORITE		);

			printf("\n");

			continue;
		}

		// ====================================
		// ライブラリ初期化
		// ====================================
		if(!POKE_NET_GDS_Initialize(&ReqAuth, SYACHI_SERVER_URL, SYACHI_SERVER_PORT))
		{
			// ライブラリ初期化に失敗
			printf("POKE_NET_GDS_Initialize() Failed!\n");
			continue;
		}

		// ====================================
		// リクエスト登録
		// ====================================
		FillMemory(tmpStr,  sizeof(tmpStr), 0);
		FillMemory(ReqBuff, sizeof(ReqBuff), 0);
		FillMemory(ResBuff, sizeof(ResBuff), 0);


		// ***************************************************
		// *                                                 *
		// ** 接続テスト用リクエスト                        **
		// **                                               **
		// ** 送信したメッセージがそのまま返信されます。    **
		// *                                                 *
		// ***************************************************
		if(InputReqNo == ACTION_DEBUG_MSG)
		{
			printf("[%d]DEBUG_MSG >> Input Message >", ACTION_DEBUG_MSG);
			rewind(stdin);

			fgets(tmpStr, sizeof(tmpStr), stdin);
			if(tmpStr[0] == NULL)
			{
				Sleep(10);
				continue;
			}

			// 末尾の改行外し
			int Length = strlen(tmpStr);
			if(tmpStr[Length - 1] == '\n')
			{
				tmpStr[Length - 1] = '\0';
			}

			// 送信
			POKE_NET_GDS_DebugMessageRequest(tmpStr, ResBuff);
		}

		// ***************************************************
		// *                                                 *
		// ** ミュージカルショットアップロード              **
		// *                                                 *
		// ***************************************************
		else if(InputReqNo == ACTION_MUSICAL_UPLOAD)
		{
			memset(ReqBuff ,0x00 ,sizeof(ReqBuff));
			memset(ResBuff ,0x00 ,sizeof(ResBuff));

			printf("[%d]MUSICAL_UPLOAD >> 作成するポケNoを入力(-1:cancel) >", ACTION_MUSICAL_UPLOAD);

			// 入力された番号=tmpNoとして
			// (tmpNo+0),(tmpNo+1),(tmpNo+2),(tmpNo+3)がポケモン番号となります。
			scanf("%I64d" ,&tmpNo);
			if(tmpNo == -1)
			{
				Sleep(10);
				continue;
			}

			// プロフィールをコピー
			memcpy(&((MUSICAL_SHOT_SEND*)ReqBuff)->profile, &Profile, sizeof(GDS_PROFILE));

			// ミュージカルショットデータを作成
			MakeMusicalshotData(&((MUSICAL_SHOT_SEND*)ReqBuff)->mus_shot, (int)tmpNo);

			// 通信
			POKE_NET_GDS_MusicalShotRegist((MUSICAL_SHOT_SEND*)ReqBuff ,ResBuff);
		}

		// ***************************************************
		// *                                                 *
		// ** ミュージカルショットダウンロード              **
		// *                                                 *
		// ***************************************************
		else if(InputReqNo == ACTION_MUSICAL_DOWNLOAD)
		{
			memset(ReqBuff ,0x00 ,sizeof(ReqBuff));
			memset(ResBuff ,0x00 ,sizeof(ResBuff));

			printf("[%d]MUSICAL_DOWNLOAD >> 検索するポケNoを入力(-1:cancel) >", ACTION_MUSICAL_DOWNLOAD);

			scanf("%I64d" ,&tmpNo);
			if(tmpNo == -1)
			{
				Sleep(10);
				continue;
			}

			POKE_NET_GDS_MusicalShotGet((int)tmpNo ,ResBuff);
		}

		// ***************************************************
		// *                                                 *
		// ** バトルデータアップロード                      **
		// *                                                 *
		// ***************************************************
		else if(InputReqNo == ACTION_BATTLEVIDEO_UPLOAD)
		{
			int tmpNo2 = 0;

			memset(ReqBuff ,0x00 ,sizeof(ReqBuff));
			memset(ResBuff ,0x00 ,sizeof(ResBuff));

			printf("[%d]BATTLEVIDEO_UPLOAD >> 作成するポケNoを入力(-1:cancel) >", ACTION_BATTLEVIDEO_UPLOAD);

			scanf("%I64d" ,&tmpNo);
			if(tmpNo == -1)
			{
				Sleep(10);
				continue;
			}

			// プロフィールをコピー
			memcpy(&((BATTLE_REC_SEND*)ReqBuff)->profile, &Profile, sizeof(GDS_PROFILE));

			printf("[%d]BATTLEVIDEO_UPLOAD >> ランキング種別を入力(0:バトルサブウェイ, Other:通信対戦) >", ACTION_BATTLEVIDEO_UPLOAD);
			scanf("%d" ,&tmpNo2);
			
			// バトルビデオヘッダを作成
			MakeBattleVideoHeader(&((BATTLE_REC_SEND*)ReqBuff)->head, (tmpNo2 == 0), (int)tmpNo);

			// バトルビデオ本体を作成
			MakeBattleVideoData(&((BATTLE_REC_SEND*)ReqBuff)->rec);

			// 通信
			POKE_NET_GDS_BattleDataRegist((BATTLE_REC_SEND*)ReqBuff ,ResBuff);
		}

		// ***************************************************
		// *                                                 *
		// ** バトルデータ検索                              **
		// **                                               **
		// ** SEARCHTYPE_CONDITIONについて･･･               **
		// ** 検索対象テーブルにイベントとランキング領域は  **
		// ** 含まれません。                                **
		// *                                                 *
		// ***************************************************
		else if(InputReqNo == ACTION_BATTLEVIDEO_SEARCH)
		{
			int tmp = 0;

			memset(ReqBuff ,0x00 ,sizeof(ReqBuff));
			memset(ResBuff ,0x00 ,sizeof(ResBuff));

			// 検索条件用の構造体
			BATTLE_REC_SEARCH_SEND Serch = {0};

			// 検索するポケモン番号を入力させる
			printf("[%d]BATTLEVIDEO_SEARCH >> 検索するポケNoを入力(0:No target.) >", ACTION_BATTLEVIDEO_SEARCH);

			scanf("%d" ,&(Serch.monsno));
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
			printf("[%d]BATTLEVIDEO_SEARCH >> 検索するランキング種別を入力(0:No target, 1:バトルサブウェイ, Other:通信対戦) >", ACTION_BATTLEVIDEO_SEARCH);
			scanf("%d" ,&tmp);
			if(tmp == 0)
			{
				Serch.battle_mode = 0;
				Serch.battle_mode_bit_mask = 0;
			}
			else if(tmp == 1)
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
			POKE_NET_GDS_BattleDataSearchCondition(&Serch, ResBuff);
		}

		// ***************************************************
		// *                                                 *
		// ** バトルデータランキング検索                    **
		// **                                               **
		// ** 全て、内部で以下のリクエストを利用します。    **
		// ** POKE_NET_GDS_REQCODE_BATTLEDATA_SEARCH        **
		// *                                                 *
		// ***************************************************
		else if(InputReqNo == ACTION_BATTLEVIDEO_RANKING)
		{
			memset(ReqBuff ,0x00 ,sizeof(ReqBuff));
			memset(ResBuff ,0x00 ,sizeof(ResBuff));

			// 検索条件用の構造体
			BATTLE_REC_RANKING_SEARCH_SEND Serch = {0};

			// 対象ランキングを選択
			printf("[%d]BATTLEVIDEO_RANKING >> 検索するランキングを入力(0:Latest30, 1:Subway, Other:Comm) >", ACTION_BATTLEVIDEO_RANKING);

			scanf("%I64d" ,&tmpNo);
			if(tmpNo == 0)
			{
				// 最新30件
				POKE_NET_GDS_BattleDataSearchRankingLatest(BATTLEVIDEO_SERVER_VER, ResBuff);
			}
			else if(tmpNo == 1)
			{
				// バトルサブウェイランキング
				POKE_NET_GDS_BattleDataSearchRankingSubway(BATTLEVIDEO_SERVER_VER, ResBuff);
			}
			else
			{
				// 通信対戦ランキング
				POKE_NET_GDS_BattleDataSearchRankingComm(BATTLEVIDEO_SERVER_VER, ResBuff);
			}
		}

		// ***************************************************
		// *                                                 *
		// ** バトルデータダウンロード                      **
		// **                                               **
		// ** 下のACTION_BATTLEVIDEO_DOWNLOAD_EVENTとの     **
		// ** 違いは、ID＋900000000000ULLの有無だけです。   **
		// *                                                 *
		// ***************************************************
		else if(InputReqNo == ACTION_BATTLEVIDEO_DOWNLOAD)
		{
			memset(ReqBuff ,0x00 ,sizeof(ReqBuff));
			memset(ResBuff ,0x00 ,sizeof(ResBuff));

			printf("[%d]BATTLEVIDEO_DOWNLOAD >> Input BattleVideoID(-1:cancel) >", ACTION_BATTLEVIDEO_DOWNLOAD);

			scanf("%I64d" ,&tmpNo);
			if(tmpNo == -1)
			{
				Sleep(10);
				continue;
			}

#ifdef BATTLEID_AUTO_ENCODE
			// 暗号化
			tmpNo = POKE_NET_EncodePlayDataID(tmpNo);
#endif

			// 通信
			POKE_NET_GDS_BattleDataGet((u64)tmpNo, BATTLEVIDEO_SERVER_VER, ResBuff);
		}

		// ***************************************************
		// *                                                 *
		// ** イベントバトルデータダウンロード              **
		// **                                               **
		// ** 上のACTION_BATTLEVIDEO_DOWNLOADとの違いは、   **
		// ** ID＋900000000000ULLの有無だけです。           **
		// *                                                 *
		// ***************************************************
		else if(InputReqNo == ACTION_BATTLEVIDEO_DOWNLOAD_EVENT)
		{
			memset(ReqBuff ,0x00 ,sizeof(ReqBuff));
			memset(ResBuff ,0x00 ,sizeof(ResBuff));

			printf("[%d]MUSICAL_DOWNLOAD >> Input EventBattleVideoID(-1:cancel) >", ACTION_BATTLEVIDEO_DOWNLOAD_EVENT);
			scanf("%I64d" ,&tmpNo);

			if(tmpNo == -1)
			{
				Sleep(10);
				continue;
			}

			// イベントビデオ用にID変換
			tmpNo = TO_EVENT_BATTLEVIDEO_ID(tmpNo);

#ifdef BATTLEID_AUTO_ENCODE
			// 暗号化
			tmpNo = POKE_NET_EncodePlayDataID(tmpNo);
#endif

			// 通信
			POKE_NET_GDS_BattleDataGet((u64)tmpNo, BATTLEVIDEO_SERVER_VER, ResBuff);
		}

		// ***************************************************
		// *                                                 *
		// ** バトルデータお気に入り登録                    **
		// *                                                 *
		// ***************************************************
		else if(InputReqNo == ACTION_BATTLEVIDEO_FAVORITE)
		{
			memset(ReqBuff ,0x00 ,sizeof(ReqBuff));
			memset(ResBuff ,0x00 ,sizeof(ResBuff));

			printf("[%d]MUSICAL_DOWNLOAD >> Input BattleVideoID(-1:cancel) >", ACTION_BATTLEVIDEO_FAVORITE);

			scanf("%I64d" ,&tmpNo);
			if(tmpNo == -1)
			{
				Sleep(10);
				continue;
			}

#ifdef BATTLEID_AUTO_ENCODE
			// 暗号化
			tmpNo = POKE_NET_EncodePlayDataID(tmpNo);
#endif

			// 通信
			POKE_NET_GDS_BattleDataFavorite((u64)tmpNo, ResBuff);
		}

		// ***************************************************
		// *                                                 *
		// ** 上記以外の値                                  **
		// *                                                 *
		// ***************************************************
		else
		{
			// ====================================
			// 解放してスタートに戻る
			// ====================================
			POKE_NET_GDS_Release();

			printf("不明な値です。\n");
			Sleep(10);
			continue;
		}

		// ====================================
		// ステータス取得／表示ループ
		// ====================================
		int LastState = POKE_NET_GDS_GetStatus();
		int State = 0;
		BOOL LoopFlag = TRUE;
		while(LoopFlag)
		{
			State = POKE_NET_GDS_GetStatus();
			if(LastState == State)
			{
				// ステータスに変化は無い
				Sleep(10);
				continue;
			}

			// 
			// ▼ 以降はステータスに変化があった場合 ▼
			//

			// 現在のステータス表示
			printf(MessageList[State]);

			if(State == POKE_NET_GDS_STATUS_INACTIVE
				|| State == POKE_NET_GDS_STATUS_INITIALIZED
				|| State == POKE_NET_GDS_STATUS_ABORTED
				|| State == POKE_NET_GDS_STATUS_FINISHED
				|| State == POKE_NET_GDS_STATUS_ERROR
				)
			{
				LoopFlag = FALSE;
			}

			// ***************************************************
			// *                                                 *
			// ** 通信が完了した。                              **
			// *                                                 *
			// ***************************************************
			if(State == POKE_NET_GDS_STATUS_FINISHED)
			{
				// レスポンス表示(DEBUG)
				POKE_NET_GDS_DEBUG_PrintResponse((POKE_NET_RESPONSE*)POKE_NET_GDS_GetResponse(), POKE_NET_GDS_GetResponseSize());
			}

			// ***************************************************
			// *                                                 *
			// ** 通信中にエラーが起きた！                      **
			// *                                                 *
			// ***************************************************
			else if(State == POKE_NET_GDS_STATUS_ERROR)
			{
				// エラー情報表示
				printf("lasterrorcode:%d\n", POKE_NET_GDS_GetLastErrorCode());
				printf("LastErrorMsg:%s\n", g_ErrorMessageList[POKE_NET_GDS_GetLastErrorCode()]);
			}

			LastState = State;
			Sleep(10);
		}

		// ====================================
		// 解放
		// ====================================
		POKE_NET_GDS_Release();
	}

	// ====================================
	// Winsock終了
	// ====================================
	WSACleanup();
}


// **
// ** プロフィール作成
// **                 
void MakeProfileData(GDS_PROFILE* _pProfile)
{
	FillMemory(_pProfile, sizeof(GDS_PROFILE), 0);

	// 7文字まで(8文字目はNULL)
	wcscpy((wchar_t*)_pProfile->name, g_TrainerName);	// プレイヤー名称		[置換] SOAPを利用する
	//_pProfile->name[wcslen(g_TrainerName)] = 0xFFFF;
	SetGDS_EOM(_pProfile->name);

	_pProfile->player_id = 0;				// プレイヤーID			[ － ] 0 - 0xffffffff
	_pProfile->player_sex = 0;				// プレイヤー性別		[拒否] 0 - 1
	_pProfile->birthday_month = 1;			// 誕生月				[拒否] 1 - 12
	_pProfile->trainer_view = 0;			// プレイヤーの見た目	[拒否] 0 - 15

	_pProfile->country_code = 0;			// 住んでいる国コード	[無視] 0 - 233
	_pProfile->local_code = 0;				// 住んでいる地方コード	[無視] * - *

	_pProfile->version_code = AUTH_ROMCODE;	// バージョンコード		[拒否] 20, 21	(WHITE=20, BLACK=21)
	_pProfile->language = AUTH_LANGCODE;	// 言語コード			[拒否] 1 - 8

	_pProfile->egg_flag = 0;				// タマゴフラグ			[無視] 0 - 1
	_pProfile->form_no = 0;					// フォルムNo			[無視] * - *
	_pProfile->mons_sex = 0;				// モンスター性別		[無視] 0 - 1
	_pProfile->monsno = 1;					// モンスターNo			[拒否] 1 - 可変

	// MESSAGE_FLAG_NORMAL(簡易会話) / MESSAGE_FLAG_EVENT(フリーワード)
	_pProfile->message_flag = MESSAGE_FLAG_NORMAL;	//				[拒否] 0 - 0

	// 簡易会話設定
	MakePMSData(&_pProfile->self_introduction);

	// MATH_CalcCRC16CCITT											[拒否] 
	MATHCRC16Table crctable;
	MATH_CRC16CCITTInitTable(&crctable);
	_pProfile->crc.crc16ccitt_hash = MATH_CalcCRC16CCITT(&crctable, _pProfile, sizeof(GDS_PROFILE) - GDS_CRC_SIZE);
}


// **                 
// ** PMSデータ作成
// **                 
void MakePMSData(PMS_DATA* _pPMS)
{
	_pPMS->sentence_type = 0;	// 文章タイプ				[拒否] 0 - 4
	_pPMS->sentence_id = 0;		// タイプ内ID				[拒否] 0 - 19
	_pPMS->word[0] = 0;			// 単語ID					[拒否] 0 - 1735
	_pPMS->word[1] = 0;
}


// **                 
// ** ミュージカルショット作成
// **                 
void MakeMusicalshotData(MUSICAL_SHOT_DATA *_pMusicalData, int _MonsNoInc)
{
	FillMemory(_pMusicalData, sizeof(MUSICAL_SHOT_DATA), 0);

	//ミュージカルタイトル(日本18・海外36＋EOM							[拒否] 題目は固定
	wcscpy((wchar_t*)_pMusicalData->title, g_MusicalTitle);
	//_pMusicalData->title[wcslen(g_MusicalTitle)] = 0xFFFF;
	SetGDS_EOM(_pMusicalData->title);
	
	_pMusicalData->bgNo		= 0;			// 背景番号					[拒否] 0 - 4
	_pMusicalData->spotBit	= 0;			// スポットライト対象(bit)(トップだったポケモン)
											//							[拒否] 0 - 15
	_pMusicalData->year		= 0;			// 年						[拒否] 0 - 99
	_pMusicalData->month	= 1;			// 月						[拒否] 1 - 12
	_pMusicalData->day		= 1;			// 日						[拒否] 1 - 31
	_pMusicalData->player	= 0;			// 自分の番号				[拒否] 0 - 3
	_pMusicalData->musVer	= 0;			// ミュージカルバージョン	[拒否] 0 - 0
	_pMusicalData->pmVersion= AUTH_ROMCODE;	// PM_VERSION				[拒否] 20, 21	(WHITE=20, BLACK=21)
	_pMusicalData->pmLang	= AUTH_LANGCODE;// PM_LANG					[拒否] 1 - 8

	for(int i = 0;i < MUSICAL_POKE_MAX; i++)
	{
		_pMusicalData->shotPoke[i].monsno	= _MonsNoInc + i;	// ポケモン番号	[ － ] 別サーバにてチェック
		_pMusicalData->shotPoke[i].sex		= 0;				// 性別			[ － ]
		_pMusicalData->shotPoke[i].rare		= 0;				// レアフラグ	[ － ]
		_pMusicalData->shotPoke[i].form		= 0;				// フォルム番号	[ － ]

		// トレーナー名															[ － ]
		// 7文字まで(8文字目はNULL)
		wcscpy((wchar_t*)_pMusicalData->shotPoke[i].trainerName, g_TrainerName);
		//_pMusicalData->shotPoke[i].trainerName[wcslen(g_TrainerName)] = 0xFFFF;
		SetGDS_EOM(_pMusicalData->shotPoke[i].trainerName);

		// 装備グッズ
		// 最大８つまで装備可能(ここでは１つしか設定しない)
		_pMusicalData->shotPoke[i].equip[0].itemNo	= 0;		// グッズ番号	[拒否] 0 - 99, 65535	(65535はデータ無しの場合)
		_pMusicalData->shotPoke[i].equip[0].angle	= i * 10;	// 角度			[無視] -32767 - 32768
		_pMusicalData->shotPoke[i].equip[0].equipPos= 0;		// 装備箇所		[拒否] 0 - 8, 10		(10はデータ無しの場合)
	}
}


// **                 
// ** バトルビデオデータ作成
// ** (ヘッダ)
// **                 
void MakeBattleVideoHeader(BATTLE_REC_HEADER* _pHeader, BOOL bSubway, int _MonsNoInc)
{
	// 初期化
	FillMemory(_pHeader, sizeof(BATTLE_REC_HEADER), 0);

	for(int i = 0; i < HEADER_MONSNO_MAX; i++)
	{
		_pHeader->monsno[i] = _MonsNoInc + i;	// モンスターNo			[拒否] 1 - 可変
		_pHeader->form_no_and_sex[i] = 0;		// ポケモンフォルム番号	[無視]
	}
	
	_pHeader->battle_counter = 0;				// 連勝数				[拒否] 0 - 9999

	// 戦闘モード														[拒否] 十数種類の固定値
	if(bSubway)
	{
		// ランキング種別はバトルサブウェイ
		_pHeader->mode = BATTLEMODE_COMMUNICATION_SUBWAY << BATTLEMODE_COMMUNICATION_BIT;
		//_pHeader->mode = 0x1234;
	}
	else
	{
		// ランキング種別は通信対戦
		_pHeader->mode = BATTLEMODE_COMMUNICATION_COMMUNICATION << BATTLEMODE_COMMUNICATION_BIT;
		//_pHeader->mode = 0x1234;
	}
	
	_pHeader->secure		= 0;		// (TRUE:安全保障, FALSE:未再生)[拒否] 0 - 0
	_pHeader->magic_key		= REC_OCC_MAGIC_KEY;	// マジックキー		[拒否] 0xe281 - 0xe281
	_pHeader->server_vesion	= 100;		// サーババージョン				[拒否] 100 - 100
	_pHeader->data_number	= 0;		// サーバ側でセットされる		[ － ] 
	//_pHeader->work[]		= 0;		// 予備							[ － ] 

	// MATH_CalcCRC16CCITT												[拒否] 
	MATHCRC16Table crctable;
	MATH_CRC16CCITTInitTable(&crctable);
	_pHeader->crc.crc16ccitt_hash = MATH_CalcCRC16CCITT(&crctable, _pHeader, sizeof(BATTLE_REC_HEADER) - GDS_CRC_SIZE);
}


// **                 
// ** バトルビデオデータ作成
// ** (本体)
// **                 
void MakeBattleVideoData(BATTLE_REC_WORK* _pData, int _MonsNoInc)
{
	// 初期化
	FillMemory(_pData, sizeof(BATTLE_REC_WORK), 0);

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
	_pData->setupSubset.fieldSituation.weather	= BTL_WEATHER_NONE;				// 天候							[拒否] 0 - 4
	_pData->setupSubset.fieldSituation.season	= 0;							// 四季							[拒否] 0 - 3
	_pData->setupSubset.fieldSituation.zoneID	= (ZONEID)0;					// コメント無し					[無視] - - -
	_pData->setupSubset.fieldSituation.hour		= 23;							// コメント無し					[無視] 0 - 23
	_pData->setupSubset.fieldSituation.minute	= 59;							// コメント無し					[無視] 0 - 59

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
	_pData->setupSubset.competitor	= 2;	// 対戦者タイプ（ゲーム内トレーナー、通信対戦）-> enum BtlCompetitor @ battle/battle.h		[拒否] 2 - 3
	_pData->setupSubset.myCommPos	= 0;	// 通信対戦時の自分の立ち位置（マルチの時、0,2 vs 1,3 になり、0,1が左側／2,3が右側になる）	[拒否] 0 - 3
	_pData->setupSubset.rule		= 0;	// ルール（シングル・ダブル・トリプル・ローテ）-> enum BtlRule @ battle/battle.h			[拒否] 0 - 3
	_pData->setupSubset.fMultiMode	= 0;	// マルチバトルフラグ（ルールは必ずダブル）		[拒否] 0 - 1

	// クライアント操作内容の保存バッファ
	// BTLREC_OPERATION_BUFFER 構造体
	_pData->opBuffer.size = 0;		// サイズ：BTLREC_OPERATION_BUFFER_SIZE					[拒否] 0 - 0xc00
	memset(_pData->opBuffer.buffer, 0, BTLREC_OPERATION_BUFFER_SIZE);		// バッファ		[無視] 

	// バトル参加プレイヤー毎の情報
	for(int i = 0; i < BTL_CLIENT_MAX; i++)
	{
		// ポケモンパーティデータ
		// REC_POKEPARTY 構造体
		_pData->rec_party[i].PokeCountMax = 6;	// 保持出来るポケモン数の最大				[拒否] 6 - 6
		_pData->rec_party[i].PokeCount = 1;		// 現在保持しているポケモン数				[拒否] 1 - 6
		
		// ポケモンデータ
		// REC_POKEPARTY :: REC_POKEPARA 構造体												[ － ] 別サーバにてチェック
		for(int j = 0; j < TEMOTI_POKEMAX; j++)
		{
			// ポケモンのステータス情報
			_pData->rec_party[i].member[j].monsno = _MonsNoInc + j;
		}

		// プレイヤー状態
		// BTLREC_CLIENT_STATUS 構造体
		_pData->clientStatus[i].type = BTLREC_CLIENTSTATUS_PLAYER;	// クライアントタイプ	[拒否] 0 - 2 

		if(_pData->clientStatus[i].type == BTLREC_CLIENTSTATUS_PLAYER)
		{
			// MYSTATUS 構造体
			wcscpy((wchar_t*)_pData->clientStatus[i].player.name, g_TrainerName);	// プレイヤー名	[置換] SOAPを利用する
			//_pData->clientStatus[i].player.name[wcslen(g_TrainerName)] = 0xFFFF;
			SetGDS_EOM(_pData->clientStatus[i].player.name);

			_pData->clientStatus[i].player.id = 0;				// トレーナーID				[無視] 
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
			wcscpy(
				(wchar_t*)_pData->clientStatus[i].trainer.name,
				g_TrainerNmae2
			);															// トレーナー名		[置換] SOAPを利用する
			//_pData->clientStatus[i].trainer.name[wcslen(g_TrainerNmae2)] = 0xFFFF;
			SetGDS_EOM(_pData->clientStatus[i].trainer.name);

			MakePMSData(&_pData->clientStatus[i].trainer.win_word);		// 戦闘終了時勝利メッセージ <-8byte
			MakePMSData(&_pData->clientStatus[i].trainer.lose_word);	// 戦闘終了時負けメッセージ <-8byte
		}
	}

	_pData->magic_key			= REC_OCC_MAGIC_KEY;	// マジックキー						[拒否] 0xe281 - 0xe281
	//_pData->padding = 0;				// 													[無視]
	
	// MATH_CalcCRC16CCITT																	[拒否] 
	MATHCRC16Table crctable;
	MATH_CRC16CCITTInitTable(&crctable);
	_pData->crc.crc16ccitt_hash = MATH_CalcCRC16CCITT(&crctable, _pData, sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE);
}


// **                 
// ** 終端文字(0xFFFF)を代入します。
// ** バッファオーバーフローに注意してください。
// ** (必ずバッファの最後の要素には0が入るようにしてください。)
// **                 
void SetGDS_EOM(STRCODE* _pStr)
{
	_pStr[wcslen((wchar_t*)_pStr)] = 0xFFFF;
}


