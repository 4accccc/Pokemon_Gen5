//============================================================================================
/**
 * @file	mcs_resident.c
 * @brief	MCS：常駐監視
 * @author	
 * @date	
 */
//============================================================================================
#ifdef PM_DEBUG

#include "gflib.h"
#include "system/main.h"
#include "arc_def.h"
#include <isdbglib.h>

#include "sound/sound_manager.h"
#include "sound/pm_sndsys.h"
#include "debug/gf_mcs.h"

#include "debug/mcs_resident.h"

#define GFL_MCS_RESIDENT_ID	(9876543)

enum {
	MCSRSDCOMM_REQHEAPSTATUS = 1,
	MCSRSDCOMM_HEAPSTATUS,
	MCSRSDCOMM_SNDHEAPSTATUS,
	MCSRSDCOMM_REQVRAMSTATUS,
	MCSRSDCOMM_VRAMIMGSTATUS,
	MCSRSDCOMM_VRAMPLTSTATUS,
};

typedef struct {
	u32 comm;
	u32 param;
}MCSRSDCOMM_HEADER;

#define MCSRSDCOMM_HEADER_SIZE (sizeof(MCSRSDCOMM_HEADER) - sizeof(u32))

#define SENDBUF_SIZE (0x08000)
#define RECVBUF_SIZE (0x08000)
static u8 MCSRSD_sendBuffer[SENDBUF_SIZE];
static u8 MCSRSD_recvBuffer[RECVBUF_SIZE];
//============================================================================================
/**
 *
 * @brief	システム起動 & 終了
 *
 */
//============================================================================================
void	GFL_MCS_Resident(void)
{
	if(GFL_MCS_Read(GFL_MCS_RESIDENT_ID, MCSRSD_recvBuffer, RECVBUF_SIZE) == TRUE){
		MCSRSDCOMM_HEADER* recv = (MCSRSDCOMM_HEADER*)MCSRSD_recvBuffer;
		switch(recv->comm){
		case MCSRSDCOMM_REQHEAPSTATUS:
			GFL_MCS_Resident_SendHeapStatus();
			break;
		case MCSRSDCOMM_REQVRAMSTATUS:
			GFL_MCS_Resident_SendTexVramStatus();
			break;
		}
	}
	if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y){
		//GFL_MCS_Resident_SendTexVramStatus();
	}
}

//============================================================================================
/**
 *
 * @brief	ヒープ情報取得
 *		GFLにある関数（デバッグウインドウに出力する関数）、構造体
 *		をそのままコピーしているので整合性に常に注意すること。
 *		MCS用のプログラムをGFLに内蔵するのには抵抗があるため
 *		あえてこの方法で実装する
 *
 */
//============================================================================================
//----------------------------------------------------------------
/**
 *  ユーザーメモリブロックヘッダ定義（サイズ:MEMHEADER_USERINFO_SIZE = 26 ）
 */
//----------------------------------------------------------------
#define FILENAME_LEN  (18)
#define HEAPNAME_LEN  (32)
typedef struct {
  char  filename[ FILENAME_LEN ];		// 呼び出し先ソース名
  u16   lineNum;
}DEBUG_MEMHEADER;

typedef struct {
	u32		adrs;												// 確保アドレス
	u32		size;												// 確保サイズ
  u16   lineNo;											// 呼び出し先ソースライン
  char  filename[ FILENAME_LEN+1 ];	// 呼び出し先ソース名
	u8		pad[3];
}MCSDATA_HEAPBLOCKINFO;

typedef struct {
  u32		heapSize;										// ヒープ全体サイズ
	u32		totalFreeSize;							// 残りのヒープサイズ
	u32		allocatableSize;						// 一度に確保できる最大サイズ
	u16		heapID;											// ヒープID
	u16		numMemoryBlocks;						// 現在のブロック確保数
	u16		numMemoryTotal;							// データ総数（ブロック数＋子ヒープ数）
	u8		pad[2];
  char  name[ HEAPNAME_LEN ];				// ヒープ名
	MCSDATA_HEAPBLOCKINFO	blockInfo;	// 各ブロック情報（以下ブロック数分）
}MCSDATA_HEAPSTATUS;

