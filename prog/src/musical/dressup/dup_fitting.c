//======================================================================
/**
 * @file  dressup_system.h
 * @brief ドレスアップ 試着メイン
 * @author  ariizumi
 * @data  09/02/10
 */
//======================================================================
#include <gflib.h>
#include <calctool.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/wipe.h"

#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"
#include "musical_item.naix"
#include "dressup_gra.naix"
#include "msg/msg_dress_up.h"

#include "print/printsys.h"
#include "print/wordset.h"
#include "infowin/infowin.h"
#include "test/ariizumi/ari_debug.h"
#include "savedata/musical_save.h"
#include "system/bmp_winframe.h"
#include "musical/musical_local.h"
#include "musical/mus_poke_draw.h"
#include "musical/mus_item_draw.h"
#include "musical/mus_item_data.h"
#include "musical/musical_camera_def.h"
#include "dup_local_def.h"
#include "dup_snd_def.h"
#include "dup_fitting.h"
#include "dup_fitting_item.h"

//======================================================================
//  define
//======================================================================
#pragma mark [> define fitting

#define FIT_FRAME_MAIN_3D   GFL_BG_FRAME0_M
#define FIT_FRAME_MAIN_INFO   GFL_BG_FRAME1_M
#define FIT_FRAME_MAIN_CASE   GFL_BG_FRAME2_M //読み替えて共用？もしくは3D
#define FIT_FRAME_MAIN_MIRROR GFL_BG_FRAME2_M //読み替えて共用？
#define FIT_FRAME_MAIN_BG   GFL_BG_FRAME3_M
#define FIT_FRAME_SUB_BG    GFL_BG_FRAME3_S
#define FIT_FRAME_SUB_MSG    GFL_BG_FRAME3_S
#define FIT_FRAME_SUB_CURTAIN_L    GFL_BG_FRAME2_S
#define FIT_FRAME_SUB_CURTAIN_R    GFL_BG_FRAME1_S
#define FIT_FRAME_SUB_TOP    GFL_BG_FRAME0_S

#define FIT_PLT_OBJ_MAIN_BUTTON (0)
#define FIT_PLT_OBJ_MAIN_EFFECT (2)

#define FIT_PLT_BG_SUB_MSGWIN (0xC)
#define FIT_PLT_BG_SUB_FONT (0xE)

#define FIT_PAL_INFO    (0xE)

#define FIT_MSGWIN_CHAR_TOP (128)  //仮数値BG決定してから

#define FIT_ANIME_SPD (6)
#define FIT_ANIME_MAX (192)

#define DEG_TO_U16(val) ((val)*0x10000/360)
#define U16_TO_DEG(val) ((val)*360/0x10000)

#define FIT_CHECK_BUTTON_TOP    (192-24)
#define FIT_CHECK_BUTTON_BOTTOM (192)
#define FIT_CHECK_BUTTON_LEFT   (128-32)
#define FIT_CHECK_BUTTON_RIGHT  (128+32)

//BBD用座標変換(カメラの幅から計算
#define FIT_POS_X(val)    MUSICAL_POS_X(val)  
#define FIT_POS_Y(val)    MUSICAL_POS_Y(val)  
#define FIT_POS_X_FX(val) MUSICAL_POS_X_FX(val)
#define FIT_POS_Y_FX(val) MUSICAL_POS_Y_FX(val)

//ポケモン表示位置
static const int FIT_POKE_POS_X = 128;
static const int FIT_POKE_POS_Y = 112;
static const fx32 FIT_POKE_POS_X_FX = FIT_POS_X( FIT_POKE_POS_X );
static const fx32 FIT_POKE_POS_Y_FX = FIT_POS_Y( FIT_POKE_POS_Y );
static const fx32 FIT_POKE_POS_Z_FX = FX32_CONST(40.0f);

//アイテム表示個数
static const u16 ITEM_LIST_NUM = 14;
//static const u16 ITEM_LIST_NUM = 48;
static const u16 ITEM_FIELD_NUM = 64;
static const u16 ITEM_EQUIP_NUM = MUS_POKE_EQUIP_MAX;//9
static const u16 ITEM_DISP_NUM = ITEM_LIST_NUM+ITEM_FIELD_NUM+ITEM_EQUIP_NUM+1; //ペンで持つ分

//アイテムリスト系定義
//中心点
static const int LIST_CENTER_X = 128;
static const int LIST_CENTER_Y = 80;
static const int LIST_CENTER_X_FX = FX32_CONST(LIST_CENTER_X);
static const int LIST_CENTER_Y_FX = FX32_CONST(LIST_CENTER_Y);
//サイズ
static const u16 LIST_SIZE_X = 72;
static const u16 LIST_SIZE_Y = 48;
static const float LIST_SIZE_RATIO = ((float)LIST_SIZE_Y/(float)LIST_SIZE_X);

#define DUP_FIT_CALC_OVAL_RATIO(val) (val*LIST_SIZE_X/LIST_SIZE_Y)

//サイズ
static const int LIST_TPHIT_MAX_X = LIST_SIZE_X+20;
static const int LIST_TPHIT_MIN_X = LIST_SIZE_X-20;
static const int LIST_TPHIT_MAX_Y = LIST_SIZE_Y+20;
static const int LIST_TPHIT_MIN_Y = LIST_SIZE_Y-20;
static const float LIST_TPHIT_RATIO_MAX = (float)LIST_TPHIT_MAX_Y/(float)LIST_TPHIT_MAX_X;
static const float LIST_TPHIT_RATIO_MIN = (float)LIST_TPHIT_MIN_Y/(float)LIST_TPHIT_MIN_X;
//縮小が始まる位置(90度)/消え始める位置(155度)/消える位置(170度)
static const u16 LIST_SIZE_DEPTH[3] = {0x4000,0x6E00,0x78E0};
//リストの回転制御
static const u16 LIST_ROTATE_ANGLE = DEG_TO_U16(1.25);
static const u16 LIST_ROTATE_LIMIT = DEG_TO_U16(90);

//アイテム1個の間隔
static const u16 LIST_ONE_ANGLE = 0x10000/ITEM_LIST_NUM;
    //アイテムのマックスが変動するので廃止
//static const u32 LIST_FULL_ANGLE = LIST_ONE_ANGLE*MUSICAL_ITEM_MAX;

//リストのZ座標
static const fx32 LIST_DEPTH_BASE = FX32_CONST( 20.0f );

//フィールド(置いてある)アイテム系
static const fx32 FIELD_ITEM_DEPTH = FX32_CONST(10.0f); //リストは0～8

//持ってるアイテム系
static const fx32 HOLD_ITEM_DEPTH = FX32_CONST(60.0f);
static const fx32 RETURN_LIST_ITEM_DEPTH = FX32_CONST(30.0f);

//装備アイテム系
static const u16 HOLD_ITEM_SNAP_LENGTH = 12;
static const fx32 EQUIP_ITEM_DEPTH = FX32_CONST(50.5f);
static const fx32 EQUIP_ITEM_DEPTH_BACK  = FX32_CONST(20.5f);
static const fx32 EQUIP_ITEM_DEPTH_FRONT = FX32_CONST(55.5f);


//リストに戻るアニメーション
static const u16 ITEM_RETURN_ANIME_CNT = 10;
static const fx32 ITEM_RETURN_DEPTH = FX32_CONST(70.0f);
//移動キャンセルでフィールドに戻るとき
static const u16 ITEM_RETURN_POS_CNT = 10;
//表示アイテムの点滅間隔
static const u8 ITEM_LIST_NEW_BLINK = 45;

//リストアイテム揺れ系
static const u8 ITEM_SWING_ORIGIN_OFSSET = 8;
static const u16 ITEM_SWING_ANGLE_MAX = 0x3000;
static const u16 ITEM_SWING_ANGLE_ADD_VALUE = 0x100;
static const u16 ITEM_SWING_ANGLE_SORT_MAX = 0x3000;  //ソート時のアニメ角度MAX
static const u16 ITEM_SWING_ANGLE_SORT_ADD_VALUE = 0x200; 

//表示順位メモ
//↑
//保持アイテム      10
//List戻り中       9
//装備品         8.5
//アイテムリスト    8～0
//フィールドアイテム  -10～-10-0.1*表示個数
//ポケモン        -40000  //たぶんmcssで座標変換が入ってる・・・これで一番下に出る
//↓

//スクロール系
#define DUP_CURTAIN_SCROLL_MAX (128)
#define DUP_CURTAIN_SCROLL_VALUE (8)

//Msg系
#define DUP_MSGWIN_LEFT (1)
#define DUP_MSGWIN_TOP (19)
#define DUP_MSGWIN_WIDTH (30)
#define DUP_MSGWIN_HEIGHT (4)

#pragma mark [> define check
//チェック画面
#define BUTTON_SIZE_X (64)
#define BUTTON_SIZE_Y (64)

#define BUTTON_ASSEPT_POS_X (BUTTON_SIZE_X/2)
#define BUTTON_ASSEPT_POS_Y (192-BUTTON_SIZE_Y/2)
#define BUTTON_RETURN_POS_X (256-BUTTON_SIZE_X/2)
#define BUTTON_RETURN_POS_Y (192-BUTTON_SIZE_Y/2)

#pragma mark [> define demo
#define DEMO_ROT_ITEM_NUM (5)

#pragma mark [> define effect
#define EFFECT_UP_NUM (8)   //上画面のきらきらの数
#define EFFECT_UP_LIMIT_X (56)
#define EFFECT_UP_LIMIT_Y (90)
#define EFFECT_END_EFFECT_CNT (60)   //上画面のきらきらの数

//======================================================================
//  enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  DUS_FITTING_DEMO,
  DUS_FITTING_MAIN,
  DUS_GO_CHECK,
  DUS_CHECK_MAIN,
  DUS_WAIT_COMM_PLAYER,
  DUS_WAIT_COMM_STRM,
  DUS_RETURN_FITTING,
  
  DUS_FADEIN_WAIT_SND,
  DUS_FADEIN_WAIT,
  DUS_FADEOUT_WAIT,
  DUS_WAIT_END_EFFECT,
  
}DUP_STATE;

typedef enum
{
  IG_NONE,
  IG_LIST,
  IG_FIELD,
  IG_EQUIP,
  IG_ANIME,
}ITEM_GROUPE;

typedef enum
{
  DOR_BUTTON_PLT,
  DOR_EFFECT_DOWN_PLT,
  DOR_EFFECT_UP_PLT,
  DOR_BUTTON_NCG,
  DOR_EFFECT_DOWN_NCG,
  DOR_EFFECT_UP_NCG,
  DOR_BUTTON_ANM,
  DOR_EFFECT_DOWN_ANM,
  DOR_EFFECT_UP_ANM,
  
  DUR_PLT_START = DOR_BUTTON_PLT,
  DUR_PLT_END =   DOR_EFFECT_UP_PLT,
  DUR_NCG_START = DOR_BUTTON_NCG,
  DUR_NCG_END =   DOR_EFFECT_UP_NCG,
  DUR_ANM_START = DOR_BUTTON_ANM,
  DUR_ANM_END =   DOR_EFFECT_UP_ANM,
  
  DUP_OBJ_RES_MAX,
}DUP_OBJ_RES;

//======================================================================
//  typedef struct
//======================================================================
#pragma mark [> struct

struct _FITTING_WORK
{
  HEAPID heapId;
  FITTING_INIT_WORK *initWork;
  GFL_TCB *vBlankTcb;
  
  DUP_STATE state;
  int     animeCnt; 
  
  //TP系
  BOOL tpIsTrg;
  BOOL tpIsTouch;
  u32 tpx,tpy;
  u32 befTpx,befTpy;
  u16 befAngle;
  BOOL befAngleEnable;  //↑の変数の有効性
  MUS_POKE_EQUIP_POS  snapPos;  //装備に吸着してる状態
  
  FIT_ITEM_WORK *holdItem;  //保持しているアイテム
  ITEM_GROUPE   holdItemType;
  ITEM_GROUPE   befItemType;
  GFL_POINT     befItemPos;

  MUS_ITEM_DRAW_WORK *shadowItem; //影用
  u16         shadowItemId;
  int         shadowItemResIdx;

  //newの文字用
  u32         newPicResIdx;
  u32         newPicBbdIdx[ITEM_LIST_NUM];
  u16         newPicBlinkCnt;

  //ソート系
  BOOL isSortAnime;   //ソート中判定
  u32  sortAnimeMoveAngle;  //ソートアニメ移動分
  u32  sortAnimeEndAngle;   //ソートアニメ完了位置
  MUS_POKE_EQUIP_USER sortType; //今のソート種類

  //アイテム系
  u16             totalItemNum;
  ITEM_STATE      itemStateBase[MUSICAL_ITEM_MAX];
  ITEM_STATE      *itemState[MUSICAL_ITEM_MAX];
  FIT_ITEM_GROUP  *itemGroupList;
  FIT_ITEM_GROUP  *itemGroupField;
  FIT_ITEM_GROUP  *itemGroupEquip;
  //リストに戻るときなど操作不可でアニメ後消滅する物
  FIT_ITEM_GROUP  *itemGroupAnime;

  u16 listAngle;
  s16 listSpeed;
  u16 listHoldMove; //持っているときの移動量
  s32 listTotalMove;  //リスト全体の移動量
  s16 listSwingAngle;      //リストの揺れ
  s16 listSwingSpeed;
  //デモ用
  BOOL isDemo;
  u16  demoCnt;
  u8   demoPhase;
  u8   demoMoveValue; //回転させる量(アイテム個数で
  
  //Obj系
  GFL_CLUNIT  *cellUnit;
  GFL_CLWK  *buttonCell[2];
  GFL_CLWK  *demoPen;
  u32     objResIdx[DUP_OBJ_RES_MAX];

  
  //3D描画に関わる物
  MUS_POKE_DRAW_SYSTEM  *drawSys;
  MUS_POKE_DRAW_WORK    *drawWork;
  MUS_ITEM_DRAW_SYSTEM  *itemDrawSys;
  GFL_G3D_CAMERA      *camera;
  GFL_BBD_SYS       *bbdSys;
  
  //演出系
  GFL_CLWK  *clwkEffectUp[EFFECT_UP_NUM];
  GFL_CLWK  *clwkEffectDown;
  u8        effUpCnt[EFFECT_UP_NUM];
  u8        endEffCnt;
  u8        bgScrollCnt;

  //SE用
  u16       listSeWaitCnt;
  s16       adjustMoveValue;

  //上画面系
  u8      curtainScrollCnt;
  BOOL    isOpenCurtain;
  BOOL    isUpdateMsg;  //描画中
  BOOL    isDispMsg;    //表示中
  
  //Msg系
  GFL_MSGDATA *msgHandle;
  GFL_FONT *fontHandle;
  PRINT_QUE *printQue;
  GFL_BMPWIN *msgWin;
};

//======================================================================
//  proto
//======================================================================
#pragma mark [> proto Fitting

static void DUP_FIT_FittingMain(  FITTING_WORK *work );

static void DUP_FIT_VBlankFunc(GFL_TCB *, void * );

static void DUP_FIT_SetupGraphic( FITTING_WORK *work );
static void DUP_FIT_SetupBgObj( FITTING_WORK *work );
static void DUP_FIT_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane );
static void DUP_FIT_SetupPokemon( FITTING_WORK *work );
static void DUP_FIT_SetupItem( FITTING_WORK *work );
static void DUP_FIT_SetupStartItem( FITTING_WORK *work , const u16 itemId );
static void DUP_FIT_TermItem( FITTING_WORK *work );
static void DUP_FIT_CalcItemListAngle( FITTING_WORK *work , u16 angle , s16 moveAngle , u16 sizeX , u16 sizeY );
static void DUP_FIT_CheckItemListPallet( FITTING_WORK *work );

static void DUP_FIT_UpdateTpMain( FITTING_WORK *work );
static void DUP_FIT_UpdateTpList( FITTING_WORK *work , s16 subX , s16 subY );
static const BOOL DUP_FIT_UpdateTpHoldItem( FITTING_WORK *work , FIT_ITEM_GROUP *itemGroupe , ITEM_GROUPE groupeType );
static void DUP_FIT_UpdateTpHoldingItem( FITTING_WORK *work , const BOOL playSnapSe );
static void DUP_FIT_UpdateTpDropItemToField( FITTING_WORK *work );
static void DUP_FIT_UpdateTpDropItemToList( FITTING_WORK *work , const BOOL isMove );
static void DUP_FIT_UpdateTpDropItemToEquip(  FITTING_WORK *work );
static void DUP_FIT_UpdateTpCancelDropItem( FITTING_WORK *work );
static const BOOL DUP_FIT_UpdateTpPokeCheck( FITTING_WORK *work );
static void DUP_FIT_UpdateSortAnime( FITTING_WORK *work );
static void DUP_FIT_UpdateSwingItem( FITTING_WORK *work );

static const BOOL DUP_FIT_CheckIsEquipItem( FITTING_WORK *work , const MUS_POKE_EQUIP_POS pos);
static MUS_POKE_EQUIP_POS DUP_FIT_SearchEquipPosition( FITTING_WORK *work , MUS_ITEM_DRAW_WORK *itemDrawWork , GFL_POINT *pos , u16 *len );
static void DUP_FIT_UpdateItemAnime( FITTING_WORK *work );
static void DUP_FIT_UpdateShadow( FITTING_WORK *work );
static void DUP_FIT_CreateItemListToField( FITTING_WORK *work );
static void DUP_FIT_SortItemIdx( FITTING_WORK *work , const MUS_POKE_EQUIP_USER ePos , const u16 startIdx );
static const u16 DUP_FIT_CalcSortPriority( FITTING_WORK *work , ITEM_STATE *itemState , const MUS_POKE_EQUIP_USER ePos );

static BOOL DUP_FIT_CheckPointInOval( s16 subX , s16 subY , s16 size , float ratioYX );

static void DUP_FIT_ChangeStateAnime( FITTING_WORK *work );

#pragma mark [> proto Check
static FITTING_RETURN DUP_CHECK_CheckMain( FITTING_WORK *work );
static void DUP_CHECK_UpdateTpMain( FITTING_WORK *work );
static void DUP_CHECK_UpdateTpHoldingItem( FITTING_WORK *work );
static void DUP_CHECK_ResetItemAngle( FITTING_WORK *work );
static void DUP_CHECK_SaveNowEquip( FITTING_WORK *work );

#pragma mark [> proto Demo
static void DUP_DEMO_DemoMain( FITTING_WORK *work );
static void DUP_DEMO_DemoStart( FITTING_WORK *work );
static void DUP_DEMO_DemoEnd( FITTING_WORK *work );

static void DUP_DEMO_DemoPhaseListRot( FITTING_WORK *work , const u16 start , const s32 value , const u16 cnt);
static void DUP_DEMO_DemoPhaseWait( FITTING_WORK *work , const u16 cnt );
static void DUP_DEMO_DemoPhaseDragPen( FITTING_WORK *work , const GFL_POINT *start , const GFL_POINT *end , const u16 cnt );

