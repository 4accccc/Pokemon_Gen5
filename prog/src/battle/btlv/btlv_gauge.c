
//============================================================================================
/**
 * @file  btlv_gauge.c
 * @brief 戦闘ゲージ
 * @author  soga
 * @date  2009.07.15
 */
//============================================================================================

#include <gflib.h>

#include "poke_tool/monsno_def.h"
#include "print/printsys.h"
#include "font/font.naix"
#include "poke_tool/gauge_tool.h"
#include "app/app_menu_common.h"
#include "print/str_tool.h"

#include "btlv_effect.h"
#include "btlv_gauge.h"

#include "pm_define.h"
#include "sound/pm_sndsys.h"

#include "arc_def.h"
#include "battle/battgra_wb.naix"


//============================================================================================
/**
 *  定数宣言
 */
//============================================================================================

#define BTLV_GAUGE_CLWK_MAX ( BTLV_MCSS_POS_MAX )              //ゲージCLWKのMAX値
#define BTLV_GAUGE_CLUNIT_CLWK_MAX ( BTLV_MCSS_POS_MAX * 5 )   //CLUNITを生成するときのCLWKのMAX値

///1キャラの中に入っているゲージのドット数
#define BTLV_GAUGE_DOTTO      (8)

///HPバーのキャラ数
#define BTLV_GAUGE_HP_CHARMAX   (6)
///EXPバーのキャラ数
#define BTLV_GAUGE_EXP_CHARMAX  (10)

///HPゲージのドット数
#define BTLV_GAUGE_HP_DOTTOMAX  ( BTLV_GAUGE_DOTTO * BTLV_GAUGE_HP_CHARMAX )

///作業用ワークの初期値として使用
#define BTLV_GAUGE_HP_WORK_INIT_VALUE (-2147483648)

//数値の桁数
#define BTLV_GAUGE_NUM_MAX ( 3 )

//ゲージパーツ
enum
{
  GP_HPBAR_GREEN  = 0x00 * 0x20,      //HPバー緑
  GP_HPBAR_YELLOW = 0x09 * 0x20,      //HPバー黄
  GP_HPBAR_RED    = 0x12 * 0x20,      //HPバー赤

  GP_BALL         = 0x1b * 0x20,      //捕獲ボール

  GP_MALE_U       = 0x1c * 0x20,      //♂上
  GP_MALE_D       = 0x1d * 0x20,      //♂下
  GP_FEMALE_U     = 0x1e * 0x20,      //♀上
  GP_FEMALE_D     = 0x1f * 0x20,      //♀下

  GP_EXPBAR       = 0x20 * 0x20,      //EXPバー

  GP_NUM_0        = 0x29 * 0x20,      //数字０
  GP_NUM_1        = 0x2a * 0x20,      //数字１
  GP_SPACE        = 0x33 * 0x20,      //空白
  GP_LV           = 0x34 * 0x20,      //LVラベル

};

enum
{
  BTLV_GAUGE_POS_AA_X = 200,
  BTLV_GAUGE_POS_AA_Y = 120,

  BTLV_GAUGE_POS_BB_X = 56,
  BTLV_GAUGE_POS_BB_Y = 40,

  BTLV_GAUGE_POS_A_X = 192,
  BTLV_GAUGE_POS_A_Y = 104,

  BTLV_GAUGE_POS_B_X = 64,
  BTLV_GAUGE_POS_B_Y = 28,

  BTLV_GAUGE_POS_C_X = 196,
  BTLV_GAUGE_POS_C_Y = 132,

  BTLV_GAUGE_POS_D_X = 60,
  BTLV_GAUGE_POS_D_Y = 52,

  BTLV_GAUGE_POS_E_X = 200,
  BTLV_GAUGE_POS_E_Y = 160,

  BTLV_GAUGE_POS_F_X = 56,
  BTLV_GAUGE_POS_F_Y = 76,

  BTLV_GAUGE_HPNUM_X  = 16,
  BTLV_GAUGE_HPNUM_Y  = 13,
  BTLV_GAUGE_MINE_HP_X   = 16,
  BTLV_GAUGE_MINE_HP_Y   = 7,
  BTLV_GAUGE_ENEMY_HP_X  = 0,
  BTLV_GAUGE_ENEMY_HP_Y  = 7,
  BTLV_GAUGE_EXP_X  =  8,
  BTLV_GAUGE_EXP_Y  = 21,
  BTLV_STATUS_MINE_X     = -22,
  BTLV_STATUS_MINE_Y     = 8,
  BTLV_STATUS_ENEMY_X    = -38,
  BTLV_STATUS_ENEMY_Y    = 8,

  BTLV_GAUGE_BMP_SIZE_X = 8,
  BTLV_GAUGE_BMP_SIZE_Y = 2,
  BTLV_GAUGE_BMP_POS_X = 8,
  BTLV_GAUGE_BMP_POS_Y = 7,

  BTLV_GAUGE_NAME1U_CHARSTART = 0x02,
  BTLV_GAUGE_NAME1D_CHARSTART = 0x0a,
  BTLV_GAUGE_NAME2U_CHARSTART = 0x20,
  BTLV_GAUGE_NAME2D_CHARSTART = 0x28,

  BTLV_GAUGE_SEXU_CHARSTART = 0x22,
  BTLV_GAUGE_SEXD_CHARSTART = 0x2a,

  BTLV_GAUGE_HP_CHARSTART = 0x02,
  BTLV_GAUGE_NOWHP_CHARSTART = 0x00,
  BTLV_GAUGE_MAXHP_CHARSTART = 0x04,
  BTLV_GAUGE_LV_CHARSTART = 0x2b,
  BTLV_GAUGE_EXP_CHARSTART = 0x01,

  BTLV_GAUGE_TYPE_3vs3_YOFFSET_E = -16,   //3vs3時のゲージY方向オフセット（相手側）
  BTLV_GAUGE_TYPE_3vs3_YOFFSET_M = -24,   //3vs3時のゲージY方向オフセット（自分側）
};

typedef enum
{
  BTLV_GAUGE_REQ_HP = 0,
  BTLV_GAUGE_REQ_EXP,
}BTLV_GAUGE_REQ;

//============================================================================================
/**
 *  構造体宣言
 */
//============================================================================================

struct _BTLV_GAUGE_CLWK
{
  GFL_CLWK*     base_clwk;
  GFL_CLWK*     hpnum_clwk;
  GFL_CLWK*     hp_clwk;
  GFL_CLWK*     exp_clwk;
  GFL_CLWK*     status_clwk;

  //戦闘ゲージ本体
  u32           base_charID;
  u32           base_cellID;
  //HP数値
  u32           hpnum_charID;
  u32           hpnum_cellID;
  //HPゲージ
  u32           hp_charID;
  u32           hp_cellID;
  //EXPゲージ
  u32           exp_charID;
  u32           exp_cellID;

  int           gauge_type;
  int           gauge_dir;

  s32           hp;
  s32           hpmax;
  s32           hp_work;

  s32           exp;
  s32           exp_max;
  s32           next_exp_max;
  s32           exp_add;
  s32           exp_work;
  s32           damage;

  u8            level;
  u8            sex;
  u8            status;
  u8            getball;

  u32           hp_calc_req   :1;
  u32           exp_calc_req  :1;
  u32           level_up_req  :1;
  u32           gauge_enable  :1;
  u32           seq_no        :4;
  u32                         :24;
};

struct _BTLV_GAUGE_WORK
{
  GFL_CLUNIT*     clunit;
  ARCHANDLE*      handle;
  GFL_FONT*       font;
  void*           arc_data;
  const u8*       parts_address;

  u32             plttID;

  //状態異常アイコン
  u32             status_charID;
  u32             status_cellID;
  u32             status_plttID;

  BTLV_GAUGE_CLWK bgcl[ BTLV_GAUGE_CLWK_MAX ];

  u32             vanish_flag   :1;
  u32             bgm_fade_flag :1;
  u32             pinch_bgm_flag:1;
  u32                           :30;

  u32             now_bgm_no;

  HEAPID          heapID;
};

//============================================================================================
/**
 *  プロトタイプ宣言
 */
//============================================================================================

