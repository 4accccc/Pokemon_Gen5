//============================================================================================
/**
* @file   d_font_view.c
* @brief  フォントビューア
* @author ichibayashi
* @date   2010.05.20
*/
//============================================================================================

#ifdef PM_DEBUG

//ライブラリ
#include <gflib.h>
#include <msgdata.h>
#include <wchar.h>

//システム
#include "system/main.h"
#include "system/gfl_use.h"
#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"

//Proc
#include "title/title.h"

//アーカイブ
#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"
#include "msg/msg_d_ncl.h"

// 文字セットをあらわす配列 UserInputCode、DefinedCode
#include "d_font_view.cdat"

//======================================================================
//  define
//======================================================================

#define VIEWHEAD_BUFSIZE (128)
#define VIEWBODY_LINESIZE (16)
#define VIEWBODY_LINECOUNT (8)
#define VIEWBODY_INDEXCOUNT (VIEWBODY_LINESIZE*VIEWBODY_LINECOUNT)  // 1画面に表示する文字数
#define VIEWBODY_BUFSIZE ((VIEWBODY_LINESIZE+2)*VIEWBODY_LINECOUNT+1)  // (1行16文字+右端を分かりやすくするための文字+改行)*8行+EOM

#define WORDSET_BUFNUM (5)
#define WORDSET_BUFLEN (64)
#define WORDINDEX_CODEBEGIN (0)
#define WORDINDEX_CODEEND (1)
#define WORDINDEX_FONTNAME (2)
#define WORDINDEX_INDEXMODE (3)
#define WORDINDEX_LOADTYPE (4)

// 上下ボタンで文字コードが変化する量
#define CODEMOVE_COUNT (VIEWBODY_LINESIZE*VIEWBODY_LINECOUNT)

// 表示しているフォント・範囲の情報の位置
#define VIEWHEAD_PX (1)
#define VIEWHEAD_PY (0)

// フォントを表示する位置
#define VIEWBODY_PX (1)
#define VIEWBODY_PY (24)

// ヘルプを表示する位置
#define HELP_PX (8)
#define HELP_PY (0)

#define BG_PLANE_VIEW GFL_BG_FRAME1_M
#define BG_PLANE_HELP GFL_BG_FRAME1_S
#define FONTVIEWER_PLT_FONT (0)
#define FONTVIEWER_CLEAR_COLOR (15)

#define FONTID_MAX (4)

#define CR_CODE (0xfffe)


// 文字インデックスが表す文字を決定するモード
typedef enum
{
  INDEXMODE_DEFINED = 0,  // syachiで定義されている文字
  INDEXMODE_USERINPUT,    // ユーザー入力可能な文字
  INDEXMODE_ALL,         // 全Unicode (U+0000～U+FFFF)
  INDEXMODE_MAX = INDEXMODE_ALL,
}FONTVIEWER_INDEXMODE;

//======================================================================
//  typedef struct
//======================================================================
typedef struct
{
  HEAPID heapId;

  GFL_FONT      *fontDefault;
  GFL_FONT      *fontView;
  GFL_BMPWIN    *bmpWinView;  //上画面フォント表示
  GFL_BMPWIN    *bmpWinHelp;  //下画面ヘルプ
  GFL_MSGDATA   *msgman;
  WORDSET       *wordset;

  FONTVIEWER_INDEXMODE indexMode;
  int           indexBegin;    // 現在表示している文字の始点
  int           indexEnd;      // 現在表示している文字の終点
  u32           fontId;       // 現在表示しているフォントId
  int           fontLoadType; // フォントが常駐かどうか
  STRBUF        *viewHeadStr; // 表示しているフォント・範囲のタグ代入前文字列
  STRBUF        *viewHeadBuf; // 表示しているフォント・範囲の情報
  STRBUF        *viewBodyBuf; // 表示している文字列
} FONTVIEWER_WORK;

