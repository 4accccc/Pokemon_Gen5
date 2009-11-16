//============================================================================================
/**
 * @file	dlplay_parent_sample.c
 * @brief	ダウンロードプレイサンプル
 * @author	ariizumi
 * @date	2008.10.8
 */
//============================================================================================
#include "string.h"
#include "gflib.h"
#include "procsys.h"
#include "ui.h"
#include "system/main.h"
#include "arc_def.h"
#include "net.h"

#include "test/ariizumi/ari_debug.h"
#include "dlplay_parent_sample.h"
#include "dlplay_parent_main.h"
#include "dlplay_func.h"
#include "dlplay_comm_func.h"
#include "dlplay_disp_sys.h"

#include "savedata/save_tbl.h"
#include "savedata/save_control.h"
#include "savedata/box_savedata.h"
#include "test/performance.h"

#include "test_graphic/mb_test.naix"
#include "message.naix"
#include "font/font.naix"

//------------------------------------------------------------------
//	typedef struct
//------------------------------------------------------------------
typedef struct
{
	int heapID_;

	u8	mainSeq_;
	u8	subSeq_;
	int	currTray_;
	u8	errorState_;
	u16	waitCounter_;
	u16	waitTimer_;

	DLPLAY_SEND_DATA	*dlData_;
	DLPLAY_COMM_DATA	*commSys_;
	DLPLAY_MSG_SYS		*msgSys_;
	DLPLAY_DISP_SYS		*dispSys_;

}DLPLAY_PARENT_DATA;

enum DLPLAY_PARENT_MODE
{

	DPM_INIT_COMM,
	DPM_WAIT_INIT_COMM,
	DPM_WAIT_START,
	DPM_MAIN_LOOP,

	DPM_START_SHARE,
	DPM_WAIT_NEGOTIATION,
	DPM_WAIT_CHILD,
	DPM_WAIT_START_POST_INDEX,
	DPM_WAIT_INDEX_DATA,
	DPM_SELECT_BOX_TEMP,
	DPM_SELECT_BOX_CONFIRM,
	DPM_WAIT_POST_BOXDATA,
	DPM_SAVE_MAIN,
	DPM_WAIT_END,

	DPM_ERROR_INIT,
	DPM_ERROR_LOOP,

	DPM_END,		//終了用
	DPM_MAX,

};

//------------------------------------------------------------------
//	local
//------------------------------------------------------------------
DLPLAY_PARENT_DATA *parentData;
//============================================================================================
//	proto
//============================================================================================
static void DLPlayMain_InitBg(void);
static BOOL DLPlaySend_SaveMain( DLPLAY_SEND_DATA *dlData );


//------------------------------------------------------------------
//	VRAM用定義
//------------------------------------------------------------------
static const GFL_BG_SYS_HEADER bgsysHeader = {
	GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_3D
};	
static const GFL_BG_BGCNT_HEADER bgCont3 = {
	0, 0, 0x800, 0,
	GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
	GX_BG_SCRBASE_0x2800, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
	GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
};
static const GFL_BG_BGCNT_HEADER bgContStrWin = {
	0, 0, 0x800, 0,
	GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
	GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
	GX_BG_EXTPLTT_01, DLPLAY_MSGWIN_PLANE_PRI, 0, 0, FALSE
};
static const GFL_BG_BGCNT_HEADER bgContFontMsg = {
	0, 0, 0x800, 0,
	GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
	GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x256,
	GX_BG_EXTPLTT_01, DLPLAY_FONT_MSG_PLANE_PRI, 0, 0, FALSE
};

static const GFL_DISP_VRAM vramBank = {
	GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
	GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
	GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
	GX_VRAM_SUB_OBJ_128_D,			// サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
	GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_NONE,			// テクスチャパレットスロット
	GX_OBJVRAMMODE_CHAR_1D_128K,	// メインOBJマッピングモード
	GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
};

