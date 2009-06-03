//======================================================================
/**
 * @file    debug_makepoke.c
 * @brief   デバッグ用ポケモンデータ生成ツール
 * @author  taya
 * @data    09/05/29
 */
//======================================================================
#include <gflib.h>
#include <skb.h>
#include "system/main.h"
#include "print/printsys.h"
#include "print/str_tool.h"

#include "font/font.naix"
#include "arc_def.h"
#include "message.naix"
#include "msg/msg_debug_makepoke.h"

#include "debug/debug_makepoke.h"

//--------------------------------------------------------------
/**
 *  Consts
 */
//--------------------------------------------------------------
enum {
  PROC_HEAP_SIZE = 0x18000,
  PRINT_FRAME = GFL_BG_FRAME1_M,

  PRINT_PALIDX  = 0,
  SKB_PALIDX1   = 1,
  SKB_PALIDX2   = 2,

  STRBUF_LEN = 64,
  NUMINPUT_KETA_MAX = 10,

  COLIDX_BLACK=1,
  COLIDX_GRAY,
  COLIDX_RED_D,
  COLIDX_RED_L,
  COLIDX_BULE_D,
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

#define CALC_NUMBOX_WIDTH(keta)      ((keta)*NUMBOX_CHAR_WIDTH +NUMBOX_MARGIN*2)
#define CALC_STRBOX_WIDTH(letterCnt)   ((letterCnt)*8 +8)
#define CALC_CAP_BOX_MARGIN(letterCnt)  ((letterCnt*8)+4)

enum {
  LINE_HEIGHT = 12,
  LINE_MARGIN = 2,
  LINE_DIFFER = LINE_HEIGHT + LINE_MARGIN,

  LY_LV1 = 6,
  LY_LV2 = LY_LV1+LINE_DIFFER,
  LY_LV3 = LY_LV2+LINE_DIFFER,
  LY_LV4 = LY_LV3+LINE_DIFFER,
  LY_LV5 = LY_LV4+LINE_DIFFER,
  LY_LV6 = LY_LV5+LINE_DIFFER,
  LY_LV7 = LY_LV6+LINE_DIFFER,
  LY_LV8 = LY_LV7+LINE_DIFFER,
  LY_LV9 = LY_LV8+LINE_DIFFER,
  LY_LV10 = LY_LV9+LINE_DIFFER,
  LY_LV11 = LY_LV10+LINE_DIFFER,
  LY_LV12 = LY_LV11+LINE_DIFFER,

  LX_POKETYPE_CAP = 4,
  LY_POKETYPE_CAP = LY_LV1,
  LX_POKETYPE_BOX = LX_POKETYPE_CAP+CALC_CAP_BOX_MARGIN(4),
  LY_POKETYPE_BOX = LY_LV1,

  LX_LEVEL_CAP = 116,
  LY_LEVEL_CAP = LY_LV1,
  LX_LEVEL_BOX = LX_LEVEL_CAP+CALC_CAP_BOX_MARGIN(3),
  LY_LEVEL_BOX = LY_LEVEL_CAP,

  LX_SEX_CAP = 180,
  LY_SEX_CAP = LY_LV1,
  LX_SEX_BOX = LX_SEX_CAP+CALC_CAP_BOX_MARGIN(4),
  LY_SEX_BOX = LY_SEX_CAP,

  LX_ITEM_CAP = 4,
  LY_ITEM_CAP = LY_LV2,
  LX_ITEM_BOX = LX_ITEM_CAP + CALC_CAP_BOX_MARGIN(4),
  LY_ITEM_BOX = LY_ITEM_CAP,

  LX_WAZA1_CAP = 4,
  LY_WAZA1_CAP = LY_LV4,
  LX_WAZA1_BOX = LX_WAZA1_CAP + CALC_CAP_BOX_MARGIN(3),
  LY_WAZA1_BOX = LY_WAZA1_CAP,

  LX_WAZA2_CAP = 4,
  LY_WAZA2_CAP = LY_LV5,
  LX_WAZA2_BOX = LX_WAZA2_CAP + CALC_CAP_BOX_MARGIN(3),
  LY_WAZA2_BOX = LY_WAZA2_CAP,

  LX_WAZA3_CAP = 4,
  LY_WAZA3_CAP = LY_LV6,
  LX_WAZA3_BOX = LX_WAZA3_CAP + CALC_CAP_BOX_MARGIN(3),
  LY_WAZA3_BOX = LY_WAZA3_CAP,

