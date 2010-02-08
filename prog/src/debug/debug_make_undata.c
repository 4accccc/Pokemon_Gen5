//======================================================================
/**
 * @file    debug_make_undata.c
 * @brief   デバッグ用国連データ作成
 * @author  saito
 * @data    10/02/03
 */
//======================================================================

#include <gflib.h>
//#include <skb.h>
#include "system/main.h"
#include "print/printsys.h"
#include "print/str_tool.h"
#include "poke_tool/monsno_def.h"

#include "font/font.naix"
#include "arc_def.h"
#include "message.naix"
#include "msg/msg_debug_make_undata.h"

#include "debug/debug_make_undata.h"

#include "savedata/wifihistory_local.h"
#include "savedata/wifihistory.h"
#include "savedata/undata_update.h"

#define PROC_HEAP_SIZE  (0x20000)
#define PRINT_FRAME (GFL_BG_FRAME1_M)
#define PRINT_PALIDX  (0)
#define LINE_HEIGHT (16)
#define STRBUF_LEN  (64)
#define INPUTBOX_NUM_ARG_EXP (0xffffffff)
#define INPUTBOX_NUM_ARG_DEPEND (0xfffffffe)
#define TOUCH_H   (16)
#define NUMINPUT_DISIT_MAX (10)
#define NUMBOX_MARGIN (3)
#define NUMBOX_CHAR_WIDTH (8)
#define SKB_PALIDX1 (1)
#define SKB_PALIDX2 (2)

#define INPUT_VALUE_MAX (0xffffffff)

enum {
  STR_POS_X = 0,
  STR_ID_POS_Y = 0,
  STR_NAME_POS_Y = 2*8,
  STR_SEX_POS_Y = 4*8,
  STR_COUNTRY_POS_Y = 6*8,
  STR_RECV_POS_Y = 8*8,
  STR_SEND_POS_Y = 10*8,
  STR_NUM_POS_Y = 12*8,
  STR_NATURE_POS_Y = 14*8,
  STR_FAVORITE_POS_Y = 16*8,
  STR_IDX_POS_Y = 18*8,

  TOUCH_X_ID = 12*8,
  TOUCH_X_NAME = 12*8,
  TOUCH_X_SEX = 12*8,
  TOUCH_X_COUNTRY = 12*8,
  TOUCH_X_RECV = 12*8,
  TOUCH_X_SEND = 12*8,
  TOUCH_X_NUM = 18*8,
  TOUCH_X_NATURE = 12*8,
  TOUCH_X_FAVORITE = 12*8,
  TOUCH_X_IDX = 12*8,

  TOUCH_W_ID = 11*8,
  TOUCH_W_NAME = 8*8,
  TOUCH_W_SEX = 2*8,
  TOUCH_W_COUNTRY = 12*8,
  TOUCH_W_RECV = 8*8,
  TOUCH_W_SEND = 8*8,
  TOUCH_W_NUM = 8*8,
  TOUCH_W_NATURE = 8*8,
  TOUCH_W_FAVORITE = 8*8,
  TOUCH_W_IDX = 8*8,
};

typedef enum {
  SEQ_DRAW_CAPTION = 0,
  SEQ_DRAW_BOX,
  SEQ_WAIT_CTRL,
  SEQ_INPUT_STR,
  SEQ_INPUT_NUM,

  SEQ_LOOP,
}MAIN_CTRL_SEQ;

typedef enum {
  STR_TYPE_STR,      ///< 入力対応文字列（文字列から導かれるインデックスを保存する場合）
  STR_TYPE_NUM,      ///< 入力対応数値
  STR_TYPE_SWITCH,   ///< タッチ対応トグルスイッチ
  STR_TYPE_EDITSTR,  ///< 入力対応文字列（文字列自体を保存する場合）
}STR_TYPE;

