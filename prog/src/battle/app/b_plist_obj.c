//============================================================================================
/**
 * @file  b_plist_obj.c
 * @brief 戦闘用ポケモンリスト画面OBJ処理
 * @author  Hiroyuki Nakamura
 * @date  05.02.07
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "poke_tool/gauge_tool.h"
#include "pokeicon/pokeicon.h"
#include "item/item.h"
//#include "item/itemsym.h"
#include "app/app_menu_common.h"
#include "waza_tool/wazadata.h"

/*↑[GS_CONVERT_TAG]*/
//#include "system/procsys.h"
/*↑[GS_CONVERT_TAG]*/
/*
#include "system/palanm.h"
#include "system/msgdata.h"
#include "system/numfont.h"
#include "system/wordset.h"
*/
//#include "system/clact_tool.h"
/*↑[GS_CONVERT_TAG]*/
/*
#include "battle/battle_common.h"
#include "battle/fight_tool.h"
#include "battle/wazatype_icon.h"
#include "poketool/pokeparty.h"
#include "poketool/pokeicon.h"
#include "application/p_status.h"
#include "application/pokelist.h"
#include "application/app_tool.h"
#include "itemtool/item.h"
#include "b_app_tool.h"
*/

#include "battle/battgra_wb.naix"
#include "app_menu_common.naix"

#include "b_plist.h"
#include "b_plist_main.h"
#include "b_plist_obj.h"
#include "b_plist_bmp.h"
#include "b_plist_gra.naix"


//============================================================================================
//  定数定義
//============================================================================================
/*
#define CLACT_ID_COMMON   ( 45063 ) // この画面で使用するセルアクターのID

// キャラリソースID
enum {
  CHR_ID_POKE1 = CLACT_ID_COMMON,   // ポケモンアイコン：１匹目
  CHR_ID_POKE2,           // ポケモンアイコン：２匹目
  CHR_ID_POKE3,           // ポケモンアイコン：３匹目
  CHR_ID_POKE4,           // ポケモンアイコン：４匹目
  CHR_ID_POKE5,           // ポケモンアイコン：５匹目
  CHR_ID_POKE6,           // ポケモンアイコン：６匹目

  CHR_ID_STATUS,            // 状態異常アイコン：１匹目

  CHR_ID_POKETYPE1,         // ポケモンタイプアイコン１
  CHR_ID_POKETYPE2,         // ポケモンタイプアイコン２

  CHR_ID_WAZATYPE1,         // 技タイプアイコン１
  CHR_ID_WAZATYPE2,         // 技タイプアイコン２
  CHR_ID_WAZATYPE3,         // 技タイプアイコン３
  CHR_ID_WAZATYPE4,         // 技タイプアイコン４
  CHR_ID_WAZATYPE5,         // 技タイプアイコン５

  CHR_ID_BUNRUI,            // 分類アイコン

  CHR_ID_ITEM,            // アイテムアイコン

  CHR_ID_CND,             // コンディション

  CHR_ID_CURSOR,            // カーソル

  CHR_ID_MAX = CHR_ID_CURSOR - CLACT_ID_COMMON + 1
};

// パレットリソースID
enum {
  PAL_ID_POKE = CLACT_ID_COMMON,    // ポケモンアイコン（３本）
  PAL_ID_STATUS,            // 状態異常アイコン（１本）
  PAL_ID_TYPE,            // タイプ/分類アイコン（３本）
  PAL_ID_ITEM,            // アイテムアイコン（１本）
  PAL_ID_CND,             // コンディション（１本）
  PAL_ID_CURSOR,            // カーソル（１本）
  PAL_ID_MAX = PAL_ID_CURSOR - CLACT_ID_COMMON + 1
};

// セルリソースID
enum {
  CEL_ID_POKE = CLACT_ID_COMMON,    // ポケモンアイコン
  CEL_ID_STATUS,            // 状態異常アイコン
  CEL_ID_TYPE,            // タイプ/分類アイコン
  CEL_ID_ITEM,            // アイテムアイコン
  CEL_ID_CND,             // コンディション
  CEL_ID_CURSOR,            // カーソル
  CEL_ID_MAX = CEL_ID_CURSOR - CLACT_ID_COMMON + 1
};

// セルアニメリソースID
enum {
  ANM_ID_POKE = CLACT_ID_COMMON,    // ポケモンアイコン
  ANM_ID_STATUS,            // 状態異常アイコン
  ANM_ID_TYPE,            // タイプ/分類アイコン
  ANM_ID_ITEM,            // アイテムアイコン
  ANM_ID_CND,             // コンディション
  ANM_ID_CURSOR,            // カーソル
  ANM_ID_MAX = ANM_ID_CURSOR - CLACT_ID_COMMON + 1
};
*/

#define CURSOR_CLA_MAX    ( 5 )   // カーソルのOBJ数

// パレット位置＆サイズ
#define PALOFS_POKEICON     ( 0 )
#define PALSIZ_POKEICON     ( 3 )
#define PALOFS_STATUSICON   ( PALOFS_POKEICON + 0x20 * PALSIZ_POKEICON )
#define PALSIZ_STATUSICON   ( 1 )
#define PALOFS_ITEMICON     ( PALOFS_STATUSICON + 0x20 * PALSIZ_STATUSICON )
#define PALSIZ_ITEMICON     ( 1 )
#define PALOFS_TYPEICON     ( PALOFS_ITEMICON + 0x20 * PALSIZ_ITEMICON )
#define PALSIZ_TYPEICON     ( 3 )
#define PALOFS_CURSOR       ( PALOFS_TYPEICON + 0x20 * PALSIZ_TYPEICON )
#define PALSIZ_CURSOR       ( 1 )
#define PALOFS_HPGAUGE      ( PALOFS_CURSOR + 0x20 * PALSIZ_CURSOR )
#define PALSIZ_HPGAUGE      ( 1 )


//============================================================================================
//  プロトタイプ宣言
//============================================================================================
static void BPL_ClactResManInit( BPLIST_WORK * wk );
static void BPL_ClactPokeLoad( BPLIST_WORK * wk );
static void BPL_ClactStatusLoad( BPLIST_WORK * wk );
//static void BPL_ClactConditionLoad( BPLIST_WORK * wk );
static void BPL_ClactTypeLoad( BPLIST_WORK * wk );
static void BPL_ClactItemLoad( BPLIST_WORK * wk );
static void BPL_ClactAddAll( BPLIST_WORK * wk );

static void BPL_PokeIconPaletteChg( BPLIST_WORK * wk );

static void BPL_Page1ObjSet( BPLIST_WORK * wk );
static void BPL_ChgPageObjSet( BPLIST_WORK * wk );
static void BPL_StMainPageObjSet( BPLIST_WORK * wk );
static void BPL_StWazaSelPageObjSet( BPLIST_WORK * wk );
static void BPL_Page4ObjSet( BPLIST_WORK * wk );
static void BPL_Page5ObjSet( BPLIST_WORK * wk );
static void BPL_Page6ObjSet( BPLIST_WORK * wk );
static void BPL_Page7ObjSet( BPLIST_WORK * wk );
//static void BPL_Page8ObjSet( BPLIST_WORK * wk );
//static void BPL_Page9ObjSet( BPLIST_WORK * wk );

//static void BPL_EzConditionPut( BPLIST_WORK * wk );

static void BPL_ClactCursorAdd( BPLIST_WORK * wk );
static void BPL_CursorDel( BPLIST_WORK * wk );

static void CursorResLoad( BPLIST_WORK * wk );
static void HPGaugeResLoad( BPLIST_WORK * wk );


