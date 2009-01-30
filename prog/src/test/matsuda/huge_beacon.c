//==============================================================================
/**
 * @file	huge_beacon.c
 * @brief	ビーコンを使用して巨大データを送受信
 * @author	matsuda
 * @date	2009.01.23(金)
 */
//==============================================================================
#include <gflib.h>
#include "huge_beacon.h"


//==============================================================================
//	定数定義
//==============================================================================
///ビーコンで一度に送るユーザーデータサイズ
#define HB_SEND_DATA_SIZE		(64)

///子機モードでいられる時間
#define HB_CHILD_MODE_TIME		(60 * 2)
///子機モードでロックオンしている親機からのタイムアウト時間
#define HB_ROCKON_TIMEOUT		(60 * 9)

///親機モードでいられる時間
#define HB_PARENT_MODE_TIME		(60 * 6)
///親機モードで同じデータを何回送信したら次のデータへ切り替えるか
#define HB_PARENT_SEND_LOOP		(10)

//--------------------------------------------------------------
//	デバッグ定義
//--------------------------------------------------------------
///チャンネルを固定してビーコンを配信する
#define CHANNEL_KOTEI		(1)	//１＝固定
///親機を見つけた後はScanExではなく、Scanでサーチする
#define SCAN_USE			(0)	//1=Scanを使う
///親子を固定するモード
#define PARENTCHILD_KOTEI	(0)	//1=固定

//==============================================================================
//	構造体定義
//==============================================================================
///送信データのヘッダ
typedef struct{
	u16 data_no;		///<データ番号
	u8 size;			///<送信されてきたユーザーデータサイズ(ヘッダは含まない)
	u8 padding;
}HB_USER_HEADER;

///送信データ
typedef struct{
	HB_USER_HEADER head;
	u8 data[HB_SEND_DATA_SIZE];
}HB_SEND_DATA;

///子機モード時の動作用ワーク
typedef struct{
	BOOL parent_lockon;			///<TRUE:一台の親機にロックオンしている(受信専用モードで動く)
	BOOL parent_hit;			///<TRUE:親機検索でヒットした
	BOOL receive_finish;		///<TRUE:全ての受信を完了
	u8 parent_macAddress[6];	///<親のMACアドレス
	u8 parent_link_level;		///<ロックオンしている親の電波レベル
	u8 padding;
	u16 errcode;
	WMStartScanExCallback wsec;
	WMStartScanCallback wsc;
	WMScanExParam scan_ex_param;
	WMScanParam scan_param;
}HB_CHILD_MODE;

///親機モード時の動作用ワーク
typedef struct{
	WMMeasureChannelCallback mcc;
	WMParentParam ppara;
	WMStartParentCallback wpc;
	u16 errcode;
	u16 send_loop;
	s16 add_data_count;
	u16 padding;
}HB_PARENT_MODE;

///巨大ビーコン送受信管理ワーク
typedef struct{
	int heap_id;
	u16 seq;
	u16 tgid;

	HB_CHILD_MODE cm;
	HB_PARENT_MODE pm;
	
	BOOL scan_callback;
	int timeout;
	u8 scan_buf[WM_SIZE_SCAN_EX_BUF] ATTRIBUTE_ALIGN(32);
	HB_SEND_DATA beacon_send_data;
	
	u32 *receive_bit;			///<受信したデータ番号をbit管理
	const u8 *send_data;		///<送信データへのポインタ
	u8 *receive_buf;			///<受信データ代入先ワークへのポインタ
	u32 send_data_size;			///<送信データサイズ
	u16 data_no_max;			///<分割データ数
	u16 receive_data_count;		///<受信したデータの数
	
	u16 parent_channel;
	u16 use_channel_list;
	u16 use_channel_level;
	u16 use_channel;

#if PARENTCHILD_KOTEI
	BOOL pc_kotei;		///<TRUE:親で固定　FALSE:子で固定
#endif
}HUGEBEACON_SYS;


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static int _HUGEBEACON_Wait(HUGEBEACON_SYS *hb);
static int _HUGEBEACON_ChildStartInit(HUGEBEACON_SYS *hb);
static int _HUGEBEACON_ChildStart(HUGEBEACON_SYS *hb);
static int _HUGEBEACON_ChildStartWait(HUGEBEACON_SYS *hb);
static int _HUGEBEACON_ChildEnd(HUGEBEACON_SYS *hb);
static int _HUGEBEACON_ChildEndWait(HUGEBEACON_SYS *hb);
static int _HUGEBEACON_ParentStartInit(HUGEBEACON_SYS *hb);
static int _HUGEBEACON_ParentStartInitWait(HUGEBEACON_SYS *hb);
static int _HUGEBEACON_ParentStart(HUGEBEACON_SYS *hb);
static int _HUGEBEACON_ParentStartWait(HUGEBEACON_SYS *hb);
static int _HUGEBEACON_ParentEnd(HUGEBEACON_SYS *hb);
static int _HUGEBEACON_ParentEndWait(HUGEBEACON_SYS *hb);
static int _HUGEBEACON_ParentSendChange(HUGEBEACON_SYS *hb);
static int _HUGEBEACON_ParentSendChangeWait(HUGEBEACON_SYS *hb);
static int _HUGEBEACON_UseChannelSelectInit(HUGEBEACON_SYS *hb);
static int _HUGEBEACON_UseChannelSelectMain(HUGEBEACON_SYS *hb);
static int _HUGEBEACON_UseChannelSelectWait(HUGEBEACON_SYS *hb);
static void _Callback_ChildScan( void* arg );
static void _Callback_ChildEndScan(void *arg);
static void _Callback_MeasureChannel(void *arg);
static void _Callback_ParentParam(void *arg);
static void _Callback_StartParent(void *arg);
static void _Callback_EndParent(void *arg);
static void _Callback_ParentSendChange(void *arg);
static void _HB_TOOL_SendDataMake(HUGEBEACON_SYS *hb);
static void _HB_TOOL_ReceiveDataSet(HUGEBEACON_SYS *hb, const void *receive_data);