static  void  Gauge_InitCalcHP( BTLV_GAUGE_CLWK* bgcl, int value );
static  void  Gauge_CalcHP( BTLV_GAUGE_WORK* bgw, BTLV_GAUGE_CLWK* bgcl );
static  void  Gauge_InitCalcEXP( BTLV_GAUGE_CLWK* bgcl, int add_exp );
static  void  Gauge_CalcEXP( BTLV_GAUGE_WORK* bgw, BTLV_GAUGE_CLWK* bgcl );
static  s32   GaugeProc( s32 MaxHP, s32 NowHP, s32 beHP, s32 *HP_Work, u8 GaugeMax, u16 add_dec );
static  u8    PutGaugeProc( s32 MaxHP, s32 NowHP, s32 beHP, s32 *HP_Work, u8 *gauge_chr, u8 GaugeMax );
static  u32   DottoOffsetCalc( s32 nowHP, s32 beHP, s32 MaxHP, u8 GaugeMax );
static  void  PutNameOBJ( BTLV_GAUGE_WORK* bgw, BTLV_GAUGE_CLWK *bgcl, const BTL_POKEPARAM* bpp );
static  void  PutSexOBJ( BTLV_GAUGE_WORK* bgw, BTLV_GAUGE_CLWK *bgcl );
static  void  PutGaugeOBJ( BTLV_GAUGE_WORK* bgw, BTLV_GAUGE_CLWK *bgcl, BTLV_GAUGE_REQ req );
static  void  PutHPNumOBJ( BTLV_GAUGE_WORK* bgw, BTLV_GAUGE_CLWK *bgcl, s32 nowHP );
static  void  PutLVNumOBJ( BTLV_GAUGE_WORK* bgw, BTLV_GAUGE_CLWK *bgcl );
static  void  Gauge_LevelUp( BTLV_GAUGE_WORK* bgw, BTLV_GAUGE_CLWK* bgcl );

static  void  pinch_bgm_check( BTLV_GAUGE_WORK* bgw );

//============================================================================================
/**
 *  @brief  システム初期化
 *
 *  @param[in] heapID    ヒープID
 *
 *  @retval  bgw  BTLV_GAUGE_WORK管理構造体へのポインタ
 */
//============================================================================================
BTLV_GAUGE_WORK*  BTLV_GAUGE_Init( GFL_FONT* fontHandle, HEAPID heapID )
{
  BTLV_GAUGE_WORK *bgw = GFL_HEAP_AllocClearMemory( heapID, sizeof( BTLV_GAUGE_WORK ) );

  bgw->heapID = heapID;
  bgw->handle = GFL_ARC_OpenDataHandle( ARCID_BATTGRA, bgw->heapID );

  bgw->clunit = GFL_CLACT_UNIT_Create( BTLV_GAUGE_CLUNIT_CLWK_MAX, 0, bgw->heapID );

  //共通パレット読み込み
  bgw->plttID = GFL_CLGRP_PLTT_Register( bgw->handle, NARC_battgra_wb_gauge_NCLR,
                                         CLSYS_DRAW_MAIN, BTLV_OBJ_PLTT_HP_GAUGE, bgw->heapID );
  PaletteWorkSet_VramCopy( BTLV_EFFECT_GetPfd(), FADE_MAIN_OBJ,
                           GFL_CLGRP_PLTT_GetAddr( bgw->plttID, CLSYS_DRAW_MAIN ) / 2, 0x20 * 1 );
  {
    ARCHANDLE*  handle = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), bgw->heapID );

    bgw->status_charID = GFL_CLGRP_CGR_Register( handle, APP_COMMON_GetStatusIconCharArcIdx(),
                                                 FALSE, CLSYS_DRAW_MAIN, bgw->heapID );
    bgw->status_cellID = GFL_CLGRP_CELLANIM_Register( handle,
                                                      APP_COMMON_GetStatusIconCellArcIdx( APP_COMMON_MAPPING_64K ),
                                                      APP_COMMON_GetStatusIconAnimeArcIdx( APP_COMMON_MAPPING_64K ),
                                                      bgw->heapID );
    bgw->status_plttID = GFL_CLGRP_PLTT_Register( handle, APP_COMMON_GetStatusIconPltArcIdx(),
                                                  CLSYS_DRAW_MAIN, BTLV_OBJ_PLTT_STATUS_ICON, bgw->heapID );
    PaletteWorkSet_VramCopy( BTLV_EFFECT_GetPfd(), FADE_MAIN_OBJ,
                             GFL_CLGRP_PLTT_GetAddr( bgw->status_plttID, CLSYS_DRAW_MAIN ) / 2, 0x20 * 1 );
    GFL_ARC_CloseDataHandle( handle );
  }


  //パーツデータ読み込み
  {
    NNSG2dCharacterData *char_data;

    bgw->arc_data = GFL_ARCHDL_UTIL_LoadOBJCharacter( bgw->handle, NARC_battgra_wb_gauge_parts_NCGR, NULL,
                                                      &char_data, bgw->heapID );
    bgw->parts_address = ( u8 * )char_data->pRawData;
  }

  bgw->font = fontHandle;

  //今鳴っているBGMを保存
  bgw->now_bgm_no = PMSND_GetBGMsoundNo();

  return bgw;
}

//============================================================================================
/**
 *  @brief  システム終了
 *
 *  @param[in] bgw  BTLV_GAUGE_WORK管理構造体へのポインタ
 */
//============================================================================================
void  BTLV_GAUGE_Exit( BTLV_GAUGE_WORK *bgw )
{
  BtlvMcssPos pos;

  for( pos = 0 ;  pos < BTLV_GAUGE_CLWK_MAX ; pos++ )
  {
    BTLV_GAUGE_Del( bgw, pos );
  }
  if( bgw->pinch_bgm_flag )
  {
    PMSND_PopBGM();
  }
  GFL_CLGRP_PLTT_Release( bgw->plttID );
  GFL_CLGRP_CGR_Release( bgw->status_charID );
  GFL_CLGRP_CELLANIM_Release( bgw->status_cellID );
  GFL_CLGRP_PLTT_Release( bgw->status_plttID );
  GFL_CLACT_UNIT_Delete( bgw->clunit );
  GFL_HEAP_FreeMemory( bgw->arc_data );
  GFL_FONT_Delete( bgw->font );
  GFL_ARC_CloseDataHandle( bgw->handle );
  GFL_HEAP_FreeMemory( bgw );
}

//============================================================================================
/**
 *  @brief  システムメイン
 *
 *  @param[in] bgw  BTLV_GAUGE_WORK管理構造体へのポインタ
 */
//============================================================================================
void  BTLV_GAUGE_Main( BTLV_GAUGE_WORK *bgw )
{
  int i;

  for( i = 0 ; i < BTLV_GAUGE_CLWK_MAX ; i++ )
  {
    if( bgw->bgcl[ i ].hp_calc_req )
    {
      Gauge_CalcHP( bgw, &bgw->bgcl[ i ] );
    }
    if( bgw->bgcl[ i ].exp_calc_req )
    {
      PMSND_PlaySE( SEQ_SE_EXP );
      Gauge_CalcEXP( bgw, &bgw->bgcl[ i ] );
      if( bgw->bgcl[ i ].exp_calc_req == 0 )
      {
        PMSND_StopSE();
      }
    }
    if( bgw->bgcl[ i ].level_up_req )
    {
      Gauge_LevelUp( bgw, &bgw->bgcl[ i ] );
    }
  }
  //ピンチBGM再生チェック
  pinch_bgm_check( bgw );
}

//============================================================================================
/**
 *  @brief  ゲージ生成
 *
 *  @param[in] bgw  BTLV_GAUGE_WORK管理構造体へのポインタ
 *  @param[in] type ゲージタイプ
 *  @param[in] pos  立ち位置
 */
