//==============================================================================
/**
 * @file	net_err.c
 * @brief	通信エラー画面制御
 * @author	matsuda
 * @date	2008.11.14(金)
 */
//==============================================================================
#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>
#include "system\main.h"
#include "net\network_define.h"
#include "system/net_err.h"
#include "arc_def.h"
#include "net_err.naix"

#include "print\printsys.h"
#include "print\gf_font.h"
#include "message.naix"
#include "font/font.naix"
#include "msg\msg_d_matsu.h"


//==============================================================================
//	定数定義
//==============================================================================
///通信エラー画面で使用するテンポラリヒープ
#define HEAPID_NET_TEMP			(GFL_HEAPID_APP)

///通信エラー画面を終了させるボタン
#define ERR_DISP_END_BUTTON		(PAD_BUTTON_A | PAD_BUTTON_B)

///エラー画面表示後、キーを有効にするまでのウェイト
#define ERR_DISP_KEY_OCC_WAIT		(60*2)

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

//背景BG＋メッセージデータでキャラクタ領域NETERR_PUSH_CHARVRAM_SIZEをオーバーしていないかチェック
SDK_COMPILER_ASSERT(MESSAGE_X_LEN*0x20*MESSAGE_Y_LEN + BG_DATA_SIZE <= NETERR_PUSH_CHARVRAM_SIZE);


//==============================================================================
//	構造体定義
//==============================================================================
///通信エラー画面制御構造体
typedef struct{
	u8 *push_char_p;			///<キャラクタVRAM退避先
	u16 *push_scrn_p;			///<スクリーンVRAM退避先
	u16 *push_pltt_p;			///<パレットVRAM退避先

	GXDispCnt dispcnt;
	GXBg01Control bg1cnt;
	int master_brightness;		///<メイン画面マスター輝度
	int master_brightness_sub;	///<サブ画面マスター輝度
	s32 h_intr;					///<Hブランク割り込み許可
	s32 v_intr;					///<Vブランク割り込み許可
	GXVRamBG bg_bank;			///<BG VRAMバンク情報
	
	u8 font_letter;
	u8 font_shadow;
	u8 font_back;
	
	NET_ERR_STATUS status;		///<エラー画面システムの状況
	s32 key_timer;				///<キー入力受付までのタイマー
}NET_ERR_SYSTEM;


//==============================================================================
//	ローカル変数
//==============================================================================
static NET_ERR_SYSTEM NetErrSystem = {0};

static const int asd[]={1,2,3,4,5};

//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static BOOL NetErr_DispMain(void);
static void Local_ErrDispInit(void);
static void Local_ErrDispExit(void);
static BOOL Local_SystemOccCheck(void);
static void Local_ErrDispDraw(void);
static void Local_ErrMessagePrint(void);


//==============================================================================
//
//	
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   通信エラー画面システム初期化
 *
 * ゲーム内で一度だけ呼び出す
 */
//--------------------------------------------------------------
void NetErr_SystemInit(void)
{
  int a;
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_NET_ERR, NETERR_PUSH_CHARVRAM_SIZE + NETERR_PUSH_SCRNVRAM_SIZE + NETERR_PUSH_PLTTVRAM_SIZE + 0x100);  //+0x100はヘッダ分
  a=asd[0];
  a++;
	GFL_STD_MemClear(&NetErrSystem, sizeof(NET_ERR_SYSTEM));
}

//--------------------------------------------------------------
/**
 * @brief   通信エラー画面システム作成
 * 
 * @param	ヒープID
 * 
 * ゲームの最初に作って、以後ずっと使用するのであれば一度だけの呼び出しでOK。
 * ヒープ節約のために、各アプリなどでのみ使用する場合は、その都度
 * SystemCreate,SystemExitを呼ぶようにして使ってください
 */
