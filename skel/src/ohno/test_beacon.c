//============================================================================================
/**
 * @file	test_beacon.c
 * @brief	通信動作テスト用関数
 * @author	ohno
 * @date	2006.12.19
 */
//============================================================================================


#include "gflib.h"
#include "procsys.h"
#include "gf_standard.h"
#include "test_beacon.h"
#include "main.h"
#include "fatal_error.h"
#include "net_icondata.h"
#include "textprint.h"
#include "sample_graphic/radar.naix"


#define _BCON_GET_NUM  (6)
#define _BACKUP_NUM  (5)    // RSSIの平均を出すためにバックアップ
#define _DIR_SAMPL_NUM (8)  // 方向の取得サンプル
#define _GAME_BMPWIN (5)
#define _SAMP_TIMER  (60*3)  // ５秒サンプリング

//------------------------------------------------------------------
// NETのテスト
//------------------------------------------------------------------




// ローカル通信コマンドの定義
enum _testCommand_e {
    _TEST_VARIABLE = GFL_NET_CMD_COMMAND_MAX,
    _TEST_GETSIZE,
    _TEST_HUGE,
    _TEST_VARIABLE_HUGE,
};

typedef struct{
    int gameNo;   ///< ゲーム種類
} _testBeaconStruct;

static _testBeaconStruct _testBeacon = { 5 };

//-------------------------------------------------デバッグ表示

typedef struct {
	const char*	msg;
	u8 posx;
	u8 posy;
	u8 sizx;
	u8 sizy;
}TESTMODE_PRINTLIST;

static const char	test_index1[] = {"beacon catch"};
static const char	test_index2[] = {"scan mode"};
static const char	test_index3[] = {"point mode"};
static const char	test_index4[] = {"xxxxxxxx"};
static const char	test_index5[] = {"start"};
static const char	test_index6[] = {"end"};


enum {
	NUM_TITLE = 0,
	NUM_URL,
};

static const TESTMODE_PRINTLIST _childIndexList[] = {
	{ test_index1, 2, 1, 30, 1 },
	{ test_index2, 2, 2, 30, 1 },
};

static const TESTMODE_PRINTLIST _parentIndexList[] = {
	{ test_index1, 2, 1, 30, 1 },
	{ test_index3, 2, 2, 30, 1 },
};

static const TESTMODE_PRINTLIST _gameIndexList[] = {
	{ test_index5, 2, 3, 30, 1 },
	{ test_index4, 2, 4, 30, 1 },
};

//------------------------------------------------------------------
/**
 * @brief		２Ｄデータコントロール
 */
//------------------------------------------------------------------
static const char font_path[] = {"src/gfl_graphic/gfl_font.dat"};

#define G2D_BACKGROUND_COL	(0x0000)
#define G2D_FONT_COL		(0x7fff)
#define G2D_FONTSELECT_COL	(0x001f)

static const GFL_TEXT_PRINTPARAM default_param = 
{ NULL, 0, 0, 1, 1, 1, 0, GFL_TEXT_WRITE_16 };

#define TEXT_FRM			(GFL_BG_FRAME3_S)
#define TEXT_FRM_PRI		(0)

//------------------------------------------------------------------
/**
 * @brief		ＢＧ設定
 */
//------------------------------------------------------------------
static const GFL_BG_SYS_HEADER bgsysHeader = {
	GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_2D
};	


//------------------------------------------------------------------
/**
 * @brief		内部ワーク構造体
 */
//------------------------------------------------------------------

typedef struct {
	GFL_CLWK		*clwk;
	GFL_CLACTPOS	now_pos;
	GFL_CLACTPOS	offset_pos;
	u8			clact_no;		//セルアクターナンバー
} _ACCESS_POINT_PARAM;

//------------------------------------------------------------------
/**
 * @brief		内部ワーク構造体
 */
//------------------------------------------------------------------

typedef struct {
	void* p_cellbuff;
	void* p_cellanmbuff;
	NNSG2dCellDataBank*		p_cell;
	NNSG2dCellAnimBankData*	p_cellanm;
	NNSG2dImageProxy		imageproxy;
	NNSG2dImagePaletteProxy	plttproxy;
} _CLACT_RES;


/// コールバック関数の書式 内部ステート遷移用
typedef void (*_PTRFunc)(SKEL_TEST_BEACON_WORK* pNetHandle);

