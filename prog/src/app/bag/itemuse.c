//============================================================================================
/**
 * @file    itemuse.c
 * @brief   フィールドアイテム使用処理
 * @author  Hiroyuki Nakamura
 * @date    05.12.12
 */
//============================================================================================
#include "gflib.h"


#include "app/itemuse.h"
#include "itemuse_local.h"

enum{ //便利ボタン機能関数呼び出しモード
  CONV_MODE_FUNC,
  CONV_MODE_EVENT,
};

//============================================================================================
//  定数定義
//============================================================================================
typedef void * (*CNVFUNC_CALL)(void*);

// 便利ボタンワーク
typedef struct {
  ITEMCHECK_WORK  icwk;
  CNVFUNC_CALL  call;
  void * app_wk;
  u16 item;
  u8  seq;
  u8  mode; //CONV_MODE_FUNC,CONV_MODE_EVENT
}CONV_WORK;

typedef BOOL (*ITEMCNV_FUNC)(CONV_WORK*); // 便利ボタン使用関数

typedef struct {
  ITEMUSE_FUNC  use_func;   // バッグから使用
  ITEMCNV_FUNC  cnv_func;   // 便利ボタンから使用
  ITEMCHECK_FUNC  check_func;   // 使用チェック
}ITEMUSE_FUNC_DATA;


//============================================================================================
//  プロトタイプ宣言
//============================================================================================
static void ItemUse_Recover( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk);
static void ItemUse_Townmap( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk);
static void ItemUse_PhotoAlbum( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk);
static void ItemUse_Cycle( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk);
static void ItemUse_WazaMachine( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk);
static void ItemUse_Mail( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk);
static void ItemUse_Kinomi( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk);
static void ItemUse_FriendBook( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk);
static void ItemUse_SweetHoney( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk);
static void ItemUse_FishingRodBad( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk);
static void ItemUse_FishingRodGood( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk);
static void ItemUse_FishingRodGreat( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk);
static void ItemUse_Evolution( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk);
static void ItemUse_Ananuke( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk);
static void ItemUse_BonguriCase( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk);
static void ItemUse_KinomiPlanter( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk);
static void ItemUse_UnknownNote( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk);
static void ItemUse_DowsingMachine( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk);
static void ItemUse_Grashidea( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk);
static void ItemUse_BattleRec( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk);


static BOOL ItemCnv_Cycle( CONV_WORK * wk );
static BOOL ItemCnv_FishingRodBad( CONV_WORK * wk );
static BOOL ItemCnv_FishingRodGood( CONV_WORK * wk );
static BOOL ItemCnv_FishingRodGreat( CONV_WORK * wk );
static BOOL ItemCnv_BagMsgItem( CONV_WORK * wk );
static BOOL ItemCnv_Townmap( CONV_WORK * wk );
static BOOL ItemCnv_FriendBook( CONV_WORK * wk );
static BOOL ItemCnv_PhotoAlbum( CONV_WORK * wk );
static void * ConvCall_FriendBook( void * fsys );
static void * ConvCall_Townmap( void * fsys );
static void * ConvCall_BonguriCase( void * fsys );
static void * ConvCall_KinomiPlanter( void * fsys );
static void * ConvCall_UnknownNote( void * fsys );
static void * ConvCall_PhotoAlbum( void * fsys );
static BOOL ItemCnv_BonguriCase( CONV_WORK * wk );
static BOOL ItemCnv_KinomiPlanter( CONV_WORK * wk );
static BOOL ItemCnv_UnknownNote( CONV_WORK * wk );
static BOOL ItemCnv_DowsingMachine( CONV_WORK * wk );
static BOOL ItemCnv_GB_Player( CONV_WORK * wk );
static BOOL ItemCnv_Grashidea( CONV_WORK * wk );
static void * ConvCall_Grashidea( void * fsys );
static BOOL ItemCnv_BattleRec( CONV_WORK * wk );
static void * ConvCall_BattleRec( void * fsys );


static u32 ItemUseCheck_Cycle( const ITEMCHECK_WORK * icwk );
static u32 ItemUseCheck_NoUse( const ITEMCHECK_WORK * icwk );
static u32 ItemUseCheck_Kinomi( const ITEMCHECK_WORK * icwk );
static u32 ItemUseCheck_FishingRod( const ITEMCHECK_WORK * icwk );
static u32 ItemUseCheck_Ananuke( const ITEMCHECK_WORK * icwk );

static BOOL GMEVENT_ItemUseCycle( GMEVENT * event );
static BOOL GMEVENT_BagMsgItem( GMEVENT * event );

static void ConvFuncCallSet( CONV_WORK * wk, CNVFUNC_CALL call ,u8 mode);
static BOOL GMEVENT_ConvButtonCommon( GMEVENT * event );

static BOOL GMEVENT_AnanukeMapChangeInit( GMEVENT *event );


//============================================================================================
//  グローバル変数
//============================================================================================
// 使用関数テーブル
static const ITEMUSE_FUNC_DATA ItemFuncTable[] =
{
  { // 000:ダミー
    NULL,       // バッグからの使用処理
    ItemCnv_BagMsgItem, // 便利ボタンからの使用処理
    NULL        // 使用チェック
    },
  { // 001:回復
    ItemUse_Recover,
    NULL,
    NULL
    },
  { // 002:タウンマップ
    NULL,
    NULL,
    ItemUseCheck_NoUse,
  },
  { // 003:フォトアルバム
    NULL,
    NULL,
    ItemUseCheck_NoUse,
  },
  { // 004:自転車
    ItemUse_Cycle,
    ItemCnv_Cycle,
    ItemUseCheck_Cycle,
  },
  { // 005:冒険ノート
    NULL,
    NULL,
    ItemUseCheck_NoUse,
  },
  { // 006:技マシン
    ItemUse_WazaMachine,
    NULL,
    NULL
    },
  { // 007:メール
    ItemUse_Mail,
    NULL,
    NULL
    },
  { // 008:きのみ
    ItemUse_Kinomi,
    NULL,
    ItemUseCheck_Kinomi
    },
  { // 009:ポルトケース　PORU_APP_DEL 08.06.24
    NULL,
    NULL,
    ItemUseCheck_NoUse,
  },
  { // 010:友達手帳
    ItemUse_FriendBook,
    ItemCnv_FriendBook,
    NULL
    },
  { // 011:ポケサーチャー
    NULL,
    NULL,
    ItemUseCheck_NoUse,
  },
  { // 012:じょうろ
    NULL,
    NULL,
    ItemUseCheck_NoUse,
  },
  { // 013:コヤシ
    NULL,
    NULL,
    ItemUseCheck_NoUse,
  },
  { // 014:甘い蜜
    ItemUse_SweetHoney,
    NULL,
    NULL
    },
  { // 015:バトルサーチャー
    NULL,
    NULL,
    ItemUseCheck_NoUse,
  },
  { // 016:ボロの釣竿
    ItemUse_FishingRodBad,
    ItemCnv_FishingRodBad,
    ItemUseCheck_FishingRod
    },
  { // 017:良い釣竿
    ItemUse_FishingRodGood,
    ItemCnv_FishingRodGood,
    ItemUseCheck_FishingRod
    },
  { // 018:凄い釣竿
    ItemUse_FishingRodGreat,
    ItemCnv_FishingRodGreat,
    ItemUseCheck_FishingRod
    },
  { // 019:バッグ内で使用する道具
    NULL,
    ItemCnv_BagMsgItem,
    NULL
    },
  { // 020:進化
    ItemUse_Evolution,
    NULL,
    NULL
    },
  { // 021:穴抜けの紐
    ItemUse_Ananuke,
    NULL,
    ItemUseCheck_Ananuke
    },
  { // 022:天界の笛
    NULL,
    NULL,
    ItemUseCheck_NoUse,
  },
  { // 023:ぼんぐりケース
    ItemUse_BonguriCase,
    ItemCnv_BonguriCase,
    NULL,
  },
  { // 024:きのみプランター
    ItemUse_KinomiPlanter,
    ItemCnv_KinomiPlanter,
    NULL,
  },
  { // 025:アンノーンノート
    ItemUse_UnknownNote,
    ItemCnv_UnknownNote,
    NULL,
  },
  { // 026:ダウジングマシン
    ItemUse_DowsingMachine,
    ItemCnv_DowsingMachine,
    NULL,
  },
  { // 027:ＧＢプレイヤー
    NULL,
    ItemCnv_GB_Player,
    NULL,
  },
  { // 028:グラシデアのはな
    ItemUse_Grashidea,
    ItemCnv_Grashidea,
    NULL,
  },
  { // 029:バトルレコーダー
    ItemUse_BattleRec,
    ItemCnv_BattleRec,
    NULL
    },
};