//--------------------------------------------------------------
void NetErr_SystemCreate(int heap_id)
{
	NET_ERR_SYSTEM *nes = &NetErrSystem;

	GF_ASSERT(nes->push_char_p == NULL);
	GF_ASSERT(nes->push_scrn_p == NULL);
	GF_ASSERT(nes->push_pltt_p == NULL);
	
	GFL_STD_MemClear(nes, sizeof(NET_ERR_SYSTEM));

	nes->push_char_p = GFL_HEAP_AllocMemory(heap_id, NETERR_PUSH_CHARVRAM_SIZE);
	nes->push_scrn_p = GFL_HEAP_AllocMemory(heap_id, NETERR_PUSH_SCRNVRAM_SIZE);
	nes->push_pltt_p = GFL_HEAP_AllocMemory(heap_id, NETERR_PUSH_PLTTVRAM_SIZE);
}

//--------------------------------------------------------------
/**
 * @brief   通信エラー画面システム破棄
 */
//--------------------------------------------------------------
void NetErr_SystemExit(void)
{
	NET_ERR_SYSTEM *nes = &NetErrSystem;

	GF_ASSERT(nes->push_char_p != NULL);
	GF_ASSERT(nes->push_scrn_p != NULL);
	GF_ASSERT(nes->push_pltt_p != NULL);
	
	GFL_HEAP_FreeMemory(nes->push_char_p);
	GFL_HEAP_FreeMemory(nes->push_scrn_p);
	GFL_HEAP_FreeMemory(nes->push_pltt_p);

	GFL_STD_MemClear(nes, sizeof(NET_ERR_SYSTEM));
	
	GFL_HEAP_DeleteHeap(HEAPID_NET_ERR);
}

//==================================================================
/**
 * エラー発生監視：メイン
 */
//==================================================================
void NetErr_Main(void)
{
  if(NetErrSystem.status == NET_ERR_STATUS_NULL && GFL_NET_IsInit() == TRUE){
    if(GFL_NET_GetErrorCode() || GFL_NET_SystemIsError() || (GFL_NET_StateGetError()!=0)){
      //GFL_NET_StateGetWifiErrorNo();  これはエラー情報の取得だけに使用する
      OS_TPrintf("NetErr エラー発生\n");
      NetErr_ErrorSet();
    }
  }
}

//==================================================================
/**
 * エラーが発生していればエラー画面を一発呼び出し
 *
 * @retval  TRUE:エラー画面を呼び出した
 * @retval  FALSE:呼び出していない
 * 
 * エラー画面を表示した場合、ユーザーが特定の操作を行わない限り(Aを押す)
 * この関数内で無限ループします。
 */
//==================================================================
BOOL NetErr_DispCall(void)
{
  return NetErr_DispMain();
}

//==================================================================
/**
 * Push,Pop有のエラー画面一発呼び出し　※軽度エラー専用。通信の終了処理は行いません
 *
 * エラー画面を表示した場合、ユーザーが特定の操作を行わない限り(Aを押す)
 * この関数内で無限ループします。
 */
//==================================================================
void NetErr_DispCallPushPop(void)
{
	if(Local_SystemOccCheck() == FALSE){
		GF_ASSERT(0); //システムが作られていない
		return;
	}

	//エラー画面描画
	Local_ErrDispInit();
	
//		OS_SpinWait(10000);
	
	while((PAD_Read() & ERR_DISP_END_BUTTON) != 0){
		;	//ボタンを一度離すまで待つ
	}
	
	while((PAD_Read() & ERR_DISP_END_BUTTON) == 0){
		;	//エラー画面終了ボタンが押されるまで待つ
	}
	
	//エラー画面終了
	Local_ErrDispExit();
}

//--------------------------------------------------------------
/**
 * @brief   エラーが発生したらこの関数を使用してエラー画面を呼び出す
 *
 * @param   none		後々、エラー情報を入れてもらう
 */
//--------------------------------------------------------------
void NetErr_ErrorSet(void)
{
	NET_ERR_SYSTEM *nes = &NetErrSystem;

	if(Local_SystemOccCheck() == FALSE || nes->status != NET_ERR_STATUS_NULL){
		return;
	}
	nes->status = NET_ERR_STATUS_ERROR;
}

//--------------------------------------------------------------
/**
 * @brief   エラーが発生したらこの関数を使用してエラー画面を呼び出す
 *
 * @param   none		後々、エラー情報を入れてもらう
 */