//============================================================================================
void  BTLV_GAUGE_Add( BTLV_GAUGE_WORK *bgw, const BTL_POKEPARAM* bpp, BTLV_GAUGE_TYPE type, BtlvMcssPos pos )
{
  u32 arcdatid_char;
  u32 arcdatid_cell;
  u32 arcdatid_anm;

  GF_ASSERT( bgw->bgcl[ pos ].base_clwk == NULL );

  bgw->bgcl[ pos ].gauge_type = type;
  bgw->bgcl[ pos ].gauge_dir = pos & 1;

  switch( type ){
  default:
  case BTLV_GAUGE_TYPE_1vs1:
  case BTLV_GAUGE_TYPE_2vs2:
    arcdatid_char = ( pos & 1 ) ? NARC_battgra_wb_gauge_1vs1_e_NCGR : NARC_battgra_wb_gauge_1vs1_m_NCGR;
    arcdatid_cell = ( pos & 1 ) ? NARC_battgra_wb_gauge_1vs1_e_NCER : NARC_battgra_wb_gauge_1vs1_m_NCER;
    arcdatid_anm  = ( pos & 1 ) ? NARC_battgra_wb_gauge_1vs1_e_NANR : NARC_battgra_wb_gauge_1vs1_m_NANR;
    break;
  case BTLV_GAUGE_TYPE_3vs3:
    arcdatid_char = ( pos & 1 ) ? NARC_battgra_wb_gauge_2vs2_e_NCGR : NARC_battgra_wb_gauge_2vs2_m_NCGR;
    arcdatid_cell = ( pos & 1 ) ? NARC_battgra_wb_gauge_2vs2_e_NCER : NARC_battgra_wb_gauge_2vs2_m_NCER;
    arcdatid_anm  = ( pos & 1 ) ? NARC_battgra_wb_gauge_2vs2_e_NANR : NARC_battgra_wb_gauge_2vs2_m_NANR;
    break;
  case BTLV_GAUGE_TYPE_SAFARI:
    break;
  }

  //リソース読み込み
  bgw->bgcl[ pos ].base_charID = GFL_CLGRP_CGR_Register( bgw->handle, arcdatid_char, FALSE, CLSYS_DRAW_MAIN, bgw->heapID );
  bgw->bgcl[ pos ].base_cellID = GFL_CLGRP_CELLANIM_Register( bgw->handle, arcdatid_cell, arcdatid_anm, bgw->heapID );

  bgw->bgcl[ pos ].hpnum_charID = GFL_CLGRP_CGR_Register( bgw->handle, NARC_battgra_wb_gauge_num_NCGR, FALSE,
                                                       CLSYS_DRAW_MAIN, bgw->heapID );
  bgw->bgcl[ pos ].hpnum_cellID = GFL_CLGRP_CELLANIM_Register( bgw->handle, NARC_battgra_wb_gauge_num_NCER,
                                                            NARC_battgra_wb_gauge_num_NANR, bgw->heapID );

  bgw->bgcl[ pos ].hp_charID = GFL_CLGRP_CGR_Register( bgw->handle, NARC_battgra_wb_gauge_hp_NCGR, FALSE,
                                                       CLSYS_DRAW_MAIN, bgw->heapID );
  bgw->bgcl[ pos ].hp_cellID = GFL_CLGRP_CELLANIM_Register( bgw->handle, NARC_battgra_wb_gauge_hp_NCER,
                                                            NARC_battgra_wb_gauge_hp_NANR, bgw->heapID );

  if( ( pos & 1 ) == 0 )
  {
    bgw->bgcl[ pos ].exp_charID = GFL_CLGRP_CGR_Register( bgw->handle, NARC_battgra_wb_gauge_exp_NCGR, FALSE,
                                                          CLSYS_DRAW_MAIN, bgw->heapID );
    bgw->bgcl[ pos ].exp_cellID = GFL_CLGRP_CELLANIM_Register( bgw->handle, NARC_battgra_wb_gauge_exp_NCER,
                                                               NARC_battgra_wb_gauge_exp_NANR, bgw->heapID );
  }

  //CLWK
  {
    GFL_CLWK_DATA gauge = {
      0, 0,     //x, y
      0, 0, 0,  //アニメ番号、優先順位、BGプライオリティ
    };
    GFL_CLACTPOS  gauge_pos[]={
      { BTLV_GAUGE_POS_AA_X, BTLV_GAUGE_POS_AA_Y },
      { BTLV_GAUGE_POS_BB_X, BTLV_GAUGE_POS_BB_Y },
      { BTLV_GAUGE_POS_A_X, BTLV_GAUGE_POS_A_Y },
      { BTLV_GAUGE_POS_B_X, BTLV_GAUGE_POS_B_Y },
      { BTLV_GAUGE_POS_C_X, BTLV_GAUGE_POS_C_Y },
      { BTLV_GAUGE_POS_D_X, BTLV_GAUGE_POS_D_Y },
      { BTLV_GAUGE_POS_E_X, BTLV_GAUGE_POS_E_Y },
      { BTLV_GAUGE_POS_F_X, BTLV_GAUGE_POS_F_Y },
    };
    gauge.softpri = 1;
    bgw->bgcl[ pos ].base_clwk = GFL_CLACT_WK_Create( bgw->clunit,
                                                      bgw->bgcl[ pos ].base_charID, bgw->plttID, bgw->bgcl[ pos ].base_cellID,
                                                      &gauge, CLSYS_DEFREND_MAIN, bgw->heapID );
    gauge.softpri = 0;
    bgw->bgcl[ pos ].hpnum_clwk = GFL_CLACT_WK_Create( bgw->clunit,
                                                       bgw->bgcl[ pos ].hpnum_charID, bgw->plttID,
                                                       bgw->bgcl[ pos ].hpnum_cellID,
                                                       &gauge, CLSYS_DEFREND_MAIN, bgw->heapID );
    bgw->bgcl[ pos ].hp_clwk = GFL_CLACT_WK_Create( bgw->clunit,
                                                    bgw->bgcl[ pos ].hp_charID, bgw->plttID, bgw->bgcl[ pos ].hp_cellID,
                                                    &gauge, CLSYS_DEFREND_MAIN, bgw->heapID );
    bgw->bgcl[ pos ].status_clwk = GFL_CLACT_WK_Create( bgw->clunit,
                                                        bgw->status_charID, bgw->status_plttID, bgw->status_cellID,
                                                        &gauge, CLSYS_DEFREND_MAIN, bgw->heapID );
    GFL_CLACT_WK_SetAutoAnmFlag( bgw->bgcl[ pos ].status_clwk, TRUE );
    BTLV_GAUGE_SetStatus( bgw, BPP_GetPokeSick( bpp ), pos );
    if( ( ( pos & 1 ) == 0 ) && ( bgw->bgcl[ pos ].gauge_type != BTLV_GAUGE_TYPE_3vs3 ) )
    {
      bgw->bgcl[ pos ].exp_clwk = GFL_CLACT_WK_Create( bgw->clunit,
                                                       bgw->bgcl[ pos ].exp_charID, bgw->plttID, bgw->bgcl[ pos ].exp_cellID,
                                                       &gauge, CLSYS_DEFREND_MAIN, bgw->heapID );
    }
    BTLV_GAUGE_SetPos( bgw, pos, gauge_pos[ pos ].x, gauge_pos[ pos ].y );
  }

  {
    bgw->bgcl[ pos ].hp       = BPP_GetValue( bpp, BPP_HP );
    BTL_Printf("ここでＨＰは%d\n", bgw->bgcl[ pos ].hp);
    bgw->bgcl[ pos ].hpmax    = BPP_GetValue( bpp, BPP_MAX_HP );
    bgw->bgcl[ pos ].hp_work  = BTLV_GAUGE_HP_WORK_INIT_VALUE;
    bgw->bgcl[ pos ].damage   = 0;

    bgw->bgcl[ pos ].level    = BPP_GetValue( bpp, BPP_LEVEL );

    if( bgw->bgcl[ pos ].level < 100 )
    {
      int min_exp = POKETOOL_GetMinExp( BPP_GetMonsNo( bpp ),
                                        BPP_GetValue( bpp, BPP_FORM ),
                                        BPP_GetValue( bpp, BPP_LEVEL ) );

      bgw->bgcl[ pos ].exp      = BPP_GetValue( bpp, BPP_EXP ) - min_exp;
      bgw->bgcl[ pos ].exp_max  = POKETOOL_GetMinExp( BPP_GetMonsNo( bpp ),
                                                      BPP_GetValue( bpp, BPP_FORM ),
                                                      BPP_GetValue( bpp, BPP_LEVEL ) + 1 ) -
                                  min_exp;
    }
    else
    {
      bgw->bgcl[ pos ].exp      = 0;
      bgw->bgcl[ pos ].exp_max  = 0;
    }

    bgw->bgcl[ pos ].exp_work = BTLV_GAUGE_HP_WORK_INIT_VALUE;
    bgw->bgcl[ pos ].exp_add  = 0;

    bgw->bgcl[ pos ].status   = 0;
    bgw->bgcl[ pos ].getball  = 0;

    {
      u16 mons_no = BPP_GetMonsNo( bpp );

      if( ( mons_no != MONSNO_NIDORAN_M ) && ( mons_no != MONSNO_NIDORAN_F ) )
      {
        bgw->bgcl[ pos ].sex  = BPP_GetValue( bpp, BPP_SEX );
      }
      else
      {
        bgw->bgcl[ pos ].sex  = PTL_SEX_UNKNOWN;
      }
    }

    GaugeProc( bgw->bgcl[ pos ].hpmax, bgw->bgcl[ pos ].hp, 0, &bgw->bgcl[ pos ].hp_work, BTLV_GAUGE_HP_CHARMAX, 1 );
    PutGaugeOBJ( bgw, &bgw->bgcl[ pos ], BTLV_GAUGE_REQ_HP );
    if( bgw->bgcl[ pos ].exp_clwk ){
      GaugeProc( bgw->bgcl[ pos ].exp_max, bgw->bgcl[ pos ].exp, 0, &bgw->bgcl[ pos ].exp_work, BTLV_GAUGE_EXP_CHARMAX, 1 );
      PutGaugeOBJ( bgw, &bgw->bgcl[ pos ], BTLV_GAUGE_REQ_EXP );
    }
    PutNameOBJ( bgw, &bgw->bgcl[ pos ], bpp );
    PutSexOBJ( bgw, &bgw->bgcl[ pos ] );
    PutHPNumOBJ( bgw, &bgw->bgcl[ pos ], bgw->bgcl[ pos ].hp );
    PutLVNumOBJ( bgw, &bgw->bgcl[ pos ] );
  }

  bgw->bgcl[ pos ].gauge_enable = 1;
}

//============================================================================================
/**
 *  @brief  ゲージ破棄
 *
 *  @param[in] bgw  BTLV_GAUGE_WORK管理構造体へのポインタ
 *  @param[in] pos  立ち位置
 */
