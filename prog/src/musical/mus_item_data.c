//======================================================================
/**
 * @file  mus_item_data.h
 * @brief ミュージカル用 アイテムデータ
 * @author  ariizumi
 * @data  09/02/10
 */
//======================================================================

#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "arc_def.h"
#include "musical_item.naix"

#include "musical/mus_item_data.h"
#include "test/ariizumi/ari_debug.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  enum
//======================================================================
typedef enum
{
  EPB_EAR     = 1<<MUS_POKE_EQU_TYPE_EAR,
  EPB_HEAD    = 1<<MUS_POKE_EQU_TYPE_HEAD,
  EPB_EYE     = 1<<MUS_POKE_EQU_TYPE_EYE,
  EPB_FACE    = 1<<MUS_POKE_EQU_TYPE_FACE,
  EPB_BODY    = 1<<MUS_POKE_EQU_TYPE_BODY,
  EPB_WAIST   = 1<<MUS_POKE_EQU_TYPE_WAIST,
  EPB_HAND    = 1<<MUS_POKE_EQU_TYPE_HAND,
  EPB_IS_BACK = 1<<MUS_POKE_EQU_IS_BACK,
  EPB_IS_FRONT= 1<<MUS_POKE_EQU_IS_FRONT,
  EPB_CAN_REV = 1<<MUS_POKE_EQU_CAN_REVERSE
}EQIUP_POS_BIT;

//======================================================================
//  typedef struct
//======================================================================
//データ1個分
struct _MUS_ITEM_DATA_WORK
{
  GFL_BBD_TEXSIZ    texSize;
  s8    ofsX;
  s8    ofsY;
  u16   equipPosBit;
  u8    mainPos;
  u8    conType;
  u8    useType;
  u8    pad[1];
};

struct _MUS_ITEM_DATA_SYS
{
  void* dataBuf;
  MUS_ITEM_DATA_WORK *dataArr;
};

//======================================================================
//  proto
//======================================================================

MUS_ITEM_DATA_SYS* MUS_ITEM_DATA_InitSystem( HEAPID heapId )
{
  MUS_ITEM_DATA_SYS *sysWork;
  
  sysWork = GFL_HEAP_AllocMemory( heapId , sizeof(MUS_ITEM_DATA_SYS) );
  
  sysWork->dataBuf = GFL_ARC_UTIL_Load( ARCID_MUSICAL_ITEM , NARC_musical_item_item_data_bin , FALSE , heapId );
  sysWork->dataArr = (MUS_ITEM_DATA_WORK*)sysWork->dataBuf;
  
#if DEB_ARI&0
  
  {
    u8 i;
    for( i=0;i<33;i++ )
    {
      ARI_TPrintf("[%2d][%2d][%3d][%3d][%3d][%1d][%1d][%1d]\n",i,
          sysWork->dataArr[i].texSize,
          sysWork->dataArr[i].ofsX,
          sysWork->dataArr[i].ofsY,
          sysWork->dataArr[i].equipPosBit,
          sysWork->dataArr[i].mainPos,
          sysWork->dataArr[i].conType,
          sysWork->dataArr[i].useType);
    }
  }
  
#endif
  
  return sysWork;
  
}

void MUS_ITEM_DATA_ExitSystem( MUS_ITEM_DATA_SYS *sysWork )
{
  GFL_HEAP_FreeMemory( sysWork->dataBuf );
  GFL_HEAP_FreeMemory( sysWork );
}

MUS_ITEM_DATA_WORK* MUS_ITEM_DATA_GetMusItemData( MUS_ITEM_DATA_SYS* sysWork , const u16 itemNo )
{
  return &sysWork->dataArr[itemNo];
}

void  MUS_ITEM_DATA_GetDispOffset( MUS_ITEM_DATA_WORK*  dataWork , GFL_POINT* ofs )
{
  ofs->x = dataWork->ofsX;
  ofs->y = dataWork->ofsY;
}

GFL_BBD_TEXSIZ  MUS_ITEM_DATA_GetTexType( MUS_ITEM_DATA_WORK*  dataWork )
{
  return dataWork->texSize;
}

