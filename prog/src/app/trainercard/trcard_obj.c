#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "app/app_menu_common.h"
#include "arc_def.h"

#include "trcard_obj.h"
#include "trainer_case.naix"

#define MAIN_LCD  ( GFL_BG_MAIN_DISP )  // 0
#define SUB_LCD   ( GFL_BG_SUB_DISP )   // 1

#define TR_SUBSURFACE_Y (32)

//** CharManager PlttManager用 **//
#define TR_CARD_CHAR_CONT_NUM       (2)
#define TR_CARD_CHAR_VRAMTRANS_MAIN_SIZE  (0)//(2048)
#define TR_CARD_CHAR_VRAMTRANS_SUB_SIZE   (0)
#define TR_CARD_PLTT_CONT_NUM       (2)
#define TR_CARD_PALETTE_NUM         (16)  //パレット9本
#define TR_CARD_BADGE_PALETTE_NUM     (4) //パレット4本

#define BTN_EFF_POS_X (12*8)
#define BTN_EFF_POS_Y (17*8)


typedef struct BADGE_POS_tag{
  int x;
  int y;
}BADGE_POS;

#define BADGE_LX0 (8*12)
#define BADGE_LX1 (8*18)
#define BADGE_LX2 (8*24)
#define BADGE_LX3 (8*30)

#define BADGE_LY0 (8*6)
#define BADGE_LY1 (8*11)
#define BADGE_LY2 (8*17)
#define BADGE_LY3 (8*22)

#define BADGE_NC_OFS  (8*5)

static const BADGE_POS BadgePos[] = {
  {BADGE_LX0,BADGE_LY0},
  {BADGE_LX1,BADGE_LY0},
  {BADGE_LX2,BADGE_LY0},
  {BADGE_LX3,BADGE_LY0},
  {BADGE_LX0,BADGE_LY1},
  {BADGE_LX1,BADGE_LY1},
  {BADGE_LX2,BADGE_LY1},
  {BADGE_LX3,BADGE_LY1},

  {BADGE_LX0,BADGE_LY2},
  {BADGE_LX1,BADGE_LY2},
  {BADGE_LX2,BADGE_LY2},
  {BADGE_LX3,BADGE_LY2},
  {BADGE_LX0,BADGE_LY3},
  {BADGE_LX1,BADGE_LY3},
  {BADGE_LX2,BADGE_LY3},
  {BADGE_LX3,BADGE_LY3},
};

static const BADGE_POS StarPos[] = {
  {8*3,8*5},
  {8*10,8*5},
  {8*17,8*5},
  {8*24,8*5},
  {8*3,8*12},
  {8*10,8*12},
  {8*17,8*12},
  {8*24,8*12},
};

static void InitCharPlttManager(void);