//======================================================================
//  proto
//======================================================================
static void InitGraphic( FONTVIEWER_WORK *work );
static void InitBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane );
static GFL_BMPWIN *CreateFullScreenBmpWin( FONTVIEWER_WORK *work, u8 bgPlane );
static void PrintHelp(FONTVIEWER_WORK *work, GFL_BMPWIN *dst);
static void PrintViewHead(FONTVIEWER_WORK *work, GFL_BMPWIN *dst);
static void PrintViewBody(FONTVIEWER_WORK *work, GFL_BMPWIN *dst);
static void PrintView(FONTVIEWER_WORK *work, GFL_BMPWIN *dst);

static STRCODE IndexToCode(FONTVIEWER_WORK *work, int index);
static int GetMaxIndex(FONTVIEWER_WORK *work);
static void SetIndex(FONTVIEWER_WORK *work, int new_begin);
static void MoveIndex(FONTVIEWER_WORK *work, int diff);

static void RegisterMessage(WORDSET* wordset, u32 bufID, GFL_MSGDATA *msgman, int msgid);

static GFL_PROC_RESULT FontViewerProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT FontViewerProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT FontViewerProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk);

//--------------------------------------------------------------------------
//  描画周り初期化
//--------------------------------------------------------------------------
static void FONTVIEWER_InitGraphic( FONTVIEWER_WORK *work )
{
  static const GFL_DISP_VRAM vramBank = {
    GX_VRAM_BG_128_A,       // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
    GX_VRAM_SUB_BG_128_C,     // サブ2DエンジンのBG
    GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
    GX_VRAM_OBJ_128_B,        // メイン2DエンジンのOBJ
    GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
    GX_VRAM_SUB_OBJ_128_D,      // サブ2DエンジンのOBJ
    GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
    GX_VRAM_TEX_NONE,       // テクスチャイメージスロット
    GX_VRAM_TEXPLTT_NONE,     // テクスチャパレットスロット
    GX_OBJVRAMMODE_CHAR_1D_32K,   // メインOBJマッピングモード
    GX_OBJVRAMMODE_CHAR_1D_32K,   // サブOBJマッピングモード
  };

  static const GFL_BG_SYS_HEADER sysHeader = {
    GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
  };

  static const GFL_BG_BGCNT_HEADER bgContOnePlane = {
    0, 0, 0x800, 0,
    GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
    GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
  };

  GX_SetMasterBrightness(0);
  GXS_SetMasterBrightness(0);
  GFL_DISP_GX_SetVisibleControlDirect(0);   //全BG&OBJの表示OFF
  GFL_DISP_GXS_SetVisibleControlDirect(0);

  GFL_DISP_SetBank( &vramBank );
  GFL_BG_SetBGMode( &sysHeader );
  GFL_BG_Init( work->heapId );

  InitBgFunc( &bgContOnePlane , BG_PLANE_VIEW );
  InitBgFunc( &bgContOnePlane , BG_PLANE_HELP );

  GFL_BMPWIN_Init( work->heapId );
}

//--------------------------------------------------------------------------
//  Bg初期化 機能部
//--------------------------------------------------------------------------
static void InitBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane )
{
  GFL_BG_SetBGControl( bgPlane, bgCont, GFL_BG_MODE_TEXT );
  GFL_BG_SetVisible( bgPlane, VISIBLE_ON );
  GFL_BG_ClearFrame( bgPlane );
  GFL_BG_LoadScreenReq( bgPlane );
}

//--------------------------------------------------------------------------
//  全画面BMPWIN作成
//--------------------------------------------------------------------------
static GFL_BMPWIN *CreateFullScreenBmpWin( FONTVIEWER_WORK *work, u8 bgPlane )
{
  GFL_BMPWIN *bmp_win;

  bmp_win = GFL_BMPWIN_Create(bgPlane, 0, 0, 32, 24, FONTVIEWER_PLT_FONT, GFL_BMP_CHRAREA_GET_B );
  GFL_BMPWIN_MakeScreen(bmp_win);
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(bmp_win), FONTVIEWER_CLEAR_COLOR );
  GFL_BMPWIN_TransVramCharacter(bmp_win);
  GFL_BG_LoadScreenReq( bgPlane );

  return bmp_win;
}

