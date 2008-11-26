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
struct _DLPLAY_DISP_SYS
{ 
	int		heapID_;
	u8		anmCnt_;
	BOOL	isInit_;
	BOOL	isInitBox_;	//一回BOXの表示をしているか？

	GFL_CLUNIT	*cellUnit_;
	GFL_CLWK	*cellBox_[BOX_MONS_NUM];
	BOOL		isUseBoxData_[BOX_MONS_NUM];

	NNSG2dImagePaletteProxy	boxPltProxy_;
	NNSG2dCellDataBank	*boxCellData_;
	NNSG2dAnimBankData	*boxAnmData_;
	void	*boxCellRes_;
	void	*boxAnmRes_;
};

//======================================================================
//	proto
//======================================================================
DLPLAY_DISP_SYS*	DLPlayDispSys_InitSystem( int heapID );
void	DLPlayDispSys_TermSystem( DLPLAY_DISP_SYS *dispSys );
void	DLPlayDispSys_UpdateDraw( DLPLAY_DISP_SYS *dispSys );
void	DLPlayDispSys_DispBoxIcon( DLPLAY_BOX_INDEX *boxData , u8 trayNo , DLPLAY_DISP_SYS *dispSys );

static	void	DLPlayDispSys_InitObj( DLPLAY_DISP_SYS *dispSys );
static	void	DLPlayDispSys_TermObj( DLPLAY_DISP_SYS *dispSys );
static	void	DLPlayDispSys_InitBoxIcon( DLPLAY_BOX_INDEX *boxData , u8 trayNo , DLPLAY_DISP_SYS *dispSys );
static	void	DLPlayDispSys_TermBoxIcon( DLPLAY_DISP_SYS *dispSys );
static	u8		DLPlayDispSys_SetPokemonImgProxy( NNSG2dImageProxy *imgProxy , u32 offs , HEAPID heapID , const u16 pokeNo , const u8 formNo ,const u8 isEgg);

//======================================================================
DLPLAY_DISP_SYS*	DLPlayDispSys_InitSystem( int heapID )
{
	u8	i;
	DLPLAY_DISP_SYS *dispSys;
	dispSys = GFL_HEAP_AllocClearMemory( heapID , sizeof( DLPLAY_DISP_SYS ));
	dispSys->heapID_ = heapID;
	dispSys->anmCnt_ = 0;
	//dispSys->isInit_	= FALSE;	//InitObjでTRUEにするから初期化はいいか・・・
	dispSys->isInitBox_ = FALSE;
	dispSys->boxCellRes_ = NULL;
	dispSys->boxAnmRes_ = NULL;
	for( i=0;i<BOX_MONS_NUM;i++ )
	{
		dispSys->isUseBoxData_[i] = FALSE;
	}

	DLPlayDispSys_InitObj( dispSys );

	return dispSys;
}

void	DLPlayDispSys_TermSystem( DLPLAY_DISP_SYS *dispSys )
{
	DLPlayDispSys_TermObj( dispSys );
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
				GFL_CLACT_WK_SetAnmSeqDiff( dispSys->cellBox_[i] , anmFrm );
			}
		}

		GFL_CLACT_UNIT_Draw( dispSys->cellUnit_ );
		GFL_CLACT_Main();
	}
}

