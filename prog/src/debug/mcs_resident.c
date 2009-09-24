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

#include "sound/pm_sndsys.h"
#include "debug/gf_mcs.h"

#define GFL_MCS_RESIDENT_ID	(9876543)

enum {
	MCSRSDCOMM_REQHEAPSTATUS = 1,
	MCSRSDCOMM_HEAPSTATUS,
};

typedef struct {
	u32 comm;
	u32 param;
}MCSRSDCOMM_HEADER;

#define MCSRSDCOMM_HEADER_SIZE (sizeof(MCSRSDCOMM_HEADER) - sizeof(u32))

void	GFL_MCS_Resident(void);
static u32	GetExistMemoryBlocksInfo( HEAPID heapID, u32 pBuf );

#define SENDBUF_SIZE (0x10000)
#define RECVBUF_SIZE (0x10000)
u8 MCSRSD_sendBuffer[SENDBUF_SIZE];
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
			{
				MCSRSDCOMM_HEADER* commHeader = (MCSRSDCOMM_HEADER*)MCSRSD_sendBuffer;
				u32 sendSize;
				int i;

				commHeader->comm = MCSRSDCOMM_HEAPSTATUS;

				// ヒープ情報作成＆転送
				for(i=0; i<HEAPID_CHILD_MAX-1; i++){
					ISDPrintSetBlockingMode(1);
					sendSize = GetExistMemoryBlocksInfo( i, (u32)&commHeader->param);
					if(sendSize){
						sendSize += MCSRSDCOMM_HEADER_SIZE;	// ヘッダ情報分加算
						OS_Printf("trans heapInfo ... size %x, heapID %d\n", sendSize, i);

						GFL_MCS_Write(GFL_MCS_RESIDENT_ID, MCSRSD_sendBuffer, sendSize);
						ISDPrintSetBlockingMode(0);
					}
				}
			}
			break;
		}
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

static int numMemoryTotal;

//子ヒープ用宣言
static const char* heapNameTable[HEAPID_CHILD_MAX] = { 
	//親ヒープ
	"GFL_HEAPID_SYSTEM",
	"GFL_HEAPID_APP",
	//常駐ヒープ
	"HEAPID_BG",
	"HEAPID_WORLD",
	"HEAPID_PROC",
	"HEAPID_APP_CONTROL",
	"HEAPID_COMMUNICATION",
  //アプリケーションヒープ
  "HEAPID_TITLE",
  "HEAPID_STARTMENU",
  "HEAPID_NETWORK_FIX",
  "HEAPID_NETWORK",
  "HEAPID_ITEMMENU",
  "HEAPID_WIFI",
  "HEAPID_IRC",
  "HEAPID_LAYOUT",
  "HEAPID_BTL_SYSTEM",
  "HEAPID_BTL_VIEW",
  "HEAPID_BTL_NET",
  "HEAPID_FIELDMAP",
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
  "HEAPID_WIFIP2PMATCH",
  "HEAPID_WIFIP2PMATCHEX",
  "HEAPID_WIFI_FOURMATCHVCHAT",
  "HEAPID_WIFI_FOURMATCH",
  "HEAPID_PALACE",
  "HEAPID_TRCARD_SYS",
  "HEAPID_TR_CARD",
  "HEAPID_OEKAKI",
  "HEAPID_PMS_INPUT_SYSTEM",
  "HEAPID_PMS_INPUT_VIEW",
  "HEAPID_MUSICAL_PROC",
  "HEAPID_MUSICAL_STRM",
  "HEAPID_MUSICAL_DRESSUP",
  "HEAPID_MUSICAL_STAGE",
  "HEAPID_MUSICAL_LOBBY",
  "HEAPID_WFLOBBY",
  "HEAPID_WFLBY_ROOM",
  "HEAPID_CODEIN",
  "HEAPID_WFLBY_ROOMGRA",
  "HEAPID_WIFINOTE",
  "HEAPID_CGEAR",
  "HEAPID_DEBUGWIN",
  "HEAPID_IRCAURA",
  "HEAPID_IRCRHYTHM",
  "HEAPID_SEL_MODE",
  "HEAPID_IRCCOMPATIBLE_SYSTEM",
  "HEAPID_IRCCOMPATIBLE",
  "HEAPID_IRCRESULT",
  "HEAPID_NEWSDRAW",
  "HEAPID_WLDTIMER",
  "HEAPID_DEBUG_MAKEPOKE",
  "HEAPID_BUCKET",
  "HEAPID_UNION",
  "HEAPID_POKE_STATUS",
  "HEAPID_IRCRANKING",
  "HEAPID_TOWNMAP",
  "HEAPID_BALANCE_BALL",
  "HEAPID_FOOTPRINT",
  "HEAPID_ANKETO",
  "HEAPID_WORLDTRADE",
  "HEAPID_MICTEST",
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
};

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
    strcpy( blockInfo->filename, "SYSTEM ALLOC" );
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
	strcpy( heapStatus->name, heapNameTable[heapID] );

  //拡張ヒープから確保したメモリブロック全て（Allocされたもの）に対し、指定した関数を呼ばせる
	numMemoryTotal = 0;
  NNS_FndVisitAllocatedForExpHeap
		(GFI_HEAP_GetHandle(heapID), HeapConflictVisitorFunc, (u32)&param);

	heapStatus->numMemoryTotal = numMemoryTotal;
	sendSize = (sizeof(MCSDATA_HEAPSTATUS) - sizeof(MCSDATA_HEAPBLOCKINFO)) 
						+  sizeof(MCSDATA_HEAPBLOCKINFO) * numMemoryTotal;
  if(sendSize > (SENDBUF_SIZE - MCSRSDCOMM_HEADER_SIZE)){
		OS_Printf("ヒープ情報量が多すぎて転送できません HEAPID = &d", heapID);
		return 0;		// オーバーフロー
	}
	//GFL_HEAP_DEBUG_PrintExistMemoryBlocks(heapID);
	return sendSize;
}

#endif

