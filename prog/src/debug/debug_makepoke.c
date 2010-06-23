//======================================================================
/**
 * @file    debug_makepoke.c
 * @brief   デバッグ用ポケモンデータ生成ツール
 * @author  taya & iwasawa
 * @data    09/05/29
 */
//======================================================================

#ifdef PM_DEBUG
#include <gflib.h>
#include <skb.h>
#include "system/main.h"
#include "print/printsys.h"
#include "print/str_tool.h"
#include "print/global_msg.h"
#include "poke_tool/monsno_def.h"
#include "poke_tool/poke_memo.h"
#include "waza_tool/wazano_def.h"
#include "item/item.h"
#include "buflen.h"

#include "font/font.naix"
#include "arc_def.h"
#include "message.naix"
#include "debug_message.naix"
#include "msg/debug/msg_debug_makepoke.h"

#include "debug/debug_makepoke.h"
#include "debug_makepoke_def.h"

enum {
  SEQ_DRAW_CAPTION = 0,
  SEQ_DRAW_BOX,
  SEQ_WAIT_CTRL,
  SEQ_INPUT_STR,
  SEQ_INPUT_NICKNAME,
  SEQ_INPUT_NUM,
  SEQ_PAGE_CHANGE,
  SEQ_EXIT,
};

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
  int           delayTimer;
}COMP_SKB_WORK;

//--------------------------------------------------------------
/**
 *  Input Box Params
 */
//--------------------------------------------------------------
typedef struct {
  u16  type;
  s16  cap_strID;
  u8   cap_xpos;
  u8   cap_ypos;

  u8   xpos;
  u8   ypos;
  u8   width;
  u8   height;

  u16  paraID;
  u16  pageNo;
  u32  arg;
  u32  arg2;
  u32  arg3;

}INPUT_BOX_PARAM;

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
  u32  numMin;
  u8   draw_ox;
  u8   color_def_bg;
  u8   color_cur_bg;
  u8   color_letter;

  s8   num_ary[ NUMINPUT_KETA_MAX ];
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
  STRBUF*         tmpbuf;
  PRINT_QUE*      printQue;
  PRINT_UTIL      printUtil;
  GFL_SKB*        skb;
  u32             pokeExpMin;
  u32             pokeExpMax;
  int             boxIdx;
  u32             boxValue[ INPUTBOX_ID_MAX ];
  u8              boxEnable[ INPUTBOX_ID_MAX ];

  POKEMON_PARAM*  dst;
  POKEMON_PARAM*  src;
  HEAPID  heapID;
  u8      seq;
  s8      pageNo;
  u8      touch_prev_flag;
  u32     reactionDelayTimer;

  u32       oyaID;
  u32       oyaID_org;
  STRCODE   oyaName[ PERSON_NAME_SIZE+EOM_SIZE ];
  u8        oyaSex;

  COMP_SKB_WORK   comp;
  NUMINPUT_WORK   numInput;
  GFL_SKB_SETUP   skbSetup;
  PROCPARAM_DEBUG_MAKEPOKE* proc_param;

}DMP_MAINWORK;

#include "debug_makepoke.cdat"

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static GFL_PROC_RESULT PROC_MAKEPOKE_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT PROC_MAKEPOKE_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT PROC_MAKEPOKE_Quit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );

static void UpdatePokeExpMinMax( DMP_MAINWORK* wk, const POKEMON_PARAM* pp );
static void main_setup_view( DMP_MAINWORK* wk );
static void main_cleanup_view( DMP_MAINWORK* wk );
static BOOL main_root_ctrl( DMP_MAINWORK* wk );

//ルートコントロール
static int root_wait_ctrl( DMP_MAINWORK* wk );
static int inputbox_type_str( DMP_MAINWORK* wk, const INPUT_BOX_PARAM* p );
static int inputbox_type_num( DMP_MAINWORK* wk, const INPUT_BOX_PARAM* p );
static int inputbox_type_switch( DMP_MAINWORK* wk, const INPUT_BOX_PARAM* p );
static int inputbox_type_btn( DMP_MAINWORK* wk, const INPUT_BOX_PARAM* p );

static int check_box_touch( DMP_MAINWORK* wk );
static BOOL is_hiragana( const STRBUF* buf );
static void default_waza_set( DMP_MAINWORK* wk, u16 waza );
static void rare_param_set( DMP_MAINWORK* wk );


///サブルーチン
static void update_dst( DMP_MAINWORK* wk );
static void print_caption( DMP_MAINWORK* wk, const INPUT_BOX_PARAM* p );
static PRINTSYS_LSB box_sub_get_bgcol( u8 input_type );
static void box_initwork( DMP_MAINWORK* wk );
static int box_getvalue( const DMP_MAINWORK* wk, u32 boxIdx );
static void box_setup( DMP_MAINWORK* wk, u32 boxID, const POKEMON_PARAM* pp );
static void box_update( DMP_MAINWORK* wk, u32 boxID, u32 value );
static void pp_update( DMP_MAINWORK* wk, u32 boxID, u32 value );
static void box_write_fixval( DMP_MAINWORK* wk, u32 boxID, u32 value );
static void  box_getstr( DMP_MAINWORK* wk, u32 boxID, STRBUF* buf );
static void box_relation( DMP_MAINWORK* wk, u32 updateBoxID );
static void NumInput_Setup( NUMINPUT_WORK* wk, STRBUF* buf, GFL_BMPWIN* win, GFL_FONT* font,
            PRINT_UTIL* util, PRINT_QUE* que, const INPUT_BOX_PARAM* boxParam, u32 value, const DMP_MAINWORK* mainWork );
static BOOL NumInput_Main( NUMINPUT_WORK* wk );
static u32 NumInput_GetNum( NUMINPUT_WORK* wk );
static void numinput_draw( NUMINPUT_WORK* wk );
static void numinput_sub_setary( NUMINPUT_WORK* wk, u32 value );
static u32 numinput_sub_calcnum( NUMINPUT_WORK* wk );
static u16 calc_keta( u32 value );
static void COMPSKB_Setup( COMP_SKB_WORK* wk, GFL_SKB* skb, STRBUF* buf, u32 msgArcID, u32 msgDataID, HEAPID heapID );
static void COMPSKB_Cleanup( COMP_SKB_WORK* wk );
static BOOL COMPSKB_Main( COMP_SKB_WORK* wk );
static int COMPSKB_GetWordIndex( const COMP_SKB_WORK* wk );
static BOOL compskb_strncmp( const STRBUF* str1, const STRBUF* str2, u32 len );
static u32 compskb_search( COMP_SKB_WORK* wk, const STRBUF* word, int org_idx, int* first_idx );
static BOOL comskb_is_match( COMP_SKB_WORK* wk, const STRBUF* word, int* match_idx );
static u32 personal_getparam( const POKEMON_PARAM* pp, PokePersonalParamID paramID );
static u8 personal_get_tokusei_kinds( const POKEMON_PARAM* pp );
static u16 personal_get_tokusei( const POKEMON_PARAM* pp, u8 idx );
static BOOL personal_is_sex_fixed( const POKEMON_PARAM* pp );
static u8 casetteVer_formal_to_local( u8 val );
static u8 casetteVer_local_to_formal( u8 val );
static u8 countryCode_formal_to_local( u8 val );
static u8 countryCode_local_to_formal( u8 value );
static u8 personalSpeabi_formal_to_local( const POKEMON_PARAM* pp, u8 val );
static u8 personalSpeabi_local_to_formal( const POKEMON_PARAM* pp, u8 value );
static void personalSpeabi_3rd_flag_set( POKEMON_PARAM* pp );

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

////////////////////////////////////////////////////////////////////////////////////
//==================================================================================
/*
 *  メインプロセス
 */
//==================================================================================
////////////////////////////////////////////////////////////////////////////////////