//--------------------------------------------------------------------------
//  ヘルプ画面作成表示
//--------------------------------------------------------------------------
static void PrintHelp(FONTVIEWER_WORK *work, GFL_BMPWIN *dst)
{
  STRBUF *strbuf = GFL_MSG_CreateString(work->msgman, DEBUG_FONTVIEW_HELP);
  PRINTSYS_Print(GFL_BMPWIN_GetBmp(dst), HELP_PX, HELP_PY, strbuf, work->fontDefault);
  GFL_STR_DeleteBuffer(strbuf);
}

//--------------------------------------------------------------------------
//  フォント表示画面表示
//--------------------------------------------------------------------------
static void PrintViewHead(FONTVIEWER_WORK *work, GFL_BMPWIN *dst)
{
  GFL_BMP_DATA *const bmp = GFL_BMPWIN_GetBmp(dst);

  WORDSET_RegisterHexNumber(work->wordset, WORDINDEX_CODEBEGIN, IndexToCode(work, work->indexBegin), 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU);
  WORDSET_RegisterHexNumber(work->wordset, WORDINDEX_CODEEND, IndexToCode(work, work->indexEnd), 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU);
  RegisterMessage(work->wordset, WORDINDEX_FONTNAME, work->msgman, DEBUG_FONTVIEW_FONTNAME01+work->fontId);
  RegisterMessage(work->wordset, WORDINDEX_INDEXMODE, work->msgman, DEBUG_FONTVIEW_INDEXMODE01+work->indexMode);
  RegisterMessage(work->wordset, WORDINDEX_LOADTYPE, work->msgman, DEBUG_FONTVIEW_LOADTYPE01+work->fontLoadType);

  WORDSET_ExpandStr(work->wordset, work->viewHeadBuf, work->viewHeadStr);
  PRINTSYS_Print(bmp, VIEWHEAD_PX, VIEWHEAD_PY, work->viewHeadBuf, work->fontDefault);
}

static void PrintViewBody(FONTVIEWER_WORK *work, GFL_BMPWIN *dst)
{
  GFL_BMP_DATA *const bmp = GFL_BMPWIN_GetBmp(dst);
  STRBUF *const buf = work->viewBodyBuf;
  int index;
  int code_count = 0;

  GFL_STR_ClearBuffer(buf);
  for (index = work->indexBegin; index <= work->indexEnd; index ++)
  {
    GFL_STR_AddCode(buf, IndexToCode(work, index));
    code_count ++;
    if (code_count % 16 == 0)
    {
      GFL_STR_AddCode(buf, IndexToCode(work, index));
      GFL_STR_AddCode(buf, CR_CODE);
    }
  }

  PRINTSYS_Print(bmp, VIEWBODY_PX, VIEWBODY_PY, buf, work->fontView);
}

static void PrintView(FONTVIEWER_WORK *work, GFL_BMPWIN *dst)
{
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(dst), FONTVIEWER_CLEAR_COLOR);
  PrintViewHead(work, dst);
  PrintViewBody(work, dst);
}

//--------------------------------------------------------------------------
//  現在のモードの文字インデックスをUnicodeに変換
//--------------------------------------------------------------------------
static STRCODE IndexToCode(FONTVIEWER_WORK *work, int index)
{
  switch (work->indexMode)
  {
  case INDEXMODE_DEFINED:
    GF_ASSERT(index < NELEMS(DefinedCode));
    return DefinedCode[index];
  case INDEXMODE_USERINPUT:
    GF_ASSERT(index < NELEMS(UserInputCode));
    return UserInputCode[index];
  case INDEXMODE_ALL:
    GF_ASSERT(index < 0x10000);
    return (STRCODE)index;
  default:
    GF_ASSERT(0);
  }
  return 0;
}

//--------------------------------------------------------------------------
//  現在のモードの文字インデックス最大を得る
//--------------------------------------------------------------------------
static int GetMaxIndex(FONTVIEWER_WORK *work)
{
  switch (work->indexMode)
  {
  case INDEXMODE_DEFINED:
    return NELEMS(DefinedCode)-1;
  case INDEXMODE_USERINPUT:
    return NELEMS(UserInputCode)-1;
  case INDEXMODE_ALL:
    return 0xffff;
  default:
    GF_ASSERT(0);
  }
  return 0;
}