//============================================================================================
void  BTLV_GAUGE_Del( BTLV_GAUGE_WORK *bgw, BtlvMcssPos pos )
{
  if( bgw->bgcl[ pos ].base_clwk )
  {
    GFL_CLGRP_CGR_Release( bgw->bgcl[ pos ].base_charID );
    GFL_CLGRP_CGR_Release( bgw->bgcl[ pos ].hpnum_charID );
    GFL_CLGRP_CGR_Release( bgw->bgcl[ pos ].hp_charID );

    GFL_CLGRP_CELLANIM_Release( bgw->bgcl[ pos ].base_cellID );
    GFL_CLGRP_CELLANIM_Release( bgw->bgcl[ pos ].hpnum_cellID );
    GFL_CLGRP_CELLANIM_Release( bgw->bgcl[ pos ].hp_cellID );

    GFL_CLACT_WK_Remove( bgw->bgcl[ pos ].base_clwk );
    GFL_CLACT_WK_Remove( bgw->bgcl[ pos ].hpnum_clwk );
    GFL_CLACT_WK_Remove( bgw->bgcl[ pos ].hp_clwk );
    GFL_CLACT_WK_Remove( bgw->bgcl[ pos ].status_clwk );
    bgw->bgcl[ pos ].base_clwk    = NULL;
    bgw->bgcl[ pos ].hpnum_clwk   = NULL;
    bgw->bgcl[ pos ].hp_clwk      = NULL;
    bgw->bgcl[ pos ].status_clwk  = NULL;

    if( bgw->bgcl[ pos ].exp_clwk )
    {
      GFL_CLGRP_CGR_Release( bgw->bgcl[ pos ].exp_charID );
      GFL_CLGRP_CELLANIM_Release( bgw->bgcl[ pos ].exp_cellID );
      GFL_CLACT_WK_Remove( bgw->bgcl[ pos ].exp_clwk );
      bgw->bgcl[ pos ].exp_clwk = NULL;
    }
  }
}

//============================================================================================
/**
 *  @brief  ゲージ位置セット
 *
 *  @param[in] bgw   BTLV_GAUGE_WORK管理構造体へのポインタ
 *  @param[in] pos   立ち位置
 *  @param[in] pos_x セットするX座標
 *  @param[in] pos_y セットするY座標
 */
//============================================================================================
void  BTLV_GAUGE_SetPos( BTLV_GAUGE_WORK* bgw, BtlvMcssPos pos, int pos_x, int pos_y )
{
  GFL_CLACTPOS  cl_pos;
  GFL_CLACTPOS  hp_pos_ofs[]={
    { BTLV_GAUGE_MINE_HP_X, BTLV_GAUGE_MINE_HP_Y },
    { BTLV_GAUGE_ENEMY_HP_X, BTLV_GAUGE_ENEMY_HP_Y },
  };
  GFL_CLACTPOS  status_pos_ofs[]={
    { BTLV_STATUS_MINE_X, BTLV_STATUS_MINE_Y },
    { BTLV_STATUS_ENEMY_X, BTLV_STATUS_ENEMY_Y },
  };

  if( bgw->bgcl[ pos ].gauge_type == BTLV_GAUGE_TYPE_3vs3 )
  {
    int pos_ofs_y[]={
      0, 0,
      BTLV_GAUGE_TYPE_3vs3_YOFFSET_M + 8,
      BTLV_GAUGE_TYPE_3vs3_YOFFSET_E,
      BTLV_GAUGE_TYPE_3vs3_YOFFSET_M + 4,
      BTLV_GAUGE_TYPE_3vs3_YOFFSET_E,
      BTLV_GAUGE_TYPE_3vs3_YOFFSET_M,
      BTLV_GAUGE_TYPE_3vs3_YOFFSET_E,
    };
    pos_y += pos_ofs_y[ pos ];
  }
  cl_pos.x = pos_x;
  cl_pos.y = pos_y;
  GFL_CLACT_WK_SetPos( bgw->bgcl[ pos ].base_clwk, &cl_pos, CLSYS_DEFREND_MAIN );

  if( ( ( pos & 1 ) == 0 ) && ( bgw->bgcl[ pos ].gauge_type == BTLV_GAUGE_TYPE_1vs1 ) )
  {
    cl_pos.x = pos_x + BTLV_GAUGE_HPNUM_X;
    cl_pos.y = pos_y + BTLV_GAUGE_HPNUM_Y;
    GFL_CLACT_WK_SetPos( bgw->bgcl[ pos ].hpnum_clwk, &cl_pos, CLSYS_DEFREND_MAIN );
  }

  cl_pos.x = pos_x + hp_pos_ofs[ pos & 1 ].x;
  cl_pos.y = pos_y + hp_pos_ofs[ pos & 1 ].y;
  GFL_CLACT_WK_SetPos( bgw->bgcl[ pos ].hp_clwk, &cl_pos, CLSYS_DEFREND_MAIN );

  cl_pos.x = pos_x + status_pos_ofs[ pos & 1 ].x;
  cl_pos.y = pos_y + status_pos_ofs[ pos & 1 ].y;
  GFL_CLACT_WK_SetPos( bgw->bgcl[ pos ].status_clwk, &cl_pos, CLSYS_DEFREND_MAIN );

  if( bgw->bgcl[ pos ].exp_clwk )
  {
    cl_pos.x = pos_x + BTLV_GAUGE_EXP_X;
    cl_pos.y = pos_y + BTLV_GAUGE_EXP_Y;
    GFL_CLACT_WK_SetPos( bgw->bgcl[ pos ].exp_clwk, &cl_pos, CLSYS_DEFREND_MAIN );
  }
}

//============================================================================================
/**
 *  @brief  HPゲージ計算
 *
 *  @param[in] bgw    BTLV_GAUGE_WORK管理構造体へのポインタ
 *  @param[in] pos    立ち位置
 *  @param[in] value  最終的に到達する値
 */
//============================================================================================
void  BTLV_GAUGE_CalcHP( BTLV_GAUGE_WORK *bgw, BtlvMcssPos pos, int value )
{
  Gauge_InitCalcHP( &bgw->bgcl[ pos ], value );
}

//============================================================================================
/**
 *  @brief  EXPゲージ計算
 *
 *  @param[in] bgw    BTLV_GAUGE_WORK管理構造体へのポインタ
 *  @param[in] pos    立ち位置
 *  @param[in] value  加算する経験値
 */
//============================================================================================
void  BTLV_GAUGE_CalcEXP( BTLV_GAUGE_WORK *bgw, BtlvMcssPos pos, int value )
{
  Gauge_InitCalcEXP( &bgw->bgcl[ pos ], value );
}

//============================================================================================
/**
 *  @brief  EXPゲージ計算（レベルアップ時）
 *
 *  @param[in] bgw    BTLV_GAUGE_WORK管理構造体へのポインタ
 *  @param[in] bpp    計算するポケモンパラメータ
 *  @param[in] pos    立ち位置
 */
//============================================================================================
void  BTLV_GAUGE_CalcEXPLevelUp( BTLV_GAUGE_WORK *bgw, const BTL_POKEPARAM* bpp, BtlvMcssPos pos )
{
  int value = bgw->bgcl[ pos ].exp_max - bgw->bgcl[ pos ].exp;

  bgw->bgcl[ pos ].hp       = BPP_GetValue( bpp, BPP_HP );
  bgw->bgcl[ pos ].hpmax    = BPP_GetValue( bpp, BPP_MAX_HP );
  bgw->bgcl[ pos ].hp_work  = BTLV_GAUGE_HP_WORK_INIT_VALUE;
  bgw->bgcl[ pos ].damage   = 0;

  bgw->bgcl[ pos ].level    = BPP_GetValue( bpp, BPP_LEVEL );

  bgw->bgcl[ pos ].next_exp_max  = POKETOOL_GetMinExp( BPP_GetMonsNo( bpp ),
                                                       BPP_GetValue( bpp, BPP_FORM ),
                                                       BPP_GetValue( bpp, BPP_LEVEL ) + 1 ) -
                                   POKETOOL_GetMinExp( BPP_GetMonsNo( bpp ),
                                                       BPP_GetValue( bpp, BPP_FORM ),
                                                       BPP_GetValue( bpp, BPP_LEVEL ) );

  bgw->bgcl[ pos ].seq_no = 0;
  bgw->bgcl[ pos ].level_up_req = 1;

  Gauge_InitCalcEXP( &bgw->bgcl[ pos ], value );
}

//============================================================================================
/**
 *  @brief  ゲージ計算中かチェック
 *
 *  @param[in] bgw    BTLV_GAUGE_WORK管理構造体へのポインタ
 *
 *  @retval FALSE:計算終了  TRUE:計算中
 */
//============================================================================================
BOOL  BTLV_GAUGE_CheckExecute( BTLV_GAUGE_WORK *bgw )
{
  int i;

  for( i = 0 ; i < BTLV_GAUGE_CLWK_MAX ; i++ )
  {
    if( ( bgw->bgcl[ i ].hp_calc_req ) || ( bgw->bgcl[ i ].exp_calc_req ) || ( bgw->bgcl[ i ].level_up_req ) )
    {
      return TRUE;
    }
  }
  return FALSE;
}