  LX_WAZA4_CAP = 4,
  LY_WAZA4_CAP = LY_LV7,
  LX_WAZA4_BOX = LX_WAZA4_CAP + CALC_CAP_BOX_MARGIN(3),
  LY_WAZA4_BOX = LY_WAZA4_CAP,

  NUMBOX_MARGIN = 4,
  NUMBOX_CHAR_WIDTH = 8,

};

typedef enum {
  INPUTBOX_TYPE_STR,
  INPUTBOX_TYPE_NUM,
  INPUTBOX_TYPE_SWITCH,
}InputBoxType;

typedef enum {
  INPUTBOX_ID_POKETYPE=0,
  INPUTBOX_ID_LEVEL,
  INPUTBOX_ID_SEX,
  INPUTBOX_ID_ITEM,
  INPUTBOX_ID_WAZA1,
  INPUTBOX_ID_WAZA2,
  INPUTBOX_ID_WAZA3,
  INPUTBOX_ID_WAZA4,

  INPUTBOX_ID_MAX,

}InputBoxID;

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

//--------------------------------------------------------------
/**
 *  Input Box Params
 */
//--------------------------------------------------------------
typedef struct {
  u16  cap_strID;
  u8   cap_xpos;
  u8   cap_ypos;

  u8   xpos;
  u8   ypos;
  u8   width;
  u8   height;

  u8   color_bg;
  u8   color_letter;
  u8   type;

  u16  paraID;
  u32  arg;
  u32  arg2;

}INPUT_BOX_PARAM;


static const INPUT_BOX_PARAM InputBoxParams[] = {

  { DMPSTR_POKEMON, LX_POKETYPE_CAP, LY_POKETYPE_CAP,
    LX_POKETYPE_BOX, LY_POKETYPE_BOX, 56, LINE_HEIGHT,
    COLIDX_ORANGE_D, COLIDX_BLACK, INPUTBOX_TYPE_STR,
    ID_PARA_monsno, NARC_message_monsname_dat, 0  },

  { DMPSTR_LEVEL,   LX_LEVEL_CAP,  LY_LEVEL_CAP,
    LX_LEVEL_BOX,   LY_LEVEL_BOX,  CALC_NUMBOX_WIDTH(3), LINE_HEIGHT,
    COLIDX_BLUE_L,  COLIDX_BLACK,  INPUTBOX_TYPE_NUM,
    ID_PARA_level, 100, 1 },

  { DMPSTR_SEX,   LX_SEX_CAP,  LY_SEX_CAP,
    LX_SEX_BOX,   LY_SEX_BOX,  CALC_NUMBOX_WIDTH(1), LINE_HEIGHT,
    COLIDX_GREEN_L,  COLIDX_BLACK,  INPUTBOX_TYPE_SWITCH,
    ID_PARA_sex, DMPSTR_SEX_MALE, 0 },

  { DMPSTR_ITEM,    LX_ITEM_CAP,    LY_ITEM_CAP,
    LX_ITEM_BOX,    LY_ITEM_BOX,    CALC_STRBOX_WIDTH(8),   LINE_HEIGHT,
    COLIDX_ORANGE_D,   COLIDX_BLACK, INPUTBOX_TYPE_STR,
    ID_PARA_item, NARC_message_itemname_dat, 0 },

  { DMPSTR_WAZA1,   LX_WAZA1_CAP,   LY_WAZA1_CAP,
    LX_WAZA1_BOX,   LY_WAZA1_BOX,   CALC_STRBOX_WIDTH(7), LINE_HEIGHT,
    COLIDX_ORANGE_D,  COLIDX_BLACK,   INPUTBOX_TYPE_STR,
    ID_PARA_waza1, NARC_message_wazaname_dat,  0 },

  { DMPSTR_WAZA2,   LX_WAZA2_CAP,   LY_WAZA2_CAP,
    LX_WAZA2_BOX,   LY_WAZA2_BOX,   CALC_STRBOX_WIDTH(7), LINE_HEIGHT,
    COLIDX_ORANGE_D,    COLIDX_BLACK,   INPUTBOX_TYPE_STR,
    ID_PARA_waza2, NARC_message_wazaname_dat,    0 },

  { DMPSTR_WAZA3,   LX_WAZA3_CAP,   LY_WAZA3_CAP,
    LX_WAZA3_BOX,   LY_WAZA3_BOX,   CALC_STRBOX_WIDTH(7), LINE_HEIGHT,
    COLIDX_ORANGE_D,    COLIDX_BLACK,   INPUTBOX_TYPE_STR,
    ID_PARA_waza3, NARC_message_wazaname_dat,    0 },

  { DMPSTR_WAZA4,   LX_WAZA4_CAP,   LY_WAZA4_CAP,
    LX_WAZA4_BOX,   LY_WAZA4_BOX,   CALC_STRBOX_WIDTH(7), LINE_HEIGHT,
    COLIDX_ORANGE_D,    COLIDX_BLACK,   INPUTBOX_TYPE_STR,
    ID_PARA_waza4, NARC_message_wazaname_dat,    0 },

};

//--------------------------------------------------------------
/**
 *  数値入力ワーク
 */
//--------------------------------------------------------------
typedef struct {
  GFL_BMPWIN*   win;
  GFL_BMP_DATA* bmp;
  GFL_FONT*     font;
  STRBUF*       buf;
  PRINT_UTIL*   printUtil;
  PRINT_QUE*    printQue;
  const INPUT_BOX_PARAM* boxParam;

  u32  num;
  u32  keta;
  u32  numMax;
  u8   draw_ox;
  u8   color_def_bg;
  u8   color_cur_bg;
  u8   color_letter;

  u8   num_ary[ NUMINPUT_KETA_MAX ];
  u8   cur_keta;

}NUMINPUT_WORK;

//--------------------------------------------------------------
/**
 *  Proc Work
 */
//--------------------------------------------------------------
typedef struct {

  GFL_BMPWIN*     win;
  GFL_BMP_DATA*   bmp;
  GFL_FONT*       font;
  GFL_MSGDATA*    msgData;
  STRBUF*         strbuf;
  PRINT_QUE*      printQue;
  PRINT_UTIL      printUtil;
  GFL_SKB*        skb;
  int             boxIdx;
  u32             boxValue[ INPUTBOX_ID_MAX ];
  u8              boxEnable[ INPUTBOX_ID_MAX ];

  POKEMON_PARAM*  dst;
  HEAPID  heapID;
  u8      seq;

  COMP_SKB_WORK   comp;
  NUMINPUT_WORK   numInput;

}DMP_MAINWORK;

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static GFL_PROC_RESULT PROC_MAKEPOKE_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT PROC_MAKEPOKE_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT PROC_MAKEPOKE_Quit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static void setup_view( DMP_MAINWORK* wk );
static void cleanup_view( DMP_MAINWORK* wk );
static BOOL root_ctrl( DMP_MAINWORK* wk );
static int check_box_touch( DMP_MAINWORK* wk );
static void print_caption( DMP_MAINWORK* wk, const INPUT_BOX_PARAM* p );
static void box_initwork( DMP_MAINWORK* wk );
static void box_setup( DMP_MAINWORK* wk, u32 boxID, const POKEMON_PARAM* pp );
static void box_update( DMP_MAINWORK* wk, u32 boxID, u32 value );
static void box_write_fixval( DMP_MAINWORK* wk, u32 boxID, u32 value );
static void  box_getstr( DMP_MAINWORK* wk, u32 boxID, STRBUF* buf );
static int box_getvalue( DMP_MAINWORK* wk, u32 boxIdx );
static u16 calc_keta( u32 value );
static void numinput_draw( NUMINPUT_WORK* wk );
static void numinput_sub_setary( NUMINPUT_WORK* wk, u32 value );
static u32 numinput_sub_calcnum( NUMINPUT_WORK* wk );
static void NumInput_Setup( NUMINPUT_WORK* wk, STRBUF* buf, GFL_BMPWIN* win, GFL_FONT* font,
            PRINT_UTIL* util, PRINT_QUE* que, const INPUT_BOX_PARAM* boxParam, u32 value );
static BOOL NumInput_Main( NUMINPUT_WORK* wk );
static u32 NumInput_GetNum( NUMINPUT_WORK* wk );
static void COMPSKB_Setup( COMP_SKB_WORK* wk, GFL_SKB* skb, STRBUF* buf, u32 msgDataID, HEAPID heapID );
static void COMPSKB_Cleanup( COMP_SKB_WORK* wk );
static BOOL sub_strncmp( const STRBUF* str1, const STRBUF* str2, u32 len );
static u32 sub_search( COMP_SKB_WORK* wk, const STRBUF* word, int org_idx, int* first_idx );
static BOOL COMPSKB_Main( COMP_SKB_WORK* wk );
static u32 COMPSKB_GetWordIndex( const COMP_SKB_WORK* wk );
static u32 get_personal( const POKEMON_PARAM* pp, PokePersonalParamID paramID );
static u8 personal_get_tokusei_kinds( const POKEMON_PARAM* pp );
static BOOL personal_is_sex_fixed( const POKEMON_PARAM* pp );

//--------------------------------------------------------------
/**
 *  Proc Data
 */
//--------------------------------------------------------------
const GFL_PROC_DATA ProcData_DebugMakePoke = {
  PROC_MAKEPOKE_Init,
  PROC_MAKEPOKE_Main,
  PROC_MAKEPOKE_Quit,
};


static GFL_PROC_RESULT PROC_MAKEPOKE_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_DEBUG_MAKEPOKE,   PROC_HEAP_SIZE );

  {
    DMP_MAINWORK* wk = GFL_PROC_AllocWork( proc, sizeof(DMP_MAINWORK), HEAPID_DEBUG_MAKEPOKE );
    if( wk )
    {
      const PROCPARAM_DEBUG_MAKEPOKE* proc_param = (const PROCPARAM_DEBUG_MAKEPOKE*)pwk;

      wk->heapID = HEAPID_DEBUG_MAKEPOKE;
      wk->dst = proc_param->dst;
      wk->seq = 0;
    }
  }

  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT PROC_MAKEPOKE_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  DMP_MAINWORK* wk = mywk;

  switch( *seq ){
  case 0:
    setup_view( wk );
    (*seq)++;
    break;
  case 1:
    if( root_ctrl(wk) ){
      (*seq)++;
    }
    break;
  case 2:
    cleanup_view( wk );
    return GFL_PROC_RES_FINISH;
  }

  return GFL_PROC_RES_CONTINUE;
}
static GFL_PROC_RESULT PROC_MAKEPOKE_Quit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_DEBUG_MAKEPOKE );
  return GFL_PROC_RES_FINISH;
}