static GFL_PROC_RESULT PROC_MAKEPOKE_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  const PROCPARAM_DEBUG_MAKEPOKE* proc_param = (const PROCPARAM_DEBUG_MAKEPOKE*)pwk;

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_DEBUG_MAKEPOKE,   PROC_HEAP_SIZE );

  {
    DMP_MAINWORK* wk = GFL_PROC_AllocWork( proc, sizeof(DMP_MAINWORK), HEAPID_DEBUG_MAKEPOKE );
    if( wk == NULL ){
      return GFL_PROC_RES_FINISH;
    }
    wk->heapID = HEAPID_DEBUG_MAKEPOKE;
    wk->dst = proc_param->dst;
    wk->src = GFL_HEAP_AllocMemory( wk->heapID, POKETOOL_GetWorkSize() );
    wk->proc_param = (PROCPARAM_DEBUG_MAKEPOKE*)proc_param;
    GFL_STD_MemCopy( wk->dst, wk->src, POKETOOL_GetWorkSize() );

    if( proc_param->oyaStatus ){
      if( proc_param->mode != DMP_MODE_REWRITE ){
        wk->oyaID = MyStatus_GetID( proc_param->oyaStatus );
        wk->oyaSex = MyStatus_GetMySex( proc_param->oyaStatus );
      }else{
        wk->oyaID = PP_Get( wk->src, ID_PARA_id_no, NULL );
        wk->oyaSex = PP_Get( wk->src, ID_PARA_sex, NULL);
      }
      if( proc_param->mode != DMP_MODE_D_FIGHT ){
        PP_Get(wk->dst,ID_PARA_oyaname_raw, wk->oyaName);
      }else{
        STRTOOL_Copy( MyStatus_GetMyName(proc_param->oyaStatus), wk->oyaName, NELEMS(wk->oyaName) );
      }
    }else{
      wk->oyaID = 0;
      wk->oyaSex = 0;
      GFL_MSG_GetString( wk->msgData, DMPSTR_DMY_OYA_NAME, wk->tmpbuf );
      //STRTOOL_Copyにしないと、STRBUFのバッファサイズオーバーに引っかかる
      STRTOOL_Copy( GFL_STR_GetStringCodePointer( wk->tmpbuf), wk->oyaName, NELEMS(wk->oyaName) );
    }
    wk->oyaID_org = wk->oyaID;

    if( PP_Get(wk->dst, ID_PARA_monsno, NULL) == 0 )
    {
      u32 defaultMonsNo = proc_param->defaultMonsNo;
      if( (defaultMonsNo == 0)
      ||  (defaultMonsNo > MONSNO_END)
      ){
        defaultMonsNo = MONSNO_MIZYUMARU;
      }
      PP_Setup( wk->dst, defaultMonsNo, 5, wk->oyaID  );
    }

    UpdatePokeExpMinMax( wk, wk->dst );
    wk->seq = 0;
    wk->pageNo = PAGE_0;
  }
  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT PROC_MAKEPOKE_Quit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  DMP_MAINWORK* wk = mywk;

  GFL_HEAP_FreeMemory( wk->src );
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_DEBUG_MAKEPOKE );
  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT PROC_MAKEPOKE_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  DMP_MAINWORK* wk = mywk;

  switch( *seq ){
  case 0:
    main_setup_view( wk );
    (*seq)++;
    break;
  case 1:
    if( main_root_ctrl(wk) ){
      (*seq)++;
    }
    break;
  case 2:
    main_cleanup_view( wk );
    return GFL_PROC_RES_FINISH;
  }

  return GFL_PROC_RES_CONTINUE;
}


//----------------------------------------------------------------------------------
/**
 * ポケモン経験値の最小・最大値をワークに保持する
 *
 * @param   wk
 * @param   pp
 */
//----------------------------------------------------------------------------------
static void UpdatePokeExpMinMax( DMP_MAINWORK* wk, const POKEMON_PARAM* pp )
{
  u16 monsno, level;

  monsno = PP_Get( pp, ID_PARA_monsno, NULL );
  level = PP_Get( pp, ID_PARA_level, NULL );
  wk->pokeExpMin = POKETOOL_GetMinExp( monsno, PTL_FORM_NONE, level );
  if( level < 100 ){
    wk->pokeExpMax = POKETOOL_GetMinExp( monsno, PTL_FORM_NONE, level+1 ) - 1;
  }else{
    wk->pokeExpMax = wk->pokeExpMin;
  }
}

////////////////////////////////////////////////////////////////////////////////////
//==================================================================================
/*
 *  メインシーケンス
 */
//==================================================================================
////////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------------
/**
 * 画面構築
 *
 * @param   wk
 */
//----------------------------------------------------------------------------------
static void main_setup_view( DMP_MAINWORK* wk )
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
  wk->font = GFL_FONT_Create( ARCID_FONT, NARC_font_small_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );
  wk->msgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_DEBUG_MESSAGE, NARC_debug_message_debug_makepoke_dat, wk->heapID );
  wk->strbuf = GFL_STR_CreateBuffer( STRBUF_LEN, wk->heapID );
  wk->tmpbuf = GFL_STR_CreateBuffer( STRBUF_LEN, wk->heapID );
  wk->printQue = PRINTSYS_QUE_CreateEx( 4096, wk->heapID );
  PRINT_UTIL_Setup( &wk->printUtil, wk->win );


  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 0, 0, wk->heapID );
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
static void main_cleanup_view( DMP_MAINWORK* wk )
{
  PRINTSYS_QUE_Delete( wk->printQue );
  GFL_STR_DeleteBuffer( wk->tmpbuf );
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
static BOOL main_root_ctrl( DMP_MAINWORK* wk )
{
  PRINTSYS_QUE_Main( wk->printQue );

  if( PRINT_UTIL_Trans(&wk->printUtil, wk->printQue) == FALSE ){
    return FALSE;
  }

  switch( wk->seq ){
  case SEQ_DRAW_CAPTION:
    {
      int i;
      for(i=0; i<NELEMS(InputBoxParams); ++i){
        print_caption( wk, &InputBoxParams[i] );
      }
    }
    wk->seq = SEQ_DRAW_BOX;
    break;
  case SEQ_DRAW_BOX:
    {
      int i;

      PP_Renew( wk->dst );
      for(i=0; i<NELEMS(InputBoxParams); ++i){
        box_setup( wk, i, wk->dst );
      }
      wk->seq = SEQ_WAIT_CTRL;
    }
    break;
  case SEQ_WAIT_CTRL:
    //入力取得
    wk->seq = root_wait_ctrl( wk );
    break;
  case SEQ_INPUT_STR:
    if( COMPSKB_Main(&wk->comp) )
    {
      int idx = COMPSKB_GetWordIndex( &wk->comp );
      if( idx < 0 ){
        if( GFL_STR_GetBufferLength(wk->strbuf) == 0 ){
          idx = 0;
        }
      }
      if( idx >= 0 ){
        box_update( wk, wk->boxIdx, idx );
        box_relation( wk, wk->boxIdx );
      }
      COMPSKB_Cleanup( &wk->comp );
      GFL_SKB_Delete( wk->skb );
      wk->seq = SEQ_WAIT_CTRL;
    }
    break;
  case SEQ_INPUT_NICKNAME:
    if( COMPSKB_Main(&wk->comp) )
    {
      const INPUT_BOX_PARAM* p = &InputBoxParams[ wk->boxIdx ];
      u32 len = GFL_STR_GetBufferLength(wk->strbuf);
      if( len == 0 || len > 5 )
      {
        u16  mons_no = box_getvalue( wk, INPUTBOX_ID_POKETYPE );
        GFL_MSGDATA* msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_monsname_dat, GFL_HEAP_LOWID(wk->heapID) );
        GFL_MSG_GetString( msg, mons_no, wk->strbuf );
        GFL_MSG_Delete( msg );
      }

      PP_Put( wk->dst, p->paraID, (u32)(wk->strbuf) );
      box_update( wk, wk->boxIdx, 0 );
      box_relation( wk, wk->boxIdx );

      COMPSKB_Cleanup( &wk->comp );
      GFL_SKB_Delete( wk->skb );
      wk->seq = SEQ_WAIT_CTRL;
    }
    break;
  case SEQ_INPUT_NUM:
    if( NumInput_Main(&wk->numInput) )
    {
      u32 num = NumInput_GetNum( &wk->numInput );
      box_update( wk, wk->boxIdx, num );
      box_relation( wk, wk->boxIdx );
      wk->seq = SEQ_WAIT_CTRL;
    }
    break;
  case SEQ_PAGE_CHANGE:
    GFL_BMP_Clear( wk->bmp, COLIDX_WHITE );
    GFL_BMPWIN_TransVramCharacter( wk->win );
    wk->seq = SEQ_DRAW_CAPTION;
    break;
  case SEQ_EXIT:
    return TRUE;
  }
  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////
//==================================================================================
/*
 *  メインコントロール
 */
//==================================================================================
////////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------
/*
 *
 */
//----------------------------------------------------
static int root_wait_ctrl( DMP_MAINWORK* wk )
{
  u16 key = GFL_UI_KEY_GetTrg();

  // L,R によるページ切り替え
  if( key & PAD_BUTTON_L )
  {
    if( --(wk->pageNo) < 0 ){
      wk->pageNo = PAGE_MAX - 1;
    }
    return SEQ_PAGE_CHANGE;
  }
  else if( key & PAD_BUTTON_R )
  {
    if( ++(wk->pageNo) >= PAGE_MAX ){
      wk->pageNo = PAGE_0;
    }
    return SEQ_PAGE_CHANGE;
  }

  //決定/キャンセル
  if( key & PAD_BUTTON_START ){
    update_dst( wk );
    wk->proc_param->ret_code = DMP_RET_REWRITE;
    return SEQ_EXIT;
  }
  if( key & PAD_BUTTON_SELECT ){
    update_dst( wk );
    wk->proc_param->ret_code = DMP_RET_COPY;
    return SEQ_EXIT;
  }
  if( key & PAD_BUTTON_B ){
    GFL_STD_MemCopy( wk->src, wk->dst, POKETOOL_GetWorkSize() );
    wk->proc_param->ret_code = DMP_RET_CANCEL;
    return SEQ_EXIT;
  }
  wk->boxIdx = check_box_touch( wk );

  if( wk->boxIdx >= 0 )
  {
    const INPUT_BOX_PARAM* p = &InputBoxParams[ wk->boxIdx ];
    switch( p->type ){
    case INPUTBOX_TYPE_STR:
    case INPUTBOX_TYPE_NICKNAME:
      return inputbox_type_str( wk, p );

    case INPUTBOX_TYPE_NUM:
      return inputbox_type_num( wk, p );

    case INPUTBOX_TYPE_SWITCH:
      return inputbox_type_switch( wk, p );

    case INPUTBOX_TYPE_BTN:
      return inputbox_type_btn( wk, p );

    }
  }
  return SEQ_WAIT_CTRL;
}

/*
 *
 */
static int inputbox_type_str( DMP_MAINWORK* wk, const INPUT_BOX_PARAM* p )
{
  static const GFL_SKB_SETUP setup = {
    STRBUF_LEN, GFL_SKB_STRTYPE_STRBUF,
    GFL_SKB_MODE_KATAKANA, TRUE, 0, PAD_BUTTON_START,
    PRINT_FRAME, SKB_PALIDX1, SKB_PALIDX2,
  };

  wk->skbSetup = setup;
  box_getstr( wk, wk->boxIdx, wk->strbuf );

  if( is_hiragana(wk->strbuf) ){
    wk->skbSetup.mode = GFL_SKB_MODE_HIRAGANA;
  }
  wk->skb = GFL_SKB_Create( (void*)(wk->strbuf), &wk->skbSetup, wk->heapID );
  COMPSKB_Setup( &wk->comp, wk->skb, wk->strbuf, p->arg3, p->arg, wk->heapID );

  return ((p->type == INPUTBOX_TYPE_STR)? SEQ_INPUT_STR : SEQ_INPUT_NICKNAME);
}

/*
 *
 */
static int inputbox_type_num( DMP_MAINWORK* wk, const INPUT_BOX_PARAM* p )
{
  NumInput_Setup( &wk->numInput, wk->strbuf, wk->win, wk->font,
    &wk->printUtil, wk->printQue, p, box_getvalue(wk, wk->boxIdx), wk );

  return SEQ_INPUT_NUM;
}

/*
 *
 */
static int inputbox_type_switch( DMP_MAINWORK* wk, const INPUT_BOX_PARAM* p )
{
  int val = box_getvalue( wk, wk->boxIdx );

  if( wk->touch_prev_flag ){
    if( --val < 0 ){
      val = p->arg2-1;
    }
  }else{
    if( ++val >= p->arg2 ){
      val = 0;
    }
  }
  pp_update( wk, wk->boxIdx, val );
  box_update( wk, wk->boxIdx, val );
  box_relation( wk, wk->boxIdx );

  return SEQ_WAIT_CTRL;
}

/*
 *
 */
static int inputbox_type_btn( DMP_MAINWORK* wk, const INPUT_BOX_PARAM* p )
{
  u32 i;

  update_dst( wk );

  switch( wk->boxIdx ){
  case INPUTBOX_ID_DEF_BUTTON:   // デフォルトワザセットボタン
    default_waza_set( wk, WAZANO_NULL );
    break;
  case INPUTBOX_ID_HANERU_BUTTON:   // はねるだけボタン
    default_waza_set( wk, WAZANO_HANERU );
    break;
  case INPUTBOX_ID_HATAKU_BUTTON:   // はたくだけボタン
    default_waza_set( wk, WAZANO_HATAKU );
    break;
  case INPUTBOX_ID_RARE_SET_BUTTON: // レアセットボタン
    rare_param_set( wk );
    break;
  }
  return SEQ_WAIT_CTRL;
}

/*
 *
 */
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

        if( p->pageNo != wk->pageNo ){ continue; }

        if( (p->type == INPUTBOX_TYPE_FIXVAL) ||  (p->type == INPUTBOX_TYPE_FIXSTR)){
          continue;
        }
        if( (x >= p->xpos) && (x <= (p->xpos + p->width))
        &&  (y >= p->ypos) && (y <= (p->ypos + p->height))
        ){
          u32 x_center = p->xpos + (p->width / 2);
          wk->touch_prev_flag = ( x < x_center );
          return i;
        }
      }
    }
  }
  return -1;
}

