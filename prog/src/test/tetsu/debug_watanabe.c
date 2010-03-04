//============================================================================================
/**
 * @file  debug_watanabe.c
 * @brief デバッグ＆テスト用アプリ（渡辺さん用）
 * @author  tamada
 * @date  2007.02.01
 */
//============================================================================================
#include "gflib.h"
#include "system/gfl_use.h"
#include "system\main.h"
#include "arc_def.h"
#include "message.naix"

#include "font/font.naix"
#include "print/printsys.h"
#include "print/str_tool.h"

#include "msg/msg_d_tetsu.h"
//============================================================================================
/**
 *
 *
 *
 *
 * @brief プロセス定義
 *
 *
 *
 *
 */
//============================================================================================
static void allocWork( GFL_PROC* proc );
static void freeWork( GFL_PROC* proc );
static void setNextProc( void* procwork );
static BOOL mainFunc( void* procwork );
//------------------------------------------------------------------
/**
 * @brief プロセスの初期化
 *
 * ここでヒープの生成や各種初期化処理を行う。
 * 初期段階ではmywkはNULLだが、GFL_PROC_AllocWorkを使用すると
 * 以降は確保したワークのアドレスとなる。
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugWatanabeMainProcInit
        ( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  allocWork(proc);

  return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief プロセスのメイン
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugWatanabeMainProcMain
        ( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  if( mainFunc(mywk) == FALSE ){
    return GFL_PROC_RES_FINISH;
  }
  return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief プロセスの終了処理
 *
 * 単に終了した場合、親プロセスに処理が返る。
 * GFL_PROC_SysSetNextProcを呼んでおくと、終了後そのプロセスに
 * 処理が遷移する。
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugWatanabeMainProcEnd
        ( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  setNextProc(mywk);
  freeWork(proc);

  return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_PROC_DATA DebugWatanabeMainProcData = {
  DebugWatanabeMainProcInit,
  DebugWatanabeMainProcMain,
  DebugWatanabeMainProcEnd,
};



//============================================================================================
/**
 *
 *
 *
 *
 * @brief デバッグ項目選択
 *
 *
 *
 *
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief   ＢＧ設定定義
 */
//------------------------------------------------------------------
#define TEXT_FRAME    (GFL_BG_FRAME3_S)
#define TEXT_DISPSIDE (PALTYPE_SUB_BG)
#define TEXT_PLTTID   (15)
#define PLTT_SIZ    (16*2)

//------------------------------------------------------------------
/**
 * @brief   項目リスト
 */
//------------------------------------------------------------------
#define LISTITEM_NAMESIZE (32)
typedef struct {
  u32                   msgID;
  FSOverlayID           procOvID;
  const GFL_PROC_DATA*  procData;
}DEBUGITEM_LIST;

FS_EXTERN_OVERLAY(watanabe_sample);
FS_EXTERN_OVERLAY(fieldmap);
extern const GFL_PROC_DATA DebugWatanabeSample5ProcData;

static const DEBUGITEM_LIST debugItemList[] = {
  {DEBUG_TETSU_MENU5, FS_OVERLAY_ID(watanabe_sample), &DebugWatanabeSample5ProcData},
};

//------------------------------------------------------------------
/**
 * @brief   構造体定義
 */
//------------------------------------------------------------------
typedef struct {
  HEAPID  heapID;
  u32   seq;
  u32   selectItem;

  PRINT_UTIL      printUtil;
  PRINT_QUE*      printQue;
  GFL_FONT*       fontHandle;
  GFL_MSGDATA*    msgManager;
  GFL_BMPWIN*     bmpwin;

  GFL_UI_TP_HITTBL  tpTable[NELEMS(debugItemList)+1];

  u32         wait;
}DEBUG_WATANABE_WORK;

#define NON_SELECT_ITEM (0xffffffff)
//------------------------------------------------------------------
/**
 * @brief   プロトタイプ宣言
 */
//------------------------------------------------------------------
static void systemSetup(DEBUG_WATANABE_WORK* dw);
static void systemFramework(DEBUG_WATANABE_WORK* dw);
static void systemDelete(DEBUG_WATANABE_WORK* dw);

static BOOL control(DEBUG_WATANABE_WORK* dw);
static void drawList(DEBUG_WATANABE_WORK* dw);
static void makeList(DEBUG_WATANABE_WORK* dw);

//------------------------------------------------------------------
/**
 * @brief
 */
//------------------------------------------------------------------
static void allocWork( GFL_PROC* proc )
{
  DEBUG_WATANABE_WORK* dw;
  HEAPID heapID = GFL_HEAPID_APP;

  dw = GFL_PROC_AllocWork(proc, sizeof(DEBUG_WATANABE_WORK), heapID);
  GFL_STD_MemClear(dw, sizeof(DEBUG_WATANABE_WORK));

  dw->heapID = heapID;
  dw->seq = 0;
  dw->selectItem = NON_SELECT_ITEM;
}

