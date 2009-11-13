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
#include <tcbl.h>

#include "system/gfl_use.h"
#include "system/main.h"

#include "dlplay_func.h"
#include "../ariizumi/ari_debug.h"
#include "print/printsys.h"
//======================================================================
//	define
//======================================================================

#define USE_DEBUG_MSG (0)

#define DLPLAY_FUNC_MSG_X (16)
#define DLPLAY_FUNC_MSG_LINE_NUM (24)
#define DLPLAY_FUNC_MSG_LINE_MAX (20)

#define DLPLAY_MSG_TOP (19)
#define DLPLAY_MSG_LEFT (1)
#define DLPLAY_MSG_WIDTH (30)
#define DLPLAY_MSG_HEIGHT (4)
#define DLPLAY_MSG_FONT_PLT (15)

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
	int heapID_;

	GFL_BMPWIN			*bmpwin_[DLPLAY_FUNC_MSG_LINE_NUM ];
	GFL_BMPWIN			*bmpwinMsg_;
#if USE_DEBUG_MSG
	GFL_TEXT_PRINTPARAM	*textParam_[DLPLAY_FUNC_MSG_LINE_NUM ];
#endif //USE_DEBUG_MSG

	GFL_MSGDATA		*msgData_;
	GFL_FONT		*fontHandle_;
	PRINT_QUE		*printQue_;
	PRINT_UTIL		printUtil_[1];
};

//======================================================================
//	proto
//======================================================================
DLPLAY_MSG_SYS*	DLPlayFunc_MsgInit( int	heapID , u8 bgPlane );
void	DLPlayFunc_FontInit( u8 fontArcID , u8 fontFileID , u8 msgArcID , u8 msgFileID , 
				u8 pltArcID , u8 pltFileID , u8 msgPlane , DLPLAY_MSG_SYS *msgSys );
void	DLPlayFunc_MsgTerm( DLPLAY_MSG_SYS *msgSys );
BOOL	DLPlayFunc_CanFontTerm( DLPLAY_MSG_SYS *msgSys );
void	DLPlayFunc_FontTerm( DLPLAY_MSG_SYS *msgSys );

void DLPlayFunc_PutString( char* str , DLPLAY_MSG_SYS *msgSys);
void DLPlayFunc_PutStringLine( u8 line , char* str , DLPLAY_MSG_SYS *msgSys );
void DLPlayFunc_ClearString( DLPLAY_MSG_SYS *msgSys );
void DLPlayFunc_DispMsgID( u16 msgID , DLPLAY_MSG_SYS *msgSys );

void DLPlayFunc_MsgUpdate( DLPLAY_MSG_SYS *msgSys , const u8 line , const BOOL isRefresh );
void DLPlayFunc_UpdateFont( DLPLAY_MSG_SYS *msgSys );

const u16 DLPlayFunc_DPTStrCode_To_UTF16( const u16 *dptStr , u16* utfStr , const u16 len );

//背景bg文字変換用
void DLPlayFunc_ChangeBgMsg( u8 msgIdx , DLPLAY_MSG_SYS *msgSys );

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
	msgSys->heapID_ = heapID;
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
#if USE_DEBUG_MSG
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
#endif //USE_DEBUG_MSG
	}

	return msgSys;
}

//メインのROMとDL子機で同じ関数を使うためにファイル系ID全指定・・・
void	DLPlayFunc_FontInit( u8 fontArcID , u8 fontFileID , u8 msgArcID , u8 msgFileID ,
				 u8 pltArcID , u8 pltFileID , u8 msgPlane , DLPLAY_MSG_SYS *msgSys )
{
	//MSG周り初期化
	GFL_ARC_UTIL_TransVramPalette( pltArcID , pltFileID ,
					PALTYPE_MAIN_BG, DLPLAY_MSG_FONT_PLT*32, 32, msgSys->heapID_ );
	msgSys->bmpwinMsg_ = GFL_BMPWIN_Create( msgPlane ,
					DLPLAY_MSG_LEFT , DLPLAY_MSG_TOP ,
					DLPLAY_MSG_WIDTH , DLPLAY_MSG_HEIGHT ,
					DLPLAY_MSG_FONT_PLT , GFL_BG_CHRAREA_GET_F );
	
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( msgSys->bmpwinMsg_ ), 0x0000 );
	GFL_BMPWIN_MakeScreen( msgSys->bmpwinMsg_ );
		
	GFL_BMPWIN_TransVramCharacter( msgSys->bmpwinMsg_ );
	GFL_BG_LoadScreenReq( msgPlane );
	
	msgSys->fontHandle_ = GFL_FONT_Create( fontArcID , 
					fontFileID , GFL_FONT_LOADTYPE_FILE ,
					FALSE , msgSys->heapID_ );
	msgSys->msgData_ = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , msgArcID ,
					msgFileID , msgSys->heapID_ );
	
	GFL_FONTSYS_Init();
	msgSys->printQue_ = PRINTSYS_QUE_Create( msgSys->heapID_ );
	PRINT_UTIL_Setup( msgSys->printUtil_ , msgSys->bmpwinMsg_ );
}

//======================================================================
//	DLプレイメッセージシステム開放
//======================================================================
void	DLPlayFunc_MsgTerm( DLPLAY_MSG_SYS *msgSys )
{
#if USE_DEBUG_MSG
	u8 i;
	for( i=0;i<DLPLAY_FUNC_MSG_LINE_NUM;i++ ){
		GFL_BMPWIN_Delete( msgSys->bmpwin_[i] );
		GFL_HEAP_FreeMemory( msgSys->textParam_[i] );
	}
#endif //USE_DEBUG_MSG
	GFL_HEAP_FreeMemory( msgSys );

}

