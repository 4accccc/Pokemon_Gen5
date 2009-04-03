//======================================================================
/**
 * @file	field_debug.c
 * @brief	フィールドデバッグ処理
 * @author	kagaya
 * @data	08.09.29
 */
//======================================================================
#include "field_debug.h"
#include "arc/others.naix"
#include "fldmmdl.h"

extern GAMESYS_WORK * FIELDMAP_GetGameSysWork( FIELD_MAIN_WORK *fieldWork );

//======================================================================
//	define
//======================================================================
#define DEBUG_BGFRAME (GFL_BG_FRAME2_M) //使用するBGフレーム
#define DEBUG_PANO_FONT (15) //フォントで使用するパレットNo

//======================================================================
//	typedef struct
//======================================================================

//--------------------------------------------------------------
///	FIELD_DEBUG_WORK 
//--------------------------------------------------------------
struct _TAG_FIELD_DEBUG_WORK
{
	HEAPID heapID;	//デバッグで使用するHEAPID
	u32 bgFrame;	//デバッグで使用するBG FRAME
	FIELD_MAIN_WORK *pFieldMainWork; //FIELD_MAIN_WORK*
	
	int flag_pos_print;
};

//======================================================================
//	proto
//======================================================================
static void DebugFont_Init( FIELD_DEBUG_WORK *work );
static void DebugFont_Put( u16 *screen, char c, u16 x, u16 y );
static void DebugFont_Print( u16 x, u16 y, const char *msgBuf );
static void DebugFont_ClearLine( u16 y );

static void DebugFieldPosPrint_Proc( FIELD_DEBUG_WORK *work );

//======================================================================
//	フィールドデバッグシステム
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドデバッグシステム 初期化
 * @param 	pFieldMainWork FIELD_MAIN_WORK
 * @retval	FIELD_DEBUG_WORK
 */
//--------------------------------------------------------------
FIELD_DEBUG_WORK * FIELD_DEBUG_Init(
	FIELD_MAIN_WORK *pFieldMainWork, HEAPID heapID )
{
	FIELD_DEBUG_WORK *work;
	work = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_DEBUG_WORK) );
	
	work->heapID = heapID;
	work->bgFrame = DEBUG_BGFRAME;
	work->pFieldMainWork = pFieldMainWork;
	
	{
		DebugFont_Init( work );
	}
	return( work );
}

//--------------------------------------------------------------
/**
 * フィールドデバッグシステム 削除
 * @param 	work	FIELD_DEBUG_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
void FIELD_DEBUG_Delete( FIELD_DEBUG_WORK *work )
{
	GFL_HEAP_FreeMemory( work );
}

//--------------------------------------------------------------
/**
 * フィールドデバッグシステム 常駐処理
 * @param
 * @retval
 */
//--------------------------------------------------------------
void FIELD_DEBUG_UpdateProc( FIELD_DEBUG_WORK *work )
{
	if( work->flag_pos_print == TRUE ){
		DebugFieldPosPrint_Proc( work );
	}
}

//======================================================================
//	システムフォント表示
//======================================================================
#if 0
//------------------------------------------------------------------
/**
 * @brief	指定ファイルの読み込み
 * @param	path	ファイルへのパス
 * @param	buf	読み出したデータを格納するポインタ
 * @return	int	読み込んだファイルサイズ
 */
//------------------------------------------------------------------
static void * ReadFile( const char *path, HEAPID heapID )
{
	FSFile file;
	void*  memory;

	FS_InitFile(&file);
	if (FS_OpenFile(&file, path)){

		u32	fileSize = FS_GetLength(&file);		
		memory = GFL_HEAP_AllocMemory( heapID, fileSize );

		if (memory == NULL){
			OS_Printf("no enough memory.\n");
		}else{
			if(FS_ReadFile(&file, memory, fileSize) != fileSize){
				// ファイルサイズ分読み込めていない場合
				GFL_HEAP_FreeMemory( memory );
				memory = NULL;
				OS_Printf("file reading failed.\n");
				GF_ASSERT( 0 );
			}
		}
		(void)FS_CloseFile(&file);
	}else{
		OS_Printf("FS_OpenFile(\"%s\") ... ERROR!\n",path);
		GF_ASSERT( 0 );
		memory = NULL;
	}
	return memory;
}

//------------------------------------------------------------------
/**
 * @brief	指定ファイルのサイズを返す
 * @param	path	ファイルへのパス
 * @return	int	ファイルサイズ(bytes)
 */