struct _SKEL_TEST_BEACON_WORK{
    HEAPID heapID;
    BOOL bParent;   ///< 親機モードかどうか
    GFL_TEXT_PRINTPARAM* textParam;
    GFL_BMPWIN*	bmpwin[7];
    GFL_BMPWIN* pBmpwinBeacon[_BCON_GET_NUM];
    u16 rssiBackup[_BCON_GET_NUM][_BACKUP_NUM];
    u16 rssiDir[_BCON_GET_NUM][_DIR_SAMPL_NUM];
    _ACCESS_POINT_PARAM pAP[_BCON_GET_NUM];
    int dir;
    int countDir;
    int testNo;
    _PTRFunc state;
    int timer;
    GFL_NETHANDLE* _pHandle;
    GFL_NETHANDLE* _pHandleServer;
	GFL_CLUNIT		*p_unit;
	GFL_CLWK		*clact_work[_BCON_GET_NUM];
    ACCESS_POINT_PARAM* Ap[_BCON_GET_NUM];
	_CLACT_RES res;
    
    GFL_AREAMAN			*clact_area;
};

#if defined(DEBUG_ONLY_FOR_ohno)
#define   _CHANGE_STATE(state, time)  _changeStateDebug(pWork ,state, time, __LINE__)
#else
#define   _CHANGE_STATE(state, time)  _changeState(pWork ,state, time)
#endif

static _ACCESS_POINT_PARAM	*_InitAPChr(SKEL_TEST_BEACON_WORK *pWork, u16 xpos, u16 ypos, u16 rssi);
static void _DeleteFallChr(SKEL_TEST_BEACON_WORK* pWork,int clact_no);


//==============================================================================
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//==============================================================================

static void _changeState(SKEL_TEST_BEACON_WORK* pWork,_PTRFunc state, int time)
{
    pWork->state = state;
    pWork->timer = time;
}

//==============================================================================
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//==============================================================================
#ifdef GFL_NET_DEBUG
static void _changeStateDebug(SKEL_TEST_BEACON_WORK* pWork,_PTRFunc state, int time, int line)
{
    NET_PRINT("net_state: %d\n",line);
    _changeState(pWork, state, time);
}
#endif

//------------------------------------------------------------------
/**
 * @brief		メッセージビットマップウインドウコントロール
 */
//------------------------------------------------------------------
static void msg_bmpwin_make
			(SKEL_TEST_BEACON_WORK * testmode, u8 bmpwinNum, const char* msg, u8 px, u8 py, u8 sx, u8 sy )
{
	//ビットマップ作成
	testmode->bmpwin[bmpwinNum] = GFL_BMPWIN_Create( TEXT_FRM, px, py, sx, sy, 0, 
														GFL_BG_CHRAREA_GET_B );

	//テキストをビットマップに表示
	testmode->textParam->bmp = GFL_BMPWIN_GetBmp( testmode->bmpwin[ bmpwinNum ] );
	GFL_TEXT_PrintSjisCode( msg, testmode->textParam );

	//ビットマップキャラクターをアップデート
	GFL_BMPWIN_TransVramCharacter( testmode->bmpwin[bmpwinNum] );
	//ビットマップスクリーン作成
	GFL_BMPWIN_MakeScreen( testmode->bmpwin[bmpwinNum] );
}

//------------------------------------------------------------------
/**
 * @brief		２Ｄデータ表示
 */
//------------------------------------------------------------------

static void clact_draw(SKEL_TEST_BEACON_WORK* testmode )
{
    if(testmode->p_unit){
        // セルアクターユニット描画処理
        GFL_CLACT_UNIT_Draw( testmode->p_unit );
        // セルアクターシステムメイン処理
        // 全ユニット描画が完了してから行う必要があります。
        GFL_CLACT_Main();
    }
}

//------------------------------------------------------------------
/**
 * @brief		２Ｄデータ表示
 */
//------------------------------------------------------------------

static void text_draw(SKEL_TEST_BEACON_WORK* testmode )
{
    int i;
    
	GFL_BG_ClearScreen( TEXT_FRM );

    //選択項目ビットマップの表示
    for(i=0;i<NELEMS(_parentIndexList);i++){
        GFL_BMPWIN_MakeScreen( testmode->bmpwin[i] );
    }
	GFL_BG_LoadScreenReq( TEXT_FRM );
}

//------------------------------------------------------------------
/**
 * @brief		２Ｄデータコントロール
 */