#pragma mark [> proto Message
static void DUP_FIT_InitMessage( FITTING_WORK *work );
static void DUP_FIT_TermMessage( FITTING_WORK *work );
static void DUP_FIT_DrawMessage( FITTING_WORK *work , u16 msgId , WORDSET *wordSet );

#pragma mark [> proto Effect
static void DUP_EFFECT_InitCell( FITTING_WORK *work );
static void DUP_EFFECT_TermCell( FITTING_WORK *work );
static void DUP_EFFECT_UpdateCell( FITTING_WORK *work );

static const GFL_DISP_VRAM vramBank = {
  GX_VRAM_BG_64_E,       // メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
  GX_VRAM_SUB_BG_128_C,     // サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
  GX_VRAM_OBJ_16_F,       // メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
  GX_VRAM_SUB_OBJ_16_I,     // サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
  GX_VRAM_TEX_012_ABD,        // テクスチャイメージスロット
  GX_VRAM_TEXPLTT_0_G,      // テクスチャパレットスロット
  GX_OBJVRAMMODE_CHAR_1D_64K,
  GX_OBJVRAMMODE_CHAR_1D_32K
};
//  A テクスチャ
//  B テクスチャ
//  C SubBg
//  D MainBg
//  E MainObj
//  F テクスチャパレット
//  G テクスチャパレット
//  H None
//  I SubObj


//--------------------------------------------------------------
//  試着メイン 初期化
//--------------------------------------------------------------
FITTING_WORK* DUP_FIT_InitFitting( FITTING_INIT_WORK *initWork , HEAPID heapId )
{
  FITTING_WORK *work = GFL_HEAP_AllocClearMemory( heapId , sizeof( FITTING_WORK ));

  work->heapId = heapId;
  work->initWork = initWork;
  work->state = DUS_FADEIN_WAIT_SND;
  work->animeCnt = 0;
  work->tpIsTrg = FALSE;
  work->tpIsTouch = FALSE;
  work->befAngleEnable = FALSE;
  work->holdItem = NULL;
  work->holdItemType = IG_NONE;
  work->befItemType = IG_NONE;
  work->isSortAnime = FALSE;
  work->sortType = MUS_POKE_EQUIP_USER_MAX;
  work->listAngle = 0;
  work->listSwingAngle = 0;
  work->listSwingSpeed = 0;
  work->isDemo = FALSE;
  work->curtainScrollCnt = 0;
  work->isOpenCurtain = FALSE;
  work->isDispMsg = FALSE;
  work->isUpdateMsg = FALSE;
  work->bgScrollCnt = 0;
  work->listTotalMove = 0;

  work->listSpeed = 0;
  work->snapPos = MUS_POKE_EQU_INVALID;
  DUP_FIT_SetupGraphic( work );
  DUP_FIT_SetupBgObj( work );
  DUP_FIT_SetupPokemon( work );
  DUP_FIT_SetupItem( work );
  DUP_EFFECT_InitCell( work );
  DUP_FIT_InitMessage( work );

  INFOWIN_Init( FIT_FRAME_MAIN_INFO,FIT_PAL_INFO,NULL,work->heapId);
  work->vBlankTcb = GFUser_VIntr_CreateTCB( DUP_FIT_VBlankFunc , work , 8 );
  
  WIPE_SYS_Start( WIPE_PATTERN_FSAM , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
                  WIPE_FADE_WHITE , 18 , WIPE_DEF_SYNC , work->heapId );
  
  //初回回転演出
  {
    //デフォルトでソート
    const u32 startAngle = work->listTotalMove + 0x18000;
    u16 startIdx;
    work->listSeWaitCnt = 10; //鳴るのを妨害

    if( startAngle >= work->totalItemNum * LIST_ONE_ANGLE )
    {
      startIdx = (startAngle-(work->totalItemNum * LIST_ONE_ANGLE)) / LIST_ONE_ANGLE;
    }
    else
    {
      startIdx = startAngle / LIST_ONE_ANGLE;
    }
    DUP_FIT_SortItemIdx( work , MUS_POKE_EQUIP_USER_MAX , startIdx );
    work->sortType = MUS_POKE_EQUIP_USER_MAX;
    work->isSortAnime = TRUE;
    work->sortAnimeMoveAngle = 0;
    work->sortAnimeEndAngle = 0x10000 + work->listAngle%LIST_ONE_ANGLE;
  }

  PMSND_PlayBGM( SEQ_BGM_MSL_DRESSUP );

  GFL_NET_WirelessIconEasy_HoldLCD( FALSE , work->heapId );
  GFL_NET_ReloadIcon();

  return work;
}

//--------------------------------------------------------------
//  試着メイン 開放
//--------------------------------------------------------------
void  DUP_FIT_TermFitting( FITTING_WORK *work )
{
  u8 i;

  PMSND_StopBGM();

  INFOWIN_Exit();
  DUP_FIT_TermMessage( work );

  DUP_EFFECT_TermCell( work );
  GFL_CLACT_WK_Remove( work->buttonCell[0] );
  GFL_CLACT_WK_Remove( work->buttonCell[1] );
  GFL_CLACT_WK_Remove( work->demoPen );
  //OBJ
  for( i=DUR_PLT_START ; i<=DUR_PLT_END ; i++ )
  {
    GFL_CLGRP_PLTT_Release( work->objResIdx[i] );
  }
  for( i=DUR_NCG_START ; i<=DUR_NCG_END ; i++ )
  {
    GFL_CLGRP_CGR_Release( work->objResIdx[i] );
  }
  for( i=DUR_ANM_START ; i<=DUR_ANM_END ; i++ )
  {
    GFL_CLGRP_CELLANIM_Release( work->objResIdx[i] );
  }

  GFL_CLACT_UNIT_Delete( work->cellUnit );
  GFL_CLACT_SYS_Delete();

  DUP_FIT_TermItem( work );
  MUS_POKE_DRAW_Del( work->drawSys , work->drawWork );
  MUS_POKE_DRAW_TermSystem( work->drawSys );

  GFL_TCB_DeleteTask( work->vBlankTcb );
  GFL_BBD_DeleteSys( work->bbdSys );
  GFL_G3D_CAMERA_Delete( work->camera );
  GFL_G3D_Exit();
  GFL_BG_FreeBGControl( FIT_FRAME_MAIN_3D );
  GFL_BG_FreeBGControl( FIT_FRAME_MAIN_INFO );
  GFL_BG_FreeBGControl( FIT_FRAME_MAIN_CASE );
  GFL_BG_FreeBGControl( FIT_FRAME_MAIN_BG );
  GFL_BG_FreeBGControl( FIT_FRAME_SUB_TOP );
  GFL_BG_FreeBGControl( FIT_FRAME_SUB_CURTAIN_L );
  GFL_BG_FreeBGControl( FIT_FRAME_SUB_CURTAIN_R );
  GFL_BG_FreeBGControl( FIT_FRAME_SUB_BG );
  GFL_BMPWIN_Exit();
  GFL_BG_Exit();
  GFL_HEAP_FreeMemory( work );
}

//--------------------------------------------------------------
//  試着メイン ループ
//--------------------------------------------------------------
FITTING_RETURN  DUP_FIT_LoopFitting( FITTING_WORK *work )
{
  FITTING_RETURN ret = FIT_RET_CONTINUE;
  switch( work->state )
  {
  case DUS_FADEIN_WAIT_SND:
    PMSND_PlaySE(DUP_SE_FIRST_LIST_OPEN);
    work->state = DUS_FADEIN_WAIT;
    //  break through
  case DUS_FADEIN_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      // TODO 現在デモ機能OFF
      /*
      if( DUP_FIT_ITEMGROUP_GetItemNum(work->itemGroupField) == 0 )
      {
        //フィールドにアイテム無かったらデモ
        DUP_DEMO_DemoStart( work );
      }
      else
      {
        work->state = DUS_FITTING_MAIN;
      }
      */
      work->state = DUS_FITTING_MAIN;
    }
    //初回回転アニメのためフェードイン中でも動かす
    if( work->isSortAnime == TRUE )
    {
      DUP_FIT_UpdateSortAnime( work );
    }
    break;
    
  case DUS_FADEOUT_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      return FIT_RET_GO_END;
    }
    break;
    
  case DUS_FITTING_DEMO:
    DUP_DEMO_DemoMain(work);
    break;

  case DUS_FITTING_MAIN:
    DUP_FIT_FittingMain(work);
    break;

  case DUS_GO_CHECK:
    work->animeCnt += FIT_ANIME_SPD;
    if( work->animeCnt >= FIT_ANIME_MAX )
    {
      DUP_FIT_DrawMessage( work , DUP_MSG_02 , NULL );

      work->animeCnt = FIT_ANIME_MAX;
      work->state = DUS_CHECK_MAIN;
      work->listSwingAngle = 0;
      work->listSwingSpeed = 0;
      work->adjustMoveValue = 0;
    }
    DUP_FIT_ChangeStateAnime(work);
    
    break;

  case DUS_RETURN_FITTING:
    work->animeCnt -= FIT_ANIME_SPD;
    if( work->animeCnt < 0 )
    {
      work->animeCnt = 0;
      work->state = DUS_FITTING_MAIN;
    }
    DUP_FIT_ChangeStateAnime(work);
    
    break;

  case DUS_CHECK_MAIN:
    if( DUP_CHECK_CheckMain( work ) == FIT_RET_GO_END )
    {
      work->state = DUS_WAIT_END_EFFECT;
      work->endEffCnt = 0;
      GFL_CLACT_WK_SetDrawEnable( work->clwkEffectDown, TRUE );
    }
    break;

  case DUS_WAIT_END_EFFECT:
    work->endEffCnt++;
    {
      GFL_CLACTPOS cellPos;
      cellPos.x = BUTTON_ASSEPT_POS_X;
      cellPos.y = BUTTON_ASSEPT_POS_Y+work->endEffCnt*2;
      GFL_CLACT_WK_SetPos( work->buttonCell[0] , &cellPos , CLSYS_DEFREND_MAIN );

      cellPos.x = BUTTON_RETURN_POS_X;
      cellPos.y = BUTTON_RETURN_POS_Y+work->endEffCnt*2;
      GFL_CLACT_WK_SetPos( work->buttonCell[1] , &cellPos , CLSYS_DEFREND_MAIN );
    }

    if( work->endEffCnt > EFFECT_END_EFFECT_CNT )
    {
      if( work->initWork->commWork == NULL )
      {
        WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                        WIPE_FADE_WHITE , 18 , WIPE_DEF_SYNC , work->heapId );
        work->state = DUS_FADEOUT_WAIT;
        work->isOpenCurtain = TRUE;
      }
      else
      {
        DUP_FIT_DrawMessage( work , DUP_MSG_03 , NULL );
        MUS_COMM_SendTimingCommand( work->initWork->commWork , MUS_COMM_TIMMING_WAIT_FITTING );
        work->state = DUS_WAIT_COMM_PLAYER;
      }
      
    }
    break;
  case DUS_WAIT_COMM_PLAYER:
    if( MUS_COMM_CheckTimingCommand( work->initWork->commWork , MUS_COMM_TIMMING_WAIT_FITTING ) == TRUE )
    {
        work->state = DUS_WAIT_COMM_STRM;
    }
    break;
  case DUS_WAIT_COMM_STRM:
    if( MUS_COMM_CheckFinishSendStrm( work->initWork->commWork ) == TRUE )
    {
        WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                        WIPE_FADE_WHITE , 18 , WIPE_DEF_SYNC , work->heapId );
        work->state = DUS_FADEOUT_WAIT;
        work->isOpenCurtain = TRUE;
    }
    break;
  }

  DUP_EFFECT_UpdateCell( work );
  DUP_FIT_UpdateItemAnime( work );
  MUS_POKE_DRAW_UpdateSystem( work->drawSys ); 
  MUS_ITEM_DRAW_UpdateSystem( work->itemDrawSys ); 
  
  INFOWIN_Update();

  //OBJの更新
  GFL_CLACT_SYS_Main();

  //3D描画  
  GFL_G3D_DRAW_Start();
  GFL_G3D_DRAW_SetLookAt();
  {
    MUS_POKE_DRAW_DrawSystem( work->drawSys ); 
    GFL_BBD_Draw( work->bbdSys , work->camera , NULL );
  }
  GFL_G3D_DRAW_End();


  //スクロール系
  work->bgScrollCnt++;
  if( work->bgScrollCnt > 1 )
  {
    work->bgScrollCnt = 0;
    GFL_BG_SetScrollReq( FIT_FRAME_MAIN_BG , GFL_BG_SCROLL_X_INC , 1 );
    GFL_BG_SetScrollReq( FIT_FRAME_MAIN_BG , GFL_BG_SCROLL_Y_DEC , 1 );
  }
  
  //カーテン開閉処理
  if( work->isOpenCurtain == FALSE )
  {
    if( work->curtainScrollCnt < DUP_CURTAIN_SCROLL_MAX )
    {
      work->curtainScrollCnt += DUP_CURTAIN_SCROLL_VALUE;
      GFL_BG_SetScrollReq( FIT_FRAME_SUB_CURTAIN_L , GFL_BG_SCROLL_X_SET , 128 - work->curtainScrollCnt );
      GFL_BG_SetScrollReq( FIT_FRAME_SUB_CURTAIN_R , GFL_BG_SCROLL_X_SET , -128 + work->curtainScrollCnt );
    }
    else if( work->isDispMsg == FALSE )
    {
      work->isDispMsg = TRUE;
      GFL_BG_ClearScreenCode( FIT_FRAME_SUB_MSG , 0 );
      work->msgWin = GFL_BMPWIN_Create( FIT_FRAME_SUB_MSG , 
                        DUP_MSGWIN_LEFT , DUP_MSGWIN_TOP ,
                        DUP_MSGWIN_WIDTH , DUP_MSGWIN_HEIGHT ,
                        FIT_PLT_BG_SUB_FONT , GFL_BMP_CHRAREA_GET_B );
      {
        WORDSET *wordSet = WORDSET_Create( work->heapId );
        WORDSET_RegisterPokeNickName( wordSet , 0 , work->initWork->musPoke->pokePara );
        DUP_FIT_DrawMessage( work , DUP_MSG_01 , wordSet );
        WORDSET_Delete( wordSet );
      }

      TalkWinFrame_Write( work->msgWin , WINDOW_TRANS_ON_V , 
                          FIT_MSGWIN_CHAR_TOP , FIT_PLT_BG_SUB_MSGWIN );
    }
  }
  else
  {
    if( work->isDispMsg == TRUE )
    {
      work->isDispMsg = FALSE;
      GFL_BG_SetPriority( FIT_FRAME_SUB_MSG , 3 );
      GFL_BG_SetPriority( FIT_FRAME_SUB_TOP , 0 );
      GFL_BG_SetPriority( GFL_BG_FRAME1_S , 1 );
      GFL_BG_SetPriority( GFL_BG_FRAME2_S , 2 );
      
      GFL_ARC_UTIL_TransVramScreen( ARCID_DRESSUP_GRA , NARC_dressup_gra_test_bg_u_NSCR , 
                  FIT_FRAME_SUB_BG ,  0 , 0, FALSE , work->heapId );
      GFL_BG_LoadScreenV_Req( FIT_FRAME_SUB_BG );
      
      GFL_BMPWIN_Delete( work->msgWin );

    }
    else
    if( work->curtainScrollCnt > 0 )
    {
      work->curtainScrollCnt -= DUP_CURTAIN_SCROLL_VALUE;
      GFL_BG_SetScrollReq( FIT_FRAME_SUB_CURTAIN_L , GFL_BG_SCROLL_X_SET , 128 - work->curtainScrollCnt );
      GFL_BG_SetScrollReq( FIT_FRAME_SUB_CURTAIN_R , GFL_BG_SCROLL_X_SET , -128 + work->curtainScrollCnt );
    }
  }

  if( work->isUpdateMsg == TRUE &&
      PRINTSYS_QUE_IsFinished( work->printQue ) == TRUE )
  {
    work->isUpdateMsg = FALSE;
    GFL_BMPWIN_MakeTransWindow_VBlank(work->msgWin);
    GFL_BG_SetPriority( FIT_FRAME_SUB_TOP , 1 );
    GFL_BG_SetPriority( GFL_BG_FRAME1_S , 2 );
    GFL_BG_SetPriority( GFL_BG_FRAME2_S , 3 );
    GFL_BG_SetPriority( FIT_FRAME_SUB_MSG , 0 );
  }

  //メッセージ
  PRINTSYS_QUE_Main( work->printQue );
  
  //SE用
  if( work->listSeWaitCnt > 0 )
  {
    work->listSeWaitCnt--;
  }
  
#if DEB_ARI
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_SELECT &&
    GFL_UI_KEY_GetCont() & PAD_BUTTON_START )
  {
    return FIT_RET_GO_END;
  }
#endif //DEB_ARI
  return ret;
}

static void DUP_FIT_VBlankFunc(GFL_TCB *tcb, void *wk )
{
  FITTING_WORK *work = wk;
  FIT_ITEM_WORK *item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupList );
  BOOL isChangeItem = FALSE;
  while( item != NULL )
  {
    MUS_ITEM_DRAW_WORK *itemDrawWork = DUP_FIT_ITEM_GetItemDrawWork( item );
    const ITEM_STATE *itemState = DUP_FIT_ITEM_GetItemState( item );
    ITEM_STATE *newItemState = DUP_FIT_ITEM_GetNewItemState( item );
    
    if( itemState->itemId != newItemState->itemId )
    {
      DUP_FIT_ITEM_ChengeGraphic( item, work->itemDrawSys , newItemState );
      isChangeItem = TRUE;
    }
    
    item = DUP_FIT_ITEM_GetNextItem(item);
  }
  if( isChangeItem == TRUE )
  {
    DUP_FIT_CheckItemListPallet( work );
  }

  if( work->holdItem != NULL )
  {
    const ITEM_STATE *itemState = DUP_FIT_ITEM_GetItemState( work->holdItem );
    if( work->shadowItemId != itemState->itemId )
    {
      MUS_ITEM_DATA_SYS* itemDataSys = MUS_ITEM_DRAW_GetItemDataSys( work->itemDrawSys );
      u16 newResId;
    	MUS_ITEM_DATA_WORK *itemData;
      NNSGfdPlttKey	texPlttKey;
      NNSGfdTexKey	texDataKey;
      GFL_BBD_TEXSIZ texSize;
      u16 sizeX,sizeY;
      work->shadowItemId = itemState->itemId;
      
      //MUS_ITEM_DRAW_ChengeGraphicResIdx( work->itemDrawSys , work->shadowItem , itemState->itemId , itemState->itemResIdx );
      
      GFL_BBD_GetResourcePlttKey( work->bbdSys , work->shadowItemResIdx , &texPlttKey );
      GFL_BBD_GetResourceTexKey( work->bbdSys , itemState->itemResIdx , &texDataKey );
      GFL_BBD_SetResourceTexKey( work->bbdSys , work->shadowItemResIdx , texDataKey );
      itemData = MUS_ITEM_DATA_GetMusItemData( itemDataSys , work->shadowItemId );
      texSize = MUS_ITEM_DATA_GetTexType( itemData );
      GFL_BBD_GetTexSize( texSize , &sizeX , &sizeY );
      newResId = GFL_BBD_AddResourceKey( work->bbdSys , texDataKey , texPlttKey ,
                           GFL_BBD_TEXFMT_PAL16 , texSize , sizeX , sizeY );

      MUS_ITEM_DRAW_ChengeGraphicResIdx( work->itemDrawSys , work->shadowItem , work->shadowItemId , newResId );
      //MUS_ITEM_DRAW_SetShadowPallet( work->itemDrawSys , work->shadowItem , work->shadowItemRes );
      GFL_BBD_RemoveResourceVram( work->bbdSys , work->shadowItemResIdx );
      work->shadowItemResIdx = newResId;
    }
  }
  GFL_CLACT_SYS_VBlankFunc();
  MUS_ITEM_DRAW_UpdateSystem_VBlank(work->itemDrawSys);
}