//----------------------------------------------------------------------------------
/**
 * 画面構築
 *
 * @param   wk
 */
//----------------------------------------------------------------------------------
static void setup_view( DMP_MAINWORK* wk )
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

  // BGsystem初期化
  GFL_BG_Init( wk->heapID );
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

  GFL_BMPWIN_Init( wk->heapID );
  GFL_FONTSYS_Init();

  // VRAMバンク設定
  GFL_DISP_SetBank( &vramBank );

  // 各種効果レジスタ初期化
  G2_BlendNone();
  G2S_BlendNone();

  // 上下画面設定
  GX_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );

  wk->win = GFL_BMPWIN_Create( PRINT_FRAME, 0, 0, 32, 24, PRINT_PALIDX, GFL_BMP_CHRAREA_GET_F );
  wk->bmp = GFL_BMPWIN_GetBmp( wk->win );
  wk->font = GFL_FONT_Create( ARCID_FONT, NARC_font_small_nftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );
  wk->msgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_debug_makepoke_dat, wk->heapID );
  wk->strbuf = GFL_STR_CreateBuffer( STRBUF_LEN, wk->heapID );
  wk->printQue = PRINTSYS_QUE_Create( wk->heapID );
  PRINT_UTIL_Setup( &wk->printUtil, wk->win );

  GFL_BMP_Clear( wk->bmp, 0x0f );
  GFL_BMPWIN_MakeScreen( wk->win );
  GFL_BG_LoadScreenReq( PRINT_FRAME );

  GFL_BMPWIN_TransVramCharacter( wk->win );
}
//----------------------------------------------------------------------------------
/**
 * 画面クリーンアップ
 *
 * @param   wk
 */