/*
 *
 */
static BOOL is_hiragana( const STRBUF* buf )
{
  const STRCODE* ptr;
  ptr = GFL_STR_GetStringCodePointer( buf );
  if( (*ptr >= 0x3041) && (*ptr <= 0x3093) ){
    return TRUE;
  }
  return FALSE;
}

/*
 *
 */
static void default_waza_set( DMP_MAINWORK* wk, u16 waza )
{
  int i;

  if( waza != WAZANO_NULL ){
    PP_SetWazaPos( wk->dst, waza, 0 );
    for(i=1; i<PTL_WAZA_MAX; ++i){
      PP_SetWazaPos( wk->dst, WAZANO_NULL, i );
    }
  }else{
    PP_SetWazaDefault( wk->dst );
  }
  PP_RecoverWazaPPAll( wk->dst );

  // わざパラメータを反映
  for(i=0; i<PTL_WAZA_MAX; ++i){
    box_setup( wk, INPUTBOX_ID_WAZA1+i,   wk->dst );
    box_setup( wk, INPUTBOX_ID_PPMAX1+i,  wk->dst );
    box_setup( wk, INPUTBOX_ID_PPCNT1+i,  wk->dst );
    box_setup( wk, INPUTBOX_ID_PPEDIT1+i, wk->dst );
  }
}

/*
 *
 */
static void rare_param_set( DMP_MAINWORK* wk )
{
  int i;

  wk->boxValue[ INPUTBOX_ID_RARE ] = TRUE;
  {
    u8 form_no = box_getvalue( wk, INPUTBOX_ID_FORM );
    u16 mons_no = box_getvalue( wk, INPUTBOX_ID_POKETYPE );
    u8 tokusei = box_getvalue( wk, INPUTBOX_ID_TOKUSEI );
    u8 sex = box_getvalue( wk, INPUTBOX_ID_SEX );
    u32 personal_rnd = POKETOOL_CalcPersonalRandEx( wk->oyaID, mons_no, form_no, sex, 0, TRUE );

    wk->boxValue[ INPUTBOX_ID_PER_RND_H ] = personal_rnd>>16;
    wk->boxValue[ INPUTBOX_ID_PER_RND_L ] = personal_rnd&0xFFFF;
  }
  update_dst( wk );

  box_setup( wk, INPUTBOX_ID_PER_RND_H, wk->dst );
  box_setup( wk, INPUTBOX_ID_PER_RND_L, wk->dst );
  box_setup( wk, INPUTBOX_ID_RARE, wk->dst );
}

//----------------------------------------------------------------------------------
/**
 * 出力ポケモンデータ更新
 *
 * @param   wk
 */
