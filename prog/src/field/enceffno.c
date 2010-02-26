//======================================================================
/*
 * @file	enceffno.c
 * @brief	エンカウントエフェクト抽選
 * @author saito
 */
//======================================================================
#include "field/fieldmap.h"
#include "enceffno.h"

#include "tr_tool/trtype_def.h"
#include "tr_tool/tr_tool.h"

#include "enceffno_def.h"
#include "sound/wb_sound_data.sadl" //サウンドラベルファイル

//--------------------------------------------------------------
/**
 * 野生戦エフェクト抽選（釣りを除く）
 * @param inMonsNo    モンスターナンバー
 * @param inEncType   エンカウントタイプ　encount_data.h参照
 * @param fieldmap    フィールドマップポインタ
 * @param   outEffNo    エンカウントエフェクトナンバー
 * @param   outBgmNo    BGMナンバー
 * @retval none        
 */
//--------------------------------------------------------------
void ENCEFFNO_GetWildEncEffNoBgmNo( const int inMonsNo, ENCOUNT_TYPE inEncType, FIELDMAP_WORK *fieldmap,
                                    int *outEffNo, u16 *outBgmNo )
{
  //特定のモンスターかを調べる
  switch (inMonsNo){
  case MONSNO_ZOROAAKU:
    *outEffNo = ENCEFFID_PACKAGE_POKE;    //@todo
    *outBgmNo = SEQ_BGM_VS_TSUYOPOKE;
    return;
  //パッケージ
  case MONSNO_SIN:
    *outBgmNo = SEQ_BGM_VS_SHIN;
    *outEffNo = ENCEFFID_PACKAGE_POKE;
    return;
  case MONSNO_MU:
    *outBgmNo = SEQ_BGM_VS_MU;
    *outEffNo = ENCEFFID_PACKAGE_POKE;
    return;
  case MONSNO_RAI:
    *outBgmNo = SEQ_BGM_VS_RAI;
    *outEffNo = ENCEFFID_PACKAGE_POKE;
    return;
  //三銃士  
  case MONSNO_PORUTOSU:
  case MONSNO_ARAMISU:
  case MONSNO_DARUTANISU:
    *outBgmNo = SEQ_BGM_VS_SETPOKE;
    *outEffNo = ENCEFFID_THREE_POKE;
    return;
  //移動ポケとか
  case MONSNO_KAZAKAMI:
  case MONSNO_RAIKAMI:
  case MONSNO_TUTINOKAMI:
    *outBgmNo = SEQ_BGM_VS_MOVEPOKE;
    *outEffNo = ENCEFFID_MOVE_POKE;
    return;
  }

  //釣りチェック
  if ( inEncType == ENC_TYPE_FISHING )
  {
    *outBgmNo = SEQ_BGM_VS_NORAPOKE;
    *outEffNo = ENCEFFID_WILD_WATER;
    return;

  }

  //自機のいる場所のアトリビュートを取得
  {
    int eff;
    u16 bgm;
    MAPATTR_VALUE val;
    MAPATTR attr;
    MAPATTR_FLAG attr_flag;
    FIELD_PLAYER *player = FIELDMAP_GetFieldPlayer( fieldmap );
    attr = FIELD_PLAYER_GetMapAttr( player );
    val = MAPATTR_GetAttrValue(attr);
    attr_flag = MAPATTR_GetAttrFlag( attr );
    //取得したアトリビュートで分岐
    if( attr_flag & MAPATTR_FLAGBIT_WATER )             //波乗りアトリビュート
    {
      bgm = SEQ_BGM_VS_NORAPOKE;
      eff = ENCEFFID_WILD_WATER;
    }
    else if ( MAPATTR_VALUE_CheckEncountGrassA( val ) ) //弱草
    {
      bgm = SEQ_BGM_VS_NORAPOKE;
      eff = ENCEFFID_WILD_NORMAL;
    }
    else if ( MAPATTR_VALUE_CheckEncountGrassB( val ) ) //強草
    {
      bgm = SEQ_BGM_VS_TSUYOPOKE;
      eff = ENCEFFID_WILD_HEIGH;
    }
    else if ( MAPATTR_VALUE_CheckMarsh( val ) )         //浅沼
    {
      bgm = SEQ_BGM_VS_NORAPOKE;
      eff = ENCEFFID_WILD_NORMAL;
    }
    else if ( MAPATTR_VALUE_CheckDesertDeep( val ) )    //砂
    {
      bgm = SEQ_BGM_VS_NORAPOKE;
      eff = ENCEFFID_WILD_DESERT;
    }
    else  //その他　屋内とみなす
    {
      bgm = SEQ_BGM_VS_NORAPOKE;
      eff = ENCEFFID_WILD_INNER;
    }
  
    *outEffNo = eff;
    *outBgmNo = bgm;
  }

  return;
}

//--------------------------------------------------------------
/**
 * トレーナー戦エフェクト抽選(サブウェイを除く)
 * @param inTrType      トレーナーID
 * @param fieldmap    フィールドマップポインタ  
 * @param   outEffNo    エンカウントエフェクトナンバー
 * @param   outBgmNo    BGMナンバー
 * @retval none
 */