//----------------------------------------------------------------------------------
static void cleanup_view( DMP_MAINWORK* wk )
{
  PRINTSYS_QUE_Delete( wk->printQue );
  GFL_STR_DeleteBuffer( wk->strbuf );
  GFL_MSG_Delete( wk->msgData );
  GFL_FONT_Delete( wk->font );
  GFL_BMPWIN_Delete( wk->win );
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
static BOOL root_ctrl( DMP_MAINWORK* wk )
{
  if( PRINT_UTIL_Trans(&wk->printUtil, wk->printQue) )
  {
    switch( wk->seq ){
    case 0:
      {
        int i;
        for(i=0; i<NELEMS(InputBoxParams); ++i){
          print_caption( wk, &InputBoxParams[i] );
        }
      }
      wk->seq++;
      break;
    case 1:
      {
        int i;
        for(i=0; i<NELEMS(InputBoxParams); ++i){
          box_setup( wk, i, wk->dst );
        }
        wk->seq++;
      }
      break;
    case 2:
      wk->boxIdx = check_box_touch( wk );
      if( wk->boxIdx >= 0 )
      {
        const INPUT_BOX_PARAM* p = &InputBoxParams[ wk->boxIdx ];
        switch( p->type ){
        case INPUTBOX_TYPE_STR:
          {
            static const GFL_SKB_SETUP setup = {
              STRBUF_LEN, GFL_SKB_STRTYPE_STRBUF,
              GFL_SKB_MODE_KATAKANA, TRUE, 0,
              PRINT_FRAME, SKB_PALIDX1, SKB_PALIDX2,
            };
            box_getstr( wk, wk->boxIdx, wk->strbuf );
            wk->skb = GFL_SKB_Create( (void*)(wk->strbuf), &setup, wk->heapID );
            COMPSKB_Setup( &wk->comp, wk->skb, wk->strbuf, p->arg, wk->heapID );
            wk->seq++;
          }
          break;
        case INPUTBOX_TYPE_NUM:
          NumInput_Setup( &wk->numInput, wk->strbuf, wk->win, wk->font,
              &wk->printUtil, wk->printQue, p, box_getvalue(wk, wk->boxIdx) );
          wk->seq += 2;
          break;
        case INPUTBOX_TYPE_SWITCH:
          {
            u8 val = box_getvalue( wk, wk->boxIdx );
            val = !val;
            box_update( wk, wk->boxIdx, val );
          }
          break;
        }
      }
      break;
    case 3:
      if( COMPSKB_Main(&wk->comp) )
      {
        int idx = COMPSKB_GetWordIndex( &wk->comp );
        if( idx >= 0 ){
          box_update( wk, wk->boxIdx, idx );
        }
        COMPSKB_Cleanup( &wk->comp );
        GFL_SKB_Delete( wk->skb );
        wk->seq = 2;
      }
      break;
    case 4:
      if( NumInput_Main(&wk->numInput) )
      {
        u32 num = NumInput_GetNum( &wk->numInput );
        box_update( wk, wk->boxIdx, num );
        wk->seq = 2;
      }
    }
  }
  return FALSE;
}

static int check_box_touch( DMP_MAINWORK* wk )
{
  u32 x, y;
  if( GFL_UI_TP_GetPointTrg( &x, &y ) )
  {
    u32 i;
    for(i=0; i<NELEMS(InputBoxParams); ++i)
    {
      if( wk->boxEnable[i] )
      {
        const INPUT_BOX_PARAM* p = &InputBoxParams[i];
        if( (x >= p->xpos) && (x <= (p->xpos + p->width))
        &&  (y >= p->ypos) && (y <= (p->ypos + p->height))
        ){
          return i;
        }
      }
    }
  }
  return -1;
}

//----------------------------------------------------------------------------------
/**
 * 項目名描画
 *
 * @param   wk
 * @param   p
 */
//----------------------------------------------------------------------------------
static void print_caption( DMP_MAINWORK* wk, const INPUT_BOX_PARAM* p )
{
  u32 ypos = p->cap_ypos + ((LINE_HEIGHT - GFL_FONT_GetLineHeight(wk->font)) / 2);
  GFL_MSG_GetString( wk->msgData, p->cap_strID, wk->strbuf );
  PRINT_UTIL_Print( &wk->printUtil, wk->printQue, p->cap_xpos, ypos, wk->strbuf, wk->font );
  if( p->cap_strID == DMPSTR_ITEM )
  {
    TAYA_Printf("*****アイテム書いた x=%d,y=%d\n", p->cap_xpos, ypos);
  }

}


static void box_initwork( DMP_MAINWORK* wk )
{
  int i;
  for(i=0; i<INPUTBOX_ID_MAX; ++i){
    wk->boxEnable[i] = FALSE;
  }
}
static void box_setup( DMP_MAINWORK* wk, u32 boxID, const POKEMON_PARAM* pp )
{
  const INPUT_BOX_PARAM* p = &InputBoxParams[ boxID ];
  u32 value = PP_Get( pp, p->paraID, NULL );

  switch( boxID ){
  case INPUTBOX_ID_SEX:
    if( personal_is_sex_fixed( pp ) )
    {
      box_write_fixval( wk, boxID, value );
      return;
    }
    break;

  default:
    break;
  }
  wk->boxEnable[boxID] = TRUE;
  box_update( wk, boxID, value );
}

static void box_update( DMP_MAINWORK* wk, u32 boxID, u32 value )
{
  const INPUT_BOX_PARAM* p = &InputBoxParams[ boxID ];
  u32 str_width, str_height, xpos, ypos;
  PRINTSYS_LSB color;


  GFL_BMP_Fill( wk->bmp, p->xpos, p->ypos, p->width, p->height, p->color_bg );

  wk->boxValue[ boxID ] = value;
  box_getstr( wk, boxID, wk->strbuf );

  str_width = PRINTSYS_GetStrWidth( wk->strbuf, wk->font, 0 );
  xpos = p->xpos;
  if( str_width < p->width ){
    xpos += (p->width - str_width) / 2;
  }
  str_height = GFL_FONT_GetLineHeight( wk->font );
  ypos = p->ypos;
  if( str_height < p->height ){
    ypos += (p->height - str_height) / 2;
  }

  color = PRINTSYS_LSB_Make( p->color_letter, 0, p->color_bg );
  PRINT_UTIL_PrintColor( &wk->printUtil, wk->printQue, xpos, ypos, wk->strbuf, wk->font, color );
}

static void box_write_fixval( DMP_MAINWORK* wk, u32 boxID, u32 value )
{
  const INPUT_BOX_PARAM* p = &InputBoxParams[ boxID ];
  u32 str_width, str_height, xpos, ypos;
  PRINTSYS_LSB color;

  wk->boxValue[ boxID ] = value;
  box_getstr( wk, boxID, wk->strbuf );

  str_width = PRINTSYS_GetStrWidth( wk->strbuf, wk->font, 0 );
  xpos = p->xpos;
  if( str_width < p->width ){
    xpos += (p->width - str_width) / 2;
  }
  str_height = GFL_FONT_GetLineHeight( wk->font );
  ypos = p->ypos;
  if( str_height < p->height ){
    ypos += (p->height - str_height) / 2;
  }

  color = PRINTSYS_LSB_Make( p->color_letter, 0, COLIDX_WHITE );
  PRINT_UTIL_PrintColor( &wk->printUtil, wk->printQue, xpos, ypos, wk->strbuf, wk->font, color );

}

static void  box_getstr( DMP_MAINWORK* wk, u32 boxID, STRBUF* buf )
{
  const INPUT_BOX_PARAM* p = &InputBoxParams[ boxID ];
  u32 value = wk->boxValue[ boxID ];

  switch( p->type ){
  case INPUTBOX_TYPE_STR:
    {
      GFL_MSGDATA* msgdat = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, p->arg, GFL_HEAP_LOWID(wk->heapID) );
      GFL_MSG_GetString( msgdat, value, wk->strbuf );
      GFL_MSG_Delete( msgdat );
    }
    break;
  case INPUTBOX_TYPE_NUM:
    STRTOOL_SetNumber( wk->strbuf, value, calc_keta(p->arg), STR_NUM_DISP_SPACE, STR_NUM_CODE_ZENKAKU );
    break;
  case INPUTBOX_TYPE_SWITCH:
    GFL_MSG_GetString( wk->msgData, p->arg+value, wk->strbuf );
    break;
  }
}

