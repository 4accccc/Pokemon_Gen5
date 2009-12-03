//======================================================================
/**
 * @file	mb_sel_poke.h
 * @brief	マルチブート：ボックス・ポケモン
 * @author	ariizumi
 * @data	09/12/03
 *
 * モジュール名：MB_SEL_POKE
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "mb_select_gra.naix"

#include "multiboot/mb_poke_icon.h"
#include "mb_sel_poke.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define MB_SEL_POKE_BOX_TOP  (44)
#define MB_SEL_POKE_BOX_LEFT (28)
#define MB_SEL_POKE_BOX_WIDTH (24)
#define MB_SEL_POKE_BOX_HEIGHT (24)
#define MB_SEL_POKE_BOX_X_NUM  (6)

#define MB_SEL_POKE_TRAY_TOP  (68)
#define MB_SEL_POKE_TRAY_LEFT (192)
#define MB_SEL_POKE_TRAY_WIDTH (40)
#define MB_SEL_POKE_TRAY_HEIGHT (32)
#define MB_SEL_POKE_TRAY_X_NUM  (2)
#define MB_SEL_POKE_TRAY_OFS    (16) //右のオフセット

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _MB_SEL_POKE
{
  POKEMON_PASO_PARAM *ppp;
  
  MB_SEL_POKE_TYPE type;
  u8       idx;
  u32      cellResIdx;
  GFL_CLWK *pokeIcon;
  
  BOOL isValid;
  int posX;
  int posY;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static const int MB_SEL_POKE_GetPosX( MB_SEL_POKE *pokeWork );
static const int MB_SEL_POKE_GetPosY( MB_SEL_POKE *pokeWork );

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
MB_SEL_POKE* MB_SEL_POKE_CreateWork( MB_SELECT_WORK *selWork , MB_SEL_POKE_INIT_WORK *initWork )
{
  GFL_CLWK_DATA cellInitData;
  MB_SEL_POKE* pokeWork = GFL_HEAP_AllocClearMemory( initWork->heapId , sizeof(MB_SEL_POKE) );

  pokeWork->type = initWork->iconType;
  pokeWork->idx  = initWork->idx;
  pokeWork->posX = MB_SEL_POKE_GetPosX( pokeWork );
  pokeWork->posY = MB_SEL_POKE_GetPosY( pokeWork );

  //この時点ではpppは未定とし、ダミーセルを入れる
  pokeWork->cellResIdx = GFL_CLGRP_CGR_Register( initWork->arcHandle , 
                      NARC_mb_select_gra_icon_dummy_NCGR , 
                      FALSE , CLSYS_DRAW_MAIN , initWork->heapId  );

  cellInitData.pos_x = pokeWork->posX;
  cellInitData.pos_y = pokeWork->posY;
  cellInitData.anmseq = 0;
  cellInitData.softpri = 0;
  cellInitData.bgpri = 1;

  pokeWork->pokeIcon = GFL_CLACT_WK_Create( initWork->cellUnit ,
            pokeWork->cellResIdx,
            initWork->palResIdx,
            initWork->anmResIdx,
            &cellInitData ,CLSYS_DRAW_MAIN , initWork->heapId );

  GFL_CLACT_WK_SetDrawEnable( pokeWork->pokeIcon , FALSE );

  pokeWork->ppp = NULL;
  pokeWork->isValid = FALSE;
  return pokeWork;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void MB_SEL_POKE_DeleteWork( MB_SELECT_WORK *selWork , MB_SEL_POKE *pokeWork )
{
  GFL_CLACT_WK_Remove( pokeWork->pokeIcon );
  GFL_HEAP_FreeMemory( pokeWork );
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
void MB_SEL_POKE_UpdateWork( MB_SELECT_WORK *selWork , MB_SEL_POKE *pokeWork )
{
  
}

//--------------------------------------------------------------
//	アイコン作成
//  NULLか不正なpppを渡すと消す
//--------------------------------------------------------------
void MB_SEL_POKE_SetPPP( MB_SELECT_WORK *selWork , MB_SEL_POKE *pokeWork , POKEMON_PASO_PARAM *ppp )
{
  ARCHANDLE *iconArcHandle = MB_SELECT_GetIconArcHandle( selWork );
  const DLPLAY_CARD_TYPE type = MB_SELECT_GetCardType( selWork );
  const HEAPID heapId = MB_SELECT_GetHeapId( selWork );

  pokeWork->isValid = TRUE;
  pokeWork->ppp = ppp;
  
  if( ppp == NULL )
  {
    pokeWork->isValid = FALSE;
  }
  else
  if( PPP_Get( pokeWork->ppp , ID_PARA_poke_exist , NULL ) == FALSE )
  {
    pokeWork->isValid = FALSE;
  }
  
  if( pokeWork->isValid == TRUE )
  {
    NNSG2dCharacterData *tempNcg;
    void *tempRes = GFL_ARCHDL_UTIL_LoadOBJCharacter( iconArcHandle ,
                                          MB_ICON_GetCharResId(pokeWork->ppp,type) ,
                                          FALSE ,
                                          &tempNcg ,
                                          heapId );
    GFL_CLGRP_CGR_Replace( pokeWork->cellResIdx , tempNcg );
    GFL_CLACT_WK_SetPlttOffs( pokeWork->pokeIcon , 
                              MB_ICON_GetPalNumber(pokeWork->ppp) ,
                              CLWK_PLTTOFFS_MODE_PLTT_TOP );
    GFL_CLACT_WK_SetDrawEnable( pokeWork->pokeIcon , TRUE );
    GFL_HEAP_FreeMemory( tempRes );
  }
  else
  {
    //居ない!
    GFL_CLACT_WK_SetDrawEnable( pokeWork->pokeIcon , FALSE );
  }
}



//--------------------------------------------------------------
//	位置計算
//--------------------------------------------------------------
static const int MB_SEL_POKE_GetPosX( MB_SEL_POKE *pokeWork )
{
  if( pokeWork->type == MSPT_BOX )
  {
    return (pokeWork->idx % MB_SEL_POKE_BOX_X_NUM)*MB_SEL_POKE_BOX_WIDTH + MB_SEL_POKE_BOX_LEFT;
  }
  else
  {
    return (pokeWork->idx % MB_SEL_POKE_TRAY_X_NUM)*MB_SEL_POKE_TRAY_WIDTH + MB_SEL_POKE_TRAY_LEFT;
  }
}
static const int MB_SEL_POKE_GetPosY( MB_SEL_POKE *pokeWork )
{
  if( pokeWork->type == MSPT_BOX )
  {
    return (pokeWork->idx / MB_SEL_POKE_BOX_X_NUM)*MB_SEL_POKE_BOX_HEIGHT + MB_SEL_POKE_BOX_TOP;
  }
  else
  {
    if( pokeWork->idx%MB_SEL_POKE_TRAY_X_NUM == 0 )
    {
      return (pokeWork->idx / MB_SEL_POKE_TRAY_X_NUM)*MB_SEL_POKE_TRAY_HEIGHT + MB_SEL_POKE_TRAY_TOP;
    }
    else
    {
      return (pokeWork->idx / MB_SEL_POKE_TRAY_X_NUM)*MB_SEL_POKE_TRAY_HEIGHT + MB_SEL_POKE_TRAY_TOP + MB_SEL_POKE_TRAY_OFS;
    }
  }
}
