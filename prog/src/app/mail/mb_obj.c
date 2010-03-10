//============================================================================================
/**
 * @file  mb_obj.c
 * @brief メールボックス画面 OBJ関連
 * @author  Hiroyuki Nakamura
 * @date  09.01.31
 */
//============================================================================================


#include <gflib.h>

#include "system/main.h"
#include "arc/arc_def.h"
#include "font/font.naix"

#include "pokeicon/pokeicon.h"
#include "arc/poke_icon.naix"
#include "app/app_menu_common.h"

#include "mb_main.h"
#include "mb_obj.h"
#include "mail_gra.naix"


//============================================================================================
//  定数定義
//============================================================================================


// キャラリソースID
enum {
  CHR_ID_POKEICON1=0, // ポケモンアイコン
  CHR_ID_POKEICON2,
  CHR_ID_POKEICON3,
  CHR_ID_MBOBJ,
  CHR_ID_APP_COMMON,
  CHR_ID_PMS,
  CHR_ID_MAX
};
// パレットリソースID
enum {
  PAL_ID_POKEICON=0,  // ポケモンアイコン
  PAL_ID_MBOBJ,
  PAL_ID_APP_COMMON,
  PAL_ID_PMS,
  PAL_ID_MAX
};
// セルリソースID
enum {
  CEL_ID_POKEICON=0,  // ポケモンアイコン
  CEL_ID_MBOBJ,
  CEL_ID_APP_COMMON,
  CEL_ID_PMS,
  CEL_ID_MAX
};
// セルアニメリソースID
enum {
  ANM_ID_POKEICON=0,  // ポケモンアイコン
  ANM_ID_MBOBJ,
  ANM_ID_MAX
};

enum{
  RES_CGR = 0,
  RES_PAL,
  RES_CELANM,
};


#define CLA_H_NONE  ( CLACT_U_HEADER_DATA_NONE )  // セルアクターヘッダーデータなし

#define SUB_ACTOR_DISTANCE  ( 512*FX32_ONE )    // サブ画面のOBJ表示Y座標オフセット

// パレット配置
#define PALSIZ_POKEICON     ( POKEICON_PAL_MAX )  // パレット数：ポケモンアイコン
#define PALSIZ_MBOBJ      ( 2 )         // パレット数：その他のＯＢＪ

#define MAILREAD_CUR_PX   ( 224 )   //「メールをよむ」のカーソルＸ座標
#define MAILREAD_CUR_PY   ( 176 )   //「メールをよむ」のカーソルＹ座標

#define POKEICON_PX   ( 128 )   // ポケモンアイコン表示Ｘ座標
#define POKEICON_PY   ( 160 )   // ポケモンアイコン表示Ｙ座標
#define POKEICON_SX   ( 40 )    // ポケモンアイコン表示Ｘ間隔

#define POKEICON_CGX_SIZE ( 0x20*4*4 )  // ポケモンアイコンのCGXサイズ


//============================================================================================
//  構造体宣言
//============================================================================================
typedef struct {
  
  s16 x;              ///< [ X ] 座標
  s16 y;              ///< [ Y ] 座標
  s16 z;              ///< [ Z ] 座標
  
  u16 anm;            ///< アニメ番号
  int pri;            ///< 優先順位
  int pal;            ///< パレット番号 ※この値を TCATS_ADD_S_PAL_AUTO にすることで、NCEデータのカラーNo指定を受け継ぐ
  int d_area;         ///< 描画エリア
  
  int id[ 6 ];        ///< 使用リソースIDテーブル
    
  int bg_pri;         ///< BG面への優先度
  int vram_trans;     ///< Vram転送フラグ
  
} CLACT_OBJECT_ADD_PARAM;