//------------------------------------------------------------------
static void	g2d_load( SKEL_TEST_BEACON_WORK* pWork )
{
	//フォント読み込み
	GFL_TEXT_CreateSystem( font_path );
	//パレット作成＆転送
	{
		u16* plt = GFL_HEAP_AllocClearMemoryLo( pWork->heapID, 16*2 );
		plt[0] = G2D_BACKGROUND_COL;
		plt[1] = G2D_FONT_COL;
		GFL_BG_LoadPalette( TEXT_FRM, plt, 16*2, 0 );
		plt[1] = G2D_FONTSELECT_COL;
		GFL_BG_LoadPalette( TEXT_FRM, plt, 16*2, 16*2 );

		GFL_HEAP_FreeMemory( plt );
	}
	//文字表示パラメータワーク作成
	{
		GFL_TEXT_PRINTPARAM* param = GFL_HEAP_AllocClearMemoryLo
										( pWork->heapID,sizeof(GFL_TEXT_PRINTPARAM));
		*param = default_param;
		pWork->textParam = param;
	}
	//文字表示ビットマップの作成
	{
		int i,num;
        const TESTMODE_PRINTLIST* pList;

        if(pWork->bParent){
            pList = &_parentIndexList[0];
            num = NELEMS(_parentIndexList);
        }
        else{
            pList = &_childIndexList[0];
            num = NELEMS(_childIndexList);
        }
        
		//表題ビットマップの作成
		for(i = 0 ; i < num ; i++,pList++){
			msg_bmpwin_make( pWork, i, pList->msg,
							pList->posx, pList->posy, 
							pList->sizx, pList->sizy );
		}

        for(i=0; i < NELEMS(_gameIndexList);i++, pList++){
            pList = &_gameIndexList[0];
            msg_bmpwin_make( pWork, _GAME_BMPWIN+i, pList->msg,
                             pList->posx, pList->posy, 
                             pList->sizx, pList->sizy );
        }
    }
}

//------------------------------------------------------------------
/**
 * @brief		ＢＧ設定＆データ転送
 */
//------------------------------------------------------------------
static void	bg_init( HEAPID heapID )
{
	//ＢＧコントロール設定
	GFL_BG_SetPriority( TEXT_FRM, TEXT_FRM_PRI );
	GFL_BG_SetVisible( TEXT_FRM, VISIBLE_ON );
}

//==============================================================================
/**
 * @brief   なにもしない
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//==============================================================================
static void _noneFunc(SKEL_TEST_BEACON_WORK* pWork)
{
    text_draw(pWork);
    clact_draw(pWork);
}


static void* _netBeaconGetFunc(void)    ///< ビーコンデータ取得関数
{
    return &_testBeacon;
}

static int _netBeaconGetSizeFunc(void)    ///< ビーコンデータサイズ取得関数
{
    return sizeof(_testBeacon);
}

static BOOL _netBeaconCompFunc(GameServiceID myNo,GameServiceID beaconNo)    ///< ビーコンデータ取得関数
{
    return TRUE;
}


//----------------------------------------------------------------


static u16 _backupRssi(SKEL_TEST_BEACON_WORK* pWork,int index, u16 rssi)
{
    u16 sum = 0;
    int i;

    if((pWork->rssiBackup[index][_BACKUP_NUM-1]==0) ||(pWork->rssiBackup[index][0] != rssi)){
        for(i = _BACKUP_NUM-1; (i-1) >= 0; i--){
            pWork->rssiBackup[index][i] = pWork->rssiBackup[index][i-1];
        }
        pWork->rssiBackup[index][0] = rssi;
    }
    for(i = 0; i < _BACKUP_NUM ; i++){
        sum  += pWork->rssiBackup[index][i];
    }
    if(sum!=0){
        sum /= _BACKUP_NUM;
    }
    return sum;
}

//==============================================================================
/**
 * @brief   キーを監視してゲームスタートを見る
 * @param   pWork  ゲームのワーク
 * @retval  none
 */
//==============================================================================

static void _gameStartCheck(SKEL_TEST_BEACON_WORK* pWork)
{
    int i;

    if(PAD_BUTTON_X == GFL_UI_KEY_GetTrg()){  // バッファを消す
        GFL_STD_MemClear(pWork->rssiDir,sizeof(u16)*_DIR_SAMPL_NUM*_BCON_GET_NUM);
        pWork->dir = 0;

        for(i = 0;i < _BCON_GET_NUM;i++){
            _DeleteFallChr(pWork,i);
        }
    }


    if(PAD_BUTTON_B == GFL_UI_KEY_GetTrg()){
        if((pWork->dir == 0) && (pWork->countDir==0)){
            pWork->countDir = _SAMP_TIMER;

        }
    }
}

//==============================================================================
/**
 * @brief   ゲームの実行、一定間隔でサンプリング方向を進める
 * @param   pWork  ゲームのワーク
 * @retval  none
 */
//==============================================================================