//------------------------------------------------------------------
static int GetFileSize(char *path)
{
  FSFile *fp;
  u32 size = 0;
  fp = OS_AllocFromMainArenaLo(sizeof(FSFile), 4);
  if(FS_OpenFile(fp, path) == TRUE){
    size = FS_GetLength(fp);
    (void)FS_CloseFile(fp);
  } else {
    SDK_ASSERT(FALSE);
  }
  return (int)size;
}

//--------------------------------------------------------------
/**
 * ファイルパスを指定してデータの読み込み
 *
 * @param	heapID	メモリ確保をするヒープID
 * @param	path	ファイルパス
 *
 * @retval	データを読み込んだアドレス
 */
//--------------------------------------------------------------
static void * LoadFile( HEAPID heapID, const char *path )
{
	void	*buf;
	buf = ReadFile(path,heapID);
	return buf;
}

//--------------------------------------------------------------
/**
 * NITRO-CHARACTERのパレットデータを展開
 * @param	mem		展開場所
 * @param	heapID	指定ヒープ領域定義
 * @param	path	ファイルパス
 * @return	パレットデータ
 * @li		pal->pRawData = パレットデータ
 * @li		pal->szByte   = サイズ
*/
//--------------------------------------------------------------
static NNSG2dPaletteData * DebugFont_OpenPalette(
		void **mem, HEAPID heapID, u32 arc_id, u32 arc_idxconst char * path )
{
	NNSG2dPaletteData * pal;
	
	*mem = GFL_ARC_LoadDataAlloc(
		ARCID_OTHERS, NARC_others_nfont_NCLR, HEAPID );
	
	if( mem == NULL ){
		GF_ASSERT( 0 );
	}
	
	if( NNS_G2dGetUnpackedPaletteData(*mem,&pal) == FALSE ){
		GF_ASSERT( 0 );
	}
	
	return pal;
}

//--------------------------------------------------------------
/**
 * NITRO-CHARACTERのキャラデータを読み込む
 *
 * @param	frmnum	BGフレーム番号
 * @param	path	ファイルパス
 * @param	offs	オフセット（キャラ単位
 *
 * @return	none
 */
//--------------------------------------------------------------
static void NTRCHR_BGCharLoad(
	u8 frmnum, const char * path, u32 offs, HEAPID heapID )
{
	void * buf;
	NNSG2dCharacterData * dat;
	
	buf = LoadFile( heapID, path );
	
	if( buf == NULL ){
		GF_ASSERT( 0 );
	}

	if( NNS_G2dGetUnpackedBGCharacterData(buf,&dat) == FALSE ){
		GF_ASSERT( 0 );
	}
	
	GFL_BG_LoadCharacter( frmnum, dat->pRawData, dat->szByte, offs );
	GFL_HEAP_FreeMemory( buf );
}
#endif