//--------------------------------------------------------------
#if PM_DEBUG
void NetErr_DEBUG_ErrorSet(void)
{
	NET_ERR_SYSTEM *nes = &NetErrSystem;
	nes->status = NET_ERR_STATUS_ERROR;
}
#endif  //PM_DEBUG

//--------------------------------------------------------------
/**
 * @brief   アプリ用：エラーが発生したか調べる
 *
 * @retval  TRUE:エラー発生
 * @retval  FALSE:正常動作
 *
 * アプリ側はこの関数を使用してエラーが発生しているか調べ、
 * エラーが発生していた場合は、各アプリ毎のエラー用処理へ移行してください
 */
//--------------------------------------------------------------
BOOL NetErr_App_CheckError(void)
{
	if(NetErrSystem.status != NET_ERR_STATUS_NULL){
    return TRUE;
  }
  return FALSE;
}

//==================================================================
/**
 * アプリ用：エラー画面表示リクエスト
 */
//==================================================================
void NetErr_App_ReqErrorDisp(void)
{
  GF_ASSERT(NetErrSystem.status == NET_ERR_STATUS_ERROR);
  NetErrSystem.status = NET_ERR_STATUS_REQ;
}

//--------------------------------------------------------------
/**
 * @brief   外に退避用アドレスを渡す
 */
//--------------------------------------------------------------
void NetErr_GetTempArea( u8** charArea , u16** scrnArea , u16** plttArea )
{
	NET_ERR_SYSTEM *nes = &NetErrSystem;
  *charArea = nes->push_char_p;
  *scrnArea = nes->push_scrn_p;
  *plttArea = nes->push_pltt_p;
}

//--------------------------------------------------------------
/**
 * @brief   通信エラー画面制御メイン
 *
 * @retval  TRUE:エラー画面を呼び出した
 * @retval  FALSE:呼び出していない
 */
//--------------------------------------------------------------
static BOOL NetErr_DispMain(void)
{
	NET_ERR_SYSTEM *nes = &NetErrSystem;
	
	if(Local_SystemOccCheck() == FALSE){
		return FALSE;
	}

	if(nes->status == NET_ERR_STATUS_REQ){
    //通信ライブラリ終了待ち  ※check　軽度エラーの場合は終了処理を入れない
    GFL_NET_Exit(NULL);
    do{
      GFL_NET_Main();
      OS_TPrintf("GFL_NET_IsExitの完了を待っています\n");
    }while(GFL_NET_IsExit() == FALSE);

		//エラー画面描画
		Local_ErrDispInit();
		
//		OS_SpinWait(10000);
		
		while((PAD_Read() & ERR_DISP_END_BUTTON) != 0){
			;	//ボタンを一度離すまで待つ
		}
		
		while((PAD_Read() & ERR_DISP_END_BUTTON) == 0){
			;	//エラー画面終了ボタンが押されるまで待つ
		}
		
		//エラー画面終了
		Local_ErrDispExit();
		nes->status = NET_ERR_STATUS_NULL;
		nes->key_timer = 0;
  	return TRUE;
	}
	
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   画面情報を退避し、エラー画面を表示
 */
//--------------------------------------------------------------
static void Local_ErrDispInit(void)
{
	NET_ERR_SYSTEM *nes = &NetErrSystem;
	
	if(Local_SystemOccCheck() == FALSE){
		return;
	}
	
	//書き換える画面が見えないように、まずマスター輝度を退避し、真っ暗に設定
	nes->master_brightness = GX_GetMasterBrightness();
	nes->master_brightness_sub = GXS_GetMasterBrightness();
	GX_SetMasterBrightness(-16);
	GXS_SetMasterBrightness(-16);
	
	//割り込みを止める
	nes->h_intr = GX_HBlankIntr(FALSE);
	nes->v_intr = GX_VBlankIntr(FALSE);
	
	//VRAMバンク情報退避
	nes->bg_bank = GX_ResetBankForBG();
	GX_SetBankForBG(GX_VRAM_BG_128_C);
	OS_TPrintf("VRAM BANK = %d\n", nes->bg_bank);

	//dispcnt退避
	nes->dispcnt = GX_GetDispCnt();
	GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
	GX_SetVisiblePlane(GX_PLANEMASK_BG1);
	GX_SetVisibleWnd(GX_WNDMASK_NONE);
	
	//BG1Control退避
	nes->bg1cnt = G2_GetBG1Control();
	G2_SetBG1Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16, 
		GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, nes->bg1cnt.bgExtPltt);
	G2_BG1Mosaic(FALSE);
	G2_SetBG1Offset(0, 0);
	
	//VRAMのデータを退避(念のため上でBG1の設定をしてから行っている)
	GFL_STD_MemCopy16(G2_GetBG1CharPtr(), nes->push_char_p, NETERR_PUSH_CHARVRAM_SIZE);
	GFL_STD_MemCopy16(G2_GetBG1ScrPtr(), nes->push_scrn_p, NETERR_PUSH_SCRNVRAM_SIZE);
	GFL_STD_MemCopy16((void*)HW_PLTT, nes->push_pltt_p, NETERR_PUSH_PLTTVRAM_SIZE);

	//フォントカラー退避
	GFL_FONTSYS_GetColor(&nes->font_letter, &nes->font_shadow, &nes->font_back);
	GFL_FONTSYS_SetColor(4, 0xb, 7);
	
	//※check　退避できないもの
	G2_BlendNone(); //WriteOnlyの為、レジスタ退避が出来ていない
	
	//エラー画面描画
	Local_ErrDispDraw();
	Local_ErrMessagePrint();
	
	//表示ON
	GX_SetMasterBrightness(0);
	GXS_SetMasterBrightness(16);	//サブ画面は真っ白
}