//============================================================================================
//  グローバル変数
//============================================================================================
// リソーステーブル
static const u32 ObjParamEz[][5] =
{ // キャラ、パレット、セル、セルアニメ、BGプライオリティ、OBJプライオリティ
  { BPLIST_CHRRES_ITEM, BPLIST_PALRES_ITEM, BPLIST_CELRES_ITEM, 1, 0 },     // アイテムアイコン１
  { BPLIST_CHRRES_ITEM, BPLIST_PALRES_ITEM, BPLIST_CELRES_ITEM, 1, 0 },     // アイテムアイコン２
  { BPLIST_CHRRES_ITEM, BPLIST_PALRES_ITEM, BPLIST_CELRES_ITEM, 1, 0 },     // アイテムアイコン３
  { BPLIST_CHRRES_ITEM, BPLIST_PALRES_ITEM, BPLIST_CELRES_ITEM, 1, 0 },     // アイテムアイコン４
  { BPLIST_CHRRES_ITEM, BPLIST_PALRES_ITEM, BPLIST_CELRES_ITEM, 1, 0 },     // アイテムアイコン５
  { BPLIST_CHRRES_ITEM, BPLIST_PALRES_ITEM, BPLIST_CELRES_ITEM, 1, 0 },     // アイテムアイコン６
  { BPLIST_CHRRES_ITEM, BPLIST_PALRES_ITEM, BPLIST_CELRES_ITEM, 1, 0 },     // アイテムアイコン７

  { BPLIST_CHRRES_POKE1, BPLIST_PALRES_POKE, BPLIST_CELRES_POKE, 1, 1 },      // ポケモンアイコン１
  { BPLIST_CHRRES_POKE2, BPLIST_PALRES_POKE, BPLIST_CELRES_POKE, 1, 1 },      // ポケモンアイコン２
  { BPLIST_CHRRES_POKE3, BPLIST_PALRES_POKE, BPLIST_CELRES_POKE, 1, 1 },      // ポケモンアイコン３
  { BPLIST_CHRRES_POKE4, BPLIST_PALRES_POKE, BPLIST_CELRES_POKE, 1, 1 },      // ポケモンアイコン４
  { BPLIST_CHRRES_POKE5, BPLIST_PALRES_POKE, BPLIST_CELRES_POKE, 1, 1 },      // ポケモンアイコン５
  { BPLIST_CHRRES_POKE6, BPLIST_PALRES_POKE, BPLIST_CELRES_POKE, 1, 1 },      // ポケモンアイコン６

  { BPLIST_CHRRES_STATUS, BPLIST_PALRES_STATUS, BPLIST_CELRES_STATUS, 1, 1 }, // 状態異常アイコン１
  { BPLIST_CHRRES_STATUS, BPLIST_PALRES_STATUS, BPLIST_CELRES_STATUS, 1, 1 }, // 状態異常アイコン２
  { BPLIST_CHRRES_STATUS, BPLIST_PALRES_STATUS, BPLIST_CELRES_STATUS, 1, 1 }, // 状態異常アイコン３
  { BPLIST_CHRRES_STATUS, BPLIST_PALRES_STATUS, BPLIST_CELRES_STATUS, 1, 1 }, // 状態異常アイコン４
  { BPLIST_CHRRES_STATUS, BPLIST_PALRES_STATUS, BPLIST_CELRES_STATUS, 1, 1 }, // 状態異常アイコン５
  { BPLIST_CHRRES_STATUS, BPLIST_PALRES_STATUS, BPLIST_CELRES_STATUS, 1, 1 }, // 状態異常アイコン６

  { BPLIST_CHRRES_HPGAUGE, BPLIST_PALRES_HPGAUGE, BPLIST_CELRES_HPGAUGE, 2, 0 },  // ＨＰゲージ枠１
  { BPLIST_CHRRES_HPGAUGE, BPLIST_PALRES_HPGAUGE, BPLIST_CELRES_HPGAUGE, 2, 0 },  // ＨＰゲージ枠２
  { BPLIST_CHRRES_HPGAUGE, BPLIST_PALRES_HPGAUGE, BPLIST_CELRES_HPGAUGE, 2, 0 },  // ＨＰゲージ枠３
  { BPLIST_CHRRES_HPGAUGE, BPLIST_PALRES_HPGAUGE, BPLIST_CELRES_HPGAUGE, 2, 0 },  // ＨＰゲージ枠４
  { BPLIST_CHRRES_HPGAUGE, BPLIST_PALRES_HPGAUGE, BPLIST_CELRES_HPGAUGE, 2, 0 },  // ＨＰゲージ枠５
  { BPLIST_CHRRES_HPGAUGE, BPLIST_PALRES_HPGAUGE, BPLIST_CELRES_HPGAUGE, 2, 0 },  // ＨＰゲージ枠６

  { BPLIST_CHRRES_POKETYPE1, BPLIST_PALRES_TYPE, BPLIST_CELRES_TYPE, 1, 0 },    // ポケモンタイプアイコン１
  { BPLIST_CHRRES_POKETYPE2, BPLIST_PALRES_TYPE, BPLIST_CELRES_TYPE, 1, 0 },    // ポケモンタイプアイコン２
  { BPLIST_CHRRES_POKETYPE3, BPLIST_PALRES_TYPE, BPLIST_CELRES_TYPE, 1, 0 },    // ポケモンタイプアイコン３
  { BPLIST_CHRRES_POKETYPE4, BPLIST_PALRES_TYPE, BPLIST_CELRES_TYPE, 1, 0 },    // ポケモンタイプアイコン４

  { BPLIST_CHRRES_WAZATYPE1, BPLIST_PALRES_TYPE, BPLIST_CELRES_TYPE, 1, 0 },    // 技タイプアイコン１
  { BPLIST_CHRRES_WAZATYPE2, BPLIST_PALRES_TYPE, BPLIST_CELRES_TYPE, 1, 0 },    // 技タイプアイコン２
  { BPLIST_CHRRES_WAZATYPE3, BPLIST_PALRES_TYPE, BPLIST_CELRES_TYPE, 1, 0 },    // 技タイプアイコン３
  { BPLIST_CHRRES_WAZATYPE4, BPLIST_PALRES_TYPE, BPLIST_CELRES_TYPE, 1, 0 },    // 技タイプアイコン４
  { BPLIST_CHRRES_WAZATYPE5, BPLIST_PALRES_TYPE, BPLIST_CELRES_TYPE, 1, 0 },    // 技タイプアイコン５
  { BPLIST_CHRRES_WAZATYPE6, BPLIST_PALRES_TYPE, BPLIST_CELRES_TYPE, 1, 0 },    // 技タイプアイコン６
  { BPLIST_CHRRES_WAZATYPE7, BPLIST_PALRES_TYPE, BPLIST_CELRES_TYPE, 1, 0 },    // 技タイプアイコン７
  { BPLIST_CHRRES_WAZATYPE8, BPLIST_PALRES_TYPE, BPLIST_CELRES_TYPE, 1, 0 },    // 技タイプアイコン８
  { BPLIST_CHRRES_WAZATYPE9, BPLIST_PALRES_TYPE, BPLIST_CELRES_TYPE, 1, 0 },    // 技タイプアイコン９
  { BPLIST_CHRRES_WAZATYPE10, BPLIST_PALRES_TYPE, BPLIST_CELRES_TYPE, 1, 0 },    // 技タイプアイコン１０

  { BPLIST_CHRRES_BUNRUI, BPLIST_PALRES_TYPE, BPLIST_CELRES_TYPE, 1, 0 },			 // 分類アイコン
  { BPLIST_CHRRES_BUNRUI2, BPLIST_PALRES_TYPE, BPLIST_CELRES_TYPE, 1, 0 },     // 分類アイコン２
};

// ページ１のポケモンアイコンの座標
static const GFL_CLACTPOS P1_PokePos[] =
{
  { BPL_COMM_POKEICON_LPX, BPL_COMM_POKEICON1_PY }, { BPL_COMM_POKEICON_RPX, BPL_COMM_POKEICON2_PY },
  { BPL_COMM_POKEICON_LPX, BPL_COMM_POKEICON3_PY }, { BPL_COMM_POKEICON_RPX, BPL_COMM_POKEICON4_PY },
  { BPL_COMM_POKEICON_LPX, BPL_COMM_POKEICON5_PY }, { BPL_COMM_POKEICON_RPX, BPL_COMM_POKEICON6_PY },
};

// ページ１の状態異常アイコンの座標
static const GFL_CLACTPOS P1_StatusPos[] =
{
  { BPL_COMM_POKEICON_LPX+BPL_COMM_STATUSICON_X, BPL_COMM_POKEICON1_PY+BPL_COMM_STATUSICON_Y },
	{ BPL_COMM_POKEICON_RPX+BPL_COMM_STATUSICON_X, BPL_COMM_POKEICON2_PY+BPL_COMM_STATUSICON_Y },
	{ BPL_COMM_POKEICON_LPX+BPL_COMM_STATUSICON_X, BPL_COMM_POKEICON3_PY+BPL_COMM_STATUSICON_Y },
	{ BPL_COMM_POKEICON_RPX+BPL_COMM_STATUSICON_X, BPL_COMM_POKEICON4_PY+BPL_COMM_STATUSICON_Y },
  { BPL_COMM_POKEICON_LPX+BPL_COMM_STATUSICON_X, BPL_COMM_POKEICON5_PY+BPL_COMM_STATUSICON_Y },
	{ BPL_COMM_POKEICON_RPX+BPL_COMM_STATUSICON_X, BPL_COMM_POKEICON6_PY+BPL_COMM_STATUSICON_Y },
};

// ページ１のＨＰゲージ枠の座標
static const GFL_CLACTPOS P1_HPGaugePos[] =
{
  { BPL_COMM_POKEICON_LPX+BPL_COMM_HPGAUGE_X, BPL_COMM_POKEICON1_PY+BPL_COMM_HPGAUGE_Y },
	{ BPL_COMM_POKEICON_RPX+BPL_COMM_HPGAUGE_X, BPL_COMM_POKEICON2_PY+BPL_COMM_HPGAUGE_Y},
	{ BPL_COMM_POKEICON_LPX+BPL_COMM_HPGAUGE_X, BPL_COMM_POKEICON3_PY+BPL_COMM_HPGAUGE_Y },
	{ BPL_COMM_POKEICON_RPX+BPL_COMM_HPGAUGE_X, BPL_COMM_POKEICON4_PY+BPL_COMM_HPGAUGE_Y },
  { BPL_COMM_POKEICON_LPX+BPL_COMM_HPGAUGE_X, BPL_COMM_POKEICON5_PY+BPL_COMM_HPGAUGE_Y },
	{ BPL_COMM_POKEICON_RPX+BPL_COMM_HPGAUGE_X, BPL_COMM_POKEICON6_PY+BPL_COMM_HPGAUGE_Y },
};

static const GFL_CLACTPOS P_CHG_PokePos = { 128, 72 };  // 入れ替えページのポケモンアイコンの座標

static const GFL_CLACTPOS P2_PokePos = { 24, 12 };    // ページ２のポケモンアイコンの座標
static const GFL_CLACTPOS P2_StatusPos = { 192+6, 17+3 }; // ページ２の状態異常アイコンの座標
// ページ２のポケモンのタイプアイコンの座標
static const GFL_CLACTPOS P2_PokeTypePos[] =
{
  { 128+2, 16 },
  { 160+4, 16 }
};
// ページ２のアイテム名横のアイテムアイコンの座標
static const GFL_CLACTPOS P2_ItemIconPos = { 20, 132 };
// ページ２のＨＰゲージ枠の座標
static const GFL_CLACTPOS P2_HPGaugePos = { 223, 52 };

static const GFL_CLACTPOS P3_PokePos = { 24, 12 };    // ページ３のポケモンアイコンの座標
static const GFL_CLACTPOS P3_StatusPos = { 192+6, 17+3 }; // ページ３の状態異常アイコンの座標
// ページ３のポケモンのタイプアイコンの座標
static const GFL_CLACTPOS P3_PokeTypePos[] =
{
  { 128+2, 16 },
  { 160+4, 16 }
};
// ページ３の技のタイプアイコンの座標
static const GFL_CLACTPOS P3_WazaTypePos[] =
{
  {  24,  80 },
  { 152,  80 },
  {  24, 128 },
  { 152, 128 }
};

static const GFL_CLACTPOS P4_PokePos = { 24, 12 };    // ページ４のポケモンアイコンの座標
static const GFL_CLACTPOS P4_StatusPos = { 192+6, 17+3 }; // ページ４の状態異常アイコンの座標
// ページ４のポケモンのタイプアイコンの座標
static const GFL_CLACTPOS P4_PokeTypePos[] =
{
  { 128+2, 16 },
  { 160+4, 16 }
};
static const GFL_CLACTPOS P4_WazaTypePos = { 136, 40 }; // ページ４の技タイプアイコンの座標
static const GFL_CLACTPOS P4_WazaKindPos = { 24, 80 };  // ページ４の技分類アイコンの座標

static const GFL_CLACTPOS P5_PokePos = { 24, 12 };    // ページ５のポケモンアイコンの座標
static const GFL_CLACTPOS P5_StatusPos = { 192+6, 17+3 }; // ページ５の状態異常アイコンの座標
// ページ５のポケモンのタイプアイコンの座標
static const GFL_CLACTPOS P5_PokeTypePos[] =
{
  { 128+2, 16 },
  { 160+4, 16 }
};
// ページ５の技のタイプアイコンの座標
static const GFL_CLACTPOS P5_WazaTypePos[] =
{
  {  24,  80 },
  { 152,  80 },
  {  24, 128 },
  { 152, 128 },
  {  88, 176 }
};

static const GFL_CLACTPOS P6_PokePos = { 24, 12 };    // ページ６のポケモンアイコンの座標
static const GFL_CLACTPOS P6_StatusPos = { 192+6, 17+3 }; // ページ５の状態異常アイコンの座標
// ページ６のポケモンのタイプアイコンの座標
static const GFL_CLACTPOS P6_PokeTypePos[] =
{
  { 128+2, 16 },
  { 160+4, 16 }
};
static const GFL_CLACTPOS P6_WazaTypePos = { 136, 40 }; // ページ６の技タイプアイコンの座標
static const GFL_CLACTPOS P6_WazaKindPos = { 24, 80 };  // ページ６の技分類アイコンの座標



