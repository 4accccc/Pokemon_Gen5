//======================================================================
/**
 * @file    debug_zone_jump.c
 * @brief   デバッグ用ゾーンジャンプ
 * @author  saito
 * @data    10/02/03
 */
//======================================================================

#include <gflib.h>
//#include <skb.h>
#include "system/main.h"
#include "print/printsys.h"
#include "print/str_tool.h"

#include "font/font.naix"
#include "arc_def.h"
#include "message.naix"
#include "msg/msg_debug_zone_jump.h"
#include "debug/debug_zone_jump.h"
#include "field/zonedata.h"
#include "debug/debug_str_conv.h" // for DEB_STR_CONV_SjisToStrcode

#define PROC_HEAP_SIZE  (0x20000)
#define PRINT_FRAME (GFL_BG_FRAME1_M)
#define PRINT_PALIDX  (0)
#define LINE_HEIGHT (16)
#define STRBUF_LEN  (64)
#define TOUCH_H   (16)
#define SKB_PALIDX1 (1)
#define SKB_PALIDX2 (2)

#define ZONE_JUMP_STR_NULL    (-1)

enum {
  STR_POS_X = 0,
  STR_NAME_POS_Y = 2*8,

  STR_JUMP_POS_Y = 6*8,

  TOUCH_X_NAME = 12*8,
  TOUCH_W_NAME = 16*8,
  TOUCH_X_JUMP = 12*8,
  TOUCH_W_JUMP = 8*8,
};

typedef enum {
  SEQ_DRAW_CAPTION = 0,
  SEQ_DRAW_BOX,
  SEQ_WAIT_CTRL,
  SEQ_INPUT_STR,

}MAIN_CTRL_SEQ;

typedef enum {
  STR_TYPE_STR,      ///< 入力対応文字列（文字列から導かれるインデックスを保存する場合）
  STR_TYPE_BTN, ///<ボタン
}STR_TYPE;

typedef enum {
  EDITBOX_ID_ZONEID = 0,
  EDITBOX_ID_JUMP,
  EDITBOX_ID_EXIT,
  EDITBOX_ID_MAX,
}EDITBOX_ID;

enum {
  COLIDX_BLACK=1,
  COLIDX_GRAY,
  COLIDX_RED_D,
  COLIDX_RED_L,
  COLIDX_BLUE_D,
  COLIDX_BLUE_L,
  COLIDX_YELLOW_D,
  COLIDX_YELLOW_L,
  COLIDX_GREEN_D,
  COLIDX_GREEN_L,
  COLIDX_ORANGE_D,
  COLIDX_ORANGE_L,
  COLIDX_PINK_D,
  COLIDX_PINK_L,
  COLIDX_WHITE,
};


typedef struct {
  STR_TYPE  Type;
  int  StrID;
  u8   PosX;
  u8   PosY;

  u8   TouchX;
  u8   TouchY;
  u8   TouchW;
  u8   TouchH;

//  u16  paraID;
  u32  arg;
  u32  arg2;

}STR_PARAM;

//--------------------------------------------------------------
/**
 *  入力補完ワーク
 */
//--------------------------------------------------------------
typedef struct {
  GFL_SKB*      skb;
//  GFL_MSGDATA*  msg;
  STRBUF*       buf;
  STRBUF*       subword;
  STRBUF*       fullword;
  int           index;
  int           search_first_index;
  int           searchingFlag;
  const char * nameAllBuf;
}COMP_SKB_WORK;

typedef struct {

  GFL_BMPWIN*     Win;
  GFL_BMP_DATA*   Bmp;
  GFL_FONT*       Font;
  GFL_MSGDATA*    MsgData;
  STRBUF*         StrBuf;
  PRINT_QUE*      PrintQue;
  PRINT_UTIL      PrintUtil;
  GFL_SKB*        skb;
  int             BoxIdx;
  u32             BoxValue[ EDITBOX_ID_MAX ];

  HEAPID  HeapID;
  MAIN_CTRL_SEQ      Seq;

  COMP_SKB_WORK   comp;
  GFL_SKB_SETUP   skbSetup;
  
  BOOL *Ret;
  int *ZoneID;

}JUMP_WORK;