//--------------------------------------------------------------
/**
 * @brief   エラー画面を終了させ、元の画面に復帰させる
 */
//--------------------------------------------------------------
static void Local_ErrDispExit(void)
{
	NET_ERR_SYSTEM *nes = &NetErrSystem;
	int x, y;
	
	if(Local_SystemOccCheck() == FALSE){
		return;
	}
	
	//書き換える画面が見えないように、真っ暗に設定
	GX_SetMasterBrightness(-16);
	GXS_SetMasterBrightness(-16);
	
	//フォントカラー復帰
	GFL_FONTSYS_SetColor(nes->font_letter, nes->font_shadow, nes->font_back);

	//VRAM復帰
	GFL_STD_MemCopy16(nes->push_char_p, G2_GetBG1CharPtr(), NETERR_PUSH_CHARVRAM_SIZE);
	GFL_STD_MemCopy16(nes->push_scrn_p, G2_GetBG1ScrPtr(), NETERR_PUSH_SCRNVRAM_SIZE);
	GFL_STD_MemCopy16(nes->push_pltt_p, (void*)HW_PLTT, NETERR_PUSH_PLTTVRAM_SIZE);

	//BG1Control復帰
	G2_SetBG1Control(nes->bg1cnt.screenSize, nes->bg1cnt.colorMode,
		nes->bg1cnt.screenBase, nes->bg1cnt.charBase, nes->bg1cnt.bgExtPltt);
	G2_BG1Mosaic(nes->bg1cnt.mosaic);
	GFL_NET_BG1PosGet(&x, &y);
	G2_SetBG1Offset(x, y);
	
	//dispcnt復帰
	GX_SetGraphicsMode(nes->dispcnt.dispMode, nes->dispcnt.bgMode, nes->dispcnt.bg0_2d3d);
	GX_SetVisiblePlane(nes->dispcnt.visiblePlane);
	GX_SetVisibleWnd(nes->dispcnt.visibleWnd);

	//VRAMバンク情報復帰
	GX_ResetBankForBG();
	GX_SetBankForBG(nes->bg_bank);

	//割り込み復帰
	GX_HBlankIntr(nes->h_intr);
	GX_VBlankIntr(nes->v_intr);
	
	//表示ON
	GX_SetMasterBrightness(nes->master_brightness);
	GXS_SetMasterBrightness(nes->master_brightness_sub);
}

//--------------------------------------------------------------
/**
 * @brief   通信エラー画面制御システムが有効か調べる
 *
 * @retval  TRUE:有効、FALSE:無効
 */
