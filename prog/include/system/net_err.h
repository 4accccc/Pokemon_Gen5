//==============================================================================
/**
 * @file	net_err.h
 * @brief	通信エラー画面制御のヘッダ
 * @author	matsuda
 * @date	2008.11.17(月)
 */
//==============================================================================
#ifndef __NET_ERR_H__
#define __NET_ERR_H__


//==============================================================================
//	定数定義
//==============================================================================
///エラー画面システムの状況
typedef enum{
	NET_ERR_STATUS_NULL,			///<何もなし
	NET_ERR_STATUS_ERROR,			///<エラー発生
	NET_ERR_STATUS_REQ,				///<エラー画面呼び出しリクエスト発生中
}NET_ERR_STATUS;


//--------------------------------------------------------------
//  使用ヒープサイズ
//--------------------------------------------------------------
///キャラクタVRAM退避サイズ
#define NETERR_PUSH_CHARVRAM_SIZE		(0x4000)
///スクリーンVRAM退避サイズ
#define NETERR_PUSH_SCRNVRAM_SIZE		(0x800)
///パレットVRAM退避サイズ
#define NETERR_PUSH_PLTTVRAM_SIZE		(0x20)


//==============================================================================
//	外部関数宣言
//==============================================================================

//--------------------------------------------------------------
//	システム側で使用する関数
//--------------------------------------------------------------
extern void NetErr_SystemInit(void);
extern void NetErr_SystemCreate(int heap_id);
extern void NetErr_SystemExit(void);
extern void NetErr_Main(void);
extern void NetErr_ErrorSet(void);
extern BOOL NetErr_DispCall(void);
extern void NetErr_DispCallPushPop(void);

extern void NetErr_GetTempArea( u8** charArea , u16** scrnArea , u16** plttArea );


//--------------------------------------------------------------
//	アプリ側で使用する関数
//--------------------------------------------------------------
extern BOOL NetErr_App_CheckError(void);
extern void NetErr_App_ReqErrorDisp(void);


//--------------------------------------------------------------
//  デバッグ用
//--------------------------------------------------------------
#if PM_DEBUG
extern void NetErr_DEBUG_ErrorSet(void);
#endif  //PM_DEBUG

#endif	//__NET_ERR_H__