//==============================================================================
//	シーケンステーブル
//==============================================================================
///巨大ビーコン送受信用シーケンステーブルの関数型
typedef int (*HUGEBEACON_FUNC)(HUGEBEACON_SYS *);

///巨大ビーコン送受信：シーケンステーブル
static const HUGEBEACON_FUNC HugeBeaconSeqTbl[] = {
	_HUGEBEACON_Wait,
	_HUGEBEACON_ChildStartInit,
	_HUGEBEACON_ChildStart,
	_HUGEBEACON_ChildStartWait,
	_HUGEBEACON_ChildEnd,
	_HUGEBEACON_ChildEndWait,
	_HUGEBEACON_UseChannelSelectInit,
	_HUGEBEACON_UseChannelSelectMain,
	_HUGEBEACON_UseChannelSelectWait,
	_HUGEBEACON_ParentStartInit,
	_HUGEBEACON_ParentStartInitWait,
	_HUGEBEACON_ParentStart,
	_HUGEBEACON_ParentStartWait,
	_HUGEBEACON_ParentEnd,
	_HUGEBEACON_ParentEndWait,
	_HUGEBEACON_ParentSendChange,
	_HUGEBEACON_ParentSendChangeWait,
};

//HugeBeaconSeqTblと並びを同じにしておくこと！
enum{
	HBSEQ_WAIT,
	HBSEQ_CHILD_START_INIT,
	HBSEQ_CHILD_START,
	HBSEQ_CHILD_START_WAIT,
	HBSEQ_CHILD_END,
	HBSEQ_CHILD_END_WAIT,
	HBSEQ_USE_CHANNEL_SELECT_INIT,
	HBSEQ_USE_CHANNEL_SELECT_MAIN,
	HBSEQ_USE_CHANNEL_SELECT_WAIT,
	HBSEQ_PARENT_START_INIT,
	HBSEQ_PARENT_START_INIT_WAIT,
	HBSEQ_PARENT_START,
	HBSEQ_PARENT_START_WAIT,
	HBSEQ_PARENT_END,
	HBSEQ_PARENT_END_WAIT,
	HBSEQ_PARENT_SEND_CHANGE,
	HBSEQ_PARENT_SEND_CHANGE_WAIT,
	
	HBSEQ_CONTINUE = -1,
	HBSEQ_END = -2,
	HBSEQ_ERROR = -3,
};


//==============================================================================
//	ローカル変数
//==============================================================================
///コールバック関数内で使用する為、グローバル化
static HUGEBEACON_SYS *HbSys = NULL;


//==============================================================================
//
//	
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   巨大ビーコン送受信システムを作成
 *
 * @param   heap_id				ヒープID
 * @param   send_data_size		送信するデータのサイズ
 * @param   send_data			送信するデータへのポインタ
 * @param   receive_buf			受信データを代入するバッファへのポインタ
 *
 * @retval  巨大ビーコン送受信システムワークへのポインタ
 */
//--------------------------------------------------------------
void HUGEBEACON_SystemCreate(int heap_id, u32 send_data_size, const void *send_data, void *receive_buf)
{
	u32 bit_num;		//必要な管理bit数
	u32 bit_array;		//管理bitのワーク数
	
	GF_ASSERT(HbSys == NULL);
	
	bit_num = send_data_size / HB_SEND_DATA_SIZE;
	if(send_data_size % HB_SEND_DATA_SIZE){
		bit_num++;
	}
	bit_array = bit_num / sizeof(u32);
	if(bit_num % sizeof(u32)){
		bit_array++;
	}
	
	HbSys = GFL_HEAP_AllocClearMemory(heap_id, sizeof(HUGEBEACON_SYS));
	HbSys->heap_id = heap_id;
	HbSys->receive_bit = GFL_HEAP_AllocClearMemory(heap_id, bit_array * sizeof(u32));
	HbSys->send_data_size = send_data_size;
	HbSys->send_data = send_data;
	HbSys->receive_buf = receive_buf;
	HbSys->data_no_max = bit_num;
	HbSys->seq = HBSEQ_WAIT;

#if PARENTCHILD_KOTEI
	if(GFL_UI_KEY_GetCont() & PAD_BUTTON_L){
		OS_TPrintf("親子：固定モード：ずっと親\n");
		HbSys->pc_kotei = TRUE;
	}
	else{
		OS_TPrintf("親子：固定モード：ずっと子\n");
		HbSys->pc_kotei = FALSE;
	}
#endif
}

//--------------------------------------------------------------
/**
 * @brief   巨大ビーコン送受信システムを終了
 */
//--------------------------------------------------------------
void HUGEBEACON_SystemExit(void)
{
	GF_ASSERT(HbSys != NULL);
	
	GFL_HEAP_FreeMemory(HbSys->receive_bit);
	GFL_HEAP_FreeMemory(HbSys);
	HbSys = NULL;
}

//--------------------------------------------------------------
/**
 * @brief   巨大ビーコン送受信システムの動作開始
 */
//--------------------------------------------------------------
void HUGEBEACON_Start(void)
{
	GF_ASSERT(HbSys != NULL);
	HbSys->seq = HBSEQ_CHILD_START_INIT;
#if PARENTCHILD_KOTEI
	if(HbSys->pc_kotei == TRUE){
		HbSys->seq = HBSEQ_PARENT_START_INIT;
	}
#endif
}

//--------------------------------------------------------------
/**
 * @brief   巨大ビーコン送受信システムのメイン
 */
//--------------------------------------------------------------
void HUGEBEACON_Main(void)
{
	HUGEBEACON_SYS *hb = HbSys;
	int ret;
	
	GF_ASSERT(HbSys != NULL);
	if(hb->seq == HBSEQ_END || hb->seq == HBSEQ_ERROR){
		return;
	}
	
	ret = HugeBeaconSeqTbl[hb->seq](hb);
	switch(ret){
	case HBSEQ_CONTINUE:
		break;
	case HBSEQ_ERROR:
		GF_ASSERT(0);	//とりあえず今はアサートで停止 ※check
		break;	//エラー処理へ流す
	default:
		hb->seq = ret;
		break;
	}
}


