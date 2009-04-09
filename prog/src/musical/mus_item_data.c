//======================================================================
/**
 * @file	mus_item_data.h
 * @brief	ミュージカル用 アイテムデータ
 * @author	ariizumi
 * @data	09/02/10
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
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================
typedef enum
{
	EPB_HEAD	= 1<<MUS_POKE_EQU_TYPE_HEAD,
	EPB_EAR		= 1<<MUS_POKE_EQU_TYPE_EAR,
	EPB_BODY	= 1<<MUS_POKE_EQU_TYPE_BODY,
	EPB_WAIST	= 1<<MUS_POKE_EQU_TYPE_WAIST,
	EPB_HAND	= 1<<MUS_POKE_EQU_TYPE_HAND,
}EQIUP_POS_BIT;

//======================================================================
//	typedef struct
//======================================================================
//データ1個分
struct _MUS_ITEM_DATA_WORK
{
	GFL_BBD_TEXSIZ		texSize;
	s8		ofsX;
	s8		ofsY;
	u16		equipPosBit;
};

struct _MUS_ITEM_DATA_SYS
{
	void*	dataBuf;
	MUS_ITEM_DATA_WORK *dataArr;
};

//======================================================================
//	proto
//======================================================================

MUS_ITEM_DATA_SYS* MUS_ITEM_DATA_InitSystem( HEAPID heapId )
{
	MUS_ITEM_DATA_SYS *sysWork;
	
	sysWork = GFL_HEAP_AllocMemory( heapId , sizeof(MUS_ITEM_DATA_SYS) );
	
	sysWork->dataBuf = GFL_ARC_UTIL_Load( ARCID_MUSICAL_ITEM , NARC_musical_item_item_data_bin , FALSE , heapId );
	sysWork->dataArr = (MUS_ITEM_DATA_WORK*)sysWork->dataBuf;
	
#if DEB_ARI
	
	{
		u8 i;
		for( i=0;i<33;i++ )
		{
			OS_TPrintf("[%2d][%2d][%3d][%3d][%3d]\n",i,
					sysWork->dataArr[i].texSize,
					sysWork->dataArr[i].ofsX,
					sysWork->dataArr[i].ofsY,
					sysWork->dataArr[i].equipPosBit);
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

MUS_ITEM_DATA_WORK* MUS_ITEM_DATA_LoadMusItemData( MUS_ITEM_DATA_SYS* sysWork , const u16 itemNo , HEAPID heapId )
{
	const u8 tempNo = itemNo%33;
	
	return &sysWork->dataArr[tempNo];
}

void	MUS_ITEM_DATA_GetDispOffset( MUS_ITEM_DATA_WORK*  dataWork , GFL_POINT* ofs )
{
	ofs->x = dataWork->ofsX;
	ofs->y = dataWork->ofsY;
}

GFL_BBD_TEXSIZ	MUS_ITEM_DATA_GetTexType( MUS_ITEM_DATA_WORK*  dataWork )
{
	return dataWork->texSize;
}

const BOOL	MUS_ITEM_DATA_CanEquipPos( MUS_ITEM_DATA_WORK*  dataWork , const MUS_POKE_EQUIP_POS pos )
{
	switch( pos )
	{
	case MUS_POKE_EQU_HEAD:			//頭
		if( dataWork->equipPosBit & EPB_HEAD )
		{
			return TRUE;
		}
		break;
	case MUS_POKE_EQU_EAR_R:		//右耳
	case MUS_POKE_EQU_EAR_L:		//左耳
		if( dataWork->equipPosBit & EPB_EAR )
		{
			return TRUE;
		}
		break;
	case MUS_POKE_EQU_BODY:			//胴
		if( dataWork->equipPosBit & EPB_BODY )
		{
			return TRUE;
		}
		break;
	case MUS_POKE_EQU_WAIST:		//腰
		if( dataWork->equipPosBit & EPB_WAIST )
		{
			return TRUE;
		}
		break;
	case MUS_POKE_EQU_HAND_R:		//右手
	case MUS_POKE_EQU_HAND_L:		//左手
		if( dataWork->equipPosBit & EPB_HAND )
		{
			return TRUE;
		}
		break;
	case MUS_POKE_EQU_TAIL:			//尻尾
		break;
	default:
		GF_ASSERT_MSG( NULL ,"invalid equip pos!!\n" );
		break;
	}
	return FALSE;
}