static BOOL _gameStartFunc(SKEL_TEST_BEACON_WORK* pWork)
{
    int i,msgindex=0;
    char msg[100];

//    pWork->bmpwin[_GAME_BMPWIN+i];

    GFL_STD_MemClear(msg,100);

    for(i = 0;i < _DIR_SAMPL_NUM; i++){
        if(pWork->dir > i){
            msg[i]='o';
        }
        else{
            msg[i]='x';
        }
    }
    
    GFL_BMPWIN_ClearScreen(pWork->bmpwin[ _GAME_BMPWIN+1 ]);
    pWork->textParam->bmp = GFL_BMPWIN_GetBmp( pWork->bmpwin[ _GAME_BMPWIN+1  ] );
    GFL_BMP_Clear(pWork->textParam->bmp,0);
    GFL_TEXT_PrintSjisCode( msg, pWork->textParam );
    GFL_BMPWIN_TransVramCharacter( pWork->bmpwin[_GAME_BMPWIN+1 ] );
    GFL_BMPWIN_MakeScreen( pWork->bmpwin[_GAME_BMPWIN+1 ] );

    if(pWork->countDir){
        pWork->countDir--;
        if(pWork->countDir==0){
            pWork->countDir = _SAMP_TIMER;
            pWork->dir++;
        }
        if(pWork->dir == _DIR_SAMPL_NUM){ // 収集完了
            pWork->countDir = 0;
            pWork->dir=0;
            return TRUE;
        }
    }
    return FALSE;
}

//==============================================================================
/**
 * @brief   アクセスポイント結果表示
 * @param   pWork  ゲームのワーク
 * @retval  none
 */
//==============================================================================

static void _APDisp(SKEL_TEST_BEACON_WORK* pWork,int index)
{
    u16 max=0,dir=0,j;
    fx16 posx,posy;
    u16 x,y;
    s16 sx,sy;

    for(j =0; j < _DIR_SAMPL_NUM;j++){
        if(pWork->rssiDir[index][j] > max){
            dir = j;
            max = pWork->rssiDir[index][j];
        }
    }

    if(max==0){
        return;
    }
    if(max >= 140){
        max = 140;
    }
    
    
    posx = FX_SinIdx((0xffff/_DIR_SAMPL_NUM)*dir);
    posx = (posx >> (FX16_SHIFT/2));
    posx = posx * (((140-max)));
    sx = (s16)(posx >> (FX16_SHIFT/2));
    
    posy = FX_CosIdx((0xffff/_DIR_SAMPL_NUM)*dir);
    posy = posy >> (FX16_SHIFT/2);
    posy = posy * (((140-max)));
    sy = (s16)(posy >> (FX16_SHIFT/2));
    x = (u16)(sx + 128);
    y = (u16)(96 - sy);

    _InitAPChr(pWork, x, y, 144);
}


//----------------------------------------------------------------


static void _dispBeacon(SKEL_TEST_BEACON_WORK* pWork)
{
    int i;
    BOOL bEndScan;


	GFL_BG_ClearScreen( TEXT_FRM );

    
    for(i=0;i<NELEMS(_parentIndexList);i++){
        GFL_BMPWIN_MakeScreen( pWork->bmpwin[i] );
    }

    _gameStartCheck(pWork);
    if( _gameStartFunc(pWork) ){
        for(i = 0;i < _BCON_GET_NUM; i++){
            _APDisp(pWork,i);
        }
    }
    for(i = 0;i < _BCON_GET_NUM; i++){
        u8* pData = GFL_NET_GetBeaconMacAddress(i);
        
        if(pData){
//            OS_TPrintf("%d: mac %x\n",i,);
            char msg[100];
            u16 rssi = GFL_NET_WL_GetRssi(i);

            rssi = _backupRssi(pWork,i,rssi);
            
            if(pWork->countDir){
                pWork->rssiDir[i][pWork->dir] = rssi;
            }
            
            GFL_STD_MemClear(msg,100);
            OS_SPrintf(msg,"%d : %d %d %d %d %d %d %d %d",rssi,
                       pWork->rssiDir[i][0],
                       pWork->rssiDir[i][1],
                       pWork->rssiDir[i][2],
                       pWork->rssiDir[i][3],
                       pWork->rssiDir[i][4],
                       pWork->rssiDir[i][5],
                       pWork->rssiDir[i][6],
                       pWork->rssiDir[i][7]  );

            GFL_BMPWIN_ClearScreen(pWork->pBmpwinBeacon[ i ]);
            
            pWork->textParam->bmp = GFL_BMPWIN_GetBmp( pWork->pBmpwinBeacon[ i ] );
            GFL_BMP_Clear(pWork->textParam->bmp,0);
            GFL_TEXT_PrintSjisCode( msg, pWork->textParam );

            //ビットマップキャラクターをアップデート
            GFL_BMPWIN_TransVramCharacter( pWork->pBmpwinBeacon[i] );

            GFL_BMPWIN_MakeScreen( pWork->pBmpwinBeacon[i] );

        }
        else{
            pWork->textParam->bmp = GFL_BMPWIN_GetBmp( pWork->pBmpwinBeacon[ i ] );
            GFL_BMP_Clear(pWork->textParam->bmp,0);
            GFL_BMPWIN_TransVramCharacter( pWork->pBmpwinBeacon[i] );
            GFL_BMPWIN_MakeScreen( pWork->pBmpwinBeacon[i] );
        }

    }

	GFL_BG_LoadScreenReq( TEXT_FRM );
    clact_draw(pWork);

}