static const STR_PARAM StrParams[] = {
  {STR_TYPE_STR, ZONE_JUMP_STR_ID,  STR_POS_X, STR_NAME_POS_Y,
    TOUCH_X_NAME,STR_NAME_POS_Y,TOUCH_W_NAME,TOUCH_H,
    0,0 },
  {STR_TYPE_BTN, ZONE_JUMP_STR_NULL,  TOUCH_X_JUMP, STR_JUMP_POS_Y,
    TOUCH_X_JUMP,STR_JUMP_POS_Y,TOUCH_W_JUMP,TOUCH_H,
    NARC_message_debug_zone_jump_dat, ZONE_JUMP_STR_JUMP },
};


static GFL_PROC_RESULT PROC_ZoneJump_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT PROC_ZoneJump_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT PROC_ZoneJump_Quit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static void SetupDisp( JUMP_WORK* wk );
static void CleanupDisp( JUMP_WORK* wk );
static BOOL MainCtrl( JUMP_WORK* wk );

static void PrintCaption( JUMP_WORK* wk, const STR_PARAM* p );
static int CheckBoxTouch( JUMP_WORK* wk );
static void UpdateBox( JUMP_WORK* wk, u32 inBoxID, u32 inValue );
static PRINTSYS_LSB GetBoxSubBGColor( u8 inType );
static void GetBoxStr( JUMP_WORK* wk, u32 inBoxID, STRBUF* buf );


static void COMPSKB_Setup( COMP_SKB_WORK* wk, GFL_SKB* skb, STRBUF* buf, u32 msgDataID, HEAPID heapID );
static void COMPSKB_Cleanup( COMP_SKB_WORK* wk );
static BOOL COMPSKB_Main( COMP_SKB_WORK* wk );
static int COMPSKB_GetWordIndex( const COMP_SKB_WORK* wk );
static BOOL compskb_strncmp( const STRBUF* str1, const STRBUF* str2, u32 len );
static u32 compskb_search( COMP_SKB_WORK* wk, const STRBUF* word, int org_idx, int* first_idx );
static BOOL comskb_is_match( COMP_SKB_WORK* wk, const STRBUF* word, int* match_idx );

static void setDebugZoneIDStr( const char * allName, u32 zoneID, STRBUF * strBuf );

#define HEAPID_DEBUG_ZONE_JUMP  HEAPID_DEBUG_MAKE_UNDATA

//--------------------------------------------------------------
/**
 *  Proc Data
 */
//--------------------------------------------------------------
const GFL_PROC_DATA ProcData_DebugZoneJump = {
  PROC_ZoneJump_Init,
  PROC_ZoneJump_Main,
  PROC_ZoneJump_Quit,
};


static GFL_PROC_RESULT PROC_ZoneJump_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  //ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_DEBUG_ZONE_JUMP,   PROC_HEAP_SIZE );

  {
    JUMP_WORK* wk = GFL_PROC_AllocWork( proc, sizeof(JUMP_WORK), HEAPID_DEBUG_ZONE_JUMP );
    GF_ASSERT(wk != NULL);
    wk->Seq = 0;
    wk->HeapID = HEAPID_DEBUG_ZONE_JUMP;
    {
      PROCPARAM_DEBUG_ZONE_JUMP* proc_param = (PROCPARAM_DEBUG_ZONE_JUMP*)pwk;
      wk->Ret = &proc_param->Ret;
      wk->ZoneID = &proc_param->ZoneID;
    }
    wk->comp.nameAllBuf = ZONEDATA_GetAllZoneName( wk->HeapID );
  }

  

  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT PROC_ZoneJump_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  JUMP_WORK* wk = mywk;

  switch( *seq ){
  case 0:
    SetupDisp( wk );
    (*seq)++;
    break;
  case 1:
    if( MainCtrl(wk) ){
      (*seq)++;
    }
    break;
  case 2:
    CleanupDisp( wk );
    return GFL_PROC_RES_FINISH;
  }

  return GFL_PROC_RES_CONTINUE;
}

static GFL_PROC_RESULT PROC_ZoneJump_Quit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  JUMP_WORK* wk = mywk;
  GFL_HEAP_FreeMemory( (void*)wk->comp.nameAllBuf );

  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_DEBUG_ZONE_JUMP );
  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------------
/**
 * 画面構築
 *
 * @param   wk
 */