const BOOL  MUS_ITEM_DATA_CanEquipPos( MUS_ITEM_DATA_WORK*  dataWork , const MUS_POKE_EQUIP_POS pos )
{
  switch( pos )
  {
  case MUS_POKE_EQU_EAR_R:    //右耳
  case MUS_POKE_EQU_EAR_L:    //左耳
    if( dataWork->equipPosBit & EPB_EAR )
    {
      return TRUE;
    }
    break;
  case MUS_POKE_EQU_HEAD:     //頭
    if( dataWork->equipPosBit & EPB_HEAD )
    {
      return TRUE;
    }
    break;
  case MUS_POKE_EQU_EYE:    //目・鼻
    if( dataWork->equipPosBit & EPB_EYE )
    {
      return TRUE;
    }
    break;
  case MUS_POKE_EQU_FACE:   //顔
    if( dataWork->equipPosBit & EPB_FACE )
    {
      return TRUE;
    }
    break;
  case MUS_POKE_EQU_BODY:     //胴
    if( dataWork->equipPosBit & EPB_BODY )
    {
      return TRUE;
    }
    break;
  case MUS_POKE_EQU_WAIST:    //腰
    if( dataWork->equipPosBit & EPB_WAIST )
    {
      return TRUE;
    }
    break;
  case MUS_POKE_EQU_HAND_R:   //右手
  case MUS_POKE_EQU_HAND_L:   //左手
    if( dataWork->equipPosBit & EPB_HAND )
    {
      return TRUE;
    }
    break;
  case MUS_POKE_EQUIP_MAX:    //ドレスアップ検索用
    return FALSE;
    break;
  default:
    GF_ASSERT_MSG( NULL ,"invalid equip pos!!\n" );
    break;
  }
  return FALSE;
}

const BOOL  MUS_ITEM_DATA_CanEquipPosUserData( MUS_ITEM_DATA_WORK*  dataWork , const MUS_POKE_EQUIP_USER pos )
{
  switch( pos )
  {
  case MUS_POKE_EQU_USER_EAR_R:   //耳(右耳・左耳)
  case MUS_POKE_EQU_USER_EAR_L:   //耳(右耳・左耳)
    if( dataWork->equipPosBit & EPB_EAR )
    {
      return TRUE;
    }
    break;
    
  case MUS_POKE_EQU_USER_HEAD:    //頭
    if( dataWork->equipPosBit & EPB_HEAD )
    {
      return TRUE;
    }
    break;

  case MUS_POKE_EQU_USER_FACE:    //顔
    if( dataWork->equipPosBit & (EPB_EYE|EPB_FACE) )
    {
      return TRUE;
    }
    break;

  case MUS_POKE_EQU_USER_BODY:    //胴
    if( dataWork->equipPosBit & (EPB_BODY) )
    {
      return TRUE;
    }
    break;

  case MUS_POKE_EQU_USER_WAIST:    //胴
    if( dataWork->equipPosBit & (EPB_WAIST) )
    {
      return TRUE;
    }
    break;

  case MUS_POKE_EQU_USER_HAND_R:  //手(右手)
  case MUS_POKE_EQU_USER_HAND_L:  //手(左手)
    if( dataWork->equipPosBit & EPB_HAND )
    {
      return TRUE;
    }
    break;

  case MUS_POKE_EQUIP_USER_MAX:   //ドレスアップ検索用
    return FALSE;
    break;
  default:
    GF_ASSERT_MSG( NULL ,"invalid equip pos!!\n" );
    break;
  }
  return FALSE;
}