//--------------------------------------------------------------------------
//  文字インデックスモードの変更
//--------------------------------------------------------------------------
static void SetIndexMode(FONTVIEWER_WORK *work, FONTVIEWER_INDEXMODE new_mode)
{
  GF_ASSERT(0 <= new_mode && new_mode <= INDEXMODE_MAX);

  work->indexMode = new_mode;
}

//--------------------------------------------------------------------------
//  表示する文字インデックスをnew_beginからにする
//--------------------------------------------------------------------------
static void SetIndex(FONTVIEWER_WORK *work, int new_begin)
{
  const int max_index = GetMaxIndex(work);
  int new_end;

  // 終了インデックスの計算
  new_end = new_begin + VIEWBODY_INDEXCOUNT - 1;
  if (new_end > max_index)
  {
    new_end = max_index;
  }

  work->indexBegin = new_begin;
  work->indexEnd = new_end;
}

//--------------------------------------------------------------------------
//  表示する文字インデックスをdiffだけずらす
//--------------------------------------------------------------------------
static void MoveIndex(FONTVIEWER_WORK *work, int diff)
{
  const int max_index = GetMaxIndex(work);
  int new_begin = work->indexBegin + diff;

  if (new_begin < 0)
  {
    // 最後のページ
    new_begin = (max_index / VIEWBODY_INDEXCOUNT) * VIEWBODY_INDEXCOUNT;
  }
  else if (new_begin > GetMaxIndex(work))
  {
    // 最初のページ
    new_begin = 0;
  }
  GF_ASSERT((0 <= new_begin) && (new_begin < max_index));

  SetIndex(work, new_begin);
}

//--------------------------------------------------------------------------
//  指定したフォントを読み込み
//--------------------------------------------------------------------------
static void LoadViewFont(FONTVIEWER_WORK *work, u32 font_id, int load_type)
{
  if (work->fontView != NULL)
  {
    GFL_FONT_Delete( work->fontView );
  }

  work->fontId = font_id;
  work->fontLoadType = load_type;
  work->fontView = GFL_FONT_Create( ARCID_FONT , font_id, load_type, FALSE , work->heapId );
}

//--------------------------------------------------------------------------
//  WORDSETに指定メッセージIDを登録
//--------------------------------------------------------------------------
static void RegisterMessage(WORDSET* wordset, u32 bufID, GFL_MSGDATA *msgman, int msgid)
{
  STRBUF *const strbuf = GFL_MSG_CreateString(msgman, msgid);
  WORDSET_RegisterWord(wordset, bufID, strbuf, FALSE, TRUE, PM_LANG);
  GFL_STR_DeleteBuffer(strbuf);
}

//============================================================================================
//
//
//  プロセスコントロール
//
//
//============================================================================================

//------------------------------------------------------------------
/**
 * @brief   プロセス関数テーブル
 */
//------------------------------------------------------------------
const GFL_PROC_DATA FontViewerProcData = {
  FontViewerProcInit,
  FontViewerProcMain,
  FontViewerProcEnd,
};

