//==============================================================================
/**
 * @file    link_festival_log.h
 * @brief   リンクフェスティバル の開催ログ収集関連
 * @author  iwasawa
 * @date    2011.11.11
 */
//==============================================================================

#pragma once

typedef enum{
  LINKFEST_SCORELOG_SET_FAILED,         ///<ログストア失敗
  LINKFEST_SCORELOG_SET_NEW_ENTRY,      ///<新規
  LINKFEST_SCORELOG_SET_MAP_CHANGE,     ///<上書き&マップチェンジ
  LINKFEST_SCORELOG_SET_MAP_CHANGE_SUP, ///<上書き&マップチェンジ&スコアアップ
  LINKFEST_SCORELOG_SET_UPDATE,         ///<上書き&マップチェンジ無し
  LINKFEST_SCORELOG_SET_UPDATE_SUP,     ///<上書き&マップチェンジ無し&スコアアップ
}LINKFEST_SCORELOG_SET_CODE;

//フェスログデータ構造体
typedef struct _LINKFEST_LOG{
  union{  //8byteのログ領域
    struct{ //スコアログ
      u16 tr_id;      ///<トレーナーID
      u16 tmap_root;  ///<現在地のゾーン
      u32 value;      ///<スコア
    }score;

    struct{ //受信ミッションログ
      FEST_BEACON_ID; //<ビーコンIdentityパラメータ(メンバを直接参照したいので敢えて名無し)
      u32 key;    ///<ミッションマジックキー
    }mission;
  };
}LINKFEST_LOG;

//フェスログ受け渡し構造体
typedef struct _LINKFEST_LOG_PARAM{
  FEST_MISSION  m_data;  ///<ミッションデータ
  LINKFEST_LOG  prm;
}LINKFEST_LOG_PARAM;

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ログ構造体クリア
 */
//---------------------------------------------------------
extern void LINKFEST_LOG_Clear( LINKFEST_LOG* log );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ログパラメータ構造体クリア
 */
//---------------------------------------------------------
extern void LINKFEST_LOG_PARAM_Clear( LINKFEST_LOG_PARAM* prm );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ログストア
 *
 * @param log 情報を格納したLINKFEST_LOG_PARAM構造体型ワークへのポインタ
 */
//---------------------------------------------------------
extern LINKFEST_SCORELOG_SET_CODE LINK_FESTIVAL_ScoreLogStore( LINK_FESTIVAL* wk, LINKFEST_LOG_PARAM* src, int* log_score );

//以下デバッグルーチン
#ifdef PM_DEBUG
//---------------------------------------------------------
/*
 * @brief リンクフェスティバル 参加人数セット
 */
//---------------------------------------------------------
extern void DEB_LINKFEST_SetEntryLogNum( LINK_FESTIVAL* wk, int value );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル 参加人数ゲット
 */
//---------------------------------------------------------
extern int DEB_LINKFEST_GetEntryLogNum( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル 参加メンバーパラメータゲット
 */
//---------------------------------------------------------
extern LINKFEST_LOG* DEB_LINKFEST_GetEntryLogParam( LINK_FESTIVAL* wk, int idx );

#endif  //PM_DEBUG