//--------------------------------------------------------------
//  描画系初期化
//--------------------------------------------------------------
static void DUP_FIT_SetupGraphic( FITTING_WORK *work )
{
  GFL_DISP_GX_InitVisibleControl();
  GFL_DISP_GXS_InitVisibleControl();
  GX_SetVisiblePlane( 0 );
  GXS_SetVisiblePlane( 0 );
  WIPE_SetBrightness(WIPE_DISP_MAIN,WIPE_FADE_BLACK);
  WIPE_SetBrightness(WIPE_DISP_SUB,WIPE_FADE_BLACK);
  WIPE_ResetWndMask(WIPE_DISP_MAIN);
  WIPE_ResetWndMask(WIPE_DISP_SUB);
  G2_BlendNone();
  G2S_BlendNone();
  GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);
  GFL_DISP_SetBank( &vramBank );
  

  { //3D系の設定
    static const VecFx32 cam_pos = MUSICAL_CAMERA_POS;
    static const VecFx32 cam_target = MUSICAL_CAMERA_TRG;
    static const VecFx32 cam_up = MUSICAL_CAMERA_UP;
    //エッジマーキングカラー
    static  const GXRgb stage_edge_color_table[8]=
      { GX_RGB( 0, 0, 0 ), GX_RGB( 0, 0, 0 ), 0, 0, 0, 0, 0, 0 };
    GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT80K,
            0, work->heapId, NULL );
    GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_W );
#if 0 //透視射影カメラ
    work->camera =  GFL_G3D_CAMERA_Create( GFL_G3D_PRJPERS, 
                       FX32_SIN13,
                       FX32_COS13,
                       FX_F32_TO_FX32( 1.33f ),
                       NULL,
                       FX32_ONE,
                       FX32_ONE * 180,
                       NULL,
                       &cam_pos,
                       &cam_up,
                       &cam_target,
                       work->heapId );
#else
    //正射影カメラ
    work->camera =  GFL_G3D_CAMERA_Create( GFL_G3D_PRJORTH, 
                       FX32_ONE*12.0f,
                       0,
                       0,
                       FX32_ONE*16.0f,
                       MUSICAL_CAMERA_NEAR,
                       MUSICAL_CAMERA_FAR,
                       NULL,
                       &cam_pos,
                       &cam_up,
                       &cam_target,
                       work->heapId );
#endif
    
    GFL_G3D_CAMERA_Switching( work->camera );
    //エッジマーキングカラーセット
    G3X_SetEdgeColorTable( &stage_edge_color_table[0] );
    G3X_EdgeMarking( TRUE );
    
    GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO , GX_BUFFERMODE_Z );
  }
  
  GFL_BG_Init( work->heapId );
  GFL_BMPWIN_Init( work->heapId );
  
  //Vram割り当ての設定
  {
    static const GFL_BG_SYS_HEADER sys_data = {
        GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_3D,
    };
    
    // BG1 MAIN (Infoバー
    static const GFL_BG_BGCNT_HEADER header_main1 = {
      0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000,0x1000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG2 MAIN (蓋・鏡
    static const GFL_BG_BGCNT_HEADER header_main2 = {
      0, 0, 0x1000, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x08000,0,
      GX_BG_EXTPLTT_01, 2, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG3 MAIN (背景
    static const GFL_BG_BGCNT_HEADER header_main3 = {
      0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x08000,0x8000,
      GX_BG_EXTPLTT_01, 3, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };

    // BG0 SUB (トップのカーテン
    static const GFL_BG_BGCNT_HEADER header_sub0 = {
      0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x08000,0x0000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG1 SUB (カーテン右
    static const GFL_BG_BGCNT_HEADER header_sub1 = {
      0, 0, 0x1000, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x08000,0x0000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG2 SUB (カーテン左
    static const GFL_BG_BGCNT_HEADER header_sub2 = {
      0, 0, 0x1000, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x5000, GX_BG_CHARBASE_0x08000,0x0000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG3 SUB (背景
    static const GFL_BG_BGCNT_HEADER header_sub3 = {
      0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x10000,0x8000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };

    GFL_BG_SetBGMode( &sys_data );
    GFL_BG_SetBGControl3D( 1 );
    DUP_FIT_SetupBgFunc( &header_main1, FIT_FRAME_MAIN_INFO);
    DUP_FIT_SetupBgFunc( &header_main2, FIT_FRAME_MAIN_CASE);
    DUP_FIT_SetupBgFunc( &header_main3, FIT_FRAME_MAIN_BG);

    DUP_FIT_SetupBgFunc( &header_sub3 , FIT_FRAME_SUB_BG );
    DUP_FIT_SetupBgFunc( &header_sub0 , FIT_FRAME_SUB_TOP );
    DUP_FIT_SetupBgFunc( &header_sub1 , FIT_FRAME_SUB_CURTAIN_R );
    DUP_FIT_SetupBgFunc( &header_sub2 , FIT_FRAME_SUB_CURTAIN_L );
    
    GFL_BG_SetVisible( FIT_FRAME_MAIN_3D , TRUE );

    G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_NONE , 
                      GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3 ,
                      4 , 16 );
  }

  { //OBJの初期化
    GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
    cellSysInitData.oamst_main = 0x10;  //デバッグメータの分
    cellSysInitData.oamnum_main = 128-0x10;
    GFL_CLACT_SYS_Create( &cellSysInitData , &vramBank ,work->heapId );
    
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );
    GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );
    work->cellUnit  = GFL_CLACT_UNIT_Create( 24 , 0, work->heapId );
    GFL_CLACT_UNIT_SetDefaultRend( work->cellUnit );
  }

  {
    GFL_BBD_SETUP bbdSetup = {
      512,128,
      {FX32_ONE,FX32_ONE,0},
      GX_RGB(0,0,0),
      GX_RGB(0,0,0),
      GX_RGB(0,0,0),
      GX_RGB(0,0,0),
      MUS_ITEM_POLIGON_ID , 
      GFL_BBD_ORIGIN_CENTER
    };
    //ビルボードシステム構築
    work->bbdSys = GFL_BBD_CreateSys( &bbdSetup , work->heapId );
  }
}

//--------------------------------------------------------------
//  BGの初期化
//--------------------------------------------------------------
static void DUP_FIT_SetupBgObj( FITTING_WORK *work )
{
  
  ARCHANDLE *arcHandle = GFL_ARC_OpenDataHandle( ARCID_DRESSUP_GRA , work->heapId );

  //下画面
  GFL_ARCHDL_UTIL_TransVramPalette( arcHandle , NARC_dressup_gra_test_bg_d_NCLR , 
                    PALTYPE_MAIN_BG , 0 , 0 , work->heapId );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_dressup_gra_bg_mirror_NCGR ,
                    FIT_FRAME_MAIN_BG , 0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_dressup_gra_test_bg_d_NSCR , 
                    FIT_FRAME_MAIN_BG ,  0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_dressup_gra_bg_mirror_NSCR , 
                    FIT_FRAME_MAIN_MIRROR ,  0 , 0, FALSE , work->heapId );

  //上画面
  GFL_ARCHDL_UTIL_TransVramPalette( arcHandle , NARC_dressup_gra_test_bg_NCLR , 
                    PALTYPE_SUB_BG , 0 , 0 , work->heapId );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_dressup_gra_test_bg_u3_NCGR ,
                    FIT_FRAME_SUB_TOP , 0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_dressup_gra_test_bg_u_NCGR ,
                    FIT_FRAME_SUB_BG , 0 , 0, FALSE , work->heapId );

  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_dressup_gra_test_bg_u3_NSCR , 
                    FIT_FRAME_SUB_TOP ,  0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_dressup_gra_test_bg_u2_2_NSCR , 
                    FIT_FRAME_SUB_CURTAIN_L ,  0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_dressup_gra_test_bg_u2_NSCR , 
                    FIT_FRAME_SUB_CURTAIN_R ,  0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_dressup_gra_test_bg_u_NSCR , 
                    FIT_FRAME_SUB_BG ,  0 , 0, FALSE , work->heapId );

  GFL_BG_SetScrollReq( FIT_FRAME_SUB_CURTAIN_L , GFL_BG_SCROLL_X_SET ,  128 );
  GFL_BG_SetScrollReq( FIT_FRAME_SUB_CURTAIN_R , GFL_BG_SCROLL_X_SET , -128 );

  //Obj用Res
  work->objResIdx[DOR_BUTTON_PLT] = GFL_CLGRP_PLTT_Register( arcHandle , NARC_dressup_gra_obj_main_NCLR , CLSYS_DRAW_MAIN , FIT_PLT_OBJ_MAIN_BUTTON*32 , work->heapId  );
  work->objResIdx[DOR_BUTTON_NCG] = GFL_CLGRP_CGR_Register( arcHandle , NARC_dressup_gra_obj_main_NCGR , FALSE , CLSYS_DRAW_MAIN , work->heapId  );
  work->objResIdx[DOR_BUTTON_ANM] = GFL_CLGRP_CELLANIM_Register( arcHandle , NARC_dressup_gra_obj_main_NCER , NARC_dressup_gra_obj_main_NANR, work->heapId  );
  work->objResIdx[DOR_EFFECT_UP_PLT] = GFL_CLGRP_PLTT_Register( arcHandle , NARC_dressup_gra_anime_ue_NCLR , CLSYS_DRAW_SUB , 0 , work->heapId  );
  work->objResIdx[DOR_EFFECT_UP_NCG] = GFL_CLGRP_CGR_Register( arcHandle , NARC_dressup_gra_anime_ue_NCGR , FALSE , CLSYS_DRAW_SUB , work->heapId  );
  work->objResIdx[DOR_EFFECT_UP_ANM] = GFL_CLGRP_CELLANIM_Register( arcHandle , NARC_dressup_gra_anime_ue_NCER , NARC_dressup_gra_anime_ue_NANR, work->heapId  );
  work->objResIdx[DOR_EFFECT_DOWN_PLT] = GFL_CLGRP_PLTT_Register( arcHandle , NARC_dressup_gra_anime_sita_NCLR , CLSYS_DRAW_MAIN , FIT_PLT_OBJ_MAIN_EFFECT*32 , work->heapId  );
  work->objResIdx[DOR_EFFECT_DOWN_NCG] = GFL_CLGRP_CGR_Register( arcHandle , NARC_dressup_gra_anime_sita_NCGR , FALSE , CLSYS_DRAW_MAIN , work->heapId  );
  work->objResIdx[DOR_EFFECT_DOWN_ANM] = GFL_CLGRP_CELLANIM_Register( arcHandle , NARC_dressup_gra_anime_sita_NCER , NARC_dressup_gra_anime_sita_NANR, work->heapId  );
  
  {
  //セルの生成
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = BUTTON_ASSEPT_POS_X;
    cellInitData.pos_y = BUTTON_ASSEPT_POS_Y+BUTTON_SIZE_Y;
    cellInitData.anmseq = 0;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 2;
    work->buttonCell[0] = GFL_CLACT_WK_Create( work->cellUnit ,
              work->objResIdx[DOR_BUTTON_NCG],
              work->objResIdx[DOR_BUTTON_PLT],
              work->objResIdx[DOR_BUTTON_ANM],
              &cellInitData ,CLSYS_DEFREND_MAIN , work->heapId );

    cellInitData.pos_x = BUTTON_RETURN_POS_X;
    cellInitData.pos_y = BUTTON_RETURN_POS_Y+BUTTON_SIZE_Y;
    cellInitData.anmseq = 1;
    work->buttonCell[1] = GFL_CLACT_WK_Create( work->cellUnit ,
              work->objResIdx[DOR_BUTTON_NCG],
              work->objResIdx[DOR_BUTTON_PLT],
              work->objResIdx[DOR_BUTTON_ANM],
              &cellInitData ,CLSYS_DEFREND_MAIN , work->heapId );

    cellInitData.pos_x = 128;
    cellInitData.pos_y = 96;
    cellInitData.anmseq = 2;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;
    work->demoPen = GFL_CLACT_WK_Create( work->cellUnit ,
              work->objResIdx[DOR_BUTTON_NCG],
              work->objResIdx[DOR_BUTTON_PLT],
              work->objResIdx[DOR_BUTTON_ANM],
              &cellInitData ,CLSYS_DEFREND_MAIN , work->heapId );

    GFL_CLACT_WK_SetDrawEnable( work->buttonCell[0], TRUE );
    GFL_CLACT_WK_SetDrawEnable( work->buttonCell[1], TRUE );
    GFL_CLACT_WK_SetDrawEnable( work->demoPen, FALSE );
  }

  GFL_ARC_CloseDataHandle(arcHandle);

  GFL_BG_LoadScreenReq(FIT_FRAME_MAIN_BG);
  GFL_BG_LoadScreenReq(FIT_FRAME_MAIN_MIRROR);
  GFL_BG_LoadScreenReq(FIT_FRAME_SUB_BG);


}
//--------------------------------------------------------------------------
//  Bg初期化 機能部
//--------------------------------------------------------------------------
static void DUP_FIT_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane )
{
  GFL_BG_SetBGControl( bgPlane, bgCont, GFL_BG_MODE_TEXT );
  GFL_BG_SetVisible( bgPlane, VISIBLE_ON );
  GFL_BG_ClearFrame( bgPlane );
  GFL_BG_LoadScreenReq( bgPlane );
}

//--------------------------------------------------------------
//  表示ポケモンの初期化
//--------------------------------------------------------------
static void DUP_FIT_SetupPokemon( FITTING_WORK *work )
{
  VecFx32 pos = {FIT_POKE_POS_X_FX,FIT_POKE_POS_Y_FX,FIT_POKE_POS_Z_FX};
//  VecFx32 pos = {FX32_ONE*10,FX32_ONE*10,FX32_ONE*-7};  //位置は適当
  work->drawSys = MUS_POKE_DRAW_InitSystem( work->heapId );
  MUS_POKE_DRAW_SetTexAddres( work->drawSys , 0x50000 );
  MUS_POKE_DRAW_SetPltAddres( work->drawSys , 0x4000-0x20 );
  work->drawWork = MUS_POKE_DRAW_Add( work->drawSys , work->initWork->musPoke , FALSE );
  MUS_POKE_DRAW_SetPosition( work->drawWork , &pos);
  
}

//--------------------------------------------------------------
//アイテムの初期化
//--------------------------------------------------------------
static void DUP_FIT_SetupItem( FITTING_WORK *work )
{
  int i;
  MUS_ITEM_DATA_SYS* itemDataSys;

  //所持アイテムのチェック
  work->totalItemNum = 0;
  for( i=0;i<MUSICAL_ITEM_MAX;i++ )
  {
    //  FIXME:正しい所持アイテムチェック
    if( i<100 )
    {
      work->itemStateBase[work->totalItemNum].itemId = i;
      work->itemStateBase[work->totalItemNum].isOutList = FALSE;
      /*
      if( i<100 && GFL_STD_MtRand0(4) == 0 )
      {
        work->itemStateBase[work->totalItemNum].isNew = TRUE;
      }
      else
      {
        work->itemStateBase[work->totalItemNum].isNew = FALSE;
      }
      */
      work->itemStateBase[work->totalItemNum].isNew = FALSE;
#ifdef PM_DEBUG
      if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
      {
        work->itemStateBase[work->totalItemNum].isNew = TRUE;
      }
#endif
      work->totalItemNum++;
    }
  }
  //残りに空データをつめる
  i=0;
  while( work->totalItemNum+i < MUSICAL_ITEM_MAX )
  {
    work->itemStateBase[work->totalItemNum+i].itemId = MUSICAL_ITEM_INVALID;
    work->itemStateBase[work->totalItemNum+i].isOutList = FALSE;
    work->itemStateBase[work->totalItemNum].isNew = TRUE;
    i++;
  }
  
  //グラフィック系初期化
  work->itemDrawSys = MUS_ITEM_DRAW_InitSystem( work->bbdSys , ITEM_DISP_NUM , work->heapId );
  itemDataSys = MUS_ITEM_DRAW_GetItemDataSys( work->itemDrawSys );
  for( i=0;i<work->totalItemNum;i++ )
  {
    GFL_G3D_RES *itemRes;
    NNSGfdTexKey	texDataKey;
    NNSGfdPlttKey	texPlttKey,shadowPlttKey;
  	MUS_ITEM_DATA_WORK *itemData;
    GFL_BBD_TEXSIZ texSize;
    u16 sizeX,sizeY;

    itemRes = MUS_ITEM_DRAW_LoadResource( work->itemStateBase[i].itemId );
    GFL_G3D_TransVramTexture( itemRes );
    texDataKey = GFL_G3D_GetTextureDataVramKey( itemRes );
    texPlttKey = GFL_G3D_GetTexturePlttVramKey( itemRes );
    shadowPlttKey = NNS_GfdAllocPlttVram( NNS_GfdGetPlttKeySize( texPlttKey ) , FALSE , 0 );
    itemData = MUS_ITEM_DATA_GetMusItemData( itemDataSys , work->itemStateBase[i].itemId );
    texSize = MUS_ITEM_DATA_GetTexType( itemData );
    GFL_BBD_GetTexSize( texSize , &sizeX , &sizeY );

    work->itemStateBase[i].itemResIdx = GFL_BBD_AddResourceKey( work->bbdSys , texDataKey , texPlttKey ,
                                        GFL_BBD_TEXFMT_PAL16 , texSize , sizeX , sizeY );
    {
      //パレットデータ改変
      GXRgb *pltData = (void*)GFL_G3D_GetAdrsTexturePltt( itemRes );
      const u32 pltSize = NNS_GfdGetPlttKeySize( texPlttKey );
      u8 col;
      for( col=0;col<pltSize/2;col++ )
      {
        const u8 r = (pltData[col] & GX_RGB_R_MASK)>>GX_RGB_R_SHIFT;
        const u8 g = (pltData[col] & GX_RGB_G_MASK)>>GX_RGB_G_SHIFT;
        const u8 b = (pltData[col] & GX_RGB_B_MASK)>>GX_RGB_B_SHIFT;
        //ARI_TPrintf("[%04x:%d:%d:%d]\n",pltData[i],r,g,b);
        pltData[col] = GX_RGB( r/2 , g/2 , b/2 );
      }
    }
    {
      //パレットデータ転送
      NNSG3dResFileHeader*	header = GFL_G3D_GetResourceFileHeader( itemRes );
      NNSG3dResTex* resTex = GFL_G3D_GetResTex( itemRes );
      NNS_G3dPlttSetPlttKey( resTex, shadowPlttKey );
      DC_FlushRange( header, header->fileSize );
      NNS_G3dPlttLoad( resTex, TRUE );
    }
    
    work->itemStateBase[i].itemShadowResIdx = GFL_BBD_AddResourceKey( work->bbdSys , texDataKey , shadowPlttKey ,
                                        GFL_BBD_TEXFMT_PAL16 , texSize , sizeX , sizeY );
    
    MUS_ITEM_DRAW_DeleteResource( itemRes );
    /*
    OS_TPrintf("[%d][%d][%d][%x][%x][%x]\n",
      work->itemStateBase[i].itemId,
      work->itemStateBase[i].itemResIdx,
      work->itemStateBase[i].itemShadowResIdx,
      NNS_GfdGetTexKeyAddr( texDataKey ),
      NNS_GfdGetPlttKeyAddr( texPlttKey ),
      NNS_GfdGetPlttKeyAddr( shadowPlttKey ) );
    //*/
  }
  //デフォルトでソート
  DUP_FIT_SortItemIdx( work , MUS_POKE_EQUIP_USER_MAX , 0 );
  
  work->itemGroupList = DUP_FIT_ITEMGROUP_CreateGroup( work->heapId , ITEM_LIST_NUM );
  work->itemGroupField = DUP_FIT_ITEMGROUP_CreateGroup( work->heapId , ITEM_FIELD_NUM );
  work->itemGroupEquip = DUP_FIT_ITEMGROUP_CreateGroup( work->heapId , MUS_POKE_EQUIP_MAX );
  work->itemGroupAnime = DUP_FIT_ITEMGROUP_CreateGroup( work->heapId , 4 );
  for( i=0;i<ITEM_LIST_NUM;i++ )
  {
    FIT_ITEM_WORK* item;
    VecFx32 pos = {0,0,0};
    
    item = DUP_FIT_ITEM_CreateItem( work->heapId , work->itemDrawSys ,
                                    &work->itemStateBase[i], &pos );
    DUP_FIT_ITEMGROUP_AddItem( work->itemGroupList,item );
    MUS_ITEM_DRAW_SetUseOffset( work->itemDrawSys , DUP_FIT_ITEM_GetItemDrawWork( item ) , FALSE );
  }
  //影用
  {
    VecFx32 pos = {0,0,0};
  	MUS_ITEM_DATA_WORK *itemData;
    GFL_G3D_RES *itemRes;
    GFL_BBD_TEXSIZ texSize;
    u16 sizeX,sizeY;
    NNSGfdPlttKey	texPlttKey;
    NNSGfdTexKey	texDataKey;

    work->shadowItemId = 0;
    itemData = MUS_ITEM_DATA_GetMusItemData( itemDataSys , work->shadowItemId );
    texSize = MUS_ITEM_DATA_GetTexType( itemData );
    GFL_BBD_GetTexSize( texSize , &sizeX , &sizeY );

    itemRes = MUS_ITEM_DRAW_LoadResource( work->shadowItemId );
    GFL_G3D_TransVramTexture( itemRes );

    texDataKey = GFL_G3D_GetTextureDataVramKey( itemRes );
    texPlttKey = GFL_G3D_GetTexturePlttVramKey( itemRes );

    work->shadowItemResIdx = GFL_BBD_AddResourceKey( work->bbdSys , texDataKey , texPlttKey ,
                                          GFL_BBD_TEXFMT_PAL16 , texSize , sizeX , sizeY );
    work->shadowItem = MUS_ITEM_DRAW_AddResIdx( work->itemDrawSys , work->shadowItemId , work->shadowItemResIdx , &pos );
    MUS_ITEM_DRAW_SetDrawEnable( work->itemDrawSys , work->shadowItem , FALSE );
    MUS_ITEM_DRAW_SetSize( work->itemDrawSys , work->shadowItem , FX16_ONE , FX16_ONE );
    MUS_ITEM_DRAW_SetShadowPallet( work->itemDrawSys , work->shadowItem , itemRes );
    
    MUS_ITEM_DRAW_DeleteResource( itemRes );
/*
    OS_TPrintf("[%d][%x][%x]\n",
      work->shadowItemResIdx,
      NNS_GfdGetTexKeyAddr( GFL_G3D_GetTextureDataVramKey( work->shadowItemRes ) ),
      NNS_GfdGetPlttKeyAddr( GFL_G3D_GetTexturePlttVramKey( work->shadowItemRes ) ) );
*/
  }

  //Newの表示用
  {
    u8 i;
    work->newPicResIdx = GFL_BBD_AddResourceArc( work->bbdSys , ARCID_DRESSUP_GRA , 
                      NARC_dressup_gra_new_str_nsbtx , GFL_BBD_TEXFMT_PAL4 ,
                      GFL_BBD_TEXSIZ_32x32 , 32 , 32 );
    for( i=0;i<ITEM_LIST_NUM;i++ )
    {
      const VecFx32 pos ={0,0,0};
      const BOOL flg = FALSE;
      work->newPicBbdIdx[i] = GFL_BBD_AddObject( work->bbdSys , work->newPicResIdx ,
                                  FX32_ONE, FX32_ONE , &pos , 31 , GFL_BBD_LIGHT_NONE );
    	GFL_BBD_SetObjectDrawEnable( work->bbdSys , work->newPicBbdIdx[i] , &flg );
    }

    work->newPicBlinkCnt = 0;
  }
  
  //セーブからアイテムを散らかす
  {
    u8 i;
    MUSICAL_EQUIP_SAVE *mus_bef_save = MUSICAL_SAVE_GetBefEquipData( work->initWork->mus_save );
    
    for( i=0;i<MUSICAL_ITEM_EQUIP_MAX;i++ )
    {
      if( mus_bef_save->equipData[i].pos != MUS_POKE_EQU_INVALID )
      {
        DUP_FIT_SetupStartItem( work , mus_bef_save->equipData[i].itemNo );
      }
    }
  }

  
  //半回転の位置からスタート
  work->listTotalMove = (LIST_ONE_ANGLE*work->totalItemNum)-0x8000; 
  DUP_FIT_CalcItemListAngle( work , 0 , 0 , LIST_SIZE_X , LIST_SIZE_Y );

}

static void DUP_FIT_SetupStartItem( FITTING_WORK *work , const u16 itemId )
{
  u8 i;
  ITEM_STATE *itemState = NULL;
  
  for( i=0;i<work->totalItemNum;i++ )
  {
    if( work->itemStateBase[i].itemId == itemId )
    {
      itemState = &work->itemStateBase[i];
      break;
    }
  }
  if( itemState == NULL )
  {
    //アイテムが捨てられた・・・？
    ARI_TPrintf("ItemId[%d] is not found...\n",itemId);
    return;
  }
  
  while( TRUE )
  {
    GFL_POINT pos;
    GFL_POINT subPos;
    BOOL hitOval;
    
    pos.x = 8  +GFUser_GetPublicRand0(256-( 8+ 8));
    pos.y = 32 +GFUser_GetPublicRand0(192-(32+32));
    
    subPos.x = pos.x - LIST_CENTER_X;
    subPos.y = pos.y - LIST_CENTER_Y;
    
    hitOval = DUP_FIT_CheckPointInOval( subPos.x,subPos.y,LIST_TPHIT_MAX_Y,LIST_TPHIT_RATIO_MAX );
    if( hitOval == FALSE )
    {
      FIT_ITEM_WORK* item;
      VecFx32 vecPos;
      MUS_ITEM_DRAW_WORK *itemDrawWork;

      vecPos.x = FX32_CONST(pos.x);
      vecPos.y = FX32_CONST(pos.y);
      vecPos.z = FIELD_ITEM_DEPTH - FX32_CONST(0.1f)*DUP_FIT_ITEMGROUP_GetItemNum(work->itemGroupField);
      
      item = DUP_FIT_ITEM_CreateItem( work->heapId , work->itemDrawSys , itemState , &vecPos );
      DUP_FIT_ITEMGROUP_AddItem( work->itemGroupField,item );
      DUP_FIT_ITEM_SetPosition( item , &pos );

      itemDrawWork = DUP_FIT_ITEM_GetItemDrawWork( item );
      MUS_ITEM_DRAW_SetSize( work->itemDrawSys , itemDrawWork ,FX16_ONE,FX16_ONE );

      //アイテムを取った状態に
      itemState->isOutList = TRUE;
      break;
    }
  }
}


static void DUP_FIT_TermItem( FITTING_WORK *work )
{
  int i;
  FIT_ITEM_WORK *nextItem;
  FIT_ITEM_WORK *item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupList );
  while( item != NULL )
  {
    nextItem = DUP_FIT_ITEM_GetNextItem(item);
    DUP_FIT_ITEM_DeleteItem( item , work->itemDrawSys );
    item = nextItem;
  }

  item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupField );
  while( item != NULL )
  {
    nextItem = DUP_FIT_ITEM_GetNextItem(item);
    DUP_FIT_ITEM_DeleteItem( item , work->itemDrawSys );
    item = nextItem;
  }

  item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupEquip );
  while( item != NULL )
  {
    nextItem = DUP_FIT_ITEM_GetNextItem(item);
    DUP_FIT_ITEM_DeleteItem( item , work->itemDrawSys );
    item = nextItem;
  }

  item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupAnime );
  while( item != NULL )
  {
    nextItem = DUP_FIT_ITEM_GetNextItem(item);
    DUP_FIT_ITEM_DeleteItem( item , work->itemDrawSys );
    item = nextItem;
  }
  
  //New用
  for( i=0;i<ITEM_LIST_NUM;i++ )
  {
    GFL_BBD_RemoveObject( work->bbdSys , work->newPicBbdIdx[i] );
  }
  GFL_BBD_RemoveResource( work->bbdSys , work->newPicResIdx );
  GF_ASSERT(GFL_HEAP_CheckHeapSafe( work->heapId ));

  //影用
  MUS_ITEM_DRAW_DelItem( work->itemDrawSys , work->shadowItem );
  GFL_BBD_RemoveResource( work->bbdSys , work->shadowItemResIdx );