//============================================================================================
//  プロトタイプ宣言
//============================================================================================
static void ClactResManInit( MAILBOX_APP_WORK * appwk );
static void ClactResManExit( MAILBOX_APP_WORK * appwk );
static GFL_CLWK* ClactAdd( MAILBOX_APP_WORK * appwk, const CLACT_OBJECT_ADD_PARAM * prm );
static void ClactDel( MAILBOX_APP_WORK * appwk, u32 id );
static void ClactDelAll( MAILBOX_APP_WORK * appwk );

static void PokeIconArcHandleOpen( MAILBOX_APP_WORK * appwk );
static void PokeIconArcHandleClose( MAILBOX_APP_WORK * appwk );
static void PokeIconResLoad( MAILBOX_APP_WORK * appwk );

static void MailBoxObjResLoad( MAILBOX_APP_WORK * appwk );


//============================================================================================
//  グローバル変数
//============================================================================================

// 検索ページのＯＢＪデータ
static const CLACT_OBJECT_ADD_PARAM ClaAddPrm[] =
{
  { // 左矢印
    8, 168, 0,
    APP_COMMON_BARICON_CURSOR_LEFT, 0, 0, CLSYS_DEFREND_MAIN,
    { CHR_ID_APP_COMMON, PAL_ID_APP_COMMON, CEL_ID_APP_COMMON, 0, 0, 0 },
    3, 0
  },
  { // 右矢印
    32, 168, 0,
    APP_COMMON_BARICON_CURSOR_RIGHT, 0, 0, CLSYS_DEFREND_MAIN,
    { CHR_ID_APP_COMMON, PAL_ID_APP_COMMON, CEL_ID_APP_COMMON, 0, 0, 0 },
    3, 0
  },
  { // カーソル
    64, 16, 0,
    5, 0, 1, CLSYS_DEFREND_MAIN,
    { CHR_ID_MBOBJ, PAL_ID_MBOBJ, CEL_ID_MBOBJ, 0, 0, 0 },
    1, 0
  },
  { // 戻るボタン
    224, 168, 0,
    APP_COMMON_BARICON_RETURN, 0, 0, CLSYS_DEFREND_MAIN,
    { CHR_ID_APP_COMMON, PAL_ID_APP_COMMON, CEL_ID_APP_COMMON, 0, 0, 0 },
    3, 0
  },

  { // ポケモンアイコン１
    POKEICON_PX-POKEICON_SX*0, POKEICON_PY, 0,
    0, 0, 0, CLSYS_DEFREND_SUB,
    { CHR_ID_POKEICON1, PAL_ID_POKEICON, CEL_ID_POKEICON, 0, 0, 0 },
    1, 0
  },
  { // ポケモンアイコン２
    POKEICON_PX-POKEICON_SX*1, POKEICON_PY, 0,
    0, 0, 0, CLSYS_DEFREND_SUB,
    { CHR_ID_POKEICON2, PAL_ID_POKEICON, CEL_ID_POKEICON, 0, 0, 0 },
    1, 0
  },
  { // ポケモンアイコン３
    POKEICON_PX-POKEICON_SX*2, POKEICON_PY, 0,
    0, 0, 0, CLSYS_DEFREND_SUB,
    { CHR_ID_POKEICON3, PAL_ID_POKEICON, CEL_ID_POKEICON, 0, 0, 0 },
    1, 0
  },
};