typedef enum {
  EDITBOX_ID_TRID = 0,
  EDITBOX_ID_NAME,
  EDITBOX_ID_SEX,
  EDITBOX_ID_COUNTRY,
  EDITBOX_ID_RECV,
  EDITBOX_ID_SEND,
  EDITBOX_ID_NUM,

  EDITBOX_ID_NATURE,
  EDITBOX_ID_FAVORITE,
  EDITBOX_ID_OBJIDX,

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
 *  数値入力ワーク
 */
//--------------------------------------------------------------
typedef struct {
  GFL_BMPWIN*   Win;
  GFL_BMP_DATA* Bmp;
  GFL_FONT*     Font;
  STRBUF*       Buf;
  PRINT_UTIL*   PrintUtil;
  PRINT_QUE*    PrintQue;
  const STR_PARAM* Param;

  u32  Num;
  u32  Disit;
  u32  NumMax;
  u32  NumMin;
  u8   Draw_ox;
  u8   ColorDefBg;
  u8   ColorCurBg;
  u8   ColorLetter;

  s8   NumAry[ NUMINPUT_DISIT_MAX ];
  u8   CurDisit;
  u32  DispNumWork;
}NUMINPUT_WORK;

//--------------------------------------------------------------
/**
 *  入力補完ワーク
 */
//--------------------------------------------------------------
typedef struct {
  GFL_SKB*      skb;
  GFL_MSGDATA*  msg;
  STRBUF*       buf;
  STRBUF*       subword;
  STRBUF*       fullword;
  int           index;
  int           search_first_index;
  int           searchingFlag;
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
//  u32             pokeID;
//  u32             pokeExpMin;
//  u32             pokeExpMax;
  int             BoxIdx;
  u32             BoxValue[ EDITBOX_ID_MAX ];
//  u8              boxEnable[ INPUTBOX_ID_MAX ];

//  POKEMON_PARAM*  dst;
  HEAPID  HeapID;
  MAIN_CTRL_SEQ      Seq;

  COMP_SKB_WORK   comp;
  GFL_SKB_SETUP   skbSetup;
  NUMINPUT_WORK   NumInput;
  
  WIFI_HISTORY *wh;
  _UNITEDNATIONS_SAVE UNData;

  STRBUF*         NameBuf;

}MAKE_WORK;

static const STR_PARAM StrParams[] = {
  {STR_TYPE_NUM, UND_STR_ID,  STR_POS_X, STR_ID_POS_Y,
    TOUCH_X_ID,STR_ID_POS_Y,TOUCH_W_ID,TOUCH_H,
    0xffffffff,0 },
  {STR_TYPE_EDITSTR, UND_STR_NAME,  STR_POS_X, STR_NAME_POS_Y,
    TOUCH_X_NAME,STR_NAME_POS_Y,TOUCH_W_NAME,TOUCH_H,
    0,0 },
  {STR_TYPE_SWITCH, UND_STR_SEX,  STR_POS_X, STR_SEX_POS_Y,
    TOUCH_X_SEX,STR_SEX_POS_Y,TOUCH_W_SEX,TOUCH_H,
    UND_STR_SEX_MALE,0 },
  {STR_TYPE_STR, UND_STR_COUNTRY,  STR_POS_X, STR_COUNTRY_POS_Y,
    TOUCH_X_COUNTRY,STR_COUNTRY_POS_Y,TOUCH_W_COUNTRY,TOUCH_H,
    NARC_message_wifi_place_msg_world_dat,0 },
  {STR_TYPE_STR, UND_STR_RECVPOKE,  STR_POS_X, STR_RECV_POS_Y,
    TOUCH_X_RECV,STR_RECV_POS_Y,TOUCH_W_RECV,TOUCH_H,
    NARC_message_monsname_dat,0 },
  {STR_TYPE_STR, UND_STR_SENDPOKE,  STR_POS_X, STR_SEND_POS_Y, 
    TOUCH_X_SEND,STR_SEND_POS_Y,TOUCH_W_SEND,TOUCH_H,
    NARC_message_monsname_dat,0 },
  {STR_TYPE_NUM, UND_STR_TRADE,  STR_POS_X, STR_NUM_POS_Y,
    TOUCH_X_NUM,STR_NUM_POS_Y,TOUCH_W_NUM,TOUCH_H,
    234,0 },
  {STR_TYPE_NUM, UND_STR_NATURE,  STR_POS_X, STR_NATURE_POS_Y,
    TOUCH_X_NATURE,STR_NATURE_POS_Y,TOUCH_W_NATURE,TOUCH_H,
    5,0 },
  {STR_TYPE_NUM, UND_STR_FAVORITE,  STR_POS_X, STR_FAVORITE_POS_Y,
    TOUCH_X_FAVORITE,STR_FAVORITE_POS_Y,TOUCH_W_FAVORITE,TOUCH_H,
    5,0 },
  {STR_TYPE_NUM, UND_STR_OBJIDX,  STR_POS_X, STR_IDX_POS_Y,
    TOUCH_X_IDX,STR_IDX_POS_Y,TOUCH_W_IDX,TOUCH_H,
    8*2,0 },  //<<8種類*性別2
};


static GFL_PROC_RESULT PROC_MAKE_UNDATA_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT PROC_MAKE_UNDATA_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT PROC_MAKE_UNDATA_Quit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static void SetupDisp( MAKE_WORK* wk );
static void CleanupDisp( MAKE_WORK* wk );
static BOOL MainCtrl( MAKE_WORK* wk );

static void PrintCaption( MAKE_WORK* wk, const STR_PARAM* p );
static int CheckBoxTouch( MAKE_WORK* wk );
static void UpdateBox( MAKE_WORK* wk, u32 inBoxID, u32 inValue );
static void UpdateStrBox( MAKE_WORK* wk, u32 inBoxID);
static PRINTSYS_LSB GetBoxSubBGColor( u8 inType );
static void GetBoxStr( MAKE_WORK* wk, u32 inBoxID, STRBUF* buf );

static u16 CalcDisit( u32 inValue );


static void NumInput_Setup( NUMINPUT_WORK* wk, STRBUF* buf, GFL_BMPWIN* win, GFL_FONT* font,
            PRINT_UTIL* util, PRINT_QUE* que, const STR_PARAM* strParam, u32 value, const MAKE_WORK* make_wk );
static BOOL NumInput_Main( NUMINPUT_WORK* wk );
static u32 NumInput_GetNum( NUMINPUT_WORK* wk );
static void DrawNumInput( NUMINPUT_WORK* wk );
static void SetAryNumInput( NUMINPUT_WORK* wk, u32 value );
static u32 CalcInputNum( NUMINPUT_WORK* wk );


static void COMPSKB_Setup( COMP_SKB_WORK* wk, GFL_SKB* skb, STRBUF* buf, u32 msgDataID, HEAPID heapID );
static void COMPSKB_Cleanup( COMP_SKB_WORK* wk );
static BOOL COMPSKB_Main( COMP_SKB_WORK* wk );
static int COMPSKB_GetWordIndex( const COMP_SKB_WORK* wk );
static BOOL compskb_strncmp( const STRBUF* str1, const STRBUF* str2, u32 len );
static u32 compskb_search( COMP_SKB_WORK* wk, const STRBUF* word, int org_idx, int* first_idx );
static BOOL comskb_is_match( COMP_SKB_WORK* wk, const STRBUF* word, int* match_idx );
static BOOL is_hiragana( const STRBUF* buf );

static void MakeData(MAKE_WORK *wk);
static void DumpUNData(MAKE_WORK *wk);
//--------------------------------------------------------------
/**
 *  Proc Data
 */
//--------------------------------------------------------------
const GFL_PROC_DATA ProcData_DebugMakeUNData = {
  PROC_MAKE_UNDATA_Init,
  PROC_MAKE_UNDATA_Main,
  PROC_MAKE_UNDATA_Quit,
};

static GFL_PROC_RESULT PROC_MAKE_UNDATA_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  //ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_DEBUG_MAKE_UNDATA,   PROC_HEAP_SIZE );

  {
    MAKE_WORK* wk = GFL_PROC_AllocWork( proc, sizeof(MAKE_WORK), HEAPID_DEBUG_MAKE_UNDATA );
    GF_ASSERT(wk != NULL);
    wk->Seq = 0;
    wk->HeapID = HEAPID_DEBUG_MAKE_UNDATA;
    {
      const PROCPARAM_DEBUG_MAKE_UNDATA* proc_param = (const PROCPARAM_DEBUG_MAKE_UNDATA*)pwk;
      wk->wh = proc_param->wh;
    }
/**    
    if( wk )
    {
      const PROCPARAM_DEBUG_MAKEPOKE* proc_param = (const PROCPARAM_DEBUG_MAKEPOKE*)pwk;

      wk->HeapID = HEAPID_DEBUG_MAKE_UNDATA;
      wk->dst = proc_param->dst;
      if( PP_Get(wk->dst, ID_PARA_monsno, NULL) == 0 ){
        PP_Setup( wk->dst, MONSNO_GORIDARUMA, 5, PTL_SETUP_ID_NOT_RARE );
      }
      wk->pokeID = PP_Get( wk->dst, ID_PARA_id_no, NULL );
      UpdatePokeExpMinMax( wk, wk->dst );
      wk->seq = 0;
    }
*/    
  }

  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT PROC_MAKE_UNDATA_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  MAKE_WORK* wk = mywk;

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

static GFL_PROC_RESULT PROC_MAKE_UNDATA_Quit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_DEBUG_MAKE_UNDATA );
  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------------
