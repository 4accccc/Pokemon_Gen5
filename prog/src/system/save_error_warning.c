//==============================================================================
/**
 * @file    save_error_warning.c
 * @brief   セーブ読み書きに失敗した場合の警告画面
 * @author  matsuda
 * @date    2010.04.14(水)
 */
//==============================================================================
#include <gflib.h>
#include "system\main.h"
#include "arc_def.h"

#include "print\printsys.h"
#include "print\gf_font.h"
#include "font/font.naix"
#include "msg\msg_backup_err.h"

#include "savedata/save_control.h"
#include "system/net_err.h"
#include "net_err.naix"
#include "message.naix"

#include "system/save_error_warning.h"


//==============================================================================
//	定数定義
//==============================================================================
///メッセージ描画領域開始位置：左端X
#define MESSAGE_START_X			(1)
///メッセージ描画領域開始位置：上端Y
#define MESSAGE_START_Y			(4)
///メッセージ描画領域X長
#define MESSAGE_X_LEN			(30)
///メッセージ描画領域Y長
#define MESSAGE_Y_LEN			(16)

///メッセージのパレット展開位置
#define MESSAGE_PALNO			(0)

///背景BGグラフィック(ncg)のデータサイズ
#define BG_DATA_SIZE			(32 * 0x20)
///メッセージ描画開始キャラクタNo
#define MESSAGE_START_CHARNO	(BG_DATA_SIZE / 0x20)

//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE  //通常時処理
#define SAVE_ERR_HEAPID (HEAPID_SAVE)
#else                    //DL子機時処理
#define SAVE_ERR_HEAPID (HEAPID_MULTIBOOT)
#endif //MULTI_BOOT_MAKE

//背景BG＋メッセージデータでキャラクタ領域NETERR_PUSH_CHARVRAM_SIZEをオーバーしていないかチェック
SDK_COMPILER_ASSERT(MESSAGE_X_LEN*0x20*MESSAGE_Y_LEN + BG_DATA_SIZE <= NETERR_PUSH_CHARVRAM_SIZE);


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static void Local_WarningDispMain(u32 msg_id);
static void Local_WarningDispInit(u32 msg_id);
static void Local_WarningDispDraw(void);
static void Local_WarningMessagePrint(u32 msg_id);


//==============================================================================
//
//	
//
//==============================================================================
//==================================================================
/**
 * セーブコールバック：ロードに失敗
 */
//==================================================================
void SaveErrorCall_Load(void)
{
  Local_WarningDispMain(msg_load_err);
}

//==================================================================
/**
 * セーブコールバック：セーブに失敗
 *
 * @param   err_disable		
 */
//==================================================================
void SaveErrorCall_Save(GFL_SAVEERROR_DISABLE err_disable)
{
  u32 msg_id = err_disable == GFL_SAVEERROR_DISABLE_WRITE ? msg_flash_err : msg_write_err;
  Local_WarningDispMain(msg_id);
}


//--------------------------------------------------------------
/**
 * 警告画面メイン
 *
 * @param   msg_id		表示するメッセージ
 */
//--------------------------------------------------------------
static void Local_WarningDispMain(u32 msg_id)
{
//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE  //通常時処理
  SaveControl_SystemExit(); //画面描画用のヒープ確保の為、セーブシステムを破棄する
#endif //MULTI_BOOT_MAKE
  Local_WarningDispInit(msg_id);

  //このプレイヤーに侵入されないよう通信を終了させる
  if(GFL_NET_IsInit())
  {
    GFL_NET_ResetDisconnect();  ///切断処理中でも一旦リセット
    GFL_NET_Exit(NULL);
    GFL_NET_IRCWIRELESS_ResetSystemError();  //赤外線WIRLESS切断
    do{
      GFL_NET_Main();
    }while(GFL_NET_IsExit() == FALSE);
  }
  
  //無限ループ
  do{
    OS_Halt();
  }while(1);  //一応
}

//--------------------------------------------------------------
/**
 * 画面の初期設定
 */
//--------------------------------------------------------------
static void Local_WarningDispInit(u32 msg_id)
{
	//書き換える画面が見えないように、まずマスター輝度を退避し、真っ暗に設定
	GX_SetMasterBrightness(-16);
	GXS_SetMasterBrightness(-16);
	
	//割り込みを止める
	GX_HBlankIntr(FALSE);
	GX_VBlankIntr(FALSE);
	
	//VRAMバンク情報退避
	GX_ResetBankForBG();
	GX_ResetBankForSubBG();
	GX_SetBankForBG(GX_VRAM_BG_128_C);

	//dispcnt退避
	GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
	GX_SetVisiblePlane(GX_PLANEMASK_BG1);
	GX_SetVisibleWnd(GX_WNDMASK_NONE);

	//BG1Control退避
	G2_SetBG1Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16, 
		GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GX_BG_EXTPLTT_01);
	G2_BG1Mosaic(FALSE);
	G2_SetBG1Offset(0, 0);

	G2_BlendNone();
	
	//エラー画面描画
	Local_WarningDispDraw();
	Local_WarningMessagePrint(msg_id);
	
	//表示ON
	GX_SetMasterBrightness(0);
	GXS_SetMasterBrightness(16);	//サブ画面は真っ白
}

//--------------------------------------------------------------
/**
 * @brief   エラー画面描画
 */