//--------------------------------------------------------------------------------------------
/**
 * セルアクター初期化
 *
 * @param syswk   システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBOBJ_Init( MAILBOX_SYS_WORK * syswk )
{
  u32 i;

  GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );    // MAIN DISP OBJ ON
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );   // SUB DISP OBJ ON
  for( i=0; i<MBMAIN_OBJ_MAX; i++ ){
    syswk->app->clres[0][i] = 0;
    syswk->app->clres[1][i] = 0;
    syswk->app->clres[2][i] = 0;
  }


  PokeIconArcHandleOpen( syswk->app );
  ClactResManInit( syswk->app );

  MailBoxObjResLoad( syswk->app );
  PokeIconResLoad( syswk->app );


  for( i=0; i<MBMAIN_OBJ_MAX; i++ ){
    syswk->app->clwk[i] = ClactAdd( syswk->app, &ClaAddPrm[i] );
    GFL_CLACT_WK_SetAutoAnmFlag( syswk->app->clwk[i], TRUE );

  }

  if( syswk->app->mail_max <= MBMAIN_MAILLIST_MAX ){
    MBOBJ_Vanish( syswk->app, MBMAIN_OBJ_ARROW_L, FALSE );
    MBOBJ_Vanish( syswk->app, MBMAIN_OBJ_ARROW_R, FALSE );
  }else if( syswk->sel_page == 0 ){
    MBOBJ_Vanish( syswk->app, MBMAIN_OBJ_ARROW_L, FALSE );
  }else{
    MBOBJ_Vanish( syswk->app, MBMAIN_OBJ_ARROW_R, FALSE );
  }
  MBOBJ_Vanish( syswk->app, MBMAIN_OBJ_POKEICON1, FALSE );
  MBOBJ_Vanish( syswk->app, MBMAIN_OBJ_POKEICON2, FALSE );
  MBOBJ_Vanish( syswk->app, MBMAIN_OBJ_POKEICON3, FALSE );
}

//--------------------------------------------------------------------------------------------
/**
 * セルアクター解放
 *
 * @param syswk   システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBOBJ_Exit( MAILBOX_SYS_WORK * syswk )
{

  ClactDelAll( syswk->app );
  ClactResManExit( syswk->app );

  PokeIconArcHandleClose( syswk->app );
}

//--------------------------------------------------------------------------------------------
/**
 * セルアクターアニメメイン
 *
 * @param appwk   アプリワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBOBJ_Main( MAILBOX_APP_WORK * appwk )
{
  
  if(appwk->clUnit != NULL){
    GFL_CLACT_SYS_Main();
  }

#if 0
  u32 i;

  for( i=0; i<MBMAIN_OBJ_MAX; i++ ){
    if( appwk->clwk[i] != NULL ){
      CATS_ObjectUpdateCap( appwk->clwk[i] );
    }
  }
#endif
}


extern const GFL_DISP_VRAM MAILBOX_VramTbl;
#define OBJ_MAILBOX_MAX ( 2 )

//--------------------------------------------------------------------------------------------
/**
 * リソースマネージャー初期化
 *
 * @param appwk   アプリワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ClactResManInit( MAILBOX_APP_WORK * appwk )
{
  static const GFL_CLSYS_INIT clsys_init  =
  {
    0, 0,   //メインサーフェースの左上X,Y座標
    0, 512, //サブサーフェースの左上X,Y座標
    4, 64,  //メインOAM管理開始～終了 （通信アイコン用に開始は4以上に、またすべて4の倍数いしてください）
    4, 64,  //差bOAM管理開始～終了  （通信アイコン用に開始は4以上に、またすべて4の倍数いしてください）
    0,      //セルVram転送管理数
    CHR_ID_MAX, PAL_ID_MAX, CEL_ID_MAX, 32,
    16, 16,       //メイン、サブのCGRVRAM管理領域開始オフセット（通信アイコン用に16以上にしてください）
  };


  //システム作成
  GFL_CLACT_SYS_Create( &clsys_init, &MAILBOX_VramTbl, HEAPID_MAILBOX_APP );
  appwk->clUnit = GFL_CLACT_UNIT_Create( MBMAIN_OBJ_MAX, 0, HEAPID_MAILBOX_APP );
  GFL_CLACT_UNIT_SetDefaultRend( appwk->clUnit );

  GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );

/*
  appwk->csp = CATS_AllocMemory( HEAPID_MAILBOX_APP );
  appwk->crp = CATS_ResourceCreate( appwk->csp );

  {
    TCATS_OAM_INIT  coi = {
      0, 128,
      0, 32,
      0, 128,
      0, 32,
    };
    TCATS_CHAR_MANAGER_MAKE ccmm = {
      1024,
      1024*64,
      1024*16,
      GX_OBJVRAMMODE_CHAR_1D_32K,
      GX_OBJVRAMMODE_CHAR_1D_32K
    };
    CATS_SystemInit( appwk->csp, &coi, &ccmm, 32 );
  }
  {
    TCATS_RESOURCE_NUM_LIST crnl = {
      CHR_ID_MAX, PAL_ID_MAX, CEL_ID_MAX, ANM_ID_MAX, 0, 0
    };
    CATS_ClactSetInit( appwk->csp, appwk->crp, MBMAIN_OBJ_MAX );
    CATS_ResourceManagerInit( appwk->csp, appwk->crp, &crnl );
  }
  {
    CLACT_U_EASYRENDER_DATA * renddata = CATS_EasyRenderGet( appwk->csp );
    CLACT_U_SetSubSurfaceMatrix( renddata, 0, SUB_ACTOR_DISTANCE );
  }
*/
}