//------------------------------------------------------------------
//  デバッグ用初期化関数
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugDLPlayMainProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	static const HEAPID heapID = HEAPID_MULTIBOOT;
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MULTIBOOT, 0x150000 );

	parentData = GFL_HEAP_AllocClearMemory( HEAPID_MULTIBOOT, sizeof( DLPLAY_PARENT_DATA ) );
	parentData->heapID_ = HEAPID_MULTIBOOT;
	parentData->mainSeq_ = DPM_INIT_COMM;
	parentData->subSeq_ = 0;

	DLPlayMain_InitBg();

	parentData->dlData_ = DLPlaySend_Init( heapID );
	parentData->commSys_ = DLPlayComm_InitData( heapID );
	parentData->msgSys_ = DLPlayFunc_MsgInit( heapID , DLPLAY_MSG_PLANE );
	parentData->dispSys_ = DLPlayDispSys_InitSystem( heapID , &vramBank);
	DLPlayFunc_FontInit( ARCID_FONT , NARC_font_large_gftr ,
					ARCID_MESSAGE , NARC_message_d_dlplay_dat ,
					ARCID_FONT , NARC_font_default_nclr , 
					DLPLAY_FONT_MSG_PLANE , parentData->msgSys_ );
	GFL_HEAP_DEBUG_PrintExistMemoryBlocks(heapID);

	DLPlaySend_SetMessageSystem( parentData->msgSys_ , parentData->dlData_ );

	parentData->errorState_ = DES_NONE;
#if DLPLAY_FUNC_USE_PRINT
	DLPlayComm_SetMsgSys( parentData->msgSys_ , parentData->commSys_ );
	DLPlayFunc_PutString("",parentData->msgSys_);
	DLPlayFunc_PutString("System Initialize complete.",parentData->msgSys_);
#endif
	DEBUG_PerformanceSetActive( FALSE );
	DLPlayFunc_ChangeBgMsg( MSG_SEARCH_CHILD , parentData->msgSys_ );

	return GFL_PROC_RES_FINISH;
}

