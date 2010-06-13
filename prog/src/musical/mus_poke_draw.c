//======================================================================
/**
 * @file	dressup_system.h
 * @brief	ミュージカル用 ポケモン描画
 * @author	ariizumi
 * @data	09/02/10
 */
//======================================================================

#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/pokegra.h"

#include "arc_def.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

//#include "battle/poke_mcss.h"

#include "musical/mus_poke_draw.h"
#include "musical_local.h"

#include "test/ariizumi/ari_debug.h"
//======================================================================
//	define
//======================================================================
#define MUS_POKE_DRAW_MAX (8)	//描画最大数(表裏込み

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================

//描画１匹分
struct _MUS_POKE_DRAW_WORK
{
	BOOL enable;
	BOOL isDispFront;
	MUS_MCSS_WORK	*mcss;
	MUS_MCSS_WORK	*mcssFront;
	MUS_MCSS_WORK	*mcssBack;
	
	MUS_POKE_EQUIP_DATA	equipData[MUS_POKE_EQUIP_MAX];
	VecFx32 shadowOfs;
	VecFx32 rotateOfs;
	BOOL isEnableShadow;
	BOOL isEnableRot;
};

//描画システム
struct _MUS_POKE_DRAW_SYSTEM
{
	HEAPID heapId;
	MUS_MCSS_SYS_WORK *mcssSys;
	MUS_POKE_DRAW_WORK musMcss[MUS_POKE_DRAW_MAX];
};

//======================================================================
//	proto
//======================================================================
static void	MUS_POKE_MakeMAW( const MUSICAL_POKE_PARAM *musPoke, MUS_MCSS_ADD_WORK *maw, const BOOL isBack );
static void MUS_POKE_MCSS_CallBack( const u8 pltNo , MUS_MCSS_CELL_DATA *cellData , void* work );


//--------------------------------------------------------------
//	
//--------------------------------------------------------------
//システムの初期化と開放
MUS_POKE_DRAW_SYSTEM*	MUS_POKE_DRAW_InitSystem( HEAPID heapId )
{
	int i;
	MUS_POKE_DRAW_SYSTEM *work = GFL_HEAP_AllocMemory( heapId , sizeof(MUS_POKE_DRAW_SYSTEM) );
	
	work->heapId = heapId;
	work->mcssSys = MUS_MCSS_Init( MUS_POKE_DRAW_MAX , heapId );
	for( i=0;i<MUS_POKE_DRAW_MAX;i++ )
	{
		work->musMcss[i].enable = FALSE;
	}
	
	MUS_MCSS_SetOrthoMode( work->mcssSys );
	return work;
}
void MUS_POKE_DRAW_TermSystem( MUS_POKE_DRAW_SYSTEM* work )
{
	int i;
	for( i=0;i<MUS_POKE_DRAW_MAX;i++ )
	{
		if( work->musMcss[i].enable == TRUE )
		{
			MUS_POKE_DRAW_Del( work , &work->musMcss[i] );
		}
	}
	MUS_MCSS_Exit( work->mcssSys );
	GFL_HEAP_FreeMemory( work );
}

//更新
void MUS_POKE_DRAW_UpdateSystem( MUS_POKE_DRAW_SYSTEM* work )
{
	MUS_MCSS_Main( work->mcssSys );
}
void MUS_POKE_DRAW_DrawSystem( MUS_POKE_DRAW_SYSTEM* work )
{
	MUS_MCSS_Draw( work->mcssSys , MUS_POKE_MCSS_CallBack );
}