//============================================================================================
/**
 *  @brief  ゲージ表示/非表示
 *
 *  @param[in] bgw    BTLV_GAUGE_WORK管理構造体へのポインタ
 *  @param[in] on_off TRUE:表示 FALSE:非表示
 */
//============================================================================================
void  BTLV_GAUGE_SetDrawEnable( BTLV_GAUGE_WORK* bgw, BOOL on_off )
{
  GFL_CLACT_UNIT_SetDrawEnable( bgw->clunit, on_off );
}

//--------------------------------------------------------------
/**
 * @brief   HPバーの計算を開始する前に作業用ワーク等の初期化を行う
 *
 * @param[in] bgw   ゲージワークへのポインタ
 * @param[in] value 最終的に到達する値
 *
 * Gauge_CalcHPを実行する前に必ず呼び出して置く必要があります。
 */
//--------------------------------------------------------------
static  void  Gauge_InitCalcHP( BTLV_GAUGE_CLWK* bgcl, int value )
{
  bgcl->hp_work = BTLV_GAUGE_HP_WORK_INIT_VALUE;

  bgcl->damage = bgcl->hp - value;
  if( bgcl->hp < 0 ){
    bgcl->hp = 0;
  }
  if( bgcl->hp > bgcl->hpmax ){
    bgcl->hp = bgcl->hpmax;
  }
  bgcl->hp_calc_req = 1;
}

//--------------------------------------------------------------
/**
 * @brief   HPバーの計算と書き込みを行う
 *
 * @param   bgw   ゲージワークへのポインタ
 *
 * Gauge_InitCalcHPを先に呼んである必要があります。
 */
//--------------------------------------------------------------
static  void  Gauge_CalcHP( BTLV_GAUGE_WORK* bgw, BTLV_GAUGE_CLWK* bgcl )
{
  s32 calc_hp;

  calc_hp = GaugeProc( bgcl->hpmax, bgcl->hp, bgcl->damage, &bgcl->hp_work, BTLV_GAUGE_HP_CHARMAX, 1 );
  PutGaugeOBJ( bgw, bgcl, BTLV_GAUGE_REQ_HP );
  if( calc_hp == -1 ){
    //計算終了時にゲージワークのhpパラメータを最新の値(ダメージ計算後)で更新しておく
    bgcl->hp -= bgcl->damage;
    PutHPNumOBJ( bgw, bgcl, bgcl->hp );
    bgcl->hp_calc_req = 0;
    bgcl->hp_work = 0;
  }
  else{
    PutHPNumOBJ( bgw, bgcl, calc_hp );
  }
}

//--------------------------------------------------------------
/**
 * @brief   EXPバーの計算を開始する前に作業用ワーク等の初期化を行う
 *
 * @param   bgw       ゲージワークへのポインタ
 * @param   add_exp   加算するEXP値
 *
 * Gauge_CalcEXPを実行する前に必ず呼び出して置く必要があります。
 */
//--------------------------------------------------------------
static  void  Gauge_InitCalcEXP( BTLV_GAUGE_CLWK* bgcl, int add_exp )
{
  bgcl->exp_work = BTLV_GAUGE_HP_WORK_INIT_VALUE;

  if( bgcl->exp + add_exp < 0 )
  {
    add_exp -= bgcl->exp + add_exp;
  }
  if( bgcl->exp + add_exp > bgcl->exp_max)
  {
    add_exp -= ( bgcl->exp + add_exp ) - bgcl->exp_max;
  }

  //-- 旧ゲージ計算ルーチンに合わせるため、正負の逆転などを行う --//
   bgcl->exp_add = -add_exp;
  if( bgcl->exp < 0 )
  {
    bgcl->exp = 0;
  }
  if( bgcl->exp > bgcl->exp_max )
  {
    bgcl->exp = bgcl->exp_max;
  }
  bgcl->exp_calc_req = 1;
}

//--------------------------------------------------------------
/**
 * @brief   EXPバーの計算と書き込みを行う
 *
 * @param   bgw   ゲージワークへのポインタ
 *
 * @retval  -1=終了。　-1以外=計算途中の数字(画面に表示する数値)
 *
 * Gauge_InitCalcEXPを先に呼んである必要があります。
 */
//--------------------------------------------------------------
static  void  Gauge_CalcEXP( BTLV_GAUGE_WORK* bgw, BTLV_GAUGE_CLWK* bgcl )
{
  s32 calc_exp;
  s32 dotto_offset;

  dotto_offset = DottoOffsetCalc( bgcl->exp, bgcl->exp_add, bgcl->exp_max, BTLV_GAUGE_EXP_CHARMAX );
  if( dotto_offset == 0 )
  {
    dotto_offset = 1;
  }
  dotto_offset = GFL_STD_Abs( bgcl->exp_add / dotto_offset );
  calc_exp = GaugeProc( bgcl->exp_max, bgcl->exp, bgcl->exp_add, &bgcl->exp_work, BTLV_GAUGE_EXP_CHARMAX, dotto_offset );
  PutGaugeOBJ( bgw, bgcl, BTLV_GAUGE_REQ_EXP );
  if(calc_exp == -1)
  {
    //計算終了時にゲージワークのexpパラメータを最新の値(ダメージ計算後)で更新しておく
    bgcl->exp -= bgcl->exp_add;
    bgcl->exp_calc_req = 0;
    bgcl->exp_work = 0;
  }
}


//--------------------------------------------------------------
/**
 *  @brief   ゲージバー計算
 *
 *  @param   MaxHP    最大HP
 *  @param   NowHP    現在HP(計算前のHP)
 *  @param   beHP   ダメージ(-でダメージ、+なら回復)
 *  @param   HP_Work    作業用ワーク
 *  @param   GaugeMax ゲージのキャラクタ数
 *  @param   add_dec    ｹﾞｰｼﾞを加減算していく値 GaugeDottoがGaugeMax以下の場合は無視する
 *
 *  @retval  -1 = 終了  それ以外：計算途中の数字(画面に出す数値)
 */
//--------------------------------------------------------------
static s32 GaugeProc(s32 MaxHP, s32 NowHP, s32 beHP, s32 *HP_Work, u8 GaugeMax, u16 add_dec)
{
  s32 endHP;
  s32 ret;
  u8  GaugeMaxDot;
  s32 add_hp;

  GaugeMaxDot = GaugeMax * BTLV_GAUGE_DOTTO;

  //-- HP_WorkがBTLV_GAUGE_HP_WORK_INIT_VALUEの時は初回なのでワークを初期化 --//
  if( *HP_Work == BTLV_GAUGE_HP_WORK_INIT_VALUE ){
    if( MaxHP < GaugeMaxDot ){
      *HP_Work = NowHP << 8;
    }
    else{
      *HP_Work = NowHP;
    }
  }

  //-- 終了ﾁｪｯｸ --//
  endHP = NowHP - beHP;
  if( endHP < 0 )
  {
    endHP = 0;
  }
  else if( endHP > MaxHP )
  {
    endHP = MaxHP;
  }

  if( MaxHP < GaugeMaxDot )           //↓少数部分もﾁｪｯｸ
  {
    if( endHP == ( *HP_Work >> 8 ) && ( *HP_Work & 0xff ) == 0 )
    {
      return (-1);
    }
  }
  else{
    if( endHP == *HP_Work )
    {
      return (-1);
    }
  }

  //-- 終了ﾁｪｯｸ終了 --//
  if( MaxHP < GaugeMaxDot )
  {
    add_hp = MaxHP * 0x100 / GaugeMaxDot;
    if( beHP < 0 ){ //ｹﾞｰｼﾞ回復
      *HP_Work += add_hp;
      ret = *HP_Work >> 8;
      if( ret >= endHP ){ //少数を扱っているのでｵｰﾊﾞｰﾌﾛｰﾁｪｯｸ
        *HP_Work = endHP << 8;
        ret = endHP;
      }
    }
    else{         //ｹﾞｰｼﾞ減少
      *HP_Work -= add_hp;
      ret = *HP_Work >> 8;
      if( ( *HP_Work & 0xff ) > 0 ) //少数部分があるなら少数繰り上げで表示
      {
        ret++;
      }
      if( ret <= endHP )    //少数を扱っているのでｵｰﾊﾞｰﾌﾛｰﾁｪｯｸ
      {
        *HP_Work = endHP << 8;
        ret = endHP;
      }
    }
  }
  else{
    if( beHP < 0 ){   //ｹﾞｰｼﾞ回復
      *HP_Work += add_dec;
      if( *HP_Work > endHP )
      {
        *HP_Work = endHP;
      }
    }
    else{       //ｹﾞｰｼﾞ減少
      *HP_Work -= add_dec;
      if( *HP_Work < endHP )
      {
        *HP_Work = endHP;
      }
    }
    ret = *HP_Work;
  }
  return ret;
}