//--------------------------------------------------------------
static void Local_WarningDispDraw(void)
{
	u32 data_size;
	void *arcData;
	NNSG2dCharacterData* charData;
	NNSG2dScreenData* scrnData;
	NNSG2dPaletteData* palData;
	BOOL  cmpFlag;
	NNSG2dPaletteCompressInfo*  cmpInfo;
	
	GFL_STD_MemClear32(G2_GetBG1CharPtr(), NETERR_PUSH_CHARVRAM_SIZE);

	//キャラクタ
	arcData = GFL_ARC_UTIL_Load(ARCID_NET_ERR, NARC_net_err_net_err_NCGR, 0, SAVE_ERR_HEAPID);
	if(NNS_G2dGetUnpackedBGCharacterData(arcData, &charData)){
		DC_FlushRange(charData->pRawData, charData->szByte);
		//念のためDMAを使用するのは避ける
		GFL_STD_MemCopy16(charData->pRawData, G2_GetBG1CharPtr(), charData->szByte);
		GF_ASSERT(MESSAGE_START_CHARNO*0x20 >= charData->szByte);	//このASSERTに引っかかる場合はBG_DATA_SIZEを大きくする必要がある。その分エラーメッセージに割けるキャラクタ領域が少なくなるので注意
	}
	GFL_HEAP_FreeMemory(arcData);
	
	//スクリーン
	arcData = GFL_ARC_UTIL_Load(ARCID_NET_ERR, NARC_net_err_net_err_NSCR, 0, SAVE_ERR_HEAPID);
	if( NNS_G2dGetUnpackedScreenData( arcData, &scrnData ) ){
		DC_FlushRange(scrnData->rawData, scrnData->szByte);
		GFL_STD_MemCopy16(scrnData->rawData, G2_GetBG1ScrPtr(), scrnData->szByte);
	}
	GFL_HEAP_FreeMemory( arcData );
	
	//パレット
	arcData = GFL_ARC_UTIL_Load(ARCID_NET_ERR, NARC_net_err_net_err_NCLR, 0, SAVE_ERR_HEAPID);
	cmpFlag = NNS_G2dGetUnpackedPaletteCompressInfo( arcData, &cmpInfo );
	if( NNS_G2dGetUnpackedPaletteData( arcData, &palData ) ){
		DC_FlushRange( palData->pRawData, NETERR_PUSH_PLTTVRAM_SIZE );
		GFL_STD_MemCopy16(palData->pRawData, (void*)HW_BG_PLTT, NETERR_PUSH_PLTTVRAM_SIZE);
	}
	GFL_HEAP_FreeMemory( arcData );
}

//--------------------------------------------------------------
/**
 * @brief   エラーメッセージ表示
 */
//--------------------------------------------------------------
static void Local_WarningMessagePrint(u32 msg_id)
{
	GFL_BMP_DATA *bmpdata;
	int x, y;
	u16 *scrn_vram;
	u16 data;
	u16 *test;
	
	//スクリーンをBMP領域の形で埋め尽くし
	scrn_vram = G2_GetBG1ScrPtr();
	data = MESSAGE_START_CHARNO | (MESSAGE_PALNO << 12);
	for(y = MESSAGE_START_Y; y < MESSAGE_START_Y+MESSAGE_Y_LEN; y++){
		for(x = MESSAGE_START_X; x < MESSAGE_START_X+MESSAGE_X_LEN; x++){
			scrn_vram[y*0x20+x] = data;
			data++;
		}
	}

	//BMPキャラクタ領域を文字背景色で埋める
	GFL_STD_MemFill16((void*)((u32)G2_GetBG1CharPtr() + MESSAGE_START_CHARNO*0x20), 
	  0x7777, MESSAGE_X_LEN * MESSAGE_Y_LEN * 32);
	
	//BMP作成
	bmpdata = GFL_BMP_CreateInVRAM((void*)((u32)G2_GetBG1CharPtr() + MESSAGE_START_CHARNO*0x20), 
		MESSAGE_X_LEN, MESSAGE_Y_LEN, GFL_BMP_16_COLOR, SAVE_ERR_HEAPID);
	
	//メッセージOPEN
	{
		GFL_FONT		*fontHandle;
		GFL_MSGDATA		*mm;
    STRBUF  *src;

		fontHandle = GFL_FONT_Create(ARCID_FONT, NARC_font_large_gftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, SAVE_ERR_HEAPID );
		
//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE  //通常時処理
    mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, 
        ARCID_MESSAGE, NARC_message_backup_err_dat, SAVE_ERR_HEAPID);
#else                    //DL子機時処理
      //MB_BACKUP_ERROR_MSG_FILEはArc名がポケシフターと映画転送で違うのでmustiboot/src/net_err_dl.cで定義
    mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, 
        ARCID_MESSAGE, MB_BACKUP_ERROR_MSG_FILE, SAVE_ERR_HEAPID);
#endif //MULTI_BOOT_MAKE

    src     = GFL_MSG_CreateString(mm, msg_id);
		PRINTSYS_PrintColor(bmpdata, 0, 0, src, fontHandle, PRINTSYS_LSB_Make(4, 0xb, 7));

		GFL_STR_DeleteBuffer(src);
		GFL_MSG_Delete(mm);
		
		GFL_FONT_Delete(fontHandle);
	}
	
	GFL_BMP_Delete(bmpdata);
}
