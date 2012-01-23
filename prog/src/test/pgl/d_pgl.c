//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\test\pgl\d_pgl.c
 *	@brief
 *	@author		Katsuhiko=Ichiraku
 *	@date		2011.11.26
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifdef PM_DEBUG

#include <wchar.h>    // wcslen
#include <gflib.h>
#include "debug_pgl_menu_setup.h"
#include "system/main.h"
#include "arc/arc_def.h"
#include "debug/cgearpict.naix"
#include "font/font.naix"
#include "print/printsys.h"
#include "gamesystem/game_data.h"

#include "savedata/save_tbl.h"
#include "savedata/wifilist.h"
#include "savedata/dreamworld_data.h"
#include "savedata/mystatus.h"
#include "../../savedata/mystatus_local.h"
#include "savedata/record.h"
#include "savedata/c_gear_data.h"
#include "savedata/c_gear_picture.h"
#include "savedata/save_tbl.h"
#include "savedata/medalbox.h"
#include "../../savedata/medalbox_local.h"
#include "savedata/situation.h"
#include "savedata/mystery_data.h"
#include "savedata/record.h"
#include "system\pms_data.h"
#include "savedata/worldtrade_data.h"
#include "savedata/sodateya_work.h"
#include "savedata/misc.h"
#include "savedata/misc_local.h"
#include "savedata/intrude_save.h"
#include "savedata/system_data_local.h"
#include "savedata/dreamworld_data_local.h"
#include "poke_tool/pokeparty.h"
#include "savedata/wifihistory.h"
#include "savedata/wifihistory_local.h"
#include "savedata/playtime.h"
#include "savedata/playtime_local.h"
#include "savedata/regulation.h"
#include "savedata/symbol_save.h"
#include "savedata/symbol_save_local.h"
#include "savedata/myitem_savedata_local.h"
#include "net/dreamworld_netdata.h"
#include "savedata/zukan_savedata_local.h"
#include "savedata/symbol_save.h"

#include "savedata/wifi_negotiation.h"

#include "savedata/wifilist.h"
#include "savedata/wifilist_local.h"

extern MYITEM_PTR* SaveData_GetMyItem(SAVE_CONTROL_WORK * sv);
extern WIFI_LIST* SaveData_GetWifiListData(SAVE_CONTROL_WORK * sv);

FS_EXTERN_OVERLAY(pgl_debug);

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
// 選択状態
typedef enum
{
  SELECT_STATE_NONE,    // 未選択
  SELECT_STATE_DECIDE,  // 決定
  SELECT_STATE_EXIT,    // 終了
} SELECT_STATE;

// メニュー項目インデックス
typedef enum
{
  MENU_ITEM_CGEAR,
  MENU_ITEM_SAVE,
  MENU_ITEM_EXSAVE,
  MENU_ITEM_SAVETABLE,
  MENU_ITEM_SAVECRC,
  MENU_ITEM_DLSAVE_INFO,
  MENU_ITEM_RETURN,

  MENU_ITEM_NUM
} MENU_ITEM;

// メニュー項目文字列
static const STRCODE* str_menu[] =
{
  L"Cギアデータセット",
  L"セーブアドレス",
  L"セーブアドレスEX",
  L"セーブブロックテーブル",
  L"セーブCRCアドレス",
  L"DLセーブ解析用データ出力",
  L"戻る",
};

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	プロセスワーク
//=====================================
typedef struct {
  GFL_FONT*   font;
  GFL_BMPWIN* bmpWin;
  int         curPos; // 選択項目
  SELECT_STATE state; // 選択状態
	SAVE_CONTROL_WORK* pSaveData;
  GAMEDATA      *pGameData;
  u8* pCGearWork;
} PROC_WORK;

//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================

static void Init( PROC_WORK *p_wk );
static void Exit( PROC_WORK *p_wk );
static void Main( PROC_WORK *p_wk );
static void UpdateMenuList( PROC_WORK *p_wk );
static void SetNextFunc( PROC_WORK *p_wk );

// 処理関数
// メニューの数と同じ
static void NextFunc_CGearDummyData( PROC_WORK *p_wk );
static void NextFunc_SaveAdrs( PROC_WORK *p_wk );
static void NextFunc_ExSaveAdrs( PROC_WORK * p_wk );
static void NextFunc_SaveAdrsTable( PROC_WORK * p_wk );
static void NextFunc_SaveAdrsCRC( PROC_WORK * p_wk );
static void NextFunc_DLSaveInfo( PROC_WORK * p_wk );

//============================================================================================
/**
 * @brief 初期化関数
 */
//============================================================================================
static GFL_PROC_RESULT DEBUG_PGL_MainProcFunc_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  PROC_WORK* p_wk = NULL;

  // ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_PGL_DEBUG, 0x80000 );

  // ワークを作成
  p_wk = GFL_PROC_AllocWork( proc, sizeof( PROC_WORK ), HEAPID_PGL_DEBUG );
  p_wk->pGameData = GAMEDATA_Create( HEAPID_PGL_DEBUG );
  p_wk->pSaveData = SaveControl_GetPointer();  //デバッグ

  // 初期化処理
  DEBUG_OBATA_MENU_SETUP_Init( HEAPID_PGL_DEBUG );
  Init( p_wk );

  return GFL_PROC_RES_FINISH;
}


//============================================================================================
/**
 * @brief メイン関数
 */
//============================================================================================
static GFL_PROC_RESULT DEBUG_PGL_MainProcFunc_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  PROC_WORK* p_wk = mywk;

  switch( *seq )
  {
  case 0:
    Main( p_wk );
    if( p_wk->state != SELECT_STATE_NONE )
    {
      ++( *seq );
    }
    break;
  case 1:
    return GFL_PROC_RES_FINISH;
  }

  return GFL_PROC_RES_CONTINUE;
}


//============================================================================================
/**
 * @brief 終了関数
 */
//============================================================================================
static GFL_PROC_RESULT DEBUG_PGL_MainProcFunc_End( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  PROC_WORK* p_wk = mywk;

  // 次のプロセスを設定
  SetNextFunc( p_wk );

  // 終了処理
  Exit( p_wk );
  DEBUG_OBATA_MENU_SETUP_Exit();

  GAMEDATA_Delete(p_wk->pGameData);

  // ワークを破棄
  GFL_PROC_FreeWork( proc );

  // ヒープを破棄
  GFL_HEAP_DeleteHeap( HEAPID_PGL_DEBUG );

  return GFL_PROC_RES_FINISH;
}


