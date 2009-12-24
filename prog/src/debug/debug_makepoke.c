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
#include "poke_tool/monsno_def.h"
#include "waza_tool/wazano_def.h"

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
  PROC_HEAP_SIZE = 0x20000,
  PRINT_FRAME = GFL_BG_FRAME1_M,

  PRINT_PALIDX  = 0,
  SKB_PALIDX1   = 1,
  SKB_PALIDX2   = 2,

  STRBUF_LEN = 64,
  NUMINPUT_KETA_MAX = 10,

  NUMBOX_MARGIN = 4,
  NUMBOX_CHAR_WIDTH = 8,


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

  INPUTBOX_NUM_ARG_EXP = 0xffffffff,
  INPUTBOX_NUM_ARG_DEPEND = 0xfffffffe,

  DMPSTR_NULL = -1,
};

#define CALC_NUMBOX_WIDTH(keta)      ((keta)*NUMBOX_CHAR_WIDTH +NUMBOX_MARGIN*2)
#define CALC_STRBOX_WIDTH(letterCnt)   ((letterCnt)*8 +8)
#define CALC_CAP_BOX_MARGIN(letterCnt)  ((letterCnt*8)+4)
#define CALC_CAP_BOX_MARGIN_HALF(letterCnt)  ((letterCnt*4)+4)

enum {
  LINE_HEIGHT = 12,
  LINE_MARGIN = 2,
  LINE_DIFFER = LINE_HEIGHT + LINE_MARGIN,

  LY_LV1 = 4,
  LY_LV2 = LY_LV1+LINE_DIFFER,
  LY_LV3 = LY_LV2+LINE_DIFFER,
  LY_LV4 = LY_LV3+LINE_DIFFER + 3,
  LY_LV5 = LY_LV4+LINE_DIFFER,
  LY_LV6 = LY_LV5+LINE_DIFFER,
  LY_LV7 = LY_LV6+LINE_DIFFER,
  LY_LV8 = LY_LV7+LINE_DIFFER + 3,
  LY_LV9 = LY_LV8+LINE_DIFFER,
  LY_LV10 = LY_LV9+LINE_DIFFER,
  LY_LV11 = LY_LV10+LINE_DIFFER,
  LY_LV12 = LY_LV11+LINE_DIFFER,
  LY_LV13 = LY_LV12+LINE_DIFFER,

  LX_POKETYPE_CAP = 2,
  LY_POKETYPE_CAP = LY_LV1,
  LX_POKETYPE_BOX = LX_POKETYPE_CAP+CALC_CAP_BOX_MARGIN(4),
  LY_POKETYPE_BOX = LY_LV1,

  LX_LEVEL_CAP = 96,
  LY_LEVEL_CAP = LY_LV1,
  LX_LEVEL_BOX = LX_LEVEL_CAP+CALC_CAP_BOX_MARGIN(3),
  LY_LEVEL_BOX = LY_LEVEL_CAP,

  LX_EXP_CAP = 160,
  LY_EXP_CAP = LY_LV1,
  LX_EXP_BOX = LX_EXP_CAP+CALC_CAP_BOX_MARGIN_HALF(3),
  LY_EXP_BOX = LY_EXP_CAP,

  LX_SEIKAKU_CAP = 2,
  LY_SEIKAKU_CAP = LY_LV2,
  LX_SEIKAKU_BOX = LX_SEIKAKU_CAP + CALC_CAP_BOX_MARGIN(4),
  LY_SEIKAKU_BOX = LY_SEIKAKU_CAP,

  LX_TOKUSEI_CAP = 88,
  LY_TOKUSEI_CAP = LY_LV2,
  LX_TOKUSEI_BOX = LX_TOKUSEI_CAP + CALC_CAP_BOX_MARGIN(4),
  LY_TOKUSEI_BOX = LY_TOKUSEI_CAP,

  LX_SEX_CAP = 198,
  LY_SEX_CAP = LY_LV2,
  LX_SEX_BOX = LX_SEX_CAP+CALC_CAP_BOX_MARGIN(4),
  LY_SEX_BOX = LY_SEX_CAP,

  LX_ITEM_CAP = 2,
  LY_ITEM_CAP = LY_LV3,
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

  //-------------------------------------------------------------
  // PP欄レイアウト
  //-------------------------------------------------------------
  LX_PP_UPCNT_CAP = 104,
  LX_PP_UPCNT_BOX = LX_PP_UPCNT_CAP + CALC_CAP_BOX_MARGIN_HALF(6),
  LX_PP_EDIT_BOX = LX_PP_UPCNT_BOX + CALC_CAP_BOX_MARGIN_HALF(4),
  LX_PP_MAX_CAP = LX_PP_EDIT_BOX + CALC_CAP_BOX_MARGIN_HALF(5),
  LX_PP_MAX_BOX = LX_PP_MAX_CAP + 8,


  LX_PPCNT1_CAP = LX_PP_UPCNT_CAP,
  LY_PPCNT1_CAP = LY_LV4,
  LX_PPCNT1_BOX = LX_PP_UPCNT_BOX,
  LY_PPCNT1_BOX = LY_PPCNT1_CAP,

  LX_PPCNT2_CAP = LX_PP_UPCNT_CAP,
  LY_PPCNT2_CAP = LY_LV5,
  LX_PPCNT2_BOX = LX_PP_UPCNT_BOX,
  LY_PPCNT2_BOX = LY_PPCNT2_CAP,

  LX_PPCNT3_CAP = LX_PP_UPCNT_CAP,
  LY_PPCNT3_CAP = LY_LV6,
  LX_PPCNT3_BOX = LX_PP_UPCNT_BOX,
  LY_PPCNT3_BOX = LY_PPCNT3_CAP,

  LX_PPCNT4_CAP = LX_PP_UPCNT_CAP,
  LY_PPCNT4_CAP = LY_LV7,
  LX_PPCNT4_BOX = LX_PP_UPCNT_BOX,
  LY_PPCNT4_BOX = LY_PPCNT4_CAP,

  LX_PPMAX1_CAP = LX_PP_MAX_CAP,
  LY_PPMAX1_CAP = LY_LV4,
  LX_PPMAX1_BOX = LX_PP_MAX_BOX,
  LY_PPMAX1_BOX = LY_PPMAX1_CAP,

