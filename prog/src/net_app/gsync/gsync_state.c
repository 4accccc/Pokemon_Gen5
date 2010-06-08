//=============================================================================
/**
 * @file    gsync_state.c
 * @brief   ゲームシンク
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date    09/04/30
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"
#include "arc_def.h"
//#include "libdpw/dwci_ghttp.h"
#include "net_app/gsync.h"
#include <nitroWiFi/nhttp.h>
#include "net/nitrowifidummy.h"

#include "system/main.h"  //HEAPID
#include "message.naix"
#include "print/printsys.h"
#include "print/wordset.h"
#include "print/global_font.h"
#include "font/font.naix"
#include "print/str_tool.h"
#include "sound/pm_sndsys.h"
#include "system/wipe.h"
#include "net/network_define.h"

#include "savedata/save_tbl.h"
//#include "savedata/wifilist.h"
#include "savedata/system_data.h"
#include "savedata/gametime.h"
#include "savedata/dreamworld_data.h"
#include "savedata/symbol_save.h"
#include "savedata/c_gear_data.h"
#include "savedata/zukan_wp_savedata.h"
#include "savedata/musical_dist_save.h"

#include "gsync_local.h"
#include "gsync_download.h"
#include "net/nhttp_rap.h"
#include "../../field/event_gsync.h"
#include "gsync_obj_NANR_LBLDEFS.h"
#include "gsync_poke.cdat"
#include "msg/msg_gsync.h"
#include "gsync.naix"
#include "savedata/c_gear_picture.h"
#include "field/tpoke_data.h"

#include "net/dwc_error.h"

/*
■BGM■

SEQ_BGM_GAME_SYNC	ゲームシンクBGM

■SE■

SEQ_SE_SYS_24			アップ・ダウンロード
SEQ_SE_SYS_25			アップロード完了音
SEQ_SE_SYS_26			ポケモン眠る
*/
#define _ZZZCOUNT (110)

#define _LV1MESSAGE (6)

#define _UPLOAD_PROCESS_PERCENT  (50)
#define _UPSAVE_PROCESS_PERCENT  (100)

#define _MOVETYPE_MAX (8)  //移動タイプの定義

#define _DOWNLOAD_PROCESS_PERCENT (30)
#define _DOWNLOAD_EXSAVE_PERCENT (60)
#define _DOWNLOAD_SAVE_PERCENT (100)

#define _ERRNO_DOWNLOAD_CRASH (9)

typedef void (StateFunc)(G_SYNC_WORK* pState);

#define _DOWNLOAD_ERROR  (GSYNC_ERR010)

//
// PHP とのインターフェース構造体
//
typedef struct tag_EVENT_DATA
{
  unsigned long rom_code;
  unsigned short country_code;
  unsigned char id;
  unsigned char send_flag;
  unsigned long dec_code;
  unsigned short cat_id;
  unsigned short present;
  unsigned char status;
  unsigned char idname1[26];
  unsigned char idname2[26];
  unsigned char idname3[26];
  unsigned char idname4[26];
  unsigned char idname5[26];
}
  EVENT_DATA;



static void _changeState(G_SYNC_WORK* pWork,StateFunc* state);
static void _changeStateDebug(G_SYNC_WORK* pWork,StateFunc* state, int line);

#if DEBUG_ONLY_FOR_ohno
#define _NET_DEBUG (1)
#else
#define _NET_DEBUG (0)
#endif
#ifdef _NET_DEBUG
#define   _CHANGE_STATE(state)  _changeStateDebug(pWork ,state, __LINE__)
#else  //_NET_DEBUG
#define   _CHANGE_STATE(state)  _changeState(pWork ,state)
#endif //_NET_DEBUG


typedef enum{
  _DOWNLOAD_CGEAR,
  _DOWNLOAD_MUSICAL,
  _DOWNLOAD_ZUKAN,
} _DOWNLOAD_CONT_TYPE;



struct _G_SYNC_WORK {
  HEAPID heapID;
  DREAM_WORLD_SERVER_DOWNLOAD_FINISH_DATA aDownFinish;
//  DREAM_WORLD_SERVER_STATUS_DATA aDreamStatus;
//  BOOL baDreamStatusON;
  GSYNC_DOWNLOAD_WORK* pDownload;
  POKEMON_PARAM* pp;
  NHTTP_RAP_WORK* pNHTTPRap;
  GSYNC_DISP_WORK* pDispWork;  // 描画系
  GSYNC_MESSAGE_WORK* pMessageWork; //メッセージ系
  APP_TASKMENU_WORK* pAppTask;
  EVENT_GSYNC_WORK* pParent;
  BOX_MANAGER * pBox;
  SAVE_CONTROL_WORK* pSaveData;
  GAMEDATA* pGameData;
  MUSICAL_DIST_SAVE* pMusical;
  DREAMWORLD_SAVEDATA* pBackupDream;
  u8* pZknWork;
  u8* pCGearWork;
  void* pTopAddr;
  int trayno;
  int indexno;
  StateFunc* state;      ///< ハンドルのプログラム状態
  vu32 count;
  int req;
  int getdataCount;
  int countTimer;
  int zzzCount;
  int notNetEffect;  ///< 通信して無い場合のエフェクト
  int lvup;
  int percent;
  int lv1timer;
  int time;
  int uploadCount;
  DREAM_WORLD_SERVER_ERROR_TYPE ErrorNo;   ///エラーがあった場合の番号
  char tempbuffer[30];
  u8 musicalNo;      ///< webで選択した番号  無い場合 0
  u8 cgearNo;        ///< webで選択した番号  無い場合 0
  u8 zukanNo;        ///< webで選択した番号  無い場合 0
  u8 downloadType;
  u8 msgBit;
  u8 bGet;
  u8 bBox2SleepSaveData; //寝かせた
  u8 errEnd;     // エラーの時に電源切断かどうか
  u8 bAccount;   //アカウント取得済みかどうか
  u8 bSaveDataAsync;
};


static void _ghttpKeyWait(G_SYNC_WORK* pWork);
static void _ghttpPokemonListDownload(G_SYNC_WORK* pWork);
static void _networkClose(G_SYNC_WORK* pWork);
static void _downloadcheck(G_SYNC_WORK* pWork);
static void _downloadcgearend(G_SYNC_WORK* pWork);


//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _IsNetworkMode(G_SYNC_WORK* pWork)
{
  if(GSYNC_CALLTYPE_BOXSET_NOACC == pWork->pParent->selectType){
    return FALSE;
  }
  if(pWork->pParent->baDreamStatusON){
//    if(pWork->aDreamStatus.PlayStatus == DREAM_WORLD_SERVER_PLAY_EMPTY){ //アカウント未修得
//      return FALSE;
//    }
    if(pWork->pParent->aDreamStatus.PlayStatus == DREAM_WORLD_SERVER_PLAY_FINISH){ //サービス終了
      return FALSE;
    }
  }
  if(GFL_NET_IsInit()){
    return TRUE;
  }
  return FALSE;
}



