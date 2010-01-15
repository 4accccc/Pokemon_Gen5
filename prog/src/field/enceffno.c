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

//--------------------------------------------------------------
/**
 * 野生戦エフェクト抽選（釣りを除く）
 * @param inMonsNo    モンスターナンバー
 * @param fieldmap    フィールドマップポインタ  
 * @retval int        エンカウントエフェクトナンバー
 */
//--------------------------------------------------------------
int ENCEFFNO_GetWildEncEffNo( const int inMonsNo, FIELDMAP_WORK *fieldmap )
{
  int no;
  //特定のモンスターかを調べる
  switch (inMonsNo){
  //パッケージ
  case MONSNO_SIN:
  case MONSNO_MU:
  case MONSNO_RAI:
    return ENCEFFID_PACKAGE_POKE;
  //三銃士  
  case MONSNO_PORUTOSU:
  case MONSNO_ARAMISU:
  case MONSNO_DARUTANISU:
    return ENCEFFID_THREE_POKE;
  //移動ポケとか
  case MONSNO_KAZAKAMI:
  case MONSNO_RAIKAMI:
  case MONSNO_TUTINOKAMI:
    return ENCEFFID_MOVE_POKE;
  }

  //自機のいる場所のアトリビュートを取得
  {
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
      no = ENCEFFID_WILD_WATER;
    }
    else if ( MAPATTR_VALUE_CheckEncountGrassA( val ) ) //弱草
    {
      no = ENCEFFID_WILD_NORMAL;
    }
    else if ( MAPATTR_VALUE_CheckEncountGrassB( val ) ) //強草
    {
      no = ENCEFFID_WILD_HEIGH;
    }
    else if ( MAPATTR_VALUE_CheckMarsh( val ) )         //浅沼
    {
      no = ENCEFFID_WILD_NORMAL;
    }
    else if ( MAPATTR_VALUE_CheckDesertDeep( val ) )    //砂
    {
      no = ENCEFFID_WILD_DESERT;
    }
    else  //その他　屋内とみなす
    {
      no = ENCEFFID_WILD_INNER;
    }
  }

  return no;

}

//--------------------------------------------------------------
/**
 * トレーナー戦エフェクト抽選(サブウェイを除く)
 * @param inTrType      トレーナーID
 * @param fieldmap    フィールドマップポインタ  
 * @retval int        エンカウントエフェクトナンバー
 */
//--------------------------------------------------------------
int ENCEFFNO_GetTrEncEffNo( const int inTrID, FIELDMAP_WORK *fieldmap )
{
  int no;
  int tr_type;
  tr_type = TT_TrainerDataParaGet( inTrID, ID_TD_tr_type );

  switch(tr_type){
  //ジムリーダー
  case TRTYPE_LEADER1A:
    no = ENCEFFID_LEADER1A;
    return no;
  case TRTYPE_LEADER1B:
    no = ENCEFFID_LEADER1B;
    return no;
  case TRTYPE_LEADER1C:
    no = ENCEFFID_LEADER1C;
    return no;
  case TRTYPE_LEADER2:
    no = ENCEFFID_LEADER2;
    return no;
  case TRTYPE_LEADER3:
    no = ENCEFFID_LEADER3;
    return no;
  case TRTYPE_LEADER4:
    no = ENCEFFID_LEADER4;
    return no;
  case TRTYPE_LEADER5:
    no = ENCEFFID_LEADER5;
    return no;
  case TRTYPE_LEADER6:
    no = ENCEFFID_LEADER6;
    return no;
  case TRTYPE_LEADER7:
    no = ENCEFFID_LEADER7;
    return no;
  case TRTYPE_LEADER8A:
    no = ENCEFFID_LEADER8A;
    return no;
  case TRTYPE_LEADER8B:
    no = ENCEFFID_LEADER8B;
    return no;
  //四天王
  case TRTYPE_BIGFOUR1:
    no = ENCEFFID_BIGFOUR1;
    return no;
  case TRTYPE_BIGFOUR2:
    no = ENCEFFID_BIGFOUR2;
    return no;
  case TRTYPE_BIGFOUR3:
    no = ENCEFFID_BIGFOUR3;
    return no;
  case TRTYPE_BIGFOUR4:
    no = ENCEFFID_BIGFOUR4;
    return no;
  //ライバル
  case TRTYPE_RIVAL:
    no = ENCEFFID_RIVAL;
    return no;
  //サポーター
  case TRTYPE_SUPPORT:
    no = ENCEFFID_SUPPORT;
    return no;
  //プラズマ団
  case TRTYPE_HAKAIM1:
  case TRTYPE_HAKAIW1:
    no = ENCEFFID_PRAZUMA;
    return no;
  //Ｎ
  case TRTYPE_BOSS:
    no = ENCEFFID_BOSS;
    return no;
  //ゲーツィス
  case TRTYPE_SAGE1:
    no = ENCEFFID_SAGE;
    return no;
  //チャンプ
/**
  case TRTYPE_CHAMP:
    no = ENCEFFID_CHAMP;
    return no;
*/    
  }

  //それ以外
  {
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
      no = ENCEFFID_TR_WATER;
    }
    else if ( MAPATTR_VALUE_CheckDesert( val ) )        //砂漠アトリビュート
    {
      no = ENCEFFID_TR_DESERT;
    }
    else{
      AREADATA* areadata = FIELDMAP_GetAreaData( fieldmap );
      BOOL outdoor = ( AREADATA_GetInnerOuterSwitch(areadata) != 0 );
      if ( outdoor )  //屋外
      {
        no = ENCEFFID_TR_NORMAL;
      }else{          //屋内
        no = ENCEFFID_TR_INNER;
      }
    }
  }

  return no;
}