//----------------------------------------------------------------------------------
static void SetupDisp( JUMP_WORK* wk )
{
  static const GFL_DISP_VRAM vramBank = {
    GX_VRAM_BG_128_A,           // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
    GX_VRAM_SUB_BG_128_C,       // サブ2DエンジンのBG
    GX_VRAM_SUB_BGEXTPLTT_NONE, // サブ2DエンジンのBG拡張パレット
    GX_VRAM_OBJ_128_B,          // メイン2DエンジンのOBJ
    GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
    GX_VRAM_SUB_OBJ_16_I,       // サブ2DエンジンのOBJ
    GX_VRAM_SUB_OBJEXTPLTT_NONE,// サブ2DエンジンのOBJ拡張パレット
    GX_VRAM_TEX_NONE,           // テクスチャイメージスロット
    GX_VRAM_TEXPLTT_NONE,       // テクスチャパレットスロット
    GX_OBJVRAMMODE_CHAR_1D_32K, // メインOBJマッピングモード
    GX_OBJVRAMMODE_CHAR_1D_32K, // サブOBJマッピングモード
  };

  GX_SetMasterBrightness(0);
  GXS_SetMasterBrightness(0);

  // BGsystem初期化
  GFL_BG_Init( wk->HeapID );
  {
    static const GFL_BG_SYS_HEADER sysHeader = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_3D,
    };

    static const GFL_BG_BGCNT_HEADER bgcnt = {
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x10000, 0x8000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
    };

    GFL_BG_SetBGMode( &sysHeader );
    GFL_BG_SetBGControl( PRINT_FRAME,   &bgcnt,   GFL_BG_MODE_TEXT );

    GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_OFF );
    GFL_BG_SetVisible( PRINT_FRAME,       VISIBLE_ON  );
  }

  GFL_BMPWIN_Init( wk->HeapID );
  GFL_FONTSYS_Init();

  // VRAMバンク設定
  GFL_DISP_SetBank( &vramBank );

  // 各種効果レジスタ初期化
  G2_BlendNone();
  G2S_BlendNone();

  // 上下画面設定
  GX_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );

  wk->Win = GFL_BMPWIN_Create( PRINT_FRAME, 0, 0, 32, 24, PRINT_PALIDX, GFL_BMP_CHRAREA_GET_F );
  wk->Bmp = GFL_BMPWIN_GetBmp( wk->Win );
  wk->Font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->HeapID );
  wk->MsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_debug_zone_jump_dat, wk->HeapID );
  wk->StrBuf = GFL_STR_CreateBuffer( STRBUF_LEN, wk->HeapID );
  wk->PrintQue = PRINTSYS_QUE_Create( wk->HeapID );
  PRINT_UTIL_Setup( &wk->PrintUtil, wk->Win );


  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 0, 0, wk->HeapID );
  GFL_BMP_Clear( wk->Bmp, 0x0f );
  GFL_BMPWIN_MakeScreen( wk->Win );
  GFL_BG_LoadScreenReq( PRINT_FRAME );

  GFL_BMPWIN_TransVramCharacter( wk->Win );

}
//----------------------------------------------------------------------------------
/**
 * 画面クリーンアップ
 *
 * @param   wk
 */
//----------------------------------------------------------------------------------
static void CleanupDisp( JUMP_WORK* wk )
{
  PRINTSYS_QUE_Delete( wk->PrintQue );
  GFL_STR_DeleteBuffer( wk->StrBuf );
  GFL_MSG_Delete( wk->MsgData );
  GFL_FONT_Delete( wk->Font );
  GFL_BMPWIN_Delete( wk->Win );
  GFL_BMPWIN_Exit();
  GFL_BG_Exit();
}

//----------------------------------------------------------------------------------
/**
 * 操作ルート
 *
 * @param   wk
 *
 * @retval  BOOL    終わったらTRUE
 */