//--------------------------------------------------------------------------------------------
/**
 * リソースマネージャー解放
 *
 * @param appwk   アプリワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ClactResManExit( MAILBOX_APP_WORK * appwk )
{
//  CATS_ResourceDestructor_S( appwk->csp, appwk->crp );
//  CATS_FreeMemory( appwk->csp );

  GFL_CLGRP_CELLANIM_Release( appwk->clres[RES_PAL][PAL_ID_POKEICON]);
  GFL_CLGRP_PLTT_Release( appwk->clres[RES_CELANM][CEL_ID_POKEICON] );
 
  //システム破棄
  GFL_CLACT_UNIT_Delete( appwk->clUnit );
  GFL_CLACT_SYS_Delete();
  
}

//--------------------------------------------------------------------------------------------
/**
 * セルアクター追加
 *
 * @param appwk アプリワーク
 * @param prm   追加データ
 *
 * @return  追加したセルアクターのポインタ
 */
//--------------------------------------------------------------------------------------------
static GFL_CLWK* ClactAdd( MAILBOX_APP_WORK * appwk, const CLACT_OBJECT_ADD_PARAM * prm )
{
  GFL_CLWK_DATA dat;
  dat.pos_x   = prm->x;   ///< [ X ] 座標
  dat.pos_y   = prm->y;   ///< [ Y ] 座標
  dat.anmseq  = prm->anm;        ///< アニメ番号
  dat.softpri = 0;
  dat.bgpri   = 0;

  return GFL_CLACT_WK_Create( appwk->clUnit, appwk->clres[RES_CGR][prm->id[0]], 
                                             appwk->clres[RES_PAL][prm->id[1]],
                                             appwk->clres[RES_CELANM][prm->id[2]], &dat, 
                                             prm->d_area, HEAPID_MAILBOX_APP );


}

//--------------------------------------------------------------------------------------------
/**
 * セルアクター削除
 *
 * @param appwk アプリワーク
 * @param id    OBJ ID
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ClactDel( MAILBOX_APP_WORK * appwk, u32 id )
{
  if( appwk->clwk[id] != NULL ){
    GFL_CLACT_WK_Remove( appwk->clwk[id] );
    appwk->clwk[id] = NULL;
  }
}

//--------------------------------------------------------------------------------------------
/**
 * セルアクター全削除
 *
 * @param appwk   アプリワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ClactDelAll( MAILBOX_APP_WORK * appwk )
{
  u32 i;

  for( i=0; i<MBMAIN_OBJ_MAX; i++ ){
    ClactDel( appwk, i );
  }
}

//--------------------------------------------------------------------------------------------
/**
 * セルアクターアニメ変更
 *
 * @param appwk アプリワーク
 * @param id    OBJ ID
 * @param anm   アニメ番号
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBOBJ_AnmSet( MAILBOX_APP_WORK * appwk, u32 id, u32 anm )
{
//  CATS_ObjectAnimeFrameSetCap( appwk->clwk[id], 0 );
//  CATS_ObjectAnimeSeqSetCap( appwk->clwk[id], anm );

  GFL_CLACT_WK_SetAnmSeq( appwk->clwk[id], anm );
}


//--------------------------------------------------------------------------------------------
/**
 * セルアクター表示切り替え
 *
 * @param appwk アプリワーク
 * @param id    OBJ ID
 * @param flg   表示フラグ
 *
 * @return  none
 *
 * @li  flg = TRUE : 表示
 * @li  flg = FALSE : 非表示
 */