static void rssiWindowInit(SKEL_TEST_BEACON_WORK* pWork)
{
    int i;
    
    for(i = 0;i < _BCON_GET_NUM; i++){
        pWork->pBmpwinBeacon[i] = GFL_BMPWIN_Create( TEXT_FRM,
                                                     4, 5+i*2, 24, 1, 0, 
                                                     GFL_BG_CHRAREA_GET_B );
    }
}


static void _testChild(SKEL_TEST_BEACON_WORK* pWork)
{
    u8 beacon;
    int i;

    if(PAD_BUTTON_X == GFL_UI_KEY_GetTrg()){
        for(i = 0;i < _BCON_GET_NUM; i++){
            u8* pData = GFL_NET_GetBeaconMacAddress(i);
            if(pData){
                OS_TPrintf("%d: mac %x%x%x%x%x%x\n",i,pData[0],pData[1],pData[2],pData[3],pData[4],pData[5]);
            }
        }
    }

    bg_init( pWork->heapID );
    //画面作成
    g2d_load(pWork);	//２Ｄデータ作成


    rssiWindowInit(pWork);
        
    pWork->_pHandle = GFL_NET_CreateHandle();
    GFL_NET_StartBeaconScan(pWork->_pHandle );    // ビーコンを待つ
    _CHANGE_STATE(_dispBeacon, 0);
}

//            u8* pData = GFL_NET_GetBeaconMacAddress(0);//ビーコンリストの0番目を得る


static void _testParent(SKEL_TEST_BEACON_WORK* pWork)
{

    bg_init( pWork->heapID );    //画面作成
    g2d_load(pWork);	//２Ｄデータ作成
    // サーバー側のテスト  ビーコン出しているだけ
    pWork->_pHandleServer = GFL_NET_CreateHandle();
    GFL_NET_InitServer(pWork->_pHandleServer);   // サーバ
    pWork->_pHandle = GFL_NET_CreateHandle();  // クライアント
    _CHANGE_STATE(_noneFunc, 0);

}

void TEST_BEACON_Main(SKEL_TEST_BEACON_WORK* pWork)
{

    if(pWork){
        _PTRFunc state = pWork->state;
        if(state != NULL){
            state(pWork);
        }
    }
}



#define SSID "GF_ORG01"

static u8* _netGetSSID(void)
{
    return (u8*)SSID;
}


// 可変サイズ受信
static void _testRecvVariable(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const u8* pBuff=pData;
    NET_PRINT("_testRecvVariable %d:  %d : %d %d\n",__LINE__,size,pBuff[0],pBuff[9]);
}


static void _testRecvGetSize(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const u8* pBuff=pData;
    NET_PRINT("_testRecvGetSize %d:  %d : %d %d\n",__LINE__,size,pBuff[0],pBuff[9]);
}

static int _getTestCommandSize(void)
{
    return 12;
}