#define BGMPLAYER_NUM (6)	// ref. sound/sound_manager.h
typedef struct {
  u32		heapSize;											// ヒープ全体サイズ
	u32		heapRemainsAfterSys;					// システムファイルロード後の残ヒープサイズ
	u32		heapRemainsAfterPlayer;				// プレーヤー設定後の残ヒープサイズ
	u32		heapRemainsAfterPresetSE;			// 常駐SE設定後の残ヒープサイズ
	u32		sePlayerNum;									// 設定SEプレーヤー数
	u32		totalFreeSize;								// 残りのヒープサイズ
	u32		playerSoundID[BGMPLAYER_NUM];	// 現在使用されているBGMプレーヤー内容（soundID）
}MCSDATA_SNDHEAPSTATUS;

static int numMemoryTotal;

//子ヒープ用宣言
static const char* heapNameTable[HEAPID_CHILD_MAX] = { 
	//親ヒープ
	"GFL_HEAPID_SYSTEM",
	"GFL_HEAPID_APP",
	//常駐ヒープ
	"HEAPID_BG",		//2
	"HEAPID_WORLD",
	"HEAPID_PROC",
	"HEAPID_APP_CONTROL",
	"HEAPID_COMMUNICATION",
	"HEAPID_SAVE",
	"HEAPID_NET_ERR",
  //アプリケーションヒープ
  "HEAPID_TITLE",
  "HEAPID_STARTMENU",
  "HEAPID_NETWORK_FIX",
  "HEAPID_NETWORK",		//12
  "HEAPID_ITEMMENU",
  "HEAPID_WIFI",
  "HEAPID_IRC",
  "HEAPID_LAYOUT",
  "HEAPID_BTL_SYSTEM",
  "HEAPID_BTL_VIEW",
  "HEAPID_BTL_NET",
  "HEAPID_FIELDMAP",	//20
  "HEAPID_TITLE_DEMO",
  "HEAPID_POKELIST",
  "HEAPID_TRADE",
  "HEAPID_BALLOON",
  "HEAPID_COMPATI",
  "HEAPID_COMPATI_CONTROL",
  "HEAPID_CONFIG",
  "HEAPID_MYSTERYGIFT",
  "HEAPID_NAME_INPUT",
  "HEAPID_IRCBATTLE",
  "HEAPID_WIFICLUB",
  "HEAPID_WIFIP2PMATCH",	//32
  "HEAPID_WIFIP2PMATCHEX",
  "HEAPID_WIFI_FOURMATCHVCHAT",
  "HEAPID_WIFI_FOURMATCH",
  "HEAPID_PALACE",
  "HEAPID_TRCARD_SYS",
  "HEAPID_TR_CARD",
  "HEAPID_OEKAKI",
  "HEAPID_PMS_INPUT_SYSTEM",
  "HEAPID_PMS_INPUT_VIEW",
  "HEAPID_MUSICAL_PROC",	//42
  "HEAPID_MUSICAL_STRM",
  "HEAPID_MUSICAL_DRESSUP",
  "HEAPID_MUSICAL_STAGE",
  "HEAPID_MUSICAL_LOBBY",
  "HEAPID_WFLOBBY",
  "HEAPID_WFLBY_ROOM",
  "HEAPID_CODEIN",
  "HEAPID_WFLBY_ROOMGRA",
  "HEAPID_WIFINOTE",
  "HEAPID_CGEAR",	//52
  "HEAPID_DEBUGWIN",
  "HEAPID_IRCAURA",
  "HEAPID_IRCRHYTHM",
  "HEAPID_SEL_MODE",
  "HEAPID_IRCCOMPATIBLE_SYSTEM",
  "HEAPID_IRCCOMPATIBLE",
  "HEAPID_IRCRESULT",
  "HEAPID_NEWSDRAW",
  "HEAPID_WLDTIMER",
  "HEAPID_DEBUG_MAKEPOKE",	//62
  "HEAPID_BUCKET",
  "HEAPID_UNION",
  "HEAPID_POKE_STATUS",
  "HEAPID_IRCRANKING",
  "HEAPID_TOWNMAP",
  "HEAPID_BALANCE_BALL",
  "HEAPID_FOOTPRINT",
  "HEAPID_ANKETO",
  "HEAPID_WORLDTRADE",
  "HEAPID_MICTEST",		//72
  "HEAPID_MUSICAL_SHOT",
  "HEAPID_BOX_SYS",
  "HEAPID_BOX_APP",
  "HEAPID_IRC_BATTLE",
  "HEAPID_BATTLE_CHAMPIONSHIP",
  "HEAPID_WAZAOSHIE",
  "HEAPID_FLD3DCUTIN",
  "HEAPID_PMS_SELECT",	//80
  "HEAPID_BTLRET_SYS",
  "HEAPID_BTLRET_VIEW",
  "HEAPID_GAMEOVER_MSG",
  "HEAPID_LOCAL_TVT",
	"HEAPID_WIFIBATTLEMATCH_SYS",
	"HEAPID_WIFIBATTLEMATCH_CORE",
	"HEAPID_WIFIBATTLEMATCH_SUB",
	"HEAPID_BATTLE_RECORDER_SYS",
	"HEAPID_BATTLE_RECORDER_CORE",
	"HEAPID_GDS_MAIN",
	"HEAPID_GDS_CONNECT",	//91
	"HEAPID_MULTIBOOT",
	"HEAPID_MULTIBOOT_DATA",      
	"HEAPID_MB_BOX",      
	"HEAPID_MB_CAPTURE",      
	"HEAPID_MONOLITH",
	"HEAPID_GAMESYNC",
	"HEAPID_MAILSYS",         
	"HEAPID_MAILVIEW",         
	"HEAPID_MAILBOX_SYS",
	"HEAPID_MAILBOX_APP",     //101
	"HEAPID_PDWACC",
	"HEAPID_SHINKA_DEMO",
	"HEAPID_ZUKAN_SYS",
	"HEAPID_ZUKAN_TOROKU",
	"HEAPID_ZUKAN_LIST",
	"HEAPID_DEMO3D",
	"HEAPID_COMM_TVT",
	"HEAPID_CTVT_CAMERA",   
	"HEAPID_INTRO",
	"HEAPID_FIELD_SUBSCREEN",

  //デバッグ用ヒープ
  "HEAPID_WATANABE_DEBUG",
  "HEAPID_SOGABE_DEBUG",
  "HEAPID_TAMADA_DEBUG",
  "HEAPID_OHNO_DEBUG",
  "HEAPID_TAYA_DEBUG",
  "HEAPID_TAYA_DEBUG_SUB",
  "HEAPID_NAKAHIRO_DEBUG",
  "HEAPID_TOMOYA_DEBUG",
  "HEAPID_MATSUDA_DEBUG",
  "HEAPID_GOTO_DEBUG",
  "HEAPID_ARIIZUMI_DEBUG",
  "HEAPID_NAGI_DEBUG",
  "HEAPID_NAGI_DEBUG_SUB",
  "HEAPID_OBATA_DEBUG",
  "HEAPID_UI_DEBUG",
  "HEAPID_BTL_DEBUG_SYS",
  "HEAPID_BTL_DEBUG_VIEW",
  "HEAPID_HOSAKA_DEBUG",
  "HEAPID_WIFIMATCH_DEBUG",
};

