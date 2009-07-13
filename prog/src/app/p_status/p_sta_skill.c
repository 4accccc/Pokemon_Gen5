//======================================================================
/**
 * @file	p_sta_skill.c
 * @brief	ポケモンステータス 技ページ
 * @author	ariizumi
 * @data	09/07/10
 *
 * モジュール名：PSTATUS_SKILL
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "print/global_msg.h"
#include "print/str_tool.h"

#include "arc_def.h"
#include "message.naix"
#include "p_status_gra.naix"

#include "p_sta_sys.h"
#include "p_sta_skill.h"

#include "test/ariizumi/ari_debug.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//文字位置
#define PSTATUS_SKILL_STR_HP_X (24+PSTATUS_STR_OFS_X)
#define PSTATUS_SKILL_STR_HP_Y ( 0+PSTATUS_STR_OFS_Y)
#define PSTATUS_SKILL_STR_HPNOW_X   ( 88+PSTATUS_STR_OFS_X)  //右寄せ
#define PSTATUS_SKILL_STR_HPNOW_Y   (  0+PSTATUS_STR_OFS_Y)
#define PSTATUS_SKILL_STR_HPSLASH_X ( 88+PSTATUS_STR_OFS_X)
#define PSTATUS_SKILL_STR_HPSLASH_Y (  0+PSTATUS_STR_OFS_Y)
#define PSTATUS_SKILL_STR_HPMAX_X   (120+PSTATUS_STR_OFS_X)
#define PSTATUS_SKILL_STR_HPMAX_Y   ( 0+PSTATUS_STR_OFS_Y)
  //攻撃・防御・特攻t・特防・素早さ
#define PSTATUS_SKILL_STR_PARAM_X ( 0+PSTATUS_STR_OFS_X)  
#define PSTATUS_SKILL_STR_PARAM_Y ( 0+PSTATUS_STR_OFS_Y)
#define PSTATUS_SKILL_STR_PARAM_NUM_X (104+PSTATUS_STR_OFS_X)
#define PSTATUS_SKILL_STR_PARAM_NUM_Y (  0+PSTATUS_STR_OFS_Y)
//特性
#define PSTATUS_SKILL_STR_TOKUSEI_X ( 4+PSTATUS_STR_OFS_X)  
#define PSTATUS_SKILL_STR_TOKUSEI_Y ( 0+PSTATUS_STR_OFS_Y)
#define PSTATUS_SKILL_STR_TOKUSEI_NAME_X (64+PSTATUS_STR_OFS_X)
#define PSTATUS_SKILL_STR_TOKUSEI_NAME_Y ( 0+PSTATUS_STR_OFS_Y)
#define PSTATUS_SKILL_STR_TOKUSEI_INFO_X ( 4+PSTATUS_STR_OFS_X)
#define PSTATUS_SKILL_STR_TOKUSEI_INFO_Y (16+PSTATUS_STR_OFS_Y)

//セル位置
#define PSTATUS_SKILL_CELL_HPBAR_X ( 159 );
#define PSTATUS_SKILL_CELL_HPBAR_Y (  52 );

//HPバー系
#define PSTATUS_SKILL_HPBAR_LEN (48)
#define PSTATUS_SKILL_HPBAR_TOP  (2)
#define PSTATUS_SKILL_HPBAR_LEFT (0)
//色
#define PSTATUS_SKILL_HPBAR_COL_GREEN_IN  (0x5)
#define PSTATUS_SKILL_HPBAR_COL_GREEN_OUT (0x6)
#define PSTATUS_SKILL_HPBAR_COL_YELLOW_IN  (0x7)
#define PSTATUS_SKILL_HPBAR_COL_YELLOW_OUT (0x8)
#define PSTATUS_SKILL_HPBAR_COL_RED_IN  (0x9)
#define PSTATUS_SKILL_HPBAR_COL_RED_OUT (0xa)

//SkillPlate
#define PSTATUS_SKILL_PLATE_NUM (5)
//基本位置からのbmpWin位置
#define PSTATUS_SKILL_PLATE_WIN_TOP (0)
#define PSTATUS_SKILL_PLATE_WIN_LEFT (5)
#define PSTATUS_SKILL_PLATE_WIN_HEIGHT (4)
#define PSTATUS_SKILL_PLATE_WIN_WIDTH (11)

//文字位置
#define PSTATUS_SKILL_PLATE_WAZANAME_X (0+PSTATUS_STR_OFS_X)
#define PSTATUS_SKILL_PLATE_WAZANAME_Y (0 +PSTATUS_STR_OFS_Y)
#define PSTATUS_SKILL_PLATE_PPSTR_X (12+PSTATUS_STR_OFS_X)
#define PSTATUS_SKILL_PLATE_PPSTR_Y (16 +PSTATUS_STR_OFS_Y)
#define PSTATUS_SKILL_PLATE_NOWPP_X (56+PSTATUS_STR_OFS_X)
#define PSTATUS_SKILL_PLATE_NOWPP_Y (16 +PSTATUS_STR_OFS_Y)
#define PSTATUS_SKILL_PLATE_SLASH_X (56+PSTATUS_STR_OFS_X)
#define PSTATUS_SKILL_PLATE_SLASH_Y (16 +PSTATUS_STR_OFS_Y)
#define PSTATUS_SKILL_PLATE_MAXPP_X (60+PSTATUS_STR_OFS_X)
#define PSTATUS_SKILL_PLATE_MAXPP_Y (16 +PSTATUS_STR_OFS_Y)
#define PSTATUS_SKILL_PLATE_NONE_X (48+PSTATUS_STR_OFS_X)
#define PSTATUS_SKILL_PLATE_NONE_Y (16 +PSTATUS_STR_OFS_Y)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
enum PSTATUS_SKILL_UP_BMPWIN_TYPE
{
  PSBT_HP,
  PSBT_HP_GAUGE,
  PSBT_ATK,
  PSBT_DEF,
  PSBT_SATK,
  PSBT_SDEF,
  PSBT_SPD,
  PSBT_TOKUSEI,
  
  PSBT_TYPE,
  PSBT_POW,
  PSBT_HIT,
  PSBT_INFO,
  
  PSBT_MAX,
  
  //ステータスの開始と終わり
  PSBT_STATUS_START = PSBT_HP,
  PSBT_STATUS_END = PSBT_TOKUSEI,
  //技詳細の開始と終わり
  PSBT_SKILL_START = PSBT_TYPE,
  PSBT_SKILL_END = PSBT_INFO,
};


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

typedef struct
{
  u8   idx;
  BOOL isUpdateStr;
  
  GFL_CLWK   *clwkPlate;
  GFL_CLWK   *clwkType;
  GFL_BMPWIN *bmpWin;
  
}PSTATUS_SKILL_PLATE;


struct _PSTATUS_SKILL_WORK
{
  BOOL isDisp;
  BOOL isUpdateStrStatus;
  BOOL isUpdateStrSkill;

  NNSG2dScreenData *scrDataDown;
  void *scrResDown;
  NNSG2dScreenData *scrDataUpStatus;
  void *scrResUpStatus;
  NNSG2dScreenData *scrDataUpSkill;
  void *scrResUpSkill;
  
  GFL_BMPWIN *upBmpWin[PSBT_MAX];
  
  GFL_CLWK *clwkHpBar;
  
  PSTATUS_SKILL_PLATE plateWork[PSTATUS_SKILL_PLATE_NUM];
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void PSATUS_SKILL_DispStatusPage( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork );
static void PSATUS_SKILL_DispSkillInfoPage( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork );
static void PSATUS_SKILL_DrawStrStatus( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork );
static void PSTATUS_SKILL_DrawHPBar( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork );

static void PSTATUS_SKILL_InitPlate( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork , PSTATUS_SKILL_PLATE *plateWork , const u8 idx );
static void PSTATUS_SKILL_TermPlate( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork , PSTATUS_SKILL_PLATE *plateWork );
static void PSTATUS_SKILL_UpdatePlate( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork , PSTATUS_SKILL_PLATE *plateWork );
static void PSTATUS_SKILL_DispPlate( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork , PSTATUS_SKILL_PLATE *plateWork );
static void PSTATUS_SKILL_ClearPlate( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork , PSTATUS_SKILL_PLATE *plateWork );

//上画面bmpwin座標
static const u8 winPos[PSBT_MAX][4] =
{
  {  8,  4, 16,  2},  //HP
  { 17,  6,  6,  1},  //HPゲージ
  {  8,  7, 14,  2},  //攻撃
  {  8,  9, 14,  2},  //防御
  {  8, 11, 14,  2},  //特攻
  {  8, 13, 14,  2},  //特防
  {  8, 15, 14,  2},  //素早さ
  {  6, 18, 20,  6},  //特性説明
  
  { 10,  6, 14,  2},  //分類
  { 10,  8, 14,  2},  //威力
  { 10, 10, 14,  2},  //命中
  {  1, 13, 30,  6},  //技説明
};

//下画面技プレート座標
static const u8 platePos[5][2] =
{
  { 1 , 2 },
  { 1 , 6 },
  { 1 ,10 },
  { 1 ,14 },
  { 1 ,21 },
};

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
PSTATUS_SKILL_WORK* PSTATUS_SKILL_Init( PSTATUS_WORK *work )
{
  u8 i;
  PSTATUS_SKILL_WORK* skillWork;
  
  skillWork = GFL_HEAP_AllocMemory( work->heapId , sizeof(PSTATUS_SKILL_WORK) );
  skillWork->isDisp = FALSE;
  skillWork->isUpdateStrStatus = FALSE;
  skillWork->isUpdateStrSkill = FALSE;
  
  for( i=0;i<PSTATUS_SKILL_PLATE_NUM;i++ )
  {
    skillWork->plateWork[i].isUpdateStr = FALSE;
  }
  return skillWork;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void PSTATUS_SKILL_Term( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork )
{
  GFL_HEAP_FreeMemory( skillWork );
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
void PSTATUS_SKILL_Main( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork )
{
  u8 i;
  if( skillWork->isUpdateStrStatus == TRUE )
  {
    u8 i;
    BOOL isFinish = TRUE;
    for( i=PSBT_STATUS_START;i<=PSBT_STATUS_END;i++ )
    {
      if( PRINTSYS_QUE_IsExistTarget( work->printQue , GFL_BMPWIN_GetBmp( skillWork->upBmpWin[i] )) == TRUE )
      {
        isFinish = FALSE;
        break;
      }
    }
    if( isFinish == TRUE )
    {
      for( i=PSBT_STATUS_START;i<=PSBT_STATUS_END;i++ )
      {
        GFL_BMPWIN_MakeTransWindow_VBlank( skillWork->upBmpWin[i] );
      }
      skillWork->isUpdateStrStatus = FALSE;
    }
  }
  if( skillWork->isUpdateStrSkill == TRUE )
  {
    u8 i;
    BOOL isFinish = TRUE;
    for( i=PSBT_SKILL_START;i<=PSBT_SKILL_END;i++ )
    {
      if( PRINTSYS_QUE_IsExistTarget( work->printQue , GFL_BMPWIN_GetBmp( skillWork->upBmpWin[i] )) == TRUE )
      {
        isFinish = FALSE;
        break;
      }
    }
    if( isFinish == TRUE )
    {
      for( i=PSBT_SKILL_START;i<=PSBT_SKILL_END;i++ )
      {
        GFL_BMPWIN_MakeTransWindow_VBlank( skillWork->upBmpWin[i] );
      }
      skillWork->isUpdateStrStatus = FALSE;
    }
  }
  
  for( i=0;i<PSTATUS_SKILL_PLATE_NUM;i++ )
  {
    PSTATUS_SKILL_UpdatePlate( work , skillWork , &skillWork->plateWork[i] );
  }
}

#pragma mark [>Resource
//--------------------------------------------------------------
//	リソース読み込み
//--------------------------------------------------------------
void PSTATUS_SKILL_LoadResource( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork , ARCHANDLE *archandle )
{
  u8 i;
  //書き換え用スクリーン読み込み
  skillWork->scrResDown = GFL_ARCHDL_UTIL_LoadScreen( archandle , NARC_p_status_gra_p_st_info_d_NSCR ,
                    FALSE , &skillWork->scrDataDown , work->heapId );
  skillWork->scrResUpStatus = GFL_ARCHDL_UTIL_LoadScreen( archandle , NARC_p_status_gra_p_st_info_u_NSCR ,
                    FALSE , &skillWork->scrDataUpStatus , work->heapId );
  skillWork->scrResUpSkill = GFL_ARCHDL_UTIL_LoadScreen( archandle , NARC_p_status_gra_p_st_info_u_NSCR ,
                    FALSE , &skillWork->scrDataUpSkill , work->heapId );

}

//--------------------------------------------------------------
//	リソース開放
//--------------------------------------------------------------
void PSTATUS_SKILL_ReleaseResource( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork )
{
  u8 i;
  //ここで描画物の開放もしておく
  if( skillWork->isDisp == TRUE )
  {
    PSTATUS_SKILL_ClearPage( work , skillWork );
  }

  GFL_HEAP_FreeMemory( skillWork->scrResDown );
  GFL_HEAP_FreeMemory( skillWork->scrResUpStatus );
  GFL_HEAP_FreeMemory( skillWork->scrResUpSkill );

}

#pragma mark [>Cell
void PSTATUS_SKILL_InitCell( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork )
{
  u8 i;
  {
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = PSTATUS_SKILL_CELL_HPBAR_X;
    cellInitData.pos_y = PSTATUS_SKILL_CELL_HPBAR_Y;
    cellInitData.softpri = 10;
    cellInitData.bgpri = 1;
    cellInitData.anmseq = 0;
    
    skillWork->clwkHpBar = GFL_CLACT_WK_Create( work->cellUnit ,
              work->cellRes[SCR_NCG_HPBASE],
              work->cellRes[SCR_PLT_HPBASE],
              work->cellRes[SCR_ANM_HPBASE],
              &cellInitData ,CLSYS_DEFREND_SUB , work->heapId );

    GFL_CLACT_WK_SetDrawEnable( skillWork->clwkHpBar , FALSE );
  } 

  //基本はcellの初期かなのでここで
  for( i=0;i<4;i++ )
  {
    PSTATUS_SKILL_InitPlate( work , skillWork , &skillWork->plateWork[i] , i );
  }

}

void PSTATUS_SKILL_TermCell( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork )
{
  u8 i;
  for( i=0;i<4;i++ )
  {
    PSTATUS_SKILL_TermPlate( work , skillWork , &skillWork->plateWork[i] );
  }
  GFL_CLACT_WK_Remove( skillWork->clwkHpBar );
}


#pragma mark [>Disp
//--------------------------------------------------------------
//	ページの表示
//--------------------------------------------------------------
void PSTATUS_SKILL_DispPage( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork )
{
  u8 i;

   //Window下地の張替え
  GFL_BG_WriteScreenExpand( PSTATUS_BG_PLATE , 
                    0 , 0 , PSTATUS_MAIN_PAGE_WIDTH , 24 ,
                    skillWork->scrDataDown->rawData ,
                    0 , 0 , 32 , 32 );
  GFL_BG_LoadScreenV_Req( PSTATUS_BG_PLATE );

   
  for( i=0;i<PSBT_MAX;i++ )
  {
    skillWork->upBmpWin[i] = GFL_BMPWIN_Create( PSTATUS_BG_SUB_STR ,
                winPos[i][0] , winPos[i][1] , winPos[i][2] , winPos[i][3] ,
                PSTATUS_BG_SUB_PLT_FONT , GFL_BMP_CHRAREA_GET_B );
  }
  //HPバーだけパレット違うの対応
  GFL_BMPWIN_SetPalette( skillWork->upBmpWin[PSBT_HP_GAUGE] , PSTATUS_BG_SUB_PLT_HPBAR );

  PSATUS_SKILL_DispStatusPage( work , skillWork );

  for( i=0;i<4;i++ )
  {
    PSTATUS_SKILL_DispPlate( work , skillWork , &skillWork->plateWork[i] );
  }

  skillWork->isDisp = TRUE;
}

//--------------------------------------------------------------
//	ページのクリア
//--------------------------------------------------------------
void PSTATUS_SKILL_ClearPage( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork )
{
  u8 i;
  for( i=0;i<4;i++ )
  {
    PSTATUS_SKILL_ClearPlate( work , skillWork , &skillWork->plateWork[i] );
  }
  for( i=0;i<PSBT_MAX;i++ )
  {
    GFL_BMPWIN_Delete( skillWork->upBmpWin[i] );
  }
  
  GFL_BG_FillScreen( PSTATUS_BG_PARAM , 0 , 0 , 0 , 
                     PSTATUS_MAIN_PAGE_WIDTH , 21 ,
                     GFL_BG_SCRWRT_PALNL );
  GFL_BG_LoadScreenV_Req( PSTATUS_BG_PARAM );
  
  GFL_BG_ClearScreenCodeVReq( PSTATUS_BG_SUB_STR , 0 );
  GFL_BG_LoadScreenV_Req( PSTATUS_BG_SUB_STR );

  GFL_CLACT_WK_SetDrawEnable( skillWork->clwkHpBar , FALSE );

  skillWork->isDisp = FALSE;
}

//--------------------------------------------------------------
//	ページの表示(ステータス
//--------------------------------------------------------------
static void PSATUS_SKILL_DispStatusPage( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork )
{
  GFL_BG_LoadScreen( PSTATUS_BG_SUB_PLATE, 
                     skillWork->scrDataUpStatus->rawData, 
                     skillWork->scrDataUpStatus->szByte, 
                     0 );
  GFL_BG_LoadScreenBuffer( PSTATUS_BG_SUB_PLATE, 
                     skillWork->scrDataUpStatus->rawData, 
                     skillWork->scrDataUpStatus->szByte );
  GFL_BG_LoadScreenV_Req( PSTATUS_BG_SUB_PLATE );

  GFL_CLACT_WK_SetDrawEnable( skillWork->clwkHpBar , TRUE );

  PSATUS_SKILL_DrawStrStatus( work , skillWork );
  PSTATUS_SKILL_DrawHPBar(  work , skillWork );
}

//--------------------------------------------------------------
//	ページの表示(技詳細
//--------------------------------------------------------------
static void PSATUS_SKILL_DispSkillInfoPage( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork )
{
  GFL_BG_LoadScreen( PSTATUS_BG_SUB_PLATE, 
                     skillWork->scrDataUpSkill->rawData, 
                     skillWork->scrDataUpSkill->szByte, 
                     0 );
  GFL_BG_LoadScreenBuffer( PSTATUS_BG_SUB_PLATE, 
                     skillWork->scrDataUpSkill->rawData, 
                     skillWork->scrDataUpSkill->szByte );
  GFL_BG_LoadScreenV_Req( PSTATUS_BG_SUB_PLATE );

  GFL_CLACT_WK_SetDrawEnable( skillWork->clwkHpBar , FALSE );
}


//--------------------------------------------------------------
//	文字の描画(ステータス
//--------------------------------------------------------------
static void PSATUS_SKILL_DrawStrStatus( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork )
{
  const POKEMON_PARAM *pp = PSTATUS_UTIL_GetCurrentPP( work );

  //HP
  PSTATUS_UTIL_DrawStrFunc( work , skillWork->upBmpWin[PSBT_HP] , mes_status_04_02 ,
                            PSTATUS_SKILL_STR_HP_X , PSTATUS_SKILL_STR_HP_Y , PSTATUS_STR_COL_TITLE );
  //現在HP
  {
    WORDSET *wordSet = WORDSET_Create( work->heapId );
    u32 no = PP_Get( pp , ID_PARA_hp , NULL );
    WORDSET_RegisterNumber( wordSet , 0 , no , 3 , STR_NUM_DISP_SPACE , STR_NUM_CODE_DEFAULT );
    PSTATUS_UTIL_DrawValueStrFuncRight( work , skillWork->upBmpWin[PSBT_HP] , wordSet , mes_status_04_11 , 
                                   PSTATUS_SKILL_STR_HPNOW_X , PSTATUS_SKILL_STR_HPNOW_Y , PSTATUS_STR_COL_VALUE );
    WORDSET_Delete( wordSet );
  }
  //HPのスラッシュ
  PSTATUS_UTIL_DrawStrFunc( work , skillWork->upBmpWin[PSBT_HP] , mes_status_04_09 ,
                            PSTATUS_SKILL_STR_HPSLASH_X , PSTATUS_SKILL_STR_HPSLASH_Y , PSTATUS_STR_COL_VALUE );
  //最大HP
  {
    WORDSET *wordSet = WORDSET_Create( work->heapId );
    u32 no = PP_Get( pp , ID_PARA_hpmax , NULL );
    WORDSET_RegisterNumber( wordSet , 0 , no , 3 , STR_NUM_DISP_SPACE , STR_NUM_CODE_DEFAULT );
    PSTATUS_UTIL_DrawValueStrFuncRight( work , skillWork->upBmpWin[PSBT_HP] , wordSet , mes_status_04_10 , 
                                   PSTATUS_SKILL_STR_HPMAX_X , PSTATUS_SKILL_STR_HPMAX_Y , PSTATUS_STR_COL_VALUE );
    WORDSET_Delete( wordSet );
  }

  //攻撃
  PSTATUS_UTIL_DrawStrFunc( work , skillWork->upBmpWin[PSBT_ATK] , mes_status_04_03 ,
                            PSTATUS_SKILL_STR_PARAM_X , PSTATUS_SKILL_STR_PARAM_Y , PSTATUS_STR_COL_TITLE );
  {
    WORDSET *wordSet = WORDSET_Create( work->heapId );
    u32 no = PP_Get( pp , ID_PARA_pow , NULL );
    WORDSET_RegisterNumber( wordSet , 0 , no , 3 , STR_NUM_DISP_SPACE , STR_NUM_CODE_DEFAULT );
    PSTATUS_UTIL_DrawValueStrFuncRight( work , skillWork->upBmpWin[PSBT_ATK] , wordSet , mes_status_04_12 , 
                                   PSTATUS_SKILL_STR_PARAM_NUM_X , PSTATUS_SKILL_STR_PARAM_NUM_Y , PSTATUS_STR_COL_VALUE );
    WORDSET_Delete( wordSet );
  }

  //防御
  PSTATUS_UTIL_DrawStrFunc( work , skillWork->upBmpWin[PSBT_DEF] , mes_status_04_04 ,
                            PSTATUS_SKILL_STR_PARAM_X , PSTATUS_SKILL_STR_PARAM_Y , PSTATUS_STR_COL_TITLE );
  {
    WORDSET *wordSet = WORDSET_Create( work->heapId );
    u32 no = PP_Get( pp , ID_PARA_def , NULL );
    WORDSET_RegisterNumber( wordSet , 0 , no , 3 , STR_NUM_DISP_SPACE , STR_NUM_CODE_DEFAULT );
    PSTATUS_UTIL_DrawValueStrFuncRight( work , skillWork->upBmpWin[PSBT_DEF] , wordSet , mes_status_04_13 , 
                                   PSTATUS_SKILL_STR_PARAM_NUM_X , PSTATUS_SKILL_STR_PARAM_NUM_Y , PSTATUS_STR_COL_VALUE );
    WORDSET_Delete( wordSet );
  }

  //特攻
  PSTATUS_UTIL_DrawStrFunc( work , skillWork->upBmpWin[PSBT_SATK] , mes_status_04_05 ,
                            PSTATUS_SKILL_STR_PARAM_X , PSTATUS_SKILL_STR_PARAM_Y , PSTATUS_STR_COL_TITLE );
  {
    WORDSET *wordSet = WORDSET_Create( work->heapId );
    u32 no = PP_Get( pp , ID_PARA_spepow , NULL );
    WORDSET_RegisterNumber( wordSet , 0 , no , 3 , STR_NUM_DISP_SPACE , STR_NUM_CODE_DEFAULT );
    PSTATUS_UTIL_DrawValueStrFuncRight( work , skillWork->upBmpWin[PSBT_SATK] , wordSet , mes_status_04_14 , 
                                   PSTATUS_SKILL_STR_PARAM_NUM_X , PSTATUS_SKILL_STR_PARAM_NUM_Y , PSTATUS_STR_COL_VALUE );
    WORDSET_Delete( wordSet );
  }

  //特防
  PSTATUS_UTIL_DrawStrFunc( work , skillWork->upBmpWin[PSBT_SDEF] , mes_status_04_06 ,
                            PSTATUS_SKILL_STR_PARAM_X , PSTATUS_SKILL_STR_PARAM_Y , PSTATUS_STR_COL_TITLE );
  {
    WORDSET *wordSet = WORDSET_Create( work->heapId );
    u32 no = PP_Get( pp , ID_PARA_spedef , NULL );
    WORDSET_RegisterNumber( wordSet , 0 , no , 3 , STR_NUM_DISP_SPACE , STR_NUM_CODE_DEFAULT );
    PSTATUS_UTIL_DrawValueStrFuncRight( work , skillWork->upBmpWin[PSBT_SDEF] , wordSet , mes_status_04_15 , 
                                   PSTATUS_SKILL_STR_PARAM_NUM_X , PSTATUS_SKILL_STR_PARAM_NUM_Y , PSTATUS_STR_COL_VALUE );
    WORDSET_Delete( wordSet );
  }

  //素早さ
  PSTATUS_UTIL_DrawStrFunc( work , skillWork->upBmpWin[PSBT_SPD] , mes_status_04_07 ,
                            PSTATUS_SKILL_STR_PARAM_X , PSTATUS_SKILL_STR_PARAM_Y , PSTATUS_STR_COL_TITLE );
  {
    WORDSET *wordSet = WORDSET_Create( work->heapId );
    u32 no = PP_Get( pp , ID_PARA_agi , NULL );
    WORDSET_RegisterNumber( wordSet , 0 , no , 3 , STR_NUM_DISP_SPACE , STR_NUM_CODE_DEFAULT );
    PSTATUS_UTIL_DrawValueStrFuncRight( work , skillWork->upBmpWin[PSBT_SPD] , wordSet , mes_status_04_16 , 
                                   PSTATUS_SKILL_STR_PARAM_NUM_X , PSTATUS_SKILL_STR_PARAM_NUM_Y , PSTATUS_STR_COL_VALUE );
    WORDSET_Delete( wordSet );
  }

  //特性
  PSTATUS_UTIL_DrawStrFunc( work , skillWork->upBmpWin[PSBT_TOKUSEI] , mes_status_04_08 ,
                            PSTATUS_SKILL_STR_TOKUSEI_X , PSTATUS_SKILL_STR_TOKUSEI_Y , PSTATUS_STR_COL_TITLE );
  {
    u32 no = PP_Get( pp , ID_PARA_speabino , NULL );
    {
      GFL_MSGDATA *msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_tokuseiinfo_dat , work->heapId );
      STRBUF *srcStr;

      srcStr = GFL_MSG_CreateString( msgHandle , no ); 
      PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( skillWork->upBmpWin[PSBT_TOKUSEI] ) , 
              PSTATUS_SKILL_STR_TOKUSEI_INFO_X , PSTATUS_SKILL_STR_TOKUSEI_INFO_Y , srcStr , 
              work->fontHandle , PSTATUS_STR_COL_VALUE );

      GFL_STR_DeleteBuffer( srcStr );
      GFL_MSG_Delete( msgHandle );
    }
    {
      GFL_MSGDATA *msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_tokusei_dat , work->heapId );
      STRBUF *srcStr;

      srcStr = GFL_MSG_CreateString( msgHandle , no ); 
      PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( skillWork->upBmpWin[PSBT_TOKUSEI] ) , 
              PSTATUS_SKILL_STR_TOKUSEI_NAME_X , PSTATUS_SKILL_STR_TOKUSEI_NAME_Y , srcStr , 
              work->fontHandle , PSTATUS_STR_COL_VALUE );

      GFL_STR_DeleteBuffer( srcStr );
      GFL_MSG_Delete( msgHandle );
    }
  }
  
  skillWork->isUpdateStrStatus = TRUE;
}

//--------------------------------------------------------------
//	HPバー表示
//--------------------------------------------------------------
static void PSTATUS_SKILL_DrawHPBar( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork )
{
  const POKEMON_PARAM *pp = PSTATUS_UTIL_GetCurrentPP( work );
  const u8 hpMax = PP_Get( pp , ID_PARA_hpmax , NULL );
  const u8 hp = PP_Get( pp , ID_PARA_hp , NULL );

  const u8 rate = 100*hp/hpMax;
  u8 len = PSTATUS_SKILL_HPBAR_LEN*hp/hpMax;
  u8 inCol,outCol;
  GFL_BMP_DATA *bmp = GFL_BMPWIN_GetBmp( skillWork->upBmpWin[PSBT_HP_GAUGE] );

  if( hp != 0 && len == 0 )
  {
    len = 1;
  }
  if( hp != hpMax && len == PSTATUS_SKILL_HPBAR_LEN )
  {
    len = PSTATUS_SKILL_HPBAR_LEN-1;
  }
  
  //色決定
  if( rate <= 25 )
  {
    inCol  = PSTATUS_SKILL_HPBAR_COL_RED_IN;
    outCol = PSTATUS_SKILL_HPBAR_COL_RED_OUT;
  }
  else
  if( rate <= 50 )
  {
    inCol  = PSTATUS_SKILL_HPBAR_COL_YELLOW_IN;
    outCol = PSTATUS_SKILL_HPBAR_COL_YELLOW_OUT;
  }
  else
  {
    inCol  = PSTATUS_SKILL_HPBAR_COL_GREEN_IN;
    outCol = PSTATUS_SKILL_HPBAR_COL_GREEN_OUT;
  }
  
  GFL_BMP_Fill( bmp , PSTATUS_SKILL_HPBAR_LEFT , PSTATUS_SKILL_HPBAR_TOP ,
                len , 1 , outCol );
  GFL_BMP_Fill( bmp , PSTATUS_SKILL_HPBAR_LEFT , PSTATUS_SKILL_HPBAR_TOP+1 ,
                len , 2 , inCol );
  GFL_BMP_Fill( bmp , PSTATUS_SKILL_HPBAR_LEFT , PSTATUS_SKILL_HPBAR_TOP+3 ,
                len , 1 , outCol );

}

#pragma mark [>SkillPlate
//--------------------------------------------------------------
//	スキルプレートの初期化
//--------------------------------------------------------------
static void PSTATUS_SKILL_InitPlate( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork , PSTATUS_SKILL_PLATE *plateWork , const u8 idx )
{
  plateWork->idx = idx;
  plateWork->isUpdateStr = FALSE;

  {
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = platePos[idx][0] * 8;
    cellInitData.pos_y = platePos[idx][1] * 8;
    cellInitData.softpri = 10;
    cellInitData.bgpri = 2;
    cellInitData.anmseq = 0;
    
    plateWork->clwkPlate = GFL_CLACT_WK_Create( work->cellUnit ,
              work->cellRes[SCR_NCG_SKILL],
              work->cellRes[SCR_PLT_SKILL],
              work->cellRes[SCR_ANM_SKILL],
              &cellInitData ,CLSYS_DEFREND_MAIN , work->heapId );

    GFL_CLACT_WK_SetDrawEnable( plateWork->clwkPlate , FALSE );
  } 
  
}

//--------------------------------------------------------------
//	スキルプレートの開放
//--------------------------------------------------------------
static void PSTATUS_SKILL_TermPlate( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork , PSTATUS_SKILL_PLATE *plateWork )
{
  GFL_CLACT_WK_Remove( plateWork->clwkPlate );
}

//--------------------------------------------------------------
//	スキルプレートの更新
//--------------------------------------------------------------
static void PSTATUS_SKILL_UpdatePlate( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork , PSTATUS_SKILL_PLATE *plateWork )
{
  if( plateWork->isUpdateStr == TRUE )
  {
    if( PRINTSYS_QUE_IsExistTarget( work->printQue , GFL_BMPWIN_GetBmp( plateWork->bmpWin )) == FALSE )
    {
      GFL_BMPWIN_MakeTransWindow_VBlank( plateWork->bmpWin );
      plateWork->isUpdateStr = FALSE;
    }
  }
}

//--------------------------------------------------------------
//	スキルプレートの表示
//--------------------------------------------------------------
static void PSTATUS_SKILL_DispPlate( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork , PSTATUS_SKILL_PLATE *plateWork )
{
  const POKEMON_PARAM *pp = PSTATUS_UTIL_GetCurrentPP( work );
  
  plateWork->bmpWin = GFL_BMPWIN_Create( PSTATUS_BG_PARAM ,
                platePos[plateWork->idx][0]+PSTATUS_SKILL_PLATE_WIN_LEFT , platePos[plateWork->idx][1]+PSTATUS_SKILL_PLATE_WIN_TOP ,
                PSTATUS_SKILL_PLATE_WIN_WIDTH , PSTATUS_SKILL_PLATE_WIN_HEIGHT ,
                PSTATUS_BG_PLT_FONT , GFL_BMP_CHRAREA_GET_B );
  {
    GFL_MSGDATA *msgHandelSkill = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_wazaname_dat , work->heapId );
    const u32 wazaNo = PP_Get( pp , ID_PARA_waza1+plateWork->idx , NULL );
    {
      STRBUF *srcStr = GFL_MSG_CreateString( msgHandelSkill , wazaNo );

      PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( plateWork->bmpWin ) , 
              PSTATUS_SKILL_PLATE_WAZANAME_X , PSTATUS_SKILL_PLATE_WAZANAME_Y , srcStr , 
              work->fontHandle , PSTATUS_STR_COL_BLACK );

      GFL_STR_DeleteBuffer( srcStr );
      GFL_MSG_Delete( msgHandelSkill );
    }
    if( wazaNo != 0 )
    {
      PSTATUS_UTIL_DrawStrFunc( work , plateWork->bmpWin , mes_status_06_08 ,
                                PSTATUS_SKILL_PLATE_PPSTR_X , PSTATUS_SKILL_PLATE_PPSTR_Y , PSTATUS_STR_COL_BLACK );
      {
        WORDSET *wordSet = WORDSET_Create( work->heapId );
        const u32 nowpp = PP_Get( pp , ID_PARA_pp1+plateWork->idx , NULL );
        WORDSET_RegisterNumber( wordSet , 0 , nowpp , 3 , STR_NUM_DISP_SPACE , STR_NUM_CODE_DEFAULT );
        PSTATUS_UTIL_DrawValueStrFuncRight( work , plateWork->bmpWin , wordSet , mes_status_06_09 , 
                                       PSTATUS_SKILL_PLATE_NOWPP_X , PSTATUS_SKILL_PLATE_NOWPP_Y , PSTATUS_STR_COL_BLACK );
        WORDSET_Delete( wordSet );
      }
      PSTATUS_UTIL_DrawStrFunc( work , plateWork->bmpWin , mes_status_04_09 ,
                                PSTATUS_SKILL_PLATE_SLASH_X , PSTATUS_SKILL_PLATE_SLASH_Y , PSTATUS_STR_COL_BLACK );
      {
        WORDSET *wordSet = WORDSET_Create( work->heapId );
        const u32 maxpp = PP_Get( pp , ID_PARA_pp1+plateWork->idx , NULL );
        WORDSET_RegisterNumber( wordSet , 0 , maxpp , 3 , STR_NUM_DISP_SPACE , STR_NUM_CODE_DEFAULT );
        PSTATUS_UTIL_DrawValueStrFunc( work , plateWork->bmpWin , wordSet , mes_status_06_14 , 
                                       PSTATUS_SKILL_PLATE_MAXPP_X , PSTATUS_SKILL_PLATE_MAXPP_Y , PSTATUS_STR_COL_BLACK );
        WORDSET_Delete( wordSet );
      }
    }
    else
    {
      PSTATUS_UTIL_DrawStrFunc( work , plateWork->bmpWin , mes_status_06_27 ,
                                PSTATUS_SKILL_PLATE_NONE_X , PSTATUS_SKILL_PLATE_NONE_Y , PSTATUS_STR_COL_BLACK );
      
    }
  }

  GFL_CLACT_WK_SetDrawEnable( plateWork->clwkPlate , TRUE );
  plateWork->isUpdateStr = TRUE;
}

//--------------------------------------------------------------
//	スキルプレートの消去
//--------------------------------------------------------------
static void PSTATUS_SKILL_ClearPlate( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork , PSTATUS_SKILL_PLATE *plateWork )
{
  GFL_CLACT_WK_SetDrawEnable( plateWork->clwkPlate , FALSE );
  GFL_BMPWIN_Delete( plateWork->bmpWin );
}