//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターリソース読み込み
 *
 *	@param	clact_res	ワーク
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void _ClactResourceLoad( SKEL_TEST_BEACON_WORK *pWork, u32 heapID )
{
	BOOL result;
	void* p_buff;
	NNSG2dCharacterData* p_char;
	NNSG2dPaletteData* p_pltt;
	
	// キャラクタデータ読み込み＆転送
	{
		p_buff = GFL_ARC_LoadDataAlloc( 0,NARC_radar_ug_radar_obj_NCGR, heapID );
		result = NNS_G2dGetUnpackedCharacterData( p_buff, &p_char );
		GF_ASSERT( result );
		NNS_G2dInitImageProxy( &pWork->res.imageproxy );
		NNS_G2dLoadImage1DMapping( 
				p_char, 
				0, 
				NNS_G2D_VRAM_TYPE_2DMAIN, 
				&pWork->res.imageproxy );
		NNS_G2dLoadImage1DMapping( 
				p_char, 
				0, 
				NNS_G2D_VRAM_TYPE_2DSUB, 
				&pWork->res.imageproxy );
		GFL_HEAP_FreeMemory( p_buff );
	}

	// パレットデータ読み込み＆転送
	{
		p_buff = GFL_ARC_LoadDataAlloc( 0,NARC_radar_ug_radar_obj_NCLR, heapID );
		result = NNS_G2dGetUnpackedPaletteData( p_buff, &p_pltt );
		GF_ASSERT( result );
		NNS_G2dInitImagePaletteProxy( &pWork->res.plttproxy );
		NNS_G2dLoadPalette( 
				p_pltt, 
				0, 
				NNS_G2D_VRAM_TYPE_2DMAIN, 
				&pWork->res.plttproxy );
		NNS_G2dLoadPalette( 
				p_pltt, 
				0, 
				NNS_G2D_VRAM_TYPE_2DSUB, 
				&pWork->res.plttproxy );
		GFL_HEAP_FreeMemory( p_buff );
	}

	// セルデータ読み込み
	{
		pWork->res.p_cellbuff = GFL_ARC_LoadDataAlloc( 0,NARC_radar_ug_radar_obj_NCER, heapID );
		result = NNS_G2dGetUnpackedCellBank( pWork->res.p_cellbuff, &pWork->res.p_cell );
		GF_ASSERT( result );
	}

	// セルアニメデータ読み込み
	{
		pWork->res.p_cellanmbuff = GFL_ARC_LoadDataAlloc( 0,NARC_radar_ug_radar_obj_NANR, heapID );
		result = NNS_G2dGetUnpackedAnimBank( pWork->res.p_cellanmbuff, &pWork->res.p_cellanm );
		GF_ASSERT( result );
	}
}

//------------------------------------------------------------------
/**
 * @brief		画面の初期化
 */
//------------------------------------------------------------------

static void _grapInit(SKEL_TEST_BEACON_WORK* pWork)
{
    // セルアクターユニット作成
	pWork->p_unit = GFL_CLACT_UNIT_Create( _BCON_GET_NUM, pWork->heapID );
	//エリアマネージャ初期化
	pWork->clact_area=GFL_AREAMAN_Create(_BCON_GET_NUM, pWork->heapID);
	
	//BGシステム初期化
	GFL_BG_Init(pWork->heapID);

	//VRAM設定
	{
		GFL_BG_DISPVRAM vramSetTable = {
			GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
			GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
			GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
			GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
			GX_VRAM_OBJ_64_E,				// メイン2DエンジンのOBJ
			GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
			GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
			GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
			GX_VRAM_TEX_0_B,				// テクスチャイメージスロット
			GX_VRAM_TEXPLTT_0_F				// テクスチャパレットスロット
		};
		GFL_DISP_SetBank( &vramSetTable );

		//VRAMクリア
		MI_CpuClear32((void*)HW_BG_VRAM, HW_BG_VRAM_SIZE);
		MI_CpuClear32((void*)HW_DB_BG_VRAM, HW_DB_BG_VRAM_SIZE);
		MI_CpuClear32((void*)HW_OBJ_VRAM, HW_OBJ_VRAM_SIZE);
		MI_CpuClear32((void*)HW_DB_OBJ_VRAM, HW_DB_OBJ_VRAM_SIZE);
	}

	// BG SYSTEM
    GFL_BG_SetBGMode( &bgsysHeader );

	//メイン画面フレーム設定
	{
		GFL_BG_BGCNT_HEADER TextBgCntDat[] = {
			///<FRAME1_M
			{
				0, 0, 0x2000, 0, GFL_BG_SCRSIZ_512x512, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
				GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
			},
			///<FRAME2_M
			{
				0, 0, 0x2000, 0, GFL_BG_SCRSIZ_512x512, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x256,
				GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
			},
			///<FRAME3_M
			{
				0, 0, 0x2000, 0, GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x4000, GX_BG_CHARBASE_0x18000, GFL_BG_CHRSIZ_256x256,
				GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
			},
			///<FRAME2_S
			{
				0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
				GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
			},
			///<FRAME3_S
			{
				0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
				GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
			},
		};
		GFL_BG_SetBGControl(GFL_BG_FRAME1_M, &TextBgCntDat[0], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME1_M);
		GFL_BG_SetBGControl(GFL_BG_FRAME2_M, &TextBgCntDat[1], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME2_M);
		GFL_BG_SetBGControl(GFL_BG_FRAME3_M, &TextBgCntDat[2], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME3_M);
		GFL_BG_SetBGControl(GFL_BG_FRAME2_S, &TextBgCntDat[3], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME2_S);
		GFL_BG_SetBGControl(GFL_BG_FRAME3_S, &TextBgCntDat[4], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME3_S);

		// OBJマッピングモード
		GX_SetOBJVRamModeChar( GX_OBJVRAMMODE_CHAR_1D_32K );
		GXS_SetOBJVRamModeChar( GX_OBJVRAMMODE_CHAR_1D_32K );

		GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
		GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
	}

	//画面生成
	GFL_ARC_UTIL_TransVramBgCharacter(0,NARC_radar_ug_radar_NCGR,GFL_BG_FRAME2_M,0,0,0,pWork->heapID);
	GFL_ARC_UTIL_TransVramScreen(0,NARC_radar_ug_radar_NSCR,GFL_BG_FRAME2_M,0,0,0,pWork->heapID);
	GFL_ARC_UTIL_TransVramPalette(0,NARC_radar_ug_radar_NCLR,PALTYPE_MAIN_BG,0,0x100,pWork->heapID);

	GFL_DISP_SetDispOn();
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_SUB );

	GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN|GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB,16,0,2);

	//セルアクターリソース読み込み
	_ClactResourceLoad(pWork, pWork->heapID);


	//BMP関連初期化
	GFL_BMPWIN_Init(pWork->heapID);

    // 通信アイコン再描画
    GFL_NET_ReloadIcon();


}