//  MUS_ITEM_DRAW_DeleteResource( work->shadowItemRes );

  GF_ASSERT(GFL_HEAP_CheckHeapSafe( work->heapId ));

  for( i=0;i<work->totalItemNum;i++ )
  {
    GFL_BBD_RemoveResource( work->bbdSys , work->itemStateBase[i].itemResIdx );
    GFL_BBD_RemoveResource( work->bbdSys , work->itemStateBase[i].itemShadowResIdx );
//    MUS_ITEM_DRAW_DeleteResource( work->itemStateBase[i].itemRes );
  }
  
  DUP_FIT_ITEMGROUP_DeleteGroup( work->itemGroupList );
  DUP_FIT_ITEMGROUP_DeleteGroup( work->itemGroupField );
  DUP_FIT_ITEMGROUP_DeleteGroup( work->itemGroupEquip );
  DUP_FIT_ITEMGROUP_DeleteGroup( work->itemGroupAnime );
  
  MUS_ITEM_DRAW_TermSystem( work->itemDrawSys );
}

#pragma mark [> FittingFunc

static void DUP_FIT_FittingMain(  FITTING_WORK *work )
{

#if DEB_ARI
  if(GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT )
  {
    work->listAngle += 0x100;
    DUP_FIT_CalcItemListAngle( work , work->listAngle , -0x100 , LIST_SIZE_X , LIST_SIZE_Y );
  }
  if(GFL_UI_KEY_GetCont() & PAD_KEY_LEFT )
  {
    work->listAngle -= 0x100;
    DUP_FIT_CalcItemListAngle( work , work->listAngle , +0x100 , LIST_SIZE_X , LIST_SIZE_Y );
  }
  if(GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
  {
    FIT_ITEM_WORK *item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupList );
    ARI_TPrintf("---DumpResIdx---\n");
    while( item != NULL )
    {
      MUS_ITEM_DRAW_Debug_DumpResData( work->itemDrawSys,DUP_FIT_ITEM_GetItemDrawWork( item ) );

      item = DUP_FIT_ITEM_GetNextItem(item);
    }
    MUS_ITEM_DRAW_Debug_DumpResData( work->itemDrawSys,work->shadowItem );
    ARI_TPrintf("---DumpResIdx---\n");
  }
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A &&
      work->isDemo == FALSE )
  {
    //デバグ用にデフォルト位置に戻してスタート
    //半回転の位置からスタート
    work->listAngle = 0;
    work->listTotalMove = (LIST_ONE_ANGLE*work->totalItemNum)-0x8000; 
    DUP_FIT_CalcItemListAngle( work , 0 , 0 , LIST_SIZE_X , LIST_SIZE_Y );

    DUP_DEMO_DemoStart( work );
  }
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X )
  {
    work->listSwingAngle = 0x4000;
    work->listSwingSpeed = -0x200;
    DUP_FIT_CalcItemListAngle( work , work->listAngle , 0 , LIST_SIZE_X , LIST_SIZE_Y );
  }


#endif

  if( work->isSortAnime == TRUE )
  {
    DUP_FIT_UpdateSortAnime( work );
  }
  else
  {
    DUP_FIT_UpdateTpMain( work );
    //アイテム揺らし更新
    //アニメ中の揺らしはアニメの中でやってる
    DUP_FIT_UpdateSwingItem( work );
  }
  DUP_FIT_UpdateShadow( work );

  //New!の点滅更新
  work->newPicBlinkCnt++;
  if( work->newPicBlinkCnt >= ITEM_LIST_NEW_BLINK*2 )
  {
    work->newPicBlinkCnt = 0;
    DUP_FIT_CalcItemListAngle( work , work->listAngle , 0 , LIST_SIZE_X , LIST_SIZE_Y );
  }
  if( work->newPicBlinkCnt == ITEM_LIST_NEW_BLINK )
  {
    DUP_FIT_CalcItemListAngle( work , work->listAngle , 0 , LIST_SIZE_X , LIST_SIZE_Y );
  }
  
}


//--------------------------------------------------------------
//アイテムリストの位置・サイズ計算
//--------------------------------------------------------------
static void DUP_FIT_CalcItemListAngle( FITTING_WORK *work , u16 angle , s16 moveAngle , u16 sizeX , u16 sizeY )
{
  //現在複数個所から呼び出しているので処理負荷を余計に食っているかも・・・
  //処理負荷がきつかったら値を累計させて一箇所で処理するように。
  
  //円は下が手前で0度として、depthは下で0x8000 上が0x0000
  int i = 0;
  u16 itemSwingRot;
  fx32 itemSwingOfsX,itemSwingOfsY;
  FIT_ITEM_WORK *item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupList );
  
  work->listTotalMove += moveAngle;
  
  if( work->listTotalMove < 0 )
  {
    work->listTotalMove += (LIST_ONE_ANGLE*work->totalItemNum);
  }
  if( work->listTotalMove >= (LIST_ONE_ANGLE*work->totalItemNum) )
  {
    work->listTotalMove -= (LIST_ONE_ANGLE*work->totalItemNum);
  }
  
  //リストのアイテムが揺れる処理(アイテムの回転量と位置補正を求める
  {
    itemSwingRot = work->listSwingAngle;
    itemSwingOfsX = ((fx32)FX_SinIdx( 0 )*ITEM_SWING_ORIGIN_OFSSET)-((fx32)FX_SinIdx( itemSwingRot )*ITEM_SWING_ORIGIN_OFSSET);
    itemSwingOfsY = ((fx32)FX_CosIdx( 0 )-ITEM_SWING_ORIGIN_OFSSET)-((fx32)FX_CosIdx( itemSwingRot )*-ITEM_SWING_ORIGIN_OFSSET)-FX32_CONST(ITEM_SWING_ORIGIN_OFSSET);
    //OS_Printf("[%4x][%.2f][%.2f]\n",itemSwingRot,F32_CONST(itemSwingOfsX),F32_CONST(itemSwingOfsY));
  }
  
  //SE処理
  if( moveAngle > DUP_LIST_ROTATE_SE_SPEED &&
      work->listSeWaitCnt == 0 )
  {
    work->listSeWaitCnt = DUP_LIST_ROTATE_SE_WAIT_CNT;
    PMSND_PlaySE(DUP_SE_LIST_ROTATE);
  }
  
  while( item != NULL )
  {
    GFL_POINT dispPos;
    VecFx32 pos;
    u16 oneAngle = (i*0x10000/ITEM_LIST_NUM+angle)%0x10000;
    u16 depth;
    fx16 size = 0;
    MUS_ITEM_DRAW_WORK *itemDrawWork = DUP_FIT_ITEM_GetItemDrawWork( item );
    const fx32 sin = (fx32)FX_SinIdx( oneAngle );
    const fx32 cos = (fx32)FX_CosIdx( oneAngle );
    const fx32 posX = LIST_CENTER_X_FX+sin*sizeX;
    const fx32 posY = LIST_CENTER_Y_FX+cos*sizeY;
    ITEM_STATE *nowItemState = DUP_FIT_ITEM_GetItemState( item );
    ITEM_STATE *newItemState = nowItemState;
    
    //絵の読み替え処理とか
    {
      u16 newIdx;
      s32 subAngle;
      s32 itemTotalAngle;

      subAngle = (oneAngle + 0x8000 )%0x10000;
      itemTotalAngle = (work->listTotalMove+subAngle);
      while( itemTotalAngle < 0 )
      {
        itemTotalAngle += (LIST_ONE_ANGLE*work->totalItemNum);
      }
      while( itemTotalAngle >= (LIST_ONE_ANGLE*work->totalItemNum) )
      {
        itemTotalAngle -= (LIST_ONE_ANGLE*work->totalItemNum);
      }
      if( work->isSortAnime == FALSE ||
          work->sortAnimeMoveAngle > (0x10000-oneAngle+0x8000)%0x10000 )
      {
        newIdx = (itemTotalAngle+(LIST_ONE_ANGLE/2))/LIST_ONE_ANGLE;
        //計算誤差が起こる・・・(アイテムがリストの個数(14)以下だと起こる
        if( newIdx >= work->totalItemNum )
        {
          newIdx -= work->totalItemNum;
        }
        newItemState = work->itemState[newIdx];
        DUP_FIT_ITEM_SetNewItemState( item , newItemState );
      }
    }
    
    //位置の計算
    if( oneAngle < 0x8000 )
    {
      depth = oneAngle;
    }
    else
    {
      depth = 0x8000-(oneAngle-0x8000);
    }
    dispPos.x = F32_CONST(posX);
    dispPos.y = F32_CONST(posY);
    pos.x = posX + itemSwingOfsX;
    pos.y = posY + itemSwingOfsY;
    pos.z = LIST_DEPTH_BASE + (0x8000-depth);
  
    MUS_ITEM_DRAW_SetPosition( work->itemDrawSys , itemDrawWork , &pos );
    MUS_ITEM_DRAW_SetRotation( work->itemDrawSys , itemDrawWork , itemSwingRot );
    DUP_FIT_ITEM_SetPosition( item , &dispPos );
    //サイズの計算
    {
      if( depth < LIST_SIZE_DEPTH[0] )
      {
        size = FX16_ONE;
        MUS_ITEM_DRAW_SetSize( work->itemDrawSys , itemDrawWork , FX16_ONE , FX16_ONE );
        MUS_ITEM_DRAW_SetDrawEnable( work->itemDrawSys , itemDrawWork , TRUE );
        DUP_FIT_ITEM_SetScale( item , FX32_ONE );
      }
      else if( depth < LIST_SIZE_DEPTH[1] )
      {
        //100%→50%
        const u16 subAngle = LIST_SIZE_DEPTH[1]-depth;
        size = (FX16_CONST(0.5f)*subAngle/(LIST_SIZE_DEPTH[1]-LIST_SIZE_DEPTH[0]))+FX16_CONST(0.5f);
        MUS_ITEM_DRAW_SetSize( work->itemDrawSys , itemDrawWork , size,size );
        MUS_ITEM_DRAW_SetDrawEnable( work->itemDrawSys , itemDrawWork , TRUE );
        DUP_FIT_ITEM_SetScale( item , size );
      }
      else if( depth < LIST_SIZE_DEPTH[2] )
      {
        //50%→0%
        const u16 subAngle = LIST_SIZE_DEPTH[2]-depth;
        size = FX16_CONST(0.5f)*subAngle/(LIST_SIZE_DEPTH[2]-LIST_SIZE_DEPTH[1]);
        MUS_ITEM_DRAW_SetSize( work->itemDrawSys , itemDrawWork , size,size );
        MUS_ITEM_DRAW_SetDrawEnable( work->itemDrawSys , itemDrawWork , TRUE );
        DUP_FIT_ITEM_SetScale( item , size );
      }
      else
      {
        MUS_ITEM_DRAW_SetDrawEnable( work->itemDrawSys , itemDrawWork , FALSE );
        DUP_FIT_ITEM_SetScale( item , 0 );
        size = 0;
      }
    }
    
    //new!文字
    if( newItemState->isNew == TRUE &&
        work->newPicBlinkCnt < ITEM_LIST_NEW_BLINK )
    {
      const BOOL isShow = TRUE;
      VecFx32 newPos;
      newPos.x = MUSICAL_POS_X_FX(pos.x);
      newPos.y = MUSICAL_POS_Y_FX(pos.y + FX32_CONST(8.0f));
      newPos.z = pos.z+0x600; //0x10000/14が大体0x1200なのでその半分
      GFL_BBD_SetObjectTrans( work->bbdSys , work->newPicBbdIdx[i] , &newPos );
      GFL_BBD_SetObjectSiz( work->bbdSys , work->newPicBbdIdx[i] , &size,&size );
      GFL_BBD_SetObjectDrawEnable( work->bbdSys , work->newPicBbdIdx[i] , &isShow );
    }
    else
    {
      const BOOL isShow = FALSE;
      GFL_BBD_SetObjectDrawEnable( work->bbdSys , work->newPicBbdIdx[i] , &isShow );
    }
    
    item = DUP_FIT_ITEM_GetNextItem(item);
    i++;
  }
  item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupList );
}