//--------------------------------------------------------------------------------------------
/**
 * 戦闘用ポケリストOBJ初期化
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BattlePokeList_ObjInit( BPLIST_WORK * wk )
{
  BPL_ClactResManInit( wk );
  BPL_ClactPokeLoad( wk );
  BPL_ClactStatusLoad( wk );
  BPL_ClactItemLoad( wk );
  BPL_ClactTypeLoad( wk );
//  BPL_ClactConditionLoad( wk );
  CursorResLoad( wk );
  HPGaugeResLoad( wk );
  BPL_ClactAddAll( wk );
  BPL_ClactCursorAdd( wk );

  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
}

//--------------------------------------------------------------------------------------------
/**
 * リソースマネージャー初期化
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_ClactResManInit( BPLIST_WORK * wk )
{
  u32 i;

  for( i=0; i<BPLIST_CHRRES_MAX; i++ ){
    wk->chrRes[i] = 0xffffffff;
  }
  for( i=0; i<BPLIST_PALRES_MAX; i++ ){
    wk->palRes[i] = 0xffffffff;
  }
  for( i=0; i<BPLIST_CELRES_MAX; i++ ){
    wk->celRes[i] = 0xffffffff;
  }

/*
  TCATS_RESOURCE_NUM_LIST crnl = { CHR_ID_MAX, PAL_ID_MAX, CEL_ID_MAX, ANM_ID_MAX, 0, 0 };
  CATS_SYS_PTR  csp = BattleWorkCATS_SYS_PTRGet( wk->dat->bw );

  wk->crp = CATS_ResourceCreate( csp );
  CATS_ClactSetInit( csp, wk->crp, BPL_CA_MAX+CURSOR_CLA_MAX );
  CATS_ResourceManagerInit( csp, wk->crp, &crnl );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンアイコンのグラフィックロード
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_ClactPokeLoad( BPLIST_WORK * wk )
{
  ARCHANDLE * ah;
  u32 * res;
  u16 i;
	u16	pos;

  ah = GFL_ARC_OpenDataHandle( ARCID_POKEICON, wk->dat->heap );

  // キャラ
  res = &wk->chrRes[BPLIST_CHRRES_POKE1];
  for( i=0; i<TEMOTI_POKEMAX; i++ ){
//		pos = BPLISTMAIN_GetListRow( wk, i );
		pos = i;
    if( wk->poke[pos].mons != 0 ){
      res[i] = GFL_CLGRP_CGR_Register(
                ah,
                POKEICON_GetCgxArcIndex(PP_GetPPPPointerConst(wk->poke[pos].pp)),
                FALSE,
                CLSYS_DRAW_SUB,
                wk->dat->heap );
    }else{
      res[i] = GFL_CLGRP_CGR_Register(
                ah,
                POKEICON_GetCgxArcIndexByMonsNumber(0,0,0),
                FALSE,
                CLSYS_DRAW_SUB,
                wk->dat->heap );
    }
  }

  // パレット
  wk->palRes[BPLIST_PALRES_POKE] = GFL_CLGRP_PLTT_RegisterComp(
                                    ah, POKEICON_GetPalArcIndex(),
                                    CLSYS_DRAW_SUB, PALOFS_POKEICON, wk->dat->heap );

  // セル・アニメ
  wk->celRes[BPLIST_CELRES_POKE] = GFL_CLGRP_CELLANIM_Register(
                                    ah,
                                    POKEICON_GetCellSubArcIndex(),
                                    POKEICON_GetAnmSubArcIndex(),
                                    wk->dat->heap );

  GFL_ARC_CloseDataHandle( ah );
}

//--------------------------------------------------------------------------------------------
/**
 * 状態異常アイコンのグラフィックロード
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_ClactStatusLoad( BPLIST_WORK * wk )
{
  ARCHANDLE * ah;

  ah = GFL_ARC_OpenDataHandle( ARCID_APP_MENU_COMMON, wk->dat->heap );

  // キャラ
  wk->chrRes[BPLIST_CHRRES_STATUS] = GFL_CLGRP_CGR_Register(
                                      ah, NARC_app_menu_common_p_st_ijou_NCGR,
                                      FALSE, CLSYS_DRAW_SUB, wk->dat->heap );
  // パレット
  wk->palRes[BPLIST_PALRES_STATUS] = GFL_CLGRP_PLTT_Register(
                                      ah, NARC_app_menu_common_p_st_ijou_NCLR,
                                      CLSYS_DRAW_SUB, PALOFS_STATUSICON, wk->dat->heap );
  // セル・アニメ
  wk->celRes[BPLIST_CELRES_STATUS] = GFL_CLGRP_CELLANIM_Register(
                                      ah,
                                      NARC_app_menu_common_p_st_ijou_32k_NCER,
                                      NARC_app_menu_common_p_st_ijou_32k_NANR,
                                      wk->dat->heap );

  GFL_ARC_CloseDataHandle( ah );
}

//--------------------------------------------------------------------------------------------
/**
 * タイプ/分類アイコンのグラフィックロード
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_ClactTypeLoad( BPLIST_WORK * wk )
{
  ARCHANDLE * ah;
  u32 i;

  ah = GFL_ARC_OpenDataHandle( ARCID_APP_MENU_COMMON, wk->dat->heap );

  // パレット
  wk->palRes[BPLIST_PALRES_TYPE] = GFL_CLGRP_PLTT_Register(
                                      ah, NARC_app_menu_common_p_st_type_NCLR,
                                      CLSYS_DRAW_SUB, PALOFS_TYPEICON, wk->dat->heap );
  // セル・アニメ
  wk->celRes[BPLIST_CELRES_TYPE] = GFL_CLGRP_CELLANIM_Register(
                                      ah,
                                      NARC_app_menu_common_p_st_type_32k_NCER,
                                      NARC_app_menu_common_p_st_type_32k_NANR,
                                      wk->dat->heap );
  // タイプアイコンキャラ
  for( i=BPLIST_CHRRES_POKETYPE1; i<=BPLIST_CHRRES_WAZATYPE10; i++ ){
    wk->chrRes[i] = GFL_CLGRP_CGR_Register(
                      ah, APP_COMMON_GetPokeTypeCharArcIdx(0),
                      FALSE, CLSYS_DRAW_SUB, wk->dat->heap );
  }

  // 分類アイコンキャラ
  wk->chrRes[BPLIST_CHRRES_BUNRUI] = GFL_CLGRP_CGR_Register(
                                      ah, APP_COMMON_GetWazaKindCharArcIdx(0),
                                      FALSE, CLSYS_DRAW_SUB, wk->dat->heap );
  wk->chrRes[BPLIST_CHRRES_BUNRUI2] = GFL_CLGRP_CGR_Register(
	                                      ah, APP_COMMON_GetWazaKindCharArcIdx(0),
		                                    FALSE, CLSYS_DRAW_SUB, wk->dat->heap );

  GFL_ARC_CloseDataHandle( ah );
}

//--------------------------------------------------------------------------------------------
/**
 * アイテムアイコンのグラフィックロード
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_ClactItemLoad( BPLIST_WORK * wk )
{
  ARCHANDLE * ah;

  ah = GFL_ARC_OpenDataHandle( ARCID_APP_MENU_COMMON, wk->dat->heap );

  // キャラ
  wk->chrRes[BPLIST_CHRRES_ITEM] = GFL_CLGRP_CGR_Register(
                                      ah, NARC_app_menu_common_item_icon_NCGR,
                                      FALSE, CLSYS_DRAW_SUB, wk->dat->heap );
  // パレット
  wk->palRes[BPLIST_PALRES_ITEM] = GFL_CLGRP_PLTT_Register(
                                      ah, NARC_app_menu_common_item_icon_NCLR,
                                      CLSYS_DRAW_SUB, PALOFS_ITEMICON, wk->dat->heap );
  // セル・アニメ
  wk->celRes[BPLIST_CELRES_ITEM] = GFL_CLGRP_CELLANIM_Register(
                                      ah,
                                      NARC_app_menu_common_item_icon_32k_NCER,
                                      NARC_app_menu_common_item_icon_32k_NANR,
                                      wk->dat->heap );

  GFL_ARC_CloseDataHandle( ah );
}

//--------------------------------------------------------------------------------------------
/**
 * コンディションのグラフィックロード
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
/*
static void BPL_ClactConditionLoad( BPLIST_WORK * wk )
{
  CATS_SYS_PTR  csp = BattleWorkCATS_SYS_PTRGet( wk->dat->bw );

  // パレット
  CATS_LoadResourcePlttWorkArc(
    wk->pfd, FADE_SUB_OBJ, csp, wk->crp, ARC_BPLIST_GRA,
    NARC_b_plist_gra_b_plist_obj_NCLR, 0, 1, NNS_G2D_VRAM_TYPE_2DSUB, PAL_ID_CND );
  // セル
  CATS_LoadResourceCellArc(
    csp, wk->crp, ARC_BPLIST_GRA, NARC_b_plist_gra_b_plist_obj_NCER, 0, CEL_ID_CND );
  // セルアニメ
  CATS_LoadResourceCellAnmArc(
    csp, wk->crp, ARC_BPLIST_GRA, NARC_b_plist_gra_b_plist_obj_NANR, 0, ANM_ID_CND );
  // キャラ
  CATS_LoadResourceCharArc(
    csp, wk->crp, ARC_BPLIST_GRA,
    NARC_b_plist_gra_b_plist_obj_NCGR, 0, NNS_G2D_VRAM_TYPE_2DSUB, CHR_ID_CND );
}
*/

static void CursorResLoad( BPLIST_WORK * wk )
{
  ARCHANDLE * ah;

  ah = GFL_ARC_OpenDataHandle( ARCID_BATTGRA, wk->dat->heap );

  // キャラ
  wk->chrRes[BPLIST_CHRRES_CURSOR] = GFL_CLGRP_CGR_Register(
                                      ah, NARC_battgra_wb_battle_w_cursor_NCGR,
                                      FALSE, CLSYS_DRAW_SUB, wk->dat->heap );
  // パレット
  wk->palRes[BPLIST_PALRES_CURSOR] = GFL_CLGRP_PLTT_RegisterEx(
                                      ah, NARC_battgra_wb_battle_w_obj_NCLR,
                                      CLSYS_DRAW_SUB, PALOFS_CURSOR, 1, PALSIZ_CURSOR, wk->dat->heap );

  // セル・アニメ
  wk->celRes[BPLIST_CELRES_CURSOR] = GFL_CLGRP_CELLANIM_Register(
                                      ah,
                                      NARC_battgra_wb_battle_w_cursor_NCER,
                                      NARC_battgra_wb_battle_w_cursor_NANR,
                                      wk->dat->heap );

  GFL_ARC_CloseDataHandle( ah );
}