//----------------------------------------------------------------------------------
static void update_dst( DMP_MAINWORK* wk )
{
  u32 personal_rnd, mons_no, tokusei, sex, level;
  u8 rare_flag = box_getvalue( wk, INPUTBOX_ID_RARE );
  u8 form_no = box_getvalue( wk, INPUTBOX_ID_FORM );

  mons_no = box_getvalue( wk, INPUTBOX_ID_POKETYPE );
  tokusei = box_getvalue( wk, INPUTBOX_ID_TOKUSEI );
  level = box_getvalue( wk, INPUTBOX_ID_LEVEL );
#if 0
  sex = box_getvalue( wk, INPUTBOX_ID_SEX );
  personal_rnd = POKETOOL_CalcPersonalRandEx( wk->oyaID, mons_no, form_no, sex, 0, rare_flag );
#else
  personal_rnd =  box_getvalue( wk, INPUTBOX_ID_PER_RND_H) << 16;
  personal_rnd |= box_getvalue( wk, INPUTBOX_ID_PER_RND_L);
  sex = POKETOOL_GetSex( mons_no, form_no, personal_rnd );
#endif
  PP_Get( wk->dst, ID_PARA_nickname, (void*)(wk->strbuf) );

  PP_Clear( wk->dst );

  {
    u32 pow_val, exp;
    u8 hp, pow, def, agi, spw, sdf;
    hp  = box_getvalue( wk, INPUTBOX_ID_HPRND );
    pow = box_getvalue( wk, INPUTBOX_ID_POWRND );
    def = box_getvalue( wk, INPUTBOX_ID_DEFRND );
    agi = box_getvalue( wk, INPUTBOX_ID_AGIRND );
    spw = box_getvalue( wk, INPUTBOX_ID_SPWRND );
    sdf = box_getvalue( wk, INPUTBOX_ID_SDFRND );
    pow_val = PTL_SETUP_POW_PACK( hp, pow, def, spw, sdf, agi );
    PP_SetupEx( wk->dst, mons_no, level, wk->oyaID, pow_val, personal_rnd );
    PP_Put( wk->dst, ID_PARA_oyasex, wk->oyaSex );
    PP_Put( wk->dst, ID_PARA_oyaname_raw, (u32)(wk->oyaName) );

#ifdef DEBUG_ONLYFOR_iwasawa
    if(mons_no == MONSNO_HINBASU){
      PP_Put( wk->dst, ID_PARA_beautiful, 255 );
    }
#endif
    TAYA_Printf("[[書き込み]] 性別=%d, 個性乱数=%08x\n", sex, personal_rnd);
    TAYA_Printf("             HP:%2d  ATK:%2d  DEF:%2d  SAT:%2d  SDF:%2d  AGI:%2d\n",
                hp, pow, def, spw, sdf, agi );

    {
      u32 rndResult = PP_Get( wk->dst, ID_PARA_personal_rnd, NULL );
      u8 sexResult = PP_Get( wk->dst, ID_PARA_sex, NULL );
      u8 rHP, rPow, rDef, rSat, rSde, rAgi;
      rHP  = PP_Get( wk->dst, ID_PARA_hp_rnd, NULL );
      rPow = PP_Get( wk->dst, ID_PARA_pow_rnd, NULL );
      rDef = PP_Get( wk->dst, ID_PARA_def_rnd, NULL );
      rSat = PP_Get( wk->dst, ID_PARA_spepow_rnd, NULL );
      rSde = PP_Get( wk->dst, ID_PARA_spedef_rnd, NULL );
      rAgi = PP_Get( wk->dst, ID_PARA_agi_rnd, NULL );
      TAYA_Printf("[[読み込み]] 性別=%d, 個性乱数=%08x\n", sexResult, rndResult);
      TAYA_Printf("             HP:%2d  ATK:%2d  DEF:%2d  SAT:%2d  SDF:%2d  AGI:%2d\n",
                   rHP, rPow, rDef, rSat, rSde, rAgi );
      IWASAWA_Printf("[[読み込み]] 性別=%d, 個性乱数=%08x\n", sexResult, rndResult);
      IWASAWA_Printf("             HP:%2d  ATK:%2d  DEF:%2d  SAT:%2d  SDF:%2d  AGI:%2d\n",
                   rHP, rPow, rDef, rSat, rSde, rAgi );
    }

    exp = box_getvalue( wk, INPUTBOX_ID_EXP );
    PP_Put( wk->dst, ID_PARA_exp, exp );
  }

  // 特性
  PP_Put( wk->dst, ID_PARA_speabino, tokusei );
  personalSpeabi_3rd_flag_set( wk->dst );

  {
    u32 i;
    for(i=0; i<PTL_WAZA_MAX; ++i)
    {
      {
        u16 waza = box_getvalue(wk, INPUTBOX_ID_WAZA1+i);
        PP_SetWazaPos( wk->dst, box_getvalue(wk, INPUTBOX_ID_WAZA1+i), i );
      }
      PP_Put( wk->dst, ID_PARA_pp_count1+i, box_getvalue(wk, INPUTBOX_ID_PPCNT1+i) );
      PP_Put( wk->dst, ID_PARA_pp1+i, box_getvalue(wk, INPUTBOX_ID_PPEDIT1+i) );
    }

    PP_Put( wk->dst, ID_PARA_hp_exp,  box_getvalue(wk, INPUTBOX_ID_HPEXP) );
    PP_Put( wk->dst, ID_PARA_pow_exp, box_getvalue(wk, INPUTBOX_ID_POWEXP) );
    PP_Put( wk->dst, ID_PARA_def_exp, box_getvalue(wk, INPUTBOX_ID_DEFEXP) );
    PP_Put( wk->dst, ID_PARA_agi_exp, box_getvalue(wk, INPUTBOX_ID_AGIEXP) );
    PP_Put( wk->dst, ID_PARA_spepow_exp, box_getvalue(wk, INPUTBOX_ID_SPWEXP) );
    PP_Put( wk->dst, ID_PARA_spedef_exp, box_getvalue(wk, INPUTBOX_ID_SDFEXP) );
  }

  {
    u8  seikaku = box_getvalue( wk, INPUTBOX_ID_SEIKAKU );
    PP_Put( wk->dst, ID_PARA_seikaku, seikaku );
  }

  PP_Renew( wk->dst );
  PP_Put( wk->dst, ID_PARA_hp, box_getvalue(wk, INPUTBOX_ID_HPEDIT) );
  {
    u8 rHP, rPow, rDef, rSat, rSde, rAgi;
    rHP  = PP_Get( wk->dst, ID_PARA_hpmax, NULL );
    rPow = PP_Get( wk->dst, ID_PARA_pow, NULL );
    rDef = PP_Get( wk->dst, ID_PARA_def, NULL );
    rSat = PP_Get( wk->dst, ID_PARA_spepow, NULL );
    rSde = PP_Get( wk->dst, ID_PARA_spedef, NULL );
    rAgi = PP_Get( wk->dst, ID_PARA_agi, NULL );
    TAYA_Printf("[[最終パラ]] HP:%3d  ATK:%3d  DEF:%3d  SAT:%3d  SDF:%3d  AGI:%3d\n",
                 rHP, rPow, rDef, rSat, rSde, rAgi );
  }

  // アイテム
  {
    u32 item = box_getvalue( wk, INPUTBOX_ID_ITEM );
    PP_Put( wk->dst, ID_PARA_item, item );
  }


  // たまごフラグ
  {
    u8 tamago_flg = box_getvalue( wk, INPUTBOX_ID_TAMAGO );
    PP_Put( wk->dst, ID_PARA_tamago_flag, tamago_flg );

    //ニックネーム
    if(tamago_flg){
      GFL_MSG_GetString( wk->msgData, DMPSTR_EGG_NAME, wk->tmpbuf );
      PP_Put( wk->dst, ID_PARA_nickname, (u32)(wk->tmpbuf) );
    }else{
      PP_Put( wk->dst, ID_PARA_nickname, (u32)(wk->strbuf) );
    }
  }

  PP_Put( wk->dst, ID_PARA_condition, box_getvalue(wk, INPUTBOX_ID_SICK) );
  PP_Put( wk->dst, ID_PARA_friend,    box_getvalue(wk, INPUTBOX_ID_NATSUKI) );
  PP_Put( wk->dst, ID_PARA_form_no,   box_getvalue(wk, INPUTBOX_ID_FORM) );
  PP_Put( wk->dst, ID_PARA_pokerus,   box_getvalue(wk, INPUTBOX_ID_POKERUS) );
  {
    u32 oya_id = box_getvalue(wk, INPUTBOX_ID_OYAID_H)<<16;
    oya_id |= box_getvalue(wk,INPUTBOX_ID_OYAID_L);

    PP_Put( wk->dst, ID_PARA_id_no, oya_id );
  }

  //
  //捕まえた場所/生まれた場所
  PP_Put( wk->dst, ID_PARA_get_level, box_getvalue(wk, INPUTBOX_ID_GET_LEVEL) );

  PP_Put( wk->dst, ID_PARA_get_place, box_getvalue(wk, INPUTBOX_ID_GET_PLACE) );
  PP_Put( wk->dst, ID_PARA_get_year, box_getvalue(wk, INPUTBOX_ID_GET_YEAR) );
  PP_Put( wk->dst, ID_PARA_get_month, box_getvalue(wk, INPUTBOX_ID_GET_MONTH) );
  PP_Put( wk->dst, ID_PARA_get_day, box_getvalue(wk, INPUTBOX_ID_GET_DAY) );
  PP_Put( wk->dst, ID_PARA_birth_place, box_getvalue(wk, INPUTBOX_ID_BIRTH_PLACE) );
  PP_Put( wk->dst, ID_PARA_birth_year, box_getvalue(wk, INPUTBOX_ID_BIRTH_YEAR) );
  PP_Put( wk->dst, ID_PARA_birth_month, box_getvalue(wk, INPUTBOX_ID_BIRTH_MONTH) );
  PP_Put( wk->dst, ID_PARA_birth_day, box_getvalue(wk, INPUTBOX_ID_BIRTH_DAY) );

  // 捕獲ボール
  {
    u8 getBallID = ITEM_GetBallID( box_getvalue(wk, INPUTBOX_ID_GET_BALL) );
    if( getBallID == BALLID_NULL ){
      getBallID = BALLID_MONSUTAABOORU;
    }
    PP_Put( wk->dst, ID_PARA_get_ball,  getBallID );
  }
  //親の性別
  PP_Put( wk->dst, ID_PARA_oyasex, box_getvalue(wk, INPUTBOX_ID_OYA_SEX) );

  // 捕獲カセット
  PP_Put( wk->dst, ID_PARA_get_cassette, casetteVer_local_to_formal(box_getvalue(wk, INPUTBOX_ID_GET_CASETTE)) );
  // 国コード
  PP_Put( wk->dst, ID_PARA_country_code, countryCode_local_to_formal(box_getvalue(wk, INPUTBOX_ID_COUNTRY)) );
  // イベント配布フラグ
  PP_Put( wk->dst, ID_PARA_event_get_flag, box_getvalue(wk, INPUTBOX_ID_EVENTGET_FLAG) );

  PP_Renew( wk->dst );
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
  if( p->pageNo == wk->pageNo )
  {
    if( p->cap_strID != DMPSTR_NULL ){
      u32 ypos = p->cap_ypos + ((LINE_HEIGHT - GFL_FONT_GetLineHeight(wk->font)) / 2);
      GFL_MSG_GetString( wk->msgData, p->cap_strID, wk->strbuf );
      PRINT_UTIL_Print( &wk->printUtil, wk->printQue, p->cap_xpos, ypos, wk->strbuf, wk->font );
    }
  }
}