//--------------------------------------------------------------------------------------------
/**
 * セルアクター初期化
 *
 * @param wk  トレーナーカードOBJワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void InitTRCardCellActor( TR_CARD_OBJ_WORK *wk , const GFL_DISP_VRAM *vramBank)
{
  int i;
  ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_TRAINERCARD , wk->heapId );
  InitCharPlttManager();

  GFL_CLACT_SYS_Create( &GFL_CLSYSINIT_DEF_DIVSCREEN , vramBank, wk->heapId );
  // セルアクター初期化
  wk->cellUnit = GFL_CLACT_UNIT_Create( TR_CARD_ACT_MAX, 0, wk->heapId );

  // 共通リソース転送
  for(i = 0;i < 2;i++){
    int vram_type[2] = {
      NNS_G2D_VRAM_TYPE_2DMAIN,NNS_G2D_VRAM_TYPE_2DSUB,
    };
    u8 res_name[2][4] = {
     {  NARC_trainer_case_card_badge_NCLR, 
      NARC_trainer_case_card_badge_NCGR, 
      NARC_trainer_case_card_badge_NCER, 
      NARC_trainer_case_card_badge_NANR },
     {  NARC_trainer_case_card_button_NCLR, 
      NARC_trainer_case_card_button_NCGR, 
      NARC_trainer_case_card_button_NCER, 
      NARC_trainer_case_card_button_NANR },
    };
    wk->resCell[i].pltIdx = GFL_CLGRP_PLTT_RegisterEx( p_handle, res_name[i][0], CLSYS_DRAW_MAIN+i, 0, 0, 4, wk->heapId );
//    wk->resCell[i].pltIdx = GFL_CLGRP_PLTT_Register( p_handle , res_name[i][0] , CLSYS_DRAW_MAIN+i , 4*32 , wk->heapId );
    wk->resCell[i].ncgIdx = GFL_CLGRP_CGR_Register( p_handle , res_name[i][1] , FALSE , CLSYS_DRAW_MAIN+i , wk->heapId );
    wk->resCell[i].anmIdx = GFL_CLGRP_CELLANIM_Register( p_handle , res_name[i][2] , res_name[i][3], wk->heapId );
  }

  // バッジ画面用OBJリソース転送
  {
    u8 i;
    ARCHANDLE* pal_handle;

    const int pal_arc_idx[TR_CARD_BADGE_ACT_MAX] = {
      NARC_trainer_case_card_badge2_0_NCLR,
      NARC_trainer_case_card_badge2_1_NCLR,
      NARC_trainer_case_card_badge2_2_NCLR,
      NARC_trainer_case_card_badge2_3_NCLR,
      NARC_trainer_case_card_badge2_4_NCLR,
      NARC_trainer_case_card_badge2_5_NCLR,
      NARC_trainer_case_card_badge2_6_NCLR,
      NARC_trainer_case_card_badge2_7_NCLR
    };
    //パレットアーカイブハンドルオープン
    /*
    pal_handle  = ArchiveDataHandleOpen( ARCID_TRAINERCARD, HEAPID_TR_CARD);
    for (i=0;i<TR_CARD_BADGE_ACT_MAX;i++){
      wk->PalDataBuf[i] = ArchiveDataLoadAllocByHandle( pal_handle, pal_arc_idx[i], HEAPID_TR_CARD );
      if( wk->PalDataBuf[i] != NULL )
      {
        if( NNS_G2dGetUnpackedPaletteData( wk->PalDataBuf[i], &wk->PalData[i] ) == FALSE ){
          sys_FreeMemoryEz( wk->PalData[i] );
          GF_ASSERT(0);
        }
      }else{
        GF_ASSERT(0);
      }
    }
    //ハンドルクローズ
    ArchiveDataHandleClose( pal_handle );
    */
    for (i=0;i<TR_CARD_BADGE_ACT_MAX;i++)
    {
      wk->badgePalBuf[i] = GFL_ARC_UTIL_LoadPalette( ARCID_TRAINERCARD ,
                  pal_arc_idx[i], &wk->badgePalData[i] ,wk->heapId );
    }
  }

  GFL_ARC_CloseDataHandle( p_handle );

  // タッチバー用リソース転送

  p_handle = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId() , wk->heapId );
  wk->resCell[2].pltIdx = GFL_CLGRP_PLTT_Register( p_handle, APP_COMMON_GetBarIconPltArcIdx(),
                                                   CLSYS_DRAW_SUB , 4*32 , wk->heapId );
  wk->resCell[2].ncgIdx = GFL_CLGRP_CGR_Register( p_handle, APP_COMMON_GetBarIconCharArcIdx(),
                                                   FALSE, CLSYS_DRAW_SUB, wk->heapId );
  wk->resCell[2].anmIdx = GFL_CLGRP_CELLANIM_Register( p_handle, 
                                                       APP_COMMON_GetBarIconCellArcIdx( APP_COMMON_MAPPING_32K ),
                                                       APP_COMMON_GetBarIconAnimeArcIdx( APP_COMMON_MAPPING_32K ),
                                                       wk->heapId );
  GFL_ARC_CloseDataHandle( p_handle );
  
  GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );    // MAIN DISP OBJ ON
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );   // SUB DISP OBJ ON
}