static void HPGaugeResLoad( BPLIST_WORK * wk )
{
  ARCHANDLE * ah;

  ah = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), wk->dat->heap );

  // キャラ
  wk->chrRes[BPLIST_CHRRES_HPGAUGE] = GFL_CLGRP_CGR_Register(
                                        ah,
                                        APP_COMMON_GetHPBarBaseCharArcIdx(APP_COMMON_MAPPING_32K),
                                        FALSE, CLSYS_DRAW_SUB, wk->dat->heap );
  // パレット
/*
  wk->palRes[BPLIST_PALRES_HPGAUGE] = GFL_CLGRP_PLTT_RegisterEx(
                                        ah, APP_COMMON_GetHPBarBasePltArcIdx(),
                                        CLSYS_DRAW_SUB, PALOFS_HPGAUGE, 1, PALSIZ_HPGAUGE, wk->dat->heap );
*/
  wk->palRes[BPLIST_PALRES_HPGAUGE] = GFL_CLGRP_PLTT_Register(
                                        ah, APP_COMMON_GetHPBarBasePltArcIdx(),
                                        CLSYS_DRAW_SUB, PALOFS_HPGAUGE, wk->dat->heap );

  // セル・アニメ
  wk->celRes[BPLIST_CELRES_HPGAUGE] = GFL_CLGRP_CELLANIM_Register(
                                        ah,
                                        APP_COMMON_GetHPBarBaseCellArcIdx(APP_COMMON_MAPPING_32K),
                                        APP_COMMON_GetHPBarBaseAnimeArcIdx(APP_COMMON_MAPPING_32K),
                                        wk->dat->heap );

  GFL_ARC_CloseDataHandle( ah );
}



//--------------------------------------------------------------------------------------------
/**
 * セルアクター追加（個別）
 *
 * @param wk    ワーク
 * @param id    追加するセルアクターのID
 *
 * @return  セルアクターデータ
 */
//--------------------------------------------------------------------------------------------
static GFL_CLWK * BPL_ClactAdd( BPLIST_WORK * wk, const u32 * res )
{
  GFL_CLWK * clwk;
  GFL_CLWK_DATA dat;

  dat.pos_x = 0;
  dat.pos_y = 0;
  dat.anmseq = 0;
  dat.softpri = res[4];
  dat.bgpri = res[3];

  clwk = GFL_CLACT_WK_Create(
            wk->clunit,
            wk->chrRes[res[0]],
            wk->palRes[res[1]],
            wk->celRes[res[2]],
            &dat, CLSYS_DRAW_SUB, wk->dat->heap );

//  GFL_CLACT_WK_SetPlttOffs( clwk , prm->palNum , CLWK_PLTTOFFS_MODE_PLTT_TOP );
//  GFL_CLACT_WK_SetAutoAnmFlag( plateWork->pokeIcon , TRUE );

  return clwk;

/*
  TCATS_OBJECT_ADD_PARAM_S  prm;
  CATS_SYS_PTR  csp;

  csp = BattleWorkCATS_SYS_PTRGet( wk->dat->bw );

  prm.x = 0;
  prm.y = 0;
  prm.z = 0;

  prm.anm = 0;
  prm.pri = ObjParamEz[id][4];
  prm.pal = 0;
  prm.d_area = NNS_G2D_VRAM_TYPE_2DSUB;

  prm.id[0] = ObjParamEz[id][0];
  prm.id[1] = ObjParamEz[id][1];
  prm.id[2] = ObjParamEz[id][2];
  prm.id[3] = ObjParamEz[id][3];

  prm.bg_pri = 1;
  prm.vram_trans = 0;

  return CATS_ObjectAdd_S( csp, wk->crp, &prm );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * セルアクター追加（全て）
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_ClactAddAll( BPLIST_WORK * wk )
{
  u32 i;

  wk->clunit = GFL_CLACT_UNIT_Create( BPL_CA_MAX+BAPPTOOL_CURSOR_MAX, 0, wk->dat->heap );

  for( i=0; i<BPL_CA_MAX; i++ ){
    wk->clwk[i] = BPL_ClactAdd( wk, ObjParamEz[i] );
  }
  GFL_NET_ReloadIcon();
/*
  {
    u16 * pltt_vram = (u16 *)HW_DB_OBJ_PLTT;
    OS_Printf( "■■■ pal ■■■\n" );
    for( i=0; i<16; i++ ){
      OS_Printf( "0x%x, ", pltt_vram[14*16+i] );
    }
    OS_Printf( "\n■■■ pal ■■■\n" );
  }
*/

  PaletteWorkSet_VramCopy( wk->pfd, FADE_SUB_OBJ, 0, 0x1e0 );
  BPL_PokeIconPaletteChg( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * 戦闘用ポケリストOBJ削除
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BattlePokeList_ObjFree( BPLIST_WORK * wk )
{
  u32 i;

  for( i=0; i<BPL_CA_MAX; i++ ){
    GFL_CLACT_WK_Remove( wk->clwk[i] );
  }
  BAPPTOOL_DelCursor( wk->cpwk );

  for( i=0; i<BPLIST_CHRRES_MAX; i++ ){
    GFL_CLGRP_CGR_Release( wk->chrRes[i] );
  }
  for( i=0; i<BPLIST_PALRES_MAX; i++ ){
    GFL_CLGRP_PLTT_Release( wk->palRes[i] );
  }
  for( i=0; i<BPLIST_CELRES_MAX; i++ ){
    GFL_CLGRP_CELLANIM_Release( wk->celRes[i] );
  }

  GFL_CLACT_UNIT_Delete( wk->clunit );

/*
  CATS_SYS_PTR  csp;
  u32 i;

  csp = BattleWorkCATS_SYS_PTRGet( wk->dat->bw );

  for( i=0; i<BPL_CA_MAX; i++ ){
    CATS_ActorPointerDelete_S( wk->cap[i] );
  }

  BPL_CursorDel( wk );

  CATS_ResourceDestructor_S( csp, wk->crp );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * セルアクターを表示して座標変更
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_ClactOn( GFL_CLWK * clwk, const GFL_CLACTPOS * pos )
{
  GFL_CLACT_WK_SetDrawEnable( clwk, TRUE );
  GFL_CLACT_WK_SetPos( clwk, pos, CLSYS_DRAW_SUB );
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンアイコンのパレット切り替え
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_PokeIconPaletteChg( BPLIST_WORK * wk )
{
	u16	pos;
  s16 i;

  for( i=0; i<TEMOTI_POKEMAX; i++ ){
//		pos = BPLISTMAIN_GetListRow( wk, i );
		pos = i;
    if( wk->poke[pos].mons == 0 ){ continue; }
/*
    CATS_ObjectPaletteSetCap(
      wk->cap[BPL_CA_POKE1+i],
      PokeIconPalNumGet(wk->poke[i].mons,wk->poke[i].form,wk->poke[i].egg) );
*/
// CLWK_PLTTOFFS_MODE_OAM_COLOR
    GFL_CLACT_WK_SetPlttOffs(
      wk->clwk[BPL_CA_POKE1+i],
      POKEICON_GetPalNum(wk->poke[pos].mons,wk->poke[pos].form,wk->poke[pos].egg),
      CLWK_PLTTOFFS_MODE_PLTT_TOP );
  }
}

//--------------------------------------------------------------------------------------------
/**
 * タイプアイコン切り替え
 *
 * @param wk    ワーク
 * @param cap   セルアクターデータ
 * @param res_id  リソースID
 * @param type  タイプ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_TypeIconChange( BPLIST_WORK * wk, GFL_CLWK * clwk, u32 chrResID, u32 type )
{
  NNSG2dCharacterData * dat;
  void * buf;
  ARCHANDLE * ah;

  ah = GFL_ARC_OpenDataHandle( ARCID_APP_MENU_COMMON, wk->dat->heap );

  buf = GFL_ARCHDL_UTIL_LoadOBJCharacter(
          ah, APP_COMMON_GetPokeTypeCharArcIdx(type), FALSE, &dat, wk->dat->heap );
  GFL_CLGRP_CGR_Replace( wk->chrRes[chrResID], dat );
  GFL_HEAP_FreeMemory( buf );

  GFL_ARC_CloseDataHandle( ah );

  GFL_CLACT_WK_SetPlttOffs(
    clwk, APP_COMMON_GetPokeTypePltOffset(type), CLWK_PLTTOFFS_MODE_PLTT_TOP );
}

//--------------------------------------------------------------------------------------------
/**
 * 分類アイコン切り替え
 *
 * @param wk    ワーク
 * @param cap   セルアクターデータ
 * @param kind  分類
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_KindIconChange( BPLIST_WORK * wk, u32 objID, u32 chrResID, u32 kind )
{
  NNSG2dCharacterData * dat;
  void * buf;
  ARCHANDLE * ah;

	objID += wk->waza_kind_swap;
	chrResID += wk->waza_kind_swap;

  ah = GFL_ARC_OpenDataHandle( ARCID_APP_MENU_COMMON, wk->dat->heap );

  buf = GFL_ARCHDL_UTIL_LoadOBJCharacter(
          ah, APP_COMMON_GetWazaKindCharArcIdx(kind), FALSE, &dat, wk->dat->heap );
  GFL_CLGRP_CGR_Replace( wk->chrRes[chrResID], dat );
  GFL_HEAP_FreeMemory( buf );

  GFL_ARC_CloseDataHandle( ah );

  GFL_CLACT_WK_SetPlttOffs(
    wk->clwk[objID], APP_COMMON_GetWazaKindPltOffset(kind), CLWK_PLTTOFFS_MODE_PLTT_TOP );

	wk->waza_kind_swap ^= 1;
}

//--------------------------------------------------------------------------------------------
/**
 * 状態異常アイコン切り替え
 *
 * @param st    ステータス
 * @param cap   セルアクターデータ
 * @param x   X座標
 * @param y   Y座標
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_StIconPut( u16 st, GFL_CLWK * clwk, const GFL_CLACTPOS * pos )
{
  if( st == APP_COMMON_ST_ICON_NONE ){ return; }

  GFL_CLACT_WK_SetAnmSeq( clwk, st );
  BPL_ClactOn( clwk, pos );
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンのタイプアイコン切り替え
 *
 * @param wk    ワーク
 * @param pd    ポケモンデータ
 * @param pos   座標データ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_PokeTypeIconPut( BPLIST_WORK * wk, BPL_POKEDATA * pd, const GFL_CLACTPOS * pos )
{
	u16	objID, chrRes;

	if( wk->poke_type_swap == 0 ){
		objID  = BPL_CA_POKETYPE1;
		chrRes = BPLIST_CHRRES_POKETYPE1;
	}else{
		objID  = BPL_CA_POKETYPE3;
		chrRes = BPLIST_CHRRES_POKETYPE3;
	}
  BPL_TypeIconChange( wk, wk->clwk[objID], chrRes, pd->type1 );
  BPL_ClactOn( wk->clwk[objID], &pos[0] );
  if( pd->type1 != pd->type2 ){
    BPL_TypeIconChange( wk, wk->clwk[objID+1], chrRes+1, pd->type2 );
    BPL_ClactOn( wk->clwk[objID+1], &pos[1] );
  }

	wk->poke_type_swap ^= 1;
}

//--------------------------------------------------------------------------------------------
/**
 * アイテムアイコン切り替え
 *
 * @param item  ステータス
 * @param cap   セルアクターデータ
 * @param x   X座標
 * @param y   Y座標
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_ItemIconPut( u16 item, GFL_CLWK * clwk, const GFL_CLACTPOS * pos )
{
  if( item == ITEM_DUMMY_DATA ){ return; }

  if( ITEM_CheckMail( item ) == TRUE ){
    GFL_CLACT_WK_SetAnmSeq( clwk, 1 );
  }else{
    GFL_CLACT_WK_SetAnmSeq( clwk, 0 );
  }
  BPL_ClactOn( clwk, pos );
}

//--------------------------------------------------------------------------------------------
/**
 * アイテムアイコン切り替え
 *
 * @param cb    カスタムボールID
 * @param cap   セルアクターデータ
 * @param x   X座標
 * @param y   Y座標
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
/*
static void BPL_CustomBallIconPut( u8 cb, CATS_ACT_PTR cap, const int x, const int y )
{
  if( cb == 0 ){ return; }

  GFL_CLACT_WK_SetAnmSeq( cap, 2 );
  BPL_ClactOn( cap, x, y );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * ページごとにOBJをセット
 *
 * @param wk    ワーク
 * @param page  ページ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BattlePokeList_PageObjSet( BPLIST_WORK * wk, u32 page )
{
  u32 i;

  for( i=0; i<BPL_CA_MAX; i++ ){
    GFL_CLACT_WK_SetDrawEnable( wk->clwk[i], FALSE );
  }

  switch( page ){
  case BPLIST_PAGE_SELECT:	  // ポケモン選択ページ
	case BPLIST_PAGE_DEAD:			// 瀕死入れ替え選択ページ
    BPL_Page1ObjSet( wk );
    break;

  case BPLIST_PAGE_POKE_CHG:  // ポケモン入れ替えページ
    BPL_ChgPageObjSet( wk );
    break;

  case BPLIST_PAGE_MAIN:    // ステータスメインページ
    BPL_StMainPageObjSet( wk );
    break;

  case BPLIST_PAGE_WAZA_SEL:  // ステータス技選択ページ
    BPL_StWazaSelPageObjSet( wk );
    break;

  case BPLIST_PAGE_SKILL:   // ステータス技ページ
    BPL_Page4ObjSet( wk );
    break;

  case BPLIST_PAGE_PP_RCV:    // PP回復技選択ページ
    BPL_Page7ObjSet( wk );
    break;

  case BPLIST_PAGE_WAZASET_BS:  // ステータス技忘れ１ページ（戦闘技選択）
    BPL_Page5ObjSet( wk );
    break;

  case BPLIST_PAGE_WAZASET_BI:  // ステータス技忘れ２ページ（戦闘技詳細）
    BPL_Page6ObjSet( wk );
    break;

/*
  case BPLIST_PAGE_WAZASET_CS:  // ステータス技忘れ３ページ（コンテスト技詳細）
    BPL_Page9ObjSet( wk );
    break;

  case BPLIST_PAGE_WAZASET_CI:  // ステータス技忘れ４ページ（コンテスト技選択）
    BPL_Page8ObjSet( wk );
    break;
*/
  }
}