//--------------------------------------------------------------------------------------------
void MBOBJ_Vanish( MAILBOX_APP_WORK * appwk, u32 id, BOOL flg )
{

  GFL_CLACT_WK_SetDrawEnable( appwk->clwk[id], flg );

}

//--------------------------------------------------------------------------------------------
/**
 * セルアクター移動
 *
 * @param appwk アプリワーク
 * @param id    OBJ ID
 * @param px    Ｘ座標
 * @param py    Ｙ座標
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBOBJ_PosSet( MAILBOX_APP_WORK * appwk, u32 id, s16 px, s16 py )
{
//  CATS_ObjectPosSetCap( appwk->clwk[id], px, py );
  GFL_CLACTPOS pos;
  pos.x = px;
  pos.y = py;

  if(py < 256){
    GFL_CLACT_WK_SetPos( appwk->clwk[id], &pos, CLSYS_DEFREND_MAIN );
  }else{
    GFL_CLACT_WK_SetPos( appwk->clwk[id], &pos, CLSYS_DEFREND_SUB );
  }
}

//--------------------------------------------------------------------------------------------
/**
 * セルアクター半透明設定
 *
 * @param appwk アプリワーク
 * @param id    OBJ ID
 * @param flg   ON/OFFフラグ
 *
 * @return  none
 *
 * @li  flg : TRUE = ON, FALSE = OFF
 */
//--------------------------------------------------------------------------------------------
void MBOBJ_BlendModeSet( MAILBOX_APP_WORK * appwk, u32 id, BOOL flg )
{
  if( flg == TRUE ){
    GFL_CLACT_WK_SetObjMode( appwk->clwk[id], GX_OAM_MODE_XLU );
  }else{
    GFL_CLACT_WK_SetObjMode( appwk->clwk[id], GX_OAM_MODE_NORMAL );
  }
}