static PRINTSYS_LSB box_sub_get_bgcol( u8 input_type )
{
  switch( input_type ){
  case INPUTBOX_TYPE_STR:      return PRINTSYS_LSB_Make( COLIDX_BLACK, 0, COLIDX_ORANGE_D );
  case INPUTBOX_TYPE_NICKNAME: return PRINTSYS_LSB_Make( COLIDX_BLACK, 0, COLIDX_ORANGE_D );
  case INPUTBOX_TYPE_NUM:      return PRINTSYS_LSB_Make( COLIDX_BLACK, 0, COLIDX_BLUE_L );
  case INPUTBOX_TYPE_SWITCH:   return PRINTSYS_LSB_Make( COLIDX_BLACK, 0, COLIDX_GREEN_L );
  case INPUTBOX_TYPE_FIXVAL:   return PRINTSYS_LSB_Make( COLIDX_WHITE, 0, COLIDX_GRAY );
  case INPUTBOX_TYPE_FIXSTR:   return PRINTSYS_LSB_Make( COLIDX_WHITE, 0, COLIDX_GRAY );
  case INPUTBOX_TYPE_BTN:      return PRINTSYS_LSB_Make( COLIDX_BLACK, 0, COLIDX_RED_L );
  }
  return PRINTSYS_LSB_Make( COLIDX_BLACK, 0, COLIDX_WHITE );
}


static void box_initwork( DMP_MAINWORK* wk )
{
  int i;
  for(i=0; i<INPUTBOX_ID_MAX; ++i){
    wk->boxEnable[i] = FALSE;
  }
}

static int box_getvalue( const DMP_MAINWORK* wk, u32 boxIdx )
{
  return wk->boxValue[ boxIdx ];
}

/**
 *  PPを元にボックスに値のセット
 */
static void box_setup( DMP_MAINWORK* wk, u32 boxID, const POKEMON_PARAM* pp )
{
  const INPUT_BOX_PARAM* p = &InputBoxParams[ boxID ];
  u32 value;

#if 0
  if(p->pageNo != wk->pageNo ){
    return;
  }
#endif
  switch( p->paraID ){
  case ID_PARA_seikaku:
    value = PP_GetSeikaku( pp );
    break;
  case ID_PARA_pp_count1:
  case ID_PARA_pp_count2:
  case ID_PARA_pp_count3:
  case ID_PARA_pp_count4:
    {
      u16 idx = p->paraID - ID_PARA_pp_count1;
      u16 waza = PP_Get( pp, ID_PARA_waza1 + idx, NULL );
      if( waza != WAZANO_NULL ){
        value = PP_Get( pp, p->paraID, NULL );
      }else{
        value = 0;
      }
    }
    break;
  case ID_PARA_pp_max1:
  case ID_PARA_pp_max2:
  case ID_PARA_pp_max3:
  case ID_PARA_pp_max4:
    {
      u16 idx = p->paraID - ID_PARA_pp_max1;
      u16 waza = PP_Get( pp, ID_PARA_waza1 + idx, NULL );
      if( waza != WAZANO_NULL ){
        value = PP_Get( pp, p->paraID, NULL );
      }else{
        value = 0;
      }
    }
    break;
  case ID_PARA_pp1:
  case ID_PARA_pp2:
  case ID_PARA_pp3:
  case ID_PARA_pp4:
    {
      u16 idx = p->paraID - ID_PARA_pp1;
      u16 waza = PP_Get( pp, ID_PARA_waza1 + idx, NULL );
      if( waza != WAZANO_NULL ){
        value = PP_Get( pp, p->paraID, NULL );
      }else{
        value = 0;
      }
    }
    break;
  case ID_PARA_nickname:
//    PP_Get( pp, p->paraID, NULL );
    value = 0;
    break;

  case ID_PARA_get_cassette:
    value = casetteVer_formal_to_local( PP_Get(pp, p->paraID, NULL) );
    break;

  case ID_PARA_id_no:
    {
      value = PP_Get( pp, p->paraID, NULL );
      if( boxID == INPUTBOX_ID_OYAID_L ){
        value &= 0xffff;
      }else{
        value >>= 16;
        value &= 0xffff;
      }
    }
    break;

  case ID_PARA_personal_rnd:
    {
      value = PP_Get( pp, p->paraID, NULL );
      if( boxID == INPUTBOX_ID_PER_RND_L ){
        value &= 0xffff;
      }else{
        value >>= 16;
        value &= 0xffff;
      }
    }
    break;

  case ID_PARA_country_code:
    value = countryCode_formal_to_local( PP_Get(pp, p->paraID, NULL) );
    break;

  case ID_PARA_speabino:
    if( boxID == INPUTBOX_ID_TOKUSEI_SWITCH ){
      value = personalSpeabi_formal_to_local( pp, PP_Get(pp, p->paraID, NULL) );
    }else{
      value = PP_Get( pp, p->paraID, NULL );
    }
    break;

  case ID_PARA_get_ball:
    value = ITEM_BallID2ItemID( PP_Get(pp, p->paraID, NULL));
    break;

  default:
    if( boxID == INPUTBOX_ID_RARE ){
      value = PP_CheckRare( pp );
    }else{
      value = PP_Get( pp, p->paraID, NULL );
    }
    break;
  }

  wk->boxEnable[boxID] = FALSE;

  switch( boxID ){
  case INPUTBOX_ID_SEX_SET:
    if( (p->pageNo == wk->pageNo) ){
      if( personal_is_sex_fixed( pp ) ){
        box_write_fixval( wk, boxID, value );
        return;
      }
    }
    break;
#if 0
  case INPUTBOX_ID_SEX:
  case INPUTBOX_ID_SEX_FIX:
    if( personal_is_sex_fixed( pp ) ){
      box_write_fixval( wk, boxID, value );
      return;
    }
//#else
    if( (p->pageNo == wk->pageNo) ){
      box_write_fixval( wk, boxID, value );
    }else{
      wk->boxValue[ boxID ] = value;
    }
    return;
#endif
  default:
    break;
  }
  if( (p->pageNo == wk->pageNo) ){
    wk->boxEnable[boxID] = TRUE;
    box_update( wk, boxID, value );
  }else{
    wk->boxValue[ boxID ] = value;
  }
}

static void box_update( DMP_MAINWORK* wk, u32 boxID, u32 value )
{
  const INPUT_BOX_PARAM* p = &InputBoxParams[ boxID ];
  u32 str_width, str_height, xpos, ypos;

  PRINTSYS_LSB color = box_sub_get_bgcol( p->type );
  u8 color_bg = PRINTSYS_LSB_GetB( color );

  GFL_BMP_Fill( wk->bmp, p->xpos, p->ypos, p->width, p->height, color_bg );

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

  PRINT_UTIL_PrintColor( &wk->printUtil, wk->printQue, xpos, ypos, wk->strbuf, wk->font, color );
}
static void pp_update( DMP_MAINWORK* wk, u32 boxID, u32 value )
{
  const INPUT_BOX_PARAM* p = &InputBoxParams[ boxID ];

  switch( boxID ){
  case INPUTBOX_ID_SEX_SET:
    wk->boxValue[ boxID ] = value;
    {
      u8 form_no = box_getvalue( wk, INPUTBOX_ID_FORM );
      u16 mons_no = box_getvalue( wk, INPUTBOX_ID_POKETYPE );
      u8 tokusei = box_getvalue( wk, INPUTBOX_ID_TOKUSEI );
      u8 sex = box_getvalue( wk, INPUTBOX_ID_SEX );
      u32 personal_rnd = POKETOOL_CalcPersonalRandEx( wk->oyaID, mons_no, form_no, sex, 0, value );
    }
    update_dst( wk );
    break;
#if 0
  case INPUTBOX_ID_RARE:
    wk->boxValue[ boxID ] = value;
    {
      u8 form_no = box_getvalue( wk, INPUTBOX_ID_FORM );
      u16 mons_no = box_getvalue( wk, INPUTBOX_ID_POKETYPE );
      u8 tokusei = box_getvalue( wk, INPUTBOX_ID_TOKUSEI );
      u8 sex = box_getvalue( wk, INPUTBOX_ID_SEX );
      u32 personal_rnd = POKETOOL_CalcPersonalRandEx( wk->oyaID, mons_no, form_no, sex, 0, value );
    }
    update_dst( wk );
    break;
  case INPUTBOX_ID_SEX:
  case INPUTBOX_ID_SEX_FIX:
    wk->boxValue[ boxID ] = value;
    update_dst( wk );
    break;
#endif
  case INPUTBOX_ID_GET_CASETTE:
    value = casetteVer_local_to_formal( value );
    PP_Put( wk->dst, p->paraID, value );
    break;

  case INPUTBOX_ID_COUNTRY:
    value = countryCode_local_to_formal( value );
    PP_Put( wk->dst, p->paraID, value );
    break;

  case INPUTBOX_ID_TOKUSEI_SWITCH:
    value = personalSpeabi_local_to_formal( wk->dst, value );
    PP_Put( wk->dst, p->paraID, value );
    personalSpeabi_3rd_flag_set( wk->dst );
    break;

  case INPUTBOX_ID_TOKUSEI:
    PP_Put( wk->dst, p->paraID, value );
    personalSpeabi_3rd_flag_set( wk->dst );
    break;

  default:
    PP_Put( wk->dst, p->paraID, value );
  }

}

