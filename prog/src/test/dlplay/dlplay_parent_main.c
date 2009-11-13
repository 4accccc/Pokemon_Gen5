//============================================================================================
/**
 * @file	dlplay_parent_main.c
 * @brief	ダウンロードプレイ 子機配信部分
 * @author	ariizumi
 * @date	2008.10.8
 */
//============================================================================================
#include "gflib.h"
#include "procsys.h"
#include "ui.h"
#include "system/main.h"
#include "arc_def.h"
#include "multiboot/comm/mbp.h"

#include "test/ariizumi/ari_debug.h"
#include "dlplay_parent_sample.h"
#include "dlplay_parent_main.h"
#include "dlplay_func.h"

/* このデモで使用する GGID */
#define WH_GGID				 (0x3FFF21)
 
/* このデモがダウンロードさせるプログラム情報 */
const MBGameRegistry mbGameList = {
	"/dl_rom/child.srl",					  // 子機バイナリコード
	(u16 *)L"MultiBootTest",		   // ゲーム名
	(u16 *)L"マルチブートテスト by Ariizumi",		// ゲーム内容説明
	"/dl_rom/icon.char",				 // アイコンキャラクタデータ
	"/dl_rom/icon.plt",				  // アイコンパレットデータ
	WH_GGID,						   // GGID
	MBP_CHILD_MAX + 1,				 // 最大プレイ人数、親機の数も含めた人数
};


//============================================================================================
//	struct
//============================================================================================
struct _DLPLAY_SEND_DATA
{
	u16 subSeq_;
	
	DLPLAY_MSG_SYS *msgSys_;
};

//======================================================================
//	enum
//======================================================================

//============================================================================================
//	proto
//============================================================================================
DLPLAY_SEND_DATA* DLPlaySend_Init( int heapID );
void	DLPlaySend_Term( DLPLAY_SEND_DATA *dlData );
void	DLPlaySend_StartMBP( DLPLAY_SEND_DATA *dlData );

void DLPlaySend_SetMessageSystem( DLPLAY_MSG_SYS *msgSys , DLPLAY_SEND_DATA *dlData );
BOOL DLPlaySend_MBPLoop( DLPLAY_SEND_DATA *dlData );
static void OnPreSendMBVolat(u32 ggid);
//============================================================================================

//--------------------------------------------------------------
/**
 * 初期化
 * @param	heapID	ヒープID
 * @retval	
 */
//--------------------------------------------------------------
DLPLAY_SEND_DATA* DLPlaySend_Init( int heapID )
{
	DLPLAY_SEND_DATA *dlData;
	dlData = GFL_HEAP_AllocClearMemory( heapID , sizeof( DLPLAY_SEND_DATA ) );


	dlData->subSeq_ = 0xFFFF;
	return dlData;
}

//--------------------------------------------------------------
/**
 * 開放
 * @param	heapID	ヒープID
 * @retval	
 */
//--------------------------------------------------------------
void	DLPlaySend_Term( DLPLAY_SEND_DATA *dlData )
{
	GFL_HEAP_FreeMemory( dlData );
}

void DLPlaySend_SetMessageSystem( DLPLAY_MSG_SYS *msgSys , DLPLAY_SEND_DATA *dlData )
{
	dlData->msgSys_ = msgSys;
}


void	DLPlaySend_StartMBP( DLPLAY_SEND_DATA *dlData )
{
	MBP_Init( WH_GGID , 127 );	
}