  LX_PPMAX2_CAP = LX_PP_MAX_CAP,
  LY_PPMAX2_CAP = LY_LV5,
  LX_PPMAX2_BOX = LX_PP_MAX_BOX,
  LY_PPMAX2_BOX = LY_PPMAX2_CAP,

  LX_PPMAX3_CAP = LX_PP_MAX_CAP,
  LY_PPMAX3_CAP = LY_LV6,
  LX_PPMAX3_BOX = LX_PP_MAX_BOX,
  LY_PPMAX3_BOX = LY_PPMAX3_CAP,

  LX_PPMAX4_CAP = LX_PP_MAX_CAP,
  LY_PPMAX4_CAP = LY_LV7,
  LX_PPMAX4_BOX = LX_PP_MAX_BOX,
  LY_PPMAX4_BOX = LY_PPMAX4_CAP,

  LX_PPEDIT1_BOX = LX_PP_EDIT_BOX,
  LY_PPEDIT1_BOX = LY_PPMAX1_CAP,

  LX_PPEDIT2_BOX = LX_PP_EDIT_BOX,
  LY_PPEDIT2_BOX = LY_PPMAX2_CAP,

  LX_PPEDIT3_BOX = LX_PP_EDIT_BOX,
  LY_PPEDIT3_BOX = LY_PPMAX3_CAP,

  LX_PPEDIT4_BOX = LX_PP_EDIT_BOX,
  LY_PPEDIT4_BOX = LY_PPMAX4_CAP,

  //-------------------------------------------------------------
  // ステータス欄レイアウト
  //-------------------------------------------------------------
  STATUS_CAP_LX = 4,
  STATUS_CAP_RND_SPACE = CALC_CAP_BOX_MARGIN_HALF(4),     // キャプションと[rnd]列の間隔
  STATUS_RND_VAL_SPACE = CALC_CAP_BOX_MARGIN_HALF(3),     // [rnd]列と値部分の間隔
  STATUS_RND_EXP_SPACE = CALC_CAP_BOX_MARGIN_HALF(6),     // [rnd]値部分と[EXP]列の間隔
  STATUS_EXP_VAL_SPACE = CALC_CAP_BOX_MARGIN_HALF(3),     // [EXP]列と値部分の間隔
  STATUS_EXP_RESULT_SPACE = CALC_NUMBOX_WIDTH(3)+4,       // [EXP]値部分と計算結果の間隔

  // HP  -------------------------------------------------------------
  LX_HPVAL_CAP = STATUS_CAP_LX,
  LY_HPVAL_CAP = LY_LV8,
  LX_HPRND_CAP = LX_HPVAL_CAP + STATUS_CAP_RND_SPACE,
  LY_HPRND_CAP = LY_LV8,
  LX_HPRND_BOX = LX_HPRND_CAP + STATUS_RND_VAL_SPACE,
  LY_HPRND_BOX = LY_LV8,
  LX_HPEXP_CAP = LX_HPRND_BOX + STATUS_RND_EXP_SPACE,
  LY_HPEXP_CAP = LY_LV8,
  LX_HPEXP_BOX = LX_HPEXP_CAP + STATUS_EXP_VAL_SPACE,
  LY_HPEXP_BOX = LY_LV8,
  LX_HPEDIT_BOX  = LX_HPEXP_BOX + STATUS_EXP_RESULT_SPACE,
  LY_HPEDIT_BOX  = LY_LV8,
  LX_HPSLASH_CAP = LX_HPEDIT_BOX + CALC_NUMBOX_WIDTH(3)+2,
  LY_HPSLASH_CAP = LY_LV8,
  LX_HPVAL_BOX   = LX_HPSLASH_CAP + 8,
  LY_HPVAL_BOX   = LY_LV8,

  // POW -------------------------------------------------------------
  LX_POWVAL_CAP = STATUS_CAP_LX,
  LY_POWVAL_CAP = LY_LV9,
  LX_POWRND_CAP = LX_POWVAL_CAP + STATUS_CAP_RND_SPACE,
  LY_POWRND_CAP = LY_LV9,
  LX_POWRND_BOX = LX_POWRND_CAP + STATUS_RND_VAL_SPACE,
  LY_POWRND_BOX = LY_POWRND_CAP,
  LX_POWEXP_CAP = LX_POWRND_BOX + STATUS_RND_EXP_SPACE,
  LY_POWEXP_CAP = LY_LV9,
  LX_POWEXP_BOX = LX_POWEXP_CAP + STATUS_EXP_VAL_SPACE,
  LY_POWEXP_BOX = LY_LV9,
  LX_POWVAL_BOX = LX_POWEXP_BOX + STATUS_EXP_RESULT_SPACE,
  LY_POWVAL_BOX = LY_LV9,

  // DEF -------------------------------------------------------------
  LX_DEFVAL_CAP = STATUS_CAP_LX,
  LY_DEFVAL_CAP = LY_LV10,
  LX_DEFRND_CAP = LX_DEFVAL_CAP + STATUS_CAP_RND_SPACE,
  LY_DEFRND_CAP = LY_LV10,
  LX_DEFRND_BOX = LX_DEFRND_CAP + STATUS_RND_VAL_SPACE,
  LY_DEFRND_BOX = LY_LV10,
  LX_DEFEXP_CAP = LX_DEFRND_BOX + STATUS_RND_EXP_SPACE,
  LY_DEFEXP_CAP = LY_LV10,
  LX_DEFEXP_BOX = LX_DEFEXP_CAP + STATUS_EXP_VAL_SPACE,
  LY_DEFEXP_BOX = LY_LV10,
  LX_DEFVAL_BOX = LX_DEFEXP_BOX + STATUS_EXP_RESULT_SPACE,
  LY_DEFVAL_BOX = LY_LV10,