//==============================================================================
//	シーケンス
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   動作シーケンス：待機
 * @param   hb		
 * @retval  次のシーケンス番号
 */
//--------------------------------------------------------------
static int _HUGEBEACON_Wait(HUGEBEACON_SYS *hb)
{
	return HBSEQ_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   動作シーケンス：子機として動作開始(ワークの初期化)
 * @param   hb		
 * @retval  次のシーケンス番号
 */
//--------------------------------------------------------------
static int _HUGEBEACON_ChildStartInit(HUGEBEACON_SYS *hb)
{
	hb->scan_callback = 0;
	hb->cm.parent_link_level = 0;
	hb->cm.parent_lockon = 0;
	hb->cm.parent_hit = 0;
	hb->cm.receive_finish = 0;
	hb->timeout = 0;
	return HBSEQ_CHILD_START;
}

//--------------------------------------------------------------
/**
 * @brief   動作シーケンス：子機として動作開始
 * @param   hb		
 * @retval  次のシーケンス番号
 */
//--------------------------------------------------------------
static int _HUGEBEACON_ChildStart(HUGEBEACON_SYS *hb)
{
	WMErrCode err_code;
	u16 channel_list;
	int i;
	
	channel_list = WM_GetAllowedChannel();
	if(channel_list == 0){
		OS_TPrintf("無線通信してはいけない\n");
		return HBSEQ_ERROR;
	}
	
	GF_ASSERT(WM_SIZE_SCAN_EX_BUF == sizeof(hb->scan_buf));	//デバッグ用チェック　後で消す

	hb->scan_callback = 0;
	
#if SCAN_USE
	if(hb->cm.parent_lockon == FALSE){
#else
	if(1){
#endif
		GFL_STD_MemClear(&hb->cm.scan_ex_param, sizeof(WMScanExParam));
		hb->cm.scan_ex_param.scanBuf = (void*)hb->scan_buf;
		hb->cm.scan_ex_param.scanBufSize = sizeof(hb->scan_buf);
		hb->cm.scan_ex_param.maxChannelTime = WM_GetDispersionScanPeriod();
		if(hb->cm.parent_lockon == FALSE){
			hb->cm.scan_ex_param.channelList = channel_list;
			for(i = 0; i < 6; i++){
				hb->cm.scan_ex_param.bssid[i] = 0xff;
			}
		}
		else{
		#if CHANNEL_KOTEI
			GF_ASSERT(hb->parent_channel != 0);
			hb->cm.scan_ex_param.channelList = 1 << (hb->parent_channel - 1);
		#else	//親子を切り替えて通信していると親のチャンネルも変わっていくので
			hb->cm.scan_ex_param.channelList = channel_list;
		#endif
			OS_TPrintf("子機：親機指定で探しに行く：");
			for(i = 0; i < 6; i++){
				hb->cm.scan_ex_param.bssid[i] = hb->cm.parent_macAddress[i];
				OS_TPrintf("%02x:", hb->cm.scan_ex_param.bssid[i]);
			}
			OS_TPrintf("\n");
		}
		hb->cm.scan_ex_param.scanType = WM_SCANTYPE_PASSIVE;

		err_code = WM_StartScanEx(_Callback_ChildScan, &hb->cm.scan_ex_param );
		if(err_code != WM_ERRCODE_OPERATING){
			OS_TPrintf("StartScanExに失敗　エラー:%d\n", err_code);
			return HBSEQ_ERROR;
		}
	}
	else{
		GFL_STD_MemClear(&hb->cm.scan_param, sizeof(WMScanParam));
		hb->cm.scan_param.scanBuf = (void*)hb->scan_buf;
		hb->cm.scan_param.maxChannelTime = WM_GetDispersionScanPeriod();
		hb->cm.scan_param.channel = hb->parent_channel;
		if(hb->cm.parent_lockon == FALSE){
			for(i = 0; i < 6; i++){
				hb->cm.scan_param.bssid[i] = 0xff;
			}
		}
		else{
			GF_ASSERT(hb->parent_channel != 0);
			OS_TPrintf("子機：親機指定で探しに行く：");
			for(i = 0; i < 6; i++){
				hb->cm.scan_param.bssid[i] = hb->cm.parent_macAddress[i];
				OS_TPrintf("%02x:", hb->cm.scan_param.bssid[i]);
			}
			OS_TPrintf("\n");
		}

		err_code = WM_StartScan(_Callback_ChildScan, &hb->cm.scan_param );
		if(err_code != WM_ERRCODE_OPERATING){
			OS_TPrintf("StartScanに失敗　エラー:%d\n", err_code);
			return HBSEQ_ERROR;
		}
	}
	
	return HBSEQ_CHILD_START_WAIT;
}

//--------------------------------------------------------------
/**
 * @brief   動作シーケンス：子機動作コールバック待ち
 * @param   hb		
 * @retval  次のシーケンス番号
 */
//--------------------------------------------------------------
static int _HUGEBEACON_ChildStartWait(HUGEBEACON_SYS *hb)
{
	int i;
	BOOL before_lockon;
	u16 state, errcode;
	
	hb->timeout++;
	if(hb->scan_callback == 0){
		return HBSEQ_CONTINUE;	//コールバックが発生するまで待つ
	}

	hb->cm.parent_hit = 0;
	before_lockon = hb->cm.parent_lockon;
	
#if SCAN_USE
	if(before_lockon == FALSE){
#else
	if(1){
#endif
		state = hb->cm.wsec.state;
		errcode = hb->cm.wsec.errcode;
	}
	else{
		state = hb->cm.wsc.state;
		errcode = hb->cm.wsc.errcode;
	}
	
	if(state == WM_STATECODE_PARENT_FOUND){
		//一台以上の親機が見つかった
		switch(errcode){
		case WM_ERRCODE_SUCCESS:
		#if SCAN_USE
			if(before_lockon == FALSE){
		#else
			if(1){
		#endif
				OS_TPrintf("子機：発見した親機の数 = %d\n", hb->cm.wsec.bssDescCount);
				if ( hb->cm.wsec.bssDescCount > 0 ){
					// BssDescの情報がARM7側から書き込まれているため
					// バッファに設定されたBssDescのキャッシュを破棄
					DC_InvalidateRange(hb->scan_buf, sizeof(hb->scan_buf));
				}

				for ( i = 0; i < hb->cm.wsec.bssDescCount; i++ ){
					WMBssDesc* bd = hb->cm.wsec.bssDesc[i];
					
					OS_TPrintf("length = %d, WMBssDescのサイズ=%d\n", bd->length, sizeof(WMBssDesc));	//※check
					//GF_ASSERT(sizeof(WMBssDesc) == bd->length);
					
					OS_TPrintf(" Ch%2d %02x:%02x:%02x:%02x:%02x:%02x ",
							   bd->channel,
							   bd->bssid[0], bd->bssid[1], bd->bssid[2],
							   bd->bssid[3], bd->bssid[4], bd->bssid[5]);

					// まず、WMBssDesc.gameInfoLength を確認し、
					// ggid に有効な値が入っていることから調べる必要があります。
					if (WM_IsValidGameInfo(&bd->gameInfo, bd->gameInfoLength)){
						// DS 親機
						OS_TPrintf("GGID=%08x TGID=%04x\n", bd->gameInfo.ggid, bd->gameInfo.tgid);
						if (bd->gameInfo.ggid != GGID_HUGE_DATA){
							// GGIDが違っていれば無視する
							OS_TPrintf("GGIDが違う\n");
							continue;
						}
					}
					else{
						OS_TPrintf("有効でない親機 %d\n", i);
						continue;
					}

					// 見つけたビーコンの情報を記録
					if(before_lockon == TRUE){
						//一応ちゃんとMacAddressが一致しているか確認
						for(i = 0; i < 6; i++){
							if(hb->cm.parent_macAddress[i] != bd->bssid[i]){
								break;
							}
						}
						if(i < 6){
							OS_TPrintf("ロックオンしていたMacAddressと不一致\n");
							continue;
						}
						
						hb->cm.parent_hit = TRUE;
						//ユーザーワークを読み取る
						_HB_TOOL_ReceiveDataSet(hb, bd->gameInfo.userGameInfo);
					}
					else{
						//電波状況が良いのを優先
						if(hb->cm.parent_link_level <= hb->cm.wsec.linkLevel[i]){
							OS_TPrintf("子機：接続先の親機情報更新\n");
							hb->cm.parent_link_level = hb->cm.wsec.linkLevel[i];
							hb->cm.parent_lockon = TRUE;
							hb->cm.parent_hit = TRUE;
							for(i = 0; i < 6; i++){
								hb->cm.parent_macAddress[i] = bd->bssid[i];
							}
							hb->parent_channel = bd->channel;
							//ユーザーワークを読み取る
							_HB_TOOL_ReceiveDataSet(hb, bd->gameInfo.userGameInfo);
						}
					}
				}
			}
			else{
				WMStartScanCallback *wsc = &hb->cm.wsc;
				
				OS_TPrintf("子機：ScanBufで発見\n");

				// BssDescの情報がARM7側から書き込まれているため
				// バッファに設定されたBssDescのキャッシュを破棄
				DC_InvalidateRange(hb->scan_buf, sizeof(hb->scan_buf));

				OS_TPrintf(" Ch%2d %02x:%02x:%02x:%02x:%02x:%02x ",
						   wsc->channel,
						   wsc->macAddress[0], wsc->macAddress[1], wsc->macAddress[2],
						   wsc->macAddress[3], wsc->macAddress[4], wsc->macAddress[5]);

				// まず、WMBssDesc.gameInfoLength を確認し、
				// ggid に有効な値が入っていることから調べる必要があります。
				if (WM_IsValidGameInfo(&wsc->gameInfo, wsc->gameInfoLength)){
					// DS 親機
					OS_TPrintf("GGID=%08x TGID=%04x\n", wsc->gameInfo.ggid, wsc->gameInfo.tgid);
					if (wsc->gameInfo.ggid != GGID_HUGE_DATA){
						// GGIDが違っていれば無視する
						OS_TPrintf("GGIDが違う\n");
						break;
					}
				}
				else{
					OS_TPrintf("有効でない親機\n");
					break;
				}

				//一応ちゃんとMacAddressが一致しているか確認
				for(i = 0; i < 6; i++){
					if(hb->cm.parent_macAddress[i] != wsc->macAddress[i]){
						break;
					}
				}
				if(i < 6){
					OS_TPrintf("ロックオンしていたMacAddressと不一致\n");
					break;
				}
				
				hb->cm.parent_hit = TRUE;
				//ユーザーワークを読み取る
				_HB_TOOL_ReceiveDataSet(hb, wsc->gameInfo.userGameInfo);
			}
			break;
		default:	//一応(マニュアル上では上記エラーコード以外は発生しない)
			break;
		}
	}
	else{	//WM_STATECODE_PARENT_NOT_FOUND
		//親機が見つからなかった
		switch(errcode){
		case WM_ERRCODE_SUCCESS:
		case WM_ERRCODE_FAILED:
		case WM_ERRCODE_ILLEGAL_STATE:
		case WM_ERRCODE_INVALID_PARAM:
		case WM_ERRCODE_FIFO_ERROR:
		default:	//一応(マニュアル上では上記エラーコード以外は発生しない)
			OS_TPrintf("子機：親機が見つからない\n");
			break;
		}
	}

	
	if(hb->cm.parent_hit == TRUE){	//親機が見つかっている
		hb->timeout = 0;
		if(hb->cm.receive_finish == TRUE){
			return HBSEQ_CHILD_END;
		}
	}
	else{
		int max_timeout;
	
	#if PARENTCHILD_KOTEI
		hb->timeout = 0;	//固定モードの為、タイムアウトさせない
	#endif
		max_timeout = hb->cm.parent_lockon ? HB_ROCKON_TIMEOUT : HB_CHILD_MODE_TIME;

		if(hb->timeout > max_timeout){
			hb->timeout = 0;
			return HBSEQ_CHILD_END;
		}
	}

	return HBSEQ_CHILD_START;
}

//--------------------------------------------------------------
/**
 * @brief   動作シーケンス：子機動作終了
 * @param   hb		
 * @retval  次のシーケンス番号
 */
//--------------------------------------------------------------
static int _HUGEBEACON_ChildEnd(HUGEBEACON_SYS *hb)
{
	WMErrCode err_code;
	
	hb->scan_callback = 0;
	err_code = WM_EndScan(_Callback_ChildEndScan);
	if(err_code != WM_ERRCODE_OPERATING){
		OS_TPrintf("EndScanに失敗　エラー:%d\n", err_code);
		return HBSEQ_CONTINUE;
	}
	
	hb->scan_callback = 0;
	return HBSEQ_CHILD_END_WAIT;
}

//--------------------------------------------------------------
/**
 * @brief   動作シーケンス：子機動作終了のコールバック待ち
 * @param   hb		
 * @retval  次のシーケンス番号
 */
//--------------------------------------------------------------
static int _HUGEBEACON_ChildEndWait(HUGEBEACON_SYS *hb)
{
	if(hb->scan_callback == 0){
		return HBSEQ_CONTINUE;
	}
	
	switch(hb->cm.errcode){
	case WM_ERRCODE_SUCCESS:
		if(hb->cm.receive_finish == TRUE){
			OS_TPrintf("巨大ビーコン：終了\n");
			return HBSEQ_END;
		}
		OS_TPrintf("子機モード終了\n");
		hb->timeout = 0;
		return HBSEQ_PARENT_START_INIT;
	case WM_ERRCODE_FAILED:
	case WM_ERRCODE_ILLEGAL_STATE:
	case WM_ERRCODE_FIFO_ERROR:
	default:
		OS_TPrintf("Child EndScanCb failed: %d\n", hb->cm.errcode);
		return HBSEQ_CHILD_END;
	}
}

//--------------------------------------------------------------
/**
 * @brief   動作シーケンス：親機として動作する際の使用チャンネル検索
 * @param   hb		
 * @retval  次のシーケンス番号
 */
//--------------------------------------------------------------
static int _HUGEBEACON_UseChannelSelectInit(HUGEBEACON_SYS *hb)
{
	u16 channel_list;
	int channel_no;

	channel_list = WM_GetAllowedChannel();
	if(channel_list == 0){
		OS_TPrintf("無線通信してはいけない\n");
		return HBSEQ_ERROR;
	}
	
	OS_TPrintf("使用出来るチャンネル = %x\n", channel_list);
	hb->use_channel_list = channel_list;
	hb->use_channel_level = 101;	//初回はどれでもセットされるように範囲外を設定
	return HBSEQ_USE_CHANNEL_SELECT_MAIN;
}

//--------------------------------------------------------------
/**
 * @brief   動作シーケンス：親機として動作する際の使用チャンネル検索：メイン
 * @param   hb		
 * @retval  次のシーケンス番号
 */
//--------------------------------------------------------------
static int _HUGEBEACON_UseChannelSelectMain(HUGEBEACON_SYS *hb)
{
#define WH_MEASURE_TIME         30     // 1フレームに一回通信している電波を拾えるだけの間隔(ms)
#define WH_MEASURE_CS_OR_ED     3      // キャリアセンスとED値の論理和
#define WH_MEASURE_ED_THRESHOLD 17     // 実験データによる経験的に有効と思われるお勧めED閾値

	WMErrCode errcode;
	int i, channel_no;
	
	hb->scan_callback = 0;
	
	channel_no = 1;
	for(i = 0; i < 14; i++){
		if(hb->use_channel_list & (1 << i)){
			hb->use_channel_list ^= (1 << i);
			break;
		}
		channel_no++;
	}
	
	/*
	 * 電波使用率取得パラメータとして、
	 * 実験による経験的に有効と思われる値を入れています。
	 */
	errcode = WM_MeasureChannel(_Callback_MeasureChannel,	   /* コールバック設定 */
					 WH_MEASURE_CS_OR_ED,		/* CS or ED */
					 WH_MEASURE_ED_THRESHOLD,	/* 第2引数がキャリアセンスのみの場合は無効 */
					 channel_no,  /* 今回の検索チャンネル */
					 WH_MEASURE_TIME);	/*１チャンネルあたりの調査時間[ms] */
	switch(errcode){
	case WM_ERRCODE_OPERATING:
		return HBSEQ_USE_CHANNEL_SELECT_WAIT;
	case WM_ERRCODE_ILLEGAL_STATE:
	case WM_ERRCODE_FIFO_ERROR:
	default:
		return HBSEQ_ERROR;
	}
}

//--------------------------------------------------------------
/**
 * @brief   動作シーケンス：親機として動作する際の使用チャンネル検索のコールバック待ち
 * @param   hb		
 * @retval  次のシーケンス番号
 */
//--------------------------------------------------------------
static int _HUGEBEACON_UseChannelSelectWait(HUGEBEACON_SYS *hb)
{
	WMMeasureChannelCallback *mcc = &hb->pm.mcc;
	
	if(hb->scan_callback == 0){
		return HBSEQ_CONTINUE;
	}
	
	switch(mcc->errcode){
	case WM_ERRCODE_SUCCESS:
		OS_TPrintf("WM_MeasureChannelコールバック正常\n");
		break;
	case WM_ERRCODE_FAILED:
	case WM_ERRCODE_ILLEGAL_STATE:
	case WM_ERRCODE_FIFO_ERROR:
	default:
		OS_TPrintf("WM_MeasureChannelのコールバックでエラー発生 : %d\n", mcc->errcode);
		return HBSEQ_ERROR;
	}
	
	OS_TPrintf("ヒットしたチャンネル = %d, 電波使用率=%d\n", mcc->channel, mcc->ccaBusyRatio);
	if(mcc->ccaBusyRatio <= hb->use_channel_level){
		hb->use_channel_level = mcc->ccaBusyRatio;
		hb->use_channel = mcc->channel;
		OS_TPrintf("上記チャンネルで更新されました\n");
	}
	
	if(hb->use_channel_list == 0){
		return HBSEQ_PARENT_START_INIT;
	}
	return HBSEQ_USE_CHANNEL_SELECT_MAIN;
}

//--------------------------------------------------------------
/**
 * @brief   動作シーケンス：親機として動作開始する前の基本情報設定
 * @param   hb		
 * @retval  次のシーケンス番号
 */
//--------------------------------------------------------------
static int _HUGEBEACON_ParentStartInit(HUGEBEACON_SYS *hb)
{
	WMErrCode err_code;
	u16 channel_list;
	
	hb->scan_callback = 0;

	if(hb->use_channel == 0){	//まだ使用チャンネルが決まっていない
		return HBSEQ_USE_CHANNEL_SELECT_INIT;
	}
	
	channel_list = WM_GetAllowedChannel();
	if(channel_list == 0){
		OS_TPrintf("無線通信してはいけない\n");
		return HBSEQ_ERROR;
	}
	OS_TPrintf("使用出来るチャンネル = 0x%x\n", channel_list);
	
	if(hb->pm.send_loop == 0){
		//送信データ作成
		hb->pm.add_data_count = 1;
		_HB_TOOL_SendDataMake(hb);
	}
	hb->tgid = WM_GetNextTgid();
	
	//親機情報の設定
	GFL_STD_MemClear(&hb->pm.ppara, sizeof(WMParentParam));
	hb->pm.ppara.userGameInfo = (u16*)(&hb->beacon_send_data);
	hb->pm.ppara.userGameInfoLength = sizeof(HB_SEND_DATA);
	hb->pm.ppara.ggid = GGID_HUGE_DATA;
	hb->pm.ppara.tgid = hb->tgid;
	hb->pm.ppara.entryFlag = 0;
	hb->pm.ppara.maxEntry = 0;
	hb->pm.ppara.multiBootFlag = 0;
	hb->pm.ppara.KS_Flag = 0;
	hb->pm.ppara.CS_Flag = 0;
	hb->pm.ppara.beaconPeriod = WM_GetDispersionBeaconPeriod();
	hb->pm.ppara.channel = hb->use_channel;
	hb->pm.ppara.parentMaxSize = 4;	//使用しないので少なく
	hb->pm.ppara.childMaxSize = 4;		//使用しないので少なく
	err_code = WM_SetParentParameter( _Callback_ParentParam, &hb->pm.ppara);

	if(err_code != WM_ERRCODE_OPERATING){
		OS_TPrintf("WM_SetParentParameterに失敗　エラー:%d\n", err_code);
		return HBSEQ_ERROR;
	}
	OS_TPrintf("ParentStartInit正常\n");
	return HBSEQ_PARENT_START_INIT_WAIT;
}

//--------------------------------------------------------------
/**
 * @brief   動作シーケンス：親機の基本情報設定のコールバック待ち
 * @param   hb		
 * @retval  次のシーケンス番号
 */
//--------------------------------------------------------------
static int _HUGEBEACON_ParentStartInitWait(HUGEBEACON_SYS *hb)
{
	if(HbSys->scan_callback == FALSE){
		return HBSEQ_CONTINUE;
	}
	
	switch(hb->pm.errcode){
	case WM_ERRCODE_SUCCESS:
		OS_TPrintf("ParentStartInitWait正常\n");
		return HBSEQ_PARENT_START;
	case WM_ERRCODE_FAILED:
	case WM_ERRCODE_INVALID_PARAM:
	case WM_ERRCODE_FIFO_ERROR:
	default:
		OS_TPrintf("Parent EndScanCb failed: %d\n", hb->pm.errcode);
		return HBSEQ_PARENT_START_INIT;
	}
}

//--------------------------------------------------------------
/**
 * @brief   動作シーケンス：親機として動作開始
 * @param   hb		
 * @retval  次のシーケンス番号
 */
//--------------------------------------------------------------
static int _HUGEBEACON_ParentStart(HUGEBEACON_SYS *hb)
{
	WMErrCode err_code;
	u16 channel_list;
	
	channel_list = WM_GetAllowedChannel();
	if(channel_list == 0){
		OS_TPrintf("無線通信してはいけない\n");
		return HBSEQ_ERROR;
	}
	
	hb->scan_callback = 0;
#if 1
	err_code = WM_StartParent(_Callback_StartParent);

	if(err_code != WM_ERRCODE_OPERATING){
		OS_TPrintf("WM_StartParentに失敗　エラー:%d\n", err_code);
		return HBSEQ_ERROR;
	}
#else
{
#define WH_MEASURE_TIME         30     // 1フレームに一回通信している電波を拾えるだけの間隔(ms)
#define WH_MEASURE_CS_OR_ED     3      // キャリアセンスとED値の論理和
#define WH_MEASURE_ED_THRESHOLD 17     // 実験データによる経験的に有効と思われるお勧めED閾値

    /*
     * 電波使用率取得パラメータとして、
     * 実験による経験的に有効と思われる値を入れています。
     */
    return WM_MeasureChannel(NULL,     /* コールバック設定 */
                             WH_MEASURE_CS_OR_ED,       /* CS or ED */
                             WH_MEASURE_ED_THRESHOLD,   /* 第2引数がキャリアセンスのみの場合は無効 */
                             channel_list,  /* 今回の検索チャンネル */
                             WH_MEASURE_TIME);  /*１チャンネルあたりの調査時間[ms] */
}
#endif
	OS_TPrintf("WM_StartParent正常\n");
	return HBSEQ_PARENT_START_WAIT;
}

//--------------------------------------------------------------
/**
 * @brief   動作シーケンス：親機動作のコールバック待ち
 * @param   hb		
 * @retval  次のシーケンス番号
 */
//--------------------------------------------------------------
static int _HUGEBEACON_ParentStartWait(HUGEBEACON_SYS *hb)
{
	hb->timeout++;
	if(hb->scan_callback == 0){
		return HBSEQ_CONTINUE;	//コールバックが発生するまで待つ
	}
	
#if 0	//どのstateであってもSUCCESSしか戻ってこない
	switch(hb->pm.wpc.state){
	case WM_STATECODE_PARENT_START:
		OS_TPrintf("PARENT CALLBACK STATE : START\n");
		break;
	case WM_STATECODE_BEACON_SENT:
		OS_TPrintf("PARENT CALLBACK STATE : BEACON SET\n");
		break;
	case WM_STATECODE_CONNECTED:
		OS_TPrintf("PARENT CALLBACK STATE : CONNECTED\n");
		break;
	case WM_STATECODE_DISCONNECTED:
		OS_TPrintf("PARENT CALLBACK STATE : DISCONNECTED\n");
		break;
	case WM_STATECODE_DISCONNECTED_FROM_MYSELF:
		OS_TPrintf("PARENT CALLBACK STATE : DISCONNECTED_FROM_MYSELF\n");
		break;
	default:
		OS_TPrintf("PARENT CALLBACK STATE : default 通常ありえない\n");
		break;
	}
	return HBSEQ_CONTINUE;
#endif

#if PARENTCHILD_KOTEI
	hb->timeout = 0;	//親子固定モードの為、タイムアウトさせない
#endif

	hb->pm.send_loop++;
	if(hb->pm.send_loop >= HB_PARENT_SEND_LOOP){
		hb->pm.send_loop = 0;
		if(hb->timeout > HB_PARENT_MODE_TIME){
			hb->timeout = 0;
			OS_TPrintf("親機終了\n");
			return HBSEQ_PARENT_END;
		}
		return HBSEQ_PARENT_SEND_CHANGE;
	}
//	return HBSEQ_PARENT_START;
	return HBSEQ_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   動作シーケンス：親機動作終了
 * @param   hb		
 * @retval  次のシーケンス番号
 */
//--------------------------------------------------------------
static int _HUGEBEACON_ParentEnd(HUGEBEACON_SYS *hb)
{
	WMErrCode err_code;
	s32 data_no;
	
	//親子切り替え字にデータが沢山欠損する為、次の送信時に少し前のデータナンバーから
	//送信するようにする
	data_no = hb->beacon_send_data.head.data_no;
	data_no -= hb->pm.add_data_count / 2;
	if(data_no < 0){
		hb->beacon_send_data.head.data_no = hb->data_no_max + data_no;
	}
	else{
		hb->beacon_send_data.head.data_no = data_no;
	}
	
	hb->scan_callback = 0;
	err_code = WM_EndParent(_Callback_EndParent);
	switch(err_code){
	case WM_ERRCODE_OPERATING:
		return HBSEQ_PARENT_END_WAIT;
	case WM_ERRCODE_ILLEGAL_STATE:
	case WM_ERRCODE_FIFO_ERROR:
	default:
		OS_TPrintf("WM_EndParentに失敗　エラー:%d\n", err_code);
	#if 0
		return HBSEQ_ERROR;
	#else
		return HBSEQ_CHILD_START_INIT;
	#endif
	}
}

//--------------------------------------------------------------
/**
 * @brief   動作シーケンス：親機動作終了のコールバック待ち
 * @param   hb		
 * @retval  次のシーケンス番号
 */
//--------------------------------------------------------------
static int _HUGEBEACON_ParentEndWait(HUGEBEACON_SYS *hb)
{
	if(hb->scan_callback == 0){
		return HBSEQ_CONTINUE;
	}
	
	switch(hb->pm.errcode){
	case WM_ERRCODE_SUCCESS:
	#if CHANNEL_KOTEI
		;
	#else
		hb->use_channel = 0;	//親機になるたびにチャンネルを検索させるため、0クリア
	#endif
		OS_TPrintf("親機終了\n");
		return HBSEQ_CHILD_START_INIT;
	case WM_ERRCODE_FAILED:
	case WM_ERRCODE_ILLEGAL_STATE:
	case WM_ERRCODE_FIFO_ERROR:
	default:
		OS_TPrintf("ParentEndのコールバックでエラーが発生しました errcode=%d\n", hb->pm.errcode);
		return HBSEQ_PARENT_END;
	}
}

//--------------------------------------------------------------
/**
 * @brief   動作シーケンス：親機の送信データの内容を変更する
 * @param   hb		
 * @retval  次のシーケンス番号
 */
//--------------------------------------------------------------
static int _HUGEBEACON_ParentSendChange(HUGEBEACON_SYS *hb)
{
	WMErrCode err_code;

//	return HBSEQ_PARENT_START_INIT;
	
	//送信データ作成
	hb->pm.add_data_count++;
	_HB_TOOL_SendDataMake(hb);
	hb->tgid = WM_GetNextTgid();
	
	hb->scan_callback = 0;
	err_code = WM_SetGameInfo(_Callback_ParentSendChange, 
		(u16*)(&hb->beacon_send_data), sizeof(HB_SEND_DATA), GGID_HUGE_DATA, hb->tgid, 0);

	switch(err_code){
	case WM_ERRCODE_OPERATING:
		OS_TPrintf("WM_SetGameInfoに成功\n");
		return HBSEQ_PARENT_SEND_CHANGE_WAIT;
	case WM_ERRCODE_ILLEGAL_STATE:
	case WM_ERRCODE_INVALID_PARAM:
	case WM_ERRCODE_FIFO_ERROR:
	default:
		OS_TPrintf("WM_SetGameInfoに失敗　エラー:%d\n", err_code);
//		return HBSEQ_ERROR;
		return HBSEQ_CONTINUE;
	}
}

//--------------------------------------------------------------
/**
 * @brief   動作シーケンス：親機の送信データ変更のコールバック待ち
 * @param   hb		
 * @retval  次のシーケンス番号
 */
//--------------------------------------------------------------
static int _HUGEBEACON_ParentSendChangeWait(HUGEBEACON_SYS *hb)
{
	if(hb->scan_callback == 0){
		return HBSEQ_CONTINUE;
	}
	
	switch(hb->pm.errcode){
	case WM_ERRCODE_SUCCESS:
//		return HBSEQ_PARENT_START;
		return HBSEQ_PARENT_START_WAIT;
	case WM_ERRCODE_FAILED:
	case WM_ERRCODE_FIFO_ERROR:
	default:
		return HBSEQ_PARENT_SEND_CHANGE;
	}
}


//==============================================================================
//	コールバック関数
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   WM_StartScanEx関数コールバック
 * @param   arg		
 */
//--------------------------------------------------------------
static void _Callback_ChildScan( void* arg )
{
	WMStartScanExCallback *ex = arg;
	WMStartScanCallback *scan = arg;

	HbSys->scan_callback = TRUE;
#if SCAN_USE
	if(HbSys->cm.parent_lockon == FALSE){
#else
	if(1){
#endif
		HbSys->cm.wsec = *ex;
	}
	else{
		HbSys->cm.wsc = *scan;
	}
}

//--------------------------------------------------------------
/**
 * @brief   WM_EndScanコールバック関数
 * @param   arg		
 */
//--------------------------------------------------------------
static void _Callback_ChildEndScan(void *arg)
{
	WMCallback *cb = arg;
	
	HbSys->cm.errcode = cb->errcode;
	HbSys->scan_callback = TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   WM_MeasureChannelコールバック関数
 * @param   arg		
 */
//--------------------------------------------------------------
static void _Callback_MeasureChannel(void *arg)
{
	WMMeasureChannelCallback *mcc = arg;

	HbSys->pm.mcc = *mcc;
	HbSys->scan_callback = TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   WM_SetParentParameterコールバック関数
 * @param   arg		
 */
//--------------------------------------------------------------
static void _Callback_ParentParam(void *arg)
{
	WMCallback *cb = arg;

	HbSys->pm.errcode = cb->errcode;
	HbSys->scan_callback = TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   WM_StartParentコールバック関数
 * @param   arg		
 */
//--------------------------------------------------------------
static void _Callback_StartParent(void *arg)
{
	WMStartParentCallback *wpc = arg;
	
	HbSys->pm.wpc = *wpc;
	HbSys->scan_callback = TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   WM_EndParentコールバック関数
 * @param   arg		
 */
//--------------------------------------------------------------
static void _Callback_EndParent(void *arg)
{
	WMCallback *cb = arg;
	
	HbSys->pm.errcode = cb->errcode;
	HbSys->scan_callback = TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   WM_SetGameInfoコールバック関数
 * @param   arg		
 */
//--------------------------------------------------------------
static void _Callback_ParentSendChange(void *arg)
{
	WMCallback *cb = arg;
	
	HbSys->pm.errcode = cb->errcode;
	HbSys->scan_callback = TRUE;
}


//==============================================================================
//	ツール
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   送信データを作成する
 * @param   hb		
 */
//--------------------------------------------------------------
static void _HB_TOOL_SendDataMake(HUGEBEACON_SYS *hb)
{
	HB_SEND_DATA *hsd = &hb->beacon_send_data;
	
	hsd->head.data_no++;
	if(hsd->head.data_no >= hb->data_no_max){
		hsd->head.data_no = 0;
	}
	
	OS_TPrintf("送信データ作成　data_no = %d\n", hsd->head.data_no);
	if(hb->data_no_max <= hsd->head.data_no+1){
		//必ずしもbeacon_send_dataはHB_SEND_DATA_SIZEの倍数ではないので、最後のデータだけ端数が出る
		hsd->head.size = hb->send_data_size % HB_SEND_DATA_SIZE;
	}
	else{
		hsd->head.size = HB_SEND_DATA_SIZE;
	}
	
	GFL_STD_MemCopy(
		&hb->send_data[HB_SEND_DATA_SIZE * hsd->head.data_no], hsd->data, hsd->head.size);
}

//--------------------------------------------------------------
/**
 * @brief   受信データを受信バッファへセット
 *
 * @param   hb					
 * @param   receive_data		受信データへのポインタ
 */
//--------------------------------------------------------------
static void _HB_TOOL_ReceiveDataSet(HUGEBEACON_SYS *hb, const void *receive_data)
{
	const HB_SEND_DATA *rcd = receive_data;
	u32 array_no, bit_no;
	
	if(rcd->head.data_no >= HbSys->data_no_max){
		OS_TPrintf("データ受信：data_no_maxを超えたデータナンバーが受信された\n");
		GF_ASSERT(0);
		return;
	}
	
	array_no = rcd->head.data_no / sizeof(u32);
	bit_no = rcd->head.data_no % sizeof(u32);
	if(HbSys->receive_bit[array_no] & (1 << bit_no)){
		OS_TPrintf("データ受信：既に受信済み data_no = %d\n", rcd->head.data_no);
		return;
	}
	OS_TPrintf("データ受信： data_no = %d", rcd->head.data_no);
	
	GFL_STD_MemCopy(
		rcd->data, &HbSys->receive_buf[HB_SEND_DATA_SIZE * rcd->head.data_no], rcd->head.size);
	HbSys->receive_bit[array_no] |= 1 << bit_no;
	HbSys->receive_data_count++;
	if(HbSys->receive_data_count >= HbSys->data_no_max){
		HbSys->cm.receive_finish = TRUE;
		OS_TPrintf("データ受信：全ての受信を完了\n");
	}
	OS_TPrintf("残りデータ数=%d, 現在までの受信データ状況 %d\%\n", HbSys->data_no_max - HbSys->receive_data_count, HbSys->receive_data_count * 100 / HbSys->data_no_max);
}