//--------------------------------------------------------------------------------------------
/**
 * セルアクターをセット
 *
 * @param wk        トレーナーカードOBJワーク
 * @param inBadgeDisp   バッジ表示フラグリスト
 * @param isClear     殿堂入りしているか？
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void SetTrCardActor( TR_CARD_OBJ_WORK *wk, const u8 *inBadgeDisp ,const BOOL isClear)
{
  int i;
  {
    //登録情報格納
    u8 ofs;
    u16 badge_ofs = 0;
    GFL_CLWK_DATA addData;
    
    
    //セルアクター表示開始
    if(!isClear){
      badge_ofs = BADGE_NC_OFS; 
    }

    // 上画面(メイン画面)
    //バッジ
    for(i=0;i<TR_CARD_BADGE_ACT_MAX;i++){
      addData.pos_x = BadgePos[i].x;
      addData.pos_y = BadgePos[i].y+badge_ofs;
      addData.anmseq = i;
      wk->ClActWork[i] = GFL_CLACT_WK_Create( wk->cellUnit , wk->resCell[0].pltIdx ,
                      wk->resCell[0].ncgIdx , wk->resCell[0].anmIdx ,
                      &addData , CLSYS_DEFREND_MAIN , wk->heapId );
      if (!inBadgeDisp[i]){
        GFL_CLACT_WK_SetDrawEnable( wk->ClActWork[i], FALSE); //非表示
      }
      GFL_CLACT_WK_SetAutoAnmSpeed( wk->ClActWork[i], FX32_ONE );
      GFL_CLACT_WK_SetAutoAnmFlag( wk->ClActWork[i], TRUE );
    }
  } 
}


//==========================================
// セルアクター登録用情報
//==========================================
typedef struct{
  u16 x,y;
  u8  cgx, pal, cel, pri;
  u32 anime;
} CLACT_ENTRY_DAT;

//--------------------------------------------------------------------------------------------
/**
 * サブ画面セルアクターをセット
 *
 * @param wk        トレーナーカードOBJワーク
 * @param inBadgeDisp   バッジ表示フラグリスト
 * @param isClear     殿堂入りしているか？
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void SetTrCardActorSub( TR_CARD_OBJ_WORK *wk)
{
  int i;
  const CLACT_ENTRY_DAT entry_dat[]={
    { 228, 21*8, 2,2,2,0, APP_COMMON_BARICON_RETURN },   // 「Ｕ」
    { 204, 21*8, 2,2,2,0, APP_COMMON_BARICON_EXIT   },   // 「×」
    {   4, 21*8, 1,1,1,0, ANMS_BADGE_L,             },   // リーグ画面へ
    {   0,    0, 1,1,1,0, ANMS_EFF                  },   // エフェクト
    {  36, 21*8, 1,1,1,0, ANMS_LOUPE_L              },   // 精密描画ボタン
    {  68, 21*8, 1,1,1,0, ANMS_BLACK_PEN_L          },   // ペン先ボタン
    { 100, 21*8, 1,1,1,0, ANMS_TURN_L,              },   // カード裏返しボタン
    { 180, 21*8+4, 2,2,2,0, APP_COMMON_BARICON_CHECK_OFF },// ブックマークボタン
    
  };

  {
    GFL_CLWK_DATA addData;
    
    //セルアクター表示開始
    // 下画面(サブ画面)
    for(i=0;i<TR_CARD_SUB_ACT_MAX;i++){
      addData.pos_x  = entry_dat[i].x;
      addData.pos_y  = entry_dat[i].y;  //TODO
      addData.anmseq = entry_dat[i].anime;
      addData.bgpri  = entry_dat[i].pri;
      wk->ClActWorkS[i] = GFL_CLACT_WK_Create( wk->cellUnit , 
                                               wk->resCell[entry_dat[i].pal].pltIdx,
                                               wk->resCell[entry_dat[i].cgx].ncgIdx,
                                               wk->resCell[entry_dat[i].cel].anmIdx ,
                                               &addData , CLSYS_DEFREND_SUB , wk->heapId );
      GFL_CLACT_WK_SetDrawEnable( wk->ClActWorkS[i], TRUE); //非表示
      GFL_CLACT_WK_SetAutoAnmSpeed( wk->ClActWorkS[i], FX32_ONE );
      GFL_CLACT_WK_SetAutoAnmFlag( wk->ClActWorkS[i], TRUE );
      
    }
  } 
}
//--------------------------------------------------------------------------------------------
/**
 * 2Dセルオブジェクト解放
 *
 * @param wk  オブジェクトワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void RereaseCellObject(TR_CARD_OBJ_WORK *wk)
{
  u8 i,j;
  //パレット解放
  for(i=0;i<TR_CARD_BADGE_ACT_MAX;i++){
    GFL_HEAP_FreeMemory(wk->badgePalBuf[i]);
  }

  // セルアクターリソース解放
  for(i=0;i<TR_CARD_CLACT_RES_NUM;i++)
  {
    GFL_CLGRP_PLTT_Release( wk->resCell[i].pltIdx );
    GFL_CLGRP_CGR_Release( wk->resCell[i].ncgIdx );
    GFL_CLGRP_CELLANIM_Release( wk->resCell[i].anmIdx );
  }

  // セルアクターセット破棄
  GFL_CLACT_UNIT_Delete(wk->cellUnit);

  //OAMレンダラー破棄
  GFL_CLACT_SYS_Delete();

//  DeleteCharManager();
//  DeletePlttManager();
  
  GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_32K);  
}

//--------------------------------------------------------------------------------------------
/**
 * サブ画面ボタンアクター　表示状態変更
 *
 * @param wk      OBJワーク
 * @param inBadgeNo バッジナンバー
 * @param inOalNo   パレット番号
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void SetSActDrawSt( TR_CARD_OBJ_WORK *wk, u8 act_id, u8 anm_pat ,u8 draw_f)
{
//  CLACT_SetAnmFlag(wk->ClActWorkS[act_id],TRUE);
  GFL_CLACT_WK_SetAnmSeq( wk->ClActWorkS[act_id],anm_pat);
  GFL_CLACT_WK_ResetAnm(wk->ClActWorkS[act_id]);
  GFL_CLACT_WK_SetDrawEnable(wk->ClActWorkS[act_id],draw_f);  
}

//--------------------------------------------------------------------------------------------
/**
 * サブ画面ボタンエフェクトアクター　表示状態変更
 *
 * @param wk      OBJワーク
 * @param inBadgeNo バッジナンバー
 * @param inOalNo   パレット番号
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void SetEffActDrawSt( TR_CARD_OBJ_WORK *wk, u8 pat ,u8 draw_f)
{
  GFL_CLACTPOS cellPos;
  static const u8 pos_x[2] = {28*8,7*8};
  u32 tpx,tpy;
  GFL_UI_TP_GetPointCont( &tpx , &tpy );
  if(pat == 2)
  {
    cellPos.x = tpx;
    cellPos.y = tpy; //TODO +TR_SUBSURFACE_Y
    GFL_CLACT_WK_SetPos(wk->ClActWorkS[ACTS_BTN_EFF],&cellPos,CLSYS_DEFREND_SUB);
  }
  else
  {
    cellPos.x = pos_x[pat];
    cellPos.y = 22*8; //TODO +TR_SUBSURFACE_Y
    GFL_CLACT_WK_SetPos(wk->ClActWorkS[ACTS_BTN_EFF],&cellPos,CLSYS_DEFREND_SUB);
  }
  GFL_CLACT_WK_ResetAnm(wk->ClActWorkS[ACTS_BTN_EFF]);
  GFL_CLACT_WK_SetDrawEnable(wk->ClActWorkS[ACTS_BTN_EFF],draw_f);  
}

//--------------------------------------------------------------------------------------------
/**
 * キャラクタマネージャー
 *パレットマネージャーの初期化
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void InitCharPlttManager(void)
{
  /*
  // キャラクタマネージャー初期化
  {
    CHAR_MANAGER_MAKE cm = {
      TR_CARD_CHAR_CONT_NUM,
      TR_CARD_CHAR_VRAMTRANS_MAIN_SIZE,
      TR_CARD_CHAR_VRAMTRANS_SUB_SIZE,
      HEAPID_TR_CARD
    };
    InitCharManager(&cm);
  }
  // パレットマネージャー初期化
  InitPlttManager(TR_CARD_PLTT_CONT_NUM, HEAPID_TR_CARD);

  // 読み込み開始位置を初期化
  CharLoadStartAll();
  PlttLoadStartAll();

  REND_OAM_UtilOamRamClear_Main(HEAPID_TR_CARD);
  REND_OAM_UtilOamRamClear_Sub(HEAPID_TR_CARD);

//  DellVramTransferManager();
  */
}

