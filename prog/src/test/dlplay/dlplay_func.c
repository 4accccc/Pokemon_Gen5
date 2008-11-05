//======================================================================
/**
 *
 * @file	dlplay_func.c
 * @brief	ダウンロード用機能関数
 * @author	ariizumi
 * @data	08/10/20
 */
//======================================================================
#include <gflib.h>
#include <textprint.h>

#include "system/gfl_use.h"
#include "system/main.h"

#include "dlplay_func.h"
#include "../ariizumi/ari_debug.h"
//======================================================================
//	define
//======================================================================

#define DLPLAY_FUNC_MSG_X (16)
#define DLPLAY_FUNC_MSG_LINE_NUM (24)
#define DLPLAY_FUNC_MSG_LINE_MAX (20)
//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================
struct _DLPLAY_MSG_SYS
{
	u8	line_;
	u8	bgPlane_;

	GFL_BMPWIN			*bmpwin_[DLPLAY_FUNC_MSG_LINE_NUM ];
	GFL_TEXT_PRINTPARAM	*textParam_[DLPLAY_FUNC_MSG_LINE_NUM ];
};


//======================================================================
//	proto
//======================================================================
DLPLAY_MSG_SYS*	DLPlayFunc_MsgInit( int	heapID , u8 bgPlane );
void	DLPlayFunc_MsgTerm( DLPLAY_MSG_SYS *msgSys );

void DLPlayFunc_PutString( char* str , DLPLAY_MSG_SYS *msgSys);
void DLPlayFunc_PutStringLine( u8 line , char* str , DLPLAY_MSG_SYS *msgSys );
void DLPlayFunc_ClearString( DLPLAY_MSG_SYS *msgSys );

void DLPlayFunc_MsgUpdate( DLPLAY_MSG_SYS *msgSys , const u8 line , const BOOL isRefresh );

const u16 DLPlayFunc_DPTStrCode_To_UTF16( const u16 *dptStr , u16* utfStr , const u16 len );

//背景bg文字変換用
void DLPlayFunc_ChangeBgMsg( u8 msgIdx , u8 plane );

//セーブ処理用ウェイト関数
void DLPlayFunc_InitCounter( u16 *cnt );
const BOOL DLPlayFunc_UpdateCounter( u16 *cnt , const u16 time );

//======================================================================
//	DLプレイメッセージシステム初期化
//======================================================================
DLPLAY_MSG_SYS*	DLPlayFunc_MsgInit( int	heapID , u8 bgPlane )
{
	DLPLAY_MSG_SYS *msgSys;
	u8 i;

	msgSys = GFL_HEAP_AllocClearMemory( heapID , sizeof( DLPLAY_MSG_SYS ) );
	msgSys->bgPlane_ = bgPlane;
	msgSys->line_ = 0;

	//フォントパレット作成＆転送
	{
		const u16 G2D_BACKGROUND_COL = 0x0000;
		const u16 G2D_FONT_COL = 0x7fff;
		const u16 G2D_FONTSELECT_COL = 0x001f;

		u16* plt = GFL_HEAP_AllocClearMemoryLo( heapID, 16*2 );
		plt[0] = G2D_BACKGROUND_COL;
		plt[1] = G2D_FONT_COL;
		GFL_BG_LoadPalette( bgPlane, plt, 16*2, 0 );
		plt[1] = G2D_FONTSELECT_COL;
		GFL_BG_LoadPalette( bgPlane, plt, 16*2, 16*2 );
	
		GFL_HEAP_FreeMemory( plt );
	}
	//ビットマップ作成
	for( i=0;i<DLPLAY_FUNC_MSG_LINE_NUM;i++ ){
		msgSys->bmpwin_[i]	= GFL_BMPWIN_Create( bgPlane , 0, i, 32, 1, 0, 
												GFL_BG_CHRAREA_GET_F );
	}

	{
		GFL_TEXT_PRINTPARAM defaultParam;

		defaultParam.writex	= DLPLAY_FUNC_MSG_X;
		defaultParam.writey	= 0;
		defaultParam.spacex	= 1;
		defaultParam.spacey	= 1;
		defaultParam.colorF	= 1;
		defaultParam.colorB	= 0;
		defaultParam.mode	= GFL_TEXT_WRITE_16;
		for( i=0;i<DLPLAY_FUNC_MSG_LINE_NUM;i++ ){
			//テキストをビットマップに表示
			msgSys->textParam_[i] = GFL_HEAP_AllocClearMemory( heapID , sizeof( GFL_TEXT_PRINTPARAM )); 
			*msgSys->textParam_[i] = defaultParam;
			msgSys->textParam_[i]->bmp	= GFL_BMPWIN_GetBmp( msgSys->bmpwin_[i] );
			
			//DLPlayFunc_MsgUpdate( msgSys , i , (i==DLPLAY_FUNC_MSG_LINE_NUM-1?TRUE:FALSE));
		}
	}


	return msgSys;
}

//======================================================================
//	DLプレイメッセージシステム開放
//======================================================================
void	DLPlayFunc_MsgTerm( DLPLAY_MSG_SYS *msgSys )
{
	u8 i;
	for( i=0;i<DLPLAY_FUNC_MSG_LINE_NUM;i++ ){
		GFL_BMPWIN_Delete( msgSys->bmpwin_[i] );
	}
}

//======================================================================
//	DLプレイメッセージシステム描画