//------------------------------------------------------------------
/**
 * @brief   初期化
 *
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT FontViewerProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  FONTVIEWER_WORK *work;
  HEAPID  heapId = HEAPID_NCL_DEBUG;

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, heapId, 0x80000 );

  work = GFL_PROC_AllocWork( proc, sizeof(FONTVIEWER_WORK), heapId );
  GFL_STD_MemClear(work, sizeof(FONTVIEWER_WORK));
  work->heapId = heapId;

  FONTVIEWER_InitGraphic( work );
  GFL_FONTSYS_Init();

  //フォント用パレット
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_MAIN_BG , FONTVIEWER_PLT_FONT * 32, 16*2, work->heapId );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_SUB_BG , FONTVIEWER_PLT_FONT * 32, 16*2, work->heapId );

  // フォント
  work->fontDefault = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );
  work->fontView = NULL;

  // BmpWin
  work->bmpWinView = CreateFullScreenBmpWin(work, BG_PLANE_VIEW);
  work->bmpWinHelp = CreateFullScreenBmpWin(work, BG_PLANE_HELP);

  // 文字列
  work->msgman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_ncl_dat, work->heapId );
  work->wordset = WORDSET_CreateEx( WORDSET_BUFNUM, WORDSET_BUFLEN, work->heapId );

  work->viewHeadStr = GFL_MSG_CreateString(work->msgman, DEBUG_FONTVIEW_HEAD);
  work->viewHeadBuf = GFL_STR_CreateBuffer(VIEWHEAD_BUFSIZE, work->heapId);
  work->viewBodyBuf = GFL_STR_CreateBuffer(VIEWBODY_BUFSIZE, work->heapId);

  LoadViewFont(work, 0, GFL_FONT_LOADTYPE_FILE);
  SetIndexMode(work, INDEXMODE_DEFINED);
  SetIndex(work, 0);

  PrintHelp(work, work->bmpWinHelp);
  GFL_BMPWIN_TransVramCharacter(work->bmpWinHelp);

  PrintView(work, work->bmpWinView);
  GFL_BMPWIN_TransVramCharacter(work->bmpWinView);

  return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief   メイン
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT FontViewerProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  FONTVIEWER_WORK *work = mywk;
  BOOL need_repaint = FALSE;

  // 表示文字、戻る
  if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
  {
    MoveIndex(work, -CODEMOVE_COUNT);
    need_repaint = TRUE;
  }
  // 表示文字、進む
  else if( (GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN) || (GFL_UI_KEY_GetCont() & PAD_BUTTON_A) )
  {
    MoveIndex(work, +CODEMOVE_COUNT);
    need_repaint = TRUE;
  }
  // フォント切り替え
  else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_L )
  {
    LoadViewFont(work, (work->fontId + (FONTID_MAX+1) - 1) % (FONTID_MAX+1), work->fontLoadType);
    need_repaint = TRUE;
  }
  else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R )
  {
    LoadViewFont(work, (work->fontId + 1) % (FONTID_MAX+1), work->fontLoadType);
    need_repaint = TRUE;
  }
  // 文字セット切り替え
  else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y )
  {
    work->indexMode = (work->indexMode+1) % (INDEXMODE_MAX+1);
    SetIndex(work, 0);
    need_repaint = TRUE;
  }
  // 常駐切り替え
  else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
  {
    switch (work->fontLoadType)
    {
    case GFL_FONT_LOADTYPE_FILE:
      LoadViewFont(work, work->fontId, GFL_FONT_LOADTYPE_MEMORY);
      break;
    case GFL_FONT_LOADTYPE_MEMORY:
      LoadViewFont(work, work->fontId, GFL_FONT_LOADTYPE_FILE);
      break;
    default:
      GF_ASSERT(0);
    }
    need_repaint = TRUE;
  }
  // 終了
  else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
  {
    return GFL_PROC_RES_FINISH;
  }

  if (need_repaint)
  {
    PrintView(work, work->bmpWinView);
    GFL_BMPWIN_TransVramCharacter(work->bmpWinView);
  }

  return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief   終了
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT FontViewerProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  FONTVIEWER_WORK *work = mywk;

  GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &TitleProcData, NULL);

  //開放処理
  GFL_FONT_Delete( work->fontDefault );
  GFL_FONT_Delete( work->fontView );

  GFL_BMPWIN_Delete( work->bmpWinView );
  GFL_BMPWIN_Delete( work->bmpWinHelp );

  GFL_MSG_Delete( work->msgman );
  WORDSET_Delete( work->wordset );

  GFL_STR_DeleteBuffer( work->viewHeadStr );
  GFL_STR_DeleteBuffer( work->viewHeadBuf );
  GFL_STR_DeleteBuffer( work->viewBodyBuf );

  GFL_BMPWIN_Exit();
  GFL_BG_Exit();

  GFL_PROC_FreeWork(proc);
  GFL_HEAP_DeleteHeap( HEAPID_NCL_DEBUG );

  return GFL_PROC_RES_FINISH;
}

#endif // #ifdef PM_DEBUG