//--------------------------------------------------------------
void ENCEFFNO_GetTrEncEffNoBgmNo( const int inTrID, FIELDMAP_WORK *fieldmap,
                                  int *outEffNo, u16 *outBgmNo )
{
  int tr_type;
  tr_type = TT_TrainerDataParaGet( inTrID, ID_TD_tr_type );

  switch(tr_type){
  //ジムリーダー
  case TRTYPE_LEADER1A:
    *outBgmNo = SEQ_BGM_VS_GYMLEADER;
    *outEffNo = ENCEFFID_LEADER1A;
    return;
  case TRTYPE_LEADER1B:
    *outBgmNo = SEQ_BGM_VS_GYMLEADER;
    *outEffNo = ENCEFFID_LEADER1B;
    return;
  case TRTYPE_LEADER1C:
    *outBgmNo = SEQ_BGM_VS_GYMLEADER;
    *outEffNo = ENCEFFID_LEADER1C;
    return;
  case TRTYPE_LEADER2:
    *outBgmNo = SEQ_BGM_VS_GYMLEADER;
    *outEffNo = ENCEFFID_LEADER2;
    return;
  case TRTYPE_LEADER3:
    *outBgmNo = SEQ_BGM_VS_GYMLEADER;
    *outEffNo = ENCEFFID_LEADER3;
    return;
  case TRTYPE_LEADER4:
    *outBgmNo = SEQ_BGM_VS_GYMLEADER;
    *outEffNo = ENCEFFID_LEADER4;
    return;
  case TRTYPE_LEADER5:
    *outBgmNo = SEQ_BGM_VS_GYMLEADER;
    *outEffNo = ENCEFFID_LEADER5;
    return;
  case TRTYPE_LEADER6:
    *outBgmNo = SEQ_BGM_VS_GYMLEADER;
    *outEffNo = ENCEFFID_LEADER6;
    return;
  case TRTYPE_LEADER7:
     *outBgmNo = SEQ_BGM_VS_GYMLEADER;
     *outEffNo = ENCEFFID_LEADER7;
    return;
  case TRTYPE_LEADER8A:
    *outBgmNo = SEQ_BGM_VS_GYMLEADER;
    *outEffNo = ENCEFFID_LEADER8A;
    return;
  case TRTYPE_LEADER8B:
    *outBgmNo = SEQ_BGM_VS_GYMLEADER;    
    *outEffNo = ENCEFFID_LEADER8B;
    return;
  //四天王
  case TRTYPE_BIGFOUR1:
    *outBgmNo = SEQ_BGM_VS_SHITENNO;
    *outEffNo = ENCEFFID_BIGFOUR1;
    return;
  case TRTYPE_BIGFOUR2:
    *outBgmNo = SEQ_BGM_VS_SHITENNO;
    *outEffNo = ENCEFFID_BIGFOUR2;
    return;
  case TRTYPE_BIGFOUR3:
    *outBgmNo = SEQ_BGM_VS_SHITENNO;
    *outEffNo = ENCEFFID_BIGFOUR3;
    return;
  case TRTYPE_BIGFOUR4:
    *outBgmNo = SEQ_BGM_VS_SHITENNO;
    *outEffNo = ENCEFFID_BIGFOUR4;
    return;
  //ライバル
  case TRTYPE_RIVAL:
    *outBgmNo = SEQ_BGM_VS_RIVAL;
    *outEffNo = ENCEFFID_RIVAL;
    return;
  //サポーター
  case TRTYPE_SUPPORT:
    *outBgmNo = SEQ_BGM_VS_RIVAL;
    *outEffNo = ENCEFFID_SUPPORT;
    return;
  //プラズマ団
  case TRTYPE_HAKAIM1:
  case TRTYPE_HAKAIW1:
    *outBgmNo = SEQ_BGM_VS_PLASMA;
    *outEffNo = ENCEFFID_PRAZUMA;
    return;
  //Ｎ
  case TRTYPE_BOSS:
    *outBgmNo = SEQ_BGM_VS_N;
    *outEffNo = ENCEFFID_BOSS;
    return;
  //ゲーツィス
  case TRTYPE_SAGE1:
    *outBgmNo = SEQ_BGM_VS_G_CIS;
    *outEffNo = ENCEFFID_SAGE;
    return;
  //チャンプ
  case TRTYPE_CHAMPION:
    *outBgmNo = SEQ_BGM_VS_CHAMP;
    *outEffNo = ENCEFFID_CHAMP;
    return;
  }

  //それ以外
  {
    int eff;
    MAPATTR_VALUE val;
    MAPATTR attr;
    MAPATTR_FLAG attr_flag;
    FIELD_PLAYER *player = FIELDMAP_GetFieldPlayer( fieldmap );
    attr = FIELD_PLAYER_GetMapAttr( player );
    val = MAPATTR_GetAttrValue(attr);
    attr_flag = MAPATTR_GetAttrFlag( attr );
    //取得したアトリビュートで分岐
    if( attr_flag & MAPATTR_FLAGBIT_WATER )             //波乗りアトリビュート
    {
      eff = ENCEFFID_TR_WATER;
    }
    else if ( MAPATTR_VALUE_CheckDesert( val ) )        //砂漠アトリビュート
    {
      eff = ENCEFFID_TR_DESERT;
    }
    else{
      AREADATA* areadata = FIELDMAP_GetAreaData( fieldmap );
      BOOL outdoor = ( AREADATA_GetInnerOuterSwitch(areadata) != 0 );
      if ( outdoor )  //屋外
      {
        eff = ENCEFFID_TR_NORMAL;
      }else{          //屋内
        eff = ENCEFFID_TR_INNER;
      }
    }

    *outBgmNo = SEQ_BGM_VS_TRAINER;
    *outEffNo = eff;
  }
  return;
}



