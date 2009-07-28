//======================================================================
/**
 * @file  sta_act_poke.h
 * @brief ステージ用　ポケモン
 * @author  ariizumi
 * @data  09/03/06
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "arc_def.h"
#include "stage_gra.naix"


#include "sta_local_def.h"
#include "sta_act_poke.h"

#include "test/ariizumi/ari_debug.h"


//======================================================================
//  define
//======================================================================
#pragma mark [> define


//======================================================================
//  enum
//======================================================================
#pragma mark [> enum


//======================================================================
//  typedef struct
//======================================================================
#pragma mark [> struct
struct _STA_POKE_WORK
{
  BOOL    isEnable;
  BOOL    isUpdate; //座標計算が発生したか？
  BOOL    isFront;
  BOOL    isDrawItem;
  
  VecFx32         pokePos;
  VecFx32         scale;
  VecFx32         posOfs; //移動差分(アクション用に
  u16             rotate;

  STA_POKE_DIR      dir;
  MUS_POKE_DRAW_WORK    *drawWork;
  MUS_POKE_DATA_WORK    *pokeData;

  int           shadowBbdIdx;

  GFL_G3D_RES     *itemRes[MUS_POKE_EQUIP_MAX];
  MUS_ITEM_DRAW_WORK  *itemWork[MUS_POKE_EQUIP_MAX];
};

struct _STA_POKE_SYS
{
  HEAPID heapId;

  GFL_BBD_SYS       *bbdSys;
  MUS_POKE_DRAW_SYSTEM  *drawSys;
  MUS_ITEM_DRAW_SYSTEM  *itemDrawSys;
  STA_POKE_WORK     pokeWork[MUSICAL_POKE_MAX];
  
  u16   scrollOffset;

  int   shadowResIdx;
};

//======================================================================
//  proto
//======================================================================
#pragma mark [> proto

static void STA_POKE_UpdatePokeFunc( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork );
static void STA_POKE_UpdateItemFunc( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork );
static void STA_POKE_DrawShadow( STA_POKE_SYS *work ,  STA_POKE_WORK *pokeWork );

//--------------------------------------------------------------
//  
//--------------------------------------------------------------
STA_POKE_SYS* STA_POKE_InitSystem( HEAPID heapId , MUS_POKE_DRAW_SYSTEM* drawSys , MUS_ITEM_DRAW_SYSTEM *itemDrawSys , GFL_BBD_SYS* bbdSys )
{
  u8 i;
  STA_POKE_SYS *work = GFL_HEAP_AllocMemory( heapId , sizeof(STA_POKE_SYS) );
  
  work->heapId = heapId;
  work->drawSys = drawSys;
  work->itemDrawSys = itemDrawSys;
  work->scrollOffset = 0;
  work->bbdSys = bbdSys;
  
  for( i=0 ; i<MUSICAL_POKE_MAX ; i++ )
  {
    work->pokeWork[i].isEnable = FALSE;
    work->pokeWork[i].isUpdate = FALSE;
  }
  
  
  //影ポリの読み込み
  work->shadowResIdx = GFL_BBD_AddResourceArc( work->bbdSys , ARCID_STAGE_GRA , 
                      NARC_stage_gra_shadow_tex_nsbtx , GFL_BBD_TEXFMT_PAL4 ,
                      GFL_BBD_TEXSIZ_64x64 , 64 , 64 );
  
  return work;
}

void  STA_POKE_ExitSystem( STA_POKE_SYS *work )
{
  int idx;
  for( idx=0 ; idx<MUSICAL_POKE_MAX ; idx++ )
  {
    if( work->pokeWork[idx].isEnable == TRUE )
    {
      STA_POKE_DeletePoke( work , &work->pokeWork[idx] );
    }
  }
  GFL_BBD_RemoveResource( work->bbdSys , work->shadowResIdx );

  GFL_HEAP_FreeMemory( work );
}

void  STA_POKE_UpdateSystem( STA_POKE_SYS *work )
{
  int idx;
  for( idx=0 ; idx<MUSICAL_POKE_MAX ; idx++ )
  {
    if( work->pokeWork[idx].isEnable == TRUE )
    {
      STA_POKE_UpdatePokeFunc( work , &work->pokeWork[idx] );
    }
  }
}

void  STA_POKE_UpdateSystem_Item( STA_POKE_SYS *work )
{
  int idx;
  for( idx=0 ; idx<MUSICAL_POKE_MAX ; idx++ )
  {
    if( work->pokeWork[idx].isEnable == TRUE )
    {
      STA_POKE_UpdateItemFunc( work , &work->pokeWork[idx] );
    }
  }
  MUS_ITEM_DRAW_UpdateSystem( work->itemDrawSys ); 
}

static void STA_POKE_UpdatePokeFunc( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork )
{
  /*
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X )
  {
    u16 rot;
    MUS_POKE_DRAW_GetRotation( pokeWork->drawWork , &rot);
    rot += 0x800;
    MUS_POKE_DRAW_SetRotation( pokeWork->drawWork , rot);
    
  }
  */