  // AGI -------------------------------------------------------------
  LX_AGIVAL_CAP = STATUS_CAP_LX,
  LY_AGIVAL_CAP = LY_LV11,
  LX_AGIRND_CAP = LX_AGIVAL_CAP + STATUS_CAP_RND_SPACE,
  LY_AGIRND_CAP = LY_LV11,
  LX_AGIRND_BOX = LX_AGIRND_CAP + STATUS_RND_VAL_SPACE,
  LY_AGIRND_BOX = LY_LV11,
  LX_AGIEXP_CAP = LX_AGIRND_BOX + STATUS_RND_EXP_SPACE,
  LY_AGIEXP_CAP = LY_LV11,
  LX_AGIEXP_BOX = LX_AGIEXP_CAP + STATUS_EXP_VAL_SPACE,
  LY_AGIEXP_BOX = LY_LV11,
  LX_AGIVAL_BOX = LX_AGIEXP_BOX + STATUS_EXP_RESULT_SPACE,
  LY_AGIVAL_BOX = LY_LV11,

  // SP-POW ----------------------------------------------------------
  LX_SPWVAL_CAP = STATUS_CAP_LX,
  LY_SPWVAL_CAP = LY_LV12,
  LX_SPWRND_CAP = LX_SPWVAL_CAP + STATUS_CAP_RND_SPACE,
  LY_SPWRND_CAP = LY_LV12,
  LX_SPWRND_BOX = LX_SPWRND_CAP + STATUS_RND_VAL_SPACE,
  LY_SPWRND_BOX = LY_LV12,
  LX_SPWEXP_CAP = LX_SPWRND_BOX + STATUS_RND_EXP_SPACE,
  LY_SPWEXP_CAP = LY_LV12,
  LX_SPWEXP_BOX = LX_SPWEXP_CAP + STATUS_EXP_VAL_SPACE,
  LY_SPWEXP_BOX = LY_LV12,
  LX_SPWVAL_BOX = LX_SPWEXP_BOX + STATUS_EXP_RESULT_SPACE,
  LY_SPWVAL_BOX = LY_LV12,

  // SP-DEF ----------------------------------------------------------
  LX_SDFVAL_CAP = STATUS_CAP_LX,
  LY_SDFVAL_CAP = LY_LV13,
  LX_SDFRND_CAP = LX_SDFVAL_CAP + STATUS_CAP_RND_SPACE,
  LY_SDFRND_CAP = LY_LV13,
  LX_SDFRND_BOX = LX_SDFRND_CAP + STATUS_RND_VAL_SPACE,
  LY_SDFRND_BOX = LY_LV13,
  LX_SDFEXP_CAP = LX_SDFRND_BOX + STATUS_RND_EXP_SPACE,
  LY_SDFEXP_CAP = LY_LV13,
  LX_SDFEXP_BOX = LX_SDFEXP_CAP + STATUS_EXP_VAL_SPACE,
  LY_SDFEXP_BOX = LY_LV13,
  LX_SDFVAL_BOX = LX_SDFEXP_BOX + STATUS_EXP_RESULT_SPACE,
  LY_SDFVAL_BOX = LY_LV13,


  LX_TYPE_CAP = 120,
  LY_TYPE_CAP = LY_LV3,
  LX_TYPE_BOX = LX_TYPE_CAP + CALC_CAP_BOX_MARGIN(3),
  LY_TYPE_BOX = LY_TYPE_CAP,

  LX_DEFAULT_BTN = LX_HPVAL_BOX+CALC_NUMBOX_WIDTH(3)+4,
  LY_DEFAULT_BTN = LY_LV8,

  ID_PARA_SEIKAKU = ID_PARA_end,
};

typedef enum {
  INPUTBOX_TYPE_STR,      ///< 入力対応文字列
  INPUTBOX_TYPE_NUM,      ///< 入力対応数値
  INPUTBOX_TYPE_SWITCH,   ///< タッチ対応トグルスイッチ
  INPUTBOX_TYPE_FIXVAL,   ///< 表示するだけの数値
  INPUTBOX_TYPE_FIXSTR,   ///< 表示するだけの文字列
  INPUTBOX_TYPE_BTN,      ///< タッチ反応で特殊処理するためのボタン
}InputBoxType;

typedef enum {
  INPUTBOX_ID_POKETYPE=0,
  INPUTBOX_ID_LEVEL,
  INPUTBOX_ID_EXP,
  INPUTBOX_ID_SEX,
  INPUTBOX_ID_SEIKAKU,
  INPUTBOX_ID_TOKUSEI,
  INPUTBOX_ID_ITEM,

  INPUTBOX_ID_WAZA1,
  INPUTBOX_ID_WAZA2,
  INPUTBOX_ID_WAZA3,
  INPUTBOX_ID_WAZA4,

  INPUTBOX_ID_PPCNT1,
  INPUTBOX_ID_PPCNT2,
  INPUTBOX_ID_PPCNT3,
  INPUTBOX_ID_PPCNT4,

  INPUTBOX_ID_PPMAX1,
  INPUTBOX_ID_PPMAX2,
  INPUTBOX_ID_PPMAX3,
  INPUTBOX_ID_PPMAX4,

  INPUTBOX_ID_PPEDIT1,
  INPUTBOX_ID_PPEDIT2,
  INPUTBOX_ID_PPEDIT3,
  INPUTBOX_ID_PPEDIT4,

  INPUTBOX_ID_HPRND,
  INPUTBOX_ID_HPEXP,
  INPUTBOX_ID_HPVAL,
  INPUTBOX_ID_HPEDIT,

  INPUTBOX_ID_POWRND,
  INPUTBOX_ID_POWEXP,
  INPUTBOX_ID_POWVAL,

  INPUTBOX_ID_DEFRND,
  INPUTBOX_ID_DEFEXP,
  INPUTBOX_ID_DEFVAL,

  INPUTBOX_ID_AGIRND,
  INPUTBOX_ID_AGIEXP,
  INPUTBOX_ID_AGIVAL,

  INPUTBOX_ID_SPWRND,
  INPUTBOX_ID_SPWEXP,
  INPUTBOX_ID_SPWVAL,

  INPUTBOX_ID_SDFRND,
  INPUTBOX_ID_SDFEXP,
  INPUTBOX_ID_SDFVAL,

  INPUTBOX_ID_TYPE1,
  INPUTBOX_ID_TYPE2,
  INPUTBOX_ID_DEF_BUTTON,

  INPUTBOX_ID_MAX,


}InputBoxID;