BOOL DLPlaySend_MBPLoop( DLPLAY_SEND_DATA *dlData )
{
	const u16 mbpState = MBP_GetState();
	BOOL isChangeState = FALSE;

	if( mbpState != dlData->subSeq_ ){
		isChangeState = TRUE;
		dlData->subSeq_ = mbpState;
	}
	//以下サンプル流用
	switch (MBP_GetState())
	{
		//-----------------------------------------
		// アイドル状態
	case MBP_STATE_IDLE:
		{
			u16 test = WM_GetAllowedChannel();
			ARI_TPrintf("[%d][%x]\n",test,test);
			MBP_Start(&mbGameList, 1 /*通信チャンネルsChannel*/);
			/* ユーザ定義データの送信テスト */
			MB_SetSendVolatCallback(OnPreSendMBVolat, MB_SEND_VOLAT_CALLBACK_TIMMING_BEFORE);
		}
		break;

		//-----------------------------------------
		// 子機からのエントリー受付中
	case MBP_STATE_ENTRY:
		{
			if( isChangeState == TRUE ){
				DLPlayFunc_PutString( "Now Accepting",dlData->msgSys_); 
			}
			//BgSetMessage(PLTT_WHITE, " Now Accepting			");

			if ( GFL_UI_KEY_GetTrg() == PAD_BUTTON_B )
			{
				// Bボタンでマルチブートキャンセル
				MBP_Cancel();
				break;
			}

			// エントリー中の子機が一台でも存在すれば開始可能とする
			if (MBP_GetChildBmp(MBP_BMPTYPE_ENTRY) ||
				MBP_GetChildBmp(MBP_BMPTYPE_DOWNLOADING) ||
				MBP_GetChildBmp(MBP_BMPTYPE_BOOTABLE))
			{
				//if( isChangeState == TRUE ){
					DLPlayFunc_PutString( "Find child. send RomImage start.",dlData->msgSys_); 
					DLPlayFunc_ChangeBgMsg( MSG_FIND_CHILD , dlData->msgSys_ );
				//}
				//BgSetMessage(PLTT_WHITE, " Push START Button to start   ");
				//子機が来たらとりあえず始めてしまう
				//if ( GFL_UI_KEY_GetTrg() == PAD_BUTTON_START )
				{
					// ダウンロード開始
					MBP_StartDownloadAll();
				}
			}
		}
		break;

		//-----------------------------------------
		// プログラム配信処理
	case MBP_STATE_DATASENDING:
		{

			// 全員がダウンロード完了しているならばスタート可能.
			if (MBP_IsBootableAll())
			{
				// ブート開始
				MBP_StartRebootAll();
			}
		}
		break;

		//-----------------------------------------
		// リブート処理
	case MBP_STATE_REBOOTING:
		{
			if( isChangeState == TRUE ){
				DLPlayFunc_PutString( "Rebooting now",dlData->msgSys_); 
				DLPlayFunc_ChangeBgMsg( MSG_WAIT_CHILD_CONNECT , dlData->msgSys_ );
			}
			//BgSetMessage(PLTT_WHITE, " Rebooting now				");
		}
		break;

		//-----------------------------------------
		// 再接続処理
	case MBP_STATE_COMPLETE:
		{
			if( isChangeState == TRUE ){
				DLPlayFunc_PutString( "Reconnecting now",dlData->msgSys_); 
			}
			// 全員無事に接続完了したらマルチブート処理は終了し
			// 通常の親機として無線を再起動する。
			//BgSetMessage(PLTT_WHITE, " Reconnecting now			 ");

			//OS_WaitVBlankIntr();
			return TRUE;
			//return DPM_DATA_SHARE;
		}
		break;

		//-----------------------------------------
		// エラー発生
	case MBP_STATE_ERROR:
		{
			// 通信をキャンセルする
			MBP_Cancel();
		}
		break;

		//-----------------------------------------
		// 通信キャンセル処理中
	case MBP_STATE_CANCEL:
		// None
		break;

		//-----------------------------------------
		// 通信異常終了
	case MBP_STATE_STOP:
		/*
		switch (WH_GetSystemState())
		{
		case WH_SYSSTATE_IDLE:
			(void)WH_End();
			break;
		case WH_SYSSTATE_BUSY:
			break;
		case WH_SYSSTATE_STOP:
			return FALSE;
		default:
			OS_Panic("illegal state\n");
		}
		*/
		break;
	}

	return FALSE;
}


/*---------------------------------------------------------------------------*
  Name:		 OnPreSendMBVolat

  Description:  親機がMBビーコンを送信する前に通知されるコールバック関数

  Arguments:	ggid	送信するゲーム情報のGGID.

  Returns:	  None.
 *---------------------------------------------------------------------------*/
static void OnPreSendMBVolat(u32 ggid)
{
	/*
	 * 複数のゲームを登録し個々のゲームごとに設定値を切り替える場合は
	 * 以下のように引数の ggid でゲーム情報を判定します.
	 */
	if (ggid == mbGameList.ggid)
	{
		/*
		 * 送信可能なユーザ定義データは最大 8 BYTE です.
		 * この例ではインクリメンタルな 64bit 値を送信しています.
		 * 動的に更新されたデータ内容を、子機側では必ずしもただちに
		 * 受信できるわけではないという点に注意してください.
		 */
		static u64 count ATTRIBUTE_ALIGN(8);
		SDK_COMPILER_ASSERT(sizeof(count) <= MB_USER_VOLAT_DATA_SIZE);
		SDK_COMPILER_ASSERT(MB_USER_VOLAT_DATA_SIZE == 8);
		++count;
		MB_SetUserVolatData(ggid, (u8 *)&count, sizeof(count));
	}
}
