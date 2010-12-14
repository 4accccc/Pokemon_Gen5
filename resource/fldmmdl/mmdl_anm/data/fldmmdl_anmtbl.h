//=========================================================================
/**
 * @file	mmdl_anm/data/fldmmdl_anmtbl.h
 * @brief	各アニメヘッダを統合。resource/fldmmdl/anmlist.txtから自動生成
 * @autor	miyuki iwasawa
 * @note	このファイルはresource/fldmmdl/tool/fmmdl_anm_list.rbによって自動生成されたものです。
 */
//=========================================================================
#ifndef __H_FLDMMDL_ANMTBL_H__
#define __H_FLDMMDL_ANMTBL_H__

#include "fmmdl_anm_hero.h"
#include "fmmdl_anm_flip.h"
#include "fmmdl_anm_nonflip.h"
#include "fmmdl_anm_onecell.h"
#include "fmmdl_anm_swimhero.h"
#include "fmmdl_anm_cyclehero.h"
#include "fmmdl_anm_gethero.h"
#include "fmmdl_anm_pchero.h"
#include "fmmdl_anm_savehero.h"
#include "fmmdl_anm_fishinghero.h"
#include "fmmdl_anm_cutinhero.h"
#include "fmmdl_anm_yurehero.h"
#include "fmmdl_anm_poke_flip.h"
#include "fmmdl_anm_poke_nonflip.h"
#include "fmmdl_anm_flip_pcwoman.h"
#include "fmmdl_anm_nonflip_rival.h"
#include "fmmdl_anm_nonflip_support.h"
#include "fmmdl_anm_nonflip_boss_n.h"
#include "fmmdl_anm_nonflip_sage1.h"
#include "fmmdl_anm_nonflip_ninja.h"
#include "fmmdl_anm_nonflip_champion.h"
#include "fmmdl_anm_spider.h"
#include "fmmdl_anm_melodyer.h"
#include "fmmdl_anm_shinmu_a.h"
#include "fmmdl_anm_shinmu_b.h"
#include "fmmdl_anm_shinmu_c.h"
#include "fmmdl_anm_nonflip_swleader4.h"


//===================================================
//ビルボードアクター　アニメーションテーブルID定義
//===================================================

typedef enum
{
  MMDL_BLACT_ANMTBLNO_NON,
  MMDL_BLACT_ANMTBLNO_HERO,  ///<自機基本アニメ
  MMDL_BLACT_ANMTBLNO_FLIP,  ///<汎用人型　フリップ有アニメ
  MMDL_BLACT_ANMTBLNO_NONFLIP,  ///<汎用人型　ノンフリップアニメ
  MMDL_BLACT_ANMTBLNO_ONECELL,  ///<ワンセルアニメ　同一ファイル内継承
  MMDL_BLACT_ANMTBLNO_SWIMHERO,  ///<自機波乗りアニメ
  MMDL_BLACT_ANMTBLNO_CYCLEHERO,  ///<自機自転車アニメ
  MMDL_BLACT_ANMTBLNO_GETHERO,  ///<自機アイテムゲットアニメ
  MMDL_BLACT_ANMTBLNO_PCHERO,  ///<自機ポケセン預けアニメ
  MMDL_BLACT_ANMTBLNO_SAVEHERO,  ///<自機セーブアニメ
  MMDL_BLACT_ANMTBLNO_FISHINGHERO,  ///<自機釣りアニメ
  MMDL_BLACT_ANMTBLNO_CUTINHERO,  ///<自機カットインアニメ
  MMDL_BLACT_ANMTBLNO_YUREHERO,  ///<自機ゆらゆらアニメ
  MMDL_BLACT_ANMTBLNO_POKE_FLIP,  ///<連れ歩きポケモンフリップ有アニメ
  MMDL_BLACT_ANMTBLNO_POKE_NONFLIP,  ///<連れ歩きポケモンフリップ無しアニメ
  MMDL_BLACT_ANMTBLNO_FLIP_PCWOMAN,  ///<ポケセン受付嬢おじぎアニメ　汎用人型　フリップ有アニメを継承
  MMDL_BLACT_ANMTBLNO_NONFLIP_RIVAL,  ///<汎用人型　フリップ無しアニメ＋ライバル芝居
  MMDL_BLACT_ANMTBLNO_NONFLIP_SUPPORT,  ///<汎用人型　フリップ無しアニメ＋サポート芝居
  MMDL_BLACT_ANMTBLNO_NONFLIP_BOSS_N,  ///<ボスN　フリップ無しアニメ＋ボスＮ芝居
  MMDL_BLACT_ANMTBLNO_NONFLIP_SAGE1,  ///<汎用人型　フリップ無しアニメ＋七賢人ゲーチス芝居
  MMDL_BLACT_ANMTBLNO_NONFLIP_NINJA,  ///<汎用人型　フリップ無しアニメ＋ダークトリニティ芝居(通常アニメ2倍速)
  MMDL_BLACT_ANMTBLNO_NONFLIP_CHAMPION,  ///<汎用人型　フリップ無しアニメ＋チャンピオンアデク芝居
  MMDL_BLACT_ANMTBLNO_SPIDER,  ///<蜘蛛の巣アニメ
  MMDL_BLACT_ANMTBLNO_MELODYER,  ///<メロディアアニメ
  MMDL_BLACT_ANMTBLNO_SHINMU_A,  ///<ゼクレシキュレムアニメ　待機正面・ぐったり・飛び立ち上昇・飛行
  MMDL_BLACT_ANMTBLNO_SHINMU_B,  ///<ゼクレシキュレムアニメ　待機正面・待機背面・吼える・振り向き
  MMDL_BLACT_ANMTBLNO_SHINMU_C,  ///<ゼクレシキュレムアニメ　上昇
  MMDL_BLACT_ANMTBLNO_NONFLIP_SWLEADER4,  ///<スワンカミツレ　フリップ無しアニメ＋芝居1パターン
  MMDL_BLACT_ANMTBLNO_MAX,  ///<最大
}MMDL_BLACT_ANMTBLNO;

#endif //__H_FLDMMDL_ANMTBL_H__