//--------------------------------------------------------------
/**
 * @brief   ゲージ表示計算
 *
 * @param   MaxHP   最大HP
 * @param   NowHP   現在HP(計算前のHP)
 * @param   beHP    ダメージ(-でダメージ、+なら回復)
 * @param   HP_Work   作業用ワーク
 * @param   gauge_chr ゲージ表示結果代入先(GaugeMax分の配列要素数が必要)
 * @param   GaugeMax  ゲージのキャラクタ数
 *
 * @retval  表示するドット数
 *
 * gauge_chrに入る値は、0:空っぽのｹﾞｰｼﾞ  1:1ﾄﾞｯﾄのｹﾞｰｼﾞ 2:2ﾄﾞｯﾄのｹﾞｰｼﾞ ･･･ 8:8ﾄﾞｯﾄ(満ﾀﾝ)のｹﾞｰｼﾞ
 */
//--------------------------------------------------------------
static u8 PutGaugeProc( s32 MaxHP, s32 NowHP, s32 beHP, s32 *HP_Work, u8 *gauge_chr, u8 GaugeMax )
{
  int i;
  u32 GaugeMaxDot;
  u32 put_dot, dot_ret;   //表示するﾄﾞｯﾄ数
  s32 AfterHP;

  AfterHP = NowHP - beHP;
  if( AfterHP < 0 )
  {
    AfterHP = 0;
  }
  else if( AfterHP > MaxHP )
  {
    AfterHP = MaxHP;
  }

  GaugeMaxDot = BTLV_GAUGE_DOTTO * GaugeMax;
  for( i = 0 ; i < GaugeMax ; i++ )   //空白ｹﾞｰｼﾞで初期化
  {
    gauge_chr[ i ] = 0;
  }

  //※※ここのドット取得の計算式を変更したらGetNumDottoの計算式も変更すること!!
  if( MaxHP < GaugeMaxDot )
  {
    put_dot = ( *HP_Work * GaugeMaxDot / MaxHP ) >> 8;
  }
  else{
    put_dot = *HP_Work * GaugeMaxDot / MaxHP;
  }
  dot_ret = put_dot;
  //ﾄﾞｯﾄ計算では0でも、実際のHPが1以上あるなら1ﾄﾞｯﾄのｹﾞｰｼﾞを表示する
  if( put_dot == 0 && AfterHP > 0)
  {
    gauge_chr[ 0 ] = 1;
    dot_ret = 1;
  }
  else
  {
    for( i = 0 ; i < GaugeMax ; i++ )
    {
      if( put_dot >= BTLV_GAUGE_DOTTO )
      {
        gauge_chr[ i ] = 8;
        put_dot -= BTLV_GAUGE_DOTTO;
      }
      else
      {
        gauge_chr[ i ] = put_dot;
        break;  //ﾄﾞｯﾄが尽きたので抜ける
      }
    }
  }
  return dot_ret;
}

//--------------------------------------------------------------
/**
 * @brief   ｹﾞｰｼﾞ計算前と計算後の最終的なﾄﾞｯﾄの差分を求める
 *
 * @param   nowHP   現在HP(計算前のHP)
 * @param   beHP    ダメージ(-でダメージ、+なら回復)
 * @param   MaxHP   最大HP
 * @param   GaugeMax  ゲージのキャラクタ数
 *
 * @retval  ドット差分
 */
//--------------------------------------------------------------
static u32 DottoOffsetCalc( s32 nowHP, s32 beHP, s32 MaxHP, u8 GaugeMax )
{
  s8  now_dotto, end_dotto,ret_dotto;
  u8  GaugeMaxDot;
  s32 endHP;

  GaugeMaxDot = GaugeMax * BTLV_GAUGE_DOTTO;

  endHP = nowHP - beHP;
  if( endHP < 0 )
  {
    endHP = 0;
  }
  else if( endHP > MaxHP )
  {
    endHP = MaxHP;
  }
  now_dotto = nowHP * GaugeMaxDot / MaxHP;
  end_dotto = endHP * GaugeMaxDot / MaxHP;
  ret_dotto = now_dotto - end_dotto;
  return ( GFL_STD_Abs( ret_dotto ) );
}

//--------------------------------------------------------------
/**
 * @brief  ニックネーム描画
 *
 * @param bgw   BTLV_GAUGE_WORK管理構造体へのポインタ
 * @param bgcl  ゲージワークへのポインタ
 * @param pp    POKEMON_PARAM構造体
 */
//--------------------------------------------------------------
static  void  PutNameOBJ( BTLV_GAUGE_WORK* bgw, BTLV_GAUGE_CLWK *bgcl, const BTL_POKEPARAM* bpp )
{
  STRBUF* monsname = GFL_STR_CreateBuffer( BUFLEN_POKEMON_NAME, bgw->heapID );
  PRINTSYS_LSB  color = PRINTSYS_LSB_Make( 1, 4, 0 );
  GFL_BMP_DATA* bmp = GFL_BMP_Create( BTLV_GAUGE_BMP_SIZE_X, BTLV_GAUGE_BMP_SIZE_Y, GFL_BMP_16_COLOR, bgw->heapID );
  u8 letter, shadow, back;

  GFL_BMP_Clear( bmp, 0 );

  {
    const POKEMON_PARAM* pp = BPP_GetViewSrcData( bpp );
    PP_Get( pp, ID_PARA_nickname, monsname );
  }


  GFL_FONTSYS_GetColor( &letter, &shadow, &back );
  GFL_FONTSYS_SetColor( PRINTSYS_LSB_GetL( color ), PRINTSYS_LSB_GetS( color ), PRINTSYS_LSB_GetB( color ) );
  PRINTSYS_Print( bmp, BTLV_GAUGE_BMP_POS_X, BTLV_GAUGE_BMP_POS_Y, monsname, bgw->font );
  GFL_FONTSYS_SetColor( letter, shadow, back );

  {
    void *obj_vram;
    NNSG2dImageProxy image;
    u8* bmp_data = GFL_BMP_GetCharacterAdrs( bmp );

    obj_vram = G2_GetOBJCharPtr();
    GFL_CLACT_WK_GetImgProxy( bgcl->base_clwk, &image );

    MI_CpuCopy16( bmp_data,
                  (void*)( (u32)obj_vram + BTLV_GAUGE_NAME1U_CHARSTART * 0x20 +
                  image.vramLocation.baseAddrOfVram[ NNS_G2D_VRAM_TYPE_2DMAIN ] ),
                  0x20 * 6 );
    MI_CpuCopy16( &bmp_data[ BTLV_GAUGE_BMP_SIZE_X * 0x20 ],
                  (void*)( (u32)obj_vram + BTLV_GAUGE_NAME1D_CHARSTART * 0x20 +
                  image.vramLocation.baseAddrOfVram[ NNS_G2D_VRAM_TYPE_2DMAIN ] ),
                  0x20 * 6 );
    MI_CpuCopy16( &bmp_data[ 6 * 0x20 ],
                  (void*)( (u32)obj_vram + BTLV_GAUGE_NAME2U_CHARSTART * 0x20 +
                  image.vramLocation.baseAddrOfVram[ NNS_G2D_VRAM_TYPE_2DMAIN ] ),
                  0x20 * 2 );
    MI_CpuCopy16( &bmp_data[ ( BTLV_GAUGE_BMP_SIZE_X + 6 ) * 0x20 ],
                  (void*)( (u32)obj_vram + BTLV_GAUGE_NAME2D_CHARSTART * 0x20 +
                  image.vramLocation.baseAddrOfVram[ NNS_G2D_VRAM_TYPE_2DMAIN ] ),
                  0x20 * 2 );
  }

  GFL_STR_DeleteBuffer( monsname );
  GFL_BMP_Delete( bmp );
}

//--------------------------------------------------------------
/**
 * @brief  性別描画
 *
 * @param bgw   BTLV_GAUGE_WORK管理構造体へのポインタ
 * @param bgcl  ゲージワークへのポインタ
 */
//--------------------------------------------------------------
static  void  PutSexOBJ( BTLV_GAUGE_WORK* bgw, BTLV_GAUGE_CLWK *bgcl )
{
  void *obj_vram;
  NNSG2dImageProxy image;

  if( bgcl->sex == PTL_SEX_UNKNOWN )
  {
    return;
  }

  obj_vram = G2_GetOBJCharPtr();
  GFL_CLACT_WK_GetImgProxy( bgcl->base_clwk, &image );

  MI_CpuCopy16( &bgw->parts_address[ GP_MALE_U + bgcl->sex * 0x40 ],
                (void*)( (u32)obj_vram + BTLV_GAUGE_SEXU_CHARSTART * 0x20 +
                image.vramLocation.baseAddrOfVram[ NNS_G2D_VRAM_TYPE_2DMAIN ] ),
                0x20 );
  MI_CpuCopy16( &bgw->parts_address[ GP_MALE_D + bgcl->sex * 0x40 ],
                (void*)( (u32)obj_vram + BTLV_GAUGE_SEXD_CHARSTART * 0x20 +
                image.vramLocation.baseAddrOfVram[ NNS_G2D_VRAM_TYPE_2DMAIN ] ),
                0x20 );
}