static int box_getvalue( DMP_MAINWORK* wk, u32 boxIdx )
{
  return wk->boxValue[ boxIdx ];
}

static u16 calc_keta( u32 value )
{
  u32 max, keta;
  keta = 1;
  max = 10;
  while( max < 100000000 )
  {
    if( value < max ){ break; }
    max *= 10;
    ++keta;
  }
  return keta;
}



static void numinput_draw( NUMINPUT_WORK* wk )
{
  const INPUT_BOX_PARAM* p = wk->boxParam;

  GFL_BMP_Fill( wk->bmp, p->xpos, p->ypos, p->width, p->height, p->color_bg );
  {
    int i;
    u32 xpos = wk->draw_ox;
    u32 ypos = p->ypos + ((p->height - GFL_FONT_GetLineHeight(wk->font)) / 2);
    PRINTSYS_LSB color;
    u8 col_bg;

    for(i=wk->keta-1; i>=0; --i)
    {
      col_bg = (i==wk->cur_keta)? wk->color_cur_bg : wk->color_def_bg;
      color = PRINTSYS_LSB_Make( wk->color_letter, 0, col_bg );
      STRTOOL_SetNumber( wk->buf, wk->num_ary[i], 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_ZENKAKU );
      GFL_BMP_Fill( wk->bmp, xpos, p->ypos, NUMBOX_CHAR_WIDTH, p->height, col_bg );
      PRINT_UTIL_PrintColor( wk->printUtil, wk->printQue, xpos, ypos, wk->buf, wk->font, color );
      xpos += NUMBOX_CHAR_WIDTH;
      GFL_BMPWIN_TransVramCharacter( wk->win );
    }
  }
}
static void numinput_sub_setary( NUMINPUT_WORK* wk, u32 value )
{
  u32 k = 1000000000;
  u32 i = NUMINPUT_KETA_MAX-1;
  while(i){
    wk->num_ary[i] = value / k;
    value -= (wk->num_ary[i] * k);
    k /= 10;
    i--;
  }
  wk->num_ary[i] = value;
}