//--------------------------------------------------------------
//リストのアイテム暗転処理
//--------------------------------------------------------------
static void DUP_FIT_CheckItemListPallet( FITTING_WORK *work )
{
  FIT_ITEM_WORK *item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupList );

  while( item != NULL )
  {
    ITEM_STATE *nowItemState = DUP_FIT_ITEM_GetItemState( item );
    MUS_ITEM_DRAW_WORK *itemDrawWork = DUP_FIT_ITEM_GetItemDrawWork( item );
    
    //影化処理
    if( nowItemState->isOutList == TRUE )
    {
      MUS_ITEM_DRAW_ChengeGraphicResIdx( work->itemDrawSys , itemDrawWork , nowItemState->itemId , nowItemState->itemShadowResIdx );
    }
    else
    {
      MUS_ITEM_DRAW_ChengeGraphicResIdx( work->itemDrawSys , itemDrawWork , nowItemState->itemId , nowItemState->itemResIdx );
    }
    item = DUP_FIT_ITEM_GetNextItem(item);
  }
}

//--------------------------------------------------------------
//タッチペン操作更新メイン
//--------------------------------------------------------------
static void DUP_FIT_UpdateTpMain( FITTING_WORK *work )
{
  BOOL isTouchList = FALSE;
  BOOL isTouchSort = FALSE;
  BOOL isTouchEquip = FALSE;
  //デモの場合は外から数値が入ってくる
  if( work->isDemo == FALSE )
  {
    work->tpIsTrg = GFL_UI_TP_GetTrg();
    work->tpIsTouch = GFL_UI_TP_GetPointCont( &work->tpx,&work->tpy );
  }
  if( work->tpIsTrg || work->tpIsTouch )
  {
    BOOL hitMinOval = FALSE;
    BOOL hitMaxOval = FALSE;
    
    //取り合えずInfoBarとチェックボタンにかぶらない処理
    if( work->tpy > 192-16 )
    {
      work->tpy = 192-16;
    }
    if( work->tpy < 16 )
    {
      work->tpy = 16;
    }
    
    if( work->holdItemType == IG_FIELD && work->holdItem != NULL )
    {
      //アイテム保持中の処理
      DUP_FIT_UpdateTpHoldingItem( work , TRUE );
    }
    else if( work->tpIsTrg == TRUE )
    {
      isTouchEquip = DUP_FIT_UpdateTpHoldItem(work,work->itemGroupEquip ,IG_EQUIP);
      if( isTouchEquip == TRUE )
      {
        //装備との当たり判定
        DUP_FIT_ITEMGROUP_RemoveItem( work->itemGroupEquip , work->holdItem );
        DUP_FIT_ITEMGROUP_AddItem( work->itemGroupField , work->holdItem );
        //座標あわせと深度設定
        DUP_FIT_UpdateTpHoldingItem( work , FALSE );
      }
      else
      {
        isTouchSort = DUP_FIT_UpdateTpPokeCheck( work );
      }
    }
    if( isTouchEquip == FALSE && isTouchSort == FALSE && 
        ( (work->holdItem == NULL && work->tpIsTrg == TRUE ) || work->holdItemType == IG_LIST ) )
    {
      //アイテムリストの中か？
      //X座標には比をかけてYのサイズで計算する
      const s16 subX = work->tpx - LIST_CENTER_X;
      const s16 subY = work->tpy - LIST_CENTER_Y;
      //内側の円に居るか？
      hitMinOval = DUP_FIT_CheckPointInOval( subX,subY,LIST_TPHIT_MIN_Y,LIST_TPHIT_RATIO_MIN );
      hitMaxOval = DUP_FIT_CheckPointInOval( subX,subY,LIST_TPHIT_MAX_Y,LIST_TPHIT_RATIO_MAX );
      if( hitMinOval == FALSE && hitMaxOval == TRUE )
      {
        isTouchList = TRUE;
        DUP_FIT_UpdateTpList( work , subX , subY );
      }
      else if( (hitMinOval == FALSE && hitMaxOval == FALSE )||
           (hitMinOval == TRUE && hitMaxOval == TRUE ))
      {
        //リストから引っ張り出す判定
        if( DUP_FIT_ITEMGROUP_IsItemMax(work->itemGroupField) == FALSE )
        {
          //円の内側も許可
          if( work->holdItemType == IG_LIST &&
            work->holdItem != NULL && 
            MATH_ABS(work->listSpeed) < DEG_TO_U16(2) )
          {
            DUP_TPrintf("[%d]\n",U16_TO_DEG(MATH_ABS(work->listSpeed)));
            DUP_FIT_CreateItemListToField( work );
          }
        }
      }
      if( isTouchList == FALSE && work->tpIsTrg == TRUE )
      {
        //フィールドから拾う判定
        DUP_FIT_UpdateTpHoldItem(work,work->itemGroupField ,IG_FIELD);
      }
    }
    if( work->tpIsTrg && work->holdItem == NULL &&
        work->tpy > FIT_CHECK_BUTTON_TOP &&
        work->tpx > FIT_CHECK_BUTTON_LEFT &&
        work->tpx < FIT_CHECK_BUTTON_RIGHT )
    {
      //チェックボタン
      work->state = DUS_GO_CHECK;
      work->animeCnt = 0;
      PMSND_PlaySE( DUP_SE_DECIDE );
    }
  }
  else
  {
    //アイテムリストの中か？
    //X座標には比をかけてYのサイズで計算する
    const s16 subX = work->befTpx - LIST_CENTER_X;
    const s16 subY = work->befTpy - LIST_CENTER_Y;
    //内側の円に居るか？
    BOOL hitMinOval,hitMaxOval;
    hitMinOval = DUP_FIT_CheckPointInOval( subX,subY,LIST_TPHIT_MIN_Y,LIST_TPHIT_RATIO_MIN );
    hitMaxOval = DUP_FIT_CheckPointInOval( subX,subY,LIST_TPHIT_MAX_Y,LIST_TPHIT_RATIO_MAX );
    if( work->holdItemType == IG_FIELD && work->holdItem != NULL )
    {
      if( work->snapPos != MUS_POKE_EQU_INVALID )
      {
        DUP_FIT_UpdateTpDropItemToEquip( work );
      }
      else
      if( hitMinOval == FALSE && hitMaxOval == TRUE )
      {
        DUP_FIT_UpdateTpDropItemToList( work , FALSE );
      }
      else
      if( hitMinOval == TRUE )
      {
        DUP_FIT_UpdateTpCancelDropItem( work );
      }
      else
      {
        DUP_FIT_UpdateTpDropItemToField( work );
      }
    }
    
    work->befAngleEnable = FALSE;
    work->holdItem = NULL;
    work->holdItemType = IG_NONE;
    work->befItemType = IG_NONE;
    work->snapPos = MUS_POKE_EQU_INVALID;
  }
  //慣性で回る～
  if( work->listSpeed != 0 && isTouchList == FALSE )
  {
    s32 tempAngle = (s32)work->listAngle+0x10000-work->listSpeed;
    if( tempAngle < 0 )
      tempAngle += 0x10000;
    if( tempAngle >= 0x10000 )
      tempAngle -= 0x10000;
    work->listAngle = tempAngle;
    DUP_FIT_CalcItemListAngle( work , work->listAngle , work->listSpeed , LIST_SIZE_X , LIST_SIZE_Y );
    if( work->listSpeed > 0x80 || work->listSpeed < -0x80 )
    {
      work->listSpeed *= 0.75f;
    }
    else
    { 
      work->listSpeed = 0;
    }
  }
  work->befTpx = work->tpx;
  work->befTpy = work->tpy;
  
}
//--------------------------------------------------------------
//タッチペン操作更新リスト
//--------------------------------------------------------------
static void DUP_FIT_UpdateTpList( FITTING_WORK *work , s16 subX , s16 subY )
{
  //回転は楕円なので角度で見ないほうが良いかも
  //でも角度じゃないと回せない・・・
/*
  const u16 angle = FX_Atan2Idx(FX32_CONST(subX),FX32_CONST(subY));
  const s32 subAngle = work->befAngle - angle;
*/
  const s32 subAngle = GFL_CALC_GetCircleTouchRotate( work->befTpx , DUP_FIT_CALC_OVAL_RATIO(work->befTpy) ,
                                                      work->tpx , DUP_FIT_CALC_OVAL_RATIO(work->tpy) ,
                                                      LIST_CENTER_X , LIST_CENTER_Y ,
                                                      LIST_SIZE_X );

  work->listHoldMove += MATH_ABS(subAngle);
  
  //つかみチェック
  //移動しすぎたのでアイテム保持解除
  if( work->tpIsTrg == TRUE ||
    work->listHoldMove >= DEG_TO_U16(10) )
  {
    FIT_ITEM_WORK *item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupList );
    BOOL isTouchItem = FALSE; //SEの判定に使用
    
    //ついでに初期化
    if( work->tpIsTrg == TRUE )
    {
      work->listSpeed = 0;
    }
    work->listHoldMove = 0;
    work->holdItem = NULL;

    while( item != NULL )
    {
      const ITEM_STATE *nowItemState = DUP_FIT_ITEM_GetItemState( item );
      //サイズを気にするか？
//      if( DUP_FIT_ITEM_GetScale( item ) == FX32_ONE &&
//        DUP_FIT_ITEM_CheckHit( item , work->tpx,work->tpy ) == TRUE )
      if( DUP_FIT_ITEM_CheckHit( item , work->tpx,work->tpy ) == TRUE )
      {
        isTouchItem = TRUE;
        if( nowItemState->isOutList == FALSE )
        {
          work->holdItem = item;
          work->holdItemType = IG_LIST;
          //DUP_FIT_ITEM_CheckLengthSqrt( item , work->tpx,work->tpy );
          item = NULL;
        }
      }
      
      if( work->holdItem == NULL )
      {
        item = DUP_FIT_ITEM_GetNextItem(item);
      }
    }
    
    if( work->tpIsTrg == TRUE )
    {
      if( isTouchItem == TRUE )
      {
        if( work->holdItem != NULL )
        {
          PMSND_PlaySE( DUP_SE_LIST_TOUCH_GOODS );
        }
        else
        {
          //タッチはしたけどholdItemがNULLだったら暗いアイテム
          //でもリストタッチ扱い
          work->holdItemType = IG_LIST;
          PMSND_PlaySE( DUP_SE_LIST_NO_GOODS );
        }
      }
    }

  }
  //回転処理
  if( work->befAngleEnable == TRUE )
  {
    u8 i;
    //前回とのリストに対する角度差
    //前回の座標からどっちに動いたか？の角度
    //const u16 moveAngle = FX_Atan2Idx((work->tpx-work->befTpx)*FX32_ONE,(work->tpy-work->befTpy)*FX32_ONE);
    //回転チェック
    if( MATH_ABS(subAngle) > LIST_ROTATE_ANGLE &&
      MATH_ABS(subAngle) < LIST_ROTATE_LIMIT )
    {
      s32 tempAngle = (s32)work->listAngle+0x10000-subAngle;
      if( tempAngle < 0 )
        tempAngle += 0x10000;
      if( tempAngle >= 0x10000 )
        tempAngle -= 0x10000;
      work->listAngle = tempAngle;
      DUP_FIT_CalcItemListAngle( work , work->listAngle , subAngle , LIST_SIZE_X , LIST_SIZE_Y );

      //リスト回転の計算式をリボンと共通化
      if( subAngle >= 0 && work->listSpeed > 0 &&
          subAngle < work->listSpeed )
      {
        work->listSpeed = (work->listSpeed+subAngle)/2;
      }
      else
      if( subAngle <= 0 && work->listSpeed < 0 &&
          subAngle > work->listSpeed )
      {
        work->listSpeed = (work->listSpeed+subAngle)/2;
      }
      else
      {
        work->listSpeed = subAngle;
      }
      
      //Swing処理
      if( (work->listSpeed > 0 && work->listSwingAngle > 0) ||
          (work->listSpeed < 0 && work->listSwingAngle < 0) )
      {
        //方向逆
        work->listSwingAngle = 0;
      }
      else
      if( work->listSpeed > 0 && work->listSwingAngle > -ITEM_SWING_ANGLE_MAX )
      {
        //最大角度はListSpeedの1.5倍
        if( work->listSpeed*15 > work->listSwingAngle*-10 )
        {
          work->listSwingAngle -= ITEM_SWING_ANGLE_ADD_VALUE;
        }
      }
      else
      if( work->listSpeed < 0 && work->listSwingAngle < ITEM_SWING_ANGLE_MAX )
      {
        //最大角度はListSpeedの1.5倍
        if( work->listSpeed*-15 > work->listSwingAngle*10 )
        {
          work->listSwingAngle += ITEM_SWING_ANGLE_ADD_VALUE;
        }
      }
    }
    else
    {
      work->listSpeed = 0;
    }
  }
//  work->befAngle = angle;
  work->befAngleEnable = TRUE;
}

//--------------------------------------------------------------
//フィールド・装備のアイテムを拾う
//--------------------------------------------------------------
static const BOOL DUP_FIT_UpdateTpHoldItem( FITTING_WORK *work , FIT_ITEM_GROUP *itemGroupe , ITEM_GROUPE groupeType )
{
  FIT_ITEM_WORK* item = DUP_FIT_ITEMGROUP_GetStartItem( itemGroupe );
  while( item != NULL )
  {
    if( DUP_FIT_ITEM_CheckHit( item , work->tpx,work->tpy ) == TRUE )
    {
      const GFL_POINT *itemPos = DUP_FIT_ITEM_GetPosition( item );
      work->holdItem = item;
      work->holdItemType = IG_FIELD;
      DUP_TPrintf("Item Hold[%d]\n",DUP_FIT_ITEM_GetItemState(item)->itemId);

      PMSND_PlaySE( DUP_SE_LIST_TOUCH_GOODS );

      //戻す時用の処理
      work->befItemType = groupeType;
      work->befItemPos.x = itemPos->x;
      work->befItemPos.y = itemPos->y;
      return TRUE;
    }
    else
    {
      item = DUP_FIT_ITEM_GetNextItem(item);
    }
  }
  return FALSE;
}
//--------------------------------------------------------------
//フィールドのアイテムを持っている
//--------------------------------------------------------------
static void DUP_FIT_UpdateTpHoldingItem( FITTING_WORK *work , const BOOL playSnapSe )
{
  u16 snapLen = HOLD_ITEM_SNAP_LENGTH;
  GFL_POINT dispPos;
  GFL_POINT pokePosSub;
  VecFx32 pos = {0,0,HOLD_ITEM_DEPTH};
  fx16 scaleX,scaleY;
  u16 rotZ = 0;
  const MUS_POKE_EQUIP_POS befSnapPos = work->snapPos;
  MUS_ITEM_DRAW_WORK *itemDrawWork = DUP_FIT_ITEM_GetItemDrawWork( work->holdItem );
  
  pokePosSub.x = work->tpx - FIT_POKE_POS_X;
  pokePosSub.y = work->tpy - FIT_POKE_POS_Y;
  work->snapPos = DUP_FIT_SearchEquipPosition( work ,itemDrawWork, &pokePosSub , &snapLen );
  if( work->snapPos != MUS_POKE_EQU_INVALID &&
    DUP_FIT_CheckIsEquipItem( work , work->snapPos ) == FALSE )
  {
    MUS_POKE_EQUIP_DATA *equipData = MUS_POKE_DRAW_GetEquipData( work->drawWork , work->snapPos );
    dispPos.x = (int)F32_CONST(equipData->pos.x+equipData->ofs.x)+128;
    dispPos.y = (int)F32_CONST(equipData->pos.y+equipData->ofs.y)+ 96;
    rotZ = equipData->itemRot;
    MUS_ITEM_DRAW_SetUseOffset( work->itemDrawSys , itemDrawWork , TRUE );
    if( MUS_ITEM_DRAW_IsBackItem( itemDrawWork ) == TRUE )
    {
      pos.z = EQUIP_ITEM_DEPTH_BACK;
    }
    else
    if( MUS_ITEM_DRAW_IsFrontItem( itemDrawWork ) == TRUE )
    {
      pos.z = EQUIP_ITEM_DEPTH_FRONT;
    }
    
    //一度くっついたらキャンセル時そこへ
    work->befItemType = IG_EQUIP;
    work->befItemPos.x = dispPos.x;
    work->befItemPos.y = dispPos.y;
    DUP_FIT_ITEM_SetEquipPos( work->holdItem , work->snapPos );
    
    if( befSnapPos != work->snapPos && playSnapSe == TRUE )
    {
      PMSND_PlaySE( DUP_SE_POKE_SET_GOODS );
    }
    
  }
  else
  {
    work->snapPos = MUS_POKE_EQU_INVALID;
    dispPos.x = work->tpx;
    dispPos.y = work->tpy;
    MUS_ITEM_DRAW_SetUseOffset( work->itemDrawSys , itemDrawWork , FALSE );
  }

  pos.x = FX32_CONST(dispPos.x);
  pos.y = FX32_CONST(dispPos.y);
  MUS_ITEM_DRAW_SetPosition( work->itemDrawSys , itemDrawWork , &pos );
  DUP_FIT_ITEM_SetPosition( work->holdItem , &dispPos );
  MUS_ITEM_DRAW_SetRotation( work->itemDrawSys , DUP_FIT_ITEM_GetItemDrawWork(work->holdItem) , rotZ );
  
  MUS_ITEM_DRAW_GetSize( work->itemDrawSys , itemDrawWork ,&scaleX,&scaleY );
  if( scaleX < FX16_ONE || scaleY < FX16_ONE )
  {
    scaleX += FX16_CONST( 0.1f );
    scaleY += FX16_CONST( 0.1f );
    if( scaleX > FX16_ONE )
    {
      scaleX = FX16_ONE;
    }
    if( scaleY > FX16_ONE )
    {
      scaleY = FX16_ONE;
    }
    MUS_ITEM_DRAW_SetSize( work->itemDrawSys , itemDrawWork ,scaleX,scaleY );
  }
}