//指定された箇所がメインとなる装備箇所か？
const BOOL  MUS_ITEM_DATA_CheckMainPosUserData( MUS_ITEM_DATA_WORK*  dataWork , const MUS_POKE_EQUIP_USER pos )
{
  switch( pos )
  {
  case MUS_POKE_EQU_USER_EAR_R:   //耳(右耳・左耳)
  case MUS_POKE_EQU_USER_EAR_L:   //耳(右耳・左耳)
    if( dataWork->mainPos == MUS_POKE_EQU_TYPE_EAR )
    {
      return TRUE;
    }
    break;
    
  case MUS_POKE_EQU_USER_HEAD:    //頭
    if( dataWork->mainPos == MUS_POKE_EQU_TYPE_HEAD )
    {
      return TRUE;
    }
    break;

  case MUS_POKE_EQU_USER_FACE:    //顔
    if( dataWork->mainPos == MUS_POKE_EQU_TYPE_EYE ||
        dataWork->mainPos == MUS_POKE_EQU_TYPE_FACE )
    {
      return TRUE;
    }
    break;

  case MUS_POKE_EQU_USER_BODY:    //胴
    if( dataWork->mainPos == MUS_POKE_EQU_TYPE_BODY )
    {
      return TRUE;
    }
    break;

  case MUS_POKE_EQU_USER_WAIST:    //腰
    if( dataWork->mainPos == MUS_POKE_EQU_TYPE_WAIST )
    {
      return TRUE;
    }
    break;

  case MUS_POKE_EQU_USER_HAND_R:  //手(右手)
  case MUS_POKE_EQU_USER_HAND_L:  //手(左手)
    if( dataWork->mainPos == MUS_POKE_EQU_TYPE_HAND )
    {
      return TRUE;
    }
    break;

  case MUS_POKE_EQUIP_USER_MAX:   //ドレスアップ検索用
    return FALSE;
    break;
  default:
    GF_ASSERT_MSG( NULL ,"invalid equip pos!!\n" );
    break;
  }
  return FALSE;  
}

const BOOL  MUS_ITEM_DATA_IsBackItem( MUS_ITEM_DATA_WORK*  dataWork )
{
  if( dataWork->equipPosBit & EPB_IS_BACK )
  {
    return TRUE;
  }
  return FALSE;
}

const BOOL  MUS_ITEM_DATA_IsFrontItem( MUS_ITEM_DATA_WORK*  dataWork )
{
  if( dataWork->equipPosBit & EPB_IS_FRONT )
  {
    return TRUE;
  }
  return FALSE;
}

const BOOL  MUS_ITEM_DATA_CanReverseItem( MUS_ITEM_DATA_WORK*  dataWork )
{
  if( dataWork->equipPosBit & EPB_CAN_REV )
  {
    return TRUE;
  }
  return FALSE;
}

const MUS_POKE_EQUIP_USER MUS_ITEM_DATA_EquipPosToUserType( const MUS_POKE_EQUIP_USER pos )
{
  switch( pos )
  {
  case MUS_POKE_EQU_EAR_R:    //右耳
    return MUS_POKE_EQU_USER_EAR_R;
    break;
  case MUS_POKE_EQU_EAR_L:    //左耳
    return MUS_POKE_EQU_USER_EAR_L;
    break;

  case MUS_POKE_EQU_HEAD:     //頭
    return MUS_POKE_EQU_USER_HEAD;
    break;

  case MUS_POKE_EQU_EYE:    //目・鼻
  case MUS_POKE_EQU_FACE:   //顔
    return MUS_POKE_EQU_USER_FACE;
    break;

  case MUS_POKE_EQU_BODY:     //胴
    return MUS_POKE_EQU_USER_BODY;
    break;

  case MUS_POKE_EQU_WAIST:    //腰
    return MUS_POKE_EQU_USER_WAIST;
    break;

  case MUS_POKE_EQU_HAND_R:   //右手
    return MUS_POKE_EQU_USER_HAND_R;
    break;

  case MUS_POKE_EQU_HAND_L:   //左手
    return MUS_POKE_EQU_USER_HAND_L;
    break;

  case MUS_POKE_EQUIP_MAX:    //ドレスアップ検索用
    return MUS_POKE_EQUIP_USER_MAX;
    break;

  default:
    GF_ASSERT_MSG( NULL ,"invalid equip pos!!\n" );
    return MUS_POKE_EQU_USER_INVALID;
    break;
  }
}


//アイテムのコンディションタイプを取得
const MUSICAL_CONDITION_TYPE MUS_ITEM_DATA_GetItemConditionType( MUS_ITEM_DATA_SYS* sysWork , const u16 itemNo )
{
  const MUS_ITEM_DATA_WORK *data = MUS_ITEM_DATA_GetMusItemData( sysWork , itemNo );
  return data->conType;
}

//アイテムの使用タイプを取得
const MUSICAL_ITEM_USETYPE MUS_ITEM_DATA_GetItemUseType( MUS_ITEM_DATA_SYS* sysWork , const u16 itemNo )
{
  const MUS_ITEM_DATA_WORK *data = MUS_ITEM_DATA_GetMusItemData( sysWork , itemNo );
  return data->useType;
}