//--------------------------------------------------------------------------------------------
/**
 * ページ１のOBJをセット
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_Page1ObjSet( BPLIST_WORK * wk )
{
	u16	pos;
  s16 i;

  for( i=0; i<TEMOTI_POKEMAX; i++ ){
		pos = BPLISTMAIN_GetListRow( wk, i );

    if( wk->poke[pos].mons == 0 ){ continue; }
    // ポケモンアイコン
    BPL_ClactOn( wk->clwk[BPL_CA_POKE1+pos], &P1_PokePos[i] );
    // 状態異常アイコン
    BPL_StIconPut(
      wk->poke[pos].st, wk->clwk[BPL_CA_STATUS1+pos], &P1_StatusPos[i] );
    // ＨＰゲージ枠
    BPL_ClactOn( wk->clwk[BPL_CA_HPGAUGE1+pos], &P1_HPGaugePos[i] );
    // アイテムアイコン
    {
      GFL_CLACTPOS  actpos = P1_PokePos[i];
      actpos.x += BPL_COMM_ITEMICON_X;
      actpos.y += BPL_COMM_ITEMICON_Y;
      BPL_ItemIconPut(
        wk->poke[pos].item, wk->clwk[BPL_CA_ITEM1+pos], &actpos );
    }
  }

/*
  s32 i;

  for( i=0; i<TEMOTI_POKEMAX; i++ ){
    if( wk->poke[i].mons == 0 ){ continue; }
    // ポケモンアイコン
    BPL_ClactOn( wk->cap[BPL_CA_POKE1+i], P1_PokePos[i][0], P1_PokePos[i][1] );
    // 状態異常アイコン
    BPL_StIconPut(
      wk->poke[i].st, wk->cap[BPL_CA_STATUS1+i], P1_StatusPos[i][0], P1_StatusPos[i][1] );
    // アイテムアイコン
    BPL_ItemIconPut(
      wk->poke[i].item, wk->cap[BPL_CA_ITEM1+i], P1_PokePos[i][0]+8, P1_PokePos[i][1]+8 );
    // カスタムボールアイコン
    BPL_CustomBallIconPut(
      wk->poke[i].cb, wk->cap[BPL_CA_CB1+i], P1_PokePos[i][0]+16, P1_PokePos[i][1]+8 );
  }
*/
}

//--------------------------------------------------------------------------------------------
/**
 * 入れ替えページのOBJをセット
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_ChgPageObjSet( BPLIST_WORK * wk )
{
  BPL_POKEDATA * pd;
  GFL_CLACTPOS  pos;
	u32	row;

	row = BPLISTMAIN_GetListRow( wk, wk->dat->sel_poke );
  pd  = &wk->poke[row];
  pos = P_CHG_PokePos;

  // ポケモンアイコン
  BPL_ClactOn( wk->clwk[BPL_CA_POKE1+row], &pos );
  // アイテムアイコン
  pos.x += 8;
  pos.y += 8;
  BPL_ItemIconPut( pd->item, wk->clwk[BPL_CA_ITEM1+row], &pos );

/*
  BPL_POKEDATA * pd;
  u16 i;

  pd = &wk->poke[wk->dat->sel_poke];

  // ポケアイコン
  BPL_ClactOn( wk->cap[BPL_CA_POKE1+wk->dat->sel_poke], P_CHG_PokePos[0], P_CHG_PokePos[1] );
  // アイテムアイコン
  BPL_ItemIconPut(
    pd->item, wk->cap[BPL_CA_ITEM1+wk->dat->sel_poke], P_CHG_PokePos[0]+8, P_CHG_PokePos[1]+8 );
  // カスタムボールアイコン
  BPL_CustomBallIconPut(
    pd->cb, wk->cap[BPL_CA_CB1+wk->dat->sel_poke], P_CHG_PokePos[0]+16, P_CHG_PokePos[1]+8 );
*/
}