static u32	GetExistMemoryBlocksInfo( HEAPID heapID, u32 pBuf );

BOOL	GFL_MCS_Resident_SendHeapStatus(void)
{
	MCSRSDCOMM_HEADER* commHeader = (MCSRSDCOMM_HEADER*)MCSRSD_sendBuffer;
	u32 sendSize;
	int i;

	commHeader->comm = MCSRSDCOMM_HEAPSTATUS;
	commHeader->param = 0;
	for(i=0; i<HEAPID_CHILD_MAX-1; i++){
		if(GFI_HEAP_GetHeapTotalSize(i) != 0){ commHeader->param++; }
	}
	sendSize = sizeof(MCSRSDCOMM_HEADER_SIZE) + sizeof(u32);
	GFL_MCS_Write(GFL_MCS_RESIDENT_ID, MCSRSD_sendBuffer, sendSize);

	// ヒープ情報作成＆転送
	for(i=0; i<HEAPID_CHILD_MAX-1; i++){
		sendSize = GetExistMemoryBlocksInfo( i, (u32)MCSRSD_sendBuffer);
		if(sendSize){
			GFL_MCS_Write(GFL_MCS_RESIDENT_ID, MCSRSD_sendBuffer, sendSize);
			//OS_Printf("send Heap Status %d, %d\n", i, sendSize);
		}
	}

	commHeader->comm = MCSRSDCOMM_SNDHEAPSTATUS;
	{
		MCSDATA_SNDHEAPSTATUS* shs = (MCSDATA_SNDHEAPSTATUS*)&commHeader->param;
		shs->heapSize = PMSND_GetSndHeapSize();
		shs->heapRemainsAfterSys = PMSND_GetSndHeapRemainsAfterSys();
		shs->heapRemainsAfterPlayer = PMSND_GetSndHeapRemainsAfterPlayer();
		shs->heapRemainsAfterPresetSE = PMSND_GetSndHeapRemainsAfterPresetSE();
		shs->sePlayerNum = PMSND_GetSEPlayerNum();
		shs->totalFreeSize = PMSND_GetSndHeapFreeSize();
		for(i=0; i<BGMPLAYER_NUM; i++){
			shs->playerSoundID[i] = SOUNDMAN_GetHierarchyPlayerSoundIdxByPlayerID(i);	
		}
	}
	sendSize = sizeof(MCSRSDCOMM_HEADER_SIZE) + sizeof(MCSDATA_SNDHEAPSTATUS);
	GFL_MCS_Write(GFL_MCS_RESIDENT_ID, MCSRSD_sendBuffer, sendSize);

	return TRUE;
}

