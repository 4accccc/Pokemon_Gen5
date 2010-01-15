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
    return 0;
  //三銃士  
  case MONSNO_PORUTOSU:
  case MONSNO_ARAMISU:
  case MONSNO_DARUTANISU:
    return 0;
  //移動ポケとか
  case MONSNO_KAZAKAMI:
  case MONSNO_RAIKAMI:
  case MONSNO_TUTINOKAMI:
    return 0;
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
      no = 0;
    }
    else if ( MAPATTR_VALUE_CheckEncountGrassA( val ) ) //弱草
    {
      no = 0;
    }
    else if ( MAPATTR_VALUE_CheckEncountGrassB( val ) ) //強草
    {
      no = 0;
    }
    else if ( MAPATTR_VALUE_CheckMarsh( val ) )         //浅沼
    {
      no = 0;
    }
    else if ( MAPATTR_VALUE_CheckDesertDeep( val ) )    //砂
    {
      no = 0;
    }
    else  //その他
    {
      no = 0;
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
    no = 0;
    return no;
  case TRTYPE_LEADER1B:
    no = 0;
    return no;
  case TRTYPE_LEADER1C:
    no = 0;
    return no;
  case TRTYPE_LEADER2:
    no = 0;
    return no;
  case TRTYPE_LEADER3:
    no = 0;
    return no;
  case TRTYPE_LEADER4:
    no = 0;
    return no;
  case TRTYPE_LEADER5:
    no = 0;
    return no;
  case TRTYPE_LEADER6:
    no = 0;
    return no;
  case TRTYPE_LEADER7:
    no = 0;
    return no;
  case TRTYPE_LEADER8A:
    no = 0;
    return no;
  case TRTYPE_LEADER8B:
    no = 0;
    return no;
  //四天王
  case TRTYPE_BIGFOUR1:
    no = 0;
    return no;
  case TRTYPE_BIGFOUR2:
    no = 0;
    return no;
  case TRTYPE_BIGFOUR3:
    no = 0;
    return no;
  case TRTYPE_BIGFOUR4:
    no = 0;
    return no;
  //ライバル・サポーター
  case TRTYPE_RIVAL:
  case TRTYPE_SUPPORT:
    no = 0;
    return no;
  //プラズマ団
  case TRTYPE_HAKAIM1:
  case TRTYPE_HAKAIW1:
    no = 0;
    return no;
  //Ｎ
  case TRTYPE_BOSS:
    no = 0;
    return no;
  //ゲーツィス
  case TRTYPE_SAGE1:
    no = 0;
    return no;
  //チャンプ
  ;
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
      no = 0;
    }
    else if ( MAPATTR_VALUE_CheckDesert( val ) )        //砂漠アトリビュート
    {
      no = 0;
    }
    else{
      AREADATA* areadata = FIELDMAP_GetAreaData( fieldmap );
      BOOL outdoor = ( AREADATA_GetInnerOuterSwitch(areadata) != 0 );
      if ( outdoor )  //屋外
      {
        no = 0;
      }else{          //屋内
        no = 0;
      }
    }
  }

  return no;
}



