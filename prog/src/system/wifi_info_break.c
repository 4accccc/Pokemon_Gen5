//==============================================================================
/**
 * @file    wifi_info_break.c
 * @brief   本体のWi-Fiユーザー情報が消えた場合に表示される
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
#include "msg\msg_wifi_system.h"

#include "savedata/save_control.h"
#include "net_err.naix"
#include "message.naix"

#include "system/wifi_info_break.h"
#include "system/machine_use.h"
#include "system/ds_system.h"


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

///通信エラー画面を終了させるボタン
#define ERR_DISP_END_BUTTON		(PAD_BUTTON_A | PAD_BUTTON_B)

#define _CHARVRAM_SIZE   (0x4000)
#define _SCRNVRAM_SIZE		(0x800)
#define _PLTTVRAM_SIZE		(0x20)

///WIFI破壊画面で使用するテンポラリヒープ
#define HEAPID_WIFIBREAK_TEMP			(GFL_HEAPID_APP)



//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static void Local_WarningDispInit(u32 msg_id);
static void Local_WarningDispDraw(void);
static void Local_WarningMessagePrint(u32 msg_id);
static void Local_ErrDispExit(void);


//==============================================================================
//
//	
//
//==============================================================================
//==================================================================
/**
 * 「本体のWifiユーザー情報が壊れた」画面一発呼び出し
 */
//==================================================================
void WifiInfoBreak_Call(void)
{
  u32 msg_id;
  
  if(DS_SYSTEM_IsRunOnTwl()){
    msg_id = dwctwl_message_0001;
  }
  else{
    msg_id = dwc_message_0001;
  }
  
  Local_WarningDispInit(msg_id);

	while((PAD_Read() & ERR_DISP_END_BUTTON) != 0){
    MachineSystem_Main(); //ハードリセット用
    GFL_UI_Main();  //LCDOFF対応
	}
	
	while((PAD_Read() & ERR_DISP_END_BUTTON) == 0){
    MachineSystem_Main(); //ハードリセット用
    GFL_UI_Main();  //LCDOFF対応
	}

  Local_ErrDispExit();
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
	
	//VRAMバンク情報退避
	GX_ResetBankForBG();
	GX_ResetBankForSubBG();
	GX_SetBankForBG(GX_VRAM_BG_128_C);

	GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
	GX_SetVisiblePlane(GX_PLANEMASK_BG1);
	GX_SetVisibleWnd(GX_WNDMASK_NONE);

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
	
	GFL_STD_MemClear32(G2_GetBG1CharPtr(), _CHARVRAM_SIZE);

	//キャラクタ
	arcData = GFL_ARC_UTIL_Load(ARCID_NET_ERR, NARC_net_err_net_err_NCGR, 0, HEAPID_WIFIBREAK_TEMP);
	if(NNS_G2dGetUnpackedBGCharacterData(arcData, &charData)){
		DC_FlushRange(charData->pRawData, charData->szByte);
		//念のためDMAを使用するのは避ける
		GFL_STD_MemCopy16(charData->pRawData, G2_GetBG1CharPtr(), charData->szByte);
		GF_ASSERT(MESSAGE_START_CHARNO*0x20 >= charData->szByte);	//このASSERTに引っかかる場合はBG_DATA_SIZEを大きくする必要がある。その分エラーメッセージに割けるキャラクタ領域が少なくなるので注意
	}
	GFL_HEAP_FreeMemory(arcData);
	
	//スクリーン
	arcData = GFL_ARC_UTIL_Load(ARCID_NET_ERR, NARC_net_err_net_err_NSCR, 0, HEAPID_WIFIBREAK_TEMP);
	if( NNS_G2dGetUnpackedScreenData( arcData, &scrnData ) ){
		DC_FlushRange(scrnData->rawData, scrnData->szByte);
		GFL_STD_MemCopy16(scrnData->rawData, G2_GetBG1ScrPtr(), scrnData->szByte);
	}
	GFL_HEAP_FreeMemory( arcData );
	
	//パレット
	arcData = GFL_ARC_UTIL_Load(ARCID_NET_ERR, NARC_net_err_net_err_NCLR, 0, HEAPID_WIFIBREAK_TEMP);
	cmpFlag = NNS_G2dGetUnpackedPaletteCompressInfo( arcData, &cmpInfo );
	if( NNS_G2dGetUnpackedPaletteData( arcData, &palData ) ){
		DC_FlushRange( palData->pRawData, _PLTTVRAM_SIZE );
		GFL_STD_MemCopy16(palData->pRawData, (void*)HW_BG_PLTT, _PLTTVRAM_SIZE);
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
		MESSAGE_X_LEN, MESSAGE_Y_LEN, GFL_BMP_16_COLOR, HEAPID_WIFIBREAK_TEMP);
	
	//メッセージOPEN
	{
		GFL_FONT		*fontHandle;
		GFL_MSGDATA		*mm;
    STRBUF  *src;

		fontHandle = GFL_FONT_Create(ARCID_FONT, NARC_font_large_gftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_WIFIBREAK_TEMP );
		
    mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, 
        ARCID_MESSAGE, NARC_message_wifi_system_dat, HEAPID_WIFIBREAK_TEMP);

    src     = GFL_MSG_CreateString(mm, msg_id);
		PRINTSYS_PrintColor(bmpdata, 0, 0, src, fontHandle, PRINTSYS_LSB_Make(4, 0xb, 7));

		GFL_STR_DeleteBuffer(src);
		GFL_MSG_Delete(mm);
		
		GFL_FONT_Delete(fontHandle);
	}
	
	GFL_BMP_Delete(bmpdata);
}

//--------------------------------------------------------------
/**
 * @brief   エラー画面を終了させ、元の画面に復帰させる
 */
//--------------------------------------------------------------
static void Local_ErrDispExit(void)
{
	//書き換える画面が見えないように、真っ暗に設定
	GX_SetMasterBrightness(-16);
	GXS_SetMasterBrightness(-16);
	
  //下画面の描画設定
  GFL_DISP_GXS_SetVisibleControlDirect( 0 );

	//VRAM復帰
	GFL_STD_MemClear16(G2_GetBG1CharPtr(), _CHARVRAM_SIZE);
	GFL_STD_MemClear16(G2_GetBG1ScrPtr(), _SCRNVRAM_SIZE);
	GFL_STD_MemFill16((void*)HW_PLTT, 0x7fff, _PLTTVRAM_SIZE);

	GX_SetVisiblePlane(0);

	//VRAMバンク情報復帰
  GX_ResetBankForSubBG();
	GX_ResetBankForBG();

  GX_SetMasterBrightness(0);
  GXS_SetMasterBrightness(0);
}