//============================================================================================
//  ポケモンアイコン
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * ポケモンアイコンのアーカイブハンドル取得
 *
 * @param appwk   アプリワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void PokeIconArcHandleOpen( MAILBOX_APP_WORK * appwk )
{
  appwk->pokeicon_ah = GFL_ARC_OpenDataHandle( ARCID_POKEICON, HEAPID_MAILBOX_APP );
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンアイコンのアーカイブハンドル破棄
 *
 * @param appwk   アプリワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void PokeIconArcHandleClose( MAILBOX_APP_WORK * appwk )
{
  GFL_ARC_CloseDataHandle( appwk->pokeicon_ah );
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンアイコンリソース読み込み
 *
 * @param appwk   アプリワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void PokeIconResLoad( MAILBOX_APP_WORK * appwk )
{
  u32 i;

  ARCHANDLE *handle = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), HEAPID_MAILBOX_APP);

  for( i=CHR_ID_POKEICON1; i<=CHR_ID_POKEICON3; i++ ){
//    CATS_LoadResourceCharArcH(
//      appwk->csp, appwk->crp, appwk->pokeicon_ah,
//      PokeIconCgxArcIndexGetByMonsNumber(0,0,0),
//      0, NNS_G2D_VRAM_TYPE_2DSUB, i );

    // ポケモンアイコンキャラ登録
    appwk->clres[RES_CGR][i] = GFL_CLGRP_CGR_Register( 
                                                handle,
                                                APP_COMMON_GetPokeItemIconCharArcIdx(), 0,
                                                CLSYS_DRAW_SUB, HEAPID_MAILBOX_APP );


  }


/*
  CATS_LoadResourcePlttArcH(
    appwk->csp, appwk->crp,
    appwk->pokeicon_ah, PokeIconPalArcIndexGet(), 0,
    PALSIZ_POKEICON, NNS_G2D_VRAM_TYPE_2DSUB, PAL_ID_POKEICON );
  CATS_LoadResourceCellArcH(
    appwk->csp, appwk->crp,
    appwk->pokeicon_ah, PokeIconCellArcIndexGet(), 0, CEL_ID_POKEICON );
  CATS_LoadResourceCellAnmArcH(
    appwk->csp, appwk->crp,
    appwk->pokeicon_ah, PokeIconAnmCellArcIndexGet(), 0, ANM_ID_POKEICON );
*/

 // Pltt
  appwk->clres[RES_PAL][PAL_ID_POKEICON] = GFL_CLGRP_PLTT_Register(
                                handle, POKEICON_GetPalArcIndex(),
                                CLSYS_DRAW_SUB, 0, HEAPID_MAILBOX_APP );
  // CellAnm
  appwk->clres[RES_CELANM][CEL_ID_POKEICON] = GFL_CLGRP_CELLANIM_Register(
                                      handle, 
                                      APP_COMMON_GetPokeItemIconCellArcIdx(APP_COMMON_MAPPING_32K),
                                      APP_COMMON_GetPokeItemIconAnimeArcIdx(APP_COMMON_MAPPING_32K), 
                                      HEAPID_MAILBOX_APP );

  GFL_ARC_CloseDataHandle( handle );


}

