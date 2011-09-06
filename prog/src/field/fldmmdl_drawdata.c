//======================================================================
/**
 * @file  fieldobj_drawdata.c
 * @brief  フィールド動作モデル描画データ
 * @author  kagaya
 * @date  05.07.20
 */
//======================================================================
#include "fldmmdl.h"
#include "fldmmdl_procdraw.h"

//======================================================================
//  define
//======================================================================
#define ANMFLIP_ON (GFL_BBDACT_ANMFLIP_ON)
#define ANMFLIP_OFF (GFL_BBDACT_ANMFLIP_OFF)

//======================================================================
//  フィールド動作モデル　描画関数テーブル
//======================================================================
const MMDL_DRAW_PROC_LIST * const
  DATA_MMDL_DRAW_PROC_LIST_Tbl[MMDL_DRAWPROCNO_MAX] =
{
  &DATA_MMDL_DRAWPROCLIST_Non,    //描画無し
  &DATA_MMDL_DRAWPROCLIST_Hero,  //自機
  &DATA_MMDL_DRAWPROCLIST_BlAct,  //通常ビルボードアクター
  &DATA_MMDL_DRAWPROCLIST_CycleHero,  //自機自転車
  &DATA_MMDL_DRAWPROCLIST_SwimHero,  //自機波乗り
  &DATA_MMDL_DRAWPROCLIST_BlActAlwaysAnime, //通常ビルボード＋常にアニメ
  &DATA_MMDL_DRAWPROCLIST_ItemGetHero, //自機アイテムゲット
  &DATA_MMDL_DRAWPROCLIST_PCAzukeHero, //自機PC預け
  &DATA_MMDL_DRAWPROCLIST_BlActOnePatternLoop, //ビルボード1パターンアニメ
//&DATA_MMDL_DRAWPROCLIST_BlActOnePattern, //ビルボード1パターンアニメ
  &DATA_MMDL_DRAWPROCLIST_BlActAct, //汎用+演技
  &DATA_MMDL_DRAWPROCLIST_TsurePoke, //連れ歩きポケモン
  &DATA_MMDL_DRAWPROCLIST_TsurePoke, //連れ歩きポケモン(上下アニメスピード1/2 関数内で分岐してます)
  &DATA_MMDL_DRAWPROCLIST_Model, //モデル描画
  &DATA_MMDL_DRAWPROCLIST_FishingHero, //釣り自機
  &DATA_MMDL_DRAWPROCLIST_YureHero, //揺れ自機
  &DATA_MMDL_DRAWPROCLIST_BlActAlwaysAnime, //通常ビルボード＋常にアニメ
  &DATA_MMDL_DRAWPROCLIST_BlActShinMu, //ポケモン シン・ム
  &DATA_MMDL_DRAWPROCLIST_BlActSpider, //くもの巣
  &DATA_MMDL_DRAWPROCLIST_BlActMelodyer, //ポケモン メロディア
  &DATA_MMDL_DRAWPROCLIST_BlActActNonePause, //汎用+演技 演技中はポーズ無視
  &DATA_MMDL_DRAWPROCLIST_BlActAlwaysAnime, //通常ビルボード＋常にアニメ
};

//======================================================================
//  ビルボードアクター　モデルサイズ [size][x,y]
//======================================================================
const u16 DATA_MMDL_BLACT_MdlSize[MMDL_BLACT_MDLSIZE_MAX][2] =
{
  {32,32}, //MMDL_BLACT_MDLSIZE_32x32
  {16,16}, //MMDL_BLACT_MDLSIZE_16x16
  {64,64}, //MMDL_BLACT_MDLSIZE_64x64
};

//--------------------------------------------------------------
/// モデルサイズ別描画サイズ
//--------------------------------------------------------------
#define MMDL_BBD_DRAW_SIZE_16 (FX16_ONE*1)
#define MMDL_BBD_DRAW_SIZE_32 (FX16_ONE*2)
#define MMDL_BBD_DRAW_SIZE_64 (FX16_ONE*4)

const u16 DATA_MMDL_BLACT_MdlSizeDrawSize[MMDL_BLACT_MDLSIZE_MAX][2] =
{
  {MMDL_BBD_DRAW_SIZE_32,MMDL_BBD_DRAW_SIZE_32}, //MMDL_BLACT_MDLSIZE_32x32
  {MMDL_BBD_DRAW_SIZE_16,MMDL_BBD_DRAW_SIZE_16}, //MMDL_BLACT_MDLSIZE_16x16
  {MMDL_BBD_DRAW_SIZE_64,MMDL_BBD_DRAW_SIZE_64}, //MMDL_BLACT_MDLSIZE_64x64
};

#include "fmmdl_anm/fldmmdl_anmtbl.cdat"