//----------------------------------------------------------------------------------
/**
 * メモリブロックヘッダに保存されているファイル名をバッファにコピー
 *
 * @param   dst      コピー先バッファ
 * @param   header   メモリブロックヘッダ
 *
 * @retval  BOOL     正しくファイル名が取得できたらTRUE
 *
 * @li NitroSystem Allocator 経由でメモリ確保されているケースなどでは
 *			ファイル名が取得できない可能性もある
 */
//----------------------------------------------------------------------------------
static BOOL GetFileName( char* dst, const char* src )
{
  int i, len;

  // 終端コードナシで領域めいっぱいファイル名に使ってるため、
  // 最後にNULL文字を追加してやる必要がある
  for(i=0; i<FILENAME_LEN; ++i){
    if( src[i] == '\0' ){ break; }
    dst[i] = src[i];
  }
  dst[i] = '\0';
	len = i;

  // いきなり終端コードなら正しいファイル名ではないと判定
  if( len == 0 ){
    return FALSE;
  }
  // 非ASCIIコードも正しいファイル名ではないと判定
  for(i = 0; i<len; ++i){
    if( ((u8)dst[i] < 0x20) || ((u8)dst[i]) > 0x7e){
      return FALSE;
    }
  }
  // 領域一杯を使い切っていない AND 文字コード '.' が含まれていないならファイル名ではない
  if( len < FILENAME_LEN ){
    for(i=0; i<len; ++i){
      if( (u8)(dst[i]) == '.' ){ break; }
    }
    if( i == len ){
      return FALSE;
    }
  }
  return TRUE;
}

//----------------------------------------------------------------------------------
static void HeapConflictVisitorFunc(void* memBlock, NNSFndHeapHandle heapHandle, u32 param)
{
	MCSDATA_HEAPBLOCKINFO** inParam = (MCSDATA_HEAPBLOCKINFO**)param;
	MCSDATA_HEAPBLOCKINFO* blockInfo = *inParam;

  void* memory = (u8*)memBlock + MEMHEADER_SIZE;
  DEBUG_MEMHEADER* header = (DEBUG_MEMHEADER*)GFI_HEAP_GetMemheaderUserinfo( memory );

	blockInfo->adrs = (u32)memory;
	blockInfo->size = NNS_FndGetSizeForMBlockExpHeap(memBlock)+0x10;

  if( GetFileName(blockInfo->filename, header->filename) == FALSE ){
    STD_StrCpy( blockInfo->filename, "SYSTEM ALLOC" );
    blockInfo->lineNo = 0;
  }else{
    blockInfo->lineNo = header->lineNum;
  }
	(*inParam)++;
	numMemoryTotal++;		// 子ヒープはブロック数に含められないので別枠でカウントする
}