//--------------------------------------------------------------
static BOOL Local_SystemOccCheck(void)
{
	if(NetErrSystem.push_char_p != NULL){
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   エラー画面描画
 */
//--------------------------------------------------------------
static void Local_ErrDispDraw(void)
{
	NET_ERR_SYSTEM *nes = &NetErrSystem;
	u32 data_size;
	void *arcData;
	NNSG2dCharacterData* charData;
	NNSG2dScreenData* scrnData;
	NNSG2dPaletteData* palData;
	BOOL  cmpFlag;
	NNSG2dPaletteCompressInfo*  cmpInfo;
	
	GFL_STD_MemClear32(G2_GetBG1CharPtr(), NETERR_PUSH_CHARVRAM_SIZE);

	//キャラクタ
	arcData = GFL_ARC_UTIL_Load(ARCID_NET_ERR, NARC_net_err_net_err_NCGR, 0, HEAPID_NET_TEMP);
	if(NNS_G2dGetUnpackedBGCharacterData(arcData, &charData)){
		DC_FlushRange(charData->pRawData, charData->szByte);
		//念のためDMAを使用するのは避ける
		GFL_STD_MemCopy16(charData->pRawData, G2_GetBG1CharPtr(), charData->szByte);
		OS_TPrintf("エラー画面：キャラクタサイズ＝%dbyte\n", charData->szByte);
		GF_ASSERT(MESSAGE_START_CHARNO*0x20 >= charData->szByte);	//このASSERTに引っかかる場合はBG_DATA_SIZEを大きくする必要がある。その分エラーメッセージに割けるキャラクタ領域が少なくなるので注意
	}
	GFL_HEAP_FreeMemory(arcData);
	
	//スクリーン
	arcData = GFL_ARC_UTIL_Load(ARCID_NET_ERR, NARC_net_err_net_err_NSCR, 0, HEAPID_NET_TEMP);
	if( NNS_G2dGetUnpackedScreenData( arcData, &scrnData ) ){
		DC_FlushRange(scrnData->rawData, scrnData->szByte);
		GFL_STD_MemCopy16(scrnData->rawData, G2_GetBG1ScrPtr(), scrnData->szByte);
	}
	GFL_HEAP_FreeMemory( arcData );
	
	//パレット
	arcData = GFL_ARC_UTIL_Load(ARCID_NET_ERR, NARC_net_err_net_err_NCLR, 0, HEAPID_NET_TEMP);
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
static void Local_ErrMessagePrint(void)
{
	NET_ERR_SYSTEM *nes = &NetErrSystem;
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
	
	//BMP作成
	bmpdata = GFL_BMP_CreateInVRAM((void*)((u32)G2_GetBG1CharPtr() + MESSAGE_START_CHARNO*0x20), 
		MESSAGE_X_LEN, MESSAGE_Y_LEN, GFL_BMP_16_COLOR, HEAPID_NET_TEMP);
	
	//メッセージOPEN
	{
		GFL_FONT		*fontHandle;
		GFL_MSGDATA		*mm;
		STRBUF *strbuf;

#ifndef MULTI_BOOT_MAKE
		fontHandle = GFL_FONT_Create(ARCID_FONT, NARC_font_large_gftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_NET_TEMP );
		
		mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, 
			ARCID_MESSAGE, NARC_message_d_matsu_dat, HEAPID_NET_TEMP);
#else
		fontHandle = GFL_FONT_Create(ARCID_FONT, NARC_font_large_gftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_NET_TEMP );
		
		mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, 
			ARCID_MESSAGE, NARC_message_d_matsu_dat, HEAPID_NET_TEMP);
#endif  //MULTI_BOOT_MAKE
		strbuf = GFL_MSG_CreateString(mm, DM_MSG_ERR001);
		PRINTSYS_Print(bmpdata, 0, 0, strbuf, fontHandle);

		GFL_STR_DeleteBuffer(strbuf);
		GFL_MSG_Delete(mm);
		
		GFL_FONT_Delete(fontHandle);
	}
	
	GFL_BMP_Delete(bmpdata);
}