//----------------------------------------------------------------------------------
static BOOL MainCtrl( JUMP_WORK* wk )
{
  if( PRINT_UTIL_Trans(&wk->PrintUtil, wk->PrintQue) )
  {
    switch( wk->Seq ){
    case SEQ_DRAW_CAPTION:
      {
        int i;
        for(i=0; i<NELEMS(StrParams); ++i){
          PrintCaption( wk, &StrParams[i] );
        }
      }
      wk->Seq = SEQ_DRAW_BOX;
      break;      
    case SEQ_DRAW_BOX:
      {
        int i;
        for(i=0; i<NELEMS(StrParams); ++i){
          UpdateBox( wk, i, 0 );
        }
        wk->Seq = SEQ_WAIT_CTRL;
      }
      break;      
    case SEQ_WAIT_CTRL:
      wk->BoxIdx = CheckBoxTouch( wk );
      if( wk->BoxIdx >= 0 )
      {
        const STR_PARAM* p = &StrParams[ wk->BoxIdx ];
        switch( p->Type ){
        case STR_TYPE_STR:
          {            
            static const GFL_SKB_SETUP setup = {
              STRBUF_LEN, GFL_SKB_STRTYPE_STRBUF,
              GFL_SKB_MODE_ENG, TRUE, 0, PAD_BUTTON_START,
              PRINT_FRAME, SKB_PALIDX1, SKB_PALIDX2,
            };
            wk->skbSetup = setup;
            GetBoxStr( wk, wk->BoxIdx, wk->StrBuf );
            wk->skb = GFL_SKB_Create( (void*)(wk->StrBuf), &wk->skbSetup, wk->HeapID );
            COMPSKB_Setup( &wk->comp, wk->skb, wk->StrBuf, p->arg, wk->HeapID );
            wk->Seq = SEQ_INPUT_STR;            
          }
          break;
        case STR_TYPE_BTN:
          switch(wk->BoxIdx){
          case EDITBOX_ID_JUMP:
            //結果セット
            *wk->Ret = TRUE;
            *wk->ZoneID = wk->BoxValue[EDITBOX_ID_ZONEID];
            return TRUE;
          }//end switch
          break;
        }       //end switch
        break;
      }
      {
        u16 key = GFL_UI_KEY_GetTrg();
        if( key & PAD_BUTTON_SELECT )
        {
          return TRUE;
        }
      }
      break;
    case SEQ_INPUT_STR:      
      if( COMPSKB_Main(&wk->comp) )
      {
        int idx;
        const STR_PARAM* p = &StrParams[ wk->BoxIdx ];
        {
          idx = COMPSKB_GetWordIndex( &wk->comp );
          if( idx < 0 ){
            if( GFL_STR_GetBufferLength(wk->StrBuf) == 0 ){
              idx = 0;
            }
          }
          if( idx >= 0 ){
            UpdateBox( wk, wk->BoxIdx, idx );
          }
        }
        COMPSKB_Cleanup( &wk->comp );
        GFL_SKB_Delete( wk->skb );
        wk->Seq = SEQ_WAIT_CTRL;
      }      
      break;
    }    //end switch
  } //end if
  return FALSE;
}

//----------------------------------------------------------------------------------
/**
 * 項目名描画
 *
 * @param   wk
 * @param   p
 */
//----------------------------------------------------------------------------------
static void PrintCaption( JUMP_WORK* wk, const STR_PARAM* p )
{
  if( p->StrID != ZONE_JUMP_STR_NULL ){
    u32 ypos = p->PosY;
    GFL_MSG_GetString( wk->MsgData, p->StrID, wk->StrBuf );
    PRINT_UTIL_Print( &wk->PrintUtil, wk->PrintQue, p->PosX, ypos, wk->StrBuf, wk->Font );
  }
}

static int CheckBoxTouch( JUMP_WORK* wk )
{
  u32 x, y;
  if( GFL_UI_TP_GetPointTrg( &x, &y ) )
  {
    u32 i;
    for(i=0; i<NELEMS(StrParams); ++i)
    {
      if( 1/*wk->boxEnable[i]*/ )
      {
        const STR_PARAM* p = &StrParams[i];
        if( (x >= p->TouchX) && (x <= (p->TouchX + p->TouchW))
        &&  (y >= p->TouchY) && (y <= (p->TouchY + p->TouchH))
        ){
          return i;
        }
      }
    }
  }
  return -1;
}

static void UpdateBox( JUMP_WORK* wk, u32 inBoxID, u32 inValue )
{
  const STR_PARAM* p = &StrParams[ inBoxID ];
  u32 str_width, str_height, xpos, ypos;

  PRINTSYS_LSB color = GetBoxSubBGColor( p->Type );
  u8 color_bg = PRINTSYS_LSB_GetB( color );

  GFL_BMP_Fill( wk->Bmp, p->TouchX, p->TouchY, p->TouchW, p->TouchH, color_bg );

  wk->BoxValue[ inBoxID ] = inValue;
  GetBoxStr( wk, inBoxID, wk->StrBuf );

  str_width = PRINTSYS_GetStrWidth( wk->StrBuf, wk->Font, 0 );
  xpos = p->TouchX;
  if( str_width < p->TouchW ){
    xpos += (p->TouchW - str_width) / 2;
  }
  str_height = GFL_FONT_GetLineHeight( wk->Font );
  ypos = p->TouchY;

  PRINT_UTIL_PrintColor( &wk->PrintUtil, wk->PrintQue, xpos, ypos, wk->StrBuf, wk->Font, color );
}