//--------------------------------------------------------------------------------------------
/**
 * アイテム使用関数取得
 *
 * @param prm   取得パラメータ
 * @param id    関数ID
 *
 * @return  使用関数
 */
//--------------------------------------------------------------------------------------------
u32 ITEMUSE_GetUseFunc( u16 prm, u16 id )
{
  if( prm == ITEMUSE_PRM_USEFUNC ){
    return (u32)ItemFuncTable[id].use_func;
  }else if( prm == ITEMUSE_PRM_CNVFUNC ){
    return (u32)ItemFuncTable[id].cnv_func;
  }
  return (u32)ItemFuncTable[id].check_func;
}

//--------------------------------------------------------------------------------------------
/**
 * 使用チェックワーク作成
 *
 * @param fsys  フィールドワーク
 * @param icwk  チェックワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void ItemUse_CheckWorkMake( GAMESYS_WORK * gsys, ITEMCHECK_WORK * icwk )
{

  icwk->gsys = gsys;

  //ゾーンＩＤ
//  icwk->zone_id = fsys->location->zone_id;

  //連れ歩き
//  icwk->Companion = FALSE; //SysFlag_PairCheck(SaveData_GetEventWork(fsys->savedata));
  //ロケット団変装中
//  icwk->Disguise = FALSE; //SysFlag_DisguiseCheck(SaveData_GetEventWork(fsys->savedata));

  //自機形状
//  icwk->PlayerForm = Player_FormGet( fsys->player );
//  x = Player_NowGPosXGet( fsys->player );
//  z = Player_NowGPosZGet( fsys->player );
  //現在アトリビュート
//  icwk->NowAttr = GetAttributeLSB(fsys, x, z);

  //前方アトリビュート
//  icwk->FrontAttr = GetAttributeLSB(fsys, x, z);
//  SXY_HeroFrontObjGet(fsys, &obj);
  //きのみ関連どうぐの使用チェック
  /// icwk->SeedInfo = SeedSys_ItemUseCheck(fsys, obj);

  //自機情報参照ポインタ
//  icwk->player = fsys->player;
}

//============================================================================================
//
//  スクリプト呼び出し
//
//============================================================================================
static BOOL GMEVENT_ItemScript(GMEVENT * event);
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
typedef struct {
  u32 scr_id;   ///<呼び出すスクリプトのID
  u16 param0;   ///<引き渡すパラメータその１
  u16 param1;   ///<引き渡すパラメータその２
  u16 param2;   ///<引き渡すパラメータその３
  u16 param3;   ///<引き渡すパラメータその４
}CALL_SCR_WORK;

//--------------------------------------------------------------------------------------------
/**
 * @brief
 * @param scr_id;   ///<呼び出すスクリプトのID
 * @param param0;   ///<引き渡すパラメータその１
 * @param param1;   ///<引き渡すパラメータその２
 * @param param2;   ///<引き渡すパラメータその３
 * @param param3;   ///<引き渡すパラメータその４
 * @return  CALL_SCR_WORK スクリプト呼び出し用ワーク
 */