//--------------------------------------------------------------
/**
 * @brief   ゲージバー描画
 *
 * @param bgw   BTLV_GAUGE_WORK管理構造体へのポインタ
 * @param bgcl  ゲージワークへのポインタ
 * @param req   GAUGE_REQ_???
 */
//--------------------------------------------------------------
static  void  PutGaugeOBJ( BTLV_GAUGE_WORK* bgw, BTLV_GAUGE_CLWK *bgcl, BTLV_GAUGE_REQ req )
{
  u8  i;
  u8  gauge_chr[ BTLV_GAUGE_EXP_CHARMAX ];

  u16 parts_num;
  u8  put_dot;
  u8  level;
  void *obj_vram;
  NNSG2dImageProxy image;

  obj_vram = G2_GetOBJCharPtr();

  switch(req){
  case BTLV_GAUGE_REQ_HP:
    GFL_CLACT_WK_GetImgProxy( bgcl->hp_clwk, &image );
    put_dot = PutGaugeProc( bgcl->hpmax, bgcl->hp, bgcl->damage, &bgcl->hp_work, gauge_chr, BTLV_GAUGE_HP_CHARMAX );

    //ゲージの表示ドット数から色とパーツアドレスを取得
    {
      int dotto_color;

      dotto_color = GAUGETOOL_GetGaugeDottoColor( put_dot, BTLV_GAUGE_HP_DOTTOMAX );
      switch(dotto_color){
      case GAUGETOOL_HP_DOTTO_GREEN:
        parts_num = GP_HPBAR_GREEN;
        break;
      case GAUGETOOL_HP_DOTTO_YELLOW:
        parts_num = GP_HPBAR_YELLOW;
        break;
      case GAUGETOOL_HP_DOTTO_RED:
      default:
        parts_num = GP_HPBAR_RED;
        break;
      }
    }
    for( i = 0 ; i < BTLV_GAUGE_HP_CHARMAX ; i++ ){
      MI_CpuCopy16( &bgw->parts_address[ parts_num + ( gauge_chr[ i ] << 5 ) ],
                    (void*)( (u32)obj_vram + ( i + BTLV_GAUGE_HP_CHARSTART ) * 0x20 +
                    image.vramLocation.baseAddrOfVram[ NNS_G2D_VRAM_TYPE_2DMAIN ] ),
                    0x20 );
    }
    break;

  case BTLV_GAUGE_REQ_EXP:
    if( bgcl->exp_clwk )
    {
      GFL_CLACT_WK_GetImgProxy( bgcl->exp_clwk, &image );
      PutGaugeProc( bgcl->exp_max, bgcl->exp, bgcl->exp_add, &bgcl->exp_work, gauge_chr, BTLV_GAUGE_EXP_CHARMAX );
      level = bgcl->level;
      if( level == 100 )
      {
        for( i = 0; i < BTLV_GAUGE_EXP_CHARMAX ; i++ )
        {
          gauge_chr[ i ] = 0;
        }
      }

      for(i = 0; i < BTLV_GAUGE_EXP_CHARMAX; i++){
        MI_CpuCopy16( &bgw->parts_address[ GP_EXPBAR + ( gauge_chr[ i ] << 5 ) ],
                      (void*)( (u32)obj_vram + ( i + BTLV_GAUGE_EXP_CHARSTART ) * 0x20 +
                      image.vramLocation.baseAddrOfVram[ NNS_G2D_VRAM_TYPE_2DMAIN ] ),
                      0x20 );
      }
    }
    break;
  }
}

//--------------------------------------------------------------
/**
 * @brief   HP数値描画
 *
 * @param bgw   BTLV_GAUGE_WORK管理構造体へのポインタ
 * @param bgcl  ゲージワークへのポインタ
 * @param nowHP HP現在値（途中経過を表示する必要があるので、引き数でもらうようにしてます）
 */
//--------------------------------------------------------------
static  void  PutHPNumOBJ( BTLV_GAUGE_WORK* bgw, BTLV_GAUGE_CLWK *bgcl, s32 nowHP )
{
  int i;
  u16 now_hp[ BTLV_GAUGE_NUM_MAX ];
  u16 max_hp[ BTLV_GAUGE_NUM_MAX ];
  s32 hp_max;
  void *obj_vram;
  NNSG2dImageProxy image;

  if( bgcl->hpnum_clwk == NULL )
  {
    return;
  }

  obj_vram = G2_GetOBJCharPtr();
  GFL_CLACT_WK_GetImgProxy( bgcl->hpnum_clwk, &image );

  for( i = 0 ; i < BTLV_GAUGE_NUM_MAX ; i++ )
  {
    now_hp[ i ] = GP_SPACE;
    max_hp[ i ] = GP_SPACE;
  }

  //HP現在値計算
  if( nowHP >= 100 )
  {
    now_hp[ 0 ] = GP_NUM_0 + ( nowHP / 100 ) * 0x20;
    nowHP = nowHP % 100;
  }
  if( ( nowHP >= 10 ) || ( now_hp[ 0 ] != GP_SPACE ) )
  {
    now_hp[ 1 ] = GP_NUM_0 + ( nowHP / 10 ) * 0x20;
    nowHP = nowHP % 10;
  }
  now_hp[ 2 ] = GP_NUM_0 + nowHP * 0x20;

  //HPMAX計算
  hp_max = bgcl->hpmax;
  i = 0;

  if( hp_max >= 100 )
  {
    max_hp[ i ] = GP_NUM_0 + ( hp_max / 100 ) * 0x20;
    hp_max = hp_max % 100;
    i++;
  }
  if( hp_max >= 10 )
  {
    max_hp[ i ] = GP_NUM_0 + ( hp_max / 10 ) * 0x20;
    hp_max = hp_max % 10;
    i++;
  }
  max_hp[ i ] = GP_NUM_0 + hp_max * 0x20;

  for( i = 0 ; i < BTLV_GAUGE_NUM_MAX ; i++ ){
    MI_CpuCopy16( &bgw->parts_address[ now_hp[ i ] ],
                  (void*)( (u32)obj_vram + ( i + BTLV_GAUGE_NOWHP_CHARSTART ) * 0x20 +
                  image.vramLocation.baseAddrOfVram[ NNS_G2D_VRAM_TYPE_2DMAIN ] ),
                  0x20);
    MI_CpuCopy16( &bgw->parts_address[ max_hp[ i ] ],
                  (void*)( (u32)obj_vram + ( i + BTLV_GAUGE_MAXHP_CHARSTART ) * 0x20 +
                  image.vramLocation.baseAddrOfVram[ NNS_G2D_VRAM_TYPE_2DMAIN ] ),
                  0x20);
  }
}

//--------------------------------------------------------------
/**
 * @brief   LV数値描画
 *
 * @param bgw   BTLV_GAUGE_WORK管理構造体へのポインタ
 * @param bgcl  ゲージワークへのポインタ
 */
//--------------------------------------------------------------
static  void  PutLVNumOBJ( BTLV_GAUGE_WORK* bgw, BTLV_GAUGE_CLWK *bgcl )
{
#if 0
  int i;
  u16 level[ BTLV_GAUGE_NUM_MAX ];
  int lv;
  void *obj_vram;
  NNSG2dImageProxy image;

  obj_vram = G2_GetOBJCharPtr();
  GFL_CLACT_WK_GetImgProxy( bgcl->base_clwk, &image );

  level[ 0 ] = GP_LV;
  level[ 1 ] = GP_SPACE;
  level[ 2 ] = GP_SPACE;

  //LV計算
  lv = bgcl->level;
  i = 1;
  if( lv >= 100 )
  {
    level[ 0 ] = GP_NUM_1;
    level[ 1 ] = GP_NUM_0;
    level[ 2 ] = GP_NUM_0;
  }
  else
  {
    if( lv >= 10 )
    {
      level[ i ] = GP_NUM_0 + ( lv / 10 ) * 0x20;
      lv = lv % 10;
      i++;
    }
    level[ i ] = GP_NUM_0 + lv * 0x20;
  }

  for( i = 0 ; i < BTLV_GAUGE_NUM_MAX ; i++ ){
    MI_CpuCopy16( &bgw->parts_address[ level[ i ] ],
                  (void*)( (u32)obj_vram + ( i + BTLV_GAUGE_LV_CHARSTART ) * 0x20 +
                  image.vramLocation.baseAddrOfVram[ NNS_G2D_VRAM_TYPE_2DMAIN ] ),
                  0x20);
  }
#else
  STRBUF* num = GFL_STR_CreateBuffer( 4, bgw->heapID );
  PRINTSYS_LSB  color = PRINTSYS_LSB_Make( 1, 4, 0 );
  GFL_BMP_DATA* bmp = GFL_BMP_Create( 3, BTLV_GAUGE_BMP_SIZE_Y, GFL_BMP_16_COLOR, bgw->heapID );
  u8 letter, shadow, back;

  GFL_BMP_Clear( bmp, 0 );

  STRTOOL_SetNumber( num, bgcl->level, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_ZENKAKU );

  GFL_FONTSYS_GetColor( &letter, &shadow, &back );
  GFL_FONTSYS_SetColor( PRINTSYS_LSB_GetL( color ), PRINTSYS_LSB_GetS( color ), PRINTSYS_LSB_GetB( color ) );
  PRINTSYS_Print( bmp, 0, BTLV_GAUGE_BMP_POS_Y, num, bgw->font );
  GFL_FONTSYS_SetColor( letter, shadow, back );

  {
    void *obj_vram;
    NNSG2dImageProxy image;
    u8* bmp_data = GFL_BMP_GetCharacterAdrs( bmp );

    obj_vram = G2_GetOBJCharPtr();
    GFL_CLACT_WK_GetImgProxy( bgcl->base_clwk, &image );

    MI_CpuCopy16( bmp_data,
                  (void*)( (u32)obj_vram + 0x24 * 0x20 +
                  image.vramLocation.baseAddrOfVram[ NNS_G2D_VRAM_TYPE_2DMAIN ] ),
                  0x20 * 3 );
    MI_CpuCopy16( &bmp_data[ 3 * 0x20 ],
                  (void*)( (u32)obj_vram + 0x2c * 0x20 +
                  image.vramLocation.baseAddrOfVram[ NNS_G2D_VRAM_TYPE_2DMAIN ] ),
                  0x20 * 3 );
  }

  GFL_STR_DeleteBuffer( num );
  GFL_BMP_Delete( bmp );
#endif
}