//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _IsLv1Message(G_SYNC_WORK* pWork)
{
  if(pWork->pParent->baDreamStatusON){
    if(pWork->pParent->aDreamStatus.PlayStatus == DREAM_WORLD_SERVER_PLAY_EMPTY){ //アカウント未修得
      return TRUE;
    }
  }
  return FALSE;
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _IsLv1Mode(G_SYNC_WORK* pWork)
{
  if(pWork->pParent->baDreamStatusON){
    if(pWork->pParent->aDreamStatus.PlayStatus == DREAM_WORLD_SERVER_PLAY_EMPTY){ //アカウント未修得
      return TRUE;
    }
    if(pWork->pParent->aDreamStatus.PlayStatus == DREAM_WORLD_SERVER_PLAY_FINISH){ //サービス終了
      return TRUE;
    }
  }
  return FALSE;
}



//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _changeState(G_SYNC_WORK* pWork,StateFunc state)
{
  pWork->state = state;
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------
#ifdef _NET_DEBUG
static void _changeStateDebug(G_SYNC_WORK* pWork,StateFunc state, int line)
{
#ifdef DEBUG_ONLY_FOR_ohno
  OS_TPrintf("gsync: %d\n",line);
#endif
  _changeState(pWork, state);
}
#endif

//------------------------------------------------------------------------------
/**
 * @brief   フェードアウト処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeFadeout(G_SYNC_WORK* pWork)
{
	if(WIPE_SYS_EndCheck()){
		_CHANGE_STATE( NULL);        // 終わり
	}
}

//------------------------------------------------------------------------------
/**
 * @brief   フェードアウト処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeFadeStart(G_SYNC_WORK* pWork)
{
  if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }

 
  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
  _CHANGE_STATE(_modeFadeout);        // 終わり
}

//------------------------------------------------------------------------------
/**
 * @brief   エラー表示処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ErrorDisp3(G_SYNC_WORK* pWork)
{
  if(!GSYNC_DOWNLOAD_ResultEnd(pWork->pDownload)){
    return;
  }
  GSYNC_DOWNLOAD_Exit(pWork->pDownload);
  pWork->pDownload=NULL;
  _CHANGE_STATE(_networkClose);
}

static void _ErrorDisp2(G_SYNC_WORK* pWork)
{
  if(GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg()){
    if(pWork->pDownload){
      if(GSYNC_DOWNLOAD_ExitAsync(pWork->pDownload)){
        _CHANGE_STATE(_ErrorDisp3);
      }
      _CHANGE_STATE(_networkClose);
    }
    else{
      _CHANGE_STATE(_networkClose);
    }
  }
}


static void _ErrorDisp(G_SYNC_WORK* pWork)
{
  int gmm = GSYNC_ERR001 + pWork->ErrorNo - 1;

  if(_DOWNLOAD_ERROR == pWork->ErrorNo){
    gmm = GSYNC_ERR010;
  }
  else if((pWork->ErrorNo <= 0) || (pWork->ErrorNo >= DREAM_WORLD_SERVER_ERROR_MAX)){
    gmm = GSYNC_ERR009;
  }
  GSYNC_MESSAGE_InfoMessageEnd(pWork->pMessageWork);
  
  GSYNC_MESSAGE_SetNormalMessage(pWork->pMessageWork,gmm);
  GSYNC_MESSAGE_SystemMessageDisp(pWork->pMessageWork,1,16);
  _CHANGE_STATE(_ErrorDisp2);
}

//通信切断

static void _networkClose1(G_SYNC_WORK* pWork)
{
  if(!GFL_NET_IsInit()){
    _CHANGE_STATE(_modeFadeStart);
  }
}

static void _networkClose(G_SYNC_WORK* pWork)
{
  pWork->pParent->selectType = GAMESYNC_RETURNMODE_EXIT;
//  if(!GFL_NET_IsInit()){
    _CHANGE_STATE(_modeFadeStart);
//  }
//  else{
//    GFL_NET_Exit(NULL);
//    _CHANGE_STATE(_networkClose1);
//  }
}


static void _wakeupActio10(G_SYNC_WORK* pWork)
{
  if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  if(GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg()){
//  if(GFL_UI_KEY_GetTrg()){
    _CHANGE_STATE(_networkClose);
  }
}


//ダウンロードメッセージ表示
static void _wakeupActio92(G_SYNC_WORK* pWork)
{
  int msgbuff[]={
    0,
    GSYNC_017,   //CG
    GSYNC_018,     //MU
    GSYNC_020,     //CGMU
    GSYNC_019,     //ZK
    GSYNC_022,  //zkcg
    GSYNC_021,
    GSYNC_016
    };
  
  if(pWork->msgBit!=0){
    GSYNC_MESSAGE_InfoMessageEnd(pWork->pMessageWork);
    GSYNC_MESSAGE_SetNormalMessage(pWork->pMessageWork, msgbuff[pWork->msgBit]);
    GSYNC_MESSAGE_SystemMessageDisp(pWork->pMessageWork,1,8);
    _CHANGE_STATE(_wakeupActio10);
  }
  else{
    _CHANGE_STATE(_networkClose);
  }
}




static void _wakeupActio9(G_SYNC_WORK* pWork)
{
  
  if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  pWork->time--;
  if(pWork->time>0){
    return;
  }
  if(pWork->time==0){
    PMSND_PopBGM();
    PMSND_PauseBGM(FALSE);
    PMSND_FadeInBGM(PMSND_FADE_FAST);
  }
  
  if(GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg()){
    _CHANGE_STATE(_wakeupActio92);
  }
}


static void _wakeupActio8(G_SYNC_WORK* pWork)
{
  if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  if(GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg()){
    if(pWork->lvup==0){
      _CHANGE_STATE(_wakeupActio92);
    }
    else{
      GSYNC_MESSAGE_SystemMessageEnd(pWork->pMessageWork);
      GSYNC_MESSAGE_NickNameMessageDisp(pWork->pMessageWork,GSYNC_015, pWork->lvup, pWork->pp);
      GSYNC_MESSAGE_MessageDisp(pWork->pMessageWork, FALSE);

      PMSND_PauseBGM(TRUE);
      PMSND_PushBGM();
      PMSND_PlayBGM( SEQ_ME_LVUP);
      pWork->time = 60*3;
      _CHANGE_STATE(_wakeupActio9);
    }
  }
}


//-------------------------------------
///	アニメーションコールバック関数
//=====================================
static void _anmcallbackfunc( u32 param, fx32 currentFrame )
{
  G_SYNC_WORK* pWork = (G_SYNC_WORK*)param;
  GSYNC_DISP_ObjChange(pWork->pDispWork,NANR_gsync_obj_bed,NANR_gsync_obj_bed);

  GSYNC_DISP_ObjInit(pWork->pDispWork,NANR_gsync_obj_bed_shadow);

  GSYNC_DISP_BedSyncPokemonStart(pWork->pDispWork);
}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモンを起こしてセーブ領域を変更
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _pokemonArise(G_SYNC_WORK* pWork)
{
  DREAMWORLD_SAVEDATA* pDreamSave = DREAMWORLD_SV_GetDreamWorldSaveData(pWork->pSaveData);

  if(pWork->pp){
    GFL_HEAP_FreeMemory(pWork->pp);
    pWork->pp=NULL;
  }
  if(BOXDAT_GetEmptyTrayNumberAndPos( pWork->pBox, &pWork->trayno, &pWork->indexno )){
    POKEMON_PARAM* pp = GFL_HEAP_AllocClearMemory( pWork->heapID, POKETOOL_GetWorkSize()) ;

    GFL_STD_MemCopy(DREAMWORLD_SV_GetSleepPokemon(pDreamSave), pp, POKETOOL_GetWorkSize());

    if(pWork->lvup!=0){  //レベル更新
      int level = PP_Get( pp, ID_PARA_level, 0);
      if(level==100){
        pWork->lvup=0;
      }
      if((level + pWork->lvup) > 100){
        pWork->lvup = 100 - level;
        level = 100;
      }
      else{
        level += pWork->lvup;
      }
      POKETOOL_MakeLevelRevise( pp, level);
    }
    
    BOXDAT_PutPokemon(pWork->pBox, PP_GetPPPPointerConst(pp));
    
    pWork->pp = pp;
    DREAMWORLD_SV_SetSleepPokemonFlg(pDreamSave,FALSE);
    DREAMWORLD_SV_SetUploadCount(pDreamSave, pWork->uploadCount);
    
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   ゲームシンク完了
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _wakeupAction7(G_SYNC_WORK* pWork)
{
  DREAMWORLD_SAVEDATA* pDream = DREAMWORLD_SV_GetDreamWorldSaveData(pWork->pSaveData);

  pWork->errEnd = FALSE;

  GSYNC_MESSAGE_InfoMessageEnd(pWork->pMessageWork);


  if(pWork->bGet){
    GSYNC_MESSAGE_NickNameMessageDisp(pWork->pMessageWork,GSYNC_005,0, pWork->pp);
    GSYNC_MESSAGE_SystemMessageDisp(pWork->pMessageWork,1,8);
  }
  else{
    GSYNC_MESSAGE_NickNameMessageDisp(pWork->pMessageWork,GSYNC_026,0, pWork->pp);
    GSYNC_MESSAGE_SystemMessageDisp(pWork->pMessageWork,1,4);
  }


  GSYNC_DISP_ObjInit(pWork->pDispWork, NANR_gsync_obj_bed);
  GSYNC_DISP_ObjChange(pWork->pDispWork,NANR_gsync_obj_rug_ani3,NANR_gsync_obj_rug_ani4);

  GSYNC_DISP_ObjChange(pWork->pDispWork,NANR_gsync_obj_bed,NANR_gsync_obj_bed_ani);

  {
    GFL_CLWK_ANM_CALLBACK cbwk;
    cbwk.callback_type = CLWK_ANM_CALLBACK_TYPE_LAST_FRM ;  // CLWK_ANM_CALLBACK_TYPE
    cbwk.param = (u32)pWork;          // コールバックワーク
    cbwk.p_func = _anmcallbackfunc, // コールバック関数
    GSYNC_DISP_SetCallback(pWork->pDispWork,NANR_gsync_obj_bed, &cbwk);
  }
  
  GSYNC_DISP_BlendSmokeStart(pWork->pDispWork,FALSE);
  
  
  GSYNC_DISP_PokemonIconCreate(pWork->pDispWork, PP_GetPPPPointer(pWork->pp),CLSYS_DRAW_MAIN);
  GSYNC_DISP_PokemonIconJump(pWork->pDispWork);

  PMSND_PlaySE(SEQ_SE_SYS_25);

  _CHANGE_STATE(_wakeupActio8);
}


static void _wakeupAction71(G_SYNC_WORK* pWork)
{

  if(GFL_NET_IsInit()){
    if(NHTTP_ERROR_NONE== NHTTP_RAP_Process(pWork->pNHTTPRap)){
      NET_PRINT("終了\n");
      {
        u8* pEvent = (u8*)NHTTP_RAP_GetRecvBuffer(pWork->pNHTTPRap);
        DREAMWORLD_SAVEDATA* pDreamSave = DREAMWORLD_SV_GetDreamWorldSaveData(pWork->pSaveData);
        gs_response* pRep = (gs_response*)pEvent;
        NHTTP_DEBUG_GPF_HEADER_PRINT((gs_response*)pEvent);
        _CHANGE_STATE(_wakeupAction7);
      }
    }
  }
  else{
    _CHANGE_STATE(_wakeupAction7);
  }


}


//------------------------------------------------------------------------------
/**
 * @brief   ポケモン状態検査
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _wakeupAction_save2(G_SYNC_WORK* pWork)
{
  if(pWork->percent < _DOWNLOAD_SAVE_PERCENT){
    pWork->percent++;
  }
  GSYNC_DISP_SetPerfomance(pWork->pDispWork, pWork->percent);

  if(GAMEDATA_SaveAsyncMain(pWork->pGameData) != SAVE_RESULT_OK){
    return;
  }
  pWork->bSaveDataAsync = FALSE;

  if(GFL_NET_IsInit()){
    if(NHTTP_RAP_ConectionCreate(NHTTPRAP_URL_DOWNLOAD_FINISH, pWork->pNHTTPRap)){
      pWork->aDownFinish.bGet=TRUE;
      pWork->aDownFinish.dummy=0;
      NHTTP_AddPostDataRaw(NHTTP_RAP_GetHandle(pWork->pNHTTPRap), &pWork->aDownFinish, sizeof(NHTTPRAP_URL_DOWNLOAD_FINISH) );
      if(NHTTP_RAP_StartConnect(pWork->pNHTTPRap)){
        _CHANGE_STATE(_wakeupAction71);
      }
    }
  }
  else{
  if(GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg()){
//    if(GFL_UI_KEY_GetTrg()){
      _CHANGE_STATE(_wakeupAction7);
    }
  }
}


static void _wakeupAction_1(G_SYNC_WORK* pWork)
{
  pWork->errEnd = TRUE;
  _pokemonArise(pWork);
  pWork->bSaveDataAsync=TRUE;
  GAMEDATA_SaveAsyncStart(pWork->pGameData);
  _CHANGE_STATE(_wakeupAction_save2);
}

//------------------------------------------------------------------

static void _exsaveEnd(G_SYNC_WORK* pWork)
{
  if(GSYNC_DOWNLOAD_ExitAsync(pWork->pDownload)){
    _CHANGE_STATE(_downloadcgearend);
  }
  else{
    pWork->ErrorNo = _DOWNLOAD_ERROR;
    _CHANGE_STATE(_ErrorDisp);
  }
}


static void _cgearsave3(G_SYNC_WORK* pWork)
{
  pWork->cgearNo=DREAM_WORLD_NOPICTURE;
  GFL_HEAP_FreeMemory(pWork->pCGearWork);
  pWork->pCGearWork=NULL;
  _CHANGE_STATE(_exsaveEnd);
}

static void _cgearsave2(G_SYNC_WORK* pWork)
{
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(pWork->pGameData);
  if(SAVE_RESULT_OK!=GAMEDATA_ExtraSaveAsyncMain(pWork->pGameData,SAVE_EXTRA_ID_CGEAR_PICUTRE)){
    return;
  }
  SaveControl_Extra_UnloadWork(pSave, SAVE_EXTRA_ID_CGEAR_PICUTRE);
  CGEAR_SV_SetCGearPictureONOFF(CGEAR_SV_GetCGearSaveData(pSave),TRUE);  //CGEARデカール有効
  _CHANGE_STATE(_cgearsave3);

}


static void _cgearsave(G_SYNC_WORK* pWork)
{
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(pWork->pGameData);
  CGEAR_PICTURE_SAVEDATA* pPic;
  u16* pCRC = GSYNC_DOWNLOAD_GetData(pWork->pDownload);
  int size = CGEAR_PICTURTE_CHAR_SIZE+CGEAR_PICTURTE_PAL_SIZE+CGEAR_PICTURTE_SCR_SIZE;
  int sizeh = size/2;
  u16 crc;

  pWork->pCGearWork = GFL_HEAP_AllocMemory(pWork->heapID,SAVESIZE_EXTRA_CGEAR_PICTURE);
  pPic = (CGEAR_PICTURE_SAVEDATA*)pWork->pCGearWork;
  
  
  //crc検査 pCRC
  crc = GFL_STD_CrcCalc( pCRC, size );
 // OS_TPrintf("crc%x crc%x",crc,pCRC[sizeh]);
  GF_ASSERT(crc == pCRC[sizeh]);
  if(crc != pCRC[sizeh]){
    _CHANGE_STATE(_cgearsave3);
  }
  else{
    OS_TPrintf("CGEARサイズ %x %x",size, SAVESIZE_EXTRA_CGEAR_PICTURE);


    SaveControl_Extra_LoadWork(pSave, SAVE_EXTRA_ID_CGEAR_PICUTRE, pWork->heapID,
                               pWork->pCGearWork,SAVESIZE_EXTRA_CGEAR_PICTURE);
    GFL_STD_MemCopy(pCRC, pPic->picture, size);
    CGEAR_SV_SetCGearPictureCRC(CGEAR_SV_GetCGearSaveData(pSave),crc);  //CGEARデカール有効
    GAMEDATA_ExtraSaveAsyncStart(pWork->pGameData, SAVE_EXTRA_ID_CGEAR_PICUTRE);
    _CHANGE_STATE(_cgearsave2);
  }
}


static void _zukansave3(G_SYNC_WORK* pWork)
{
  pWork->zukanNo=DREAM_WORLD_NOPICTURE;
  GFL_HEAP_FreeMemory(pWork->pZknWork);
  pWork->pZknWork=NULL;
  _CHANGE_STATE(_exsaveEnd);
}

static void _zukansave2(G_SYNC_WORK* pWork)
{
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(pWork->pGameData);
  if(SAVE_RESULT_OK!=GAMEDATA_ExtraSaveAsyncMain(pWork->pGameData,SAVE_EXTRA_ID_ZUKAN_WALLPAPER)){
    return;
  }
  SaveControl_Extra_UnloadWork(pSave, SAVE_EXTRA_ID_ZUKAN_WALLPAPER);
  _CHANGE_STATE(_zukansave3);
}

static void _zukansave(G_SYNC_WORK* pWork)
{
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(pWork->pGameData);
  u16* pCRC = GSYNC_DOWNLOAD_GetData(pWork->pDownload);
  int size = ZUKANWP_SAVEDATA_RESOURCE_SIZE;
  int sizeh = size/2;
  u16 crc;

  pWork->pZknWork = GFL_HEAP_AllocMemory(pWork->heapID, SAVESIZE_EXTRA_ZUKAN_WALLPAPER);

  //crc検査 pCRC
  crc = GFL_STD_CrcCalc( pCRC, size );
 // OS_TPrintf("crc%x crc%x",crc,pCRC[sizeh]);
  GF_ASSERT(crc == pCRC[sizeh]);
  if(crc != pCRC[sizeh]){
    _CHANGE_STATE(_zukansave3);
  }
  else{
    SaveControl_Extra_LoadWork(pSave, SAVE_EXTRA_ID_ZUKAN_WALLPAPER, pWork->heapID,
                               pWork->pZknWork,SAVESIZE_EXTRA_ZUKAN_WALLPAPER);
    GFL_STD_MemCopy(pCRC, pWork->pZknWork, size);
    ZUKANWP_SAVEDATA_SetDataCheckFlag( (ZUKANWP_SAVEDATA *) pWork->pZknWork,TRUE );
    GAMEDATA_ExtraSaveAsyncStart(pWork->pGameData, SAVE_EXTRA_ID_ZUKAN_WALLPAPER);
    _CHANGE_STATE(_zukansave2);
  }
}


static void _musicalsaveMain(G_SYNC_WORK* pWork)
{

  if( MUSICAL_DIST_SAVE_SaveMusicalArchive_Main( pWork->pMusical )){
    pWork->pMusical=NULL;
    _CHANGE_STATE(_exsaveEnd);
  }
} 

static void _musicalsave(G_SYNC_WORK* pWork,int size)
{
  pWork->musicalNo=DREAM_WORLD_NOPICTURE;

  {
    u16* pCRC = GSYNC_DOWNLOAD_GetData(pWork->pDownload);
    int sizeh = size/2;
    u16 crc;
    
    //crc検査 pCRC
    crc = GFL_STD_CrcCalc( pCRC, size );
   // OS_TPrintf("crc%x crc%x",crc,pCRC[sizeh]);
    GF_ASSERT(crc == pCRC[sizeh]);
    if(crc != pCRC[sizeh]){
      _CHANGE_STATE(_exsaveEnd);
    }

    //ミュージカルセーブ
    pWork->pMusical = MUSICAL_DIST_SAVE_SaveMusicalArchive_Init( pWork->pGameData , pCRC, size, pWork->heapID);
    _CHANGE_STATE(_musicalsaveMain);
  }
}


static void _downloadcgearend(G_SYNC_WORK* pWork)
{
  if(!GSYNC_DOWNLOAD_ResultEnd(pWork->pDownload)){
    return;
  }
  if(GSYNC_DOWNLOAD_ResultError(pWork->pDownload)){
    pWork->ErrorNo = _DOWNLOAD_ERROR;
    _CHANGE_STATE(_ErrorDisp);
    return;
  }
  GSYNC_DOWNLOAD_Exit(pWork->pDownload);
  pWork->pDownload=NULL;

  

  if(pWork->percent < 100){
    pWork->percent+=10;
  }
  GSYNC_DISP_SetPerfomance(pWork->pDispWork, pWork->percent);

  _CHANGE_STATE(_downloadcheck);
}


static void _downloadcgear7(G_SYNC_WORK* pWork)
{
  switch(pWork->downloadType){
  case _DOWNLOAD_CGEAR:
    _CHANGE_STATE(_cgearsave);
    break;
  case _DOWNLOAD_MUSICAL:
    _musicalsave(pWork,GSYNC_DOWNLOAD_GetSize(pWork->pDownload)-2);
    break;
  case _DOWNLOAD_ZUKAN:
    _CHANGE_STATE(_zukansave);
    break;
  }
}



static void _downloadcgear6(G_SYNC_WORK* pWork)
{
  if(!GSYNC_DOWNLOAD_ResultEnd(pWork->pDownload)){
    return;
  }
  if(GSYNC_DOWNLOAD_ResultError(pWork->pDownload)){
    pWork->ErrorNo = _DOWNLOAD_ERROR;
    _CHANGE_STATE(_ErrorDisp);
  }
  else{
    _CHANGE_STATE(_downloadcgear7);
  }
}



static void _downloadcgear51(G_SYNC_WORK* pWork)
{
  if(GSYNC_DOWNLOAD_FileAsync(pWork->pDownload)){
    _CHANGE_STATE(_downloadcgear6);
  }
  else{
    pWork->ErrorNo = _DOWNLOAD_ERROR;
    _CHANGE_STATE(_ErrorDisp);
  }
}

static void _downloadcgear5(G_SYNC_WORK* pWork)
{
  if(!GSYNC_DOWNLOAD_ResultEnd(pWork->pDownload)){
    return;
  }
  if(GSYNC_DOWNLOAD_ResultError(pWork->pDownload)){
    pWork->ErrorNo = _DOWNLOAD_ERROR;
    _CHANGE_STATE(_ErrorDisp);
  }
  else if(GSYNC_DOWNLOAD_GetSize(pWork->pDownload)==0){
    pWork->ErrorNo = _DOWNLOAD_ERROR;
    _CHANGE_STATE(_ErrorDisp);
  }
  else{
    _CHANGE_STATE(_downloadcgear51);
  }
}


static void _downloadcgear3(G_SYNC_WORK* pWork)
{
  if(GSYNC_DOWNLOAD_FileListAsync(pWork->pDownload)){
    _CHANGE_STATE(_downloadcgear5);
  }
  else{
    pWork->ErrorNo = _DOWNLOAD_ERROR;
    _CHANGE_STATE(_ErrorDisp);
  }
}

static void _downloadcgear2(G_SYNC_WORK* pWork)
{
  if(!GSYNC_DOWNLOAD_ResultEnd(pWork->pDownload)){
    return;
  }
  if(GSYNC_DOWNLOAD_ResultError(pWork->pDownload)){
    pWork->ErrorNo = _DOWNLOAD_ERROR;
    _CHANGE_STATE(_ErrorDisp);
    return;
  }

  switch(pWork->downloadType){
  case _DOWNLOAD_CGEAR:
    if(GSYNC_DOWNLOAD_SetAttr(pWork->pDownload, "CGEAR", pWork->cgearNo)){
      _CHANGE_STATE(_downloadcgear3);
    }
    else{
      pWork->ErrorNo = _DOWNLOAD_ERROR;
      _CHANGE_STATE(_ErrorDisp);
    }
    break;
  case _DOWNLOAD_MUSICAL:
    if(GSYNC_DOWNLOAD_SetAttr(pWork->pDownload, "MUSICAL", pWork->musicalNo)){
      _CHANGE_STATE(_downloadcgear3);
    }
    else{
      pWork->ErrorNo = _DOWNLOAD_ERROR;
      _CHANGE_STATE(_ErrorDisp);
    }
    break;
  case _DOWNLOAD_ZUKAN:
    if(GSYNC_DOWNLOAD_SetAttr(pWork->pDownload, "ZUKAN", pWork->zukanNo)){
      _CHANGE_STATE(_downloadcgear3);
    }
    else{
      pWork->ErrorNo = _DOWNLOAD_ERROR;
      _CHANGE_STATE(_ErrorDisp);
    }
    break;
  }
}


// ダウンロードCGEAR
static void _downloadcgear(G_SYNC_WORK* pWork)
{
  switch(pWork->downloadType){
  case _DOWNLOAD_CGEAR:
    pWork->pDownload = GSYNC_DOWNLOAD_Create(pWork->heapID, CGEAR_PICTURE_SAVE_GetWorkSize() + 2); //CRC
    break;
  case _DOWNLOAD_MUSICAL:
    pWork->pDownload = GSYNC_DOWNLOAD_Create(pWork->heapID, 128*1024 ); //CRCこみで128K
    break;
  case _DOWNLOAD_ZUKAN:
    pWork->pDownload = GSYNC_DOWNLOAD_Create(pWork->heapID, ZUKANWP_SAVEDATA_GetWorkSize() + 2); //CRC
    break;
  }

  if(GSYNC_DOWNLOAD_InitAsync(pWork->pDownload)){
    _CHANGE_STATE(_downloadcgear2);
  }
  else{
    pWork->ErrorNo = _DOWNLOAD_ERROR;
    _CHANGE_STATE(_ErrorDisp);
  }
}


static void _downloadcheck(G_SYNC_WORK* pWork)
{
  if(pWork->cgearNo != DREAM_WORLD_NOPICTURE){
    pWork->downloadType=_DOWNLOAD_CGEAR;
    _CHANGE_STATE(_downloadcgear);
    return;
  }
  if(pWork->musicalNo != DREAM_WORLD_NOPICTURE){
    pWork->downloadType=_DOWNLOAD_MUSICAL;
    _CHANGE_STATE(_downloadcgear);
    return;
  }
  if(pWork->zukanNo != DREAM_WORLD_NOPICTURE){
    pWork->downloadType=_DOWNLOAD_ZUKAN;
    _CHANGE_STATE(_downloadcgear);
    return;
  }
  _CHANGE_STATE(_wakeupAction_1);  //起きる演出へ
}


//------------------------------------------------------------------

//   家具をセーブエリアに移動
static void _furnitureInSaveArea(DREAMWORLD_SAVEDATA* pDreamSave,DREAM_WORLD_SERVER_DOWNLOAD_DATA* pDream)
{
  int i;
  BOOL bChnage = FALSE;

  //かぐが更新されているのか検査

  for(i=0;i<DREAM_WORLD_DATA_MAX_FURNITURE;i++){
    DREAM_WORLD_FURNITUREDATA* pF = DREAMWORLD_SV_GetFurnitureData(pDreamSave,i);
    if(0!=GFL_STD_MemComp(pF, &pDream->Furniture[i], sizeof(DREAM_WORLD_FURNITUREDATA))){
      bChnage = TRUE;
      break;
    }
  }
  if(bChnage){
    NET_PRINT("かぐ更新された\n");
    for(i=0;i<DREAM_WORLD_DATA_MAX_FURNITURE;i++){
      DREAMWORLD_SV_SetFurnitureData(pDreamSave, i,   &pDream->Furniture[i]);
    }
    DREAMWORLD_SV_SetSelectFurnitureNo(pDreamSave, DREAM_WORLD_NOFURNITURE);
  }
}


//   アイテムをセーブエリアに移動
static BOOL _itemInSaveArea(DREAMWORLD_SAVEDATA* pDreamSave,DREAM_WORLD_SERVER_DOWNLOAD_DATA* pDream)
{
  int i;
  BOOL bGet=FALSE;
  
  for(i=0;i<DREAM_WORLD_DATA_MAX_ITEMBOX;i++){
    if(pDream->itemID[i]!=0 && (pDream->itemID[i]<=ITEM_DATA_MAX)){
      DREAMWORLD_SV_SetItem(pDreamSave, i,   pDream->itemID[i], pDream->itemNum[i]);
      NET_PRINT("DREAMWORLD_SV_SetItem %d %d %d\n",i,pDream->itemID[i], pDream->itemNum[i]);
      bGet=TRUE;
    }
  }
  return bGet;
}


//ポケモンをセーブエリアに移動
static void _symbolPokemonSave(G_SYNC_WORK* pWork,DREAMWORLD_SAVEDATA* pDreamSave,int monsno,int sex, int form, int tec, int act)
{
  u8 move_type = act;

  if((monsno!=0) && (monsno <= MONSNO_ARUSEUSU)){  // データ読み込み・破棄
    TPOKE_DATA* pTP =TPOKE_DATA_Create( pWork->heapID );
    int size = SYMBOL_ZONE_TYPE_FREE_SMALL;
    u8 form_no =  POKETOOL_CheckPokeFormNo(  monsno,  form );

    if(sex > 1){
      sex = GFUser_GetPublicRand( 2 );
    }
#if 1
    OS_TPrintf("モンスター番号%d %d\n",monsno,sex);
    if(TPOKE_DATA_IsSizeBig(pWork->pGameData, pTP, monsno, sex, form_no )){
      size = SYMBOL_ZONE_TYPE_FREE_LARGE;
    }
#endif
    //ポケモンシンボルエンカウント格納
    if(move_type > _MOVETYPE_MAX){
      move_type = 0;
    }
    SymbolSave_SetFreeZone(
      SymbolSave_GetSymbolData(pWork->pSaveData), monsno, tec, sex, form_no, move_type, size);
    TPOKE_DATA_Delete( pTP );
  }
}


static void _itemDispInit(G_SYNC_WORK* pWork,DREAM_WORLD_SERVER_DOWNLOAD_DATA* pDream)
{
  int i;
  for(i=0;i<DREAM_WORLD_DATA_MAX_ITEMBOX;i++){
    if((pDream->itemID[i]!=0) && (pDream->itemNum[i]!=0) && (pDream->itemID[i]<=ITEM_DATA_MAX)){
      GSYNC_DISP_MoveIconItemAdd(pWork->pDispWork,i,pDream->itemID[i] );
    }
  }
}


static void _datacheck(G_SYNC_WORK* pWork, DREAMWORLD_SAVEDATA* pDreamSave,DREAM_WORLD_SERVER_DOWNLOAD_DATA* pDream,gs_response* pRep )
{
  int i;
  
  if(pRep->ret_cd != DREAM_WORLD_SERVER_ERROR_NONE){
    pWork->ErrorNo = pRep->ret_cd;
    _CHANGE_STATE(_ErrorDisp);
    return;
  }
  if(!pDream->bGet && (DREAMWORLD_SV_GetUploadCount(pDreamSave) != pDream->uploadCount))
  {
    pWork->uploadCount = pDream->uploadCount;
    pWork->musicalNo = pDream->musicalNo;      ///< webで選択した番号  無い場合 0
    pWork->cgearNo = pDream->cgearNo; //  u8 cgearNo;        ///< webで選択した番号  無い場合 0
    pWork->zukanNo = pDream->zukanNo; //  u8 zukanNo;        ///< webで選択した番号  無い場合 0
    pWork->lvup = pDream->sleepPokemonLv; //寝てたポケモンのレベルアップ値

    if(pWork->cgearNo != DREAM_WORLD_NOPICTURE){
      pWork->msgBit = pWork->msgBit | 0x01;
    }
    if(pWork->musicalNo != DREAM_WORLD_NOPICTURE){
      pWork->msgBit = pWork->msgBit | 0x02;
    }
    if(pWork->zukanNo != DREAM_WORLD_NOPICTURE){
      pWork->msgBit = pWork->msgBit | 0x04;
    }
    //シンボルポケ格納
    for(i=0;i<DREAM_WORLD_SERVER_DOWNLOADPOKE_MAX;i++){
      if(pDream->poke[i].findPokemon==0){
        break;
      }
      _symbolPokemonSave(pWork, pDreamSave,
                         pDream->poke[i].findPokemon,
                         pDream->poke[i].findPokemonSex,
                         pDream->poke[i].findPokemonForm,
                         pDream->poke[i].findPokemonTecnique,
                         pDream->poke[i].findPokemonAct);
      GSYNC_DISP_MoveIconPokeAdd(pWork->pDispWork, DREAM_WORLD_DATA_MAX_ITEMBOX+i,
                             pDream->poke[i].findPokemon, pDream->poke[i].findPokemonForm,
                             pDream->poke[i].findPokemonSex);
    }
      
    //サインイン
    DREAMWORLD_SV_SetSignin(pDreamSave,pDream->signin);
    // 家具
    _furnitureInSaveArea(pDreamSave, pDream);
    //アイテム
    pWork->bGet = _itemInSaveArea(pDreamSave, pDream);
    //アイテムの表示
    _itemDispInit(pWork, pDream);

    G2_SetBlendAlpha(GX_BLEND_PLANEMASK_NONE,
                     GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1|
                     GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3|GX_BLEND_PLANEMASK_BD,
                     8,8);
    //リソース読み込み
    GSYNC_DISP_DownloadFileRead(pWork->pDispWork);

    //          _CHANGE_STATE();  //セーブに行く?
  }
  else{
    pWork->bGet=FALSE;
    NET_PRINT("取得しなかった%d %d=%d\n",pDream->bGet, pDream->uploadCount,DREAMWORLD_SV_GetUploadCount(pDreamSave));
  }
  
  _downloadcheck(pWork);
}

//------------------------------------------------------------------------------
/**
 * @brief   レベル１検査
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _lv1check(G_SYNC_WORK* pWork)
{
  int i,j;
  DREAMWORLD_SAVEDATA* pDreamSave = DREAMWORLD_SV_GetDreamWorldSaveData(pWork->pSaveData);
  DREAM_WORLD_SERVER_STATUS_DATA *pDreamStatus = &pWork->pParent->aDreamStatus;

  
  //ポケモンシンボルエンカウント
  _symbolPokemonSave(pWork, pDreamSave, pDreamStatus->findPokemon,
                     pDreamStatus->findPokemonSex,
                     pDreamStatus->findPokemonForm,
                     pDreamStatus->findPokemonTecnique, GFUser_GetPublicRand( _MOVETYPE_MAX ));

  for(i=0;i<DREAM_WORLD_DATA_MAX_ITEMBOX;i++){
    for(j=i+1;j<DREAM_WORLD_DATA_MAX_ITEMBOX;j++){
      if((pDreamStatus->itemID[i] != 0) && (pDreamStatus->itemID[j] == pDreamStatus->itemID[i])){
        return FALSE;
      }
    }
  }
  for(i=0;i<DREAM_WORLD_DATA_MAX_ITEMBOX;i++){
    DREAMWORLD_SV_SetItem(pDreamSave, i, pDreamStatus->itemID[i], pDreamStatus->itemNum[i]);
  }
  return TRUE;
}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモン状態検査
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _wakeupAction6(G_SYNC_WORK* pWork)
{
  if(_IsNetworkMode(pWork) && !_IsLv1Mode(pWork)){

    if(pWork->percent < _DOWNLOAD_PROCESS_PERCENT){
      pWork->percent++;
    }
    GSYNC_DISP_SetPerfomance(pWork->pDispWork, pWork->percent);

    if(NHTTP_ERROR_NONE== NHTTP_RAP_Process(pWork->pNHTTPRap)){
      NET_PRINT("終了\n");
      {
        u8* pEvent = (u8*)NHTTP_RAP_GetRecvBuffer(pWork->pNHTTPRap);
        DREAMWORLD_SAVEDATA* pDreamSave = DREAMWORLD_SV_GetDreamWorldSaveData(pWork->pSaveData);
        DREAM_WORLD_SERVER_DOWNLOAD_DATA* pDream = (DREAM_WORLD_SERVER_DOWNLOAD_DATA*)&pEvent[sizeof(gs_response)];
        gs_response* pRep = (gs_response*)pEvent;

        NHTTP_DEBUG_GPF_HEADER_PRINT((gs_response*)pEvent);

        _datacheck(pWork, pDreamSave, pDream, pRep);
      }
    }
  }
  else{
    pWork->lv1timer++;
    if(pWork->percent < _DOWNLOAD_PROCESS_PERCENT){
      pWork->percent++;
    }
    if(pWork->lv1timer == 60*_LV1MESSAGE){
      GSYNC_MESSAGE_SetNormalMessage(pWork->pMessageWork, GSYNC_024);
      GSYNC_MESSAGE_CMMessageDisp(pWork->pMessageWork );
    }
    else if(pWork->lv1timer == 60*_LV1MESSAGE*2){
      GSYNC_MESSAGE_SetNormalMessage(pWork->pMessageWork, GSYNC_025);
      GSYNC_MESSAGE_CMMessageDisp(pWork->pMessageWork );
    }
    else if(pWork->lv1timer == 60*_LV1MESSAGE*3){
      if(_IsLv1Message(pWork)){
        GSYNC_MESSAGE_SystemMessageEnd(pWork->pMessageWork);
      }
      if(_lv1check(pWork)){
        _CHANGE_STATE(_wakeupAction_1);
//        _CHANGE_STATE(_wakeupAction7);
      }
      else{
        pWork->ErrorNo = _ERRNO_DOWNLOAD_CRASH;
        _CHANGE_STATE(_ErrorDisp);
      }
    }
    GSYNC_DISP_SetPerfomance(pWork->pDispWork, pWork->percent);
  }
}

static void _wakeupAction5(G_SYNC_WORK* pWork)
{
  if(_IsNetworkMode(pWork) && !_IsLv1Mode(pWork)){
    if(NHTTP_RAP_ConectionCreate(NHTTPRAP_URL_DOWNLOAD, pWork->pNHTTPRap)){
      if(NHTTP_RAP_StartConnect(pWork->pNHTTPRap)){
        _CHANGE_STATE(_wakeupAction6);
      }
    }
  }
  else{
    if(_IsLv1Message(pWork)){
      pWork->lv1timer = 0;
      GSYNC_MESSAGE_SetNormalMessage(pWork->pMessageWork, GSYNC_023);
      GSYNC_MESSAGE_CMMessageDisp(pWork->pMessageWork );
    }
    else{
      pWork->lv1timer = 60*_LV1MESSAGE*3-10;
    }
    _CHANGE_STATE(_wakeupAction6);
  }
}

static void _wakeupActio4_2(G_SYNC_WORK* pWork)
{
 if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  _CHANGE_STATE(_wakeupAction5);
}


static void _wakeupAction4(G_SYNC_WORK* pWork)
{
  GSYNC_MESSAGE_InfoMessageDisp(pWork->pMessageWork,GSYNC_004);
  GSYNC_MESSAGE_WindowTimeIconStart(pWork->pMessageWork);
  GSYNC_DISP_ObjEnd(pWork->pDispWork, NANR_gsync_obj_zzz_ani);
  GSYNC_DISP_DreamSmokeBgStart(pWork->pDispWork);
  GSYNC_DISP_BlendSmokeStart(pWork->pDispWork,TRUE);
  GSYNC_DISP_ObjChange(pWork->pDispWork,NANR_gsync_obj_rug_ani3,NANR_gsync_obj_rug_ani2);
  GSYNC_DISP_ObjInit(pWork->pDispWork, NANR_gsync_obj_bed);
  GSYNC_DISP_ObjInit(pWork->pDispWork, NANR_gsync_obj_bed_shadow);
  PMSND_PlaySE(SEQ_SE_SYS_24);

  _CHANGE_STATE(_wakeupActio4_2);

}




static void _wakeupActionFailed1(G_SYNC_WORK* pWork)
{
 if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  if(GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg()){
    _CHANGE_STATE(_networkClose);
  }
}





static void _wakeupActionFailed(G_SYNC_WORK* pWork)
{
 if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  GSYNC_MESSAGE_InfoMessageDisp(pWork->pMessageWork,GSYNC_009);
  _CHANGE_STATE(_wakeupActionFailed1);
}



static void _wakeupActionBoxFailed1(G_SYNC_WORK* pWork)
{
 if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  if(GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg()){
    _CHANGE_STATE(_networkClose);
  }
}




static void _wakeupActionBoxFailed(G_SYNC_WORK* pWork)
{
 if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  GSYNC_MESSAGE_InfoMessageDisp(pWork->pMessageWork, GSYNC_030);
  _CHANGE_STATE(_wakeupActionBoxFailed1);
}


//------------------------------------------------------------------------------
/**
 * @brief   ポケモン起こし処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _wakeupAction3(G_SYNC_WORK* pWork)
{

  if(pWork->zzzCount % _ZZZCOUNT==0){
    PMSND_PlaySE(SEQ_SE_SYS_26);
  }
  pWork->zzzCount++;

  
  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);

    if(selectno==0){
      if(FALSE == BOXDAT_GetEmptyTrayNumberAndPos( pWork->pBox, &pWork->trayno, &pWork->indexno )){
        _CHANGE_STATE(_wakeupActionBoxFailed);
      }
      else{
        _CHANGE_STATE(_wakeupAction4);
      }
    }
    else{
      _CHANGE_STATE(_networkClose);
    }
    GSYNC_MESSAGE_InfoMessageEnd(pWork->pMessageWork);
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
    G2S_BlendNone();
    pWork->pAppTask=NULL;
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモン起こし処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _wakeupAction2(G_SYNC_WORK* pWork)
{
  if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  pWork->pAppTask = GSYNC_MESSAGE_YesNoStart(pWork->pMessageWork,GSYNC_YESNOTYPE_INFO);

  _CHANGE_STATE(_wakeupAction3);
}


static void _wakeupAction12(G_SYNC_WORK* pWork)
{
  GSYNC_MESSAGE_InfoMessageEnd(pWork->pMessageWork);

  GSYNC_DISP_ObjInit(pWork->pDispWork, NANR_gsync_obj_bed);
  GSYNC_DISP_ObjInit(pWork->pDispWork, NANR_gsync_obj_bed_shadow);
  GSYNC_DISP_ObjInit(pWork->pDispWork, NANR_gsync_obj_rug_ani3);
  GSYNC_DISP_ObjInit(pWork->pDispWork, NANR_gsync_obj_zzz_ani);

  {
    DREAMWORLD_SAVEDATA* pDreamSave = DREAMWORLD_SV_GetDreamWorldSaveData(pWork->pSaveData);
    if( DREAMWORLD_SV_GetItemRestNum(pDreamSave) ){
      GSYNC_MESSAGE_InfoMessageDisp(pWork->pMessageWork,GSYNC_014);
    }
    else{
      GSYNC_MESSAGE_InfoMessageDisp(pWork->pMessageWork,GSYNC_003);
    }
  }
  
  PMSND_PlaySE(SEQ_SE_SYS_26);

  _CHANGE_STATE(_wakeupAction2);

}



static void _itemcheck2(G_SYNC_WORK* pWork)
{

  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);

    if(selectno==0){
      _CHANGE_STATE(_wakeupAction12);
    }
    else{
      _CHANGE_STATE(_networkClose);
    }
    GSYNC_MESSAGE_InfoMessageEnd(pWork->pMessageWork);
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
    pWork->pAppTask=NULL;
  }
}



static void _itemcheck(G_SYNC_WORK* pWork)
{
  if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }

  pWork->pAppTask = GSYNC_MESSAGE_YesNoStart(pWork->pMessageWork,GSYNC_YESNOTYPE_INFO);

  _CHANGE_STATE(_itemcheck2);

}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモン起こし処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _wakeupAction1(G_SYNC_WORK* pWork)
{
  if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
#if 0
  {
    DREAMWORLD_SAVEDATA* pDreamSave = DREAMWORLD_SV_GetDreamWorldSaveData(pWork->pSaveData);
    if( DREAMWORLD_SV_GetItemRestNum(pDreamSave) ){

      GSYNC_MESSAGE_InfoMessageDisp(pWork->pMessageWork,GSYNC_014);

      _CHANGE_STATE(_itemcheck);

    }
    else{
      _CHANGE_STATE(_wakeupAction12);
    }
  }
#endif  
  _CHANGE_STATE(_wakeupAction12);
}





//------------------------------------------------------------------------------
/**
 * @brief   ポケモン仮アカウント作成 メッセージ
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _accountCreateMessage3(G_SYNC_WORK* pWork)
{
  if(GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg()){
    GSYNC_MESSAGE_SystemMessageEnd(pWork->pMessageWork);
    ///メッセージを見たのでフラグをON
    DREAMWORLD_SV_SetAccount(DREAMWORLD_SV_GetDreamWorldSaveData(pWork->pSaveData),TRUE);
    pWork->bAccount=TRUE;
    _CHANGE_STATE(_ghttpPokemonListDownload);
  }
}

static void _accountCreateMessage2(G_SYNC_WORK* pWork)
{
  if(GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg()){
    GSYNC_MESSAGE_SetNormalMessage(pWork->pMessageWork, GSYNC_029);
    GSYNC_MESSAGE_SystemMessageDisp(pWork->pMessageWork,7, 14);
    _CHANGE_STATE(_accountCreateMessage3);
  }
}

static void _accountCreateMessage(G_SYNC_WORK* pWork)
{
  GSYNC_MESSAGE_SetPassword(pWork->pMessageWork,
                            MyStatus_GetProfileID( GAMEDATA_GetMyStatus(pWork->pParent->gameData) ));
  GSYNC_MESSAGE_MessageDisp(pWork->pMessageWork, TRUE);

  GSYNC_MESSAGE_SetNormalMessage(pWork->pMessageWork, GSYNC_028);
  GSYNC_MESSAGE_SystemMessageDisp(pWork->pMessageWork, 7, 14);
  _CHANGE_STATE(_accountCreateMessage2);
}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモン仮アカウント作成
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _createAccount2(G_SYNC_WORK* pWork)
{
  if(GFL_NET_IsInit()){
    if(NHTTP_ERROR_NONE== NHTTP_RAP_Process(pWork->pNHTTPRap)){
      {
        gs_response* pEvent = (gs_response*)NHTTP_RAP_GetRecvBuffer(pWork->pNHTTPRap);
        NHTTP_DEBUG_GPF_HEADER_PRINT((gs_response*)pEvent);
        if(pEvent->ret_cd==DREAM_WORLD_SERVER_ALREADY_EXISTS){ //アカウントはすでにある
           _CHANGE_STATE(_ghttpPokemonListDownload);
        }
        else if(pEvent->ret_cd==DREAM_WORLD_SERVER_ERROR_NONE){  //アカウント作成完了
          GSYNC_MESSAGE_InfoMessageEnd(pWork->pMessageWork);
          _CHANGE_STATE( _accountCreateMessage);
        }
        else{
          pWork->ErrorNo = pEvent->ret_cd;
          _CHANGE_STATE(_ErrorDisp);
        }
      }
    }
  }
  else{
    _CHANGE_STATE(_ghttpPokemonListDownload);
  }
}



//------------------------------------------------------------------------------
/**
 * @brief   ポケモン仮アカウント作成
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _createAccount(G_SYNC_WORK* pWork)
{

  if(GFL_NET_IsInit()){
    if(NHTTP_RAP_ConectionCreate(NHTTPRAP_URL_ACCOUNT_CREATE, pWork->pNHTTPRap)){
      s32 proid = MyStatus_GetProfileID( GAMEDATA_GetMyStatus(pWork->pGameData) );

      GFL_STD_MemClear(pWork->tempbuffer, sizeof(pWork->tempbuffer));
      STD_TSNPrintf(pWork->tempbuffer, sizeof(pWork->tempbuffer), "%d\0\0\0\0\0\0\0\0\0\0\0\0", proid);

      OS_TPrintf("NHTTP_AddPostDataRaw byte %d %d %s\n",proid,STD_StrLen(pWork->tempbuffer),pWork->tempbuffer);
      NHTTP_AddPostDataRaw( NHTTP_RAP_GetHandle(pWork->pNHTTPRap),
                            pWork->tempbuffer, 12 );

      if(NHTTP_RAP_StartConnect(pWork->pNHTTPRap)){
        _CHANGE_STATE(_createAccount2);
      }
    }
  }
  else{
  if(GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg()){
//    if(GFL_UI_KEY_GetTrg()){
      _CHANGE_STATE(_createAccount2);
    }
  }
}



static void _playingEnd1(G_SYNC_WORK* pWork)
{
  if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  if(GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg()){
    _CHANGE_STATE( _wakeupAction1 );
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   遊んで無いのですが起こす
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _playingEnd(G_SYNC_WORK* pWork)
{

 if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  GSYNC_MESSAGE_InfoMessageDisp(pWork->pMessageWork,GSYNC_010);
  _CHANGE_STATE(_playingEnd1);


}



static void _playingPlaying1(G_SYNC_WORK* pWork)
{
  if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  if(GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg()){
    _CHANGE_STATE(_networkClose);
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   遊んで無いのでおわり
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _playingPlaying(G_SYNC_WORK* pWork)
{

 if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  GSYNC_MESSAGE_InfoMessageDisp(pWork->pMessageWork,GSYNC_013);
  _CHANGE_STATE(_playingPlaying1);


}






//------------------------------------------------------------------------------
/**
 * @brief   ポケモン起こし処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _playStatusCheck(G_SYNC_WORK* pWork, int status , gs_response* pRep)
{
  switch(pRep->ret_cd){
  case DREAM_WORLD_SERVER_ERROR_NONE:    //成功
    switch(pWork->pParent->selectType){
    case GSYNC_CALLTYPE_INFO:   //セーブデータ上では寝かせている
//  DREAM_WORLD_SERVER_PLAY_EMPTY, //ポケモンを預けていない
//  DREAM_WORLD_SERVER_PLAY_NONE,        // まだ遊んでいません
//  DREAM_WORLD_SERVER_PLAY_ACTIVE,     //遊んでいる最中です
//  DREAM_WORLD_SERVER_PLAY_END,	         // ドリームワールドで遊びました
      if(DREAM_WORLD_SERVER_PLAY_EMPTY == status){ //LV1
        _CHANGE_STATE(_wakeupAction1);
      }
      else if(DREAM_WORLD_SERVER_PLAY_END == status){  //OK
        _CHANGE_STATE(_wakeupAction1);
      }
      else if(DREAM_WORLD_SERVER_PLAY_NONE==status){  //NG
        //GSYNC_010
        _CHANGE_STATE(_playingEnd);
      }
      else if(DREAM_WORLD_SERVER_PLAY_ACTIVE == status){  //NG
        //GSYNC_010
        _CHANGE_STATE(_playingPlaying);
      }
      else if(DREAM_WORLD_SERVER_PLAY_FINISH == status){ //LV1だけど サービス終わり
        _CHANGE_STATE(_wakeupAction1);
      }
      else{
        //そうでなかった場合も起こしてよいがもちろん状態としてはおかしい
        NET_PRINT("!!!!!NOTSERVERSTATE %d %d\n",status, pWork->pParent->selectType);
        _CHANGE_STATE(_wakeupAction1);
      }
      break;
    case GSYNC_CALLTYPE_POKELIST:          //セーブデータ上では眠らせるポケモンを選ぶ
      if(pWork->bAccount==FALSE){  //サーバにアカウントがあるけどメッセージみてない
        _CHANGE_STATE(_accountCreateMessage);
      }
      else{
        _CHANGE_STATE(_ghttpPokemonListDownload);
      }
      break;
    }
    break;
  case DREAM_WORLD_SERVER_NO_DATA:   //サーバーにデータが無い場合
    switch(pWork->pParent->selectType){
    case GSYNC_CALLTYPE_POKELIST:          //セーブデータ上では眠らせるポケモンを選ぶ
      _CHANGE_STATE(_ghttpPokemonListDownload);
      break;
    default:
      pWork->ErrorNo = pRep->ret_cd;
      _CHANGE_STATE(_ErrorDisp);
      break;
    }
    break;
  case DREAM_WORLD_SERVER_NO_ACCOUNT:   //アカウント未登録の場合
    switch(pWork->pParent->selectType){
    case GSYNC_CALLTYPE_INFO:
      _CHANGE_STATE(_wakeupAction1);   //起こす部分
      break;
    case GSYNC_CALLTYPE_POKELIST:          //眠らせるポケモンを選ぶ
      //初回ならありうる
      _CHANGE_STATE(_createAccount);  //アカウントを作るに変更
      break;
    }
    break;
  default: //普通のエラー処理
    pWork->ErrorNo = pRep->ret_cd;
    _CHANGE_STATE(_ErrorDisp);
    break;
  }


}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモン状態検査
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ghttpInfoWait1(G_SYNC_WORK* pWork)
{
  if(GFL_NET_IsInit()){
    if(NHTTP_ERROR_NONE== NHTTP_RAP_Process(pWork->pNHTTPRap)){
      NET_PRINT("終了\n");
      {
        u8* pEvent = (u8*)NHTTP_RAP_GetRecvBuffer(pWork->pNHTTPRap);
        gs_response* pRep = (gs_response*)pEvent;
        DREAM_WORLD_SERVER_STATUS_DATA* pDream = (DREAM_WORLD_SERVER_STATUS_DATA*)&pEvent[sizeof(gs_response)];
        GFL_STD_MemCopy(pDream, &pWork->pParent->aDreamStatus, sizeof(DREAM_WORLD_SERVER_STATUS_DATA));
        NHTTP_DEBUG_GPF_HEADER_PRINT((gs_response*)pEvent);
        NET_PRINT("PlayStatus %d\n",pDream->PlayStatus);
        pWork->pParent->baDreamStatusON = TRUE;
        _playStatusCheck(pWork, pDream->PlayStatus, pRep);  //分岐する
      }
    }
  }
  else{
    _CHANGE_STATE(_wakeupAction1);
  }
    
}


//------------------------------------------------------------------------------
/**
 * @brief   ポケモン状態検査
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ghttpInfoWait0(G_SYNC_WORK* pWork)
{

  GSYNC_MESSAGE_InfoMessageDisp(pWork->pMessageWork,GSYNC_006);
  GSYNC_MESSAGE_WindowTimeIconStart(pWork->pMessageWork);
  
  if(GFL_NET_IsInit()){
    if(NHTTP_RAP_ConectionCreate(NHTTPRAP_URL_ACCOUNTINFO, pWork->pNHTTPRap)){
      if(NHTTP_RAP_StartConnect(pWork->pNHTTPRap)){
        _CHANGE_STATE(_ghttpInfoWait1);
      }
    }
  }
  else{
  if(GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg()){
//    if(GFL_UI_KEY_GetTrg()){
      _CHANGE_STATE(_ghttpInfoWait1);
    }
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   ボックスがから
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _BoxNullMsg2(G_SYNC_WORK* pWork)
{
  if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }

  if(GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg()){
//  if(GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL)){
    pWork->pParent->selectType = GSYNC_RETURNMODE_BOXNULL;
    GSYNC_MESSAGE_InfoMessageEnd(pWork->pMessageWork);
    _CHANGE_STATE(_modeFadeStart);
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   ボックスがから
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _BoxNullMsg(G_SYNC_WORK* pWork)
{
  GSYNC_MESSAGE_InfoMessageDisp(pWork->pMessageWork,GSYNC_012);

  _CHANGE_STATE(_BoxNullMsg2);
}

//------------------------------------------------------------------------------
/**    /gs?p=account.playstatus&gsid=123456789012&rom=32&langcode=32&dreamw=32
 * @brief   ポケモン状態検査  アカウントが無いかどうかもここでわかる
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ghttpPokemonListDownload1(G_SYNC_WORK* pWork)
{
  if(GFL_NET_IsInit()){
    if(NHTTP_ERROR_NONE== NHTTP_RAP_Process(pWork->pNHTTPRap)){
      {
        u8* pEvent = (u8*)NHTTP_RAP_GetRecvBuffer(pWork->pNHTTPRap);
        gs_response* pRep = (gs_response*)pEvent;

        NHTTP_DEBUG_GPF_HEADER_PRINT(pRep);

        if((pRep->ret_cd == DREAM_WORLD_SERVER_ERROR_NONE) || (DREAM_WORLD_SERVER_NO_ACCOUNT ==pRep->ret_cd)){  //成功
          GFL_STD_MemCopy(&pEvent[sizeof(gs_response)],pWork->pParent->selectPokeList.pokemonList,
                          DREAM_WORLD_SERVER_POKMEONLIST_MAX/8);
          pWork->pParent->selectType = GAMESYNC_RETURNMODE_BOXJUMP;
          _CHANGE_STATE(_modeFadeStart);
        }
//        else if (DREAM_WORLD_SERVER_NO_ACCOUNT ==pRep->ret_cd){
 //         
   //       GFL_STD_MemCopy(&pEvent[sizeof(gs_response)],pWork->pParent->selectPokeList.pokemonList,
     //                     DREAM_WORLD_SERVER_POKMEONLIST_MAX/8);
    //      pWork->pParent->selectType = GAMESYNC_RETURNMODE_BOXJUMP_NOACC;
    //      _CHANGE_STATE(_modeFadeStart);
//        }
        else{
          pWork->ErrorNo = pRep->ret_cd;
          _CHANGE_STATE(_ErrorDisp);
        }
      }
    }
  }
  else{
      pWork->pParent->selectType = GAMESYNC_RETURNMODE_BOXJUMP;
    _CHANGE_STATE(_modeFadeStart);
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   ポケモンの選択をダウンロード
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ghttpPokemonListDownload(G_SYNC_WORK* pWork)
{
  GMTIME* pGMT = SaveData_GetGameTime(pWork->pSaveData);

#if (defined(DEBUG_ONLY_FOR_ohno) | defined(DEBUG_ONLY_FOR_mizuguchi_mai))
#else
  if(GMTIME_IsPenaltyMode(pGMT) &&  pWork->bAccount ){  //ペナルティー中は眠る事ができない+アカウント取得済み
    _CHANGE_STATE(_wakeupActionFailed);
    return;
  }
  else{          //時間を確認する
    GFDATE gd = DREAMWORLD_SV_GetTime(DREAMWORLD_SV_GetDreamWorldSaveData(pWork->pSaveData));
    RTCDate date, date2;
    GFDATE_GFDate2RTCDate(gd, &date);
    GFL_RTC_GetDate(&date2);
    if(GFL_RTC_GetDaysOffset(&date) >= GFL_RTC_GetDaysOffset(&date2)){  //一日たってないと失敗へ
      _CHANGE_STATE(_wakeupActionFailed);
      return;
    }
  }
#endif
  if(GFL_NET_IsInit()){
    if(NHTTP_RAP_ConectionCreate(NHTTPRAP_URL_POKEMONLIST, pWork->pNHTTPRap)){
      if(NHTTP_RAP_StartConnect(pWork->pNHTTPRap)){
        _CHANGE_STATE(_ghttpPokemonListDownload1);
      }
    }
  }
  else{
  if(GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg()){
//    if(GFL_UI_KEY_GetTrg()){
      _CHANGE_STATE(_ghttpPokemonListDownload1);
    }
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   データアップロード完了
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _upeffectLoop8(G_SYNC_WORK* pWork)
{

  if(pWork->zzzCount%_ZZZCOUNT==0){
    PMSND_PlaySE(SEQ_SE_SYS_26);
  }
  pWork->zzzCount++;

  if(GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg()){
    _CHANGE_STATE(_networkClose);
  }
}



//------------------------------------------------------------------------------
/**
 * @brief   データアップロード完了
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _upeffectLoop7(G_SYNC_WORK* pWork)
{
  if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  _CHANGE_STATE(_upeffectLoop8);

}

//------------------------------------------------------------------------------
/**
 * @brief   眠る完了
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _upeffectLoop6(G_SYNC_WORK* pWork)
{
  if(pWork->pTopAddr){
    GFL_HEAP_FreeMemory(pWork->pTopAddr);
    pWork->pTopAddr = NULL;
  }
  pWork->percent = _UPSAVE_PROCESS_PERCENT;
  GSYNC_DISP_SetPerfomance(pWork->pDispWork, pWork->percent);

  GSYNC_MESSAGE_NickNameMessageDisp( pWork->pMessageWork, GSYNC_007,0, pWork->pp );
  GSYNC_MESSAGE_MessageDisp(pWork->pMessageWork, FALSE);
 // GSYNC_MESSAGE_WindowTimeIconStart(pWork->pMessageWork);
  
  GSYNC_DISP_ObjInit(pWork->pDispWork, NANR_gsync_obj_zzz_ani);
  GSYNC_DISP_BlendSmokeStart(pWork->pDispWork,FALSE);
  GSYNC_DISP_ObjInit(pWork->pDispWork, NANR_gsync_obj_bed);
  GSYNC_DISP_ObjChange(pWork->pDispWork,NANR_gsync_obj_rug_ani1,NANR_gsync_obj_rug_ani3);
  PMSND_PlaySE(SEQ_SE_SYS_25);

  _CHANGE_STATE(_upeffectLoop7);
}



static void _updateSave2(G_SYNC_WORK* pWork)
{
  int result;
  if(pWork->percent < _UPSAVE_PROCESS_PERCENT){
    pWork->percent++;
  }
  GSYNC_DISP_SetPerfomance(pWork->pDispWork, pWork->percent);

  result = GAMEDATA_SaveAsyncMain(pWork->pGameData);
  switch(result){
  case SAVE_RESULT_LAST:
    if(pWork->pBackupDream){
      GFL_HEAP_FreeMemory(pWork->pBackupDream);
      pWork->pBackupDream=NULL;
    }
    pWork->bBox2SleepSaveData=FALSE;
    break;
  case SAVE_RESULT_NG:
    GFL_NET_StateSetWifiError( 
      0, 
      0, 
      0, 
      ERRORCODE_SYSTEM );
    GFL_NET_StateSetError(0);
    break;
  case SAVE_RESULT_OK:
    pWork->bSaveDataAsync=FALSE;
    _CHANGE_STATE(_upeffectLoop6);
    break;
  }
}



static void _updateSave(G_SYNC_WORK* pWork)
{
  DREAMWORLD_SAVEDATA* pDream = DREAMWORLD_SV_GetDreamWorldSaveData(pWork->pSaveData);

  //バックアップドリームワールドデータ作成
  pWork->pBackupDream = DREAMWORLD_SV_AllocWork(pWork->heapID);

  GFL_STD_MemCopy(pDream, pWork->pBackupDream, DREAMWORLD_SV_GetWorkSize());


  //受信した時間
  {
    RTCDate date;
    GFL_RTC_GetDate( &date );
    DREAMWORLD_SV_SetTime(pDream , GFDATE_Set(date.year,date.month,date.day,date.week));
    //家具を送信した
    if(DREAMWORLD_SV_GetSelectFurnitureNo(pDream) != DREAM_WORLD_NOFURNITURE){
      DREAMWORLD_SV_SetIsSyncFurniture(pDream, TRUE);
      DREAMWORLD_SV_SetSelectFurnitureNo(pDream,  DREAM_WORLD_INVALID_FURNITURE );
    }
  }
  pWork->percent=_UPLOAD_PROCESS_PERCENT;

  // レコードの記録
  {
    RECORD* pRec = GAMEDATA_GetRecordPtr(pWork->pGameData);
    RECORD_Inc(pRec, RECID_PDW_SLEEP_POKEMON);
  }
  pWork->bSaveDataAsync=TRUE;
  GAMEDATA_SaveAsyncStart(pWork->pGameData);
  
  _CHANGE_STATE(_updateSave2);
}






//------------------------------------------------------------------------------
/**
 * @brief   データアップロード完了 セーブする
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _upeffectLoop5(G_SYNC_WORK* pWork)
{
  DREAMWORLD_SAVEDATA* pDream = DREAMWORLD_SV_GetDreamWorldSaveData(pWork->pSaveData);
  GSYNC_DISP_PokemonMove(pWork->pDispWork);


  if(_IsLv1Message(pWork)){
    if(pWork->lv1timer == 0){
      GSYNC_MESSAGE_SetNormalMessage(pWork->pMessageWork, GSYNC_023);
      GSYNC_MESSAGE_CMMessageDisp(pWork->pMessageWork );
    }
    else if(pWork->lv1timer == 60*_LV1MESSAGE){
      GSYNC_MESSAGE_SetNormalMessage(pWork->pMessageWork, GSYNC_024);
      GSYNC_MESSAGE_CMMessageDisp(pWork->pMessageWork );
    }
    else if(pWork->lv1timer == 60*_LV1MESSAGE*2){
      GSYNC_MESSAGE_SetNormalMessage(pWork->pMessageWork, GSYNC_025);
      GSYNC_MESSAGE_CMMessageDisp(pWork->pMessageWork );
    }
    pWork->lv1timer++;
  }

  
  if(_IsNetworkMode(pWork) ){
    if(pWork->percent < _UPLOAD_PROCESS_PERCENT){
      pWork->percent++;
    }

    GSYNC_DISP_SetPerfomance(pWork->pDispWork, pWork->percent);

    if(NHTTP_ERROR_NONE== NHTTP_RAP_Process(pWork->pNHTTPRap)){
      NET_PRINT("終了\n");
      {
        u8* pEvent = (u8*)NHTTP_RAP_GetRecvBuffer(pWork->pNHTTPRap);
        gs_response* pRep = (gs_response*)pEvent;
        int d,j;
        u32 size;
        NHTTP_DEBUG_GPF_HEADER_PRINT((gs_response*)pEvent);

        if(_IsLv1Message(pWork)){
          GSYNC_MESSAGE_SystemMessageEnd(pWork->pMessageWork);
        }

        if(pRep->ret_cd == DREAM_WORLD_SERVER_ERROR_NONE){  //成功
          _CHANGE_STATE(_updateSave);
        }
        else{
          pWork->ErrorNo = pRep->ret_cd;
          _CHANGE_STATE(_ErrorDisp);
        }
      }
    }
  }
  else{
    GSYNC_DISP_SetPerfomance(pWork->pDispWork,pWork->notNetEffect);
    pWork->notNetEffect++;
    if(pWork->notNetEffect==100){

      if(_IsLv1Message(pWork)){
        GSYNC_MESSAGE_SystemMessageEnd(pWork->pMessageWork);
      }

      _CHANGE_STATE(_updateSave);
    }
  }
}



//------------------------------------------------------------------------------
/**
 * @brief   データアップロード中
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _upeffectLoop4(G_SYNC_WORK* pWork)
{
  DREAMWORLD_SAVEDATA* pDream = DREAMWORLD_SV_GetDreamWorldSaveData(pWork->pSaveData);

  if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }

  pWork->lv1timer=0;
  if(_IsNetworkMode(pWork)){
    if(NHTTP_RAP_ConectionCreate(NHTTPRAP_URL_UPLOAD, pWork->pNHTTPRap)){
      u32 size;
      u8* topAddr = (u8*)SaveControl_GetSaveWorkAdrs(pWork->pSaveData, &size);
      NHTTP_AddPostDataRaw(NHTTP_RAP_GetHandle(pWork->pNHTTPRap), topAddr, 0x80000 );
      GSYNC_DISP_SetPerfomance(pWork->pDispWork,0);
      pWork->percent=0;
      if(NHTTP_RAP_StartConnect(pWork->pNHTTPRap)){
        _CHANGE_STATE(_upeffectLoop5);
      }
    }
  }
  else{
    GSYNC_DISP_SetPerfomance(pWork->pDispWork,0);
    _CHANGE_STATE(_upeffectLoop5);
  }

}

//------------------------------------------------------------------------------
/**
 * @brief   眠るエフェクト中
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _upeffectLoop3(G_SYNC_WORK* pWork)
{

  pWork->countTimer--;
  if(pWork->countTimer==0){
    GSYNC_DISP_DreamSmokeBgStart(pWork->pDispWork);

    GSYNC_MESSAGE_InfoMessageDisp(pWork->pMessageWork,GSYNC_001);
    GSYNC_MESSAGE_WindowTimeIconStart(pWork->pMessageWork);

    GSYNC_DISP_BlendSmokeStart(pWork->pDispWork,TRUE);
    GSYNC_DISP_ObjChange(pWork->pDispWork,NANR_gsync_obj_rug_ani1,NANR_gsync_obj_rug_ani2);
    GSYNC_DISP_ObjChange(pWork->pDispWork,NANR_gsync_obj_bed,NANR_gsync_obj_bed);
    GSYNC_DISP_ObjInit(pWork->pDispWork,NANR_gsync_obj_bed_shadow);

    PMSND_PlaySE(SEQ_SE_SYS_24);

    _CHANGE_STATE(_upeffectLoop4);
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   眠るエフェクト中
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _upeffectLoop2(G_SYNC_WORK* pWork)
{
  pWork->countTimer--;
  if(pWork->countTimer==0){
    GSYNC_DISP_ObjInit(pWork->pDispWork, NANR_gsync_obj_rug_ani1);
    pWork->countTimer = _DREAMSMOKE_TIME;
    _CHANGE_STATE(_upeffectLoop3);
  }

}


//------------------------------------------------------------------------------
/**
 * @brief   眠るエフェクト中
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _upeffectLoop1(G_SYNC_WORK* pWork)
{
  pWork->countTimer--;
  if(pWork->countTimer==0){
    GSYNC_DISP_ObjChange(pWork->pDispWork,NANR_gsync_obj_bed,NANR_gsync_obj_bed_ani);

    GSYNC_DISP_ObjInit(pWork->pDispWork,NANR_gsync_obj_kemuri_r);
    GSYNC_DISP_ObjInit(pWork->pDispWork,NANR_gsync_obj_kemuri_l);
    
    pWork->countTimer = _HAND_UP_TIME;
    _CHANGE_STATE(_upeffectLoop2);
  }
}





//------------------------------------------------------------------------------
/**
 * @brief   眠るエフェクト中
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _upeffectLoop(G_SYNC_WORK* pWork)
{
  pWork->countTimer--;
  if(pWork->countTimer==0){
    GSYNC_DISP_HandInit(pWork->pDispWork);
    GSYNC_DISP_PokemonIconMove(pWork->pDispWork);
    pWork->countTimer = _BEDIN_TIME;
    _CHANGE_STATE(_upeffectLoop1);
  }
}



//------------------------------------------------------------------------------
/**
 * @brief   眠るエフェクト開始
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _upeffectStart(G_SYNC_WORK* pWork)
{
  GSYNC_DISP_PokemonIconCreate(pWork->pDispWork, PP_GetPPPPointer(pWork->pp),CLSYS_DRAW_SUB);
  pWork->countTimer=10;

  _CHANGE_STATE(_upeffectLoop);
}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモンを眠るエリアに移動
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _BoxPokeMove(G_SYNC_WORK* pWork)
{
  POKEMON_PASO_PARAM* ppp;
  DREAMWORLD_SAVEDATA* pDream = DREAMWORLD_SV_GetDreamWorldSaveData(pWork->pSaveData);


  if(pWork->pp){
    GFL_HEAP_FreeMemory(pWork->pp);
    pWork->pp=NULL;
  }

  ppp = BOXDAT_GetPokeDataAddress( pWork->pBox, pWork->trayno, pWork->indexno );
  GF_ASSERT(ppp);
  if(ppp){
    POKEMON_PARAM* pp = PP_CreateByPPP( ppp, pWork->heapID );
    DREAMWORLD_SV_SetSleepPokemon(pDream, pp);
    pWork->pp = pp;
    BOXDAT_ClearPokemon(pWork->pBox, pWork->trayno, pWork->indexno );
    DREAMWORLD_SV_SetSleepPokemonFlg(pDream,TRUE);
    pWork->bBox2SleepSaveData = TRUE;
  }
  GSYNC_DISP_ObjInit(pWork->pDispWork, NANR_gsync_obj_bed);
  GSYNC_DISP_ObjInit(pWork->pDispWork, NANR_gsync_obj_bed_shadow);
  
  _CHANGE_STATE(_upeffectStart);

}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモンを眠るエリアから元に戻す
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _BoxPokeRemove(G_SYNC_WORK* pWork)
{
  POKEMON_PASO_PARAM* ppp;
  DREAMWORLD_SAVEDATA* pDream = DREAMWORLD_SV_GetDreamWorldSaveData(pWork->pSaveData);

  if(pWork->pBackupDream){
    GFL_STD_MemCopy(pWork->pBackupDream,pDream,DREAMWORLD_SV_GetWorkSize());
    GFL_HEAP_FreeMemory(pWork->pBackupDream);
    pWork->pBackupDream = NULL;
  }
  if(pWork->bBox2SleepSaveData){
    POKEMON_PARAM* pp= DREAMWORLD_SV_GetSleepPokemon(pDream);
    BOXDAT_PutPokemonPos( pWork->pBox, pWork->trayno, pWork->indexno, PP_GetPPPPointerConst(pp) );
    PP_Clear(pp);
    DREAMWORLD_SV_SetSleepPokemonFlg(pDream,FALSE);
    pWork->bBox2SleepSaveData = FALSE;
  }
}

FS_EXTERN_OVERLAY(dpw_common);

//------------------------------------------------------------------------------
/**
 * @brief   PROC初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

static GFL_PROC_RESULT GSYNCProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  EVENT_GSYNC_WORK* pParent = pwk;
  G_SYNC_WORK* pWork;
  s32 profileID;

  GFL_OVERLAY_Load( FS_OVERLAY_ID(dpw_common));
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_GAMESYNC, 0x88000 );
  pWork = GFL_PROC_AllocWork( proc, sizeof( G_SYNC_WORK ), HEAPID_GAMESYNC );
  GFL_STD_MemClear(pWork, sizeof(G_SYNC_WORK));

  pWork->heapID = HEAPID_GAMESYNC;

  GF_ASSERT(pParent);
  if(pParent){
    pWork->pParent = pParent;
    pWork->pGameData = pParent->gameData;
    pWork->pSaveData = GAMEDATA_GetSaveControlWork(pParent->gameData);
    profileID = MyStatus_GetProfileID( GAMEDATA_GetMyStatus(pParent->gameData) );
    pWork->pNHTTPRap = NHTTP_RAP_Init(HEAPID_GAMESYNC, profileID, &pParent->aSVL);
    pWork->pBox = GAMEDATA_GetBoxManager(pParent->gameData);
    pWork->trayno = pParent->boxNo;
    pWork->indexno = pParent->boxIndex;
    pWork->bAccount = DREAMWORLD_SV_GetAccount(DREAMWORLD_SV_GetDreamWorldSaveData(pWork->pSaveData));

    switch(pParent->selectType){
    case GSYNC_CALLTYPE_INFO:      //起こす
      GF_ASSERT(profileID);
      _CHANGE_STATE(_ghttpInfoWait0);
      break;
    case GSYNC_CALLTYPE_BOXSET:   //BOXポケモン選択後
      GF_ASSERT(profileID);
      _CHANGE_STATE(_BoxPokeMove);
      break;
    case GSYNC_CALLTYPE_BOXSET_NOACC:   //BOXポケモン選択後 アカウントなし
      GF_ASSERT(profileID);
      _CHANGE_STATE(_BoxPokeMove);
      break;
    case GSYNC_CALLTYPE_POKELIST:   //アカウントの検査後眠らせるポケモンを選ぶ
      GF_ASSERT(profileID);
      _CHANGE_STATE(_ghttpInfoWait0);
      break;
    case GSYNC_CALLTYPE_BOXNULL:  //ポケモンがいない
      _CHANGE_STATE(_BoxNullMsg);
      break;
    }
  }

  pWork->pDispWork = GSYNC_DISP_Init(pWork->heapID);
  pWork->pMessageWork = GSYNC_MESSAGE_Init(pWork->heapID, NARC_message_gsync_dat);


  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN ,
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );

  //PMSND_PlayBGM(SEQ_BGM_GAME_SYNC);
  GFL_NET_ReloadIconTopOrBottom(FALSE, pWork->heapID);
  
  return GFL_PROC_RES_FINISH;
}


static GFL_PROC_RESULT GSYNCProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  G_SYNC_WORK* pWork = mywk;
  StateFunc* state = pWork->state;
  GFL_PROC_RESULT ret = GFL_PROC_RES_FINISH;

  if( state ){
    state( pWork );
    ret = GFL_PROC_RES_CONTINUE;
  }
  
  if(pWork->pAppTask){
    APP_TASKMENU_UpdateMenu(pWork->pAppTask);
  }
  if(pWork->pDownload){
    GSYNC_DOWNLOAD_Main(pWork->pDownload);
  }

  GSYNC_DISP_Main(pWork->pDispWork);
  GSYNC_MESSAGE_Main(pWork->pMessageWork);

  if(NET_ERR_CHECK_NONE != NetErr_App_CheckError()){
    NHTTP_RAP_ErrorClean(pWork->pNHTTPRap);

    if(pWork->bSaveDataAsync){
      GAMEDATA_SaveAsyncCancel( pWork->pGameData );
      pWork->bSaveDataAsync = FALSE;
    }
    _BoxPokeRemove(pWork);
    if(pWork->errEnd){
      NetErr_DispCall( TRUE );
    }
    WIPE_SetBrightness(WIPE_DISP_MAIN,WIPE_FADE_BLACK);
    WIPE_SetBrightness(WIPE_DISP_SUB,WIPE_FADE_BLACK);
    GFL_NET_DWC_ERROR_ReqErrorDisp(TRUE, FALSE);
    pWork->pParent->selectType = GAMESYNC_RETURNMODE_ERROR;
    ret = GFL_PROC_RES_FINISH;
  }
  return ret;
}

static GFL_PROC_RESULT GSYNCProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  G_SYNC_WORK* pWork = mywk;


  if(!WIPE_SYS_EndCheck()){
    return GFL_PROC_RES_CONTINUE;
  }
  
  if(pWork->pAppTask){
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
  }
  GSYNC_MESSAGE_End(pWork->pMessageWork);
  GSYNC_DISP_End(pWork->pDispWork);
  GSYNC_DOWNLOAD_Exit(pWork->pDownload);

  if(pWork->pMusical){
    GFL_HEAP_FreeMemory(pWork->pMusical);
  }
  if(pWork->pZknWork){
    GFL_HEAP_FreeMemory(pWork->pZknWork);
  }
  if(pWork->pCGearWork){
    GFL_HEAP_FreeMemory(pWork->pCGearWork);
  }
#if PM_DEBUG
  if(!GFL_HEAP_CheckHeapSafe(pWork->heapID)){
    GF_ASSERT(0);
  }
#endif
  if(pWork->pp){
    GFL_HEAP_FreeMemory(pWork->pp);
  }
  if(pWork->pTopAddr){
    GFL_HEAP_FreeMemory(pWork->pTopAddr);
  }

  NHTTP_RAP_End(pWork->pNHTTPRap);

  
  GFL_PROC_FreeWork(proc);
 
  GFL_HEAP_DeleteHeap(HEAPID_GAMESYNC);
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(dpw_common));
  return GFL_PROC_RES_FINISH;
}

// プロセス定義データ
const GFL_PROC_DATA G_SYNC_ProcData = {
  GSYNCProc_Init,
  GSYNCProc_Main,
  GSYNCProc_End,
};