//--------------------------------------------------------------------------------------------
/**
 * ページ２のOBJをセット
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_StMainPageObjSet( BPLIST_WORK * wk )
{
  BPL_POKEDATA * pd;
  GFL_CLACTPOS  pos;
	u32	row;

	row = BPLISTMAIN_GetListRow( wk, wk->dat->sel_poke );
  pd  = &wk->poke[row];
  pos = P2_PokePos;

  // ポケモンアイコン
  BPL_ClactOn( wk->clwk[BPL_CA_POKE1+row], &pos );
  // 状態異常アイコン
  BPL_StIconPut(
    pd->st, wk->clwk[BPL_CA_STATUS1+row], &P2_StatusPos );
  // タイプ
  BPL_PokeTypeIconPut( wk, pd, P2_PokeTypePos );
  // アイテムアイコン
  pos.x += 8;
  pos.y += 8;
  BPL_ItemIconPut( pd->item, wk->clwk[BPL_CA_ITEM1+row], &pos );

  // アイテムアイコン（アイテム名の横の）
  BPL_ItemIconPut( pd->item, wk->clwk[BPL_CA_ITEM7], &P2_ItemIconPos );

  // ＨＰゲージ
  BPL_ClactOn( wk->clwk[BPL_CA_HPGAUGE1], &P2_HPGaugePos );
}

//--------------------------------------------------------------------------------------------
/**
 * ステータス技選択ページのOBJをセット
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_StWazaSelPageObjSet( BPLIST_WORK * wk )
{
  BPL_POKEDATA * pd;
  GFL_CLACTPOS  pos;
	u32	row;
//  u32 i;

	row = BPLISTMAIN_GetListRow( wk, wk->dat->sel_poke );
  pd  = &wk->poke[row];
  pos = P3_PokePos;

  // ポケモンアイコン
  BPL_ClactOn( wk->clwk[BPL_CA_POKE1+row], &pos );
  // 状態異常アイコン
  BPL_StIconPut(
    pd->st, wk->clwk[BPL_CA_STATUS1+row], &P3_StatusPos );
  // タイプ
  BPL_PokeTypeIconPut( wk, pd, P3_PokeTypePos );
  // アイテムアイコン
  pos.x += 8;
  pos.y += 8;
  BPL_ItemIconPut( pd->item, wk->clwk[BPL_CA_ITEM1+row], &pos );
  // 技タイプ
  BattlePokelist_WazaTypeSet( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * ページ４のOBJをセット
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_Page4ObjSet( BPLIST_WORK * wk )
{
  BPL_POKEDATA * pd;
  GFL_CLACTPOS  pos;
	u32	row;
  u32 i;

	row = BPLISTMAIN_GetListRow( wk, wk->dat->sel_poke );
  pd  = &wk->poke[row];
  pos = P4_PokePos;

  // ポケモンアイコン
  BPL_ClactOn( wk->clwk[BPL_CA_POKE1+row], &pos );
  // 状態異常アイコン
  BPL_StIconPut(
    pd->st, wk->clwk[BPL_CA_STATUS1+row], &P4_StatusPos );
  // タイプ
  BPL_PokeTypeIconPut( wk, pd, P4_PokeTypePos );
  // アイテムアイコン
  pos.x += 8;
  pos.y += 8;
  BPL_ItemIconPut( pd->item, wk->clwk[BPL_CA_ITEM1+row], &pos );
	// 技タイプアイコン
	if( wk->waza_type_swap == 0 ){
		BPL_ClactOn( wk->clwk[BPL_CA_WAZATYPE6+wk->dat->sel_wp], &P4_WazaTypePos );
	}else{
		BPL_ClactOn( wk->clwk[BPL_CA_WAZATYPE1+wk->dat->sel_wp], &P4_WazaTypePos );
	}
  // 分類アイコン
	if( wk->waza_kind_swap == 0 ){
	  BPL_ClactOn( wk->clwk[BPL_CA_BUNRUI], &P4_WazaKindPos );
	}else{
	  BPL_ClactOn( wk->clwk[BPL_CA_BUNRUI2], &P4_WazaKindPos );
	}
	BPL_KindIconChange( wk, BPL_CA_BUNRUI, BPLIST_CHRRES_BUNRUI, pd->waza[wk->dat->sel_wp].kind );
}

//--------------------------------------------------------------------------------------------
/**
 * ページ５のOBJをセット
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_Page5ObjSet( BPLIST_WORK * wk )
{
  BPL_POKEDATA * pd;
  GFL_CLACTPOS  pos;
	u32	row;

	row = BPLISTMAIN_GetListRow( wk, wk->dat->sel_poke );
  pd  = &wk->poke[row];
  pos = P5_PokePos;

  // ポケモンアイコン
  BPL_ClactOn( wk->clwk[BPL_CA_POKE1+row], &pos );
  // 状態異常アイコン
  BPL_StIconPut(
    pd->st, wk->clwk[BPL_CA_STATUS1+row], &P5_StatusPos );
  // タイプ
  BPL_PokeTypeIconPut( wk, pd, P5_PokeTypePos );
  // アイテムアイコン
  pos.x += 8;
  pos.y += 8;
  BPL_ItemIconPut( pd->item, wk->clwk[BPL_CA_ITEM1+row], &pos );
  // 技タイプアイコン
  BattlePokelist_WazaTypeSet( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * ページ６のOBJをセット
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_Page6ObjSet( BPLIST_WORK * wk )
{
  BPL_POKEDATA * pd;
  GFL_CLACTPOS  pos;
  u32 i;
	u32	row;

	row = BPLISTMAIN_GetListRow( wk, wk->dat->sel_poke );
  pd  = &wk->poke[row];
  pos = P6_PokePos;

  // ポケモンアイコン
  BPL_ClactOn( wk->clwk[BPL_CA_POKE1+row], &pos );
  // 状態異常アイコン
  BPL_StIconPut(
    pd->st, wk->clwk[BPL_CA_STATUS1+row], &P6_StatusPos );
  // タイプ
  BPL_PokeTypeIconPut( wk, pd, P6_PokeTypePos );
  // アイテムアイコン
  pos.x += 8;
  pos.y += 8;
  BPL_ItemIconPut( pd->item, wk->clwk[BPL_CA_ITEM1+row], &pos );
  // 技タイプアイコン
	if( wk->waza_type_swap == 0 ){
	  BPL_ClactOn( wk->clwk[BPL_CA_WAZATYPE6+wk->dat->sel_wp], &P6_WazaTypePos );
	}else{
	  BPL_ClactOn( wk->clwk[BPL_CA_WAZATYPE1+wk->dat->sel_wp], &P6_WazaTypePos );
	}
  // 分類アイコン
	if( wk->waza_kind_swap == 0 ){
	  BPL_ClactOn( wk->clwk[BPL_CA_BUNRUI], &P6_WazaKindPos );
	}else{
	  BPL_ClactOn( wk->clwk[BPL_CA_BUNRUI2], &P6_WazaKindPos );
	}
  if( wk->dat->sel_wp < 4 ){
    BPL_KindIconChange( wk, BPL_CA_BUNRUI, BPLIST_CHRRES_BUNRUI, pd->waza[wk->dat->sel_wp].kind );
  }else{
    BPL_KindIconChange(
      wk, BPL_CA_BUNRUI, BPLIST_CHRRES_BUNRUI, WAZADATA_GetParam(wk->dat->chg_waza,WAZAPARAM_DAMAGE_TYPE) );
  }
}

//--------------------------------------------------------------------------------------------
/**
 * ページ７のOBJをセット
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_Page7ObjSet( BPLIST_WORK * wk )
{
  BPL_POKEDATA * pd;
  GFL_CLACTPOS  pos;
	u32	row;

	row = BPLISTMAIN_GetListRow( wk, wk->dat->sel_poke );
  pd  = &wk->poke[row];
  pos = P5_PokePos;

  // ポケモンアイコン
  BPL_ClactOn( wk->clwk[BPL_CA_POKE1+row], &pos );
  // 状態異常アイコン
  BPL_StIconPut(
    pd->st, wk->clwk[BPL_CA_STATUS1+row], &P5_StatusPos );
  // タイプ
  BPL_PokeTypeIconPut( wk, pd, P5_PokeTypePos );
  // アイテムアイコン
  pos.x += 8;
  pos.y += 8;
  BPL_ItemIconPut( pd->item, wk->clwk[BPL_CA_ITEM1+row], &pos );
  // 技タイプアイコン
  BattlePokelist_WazaTypeSet( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * ページ８のOBJをセット
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
/*
static void BPL_Page8ObjSet( BPLIST_WORK * wk )
{
  BPL_POKEDATA * pd = &wk->poke[wk->dat->sel_poke];

  // ポケアイコン
  BPL_ClactOn( wk->cap[BPL_CA_POKE1+wk->dat->sel_poke], P6_PokePos[0], P6_PokePos[1] );

  // 技タイプ
  BPL_ClactOn(
    wk->cap[BPL_CA_WAZATYPE1+wk->dat->sel_wp], P8_WazaTypePos[0], P8_WazaTypePos[1] );
  // アイテムアイコン
  BPL_ItemIconPut(
    pd->item, wk->cap[BPL_CA_ITEM1+wk->dat->sel_poke], P6_PokePos[0]+8, P6_PokePos[1]+8 );
  // カスタムボールアイコン
  BPL_CustomBallIconPut(
    pd->cb, wk->cap[BPL_CA_CB1+wk->dat->sel_poke], P6_PokePos[0]+16, P6_PokePos[1]+8 );

  // コンディション
  BPL_EzConditionPut( wk );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * ページ９のOBJをセット
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
/*
static void BPL_Page9ObjSet( BPLIST_WORK * wk )
{
  BPL_POKEDATA * pd = &wk->poke[wk->dat->sel_poke];

  // ポケアイコン
  BPL_ClactOn( wk->cap[BPL_CA_POKE1+wk->dat->sel_poke], P6_PokePos[0], P6_PokePos[1] );

  // 技タイプ
//  BPL_ClactOn(
//    wk->cap[BPL_CA_WAZATYPE1+wk->dat->sel_wp], P8_WazaTypePos[0], P8_WazaTypePos[1] );

  // アイテムアイコン
  BPL_ItemIconPut(
    pd->item, wk->cap[BPL_CA_ITEM1+wk->dat->sel_poke], P6_PokePos[0]+8, P6_PokePos[1]+8 );
  // カスタムボールアイコン
  BPL_CustomBallIconPut(
    pd->cb, wk->cap[BPL_CA_CB1+wk->dat->sel_poke], P6_PokePos[0]+16, P6_PokePos[1]+8 );

  // 技タイプ
  BattlePokelist_WazaTypeSet( wk );

  // コンディション
  BPL_EzConditionPut( wk );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 技アイコンセット：戦闘 or コンテスト
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BattlePokelist_WazaTypeSet( BPLIST_WORK * wk )
{
  BPL_POKEDATA * pd;
  u32 i;
	u16	objID, chrRes;

  pd = &wk->poke[ BPLISTMAIN_GetListRow(wk,wk->dat->sel_poke) ];

	if( wk->waza_type_swap == 0 ){
		objID  = BPL_CA_WAZATYPE1;
		chrRes = BPLIST_CHRRES_WAZATYPE1;
	}else{
		objID  = BPL_CA_WAZATYPE6;
		chrRes = BPLIST_CHRRES_WAZATYPE6;
	}

  // 技タイプ
  for( i=0; i<4; i++ ){
    if( pd->waza[i].id == 0 ){ continue; }

    BPL_TypeIconChange( wk, wk->clwk[objID+i], chrRes+i, pd->waza[i].type );
    BPL_ClactOn( wk->clwk[objID+i], &P5_WazaTypePos[i] );
  }

  if( wk->dat->chg_waza != 0 && wk->dat->mode == BPL_MODE_WAZASET ){
      BPL_TypeIconChange(
        wk, wk->clwk[objID+i], chrRes+i,
        WAZADATA_GetParam( wk->dat->chg_waza, WAZAPARAM_TYPE ) );
      BPL_ClactOn( wk->clwk[objID+i], &P5_WazaTypePos[i] );
  }

	wk->waza_type_swap ^= 1;
}


//--------------------------------------------------------------------------------------------
/**
 * ポケモンアイコンアニメ切り替え
 *
 * @param cap   セルアクターデータ
 * @param anm   アニメ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_PokeIconAnmChg( GFL_CLWK * clwk, u16 anm )
{
/*
  if( CATS_ObjectAnimeSeqGetCap( cap ) == anm ){ return; }
  GFL_CLACT_WK_SetAnmFrameCap( cap, 0 );
  GFL_CLACT_WK_SetAnmSeq( cap, anm );
*/
  if( GFL_CLACT_WK_GetAnmSeq( clwk ) == anm ){ return; }

  GFL_CLACT_WK_SetAnmFrame( clwk, 0 );
  GFL_CLACT_WK_SetAnmSeq( clwk, anm );
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンアイコンアニメチェック
 *
 * @param pd    ポケモンデータ
 *
 * @return  アニメ番号
 */