//============================================================================================
/**
 * @brief プロセス定義データ
 */
//============================================================================================
const GFL_PROC_DATA DebugPGLProcData =
{
  DEBUG_PGL_MainProcFunc_Init,
  DEBUG_PGL_MainProcFunc_Main,
  DEBUG_PGL_MainProcFunc_End,
};


//=============================================================================
/**
 *					static関数
*/
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  初期化
 */
//-----------------------------------------------------------------------------
static void Init( PROC_WORK *p_wk )
{
  // 初期化
  p_wk->curPos = 0;
  p_wk->state = SELECT_STATE_NONE;

  // フォントデータハンドラ作成
  p_wk->font = GFL_FONT_Create(
      ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_PGL_DEBUG );

  // ビットマップウィンドウを作成
  p_wk->bmpWin = GFL_BMPWIN_Create( GFL_BG_FRAME1_M, 0, 0, 32, 24, 0, GFL_BMP_CHRAREA_GET_F );

  // メニューリストの更新
  UpdateMenuList( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  終了
 */
//-----------------------------------------------------------------------------
static void Exit( PROC_WORK *p_wk )
{
  // ビットマップウィンドウの破棄
  GFL_BMPWIN_Delete( p_wk->bmpWin );

  // フォントデータハンドラ破棄
  GFL_FONT_Delete( p_wk->font );
}

//----------------------------------------------------------------------------
/**
 *	@brief  メイン
 */
//-----------------------------------------------------------------------------
static void Main( PROC_WORK *p_wk )
{
  int      trg = GFL_UI_KEY_GetTrg();
  int move_dir = 0;

  if( trg & PAD_KEY_UP ) move_dir = -1;
  if( trg & PAD_KEY_DOWN ) move_dir = 1;
  if( trg & PAD_BUTTON_A ) p_wk->state = SELECT_STATE_DECIDE;
  if( trg & PAD_BUTTON_SELECT ) p_wk->state = SELECT_STATE_EXIT;

  // カーソルが移動した場合
  if( move_dir != 0 )
  {
    // カーソル位置を補正
    p_wk->curPos = p_wk->curPos + move_dir;
    if( p_wk->curPos < 0 ) p_wk->curPos = 0;
    p_wk->curPos = p_wk->curPos % MENU_ITEM_NUM;

    // メニュー項目更新
    UpdateMenuList( p_wk );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  メニューリスト更新
 */
//-----------------------------------------------------------------------------
static void UpdateMenuList( PROC_WORK *p_wk )
{
  // ビットマップウィンドウに文字を書き込む
  {
    int i, len;
    STRBUF* strbuf;
    STRCODE strcode[256];
    GFL_BMP_DATA* bmp;
    PRINTSYS_LSB lsb;


    // 書き込み
    for( i=0; i<MENU_ITEM_NUM; i++ )
    {
      // バッファ作成
      strbuf = GFL_STR_CreateBuffer( 256, HEAPID_PGL_DEBUG );

      // 文字色決定
      if( i == p_wk->curPos )
      {
        lsb = PRINTSYS_LSB_Make(1, 3, 0);
      }
      else
      {
        lsb = PRINTSYS_LSB_Make(1, 2, 0);
      }

      // 文字列作成
      len = wcslen( str_menu[i] );
      GFL_STD_MemCopy( str_menu[i], strcode, sizeof( STRCODE )*len );
      strcode[len] = GFL_STR_GetEOMCode();
      GFL_STR_SetStringCode( strbuf, strcode );

      // ビットマップに書き込み
      bmp = GFL_BMPWIN_GetBmp( p_wk->bmpWin );
      PRINTSYS_PrintColor( bmp, 0, i*16, strbuf, p_wk->font, lsb );

      // バッファ破棄
      GFL_STR_DeleteBuffer( strbuf );
    }
  }

  // ビットマップウィンドウ内部バッファをVRAMへ転送
  GFL_BMPWIN_TransVramCharacter( p_wk->bmpWin );

  // ビットマップ用スクリーンをBGスクリーンバッファに作成
  GFL_BMPWIN_MakeScreen( p_wk->bmpWin );

  // BGスクリーンデータを反映させる
  GFL_BG_LoadScreenReq( GFL_BG_FRAME1_M );
}

//----------------------------------------------------------------------------
/**
 *	@brief  選択した処理
 */
//-----------------------------------------------------------------------------
static void SetNextFunc( PROC_WORK *p_wk )
{
  // カーソルの位置で決定
  switch( p_wk->curPos ){
  case MENU_ITEM_CGEAR:
    NextFunc_CGearDummyData(p_wk);
    OS_Printf("CGEAR\n");
    break;
  case MENU_ITEM_SAVE:
    NextFunc_SaveAdrs(p_wk);
    OS_Printf("SAVE\n");
    break;
  case MENU_ITEM_EXSAVE:
    NextFunc_ExSaveAdrs(p_wk);
    OS_Printf("SAVE\n");
    break;
  case MENU_ITEM_SAVETABLE:
    NextFunc_SaveAdrsTable(p_wk);
    break;
  case MENU_ITEM_SAVECRC:
    NextFunc_SaveAdrsCRC(p_wk);
    break;
  case MENU_ITEM_DLSAVE_INFO:
    NextFunc_DLSaveInfo(p_wk);
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  Cギアダミーデータセット
 *	大野さんのソースのコピー
 *	test/ohno/debug_saveaddr.c
 */
//-----------------------------------------------------------------------------
static void NextFunc_CGearDummyData( PROC_WORK *p_wk )
{
  int i;
  u16 crc=0;
  ARCHANDLE* p_handle;
  void* pCRC;
  int size = CGEAR_PICTURTE_CHAR_SIZE+CGEAR_PICTURTE_PAL_SIZE+CGEAR_PICTURTE_SCR_SIZE;
  int sizeh = size/2;
  CGEAR_PICTURE_SAVEDATA* pPic;
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->pGameData);

  p_handle = GFL_ARC_OpenDataHandle( ARCID_DEBUG_CGEAR_PICT, HEAPID_PGL_DEBUG );

  pCRC = GFL_ARCHDL_UTIL_Load( p_handle, NARC_cgearpict_mun_decal01_bin, FALSE, HEAPID_PGL_DEBUG);

  crc = GFL_STD_CrcCalc( pCRC, size );

  
  p_wk->pCGearWork = GFL_HEAP_AllocMemory(HEAPID_PGL_DEBUG,SAVESIZE_EXTRA_CGEAR_PICTURE);
  pPic = (CGEAR_PICTURE_SAVEDATA*)p_wk->pCGearWork;

  CGEAR_SV_SetCGearPictureONOFF(CGEAR_SV_GetCGearSaveData(pSave),TRUE);  //CGEARデカール有効

  SaveControl_Extra_LoadWork(pSave, SAVE_EXTRA_ID_CGEAR_PICUTRE, HEAPID_PGL_DEBUG,
                             p_wk->pCGearWork,SAVESIZE_EXTRA_CGEAR_PICTURE);
  GFL_STD_MemCopy(pCRC, pPic->picture, size);
  CGEAR_SV_SetCGearPictureCRC(CGEAR_SV_GetCGearSaveData(pSave),crc);  //CGEARデカール有効
  GAMEDATA_ExtraSaveAsyncStart(p_wk->pGameData, SAVE_EXTRA_ID_CGEAR_PICUTRE);

  while(1){
    SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->pGameData);
    if(SAVE_RESULT_OK==GAMEDATA_ExtraSaveAsyncMain(p_wk->pGameData,SAVE_EXTRA_ID_CGEAR_PICUTRE)){
      break;
    }
  }
  SaveControl_Extra_UnloadWork(pSave, SAVE_EXTRA_ID_CGEAR_PICUTRE);

  GFL_HEAP_FreeMemory(p_wk->pCGearWork);
  GFL_HEAP_FreeMemory(pCRC);
  GFL_ARC_CloseDataHandle( p_handle );
}

//----------------------------------------------------------------------------
/**
 *	@brief  セーブデータアドレス出力：PGLでのセーブデータ解析の資料用
 *	大野さんのソースのコピー
 *	test/ohno/debug_saveaddr.c
 */
//-----------------------------------------------------------------------------
static void NextFunc_SaveAdrs( PROC_WORK *p_wk )
{
  {
    int i;
    u32 size;
    u8* pAddr;
    u8 const * topAddr = (u8*)SaveControl_GetSaveWorkAdrs(p_wk->pSaveData, &size);

    OS_TPrintf("SAVESIZE ,%x\n", size);

    {
      u8 data[]="abcdefg";
      OS_TPrintf("CRC ,%x %d\n", GFL_STD_CrcCalc(data,7) , sizeof(REGULATION_CARDDATA));
    }
    {
      DREAM_WORLD_SERVER_DOWNLOAD_DATA buff;

      GFL_STD_MemFill(&buff, 0, sizeof(buff));

      // buff に値を設定
//      buff.findPokemon = 100;
//      buff.findPokemon = 50;
 //     buff.findPokemonForm = 1;
      
      OS_TPrintf("CRC ,%x %d\n", GFL_STD_CrcCalc(&buff,sizeof(buff)-2) , sizeof(buff));
    }
    

    {  //シンボルエンカウント
      SYMBOL_SAVE_WORK* pSym = SymbolSave_GetSymbolData(p_wk->pSaveData);

      SymbolSave_WorkInit(pSym);
      SymbolSave_SetFreeZone(pSym, 12, 12, 0, 0, 0, SYMBOL_ZONE_TYPE_FREE_SMALL);
      SymbolSave_SetFreeZone(pSym, 13, 13, 0, 0, 0, SYMBOL_ZONE_TYPE_FREE_SMALL);
      SymbolSave_SetFreeZone(pSym, 14, 14, 0, 0, 0, SYMBOL_ZONE_TYPE_FREE_SMALL);
      {
        SYMBOL_POKEMON aSymbol;
        SymbolSave_GetSymbolPokemon(pSym, 0, &aSymbol);
        OS_TPrintf("mons %d",aSymbol.monsno);
        OS_TPrintf("waza %d ",aSymbol.wazano);
        OS_TPrintf("fem %d",aSymbol.sex);
        OS_TPrintf("move %d",aSymbol.move_type);
        OS_TPrintf("form %d\n",aSymbol.form_no);
      }
      {
        SYMBOL_POKEMON aSymbol;
        SymbolSave_GetSymbolPokemon(pSym, 1, &aSymbol);
        OS_TPrintf("mons %d",aSymbol.monsno);
        OS_TPrintf("waza %d ",aSymbol.wazano);
        OS_TPrintf("fem %d",aSymbol.sex);
        OS_TPrintf("move %d",aSymbol.move_type);
        OS_TPrintf("form %d\n",aSymbol.form_no);
      }
      {
        SYMBOL_POKEMON aSymbol;
        SymbolSave_GetSymbolPokemon(pSym, 2, &aSymbol);
        OS_TPrintf("mons %d",aSymbol.monsno);
        OS_TPrintf("waza %d ",aSymbol.wazano);
        OS_TPrintf("fem %d",aSymbol.sex);
        OS_TPrintf("move %d",aSymbol.move_type);
        OS_TPrintf("form %d\n",aSymbol.form_no);
      }
    }
      
    
/*
    for(i=0;i<26;i++){
      REGULATION* pData = GFL_ARC_UTIL_Load(ARCID_REGULATION, i, FALSE, HEAPID_PROC );

      OS_TPrintf("read %d \n",i);

      OS_TPrintf("LOHI %d %d\n",Regulation_GetParam(pData, REGULATION_NUM_LO), Regulation_GetParam(pData, REGULATION_NUM_HI));
      GFL_HEAP_FreeMemory(pData);
    }
*/
    /*
    {//ミステリー関連 
      MYSTERY_DATA *pMystery = MYSTERYDATA_GetCardDataOld(p_wk->pSaveData); 
      GIFT_PACK_DATA  *pGift  = MYSTERYDATA_GetCardData( pMystery, 0 );
      pAddr = (u8*)pGift;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","MYSTERY", (u32)pAddr-(u32)topAddr, sizeof(GIFT_PACK_DATA));
      OS_TPrintf( "download %d  pack%d present%d\n", sizeof(DOWNLOAD_GIFT_DATA), sizeof(GIFT_PACK_DATA), sizeof(GIFT_PRESENT) );
    }
*/
    
    {//DreamWorld関連
      DREAMWORLD_SAVEDATA* pDW = DREAMWORLD_SV_GetDreamWorldSaveData(p_wk->pSaveData);
      pAddr = (u8*)DREAMWORLD_SV_GetSleepPokemon(pDW);
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","SLEEP_POKEMON", (u32)pAddr-(u32)topAddr, POKETOOL_GetWorkSize());
      pAddr = (u8*)&pDW->pokemonIn;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","ISIN_SLEEP_POKEMON", (u32)pAddr-(u32)topAddr, sizeof(u8));
      pAddr = (u8*)&pDW->bAccount;  //ここは並びが変更されるとまずい
      pAddr++;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","FURNITURENO", (u32)pAddr-(u32)topAddr, sizeof(u8));
      
    }
    {//MISC
      MISC* pMisc = SaveData_GetMisc(p_wk->pSaveData);
      pAddr = (u8*)&pMisc->gold;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","PLAYER_GOLD", (u32)pAddr-(u32)topAddr, sizeof(pMisc->gold));
      pAddr = (u8*)&pMisc->badge;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","PLAYER_BADGE_BIT", (u32)pAddr-(u32)topAddr, sizeof(pMisc->badge));
      pAddr = (u8*)&pMisc->suretigai_count;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","PLAYER_CROSS_COUNT", (u32)pAddr-(u32)topAddr, sizeof(pMisc->suretigai_count));
      pAddr = (u8*)&pMisc->thanks_recv_count;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","PLAYER_THANKS_COUNT", (u32)pAddr-(u32)topAddr, sizeof(pMisc->thanks_recv_count));
    }


    {//Myステータス
      MYSTATUS* pMy = SaveData_GetMyStatus(p_wk->pSaveData);
      pAddr = (u8*)&pMy->name;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","PLAYER_NAME", (u32)pAddr-(u32)topAddr, sizeof(pMy->name));
      pAddr = (u8*)&pMy->id;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n", "PLAYER_ID",(u32)pAddr-(u32)topAddr,sizeof(pMy->id));
      pAddr = (u8*)&pMy->sex;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n", "PLAYER_SEX",((u32)pAddr-(u32)topAddr), sizeof(pMy->sex));
      pAddr = (u8*)&pMy->nation;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","MY_NATION", (u32)pAddr-(u32)topAddr, sizeof(pMy->nation));
      pAddr = (u8*)&pMy->area;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","MY_AREA", (u32)pAddr-(u32)topAddr, sizeof(pMy->area));
    }
    {//PLAYTIME
      PLAYTIME * playtime = SaveData_GetPlayTime(p_wk->pSaveData);

      pAddr = (u8*)&playtime->hour;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","PLAYTIME_HOUR", (u32)pAddr-(u32)topAddr, sizeof(playtime->hour));
      pAddr = (u8*)&playtime->minute;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","PLAYTIME_MINUTE", (u32)pAddr-(u32)topAddr, sizeof(playtime->minute));
      pAddr = (u8*)&playtime->second;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","PLAYTIME_SECOND", (u32)pAddr-(u32)topAddr, sizeof(playtime->second));
    }


    {//WIFIリスト
      WIFI_LIST* pWifiList = SaveData_GetWifiListData(p_wk->pSaveData);
      DWCUserData *userdata = WifiList_GetMyUserInfo(pWifiList);
      pAddr = (u8*)&userdata->gs_profile_id;
  //    OS_TPrintf("%d\n",userdata->gs_profile_id);
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","PROFILE_ID", (u32)pAddr-(u32)topAddr, sizeof(userdata->gs_profile_id));

    }

    
    { //GTSネゴ履歴
      WIFI_NEGOTIATION_SAVEDATA* pGTSNEGO = WIFI_NEGOTIATION_SV_GetSaveData(p_wk->pSaveData);

      for(i=0;i<WIFI_NEGOTIATION_DATAMAX;i++){
        MYSTATUS* pMys = WIFI_NEGOTIATION_SV_GetMyStatus_DEBUG(pGTSNEGO, i);
        pAddr = (u8*)&pMys->profileID;
        OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n", "GTSNEGO_GSID",(u32)pAddr-(u32)topAddr,sizeof(pMys->profileID));
      }
    }

#if 1
    { //ジオネット
      WIFI_HISTORY* pHis = SaveData_GetWifiHistory(p_wk->pSaveData);
#if 0
      for(i=0;i<UNITEDNATIONS_PEOPLE_MAX;i++){
        MYSTATUS* pMys = &pHis->aUnitedPeople[i].aMyStatus;
        pAddr = (u8*)&pMys->profileID;
        OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n", "GTS_GSID",(u32)pAddr-(u32)topAddr,sizeof(pMys->profileID));
    //    OS_TPrintf(" id %d \n",pMys->profileID);
      }
#endif 
      pAddr = (u8*)&pHis->myCountryCount;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","GTS_COUNTRYCOUNT", (u32)pAddr-(u32)topAddr, sizeof(pHis->myCountryCount));
    }
#endif

    {  //FRIENDCODE

      

      

    }

    
    {//レコード
      short* rec = (short*)SaveData_GetRecord(p_wk->pSaveData);
      int num = RECORD_GetWorkSize();

      GF_ASSERT(!(num%2));  //レコードデータは４バイトアライメントでないと

      num = num /2;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","RECORDTOP_ADDR", (u32)&rec[0]-(u32)topAddr, 4);
      OS_TPrintf("\"%s\",\"\",\"%d\"\n","RECORDSIZE", (u32)RECORD_GetWorkSize() );

      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","RECORD_CRC", (u32)&rec[num-2]-(u32)topAddr , 2);
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","RECORD_CODED_NUM", (u32)&rec[num-1]-(u32)topAddr, 2);

      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","RECID_CAPTURE_POKE", (u32)&rec[RECID_CAPTURE_POKE]-(u32)topAddr, 4);
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","RECID_FISHING_SUCCESS", (u32)&rec[RECID_FISHING_SUCCESS]-(u32)topAddr, 4);

    }
    
    {//シンボルエンカウント
//      SYSTEMDATA* pSys = SaveData_GetSystemData(p_wk->pSaveData);
      SYMBOL_SAVE_WORK* pSym = SymbolSave_GetSymbolData(p_wk->pSaveData);

      pAddr = (u8*)pSym;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n", "SYMBOL_SAVE_WORK",(u32)pAddr-(u32)topAddr,sizeof(SYMBOL_SAVE_WORK));

    }

    //フレンドProfileID
    {
      WIFI_LIST* pWifi = SaveData_GetWifiListData(p_wk->pSaveData);

      for(i=0;i<32;i++){
        pAddr = (u8*)&pWifi->friendData[i].GameSyncID;

  //      OS_TPrintf("%d \n",pWifi->friendData[i].GameSyncID);
        OS_TPrintf("\"%s%2d\",\"0x%x\",\"%d\"\n", "FRIEND_GAMESYNCID",
                   i,(u32)pAddr-(u32)topAddr,sizeof(pWifi->friendData[i].GameSyncID));
      }
    }
    

    
    {//システム
//      SYSTEMDATA* pSys = SaveData_GetSystemData(p_wk->pSaveData);

//      pAddr = (u8*)&pSys->profileId;
//      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","PROFILE_ID", (u32)pAddr-(u32)topAddr, sizeof(pSys->profileId));
//      MYSTATUS* pMy = SaveData_GetMyStatus(p_wk->pSaveData);

//      pAddr = (u8*)&pMy->profileID;
//      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n", "PROFILE_ID",(u32)pAddr-(u32)topAddr,sizeof(pMy->profileID));

    }

    {//アイテム
      MYITEM* pMyItem = (MYITEM*)SaveData_GetMyItem(p_wk->pSaveData);

      pAddr = (u8*)pMyItem;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","MYBAG_START", (u32)pAddr-(u32)topAddr, sizeof(ITEM_ST));

      OS_TPrintf("\"%s\",\"0\",\"%d\"\n","ITEMST_TOTAL_NUM", (u32)BAG_TOTAL_NUM );
      OS_TPrintf("\"%s\",\"0\",\"%d\"\n","ITEM_MAX", (u32)ITEM_DATA_MAX);

      pAddr = (u8*)&pMyItem->MyNutsItem[0];
      
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","NUTSBAG_START", (u32)pAddr-(u32)topAddr, sizeof(ITEM_ST));

      OS_TPrintf("\"%s\",\"0\",\"%d\"\n","BAG_NUTS_ITEM_MAX ", (u32)BAG_NUTS_ITEM_MAX );
      OS_TPrintf("\"%s\",\"0\",\"%d\"\n","ITEM_NUTS_MAX", (u32)ITEM_NUTS_MAX);
      
    }

    {//図鑑
      ZUKAN_SAVEDATA *pZukan = ZUKAN_SAVEDATA_GetZukanSave( p_wk->pSaveData );

      pAddr = (u8*)&pZukan->get_flag[0];
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","POKEMONGET_BIT_ARRAY", (u32)pAddr-(u32)topAddr, sizeof(u32)*POKEZUKAN_ARRAY_LEN);

      pAddr = (u8*)&pZukan->sex_flag[0][0];
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","POKEMONLOOK_MALE_BIT_ARRAY", (u32)pAddr-(u32)topAddr, sizeof(u32)*POKEZUKAN_ARRAY_LEN);
      pAddr = (u8*)&pZukan->sex_flag[1][0];
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","POKEMONLOOK_FEMALE_BIT_ARRAY", (u32)pAddr-(u32)topAddr, sizeof(u32)*POKEZUKAN_ARRAY_LEN);
      pAddr = (u8*)&pZukan->sex_flag[2][0];
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","POKEMONLOOK_RARE_MALE_BIT_ARRAY", (u32)pAddr-(u32)topAddr, sizeof(u32)*POKEZUKAN_ARRAY_LEN);
      pAddr = (u8*)&pZukan->sex_flag[3][0];
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","POKEMONLOOK_RARE_FEMALE_BIT_ARRAY", (u32)pAddr-(u32)topAddr, sizeof(u32)*POKEZUKAN_ARRAY_LEN);

      pAddr = (u8*)&pZukan->draw_sex[0][0];
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","POKEMONZUKAN_MALE_BIT_ARRAY", (u32)pAddr-(u32)topAddr, sizeof(u32)*POKEZUKAN_ARRAY_LEN);
      pAddr = (u8*)&pZukan->draw_sex[1][0];
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","POKEMONZUKAN_FEMALE_BIT_ARRAY", (u32)pAddr-(u32)topAddr, sizeof(u32)*POKEZUKAN_ARRAY_LEN);
      pAddr = (u8*)&pZukan->draw_sex[2][0];
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","POKEMONZUKAN_RARE_MALE_BIT_ARRAY", (u32)pAddr-(u32)topAddr, sizeof(u32)*POKEZUKAN_ARRAY_LEN);
      pAddr = (u8*)&pZukan->draw_sex[3][0];
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","POKEMONZUKAN_RARE_FEMALE_BIT_ARRAY", (u32)pAddr-(u32)topAddr, sizeof(u32)*POKEZUKAN_ARRAY_LEN);
      
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","ZUKANSAVE_ZENKOKU_MONSMAX", (u32)0, ZUKANSAVE_ZENKOKU_MONSMAX);
      

    }

    
    {//手持ちポケモン
      POKEPARTY* Party = SaveData_GetTemotiPokemon(p_wk->pSaveData);
      POKEPARTY* dmyParty = PokeParty_AllocPartyWork(HEAPID_PGL_DEBUG);
      POKEMON_PARAM * dmyPoke = PP_Create( MONSNO_ROTOMU, 1, 0, HEAPID_PGL_DEBUG );
      for(i = 0 ; i < TEMOTI_POKEMAX ; i++){
        PokeParty_Add( dmyParty, dmyPoke );
        pAddr = (u8*)((u32)Party + (u32)PokeParty_GetMemberPointer( dmyParty, i ) - (u32)dmyParty);
        OS_TPrintf("\"%s%d\",\"0x%x\",\"%d\"\n","PARTY_POKEMON",i, (u32)pAddr-(u32)topAddr, POKETOOL_GetWorkSize());
      }
      GFL_HEAP_FreeMemory(dmyPoke);
      GFL_HEAP_FreeMemory(dmyParty);
    }

    {//ボックスポケモン
      BOX_MANAGER* pBox = BOX_DAT_InitManager( HEAPID_PGL_DEBUG , p_wk->pSaveData);
      int j,i;
      for(i=0;i<BOX_MAX_TRAY;i++){
        for(j=0;j<BOX_MAX_POS;j++){
          pAddr = (u8*)BOXDAT_GetPokeDataAddress(pBox, i, j);
          OS_TPrintf("\"%s%d\",\"0x%x\",\"%d\"\n","BOX_POKEMON",i*BOX_MAX_POS+j, (u32)pAddr-(u32)topAddr, POKETOOL_GetPPPWorkSize());
        }
      }
      BOX_DAT_ExitManager( pBox );
    }

    {//メダルデータ：swan新規
      MEDALBOX * p_mdlbox = SaveData_GetMedalBox( p_wk->pSaveData );
      int i;
      for (i = 0; i < MDLID_MAX; i ++ )
      {
        const MEDAL_ST * pAddr = &p_mdlbox->medal[i];
        OS_Printf("\"%s%03d\",\"0x%x\",\"%d\n","MEDALID",i, (u32)pAddr-(u32)topAddr, sizeof(MEDAL_ST));
      }
      {
        const u8 * pAddr = &p_mdlbox->push_medal;
        OS_Printf("\"%s\",\"0x%x\",\"%d\n","PUSH_MEDAL", (u32)pAddr-(u32)topAddr, sizeof(u8));
      }
    }
  }

  {
    int size,sizetotal=0;
    size = Situation_GetWorkSize();
    OS_TPrintf("%d Situation_GetWorkSize\n",size);
    sizetotal+=size;

    size = MYSTERYDATA_GetWorkSize();
    OS_TPrintf("%d MYSTERYDATA_GetWorkSize\n",size);
    OS_TPrintf("%d MYSTERYDATA  GIFT_PRESENT_POKEMON\n",sizeof(GIFT_PRESENT_POKEMON));
    sizetotal+=size;

    size = RECORD_GetWorkSize();
    OS_TPrintf("%d RECORD_GetWorkSize\n",size);
    sizetotal+=size;

    size = PMSW_GetSaveDataSize();
    OS_TPrintf("%d PMSW_GetSaveDataSize\n",size);
    sizetotal+=size;

    size = WorldTradeData_GetWorkSize();
    OS_TPrintf("%d WorldTradeData_GetWorkSize\n",size);
    sizetotal+=size;

    size = SODATEYA_WORK_GetWorkSize();
    OS_TPrintf("%d SODATEYA_WORK_GetWorkSize\n",size);
    sizetotal+=size;

    size = MISC_GetWorkSize();
    OS_TPrintf("%d MISC_GetWorkSize\n",size);
    sizetotal+=size;

    size = IntrudeSave_GetWorkSize();
    OS_TPrintf("%d IntrudeSave_GetWorkSize\n",size);
    sizetotal+=size;

    size = DREAMWORLD_SV_GetWorkSize();
    OS_TPrintf("%d DREAMWORLD_SV_GetWorkSize\n",size);
    sizetotal+=size;

    OS_TPrintf("%d total\n",sizetotal);
  }

}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

enum {
  LAST_DATA_SIZE = 4 + 4 + 4 + 2 + 2,
  BLOCK_FOOTER_SIZE = 4,
  TOTAL_FOOTER_SIZE = 16,
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static void printInnerSaveAdrsList( BOOL is_table, SAVE_CONTROL_WORK * svctrl, u32 adrs_ofs, u32 id_ofs )
{
  int i;
  u32 size, adrs;
  u32 total_size;
  u32 top_adrs = (u32)SaveControl_GetSaveWorkAdrs( svctrl, &total_size );
  u32 crc_total_size = MATH_ROUNDUP(sizeof(u16) * SaveParam_Normal.table_max, 4 );

  // blocks
  for ( i = 0; i < GMDATA_ID_MAX; i ++ )
  {
    adrs = (u32)SaveControl_DataPtrGet( svctrl, i ) - top_adrs + adrs_ofs;
    size = SaveParam_Normal.table[i].get_size();
    size = MATH_ROUNDUP( size ,4 );
    if ( is_table )
    {
      OS_TPrintf(" +----------------------------------------------- %05x\n", adrs );
      if ( id_ofs == 0 ) {
        OS_TPrintf(" |   内部BLOCK%d\n", i );
      } else {
        OS_TPrintf(" |   内部BLOCK%d ミラー\n", i );
      }
    } else {
      OS_TPrintf("%3d: %05x: %05x(%5d) 0x%05x\n", i + id_ofs, adrs, size, size, adrs + size + 2 );
    }
  }
  //CRC Table
  size = crc_total_size;
  adrs = adrs_ofs + total_size - LAST_DATA_SIZE - crc_total_size;
  if ( is_table )
  {
    OS_TPrintf(" +----------------------------------------------- %05x\n", adrs );
    if ( id_ofs == 0 ) {
      OS_TPrintf(" |   内部 CRC テーブル\n" );
    } else {
      OS_TPrintf(" |   内部 CRC テーブル ミラー\n" );
    }
  } else {
   // OS_TPrintf("%3d: %05x: %05x(%5d) 0x%05x\n", i + id_ofs, adrs, size, size, adrs + size + 2 );
  } 

  // Footer
  i ++;
  size = LAST_DATA_SIZE;
  adrs = adrs_ofs + total_size - LAST_DATA_SIZE;
  if ( is_table )
  {
    OS_TPrintf(" +----------------------------------------------- %05x\n", adrs );
    OS_TPrintf(" |   内部 フッター\n" );
  } else {
    OS_TPrintf("%3d: %05x: %05x(%5d) 0x%05x\n", i + id_ofs, adrs, size, size, adrs + size + 2 );
  }


}

//---------------------------------------------------------------------------------------------------
// ダウンロードしてきたセーブデータを解析を焼けるデータに変換するツールに必要なデータ出力
// (gsync_save_conv.rb用)
//---------------------------------------------------------------------------------------------------
static void printGsyncSaveAdrsList( const SAVE_CONTROL_WORK * svctrl,const GFL_SVLD_PARAM * params )
{
  int i;
  u32 size;
  u32 total_size;
  u32 sv_total_sizes = 0;
  u64 footer_crc_addr;
  u32 block_crc_total_num;
  u32 block_crc_total_size;
  u64 footer_addr;
  u32 mirror_adrs;

  // セーブデータの実サイズを取得
  GFL_SAVEDATA *sv_normal = DEBUG_SaveData_PtrGet();
  GFL_SAVEDATA_GetSaveWorkAdrs( sv_normal ,&sv_total_sizes);
  
  OS_TPrintf("# =============================================\n" );
  
  // sizeof(SAVE_FOOTER)と同じサイズにする必要がある
  OS_TPrintf("# SAVE_FOOTERのサイズ\n" );
  OS_TPrintf("FOTTER_SIZE = %d\n\n",LAST_DATA_SIZE );

  // ブロック数
  OS_TPrintf("# セーブデータのブロック数\n" );
  OS_TPrintf("BLOCK_NUM = %d\n\n",params->table_max );

  ///<ブロックのCRC管理テーブルバッファの合計数
  //block_crc_total_numを偶数個にすることで、4バイトアライメントを維持
  block_crc_total_num = params->table_max;		
  if( params->table_max%2 != 0 ){
    block_crc_total_num += 1;	
  }

  // CRCは1つにつき2バイト
  block_crc_total_size = block_crc_total_num*2;
  OS_TPrintf("# フッターの全CRCテーブルの長さ\n" );
  OS_TPrintf("# 4バイトアライメントを守るために偶数個になっている（CRCは2バイト）\n" );
  OS_TPrintf("BLOCK_NUM_CRC = %d\n\n",block_crc_total_num );

  // セーブブロックのCRC管理ワークのCRC開始アドレス
  // ※ フッターのCRCが格納されているアドレス
  footer_crc_addr = sv_total_sizes - LAST_DATA_SIZE - block_crc_total_size;
  OS_TPrintf("# CRCテーブルの開始アドレス\n" );
  OS_TPrintf("TOTAL_CRC_ADDRESS = 0x%x\n\n",footer_crc_addr );

  // フッターの格納アドレス
  // ※ フッターのCRCが格納されているアドレス
  footer_addr = sv_total_sizes - LAST_DATA_SIZE;
  OS_TPrintf("# セーブ管理フッターの開始アドレス\n" );
  OS_TPrintf("FOTTER_ADDRESS = 0x%x\n\n",footer_addr );
  
  // ミラーセーブのアドレス
  mirror_adrs = (u32)params->savearea_mirror_address;
  OS_TPrintf("# ミラーセーブの開始アドレス\n" );
  OS_TPrintf("MIRROR_ADDRESS = 0x%x\n\n",mirror_adrs );

  // 実サイズ(4バイトアライメント)
  OS_TPrintf("SAVE_REAL_SIZE = [ \n" );
  for (i = 0; i < params->table_max; i ++ )
  {
    size = params->table[i].get_size() + BLOCK_FOOTER_SIZE;
    size = MATH_ROUNDUP( size ,4);
    OS_Printf("%d,\n",size);
  }
  OS_Printf("]\n\n");

  // ブロックごとのサイズ
  OS_TPrintf("SAVE_PAD_SIZE = [ \n" );
  for (i = 0; i < params->table_max; i ++ )
  {
    size = params->table[i].get_size();
    size = MATH_ROUNDUP( size ,4);
    size = MATH_ROUNDUP( size + BLOCK_FOOTER_SIZE, 256 );
    OS_Printf("%d,\n",size);
  }
  OS_Printf("]\n\n");

  // セーブ開始アドレス
  total_size = 0;
  OS_TPrintf("SAVE_TOP_ADDRESS = [ \n" );
  for (i = 0; i < params->table_max; i ++ )
  {
    OS_Printf("%d,\n",total_size);
    size = params->table[i].get_size();
    size = MATH_ROUNDUP( size ,4);
    total_size += MATH_ROUNDUP( size + BLOCK_FOOTER_SIZE, 256 );
  }
  OS_Printf("]\n\n");
   
  OS_TPrintf("# =============================================\n" );
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static void printInnerSaves( BOOL is_table, SAVE_CONTROL_WORK * svctrl )
{
  //OS_TPrintf("INNER SAVE AREA ADRS\n");
  printInnerSaveAdrsList( is_table, svctrl, SaveParam_Normal.savearea_top_address, 0 );
  printInnerSaveAdrsList( is_table, svctrl, SaveParam_Normal.savearea_mirror_address, GMDATA_ID_MAX + 2 );
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static void printOuterSave( BOOL is_table, const GFL_SVLD_PARAM * params, u32 id_ofs, u32 adrs_ofs )
{
  int i;
  u32 size, adrs;
  u32 total_size;
  u32 top_adrs = (u32)params->savearea_top_address;
  u32 crc_total_size = MATH_ROUNDUP(sizeof(u16) * params->table_max, 4 );

  total_size = 0;
  //blocks
  for (i = 0; i < params->table_max; i ++ )
  {
    size = params->table[i].get_size();
    adrs = adrs_ofs + top_adrs + total_size;
    if ( !is_table )
    {
      OS_TPrintf("%3d: %05x: %05x(%5d) 0x%05x\n", i + id_ofs, adrs, size, size, adrs + size + 2 );
    }
    total_size += MATH_ROUNDUP( size + BLOCK_FOOTER_SIZE, 256 );
  }
  if ( is_table )
  {
    OS_TPrintf(" +----------------------------------------------- %05x\n", adrs );
    OS_TPrintf(" |   外部 BLOCK%d\n", id_ofs );
  }
  //CRC Table
  size = crc_total_size;
  adrs = adrs_ofs + top_adrs + total_size;// - LAST_DATA_SIZE - crc_total_size;
  i = 100;
  if ( is_table )
  {
    OS_TPrintf(" +----------------------------------------------- %05x\n", adrs );
    OS_TPrintf(" |   外部 BLOCK%d CRC テーブル\n", id_ofs );
  } else {
    OS_TPrintf("%3d: %05x: %05x(%5d) 0x%05x\n", i + id_ofs, adrs, size, size, 0 );
  }
  // Footer
  i = 101;
  size = LAST_DATA_SIZE;
  adrs = adrs_ofs + top_adrs + total_size + crc_total_size;
  if ( is_table )
  {
    OS_TPrintf(" +----------------------------------------------- %05x\n", adrs );
    OS_TPrintf(" |   外部 BLOCK%d フッター\n", id_ofs );
  } else {
    OS_TPrintf("%3d: %05x: %05x(%5d) 0x%05x\n", i, adrs, size, size, adrs + size - 2 );
  }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static void printOuterSaves( BOOL is_table )
{
  int i;
  OS_TPrintf("OUTER SAVE AREA ADRS\n");
  for ( i = 0; i < SAVE_EXTRA_ID_MAX; i ++ )
  {
    const GFL_SVLD_PARAM * params = &SaveParam_ExtraTbl[i];
    printOuterSave( is_table, params, i, 0 );
    if (params->savearea_top_address != params->savearea_mirror_address )
    {
      printOuterSave( is_table,
          params, 0, params->savearea_mirror_address - params->savearea_top_address );
    }
  }
}

//---------------------------------------------------------------------------
/// ロットチェック資料：「1.3バックアップデータの信頼性について」の項目生成用
/// このままでは使用できないのでExcel上で加工している
//---------------------------------------------------------------------------
static void NextFunc_SaveAdrsCRC( PROC_WORK * p_wk )
{
  printInnerSaves( FALSE, p_wk->pSaveData );
  printOuterSaves( FALSE );
}
//---------------------------------------------------------------------------
/// ロットチェック資料：「1.5メモリーイメージ」の項目生成用
//---------------------------------------------------------------------------
static void NextFunc_SaveAdrsTable( PROC_WORK * p_wk )
{
  printInnerSaves( TRUE, p_wk->pSaveData );
  printOuterSaves( TRUE );
}
//---------------------------------------------------------------------------
/// DLセーブ解析用データ出力
//---------------------------------------------------------------------------
static void NextFunc_DLSaveInfo( PROC_WORK * p_wk )
{
  printGsyncSaveAdrsList( p_wk->pSaveData, &SaveParam_Normal );
}

//---------------------------------------------------------------------------
/**
 * @brief
 */
//---------------------------------------------------------------------------
static void NextFunc_ExSaveAdrs( PROC_WORK * p_wk )
{
  OS_TPrintf("SAVE_NORMAL NUM %d\n", SaveParam_Normal.table_max );
  OS_TPrintf("SAVE_NORMAL               %08x(%08x)\n",  0, SAVESIZE_NORMAL );
  OS_TPrintf("SAVE_MIRROR               %08x(%08x)\n",  MIRROR_SAVE_ADDRESS, SAVESIZE_NORMAL );
  OS_TPrintf("EXTRA_SAVE_START_ADDRESS  %08x(%08x)\n",  EXTRA_SAVE_START_ADDRESS, EXTRA_MM_SAVE_SIZE );
  OS_PutString("\n");

  OS_TPrintf("EXTRA_MM_REC_MINE         %08x(%08x)\n", EXTRA_MM_REC_MINE, SAVESIZE_EXTRA_BATTLE_REC);
  OS_TPrintf("EXTRA_MM_CGEAR            %08x(%08x)\n", EXTRA_MM_CGEAR, SAVESIZE_EXTRA_CGEAR_PICTURE);
  OS_TPrintf("EXTRA_MM_BATTLE_EXAMINAT  %08x(%08x)\n", EXTRA_MM_BATTLE_EXAMINATION,
      SAVESIZE_EXTRA_BATTLE_EXAMINATION);
  OS_TPrintf("EXTRA_MM_STREAMING        %08x(%08x)\n", EXTRA_MM_STREAMING, SAVESIZE_EXTRA_STREAMING);
  OS_TPrintf("EXTRA_MM_DENDOU           %08x(%08x)\n", EXTRA_MM_DENDOU, SAVESIZE_EXTRA_DENDOU );
  OS_TPrintf("EXTRA_MM_DENDOU_MIRROR    %08x(%08x)\n", EXTRA_MM_DENDOU_MIRROR, SAVESIZE_EXTRA_DENDOU );
  OS_TPrintf("EXTRA_MM_POKEWOOD_00      %08x(%08x)\n", EXTRA_MM_POKEWOOD_00, SAVESIZE_EXTRA_POKEWOOD);
  OS_TPrintf("EXTRA_MM_WBT_00           %08x(%08x)\n", EXTRA_MM_WBT_00, SAVESIZE_EXTRA_WBT );
  OS_TPrintf("EXTRA_MM_WBT_01           %08x(%08x)\n", EXTRA_MM_WBT_01, SAVESIZE_EXTRA_WBT );
  OS_TPrintf("EXTRA_MM_WBT_02           %08x(%08x)\n", EXTRA_MM_WBT_02, SAVESIZE_EXTRA_WBT );
  OS_TPrintf("EXTRA_MM_3DS              %08x(%08x)\n", EXTRA_MM_3DS, SAVESIZE_EXTRA_3DS );
  OS_TPrintf("EXTRA_MM_SAVE_END         %08x(%08x)\n", EXTRA_MM_SAVE_END, EXTRA_MM_SAVE_SIZE );
  OS_PutString("\n");

  OS_TPrintf("OUTSIDE_MM_MYSTERY        %08x(%08x)\n", OUTSIDE_MM_MYSTERY, SAVESIZE_OUTSIDE_MYSTERY );
  OS_TPrintf("OUTSIDE_MM_MYSTERY_MIRROR %08x(%08x)\n", OUTSIDE_MM_MYSTERY_MIRROR, SAVESIZE_OUTSIDE_MYSTERY );
  OS_TPrintf("OUTSIDE_MM_PERMANENT      %08x(%08x)\n", OUTSIDE_MM_PERMANENT, SAVESIZE_OUTSIDE_PERMANENT);
  OS_PutString("\n");

  OS_TPrintf("DEBUG_FIGHT_SAVE          %08x(%08x)\n", DEBUG_FIGHT_SAVE, 0 );
  
  //printInnerSaves( p_wk->pSaveData );
  //printOuterSaves();
}

#endif // PM_DEBUG