MUS_POKE_DRAW_WORK* MUS_POKE_DRAW_Add( MUS_POKE_DRAW_SYSTEM* work , MUSICAL_POKE_PARAM *musPoke , const BOOL useBack )
{
	int i,idx;
	VecFx32 scale;
	MUS_MCSS_ADD_WORK maw;
	
	
	//空きスペースの検索
	
	for( idx=0;idx<MUS_POKE_DRAW_MAX;idx++ )
	{
		if( work->musMcss[idx].enable == FALSE )
		{
			break;
		}
	}
	GF_ASSERT_MSG( idx < MUS_POKE_DRAW_MAX , "MusicalPokemon draw max over!!\n");
	
	work->musMcss[idx].enable = TRUE;

	MUS_POKE_MakeMAW( musPoke , &maw , FALSE );
	work->musMcss[idx].mcssFront = MUS_MCSS_Add( work->mcssSys , 0,0,0,&maw , &work->musMcss[idx] , FALSE );
	if( useBack == TRUE )
	{
  	MUS_POKE_MakeMAW( musPoke , &maw , TRUE );
  	work->musMcss[idx].mcssBack = MUS_MCSS_Add( work->mcssSys , 0,0,0,&maw , &work->musMcss[idx] , FALSE );
		MUS_MCSS_SetVanishFlag( work->musMcss[idx].mcssBack );
  }
  else
  {
    work->musMcss[idx].mcssBack = NULL;
  }
  
	work->musMcss[idx].isDispFront = TRUE;
	work->musMcss[idx].mcss = work->musMcss[idx].mcssFront;
	
	VEC_Set( &scale, 
			 FX32_ONE*16, 
			 FX32_ONE*16,
			 FX32_ONE );
	for( i=0;i<MUS_POKE_EQUIP_MAX;i++ )
	{
		work->musMcss[idx].equipData[i].isEnable = FALSE;
	}
	work->musMcss[idx].isEnableShadow = FALSE;
	work->musMcss[idx].isEnableRot = FALSE;
	VEC_Set( &work->musMcss[idx].shadowOfs ,0,0,0);
	VEC_Set( &work->musMcss[idx].rotateOfs ,0,0,0);
	
	MUS_POKE_DRAW_SetScale( &work->musMcss[idx], &scale );
	MUS_MCSS_SetAnmStopFlag( work->musMcss[idx].mcss );
	
	return &work->musMcss[idx];
}

void MUS_POKE_DRAW_Del( MUS_POKE_DRAW_SYSTEM* work , MUS_POKE_DRAW_WORK *drawWork )
{
  if( drawWork->mcssBack != NULL )
  {
  	MUS_MCSS_Del( work->mcssSys , drawWork->mcssBack );
  }
	MUS_MCSS_Del( work->mcssSys , drawWork->mcssFront );
	drawWork->enable = FALSE;
}

MUS_MCSS_SYS_WORK* MUS_POKE_DRAW_GetMcssSys( MUS_POKE_DRAW_SYSTEM *work )
{
  return work->mcssSys;
}

void MUS_POKE_DRAW_SetPosition( MUS_POKE_DRAW_WORK *drawWork , VecFx32 *pos )
{
	MUS_MCSS_SetPosition( drawWork->mcss , pos );
}

void MUS_POKE_DRAW_GetPosition( MUS_POKE_DRAW_WORK *drawWork , VecFx32 *pos )
{
	MUS_MCSS_GetPosition( drawWork->mcss , pos );
}

void MUS_POKE_DRAW_SetScale( MUS_POKE_DRAW_WORK *drawWork , VecFx32 *scale )
{
	MUS_MCSS_SetScale( drawWork->mcss , scale );
}

void MUS_POKE_DRAW_GetScale( MUS_POKE_DRAW_WORK *drawWork , VecFx32 *scale )
{
	MUS_MCSS_GetScale( drawWork->mcss , scale );
}

void MUS_POKE_DRAW_SetRotation( MUS_POKE_DRAW_WORK *drawWork , u16 rot )
{
	MUS_MCSS_SetRotation( drawWork->mcss , rot );
}

void MUS_POKE_DRAW_GetRotation( MUS_POKE_DRAW_WORK *drawWork , u16 *rot )
{
	MUS_MCSS_GetRotation( drawWork->mcss , rot );
}