static void DLPlayMain_InitBg(void)
{

	//VRAM系初期化
	//BGシステム起動
	GFL_BG_Init( parentData->heapID_ );

	//VRAM設定
	GFL_DISP_SetBank( &vramBank );

	//BGモード設定
	GFL_BG_SetBGMode( &bgsysHeader );

	//ＢＧコントロール設定
	GFL_BG_SetBGControl( DLPLAY_MSG_PLANE, &bgCont3, GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( DLPLAY_MSGWIN_PLANE, &bgContStrWin, GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( DLPLAY_FONT_MSG_PLANE, &bgContFontMsg, GFL_BG_MODE_TEXT );
	GFL_BG_SetVisible( DLPLAY_MSG_PLANE, VISIBLE_ON );
	GFL_BG_SetVisible( DLPLAY_MSGWIN_PLANE, VISIBLE_ON );
	GFL_BG_SetVisible( DLPLAY_FONT_MSG_PLANE, VISIBLE_ON );

	GFL_BG_FillCharacter( DLPLAY_MSG_PLANE, 0x00, 1, 0 );
	GFL_BG_FillCharacter( DLPLAY_MSGWIN_PLANE , 0x00, 1, 0 );
	GFL_BG_FillCharacter( DLPLAY_FONT_MSG_PLANE , 0x00, 1, 0 );
	GFL_BG_FillScreen( DLPLAY_MSG_PLANE,
		0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_FillScreen( DLPLAY_MSGWIN_PLANE,
		0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_FillScreen( DLPLAY_FONT_MSG_PLANE,
		0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_LoadScreenReq( DLPLAY_MSG_PLANE );
	GFL_BG_LoadScreenReq( DLPLAY_MSGWIN_PLANE );
	GFL_BG_LoadScreenReq( DLPLAY_FONT_MSG_PLANE );
	
	//BG読み込み開始
	GFL_ARC_UTIL_TransVramBgCharacter( ARCID_MB_TEST , NARC_mb_test_test_bg_NCGR ,
			DLPLAY_MSGWIN_PLANE , 0 , 0 , FALSE , HEAPID_MULTIBOOT );
	GFL_ARC_UTIL_TransVramScreen( ARCID_MB_TEST , NARC_mb_test_test_bg_NSCR ,
			DLPLAY_MSGWIN_PLANE , 0 , 0 , FALSE , HEAPID_MULTIBOOT );
	GFL_ARC_UTIL_TransVramPalette( ARCID_MB_TEST , NARC_mb_test_test_bg_NCLR ,
			PALTYPE_MAIN_BG , 0 , 0 , HEAPID_MULTIBOOT );

//ビットマップウインドウシステムの起動
	GFL_BMPWIN_Init( parentData->heapID_ );

}

//------------------------------------------------------------------
/**  デバッグ用Exit
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugDLPlayMainProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	DLPlayFunc_FontTerm( parentData->msgSys_ );
	DLPlaySend_Term( parentData->dlData_ );
	DLPlayComm_TermData( parentData->commSys_ );
	DLPlayDispSys_TermSystem( parentData->dispSys_ );
	DLPlayFunc_MsgTerm( parentData->msgSys_ );
	
	GFL_BMPWIN_Exit();
	GFL_BG_FreeBGControl( DLPLAY_MSG_PLANE );
	GFL_BG_FreeBGControl( DLPLAY_MSGWIN_PLANE );
	GFL_BG_FreeBGControl( DLPLAY_FONT_MSG_PLANE );
	GFL_BG_Exit();

	GFL_HEAP_FreeMemory( parentData );

	GFL_HEAP_DeleteHeap( HEAPID_MULTIBOOT );

	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**   デバッグ用メイン
 */  
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugDLPlayMainProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	DLPlayFunc_UpdateFont( parentData->msgSys_ );
	switch( parentData->mainSeq_ )
	{
	case DPM_INIT_COMM:
		DLPlayComm_InitSystem( parentData->commSys_ );
		parentData->mainSeq_ = DPM_WAIT_INIT_COMM;
		break;

	case DPM_WAIT_INIT_COMM:
		if( DLPlayComm_IsFinish_InitSystem( parentData->commSys_ ) ){
			DLPlayFunc_PutString("Commnicate system initialize omplete.",parentData->msgSys_);
			DLPlayFunc_PutString("Press A Button to start.",parentData->msgSys_);
			parentData->mainSeq_ = DPM_WAIT_START;
		}
		break;

	case DPM_WAIT_START:
		//if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_A ){
			DLPlaySend_StartMBP( parentData->dlData_ );
			parentData->mainSeq_ = DPM_MAIN_LOOP;
			DLPlayFunc_PutString( "MultiBoot is start."  ,parentData->msgSys_ );
		//}
		break;

	case DPM_MAIN_LOOP:
		if( DLPlaySend_MBPLoop( parentData->dlData_ ) == TRUE )
		{
			parentData->mainSeq_ = DPM_START_SHARE;
		}
		break;

	case DPM_START_SHARE:
		DLPlayComm_InitParent( parentData->commSys_ );
		DLPlayFunc_PutString( "Wait child......",parentData->msgSys_); 
		parentData->mainSeq_ = DPM_WAIT_NEGOTIATION;
		parentData->subSeq_ = 0;
		break;

	case DPM_WAIT_NEGOTIATION:
		if( DLPlayComm_IsFinish_Negotiation( parentData->commSys_ ) == TRUE )
		{
			parentData->mainSeq_ = DPM_WAIT_CHILD;
		}
		break;

	case DPM_WAIT_CHILD:
        //if(GFL_NET_HANDLE_GetNumNegotiation() != 0){
            //if(GFL_NET_HANDLE_RequestNegotiation()){
                if ( DLPlayComm_IsConnect( parentData->commSys_ ) == TRUE ){

                    DLPlayFunc_ChangeBgMsg( MSG_WAIT_CHILD_PROC , parentData->msgSys_ );
			
                    DLPlayFunc_PutString( "Child connect.",parentData->msgSys_); 
                    parentData->mainSeq_ = DPM_WAIT_START_POST_INDEX;
                }
            //}
		//}
		break;
	case DPM_WAIT_START_POST_INDEX:
		if ( DLPlayComm_IsStartPostIndex( parentData->commSys_ ) == TRUE ){
			DLPlayFunc_ChangeBgMsg( MSG_POST_DATA_CHILD , parentData->msgSys_ );
			
			parentData->mainSeq_ = DPM_WAIT_INDEX_DATA;
		}
		break;

	case DPM_WAIT_INDEX_DATA:
		if(	DLPlayComm_IsPostIndex( parentData->commSys_ ) == TRUE ){
			DLPlayFunc_PutString( "Post box index data complete.",parentData->msgSys_); 
			DLPlayFunc_ChangeBgMsg( MSG_SELECT_BOX , parentData->msgSys_ );
			parentData->mainSeq_ = DPM_SELECT_BOX_TEMP;
			parentData->currTray_ = 0xFF;
		}
		break;
	case DPM_SELECT_BOX_TEMP:
		{
			u8 i;
			BOOL isUpdate = FALSE;
			//初回更新を有効にするため0xffの時に初回とみなしている。
			if( parentData->currTray_ == 0xFF )
			{
				isUpdate = TRUE;
				parentData->currTray_ = 0;
			}
			if( GFL_UI_KEY_GetTrg() == PAD_KEY_RIGHT )
			{
				isUpdate = TRUE;
				parentData->currTray_++;
				if( parentData->currTray_ >= 18 )
				{
					parentData->currTray_ = 0;
				}
			}
			if( GFL_UI_KEY_GetTrg() == PAD_KEY_LEFT )
			{
				isUpdate = TRUE;
				parentData->currTray_--;
				if( parentData->currTray_ < 0 )
				{
					parentData->currTray_ = 17;
				}
			}
			if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_A )
			{
				parentData->mainSeq_ = DPM_SELECT_BOX_CONFIRM;
				DLPlayFunc_ChangeBgMsg( MSG_CONFIRM , parentData->msgSys_ );
			}
			if( isUpdate == TRUE )
			{
#if 1
				DLPLAY_BOX_INDEX *boxIndex = DLPlayComm_GetBoxIndexBuff( parentData->commSys_ );
				DLPlayDispSys_DispBoxIcon( boxIndex , parentData->currTray_ , parentData->dispSys_ );
//#else		//文字表示用のデバッグ(した画面に別途表示したいといわれたので通してます
				{
					const char sexStr[3][8] ={"M","F","?"};
					const int bi = parentData->currTray_;
					u16 i;
					int	strLen = 64;
					char str[128],w1Str[64],w2Str[64];
					DLPLAY_BOX_INDEX *boxIndex = DLPlayComm_GetBoxIndexBuff( parentData->commSys_ );
	
					DLPlayFunc_ClearString( parentData->msgSys_ );
	 
					STD_ConvertStringUnicodeToSjis( w1Str , &strLen , boxIndex->boxName_[bi] , NULL , NULL );
					w1Str[strLen] = '\0';
					sprintf(str,"BoxName[%s]",w1Str);
					DLPlayFunc_PutStringLine( 0,str,parentData->msgSys_ );
	
					for( i=0;i<30;i++ )
					{
						DLPLAY_MONS_INDEX *pokeData = &boxIndex->pokeData_[bi][i];
						if( pokeData->pokeNo_ == 0 )
						{
							sprintf(w2Str,"[---][----------]");
						}
						else
						{
							strLen = 64;
							STD_ConvertStringUnicodeToSjis( w1Str , &strLen , pokeData->name_ , NULL , NULL );
							w1Str[strLen] = '\0';
							sprintf(w2Str,"[%3d][%s:%s][%d]",pokeData->pokeNo_,w1Str,sexStr[pokeData->sex_],pokeData->lv_);
							if( pokeData->isEgg_ == 1 )
							{
								strcat( w2Str , "[E]" );
							}
							if( pokeData->rare_ == 1 )
							{
								strcat( w2Str , "[R]" );
							}
							strcat(w2Str,"      ");
						}
						if( i%2==1 )
						{
							DLPlayFunc_PutStringLineDiv( i/2+1,str,w2Str,parentData->msgSys_ );
						}
						else
						{
							strcpy(str,w2Str);
						}
					}
				}
#endif
			}
		}
		break;
		
	case DPM_SELECT_BOX_CONFIRM:
		if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_A )
		{
			DLPlayComm_Send_BoxNumber( parentData->currTray_ , parentData->commSys_ );
			DLPlayFunc_ChangeBgMsg( MSG_POST_DATA_CHILD , parentData->msgSys_ );
			parentData->mainSeq_ = DPM_WAIT_POST_BOXDATA;
			parentData->subSeq_ = 0;
		}
		else if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_B )
		{
			DLPlayFunc_ChangeBgMsg( MSG_SELECT_BOX , parentData->msgSys_ );
			parentData->mainSeq_ = DPM_SELECT_BOX_TEMP;
		}
		break;
	case DPM_WAIT_POST_BOXDATA:
		if( DLPlayComm_IsPostData( parentData->commSys_ ) == TRUE )
		{
			DLPlayComm_Send_CommonFlg( DC_FLG_POST_BOX_DATA , 0 , parentData->commSys_ );
			parentData->mainSeq_ = DPM_SAVE_MAIN;
			DLPlayFunc_ChangeBgMsg( MSG_SAVE , parentData->msgSys_ );
		}
		break;
	
	case DPM_SAVE_MAIN:
		if( DLPlayComm_IsPost_SendData( parentData->commSys_ ) == TRUE )
		{
			if( DLPlaySend_SaveMain( parentData->dlData_ ) == TRUE )
			{
				parentData->mainSeq_ = DPM_WAIT_END;
			}
		}
		break;

	case DPM_WAIT_END:
		//if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_A )
		{
			//通信ライブラリの切断のため、早めに呼ぶ
			DLPlayComm_TermSystem( parentData->commSys_ );
			parentData->mainSeq_ = DPM_END;
		}
		break;
	
	case DPM_END:
		//通信ライブラリの終了を待ってから終了
		if( DLPlayComm_IsFinish_TermSystem( parentData->commSys_ ) == TRUE &&
			DLPlayFunc_CanFontTerm( parentData->msgSys_ ) == TRUE )
		{
			return GFL_PROC_RES_FINISH;
		}
		break;

	case DPM_ERROR_INIT:
		parentData->mainSeq_ = DPM_ERROR_LOOP;
		if( parentData->errorState_ == DES_MISS_LOAD_BACKUP )
		{
			DLPlayFunc_ChangeBgMsg( MSG_MISS_LOAD_BACKUP , parentData->msgSys_ );
		}
		else
		{
			DLPlayFunc_ChangeBgMsg( MSG_ERROR , parentData->msgSys_ );
		}
		if( DLPlayComm_GetPostErrorState( parentData->commSys_ ) == DES_NONE )
		{
			//受信してエラーに来たときは送らない
			DLPlayComm_Send_ErrorState( parentData->errorState_ , parentData->commSys_ );
		}
		break;

	case DPM_ERROR_LOOP:
		break;

	}

	//エラー処理
	if( parentData->errorState_ != DPM_ERROR_INIT &&
		parentData->errorState_ != DPM_ERROR_LOOP )
	{
		if( parentData->errorState_ != DES_NONE )
		{
			parentData->mainSeq_ = DPM_ERROR_INIT;
		}
		else
		if( DLPlayComm_GetPostErrorState( parentData->commSys_ ) != DES_NONE )
		{
			parentData->errorState_ = DLPlayComm_GetPostErrorState( parentData->commSys_ );
			parentData->mainSeq_ = DPM_ERROR_INIT;
		}
	}
	DLPlayDispSys_UpdateDraw( parentData->dispSys_ );

	return GFL_PROC_RES_CONTINUE;
}

#define DLPLAY_CHECK_SAVE_TICK 1
#if DLPLAY_CHECK_SAVE_TICK
static OSTick dlplaySaveTick;
#endif

static BOOL	DLPlaySend_SaveMain( DLPLAY_SEND_DATA *dlData )
{
	switch( parentData->subSeq_ )
	{
	case 0:	
		{
		BOX_MANAGER *pBoxData;
		DLPLAY_LARGE_PACKET *lPacket;
		//セーブ前準備
		DLPlayFunc_ClearString( parentData->msgSys_ );
		DLPlayFunc_PutString("Save Ready.",parentData->msgSys_);

		lPacket = DLPlayComm_GetLargePacketBuff( parentData->commSys_ );
		pBoxData = SaveControl_DataPtrGet( SaveControl_GetPointer() , GMDATA_ID_BOXDATA );
		BOXDAT_SetPPPData_Tray( DLPlayComm_GetSelectBoxNumber( parentData->commSys_ ) ,
						(void*)lPacket->pokeData_ , pBoxData );
								
		parentData->subSeq_++;
		}
		break;
	case 1:
		//セーブ開始
		DLPlayFunc_PutString("Save Start.",parentData->msgSys_);
		//parentData->waitTimer_ = 600;
		//DLPlayFunc_InitCounter( &parentData->waitCounter_ );
		SaveControl_SaveAsyncInit( SaveControl_GetPointer() );
		parentData->subSeq_++;
#if DLPLAY_CHECK_SAVE_TICK
		dlplaySaveTick = OS_GetTick();
#endif
		break;
	case 2:
		{
			//セーブ完了待ち
			const SAVE_RESULT ret = SaveControl_SaveAsyncMain( SaveControl_GetPointer() );
			//if( DLPlayFunc_UpdateCounter( &parentData->waitCounter_ , parentData->waitTimer_ ) == TRUE )
			if( ret == SAVE_RESULT_LAST )
			{
				DLPlayFunc_PutString("Save end. Wait child.",parentData->msgSys_);
				DLPlayComm_Send_CommonFlg( DC_FLG_FINISH_SAVE1_PARENT , 0 , parentData->commSys_ );
				parentData->subSeq_++;
#if DLPLAY_CHECK_SAVE_TICK
			{
				const OSTick nowTick = OS_GetTick();
				OS_TPrintf("-SAVE TICK CHECK- SAVE_WAIT[%d]\n",OS_TicksToMilliSeconds( nowTick - dlplaySaveTick ));
			}
#endif
			}
		}
		break;
	case 3:
		//向こうもセーブ待ちになった
		if( DLPlayComm_IsFinishSaveFlg( DC_FLG_FINISH_SAVE1_CHILD , parentData->commSys_ ) == TRUE )
		{
			u16 waitTime = GFL_STD_MtRand(60)+30;
			DLPlayFunc_PutString("Send flg last save wait.",parentData->msgSys_);
			DLPlayComm_Send_CommonFlg( DC_FLG_PERMIT_LASTBIT_SAVE , waitTime , parentData->commSys_ );
			parentData->subSeq_++;
		}
		break;
	case 4:	//最終ビットのセーブ
		if( DLPlayComm_IsFinishSaveFlg( DC_FLG_PERMIT_LASTBIT_SAVE , parentData->commSys_ ) > 0 )
		{
			DLPlayFunc_PutString("Start last save wait.",parentData->msgSys_);
			parentData->waitTimer_ = DLPlayComm_IsFinishSaveFlg( DC_FLG_PERMIT_LASTBIT_SAVE , parentData->commSys_ );
			DLPlayFunc_InitCounter( &parentData->waitCounter_ );
			parentData->subSeq_++;
#if DLPLAY_CHECK_SAVE_TICK
			{
				const OSTick nowTick = OS_GetTick();
				OS_TPrintf("-SAVE TICK CHECK- SAVE_LAST[%d]\n",OS_TicksToMilliSeconds( nowTick - dlplaySaveTick ));
			}
#endif
		}
		break;
	case 5:
		if( DLPlayFunc_UpdateCounter( &parentData->waitCounter_ , parentData->waitTimer_ ) == TRUE )
		{
			//最終ビットセーブ
			DLPlayFunc_PutString("Last save.",parentData->msgSys_);
			parentData->subSeq_++;
#if DLPLAY_CHECK_SAVE_TICK
			dlplaySaveTick = OS_GetTick();
#endif
		}
		break;
	case 6:
		{
			//セーブ完了待ち
			const SAVE_RESULT ret = SaveControl_SaveAsyncMain( SaveControl_GetPointer() );
			//if( DLPlayFunc_UpdateCounter( &parentData->waitCounter_ , parentData->waitTimer_ ) == TRUE )
			if( ret == SAVE_RESULT_OK )
			{
				DLPlayComm_Send_CommonFlg( DC_FLG_FINISH_SAVE_ALL , 0 , parentData->commSys_ );
				parentData->subSeq_++;
#if DLPLAY_CHECK_SAVE_TICK
			{
				const OSTick nowTick = OS_GetTick();
				OS_TPrintf("-SAVE TICK CHECK- SAVE_LAST[%d]\n",OS_TicksToMilliSeconds( nowTick - dlplaySaveTick ));
			}
#endif
			}
		}	
		break;
	case 7:	//両方のセーブが終わるまで完了表示を出さない
		if( DLPlayComm_IsFinishSaveFlg( DC_FLG_FINISH_SAVE_ALL , parentData->commSys_ ) > 0 )
		{
			DLPlayFunc_PutString("Save Complete!!.",parentData->msgSys_);
			DLPlayFunc_ChangeBgMsg( MSG_SAVE_END , parentData->msgSys_ );
			return TRUE;
		}
	}
	return FALSE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_PROC_DATA DebugDLPlayMainProcData = {
	DebugDLPlayMainProcInit,
	DebugDLPlayMainProcMain,
	DebugDLPlayMainProcEnd,
};