//----------------------------------------------------------------------------------
static u32	GetExistMemoryBlocksInfo( HEAPID heapID, u32 pBuf )
{
	MCSDATA_HEAPSTATUS* heapStatus = (MCSDATA_HEAPSTATUS*)pBuf;
	MCSDATA_HEAPBLOCKINFO* param = &heapStatus->blockInfo;
  u32 sendSize;

  heapStatus->heapSize = GFI_HEAP_GetHeapTotalSize(heapID);
	if(heapStatus->heapSize == 0){ return 0; }

	heapStatus->totalFreeSize = GFI_HEAP_GetHeapFreeSize(heapID);
	heapStatus->allocatableSize = GFI_HEAP_GetHeapAllocatableSize(heapID);
	heapStatus->heapID = heapID;
	heapStatus->numMemoryBlocks = GFI_HEAP_GetBlockCount(heapID);
	STD_StrCpy( heapStatus->name, heapNameTable[heapID] );

  //拡張ヒープから確保したメモリブロック全て（Allocされたもの）に対し、指定した関数を呼ばせる
	numMemoryTotal = 0;
  NNS_FndVisitAllocatedForExpHeap
		(GFI_HEAP_GetHandle(heapID), HeapConflictVisitorFunc, (u32)&param);

	heapStatus->numMemoryTotal = numMemoryTotal;
	sendSize = (sizeof(MCSDATA_HEAPSTATUS) - sizeof(MCSDATA_HEAPBLOCKINFO)) 
						+  sizeof(MCSDATA_HEAPBLOCKINFO) * numMemoryTotal;
  if(sendSize > SENDBUF_SIZE){
		OS_Printf("ヒープ情報量が多すぎて転送できません HEAPID = &d", heapID);
		return 0;		// オーバーフロー
	}
	//GFL_HEAP_DEBUG_PrintExistMemoryBlocks(heapID);
	return sendSize;
}


//============================================================================================
/**
 *
 * @brief	テクスチャVRAM情報取得
 *
 */
//============================================================================================
typedef struct {
	u16		type;			// VRAM割り振りタイプ
	u16		size1;		// ブロック1サイズ(>>12)
	u16		size2;		// ブロック2サイズ(>>12)
	u8		pad[2];
}MCSDATA_TEXVRAMSTATUS;

//----------------------------------------------------------------------------
// LCDCスペース内配置データテーブル
//----------------------------------------------------------------------------
#define TEX_ADRSDEF_A		((u16)(HW_LCDC_VRAM_A >> 12))
#define TEX_ADRSDEF_B		((u16)(HW_LCDC_VRAM_B >> 12))
#define TEX_ADRSDEF_C		((u16)(HW_LCDC_VRAM_C >> 12))
#define TEX_ADRSDEF_D		((u16)(HW_LCDC_VRAM_D >> 12))
#define TEX_SIZEDEF_A		((u16)(HW_VRAM_A_SIZE >> 12))
#define TEX_SIZEDEF_B		((u16)(HW_VRAM_B_SIZE >> 12))
#define TEX_SIZEDEF_C		((u16)(HW_VRAM_C_SIZE >> 12))
#define TEX_SIZEDEF_D		((u16)(HW_VRAM_D_SIZE >> 12))
#define TEX_SIZEDEF_AB	((u16)((HW_VRAM_A_SIZE + HW_VRAM_B_SIZE) >> 12))
#define TEX_SIZEDEF_BC	((u16)((HW_VRAM_B_SIZE + HW_VRAM_C_SIZE) >> 12))
#define TEX_SIZEDEF_CD	((u16)((HW_VRAM_C_SIZE + HW_VRAM_D_SIZE) >> 12))
#define TEX_SIZEDEF_ABC	((u16)((HW_VRAM_A_SIZE + HW_VRAM_B_SIZE + HW_VRAM_C_SIZE) >> 12))
#define TEX_SIZEDEF_BCD	((u16)((HW_VRAM_D_SIZE + HW_VRAM_C_SIZE + HW_VRAM_D_SIZE) >> 12))
#define TEX_SIZEDEF_ABCD ((u16)((HW_VRAM_A_SIZE+HW_VRAM_B_SIZE+HW_VRAM_C_SIZE+HW_VRAM_D_SIZE)>>12))
static const struct {
	u16     blk1adrs;	// 12 bit shift
	u16     blk1size;	// 12 bit shift
	u16     blk2adrs;	// 12 bit shift
	u16     blk2size;	// 12 bit shift
} sTexImgStartAddrTable[16] = {
	{0, 0, 0, 0},																										// GX_VRAM_TEX_NONE
	{TEX_ADRSDEF_A, TEX_SIZEDEF_A,	0, 0},													// GX_VRAM_TEX_0_A
	{TEX_ADRSDEF_B, TEX_SIZEDEF_B,	0, 0},													// GX_VRAM_TEX_0_B
	{TEX_ADRSDEF_A,	TEX_SIZEDEF_AB,	0, 0},													// GX_VRAM_TEX_01_AB
	{TEX_ADRSDEF_C,	TEX_SIZEDEF_C,	0, 0},													// GX_VRAM_TEX_0_C
	{TEX_ADRSDEF_A, TEX_SIZEDEF_A,	TEX_ADRSDEF_C, TEX_SIZEDEF_C},	// GX_VRAM_TEX_01_AC
	{TEX_ADRSDEF_B, TEX_SIZEDEF_BC,	0, 0},													// GX_VRAM_TEX_01_BC
	{TEX_ADRSDEF_A, TEX_SIZEDEF_ABC, 0, 0},													// GX_VRAM_TEX_012_ABC
	{TEX_ADRSDEF_D, TEX_SIZEDEF_D,	0, 0},													// GX_VRAM_TEX_0_D
	{TEX_ADRSDEF_A,	TEX_SIZEDEF_A,	TEX_ADRSDEF_D, TEX_SIZEDEF_D},	// GX_VRAM_TEX_01_AD
	{TEX_ADRSDEF_B,	TEX_SIZEDEF_B,	TEX_ADRSDEF_D, TEX_SIZEDEF_D},	// GX_VRAM_TEX_01_BD
	{TEX_ADRSDEF_A,	TEX_SIZEDEF_AB,	TEX_ADRSDEF_D, TEX_SIZEDEF_D},	// GX_VRAM_TEX_012_ABD
	{TEX_ADRSDEF_C, TEX_SIZEDEF_CD,	0, 0},													// GX_VRAM_TEX_01_CD
	{TEX_ADRSDEF_A, TEX_SIZEDEF_A,	TEX_ADRSDEF_C, TEX_SIZEDEF_CD},	// GX_VRAM_TEX_012_ACD
	{TEX_ADRSDEF_B, TEX_SIZEDEF_BCD, 0, 0},													// GX_VRAM_TEX_012_BCD
	{TEX_ADRSDEF_A, TEX_SIZEDEF_ABCD, 0, 0},												// GX_VRAM_TEX_0123_ABCD
};