typedef enum {
  SWITCH_STR_DEFAULT,
  SWITCH_STR_TOKUSEI,
}SwitchBoxStrType;

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
  u16  type;
  s16  cap_strID;
  u8   cap_xpos;
  u8   cap_ypos;

  u8   xpos;
  u8   ypos;
  u8   width;
  u8   height;

  u16  paraID;
  u32  arg;
  u32  arg2;

}INPUT_BOX_PARAM;


static const INPUT_BOX_PARAM InputBoxParams[] = {

  { INPUTBOX_TYPE_STR, DMPSTR_POKEMON, LX_POKETYPE_CAP, LY_POKETYPE_CAP,
    LX_POKETYPE_BOX, LY_POKETYPE_BOX, 56, LINE_HEIGHT,
    ID_PARA_monsno, NARC_message_monsname_dat, 0  },

  { INPUTBOX_TYPE_NUM, DMPSTR_LEVEL,   LX_LEVEL_CAP,  LY_LEVEL_CAP,
    LX_LEVEL_BOX,   LY_LEVEL_BOX,  CALC_NUMBOX_WIDTH(3), LINE_HEIGHT,
    ID_PARA_level, 100, 1 },

  { INPUTBOX_TYPE_NUM, DMPSTR_HP_EXP,  LX_EXP_CAP,  LY_EXP_CAP,
    LX_EXP_BOX,   LY_EXP_BOX,  CALC_NUMBOX_WIDTH(8), LINE_HEIGHT,
    ID_PARA_exp, INPUTBOX_NUM_ARG_EXP, 0 },

  { INPUTBOX_TYPE_SWITCH,  DMPSTR_SEX,   LX_SEX_CAP,  LY_SEX_CAP,
    LX_SEX_BOX,   LY_SEX_BOX,  CALC_NUMBOX_WIDTH(1), LINE_HEIGHT,
    ID_PARA_sex, DMPSTR_SEX_MALE, SWITCH_STR_DEFAULT },

  { INPUTBOX_TYPE_STR,  DMPSTR_SEIKAKU,    LX_SEIKAKU_CAP,    LY_SEIKAKU_CAP,
    LX_SEIKAKU_BOX,    LY_SEIKAKU_BOX,    CALC_STRBOX_WIDTH(5),   LINE_HEIGHT,
    ID_PARA_SEIKAKU,   NARC_message_chr_dat, 0 },

  { INPUTBOX_TYPE_STR,  DMPSTR_TOKUSEI,    LX_TOKUSEI_CAP,    LY_TOKUSEI_CAP,
    LX_TOKUSEI_BOX,    LY_TOKUSEI_BOX,    CALC_STRBOX_WIDTH(8),   LINE_HEIGHT,
    ID_PARA_speabino,  NARC_message_tokusei_dat, SWITCH_STR_TOKUSEI },

  { INPUTBOX_TYPE_STR,  DMPSTR_ITEM,    LX_ITEM_CAP,    LY_ITEM_CAP,
    LX_ITEM_BOX,    LY_ITEM_BOX,    CALC_STRBOX_WIDTH(8),   LINE_HEIGHT,
    ID_PARA_item, NARC_message_itemname_dat, 0 },

  { INPUTBOX_TYPE_STR,  DMPSTR_WAZA1,   LX_WAZA1_CAP,   LY_WAZA1_CAP,
    LX_WAZA1_BOX,   LY_WAZA1_BOX,   CALC_STRBOX_WIDTH(7), LINE_HEIGHT,
    ID_PARA_waza1, NARC_message_wazaname_dat,  0 },

  { INPUTBOX_TYPE_STR,  DMPSTR_WAZA2,   LX_WAZA2_CAP,   LY_WAZA2_CAP,
    LX_WAZA2_BOX,   LY_WAZA2_BOX,   CALC_STRBOX_WIDTH(7), LINE_HEIGHT,
    ID_PARA_waza2, NARC_message_wazaname_dat,    0 },

  { INPUTBOX_TYPE_STR,  DMPSTR_WAZA3,   LX_WAZA3_CAP,   LY_WAZA3_CAP,
    LX_WAZA3_BOX,   LY_WAZA3_BOX,   CALC_STRBOX_WIDTH(7), LINE_HEIGHT,
    ID_PARA_waza3, NARC_message_wazaname_dat,    0 },

  { INPUTBOX_TYPE_STR,  DMPSTR_WAZA4,   LX_WAZA4_CAP,   LY_WAZA4_CAP,
    LX_WAZA4_BOX,   LY_WAZA4_BOX,   CALC_STRBOX_WIDTH(7), LINE_HEIGHT,
    ID_PARA_waza4, NARC_message_wazaname_dat,    0 },

  //---------------------------------------------------
  // PP系
  //---------------------------------------------------
  { INPUTBOX_TYPE_NUM,  DMPSTR_PPCNT1,   LX_PPCNT1_CAP,   LY_PPCNT1_CAP,
    LX_PPCNT1_BOX,   LY_PPCNT1_BOX,   CALC_NUMBOX_WIDTH(1), LINE_HEIGHT,
    ID_PARA_pp_count1, 3,  0 },

  { INPUTBOX_TYPE_NUM,  DMPSTR_PPCNT2,   LX_PPCNT2_CAP,   LY_PPCNT2_CAP,
    LX_PPCNT2_BOX,   LY_PPCNT2_BOX,   CALC_NUMBOX_WIDTH(1), LINE_HEIGHT,
    ID_PARA_pp_count2, 3,    0 },

  { INPUTBOX_TYPE_NUM,  DMPSTR_PPCNT3,   LX_PPCNT3_CAP,   LY_PPCNT3_CAP,
    LX_PPCNT3_BOX,   LY_PPCNT3_BOX,   CALC_NUMBOX_WIDTH(1), LINE_HEIGHT,
    ID_PARA_pp_count3, 3,    0 },

  { INPUTBOX_TYPE_NUM,  DMPSTR_PPCNT4,   LX_PPCNT4_CAP,   LY_PPCNT4_CAP,
    LX_PPCNT4_BOX,   LY_PPCNT4_BOX,   CALC_NUMBOX_WIDTH(1), LINE_HEIGHT,
    ID_PARA_pp_count4, 3,    0 },

  { INPUTBOX_TYPE_FIXVAL,  DMPSTR_SLASH,   LX_PPMAX1_CAP,  LY_PPMAX1_CAP,
    LX_PPMAX1_BOX,   LY_PPMAX1_BOX,   CALC_NUMBOX_WIDTH(2), LINE_HEIGHT,
    ID_PARA_pp_max1, 100,  0 },

  { INPUTBOX_TYPE_FIXVAL,  DMPSTR_SLASH,   LX_PPMAX2_CAP,  LY_PPMAX2_CAP,
    LX_PPMAX2_BOX,   LY_PPMAX2_BOX,   CALC_NUMBOX_WIDTH(2), LINE_HEIGHT,
    ID_PARA_pp_max2, 100,    0 },

  { INPUTBOX_TYPE_FIXVAL,  DMPSTR_SLASH,   LX_PPMAX3_CAP,  LY_PPMAX3_CAP,
    LX_PPMAX3_BOX,   LY_PPMAX3_BOX,   CALC_NUMBOX_WIDTH(2), LINE_HEIGHT,
    ID_PARA_pp_max3, 100,    0 },

  { INPUTBOX_TYPE_FIXVAL,  DMPSTR_SLASH,   LX_PPMAX4_CAP,  LY_PPMAX4_CAP,
    LX_PPMAX4_BOX,   LY_PPMAX4_BOX,   CALC_NUMBOX_WIDTH(2), LINE_HEIGHT,
    ID_PARA_pp_max4,  100,  0 },

  { INPUTBOX_TYPE_NUM,  DMPSTR_NULL,   0,   0,
    LX_PPEDIT1_BOX,   LY_PPEDIT1_BOX,   CALC_NUMBOX_WIDTH(2), LINE_HEIGHT,
    ID_PARA_pp1, INPUTBOX_NUM_ARG_DEPEND,    INPUTBOX_ID_PPMAX1 },

  { INPUTBOX_TYPE_NUM,  DMPSTR_NULL,   0,   0,
    LX_PPEDIT2_BOX,   LY_PPEDIT2_BOX,   CALC_NUMBOX_WIDTH(2), LINE_HEIGHT,
    ID_PARA_pp2, INPUTBOX_NUM_ARG_DEPEND,    INPUTBOX_ID_PPMAX2 },

  { INPUTBOX_TYPE_NUM,  DMPSTR_NULL,   0,   0,
    LX_PPEDIT3_BOX,   LY_PPEDIT3_BOX,   CALC_NUMBOX_WIDTH(2), LINE_HEIGHT,
    ID_PARA_pp3, INPUTBOX_NUM_ARG_DEPEND,    INPUTBOX_ID_PPMAX3 },

  { INPUTBOX_TYPE_NUM,  DMPSTR_NULL,   0,   0,
    LX_PPEDIT4_BOX,   LY_PPEDIT4_BOX,   CALC_NUMBOX_WIDTH(2), LINE_HEIGHT,
    ID_PARA_pp4, INPUTBOX_NUM_ARG_DEPEND,    INPUTBOX_ID_PPMAX4 },

  //---------------------------------------------------
  // パラメータ系
  //---------------------------------------------------
  { INPUTBOX_TYPE_NUM,  DMPSTR_HP_RND,   LX_HPRND_CAP,   LY_HPRND_CAP,
    LX_HPRND_BOX,    LY_HPRND_BOX,    CALC_NUMBOX_WIDTH(2), LINE_HEIGHT,
    ID_PARA_hp_rnd,  31,  0 },

  { INPUTBOX_TYPE_NUM,  DMPSTR_HP_EXP,   LX_HPEXP_CAP,   LY_HPEXP_CAP,
    LX_HPEXP_BOX,    LY_HPEXP_BOX,    CALC_NUMBOX_WIDTH(3), LINE_HEIGHT,
    ID_PARA_hp_exp,  255,  0 },

  { INPUTBOX_TYPE_FIXVAL,  DMPSTR_HP,   LX_HPVAL_CAP,   LY_HPVAL_CAP,
    LX_HPVAL_BOX,    LY_HPVAL_BOX,    CALC_NUMBOX_WIDTH(3), LINE_HEIGHT,
    ID_PARA_hpmax,   512,  0 },

  { INPUTBOX_TYPE_NUM,  DMPSTR_SLASH,    LX_HPSLASH_CAP,       LY_HPSLASH_CAP,
    LX_HPEDIT_BOX,   LY_HPEDIT_BOX,   CALC_NUMBOX_WIDTH(3), LINE_HEIGHT,
    ID_PARA_hp,      INPUTBOX_NUM_ARG_DEPEND,  INPUTBOX_ID_HPVAL },

  { INPUTBOX_TYPE_NUM,  DMPSTR_POW_RND,   LX_POWRND_CAP,   LY_POWRND_CAP,
    LX_POWRND_BOX,    LY_POWRND_BOX,    CALC_NUMBOX_WIDTH(2), LINE_HEIGHT,
    ID_PARA_pow_rnd,  31,  0 },

  { INPUTBOX_TYPE_NUM,  DMPSTR_POW_EXP,   LX_POWEXP_CAP,   LY_POWEXP_CAP,
    LX_POWEXP_BOX,    LY_POWEXP_BOX,    CALC_NUMBOX_WIDTH(3), LINE_HEIGHT,
    ID_PARA_pow_exp,  255,  0 },

  { INPUTBOX_TYPE_FIXVAL,  DMPSTR_POW,   LX_POWVAL_CAP,   LY_POWVAL_CAP,
    LX_POWVAL_BOX,    LY_POWVAL_BOX,    CALC_NUMBOX_WIDTH(3), LINE_HEIGHT,
    ID_PARA_pow,   255,  0 },

  { INPUTBOX_TYPE_NUM,  DMPSTR_DEF_RND,   LX_DEFRND_CAP,   LY_DEFRND_CAP,
    LX_DEFRND_BOX,    LY_DEFRND_BOX,    CALC_NUMBOX_WIDTH(2), LINE_HEIGHT,
    ID_PARA_def_rnd,  31,  0 },

  { INPUTBOX_TYPE_NUM,  DMPSTR_DEF_EXP,   LX_DEFEXP_CAP,   LY_DEFEXP_CAP,
    LX_DEFEXP_BOX,    LY_DEFEXP_BOX,    CALC_NUMBOX_WIDTH(3), LINE_HEIGHT,
    ID_PARA_def_exp,  255,  0 },

  { INPUTBOX_TYPE_FIXVAL,  DMPSTR_DEF,   LX_DEFVAL_CAP,   LY_DEFVAL_CAP,
    LX_DEFVAL_BOX,    LY_DEFVAL_BOX,    CALC_NUMBOX_WIDTH(3), LINE_HEIGHT,
    ID_PARA_def,   255,  0 },

  { INPUTBOX_TYPE_NUM,  DMPSTR_AGI_RND,   LX_AGIRND_CAP,   LY_AGIRND_CAP,
    LX_AGIRND_BOX,    LY_AGIRND_BOX,    CALC_NUMBOX_WIDTH(2), LINE_HEIGHT,
    ID_PARA_agi_rnd,  31,  0 },

  { INPUTBOX_TYPE_NUM,  DMPSTR_AGI_EXP,   LX_AGIEXP_CAP,   LY_AGIEXP_CAP,
    LX_AGIEXP_BOX,    LY_AGIEXP_BOX,    CALC_NUMBOX_WIDTH(3), LINE_HEIGHT,
    ID_PARA_agi_exp,  255,  0 },

  { INPUTBOX_TYPE_FIXVAL,  DMPSTR_AGI,   LX_AGIVAL_CAP,   LY_AGIVAL_CAP,
    LX_AGIVAL_BOX,    LY_AGIVAL_BOX,    CALC_NUMBOX_WIDTH(3), LINE_HEIGHT,
    ID_PARA_agi,   255,  0 },

  { INPUTBOX_TYPE_NUM,  DMPSTR_SPW_RND,   LX_SPWRND_CAP,   LY_SPWRND_CAP,
    LX_SPWRND_BOX,    LY_SPWRND_BOX,    CALC_NUMBOX_WIDTH(2), LINE_HEIGHT,
    ID_PARA_spepow_rnd,  31,  0 },

  { INPUTBOX_TYPE_NUM,  DMPSTR_SPW_EXP,   LX_SPWEXP_CAP,   LY_SPWEXP_CAP,
    LX_SPWEXP_BOX,    LY_SPWEXP_BOX,    CALC_NUMBOX_WIDTH(3), LINE_HEIGHT,
    ID_PARA_spepow_exp,  255,  0 },

  { INPUTBOX_TYPE_FIXVAL,  DMPSTR_SPW,   LX_SPWVAL_CAP,   LY_SPWVAL_CAP,
    LX_SPWVAL_BOX,    LY_SPWVAL_BOX,    CALC_NUMBOX_WIDTH(3), LINE_HEIGHT,
    ID_PARA_spepow,   255,  0 },

  { INPUTBOX_TYPE_NUM,  DMPSTR_SDF_RND,   LX_SDFRND_CAP,   LY_SDFRND_CAP,
    LX_SDFRND_BOX,    LY_SDFRND_BOX,    CALC_NUMBOX_WIDTH(2), LINE_HEIGHT,
    ID_PARA_spedef_rnd,  31,  0 },

  { INPUTBOX_TYPE_NUM,  DMPSTR_SDF_EXP,   LX_SDFEXP_CAP,   LY_SDFEXP_CAP,
    LX_SDFEXP_BOX,    LY_SDFEXP_BOX,    CALC_NUMBOX_WIDTH(3), LINE_HEIGHT,
    ID_PARA_spedef_exp,  255,  0 },

  { INPUTBOX_TYPE_FIXVAL,  DMPSTR_SDF,   LX_SDFVAL_CAP,   LY_SDFVAL_CAP,
    LX_SDFVAL_BOX,    LY_SDFVAL_BOX,    CALC_NUMBOX_WIDTH(3), LINE_HEIGHT,
    ID_PARA_spedef,   255,  0 },

  { INPUTBOX_TYPE_FIXSTR,  DMPSTR_TYPE,   LX_TYPE_CAP,   LY_TYPE_CAP,
    LX_TYPE_BOX,    LY_TYPE_BOX,    CALC_STRBOX_WIDTH(4), LINE_HEIGHT,
    ID_PARA_type1,  NARC_message_typename_dat,  0 },

  { INPUTBOX_TYPE_FIXSTR,  DMPSTR_NULL,      LX_TYPE_CAP,   LY_TYPE_CAP,
    LX_TYPE_BOX+CALC_STRBOX_WIDTH(4),    LY_TYPE_BOX,    CALC_STRBOX_WIDTH(4), LINE_HEIGHT,
    ID_PARA_type2,  NARC_message_typename_dat,  0 },

  { INPUTBOX_TYPE_BTN,  DMPSTR_NULL,      LX_TYPE_CAP,   LY_TYPE_CAP,
    LX_DEFAULT_BTN, LY_DEFAULT_BTN,    CALC_STRBOX_WIDTH(6), LINE_HEIGHT,
    ID_PARA_type2,  NARC_message_debug_makepoke_dat,  DMPSTR_DEFWAZA },

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
  PRINT_QUE*      printQue;
  PRINT_UTIL      printUtil;
  GFL_SKB*        skb;
  u32             pokeID;
  u32             pokeExpMin;
  u32             pokeExpMax;
  int             boxIdx;
  u32             boxValue[ INPUTBOX_ID_MAX ];
  u8              boxEnable[ INPUTBOX_ID_MAX ];

  POKEMON_PARAM*  dst;
  HEAPID  heapID;
  u8      seq;

  COMP_SKB_WORK   comp;
  NUMINPUT_WORK   numInput;
  GFL_SKB_SETUP   skbSetup;

}DMP_MAINWORK;

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static GFL_PROC_RESULT PROC_MAKEPOKE_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT PROC_MAKEPOKE_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT PROC_MAKEPOKE_Quit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static void UpdatePokeExpMinMax( DMP_MAINWORK* wk, const POKEMON_PARAM* pp );
static void setup_view( DMP_MAINWORK* wk );
static void cleanup_view( DMP_MAINWORK* wk );
static BOOL root_ctrl( DMP_MAINWORK* wk );
static int check_box_touch( DMP_MAINWORK* wk );
static BOOL is_hiragana( const STRBUF* buf );
static void update_dst( DMP_MAINWORK* wk );
static void print_caption( DMP_MAINWORK* wk, const INPUT_BOX_PARAM* p );
static PRINTSYS_LSB box_sub_get_bgcol( u8 input_type );
static void box_initwork( DMP_MAINWORK* wk );
static int box_getvalue( const DMP_MAINWORK* wk, u32 boxIdx );
static void box_setup( DMP_MAINWORK* wk, u32 boxID, const POKEMON_PARAM* pp );
static void box_update( DMP_MAINWORK* wk, u32 boxID, u32 value );
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
static void COMPSKB_Setup( COMP_SKB_WORK* wk, GFL_SKB* skb, STRBUF* buf, u32 msgDataID, HEAPID heapID );
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
      if( PP_Get(wk->dst, ID_PARA_monsno, NULL) == 0 ){
        PP_Setup( wk->dst, MONSNO_GORIDARUMA, 5, PTL_SETUP_ID_NOT_RARE );
      }
      wk->pokeID = PP_Get( wk->dst, ID_PARA_id_no, NULL );
      UpdatePokeExpMinMax( wk, wk->dst );
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
  wk->msgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_debug_makepoke_dat, wk->heapID );
  wk->strbuf = GFL_STR_CreateBuffer( STRBUF_LEN, wk->heapID );
  wk->printQue = PRINTSYS_QUE_Create( wk->heapID );
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
  enum {
    SEQ_DRAW_CAPTION = 0,
    SEQ_DRAW_BOX,
    SEQ_WAIT_CTRL,
    SEQ_INPUT_STR,
    SEQ_INPUT_NUM,
  };

  if( PRINT_UTIL_Trans(&wk->printUtil, wk->printQue) )
  {
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
        for(i=0; i<NELEMS(InputBoxParams); ++i){
          box_setup( wk, i, wk->dst );
        }
        wk->seq = SEQ_WAIT_CTRL;
      }
      break;
    case SEQ_WAIT_CTRL:
      wk->boxIdx = check_box_touch( wk );
      if( wk->boxIdx >= 0 )
      {
        const INPUT_BOX_PARAM* p = &InputBoxParams[ wk->boxIdx ];
        switch( p->type ){
        case INPUTBOX_TYPE_STR:
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
            COMPSKB_Setup( &wk->comp, wk->skb, wk->strbuf, p->arg, wk->heapID );
            wk->seq = SEQ_INPUT_STR;
          }
          break;
        case INPUTBOX_TYPE_NUM:
          NumInput_Setup( &wk->numInput, wk->strbuf, wk->win, wk->font,
              &wk->printUtil, wk->printQue, p, box_getvalue(wk, wk->boxIdx), wk );
          wk->seq = SEQ_INPUT_NUM;
          break;
        case INPUTBOX_TYPE_SWITCH:
          {
            u8 val = box_getvalue( wk, wk->boxIdx );
            val = !val;
            box_update( wk, wk->boxIdx, val );
          }
          break;
        case INPUTBOX_TYPE_BTN: // 今のところデフォルトワザセットボタンだけ
          {
            u32 i;
            update_dst( wk );
            PP_SetWazaDefault( wk->dst );
            for(i=0; i<PTL_WAZA_MAX; ++i)
            {
              box_setup( wk, INPUTBOX_ID_WAZA1+i, wk->dst );
              box_setup( wk, INPUTBOX_ID_PPMAX1+i, wk->dst );
            }
          }
          break;
        }
        break;
      }
      {
        u16 key = GFL_UI_KEY_GetTrg();
        if( key & PAD_BUTTON_START ){
          update_dst( wk );
          return TRUE;
        }
      }
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
    case SEQ_INPUT_NUM:
      if( NumInput_Main(&wk->numInput) )
      {
        u32 num = NumInput_GetNum( &wk->numInput );
        box_update( wk, wk->boxIdx, num );
        box_relation( wk, wk->boxIdx );
        wk->seq = SEQ_WAIT_CTRL;
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

        if( (p->type == INPUTBOX_TYPE_FIXVAL) ||  (p->type == INPUTBOX_TYPE_FIXSTR)){
          continue;
        }
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

static BOOL is_hiragana( const STRBUF* buf )
{
  const STRCODE* ptr;
  ptr = GFL_STR_GetStringCodePointer( buf );
  if( (*ptr >= 0x3041) && (*ptr <= 0x3093) ){
    return TRUE;
  }
  return FALSE;
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

  mons_no = box_getvalue( wk, INPUTBOX_ID_POKETYPE );
  tokusei = box_getvalue( wk, INPUTBOX_ID_TOKUSEI );
  sex = box_getvalue( wk, INPUTBOX_ID_SEX );
  level = box_getvalue( wk, INPUTBOX_ID_LEVEL );
  personal_rnd = POKETOOL_CalcPersonalRand( mons_no, PTL_FORM_NONE, sex );
  TAYA_Printf("個性乱数=%08x\n", personal_rnd);

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
    PP_SetupEx( wk->dst, mons_no, level, wk->pokeID, pow_val, personal_rnd );

    exp = box_getvalue( wk, INPUTBOX_ID_EXP );
    PP_Put( wk->dst, ID_PARA_exp, exp );
  }

  PP_Put( wk->dst, ID_PARA_speabino, tokusei );
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
  PP_Renew( wk->dst );
  PP_Put( wk->dst, ID_PARA_hp, box_getvalue(wk, INPUTBOX_ID_HPEDIT) );


  {
    u32 item = box_getvalue( wk, INPUTBOX_ID_ITEM );
    PP_Put( wk->dst, ID_PARA_item, item );
  }
  {
    u8  seikaku = box_getvalue( wk, INPUTBOX_ID_SEIKAKU );
    PP_Put( wk->dst, ID_PARA_seikaku, seikaku );
  }
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
  if( p->cap_strID != DMPSTR_NULL ){
    u32 ypos = p->cap_ypos + ((LINE_HEIGHT - GFL_FONT_GetLineHeight(wk->font)) / 2);
    GFL_MSG_GetString( wk->msgData, p->cap_strID, wk->strbuf );
    PRINT_UTIL_Print( &wk->printUtil, wk->printQue, p->cap_xpos, ypos, wk->strbuf, wk->font );
  }
}