static void box_write_fixval( DMP_MAINWORK* wk, u32 boxID, u32 value )
{
  const INPUT_BOX_PARAM* p = &InputBoxParams[ boxID ];
  u32 str_width, str_height, xpos, ypos;
  PRINTSYS_LSB color;

  GFL_BMP_Fill( wk->bmp, p->xpos, p->ypos, p->width, p->height, COLIDX_GRAY );

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

  color = PRINTSYS_LSB_Make( COLIDX_WHITE, 0, COLIDX_GRAY );
  PRINT_UTIL_PrintColor( &wk->printUtil, wk->printQue, xpos, ypos, wk->strbuf, wk->font, color );

}

static void  box_getstr( DMP_MAINWORK* wk, u32 boxID, STRBUF* buf )
{
  const INPUT_BOX_PARAM* p = &InputBoxParams[ boxID ];
  u32 value = wk->boxValue[ boxID ];

  switch( p->type ){
  case INPUTBOX_TYPE_STR:
    {
      GFL_MSGDATA* msgdat = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, p->arg3, p->arg, GFL_HEAP_LOWID(wk->heapID) );
      GFL_MSG_GetString( msgdat, value+p->arg2, wk->strbuf );
      GFL_MSG_Delete( msgdat );
    }
    break;
  case INPUTBOX_TYPE_FIXSTR:
    {
      GFL_MSGDATA* msgdat;
      switch( p->arg ){
      case FIXSTR_CODE_PLACE:
        {
          msgdat = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
              ARCID_MESSAGE, POKE_PLACE_GetMessageDatId( value ), GFL_HEAP_LOWID(wk->heapID) );
          value = POKE_PLACE_GetMessageId( value );
        }
        break;
      default:
        msgdat = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, p->arg3, p->arg, GFL_HEAP_LOWID(wk->heapID) );
        break;
      }
      GFL_MSG_GetString( msgdat, value+p->arg2, wk->strbuf );
      GFL_MSG_Delete( msgdat );
    }
    break;

  case INPUTBOX_TYPE_NICKNAME:
    PP_Get( wk->dst, ID_PARA_nickname, (void*)buf );
    break;

  case INPUTBOX_TYPE_BTN:
    {
      GFL_MSGDATA* msgdat = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, p->arg3, p->arg, GFL_HEAP_LOWID(wk->heapID) );
      GFL_MSG_GetString( msgdat, p->arg2, wk->strbuf );
      GFL_MSG_Delete( msgdat );
    }
    break;

  case INPUTBOX_TYPE_NUM:
    {
      u8 keta = 1;
      switch( p->arg ){
      default:
        keta = calc_keta(p->arg);
        break;
      case INPUTBOX_NUM_ARG_EXP:
        keta = 8;
        break;
      case INPUTBOX_NUM_ARG_DEPEND:
        keta = calc_keta( box_getvalue(wk, p->arg2) );
        break;
      case INPUTBOX_NUM_ARG_FORM:
        keta = 2;
        break;
      }
      STRTOOL_SetNumber( wk->strbuf, value, keta, STR_NUM_DISP_SPACE, STR_NUM_CODE_ZENKAKU );
    }
    break;

  case INPUTBOX_TYPE_FIXVAL:
    STRTOOL_SetNumber( wk->strbuf, value, calc_keta(p->arg), STR_NUM_DISP_SPACE, STR_NUM_CODE_ZENKAKU );
    break;
  case INPUTBOX_TYPE_SWITCH:
    GFL_MSG_GetString( wk->msgData, p->arg+value, wk->strbuf );
    break;
  }
}

static void box_setup_renew( DMP_MAINWORK* wk )
{
  box_setup( wk, INPUTBOX_ID_TYPE1, wk->dst );
  box_setup( wk, INPUTBOX_ID_TYPE2, wk->dst );
  box_setup( wk, INPUTBOX_ID_TOKUSEI, wk->dst );
  box_setup( wk, INPUTBOX_ID_TOKUSEI_SWITCH, wk->dst );
  box_setup( wk, INPUTBOX_ID_TOKUSEI_3RD, wk->dst );
  box_setup( wk, INPUTBOX_ID_SEX, wk->dst );
  box_setup( wk, INPUTBOX_ID_SEX_FIX, wk->dst );
  box_setup( wk, INPUTBOX_ID_SEX_SET, wk->dst );
  box_setup( wk, INPUTBOX_ID_FORM, wk->dst );
  box_setup( wk, INPUTBOX_ID_HPVAL, wk->dst );
  box_setup( wk, INPUTBOX_ID_HPEDIT, wk->dst );
  box_setup( wk, INPUTBOX_ID_POWVAL, wk->dst );
  box_setup( wk, INPUTBOX_ID_DEFVAL, wk->dst );
  box_setup( wk, INPUTBOX_ID_AGIVAL, wk->dst );
  box_setup( wk, INPUTBOX_ID_SPWVAL, wk->dst );
  box_setup( wk, INPUTBOX_ID_SDFVAL, wk->dst );
  box_setup( wk, INPUTBOX_ID_EXP, wk->dst );
}

static void box_setup_form_change( DMP_MAINWORK* wk )
{
  u16 item = box_getvalue( wk, INPUTBOX_ID_ITEM );
  u16 mons = box_getvalue( wk, INPUTBOX_ID_POKETYPE );
  u16 form = box_getvalue( wk, INPUTBOX_ID_FORM );
  u8  now_form = PP_Get( wk->dst, ID_PARA_form_no, NULL );

  switch( mons ){
  case MONSNO_GIRATHINA:
    form = FORMNO_GIRATHINA_ANOTHER+(item==ITEM_HAKKINDAMA);
    break;
  case MONSNO_ARUSEUSU:
    form = POKETOOL_GetPokeTypeFromItem( item );
    break;
  case MONSNO_656:
    form = POKETOOL_GetPokeInsekutaFromItem( item );
    break;
  default:
    break;
  }
  if( form == now_form ){
    return;
  }
  box_update( wk, INPUTBOX_ID_FORM, form );
  update_dst( wk );
  box_setup_renew( wk );
}

//----------------------------------------------------------------------------------
/**
 * １つのボックス内容が更新された時、関連するボックス＆ポケモンパラメータの更新を行う
 *
 * @param   wk
 * @param   updateBoxID
 */