static PRINTSYS_LSB GetBoxSubBGColor( u8 inType )
{
  switch( inType ){
  case STR_TYPE_STR:     return PRINTSYS_LSB_Make( COLIDX_BLACK, 0, COLIDX_ORANGE_D );
  case STR_TYPE_BTN:     return PRINTSYS_LSB_Make( COLIDX_BLACK, 0, COLIDX_RED_L );

  }
  return PRINTSYS_LSB_Make( COLIDX_BLACK, 0, COLIDX_WHITE );
}

static void GetBoxStr( JUMP_WORK* wk, u32 inBoxID, STRBUF* buf )
{
  const STR_PARAM* p = &StrParams[ inBoxID ];
  u32 value = wk->BoxValue[ inBoxID ];
  
  switch( p->Type ){
  case STR_TYPE_STR:
    {
      setDebugZoneIDStr( wk->comp.nameAllBuf, value, wk->StrBuf );
    }
    break;
  case STR_TYPE_BTN:
    {
      GFL_MSGDATA* msgdat = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, p->arg, GFL_HEAP_LOWID(wk->HeapID) );
      GFL_MSG_GetString( msgdat, p->arg2, wk->StrBuf );
      GFL_MSG_Delete( msgdat );
    }
    break;
  } //end switch  
}

//==============================================================================================
//  入力補完処理
//==============================================================================================
static void COMPSKB_Setup( COMP_SKB_WORK* wk, GFL_SKB* skb, STRBUF* buf, u32 msgDataID, HEAPID heapID )
{
  wk->skb = skb;
//  wk->msg = GFL_MSG_Create( GFL_MSG_LOAD_FAST, ARCID_MESSAGE, msgDataID, heapID );
  wk->buf = buf;
  wk->subword  = GFL_STR_CreateBuffer( 64, heapID );
  wk->fullword = GFL_STR_CreateBuffer( 64, heapID );
  wk->index = -1;
  wk->search_first_index = -1;
  wk->searchingFlag = FALSE;
}
static void COMPSKB_Cleanup( COMP_SKB_WORK* wk )
{
  GFL_STR_DeleteBuffer( wk->fullword );
  GFL_STR_DeleteBuffer( wk->subword );
//  GFL_MSG_Delete( wk->msg );
}