#if DEB_ARI|1
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
    {
      STA_POKE_SetFrontBack( work , pokeWork , !pokeWork->isFront );
    }
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y )
    {
      STA_POKE_SetDrawItem( work , pokeWork , !pokeWork->isDrawItem );
    }
#endif

  if( pokeWork->isUpdate == TRUE )
  {
    VecFx32 musPos;
    VecFx32 musScale;
    
    //ポケモンの座標
    VEC_Add( &pokeWork->pokePos , &pokeWork->posOfs , &musPos );
    
    musPos.x = ACT_POS_X_FX(musPos.x - FX32_CONST(work->scrollOffset));
    musPos.y = ACT_POS_Y_FX(musPos.y);
//    musPos.z = musPos.z;
    
    MUS_POKE_DRAW_SetPosition( pokeWork->drawWork , &musPos);

    //回転の適用
    MUS_POKE_DRAW_SetRotation( pokeWork->drawWork , pokeWork->rotate);
    
    //スケールの調整と向きの確認
    musScale.x = pokeWork->scale.x * 16;
    musScale.y = pokeWork->scale.y * 16;
    musScale.z = pokeWork->scale.z * 16;
    if( pokeWork->dir == SPD_RIGHT )
    {
      musScale.x *= -1;
    }
    MUS_POKE_DRAW_SetScale( pokeWork->drawWork , &musScale );
    
    //アイテムの更新要る？
    pokeWork->isUpdate = FALSE;
  }
}

static void STA_POKE_UpdateItemFunc( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork )
{
  int ePos;
  VecFx32 pos;
  for( ePos=0;ePos<MUS_POKE_EQUIP_MAX;ePos++ )
  {
    if( pokeWork->itemWork[ePos] != NULL )
    {
      if( pokeWork->isDrawItem == TRUE &&
          pokeWork->isFront == TRUE )
      {
      
        MUS_POKE_EQUIP_DATA *equipData = MUS_POKE_DRAW_GetEquipData( pokeWork->drawWork , ePos );
        if( equipData->isEnable == TRUE )
        {
          const BOOL flipS = ( equipData->scale.x < 0 ? TRUE : FALSE);
          const u16 itemRot = ( flipS==TRUE ? 0x10000-equipData->itemRot : equipData->itemRot);
          const u16 rotZ = 0x10000-equipData->rot;//( flipS==TRUE ? 0x10000-equipData->rot : equipData->rot);

          MtxFx33 rotWork;
          VecFx32 rotOfs;
          VecFx32 ofs;
          MTX_RotZ33( &rotWork , -FX_SinIdx( rotZ ) , FX_CosIdx( rotZ ) );
          MTX_MultVec33( &equipData->ofs , &rotWork , &ofs );
          
          {
            MTX_MultVec33( &equipData->rotOfs , &rotWork , &rotOfs );
            VEC_Subtract( &equipData->rotOfs , &rotOfs , &rotOfs );
          }

          pos.x = (equipData->pos.x+ofs.x+FX32_CONST(128.0f) + rotOfs.x);
          pos.y = (equipData->pos.y+ofs.y+FX32_CONST(96.0f) + rotOfs.y);
          if( MUS_ITEM_DRAW_IsBackItem( pokeWork->itemWork[ePos] ) == TRUE )
          {
            //背面用アイテム
            pos.z = pokeWork->pokePos.z-FX32_CONST(20.0f);
          }
          else
          {
            //とりあえずポケの前に出す
            pos.z = pokeWork->pokePos.z+FX32_HALF;  
          }

          //OS_Printf("[%.2f][%.2f]\n",F32_CONST(equipData->pos.z),F32_CONST(pokePos.z));
          MUS_ITEM_DRAW_SetPosition(  work->itemDrawSys , 
                        pokeWork->itemWork[ePos] ,
                        &pos );
          MUS_ITEM_DRAW_SetRotation(  work->itemDrawSys , 
                        pokeWork->itemWork[ePos] ,
                        itemRot-rotZ );
          MUS_ITEM_DRAW_SetSize(    work->itemDrawSys , 
                        pokeWork->itemWork[ePos] ,
                        equipData->scale.x /16 /4,
                        equipData->scale.y /16 /4);
          MUS_ITEM_DRAW_SetDrawEnable( work->itemDrawSys , 
                          pokeWork->itemWork[ePos] , TRUE );
  //        MUS_ITEM_DRAW_SetFlipS( work->itemDrawSys , 
  //                    pokeWork->itemWork[ePos] ,
  //                    flipS );
        }
      }
      else
      {
        MUS_ITEM_DRAW_SetDrawEnable( work->itemDrawSys , 
                        pokeWork->itemWork[ePos] , FALSE );
      }
    }
  }

  //追従なのでここで一緒にやる
  //影の座標
  {
    VecFx32 *shadowOfs; //影差分
    shadowOfs = MUS_POKE_DRAW_GetShadowOfs( pokeWork->drawWork );
    pos.x = ACT_POS_X_FX(pokeWork->pokePos.x - FX32_CONST(work->scrollOffset) + shadowOfs->x );
    pos.y = ACT_POS_Y_FX(pokeWork->pokePos.y + shadowOfs->y );
    pos.z = 5.0f;
    GFL_BBD_SetObjectTrans( work->bbdSys , pokeWork->shadowBbdIdx , &pos );
  }
}