static PRINTSYS_LSB box_sub_get_bgcol( u8 input_type )
{
  switch( input_type ){
  case INPUTBOX_TYPE_STR:     return PRINTSYS_LSB_Make( COLIDX_BLACK, 0, COLIDX_ORANGE_D );
  case INPUTBOX_TYPE_NUM:     return PRINTSYS_LSB_Make( COLIDX_BLACK, 0, COLIDX_BLUE_L );
  case INPUTBOX_TYPE_SWITCH:  return PRINTSYS_LSB_Make( COLIDX_BLACK, 0, COLIDX_GREEN_L );
  case INPUTBOX_TYPE_FIXVAL:  return PRINTSYS_LSB_Make( COLIDX_WHITE, 0, COLIDX_GRAY );
  case INPUTBOX_TYPE_FIXSTR:  return PRINTSYS_LSB_Make( COLIDX_WHITE, 0, COLIDX_GRAY );
  case INPUTBOX_TYPE_BTN:     return PRINTSYS_LSB_Make( COLIDX_BLACK, 0, COLIDX_RED_L );
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
 *  ボックスに値のセットと同時にPPも更新
 */
static void box_setup( DMP_MAINWORK* wk, u32 boxID, const POKEMON_PARAM* pp )
{
  const INPUT_BOX_PARAM* p = &InputBoxParams[ boxID ];

  u32 value;

  switch( p->paraID ){
  case ID_PARA_SEIKAKU:
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
  default:
    value = PP_Get( pp, p->paraID, NULL );
    break;
  }

  wk->boxEnable[boxID] = FALSE;

  switch( boxID ){
  case INPUTBOX_ID_SEX:
    if( personal_is_sex_fixed( pp ) ){
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
  if( boxID == INPUTBOX_ID_HPEDIT ){
    TAYA_Printf("Box-xpos=%d, width=%d, strWidth=%d, strX=%d\n", p->xpos, p->width, str_width, xpos);
  }
  str_height = GFL_FONT_GetLineHeight( wk->font );
  ypos = p->ypos;
  if( str_height < p->height ){
    ypos += (p->height - str_height) / 2;
  }

  PRINT_UTIL_PrintColor( &wk->printUtil, wk->printQue, xpos, ypos, wk->strbuf, wk->font, color );
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
  case INPUTBOX_TYPE_FIXSTR:
    {
      GFL_MSGDATA* msgdat = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, p->arg, GFL_HEAP_LOWID(wk->heapID) );
      GFL_MSG_GetString( msgdat, value, wk->strbuf );
      GFL_MSG_Delete( msgdat );
    }
    break;

  case INPUTBOX_TYPE_BTN:
    {
      GFL_MSGDATA* msgdat = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, p->arg, GFL_HEAP_LOWID(wk->heapID) );
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
      }
      STRTOOL_SetNumber( wk->strbuf, value, keta, STR_NUM_DISP_SPACE, STR_NUM_CODE_ZENKAKU );
    }
    break;

  case INPUTBOX_TYPE_FIXVAL:
    STRTOOL_SetNumber( wk->strbuf, value, calc_keta(p->arg), STR_NUM_DISP_SPACE, STR_NUM_CODE_ZENKAKU );
    break;
  case INPUTBOX_TYPE_SWITCH:
    if( p->arg2 == SWITCH_STR_DEFAULT ){
      GFL_MSG_GetString( wk->msgData, p->arg+value, wk->strbuf );
    }else if( p->arg2 == SWITCH_STR_TOKUSEI ){
      GFL_MSGDATA* msgdat = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, p->arg, GFL_HEAP_LOWID(wk->heapID) );
      value = personal_get_tokusei( wk->dst, value );
      GFL_MSG_GetString( msgdat, value, wk->strbuf );
      GFL_MSG_Delete( msgdat );
    }
    break;
  }
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
      PP_Renew( wk->dst );

      box_setup( wk, INPUTBOX_ID_TYPE1, wk->dst );
      box_setup( wk, INPUTBOX_ID_TYPE2, wk->dst );
      box_setup( wk, INPUTBOX_ID_TOKUSEI, wk->dst );
      box_setup( wk, INPUTBOX_ID_SEX, wk->dst );
      box_setup( wk, INPUTBOX_ID_HPVAL, wk->dst );
      box_setup( wk, INPUTBOX_ID_HPEDIT, wk->dst );
      box_setup( wk, INPUTBOX_ID_POWVAL, wk->dst );
      box_setup( wk, INPUTBOX_ID_DEFVAL, wk->dst );
      box_setup( wk, INPUTBOX_ID_AGIVAL, wk->dst );
      box_setup( wk, INPUTBOX_ID_SPWVAL, wk->dst );
      box_setup( wk, INPUTBOX_ID_SDFVAL, wk->dst );
      box_setup( wk, INPUTBOX_ID_EXP, wk->dst );

      UpdatePokeExpMinMax( wk, wk->dst );
    }
    break;
  case INPUTBOX_ID_SEIKAKU:
    {
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
      PP_Put( wk->dst, ID_PARA_pp_count1 + idx, cnt );
      box_setup( wk, INPUTBOX_ID_PPMAX1+idx, wk->dst );
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
  }
  wk->keta = calc_keta( wk->numMax );
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
          TAYA_Printf("↑　ケタあふれ\n");
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
          TAYA_Printf("↓　ケタあふれ\n");
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
static void COMPSKB_Setup( COMP_SKB_WORK* wk, GFL_SKB* skb, STRBUF* buf, u32 msgDataID, HEAPID heapID )
{
  wk->skb = skb;
  wk->msg = GFL_MSG_Create( GFL_MSG_LOAD_FAST, ARCID_MESSAGE, msgDataID, heapID );
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


