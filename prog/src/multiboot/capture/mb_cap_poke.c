//======================================================================
/**
 * @file	  mb_cap_poke.c
 * @brief	  捕獲ゲーム・ポケモン
 * @author	ariizumi
 * @data	  09/11/24
 *
 * モジュール名：MB_CAP_POKE
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "poke_tool/monsno_def.h"
#include "poke_tool/poke_personal.h"

#include "mb_capture_gra.naix"

#include "multiboot/mb_poke_icon.h"

#include "./mb_cap_local_def.h"
#include "./mb_cap_poke.h"
#include "./mb_cap_obj.h"
#include "./mb_cap_effect.h"

#include "./mb_cap_snd_def.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define MB_CAP_POKE_OFS_Y (8)
#define MB_CAP_POKE_MOVE_OFS_Y (4)

#define MB_CAP_POKE_HIDE_LOOK_OFS (5)
#define MB_CAP_POKE_HIDE_LOOK_OFS_SMALL (8)

#define MB_CAP_POKE_ANIM_SPEED (8)
#define MB_CAP_POKE_ANIM_FRAME (2)

#define MB_CAP_POKE_FALL_SPEED (FX32_HALF)
#define MB_CAP_POKE_DOWN_EFF_OFS (FX32_CONST(16))

#define MB_CAP_POKE_ESCAPE_TIME (60*7)   //画面外に逃げている時間
#define MB_CAP_POKE_ESCAPE_TIME_LAST (60*1)   //画面外に逃げている時間(最後の一匹

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef void (*MB_CAP_POKE_STATE_FUNC)(MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork );

struct _MB_CAP_POKE
{
  MB_CAP_POKE_STATE state;
  MB_CAP_POKE_STATE_FUNC stateFunc;
  u16 cnt;  //各ステート内で使いまわせるカウンタ

  u16 anmFrame;
  u16 anmIdx;
  u16 actSpeed; //基本速度を20として速度微調整用
  BOOL isRun;   //Run状態で移動中か？
  
  s8 posXidx;
  s8 posYidx;
  MB_CAP_POKE_DIR dir;
  VecFx32 startPos;
  VecFx32 pos;
  fx32 height;
  
  int resIdx;
  int resEdgeIdx;
  int objIdx;
  int objShadowIdx;
  int objEdgeIdx;
  
  MB_CAP_EFFECT *downEff;
  
  BOOL isGround;  //ディグダ
  BOOL isNoFlip;  //反転不可
  BOOL isSmall;   //小さいポケ(個別定義
  BOOL isHard;    //伝説・Lv50以上

  u8 befHitNo;    //連続ヒットを防ぐ処理
};


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void MB_CAP_POKE_SetEscape( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork );

static void MB_CAP_POKE_StateNone(MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork );
static void MB_CAP_POKE_StateHide(MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork );
static void MB_CAP_POKE_StateRun(MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork );
static void MB_CAP_POKE_StateDown(MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork );

static void MB_CAP_POKE_CheckActSpeed( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork );

//--------------------------------------------------------------
//	オブジェクト作成
//--------------------------------------------------------------
MB_CAP_POKE* MB_CAP_POKE_CreateObject( MB_CAPTURE_WORK *capWork , MB_CAP_POKE_INIT_WORK *initWork )
{
  const HEAPID heapId = MB_CAPTURE_GetHeapId( capWork );
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  ARCHANDLE *arcHandle = MB_CAPTURE_GetArcHandle( capWork );
  const DLPLAY_CARD_TYPE cardType = MB_CAPTURE_GetCardType( capWork );
  const int resShadowIdx = MB_CAPTURE_GetBbdResIdx( capWork , MCBR_SHADOW );

  MB_CAP_POKE *pokeWork = GFL_HEAP_AllocClearMemory( heapId , sizeof( MB_CAP_POKE ) );

  GFL_G3D_RES* res = GFL_G3D_CreateResourceHandle( arcHandle , NARC_mb_capture_gra_cap_poke_dummy_nsbtx );
  const BOOL flg = TRUE;
  pokeWork->pos.x = 0;
  pokeWork->pos.y = 0;
  pokeWork->pos.z = 0;
  GFL_G3D_TransVramTexture( res );
  pokeWork->resIdx = GFL_BBD_AddResource( bbdSys , 
                                       res , 
                                       GFL_BBD_TEXFMT_PAL16 ,
                                       GFL_BBD_TEXSIZ_32x64 ,
                                       32 , 32 );
  GFL_BBD_CutResourceData( bbdSys , pokeWork->resIdx );
  
  //文字演出のためサイズを2倍にしてるのでデフォは半分
  pokeWork->objIdx = GFL_BBD_AddObject( bbdSys , 
                                     pokeWork->resIdx ,
                                     FX32_HALF , 
                                     FX32_HALF , 
                                     &pokeWork->pos ,
                                     31 ,
                                     GFL_BBD_LIGHT_NONE );
  pokeWork->objShadowIdx = GFL_BBD_AddObject( bbdSys , 
                                     resShadowIdx ,
                                     FX32_HALF , 
                                     FX32_HALF , 
                                     &pokeWork->pos ,
                                     31 ,
                                     GFL_BBD_LIGHT_NONE );
  //3D設定の関係で反転させる・・・
  GFL_BBD_SetObjectFlipT( bbdSys , pokeWork->objIdx , &flg );

  //ふち
#if MB_CAPTURE_USE_EDGE
  {
    NNSGfdTexKey texKey;
    GFL_BBD_GetResourceTexKey( bbdSys , pokeWork->resIdx , &texKey );
    pokeWork->resEdgeIdx = GFL_BBD_AddResourceKey(  bbdSys , 
                                       texKey , 
                                       initWork->edgePlttKey ,
                                       GFL_BBD_TEXFMT_PAL16 ,
                                       GFL_BBD_TEXSIZDEF_32x64 ,
                                       32 , 32 );
    pokeWork->objEdgeIdx = GFL_BBD_AddObject( bbdSys , 
                                       pokeWork->resEdgeIdx ,
                                       FX32_CONST(0.6f) , 
                                       FX32_CONST(0.6f) , 
                                       &pokeWork->pos ,
                                       31 ,
                                       GFL_BBD_LIGHT_NONE );
    GFL_BBD_SetObjectFlipT( bbdSys , pokeWork->objEdgeIdx , &flg );
  }
#endif //MB_CAPTURE_USE_EDGE

  //アイコンデータをVRAMへ送る
  {
    const u32 pltResNo = MB_ICON_GetPltResId( cardType );
    const u32 ncgResNo = MB_ICON_GetCharResId( initWork->ppp , cardType );
    const u8  palNo = MB_ICON_GetPalNumber( initWork->ppp );
    NNSG2dPaletteData *pltData;
    NNSG2dCharacterData *charData;
    void *pltRes = GFL_ARCHDL_UTIL_LoadPalette( initWork->pokeArcHandle , pltResNo , &pltData , GFL_HEAP_LOWID( heapId ) );
    void *ncgRes = GFL_ARCHDL_UTIL_LoadOBJCharacter( initWork->pokeArcHandle , ncgResNo , FALSE , &charData , GFL_HEAP_LOWID( heapId ) );
    u32 texAdr,pltAdr,pltDataAdr;
    GFL_BMP_DATA *bmpData = GFL_BMP_CreateWithData( charData->pRawData , 4 , 8 , GFL_BMP_16_COLOR , GFL_HEAP_LOWID( heapId ) );
    
    GFL_BMP_DataConv_to_BMPformat( bmpData , FALSE , GFL_HEAP_LOWID( heapId ) );
    
    GFL_BBD_GetResourceTexPlttAdrs( bbdSys , pokeWork->resIdx , &pltAdr );
    GFL_BBD_GetResourceTexDataAdrs( bbdSys , pokeWork->resIdx , &texAdr );
    
    GX_BeginLoadTex();
    DC_FlushRange( charData->pRawData , 32*64*0x20 );
    GX_LoadTex( charData->pRawData , texAdr , 32*64*0x20 );
    GX_EndLoadTex();

    pltDataAdr = (u32)pltData->pRawData + palNo*32;
    GX_BeginLoadTexPltt();
    DC_FlushRange( (void*)pltDataAdr , 32 );
    GX_LoadTexPltt( (void*)pltDataAdr , pltAdr , 32 );
    GX_EndLoadTexPltt();
    
    GFL_HEAP_FreeMemory( pltRes );
    GFL_HEAP_FreeMemory( ncgRes );
    GFL_BMP_Delete( bmpData );
  }

  pokeWork->state = MCPS_NONE;
  pokeWork->stateFunc = MB_CAP_POKE_StateNone;
  pokeWork->anmFrame = 0;
  pokeWork->anmIdx = 0;
  pokeWork->actSpeed = MB_CAP_POKE_ACT_RATE;
  pokeWork->isRun = FALSE;
  
  pokeWork->posXidx = 0xFF;
  pokeWork->posYidx = 0xFF;
  pokeWork->height = 0;
  
  pokeWork->downEff = NULL;
  
  pokeWork->isGround = FALSE;
  pokeWork->isNoFlip = FALSE;
  pokeWork->isSmall = FALSE;
  pokeWork->isHard = FALSE;
  
  pokeWork->befHitNo = 0xFF;
  
  //ディグダ・反転チェック・小さいチェック・伝説チェック
  {
    const u32 monsno = PPP_Get( initWork->ppp , ID_PARA_monsno , NULL );
    const u32 formno = PPP_Get( initWork->ppp , ID_PARA_form_no , NULL );
    const u8  lv = PPP_CalcLevel( initWork->ppp );
    POKEMON_PERSONAL_DATA *personalData = POKE_PERSONAL_OpenHandle( monsno , formno ,heapId );

    //ディグダ
    if( POKE_PERSONAL_GetParam( personalData ,  POKEPER_ID_no_jump ) == TRUE )
    {
      pokeWork->isGround = TRUE;
      MB_TPrintf("Ground!\n");
    }

    //反転不可
    if( POKE_PERSONAL_GetParam( personalData , POKEPER_ID_reverse ) == TRUE )
    {
      pokeWork->isNoFlip = TRUE;
      MB_TPrintf("NoFlip!\n");
    }
    POKE_PERSONAL_CloseHandle( personalData );
    
    //小さいポケ(個別定義
    if( monsno == MONSNO_TORANSERU  ||
        monsno == MONSNO_BIIDORU  ||
        monsno == MONSNO_KORATTA  ||
        monsno == MONSNO_NAZONOKUSA  ||
        monsno == MONSNO_DHIGUDA  ||
        monsno == MONSNO_KOIRU  ||
        monsno == MONSNO_KABUTO  ||
        monsno == MONSNO_ANNOON ||
        monsno == MONSNO_TEPPOUO ||
        monsno == MONSNO_ATYAMO ||
        monsno == MONSNO_KINOKOKO ||
        monsno == MONSNO_NOZUPASU ||
        monsno == MONSNO_KOKODORA ||
        monsno == MONSNO_GOKURIN ||
        monsno == MONSNO_DOZYOTTI ||
        monsno == MONSNO_POWARUN ||
        monsno == MONSNO_RABUKASU ||
        monsno == MONSNO_MINOMUTTI ||
        monsno == MONSNO_MANENE ||
        monsno == MONSNO_TAMANTA )
    {
      pokeWork->isSmall = TRUE;
      MB_TPrintf("Small!\n");
    }
    
    //伝説・Lv50以上
    if( lv >= 50 )
    {
      pokeWork->isHard = TRUE;
      MB_TPrintf("Lv over 50 !!!\n");
    }
    else
    if( monsno == MONSNO_HURIIZAA   || monsno == MONSNO_SANDAA ||
        monsno == MONSNO_FAIYAA     || monsno == MONSNO_MYUUTUU ||
        monsno == MONSNO_MYUU       || monsno == MONSNO_RAIKOU ||
        monsno == MONSNO_ENTEI      || monsno == MONSNO_SUIKUN ||
        monsno == MONSNO_RUGIA      || monsno == MONSNO_HOUOU ||
        monsno == MONSNO_SEREBHI    || monsno == MONSNO_REZIROKKU ||
        monsno == MONSNO_REZIAISU   || monsno == MONSNO_REZISUTIRU ||
        monsno == MONSNO_RATHIASU   || monsno == MONSNO_RATHIOSU ||
        monsno == MONSNO_KAIOOGA    || monsno == MONSNO_GURAADON ||
        monsno == MONSNO_REKKUUZA   || monsno == MONSNO_ZIRAATI ||
        monsno == MONSNO_DEOKISISU  || monsno == MONSNO_ROTOMU ||
        monsno == MONSNO_YUKUSII    || monsno == MONSNO_EMURITTO ||
        monsno == MONSNO_AGUNOMU    || monsno == MONSNO_DHIARUGA ||
        monsno == MONSNO_PARUKIA    || monsno == MONSNO_HIIDORAN ||
        monsno == MONSNO_REZIGIGASU || monsno == MONSNO_GIRATHINA ||
        monsno == MONSNO_KURESERIA  || monsno == MONSNO_FIONE ||
        monsno == MONSNO_MANAFI     || monsno == MONSNO_DAAKURAI ||
        monsno == MONSNO_SHEIMI     || monsno == MONSNO_ARUSEUSU )
    {
      pokeWork->isHard = TRUE;
      MB_TPrintf("Legend!!!\n");
    }
  }
  

  return pokeWork;
}

//--------------------------------------------------------------
//	オブジェクト開放
//--------------------------------------------------------------
void MB_CAP_POKE_DeleteObject( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );

#if MB_CAPTURE_USE_EDGE
  GFL_BBD_RemoveObject( bbdSys , pokeWork->objEdgeIdx );
  GFL_BBD_RemoveResource( bbdSys , pokeWork->resEdgeIdx );
#endif //MB_CAPTURE_USE_EDGE
  GFL_BBD_RemoveObject( bbdSys , pokeWork->objShadowIdx );
  GFL_BBD_RemoveObject( bbdSys , pokeWork->objIdx );
  GFL_BBD_RemoveResource( bbdSys , pokeWork->resIdx );

  GFL_HEAP_FreeMemory( pokeWork );
}


//--------------------------------------------------------------
//	オブジェクト更新
//--------------------------------------------------------------
void MB_CAP_POKE_UpdateObject( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  pokeWork->stateFunc( capWork , pokeWork);
  
  //アニメの更新
  pokeWork->anmFrame++;
  if( pokeWork->anmFrame >= MB_CAP_POKE_ANIM_SPEED )
  {
    pokeWork->anmFrame = 0;
    pokeWork->anmIdx++;
    if( pokeWork->anmIdx >= MB_CAP_POKE_ANIM_FRAME )
    {
      pokeWork->anmIdx = 0;
    }
    GFL_BBD_SetObjectCelIdx( bbdSys , pokeWork->objIdx , &pokeWork->anmIdx );
  }
  
  //枠の追従
#if MB_CAPTURE_USE_EDGE
  {
    VecFx32 pos;
    BOOL isDisp = GFL_BBD_GetObjectDrawEnable( bbdSys , pokeWork->objIdx );
    BOOL isFlip = GFL_BBD_GetObjectFlipS( bbdSys , pokeWork->objIdx );
    GFL_BBD_SetObjectDrawEnable( bbdSys , pokeWork->objEdgeIdx , &isDisp );
    GFL_BBD_SetObjectFlipS( bbdSys , pokeWork->objEdgeIdx , &isFlip );

    GFL_BBD_GetObjectTrans( bbdSys , pokeWork->objIdx , &pos );
    //pos.z -= FX32_CONST(0.1f);
    GFL_BBD_SetObjectTrans( bbdSys , pokeWork->objEdgeIdx , &pos );
    GFL_BBD_SetObjectCelIdx( bbdSys , pokeWork->objEdgeIdx , &pokeWork->anmIdx );
  }
#endif //MB_CAPTURE_USE_EDGE
  
}

//--------------------------------------------------------------
//	ポケモンセット(開始時隠れ用
//--------------------------------------------------------------
void MB_CAP_POKE_SetHide( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork , const u8 idxX , const u8 idxY )
{
  pokeWork->cnt = GFUser_GetPublicRand0(MB_CAP_POKE_HIDE_TOTAL_TIME);
  pokeWork->posXidx = idxX;
  pokeWork->posYidx = idxY;
  
  pokeWork->state = MCPS_HIDE;
  pokeWork->stateFunc = MB_CAP_POKE_StateHide;
}

//--------------------------------------------------------------
//	ポケモンセット(逃げる
//--------------------------------------------------------------
void MB_CAP_POKE_SetRun( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork )
{
  if( pokeWork->state == MCPS_LOOK )
  {
    if( pokeWork->cnt < MB_CAP_POKE_HIDE_LOOK_TIME )
    {
      pokeWork->dir = MCPD_LEFT;
    }
    else
    {
      pokeWork->dir = MCPD_RIGHT;
    }
    pokeWork->cnt = 0;
    pokeWork->stateFunc = MB_CAP_POKE_StateRun;
    //草からの基準位置
    MB_CAPTURE_GetGrassObjectPos( pokeWork->posXidx , pokeWork->posYidx , &pokeWork->startPos );
    pokeWork->startPos.y -= FX32_CONST( MB_CAP_POKE_MOVE_OFS_Y );
    pokeWork->startPos.z -= FX32_CONST( MB_CAP_OBJ_BASE_Z-MB_CAP_POKE_BASE_Z );
  }
  else
  if( pokeWork->state == MCPS_ESCAPE )
  {
    //画面外逃げた状態からの復帰
    pokeWork->cnt = 0;
    pokeWork->stateFunc = MB_CAP_POKE_StateRun;
    //草からの基準位置
    MB_CAPTURE_GetGrassObjectPos( pokeWork->posXidx , pokeWork->posYidx , &pokeWork->startPos );
    pokeWork->startPos.y -= FX32_CONST( MB_CAP_POKE_MOVE_OFS_Y );
    pokeWork->startPos.z -= FX32_CONST( MB_CAP_OBJ_BASE_Z-MB_CAP_POKE_BASE_Z );
  }
  else
  {
    //隠れている時はすぐ移動を始める
    pokeWork->cnt = 0;
  }
  pokeWork->isRun = TRUE;
  MB_CAP_POKE_CheckActSpeed( capWork , pokeWork );
}
//--------------------------------------------------------------
//	ポケモンセット(ゲーム開始時の即逃げ対応
//--------------------------------------------------------------
void MB_CAP_POKE_SetRunStart( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork )
{
  if( GFUser_GetPublicRand0(2) == 0 )
  {
    pokeWork->dir = MCPD_LEFT;
  }
  else
  {
    pokeWork->dir = MCPD_RIGHT;
  }
  pokeWork->cnt = 0;
  pokeWork->stateFunc = MB_CAP_POKE_StateRun;
  //草からの基準位置
  MB_CAPTURE_GetGrassObjectPos( pokeWork->posXidx , pokeWork->posYidx , &pokeWork->startPos );
  pokeWork->startPos.y -= FX32_CONST( MB_CAP_POKE_MOVE_OFS_Y );
  pokeWork->startPos.z -= FX32_CONST( MB_CAP_OBJ_BASE_Z-MB_CAP_POKE_BASE_Z );
  pokeWork->isRun = TRUE;
  MB_CAP_POKE_CheckActSpeed( capWork , pokeWork );
}

//--------------------------------------------------------------
//	ポケモンセット(逃げる・周囲にボールが落ちた場合
//--------------------------------------------------------------
void MB_CAP_POKE_SetRunChangeDir( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork , const MB_CAP_POKE_DIR dir )
{
  pokeWork->dir = dir;
  pokeWork->cnt = 0;
  pokeWork->stateFunc = MB_CAP_POKE_StateRun;
  pokeWork->isRun = TRUE;
  MB_CAP_POKE_CheckActSpeed( capWork , pokeWork );
}

//--------------------------------------------------------------
//	ポケモンセット(捕獲
//--------------------------------------------------------------
void MB_CAP_POKE_SetCapture( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  const BOOL isDisp = FALSE;
  GFL_BBD_SetObjectDrawEnable( bbdSys , pokeWork->objIdx , &isDisp );
  GFL_BBD_SetObjectDrawEnable( bbdSys , pokeWork->objShadowIdx , &isDisp );

  pokeWork->state = MCPS_CAPTURE;
  pokeWork->stateFunc = MB_CAP_POKE_StateNone;
  
  if( pokeWork->downEff != NULL )
  {
    MB_CAP_EFFECT_SetIsFinish( pokeWork->downEff , TRUE );
    pokeWork->downEff = NULL;
  }

}

//--------------------------------------------------------------
//	ポケモンセット(ダウン
//--------------------------------------------------------------
void MB_CAP_POKE_SetDown( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  pokeWork->state = MCPS_DOWN_MOVE;
  pokeWork->stateFunc = MB_CAP_POKE_StateDown;
}

//--------------------------------------------------------------
//	ポケモンセット(眠り・ダウンとステート共通
//--------------------------------------------------------------
void MB_CAP_POKE_SetSleep( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  if( pokeWork->state == MCPS_HIDE ||
      pokeWork->state == MCPS_LOOK )
  {
    VecFx32 effPos = pokeWork->pos;
    effPos.y -= MB_CAP_POKE_DOWN_EFF_OFS;
    effPos.z = FX32_CONST(MB_CAP_EFFECT_ZZZ_Z);
    pokeWork->downEff = MB_CAPTURE_CreateEffect( capWork , &effPos , MCET_ZZZ );
    MB_CAP_EFFECT_SetIsLoop( pokeWork->downEff , TRUE );
    pokeWork->state = MCPS_SLEEP_WAIT_GRASS;
  }
  else
  {
    pokeWork->state = MCPS_SLEEP_FALL;
  }
  pokeWork->cnt = 0;
  pokeWork->stateFunc = MB_CAP_POKE_StateDown;
}

void MB_CAP_POKE_SetDownToSleep( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork )
{
  //ダウン状態から眠りへ
  if( pokeWork->downEff != NULL )
  {
    MB_CAP_EFFECT_SetIsFinish( pokeWork->downEff , TRUE );
    pokeWork->downEff = NULL;
  }
  pokeWork->cnt = 0;
  pokeWork->state = MCPS_SLEEP_FALL;
}

//--------------------------------------------------------------
//	ポケモンセット(逃げた
//--------------------------------------------------------------
static void MB_CAP_POKE_SetEscape( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork )
{
  pokeWork->state = MCPS_ESCAPE;
  pokeWork->stateFunc = MB_CAP_POKE_StateNone;
  pokeWork->cnt = MB_CAP_POKE_ESCAPE_TIME;
}

#pragma mark [>state func
//--------------------------------------------------------------
//	ステート：無し・捕獲・逃げている
//--------------------------------------------------------------
static void MB_CAP_POKE_StateNone(MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork )
{
  if( pokeWork->state == MCPS_ESCAPE )
  {
    if( MB_CAPTURE_GetLeastPoke( capWork ) == 1 )
    {
      if( pokeWork->cnt > MB_CAP_POKE_ESCAPE_TIME_LAST )
      {
        pokeWork->cnt = MB_CAP_POKE_ESCAPE_TIME_LAST;
      }
    }
    if( pokeWork->cnt > 0 )
    {
      if( MB_CAPTURE_IsBonusTime( capWork ) == FALSE )
      {
        pokeWork->cnt--;
      }
    }
    else
    {
      if( GFUser_GetPublicRand0(2) == 0 )
      {
        pokeWork->posXidx = -1;
        pokeWork->dir = MCPD_RIGHT;
      }
      else
      {
        pokeWork->posXidx = MB_CAP_OBJ_X_NUM;
        pokeWork->dir = MCPD_LEFT;
      }
      pokeWork->posYidx = GFUser_GetPublicRand0(MB_CAP_OBJ_Y_NUM);
      MB_CAP_POKE_SetRun( capWork , pokeWork );
      MB_TPrintf("ReturnPoke!\n");
    }
  }
}

//--------------------------------------------------------------
//	ステート：隠れている
//--------------------------------------------------------------
static void MB_CAP_POKE_StateHide(MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  //ハードポケとの区別
  u16 lookTime;
  u16 hideTime;
  u16 totalTime;
  if( pokeWork->isHard == FALSE )
  {
    lookTime = MB_CAP_POKE_HIDE_LOOK_TIME;
    hideTime = MB_CAP_POKE_HIDE_HIDE_TIME;
    totalTime = MB_CAP_POKE_HIDE_TOTAL_TIME;
  }
  else
  {
    lookTime = MB_CAP_POKE_HIDE_LOOK_TIME_H;
    hideTime = MB_CAP_POKE_HIDE_HIDE_TIME_H;
    totalTime = MB_CAP_POKE_HIDE_TOTAL_TIME_H;
  }
  
  //ステートの変更を == にしているのは、初期設定値でいきなり出ないように
  pokeWork->cnt++;
  if( pokeWork->cnt >= totalTime )
  {
    //左に出る
    const BOOL flg = TRUE;
    const BOOL flipFlg = FALSE;
    const u16 ofs = (pokeWork->isSmall == TRUE?MB_CAP_POKE_HIDE_LOOK_OFS_SMALL:MB_CAP_POKE_HIDE_LOOK_OFS);
    MB_CAPTURE_GetGrassObjectPos( pokeWork->posXidx , pokeWork->posYidx , &pokeWork->pos );
    pokeWork->pos.x -= FX32_CONST( ofs );
    pokeWork->pos.y -= FX32_CONST( MB_CAP_POKE_OFS_Y );
    pokeWork->pos.z -= FX32_CONST( MB_CAP_OBJ_BASE_Z-MB_CAP_POKE_BASE_Z );
    GFL_BBD_SetObjectDrawEnable( bbdSys , pokeWork->objIdx , &flg );
    GFL_BBD_SetObjectTrans( bbdSys , pokeWork->objIdx , &pokeWork->pos );
    GFL_BBD_SetObjectFlipS( bbdSys , pokeWork->objIdx , &flipFlg );

    pokeWork->cnt = 0;
    pokeWork->state = MCPS_LOOK;
  }
  else
  if( pokeWork->cnt == lookTime+hideTime )
  {
    //右に出る
    const BOOL flg = TRUE;
    const BOOL flipFlg = ( pokeWork->isNoFlip == FALSE ? TRUE : FALSE );
    const u16 ofs = (pokeWork->isSmall == TRUE?MB_CAP_POKE_HIDE_LOOK_OFS_SMALL:MB_CAP_POKE_HIDE_LOOK_OFS);
    MB_CAPTURE_GetGrassObjectPos( pokeWork->posXidx , pokeWork->posYidx , &pokeWork->pos );
    pokeWork->pos.x += FX32_CONST( ofs );
    pokeWork->pos.y -= FX32_CONST( MB_CAP_POKE_OFS_Y );
    pokeWork->pos.z -= FX32_CONST( MB_CAP_OBJ_BASE_Z-MB_CAP_POKE_BASE_Z );
    GFL_BBD_SetObjectDrawEnable( bbdSys , pokeWork->objIdx , &flg );
    GFL_BBD_SetObjectTrans( bbdSys , pokeWork->objIdx , &pokeWork->pos );
    GFL_BBD_SetObjectFlipS( bbdSys , pokeWork->objIdx , &flipFlg );

    pokeWork->state = MCPS_LOOK;

  }
  else
  if( pokeWork->cnt == lookTime ||
      pokeWork->cnt == lookTime*2+hideTime )
  {
    //隠れる
    const BOOL flg = FALSE;
    MB_CAPTURE_GetGrassObjectPos( pokeWork->posXidx , pokeWork->posYidx , &pokeWork->pos );
    pokeWork->pos.y -= FX32_CONST( MB_CAP_POKE_OFS_Y );
    pokeWork->pos.z -= FX32_CONST( MB_CAP_OBJ_BASE_Z-MB_CAP_POKE_BASE_Z );
    GFL_BBD_SetObjectDrawEnable( bbdSys , pokeWork->objIdx , &flg );
    GFL_BBD_SetObjectTrans( bbdSys , pokeWork->objIdx , &pokeWork->pos );

    pokeWork->state = MCPS_HIDE;
  }
}

//--------------------------------------------------------------
//	ステート：逃げる
//--------------------------------------------------------------
static void MB_CAP_POKE_StateRun(MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  if( pokeWork->cnt == 0)
  {
    PMSND_PlaySE( MB_SND_POKE_JUMP );
  }

  pokeWork->cnt += pokeWork->actSpeed;
  if( pokeWork->cnt < MB_CAP_POKE_RUN_LOOK_TIME*MB_CAP_POKE_ACT_RATE )
  {
    //飛ぶ
    const BOOL isDisp = TRUE;
    const BOOL isDispShadow = (pokeWork->isGround==FALSE?TRUE:FALSE);
    BOOL flipFlg = FALSE;
    VecFx32 dispPos;
    
    {
      //ジャンプの計算
      const u16 rot = 0x8000*pokeWork->cnt/MB_CAP_POKE_RUN_LOOK_TIME/MB_CAP_POKE_ACT_RATE;
      fx32 ofs;
      VecFx32 nextPos;
      pokeWork->height = FX_SinIdx( rot ) * MB_CAP_POKE_JUMP_HEIGHT;
      switch( pokeWork->dir )
      {
      case MCPD_LEFT:
        //草からの基準位置
        MB_CAPTURE_GetGrassObjectPos( pokeWork->posXidx-1 , pokeWork->posYidx , &nextPos );
        ofs = nextPos.x - pokeWork->startPos.x;
        if( ofs <= -FX32_CONST(MB_CAP_OBJ_MAIN_X_SPACE+10) )
        {
          MB_TPrintf("ADJUST!!!\n");
          pokeWork->posXidx++;
          MB_CAPTURE_GetGrassObjectPos( pokeWork->posXidx-1 , pokeWork->posYidx , &nextPos );
          ofs = nextPos.x - pokeWork->startPos.x;
        }
        pokeWork->pos.x = pokeWork->startPos.x + ofs*pokeWork->cnt/MB_CAP_POKE_RUN_LOOK_TIME/MB_CAP_POKE_ACT_RATE;
        pokeWork->pos.y = pokeWork->startPos.y;
        pokeWork->pos.z = pokeWork->startPos.z;
        break;
      case MCPD_RIGHT:
        //草からの基準位置
        MB_CAPTURE_GetGrassObjectPos( pokeWork->posXidx+1 , pokeWork->posYidx , &nextPos );
        ofs = nextPos.x - pokeWork->startPos.x;
        if( ofs >= FX32_CONST(MB_CAP_OBJ_MAIN_X_SPACE+10) )
        {
          MB_TPrintf("ADJUST!!!\n");
          pokeWork->posXidx--;
          MB_CAPTURE_GetGrassObjectPos( pokeWork->posXidx+1 , pokeWork->posYidx , &nextPos );
          ofs = nextPos.x - pokeWork->startPos.x;
        }
        pokeWork->pos.x = pokeWork->startPos.x + ofs*pokeWork->cnt/MB_CAP_POKE_RUN_LOOK_TIME/MB_CAP_POKE_ACT_RATE;
        pokeWork->pos.y = pokeWork->startPos.y;
        pokeWork->pos.z = pokeWork->startPos.z;
        flipFlg = ( pokeWork->isNoFlip == FALSE ? TRUE : FALSE );
        break;
      case MCPD_UP:
        MB_CAPTURE_GetGrassObjectPos( pokeWork->posXidx , pokeWork->posYidx-1 , &nextPos );
        if( pokeWork->posYidx <= 0 )
        {
          //最上段補正！！！
          nextPos.y = FX32_CONST(MB_CAP_OBJ_MAIN_TOP-8);
        }
        
        ofs = (nextPos.y - FX32_CONST( MB_CAP_POKE_MOVE_OFS_Y )) - pokeWork->startPos.y;
        if( ofs <= -FX32_CONST(MB_CAP_OBJ_MAIN_Y_SPACE+10) &&
            pokeWork->posYidx > 0)
        {
          MB_TPrintf("ADJUST_DOWN!!!\n");
          pokeWork->posYidx++;
          MB_CAPTURE_GetGrassObjectPos( pokeWork->posXidx , pokeWork->posYidx-1 , &nextPos );
          ofs = (nextPos.y - FX32_CONST( MB_CAP_POKE_MOVE_OFS_Y )) - pokeWork->startPos.y;
        }

        pokeWork->pos.x = pokeWork->startPos.x;
        pokeWork->pos.y = pokeWork->startPos.y + ofs*pokeWork->cnt/MB_CAP_POKE_RUN_LOOK_TIME/MB_CAP_POKE_ACT_RATE;

        if( pokeWork->posYidx <= 0 &&
            pokeWork->cnt > MB_CAP_POKE_RUN_LOOK_TIME/2*MB_CAP_POKE_ACT_RATE )
        {
          //最上段補正！！！
          pokeWork->pos.z = FX32_CONST(MB_CAP_OBJ_BASE_Z - MB_CAP_OBJ_LINEOFS_Z - MB_CAP_OBJ_LINEOFS_Z/2);
        }
        else
        {
          pokeWork->pos.z = pokeWork->startPos.z;
        }
        flipFlg = ( pokeWork->isNoFlip == FALSE ? TRUE : FALSE );
        break;
      case MCPD_DOWN:
        MB_CAPTURE_GetGrassObjectPos( pokeWork->posXidx , pokeWork->posYidx+1 , &nextPos );
        ofs = (nextPos.y - FX32_CONST( MB_CAP_POKE_MOVE_OFS_Y )) - pokeWork->startPos.y;
        if( ofs >= FX32_CONST(MB_CAP_OBJ_MAIN_Y_SPACE+10) )
        {
          MB_TPrintf("ADJUST_DOWN!!!\n");
          pokeWork->posYidx--;
          MB_CAPTURE_GetGrassObjectPos( pokeWork->posXidx , pokeWork->posYidx+1 , &nextPos );
          ofs = (nextPos.y - FX32_CONST( MB_CAP_POKE_MOVE_OFS_Y )) - pokeWork->startPos.y;
        }

        pokeWork->pos.x = pokeWork->startPos.x;
        pokeWork->pos.y = pokeWork->startPos.y + ofs*pokeWork->cnt/MB_CAP_POKE_RUN_LOOK_TIME/MB_CAP_POKE_ACT_RATE;
        pokeWork->pos.z = pokeWork->startPos.z;
        //下移動のときだけZ補正
        pokeWork->pos.z += FX32_CONST( MB_CAP_OBJ_LINEOFS_Z );
        break;
      }
      //ディグダ
      if( pokeWork->isGround == TRUE )
      {
        if( pokeWork->dir == MCPD_UP && 
            pokeWork->posYidx <= 0 )
        {
          //最上段補正(影を消しているので上移動に見せかける
          pokeWork->height = FX_SinIdx( rot ) * (MB_CAP_POKE_JUMP_HEIGHT-8);
        }
        else
        {
          pokeWork->height = 0;
        }
      }
    
      dispPos.x = pokeWork->pos.x;
      dispPos.y = pokeWork->pos.y+FX32_CONST(6.0f);
      dispPos.z = pokeWork->pos.z;

      GFL_BBD_SetObjectDrawEnable( bbdSys , pokeWork->objShadowIdx , &isDispShadow );
      GFL_BBD_SetObjectTrans( bbdSys , pokeWork->objShadowIdx , &dispPos );
      
      dispPos.y = pokeWork->pos.y-pokeWork->height;

      GFL_BBD_SetObjectDrawEnable( bbdSys , pokeWork->objIdx , &isDisp );
      GFL_BBD_SetObjectTrans( bbdSys , pokeWork->objIdx , &dispPos );
      GFL_BBD_SetObjectFlipS( bbdSys , pokeWork->objIdx , &flipFlg );
    }

    pokeWork->state = MCPS_RUN_LOOK;
    
  }
  else
  if( pokeWork->cnt >= MB_CAP_POKE_RUN_LOOK_TIME*MB_CAP_POKE_ACT_RATE &&
      pokeWork->isRun == TRUE )
  {
    //着地
    BOOL isCrossPoke = FALSE;
    const BOOL flg = FALSE;
    GFL_BBD_SetObjectDrawEnable( bbdSys , pokeWork->objShadowIdx , &flg );
    GFL_BBD_SetObjectDrawEnable( bbdSys , pokeWork->objIdx , &flg );
    
    //ポケあたりチェックとかで上書きする
    pokeWork->isRun = FALSE;

    switch( pokeWork->dir )
    {
    case MCPD_LEFT:
      pokeWork->posXidx--;
      break;
    case MCPD_RIGHT:
      pokeWork->posXidx++;
      break;
    case MCPD_UP:
      pokeWork->posYidx--;
      break;
    case MCPD_DOWN:
      pokeWork->posYidx++;
      break;
    }
    
    //OBJアニメのリクエスト
    {
      MB_CAP_OBJ *objWork;
      u8 objIdx;
      if( pokeWork->posXidx == -1 )
      {
        objIdx = MB_CAP_OBJ_SUB_L_START+pokeWork->posYidx;
      }
      else
      if( pokeWork->posXidx == MB_CAP_OBJ_X_NUM )
      {
        objIdx = MB_CAP_OBJ_SUB_R_START+pokeWork->posYidx;
      }
      else
      if( pokeWork->posYidx == -1 )
      {
        //木は揺らさない
        objIdx = 0xFF;
      }
      else
      if( pokeWork->posYidx == MB_CAP_OBJ_Y_NUM )
      {
        objIdx = MB_CAP_OBJ_SUB_D_START+pokeWork->posXidx;
      }
      else
      {
        objIdx = pokeWork->posXidx + pokeWork->posYidx*MB_CAP_OBJ_X_NUM;
      }
      if( objIdx != 0xFF )
      {
        objWork = MB_CAPTURE_GetObjWork( capWork , objIdx );
        if( objWork != NULL )
        {
          MB_CAP_OBJ_StartAnime( objWork );
        }
      }
    }
  
    
    //逃げ先にポケがいるかチェック
    {
      u8 i;
      for( i=0;i<MB_CAP_POKE_NUM;i++ )
      {
        MB_CAP_POKE *checkPoke = MB_CAPTURE_GetPokeWork( capWork , i );
        if( checkPoke != pokeWork )
        {
          if( pokeWork->posXidx == checkPoke->posXidx &&
              pokeWork->posYidx == checkPoke->posYidx )
          {
            if( checkPoke->state == MCPS_HIDE ||
                checkPoke->state == MCPS_LOOK ||
                checkPoke->state == MCPS_RUN_HIDE )
            {
              isCrossPoke = TRUE;
            }
          }
        }
      }
    }
    
    if( isCrossPoke == FALSE )
    {
      pokeWork->state = MCPS_RUN_HIDE;
      
      if( pokeWork->posXidx <= -1 ||
          pokeWork->posXidx >= MB_CAP_OBJ_X_NUM ||
          pokeWork->posYidx <= -1 ||
          pokeWork->posYidx >= MB_CAP_OBJ_Y_NUM )
      {
        MB_CAP_POKE_SetEscape( capWork , pokeWork );
      }
    }
    else
    {
      //時計回りに方向転換
      pokeWork->cnt = 0;
      pokeWork->isRun = TRUE;
      MB_CAP_POKE_CheckActSpeed( capWork , pokeWork );
      switch( pokeWork->dir )
      {
      case MCPD_LEFT:
        pokeWork->dir = MCPD_UP;
        break;
      case MCPD_RIGHT:
        pokeWork->dir = MCPD_DOWN;
        break;
      case MCPD_UP:
        pokeWork->dir = MCPD_RIGHT;
        break;
      case MCPD_DOWN:
        pokeWork->dir = MCPD_LEFT;
        break;
      }
    }
    
    
    MB_CAPTURE_GetGrassObjectPos( pokeWork->posXidx , pokeWork->posYidx , &pokeWork->startPos );
    pokeWork->startPos.y -= FX32_CONST( MB_CAP_POKE_MOVE_OFS_Y );
    pokeWork->startPos.z -= FX32_CONST( MB_CAP_OBJ_BASE_Z-MB_CAP_POKE_BASE_Z );

    pokeWork->befHitNo = 0xFF;
    MB_CAP_POKE_CheckActSpeed( capWork , pokeWork );

    PMSND_PlaySE( MB_SND_GRASS_SHAKE );
  }
  else
  if( pokeWork->cnt >= (MB_CAP_POKE_RUN_LOOK_TIME+MB_CAP_POKE_RUN_HIDE_TIME)*MB_CAP_POKE_ACT_RATE )
  {
    pokeWork->isRun = TRUE;
    pokeWork->cnt = 0;
    MB_CAP_POKE_CheckActSpeed( capWork , pokeWork );
  }
}

//--------------------------------------------------------------
//	ステート：ダウン！(眠り
//--------------------------------------------------------------
static void MB_CAP_POKE_StateDown(MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  if( pokeWork->state == MCPS_DOWN_MOVE ||
      pokeWork->state == MCPS_SLEEP_FALL )
  {
    VecFx32 pos;
    if( pokeWork->height <= MB_CAP_POKE_FALL_SPEED )
    {
      VecFx32 effPos = pokeWork->pos;
      effPos.y -= MB_CAP_POKE_DOWN_EFF_OFS;

      pokeWork->height = 0;
      pokeWork->cnt = 0;
      if( pokeWork->state == MCPS_DOWN_MOVE )
      {
        effPos.z = FX32_CONST(MB_CAP_EFFECT_Z);
        pokeWork->downEff = MB_CAPTURE_CreateEffect( capWork , &effPos , MCET_DOWN );
        MB_CAP_EFFECT_SetIsLoop( pokeWork->downEff , TRUE );
        pokeWork->state = MCPS_DOWN_WAIT;
      }
      else
      {
        effPos.z = FX32_CONST(MB_CAP_EFFECT_ZZZ_Z);
        pokeWork->downEff = MB_CAPTURE_CreateEffect( capWork , &effPos , MCET_ZZZ );
        MB_CAP_EFFECT_SetIsLoop( pokeWork->downEff , TRUE );
        pokeWork->state = MCPS_SLEEP_WAIT;
      }
    }
    else
    {
      pokeWork->height -= MB_CAP_POKE_FALL_SPEED;
    }
    pos.x = pokeWork->pos.x;
    pos.y = pokeWork->pos.y - pokeWork->height;
    pos.z = pokeWork->pos.z + FX32_ONE;
    GFL_BBD_SetObjectTrans( bbdSys , pokeWork->objIdx , &pos );
  }
  else
  {
    const u16 downTime = (pokeWork->state == MCPS_DOWN_WAIT ? MB_CAP_POKE_DOWN_TIME : MB_CAP_BONUSTIME );
    pokeWork->cnt++;
    if( pokeWork->cnt > downTime )
    {
      if( pokeWork->state == MCPS_DOWN_WAIT )
      {
        //ぶつかってダウンした時のみ
        //方向転換して移動
        MB_CAP_POKE_FlipDir( pokeWork );
      }

      if( pokeWork->state == MCPS_DOWN_WAIT ||
          pokeWork->state == MCPS_SLEEP_WAIT )
      {
        //ダウンと移動中の睡眠のみ
        pokeWork->startPos = pokeWork->pos;
        pokeWork->cnt = 0;
        pokeWork->isRun = TRUE;
        pokeWork->stateFunc = MB_CAP_POKE_StateRun;
        MB_CAP_POKE_CheckActSpeed( capWork , pokeWork );
      }
      else
      {
        MB_CAP_POKE_SetHide( capWork , pokeWork , pokeWork->posXidx , pokeWork->posYidx );
      }

      MB_CAP_EFFECT_SetIsFinish( pokeWork->downEff , TRUE );
      pokeWork->downEff = NULL;
    }
  }
}
#pragma mark [>util func
//--------------------------------------------------------------
//移動速度レート更新
//--------------------------------------------------------------
static void MB_CAP_POKE_CheckActSpeed( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork )
{
  u8 least = MB_CAPTURE_GetLeastPoke( capWork );
#if MB_CAP_DEB  
  u8 rateTbl[MB_CAP_POKE_NUM];
  rateTbl[0] = MB_CAP_POKE_RATE1;
  rateTbl[1] = MB_CAP_POKE_RATE2;
  rateTbl[2] = MB_CAP_POKE_RATE3;
  rateTbl[3] = MB_CAP_POKE_RATE4;
  rateTbl[4] = MB_CAP_POKE_RATE5;
  rateTbl[5] = MB_CAP_POKE_RATE6;
#else
  u8 rateTbl[MB_CAP_POKE_NUM] = 
  {
    MB_CAP_POKE_RATE1,
    MB_CAP_POKE_RATE2,
    MB_CAP_POKE_RATE3,
    MB_CAP_POKE_RATE4,
    MB_CAP_POKE_RATE5,
    MB_CAP_POKE_RATE6
  };
#endif
  pokeWork->actSpeed = rateTbl[least-1];
}

#pragma mark [>outer func
//--------------------------------------------------------------
//指定座標にいるか？
//--------------------------------------------------------------
const BOOL MB_CAP_POKE_CheckPos( const MB_CAP_POKE *pokeWork , const u8 idxX , const u8 idxY )
{
  if( pokeWork->posXidx == idxX &&
      pokeWork->posYidx == idxY )
  {
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
//
//--------------------------------------------------------------
const MB_CAP_POKE_STATE MB_CAP_POKE_GetState( const MB_CAP_POKE *pokeWork )
{
  return pokeWork->state;
}

//--------------------------------------------------------------
//  当たり判定作成
//--------------------------------------------------------------
void MB_CAP_POKE_GetHitWork( MB_CAP_POKE *pokeWork , MB_CAP_HIT_WORK *hitWork )
{
  hitWork->pos = &pokeWork->pos;
  hitWork->height = pokeWork->height;
  hitWork->size.x = FX32_CONST(MB_CAP_POKE_HITSIZE_X);
  hitWork->size.y = FX32_CONST(MB_CAP_POKE_HITSIZE_Y);
  hitWork->size.z = FX32_CONST(MB_CAP_POKE_HITSIZE_Z);
}

//--------------------------------------------------------------
//  捕獲可能なステートか？
//--------------------------------------------------------------
const BOOL MB_CAP_POKE_CheckCanCapture( MB_CAP_POKE *pokeWork )
{
  if( pokeWork->state == MCPS_RUN_LOOK ||
      pokeWork->state == MCPS_DOWN_MOVE ||
      pokeWork->state == MCPS_DOWN_WAIT ||
      pokeWork->state == MCPS_SLEEP_FALL ||
      pokeWork->state == MCPS_SLEEP_WAIT ||
      pokeWork->state == MCPS_SLEEP_WAIT_GRASS )
  {
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
//  連続ヒット防止(前回ヒットポケ
//--------------------------------------------------------------
const u8 MB_CAP_POKE_GetBefHitPoke( MB_CAP_POKE *pokeWork )
{
  return pokeWork->befHitNo;
}

void MB_CAP_POKE_SetBefHitPoke( MB_CAP_POKE *pokeWork , const u8 no )
{
  pokeWork->befHitNo = no;
}

//--------------------------------------------------------------
//  連続ヒット防止(同じ方向チェック
//--------------------------------------------------------------
const MB_CAP_POKE_DIR MB_CAP_POKE_GetPokeDir( MB_CAP_POKE *pokeWork )
{
  return pokeWork->dir;
}
void MB_CAP_POKE_FlipDir( MB_CAP_POKE *pokeWork )
{
  switch( pokeWork->dir )
  {
  case MCPD_LEFT:
    pokeWork->dir = MCPD_RIGHT;
    break;
  case MCPD_RIGHT:
    pokeWork->dir = MCPD_LEFT;
    break;
  case MCPD_UP:
    pokeWork->dir = MCPD_DOWN;
    break;
  case MCPD_DOWN:
    pokeWork->dir = MCPD_UP;
    break;
  }
}

//--------------------------------------------------------------
//  隠れ時間リセット
//--------------------------------------------------------------
void MB_CAP_POKE_ResetHideTime(MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork )
{
  //ハードポケとの区別
  u16 lookTime;
  u16 hideTime;
  u16 totalTime;
  if( pokeWork->isHard == FALSE )
  {
    lookTime = MB_CAP_POKE_HIDE_LOOK_TIME;
    hideTime = MB_CAP_POKE_HIDE_HIDE_TIME;
    totalTime = MB_CAP_POKE_HIDE_TOTAL_TIME;
  }
  else
  {
    lookTime = MB_CAP_POKE_HIDE_LOOK_TIME_H;
    hideTime = MB_CAP_POKE_HIDE_HIDE_TIME_H;
    totalTime = MB_CAP_POKE_HIDE_TOTAL_TIME_H;
  }
  if( pokeWork->cnt < lookTime+hideTime )
  {
    //次は右
    pokeWork->cnt = lookTime;
  }
  else
  {
    //次は左
    pokeWork->cnt = lookTime*2+hideTime;
  }
}