//--------------------------------------------------------------
/**
 * システムフォント表示　グラフィック初期化
 * @param	heapID HEAPID
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DebugFont_SetGraphic( HEAPID heapID )
{
	void *buf;
	
	{	//パレット
		NNSG2dPaletteData *pal;
		buf = GFL_ARC_LoadDataAlloc(
			ARCID_OTHERS, NARC_others_nfont_NCLR, heapID );
		GF_ASSERT( buf != NULL );
		
		if( NNS_G2dGetUnpackedPaletteData(buf,&pal) == FALSE ){
			GF_ASSERT( 0 );
		}
		
		GFL_BG_LoadPalette( DEBUG_BGFRAME,
			pal->pRawData, 32, DEBUG_PANO_FONT*32 );
		GFL_HEAP_FreeMemory( buf );
	}
	
	{	//キャラ
		NNSG2dCharacterData *chr;
		buf = GFL_ARC_LoadDataAlloc(
			ARCID_OTHERS, NARC_others_nfont_NCGR, heapID );
		GF_ASSERT( buf != NULL );
		
		if( NNS_G2dGetUnpackedBGCharacterData(buf,&chr) == FALSE ){
			GF_ASSERT( 0 );
		}
		GFL_BG_LoadCharacter( DEBUG_BGFRAME,
			chr->pRawData, chr->szByte, 0 );
		GFL_HEAP_FreeMemory( buf );
	}
}

//--------------------------------------------------------------
/**
 * システムフォント表示　初期化
 * @param	work	FIELD_DEBUG_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DebugFont_Init( FIELD_DEBUG_WORK *work )
{
	{	//BG Frame
		GFL_BG_BGCNT_HEADER bgcntText = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x18000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		
		GFL_BG_SetBGControl( work->bgFrame, &bgcntText, GFL_BG_MODE_TEXT );
		GFL_BG_SetVisible( work->bgFrame, VISIBLE_ON );
		GFL_BG_SetPriority( work->bgFrame, 0 );
		GFL_BG_FillCharacter( work->bgFrame, 0x00, 1, 0 );
		GFL_BG_FillScreen( work->bgFrame,
			0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( work->bgFrame );
	}
	
	DebugFont_SetGraphic( work->heapID );
}

//--------------------------------------------------------------
/**
 * システムフォント表示 一文字表示
 * @param	work	FIELD_DEBUG_WORK
 * @param	x		表示するX座標 キャラ単位
 * @param	y		表示するY座標 キャラ単位
 * @param	msgBuf	表示する文字列
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DebugFont_Put( u16 *screen, char c, u16 x, u16 y )
{
	if(c>=0x61&&c<=(0x61+6)){ //sprintf()16進英字コードを置換
		c = (c-0x61) + 0x41; //0x41 = 'A'
	}
	if(c==0x20){
		c = 0x29;
	}
	if(c=='='){
		c = 0x30+10;
	}
	if(c=='-'){   // マイナス表示が出ていなかったのでMに置換
		c = 'M';
	}
	screen[x+y*32] = (DEBUG_PANO_FONT<<12)+(c-0x2F);
}

//--------------------------------------------------------------
/**
 * システムフォント表示 表示
 * @param	work	FIELD_DEBUG_WORK
 * @param	x		表示するX座標 キャラ単位
 * @param	y		表示するY座標 キャラ単位
 * @param	msgBuf	表示する文字列
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DebugFont_Print( u16 x, u16 y, const char *msgBuf )
{
	u16 n = 0;
	u16 *screen;
	screen = GFL_BG_GetScreenBufferAdrs( DEBUG_BGFRAME );
	while( msgBuf[n] != 0 ){
		DebugFont_Put(screen,msgBuf[n],x+n,y);
		n++;
	}
	GFL_BG_LoadScreenReq( DEBUG_BGFRAME );
}

//--------------------------------------------------------------
/**
 * システムフォント表示　行クリア
 * @param	y
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DebugFont_ClearLine( u16 y )
{
	char buf[33];
	MI_CpuFill8( buf, 0x20, 32 );
	buf[32] = 0;
	DebugFont_Print( 0, y, buf );
}

//======================================================================
//	フィールドデバッグ　フィールド座標表示
//======================================================================
//--------------------------------------------------------------
/**
 * フィールド座標表示
 * @param	work	FIELD_DEBUG_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
void FIELD_DEBUG_SetPosPrint( FIELD_DEBUG_WORK *work )
{
	if( work->flag_pos_print == FALSE ){
		work->flag_pos_print = TRUE;
		GFL_BG_SetVisible( work->bgFrame, VISIBLE_ON );
	}else{
		work->flag_pos_print = FALSE;
		GFL_BG_SetVisible( work->bgFrame, VISIBLE_OFF );
	}
}

//--------------------------------------------------------------
/**
 * フィールド座標表示
 * @param	work	FIELD_DEBUG_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DebugFieldPosPrint_Proc( FIELD_DEBUG_WORK *work )
{
	GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( work->pFieldMainWork );
	PLAYER_WORK *player = GAMESYSTEM_GetMyPlayerWork( gsys );
	const VecFx32 *pos = PLAYERWORK_getPosition( player );
	
	{
		char str[256];
		DebugFont_ClearLine( 0 );
		sprintf( str, "X %d %xH GRID %d",
			FX_Whole(pos->x), pos->x, SIZE_GRID_FX32(pos->x) );
		DebugFont_Print( 0, 0, str );
		
		DebugFont_ClearLine( 1 );
		sprintf( str, "Y %d %dH GRID %d",
			FX_Whole(pos->y), pos->y, SIZE_GRID_FX32(pos->y) );
		DebugFont_Print( 0, 1, str );
		
		DebugFont_ClearLine( 2 );
		sprintf( str, "Z %d %dH GRID %d",
			FX_Whole(pos->z), pos->z, SIZE_GRID_FX32(pos->z) );
		DebugFont_Print( 0, 2, str );
	}
}
