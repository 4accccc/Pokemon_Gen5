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
    *outEffNo = ENCEFFID_WILD_HEIGH;
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
    else if ( MAPATTR_VALUE_CheckEncountGrassLow( val ) ) //弱草
    {
      bgm = SEQ_BGM_VS_NORAPOKE;
      eff = ENCEFFID_WILD_NORMAL;
    }
    else if ( MAPATTR_VALUE_CheckEncountGrassHigh( val ) ) //強草
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
  int tr_type = TT_TrainerDataParaGet( inTrID, ID_TD_tr_type );
  u8  type_grp = TT_TrainerTypeGrpGet( tr_type );
  u8  grp_idx = TT_TrainerTypeGrpEntryIdxGet( tr_type );

  static const u16 DATA_VsBgmTbl[] = {
		SEQ_BGM_VS_RIVAL,   	///<TRTYPE_GRP_RIVAL ライバル
		SEQ_BGM_VS_RIVAL,   	///<TRTYPE_GRP_SUPPORT サポート
		SEQ_BGM_VS_GYMLEADER,	///<TRTYPE_GRP_LEADER ジムリーダー
		SEQ_BGM_VS_SHITENNO,	///<TRTYPE_GRP_BIGFOUR 四天王
		SEQ_BGM_VS_CHAMP,	    ///<TRTYPE_GRP_CHAMPION チャンピオン
		SEQ_BGM_VS_N,	        ///<TRTYPE_GRP_BOSS N
		SEQ_BGM_VS_G_CIS,	    ///<TRTYPE_GRP_SAGE ゲーチス
		SEQ_BGM_VS_PLASMA,	  ///<TRTYPE_GRP_PLASMA プラズマ団
  };
  static const u8 DATA_EncEffTbl[] = {
		ENCEFFID_RIVAL,	    ///<TRTYPE_GRP_IDX_RIVAL
		ENCEFFID_SUPPORT,	  ///<TRTYPE_GRP_IDX_SUPPORT
		ENCEFFID_LEADER1A,	///<TRTYPE_GRP_IDX_LEADER1A
		ENCEFFID_LEADER1B,	///<TRTYPE_GRP_IDX_LEADER1B
		ENCEFFID_LEADER1C,	///<TRTYPE_GRP_IDX_LEADER1C
		ENCEFFID_LEADER2,	  ///<TRTYPE_GRP_IDX_LEADER2
		ENCEFFID_LEADER3,	  ///<TRTYPE_GRP_IDX_LEADER3
		ENCEFFID_LEADER4,	  ///<TRTYPE_GRP_IDX_LEADER4
		ENCEFFID_LEADER5,	  ///<TRTYPE_GRP_IDX_LEADER5
		ENCEFFID_LEADER6,	  ///<TRTYPE_GRP_IDX_LEADER6
		ENCEFFID_LEADER7,	  ///<TRTYPE_GRP_IDX_LEADER7
		ENCEFFID_LEADER8A,	///<TRTYPE_GRP_IDX_LEADER8A
		ENCEFFID_LEADER8B,	///<TRTYPE_GRP_IDX_LEADER8B
		ENCEFFID_BIGFOUR1,	///<TRTYPE_GRP_IDX_BIGFOUR1
		ENCEFFID_BIGFOUR2,	///<TRTYPE_GRP_IDX_BIGFOUR2
		ENCEFFID_BIGFOUR3,	///<TRTYPE_GRP_IDX_BIGFOUR3
		ENCEFFID_BIGFOUR4,	///<TRTYPE_GRP_IDX_BIGFOUR4
		ENCEFFID_CHAMP,	    ///<TRTYPE_GRP_IDX_CHAMPION
		ENCEFFID_BOSS,	    ///<TRTYPE_GRP_IDX_BOSS
		ENCEFFID_SAGE,	    ///<TRTYPE_GRP_IDX_SAGE
		ENCEFFID_PRAZUMA,	  ///<TRTYPE_GRP_IDX_HAKAIM1
		ENCEFFID_PRAZUMA,	  ///<TRTYPE_GRP_IDX_HAKAIW1
  };

  if( type_grp < TRTYPE_GRP_BCHAMP ){
    *outBgmNo = DATA_VsBgmTbl[type_grp];
    *outEffNo = DATA_EncEffTbl[grp_idx];
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