static u32 numinput_sub_calcnum( NUMINPUT_WORK* wk )
{
  u32 i, k, num;
  num = 0;
  for(i=0, k=1; i<wk->keta; ++i, k*=10)
  {
    num += ( wk->num_ary[i] * k );
  }
  return num;
}

static void NumInput_Setup( NUMINPUT_WORK* wk, STRBUF* buf, GFL_BMPWIN* win, GFL_FONT* font,
            PRINT_UTIL* util, PRINT_QUE* que, const INPUT_BOX_PARAM* boxParam, u32 value )
{
  wk->win = win;
  wk->bmp = GFL_BMPWIN_GetBmp( win );
  wk->font = font;
  wk->buf = buf;
  wk->printUtil = util;
  wk->printQue = que;
  wk->boxParam = boxParam;

  wk->draw_ox = boxParam->xpos + NUMBOX_MARGIN;
  wk->color_def_bg = boxParam->color_bg;
  wk->color_cur_bg = COLIDX_RED_L;
  wk->color_letter = boxParam->color_letter;

  wk->num = value;
  wk->keta = calc_keta( boxParam->arg );
  wk->numMax = boxParam->arg;
  wk->cur_keta = 0;

  numinput_sub_setary( wk, value );
  numinput_draw( wk );
}
static BOOL NumInput_Main( NUMINPUT_WORK* wk )
{
  u16 key = GFL_UI_KEY_GetTrg();

  if( key & PAD_BUTTON_A ){
    wk->num = numinput_sub_calcnum( wk );
    return TRUE;
  }
  if( key & PAD_BUTTON_B ){
    return TRUE;
  }

  {
    u8 update = TRUE;
    do {
      if( key & PAD_KEY_LEFT ){
        if( wk->cur_keta < (wk->keta-1) ){
          wk->cur_keta++;
          break;
        }
      }
      if( key & PAD_KEY_RIGHT ){
        if( wk->cur_keta > 0 ){
          wk->cur_keta--;
          break;
        }
      }
      if( key & PAD_KEY_UP ){
        if( wk->num_ary[wk->cur_keta] < 9 ){
          wk->num_ary[wk->cur_keta]++;
        }else{
          wk->num_ary[wk->cur_keta] = 0;
        }
        break;
      }
      if( key & PAD_KEY_DOWN ){
        if( wk->num_ary[wk->cur_keta] != 0 ){
          wk->num_ary[wk->cur_keta]--;
        }else{
          wk->num_ary[wk->cur_keta] = 9;
        }
        break;
      }
      update = FALSE;
    }while(0);

    if( update ){
      u32 num = numinput_sub_calcnum( wk );
      if( num > wk->numMax ){
        num = wk->numMax;
        numinput_sub_setary( wk, num );
      }
      numinput_draw( wk );
    }
  }
  return FALSE;
}

