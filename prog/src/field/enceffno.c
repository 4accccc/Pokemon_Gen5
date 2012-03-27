//======================================================================
/**
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
#include "sound/swan_sound_data.sadl" //サウンドラベルファイル

#include "field/zonedata.h"
#include "arc/fieldmap/zone_id.h"

#define ZONE_GRP_CHAMPION_LOAD  (ZONE_ID_D09)

//--------------------------------------------------------------
/**
 * 野生戦エフェクト抽選（釣りを除く）
 * @param inMonsNo        モンスターナンバー
 * @param inFormNo        フォルムナンバー
 * @param inEncType       エンカウントタイプ　encount_data.h参照
 * @param inDoubleBattle  ダブルバトルならTRUE
 * @param fieldmap    フィールドマップポインタ
 * @param   outEffNo    エンカウントエフェクトナンバー
 * @param   outBgmNo    BGMナンバー
 * @retval none        
 */
//--------------------------------------------------------------
void ENCEFFNO_GetWildEncEffNoBgmNo( const int inMonsNo, const int inFormNo, ENCOUNT_TYPE inEncType, BOOL inDoubleBattle, 
    FIELDMAP_WORK *fieldmap,int *outEffNo, u16 *outBgmNo )
{
  u16 zone_id = FIELDMAP_GetZoneID( fieldmap );
  u16 zone_group = ZONEDATA_GetGroupID( zone_id );

  //特定のモンスターかを調べる
  switch (inMonsNo){
  case MONSNO_530: //ゾロアーク MONSNO_ZOROAAKU
    *outBgmNo = SEQ_BGM_VS_TSUYOPOKE;
    *outEffNo = ENCEFFID_ZOROARK;
    return;
  //パッケージ
  case MONSNO_650:  //シン MONSNO_SIN
    *outBgmNo = SEQ_BGM_VS_SHIN;
    *outEffNo = ENCEFFID_PACKAGE_POKE;
    return;
  case MONSNO_651: //ム MONSNO_MU
    *outBgmNo = SEQ_BGM_VS_MU;
    *outEffNo = ENCEFFID_PACKAGE_POKE;
    return;
  case MONSNO_653:  //キュレム MONSNO_KYUREMU
    if( inFormNo == FORMNO_653_WHITE )  // キュラム FORMNO_KYUREMU_WHITE
    {
      *outBgmNo = SEQ_BGM_VS_KYURAMU;
    }
    else if( inFormNo == FORMNO_653_BLACK ) // キュロム FORMNO_KYUREMU_BLACK
    {
      *outBgmNo = SEQ_BGM_VS_KYUROMU;
    }
    else
    {
      *outBgmNo = SEQ_BGM_VS_RAI;
    }
    *outEffNo = ENCEFFID_PACKAGE_POKE;
    return;
  case MONSNO_644:  //ウルガモス MONSNO_URUGAMOSU
    *outBgmNo = SEQ_BGM_VS_TSUYOPOKE;
    *outEffNo = ENCEFFID_EFF_POKE;
    return;
  case MONSNO_657:  //ビクティ MONSNO_BIKUTHI
    *outBgmNo = SEQ_BGM_VS_SETPOKE;
    *outEffNo = ENCEFFID_EFF_POKE;
    return;
  //三銃士  
  case MONSNO_645:  //アトス MONSNO_ATOSU
  case MONSNO_646:  //ポルトス MONSNO_PORUTOSU
  case MONSNO_647:  //アラミス MONSNO_ARAMISU
  case MONSNO_654:  //ダルタニス MONSNO_DARUTANISU
    *outBgmNo = SEQ_BGM_VS_SETPOKE;
    *outEffNo = ENCEFFID_THREE_POKE;
    return;
  //移動ポケとか
  case MONSNO_648:  //カザカミ MONSNO_KAZAKAMI:
  case MONSNO_649:  //ライカミ MONSNO_RAIKAMI:
  case MONSNO_652:  //ツチノカミ MONSNO_TUTINOKAMI:
    *outBgmNo = SEQ_BGM_VS_MOVEPOKE;
    *outEffNo = ENCEFFID_MOVE_POKE;
    return;
  case MONSNO_380:  //ラティアス MONSNO_RATHIASU
  case MONSNO_381:  //ラティオス MONSNO_RATHIOSU
  case MONSNO_488:  //クレセリア MONSNO_KURESERIA
  case MONSNO_509:  //オノノクス  MONSNO_ONONOKUSU
    *outBgmNo = SEQ_BGM_VS_TSUYOPOKE;
    *outEffNo = ENCEFFID_EFF_POKE;
    return;
  // レジ系
  case MONSNO_377:  //レジロック MONSNO_REZIROKKU
  case MONSNO_378:  //レジアイス MONSNO_REZIAISU
  case MONSNO_379:  //レジスチル MONSNO_REZISUTIRU
  case MONSNO_486:  //レジギガス MONSNO_REZIGIGASU
    *outBgmNo = SEQ_BGM_VS_REGI;
    *outEffNo = ENCEFFID_EFF_POKE;
    return;
  // UMA
  case MONSNO_480:  //ユクシー    MONSNO_YUKUSII
  case MONSNO_481:  //エムリット  MONSNO_EMURITTO
  case MONSNO_482:  //アグノム    MONSNO_AGUNOMU
    *outBgmNo = SEQ_BGM_VS_UMA;
    *outEffNo = ENCEFFID_EFF_POKE;
    return;
  case MONSNO_485:  //ヒードラン  MONSNO_HIIDORAN
    *outBgmNo = SEQ_BGM_VS_DPLEGEND;
    *outEffNo = ENCEFFID_EFF_POKE;
    return;
  }

  //エフェクトエンカウントチェック
  if ( inEncType == ENC_TYPE_EFFECT )
  {
    *outBgmNo = SEQ_BGM_VS_TSUYOPOKE;
    *outEffNo = ENCEFFID_EFF_POKE;
    return;
  }
  //釣りチェック
  if ( inEncType == ENC_TYPE_FISHING )
  {
    //チャンピオンロードは固定曲 10.05.15
    if( zone_group == ZONE_GRP_CHAMPION_LOAD ){
      *outBgmNo = SEQ_BGM_VS_TSUYOPOKE;
    }else{
      *outBgmNo = SEQ_BGM_VS_NORAPOKE;
    }
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
    else if ( MAPATTR_VALUE_CheckEncountGrass( val ) )
    {
      if( inDoubleBattle ){
        bgm = SEQ_BGM_VS_TSUYOPOKE;
        eff = ENCEFFID_WILD_HEIGH;
      }else{
        bgm = SEQ_BGM_VS_NORAPOKE;
        eff = ENCEFFID_WILD_NORMAL;
      }
    }
    else if ( MAPATTR_VALUE_CheckMarsh( val ) )         //浅沼
    {
      bgm = SEQ_BGM_VS_NORAPOKE;
      eff = ENCEFFID_WILD_NORMAL;
    }
    else if ( MAPATTR_VALUE_CheckDesert( val ) )        //砂漠アトリビュート
    {
      bgm = SEQ_BGM_VS_NORAPOKE;
      eff = ENCEFFID_WILD_DESERT;
    }
    else if( ZONEDATA_IsAnimalTrail( zone_id ) )        // 獣道
    {
        bgm = SEQ_BGM_VS_NORAPOKE;
        eff = ENCEFFID_WILD_NORMAL;
    }
    else  //その他　屋内とみなす
    {
      bgm = SEQ_BGM_VS_NORAPOKE;
      eff = ENCEFFID_WILD_INNER;
    }
  
    *outEffNo = eff;

    //チャンピオンロードは固定曲 10.05.15
    if( zone_group == ZONE_GRP_CHAMPION_LOAD ){
      *outBgmNo = SEQ_BGM_VS_TSUYOPOKE;
    }else{
      *outBgmNo = bgm;
    }
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
		SEQ_BGM_VS_GYMLEADER,	///<TRTYPE_GRP_LEADER ジムリーダー
		SEQ_BGM_VS_SHITENNO,	///<TRTYPE_GRP_BIGFOUR 四天王
		SEQ_BGM_VS_SHIRONA,	  ///<TRTYPE_GRP_DPCHAMP シロナ(DPチャンピオン)
    // swan
    SEQ_BGM_VS_HUE,       ///<TRTYPE_GRP_RIVAL ライバル
    SEQ_BGM_VS_TRAINER,   ///<TRTYPE_GRP_TRAINER_SWBOSS ポケモントレーナーのアクロマ
    SEQ_BGM_VS_ACHROMA,   ///<TRTYPE_GRP_PLASMA_SWBOSS プラズマ団のアクロマ
    SEQ_BGM_VS_SWAN_N,    ///<TRTYPE_GRP_N ポケモントレーナーのN
    SEQ_BGM_VS_NEO_PLASMA,///<TRTYPE_GRP_PLASMA プラズマ団(ネオ)
    SEQ_BGM_VS_IRIS,      ///<TRTYPE_GRP_CHAMPION チャンピオン
    SEQ_BGM_VS_NEW_G_CIS, ///<TRTYPE_GRP_SAGE1  ゲーチス
    SEQ_BGM_VS_ELITE_PLASMA, ///<TRTYPE_GRP_SAGE3  ヴィオ
    SEQ_BGM_VS_BANJIROU,  ///<TRTYPE_GRP_INFINITY 無限ダンジョンボス
    SEQ_BGM_VS_ACHROMA,   ///<TRTYPE_GRP_CLEAR_SWBOSS クリア後のアクロマ
    SEQ_BGM_VS_RIVAL,     ///<TRTYPE_GRP_RIVAL2 ライバル2
               
  };
  static const u8 DATA_EncEffTbl[] = {
		ENCEFFID_LEADER1,	    ///<TRTYPE_GRP_IDX_SWLEADER1
		ENCEFFID_LEADER2,	    ///<TRTYPE_GRP_IDX_SWLEADER2
		ENCEFFID_LEADER3,	    ///<TRTYPE_GRP_IDX_SWLEADER3
		ENCEFFID_LEADER4,	    ///<TRTYPE_GRP_IDX_SWLEADER4
		ENCEFFID_LEADER5,	    ///<TRTYPE_GRP_IDX_SWLEADER5
		ENCEFFID_LEADER6,	    ///<TRTYPE_GRP_IDX_SWLEADER6
		ENCEFFID_LEADER7,	    ///<TRTYPE_GRP_IDX_SWLEADER7
		ENCEFFID_LEADER8,	    ///<TRTYPE_GRP_IDX_SWLEADER8
		ENCEFFID_BIGFOUR1,	///<TRTYPE_GRP_IDX_BIGFOUR1
		ENCEFFID_BIGFOUR2,	///<TRTYPE_GRP_IDX_BIGFOUR2
		ENCEFFID_BIGFOUR3,	///<TRTYPE_GRP_IDX_BIGFOUR3
		ENCEFFID_BIGFOUR4,	///<TRTYPE_GRP_IDX_BIGFOUR4
		ENCEFFID_TR_NORMAL, ///<TRTYPE_GRP_IDX_DPCHAMP
    // swan
		ENCEFFID_RIVAL,       ///<TRTYPE_GRP_IDX_SWRIVAL
		ENCEFFID_TR_NORMAL,   ///<TRTYPE_GRP_IDX_TRAINER_SWBOSS
		ENCEFFID_SWBOSS,      ///<TRTYPE_GRP_IDX_PLASMA_SWBOSS  
		ENCEFFID_N,           ///<TRTYPE_GRP_IDX_BOSS
		ENCEFFID_NEO_PLASMA,  ///<TRTYPE_GRP_IDX_SWPLAZMAM
		ENCEFFID_NEO_PLASMA,  ///<TRTYPE_GRP_IDX_SWPLAZMAM
		ENCEFFID_NEO_PLASMA,  ///<TRTYPE_GRP_IDX_SWSAGE3
		ENCEFFID_NEO_PLASMA,  ///<TRTYPE_GRP_IDX_NINJA      
		ENCEFFID_CHAMP,       ///<TRTYPE_GRP_IDX_SWCHAMPION       
		ENCEFFID_SAGE,        ///<TRTYPE_GRP_IDX_SWSAGE1           
		ENCEFFID_MUGEN_BOSS,  ///<TRTYPE_GRP_IDX_MUGENBOSS             
		ENCEFFID_SWBOSS2,     ///<TRTYPE_GRP_IDX_CLEAR_SWBOSS  
		ENCEFFID_TR_NORMAL,   ///<TRTYPE_GRP_IDX_SWLEADER1_1  
		ENCEFFID_TR_NORMAL,   ///<TRTYPE_GRP_IDX_SUPPORT  
    // ↓は参照されないが一応入れておく
		ENCEFFID_SUBWAY,      ///<TRTYPE_BCHAMP  
		ENCEFFID_SUBWAY,      ///<TRTYPE_BCHAMP2
  };

  // BCHAMPは個別で設定しているため
  if( type_grp < TRTYPE_GRP_BCHAMP ){
    GF_ASSERT( type_grp < TRTYPE_GRP_MAX );
    GF_ASSERT( grp_idx  < TRTYPE_GRP_IDX_MAX );
    *outBgmNo = DATA_VsBgmTbl[type_grp];
    *outEffNo = DATA_EncEffTbl[grp_idx];
    ICHI_Printf("trtype_grp %d grp_idx %d \n", type_grp, grp_idx );
    OS_Printf("ENCFFID %d\n", *outEffNo);
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