//--------------------------------------------------------------
//リストからフィールドへアイテムを置く(そのまま保持状態へ
//--------------------------------------------------------------
static void DUP_FIT_CreateItemListToField( FITTING_WORK *work )
{
  FIT_ITEM_WORK* item;
  VecFx32 pos = {0,0,HOLD_ITEM_DEPTH};
  ITEM_STATE *itemState = DUP_FIT_ITEM_GetItemState( work->holdItem );
  MUS_ITEM_DRAW_WORK *itemDrawWork,*holdDrawWork;
  GFL_POINT dispPos;
  GFL_POINT *befPos;
  fx16 scaleX,scaleY;

  dispPos.x = work->tpx;
  dispPos.y = work->tpy;
  pos.x = FX32_CONST(work->tpx);
  pos.y = FX32_CONST(work->tpy);

  item = DUP_FIT_ITEM_CreateItem( work->heapId , work->itemDrawSys , itemState , &pos );
  DUP_FIT_ITEMGROUP_AddItem( work->itemGroupField,item );

  DUP_FIT_ITEM_SetPosition( item , &dispPos );
  //サイズの引継ぎ
  holdDrawWork = DUP_FIT_ITEM_GetItemDrawWork( work->holdItem );
  itemDrawWork = DUP_FIT_ITEM_GetItemDrawWork( item );
  MUS_ITEM_DRAW_GetSize( work->itemDrawSys , holdDrawWork ,&scaleX,&scaleY );
  MUS_ITEM_DRAW_SetSize( work->itemDrawSys , itemDrawWork ,scaleX,scaleY );

  //座標記憶
  befPos = DUP_FIT_ITEM_GetPosition( work->holdItem );
  work->befItemType = IG_LIST;
  work->befItemPos.x = befPos->x;
  work->befItemPos.y = befPos->y;

  //保持アイテムを今生成したものに変える
  work->holdItem = item;
  work->holdItemType = IG_FIELD;
  work->befItemType = IG_LIST;
  
  //アイテムを取った状態に
  itemState->isOutList = TRUE;
  DUP_FIT_CheckItemListPallet( work );
  
  PMSND_PlaySE( DUP_SE_LIST_TAKE_GOODS );

}

//--------------------------------------------------------------
//アイテムを離す フィールドへ
//--------------------------------------------------------------
static void DUP_FIT_UpdateTpDropItemToField( FITTING_WORK *work )
{
  fx32 depth = FIELD_ITEM_DEPTH;
  FIT_ITEM_WORK *item;
  MUS_ITEM_DRAW_WORK *holdDrawWork = DUP_FIT_ITEM_GetItemDrawWork( work->holdItem );
  //一度アイテムをリストから取り、頭につなぎ直す
  DUP_FIT_ITEMGROUP_RemoveItem( work->itemGroupField , work->holdItem );
  DUP_FIT_ITEMGROUP_AddItemTop( work->itemGroupField , work->holdItem );
  
  MUS_ITEM_DRAW_SetSize( work->itemDrawSys , holdDrawWork ,FX16_ONE,FX16_ONE );
  //深度の再設定
  item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupField );
  while( item != NULL )
  {
    VecFx32 pos;
    MUS_ITEM_DRAW_WORK *drawWork = DUP_FIT_ITEM_GetItemDrawWork( item );
    MUS_ITEM_DRAW_GetPosition( work->itemDrawSys , drawWork , &pos );
    pos.z = depth;
    MUS_ITEM_DRAW_SetPosition( work->itemDrawSys , drawWork , &pos );
    
    depth -= FX32_CONST(0.1f);
    item = DUP_FIT_ITEM_GetNextItem(item);
  }
  
#if DEB_ARI
  DUP_FIT_ITEM_DumpList( work->itemGroupField ,2 );
#endif
}
//--------------------------------------------------------------
//アイテムを離す リストへ
//--------------------------------------------------------------
static void DUP_FIT_UpdateTpDropItemToList( FITTING_WORK *work , const BOOL isMove )
{
  VecFx32 pos;
  MUS_ITEM_DRAW_WORK *drawWork = DUP_FIT_ITEM_GetItemDrawWork( work->holdItem );
  MUS_ITEM_DRAW_WORK *holdDrawWork = DUP_FIT_ITEM_GetItemDrawWork( work->holdItem );
  ITEM_STATE *itemState = DUP_FIT_ITEM_GetItemState( work->holdItem );

  //リストを付け替え
  DUP_FIT_ITEMGROUP_RemoveItem( work->itemGroupField , work->holdItem );
  DUP_FIT_ITEMGROUP_AddItemTop( work->itemGroupAnime , work->holdItem );

  //開始位置設定
  {
    GFL_POINT dispPos;
    VecFx32 pos = {0,0,ITEM_RETURN_DEPTH};
    dispPos.x = work->tpx;
    dispPos.y = work->tpy;
    pos.x = FX32_CONST(work->befItemPos.x);
    pos.y = FX32_CONST(work->befItemPos.y);
    DUP_FIT_ITEM_SetBefPosition( work->holdItem , &dispPos );
    MUS_ITEM_DRAW_SetPosition( work->itemDrawSys , holdDrawWork , &pos );
    MUS_ITEM_DRAW_SetSize( work->itemDrawSys , holdDrawWork ,FX16_ONE,FX16_ONE );
    DUP_FIT_ITEM_SetScale( work->holdItem , FX16_ONE );
  }
  
  //動作終了地点設定
  //リスト内に戻すアイテムがあるか？
  {
    FIT_ITEM_WORK *targetItem = NULL;
    FIT_ITEM_WORK *listItem = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupList );
    while( listItem != NULL && targetItem == NULL )
    {
      const ITEM_STATE *listItemState = DUP_FIT_ITEM_GetItemState( listItem );
      if( listItemState->itemId == itemState->itemId )
      {
        targetItem = listItem;
      }
      listItem = DUP_FIT_ITEM_GetNextItem(listItem);
    }
    
    //リスト内にアイテムがあった！
    if( targetItem != NULL )
    {
      GFL_POINT *befPos = DUP_FIT_ITEM_GetPosition( targetItem );
      DUP_FIT_ITEM_SetPosition( work->holdItem , befPos );
      DUP_FIT_ITEM_SetBefScale( work->holdItem , DUP_FIT_ITEM_GetScale(targetItem) );

    }
    else
    {
      //無かった！！
      GFL_POINT befPos = { LIST_CENTER_X , LIST_CENTER_Y-LIST_SIZE_Y };
      DUP_FIT_ITEM_SetPosition( work->holdItem , &befPos );
      DUP_FIT_ITEM_SetBefScale( work->holdItem , 0 );
    }
  }

  DUP_FIT_ITEM_SetCount( work->holdItem , ITEM_RETURN_ANIME_CNT );

  //アイテムを戻した状態に
  itemState->isOutList = FALSE;
  DUP_FIT_CheckItemListPallet( work );

  PMSND_PlaySE( DUP_SE_LIST_RETURN_GOODS );

#if DEB_ARI
  DUP_FIT_ITEM_DumpList( work->itemGroupField ,2 );
#endif
}

//--------------------------------------------------------------
//アイテムを離す 装備へ
//--------------------------------------------------------------
static void DUP_FIT_UpdateTpDropItemToEquip(  FITTING_WORK *work )
{
  VecFx32 pos;
  fx32 depthOfs = 0;
  FIT_ITEM_WORK *item;
  MUS_ITEM_DRAW_WORK *drawWork = DUP_FIT_ITEM_GetItemDrawWork( work->holdItem );
  MUS_ITEM_DRAW_WORK *holdDrawWork = DUP_FIT_ITEM_GetItemDrawWork( work->holdItem );
  //リストを付け替えて座標を再設定
  DUP_FIT_ITEMGROUP_RemoveItem( work->itemGroupField , work->holdItem );
  DUP_FIT_ITEMGROUP_AddItemTop( work->itemGroupEquip , work->holdItem );

  MUS_ITEM_DRAW_SetSize( work->itemDrawSys , holdDrawWork ,FX16_ONE,FX16_ONE );

  DUP_FIT_ITEM_SetEquipPos( work->holdItem , work->snapPos );
  
  //深度の再設定
  item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupEquip );
  while( item != NULL )
  {
    VecFx32 pos;
    MUS_ITEM_DRAW_WORK *drawWork = DUP_FIT_ITEM_GetItemDrawWork( item );
    MUS_ITEM_DRAW_GetPosition( work->itemDrawSys , drawWork , &pos );
    if( MUS_ITEM_DRAW_IsBackItem( drawWork ) == TRUE )
    {
      pos.z = depthOfs+EQUIP_ITEM_DEPTH_BACK;
    }
    else
    if( MUS_ITEM_DRAW_IsFrontItem( drawWork ) == TRUE )
    {
      pos.z = depthOfs+EQUIP_ITEM_DEPTH_FRONT;
    }
    else
    {
      pos.z = depthOfs+EQUIP_ITEM_DEPTH;
    }
    MUS_ITEM_DRAW_SetPosition( work->itemDrawSys , drawWork , &pos );
    
    depthOfs -= FX32_CONST(0.1f);
    item = DUP_FIT_ITEM_GetNextItem(item);
  }
  
#if DEB_ARI
  DUP_FIT_ITEM_DumpList( work->itemGroupField ,2 );
#endif
  
}

//--------------------------------------------------------------
//アイテムを取った位置に戻す
//--------------------------------------------------------------
static void DUP_FIT_UpdateTpCancelDropItem( FITTING_WORK *work )
{
  if( work->befItemType == IG_FIELD )
  {
      //元フィールドアイテム
    VecFx32 pos = {0,0,HOLD_ITEM_DEPTH};
    MUS_ITEM_DRAW_WORK *itemDrawWork = DUP_FIT_ITEM_GetItemDrawWork( work->holdItem );
    GFL_POINT dispPos;
    //動作開始地点設定
    dispPos.x = work->tpx;
    dispPos.y = work->tpy;
    pos.x = FX32_CONST(work->befItemPos.x);
    pos.y = FX32_CONST(work->befItemPos.y);
    DUP_FIT_ITEM_SetBefPosition( work->holdItem , &dispPos );
    MUS_ITEM_DRAW_SetPosition( work->itemDrawSys , itemDrawWork , &pos );
    //動作終了地点設定
    DUP_FIT_ITEM_SetPosition( work->holdItem , &work->befItemPos );
    DUP_FIT_ITEM_SetCount( work->holdItem , ITEM_RETURN_POS_CNT );
    
    DUP_FIT_UpdateTpDropItemToField( work );
  }
  else
  if( work->befItemType == IG_EQUIP )
  {
    //元装備アイテム
    VecFx32 pos;
    GFL_POINT dispPos;
    GFL_POINT befPos;
    u16 rotZ;
    MUS_ITEM_DRAW_WORK *itemDrawWork = DUP_FIT_ITEM_GetItemDrawWork( work->holdItem );
    const MUS_POKE_EQUIP_POS ePos = DUP_FIT_ITEM_GetEquipPos( work->holdItem);
    MUS_POKE_EQUIP_DATA *equipData = MUS_POKE_DRAW_GetEquipData( work->drawWork , ePos );
    
    //動作終了地点設定
    befPos.x = (int)F32_CONST(equipData->pos.x+equipData->ofs.x)+128;
    befPos.y = (int)F32_CONST(equipData->pos.y+equipData->ofs.y)+96;
    
    pos.x = FX32_CONST(befPos.x);
    pos.y = FX32_CONST(befPos.y);
    if( MUS_ITEM_DRAW_IsBackItem( itemDrawWork ) == TRUE )
    {
      pos.z = EQUIP_ITEM_DEPTH_BACK;
    }
    else
    if( MUS_ITEM_DRAW_IsFrontItem( itemDrawWork ) == TRUE )
    {
      pos.z = EQUIP_ITEM_DEPTH_FRONT;
    }
    else
    {
      pos.z = EQUIP_ITEM_DEPTH;
    }
    rotZ = equipData->itemRot;
    
    MUS_ITEM_DRAW_SetUseOffset( work->itemDrawSys , itemDrawWork , TRUE );
    MUS_ITEM_DRAW_SetPosition( work->itemDrawSys , itemDrawWork , &pos );
    MUS_ITEM_DRAW_SetRotation( work->itemDrawSys , DUP_FIT_ITEM_GetItemDrawWork(work->holdItem) , rotZ );
    DUP_FIT_ITEM_SetPosition( work->holdItem , &befPos );

    //動作開始地点設定
    dispPos.x = work->tpx;
    dispPos.y = work->tpy;
    DUP_FIT_ITEM_SetBefPosition( work->holdItem , &dispPos );
    DUP_FIT_ITEM_SetCount( work->holdItem , ITEM_RETURN_POS_CNT );

    //吸着地点をダミー設定。一応終わったら戻す
    work->snapPos = ePos;
    DUP_FIT_UpdateTpDropItemToEquip( work );
    work->snapPos = MUS_POKE_EQU_INVALID;
  }
  else
  if( work->befItemType == IG_LIST )
  {
    //元リストアイテム
    GFL_POINT *befPos = DUP_FIT_ITEM_GetBefPosition( work->holdItem );
    GFL_POINT dispPos;
    VecFx32 pos = {0,0,ITEM_RETURN_DEPTH};
    MUS_ITEM_DRAW_WORK *itemDrawWork = DUP_FIT_ITEM_GetItemDrawWork( work->holdItem );

   //動作終了地点設定
    DUP_FIT_ITEM_SetPosition( work->holdItem , &work->befItemPos );
    DUP_FIT_ITEM_SetCount( work->holdItem , ITEM_RETURN_POS_CNT );
    
    //動作開始地点設定
    dispPos.x = work->tpx;
    dispPos.y = work->tpy;
    pos.x = FX32_CONST(work->befItemPos.x);
    pos.y = FX32_CONST(work->befItemPos.y);
    DUP_FIT_ITEM_SetBefPosition( work->holdItem , &dispPos );
    MUS_ITEM_DRAW_SetPosition( work->itemDrawSys , itemDrawWork , &pos );

    DUP_FIT_UpdateTpDropItemToList( work , TRUE);
  }
  else
  {
    GF_ASSERT_MSG(0,"Invalid befItemType!![%d]\n",work->befItemType);
    DUP_FIT_UpdateTpDropItemToList( work , FALSE);
  }
}

//--------------------------------------------------------------
//ポケモンのタッチチェック（ソート用)
//--------------------------------------------------------------
static const BOOL DUP_FIT_UpdateTpPokeCheck( FITTING_WORK *work )
{
  u16 snapLen = 16;
  GFL_POINT pokePosSub;
  MUS_POKE_EQUIP_POS touchPos;
  u16 startIdx;
  //1週半と1個先のところを先頭にしてソート(スタートは下
  const u32 startAngle = work->listTotalMove + 0x18000;
  //1と1/4週と1個先のところを先頭にしてソート(スタートは左
  //const u32 startAngle = work->listTotalMove + 0x14000+LIST_ONE_ANGLE;
  
  if( startAngle >= work->totalItemNum * LIST_ONE_ANGLE )
  {
    startIdx = (startAngle-(work->totalItemNum * LIST_ONE_ANGLE)) / LIST_ONE_ANGLE;
  }
  else
  {
    startIdx = startAngle / LIST_ONE_ANGLE;
  }
  
  pokePosSub.x = work->tpx - FIT_POKE_POS_X;
  pokePosSub.y = work->tpy - FIT_POKE_POS_Y;
  touchPos = DUP_FIT_SearchEquipPosition( work ,NULL, &pokePosSub , &snapLen );

  if( touchPos != MUS_POKE_EQU_INVALID )
  {
    MUS_POKE_EQUIP_USER userType = MUS_ITEM_DATA_EquipPosToUserType( touchPos );
    //手は出てくるものは同じだけど、左右バラ扱いなので
    if( userType == MUS_POKE_EQU_USER_HAND_L )
    {
      userType = MUS_POKE_EQU_USER_HAND_R;
    }
    if( userType == work->sortType )
    {
      //デフォルトでソート
      DUP_FIT_SortItemIdx( work , MUS_POKE_EQUIP_USER_MAX , startIdx );
      work->sortType = MUS_POKE_EQUIP_USER_MAX;
    }
    else
    {
      DUP_FIT_SortItemIdx( work , userType , startIdx );
      work->sortType = userType;
    }
    work->isSortAnime = TRUE;
    work->sortAnimeMoveAngle = 0;
    work->sortAnimeEndAngle = 0x10000 + work->listAngle%LIST_ONE_ANGLE;
    //listAngleのあまりが０だったら
    /*  //1週半のときだけ有効にする
    if( work->sortAnimeEndAngle == 0x10000 )
    {
      work->sortAnimeEndAngle += LIST_ONE_ANGLE;
    }
    */
    
    DUP_FIT_CalcItemListAngle( work , work->listAngle , 0 , LIST_SIZE_X , LIST_SIZE_Y );
    
    PMSND_PlaySE( DUP_SE_POKE_TOUCH );
    return TRUE;
  }
  else
  {
    return FALSE;
    //円の内側をタッチし、何も無かったらデフォルトでソート
    //DUP_FIT_SortItemIdx( work , MUS_POKE_EQUIP_USER_MAX , startIdx );
    //work->sortType = MUS_POKE_EQUIP_USER_MAX;
  }
  
}