#define TEXPLT_ADRSDEF_E		((u16)(HW_LCDC_VRAM_E >> 12))
#define TEXPLT_ADRSDEF_F		((u16)(HW_LCDC_VRAM_F >> 12))
#define TEXPLT_ADRSDEF_G		((u16)(HW_LCDC_VRAM_G >> 12))
#define TEXPLT_SIZEDEF_E		((u16)(HW_VRAM_E_SIZE >> 12))
#define TEXPLT_SIZEDEF_F		((u16)(HW_VRAM_F_SIZE >> 12))
#define TEXPLT_SIZEDEF_G		((u16)(HW_VRAM_G_SIZE >> 12))
#define TEXPLT_SIZEDEF_EF		((u16)((HW_VRAM_E_SIZE + HW_VRAM_G_SIZE) >> 12))
#define TEXPLT_SIZEDEF_FG		((u16)((HW_VRAM_F_SIZE + HW_VRAM_G_SIZE) >> 12))
#define TEXPLT_SIZEDEF_EFG	((u16)((HW_VRAM_E_SIZE + HW_VRAM_F_SIZE + HW_VRAM_G_SIZE) >> 12))

static const struct {
	u16     blk1adrs;	// 12 bit shift
	u16     blk1size;	// 12 bit shift
} sTexPltStartAddrTable[16] = {
	{0, 0},																				// GX_VRAM_TEXPLTT_NONE
	{TEXPLT_ADRSDEF_F,		TEXPLT_SIZEDEF_F},			// GX_VRAM_TEXPLTT_0_F
	{TEXPLT_ADRSDEF_G,		TEXPLT_SIZEDEF_G},			// GX_VRAM_TEXPLTT_0_G
	{TEXPLT_ADRSDEF_F,		TEXPLT_SIZEDEF_FG},			// GX_VRAM_TEXPLTT_01_FG
	{TEXPLT_ADRSDEF_E,		TEXPLT_SIZEDEF_E},			// GX_VRAM_TEXPLTT_0123_E
	{TEXPLT_ADRSDEF_E,		TEXPLT_SIZEDEF_EF},			// GX_VRAM_TEXPLTT_01234_EF
	{0, 0},																				// dummy
	{TEXPLT_ADRSDEF_E,	TEXPLT_SIZEDEF_EFG},			// GX_VRAM_TEXPLTT_012345_EFG
};