static BOOL COMPSKB_Main( COMP_SKB_WORK* wk )
{
  GflSkbReaction reaction = GFL_SKB_Main( wk->skb );
  BOOL fSearchReq = FALSE;

  if( reaction != GFL_SKB_REACTION_NONE
  &&  reaction != GFL_SKB_REACTION_PAGE ){
    wk->searchingFlag = FALSE;
  }

  switch( reaction ){

  case GFL_SKB_REACTION_QUIT:
    if( wk->index == -1 )
    {
      int idx;
      GFL_SKB_PickStr( wk->skb );
      if( comskb_is_match(wk, wk->buf, &idx) ){
        wk->index = idx;
      }
    }
    return TRUE;
  case GFL_SKB_REACTION_INPUT:
    {
      int idx;
      GFL_SKB_PickStr( wk->skb );
      if( compskb_search(wk, wk->buf, -1, &idx) == 1 )
      {
        //GFL_MSG_GetString( wk->msg, idx, wk->fullword );
        setDebugZoneIDStr( wk->nameAllBuf, idx, wk->fullword );
        GFL_SKB_ReloadStr( wk->skb, wk->fullword );
        wk->index = idx;
      }
      else{
        wk->index = -1;
      }
    }
    break;
  case GFL_SKB_REACTION_BACKSPACE:
    {
      int idx;
      GFL_SKB_PickStr( wk->skb );
      if( compskb_search(wk, wk->buf, -1, &idx) == 1 ){
        wk->index = idx;
      }else{
        wk->index = -1;
      }
    }
    break;
  case GFL_SKB_REACTION_PAGE:
    //fSearchReq = TRUE;
    break;
  case GFL_SKB_REACTION_NONE:
    {
      u16 key = GFL_UI_KEY_GetTrg();
      if( (key & PAD_BUTTON_SELECT)||(key & PAD_BUTTON_R)||(key & PAD_BUTTON_L) ){
        fSearchReq = TRUE;
      }
    }
    break;
  }

  if( fSearchReq )
  {
    if( wk->searchingFlag == FALSE ){
      GFL_SKB_PickStr( wk->skb );
      GFL_STR_CopyBuffer( wk->subword, wk->buf );
      wk->search_first_index = -1;
      wk->index = -1;
      wk->searchingFlag = TRUE;
    }
    {
      int idx;
      if( compskb_search(wk, wk->subword, wk->index, &idx) )
      {
        wk->index = idx;
        if( wk->search_first_index == -1 ){
          wk->search_first_index = idx;
        }
      }
      else
      {
        wk->index = wk->search_first_index;
      }

      if( wk->index != -1 )
      {
        //GFL_MSG_GetString( wk->msg, wk->index, wk->fullword );
        setDebugZoneIDStr( wk->nameAllBuf, wk->index, wk->fullword );
        GFL_SKB_ReloadStr( wk->skb, wk->fullword );
      }

    }
  }

  return FALSE;
}
static int COMPSKB_GetWordIndex( const COMP_SKB_WORK* wk )
{
  return wk->index;
}
static BOOL compskb_strncmp( const STRBUF* str1, const STRBUF* str2, u32 len )
{
  if( GFL_STR_GetBufferLength(str1) < len ){
    return FALSE;
  }
  if( GFL_STR_GetBufferLength(str2) < len ){
    return FALSE;
  }

  {
    const STRCODE *p1 = GFL_STR_GetStringCodePointer( str1 );
    const STRCODE *p2 = GFL_STR_GetStringCodePointer( str2 );
    u32 i;
    for(i=0; i<len; ++i){
      if( *p1++ != *p2++ ){ return FALSE; }
    }
    return TRUE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * 前方一致する文字列数をサーチ
 *
 * @param   wk
 * @param   word
 * @param   org_idx
 * @param   first_idx   [out] 最初に一致した文字列index
 *
 * @retval  u32   前方一致する文字列数
 */
//----------------------------------------------------------------------------------
static u32 compskb_search( COMP_SKB_WORK* wk, const STRBUF* word, int org_idx, int* first_idx )
{
  u32 word_len = GFL_STR_GetBufferLength( word );
  if( word_len )
  {
    u32 str_cnt, match_cnt, i;

    *first_idx = -1;
    match_cnt = 0;
    str_cnt = ZONEDATA_GetZoneIDMax();//GFL_MSG_GetStrCount( wk->msg );
    i = (org_idx < 0)? 0 : org_idx+1;
    while( i < str_cnt )
    {
      //GFL_MSG_GetString( wk->msg, i, wk->fullword );
      setDebugZoneIDStr( wk->nameAllBuf, i, wk->fullword );
      if( compskb_strncmp( word, wk->fullword, GFL_STR_GetBufferLength(word) ) ){
        if( *first_idx == -1 ){
          *first_idx = i;
        }
        ++match_cnt;
      }
      ++i;
    }
    return match_cnt;
  }
  return 0;
}
//----------------------------------------------------------------------------------
/**
 * 完全一致する文字列をサーチ
 *
 * @param   wk
 * @param   word
 * @param   match_idx   [OUT] 完全一致した文字列index
 *
 * @retval  BOOL    完全一致が見つかればTRUE
 */
//----------------------------------------------------------------------------------
static BOOL comskb_is_match( COMP_SKB_WORK* wk, const STRBUF* word, int* match_idx )
{
  u32 word_len = GFL_STR_GetBufferLength( word );
  if( word_len )
  {
    u32 str_cnt, i=0;

    str_cnt = ZONEDATA_GetZoneIDMax();//GFL_MSG_GetStrCount( wk->msg );
    while( i < str_cnt )
    {
      //GFL_MSG_GetString( wk->msg, i, wk->fullword );
      setDebugZoneIDStr( wk->nameAllBuf, i, wk->fullword );
      if( GFL_STR_CompareBuffer(word, wk->fullword) ){
        *match_idx = i;
        return TRUE;
      }
      ++i;
    }
  }
  return FALSE;
}

static void setDebugZoneIDStr( const char * allName, u32 zoneID, STRBUF * strBuf )
{
  u16 pStrCode[64];
  
  DEB_STR_CONV_SjisToStrcode( &allName[ ZONEDATA_NAME_LENGTH * zoneID ], pStrCode, 64 );
  GFL_STR_SetStringCode( strBuf, pStrCode );
}