void MUS_POKE_DRAW_SetShowFlg( MUS_POKE_DRAW_WORK *drawWork , const BOOL flg )
{
	if( flg == TRUE )
	{
		MUS_MCSS_ResetVanishFlag( drawWork->mcss );
	}
	else
	{
		MUS_MCSS_SetVanishFlag( drawWork->mcss );
	}
}
BOOL MUS_POKE_DRAW_GetShowFlg( MUS_POKE_DRAW_WORK *drawWork )
{
	const int flg = MUS_MCSS_GetVanishFlag( drawWork->mcss );
	if( flg == MUS_MCSS_VANISH_ON )
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

void MUS_POKE_DRAW_StartAnime( MUS_POKE_DRAW_WORK *drawWork )
{
	MUS_MCSS_ResetAnmStopFlag( drawWork->mcssFront );
	if( drawWork->mcssBack != NULL )
	{
  	MUS_MCSS_ResetAnmStopFlag( drawWork->mcssBack );
  }
}

void MUS_POKE_DRAW_StopAnime( MUS_POKE_DRAW_WORK *drawWork )
{
	MUS_MCSS_SetAnmStopFlag( drawWork->mcssFront );
	if( drawWork->mcssBack != NULL )
	{
  	MUS_MCSS_SetAnmStopFlag( drawWork->mcssBack );
  }
}

void MUS_POKE_DRAW_ChangeAnime( MUS_POKE_DRAW_WORK *drawWork , const u8 anmIdx )
{
	MUS_MCSS_ChangeAnm( drawWork->mcss , anmIdx );
}

//前後の切り替え
void MUS_POKE_DRAW_FlipFrontBack( MUS_POKE_DRAW_WORK *drawWork )
{
  if( drawWork->isDispFront == TRUE )
  {
    MUS_MCSS_CopyState( drawWork->mcssFront , drawWork->mcssBack );
    drawWork->mcss = drawWork->mcssBack;
    drawWork->isDispFront = FALSE;

    MUS_MCSS_SetVanishFlag( drawWork->mcssFront );
  }
  else
  {
    MUS_MCSS_CopyState( drawWork->mcssBack , drawWork->mcssFront );
    drawWork->mcss = drawWork->mcssFront;
    drawWork->isDispFront = TRUE;

    MUS_MCSS_SetVanishFlag( drawWork->mcssBack );
  }
}

//前後を指定して切り替え
void MUS_POKE_DRAW_SetFrontBack( MUS_POKE_DRAW_WORK *drawWork , const BOOL isDispFront )
{
  if( isDispFront != drawWork->isDispFront )
  {
    MUS_POKE_DRAW_FlipFrontBack( drawWork );
  }
}

void MUS_POKE_DRAW_SetMepachiFlg( MUS_POKE_DRAW_WORK *drawWork , BOOL flg )
{
  if( flg == TRUE )
  {
    MUS_MCSS_SetMepachiFlag( drawWork->mcss );
  }
  else
  {
    MUS_MCSS_ResetMepachiFlag( drawWork->mcss );
  }
}

//MCSSシステムのテクスチャ読み込みアドレスの変更(読み込み前に！
void MUS_POKE_DRAW_SetTexAddres(  MUS_POKE_DRAW_SYSTEM* work , u32 adr )
{
	MUS_MCSS_SetTexAddres( work->mcssSys , adr );
  
}
void MUS_POKE_DRAW_SetPltAddres(  MUS_POKE_DRAW_SYSTEM* work , u32 adr )
{
	MUS_MCSS_SetPltAddres( work->mcssSys , adr );
  
}
MUS_POKE_EQUIP_DATA* MUS_POKE_DRAW_GetEquipData( MUS_POKE_DRAW_WORK *drawWork , const MUS_POKE_EQUIP_POS pos )
{
	return &drawWork->equipData[pos];
}
VecFx32 *MUS_POKE_DRAW_GetShadowOfs( MUS_POKE_DRAW_WORK *drawWork )
{
	return &drawWork->shadowOfs;
}

VecFx32 *MUS_POKE_DRAW_GetRotateOfs( MUS_POKE_DRAW_WORK *drawWork )
{
	return &drawWork->rotateOfs;
}

const VecFx32 *MUS_POKE_DRAW_GetRotateOfsMcss( MUS_POKE_DRAW_WORK *drawWork )
{
	return MUS_MCSS_GetRotOfs( drawWork->mcss );
}

//デバッグ用
BOOL* MUS_POKE_DRAW_GetEnableRotateOfs( MUS_POKE_DRAW_WORK *drawWork )
{
	return &drawWork->isEnableRot;
}

BOOL* MUS_POKE_DRAW_GetEnableShadowOfs( MUS_POKE_DRAW_WORK *drawWork )
{
	return &drawWork->isEnableShadow;
}

static void MUS_POKE_MCSS_CallBack( const u8 pltNo , MUS_MCSS_CELL_DATA *cellData , void* work )
{
	MUS_POKE_DRAW_WORK *drawWork = work;
	
	if( pltNo >= MUS_POKE_PLT_START )
	{
		const MUS_POKE_EQUIP_POS pos = MUS_POKE_PLT_TO_POS(pltNo);
		
		drawWork->equipData[pos].isEnable = TRUE;
		drawWork->equipData[pos].pos = cellData->pos;
		drawWork->equipData[pos].ofs = cellData->ofs;
		drawWork->equipData[pos].rot = cellData->rotZ;
		drawWork->equipData[pos].itemRot = cellData->itemRotZ;
		drawWork->equipData[pos].scale = cellData->scale;
		drawWork->equipData[pos].rotOfs = cellData->rotOfs;
	}
	else if( pltNo == MUS_POKE_PLT_SHADOW )
	{
		VEC_Set( &drawWork->shadowOfs , cellData->ofs.x ,cellData->ofs.y ,0 );
	  drawWork->isEnableShadow = TRUE;
	}
	else if( pltNo == MUS_POKE_PLT_ROTATE )
	{
		VEC_Set( &drawWork->rotateOfs , cellData->ofs.x ,cellData->ofs.y ,0 );
	  drawWork->isEnableRot = TRUE;
	}
	
	//	MUS_TPrintf("[%d:%d]:[%d:%d]\n"	,(int)F32_CONST(cellData->pos->x),(int)F32_CONST(cellData->pos->y)
	//									,(int)F32_CONST(cellData->ofs.x),(int)F32_CONST(cellData->ofs.y)	);
}

//ミュージカル専用ポケモンMCSS読み込み
#pragma mark [> McssFileFunc
//ポケモン一体を構成するMCSS用ファイルの構成
enum{
	MUS_POKEGRA_FRONT_M_NCGR = 0,
	MUS_POKEGRA_FRONT_F_NCGR,
	MUS_POKEGRA_FRONT_M_NCBR,
	MUS_POKEGRA_FRONT_F_NCBR,
	MUS_POKEGRA_FRONT_NCER,
	MUS_POKEGRA_FRONT_NANR,
	MUS_POKEGRA_FRONT_NMCR,
	MUS_POKEGRA_FRONT_NMAR,
	MUS_POKEGRA_FRONT_NCEC,
	MUS_POKEGRA_BACK_M_NCGR,
	MUS_POKEGRA_BACK_F_NCGR,
	MUS_POKEGRA_BACK_M_NCBR,
	MUS_POKEGRA_BACK_F_NCBR,
	MUS_POKEGRA_BACK_NCER,
	MUS_POKEGRA_BACK_NANR,
	MUS_POKEGRA_BACK_NMCR,
	MUS_POKEGRA_BACK_NMAR,
	MUS_POKEGRA_BACK_NCEC,
	MUS_POKEGRA_NORMAL_NCLR,
	MUS_POKEGRA_RARE_NCLR,

	MUS_POKEGRA_FILE_MAX,			//ポケモン一体を構成するMCSS用ファイルの総数

	MUS_POKEGRA_M_NCGR = 0,
	MUS_POKEGRA_F_NCGR,
	MUS_POKEGRA_M_NCBR,
	MUS_POKEGRA_F_NCBR,
	MUS_POKEGRA_NCER,
	MUS_POKEGRA_NANR,
	MUS_POKEGRA_NMCR,
	MUS_POKEGRA_NMAR,
	MUS_POKEGRA_NCEC
};

static void	MUS_POKE_MakeMAW( const MUSICAL_POKE_PARAM *musPoke, MUS_MCSS_ADD_WORK *maw , const BOOL isBack )
{
	const POKEMON_PASO_PARAM *ppp = PP_GetPPPPointerConst( musPoke->pokePara );
	int	mons_no = musPoke->mcssParam.monsno;
	int	form_no = POKETOOL_CheckPokeFormNo( musPoke->mcssParam.monsno , musPoke->mcssParam.form );
	int	sex		= musPoke->mcssParam.sex;
	int	rare	= musPoke->mcssParam.rare;
	BOOL egg = FALSE;
  u8 dir = 0;
  //フィールドのみのチェック
	form_no = POKETOOL_CheckPokeFormNo_Field( musPoke->mcssParam.monsno , form_no );
  if( isBack == TRUE )
  {
    dir = 1;
  }

	maw->arcID = ARCID_POKEGRA_MUS;

	maw->ncbr = POKEGRA_GetCbrArcIndex( maw->arcID , mons_no, form_no, sex, rare,  dir, egg );
	maw->nclr = POKEGRA_GetPalArcIndex( maw->arcID , mons_no, form_no, sex, rare,  dir, egg );
	maw->ncer = POKEGRA_GetCelArcIndex( maw->arcID , mons_no, form_no, sex, rare,  dir, egg );
	maw->nanr = POKEGRA_GetAnmArcIndex( maw->arcID , mons_no, form_no, sex, rare,  dir, egg );
	maw->nmcr = POKEGRA_GetMCelArcIndex( maw->arcID , mons_no, form_no, sex, rare,  dir, egg );
	maw->nmar = POKEGRA_GetMAnmArcIndex( maw->arcID , mons_no, form_no, sex, rare,  dir, egg );
	maw->ncec = POKEGRA_GetNcecArcIndex( maw->arcID , mons_no, form_no, sex, rare,  dir, egg );
	maw->rand = musPoke->mcssParam.perRand;

}


#if PM_DEBUG
void  MUS_POKE_StepAnime( MUS_POKE_DRAW_SYSTEM* work , MUS_POKE_DRAW_WORK *drawWork , fx32 frame )
{
  MUS_MCSS_StepAnime( work->mcssSys , drawWork->mcss , frame );
}
#endif	