//--------------------------------------------------------------------------------------------
static CALL_SCR_WORK * CSW_Create(HEAPID heapID, u32 scr_id, u16 param0, u16 param1, u16 param2, u16 param3)
{
  CALL_SCR_WORK * csw;
  csw = GFL_HEAP_AllocMemory(heapID, sizeof(CALL_SCR_WORK));
  csw->scr_id = scr_id;
  csw->param0 = param0;
  csw->param1 = param1;
  csw->param2 = param2;
  csw->param3 = param3;
  return csw;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief アイテム用スクリプト起動の設定
 * @param iuwk  アイテム使用ワーク
 * @param icwk  アイテム使用チェックワーク
 * @param scr_id  スクリプトID
 */
//--------------------------------------------------------------------------------------------
static void SetUpItemScript(ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk, u32 scr_id,HEAPID heapID)
{

//  FieldEvent_Cmd_SetMapProc( fsys );

//  mwk->app_func = GMEVENT_ItemScript;
  //mwk->app_wk   = CSW_Create(HEAPID heapID,scr_id, iuwk->item, 0, 0, 0);
  //mwk->seq      = FLD_MENU_SEQ_BLACK_IN_EVCHG;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief アイテム用スクリプト起動の設定（便利ボタン）
 * @param iuwk  アイテム使用ワーク
 * @param icwk  アイテム使用チェックワーク
 * @param scr_id  スクリプトID
 */
//--------------------------------------------------------------------------------------------
static void SetUpItemScriptCnv( CONV_WORK * wk, u32 scr_id )
{
#if 0
  void * work = CSW_Create(scr_id, wk->item, 0, 0, 0);
  FieldEvent_Set( wk->fsys, GMEVENT_ItemScript, work );
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * @brief アイテム用スクリプトの呼び出しイベント
 * @param event イベント制御ワークへのポインタ
 * @return  BOOL  TRUEでイベント終了、FALSEでイベント続行
 */
//--------------------------------------------------------------------------------------------
static BOOL GMEVENT_ItemScript(GMEVENT * event)
{
#if 0
  FIELDSYS_WORK * fsys = FieldEvent_GetFieldSysWork(event);
  CALL_SCR_WORK * csw = FieldEvent_GetSpecialWork(event);
  int * seq = FieldEvent_GetSeqWork(event);
  FIELD_OBJ_PTR obj;
  switch (*seq) {
  case 0:
    SXY_HeroFrontObjGet(fsys, &obj);
    EventCall_Script(event, csw->scr_id, obj, NULL);
    *(u16*)(GetEvScriptWorkMemberAdrs(fsys,ID_EVSCR_WK_PARAM0)) = csw->param0;
    *(u16*)(GetEvScriptWorkMemberAdrs(fsys,ID_EVSCR_WK_PARAM1)) = csw->param1;
    *(u16*)(GetEvScriptWorkMemberAdrs(fsys,ID_EVSCR_WK_PARAM2)) = csw->param2;
    *(u16*)(GetEvScriptWorkMemberAdrs(fsys,ID_EVSCR_WK_PARAM3)) = csw->param3;

    (*seq) ++;
    break;
  case 1:
    sys_FreeMemoryEz(csw);
    return TRUE;
  }
#endif
  return FALSE;
}

//============================================================================================
//  001:回復
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * アイテム使用関数：回復
 *
 * @param iuwk  アイテム使用ワーク
 * @param icwk  チェックワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ItemUse_Recover( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk)
{
#if 0
  FIELDSYS_WORK * fsys;
  FLD_MENU * mwk;
  PLIST_DATA * pld;

  fsys = FieldEvent_GetFieldSysWork( iuwk->event );
  mwk  = FieldEvent_GetSpecialWork( iuwk->event );
  pld  = sys_AllocMemory( HEAPID_WORLD, sizeof(PLIST_DATA) );

  memset( pld, 0, sizeof(PLIST_DATA) );
  pld->pp     = SaveData_GetTemotiPokemon( fsys->savedata );
  pld->myitem = SaveData_GetMyItem( fsys->savedata );
  pld->mailblock = SaveData_GetMailBlock( fsys->savedata );
  pld->cfg = SaveData_GetConfig( fsys->savedata );
  pld->tvwk = SaveData_GetTvWork( fsys->savedata );
  pld->scwk = &mwk->skill_check;
  pld->type = PL_TYPE_SINGLE;
  pld->mode = PL_MODE_ITEMUSE;
  pld->fsys = fsys;
  pld->item = iuwk->item;
  pld->ret_sel = iuwk->use_poke;
  pld->kt_status = &fsys->KeyTouchStatus;
  GameSystem_StartSubProc( fsys, &PokeListProcData, pld );

  mwk->app_wk = pld;
  FldMenu_AppWaitFuncSet( mwk, FldMenu_PokeListEnd );
#endif
}


//============================================================================================
//  002:タウンマップ
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * アイテム使用関数：タウンマップ
 *
 * @param iuwk  アイテム使用ワーク
 * @param icwk  チェックワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ItemUse_Townmap( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk)
{
#if 0
  FIELDSYS_WORK * fsys;
  FLD_MENU * mwk;

  fsys = FieldEvent_GetFieldSysWork( iuwk->event );
  mwk  = FieldEvent_GetSpecialWork( iuwk->event );

  mwk->app_wk =FieldPGearSky_SetProc(fsys ,PGSKY_CALL_BG);
  FldMenu_AppWaitFuncSet( mwk, FldMenu_SubAppFreeEnd );
#endif
}


//--------------------------------------------------------------------------------------------
/**
 * 便利ボタン使用関数：タウンマップ
 *
 * @param wk  便利ボタンワーク
 *
 * @return  TRUE（便利ボタンワークを解放しない）
 */
//--------------------------------------------------------------------------------------------
static BOOL ItemCnv_Townmap( CONV_WORK * wk )
{
#if 0
  ConvFuncCallSet( wk, ConvCall_Townmap, CONV_MODE_FUNC);
#endif
  return TRUE;
}

static void * ConvCall_Townmap( void * fsys )
{
#if 0
  return FieldPGearSky_SetProc(fsys ,PGSKY_CALL_BG);
#endif
  return NULL;
}



//============================================================================================
//  003:フォトアルバム
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * アイテム使用関数：フォトアルバム
 *
 * @param iuwk  アイテム使用ワーク
 * @param icwk  チェックワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ItemUse_PhotoAlbum( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk)
{
#if 0
  FIELDSYS_WORK * fsys;
  FLD_MENU * mwk;

  fsys = FieldEvent_GetFieldSysWork( iuwk->event );
  mwk  = FieldEvent_GetSpecialWork( iuwk->event );

  EventCmd_CallFieldAlbum(fsys);

  FldMenu_EvCallWaitFuncSet( mwk, FldMenu_AlbumEnd);
  fsys->startmenu_pos = 0;    // メニュー位置クリア
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * 便利ボタン使用関数：フォトアルバム
 *
 * @param wk  便利ボタンワーク
 *
 * @return  FALSE（便利ボタンワーク解放）
 */
//--------------------------------------------------------------------------------------------
static BOOL ItemCnv_PhotoAlbum( CONV_WORK * wk )
{
#if 0
  ConvFuncCallSet( wk, ConvCall_PhotoAlbum, CONV_MODE_EVENT );
  wk->fsys->startmenu_pos = 0;    // メニュー位置クリア
#endif
  return TRUE;
}

static void * ConvCall_PhotoAlbum( void * fsys )
{
#if 0
  EventCmd_CallFieldAlbum(fsys);
#endif
  return NULL;
}

//--------------------------------------------------------------------------------------------
/**
 * アイテム使用チェック関数：汎用
 *
 * @param icwk  アイテム使用チェックワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static u32 ItemUseCheck_NoUse( const ITEMCHECK_WORK * icwk )
{
  return ITEMCHECK_FALSE; //アイテムとしては使えない
}


//============================================================================================
//  004:自転車
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * アイテム使用関数：自転車
 *
 * @param iuwk  アイテム使用ワーク
 * @param icwk  チェックワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ItemUse_Cycle( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk)
{
//  FIELDSYS_WORK * fsys;
//  FLD_MENU * mwk;

//  fsys = FieldEvent_GetFieldSysWork( iuwk->event );
//  mwk  = FieldEvent_GetSpecialWork( iuwk->event );

//  FieldEvent_Cmd_SetMapProc( fsys );
//  mwk->app_func = GMEVENT_ItemUseCycle;
//  mwk->app_wk   = NULL;
//  mwk->seq      = FLD_MENU_SEQ_BLACK_IN_EVCHG;
}

//--------------------------------------------------------------------------------------------
/**
 * 便利ボタン使用関数：自転車
 *
 * @param wk  便利ボタンワーク
 *
 * @return  FALSE（便利ボタンワーク解放）
 */
//--------------------------------------------------------------------------------------------
static BOOL ItemCnv_Cycle( CONV_WORK * wk )
{
//  FieldEvent_Set( wk->fsys, GMEVENT_ItemUseCycle, NULL );
//  wk->fsys->sublcd_menu_lock_passive = 1;
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * アイテム使用イベント：自転車
 *
 * @param event フィールドイベント用パラメータ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static BOOL GMEVENT_ItemUseCycle( GMEVENT * event )
{
#if 0
  FIELDSYS_WORK * fsys;
  int * seq;
  fsys = FieldEvent_GetFieldSysWork( event );
  seq = FieldEvent_GetSeqWork( event );

  switch (*seq) {
  case 0:
    if (PairPoke_IsPairPoke(fsys)){
      //ポケモンの移動アニメを終わらせる
      if ( FieldOBJ_AcmdEndCheck( PairPoke_GetFldObjPtr(fsys) ) ){
        int code;
        if( Player_FormGet( fsys->player ) == HERO_FORM_CYCLE ){
          code = MV_PAIR;
        }else{
          //code = MV_PAIR_NO_RATE;
          code = MV_PAIR_NO_RATE_CP_MV;
        }
        //コードチェンジ
        FieldOBJ_MoveCodeChange( PairPoke_GetFldObjPtr(fsys), code );
        (*seq) ++;
      }
    }else{
      (*seq) ++;
    }
    break;
  case 1:
    if( Player_FormGet( fsys->player ) == HERO_FORM_CYCLE ){

      FieldOBJ_MovePauseClear( Player_FieldOBJGet( fsys->player ) );
      Player_RequestSet( fsys->player, HERO_REQBIT_NORMAL );
      Player_Request( fsys->player );

      //先に形状を戻してから、BGM処理を呼ぶ！(060803)

      Snd_FieldBgmSetSpecial( fsys, 0 );          //クリア
      // ラジオが再生されている時は自転車曲が鳴らない
      if(PMGS_GetRadioMusicID()==0){
        Snd_FadeOutNextPlayCall(fsys,
                                Snd_FieldBgmNoGet(fsys,fsys->location->zone_id), BGM_FADE_FIELD_MODE );

      }
      //ポケモンを主人公の位置へ
      PairPoke_SetPlayerPos(fsys, Player_DirGet( fsys->player ));
      //連れ歩きポケ出現アニメリクエスト
      if (PairPoke_IsPairPoke(fsys)){
        PairPoke_SetAppAnmPrm(PairPoke_GetFldObjPtr(fsys), 1);
        //一歩移動でバニッシュ解除状態にする
        PairPoke_StartVanish(PairPoke_GetFldObjPtr(fsys), 1);
      }
    }else{
      //先にBGM処理を呼んでから、形状を自転車にする！(060803)

      // ラジオが再生されている時は自転車曲が鳴らない
      if(PMGS_GetRadioMusicID()==0){
        Snd_FieldBgmSetSpecial( fsys, SEQ_GS_BICYCLE );   //セット
        Snd_FadeOutNextPlayCall( fsys, SEQ_GS_BICYCLE, BGM_FADE_FIELD_MODE );
      }

      FieldOBJ_MovePauseClear( Player_FieldOBJGet( fsys->player ) );
      Player_RequestSet( fsys->player, HERO_REQBIT_CYCLE );
      Player_Request( fsys->player );

      PairPokeEvAnm_BallIn(fsys);
      //連れ歩きポケ出現アニメクリア
      if (PairPoke_IsPairPoke(fsys)){
        PairPoke_SetAppAnmPrm(PairPoke_GetFldObjPtr(fsys), 0);
      }
    }
    (*seq) ++;
    break;
  case 2:
    FieldOBJSys_MovePauseAllClear( fsys->fldobjsys );
    return TRUE;
  }
#endif
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * アイテム使用チェック関数：自転車
 *
 * @param icwk  アイテム使用チェックワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static u32 ItemUseCheck_Cycle( const ITEMCHECK_WORK * icwk )
{
#if 0

  // 連れ歩き時は使用不可
  if( icwk->Companion == TRUE ){
    return ITEMCHECK_ERR_COMPANION;
  }
  // ロケット団変装時は使用不可
  if( icwk->Disguise == TRUE ){
    return ITEMCHECK_ERR_DISGUISE;
  }

  // サイクリングロードチェック
  if( Player_MoveBitCheck_CyclingRoad( icwk->player ) == TRUE ){
    return ITEMCHECK_ERR_CYCLE_OFF;
  }

  // 長い草 / 沼
  if( MATR_IsLongGrass( icwk->NowAttr ) == TRUE ||
      MATR_IsSwamp( icwk->NowAttr ) == TRUE ){
    return ITEMCHECK_FALSE;
  }

  // ゾーンチェック
  if( ZoneData_GetEnableBicycleFlag(icwk->zone_id) == FALSE ){
    return ITEMCHECK_FALSE;
  }

  //なみのり状態かどうか
  if( icwk->PlayerForm == HERO_FORM_SWIM ){
    return ITEMCHECK_FALSE;
  }
#endif
  return ITEMCHECK_TRUE;
}


//============================================================================================
//  005:冒険ノート
//============================================================================================

//============================================================================================
//  006:技マシン
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * アイテム使用関数：技マシン
 *
 * @param iuwk  アイテム使用ワーク
 * @param icwk  チェックワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ItemUse_WazaMachine( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk)
{
#if 0
  FIELDSYS_WORK * fsys;
  FLD_MENU * mwk;
  PLIST_DATA * pld;

  fsys = FieldEvent_GetFieldSysWork( iuwk->event );
  mwk  = FieldEvent_GetSpecialWork( iuwk->event );
  pld  = sys_AllocMemory( HEAPID_WORLD, sizeof(PLIST_DATA) );

  memset( pld, 0, sizeof(PLIST_DATA) );
  pld->pp     = SaveData_GetTemotiPokemon( fsys->savedata );
  pld->myitem = SaveData_GetMyItem( fsys->savedata );
  pld->mailblock = SaveData_GetMailBlock( fsys->savedata );
  pld->cfg = SaveData_GetConfig( fsys->savedata );
  pld->scwk = &mwk->skill_check;
  pld->type = PL_TYPE_SINGLE;
  pld->mode = PL_MODE_WAZASET;
  pld->fsys = fsys;
  pld->item = iuwk->item;
  pld->ret_sel = iuwk->use_poke;
  pld->waza = GetWazaNoItem( iuwk->item );
  pld->kt_status = &fsys->KeyTouchStatus;
  GameSystem_StartSubProc( fsys, &PokeListProcData, pld );

  mwk->app_wk = pld;
  FldMenu_AppWaitFuncSet( mwk, FldMenu_PokeListEnd );
#endif
}


//============================================================================================
//  007:メール
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * アイテム使用関数：メール
 *
 * @param iuwk  アイテム使用ワーク
 * @param icwk  チェックワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ItemUse_Mail( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk)
{
#if 0
  FIELDSYS_WORK * fsys;
  FLD_MENU * mwk;
  MAIL_PARAM * mail;

  fsys = FieldEvent_GetFieldSysWork( iuwk->event );
  mwk  = FieldEvent_GetSpecialWork( iuwk->event );
  mail = FieldMailView_InitCall(
    fsys, MAILBLOCK_NULL, ItemMailDesignGet(iuwk->item), HEAPID_WORLD );

  mwk->tmp_wk = FieldMenu_MailWorkMake( iuwk->item, FM_MAIL_MODE_VIEW, 0 );
  mwk->app_wk = mail;
  FldMenu_AppWaitFuncSet( mwk, FldMenu_MailEnd );
#endif
}


//============================================================================================
//  008:きのみ
//============================================================================================
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
static u32 ItemUseCheck_Kinomi(const ITEMCHECK_WORK * icwk)
{
  return ITEMCHECK_TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * アイテム使用関数：きのみ
 *
 * @param iuwk  アイテム使用ワーク
 * @param icwk  チェックワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ItemUse_Kinomi( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk)
{
#if 0
  FIELDSYS_WORK * fsys;
  FLD_MENU * mwk;
  FIELD_OBJ_PTR obj;

  fsys = FieldEvent_GetFieldSysWork( iuwk->event );
  mwk  = FieldEvent_GetSpecialWork( iuwk->event );
  ItemUse_Recover( iuwk, icwk );
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * きのみ使用タイプ取得
 *
 * @param iuwk  アイテム使用ワーク
 *
 * @retval  "TRUE = 埋める"
 * @retval  "FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL ItemUse_KinomiTypeCheck( const ITEMCHECK_WORK * icwk )
{
  return FALSE;
}

//============================================================================================
//  010:友達手帳
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * アイテム使用関数：友達手帳
 *
 * @param iuwk  アイテム使用ワーク
 * @param icwk  チェックワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ItemUse_FriendBook( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk)
{
//  FIELDSYS_WORK * fsys;
//  FLD_MENU * mwk;

  //fsys = FieldEvent_GetFieldSysWork( iuwk->event );
  //mwk  = FieldEvent_GetSpecialWork( iuwk->event );

//  mwk->app_wk = WifiNote_SetProc( fsys, fsys->savedata ,HEAPID_WORLD);

  ///// @@ WifiNoteProcData

  //FldMenu_AppWaitFuncSet( mwk, FldMenu_SubAppFreeEnd );
}

//--------------------------------------------------------------------------------------------
/**
 * 便利ボタン使用関数：友達手帳
 *
 * @param wk  便利ボタンワーク
 *
 * @return  TRUE（便利ボタンワークを解放しない）
 */
//--------------------------------------------------------------------------------------------
static BOOL ItemCnv_FriendBook( CONV_WORK * wk )
{
#if 0
  ConvFuncCallSet( wk, ConvCall_FriendBook,CONV_MODE_FUNC );
#endif
  return TRUE;
}

static void * ConvCall_FriendBook( void * fsys )
{
#if 0
  return WifiNote_SetProc( fsys, ((FIELDSYS_WORK *)fsys)->savedata ,HEAPID_WORLD);
#endif
  return NULL;
}

//============================================================================================
//  014:甘い蜜
//============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * @brief アイテム使用関数：甘い蜜
 */
//--------------------------------------------------------------------------------------------
static void ItemUse_SweetHoney( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk)
{
#if 0
  FIELDSYS_WORK * fsys;
  FLD_MENU * mwk;
  EVENT_AMAIKAORI_EFF_WORK  *work;
  int size;

  fsys = FieldEvent_GetFieldSysWork( iuwk->event );
  mwk  = FieldEvent_GetSpecialWork( iuwk->event );

  FieldEvent_Cmd_SetMapProc( fsys );

  size = FLDMAP_Amaikaori_WorkSize();
  work = sys_AllocMemoryLo( HEAPID_WORLD, size );
  memset(work, 0, size);

  mwk->app_func = FLDMAP_AmaikaoriEvent;
  mwk->app_wk   = work;
  mwk->seq      = FLD_MENU_SEQ_BLACK_IN_EVCHG;
  //アイテム減らす
  MyItem_SubItem( SaveData_GetMyItem(fsys->savedata), iuwk->item, 1, HEAPID_WORLD );
#endif
}

//============================================================================================
//  016:ボロい釣竿
//============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * @brief アイテム使用関数：ボロい釣竿
 */
//--------------------------------------------------------------------------------------------
static void ItemUse_FishingRodBad( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk)
{
  #if 0

  FIELDSYS_WORK * fsys;
  FLD_MENU * mwk;

  fsys = FieldEvent_GetFieldSysWork( iuwk->event );
  mwk  = FieldEvent_GetSpecialWork( iuwk->event );

  FieldEvent_Cmd_SetMapProc( fsys );
  mwk->app_func = GMEVENT_Fishing;
  mwk->app_wk   = EventWorkSet_Fishing( fsys, HEAPID_WORLD, FISHINGROD_BAD );
  mwk->seq      = FLD_MENU_SEQ_BLACK_IN_EVCHG;
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * 便利ボタン使用関数：ボロい釣竿
 * @param wk  便利ボタンワーク
 * @return  FALSE（便利ボタンワーク解放）
 */
//--------------------------------------------------------------------------------------------
static BOOL ItemCnv_FishingRodBad( CONV_WORK * wk )
{
#if 0
  void *work = EventWorkSet_Fishing( wk->fsys, HEAPID_FIELD, FISHINGROD_BAD );
  FieldEvent_Set( wk->fsys, GMEVENT_Fishing, work );
#endif
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief アイテム使用関数：良い釣竿
 */
//--------------------------------------------------------------------------------------------
static void ItemUse_FishingRodGood( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk)
{
#if 0
  FIELDSYS_WORK * fsys;
  FLD_MENU * mwk;

  fsys = FieldEvent_GetFieldSysWork( iuwk->event );
  mwk  = FieldEvent_GetSpecialWork( iuwk->event );

  FieldEvent_Cmd_SetMapProc( fsys );
  mwk->app_func = GMEVENT_Fishing;
  mwk->app_wk   = EventWorkSet_Fishing( fsys, HEAPID_WORLD, FISHINGROD_GOOD );
  mwk->seq      = FLD_MENU_SEQ_BLACK_IN_EVCHG;
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * 便利ボタン使用関数：良い釣竿
 * @param wk  便利ボタンワーク
 * @return  FALSE（便利ボタンワーク解放）
 */
//--------------------------------------------------------------------------------------------
static BOOL ItemCnv_FishingRodGood( CONV_WORK * wk )
{
#if 0
  void *work = EventWorkSet_Fishing( wk->fsys, HEAPID_FIELD, FISHINGROD_GOOD );
  FieldEvent_Set( wk->fsys, GMEVENT_Fishing, work );
#endif
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief アイテム使用関数：凄い釣竿
 */
//--------------------------------------------------------------------------------------------
static void ItemUse_FishingRodGreat( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk)
{
#if 0
  FIELDSYS_WORK * fsys;
  FLD_MENU * mwk;

  fsys = FieldEvent_GetFieldSysWork( iuwk->event );
  mwk  = FieldEvent_GetSpecialWork( iuwk->event );

  FieldEvent_Cmd_SetMapProc( fsys );
  mwk->app_func = GMEVENT_Fishing;
  mwk->app_wk   = EventWorkSet_Fishing( fsys, HEAPID_WORLD, FISHINGROD_GREAT );
  mwk->seq      = FLD_MENU_SEQ_BLACK_IN_EVCHG;
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * 便利ボタン使用関数：凄い釣竿
 * @param wk  便利ボタンワーク
 * @return  FALSE（便利ボタンワーク解放）
 */
//--------------------------------------------------------------------------------------------
static BOOL ItemCnv_FishingRodGreat( CONV_WORK * wk )
{
#if 0
  void *work = EventWorkSet_Fishing( wk->fsys, HEAPID_FIELD, FISHINGROD_GREAT );
  FieldEvent_Set( wk->fsys, GMEVENT_Fishing, work );
#endif
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * アイテム使用チェック関数：釣竿
 *
 * @param icwk  アイテム使用チェックワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static u32 ItemUseCheck_FishingRod( const ITEMCHECK_WORK * icwk )
{
#if 0
  // 連れ歩き時は使用不可
  if( icwk->Companion == TRUE ){
    return ITEMCHECK_ERR_COMPANION;
  }
  // ロケット団変装時は使用不可
  if( icwk->Disguise == TRUE ){
    return ITEMCHECK_ERR_DISGUISE;
  }

  // 目の前が水
  if( MATR_IsWater(icwk->FrontAttr) == TRUE ){
    // 橋の上にいなければOK
    if( MATR_IsBridge( icwk->NowAttr ) == TRUE ||
        MATR_IsBridgeFlag( icwk->NowAttr ) == TRUE ){
      FIELD_OBJ_PTR fldobj = Player_FieldOBJGet( icwk->player );
      if( FieldOBJ_StatusBitCheck_Bridge(fldobj) == TRUE ){
        return ITEMCHECK_FALSE;
      }
    }
    return ITEMCHECK_TRUE;
  }
#endif
  return ITEMCHECK_FALSE;
}


//============================================================================================
//  019:バッグ内で使用する道具
//    コインケース、アクセサリー入れ、シール入れ、ポイントカード
//============================================================================================
typedef struct {
  GFL_BMPWIN *win;  // ウィンドウデータ
  STRBUF * msg;   // メッセージ
  u16 midx;     // メッセージインデックス
  u16 seq;      // シーケンス
}BAGMSG_WORK;

#define BAGMSG_TMP_SIZ    ( 128 )


//--------------------------------------------------------------------------------------------
/**
 * 便利ボタン使用関数：バッグ内で使用する道具
 *
 * @param wk  便利ボタンワーク
 *
 * @return  FALSE（便利ボタンワークを解放）
 */
//--------------------------------------------------------------------------------------------
static BOOL ItemCnv_BagMsgItem( CONV_WORK * wk )
{
#if 0
  BAGMSG_WORK * bmwk = sys_AllocMemory( HEAPID_WORLD, sizeof(BAGMSG_WORK) );
  bmwk->seq = 0;

  bmwk->msg = STRBUF_Create( BAGMSG_TMP_SIZ, HEAPID_WORLD );
  BAG_ItemUseMsgSet( wk->fsys->savedata, bmwk->msg, wk->item, HEAPID_WORLD );

  FieldEvent_Set( wk->fsys, GMEVENT_BagMsgItem, bmwk );
#endif
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * バッグ内で使用する道具：便利ボタンから起動された場合のイベント
 *
 * @param event イベントデータ
 *
 * @retval  "TRUE = 終了"
 * @retval  "FALSE = 処理中"
 */
//--------------------------------------------------------------------------------------------
static BOOL GMEVENT_BagMsgItem( GMEVENT * event )
{
#if 0
  FIELDSYS_WORK * fsys;
  BAGMSG_WORK * wk;

  fsys = FieldEvent_GetFieldSysWork( event );
  wk   = FieldEvent_GetSpecialWork( event );

  switch( wk->seq ){
  case 0:
    fsys->sublcd_menu_next_flag = 1;

    FieldOBJSys_MovePauseAll( fsys->fldobjsys );

    /*
    wk->msg = STRBUF_Create( BAGMSG_TMP_SIZ, HEAPID_WORLD );

    BAG_ItemUseMsgSet(
      fsys->savedata, wk->msg,
      MyItem_CnvButtonItemGet( SaveData_GetMyItem(fsys->savedata) ), HEAPID_WORLD );
     */

    FldTalkBmpAdd( fsys->bgl, &wk->win, FLD_MBGFRM_FONT );
    {
      const CONFIG * cfg = SaveData_GetConfig( fsys->savedata );
      FieldTalkWinPut( &wk->win, cfg );
      wk->midx = FieldTalkMsgStart( &wk->win, wk->msg, cfg, 1 );
    }
    wk->seq++;
    break;

  case 1:
    if( FldTalkMsgEndCheck( wk->midx ) == 1 ){
      if(sys.trg & (PAD_KEY_UP|PAD_KEY_DOWN|PAD_KEY_LEFT|PAD_KEY_RIGHT|PAD_BUTTON_A|PAD_BUTTON_B) || sys.tp_trg){
        fsys->sublcd_menu_next_flag = 0;
        BmpTalkWinClear( &wk->win, WINDOW_TRANS_ON );
        wk->seq++;
      }
    }
    break;

  case 2:
    FieldOBJSys_MovePauseAllClear( fsys->fldobjsys );
    GF_BGL_BmpWinDel( &wk->win );
    STRBUF_Delete( wk->msg );
    sys_FreeMemoryEz( wk );
    return TRUE;
  }
#endif
  return FALSE;
}


//============================================================================================
//  020:進化
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * アイテム使用関数：進化アイテム
 *
 * @param iuwk  アイテム使用ワーク
 * @param icwk  チェックワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ItemUse_Evolution( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk )
{
#if 0
  FIELDSYS_WORK * fsys;
  FLD_MENU * mwk;
  PLIST_DATA * pld;

  fsys = FieldEvent_GetFieldSysWork( iuwk->event );

  //アルフの遺跡　隠し部屋オープンギミック　水の石使用チェック
  if( iuwk->item == ITEM_MIZUNOISI &&
      AlfGimmick_D24R0206MizunoisiCheck(fsys)){
    SetUpItemScript(iuwk, icwk, SCRID_D24R0206_MIZU_ACTION);
    return;
  }

  mwk  = FieldEvent_GetSpecialWork( iuwk->event );
  pld  = sys_AllocMemory( HEAPID_WORLD, sizeof(PLIST_DATA) );

  memset( pld, 0, sizeof(PLIST_DATA) );
  pld->pp     = SaveData_GetTemotiPokemon( fsys->savedata );
  pld->myitem = SaveData_GetMyItem( fsys->savedata );
  pld->mailblock = SaveData_GetMailBlock( fsys->savedata );
  pld->cfg = SaveData_GetConfig( fsys->savedata );
  pld->tvwk = SaveData_GetTvWork( fsys->savedata );
  pld->scwk = &mwk->skill_check;
  pld->type = PL_TYPE_SINGLE;
  pld->mode = PL_MODE_SHINKA;
  pld->item = iuwk->item;
  pld->ret_sel = iuwk->use_poke;
  pld->fsys    = fsys;
  pld->kt_status = &fsys->KeyTouchStatus;
  GameSystem_StartSubProc( fsys, &PokeListProcData, pld );

  mwk->app_wk = pld;
  FldMenu_AppWaitFuncSet( mwk, FldMenu_PokeListEnd );]
#endif
}


//============================================================================================
//  021 :穴抜けの紐
//============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * @brief アイテム使用関数：穴抜けの紐
 */
//--------------------------------------------------------------------------------------------
static void ItemUse_Ananuke( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk)
{
#if 0
  FIELDSYS_WORK * fsys;
  FLD_MENU * mwk;

  //アルフの遺跡　穴抜けギミックチェック
  if(AlfGimmick_D24R0202AnanukeCheck(icwk->fsys)){
    SetUpItemScript(iuwk, icwk, SCRID_D24R0202_ANANUKE_ACTION);
    return;
  }
  //通常処理
  fsys = FieldEvent_GetFieldSysWork( iuwk->event );
  mwk  = FieldEvent_GetSpecialWork( iuwk->event );

  FieldEvent_Cmd_SetMapProc( fsys );
  mwk->app_func = GMEVENT_AnanukeMapChangeInit;
  mwk->app_wk   = NULL;
  mwk->seq      = FLD_MENU_SEQ_BLACK_IN_EVCHG;
  //アイテム減らす
  MyItem_SubItem( SaveData_GetMyItem(fsys->savedata), iuwk->item, 1, HEAPID_WORLD );
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * アイテム使用チェック関数：穴抜けの紐
 *
 * @param icwk  アイテム使用チェックワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static u32 ItemUseCheck_Ananuke( const ITEMCHECK_WORK * icwk )
{
#if 0
  // 連れ歩き時は使用不可
  if( icwk->Companion == TRUE ){
    return ITEMCHECK_ERR_COMPANION;
  }
  // ロケット団変装時は使用不可
  if( icwk->Disguise == TRUE ){
    return ITEMCHECK_ERR_DISGUISE;
  }
  if(AlfGimmick_D24R0202AnanukeCheck(icwk->fsys)){
    return ITEMCHECK_TRUE;
  }
  if( ZoneData_IsDungeon(icwk->zone_id) == TRUE &&
      ZoneData_GetEnableEscapeFlag(icwk->zone_id) == TRUE ){
    return ITEMCHECK_TRUE;
  }
#endif

  return ITEMCHECK_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief 穴抜けの紐ワーク初期化→穴抜けイベントへ
 */
//--------------------------------------------------------------------------------------------
static BOOL GMEVENT_AnanukeMapChangeInit( GMEVENT *event )
{
#if 0
  FIELDSYS_WORK *fsys = FieldEvent_GetFieldSysWork( event );
  void *work = EventWorkSet_AnanukeMapChange( fsys, HEAPID_WORLD );
  FieldEvent_Change( event, GMEVENT_AnanukeMapChange, work );
#endif
  return( FALSE );
}

//============================================================================================
//  023:ぼんぐりケース
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * アイテム使用関数：ぼんぐりケース
 *
 * @param iuwk  アイテム使用ワーク
 * @param icwk  チェックワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ItemUse_BonguriCase( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk)
{
#if 0
  FIELDSYS_WORK * fsys;
  FLD_MENU * mwk;

  fsys = FieldEvent_GetFieldSysWork( iuwk->event );
  mwk  = FieldEvent_GetSpecialWork( iuwk->event );

  mwk->app_wk = FieldBongCase_SetProc( fsys, BONGCASE_POT);

  FldMenu_AppWaitFuncSet( mwk, FldMenu_SubAppFreeEnd );
#endif
}


//--------------------------------------------------------------------------------------------
/**
 * 便利ボタン使用関数：ぼんぐりケース
 *
 * @param wk  便利ボタンワーク
 *
 * @return  TRUE（便利ボタンワークを解放しない）
 */
//--------------------------------------------------------------------------------------------
static BOOL ItemCnv_BonguriCase( CONV_WORK * wk )
{
#if 0
  ConvFuncCallSet( wk, ConvCall_BonguriCase, CONV_MODE_FUNC );
#endif
  return TRUE;
}

static void * ConvCall_BonguriCase( void * fsys )
{
#if 0
  return FieldBongCase_SetProc( fsys, BONGCASE_POT);
#endif
  return NULL;
}

//============================================================================================
//  024:きのみプランター
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * アイテム使用関数：きのみプランター
 *
 * @param iuwk  アイテム使用ワーク
 * @param icwk  チェックワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ItemUse_KinomiPlanter( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk)
{
#if 0
  FIELDSYS_WORK * fsys;
  FLD_MENU * mwk;

  fsys = FieldEvent_GetFieldSysWork( iuwk->event );
  mwk  = FieldEvent_GetSpecialWork( iuwk->event );

  mwk->app_wk = FieldNutPlanter_SetProc( fsys );

  FldMenu_AppWaitFuncSet( mwk, FldMenu_SubAppFreeEnd );
#endif
}


//--------------------------------------------------------------------------------------------
/**
 * 便利ボタン使用関数：きのみプランター
 *
 * @param wk  便利ボタンワーク
 *
 * @return  TRUE（便利ボタンワークを解放しない）
 */
//--------------------------------------------------------------------------------------------
static BOOL ItemCnv_KinomiPlanter( CONV_WORK * wk )
{
#if 0
  ConvFuncCallSet( wk, ConvCall_KinomiPlanter, CONV_MODE_FUNC );
#endif
  return TRUE;
}

static void * ConvCall_KinomiPlanter( void * fsys )
{
#if 0
  return FieldNutPlanter_SetProc( fsys );
#endif
  return NULL;
}

//============================================================================================
//  024:アンノーンノート
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * アイテム使用関数：アンノーンノート
 *
 * @param iuwk  アイテム使用ワーク
 * @param icwk  チェックワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ItemUse_UnknownNote( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk)
{
#if 0
  FIELDSYS_WORK * fsys;
  FLD_MENU * mwk;

  fsys = FieldEvent_GetFieldSysWork( iuwk->event );
  mwk  = FieldEvent_GetSpecialWork( iuwk->event );

  mwk->app_wk = FieldAnNote_SetProc( fsys );

  FldMenu_AppWaitFuncSet( mwk, FldMenu_SubAppFreeEnd );
#endif
}


//--------------------------------------------------------------------------------------------
/**
 * 便利ボタン使用関数：アンノーンノート
 *
 * @param wk  便利ボタンワーク
 *
 * @return  TRUE（便利ボタンワークを解放しない）
 */
//--------------------------------------------------------------------------------------------
static BOOL ItemCnv_UnknownNote( CONV_WORK * wk )
{
#if 0
  ConvFuncCallSet( wk, ConvCall_UnknownNote, CONV_MODE_FUNC );
#endif
  return TRUE;
}

static void * ConvCall_UnknownNote( void * fsys )
{
#if 0
  return FieldAnNote_SetProc( fsys );
#endif
  return NULL;
}


//============================================================================================
//  026:ダウジングマシン
//============================================================================================
//#include "field_poketch.h"

static BOOL GMEVENT_ItemUseDowsingMachine( GMEVENT * event );

static void ItemUse_DowsingMachine( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk)
{
#if 0
  FIELDSYS_WORK * fsys;
  FLD_MENU * mwk;

  fsys = FieldEvent_GetFieldSysWork( iuwk->event );
  mwk  = FieldEvent_GetSpecialWork( iuwk->event );

  FieldEvent_Cmd_SetMapProc( fsys );
  mwk->app_func = GMEVENT_ItemUseDowsingMachine;
  mwk->app_wk   = NULL;
  mwk->seq      = FLD_MENU_SEQ_BLACK_IN_EVCHG;
#endif

  //  FieldSubLcdControler_ChangeSubSeq( fsys, SUBLCD_MODE_DOWSING_MENU, 7, NULL );
}

static BOOL ItemCnv_DowsingMachine( CONV_WORK * wk )
{
#if 0
  // はい・いいえ処理に切り替えリクエスト
  //  FieldSubLcdControler_ChangeSubSeq( fsys, SUBLCD_MODE_DOWSING_MENU, 3, NULL );
  FieldEvent_Set( wk->fsys, GMEVENT_ItemUseDowsingMachine, NULL );
#endif
  return FALSE;
}

static BOOL GMEVENT_ItemUseDowsingMachine( GMEVENT * event )
{
#if 0
  FIELDSYS_WORK * fsys;
  int * seq;

  fsys = FieldEvent_GetFieldSysWork( event );
  seq  = FieldEvent_GetSeqWork( event );

  switch( *seq ){
  case 0:
    FieldSubLcdControler_ChangeApp( fsys, SUBLCD_MODE_DOWSING_MENU, NULL );
    fsys->sublcd_menu_lock_passive = 1;
    (*seq)++;
    break;

  case 1:
    if( FieldSubLcdControler_GetSequence(fsys) == SUBLCD_SEQ_MAIN ){
      FieldOBJSys_MovePauseAllClear( fsys->fldobjsys );
      return TRUE;
    }
    break;
  }
#endif

  return FALSE;

}


static BOOL ItemCnv_GB_Player( CONV_WORK * wk )
{
#if 0
  BAGMSG_WORK * bmwk = sys_AllocMemory( HEAPID_WORLD, sizeof(BAGMSG_WORK) );
  bmwk->seq = 0;


  {
    MSGDATA_MANAGER * mman;
    mman = MSGMAN_Create( MSGMAN_TYPE_NORMAL, ARC_MSG, NARC_msg_bag_dat, HEAPID_WORLD );

    // メッセージ表示
    if(Snd_GameBoyFlagCheck() == TRUE){
      // 「GBプレイヤーをONにした！」
      bmwk->msg = MSGMAN_AllocString( mman, mes_bag_111 );
    }else{
      // 「GBプレイヤーをOFFにした！」
      bmwk->msg = MSGMAN_AllocString( mman, mes_bag_110 );
    }
    MSGMAN_Delete( mman );
  }
  // ゲームボーイ音切り替え
  Snd_GameBoyItemUse();


  FieldEvent_Set( wk->fsys, GMEVENT_BagMsgItem, bmwk );

#endif

  return FALSE;
}

//============================================================================================
//  028:グラシデアのはな
//============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * アイテム使用関数：グラシデアのはな
 *
 * @param iuwk  アイテム使用ワーク
 * @param icwk  チェックワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ItemUse_Grashidea( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk)
{
#if 0
  FIELDSYS_WORK * fsys;
  FLD_MENU * mwk;
  PLIST_DATA * pld;


  fsys = FieldEvent_GetFieldSysWork( iuwk->event );
  mwk  = FieldEvent_GetSpecialWork( iuwk->event );

  mwk->app_wk = FieldFormChangePokeList_SetProc(fsys, HEAPID_WORLD, ITEM_GURASIDEANOHANA);

  FldMenu_AppWaitFuncSet( mwk, FldMenu_PokeListEnd );
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * 便利ボタン使用関数：グラシデアのはな
 *
 * @param wk  便利ボタンワーク
 *
 * @return  TRUE（便利ボタンワークを解放しない）
 */
//--------------------------------------------------------------------------------------------
static BOOL ItemCnv_Grashidea( CONV_WORK * wk )
{
#if 0
  ConvFuncCallSet( wk, ConvCall_Grashidea, CONV_MODE_FUNC );
#endif
  return TRUE;
}

static void * ConvCall_Grashidea( void * fsys )
{
#if 0
  return FieldFormChangePokeList_SetProc(fsys, HEAPID_WORLD, ITEM_GURASIDEANOHANA);
#else
  return NULL;
#endif
}

//============================================================================================
//  029:バトルレコーダー
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * アイテム使用関数：バトルレコーダー
 *
 * @param iuwk  アイテム使用ワーク
 * @param icwk  チェックワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ItemUse_BattleRec( ITEMUSE_WORK * iuwk, const ITEMCHECK_WORK * icwk)
{
#if 0
  FIELDSYS_WORK * fsys;
  FLD_MENU * mwk;

  fsys = FieldEvent_GetFieldSysWork( iuwk->event );
  mwk  = FieldEvent_GetSpecialWork( iuwk->event );

  BattleRecorder_SetProc( fsys, fsys->savedata );

  mwk->app_wk = NULL;
  FldMenu_AppWaitFuncSet( mwk, FldMenu_BattleRecoderEnd );
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * 便利ボタン使用関数：バトルレコーダー
 *
 * @param wk  便利ボタンワーク
 *
 * @return  TRUE（便利ボタンワークを解放しない）
 */
//--------------------------------------------------------------------------------------------
static BOOL ItemCnv_BattleRec( CONV_WORK * wk )
{
#if 0
  ConvFuncCallSet( wk, ConvCall_BattleRec, CONV_MODE_FUNC );
#endif
  return TRUE;
}

static void * ConvCall_BattleRec( void * fsys )
{
#if 0
  //ビデオの初視聴、削除でセーブが行われるので事前にパラメータセット 2008.05.12(月) matsuda
  Field_SaveParam_BattleRecorder(fsys);

  BattleRecorder_SetProc( fsys, ((FIELDSYS_WORK *)fsys)->savedata );
#endif
  return NULL;
}



//--------------------------------------------------------------------------------------------
/**
 * 画面切り替えアイテム用共通イベント
 *
 * @param event
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static BOOL GMEVENT_ConvButtonCommon( GMEVENT * event )
{
#if 0
  FIELDSYS_WORK * fsys;
  CONV_WORK * wk;

  fsys = FieldEvent_GetFieldSysWork( event );
  wk   = FieldEvent_GetSpecialWork( event );

  switch( wk->seq ){
  case SEQ_FADEOUT_SET: // フェードアウトセット
    FieldOBJSys_MovePauseAll( fsys->fldobjsys );
    FieldFadeWipeSet( FLD_DISP_BRIGHT_BLACKOUT );
    wk->seq = SEQ_FADEOUT_WAIT;
    break;

  case SEQ_FADEOUT_WAIT:  // フェードアウト待ち
    if( WIPE_SYS_EndCheck() ){
      wk->app_wk = wk->call( fsys );
      if(wk->mode == CONV_MODE_FUNC){
        wk->seq = SEQ_FUNC_WAIT;
      }else{
        wk->seq = SEQ_END_WAIT;
      }
    }
    break;

  case SEQ_FUNC_WAIT:   // サブプロセス終了待ち
    if( FieldEvent_Cmd_WaitSubProcEnd( fsys ) ) {
      break;
    }
    //ここはスルー
  case SEQ_END_WAIT:
    if( wk->app_wk != NULL ){
      sys_FreeMemoryEz( wk->app_wk );
    }
    FieldEvent_Cmd_SetMapProc( fsys );
    wk->seq = SEQ_RCV_WAIT;
    break;

  case SEQ_RCV_WAIT:    // フィールドマッププロセス開始待ち
    if( FieldEvent_Cmd_WaitMapProcStart(fsys) ) {
      FieldOBJSys_MovePauseAll( fsys->fldobjsys );
      FieldFadeWipeSet( FLD_DISP_BRIGHT_BLACKIN );
      wk->seq = SEQ_FADEIN_WAIT;
    }
    break;

  case SEQ_FADEIN_WAIT: // フェードイン待ち
    if( WIPE_SYS_EndCheck() ){
      FieldOBJSys_MovePauseAllClear( fsys->fldobjsys );
      sys_FreeMemoryEz( wk );
      return TRUE;
    }
  }
#endif
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * 画面切り替えアイテム用共通イベントセット
 *
 * @param wk    便利ボタンワーク
 * @param call  フェードアウト後に呼ばれる処理
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ConvFuncCallSet( CONV_WORK * wk, CNVFUNC_CALL call,u8 mode)
{
#if 0
  wk->call = call;
  wk->mode = mode;
  FieldEvent_Set( wk->fsys, GMEVENT_ConvButtonCommon, wk );
#endif
}


//=============================================================================================
/**
 * @brief 指定の道具が使えるか許可・不許可情報を格納する(itemmenu.h内のenum定義ITEMCHECK_ENABLE)
 *
 * @param   param   BAG_PARAMのポインタ
 * @param   type    使う道具の種類（itemmenu.h内のenum定義ITEMCHECK_ENABLEから指定）
 * @param   check   0:使えない  1:使える
 *
 * @retval  none
 */
//=============================================================================================
void ITEMUSE_SetItemUseCheck( ITEMCHECK_WORK * icwk, ITEMCHECK_ENABLE type, BOOL check )
{
  GF_ASSERT( type<ITEMCHECK_MAX );

  icwk->item_enable |= (check<<type);
}