//----------------------------------------------------------------------------------
static void box_relation( DMP_MAINWORK* wk, u32 updateBoxID )
{
  switch( updateBoxID ){
  case INPUTBOX_ID_POKETYPE:
    {
      u16 monsno = box_getvalue( wk, INPUTBOX_ID_POKETYPE );
      u16 level = box_getvalue( wk, INPUTBOX_ID_LEVEL );
      u32 exp = POKETOOL_GetMinExp( monsno, PTL_FORM_NONE, level );

      PP_ChangeMonsNo( wk->dst, monsno );
      PP_Put( wk->dst, ID_PARA_exp, exp );
      PP_Put( wk->dst, ID_PARA_form_no, 0 );
      personalSpeabi_3rd_flag_set( wk->dst );
      PP_Renew( wk->dst );
      box_setup_form_change( wk );

      box_setup_renew( wk );
      UpdatePokeExpMinMax( wk, wk->dst );
    }
    break;
  case INPUTBOX_ID_SEIKAKU:
    {
      const INPUT_BOX_PARAM* p = &InputBoxParams[ updateBoxID ];
      u32 value = box_getvalue( wk, updateBoxID );
      PP_Put( wk->dst, p->paraID, value );

      PP_Renew( wk->dst );
      box_setup( wk, INPUTBOX_ID_HPVAL, wk->dst );
      box_setup( wk, INPUTBOX_ID_HPEDIT, wk->dst );
      box_setup( wk, INPUTBOX_ID_POWVAL, wk->dst );
      box_setup( wk, INPUTBOX_ID_DEFVAL, wk->dst );
      box_setup( wk, INPUTBOX_ID_AGIVAL, wk->dst );
      box_setup( wk, INPUTBOX_ID_SPWVAL, wk->dst );
      box_setup( wk, INPUTBOX_ID_SDFVAL, wk->dst );
    }
    break;

  case INPUTBOX_ID_PPCNT1:
  case INPUTBOX_ID_PPCNT2:
  case INPUTBOX_ID_PPCNT3:
  case INPUTBOX_ID_PPCNT4:
    {
      u8 idx = updateBoxID - INPUTBOX_ID_PPCNT1;
      u8 cnt = box_getvalue( wk, updateBoxID );
      u8 pp_max;
      PP_Put( wk->dst, ID_PARA_pp_count1 + idx, cnt );
      box_setup( wk, INPUTBOX_ID_PPMAX1+idx, wk->dst );
      pp_max = box_getvalue( wk, INPUTBOX_ID_PPMAX1+idx);
      IWASAWA_Printf("pp_cnt %d, pp_max = %d\n",cnt,pp_max);
      if( pp_max < box_getvalue( wk, INPUTBOX_ID_PPEDIT1+idx)){
        box_update( wk, INPUTBOX_ID_PPEDIT1, pp_max );
      }
    }
    break;
  case INPUTBOX_ID_WAZA1:
  case INPUTBOX_ID_WAZA2:
  case INPUTBOX_ID_WAZA3:
  case INPUTBOX_ID_WAZA4:
    {
      u8 idx = updateBoxID - INPUTBOX_ID_WAZA1;
      u16 waza = box_getvalue( wk, updateBoxID );
      PP_SetWazaPos( wk->dst, waza, idx );
      box_setup( wk, INPUTBOX_ID_PPMAX1+idx, wk->dst );
      box_setup( wk, INPUTBOX_ID_PPEDIT1+idx, wk->dst );
    }
    break;

  case INPUTBOX_ID_HPRND:
  case INPUTBOX_ID_HPEXP:
  case INPUTBOX_ID_POWRND:
  case INPUTBOX_ID_POWEXP:
  case INPUTBOX_ID_DEFRND:
  case INPUTBOX_ID_DEFEXP:
  case INPUTBOX_ID_AGIRND:
  case INPUTBOX_ID_AGIEXP:
  case INPUTBOX_ID_SPWRND:
  case INPUTBOX_ID_SPWEXP:
  case INPUTBOX_ID_SDFRND:
  case INPUTBOX_ID_SDFEXP:
    {
      const INPUT_BOX_PARAM* p = &InputBoxParams[ updateBoxID ];
      u32 value = box_getvalue( wk, updateBoxID );
      PP_Put( wk->dst, p->paraID, value );
      PP_Renew( wk->dst );
      if( (updateBoxID==INPUTBOX_ID_HPRND) || (updateBoxID==INPUTBOX_ID_HPEXP) )
      {
        u32 hpMax = PP_Get( wk->dst, ID_PARA_hpmax, NULL );
        PP_Put( wk->dst, ID_PARA_hp, hpMax );

        box_setup( wk, INPUTBOX_ID_HPVAL, wk->dst );
        box_setup( wk, INPUTBOX_ID_HPEDIT, wk->dst );
      }
      else
      {
        u32 upbox = ( (updateBoxID-INPUTBOX_ID_POWRND) % 3 == 0 )? updateBoxID+2 : updateBoxID+1;
        box_setup( wk, upbox, wk->dst );
      }
    }
    break;

  case INPUTBOX_ID_LEVEL:
    {
      u32 monsno, exp, level;

      monsno = box_getvalue( wk, INPUTBOX_ID_POKETYPE );
      level = box_getvalue( wk, INPUTBOX_ID_LEVEL );
      exp = POKETOOL_GetMinExp( monsno, PTL_FORM_NONE, level );

      PP_Put( wk->dst, ID_PARA_exp, exp );
      PP_Renew( wk->dst );
      box_setup( wk, INPUTBOX_ID_HPVAL, wk->dst );
      box_setup( wk, INPUTBOX_ID_HPEDIT, wk->dst );
      box_setup( wk, INPUTBOX_ID_POWVAL, wk->dst );
      box_setup( wk, INPUTBOX_ID_DEFVAL, wk->dst );
      box_setup( wk, INPUTBOX_ID_AGIVAL, wk->dst );
      box_setup( wk, INPUTBOX_ID_SPWVAL, wk->dst );
      box_setup( wk, INPUTBOX_ID_SDFVAL, wk->dst );

      UpdatePokeExpMinMax( wk, wk->dst );
      box_setup( wk, INPUTBOX_ID_EXP, wk->dst );
    }
    break;

  case INPUTBOX_ID_NICKNAME:
    {
      u8 flg = PP_Get( wk->dst, ID_PARA_nickname_flag, NULL );
      box_update( wk, INPUTBOX_ID_NICKNAME_FLG, flg );
    }
    break;

  case INPUTBOX_ID_GET_BALL:
    {
      u16 value = box_getvalue( wk, INPUTBOX_ID_GET_BALL );
      u8 ballID = ITEM_GetBallID( value );
      if( ballID == BALLID_NULL ){
        ballID = BALLID_MONSUTAABOORU;
        value = ITEM_MONSUTAABOORU;
        box_update( wk, INPUTBOX_ID_GET_BALL, value );
      }
      PP_Put( wk->dst, ID_PARA_get_ball, ballID );
    }
    break;

  case INPUTBOX_ID_GET_PLACE:
  case INPUTBOX_ID_BIRTH_PLACE:
    {
      const INPUT_BOX_PARAM* p = &InputBoxParams[ updateBoxID ];
      u32 value = box_getvalue( wk, updateBoxID );
      PP_Put( wk->dst, p->paraID, value );
      box_update( wk, updateBoxID+1, value );
    }
    break;

  case INPUTBOX_ID_OYAID_H:
  case INPUTBOX_ID_OYAID_L:
    {
      u32 id_H = box_getvalue( wk, INPUTBOX_ID_OYAID_H );
      u32 id_L = box_getvalue( wk, INPUTBOX_ID_OYAID_L );
      wk->oyaID = (id_H << 16) | (id_L);
      update_dst( wk );

      box_update( wk, INPUTBOX_ID_RARE, PP_CheckRare( wk->dst ) );
    }
    break;

  case INPUTBOX_ID_SEX_SET:
    {
      u8 form_no = box_getvalue( wk, INPUTBOX_ID_FORM );
      u16 mons_no = box_getvalue( wk, INPUTBOX_ID_POKETYPE );
      u8 tokusei = box_getvalue( wk, INPUTBOX_ID_TOKUSEI );
      u8 sex = box_getvalue( wk, INPUTBOX_ID_SEX_SET );
      u8 rare = box_getvalue( wk, INPUTBOX_ID_RARE );
      u32 personal_rnd = POKETOOL_CalcPersonalRandEx( wk->oyaID, mons_no, form_no, sex, 0, rare );
      box_update( wk, INPUTBOX_ID_PER_RND_L, personal_rnd&0xFFFF );
      box_update( wk, INPUTBOX_ID_PER_RND_H, personal_rnd>>16 );
    }
    //ブレイクスルー
  case INPUTBOX_ID_PER_RND_H:
  case INPUTBOX_ID_PER_RND_L:
    {
      u8 value;
      u8 rare;
      update_dst( wk );

      value = PP_Get( wk->dst, ID_PARA_sex, NULL );
      wk->boxValue[ INPUTBOX_ID_SEX_FIX] = value;
      box_update( wk, INPUTBOX_ID_SEX, value );
      box_update( wk, INPUTBOX_ID_SEX_SET, value );
      box_update( wk, INPUTBOX_ID_RARE, PP_CheckRare( wk->dst ) );
    }
    break;

  case INPUTBOX_ID_FORM:
  case INPUTBOX_ID_ITEM:
    box_setup_form_change( wk );
    break;

  case INPUTBOX_ID_TOKUSEI:
    {
      u32 value = box_getvalue( wk, updateBoxID );
      PP_Put( wk->dst, ID_PARA_speabino, value );
      personalSpeabi_3rd_flag_set( wk->dst );
      box_update( wk, INPUTBOX_ID_TOKUSEI_SWITCH,
          personalSpeabi_formal_to_local( wk->dst, value ) );
      box_setup( wk, INPUTBOX_ID_TOKUSEI_3RD, wk->dst );
      IWASAWA_Printf("Setup tokusei %d, switch %d flag %d\n", value,
          box_getvalue( wk, INPUTBOX_ID_TOKUSEI_SWITCH),
          box_getvalue( wk, INPUTBOX_ID_TOKUSEI_3RD));
    }
    break;

  case INPUTBOX_ID_TOKUSEI_SWITCH:
    {
      box_setup( wk, INPUTBOX_ID_TOKUSEI, wk->dst );
      box_setup( wk, INPUTBOX_ID_TOKUSEI_3RD, wk->dst );
      IWASAWA_Printf("Setup tokusei %d, switch %d flag %d\n",
          box_getvalue( wk, INPUTBOX_ID_TOKUSEI),
          box_getvalue( wk, INPUTBOX_ID_TOKUSEI_SWITCH),
          box_getvalue( wk, INPUTBOX_ID_TOKUSEI_3RD));
    }
    break;

  case INPUTBOX_ID_TAMAGO:
    {
      u8 tamago_flg = box_getvalue( wk, INPUTBOX_ID_TAMAGO );

      //ニックネーム
      if(tamago_flg){
        GFL_MSG_GetString( wk->msgData, DMPSTR_EGG_NAME, wk->tmpbuf );
      }else{
        GFL_MSG_GetString( GlobalMsg_PokeName, box_getvalue( wk, INPUTBOX_ID_POKETYPE ), wk->tmpbuf );
      }
      PP_Put( wk->dst, ID_PARA_nickname, (u32)wk->tmpbuf );
      box_setup( wk, INPUTBOX_ID_NICKNAME, wk->dst );
      box_setup( wk, INPUTBOX_ID_NICKNAME_FLG, wk->dst );
    }
    break;

  case INPUTBOX_ID_PPEDIT1:
  case INPUTBOX_ID_PPEDIT2:
  case INPUTBOX_ID_PPEDIT3:
  case INPUTBOX_ID_PPEDIT4:
  case INPUTBOX_ID_POKERUS:
  case INPUTBOX_ID_GET_LEVEL:
  case INPUTBOX_ID_GET_YEAR:
  case INPUTBOX_ID_GET_MONTH:
  case INPUTBOX_ID_GET_DAY:
  case INPUTBOX_ID_BIRTH_YEAR:
  case INPUTBOX_ID_BIRTH_MONTH:
  case INPUTBOX_ID_BIRTH_DAY:
    {
      const INPUT_BOX_PARAM* p = &InputBoxParams[ updateBoxID ];
      u32 value = box_getvalue( wk, updateBoxID );
      PP_Put( wk->dst, p->paraID, value );
    }
    break;
  }
}