BOOL	GFL_MCS_Resident_SendTexVramStatus(void)
{
	MCSRSDCOMM_HEADER* commHeader = (MCSRSDCOMM_HEADER*)MCSRSD_sendBuffer;
	MCSDATA_TEXVRAMSTATUS* vStatus;
	u32 sendSize;

	// VRAMイメージ転送
	{
		GXVRamTex stTexImg = (GXVRamTex)(0);
		u32 blk1adrs, blk2adrs, blk1size, blk2size;

		commHeader->comm = MCSRSDCOMM_VRAMIMGSTATUS;
		vStatus = (MCSDATA_TEXVRAMSTATUS*)&commHeader->param;

		//テクスチャスロットをLCDCスペースに割り振る + 現在の割り振り状態を取得
		stTexImg = GX_ResetBankForTex();
		// 定義データがそのままbitfieldとして使用できるのでそのまま代入
		vStatus->type = stTexImg;

		// VRAM情報作成＆転送
		blk1adrs = (u32)sTexImgStartAddrTable[vStatus->type].blk1adrs;
		blk1size = (u32)sTexImgStartAddrTable[vStatus->type].blk1size;
		blk2adrs = (u32)sTexImgStartAddrTable[vStatus->type].blk2adrs;
		blk2size = (u32)sTexImgStartAddrTable[vStatus->type].blk2size;

		vStatus->size1 = blk1size;
		vStatus->size2 = blk2size;
		sendSize = sizeof(MCSRSDCOMM_HEADER_SIZE) + sizeof(MCSDATA_TEXVRAMSTATUS);
		GFL_MCS_Write(GFL_MCS_RESIDENT_ID, MCSRSD_sendBuffer, sendSize);

		if(vStatus->size1){
			//OS_Printf("texvram_img send 1\n");
			GFL_MCS_Write(GFL_MCS_RESIDENT_ID, (void*)(blk1adrs << 12), (blk1size << 12));
		}
		if(vStatus->size2){
			//OS_Printf("texvram_img send 2\n");
			GFL_MCS_Write(GFL_MCS_RESIDENT_ID, (void*)(blk2adrs << 12), (blk2size << 12));
		}
		// テクスチャスロットの復帰
		GX_SetBankForTex(stTexImg);
	}
	// VRAMパレット転送
	{
		GXVRamTexPltt stTexPlt = (GXVRamTexPltt)(0);
		u32 blk1adrs, blk1size;

		commHeader->comm = MCSRSDCOMM_VRAMPLTSTATUS;
		vStatus = (MCSDATA_TEXVRAMSTATUS*)&commHeader->param;

		//パレットスロットをLCDCスペースに割り振る + 現在の割り振り状態を取得
		stTexPlt = GX_ResetBankForTexPltt();
		// bitfieldデータとして変換する。
		switch(stTexPlt){
		default:
		case GX_VRAM_TEXPLTT_NONE:
			vStatus->type = 0;
			break;
		case GX_VRAM_TEXPLTT_0_F:
			vStatus->type = 1;
			break;
		case GX_VRAM_TEXPLTT_0_G:
			vStatus->type = 2;
			break;
		case GX_VRAM_TEXPLTT_01_FG:
			vStatus->type = 3;
			break;
		case GX_VRAM_TEXPLTT_0123_E:
			vStatus->type = 4;
			break;
		case GX_VRAM_TEXPLTT_01234_EF:
			vStatus->type = 5;
			break;
		case GX_VRAM_TEXPLTT_012345_EFG:
			vStatus->type = 7;
			break;
		}
		// VRAM情報作成＆転送
		blk1adrs = (u32)sTexPltStartAddrTable[vStatus->type].blk1adrs;
		blk1size = (u32)sTexPltStartAddrTable[vStatus->type].blk1size;

		vStatus->size1 = blk1size;
		vStatus->size2 = 0;
		sendSize = sizeof(MCSRSDCOMM_HEADER_SIZE) + sizeof(MCSDATA_TEXVRAMSTATUS);
		GFL_MCS_Write(GFL_MCS_RESIDENT_ID, MCSRSD_sendBuffer, sendSize);
		if(vStatus->size1){
			//OS_Printf("texvram_plt send 1\n");
			GFL_MCS_Write(GFL_MCS_RESIDENT_ID, (void*)(blk1adrs << 12), (blk1size << 12));
		}
		// パレットスロットの復帰
		GX_SetBankForTexPltt(stTexPlt);
	}
	return TRUE;
}


#endif

