//======================================================================
/**
 * @file  map_attr.h
 * @brief	マップアトリビュート
 * @author kagaya
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "field_g3d_mapper.h"

//======================================================================
//  define
//======================================================================
#define MAPATTR_ERROR (0xffffffff) ///<アトリビュートエラー

//--------------------------------------------------------------
/// MAPATTR_FLAGBIT
//--------------------------------------------------------------
typedef enum
{
  MAPATTR_FLAGBIT_ENTER = (1<<0), ///<進入可能フラグ ON=可能
  MAPATTR_FLAGBIT_WATER = (1<<1), ///<水アトリビュート ON=水
  MAPATTR_FLAGBIT_ENCOUNT = (1<<2), ///<敵出現フラグ ON=出現
  MAPATTR_FLAGBIT_FOOTMARK = (1<<3), ///<足跡表示フラグ ON=足跡有り
  MAPATTR_FLAGBIT_SPLASH = (1<<4), ///<水飛沫表示フラグ ON=水飛沫有り
  MAPATTR_FLAGBIT_REFLECT = (1<<5), ///<映り込み表示フラグ ON=映り込み有り
  MAPATTR_FLAGBIT_SHADOW = (1<<6), ///<影表示フラグ ON=影表示有り
}MAPATTR_FLAGBIT;

//======================================================================
//  struct
//======================================================================
///MAPATTR
typedef u32 MAPATTR;
///MAPATTR_VAL
typedef u16 MAPATTR_VAL;
///MAPATTR_FLAG
typedef u16 MAPATTR_FLAG;

//======================================================================
//  proto
//======================================================================
extern MAPATTR MAPATTR_GetMapAttribute(FLDMAPPER *mapper, const VecFx32 *pos );
extern MAPATTR_VAL MAPATTR_GetMapAttrValue( const MAPATTR attr );
extern MAPATTR_FLAG MAPATTR_GetMapAttrFlag( const MAPATTR attr );
