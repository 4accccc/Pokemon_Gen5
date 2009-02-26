//======================================================================
/**
 *
 * @file	dlplay_disp_func
 * @brief	ダウンロード親機の表示周り
 * @author	ariizumi
 * @data	08/10/31
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "arc_def.h"

#include "test/ariizumi/ari_debug.h"
#include "dlplay_comm_func.h"
#include "dlplay_disp_sys.h"

#include "pokeicon/pokeicon.h"

//======================================================================
//	define
//======================================================================
#define DLPLAY_DISPSYS_CELL_MAX (BOX_MONS_NUM)	//セルの表示数

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================
typedef struct
{
	u32 pltIdx;
	u32 ncgIdx[BOX_MONS_NUM];
	u32 anmIdx;
}DLPLAY_CELL_RES;

struct _DLPLAY_DISP_SYS
{ 
	int		heapID_;
	u8		anmCnt_;
	BOOL	isInit_;
	BOOL	isInitBox_;	//一回BOXの表示をしているか？

	GFL_CLUNIT	*cellUnit_;
	GFL_CLWK	*cellBox_[BOX_MONS_NUM];
	
	GFL_TCB		*vblankTcb_;
	BOOL		isUseBoxData_[BOX_MONS_NUM];

	DLPLAY_CELL_RES	resCell_;
};

//======================================================================
//	proto
//======================================================================
DLPLAY_DISP_SYS*	DLPlayDispSys_InitSystem( int heapID , const GFL_DISP_VRAM *vramSet );
void	DLPlayDispSys_TermSystem( DLPLAY_DISP_SYS *dispSys );
void	DLPlayDispSys_UpdateDraw( DLPLAY_DISP_SYS *dispSys );
void	DLPlayDispSys_DispBoxIcon( DLPLAY_BOX_INDEX *boxData , u8 trayNo , DLPLAY_DISP_SYS *dispSys );

static	void	DLPlaySys_UpdateDraw( GFL_TCB *tcb, void *work );
static	void	DLPlayDispSys_InitObj( DLPLAY_DISP_SYS *dispSys , const GFL_DISP_VRAM *vramSet );
static	void	DLPlayDispSys_TermObj( DLPLAY_DISP_SYS *dispSys );
static	void	DLPlayDispSys_InitBoxIcon( DLPLAY_BOX_INDEX *boxData , u8 trayNo , DLPLAY_DISP_SYS *dispSys );
static	void	DLPlayDispSys_TermBoxIcon( DLPLAY_DISP_SYS *dispSys );
static	u8	DLPlayDispSys_SetPokemonImgProxy( HEAPID heapID ,const u16 pokeNo , const u8 formNo , const u8 isEgg , ARCHANDLE *p_handle, u32 *resIdx );

//======================================================================
DLPLAY_DISP_SYS*	DLPlayDispSys_InitSystem( int heapID , const GFL_DISP_VRAM *vramBank )
{
	u8	i;
	DLPLAY_DISP_SYS *dispSys;
	dispSys = GFL_HEAP_AllocClearMemory( heapID , sizeof( DLPLAY_DISP_SYS ));
	dispSys->heapID_ = heapID;
	dispSys->anmCnt_ = 0;
	//dispSys->isInit_	= FALSE;	//InitObjでTRUEにするから初期化はいいか・・・
	dispSys->isInitBox_ = FALSE;
	for( i=0;i<BOX_MONS_NUM;i++ )
	{
		dispSys->isUseBoxData_[i] = FALSE;
		dispSys->resCell_.ncgIdx[i] = GFL_CLGRP_REGISTER_FAILED;
	}

	DLPlayDispSys_InitObj( dispSys ,vramBank );
	
	dispSys->vblankTcb_ = GFUser_VIntr_CreateTCB( DLPlaySys_UpdateDraw , (void*)dispSys , 0 );
	
	return dispSys;
}

void	DLPlayDispSys_TermSystem( DLPLAY_DISP_SYS *dispSys )
{
	DLPlayDispSys_TermObj( dispSys );
	GFL_TCB_DeleteTask( dispSys->vblankTcb_ );
	GFL_HEAP_FreeMemory( dispSys );
}

void	DLPlayDispSys_UpdateDraw( DLPLAY_DISP_SYS *dispSys )
{
	if( dispSys->isInit_ == TRUE )
	{
		u8 i;
		const u8 anmSpd = 30;
		const u8 anmFrm = dispSys->anmCnt_/anmSpd;
		dispSys->anmCnt_++;
		if( dispSys->anmCnt_ >= anmSpd*2 )
		{
			dispSys->anmCnt_ = 0;
		}
		for( i=0;i<BOX_MONS_NUM;i++ )
		{
			if( dispSys->isUseBoxData_[i] == TRUE )
			{
				//アニメはオートアニメでさせるように変更
			//	GFL_CLACT_WK_SetAnmSeqDiff( dispSys->cellBox_[i] , anmFrm );
			}
		}

		GFL_CLACT_SYS_Main();
	}
}

static void DLPlaySys_UpdateDraw( GFL_TCB *tcb, void *work )
{
	GFL_CLACT_SYS_VBlankFunc();
}

//======================================================================
//	Cell周り初期化
//======================================================================
static	void	DLPlayDispSys_InitObj( DLPLAY_DISP_SYS *dispSys , const GFL_DISP_VRAM *vramBank )
{
	GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
	cellSysInitData.oamst_main = 0x10;	//通信アイコンの分
	cellSysInitData.oamnum_main = 128-0x10;

	GFL_CLACT_SYS_Create( &cellSysInitData , vramBank , dispSys->heapID_ );
	dispSys->cellUnit_  = GFL_CLACT_UNIT_Create( DLPLAY_DISPSYS_CELL_MAX , 0,dispSys->heapID_ );
	GFL_CLACT_UNIT_SetDefaultRend( dispSys->cellUnit_ );

	dispSys->isInit_ = TRUE;
}

static	void	DLPlayDispSys_TermObj( DLPLAY_DISP_SYS *dispSys )
{
	u8 i;
	if( dispSys->isInit_ == TRUE )
	{
		if( dispSys->isInitBox_ == TRUE )
		{
			DLPlayDispSys_TermBoxIcon( dispSys );
			GFL_CLGRP_PLTT_Release( dispSys->resCell_.pltIdx );
			GFL_CLGRP_CELLANIM_Release( dispSys->resCell_.anmIdx );
		}
		
		GFL_CLACT_UNIT_Delete( dispSys->cellUnit_ );
		GFL_CLACT_SYS_Delete();

		dispSys->isInit_ = FALSE;
	}

}

void	DLPlayDispSys_DispBoxIcon( DLPLAY_BOX_INDEX *boxData , u8 trayNo , DLPLAY_DISP_SYS *dispSys )
{
	if( dispSys->isInitBox_ == TRUE )
	{
		DLPlayDispSys_TermBoxIcon( dispSys );
	}
	DLPlayDispSys_InitBoxIcon( boxData , trayNo , dispSys );

	dispSys->isInitBox_ = TRUE;
}

static	void	DLPlayDispSys_InitBoxIcon( DLPLAY_BOX_INDEX *boxData , u8 trayNo , DLPLAY_DISP_SYS *dispSys )
{
	//パレット・セルファイルを共通で用意し使う
	//キャラクタファイルのみ読み替え
	u8 i;
	u8 loadNum;
	
	ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_POKEICON , dispSys->heapID_ );

	if( dispSys->isInitBox_ == FALSE )
	{
		dispSys->resCell_.pltIdx = GFL_CLGRP_PLTT_RegisterComp( p_handle , POKEICON_GetPalArcIndex() , CLSYS_DRAW_MAIN , 0 , dispSys->heapID_ );
		dispSys->resCell_.anmIdx = GFL_CLGRP_CELLANIM_Register( p_handle , POKEICON_GetCellArcIndex() , POKEICON_GetAnmArcIndex(), dispSys->heapID_ );
	}
	
	loadNum = 0;
	for( i=0;i<BOX_MONS_NUM;i++ )
	{
		int	fileNo;
		if( boxData->pokeData_[trayNo][i].pokeNo_ != 0 )	//ポケモンがいるかのチェック
		{
			GFL_CLWK_DATA	cellInitData;
			static const u8	iconSize = 24;
			static const u8 iconTop = 16;
			static const u8 iconLeft = 24;
			u8	pltNo;

			fileNo = 495;	//テスト(egg
			pltNo = DLPlayDispSys_SetPokemonImgProxy( dispSys->heapID_ , 
						boxData->pokeData_[trayNo][i].pokeNo_ , boxData->pokeData_[trayNo][i].form_ , 
						boxData->pokeData_[trayNo][i].isEgg_ , p_handle , &dispSys->resCell_.ncgIdx[loadNum] );


			cellInitData.pos_x = (i%6) * iconSize + iconLeft;
			cellInitData.pos_y = (i/6) * iconSize + iconTop;
			cellInitData.anmseq = POKEICON_ANM_HPMAX;
			cellInitData.softpri = 0;
			cellInitData.bgpri = 0;

			dispSys->cellBox_[i] = GFL_CLACT_WK_Create( dispSys->cellUnit_ , dispSys->resCell_.ncgIdx[loadNum] ,
											dispSys->resCell_.pltIdx , dispSys->resCell_.anmIdx ,
											&cellInitData ,CLWK_SETSF_NONE , dispSys->heapID_ );

			GFL_CLACT_WK_SetAutoAnmSpeed( dispSys->cellBox_[i], FX32_ONE );
			GFL_CLACT_WK_SetAutoAnmFlag( dispSys->cellBox_[i], TRUE );
			
			GFL_CLACT_WK_SetPlttOffs( dispSys->cellBox_[i] , pltNo, CLWK_PLTTOFFS_MODE_OAM_COLOR );
			
			loadNum++;
			dispSys->isUseBoxData_[i] = TRUE;
		}
	}
	GFL_ARC_CloseDataHandle( p_handle );
}
 
static	void	DLPlayDispSys_TermBoxIcon( DLPLAY_DISP_SYS *dispSys )
{
	u8 i;
	for( i=0;i<BOX_MONS_NUM;i++ )
	{
		if( dispSys->isUseBoxData_[i] == TRUE )
		{
			GFL_CLACT_WK_Remove( dispSys->cellBox_[i] );
			dispSys->isUseBoxData_[i] = FALSE;
		}
		if( dispSys->resCell_.ncgIdx[i] != GFL_CLGRP_REGISTER_FAILED )
		{
			GFL_CLGRP_CGR_Release( dispSys->resCell_.ncgIdx[i] );
			dispSys->resCell_.ncgIdx[i] = GFL_CLGRP_REGISTER_FAILED;
		}
	}
	//ごみが出るのを防ぐため一回OAMをクリア
	//本来は1ループ間を空けるべき
	GFL_CLACT_SYS_ClearAllOamBuffer();
}

//ポケモンの番号とフォルムからファイル名を識別し、イメージプロキシを設定。
//戻り値はパレット番号
static	u8	DLPlayDispSys_SetPokemonImgProxy( HEAPID heapID ,const u16 pokeNo , const u8 formNo , const u8 isEgg , ARCHANDLE *p_handle, u32 *resIdx )
{
	const	ARCID arcID = ARCID_POKEICON;
	ARCDATID datID;
	const	BOOL compressedFlag = FALSE;
	const	CHAR_MAPPING_TYPE mapType = CHAR_MAP_1D;
	const	CLSYS_DRAW_TYPE vramType = CLSYS_DRAW_MAIN;
	void* arcData;
	
	datID = POKEICON_GetCgxArcIndexByMonsNumber( pokeNo , formNo , isEgg );	

	*resIdx = GFL_CLGRP_CGR_Register( p_handle , datID , FALSE , vramType , heapID );

	return POKEICON_GetPalNum(pokeNo, formNo, isEgg);
}