/**
 * 画面構築
 *
 * @param   wk
 */
//----------------------------------------------------------------------------------
static void SetupDisp( MAKE_WORK* wk )
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
  wk->MsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_debug_make_undata_dat, wk->HeapID );
  wk->StrBuf = GFL_STR_CreateBuffer( STRBUF_LEN, wk->HeapID );
  wk->NameBuf = GFL_STR_CreateBuffer( PERSON_NAME_SIZE + EOM_SIZE, wk->HeapID );
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
static void CleanupDisp( MAKE_WORK* wk )
{
  PRINTSYS_QUE_Delete( wk->PrintQue );
  GFL_STR_DeleteBuffer( wk->NameBuf );
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
static BOOL MainCtrl( MAKE_WORK* wk )
{
  if( PRINT_UTIL_Trans(&wk->PrintUtil, wk->PrintQue) )
  {
    switch( wk->Seq ){
    case SEQ_LOOP:
      break;
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
        case STR_TYPE_EDITSTR:  
          {            
            static const GFL_SKB_SETUP setup = {
              STRBUF_LEN, GFL_SKB_STRTYPE_STRBUF,
              GFL_SKB_MODE_KATAKANA, TRUE, 0, PAD_BUTTON_START,
              PRINT_FRAME, SKB_PALIDX1, SKB_PALIDX2,
            };
            wk->skbSetup = setup;
            GetBoxStr( wk, wk->BoxIdx, wk->StrBuf );
            if( is_hiragana(wk->StrBuf) ){
              wk->skbSetup.mode = GFL_SKB_MODE_HIRAGANA;
            }
            wk->skb = GFL_SKB_Create( (void*)(wk->StrBuf), &wk->skbSetup, wk->HeapID );
            COMPSKB_Setup( &wk->comp, wk->skb, wk->StrBuf, p->arg, wk->HeapID );
            wk->Seq = SEQ_INPUT_STR;            
          }
          break;
        case STR_TYPE_NUM:
          NumInput_Setup( &wk->NumInput, wk->StrBuf, wk->Win, wk->Font,
              &wk->PrintUtil, wk->PrintQue, p, wk->BoxValue[wk->BoxIdx], wk );
          wk->Seq = SEQ_INPUT_NUM;
          break;          
        case STR_TYPE_SWITCH:
          {
            u8 val = wk->BoxValue[wk->BoxIdx];
            val = !val;
            UpdateBox( wk, wk->BoxIdx, val );
          }
          break;
        }       //end switch
        break;
      }
      {
        u16 key = GFL_UI_KEY_GetTrg();
        if ( key & PAD_BUTTON_START )
        {
          MakeData(wk);
        }
        else if( key & PAD_BUTTON_SELECT )
        {
          return TRUE;
        }
        else if (key & PAD_BUTTON_DEBUG)
        {
          DumpUNData(wk);
        }
      }
      break;
    case SEQ_INPUT_STR:      
      if( COMPSKB_Main(&wk->comp) )
      {
        int idx;
        const STR_PARAM* p = &StrParams[ wk->BoxIdx ];
        if ( p->Type == STR_TYPE_EDITSTR)
        {
          //@todo
          if( GFL_STR_GetBufferLength(wk->StrBuf) == 0 ){
            idx = 0;
          }
          GFL_STR_CopyBuffer( wk->NameBuf, wk->StrBuf );
          idx = 0;
          UpdateStrBox( wk, wk->BoxIdx);
        }
        else
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
    case SEQ_INPUT_NUM:
      if( NumInput_Main(&wk->NumInput) )
      {
        u32 num = NumInput_GetNum( &wk->NumInput );
        UpdateBox( wk, wk->BoxIdx, num );
        wk->Seq = SEQ_WAIT_CTRL;
      }      
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
static void PrintCaption( MAKE_WORK* wk, const STR_PARAM* p )
{
  if( /*p->StrID != DMPSTR_NULL*/1 ){
    u32 ypos = p->PosY/* + ((LINE_HEIGHT - GFL_FONT_GetLineHeight(wk->Font)) / 2)*/;
    GFL_MSG_GetString( wk->MsgData, p->StrID, wk->StrBuf );
    PRINT_UTIL_Print( &wk->PrintUtil, wk->PrintQue, p->PosX, ypos, wk->StrBuf, wk->Font );
  }
}

static int CheckBoxTouch( MAKE_WORK* wk )
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

static void UpdateBox( MAKE_WORK* wk, u32 inBoxID, u32 inValue )
{
  const STR_PARAM* p = &StrParams[ inBoxID ];
  u32 str_width, str_height, xpos, ypos;

  PRINTSYS_LSB color = GetBoxSubBGColor( p->Type );
  u8 color_bg = PRINTSYS_LSB_GetB( color );

  if ( p->Type == STR_TYPE_EDITSTR ) return;

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

static void UpdateStrBox( MAKE_WORK* wk, u32 inBoxID)
{
  const STR_PARAM* p = &StrParams[ inBoxID ];
  u32 str_width, str_height, xpos, ypos;

  PRINTSYS_LSB color = GetBoxSubBGColor( p->Type );
  u8 color_bg = PRINTSYS_LSB_GetB( color );

  GFL_BMP_Fill( wk->Bmp, p->TouchX, p->TouchY, p->TouchW, p->TouchH, color_bg );

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
  case STR_TYPE_NUM:     return PRINTSYS_LSB_Make( COLIDX_BLACK, 0, COLIDX_BLUE_L );
  case STR_TYPE_SWITCH:  return PRINTSYS_LSB_Make( COLIDX_BLACK, 0, COLIDX_GREEN_L );
  case STR_TYPE_EDITSTR: return PRINTSYS_LSB_Make( COLIDX_BLACK, 0, COLIDX_RED_L );
  }
  return PRINTSYS_LSB_Make( COLIDX_BLACK, 0, COLIDX_WHITE );
}

static void GetBoxStr( MAKE_WORK* wk, u32 inBoxID, STRBUF* buf )
{
  const STR_PARAM* p = &StrParams[ inBoxID ];
  u32 value = wk->BoxValue[ inBoxID ];

  switch( p->Type ){
  case STR_TYPE_STR:
    {
      GFL_MSGDATA* msgdat = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, p->arg, GFL_HEAP_LOWID(wk->HeapID) );
      GFL_MSG_GetString( msgdat, value, wk->StrBuf );
      GFL_MSG_Delete( msgdat );
    }
    break;
  case STR_TYPE_EDITSTR:
    break;
  case STR_TYPE_NUM:
    {
      u8 keta = 1;
#if 0     //@todo 
      switch( p->arg ){
      default:
        keta = CalcDisit(p->arg);
        break;
      case INPUTBOX_NUM_ARG_EXP:
        keta = 8;
        break;
      case INPUTBOX_NUM_ARG_DEPEND:
        keta = CalcDisit( wk->BoxValue[p->arg2]/*box_getvalue(wk, p->arg2)*/ ); //@todo
        break;
      }
#else
      keta = CalcDisit(p->arg);
#endif      
      STRTOOL_SetUnsignedNumber( wk->StrBuf, value, keta, STR_NUM_DISP_SPACE, STR_NUM_CODE_ZENKAKU );
    }
    break;
  case STR_TYPE_SWITCH:
    GFL_MSG_GetString( wk->MsgData, p->arg+value, wk->StrBuf );
    break;    
  } //end switch  
}

static u16 CalcDisit( u32 inValue )
{
  u32 max, disit;

  if ( inValue == INPUT_VALUE_MAX ) return 10;
  
  disit = 1;
  max = 10;
  while( max < 100000000 )
  {
    if( inValue < max ){ break; }
    max *= 10;
    ++disit;
  }
  return disit;
}


//---------------------数値入力---------------------------------------------------------------------


//==============================================================================================
//  数値入力処理
//==============================================================================================
static void NumInput_Setup( NUMINPUT_WORK* wk, STRBUF* buf, GFL_BMPWIN* win, GFL_FONT* font,
            PRINT_UTIL* util, PRINT_QUE* que, const STR_PARAM* strParam, u32 value, const MAKE_WORK* make_wk )
{
  PRINTSYS_LSB color = GetBoxSubBGColor( strParam->Type );

  wk->Win = win;
  wk->Bmp = GFL_BMPWIN_GetBmp( win );
  wk->Font = font;
  wk->Buf = buf;
  wk->PrintUtil = util;
  wk->PrintQue = que;
  wk->Param = strParam;

  wk->Draw_ox = strParam->TouchX + NUMBOX_MARGIN;
  wk->ColorDefBg = PRINTSYS_LSB_GetB( color );
  wk->ColorCurBg = COLIDX_RED_L;
  wk->ColorLetter = PRINTSYS_LSB_GetL( color );

  wk->Num = value;
#if 0  //@todo
  switch( strParam->arg ){
  default:
    wk->numMax = boxParam->arg;
    wk->numMin = boxParam->arg2;
    break;
  case INPUTBOX_NUM_ARG_DEPEND:
    wk->numMax = box_getvalue( mainWork, boxParam->arg2 );
    wk->numMin = 0;
    break;
  case INPUTBOX_NUM_ARG_EXP:
    wk->numMax = mainWork->pokeExpMax;
    wk->numMin = mainWork->pokeExpMin;
    break;
  }
#else
  wk->NumMax = strParam->arg;
  wk->NumMin = strParam->arg2;
  wk->DispNumWork = value;
#endif
  wk->Disit = CalcDisit( wk->NumMax );
  wk->CurDisit = 0;

  SetAryNumInput( wk, value );
  DrawNumInput( wk );
}
static BOOL NumInput_Main( NUMINPUT_WORK* wk )
{
  u32 x,y;
  u16 key = GFL_UI_KEY_GetTrg();

  if( (key & PAD_BUTTON_A)|| GFL_UI_TP_GetPointTrg( &x, &y ) ){
    wk->Num = CalcInputNum( wk );
    return TRUE;
  }
  if( key & PAD_BUTTON_B ){
    return TRUE;
  }

  {
    u8 update = TRUE;
    u8 up_key_flg = FALSE;
    do {
      if( key & PAD_KEY_LEFT ){
        if( wk->CurDisit < (wk->Disit-1) ){
          wk->CurDisit++;
          break;
        }
      }
      if( key & PAD_KEY_RIGHT ){
        if( wk->CurDisit > 0 ){
          wk->CurDisit--;
          break;
        }
      }
      if( key & PAD_KEY_UP )
      {
        int k = wk->CurDisit;
        while( k < (wk->Disit) ){
          wk->NumAry[k]++;
          if( wk->NumAry[k] <= 9 ){ break; }
          wk->NumAry[k] = 0;
          ++k;
        }
        // 桁あふれしたら最大値に
        if( k == (wk->Disit) ){
          OS_Printf("↑　ケタあふれ\n");
          SetAryNumInput( wk, wk->NumMax );
        }
        //上キー押下チェックON
        up_key_flg = TRUE;
        break;
      }
      if( key & PAD_KEY_DOWN )
      {
        int k = wk->CurDisit;
        while( k < (wk->Disit) )
        {
          wk->NumAry[k]--;
          if( wk->NumAry[k] >= 0 ){ break; }
          wk->NumAry[k] = 9;
          ++k;
        }
        // 桁あふれしたら最小値に
        if( k == (wk->Disit) ){
          OS_Printf("↓　ケタあふれ\n");
          SetAryNumInput( wk, wk->NumMin );
        }
        break;
      }
      if( key & PAD_BUTTON_L ){
        SetAryNumInput( wk, wk->NumMax );
        break;
      }
      if( key & PAD_BUTTON_R ){
        SetAryNumInput( wk, wk->NumMin );
        break;
      }

      update = FALSE;
    }while(0);

    if( update ){
      u32 num = CalcInputNum( wk );

      //数値加算したのに値が下回ったをチェック（値のオーバーフローチェック）
      if ( up_key_flg )
      {
        if (num < wk->DispNumWork)
        {
          num = wk->NumMax;
          SetAryNumInput( wk, num );
        }
      }

      if( num > wk->NumMax ){
        num = wk->NumMax;
        SetAryNumInput( wk, num );
      }
      if( num < wk->NumMin ){
        num = wk->NumMin;
        SetAryNumInput( wk, num );
      }
      DrawNumInput( wk );
      wk->DispNumWork = num;
    }
  }
  return FALSE;
}
static u32 NumInput_GetNum( NUMINPUT_WORK* wk )
{
  return wk->Num;
}
static void DrawNumInput( NUMINPUT_WORK* wk )
{
  const STR_PARAM* p = wk->Param;

  GFL_BMP_Fill( wk->Bmp, p->TouchX, p->TouchY, p->TouchW, p->TouchH, wk->ColorDefBg );
  {
    int i;
    u32 xpos = wk->Draw_ox;
    u32 ypos = p->TouchY;
    PRINTSYS_LSB color;
    u8 col_bg;

    for(i=wk->Disit-1; i>=0; --i)
    {
      col_bg = (i==wk->CurDisit)? wk->ColorCurBg : wk->ColorDefBg;
      color = PRINTSYS_LSB_Make( wk->ColorLetter, 0, col_bg );
      STRTOOL_SetUnsignedNumber( wk->Buf, wk->NumAry[i], 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_ZENKAKU );
      GFL_BMP_Fill( wk->Bmp, xpos, p->TouchY, NUMBOX_CHAR_WIDTH, p->TouchH, col_bg );
      PRINT_UTIL_PrintColor( wk->PrintUtil, wk->PrintQue, xpos, ypos, wk->Buf, wk->Font, color );
      xpos += NUMBOX_CHAR_WIDTH;
      GFL_BMPWIN_TransVramCharacter( wk->Win );
    }
  }
}
static void SetAryNumInput( NUMINPUT_WORK* wk, u32 value )
{
  u32 k = 1000000000;
  u32 i = NUMINPUT_DISIT_MAX-1;

  GFL_STD_MemClear( wk->NumAry, sizeof(wk->NumAry) );
  while(i){
    wk->NumAry[i] = value / k;
    value -= (wk->NumAry[i] * k);
    k /= 10;
    i--;
  }
  wk->NumAry[i] = value;
}
static u32 CalcInputNum( NUMINPUT_WORK* wk )
{
  u32 i, k;
  u32 num;
  num = 0;
  for(i=0, k=1; i<wk->Disit; ++i, k*=10)
  {
    num += ( wk->NumAry[i] * k );
  }
  return num;
}




//==============================================================================================
//  入力補完処理
//==============================================================================================
static void COMPSKB_Setup( COMP_SKB_WORK* wk, GFL_SKB* skb, STRBUF* buf, u32 msgDataID, HEAPID heapID )
{
  wk->skb = skb;
  wk->msg = GFL_MSG_Create( GFL_MSG_LOAD_FAST, ARCID_MESSAGE, msgDataID, heapID );
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
  GFL_MSG_Delete( wk->msg );
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
        GFL_MSG_GetString( wk->msg, idx, wk->fullword );
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
    fSearchReq = TRUE;
    break;
  case GFL_SKB_REACTION_NONE:
    {
      u16 key = GFL_UI_KEY_GetTrg();
      if( key & PAD_BUTTON_SELECT ){
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
        GFL_MSG_GetString( wk->msg, wk->index, wk->fullword );
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
    str_cnt = GFL_MSG_GetStrCount( wk->msg );
    i = (org_idx < 0)? 0 : org_idx+1;
    while( i < str_cnt )
    {
      GFL_MSG_GetString( wk->msg, i, wk->fullword );
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

    str_cnt = GFL_MSG_GetStrCount( wk->msg );
    while( i < str_cnt )
    {
      GFL_MSG_GetString( wk->msg, i, wk->fullword );
      if( GFL_STR_CompareBuffer(word, wk->fullword) ){
        *match_idx = i;
        return TRUE;
      }
      ++i;
    }
  }
  return FALSE;
}

static BOOL is_hiragana( const STRBUF* buf )
{
  const STRCODE* ptr;
  ptr = GFL_STR_GetStringCodePointer( buf );
  if( (*ptr >= 0x3041) && (*ptr <= 0x3093) ){
    return TRUE;
  }
  return FALSE;
}











static void MakeData(MAKE_WORK *wk)
{
  MYSTATUS * my;
  my = &wk->UNData.aMyStatus;
  //マイステータス作成
  MyStatus_Init(my);
  {
    //名前
    static const STRCODE default_name[] = {
      0x30d6, 0x30e9, 0x30c3, 0x30af, 0xffff,
    };
    
    if ( GFL_STR_GetBufferLength(wk->NameBuf) ) 
    {
      MyStatus_SetMyNameFromString(my, wk->NameBuf);
    }
    else
    {
      MyStatus_SetMyName(my, default_name);
    }
    //ID
    MyStatus_SetID(my, wk->BoxValue[EDITBOX_ID_TRID]);
    //プロファイル※
    //国(地域※)
    MyStatus_SetMyNationArea(my, wk->BoxValue[EDITBOX_ID_COUNTRY], 0);
    //リージョン※
    //ロムコード※
    //ビュー
    MyStatus_SetTrainerView( my, wk->BoxValue[EDITBOX_ID_OBJIDX] );
    //性別
    MyStatus_SetMySex(my, wk->BoxValue[EDITBOX_ID_SEX]);
  }
  wk->UNData.recvPokemon = wk->BoxValue[EDITBOX_ID_RECV];  //貰ったポケモン
  wk->UNData.sendPokemon = wk->BoxValue[EDITBOX_ID_SEND];  //あげたポケモン
  wk->UNData.favorite = wk->BoxValue[EDITBOX_ID_FAVORITE];   //趣味
  wk->UNData.countryCount = wk->BoxValue[EDITBOX_ID_NUM];  //交換した国の回数
  wk->UNData.nature = wk->BoxValue[EDITBOX_ID_NATURE];   //性格

  //追加
  UNDATAUP_Update(wk->wh, &wk->UNData);

  OS_Printf("UN DATA ADD\n");
}

static void DumpUNData(MAKE_WORK *wk)
{
  int i;
  int valid_num;
  MYSTATUS * my;
  UNITEDNATIONS_SAVE *un_data;
  un_data = WIFIHISTORY_GetUNDataPtr(wk->wh);
  valid_num = 0;
  OS_Printf("-- DUMP UN DATA --\n");
  for (i=0;i<20;i++)
  {
    OS_Printf("DATA IDX = %d  START\n",i);
    my = &un_data[i].aMyStatus;
    //マイステータス系
    OS_Printf("TRID = %d\n",MyStatus_GetID(my));
    OS_Printf("COUNTRY = %d\n",MyStatus_GetMyNation(my));
    if ( MyStatus_GetMySex(my) ) OS_Printf("SEX = ♀\n");
    else OS_Printf("SEX = ♂\n");
    OS_Printf("VIEW = %d\n",MyStatus_GetTrainerView(my));
    //そのほか
    OS_Printf("RECV = %d\n",un_data[i].recvPokemon);
    OS_Printf("SEND = %d\n",un_data[i].sendPokemon);
    OS_Printf("FAVORITE = %d\n",un_data[i].favorite);
    OS_Printf("NATURE = %d\n",un_data[i].nature);
    OS_Printf("TRADE = %d\n",un_data[i].countryCount);
    OS_Printf("TALK = %d\n",un_data[i].bTalk);
    OS_Printf("VALID = %d\n",un_data[i].valid);
    if (un_data[i].valid) valid_num++;
  }

  OS_Printf("有効データ数 %d\n",valid_num);
}