//======================================================================
//	Cell周り初期化
//======================================================================
static	void	DLPlayDispSys_InitObj( DLPLAY_DISP_SYS *dispSys )
{
	GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
	cellSysInitData.oamst_main = 0x10;	//通信アイコンの分
	cellSysInitData.oamnum_main = 128-0x10;

	GFL_CLACT_Init( &cellSysInitData , dispSys->heapID_ );
	dispSys->cellUnit_  = GFL_CLACT_UNIT_Create( DLPLAY_DISPSYS_CELL_MAX , dispSys->heapID_ );
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
			GFL_HEAP_FreeMemory( dispSys->boxCellRes_ );
			GFL_HEAP_FreeMemory( dispSys->boxAnmRes_ );
		}
		
		GFL_CLACT_UNIT_Delete( dispSys->cellUnit_ );
		GFL_CLACT_Exit();

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
	NNSG2dImageProxy	imgProxy;
	
	if( dispSys->isInitBox_ == FALSE )
	{
		NNS_G2dInitImagePaletteProxy( &dispSys->boxPltProxy_ );
		GFL_ARC_UTIL_TransVramPaletteMakeProxy( ARCID_POKEICON , POKEICON_GetPalArcIndex() , 
				NNS_G2D_VRAM_TYPE_2DMAIN , 0 , dispSys->heapID_ , &dispSys->boxPltProxy_ );
		
		dispSys->boxCellRes_ = GFL_ARC_UTIL_LoadCellBank( ARCID_POKEICON , POKEICON_GetAnmCellArcIndex() , 
					FALSE , &dispSys->boxCellData_ , dispSys->heapID_ );
	
		dispSys->boxAnmRes_ = GFL_ARC_UTIL_LoadAnimeBank( ARCID_POKEICON , POKEICON_GetAnmCellAnmArcIndex() ,
					FALSE , &dispSys->boxAnmData_ , dispSys->heapID_ );
	}
	
	loadNum = 0;
	for( i=0;i<BOX_MONS_NUM;i++ )
	{
		int	fileNo;
		if( boxData->pokeData_[trayNo][i].pokeNo_ != 0 )	//ポケモンがいるかのチェック
		{
			GFL_CLWK_RES	cellRes;
			GFL_CLWK_DATA	cellInitData;
			static const u8	iconSize = 24;
			static const u8 iconTop = 16;
			static const u8 iconLeft = 24;
			u8	pltNo;

			fileNo = 495;	//テスト(egg
			NNS_G2dInitImageProxy( &imgProxy );
			pltNo = DLPlayDispSys_SetPokemonImgProxy( &imgProxy , 0x800+(loadNum*0x400) , dispSys->heapID_ , 
						boxData->pokeData_[trayNo][i].pokeNo_ , boxData->pokeData_[trayNo][i].form_ , 
						boxData->pokeData_[trayNo][i].isEgg_ );

						
/*
			GFL_ARC_UTIL_TransVramCharacterMakeProxy( ARCID_POKEICON , fileNo , FALSE ,
						CHAR_MAP_1D , 0 , NNS_G2D_VRAM_TYPE_2DMAIN , 0x1000+(loadNum*0x400) ,
						dispSys->heapID_ , &imgProxy );
*/
			GFL_CLACT_WK_SetCellResData( &cellRes , &imgProxy , &dispSys->boxPltProxy_ ,
						dispSys->boxCellData_ , dispSys->boxAnmData_ );

			cellInitData.pos_x = (i%6) * iconSize + iconLeft;
			cellInitData.pos_y = (i/6) * iconSize + iconTop;
			cellInitData.anmseq = 0;
			cellInitData.softpri = 0;
			cellInitData.bgpri = 0;

			dispSys->cellBox_[i] = GFL_CLACT_WK_Add( dispSys->cellUnit_ , &cellInitData ,
						&cellRes , CLWK_SETSF_NONE , dispSys->heapID_ );

			GFL_CLACT_WK_SetPlttOffs( dispSys->cellBox_[i] , pltNo );
			
			loadNum++;
			dispSys->isUseBoxData_[i] = TRUE;
		}
	}
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
	}
}

//ポケモンの番号とフォルムからファイル名を識別し、イメージプロキシを設定。
//戻り値はパレット番号
static	u8	DLPlayDispSys_SetPokemonImgProxy( NNSG2dImageProxy *imgProxy , u32 offs , HEAPID heapID ,const u16 pokeNo , const u8 formNo , const u8 isEgg )
{
	const	ARCID arcID = ARCID_POKEICON;
	ARCDATID datID;
	const	BOOL compressedFlag = FALSE;
	const	CHAR_MAPPING_TYPE mapType = CHAR_MAP_1D;
	const	NNS_G2D_VRAM_TYPE vramType = NNS_G2D_VRAM_TYPE_2DMAIN;
	void* arcData;
	
	datID = POKEICON_GetCgxArcIndexByMonsNumber( pokeNo , formNo , isEgg );	

	arcData = GFL_ARC_UTIL_Load( arcID, datID, compressedFlag, GetHeapLowID(heapID) );

	if( arcData != NULL )
	{
		NNSG2dCharacterData* charData;

		if( NNS_G2dGetUnpackedCharacterData( arcData, &charData ) )
		{
			NNS_G2dLoadImage1DMapping( charData, offs, vramType, imgProxy );
		}
		GFL_HEAP_FreeMemory( arcData );
	}
	return POKEICON_GetPalNum(pokeNo, formNo, isEgg);
}