static u32 NumInput_GetNum( NUMINPUT_WORK* wk )
{
  return wk->num;
}


//==============================================================================================
//  入力補完処理
//==============================================================================================
static void COMPSKB_Setup( COMP_SKB_WORK* wk, GFL_SKB* skb, STRBUF* buf, u32 msgDataID, HEAPID heapID )
{
  wk->skb = skb;
  wk->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, msgDataID, heapID );
  wk->buf = buf;
  wk->subword  = GFL_STR_CreateBuffer( 32, heapID );
  wk->fullword = GFL_STR_CreateBuffer( 32, heapID );
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

static BOOL sub_strncmp( const STRBUF* str1, const STRBUF* str2, u32 len )
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

static u32 sub_search( COMP_SKB_WORK* wk, const STRBUF* word, int org_idx, int* first_idx )
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
      if( sub_strncmp( word, wk->fullword, GFL_STR_GetBufferLength(word) ) ){
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

static BOOL COMPSKB_Main( COMP_SKB_WORK* wk )
{
  GflSkbReaction reaction = GFL_SKB_Main( wk->skb );

  if( reaction != GFL_SKB_REACTION_NONE
  &&  reaction != GFL_SKB_REACTION_PAGE ){
    wk->searchingFlag = FALSE;
  }

  switch( reaction ){
  case GFL_SKB_REACTION_QUIT:
    return TRUE;
  case GFL_SKB_REACTION_INPUT:
    {
      int idx;
      GFL_SKB_PickStr( wk->skb );
      if( sub_search(wk, wk->buf, -1, &idx) == 1 )
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
  case GFL_SKB_REACTION_PAGE:
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
        if( sub_search(wk, wk->subword, wk->index, &idx) )
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
    break;
  }
  return FALSE;
}

static u32 COMPSKB_GetWordIndex( const COMP_SKB_WORK* wk )
{
  return wk->index;
}
//==============================================================================================
//  パーソナルデータアクセス
//==============================================================================================
static u32 get_personal( const POKEMON_PARAM* pp, PokePersonalParamID paramID )
{
  u16 monsno = PP_Get( pp, ID_PARA_monsno,  NULL );
  u16 formno = PP_Get( pp, ID_PARA_form_no, NULL );
  return POKETOOL_GetPersonalParam( monsno, formno, paramID );
}

static u8 personal_get_tokusei_kinds( const POKEMON_PARAM* pp )
{
  u8 n = 1;
  if( get_personal(pp, POKEPER_ID_speabi2) ){
    ++n;
  }
  return n;
}
static BOOL personal_is_sex_fixed( const POKEMON_PARAM* pp )
{
  u32 v = get_personal( pp, POKEPER_ID_sex );
  if( (v == POKEPER_SEX_MALE)
  ||  (v == POKEPER_SEX_FEMALE)
  ||  (v == POKEPER_SEX_UNKNOWN)
  ){
    return TRUE;
  }
  return FALSE;
}


#if 0
//============================================================================================
/**
 *  指定されたパラメータになるように個性乱数を計算する
 *
 * @param[in] id          トレーナーID
 * @param[in] mons_no     個性乱数を計算するモンスターナンバー
 * @param[in] chr         性格
 * @param[in] sex         性別
 * @param[in] tokusei     特性（ 0 or 1 で指定 ）
 * @param[in] rare_flag   レアにするかどうか( FALSE:レアではない　TRUE:レアにする ）
 *
 * @retval  計算した個性乱数
 */
//============================================================================================
u32   PokeParaCalcPersonalRnd( u32 id, u16 mons_no, u8 chr, u8 sex, u8 tokusei, BOOL rare_flag )
{
  u32 rnd;

  //特性ナンバーではないので、2以上はアサートにする
  GF_ASSERT( tokusei < 2 );
  //性格と特性の奇数、偶数は一致したものしか作れないのでアサート
  GF_ASSERT( ( chr & 1 ) == tokusei );
  //性別にMONS_UNKNOWNは指定できないのでアサート
  GF_ASSERT( sex != MONS_UNKNOWN );

  if( rare_flag )
  {
    u32 mask = ( ( ( id & 0xffff0000 ) >> 16 ) ^ ( id & 0x0000ffff ) ) & 0xff00;

    rnd = PokeParaSexChrRndGet( mons_no, sex, chr );
    rnd |= ( mask ^ ( rnd & 0x0000ffff ) ) << 16;
  }
  else
  {
    u8 per_sex = PokePersonalParaGet( mons_no, ID_PER_sex );

    rnd = ( ( ( id & 0xffff0000 ) >> 16 ) ^ ( id & 0x0000ffff ) ) & 0xff00;
    rnd = ( rnd ^ 0xff00 ) << 16;
    rnd += ( chr - ( rnd % 25 ) );

    if( ( per_sex != MONS_MALE ) &&
        ( per_sex != MONS_FEMALE ) &&
        ( per_sex != MONS_UNKNOWN ) )
    {
      if( sex == PARA_MALE )
      {
        if( per_sex > ( rnd & 0xff ) )
        {
          rnd += 25 * ( ( ( per_sex - ( rnd & 0xff ) ) / 25 ) + 1 );
          if( ( rnd & 1 ) != tokusei )
          {
            GF_ASSERT( ( rnd & 0xff ) <= ( 255 - 25 ) );
            rnd += 25;
          }
        }
      }
      else
      {
        if( per_sex < ( rnd & 0xff ) )
        {
          rnd -= 25 * ( ( ( ( rnd & 0xff ) - per_sex ) / 25 ) + 1 );
          if( ( rnd & 1 ) != tokusei )
          {
            GF_ASSERT( ( rnd & 0xff ) >= 25 );
            rnd -= 25;
          }
        }
      }
    }
  }
  return rnd;
}
#endif