static void freeWork( GFL_PROC* proc )
{
  GFL_PROC_FreeWork(proc);
}

static void setNextProc( void* procwork )
{
  DEBUG_WATANABE_WORK* dw;
  u32 itemID;

  dw = procwork;
  itemID = dw->selectItem;

  GFL_PROC_SysSetNextProc(debugItemList[itemID].procOvID, debugItemList[itemID].procData, NULL);
}

//------------------------------------------------------------------
/**
 *
 * @brief メイン
 *
 */
//------------------------------------------------------------------
static BOOL mainFunc( void* procwork )
{
  DEBUG_WATANABE_WORK* dw;
  dw = procwork;

  switch(dw->seq){
  case 0:
    systemSetup(dw);
    makeList(dw);

    dw->seq++;
    break;

  case 1:
    if( control(dw) == FALSE ){ dw->seq++; }

    drawList(dw);
    systemFramework(dw);
    break;

  case 2:
    systemDelete(dw);
    return FALSE;
  }
  return TRUE;
}

//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief システムセットアップ
 *
 *
 *
 *
 *
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief ディスプレイ環境データ
 */
//------------------------------------------------------------------
///ＶＲＡＭバンク設定構造体
static const GFL_DISP_VRAM dispVram = {
  GX_VRAM_BG_128_A,       //メイン2DエンジンのBGに割り当て
  GX_VRAM_BGEXTPLTT_NONE,     //メイン2DエンジンのBG拡張パレットに割り当て
  GX_VRAM_SUB_BG_32_H,      //サブ2DエンジンのBGに割り当て
  GX_VRAM_SUB_BGEXTPLTT_NONE,   //サブ2DエンジンのBG拡張パレットに割り当て
  GX_VRAM_OBJ_64_E,       //メイン2DエンジンのOBJに割り当て
  GX_VRAM_OBJEXTPLTT_NONE,    //メイン2DエンジンのOBJ拡張パレットにに割り当て
  GX_VRAM_SUB_OBJ_NONE,     //サブ2DエンジンのOBJに割り当て
  GX_VRAM_SUB_OBJEXTPLTT_NONE,  //サブ2DエンジンのOBJ拡張パレットにに割り当て
  GX_VRAM_TEX_NONE,       //テクスチャイメージスロットに割り当て
  GX_VRAM_TEXPLTT_NONE,     //テクスチャパレットスロットに割り当て
  GX_OBJVRAMMODE_CHAR_1D_128K,  // メインOBJマッピングモード
  GX_OBJVRAMMODE_CHAR_1D_32K,   // サブOBJマッピングモード
};

static const GFL_BG_SYS_HEADER bgsysHeader = {
  GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_3D
};

static const GFL_BG_BGCNT_HEADER Textcont = {
  0, 0, 0x800, 0,
  GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x256,
  GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
};

//------------------------------------------------------------------
/**
 * @brief   ＢＧ設定＆データ転送
 */
//------------------------------------------------------------------
static void bg_init(HEAPID heapID)
{
  //VRAM設定
  GFL_DISP_SetBank(&dispVram);

  //ＢＧシステム起動
  GFL_BG_Init(heapID);

  //ＢＧモード設定
  GFL_BG_SetBGMode(&bgsysHeader);

  //ＢＧコントロール設定
  GFL_BG_SetBGControl(TEXT_FRAME, &Textcont, GFL_BG_MODE_TEXT);
  GFL_BG_SetPriority(TEXT_FRAME, 0);
  GFL_BG_SetVisible(TEXT_FRAME, VISIBLE_ON);

  GFL_BG_FillCharacter(TEXT_FRAME, 0, 1, 0);  // 先頭にクリアキャラ配置
  GFL_BG_ClearScreen(TEXT_FRAME);

  //ビットマップウインドウ起動
  GFL_BMPWIN_Init(heapID);

  //ディスプレイ面の選択
  GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_MAIN);
  GFL_DISP_SetDispOn();
}

static void bg_exit(void)
{
  GFL_BMPWIN_Exit();
  GFL_BG_FreeBGControl(TEXT_FRAME);
  GFL_BG_Exit();
}

//------------------------------------------------------------------
/**
 * @brief   セットアップ
 */