//--------------------------------------------------------------
//ソート中アニメ
//--------------------------------------------------------------
static void DUP_FIT_UpdateSortAnime( FITTING_WORK *work )
{
  static const u16 SORT_ANIME_MIN_SPD = 256;
  u32 leastAngle;
  u32 moveAngle;

  leastAngle = work->sortAnimeEndAngle - work->sortAnimeMoveAngle;
  
  moveAngle = leastAngle/10;
  if( moveAngle < SORT_ANIME_MIN_SPD )
  {
    if( leastAngle < SORT_ANIME_MIN_SPD )
    {
      moveAngle = leastAngle;
      work->isSortAnime = FALSE;
    }
    else
    {
      moveAngle = SORT_ANIME_MIN_SPD;
    }
  }
  
  if( leastAngle > 0x1000 )
  {
    work->listSwingAngle -= ITEM_SWING_ANGLE_SORT_ADD_VALUE;
    if( work->listSwingAngle < -ITEM_SWING_ANGLE_SORT_MAX )
    {
      work->listSwingAngle = -ITEM_SWING_ANGLE_SORT_MAX;
    }
  }
  else
  {
    //アイテム揺らし更新
    DUP_FIT_UpdateSwingItem( work );
  }

  work->sortAnimeMoveAngle += moveAngle;
  work->listAngle -= moveAngle;
  DUP_FIT_CalcItemListAngle( work , work->listAngle , moveAngle , LIST_SIZE_X , LIST_SIZE_Y );
}

//--------------------------------------------------------------
//アイテム揺らし更新
//--------------------------------------------------------------
static void DUP_FIT_UpdateSwingItem( FITTING_WORK *work )
{
  //リストの速度が一定以下＆ソートアニメ以外
  if( MATH_ABS(work->listSpeed) < 0x30 )
  {
    if( work->listSwingAngle != 0 ||
        work->listSwingSpeed != 0 )
    {
      static const s16 accSpeed = 0x80;
      work->listSwingAngle += work->listSwingSpeed;
      if( work->listSwingAngle > 0 )
      {
        if( work->listSwingSpeed > 0 )
        {
          work->listSwingSpeed -= accSpeed;
        }
        else
        {
          //向きが逆なので少なく
          work->listSwingSpeed -= accSpeed/3;
        }
      }
      else
      if( work->listSwingAngle < 0 )
      {
        if( work->listSwingSpeed < 0 )
        {
          work->listSwingSpeed += accSpeed;
        }
        else
        {
          //向きが逆なので少なく
          work->listSwingSpeed += accSpeed/3;
        }
      }
      //DUP_TPrintf("[%d][%d]\n",work->listSwingAngle,work->listSwingSpeed);
      //停止条件。速度＆加速度が一定以下
      if( MATH_ABS(work->listSwingAngle) < 0x200 &&
          MATH_ABS(work->listSwingSpeed) < 0x200 )
      {
        work->listSwingSpeed = 0;
        work->listSwingAngle = 0;
      }
      DUP_FIT_CalcItemListAngle( work , work->listAngle , 0 , LIST_SIZE_X , LIST_SIZE_Y );
    }
  }
}

//--------------------------------------------------------------
//指定箇所に装備があるか？
//--------------------------------------------------------------
static const BOOL DUP_FIT_CheckIsEquipItem( FITTING_WORK *work , const MUS_POKE_EQUIP_POS pos)
{
  const MUS_POKE_EQUIP_USER uPos = MUS_ITEM_DATA_EquipPosToUserType( pos );
  FIT_ITEM_WORK *item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupEquip );
  while( item != NULL )
  {
    const MUS_POKE_EQUIP_USER checkUPos = MUS_ITEM_DATA_EquipPosToUserType( DUP_FIT_ITEM_GetEquipPos( item ) );
    if( uPos == checkUPos )
    {
      return TRUE;
    }
    item = DUP_FIT_ITEM_GetNextItem(item);
  }
  return FALSE;
}
//--------------------------------------------------------------
//指定点から近い位置を探す(相対座標にして渡してください
//--------------------------------------------------------------
static MUS_POKE_EQUIP_POS DUP_FIT_SearchEquipPosition(  FITTING_WORK *work , MUS_ITEM_DRAW_WORK *itemDrawWork , GFL_POINT *pos , u16 *len )
{
  MUS_POKE_EQUIP_POS i;
  u16 minLen = (*len)*(*len); //あらかじめ最小距離に指定距離を入れておく
  MUS_POKE_EQUIP_POS minPos = MUS_POKE_EQU_INVALID;
  for( i=0; i<MUS_POKE_EQUIP_MAX ;i++ )
  {
    const BOOL canEquipPos = ( itemDrawWork == NULL ? TRUE : MUS_ITEM_DRAW_CanEquipPos( itemDrawWork , i ) );
    MUS_POKE_EQUIP_DATA *equipData = MUS_POKE_DRAW_GetEquipData( work->drawWork , i );
    if( equipData->isEnable == TRUE && 
       canEquipPos == TRUE )
    {
      const int equipPosX = (int)F32_CONST(equipData->pos.x+equipData->ofs.x)+128-FIT_POKE_POS_X;
      const int equipPosY = (int)F32_CONST(equipData->pos.y+equipData->ofs.y)+96 -FIT_POKE_POS_Y;
      const int subX = pos->x - equipPosX;
      const int subY = pos->y - equipPosY;
      const int subLen = (subX*subX)+(subY*subY);
      if( subLen < minLen )
      {
        minLen = subLen;
        minPos = i;
      }
    }
  }
  *len = minLen;
  return minPos;
}

//--------------------------------------------------------------
//アイテムのアニメーション
//--------------------------------------------------------------
static void DUP_FIT_UpdateItemAnime( FITTING_WORK *work )
{
  //itemGroupAnimeに登録されている物の処理
  {
    FIT_ITEM_WORK *nextItem = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupAnime );
    while( nextItem != NULL )
    {
      FIT_ITEM_WORK *item = nextItem;
      MUS_ITEM_DRAW_WORK *drawWork = DUP_FIT_ITEM_GetItemDrawWork( item );
      u16 cnt = DUP_FIT_ITEM_GetCount( item );
      GFL_POINT *befPos = DUP_FIT_ITEM_GetBefPosition( item );
      GFL_POINT *endPos = DUP_FIT_ITEM_GetPosition( item );
      fx16 befScale = DUP_FIT_ITEM_GetBefScale( item );

      //removeする可能性があるので、先に次をとっておく
      nextItem = DUP_FIT_ITEM_GetNextItem(item);

      cnt--;
      if( cnt > 0 )
      {
        VecFx32 pos;
        fx16 size = ((FX16_ONE-befScale) * (cnt) / ITEM_RETURN_ANIME_CNT)+befScale;
        MUS_ITEM_DRAW_SetSize( work->itemDrawSys , drawWork , size , size );
        MUS_ITEM_DRAW_GetPosition( work->itemDrawSys , drawWork , &pos );
        
        if( cnt >= ITEM_RETURN_ANIME_CNT-ITEM_RETURN_POS_CNT )
        {
          //縮小は30レーム
          //戻りの処理は他とあわせて5フレーム
          const u16 tempCnt = cnt - (ITEM_RETURN_ANIME_CNT-ITEM_RETURN_POS_CNT);
          pos.x = FX32_CONST((endPos->x-befPos->x)*(ITEM_RETURN_POS_CNT-tempCnt)/ITEM_RETURN_POS_CNT+befPos->x);
          pos.y = FX32_CONST((endPos->y-befPos->y)*(ITEM_RETURN_POS_CNT-tempCnt)/ITEM_RETURN_POS_CNT+befPos->y);
        }
        else
        {
          pos.x = FX32_CONST(endPos->x);
          pos.y = FX32_CONST(endPos->y);
        }
        pos.z = ITEM_RETURN_DEPTH;
        MUS_ITEM_DRAW_SetPosition( work->itemDrawSys , drawWork , &pos );
        DUP_FIT_ITEM_SetCount( item , cnt );
      }
      else
      {
        DUP_FIT_ITEMGROUP_RemoveItem( work->itemGroupAnime , item );
        DUP_FIT_ITEM_DeleteItem( item , work->itemDrawSys );
      }
    }
  }
  
  //フィールド・装備のアイテムキャンセル戻り処理
  {
    BOOL isEquipList = FALSE;
    FIT_ITEM_WORK *item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupField );
    if( item == NULL )
    {
      //フィールドが無かったら装備に
      isEquipList = TRUE;
      item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupEquip );
    }
    while( item != NULL )
    {
      u16 cnt = DUP_FIT_ITEM_GetCount( item );
      if( cnt > 0 )
      {
        MUS_ITEM_DRAW_WORK *drawWork = DUP_FIT_ITEM_GetItemDrawWork( item );
        GFL_POINT *befPos = DUP_FIT_ITEM_GetBefPosition( item );
        GFL_POINT *endPos = DUP_FIT_ITEM_GetPosition( item );
        cnt--;
        if( cnt > 0 )
        {
          VecFx32 pos;
          MUS_ITEM_DRAW_GetPosition( work->itemDrawSys , drawWork , &pos );
          pos.x = FX32_CONST((endPos->x-befPos->x)*(ITEM_RETURN_POS_CNT-cnt)/ITEM_RETURN_POS_CNT+befPos->x);
          pos.y = FX32_CONST((endPos->y-befPos->y)*(ITEM_RETURN_POS_CNT-cnt)/ITEM_RETURN_POS_CNT+befPos->y);
          MUS_ITEM_DRAW_SetPosition( work->itemDrawSys , drawWork , &pos );
        }
        else
        {
          VecFx32 pos;
          MUS_ITEM_DRAW_GetPosition( work->itemDrawSys , drawWork , &pos );
          pos.x = FX32_CONST(endPos->x);
          pos.y = FX32_CONST(endPos->y);
          MUS_ITEM_DRAW_SetPosition( work->itemDrawSys , drawWork , &pos );
          DUP_FIT_ITEM_SetBefPosition( item , endPos );
        }
        
        DUP_FIT_ITEM_SetCount( item , cnt );
      }
      
      item = DUP_FIT_ITEM_GetNextItem(item);
      
      if( item == NULL && isEquipList == FALSE )
      {
          //フィールドが見終わったら装備も
        isEquipList = TRUE;
        item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupEquip );
      }
    }
  }

}

//--------------------------------------------------------------
//影の更新
//--------------------------------------------------------------
static void DUP_FIT_UpdateShadow( FITTING_WORK *work )
{
  if( work->holdItem == NULL )
  {
    MUS_ITEM_DRAW_SetDrawEnable( work->itemDrawSys , work->shadowItem , FALSE );
  }
  else  //VBlankで絵の転送ができないと表示しない
  {
    const ITEM_STATE *itemState = DUP_FIT_ITEM_GetItemState(work->holdItem);
    if( work->shadowItemId == itemState->itemId )
    {
      VecFx32 pos;
      MUS_ITEM_DRAW_SetDrawEnable( work->itemDrawSys , work->shadowItem , TRUE );
      /*
      MUS_ITEM_DRAW_GetPosition( work->itemDrawSys , DUP_FIT_ITEM_GetItemDrawWork(work->holdItem) , &pos );
  //    OS_Printf("[%.2f][%.2f]\n",F32_CONST(pos.x),F32_CONST(pos.y));
      pos.x += FX32_CONST(32.0f );
      pos.y += FX32_CONST(32.0f );
      pos.z -= FX32_CONST( 0.5f );
      MUS_ITEM_DRAW_SetPosition( work->itemDrawSys , work->shadowItem , &pos );
      MUS_ITEM_DRAW_SetUseOffset( work->itemDrawSys , work->shadowItem
              , MUS_ITEM_DRAW_GetUseOffset( work->itemDrawSys , DUP_FIT_ITEM_GetItemDrawWork(work->holdItem) ) );
      */
      MUS_ITEM_DRAW_CopyItemDataToShadow( work->itemDrawSys , DUP_FIT_ITEM_GetItemDrawWork(work->holdItem) , work->shadowItem );
    }
  }
}


//--------------------------------------------------------------
//楕円の当たり判定
//--------------------------------------------------------------
static BOOL DUP_FIT_CheckPointInOval( s16 subX , s16 subY , s16 size , float ratioYX )
{
  const s16 newSubX = (s16)(subX*ratioYX);
  if( newSubX * newSubX + subY * subY < size*size )
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

//--------------------------------------------------------------
//列の並べ替え
//--------------------------------------------------------------
static void DUP_FIT_SortItemIdx( FITTING_WORK *work , const MUS_POKE_EQUIP_USER ePos , const u16 startIdx)
{
  u16 i,j;
  for( i=0 ; i<work->totalItemNum ; i++ )
  {
    const u16 idx = (startIdx+i)%work->totalItemNum;
    work->itemState[idx] = &work->itemStateBase[i];
  }
  for( i=0 ; i<work->totalItemNum ; i++ )
  {
    const u16 idx_i = (startIdx+i)%work->totalItemNum;

    u16 biggerIdx = idx_i;
    u16 biggerPri;
    biggerPri = DUP_FIT_CalcSortPriority( work , work->itemState[idx_i] , ePos );
    for( j=i+1 ; j<work->totalItemNum ; j++ )
    {
      const u16 idx_j = (startIdx+j)%work->totalItemNum;
      const u16 jPir = DUP_FIT_CalcSortPriority( work , work->itemState[idx_j] , ePos );
      if( biggerPri < jPir )
      {
        biggerPri = jPir;
        biggerIdx = idx_j;
      }
    }
    if( biggerIdx != idx_i )
    {
      ITEM_STATE *temp;
      temp = work->itemState[idx_i];
      work->itemState[idx_i] = work->itemState[biggerIdx];
      work->itemState[biggerIdx] = temp;
    }
  }
  
  //dump
#if 0
  for( i=0 ; i<work->totalItemNum ; i++ )
  {
    u16 biggerPri;
    biggerPri = DUP_FIT_CalcSortPriority( work , work->itemState[i] , ePos );
    ARI_TPrintf("[%2d:%4d]\n",work->itemState[i]->itemId,biggerPri);
  }
#endif
}

static const u16 DUP_FIT_CalcSortPriority( FITTING_WORK *work , ITEM_STATE *itemState , const MUS_POKE_EQUIP_USER uPos )
{
  const BOOL isMainPos = MUS_ITEM_DRAW_CheckMainPosUserDataItemNo(work->itemDrawSys , itemState->itemId , uPos );
  const BOOL isTrgPos = MUS_ITEM_DRAW_CanEquipPosUserDataItemNo(work->itemDrawSys , itemState->itemId , uPos );
  return ( isTrgPos == TRUE  ? 3000 : 0 ) +
         ( itemState->isNew  ? 2000 : 0 ) +
         ( (isMainPos==TRUE && itemState->isNew==FALSE) ? 1000 : 0 ) +
         ( 999 - itemState->itemId );

  //表示順
  // New + Trg(+Main)
  // Trg + Main
  // Trg
  // New
  // Normal
}

#pragma mark [> StateChangeAnime
//--------------------------------------------------------------
//ステート変更のアニメ
//--------------------------------------------------------------
static void DUP_FIT_ChangeStateAnime( FITTING_WORK *work )
{
  FIT_ITEM_WORK *item;
  //リストのアイテム
  DUP_FIT_CalcItemListAngle( work , work->listAngle , 0 , LIST_SIZE_X+work->animeCnt , LIST_SIZE_Y+work->animeCnt*LIST_SIZE_RATIO );
  
  //フィールドのアイテム
  item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupField );
  while( item != NULL )
  {
    VecFx32 pos;
    GFL_POINT* dispPos = DUP_FIT_ITEM_GetPosition( item );
    MUS_ITEM_DRAW_WORK *drawWork = DUP_FIT_ITEM_GetItemDrawWork( item );
    
    //中心からの角度を求める
    const s16 subX = dispPos->x - LIST_CENTER_X;
    const s16 subY = dispPos->y - LIST_CENTER_Y;
    const fx32 len = FX_Sqrt( FX32_CONST( subX*subX+subY*subY ));
    const u16 angle = FX_Atan2Idx(FX32_CONST(subY),FX32_CONST(subX));
    
    MUS_ITEM_DRAW_GetPosition( work->itemDrawSys , drawWork , &pos );

    pos.x = (FX_Mul(FX_CosIdx( angle ),( len + FX32_CONST(work->animeCnt) ))          + LIST_CENTER_X_FX);
    pos.y = (FX_Mul(FX_SinIdx( angle ),( len + FX32_CONST(work->animeCnt*LIST_SIZE_RATIO ) )) + LIST_CENTER_Y_FX);
    //Zはそのまま
    MUS_ITEM_DRAW_SetPosition( work->itemDrawSys , drawWork , &pos );
    
    item = DUP_FIT_ITEM_GetNextItem(item);
  }
  
  //BG
  GFL_BG_SetScroll( FIT_FRAME_MAIN_MIRROR , GFL_BG_SCROLL_Y_SET , -work->animeCnt );

  //OBJ(ボタン)
  {
    GFL_CLACTPOS cellPos;
    cellPos.x = BUTTON_ASSEPT_POS_X;
    cellPos.y = BUTTON_ASSEPT_POS_Y+( FIT_ANIME_MAX-work->animeCnt < BUTTON_SIZE_Y ?
              FIT_ANIME_MAX-work->animeCnt : BUTTON_SIZE_Y );
    GFL_CLACT_WK_SetPos( work->buttonCell[0] , &cellPos , CLSYS_DEFREND_MAIN );

    cellPos.x = BUTTON_RETURN_POS_X;
    cellPos.y = BUTTON_RETURN_POS_Y+( FIT_ANIME_MAX-work->animeCnt < BUTTON_SIZE_Y ?
              FIT_ANIME_MAX-work->animeCnt : BUTTON_SIZE_Y );
    GFL_CLACT_WK_SetPos( work->buttonCell[1] , &cellPos , CLSYS_DEFREND_MAIN );
  }

}

#pragma mark [> CheckFunc
//--------------------------------------------------------------
//チェック画面
//--------------------------------------------------------------