//--------------------------------------------------------------
/**
 * @brief   ゲージレベルアップ処理
 *
 * @param bgw   BTLV_GAUGE_WORK管理構造体へのポインタ
 * @param bgcl  ゲージワークへのポインタ
 */
//--------------------------------------------------------------
static  void  Gauge_LevelUp( BTLV_GAUGE_WORK* bgw, BTLV_GAUGE_CLWK* bgcl )
{
  if( bgcl->exp_calc_req )
  {
    return;
  }
  switch( bgcl->seq_no ){
  case 0:
    PMSND_PlaySE( SEQ_SE_EXPMAX );
    /** @TODO レベルアップエフェクトを入れたい */
    bgcl->seq_no++;
    break;
  case 1:
    /** @TODO レベルアップエフェクト終了チェックを入れたい */
    bgcl->seq_no++;
    break;
  case 2:
    bgcl->exp       = 0;
    bgcl->exp_max   = bgcl->next_exp_max;
    bgcl->exp_work  = BTLV_GAUGE_HP_WORK_INIT_VALUE;
    bgcl->exp_add   = 0;
    GaugeProc( bgcl->hpmax, bgcl->hp, 0, &bgcl->hp_work, BTLV_GAUGE_HP_CHARMAX, 1 );
    GaugeProc( bgcl->exp_max, bgcl->exp, 0, &bgcl->exp_work, BTLV_GAUGE_EXP_CHARMAX, 1 );
    PutGaugeOBJ( bgw, bgcl, BTLV_GAUGE_REQ_HP );
    PutGaugeOBJ( bgw, bgcl, BTLV_GAUGE_REQ_EXP );
    PutHPNumOBJ( bgw, bgcl, bgcl->hp );
    PutLVNumOBJ( bgw, bgcl );
    bgcl->level_up_req = 0;
    break;
  }
}

//--------------------------------------------------------------
/**
 * @brief   状態異常アイコンセット
 *
 * @param bgw   BTLV_GAUGE_WORK管理構造体へのポインタ
 * @param sick  セットする状態異常
 * @param pos   セットするポケモンの立ち位置
 */
//--------------------------------------------------------------
void  BTLV_GAUGE_SetStatus( BTLV_GAUGE_WORK* bgw, PokeSick sick, BtlvMcssPos pos )
{
  int sick_anm[]={
    APP_COMMON_ST_ICON_NONE,        // なし（アニメ番号的にもなし）
    APP_COMMON_ST_ICON_MAHI,        // 麻痺
    APP_COMMON_ST_ICON_NEMURI,      // 眠り
    APP_COMMON_ST_ICON_KOORI,       // 氷
    APP_COMMON_ST_ICON_YAKEDO,      // 火傷
    APP_COMMON_ST_ICON_DOKU,        // 毒
  };
  //ゲージが存在しないときはなにもせずにリターン
  if( bgw->bgcl[ pos ].status_clwk == NULL )
  {
    return;
  }
  if( sick == POKESICK_NULL )
  {
    GFL_CLACT_WK_SetDrawEnable( bgw->bgcl[ pos ].status_clwk, FALSE );
  }
  else
  {
    BTL_Printf("GAUGE[%d] SetStatus(%d)\n", pos, sick);
    GFL_CLACT_WK_SetDrawEnable( bgw->bgcl[ pos ].status_clwk, TRUE );
    GFL_CLACT_WK_SetAnmSeq( bgw->bgcl[ pos ].status_clwk, sick_anm[ sick ] );
  }
}

//--------------------------------------------------------------
/**
 * @brief   ピンチBGM再生チェック
 *
 * @param bgw     BTLV_GAUGE_WORK管理構造体へのポインタ
 */
//--------------------------------------------------------------
static  void  pinch_bgm_check( BTLV_GAUGE_WORK* bgw )
{
  int i;

  if( bgw->bgm_fade_flag )
  {
    if( PMSND_CheckFadeOnBGM() == FALSE )
    {
      bgw->bgm_fade_flag = 0;
      if( bgw->pinch_bgm_flag )
      {
        PMSND_PopBGM();
        PMSND_FadeInBGM( 24 );
      }
      else
      {
        PMSND_PushBGM();
        PMSND_PlayBGM( SEQ_BGM_BATTLEPINCH );
        PMSND_FadeInBGM( 8 );
      }
      bgw->pinch_bgm_flag ^= 1;
    }
  }
  else
  {
    BOOL  pinch_flag = FALSE;

    for( i = 0 ; i < BTLV_GAUGE_NUM_MAX ; i++ )
    {
      if( ( bgw->bgcl[ i ].gauge_dir == 0 ) &&
          ( bgw->bgcl[ i ].gauge_enable ) &&
          ( bgw->bgcl[ i ].gauge_type != BTLV_GAUGE_TYPE_SAFARI ) )
      {
        u8  color = GAUGETOOL_GetHPGaugeDottoColor( bgw->bgcl[ i ].hp, bgw->bgcl[ i ].hpmax, BTLV_GAUGE_HP_DOTTOMAX );

        if( color == GAUGETOOL_HP_DOTTO_RED )
        {
          pinch_flag = TRUE;
        }
      }
    }
    //勝利ジングルとかに変化していたら、ピンチBGMチェックはしないようにする
    if( ( bgw->pinch_bgm_flag ) || ( PMSND_GetBGMsoundNo() == bgw->now_bgm_no ) )
    {
      if( ( ( bgw->pinch_bgm_flag ) && ( pinch_flag == FALSE ) ) ||
          ( ( bgw->pinch_bgm_flag == 0 ) && ( pinch_flag == TRUE ) ) )
      {
        bgw->bgm_fade_flag = 1;
        PMSND_FadeOutBGM( 8 );
      }
    }
  }
}
#if 0
static  void  pinch_bgm_check( BTLV_GAUGE_WORK* bgw )
{
  int i;

  if( bgw->bgm_fade_flag )
  {
    if( PMSND_CheckFadeOnBGM() == FALSE )
    {
      bgw->bgm_fade_flag = 0;
      if( bgw->now_bgm_no != PMSND_GetBGMsoundNo() )
      {
        OS_TPrintf("PopBGM\n");
        PMSND_PopBGM();
        PMSND_FadeInBGM( 24 );
      }
      else
      {
        PMSND_PushBGM();
        PMSND_PlayBGM( SEQ_BGM_BATTLEPINCH );
        PMSND_FadeInBGM( 8 );
      }
    }
  }
  else
  {
    BOOL  pinch_flag = FALSE;

    for( i = 0 ; i < BTLV_GAUGE_NUM_MAX ; i++ )
    {
      if( ( bgw->bgcl[ i ].gauge_dir == 0 ) &&
          ( bgw->bgcl[ i ].gauge_enable ) &&
          ( bgw->bgcl[ i ].gauge_type != BTLV_GAUGE_TYPE_SAFARI ) )
      {
        u8  color = GAUGETOOL_GetHPGaugeDottoColor( bgw->bgcl[ i ].hp, bgw->bgcl[ i ].hpmax, BTLV_GAUGE_HP_DOTTOMAX );

        if( color == GAUGETOOL_HP_DOTTO_RED )
        {
          pinch_flag = TRUE;
        }
      }
    }
    if( ( ( bgw->now_bgm_no != PMSND_GetBGMsoundNo() ) && ( pinch_flag == FALSE ) ) ||
        ( ( bgw->now_bgm_no == PMSND_GetBGMsoundNo() ) && ( pinch_flag == TRUE ) ) )
    {
      bgw->bgm_fade_flag = 1;
      PMSND_FadeOutBGM( 8 );
    }
  }
}
#endif