/*
static void PokeIconResFree( MAILBOX_APP_WORK * appwk )
{
  u32 i;

  for( i=CHR_ID_POKEICON1; i<=CHR_ID_POKEICON3; i++ ){
    CATS_FreeResourceChar( appwk->crp, i );
  }
  CATS_FreeResourcePltt( appwk->crp, PAL_ID_POKEICON );
  CATS_FreeResourceCell( appwk->crp, CEL_ID_POKEICON );
  CATS_FreeResourceCellAnm( appwk->crp, ANM_ID_POKEICON );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * ポケモンアイコン表示
 *
 * @param syswk システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBOBJ_PokeIconPut( MAILBOX_SYS_WORK * syswk )
{
  MAIL_DATA * md;
  NNSG2dCharacterData * chr;
  NNSG2dImageProxy * ipc;
  void * buf;
  u32 cgx;
  u32 i;
  MAIL_ICON mi;
  
  md = syswk->app->mail[ syswk->lst_pos ];

  for( i=0; i<MAILDAT_ICONMAX; i++ ){
    mi.dat = MailData_GetIconParamByIndex( md, i, MAIL_ICONPRM_ALL, MailData_GetFormBit(md) );
    if( mi.dat == MAIL_ICON_NULL || mi.cgxID == NARC_poke_icon_poke_icon_000_m_NCGR ){
      MBOBJ_Vanish( syswk->app, MBMAIN_OBJ_POKEICON1+i, FALSE );
      continue;
    }

//    ipc = CLACT_ImageProxyGet( syswk->app->clwk[MBMAIN_OBJ_POKEICON1+i]->act );
//    cgx = NNS_G2dGetImageLocation( ipc, NNS_G2D_VRAM_TYPE_2DSUB );
    cgx = GFL_CLGRP_CGR_GetAddr( syswk->app->clres[RES_CGR][CHR_ID_POKEICON1+i], CLSYS_DRAW_SUB );
    buf = GFL_ARCHDL_UTIL_LoadOBJCharacter( syswk->app->pokeicon_ah, mi.cgxID, 0, &chr, HEAPID_MAILBOX_APP );
    DC_FlushRange( chr->pRawData, POKEICON_CGX_SIZE );
    GXS_LoadOBJ( chr->pRawData, cgx, POKEICON_CGX_SIZE );
    GFL_HEAP_FreeMemory( buf );

    GFL_CLACT_WK_SetPlttOffs( syswk->app->clwk[MBMAIN_OBJ_POKEICON1+i], mi.palID, 
                              CLWK_PLTTOFFS_MODE_PLTT_TOP );
    MBOBJ_Vanish( syswk->app, MBMAIN_OBJ_POKEICON1+i, TRUE );
  }
}


//============================================================================================
//  その他のＯＢＪ
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * メールボックス画面のリソース読み込み
 *
 * @param appwk   アプリワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void MailBoxObjResLoad( MAILBOX_APP_WORK * appwk )
{
  ARCHANDLE * handle = GFL_ARC_OpenDataHandle( ARCID_MAIL_GRA, HEAPID_MAILBOX_APP );

  // メールボックス素材
  // Char
  appwk->clres[RES_CGR][CHR_ID_MBOBJ] = GFL_CLGRP_CGR_Register( handle,
                                            NARC_mail_gra_mailbox_obj_m_lz_NCGR, 1,
                                            CLSYS_DRAW_MAIN, HEAPID_MAILBOX_APP );
  // Pltt
  appwk->clres[RES_PAL][PAL_ID_MBOBJ] = GFL_CLGRP_PLTT_Register( handle,
                                            NARC_mail_gra_mailbox_obj_NCLR,
                                            CLSYS_DRAW_MAIN, 0, HEAPID_MAILBOX_APP );
  // CellAnm
  appwk->clres[RES_CELANM][CEL_ID_MBOBJ] = GFL_CLGRP_CELLANIM_Register( handle,
                                            NARC_mail_gra_mailbox_obj_NCER,
                                            NARC_mail_gra_mailbox_obj_NANR, 
                                            HEAPID_MAILBOX_APP );

  GFL_ARC_CloseDataHandle( handle );

  // タッチバー素材
  handle = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), HEAPID_MAILBOX_APP );

  // Char
  appwk->clres[RES_CGR][CHR_ID_APP_COMMON] = GFL_CLGRP_CGR_Register( handle,
                                            APP_COMMON_GetBarIconCharArcIdx(), 0,
                                            CLSYS_DRAW_MAIN, HEAPID_MAILBOX_APP );
  // Pltt
  appwk->clres[RES_PAL][PAL_ID_APP_COMMON] = GFL_CLGRP_PLTT_Register( handle,
                                            APP_COMMON_GetBarIconPltArcIdx(),
                                            CLSYS_DRAW_MAIN, 2*32, HEAPID_MAILBOX_APP );
  // CellAnm
  appwk->clres[RES_CELANM][CEL_ID_APP_COMMON] = GFL_CLGRP_CELLANIM_Register( handle,
                                            APP_COMMON_GetBarIconCellArcIdx(APP_COMMON_MAPPING_32K),
                                            APP_COMMON_GetBarIconAnimeArcIdx(APP_COMMON_MAPPING_32K), 
                                            HEAPID_MAILBOX_APP );
  
  GFL_ARC_CloseDataHandle( handle );

}

//--------------------------------------------------------------------------------------------
/**
 * 「メールをよむ」カーソルセット
 *
 * @param appwk   アプリワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBOBJ_MailReadCurMove( MAILBOX_APP_WORK * appwk )
{
  MBOBJ_PosSet( appwk, MBMAIN_OBJ_CURSOR, MAILREAD_CUR_PX, MAILREAD_CUR_PY );
  MBOBJ_AnmSet( appwk, MBMAIN_OBJ_CURSOR, 4 );
}