static FITTING_RETURN DUP_CHECK_CheckMain(  FITTING_WORK *work )
{
  GFL_UI_TP_HITTBL buttonHitTbl[3] =
  {
    {
      BUTTON_ASSEPT_POS_Y - BUTTON_SIZE_Y/2 ,
      BUTTON_ASSEPT_POS_Y + BUTTON_SIZE_Y/2 ,
      BUTTON_ASSEPT_POS_X - BUTTON_SIZE_X/2 ,
      BUTTON_ASSEPT_POS_X + BUTTON_SIZE_X/2 ,
    },{
      BUTTON_RETURN_POS_Y - BUTTON_SIZE_Y/2 ,
      BUTTON_RETURN_POS_Y + BUTTON_SIZE_Y/2 ,
      BUTTON_RETURN_POS_X - BUTTON_SIZE_X/2 ,
      BUTTON_RETURN_POS_X + BUTTON_SIZE_X/2 ,
    },{
      GFL_UI_TP_HIT_END,0,0,0
    }
  };
  int hitRet;
  
  hitRet = GFL_UI_TP_HitTrg( buttonHitTbl );
  switch( hitRet )
  {
  case 0: //決定ボタン
    DUP_CHECK_SaveNowEquip( work );
    PMSND_PlaySE( DUP_SE_KIRAKIRA );
    return FIT_RET_GO_END;
    break;
  case 1: //戻るボタン
    work->state = DUS_RETURN_FITTING;
    work->animeCnt = FIT_ANIME_MAX;
    PMSND_PlaySE( DUP_SE_CANCEL );
    {
      DUP_CHECK_ResetItemAngle( work );
    }
    {
      WORDSET *wordSet = WORDSET_Create( work->heapId );
      WORDSET_RegisterPokeNickName( wordSet , 0 , work->initWork->musPoke->pokePara );
      DUP_FIT_DrawMessage( work , DUP_MSG_01 , wordSet );
      WORDSET_Delete( wordSet );
    }
    break;
  case GFL_UI_TP_HIT_NONE:
    DUP_CHECK_UpdateTpMain( work );
    break;
  }
  
  DUP_FIT_UpdateShadow( work );
  return FIT_RET_CONTINUE;
}

static void DUP_CHECK_ResetItemAngle( FITTING_WORK *work )
{
  //角度を戻す
  u8 i;
  FIT_ITEM_WORK *item;
  for( i=0;i<MUS_POKE_EQUIP_MAX;i++ )
  {
    work->initWork->musPoke->equip[i].angle = 0;
  }
  item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupEquip );
  while( item != NULL )
  {
    u16 equipPos = DUP_FIT_ITEM_GetEquipPos( item );
    MUS_POKE_EQUIP_DATA *equipData = MUS_POKE_DRAW_GetEquipData( work->drawWork , equipPos );
    MUS_ITEM_DRAW_SetRotation( work->itemDrawSys , DUP_FIT_ITEM_GetItemDrawWork(item) , equipData->itemRot );
    item = DUP_FIT_ITEM_GetNextItem(item);
  }
}

static void DUP_CHECK_UpdateTpMain( FITTING_WORK *work )
{
  work->befTpx = work->tpx;
  work->befTpy = work->tpy;
  work->tpIsTrg = GFL_UI_TP_GetTrg();
  work->tpIsTouch = GFL_UI_TP_GetPointCont( &work->tpx,&work->tpy );
  
  if( work->tpIsTrg == TRUE )
  {
    //装備品との当たり判定
    FIT_ITEM_WORK *item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupEquip );
    while( item != NULL )
    {
      if( DUP_FIT_ITEM_CheckHit( item , work->tpx,work->tpy ) == TRUE )
      {
        work->holdItem = item;
        DUP_TPrintf("Equip Item Hold[%d]\n",DUP_FIT_ITEM_GetItemState(item)->itemId);
      }
      item = DUP_FIT_ITEM_GetNextItem(item);
    }
  }
  
  if( work->tpIsTouch == TRUE && work->holdItem != NULL )
  {
    DUP_CHECK_UpdateTpHoldingItem( work );
    
  }
  if( work->tpIsTouch == FALSE )
  {
    work->befAngleEnable = FALSE;
    work->holdItem = NULL;
  }
}

static void DUP_CHECK_UpdateTpHoldingItem( FITTING_WORK *work )
{
  fx32 subX;
  fx32 subY;
  u16 tpAngle;

  GFL_POINT *itemPos = DUP_FIT_ITEM_GetPosition( work->holdItem );
  subX = FX32_CONST((s32)work->tpx - itemPos->x);
  subY = FX32_CONST((s32)work->tpy - itemPos->y);
  tpAngle = FX_Atan2Idx(subX,subY);
  
  if( work->befAngleEnable == TRUE )
  {
    u16 rot;
    s32 subAngle = work->befAngle - tpAngle;
    u16 equipPos = DUP_FIT_ITEM_GetEquipPos( work->holdItem );
    s16 angle = work->initWork->musPoke->equip[equipPos].angle;
    MUS_POKE_EQUIP_DATA *equipData = MUS_POKE_DRAW_GetEquipData( work->drawWork , equipPos );
    
    angle += subAngle;
    
    if( angle < -MUS_POKE_EQUIP_ANGLE_MAX )
    {
      angle = -MUS_POKE_EQUIP_ANGLE_MAX;
    }
    if( angle >  MUS_POKE_EQUIP_ANGLE_MAX )
    {
      angle =  MUS_POKE_EQUIP_ANGLE_MAX;
    }
    
    work->adjustMoveValue += work->initWork->musPoke->equip[equipPos].angle - angle;
    if( MATH_ABS( work->adjustMoveValue ) > DUP_POKE_ADJUST_SE_VALUE )
    {
      work->adjustMoveValue = 0;
      PMSND_PlaySE( DUP_SE_POKE_ADJUST );
    }
      
    rot = equipData->itemRot+angle;
    MUS_ITEM_DRAW_SetRotation( work->itemDrawSys , DUP_FIT_ITEM_GetItemDrawWork(work->holdItem) , rot );
    
    work->initWork->musPoke->equip[equipPos].angle = angle;
  }
  work->befAngle = tpAngle;
  work->befAngleEnable = TRUE;
  
}

static void DUP_CHECK_SaveNowEquip( FITTING_WORK *work )
{
  
  MUSICAL_EQUIP_SAVE *mus_bef_save = MUSICAL_SAVE_GetBefEquipData( work->initWork->mus_save );
  FIT_ITEM_WORK *item;
  u8 i;
  u8 save_pos = 0;
  
  MUSICAL_SAVE_ResetBefEquip(work->initWork->mus_save);
  
  item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupEquip );
  while( item != NULL && save_pos < MUSICAL_ITEM_EQUIP_MAX )
  {
    u16 equip_pos = DUP_FIT_ITEM_GetEquipPos( item );
    MUS_POKE_EQUIP_DATA *equip_data = MUS_POKE_DRAW_GetEquipData( work->drawWork , equip_pos );
    
    GF_ASSERT_MSG( save_pos < MUSICAL_ITEM_EQUIP_MAX , "装備品が多すぎ！\n" );

    mus_bef_save->equipData[save_pos].pos = equip_pos;
    mus_bef_save->equipData[save_pos].itemNo = DUP_FIT_ITEM_GetItemState( item )->itemId;
    mus_bef_save->equipData[save_pos].angle = work->initWork->musPoke->equip[equip_pos].angle;
    work->initWork->musPoke->equip[equip_pos].itemNo = DUP_FIT_ITEM_GetItemState( item )->itemId;
    work->initWork->musPoke->equip[equip_pos].priority = save_pos;
    
    item = DUP_FIT_ITEM_GetNextItem(item);

    save_pos++;
  }
  
}

#pragma mark [> DemoFunc
//--------------------------------------------------------------
//デモ画面
//--------------------------------------------------------------
static void DUP_DEMO_DemoStart( FITTING_WORK *work )
{
  u16 idx;
  work->isDemo = TRUE;
  work->demoCnt = 0;
  work->demoPhase = 1;
  
  //移動量の設定
  //初期値の位置から取られていないアイテムを探す
  idx = DEMO_ROT_ITEM_NUM;
  while( TRUE )
  {
    if( work->itemState[idx]->isOutList == TRUE )
    {
      idx++;
      if( idx == work->totalItemNum )
      {
        idx = 0;
      }
      if( idx == DEMO_ROT_ITEM_NUM )
      {
        //1週した・・・一応動くように
        GF_ASSERT_MSG(0,"Demo:Item is out of list all !!\n");
        break;
      }
    }
    else
    {
      break;
    }
  }

  if( idx == 0 )
  {
    work->demoMoveValue = work->totalItemNum-1;
  }
  else
  {
    work->demoMoveValue = idx-1;
  }
  work->state = DUS_FITTING_DEMO;
}

static void DUP_DEMO_DemoEnd( FITTING_WORK *work )
{
  work->isDemo = FALSE;
  work->state = DUS_FITTING_MAIN;
  GFL_CLACT_WK_SetDrawEnable( work->demoPen, FALSE );
}

static void DUP_DEMO_DemoMain( FITTING_WORK *work )
{
  switch( work->demoPhase )
  {
  case 1:
    //タッチペンが右から下へリストを回す
    {
      const s32 angle = work->demoMoveValue*LIST_ONE_ANGLE;
      DUP_DEMO_DemoPhaseListRot( work , angle , angle , work->demoMoveValue*15 );
    }
    break;
  case 2:
    DUP_DEMO_DemoPhaseWait( work , 30 );
    break;
  case 3:
    //タッチペンが左から下へリストを回す
    {
      const s32 angle = work->demoMoveValue*LIST_ONE_ANGLE;
      DUP_DEMO_DemoPhaseListRot( work , -(u16)angle , -angle , work->demoMoveValue*15 );
    }
    break;
  case 4:
    DUP_DEMO_DemoPhaseWait( work , 30 );
    break;
  case 5:
    {
      const GFL_POINT start ={ 162,120 };
      const GFL_POINT end   ={ 208,160 };
      DUP_DEMO_DemoPhaseDragPen( work , &start , &end , 60 );
    }
    break;
  case 6:
    DUP_DEMO_DemoPhaseWait( work , 30 );
    break;
  case 7:
    {
      const GFL_POINT start ={ 208,160 };
      const GFL_POINT end   ={ 162,120 };
      DUP_DEMO_DemoPhaseDragPen( work , &start , &end , 60 );
    }
    break;
  case 8:
    DUP_DEMO_DemoPhaseWait( work , 30 );
    break;
  case 9:
    DUP_DEMO_DemoEnd( work );
    return;
    break;
  }
  
//  if( work->tpIsTouch == TRUE )
  {
    GFL_CLACTPOS cellPos;
    cellPos.x = work->tpx;
    cellPos.y = work->tpy;
    GFL_CLACT_WK_SetPos( work->demoPen , &cellPos , CLSYS_DEFREND_MAIN );
    //GFL_CLACT_WK_SetDrawEnable( work->demoPen, TRUE );
  }
//  else
  {
    //GFL_CLACT_WK_SetDrawEnable( work->demoPen, FALSE );
  }
  
  DUP_FIT_FittingMain( work );
  
#if DEB_ARI
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
  {
    DUP_DEMO_DemoEnd( work );
  }
#endif //DEB_ARI
}

//タッチペンがリストを回す
static void DUP_DEMO_DemoPhaseListRot( FITTING_WORK *work , const u16 start , const s32 value , const u16 cnt)
{
  work->tpIsTouch = FALSE;
  work->tpIsTrg = FALSE;
  GFL_CLACT_WK_SetDrawEnable( work->demoPen, TRUE );
  //座標
  {
    const s32 oneAngle = value/cnt;
    const u16 Angle = (start-oneAngle*work->demoCnt + 0x10000)%0x10000;
    const fx32 sin = (fx32)FX_SinIdx( Angle );
    const fx32 cos = (fx32)FX_CosIdx( Angle );
    const fx32 posX = LIST_CENTER_X_FX+sin*LIST_SIZE_X;
    const fx32 posY = LIST_CENTER_Y_FX+cos*LIST_SIZE_Y;
    work->tpx = F32_CONST( posX );
    work->tpy = F32_CONST( posY );
    
    work->listAngle -= oneAngle;
    DUP_FIT_CalcItemListAngle( work , work->listAngle , oneAngle , LIST_SIZE_X , LIST_SIZE_Y );
  }

  work->demoCnt++;
  if( work->demoCnt > cnt )
  {
    work->demoCnt = 0;
    work->demoPhase++;
  }

}

//ウェイト
static void DUP_DEMO_DemoPhaseWait( FITTING_WORK *work , const u16 cnt )
{
  work->tpIsTouch = FALSE;
  work->tpIsTrg = FALSE;
  work->demoCnt++;
  GFL_CLACT_WK_SetDrawEnable( work->demoPen, FALSE );
  if( work->demoCnt > cnt )
  {
    work->demoCnt = 0;
    work->demoPhase++;
  }
}

//タッチペンの移動
static void DUP_DEMO_DemoPhaseDragPen( FITTING_WORK *work , const GFL_POINT *start , const GFL_POINT *end , const u16 cnt )
{
  GFL_POINT pos;
  
  work->tpIsTouch = TRUE;
  if( work->demoCnt == 0 )
  {
    work->tpIsTrg = TRUE;
  }
  else
  {
    work->tpIsTrg = FALSE;
  }
  pos.x = start->x+(end->x-start->x)*work->demoCnt/cnt;
  pos.y = start->y+(end->y-start->y)*work->demoCnt/cnt;

  work->tpx = pos.x;
  work->tpy = pos.y;

  GFL_CLACT_WK_SetDrawEnable( work->demoPen, TRUE );
  work->demoCnt++;
  if( work->demoCnt > cnt )
  {
    work->demoCnt = 0;
    work->demoPhase++;
  }
  
}

#pragma mark [> MessageFunc
//--------------------------------------------------------------
//メッセージ系
//--------------------------------------------------------------

static void DUP_FIT_InitMessage( FITTING_WORK *work )
{
  work->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );
  
  //メッセージ
  work->msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_dress_up_dat , work->heapId );

  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_SUB_BG , FIT_PLT_BG_SUB_FONT*16*2, 16*2, work->heapId );
  
  work->printQue = PRINTSYS_QUE_Create( work->heapId );

  BmpWinFrame_GraphicSet( FIT_FRAME_SUB_BG , FIT_MSGWIN_CHAR_TOP , FIT_PLT_BG_SUB_MSGWIN ,
                          0 , work->heapId );
}

static void DUP_FIT_TermMessage( FITTING_WORK *work )
{
  PRINTSYS_QUE_Delete( work->printQue );
  GFL_MSG_Delete( work->msgHandle );
  GFL_FONT_Delete( work->fontHandle );
}

static void DUP_FIT_DrawMessage( FITTING_WORK *work , u16 msgId , WORDSET *wordSet )
{
  STRBUF *srcStr = GFL_MSG_CreateString( work->msgHandle , msgId ); 

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->msgWin) , 0xf );
  
  if( wordSet != NULL )
  {
    STRBUF *workStr  = GFL_STR_CreateBuffer( 128 , work->heapId );
    WORDSET_ExpandStr( wordSet , workStr , srcStr );
    
    GFL_STR_DeleteBuffer( srcStr );
    srcStr = workStr;
  }

  PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( work->msgWin ) , 
          2 , 2 , srcStr , work->fontHandle , PRINTSYS_LSB_Make(1,2,0) );
  GFL_STR_DeleteBuffer( srcStr );
  work->isUpdateMsg = TRUE;
}


#pragma mark [> EffectFunc
//演出エフェクト系

static void DUP_EFFECT_InitCell( FITTING_WORK *work )
{
  u8 i;
  
  //下用
  for( i=0;i<EFFECT_UP_NUM;i++ )
  {
    //セルの生成
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = 0;
    cellInitData.pos_y = 0;
    cellInitData.anmseq = 0;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;
    work->clwkEffectUp[i] = GFL_CLACT_WK_Create( work->cellUnit ,
              work->objResIdx[DOR_EFFECT_UP_PLT],
              work->objResIdx[DOR_EFFECT_UP_NCG],
              work->objResIdx[DOR_EFFECT_UP_ANM],
              &cellInitData ,CLSYS_DEFREND_SUB , work->heapId );
    GFL_CLACT_WK_SetDrawEnable( work->clwkEffectUp[i], TRUE );
    GFL_CLACT_WK_SetAutoAnmFlag( work->clwkEffectUp[i], TRUE );
    GFL_CLACT_WK_SetObjMode( work->clwkEffectUp[i], GX_OAM_MODE_NORMAL );
    work->effUpCnt[i] = 0;
  }

  //上用
  {
    //セルの生成
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = 128;
    cellInitData.pos_y = 96;
    cellInitData.anmseq = 0;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;
    work->clwkEffectDown = GFL_CLACT_WK_Create( work->cellUnit ,
              work->objResIdx[DOR_EFFECT_DOWN_PLT],
              work->objResIdx[DOR_EFFECT_DOWN_NCG],
              work->objResIdx[DOR_EFFECT_DOWN_ANM],
              &cellInitData ,CLSYS_DEFREND_MAIN , work->heapId );
    GFL_CLACT_WK_SetDrawEnable( work->clwkEffectDown, FALSE );
    GFL_CLACT_WK_SetAutoAnmFlag( work->clwkEffectDown, TRUE );
  }
}
static void DUP_EFFECT_TermCell( FITTING_WORK *work )
{
  u8 i;
  for( i=0;i<EFFECT_UP_NUM;i++ )
  {
    GFL_CLACT_WK_Remove( work->clwkEffectUp[i] );
  }
  GFL_CLACT_WK_Remove( work->clwkEffectDown );
}
static void DUP_EFFECT_UpdateCell( FITTING_WORK *work )
{
  u8 i;
  for( i=0;i<EFFECT_UP_NUM;i++ )
  {
    //

    GFL_CLACT_WK_SetBgPri( work->clwkEffectUp[i] , GFL_BG_GetPriority(FIT_FRAME_SUB_TOP) );
    GFL_CLACT_WK_SetObjMode( work->clwkEffectUp[i], GX_OAM_MODE_NORMAL );
    if( work->effUpCnt[i] == 0 )
    {
      GFL_CLACTPOS pos;
      pos.x = GFL_STD_MtRand( EFFECT_UP_LIMIT_X );
      if( GFL_STD_MtRand( 2 ) == 1 )
      {
        pos.x = 256-pos.x;
      }
      pos.y = GFL_STD_MtRand( 192 );
      GFL_CLACT_WK_SetPos( work->clwkEffectUp[i] , &pos , CLSYS_DEFREND_SUB );
      
      if( pos.y > EFFECT_UP_LIMIT_Y )
      {
        GFL_CLACT_WK_SetAnmSeq( work->clwkEffectUp[i] , GFL_STD_MtRand( 2 ) );
      }
      else
      {
        GFL_CLACT_WK_SetAnmSeq( work->clwkEffectUp[i] , GFL_STD_MtRand( 2 )+2 );
      }
      
      work->effUpCnt[i] = 400 + GFL_STD_MtRand( 200 );
    }
    else
    {
      if( (GFL_UI_KEY_GetCont() & PAD_BUTTON_Y) == 0 )
      {
        work->effUpCnt[i]--;
      }
    }
  }
  
}