void  STA_POKE_DrawSystem( STA_POKE_SYS *work )
{
  //丸影
  int idx;
  for( idx=0 ; idx<MUSICAL_POKE_MAX ; idx++ )
  {
    if( work->pokeWork[idx].isEnable == TRUE )
    {
      if( STA_POKE_GetShowFlg( work , &work->pokeWork[idx] ) == TRUE )
      {
        STA_POKE_DrawShadow( work , &work->pokeWork[idx] );
      }
    }
  }
}
#if 0 //ポリゴン直書きVer
static void STA_POKE_DrawShadow( STA_POKE_SYS *work ,  STA_POKE_WORK *pokeWork )
{
  u32 i;
  u16 add = 0x1000;
  VecFx32 shadowPos;
  const fx32 sizeX = FX32_HALF;
  const fx32 sizeY = FX32_CONST(0.25f);
  
  shadowPos.x = ACT_POS_X_FX(pokeWork->pokePos.x - FX32_CONST(work->scrollOffset)) /4;
  shadowPos.y = ACT_POS_Y_FX(pokeWork->pokePos.y)/4;
  shadowPos.z = FX32_CONST(10.0f)/4;

  G3_PushMtx();
  G3_PolygonAttr(GX_LIGHTMASK_NONE,  // no lights
           GX_POLYGONMODE_MODULATE,   // modulation mode
           GX_CULL_NONE,     // cull none
           ACT_POLYID_SHADOW, // polygon ID(0 - 63)
           4,    // alpha(0 - 31)
           0           // OR of GXPolygonAttrMisc's value
    );

//  G3_Translate( 0,0,FX32_CONST(199.0f));
  G3_Scale( FX32_ONE*4,FX32_ONE*4,FX32_ONE);
  G3_Begin(GX_BEGIN_TRIANGLES);
  {
    G3_Color(GX_RGB(0,0,0));
    for( i=0;i<0x10000;i+=add)
    {
      G3_Vtx( shadowPos.x,shadowPos.y,shadowPos.z);
      G3_Vtx( shadowPos.x + FX_Mul(FX_SinIdx((u16)i),sizeX) ,
          shadowPos.y + FX_Mul(FX_CosIdx((u16)i),sizeY) ,
          shadowPos.z);
      G3_Vtx( shadowPos.x + FX_Mul(FX_SinIdx((u16)(i+add)),sizeX) ,
          shadowPos.y + FX_Mul(FX_CosIdx((u16)(i+add)),sizeY) ,
          shadowPos.z);
      
    }
  }
  G3_End();
  G3_PopMtx(1); 
}
#else
//板ポリVer
static void STA_POKE_DrawShadow( STA_POKE_SYS *work ,  STA_POKE_WORK *pokeWork )
{
  //Updateでポケモンと一緒に座標設定してる
}