//==============================================================================================
//  数値入力処理
//==============================================================================================
static void NumInput_Setup( NUMINPUT_WORK* wk, STRBUF* buf, GFL_BMPWIN* win, GFL_FONT* font,
            PRINT_UTIL* util, PRINT_QUE* que, const INPUT_BOX_PARAM* boxParam, u32 value, const DMP_MAINWORK* mainWork )
{
  PRINTSYS_LSB color = box_sub_get_bgcol( boxParam->type );

  wk->win = win;
  wk->bmp = GFL_BMPWIN_GetBmp( win );
  wk->font = font;
  wk->buf = buf;
  wk->printUtil = util;
  wk->printQue = que;
  wk->boxParam = boxParam;

  wk->draw_ox = boxParam->xpos + NUMBOX_MARGIN;
  wk->color_def_bg = PRINTSYS_LSB_GetB( color );
  wk->color_cur_bg = COLIDX_RED_L;
  wk->color_letter = PRINTSYS_LSB_GetL( color );

  wk->num = value;
  switch( boxParam->arg ){
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
  case INPUTBOX_NUM_ARG_FORM:
    wk->numMax = personal_getparam( mainWork->dst, POKEPER_ID_form_max )-1;
    wk->numMin = 0;
    break;
  }
  wk->keta = calc_keta( wk->numMax );
  IWASAWA_Printf(" num keta = %d max = %d\n", wk->keta, wk->numMax );
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
      if( key & PAD_KEY_UP )
      {
        int k = wk->cur_keta;
        while( k < (wk->keta) ){
          wk->num_ary[k]++;
          if( wk->num_ary[k] <= 9 ){ break; }
          wk->num_ary[k] = 0;
          ++k;
        }
        // 桁あふれしたら最大値に
        if( k == (wk->keta) ){
          numinput_sub_setary( wk, wk->numMax );
        }
        break;
      }
      if( key & PAD_KEY_DOWN )
      {
        int k = wk->cur_keta;
        while( k < (wk->keta) )
        {
          wk->num_ary[k]--;
          if( wk->num_ary[k] >= 0 ){ break; }
          wk->num_ary[k] = 9;
          ++k;
        }
        // 桁あふれしたら最小値に
        if( k == (wk->keta) ){
          numinput_sub_setary( wk, wk->numMin );
        }
        break;
      }
      if( key & PAD_BUTTON_L ){
        numinput_sub_setary( wk, wk->numMax );
        break;
      }
      if( key & PAD_BUTTON_R ){
        numinput_sub_setary( wk, wk->numMin );
        break;
      }

      update = FALSE;
    }while(0);

    if( update ){
      int num = numinput_sub_calcnum( wk );
      if( num > wk->numMax ){
        num = wk->numMax;
        numinput_sub_setary( wk, num );
      }
      if( num < wk->numMin ){
        num = wk->numMin;
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
static void numinput_draw( NUMINPUT_WORK* wk )
{
  const INPUT_BOX_PARAM* p = wk->boxParam;

  GFL_BMP_Fill( wk->bmp, p->xpos, p->ypos, p->width, p->height, wk->color_def_bg );
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

  GFL_STD_MemClear( wk->num_ary, sizeof(wk->num_ary) );
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
  for(i=0, k=1; i<(wk->keta+1); ++i, k*=10)
  {
    num += ( wk->num_ary[i] * k );
  }
  return num;
}
static u16 calc_keta( u32 value )
{
  u32 max, keta;

  if( value > 100000000 ){  //最大10桁
    return 10;
  }
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
//==============================================================================================
//  入力補完処理
//==============================================================================================
static void COMPSKB_Setup( COMP_SKB_WORK* wk, GFL_SKB* skb, STRBUF* buf, u32 msgArcID, u32 msgDataID, HEAPID heapID )
{
  wk->skb = skb;
  wk->msg = GFL_MSG_Create( GFL_MSG_LOAD_FAST, msgArcID, msgDataID, heapID );
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
    #if 0
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
    #endif
    wk->delayTimer = 90;
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
    if( wk->delayTimer )
    {
      wk->delayTimer--;
      if( wk->delayTimer == 0 ){
        fSearchReq = TRUE;
      }
    }
    {
      u16 key = GFL_UI_KEY_GetTrg();
      if( key & PAD_BUTTON_SELECT )
      {
        wk->delayTimer = 0;
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


//==============================================================================================
//  パーソナルデータアクセス
//==============================================================================================
static u32 personal_getparam( const POKEMON_PARAM* pp, PokePersonalParamID paramID )
{
  u16 monsno = PP_Get( pp, ID_PARA_monsno,  NULL );
  u16 formno = PP_Get( pp, ID_PARA_form_no, NULL );
  return POKETOOL_GetPersonalParam( monsno, formno, paramID );
}

static u8 personal_get_tokusei_kinds( const POKEMON_PARAM* pp )
{
  u8 n = 0;
  if( personal_getparam(pp, POKEPER_ID_speabi1) ){
    ++n;
  }
  if( personal_getparam(pp, POKEPER_ID_speabi2) ){
    ++n;
  }
  return n;
}
static u16 personal_get_tokusei( const POKEMON_PARAM* pp, u8 idx )
{
  u16 tok[2];
  u8 n = 0;
  if( (tok[n] = personal_getparam(pp, POKEPER_ID_speabi1)) != 0 ){
    ++n;
  }
  if( (tok[n] = personal_getparam(pp, POKEPER_ID_speabi2)) != 0 ){
    ++n;
  }
  if( idx >= n ){
    idx = n-1;
  }
  return tok[ idx ];
}
static BOOL personal_is_sex_fixed( const POKEMON_PARAM* pp )
{
  u32 v = personal_getparam( pp, POKEPER_ID_sex );
  if( (v == POKEPER_SEX_MALE)
  ||  (v == POKEPER_SEX_FEMALE)
  ||  (v == POKEPER_SEX_UNKNOWN)
  ){
    return TRUE;
  }
  return FALSE;
}
//==============================================================================================
//  カセットバージョン変換
//==============================================================================================
/**
 *  カセットバージョン（gmmのID順と同じにしておく）
 */
static const u8 CasetteVer[] = {
  VERSION_BLACK,    VERSION_WHITE,    VERSION_GOLD,     VERSION_SILVER,
  VERSION_DIAMOND,  VERSION_PEARL,    VERSION_PLATINUM, VERSION_RUBY,
  VERSION_SAPPHIRE, VERSION_EMERALD,  VERSION_RED,      VERSION_GREEN,
  VERSION_COLOSSEUM,
};

// PP設定 -> ローカル値
static u8 casetteVer_formal_to_local( u8 val )
{
  int i;
  for(i=0; i<NELEMS(CasetteVer); ++i){
    if( CasetteVer[i] == val ){
      return i;
    }
  }
  return 0;
}
static u8 casetteVer_local_to_formal( u8 val )
{
  return CasetteVer[ val ];
}
//==============================================================================================
//  国コード変換
//==============================================================================================
static const u8 CountryCode[] = {
  LANG_JAPAN, LANG_ENGLISH, LANG_FRANCE,
  LANG_ITALY, LANG_GERMANY, LANG_SPAIN,
  LANG_KOREA,
};
// PP設定 -> ローカル値
static u8 countryCode_formal_to_local( u8 val )
{
  int i;
  for(i=0; i<NELEMS(CountryCode); ++i){
    if( CountryCode[i] == val ){
      return i;
    }
  }
  return 0;
}
static u8 countryCode_local_to_formal( u8 value )
{
  return CountryCode[ value ];
}

//==============================================================================================
// パーソナル特性コード変換
//==============================================================================================
static void speabi_ary_get( const POKEMON_PARAM* pp ,u8* abi )
{
  int i;

  for(i = 0;i < 3;i++){
    abi[i] = personal_getparam(pp,POKEPER_ID_speabi1+i);
  }
  IWASAWA_Printf(" Speabi Get %d, %d,%d\n", abi[0],abi[1],abi[2]);
}

// PP設定 -> ローカル値
static u8 personalSpeabi_formal_to_local( const POKEMON_PARAM* pp, u8 val )
{
  int i;
  u8  abi[3];

  speabi_ary_get( pp, abi );

  for(i=0; i<3; i++){
    if( abi[i] == val ){
      return i;
    }
  }
  return 3;
}
static u8 personalSpeabi_local_to_formal( const POKEMON_PARAM* pp, u8 value )
{
  u8  abi[3];
  speabi_ary_get( pp, abi );

  if( value > 2 ){
    return PP_Get( pp, ID_PARA_speabino, NULL );
  }
  if( abi[value] == 0 ){
    return abi[0];
  }
  return abi[ value ];
}
static void personalSpeabi_3rd_flag_set( POKEMON_PARAM* pp )
{
  u8  abi[3];
  u8  speabi = PP_Get( pp, ID_PARA_speabino, NULL );

  speabi_ary_get( pp, abi );

  PP_Put( pp, ID_PARA_tokusei_3_flag, (speabi == abi[2]));

  IWASAWA_Printf(" Speabi 3rd = %d abi=%d, %d\n", (speabi == abi[2] ), speabi, abi[2]);
}

#endif