//--------------------------------------------------------------------------------------------
static u8 BPL_PokeIconAnmCheck( BPL_POKEDATA * pd )
{
  if( pd->hp == 0 ){
    return POKEICON_ANM_DEATH;
  }

  if( pd->st != APP_COMMON_ST_ICON_NONE && pd->st != APP_COMMON_ST_ICON_HINSI ){
    return POKEICON_ANM_STCHG;
  }

  if( pd->hp == pd->mhp ){
    return POKEICON_ANM_HPMAX;
  }

  switch( GAUGETOOL_GetGaugeDottoColor( pd->hp, pd->mhp ) ){
  case GAUGETOOL_HP_DOTTO_GREEN:    // 緑
    return POKEICON_ANM_HPGREEN;
  case GAUGETOOL_HP_DOTTO_YELLOW: // 黄
    return POKEICON_ANM_HPYERROW;
  case GAUGETOOL_HP_DOTTO_RED:      // 赤
    return POKEICON_ANM_HPRED;
  }

  return POKEICON_ANM_DEATH;

  return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンアイコンアニメ
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BPL_PokeIconAnime( BPLIST_WORK * wk )
{
  u16 i;
  u8	anm;
	u8	pos;

//  for( i=0; i<PokeParty_GetPokeCount(wk->dat->pp); i++ ){
  for( i=0; i<TEMOTI_POKEMAX; i++ ){
		pos = BPLISTMAIN_GetListRow( wk, i );

    if( wk->poke[pos].mons == 0 ){ continue; }

    anm = BPL_PokeIconAnmCheck( &wk->poke[pos] );

    BPL_PokeIconAnmChg( wk->clwk[BPL_CA_POKE1+pos], anm );
    GFL_CLACT_WK_AddAnmFrame( wk->clwk[BPL_CA_POKE1+pos], FX32_ONE );
//    GFL_CLACT_WK_AddAnmFrameNumCap( wk->cap[BPL_CA_POKE1+i], FX32_ONE );
  }
}

//============================================================================================
//  簡易コンディション
//============================================================================================
/*
#define EZCON1_MIN_X  ( 144 )
#define EZCON1_MAX_X  ( 144 )
#define EZCON1_MIN_Y  ( 24 )
#define EZCON1_MAX_Y  ( 2 )

#if AFTER_MASTER_070409_59_EUR_FIX

#define EZCON2_MIN_X  ( 144 )
#define EZCON2_MAX_X  ( 164 )
#define EZCON2_MIN_Y  ( 24 )
#define EZCON2_MAX_Y  ( 16 )

#define EZCON3_MIN_X  ( 144 )
#define EZCON3_MAX_X  ( 156 )
#define EZCON3_MIN_Y  ( 24 )
#define EZCON3_MAX_Y  ( 41 )

#define EZCON4_MIN_X  ( 143 )
#define EZCON4_MAX_X  ( 131 )
#define EZCON4_MIN_Y  ( 24 )
#define EZCON4_MAX_Y  ( 41 )

#define EZCON5_MIN_X  ( 143 )
#define EZCON5_MAX_X  ( 123 )
#define EZCON5_MIN_Y  ( 24 )
#define EZCON5_MAX_Y  ( 16 )

#else

#define EZCON2_MIN_X  ( 144 )
#define EZCON2_MAX_X  ( 167 )
#define EZCON2_MIN_Y  ( 24 )
#define EZCON2_MAX_Y  ( 18 )

#define EZCON3_MIN_X  ( 144 )
#define EZCON3_MAX_X  ( 159 )
#define EZCON3_MIN_Y  ( 24 )
#define EZCON3_MAX_Y  ( 47 )

#define EZCON4_MIN_X  ( 143 )
#define EZCON4_MAX_X  ( 128 )
#define EZCON4_MIN_Y  ( 24 )
#define EZCON4_MAX_Y  ( 47 )

#define EZCON5_MIN_X  ( 143 )
#define EZCON5_MAX_X  ( 120 )
#define EZCON5_MIN_Y  ( 24 )
#define EZCON5_MAX_Y  ( 18 )

#endif

#define EZCND_MAX_RPM ( 300 )
#define EZCND_DEF_RPM ( 44 )    // 300 = 256 + EZCND_DEF_RPM
*/

//--------------------------------------------------------------------------------------------
/**
 * 簡易コンディションの表示座標取得
 *
 * @param prm   パラメータ
 * @param max   最大値
 * @param min   最小値
 *
 * @return  表示座標
 */
//--------------------------------------------------------------------------------------------
/*
static s16 BPL_EzCndPosGet( u32 prm, s16 max, s16 min )
{
  u32 dot;

  prm += EZCND_DEF_RPM;

  if( min > max ){
    dot = ( ( min - max ) * prm ) << 16;
    dot = ( dot / EZCND_MAX_RPM ) >> 16;
    return min + (s16)dot * -1;
  }

  dot = ( ( max - min ) * prm ) << 16;
  dot = ( dot / EZCND_MAX_RPM ) >> 16;
  return min + (s16)dot;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 簡易コンディション表示
 *
 * @param wk    戦闘ポケリストのワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
/*
static void BPL_EzConditionPut( BPLIST_WORK * wk )
{
  BPL_POKEDATA * pd;
  u16 i;

  pd = &wk->poke[wk->dat->sel_poke];

  BPL_PokeIconAnmChg( wk->cap[BPL_CA_CND1], 0 );
  BPL_PokeIconAnmChg( wk->cap[BPL_CA_CND2], 1 );
  BPL_PokeIconAnmChg( wk->cap[BPL_CA_CND3], 3 );
  BPL_PokeIconAnmChg( wk->cap[BPL_CA_CND4], 4 );
  BPL_PokeIconAnmChg( wk->cap[BPL_CA_CND5], 2 );

  BPL_ClactOn(
    wk->cap[BPL_CA_CND1],
    BPL_EzCndPosGet( pd->style, EZCON1_MAX_X, EZCON1_MIN_X ),
    BPL_EzCndPosGet( pd->style, EZCON1_MAX_Y, EZCON1_MIN_Y ) );
  BPL_ClactOn(
    wk->cap[BPL_CA_CND2],
    BPL_EzCndPosGet( pd->beautiful, EZCON2_MAX_X, EZCON2_MIN_X ),
    BPL_EzCndPosGet( pd->beautiful, EZCON2_MAX_Y, EZCON2_MIN_Y ) );
  BPL_ClactOn(
    wk->cap[BPL_CA_CND3],
    BPL_EzCndPosGet( pd->cute, EZCON3_MAX_X, EZCON3_MIN_X ),
    BPL_EzCndPosGet( pd->cute, EZCON3_MAX_Y, EZCON3_MIN_Y ) );
  BPL_ClactOn(
    wk->cap[BPL_CA_CND4],
    BPL_EzCndPosGet( pd->clever, EZCON4_MAX_X, EZCON4_MIN_X ),
    BPL_EzCndPosGet( pd->clever, EZCON4_MAX_Y, EZCON4_MIN_Y ) );
  BPL_ClactOn(
    wk->cap[BPL_CA_CND5],
    BPL_EzCndPosGet( pd->strong, EZCON5_MAX_X, EZCON5_MIN_X ),
    BPL_EzCndPosGet( pd->strong, EZCON5_MAX_Y, EZCON5_MIN_Y ) );
}
*/


//============================================================================================
//  カーソル
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル追加
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_ClactCursorAdd( BPLIST_WORK * wk )
{
  BAPPTOOL_AddCursor(
    wk->cpwk,
    wk->clunit,
    wk->chrRes[BPLIST_CHRRES_CURSOR],
    wk->palRes[BPLIST_PALRES_CURSOR],
    wk->celRes[BPLIST_CELRES_CURSOR] );

  BAPPTOOL_VanishCursor( wk->cpwk, FALSE );

/*
  CATS_SYS_PTR csp;
  BCURSOR_PTR cursor;

  csp = BattleWorkCATS_SYS_PTRGet( wk->dat->bw );

  BCURSOR_ResourceLoad(
    csp, wk->crp, wk->pfd, wk->dat->heap, CHR_ID_CURSOR, PAL_ID_CURSOR, CEL_ID_CURSOR, ANM_ID_CURSOR );

  cursor = BCURSOR_ActorCreate(
        csp, wk->crp, wk->dat->heap,
        CHR_ID_CURSOR, PAL_ID_CURSOR, CEL_ID_CURSOR, ANM_ID_CURSOR, 0, 1 );

  BAPP_CursorMvWkSetBCURSOR_PTR( wk->cmv_wk, cursor );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル削除
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_CursorDel( BPLIST_WORK * wk )
{
  BAPPTOOL_DelCursor( wk->cpwk );

/*
  BCURSOR_ActorDelete( BAPP_CursorMvWkGetBCURSOR_PTR( wk->cmv_wk ) );
  BCURSOR_ResourceFree(
    wk->crp, CHR_ID_CURSOR, PAL_ID_CURSOR, CEL_ID_CURSOR, ANM_ID_CURSOR );
*/
}

/*
// ポケモン選択画面移動テーブル
static const POINTSEL_WORK P1_CursorPosTbl[] =
{
  {   8,   8, 120,  40, 6, 2, 6, 1 },   // 0
  { 136,  16, 248,  48, 4, 3, 0, 2 },   // 1
  {   8,  56, 120,  88, 0, 4, 1, 3 },   // 2
  { 136,  64, 248,  96, 1, 5, 2, 4 },   // 3
  {   8, 104, 120, 136, 2, 1, 3, 5 },   // 4
  { 136, 112, 248, 144, 3, 6, 4, 6 },   // 5
  { 224, 160, 248, 184, 5, 0, 5, 0 },   // 6（戻る）
};
static const POINTSEL_WORK Chg_CursorPosTbl[] =
{
  {  16,  16, 240, 136, 0, BAPP_CMV_RETBIT|1, 0, 0 },   // 0 : いれかえる
  {   8, 160,  96, 184, 0, 1, 1, 2 },           // 1 : 強さを見る
  { 112, 160, 200, 184, 0, 2, 1, 3 },           // 2 : 技を見る
  { 224, 160, 248, 184, 0, 3, 2, 3 },           // 3 : 戻る
};
static const POINTSEL_WORK StMain_CursorPosTbl[] =
{
  {   8, 160,  32, 184, 0, 0, 0, 1 },   // 0 : 上
  {  48, 160,  72, 184, 1, 1, 0, 2 },   // 1 : 下
  { 104, 160, 192, 184, 2, 2, 1, 3 },   // 2 : 技を見る
  { 224, 160, 248, 184, 3, 3, 2, 3 },   // 3 : 戻る
};
static const POINTSEL_WORK StWazaSel_CursorPosTbl[] =
{
  {   8,  56, 120,  88, 0, 2, 0, 1 },   // 0 : 技１
  { 136,  56, 248,  88, 1, 3, 0, 1 },   // 1 : 技２
  {   8, 104, 120, 136, 0, BAPP_CMV_RETBIT|4, 2, 3 },   // 2 : 技３
  { 136, 104, 248, 136, 1, BAPP_CMV_RETBIT|7, 2, 3 },   // 3 : 技４
  {   8, 160,  32, 184, 2, 4, 4, 5 },   // 4 : 上
  {  48, 160,  72, 184, 2, 5, 4, 6 },   // 5 : 下
  { 104, 160, 192, 184, 3, 6, 5, 7 },   // 6 : 強さを見る
  { 224, 160, 248, 184, 3, 7, 6, 7 },   // 7 : 戻る
};
static const POINTSEL_WORK StWazaInfo_CursorPosTbl[] =
{
  {  92, 157, 124, 165, 0, 2, 0, 1 },   // 0 : 技１
  { 132, 157, 164, 165, 1, 3, 0, 4 },   // 1 : 技２
  {  92, 173, 124, 181, 0, 2, 2, 3 },   // 2 : 技３
  { 132, 173, 164, 181, 1, 3, 2, 4 },   // 3 : 技４
  { 224, 160, 248, 184, 4, 4, BAPP_CMV_RETBIT|3, 4 },   // 4 : 戻る
};
static const POINTSEL_WORK DelSel_CursorPosTbl[] =
{
  {   8,  56, 120,  88, 5, 2, 0, 1 },           // 0 : 技１画面へ
  { 136,  56, 248,  88, 5, 3, 0, 1 },           // 1 : 技２画面へ
  {   8, 104, 120, 136, 0, 4, 2, 3 },           // 2 : 技３画面へ
  { 136, 104, 248, 136, 1, 6, 2, 3 },           // 3 : 技４画面へ
  {  72, 152, 184, 184, 2, 4, 4, 6 },           // 4 : 技５画面へ
  { 192,   8, 248,  24, 5, BAPP_CMV_RETBIT|1, 0, 5 },   // 5 : 戦闘<->コンテストの切り替え
  { 224, 160, 248, 184, 3, 6, 4, 6 },           // 6 : 戻る
};

static const POINTSEL_WORK DelInfo_CursorPosTbl[] =
{
  {   8, 160, 200, 184, 1, 0, 0, 2 },           // 0 : わすれる
  { 192,   8, 248,  24, 1, BAPP_CMV_RETBIT|2, 0, 1 },   // 1 : 戦闘<->コンテストの切り替え
  { 224, 160, 248, 184, 1, 2, 0, 2 },           // 2 : 戻る
};

static const POINTSEL_WORK PPRcv_CursorPosTbl[] =
{
  {   8,  56, 120,  88, 0, 2, 0, 1 },           // 0 : 技１回復
  { 136,  56, 248,  88, 1, 3, 0, 1 },           // 1 : 技２回復
  {   8, 104, 120, 136, 0, 4, 2, 3 },           // 2 : 技３回復
  { 136, 104, 248, 136, 1, 4, 2, 3 },           // 3 : 技４回復
  { 224, 160, 248, 184, BAPP_CMV_RETBIT|3, 4, 4, 4 },   // 4 : 戻る
};

static const POINTSEL_WORK * const CursorPosTable[] = {
  P1_CursorPosTbl,      // ポケモン選択ページ
  Chg_CursorPosTbl,     // ポケモン入れ替えページ
  StMain_CursorPosTbl,    // ステータスメインページ
  StWazaSel_CursorPosTbl,   // ステータス技選択ページ
  StWazaInfo_CursorPosTbl,  // ステータス技詳細ページ
  PPRcv_CursorPosTbl,     // PP回復技選択ページ
  DelSel_CursorPosTbl,    // ステータス技忘れ１ページ（戦闘技選択）
  DelInfo_CursorPosTbl,   // ステータス技忘れ２ページ（戦闘技詳細）
  DelSel_CursorPosTbl,    // ステータス技忘れ３ページ（コンテスト技選択）
  DelInfo_CursorPosTbl,   // ステータス技忘れ４ページ（コンテスト技詳細）
};
*/
/*

/*
#define WAZADEL_SEL_MV_TBL_N  ( 0x7f )  // 技忘れの技選択のカーソル移動テーブル（通常）
#define WAZADEL_SEL_MV_TBL_C  ( 0x5f )  // 技忘れの技選択のカーソル移動テーブル（コンテストなし）
#define WAZADEL_MAIN_MV_TBL_N ( 7 ) // 技忘れの詳細のカーソル移動テーブル（通常）
#define WAZADEL_MAIN_MV_TBL_C ( 5 ) // 技忘れの詳細のカーソル移動テーブル（コンテストなし）
*/

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動テーブル作成（技忘れ技選択ページ）
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
/*
static void BPL_WazaDelSelCursorMvTblMake( BPLIST_WORK * wk )
{
  if( wk->ev_contest == 0 ){
    BAPP_CursorMvWkSetMvTbl( wk->cmv_wk, WAZADEL_SEL_MV_TBL_C );
  }else{
    BAPP_CursorMvWkSetMvTbl( wk->cmv_wk, WAZADEL_SEL_MV_TBL_N );
  }
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動テーブル作成（技忘れ技決定ページ）
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
/*
static void BPL_WazaDelMainCursorMvTblMake( BPLIST_WORK * wk )
{
  if( wk->ev_contest == 0 ){
    BAPP_CursorMvWkSetMvTbl( wk->cmv_wk, WAZADEL_MAIN_MV_TBL_C );
  }else{
    BAPP_CursorMvWkSetMvTbl( wk->cmv_wk, WAZADEL_MAIN_MV_TBL_N );
  }
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソルセット
 *
 * @param wk    ワーク
 * @param page  ページID
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
/*
void BattlePokeList_CursorMoveSet( BPLIST_WORK * wk, u8 page )
{
  BAPP_CursorMvWkSetPoint( wk->cmv_wk, CursorPosTable[page] );

  switch( page ){

  case BPLIST_PAGE_SELECT:    // ポケモン選択ページ
      BAPP_CursorMvWkSetPos( wk->cmv_wk, wk->dat->sel_poke );
    wk->chg_page_cp = 0;
    wk->dat->sel_wp = 0;
    break;

  case BPLIST_PAGE_POKE_CHG:    // ポケモン入れ替えページ
    BAPP_CursorMvWkSetPos( wk->cmv_wk, wk->chg_page_cp );
    wk->dat->sel_wp = 0;
    break;

  case BPLIST_PAGE_WAZA_SEL:    // ステータス技選択ページ
  case BPLIST_PAGE_SKILL:     // ステータス技詳細ページ
    BAPP_CursorMvWkSetPos( wk->cmv_wk, wk->dat->sel_wp );
    break;

  case BPLIST_PAGE_WAZASET_BS:  // ステータス技忘れ１ページ（戦闘技選択）
//  case BPLIST_PAGE_WAZASET_CS:  // ステータス技忘れ３ページ（コンテスト技選択）
    BPL_WazaDelSelCursorMvTblMake( wk );
    BAPP_CursorMvWkSetPos( wk->cmv_wk, wk->wws_page_cp );
    break;

  case BPLIST_PAGE_WAZASET_BI:  // ステータス技忘れ２ページ（戦闘技詳細）
//  case BPLIST_PAGE_WAZASET_CI:  // ステータス技忘れ４ページ（コンテスト技詳細）
    BPL_WazaDelMainCursorMvTblMake( wk );
    BAPP_CursorMvWkSetPos( wk->cmv_wk, wk->wwm_page_cp );
    break;
  }
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル非表示
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
/*
void BattlePokeList_CursorOff( BPLIST_WORK * wk )
{
  BAPP_CursorMvWkSetFlag( wk->cmv_wk, 0 );
  BAPP_CursorMvWkPosInit( wk->cmv_wk );
  BCURSOR_OFF( BAPP_CursorMvWkGetBCURSOR_PTR( wk->cmv_wk ) );
}
*/

void BPLISTOBJ_MoveDeadChg( BPLIST_WORK * wk, u8 num, s8 mv )
{
	GFL_CLACTPOS	pos;
	u32	row;

	row = BPLISTMAIN_GetListRow( wk, num );

  GFL_CLACT_WK_GetPos( wk->clwk[BPL_CA_ITEM1+row], &pos, CLSYS_DRAW_SUB );
	pos.x += mv;
  GFL_CLACT_WK_SetPos( wk->clwk[BPL_CA_ITEM1+row], &pos, CLSYS_DRAW_SUB );

  GFL_CLACT_WK_GetPos( wk->clwk[BPL_CA_POKE1+row], &pos, CLSYS_DRAW_SUB );
	pos.x += mv;
  GFL_CLACT_WK_SetPos( wk->clwk[BPL_CA_POKE1+row], &pos, CLSYS_DRAW_SUB );

  GFL_CLACT_WK_GetPos( wk->clwk[BPL_CA_STATUS1+row], &pos, CLSYS_DRAW_SUB );
	pos.x += mv;
  GFL_CLACT_WK_SetPos( wk->clwk[BPL_CA_STATUS1+row], &pos, CLSYS_DRAW_SUB );

  GFL_CLACT_WK_GetPos( wk->clwk[BPL_CA_HPGAUGE1+row], &pos, CLSYS_DRAW_SUB );
	pos.x += mv;
  GFL_CLACT_WK_SetPos( wk->clwk[BPL_CA_HPGAUGE1+row], &pos, CLSYS_DRAW_SUB );
}

void BPLISTOBJ_ChgDeadSel( BPLIST_WORK * wk, u8 num1, u8 num2 )
{
	GFL_CLACTPOS	pos1, pos2;
	u32	row1, row2;

	row1 = BPLISTMAIN_GetListRow( wk, num1 );
	row2 = BPLISTMAIN_GetListRow( wk, num2 );

  GFL_CLACT_WK_GetPos( wk->clwk[BPL_CA_ITEM1+row1], &pos1, CLSYS_DRAW_SUB );
  GFL_CLACT_WK_GetPos( wk->clwk[BPL_CA_ITEM1+row2], &pos2, CLSYS_DRAW_SUB );
  GFL_CLACT_WK_SetPos( wk->clwk[BPL_CA_ITEM1+row1], &pos2, CLSYS_DRAW_SUB );
  GFL_CLACT_WK_SetPos( wk->clwk[BPL_CA_ITEM1+row2], &pos1, CLSYS_DRAW_SUB );

  GFL_CLACT_WK_GetPos( wk->clwk[BPL_CA_POKE1+row1], &pos1, CLSYS_DRAW_SUB );
  GFL_CLACT_WK_GetPos( wk->clwk[BPL_CA_POKE1+row2], &pos2, CLSYS_DRAW_SUB );
  GFL_CLACT_WK_SetPos( wk->clwk[BPL_CA_POKE1+row1], &pos2, CLSYS_DRAW_SUB );
  GFL_CLACT_WK_SetPos( wk->clwk[BPL_CA_POKE1+row2], &pos1, CLSYS_DRAW_SUB );

  GFL_CLACT_WK_GetPos( wk->clwk[BPL_CA_STATUS1+row1], &pos1, CLSYS_DRAW_SUB );
  GFL_CLACT_WK_GetPos( wk->clwk[BPL_CA_STATUS1+row2], &pos2, CLSYS_DRAW_SUB );
  GFL_CLACT_WK_SetPos( wk->clwk[BPL_CA_STATUS1+row1], &pos2, CLSYS_DRAW_SUB );
  GFL_CLACT_WK_SetPos( wk->clwk[BPL_CA_STATUS1+row2], &pos1, CLSYS_DRAW_SUB );

  GFL_CLACT_WK_GetPos( wk->clwk[BPL_CA_HPGAUGE1+row1], &pos1, CLSYS_DRAW_SUB );
  GFL_CLACT_WK_GetPos( wk->clwk[BPL_CA_HPGAUGE1+row2], &pos2, CLSYS_DRAW_SUB );
  GFL_CLACT_WK_SetPos( wk->clwk[BPL_CA_HPGAUGE1+row1], &pos2, CLSYS_DRAW_SUB );
  GFL_CLACT_WK_SetPos( wk->clwk[BPL_CA_HPGAUGE1+row2], &pos1, CLSYS_DRAW_SUB );
}