#endif

void  STA_POKE_System_SetScrollOffset( STA_POKE_SYS *work , const u16 scrOfs )
{
  u8 idx;
  work->scrollOffset = scrOfs;

  for( idx=0 ; idx<MUS_POKE_EQUIP_MAX ; idx++ )
  {
    if( work->pokeWork[idx].isEnable == TRUE )
    {
      work->pokeWork[idx].isUpdate = TRUE;
    }
  }
}


STA_POKE_WORK* STA_POKE_CreatePoke( STA_POKE_SYS *work , MUSICAL_POKE_PARAM *musPoke )
{
  u8 idx;
  u8 ePos;  //equipPos
  STA_POKE_WORK *pokeWork;
  
  for( idx=0 ; idx<MUS_POKE_EQUIP_MAX ; idx++ )
  {
    if( work->pokeWork[idx].isEnable == FALSE )
    {
      break;
    }
  }
  GF_ASSERT_MSG( idx < MUS_POKE_EQUIP_MAX , "Stage acting poke work is full!\n" );
  
  pokeWork = &work->pokeWork[idx];

  pokeWork->isEnable = TRUE;
  pokeWork->isUpdate = TRUE;
  pokeWork->isFront = TRUE;
  pokeWork->isDrawItem = FALSE;
  pokeWork->drawWork = MUS_POKE_DRAW_Add( work->drawSys , musPoke , TRUE );
  pokeWork->pokeData = MUS_POKE_DRAW_GetPokeData( pokeWork->drawWork );
  VEC_Set( &pokeWork->pokePos , 0,0,0 );
  VEC_Set( &pokeWork->posOfs , 0,0,0 );
  VEC_Set( &pokeWork->scale , FX32_ONE,FX32_ONE,FX32_ONE );
  pokeWork->rotate = 0;
//  MUS_POKE_DRAW_StartAnime( pokeWork->drawWork );
  //リソース読み込みのTCBでTRUEにされるので・・・
//  MUS_POKE_DRAW_SetShowFlg( pokeWork->drawWork , FALSE );

  for( ePos=0;ePos<MUS_POKE_EQUIP_MAX;ePos++ )
  {
    const u16 itemNo = musPoke->equip[ePos].itemNo;
    if( itemNo != MUSICAL_ITEM_INVALID )
    {
      pokeWork->itemRes[ePos] = MUS_ITEM_DRAW_LoadResource( itemNo );
      pokeWork->itemWork[ePos] = MUS_ITEM_DRAW_AddResource( work->itemDrawSys , itemNo , pokeWork->itemRes[ePos] , &pokeWork->pokePos );
      MUS_ITEM_DRAW_SetSize( work->itemDrawSys , pokeWork->itemWork[ePos] , FX16_CONST(0.25f) , FX16_CONST(0.25f) );
      //ACT_BBD_SIZE(32.0f)
    }
    else
    {
      pokeWork->itemRes[ePos] = NULL;
      pokeWork->itemWork[ePos] = NULL;
    }
  }

  //影ポリの作成
  //エッジマーキング防止のため半透明にする
  pokeWork->shadowBbdIdx = GFL_BBD_AddObject( work->bbdSys , work->shadowResIdx 
                    , FX16_CONST(1f)/4 , FX16_CONST(0.75f)/4 , 
                    &pokeWork->pokePos , 12 , GFL_BBD_LIGHT_NONE );
  return pokeWork;
}