//======================================================================
void DLPlayFunc_PutString( char* str , DLPLAY_MSG_SYS *msgSys)
{
	ARI_TPrintf("MsgPut:[%s]\n",str);
	//GFL_BMP_Clear( msgSys->textParam_[msgSys->line_]->bmp , 0x0000 );

	msgSys->textParam_[msgSys->line_]->writex	= DLPLAY_FUNC_MSG_X;
	msgSys->textParam_[msgSys->line_]->writey	= 0;
	GFL_TEXT_PrintSjisCode( str , msgSys->textParam_[msgSys->line_] );
	DLPlayFunc_MsgUpdate( msgSys , msgSys->line_ , FALSE );
	
	msgSys->line_++;
	if( msgSys->line_ >= DLPLAY_FUNC_MSG_LINE_MAX )
		msgSys->line_ = 0;
	//次のラインをクリアしておく:
	//
	GFL_BMP_Clear( msgSys->textParam_[msgSys->line_]->bmp , 0x0000 );
	DLPlayFunc_MsgUpdate( msgSys , msgSys->line_ , TRUE );
	
}

void DLPlayFunc_PutStringLine( u8 line , char* str , DLPLAY_MSG_SYS *msgSys )
{
	ARI_TPrintf("MsgPut:[%d][%s]\n",line,str);
	GFL_BMP_Clear( msgSys->textParam_[line]->bmp , 0x0000 );
	DLPlayFunc_MsgUpdate( msgSys , line , FALSE );
	msgSys->textParam_[line]->writex	= DLPLAY_FUNC_MSG_X;
	msgSys->textParam_[line]->writey	= 0;
	GFL_TEXT_PrintSjisCode( str , msgSys->textParam_[line] );
	DLPlayFunc_MsgUpdate( msgSys , line , TRUE );
}

void DLPlayFunc_PutStringLineDiv( u8 line , char* str1 , char* str2 , DLPLAY_MSG_SYS *msgSys )
{
	ARI_TPrintf("MsgPut:[%d][%32s][%32s]\n",line,str1,str2);
	GFL_BMP_Clear( msgSys->textParam_[line]->bmp , 0x0000 );
	DLPlayFunc_MsgUpdate( msgSys , line , FALSE );
	msgSys->textParam_[line]->writex	= DLPLAY_FUNC_MSG_X;
	msgSys->textParam_[line]->writey	= 0;
	GFL_TEXT_PrintSjisCode( str1 , msgSys->textParam_[line] );
	msgSys->textParam_[line]->writex	= DLPLAY_FUNC_MSG_X+128;
	GFL_TEXT_PrintSjisCode( str2 , msgSys->textParam_[line] );
	DLPlayFunc_MsgUpdate( msgSys , line , TRUE );
}

void DLPlayFunc_ClearString( DLPLAY_MSG_SYS *msgSys )
{
	u8 i;
	for(i=0;i<DLPLAY_FUNC_MSG_LINE_NUM;i++)
		GFL_BMP_Clear( msgSys->textParam_[i]->bmp , 0x0000 );
}

//======================================================================
//	DLプレイメッセージシステム描画
//======================================================================
void DLPlayFunc_MsgUpdate( DLPLAY_MSG_SYS *msgSys , const u8 line , const BOOL isRefresh )
{
	GFL_BMPWIN_TransVramCharacter( msgSys->bmpwin_[line] );
	
	if( isRefresh == TRUE ){
		GFL_BMPWIN_MakeScreen( msgSys->bmpwin_[line] );
		GFL_BG_LoadScreenReq( msgSys->bgPlane_ );
	}
}

//======================================================================
//	DPTからUTF-16へ変換
//	@param  [in] DPTでの文字コード
//　@param  [out]UTF-16形式を格納するポインタ
//　@param  [in] 文字列長
//	@return [out]変換後の文字列
//======================================================================
#include "pt_str_arr.dat"
const u16 DLPlayFunc_DPTStrCode_To_UTF16( const u16 *dptStr , u16* utfStr , const u16 len )
{
	static const u16 EomCode = 0x0000;
	static const u16 UnknownCode = L'?';
	int i,j;
	for( i=0;i<len;i++ )
	{
		if( dptStr[i] == 0xFFFF ){
			utfStr[i] = EomCode;
			break;
		}
		for( j=0;j<PT_STR_ARR_NUM;j++ ){
			if( dptStr[i] == PT_STR_ARR[j][1] ){
				utfStr[i] = PT_STR_ARR[j][0];
				break;
			}
		}
		if( j == PT_STR_ARR_NUM ){
			//該当文字なし
			utfStr[i] = UnknownCode;
		}
	}
	return i;
}


//メッセージ用BGの切り替え
//メッセージの縦幅
static const u8 msgHeightNum[DLPLAY_MSG_MAX]={2,3,2,2,2,2,4,2,2,2,2,3,2,2,2,2,2,3};
void DLPlayFunc_ChangeBgMsg( u8 msgIdx , u8 plane )
{
	u8 startLine = 1;	//絵の枠素材分
	u8 i,x,y;
	for( i=0;i<msgIdx;i++ )
	{
		startLine += msgHeightNum[i];
	}
	for( y=0;y<msgHeightNum[i];y++ )
	{
		for( x=0;x<24;x++ )
		{
			GFL_BG_FillScreen( plane , 0x18*(y+startLine )+x ,
					x,(19+y),1,1,2);
		}
	}
	for( ;y<4;y++ )
	{
		for( x=0;x<24;x++ )
		{
			GFL_BG_FillScreen( plane , 0x07 ,
					x,(19+y),1,1,2);
		}
	}
	GFL_BG_LoadScreenReq( plane );
}

//セーブ処理用ウェイト関数
void DLPlayFunc_InitCounter( u16 *cnt )
{
	*cnt = 0;
}
const BOOL DLPlayFunc_UpdateCounter( u16 *cnt , const u16 time )
{
	if( *cnt < time )
	{
		*cnt += 1;
		return FALSE;
	}
	else
	{
		return TRUE;	
	}
}