BOOL	DLPlayFunc_CanFontTerm( DLPLAY_MSG_SYS *msgSys )
{
	return PRINTSYS_QUE_IsFinished( msgSys->printQue_ );
}

void	DLPlayFunc_FontTerm( DLPLAY_MSG_SYS *msgSys )
{
	GFL_BMPWIN_Delete( msgSys->bmpwinMsg_ );
	PRINTSYS_QUE_Delete( msgSys->printQue_ );
	GFL_MSG_Delete( msgSys->msgData_ );
	GFL_FONT_Delete( msgSys->fontHandle_ );
}

//======================================================================
//	DLプレイメッセージシステム描画

//======================================================================
void DLPlayFunc_PutString( char* str , DLPLAY_MSG_SYS *msgSys)
{
	ARI_TPrintf("MsgPut:[%s]\n",str);
#if USE_DEBUG_MSG
	//GFL_BMP_Clear( msgSys->textParam_[msgSys->line_]->bmp , 0x0000 );

	msgSys->textParam_[msgSys->line_]->writex	= DLPLAY_FUNC_MSG_X;
	msgSys->textParam_[msgSys->line_]->writey	= 0;
	GFL_TEXT_PrintSjisCode( str , msgSys->textParam_[msgSys->line_] );
	DLPlayFunc_MsgUpdate( msgSys , msgSys->line_ , FALSE );
	
	msgSys->line_++;
	if( msgSys->line_ >= DLPLAY_FUNC_MSG_LINE_MAX )
		msgSys->line_ = 0;
	//次のラインをクリアしておく:
	GFL_BMP_Clear( msgSys->textParam_[msgSys->line_]->bmp , 0x0000 );
	DLPlayFunc_MsgUpdate( msgSys , msgSys->line_ , TRUE );
#endif USE_DEBUG_MSG
}

void DLPlayFunc_PutStringLine( u8 line , char* str , DLPLAY_MSG_SYS *msgSys )
{
	ARI_TPrintf("MsgPut:[%d][%s]\n",line,str);
#if USE_DEBUG_MSG
	GFL_BMP_Clear( msgSys->textParam_[line]->bmp , 0x0000 );
	DLPlayFunc_MsgUpdate( msgSys , line , FALSE );
	msgSys->textParam_[line]->writex	= DLPLAY_FUNC_MSG_X;
	msgSys->textParam_[line]->writey	= 0;
	GFL_TEXT_PrintSjisCode( str , msgSys->textParam_[line] );
	DLPlayFunc_MsgUpdate( msgSys , line , TRUE );
#endif //USE_DEBUG_MSG
}

void DLPlayFunc_PutStringLineDiv( u8 line , char* str1 , char* str2 , DLPLAY_MSG_SYS *msgSys )
{
	ARI_TPrintf("MsgPut:[%d][%32s][%32s]\n",line,str1,str2);
#if USE_DEBUG_MSG
	GFL_BMP_Clear( msgSys->textParam_[line]->bmp , 0x0000 );
	DLPlayFunc_MsgUpdate( msgSys , line , FALSE );
	msgSys->textParam_[line]->writex	= DLPLAY_FUNC_MSG_X;
	msgSys->textParam_[line]->writey	= 0;
	GFL_TEXT_PrintSjisCode( str1 , msgSys->textParam_[line] );
	msgSys->textParam_[line]->writex	= DLPLAY_FUNC_MSG_X+128;
	GFL_TEXT_PrintSjisCode( str2 , msgSys->textParam_[line] );
	DLPlayFunc_MsgUpdate( msgSys , line , TRUE );
#endif //USE_DEBUG_MSG
}

void DLPlayFunc_ClearString( DLPLAY_MSG_SYS *msgSys )
{
#if USE_DEBUG_MSG
	u8 i;
	for(i=0;i<DLPLAY_FUNC_MSG_LINE_NUM;i++)
		GFL_BMP_Clear( msgSys->textParam_[i]->bmp , 0x0000 );
#endif //USE_DEBUG_MSG
}

//======================================================================
//	DLプレイメッセージ描画
//======================================================================
void DLPlayFunc_DispMsgID( u16 msgID , DLPLAY_MSG_SYS *msgSys )
{
  /*
	//文字列取得用
	STRBUF *strTemp;
	
	//前回文字列の消去
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(msgSys->bmpwinMsg_) , 0x0000 );

	strTemp = GFL_STR_CreateBuffer( 256 , msgSys->heapID_ );
	GFL_MSG_GetString( msgSys->msgData_ , msgID , strTemp );
	PRINT_UTIL_Print( msgSys->printUtil_ , msgSys->printQue_ ,
				0 , 0 , (void*)strTemp , msgSys->fontHandle_ );
	GFL_STR_DeleteBuffer( strTemp );
	*/
}

void DLPlayFunc_UpdateFont( DLPLAY_MSG_SYS *msgSys )
{
  	PRINTSYS_QUE_Main( msgSys->printQue_ );
	if( PRINT_UTIL_Trans( msgSys->printUtil_ , msgSys->printQue_ ) == FALSE )
	{}
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
void DLPlayFunc_ChangeBgMsg( u8 msgIdx , DLPLAY_MSG_SYS *msgSys )
{
	//互換のためそのまま使用
	DLPlayFunc_DispMsgID( msgIdx , msgSys );
	/*
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
	*/
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