void STA_POKE_DeletePoke( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork )
{
  u8 ePos;  //equipPos
  GFL_BBD_RemoveObject( work->bbdSys , pokeWork->shadowBbdIdx );
  for( ePos=0;ePos<MUS_POKE_EQUIP_MAX;ePos++ )
  {
    if( pokeWork->itemWork[ePos] != NULL )
    {
      MUS_ITEM_DRAW_DelItem( work->itemDrawSys , pokeWork->itemWork[ePos] );
      MUS_ITEM_DRAW_DeleteResource( pokeWork->itemRes[ePos] );

      pokeWork->itemRes[ePos] = NULL;
      pokeWork->itemWork[ePos] = NULL;
    }
  }
  MUS_POKE_DRAW_Del( work->drawSys , pokeWork->drawWork );
  pokeWork->isEnable = FALSE;
}
void STA_POKE_GetPosition( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , VecFx32 *pos )
{
  pos->x = pokeWork->pokePos.x;
  pos->y = pokeWork->pokePos.y;
  pos->z = pokeWork->pokePos.z;
}

void STA_POKE_SetPosition( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , const VecFx32 *pos )
{
  pokeWork->pokePos.x = pos->x;
  pokeWork->pokePos.y = pos->y;
  pokeWork->pokePos.z = pos->z;
  
  pokeWork->isUpdate = TRUE;
}

void STA_POKE_GetRotate( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , u16 *rotate )
{
  *rotate = pokeWork->rotate;
}

void STA_POKE_SetRotate( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , const u16 rotate )
{
  pokeWork->rotate = rotate;
  
  pokeWork->isUpdate = TRUE;
}

void STA_POKE_GetScale( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , VecFx32 *scale )
{
  scale->x = pokeWork->scale.x;
  scale->y = pokeWork->scale.y;
  scale->z = pokeWork->scale.z;
}

void STA_POKE_SetScale( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , VecFx32 *scale )
{
  pokeWork->scale.x = scale->x;
  pokeWork->scale.y = scale->y;
  pokeWork->scale.z = scale->z;
  
  pokeWork->isUpdate = TRUE;
}
void STA_POKE_GetPositionOffset( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , VecFx32 *pos )
{
  pos->x = pokeWork->posOfs.x;
  pos->y = pokeWork->posOfs.y;
  pos->z = pokeWork->posOfs.z;
}

void STA_POKE_SetPositionOffset( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , const VecFx32 *pos )
{
  pokeWork->posOfs.x = pos->x;
  pokeWork->posOfs.y = pos->y;
  pokeWork->posOfs.z = pos->z;
  
  pokeWork->isUpdate = TRUE;
}

void STA_POKE_StartAnime( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork )
{
  MUS_POKE_DRAW_StartAnime( pokeWork->drawWork );
}

void STA_POKE_StopAnime( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork )
{
  MUS_POKE_DRAW_StopAnime( pokeWork->drawWork );
}

void STA_POKE_ChangeAnime( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , const u8 anmIdx )
{
  MUS_POKE_DRAW_ChangeAnime( pokeWork->drawWork , anmIdx );
}

void STA_POKE_SetShowFlg( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , const BOOL flg )
{
  int ePos;
  MUS_POKE_DRAW_SetShowFlg( pokeWork->drawWork , flg );
  for( ePos=0;ePos<MUS_POKE_EQUIP_MAX;ePos++ )
  {
    if( pokeWork->itemWork[ePos] != NULL )
    {
      MUS_ITEM_DRAW_SetDrawEnable( work->itemDrawSys , pokeWork->itemWork[ePos] , flg );
    }
  }
  GFL_BBD_SetObjectDrawEnable( work->bbdSys , pokeWork->shadowBbdIdx , &flg );

}
BOOL STA_POKE_GetShowFlg( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork )
{
  return MUS_POKE_DRAW_GetShowFlg( pokeWork->drawWork );
}

STA_POKE_DIR STA_POKE_GetPokeDir( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork )
{
  return pokeWork->dir;
}

void STA_POKE_SetPokeDir( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , const STA_POKE_DIR dir )
{
  pokeWork->dir = dir;
  pokeWork->isUpdate = TRUE;
}

void STA_POKE_SetFrontBack( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , const BOOL isFront )
{
  pokeWork->isFront = isFront;
  MUS_POKE_DRAW_SetFrontBack( pokeWork->drawWork , pokeWork->isFront );
  pokeWork->isUpdate = TRUE;
}

void STA_POKE_SetDrawItem( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , const BOOL isDrawItem )
{
  pokeWork->isDrawItem = isDrawItem;
  pokeWork->isUpdate = TRUE;
}