// ローカル通信テーブル
static const NetRecvFuncTable _CommPacketTbl[] = {
    // 可変サイズテスト
    {_testRecvVariable,          GFL_NET_COMMAND_VARIABLE, NULL},
    // サイズ取得関数テスト
    {_testRecvGetSize,           _getTestCommandSize, NULL},
};

#define _MAXNUM (2)     // 接続最大
#define _MAXSIZE (120)  // 送信最大サイズ

// 通信初期化構造体  wifi用
static GFLNetInitializeStruct aGFLNetInit = {
    _CommPacketTbl,  // 受信関数テーブル
    NELEMS(_CommPacketTbl), // 受信テーブル要素数
    NULL,   // ワークポインタ
    _netBeaconGetFunc,  // ビーコンデータ取得関数
    _netBeaconGetSizeFunc,  // ビーコンデータサイズ取得関数
    _netBeaconCompFunc,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
    FatalError_Disp,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
    NULL,  // 通信切断時に呼ばれる関数
    NULL,  // オート接続で親になった場合
    NET_ICONDATA_GetTableData,   // 通信アイコンのファイルARCテーブルを返す関数
    NET_ICONDATA_GetNoBuff,      // 通信アイコンのファイルARCの番号を返す関数
#if GFL_NET_WIFI
    NULL,     ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
    NULL, ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
    NULL,   ///< DWC形式の友達リスト	
    NULL,  ///< DWCのユーザデータ（自分のデータ）
#endif  //GFL_NET_WIFI
    _netGetSSID,  // 親子接続時に認証する為のバイト列  
    1,  //gsid
    0,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
    GFL_HEAPID_APP,  //元になるheapid
    HEAPID_NETWORK,  //通信用にcreateされるHEAPID
    HEAPID_WIFI,  //wifi用にcreateされるHEAPID
    GFL_WICON_POSX,GFL_WICON_POSY,        // 通信アイコンXY位置
    _MAXNUM,     // 最大接続人数
    _MAXSIZE,  //最大送信バイト数
    _BCON_GET_NUM,    // 最大ビーコン収集数
    FALSE,  //CRCを自動計算するかどうか TRUEの場合すべて計算する
    FALSE,     // MP通信＝親子型通信モードかどうか
    FALSE,  //wifi通信を行うかどうか
    TRUE,     // 通信を開始するかどうか
};

//----------------------------------------------------------------------------
/**
 *	@brief	初期化処理
 */
//-----------------------------------------------------------------------------

