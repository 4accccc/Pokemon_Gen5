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

#include "test/ariizumi/ari_debug.h"
#include "dlplay_parent_sample.h"
#include "dlplay_parent_main.h"
#include "dlplay_func.h"

#include "mbp.h"

/* このデモで使用する GGID */
#define WH_GGID                 (0x3FFF21)

/* このデモがダウンロードさせるプログラム情報 */
const MBGameRegistry mbGameList = {
    "/dl_rom/child.srl",                      // 子機バイナリコード
    (u16 *)L"MultiBootTest",           // ゲーム名
    (u16 *)L"マルチブートテスト by Ariizumi",        // ゲーム内容説明
    "/dl_rom/icon.char",                 // アイコンキャラクタデータ
    "/dl_rom/icon.plt",                  // アイコンパレットデータ
    WH_GGID,                           // GGID
    MBP_CHILD_MAX + 1,                 // 最大プレイ人数、親機の数も含めた人数
};


//============================================================================================
//	struct
//============================================================================================
struct _DLPLAY_SEND_DATA
{
	u8 mainSeq_;
	u16 subSeq_;

	DLPLAY_MSG_SYS *msgSys_;
};

//======================================================================
//	enum
//======================================================================
enum DLPLAY_SEND_STATE
{
	DSS_WAIT_START,
	DSS_MAIN_LOOP,
};

//============================================================================================
//	proto
//============================================================================================
DLPLAY_SEND_DATA* DLPlaySend_Init( int heapID );
u8		DLPlaySend_Loop( DLPLAY_SEND_DATA *dlData );
void	DLPlaySend_Term( DLPLAY_SEND_DATA *dlData );

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

	dlData->msgSys_ = DLPlayFunc_MsgInit( heapID , DLPLAY_MSG_PLANE );

	dlData->mainSeq_ = DSS_WAIT_START;
	dlData->subSeq_ = 0xFFFF;
	DLPlayFunc_PutString("",dlData->msgSys_);
	DLPlayFunc_PutString("Initialize complete.",dlData->msgSys_);
	DLPlayFunc_PutString("Press A Button to start.",dlData->msgSys_);
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
	DLPlayFunc_MsgTerm( dlData->msgSys_ );
}

//--------------------------------------------------------------
/**
 * メインループ
 * @param	
 * @retval	次のパート	
 */
//--------------------------------------------------------------
u8		DLPlaySend_Loop( DLPLAY_SEND_DATA *dlData )
{
	switch( dlData->mainSeq_ )
	{
	case DSS_WAIT_START:
		if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_A ){
			MBP_Init( WH_GGID , 127 );	

			dlData->mainSeq_ = DSS_MAIN_LOOP;
			DLPlayFunc_PutString( "MultiBoot is start."  ,dlData->msgSys_ );
		}
		break;

	case DSS_MAIN_LOOP:
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
	                //BgSetMessage(PLTT_WHITE, " Now Accepting            ");
	
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
							DLPlayFunc_PutString( "Push START Button to start",dlData->msgSys_); 
						//}
	                    //BgSetMessage(PLTT_WHITE, " Push START Button to start   ");
	
						if ( GFL_UI_KEY_GetTrg() == PAD_BUTTON_START )
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
					}
	                //BgSetMessage(PLTT_WHITE, " Rebooting now                ");
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
	                //BgSetMessage(PLTT_WHITE, " Reconnecting now             ");
	
	                OS_WaitVBlankIntr();
	                return DPM_DATA_SHARE;
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
#ifdef MBP_USING_MB_EX
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
#else
	            return FALSE;
#endif
	        }
	
	        // 子機状態を表示する
	        //PrintChildState();
	    }		
	
	}
	return DPM_SEND_IMAGE;
	
#if 0
	static u16 val = 0;
	char str[32];
	val++;
	sprintf(str,"No.%d",val);

	if( val%60 == 0 )
		DLPlayFunc_PutString( str , dlData->msgSys_ );
	return DPM_SEND_IMAGE;
#endif
	
}


/*---------------------------------------------------------------------------*
  Name:         OnPreSendMBVolat

  Description:  親機がMBビーコンを送信する前に通知されるコールバック関数

  Arguments:    ggid    送信するゲーム情報のGGID.

  Returns:      None.
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