//------------------------------------------------------------------
static void systemSetup(DEBUG_WATANABE_WORK* dw)
{
  //基本セットアップ
  bg_init(dw->heapID);
  //フォント用パレット転送
  GFL_ARC_UTIL_TransVramPalette(  ARCID_FONT,
                  NARC_font_default_nclr,
                  TEXT_DISPSIDE,
                  TEXT_PLTTID * PLTT_SIZ,
                  PLTT_SIZ,
                  dw->heapID);
  //フォントハンドル作成
  dw->fontHandle = GFL_FONT_Create( ARCID_FONT,
                    NARC_font_large_gftr,
                    GFL_FONT_LOADTYPE_FILE,
                    FALSE,
                    dw->heapID);
  //プリントキューハンドル作成
  dw->printQue = PRINTSYS_QUE_Create(dw->heapID);

  dw->msgManager = GFL_MSG_Create
    (GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_tetsu_dat, dw->heapID);

  //描画用ビットマップ作成（画面全体）
  dw->bmpwin = GFL_BMPWIN_Create( TEXT_FRAME,
                  0, 0, 32, 24,
                  TEXT_PLTTID, GFL_BG_CHRAREA_GET_F );
  PRINT_UTIL_Setup(&dw->printUtil, dw->bmpwin);

  GFL_BMPWIN_MakeScreen(dw->bmpwin);
  GFL_BG_LoadScreenReq(TEXT_FRAME);
}

//------------------------------------------------------------------
/**
 * @brief   フレームワーク
 */
//------------------------------------------------------------------
static void systemFramework(DEBUG_WATANABE_WORK* dw)
{
  PRINT_UTIL_Trans(&dw->printUtil, dw->printQue);
  PRINTSYS_QUE_Main(dw->printQue);
}

//------------------------------------------------------------------
/**
 * @brief   破棄
 */
//------------------------------------------------------------------
static void systemDelete(DEBUG_WATANABE_WORK* dw)
{
  PRINTSYS_QUE_Clear(dw->printQue);
  GFL_BMPWIN_Delete(dw->bmpwin);

  GFL_MSG_Delete(dw->msgManager);

  PRINTSYS_QUE_Delete(dw->printQue);
  GFL_FONT_Delete(dw->fontHandle);

  bg_exit();
}


//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief リストコントロール
 *
 *
 *
 *
 *
 */
//============================================================================================
#define ITEM_PX (8*2)
#define ITEM_PY (8*2)
#define ITEM_SX (256-ITEM_PX*2)
#define ITEM_SY (16)

//------------------------------------------------------------------
/**
 * @brief 描画
 */
//------------------------------------------------------------------
static void drawList(DEBUG_WATANABE_WORK* dw)
{
  STRBUF*     strBuf;
  PRINTSYS_LSB  lsb;
  int i;

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(dw->bmpwin), 15);

  strBuf = GFL_STR_CreateBuffer(LISTITEM_NAMESIZE+1, dw->heapID);

  for( i=0; i<NELEMS(debugItemList); i++ ){
    GFL_MSG_GetString(dw->msgManager, debugItemList[i].msgID, strBuf);

    if( i == dw->selectItem ){ lsb = PRINTSYS_LSB_Make(3,2,15); }
    else { lsb = PRINTSYS_LSB_Make(1,2,15); }

    PRINT_UTIL_PrintColor(  &dw->printUtil, dw->printQue,
                ITEM_PX, i * ITEM_SY + ITEM_PY,
                strBuf, dw->fontHandle, lsb);
  }
  GFL_STR_DeleteBuffer(strBuf);
}

//------------------------------------------------------------------
/**
 * @brief タッチパネル判定テーブル作成
 */
//------------------------------------------------------------------
static void makeList(DEBUG_WATANABE_WORK* dw)
{
  int i;

  for( i=0; i<NELEMS(debugItemList); i++ ){
    //タッチパネル判定テーブル作成
    dw->tpTable[i].rect.top = i * ITEM_SY + ITEM_PY;
    dw->tpTable[i].rect.bottom = dw->tpTable[i].rect.top + (ITEM_SY-1);
    dw->tpTable[i].rect.left = ITEM_PX;
    dw->tpTable[i].rect.right = dw->tpTable[i].rect.left + (ITEM_SX-1);
  }
  dw->tpTable[NELEMS(debugItemList)].rect.top = GFL_UI_TP_HIT_END;//終了データ埋め込み
  dw->tpTable[NELEMS(debugItemList)].rect.bottom = 0;
  dw->tpTable[NELEMS(debugItemList)].rect.left = 0;
  dw->tpTable[NELEMS(debugItemList)].rect.right = 0;
}



//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief 動作関数
 *
 *
 *
 *
 *
 */
//============================================================================================
static  BOOL control(DEBUG_WATANABE_WORK* dw)
{
  if( dw->selectItem == NON_SELECT_ITEM ){
    //未選択
    u32 tblPos = GFL_UI_TP_HitTrg(dw->tpTable);
    if( tblPos != GFL_UI_TP_HIT_NONE ){
			if( debugItemList[tblPos].procData != NULL ){
				dw->selectItem = tblPos;
				dw->wait = 0;
			}
    }
    return TRUE;
  }
  if( dw->wait < 16 ){
    //選択後ウェイト処理
    dw->wait++;
    return TRUE;
  }
  return FALSE;
}