SKEL_TEST_BEACON_WORK* TEST_BEACON_Init(HEAPID heapID)
{

    SKEL_TEST_BEACON_WORK* pWork = GFL_HEAP_AllocMemory(heapID, sizeof(SKEL_TEST_BEACON_WORK));
    GFL_STD_MemClear(pWork, sizeof(SKEL_TEST_BEACON_WORK));
    pWork->heapID = heapID;

    //イクニューモンを使用する前に VRAMDをdisableにする必要があるのだが
    //VRAMDが何に使われていたのかがわからないと、消すことができない
    
    if(GX_VRAM_LCDC_D == GX_GetBankForLCDC()){
        GX_DisableBankForLCDC(); // LCDCにVRAMDが割り当てられてるようなので打ち消す
    }
    else if(GX_VRAM_D & GX_GetBankForBG()){
        GX_DisableBankForBG();
    }
    else if(GX_VRAM_D & GX_GetBankForTex()){
        GX_DisableBankForTex();
    }
    else if(GX_VRAM_D & GX_GetBankForClearImage()){
        GX_DisableBankForClearImage();
    }
    else if(GX_VRAM_D & GX_GetBankForSubOBJ()){
        GX_DisableBankForSubOBJ();
    }

    GX_DisableBankForLCDC(); // LCDCにVRAMDが割り当てられてるようなので打ち消す
    GX_DisableBankForBG();
    GX_DisableBankForTex();
    GX_DisableBankForClearImage();
    GX_DisableBankForSubOBJ();

    _grapInit(pWork);
    
    GFL_NET_Init(&aGFLNetInit);

    if(PAD_BUTTON_R & GFL_UI_KEY_GetCont()){
        OS_TPrintf("親になった\n");
        pWork->bParent = TRUE;
        _CHANGE_STATE(_testParent,0);
    }
    else{
        OS_TPrintf("子になった\n");
        pWork->bParent = FALSE;
        _CHANGE_STATE(_testChild,0);
    }

    return pWork;
}

//----------------------------------------------------------------------------
/**
 *	@brief	終了処理
 */
//-----------------------------------------------------------------------------

void TEST_BEACON_End(SKEL_TEST_BEACON_WORK* pWork)
{
    GFL_HEAP_FreeMemory(pWork);
}


//----------------------------------------------------------------------------
/**
 *	@brief	CLACT登録
 *  @param  ワークポインタ
 *	@return	生成したワーク
 */
//-----------------------------------------------------------------------------
static GFL_CLWK* _ClactWorkAdd(SKEL_TEST_BEACON_WORK *pWork, _ACCESS_POINT_PARAM* pAP,
                               s16 posx, s16 posy)
{
    GFL_CLWK_RES	resdat;
	GFL_CLWK		*p_wk;
	GFL_CLWK_DATA	data={
        0,0,
        NARC_radar_ug_radar_obj_NANR,		//アニメーションシーケンス
        0,		//優先順位
        0		//bg優先順位
    };

    data.pos_x = posx;
    data.pos_y = posy;

	// リソースデータ代入
	GFL_CLACT_WK_SetCellResData( &resdat, 
                                 &pWork->res.imageproxy, &pWork->res.plttproxy,
                                 pWork->res.p_cell, pWork->res.p_cellanm );
	// 登録
	//data.anmseq+=fcp->type;
//	data.softpri=soft_pri[fcp->type];
//	data.bgpri=bg_pri[fcp->type];
	p_wk = GFL_CLACT_WK_Add( pWork->p_unit, 
			&data, &resdat,
			CLWK_SETSF_NONE,
			pWork->heapID );
    GF_ASSERT(p_wk);

	// オートアニメーション設定
	GFL_CLACT_WK_SetAutoAnmFlag( p_wk, TRUE );

	return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief	AP CLACT生成
 *  @param  ワークポインタ
 *	@return	生成したワーク
 */
//-----------------------------------------------------------------------------
static _ACCESS_POINT_PARAM	*_InitAPChr(SKEL_TEST_BEACON_WORK *pWork, u16 xpos, u16 ypos, u16 rssi)
{
	int clact_no=GFL_AREAMAN_ReserveAuto(pWork->clact_area,1);

    GF_ASSERT(clact_no < _BCON_GET_NUM);
    
    pWork->pAP[clact_no].clact_no = clact_no;
	pWork->clact_work[clact_no] = _ClactWorkAdd(pWork, &pWork->pAP[clact_no],xpos,ypos);
    pWork->pAP[clact_no].clwk = pWork->clact_work[clact_no];
	GFL_CLACT_WK_GetWldPos(pWork->pAP[clact_no].clwk, &pWork->pAP[clact_no].now_pos);

    
	return &pWork->pAP[clact_no];
}

//----------------------------------------------------------------------------
/**
 *	@brief	キャラ削除
 *	@param	clact_no		落下ラインナンバー
 *	@retval void
 */
//-----------------------------------------------------------------------------
static void _DeleteFallChr(SKEL_TEST_BEACON_WORK* pWork,int clact_no)
{

    if(pWork->pAP[clact_no].clwk){
        GFL_CLACT_WK_Remove(pWork->pAP[clact_no].clwk);
    }
    pWork->pAP[clact_no].clwk=NULL;
}
