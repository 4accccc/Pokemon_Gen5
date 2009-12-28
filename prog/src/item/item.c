//============================================================================================
/**
 * @file  item.c
 * @brief アイテムデータ処理
 * @author  Hiroyuki Nakamura
 * @date  05.09.06
 */
//============================================================================================
#include <gflib.h>
#include "arc_def.h"
#include "item_data.naix"
#include "item_icon.naix"
#include "agb_itemsym.h"
#include "item/item.h"
#include "waza_tool/wazano_def.h"
#include "itemdata.h"
#include "message.naix"
#include "msg\msg_itemname.h"
#include "msg\msg_iteminfo.h"


//============================================================================================
//  定数定義
//============================================================================================
#define NORMAL_WAZAMACHINE_MAX  ( 101 )   // 通常の技マシン数
#define HIDENWAZA_MAX						( 8 )			// 秘伝技数
#define HIDENWAZA_START_POS			( 92 )		// 秘伝マシン開始位置


//==============================================================================
//  構造体定義
//==============================================================================
typedef struct {
  u16 arc_data;
  u16 arc_cgx;
  u16 arc_pal;
  u16 agb_id;
}ITEMDATA_INDEX;


//============================================================================================
//  データ
//============================================================================================
#include "itemindex.dat"
#include "waza_mcn.dat"
#include "itemtype.dat"


//============================================================================================
//  プロトタイプ宣言
//============================================================================================
static s32 ItemParamRcvGet( ITEMPARAM_RCV * rcv, u16 param );



//--------------------------------------------------------------------------------------------
/**
 * 指定アイテムを指定位置に挿入
 *
 * @param item  アイテムデータ
 * @param pos1  指定アイテム位置
 * @param pos2  挿入位置
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
#if 0
まだ手持ちアイテムのセーブデータまではもってきていないのでコメントアウト
2008.12.01(月) matsuda
void ItemPutIn( void * item, u16 pos1, u16 pos2 )
{
  MINEITEM * data;
  MINEITEM copy;
  s16 i;

  if( pos1 == pos2 ){ return; }

  data = (MINEITEM *)item;
  copy = data[pos1];

  if( pos2 > pos1 ){
    pos2 -= 1;
    for( i=(s16)pos1; i<(s16)pos2; i++ ){
      data[i] = data[i+1];
    }
  }else{
    for( i=(s16)pos1; i>(s16)pos2; i-- ){
      data[i] = data[i-1];
    }

  }
  data[pos2] = copy;
}
#endif


//============================================================================================
//============================================================================================
//  データ取得
//============================================================================================
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * データインデックス取得
 *
 * @param item    アイテム番号
 * @param type    取得データ
 *
 * @return  指定データ
 *
 * @li  type = ITEM_GET_DATA    : アイテムデータのアーカイブインデックス
 * @li  type = ITEM_GET_ICON_CGX  : アイコンキャラのアーカイブインデックス
 * @li  type = ITEM_GET_ICON_PAL  : アイコンパレットのアーカイブインデックス
 * @li  type = ITEM_GET_AGB_NUM   : AGBのアイテム番号
 */
//--------------------------------------------------------------------------------------------
u16 ITEM_GetIndex( u16 item, u16 type )
{
  switch( type ){
  case ITEM_GET_DATA:   // アイテムデータ
    if( item == ITEM_DUMMY_ID || item == ITEM_RETURN_ID ){ break; }
    return ItemDataIndex[item].arc_data;

  case ITEM_GET_ICON_CGX: // アイコンキャラデータ
    if( item == ITEM_DUMMY_ID ){ return NARC_item_icon_item_dumy_NCGR; }
    if( item == ITEM_RETURN_ID ){ return NARC_item_icon_item_yaji_NCGR; }
    return ItemDataIndex[item].arc_cgx;

  case ITEM_GET_ICON_PAL: // アイコンパレットデータ
    if( item == ITEM_DUMMY_ID ){ return NARC_item_icon_item_dumy_NCLR; }
    if( item == ITEM_RETURN_ID ){ return NARC_item_icon_item_yaji_NCLR; }
    return ItemDataIndex[item].arc_pal;

  case ITEM_GET_AGB_NUM:  // AGBのアイテム番号
    if( item == ITEM_DUMMY_ID || item == ITEM_RETURN_ID ){ break; }
    return ItemDataIndex[item].agb_id;
  }
  return 0;
}

//--------------------------------------------------------------------------------------------
/**
 * AGBのアイテムをDPのアイテムに変換
 *
 * @param agb   AGBのアイテム番号
 *
 * @retval  "ITEM_DUMMY_ID = DPにないアイテム"
 * @retval  "ITEM_DUMMY_ID != DPのアイテム番号"
 */
//--------------------------------------------------------------------------------------------
u16 ITEM_CnvAgbItem( u16 agb )
{
  u16 i;

  for( i=1; i<=ITEM_DATA_MAX; i++ ){
    if( agb == ItemDataIndex[i].agb_id ){
      return i;
    }
  }
  return ITEM_DUMMY_ID;
}

//--------------------------------------------------------------
/**
 * @brief   アイテムアイコンのアーカイブID取得
 *
 * @retval  アイテムアイコンのアーカイブID
 */
//--------------------------------------------------------------
u16 ITEM_GetIconArcID(void)
{
  return ARCID_ITEMICON;
}

//--------------------------------------------------------------------------------------------
/**
 * アイテムアイコンのセルアーカイブインデックス取得
 *
 * @param none
 *
 * @return  セルアーカイブインデックス
 */
//--------------------------------------------------------------------------------------------
u16 ITEM_GetIconCell(void)
{
  return NARC_item_icon_itemicon_NCER;
}

//--------------------------------------------------------------------------------------------
/**
 * アイテムアイコンのセルアニメアーカイブインデックス取得
 *
 * @param none
 *
 * @return  セルアニメアーカイブインデックス
 */
//--------------------------------------------------------------------------------------------
u16 ITEM_GetIconCellAnm(void)
{
  return NARC_item_icon_itemicon_NANR;
}

//--------------------------------------------------------------------------------------------
/**
 * アーカイブデータロード
 *
 * @param item    アイテム番号
 * @param type    取得データ
 * @param heap_id   ヒープID
 *
 * @return  指定データ
 *
 * @li  type = ITEM_GET_DATA    : アイテムデータ
 * @li  type = ITEM_GET_ICON_CGX  : アイコンのキャラデータ
 * @li  type = ITEM_GET_ICON_PAL  : アイコンのパレットデータ
 */
//--------------------------------------------------------------------------------------------
void * ITEM_GetItemArcData( u16 item, u16 type, HEAPID heap_id )
{
  if( item > ITEM_DATA_MAX ){ item = 0; }

  switch( type ){
  case ITEM_GET_DATA:   // アイテムデータ
    return GFL_ARC_LoadDataAlloc(ARCID_ITEMDATA, ItemDataIndex[item].arc_data, heap_id);
  case ITEM_GET_ICON_CGX: // アイコンキャラデータ
    return GFL_ARC_LoadDataAlloc(ARCID_ITEMICON, ItemDataIndex[item].arc_cgx, heap_id);
  case ITEM_GET_ICON_PAL: // アイコンパレットデータ
    return GFL_ARC_LoadDataAlloc(ARCID_ITEMICON, ItemDataIndex[item].arc_pal, heap_id);
  }
  return NULL;
}

//--------------------------------------------------------------------------------------------
/**
 * アイテム名取得
 *
 * @param   buf     アイテム名格納先バッファ
 * @param   item    アイテム番号
 * @param   heap_id   ヒープID（テンポラリとして使用）
 *
 */
//--------------------------------------------------------------------------------------------
void ITEM_GetItemName( STRBUF* buf, u16 item, HEAPID heap_id )
{
  GFL_MSGDATA *man;

  man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_itemname_dat, heap_id);
  GFL_MSG_GetString(man, item, buf);
  GFL_MSG_Delete( man );
}

//--------------------------------------------------------------------------------------------
/**
 * 説明取得
 *
 * @param   buf     アイテム名格納先バッファ
 * @param item    アイテム番号
 * @param heap_id   ヒープID
 *
 * @return  説明
 */
//--------------------------------------------------------------------------------------------
void ITEM_GetInfo( STRBUF * buf, u16 item, HEAPID heap_id )
{
  GFL_MSGDATA *man;

  man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_iteminfo_dat, heap_id);
  GFL_MSG_GetString(man, item, buf);
  GFL_MSG_Delete( man );
}

//--------------------------------------------------------------------------------------------
/**
 * パラメータ取得（アイテム番号指定）
 *
 * @param item  アイテム番号
 * @param param 取得パラメータ定義
 * @param heap_id   ヒープID
 *
 * @return  指定パラメータ
 */
//--------------------------------------------------------------------------------------------
s32 ITEM_GetParam( u16 item, u16 param, HEAPID heap_id )
{
  ITEMDATA * dat;
  s32 ret;

  dat = (ITEMDATA *)ITEM_GetItemArcData( item, ITEM_GET_DATA, heap_id );
  ret = ITEM_GetBufParam( dat, param );
  GFL_HEAP_FreeMemory(dat);

  return ret;
}

//--------------------------------------------------------------------------------------------
/**
 * パラメータ取得（アイテムデータ指定）
 *
 * @param item  アイテムデータ
 * @param param 取得パラメータ定義
 *
 * @return  指定パラメータ
 */
//--------------------------------------------------------------------------------------------
s32 ITEM_GetBufParam( ITEMDATA * item, u16 param )
{
  switch( param ){
  case ITEM_PRM_PRICE:							// 買値
    return (s32)item->price;
  case ITEM_PRM_EQUIP:							// 装備効果
    return (s32)item->eqp;
  case ITEM_PRM_ATTACK:							// 威力
    return (s32)item->atc;
  case ITEM_PRM_EVENT:							// 重要
    return (s32)item->imp;
  case ITEM_PRM_CNV:								// 便利ボタン
    return (s32)item->cnv_btn;
  case ITEM_PRM_POCKET:							// 保存先（ポケット番号）
    return (s32)item->fld_pocket;
  case ITEM_PRM_FIELD:							// field機能
    return (s32)item->field_func;
  case ITEM_PRM_BATTLE:							// battle機能
    return (s32)item->battle_func;
  case ITEM_PRM_TUIBAMU_EFF:				// ついばむ効果
    return (s32)item->tuibamu_eff;
  case ITEM_PRM_NAGETUKERU_EFF:			// なげつける効果
    return (s32)item->nage_eff;
  case ITEM_PRM_NAGETUKERU_ATC:			// なげつける威力
    return (s32)item->nage_atc;
  case ITEM_PRM_SIZENNOMEGUMI_ATC:  // しぜんのめぐみ威力
    return (s32)item->sizen_atc;
  case ITEM_PRM_SIZENNOMEGUMI_TYPE: // しぜんのめぐみタイプ
    return (s32)item->sizen_type;
  case ITEM_PRM_BTL_POCKET:					// 戦闘保存先（ポケット番号）
    return (s32)item->btl_pocket;
  case ITEM_PRM_W_TYPE:							// ワークタイプ
    return (s32)item->work_type;
	case ITEM_PRM_ITEM_TYPE:					// アイテム種類
		return (s32)item->type;
	case ITEM_PRM_ITEM_SPEND:					// 消費するか
		return (s32)item->spend;

  default:              // 汎用ワーク
    switch( item->work_type ){
    case ITEM_WKTYPE_DUMMY:     // 普通データ
      return (s32)item->work[0];
    case ITEM_WKTYPE_POKEUSE:   // ポケモンに使う道具
      return ItemParamRcvGet( (ITEMPARAM_RCV *)item->work, param );
    }
  }

  return 0;
}

//--------------------------------------------------------------------------------------------
/**
 * 回復パラメータ取得
 *
 * @param rcv   回復データ
 * @param param 取得パラメータ定義
 *
 * @return  指定パラメータ
 */
//--------------------------------------------------------------------------------------------
static s32 ItemParamRcvGet( ITEMPARAM_RCV * rcv, u16 param )
{
  switch( param ){
  case ITEM_PRM_SLEEP_RCV:      // 眠り回復
    return (s32)rcv->sleep_rcv;
  case ITEM_PRM_POISON_RCV:     // 毒回復
    return (s32)rcv->poison_rcv;
  case ITEM_PRM_BURN_RCV:       // 火傷回復
    return (s32)rcv->burn_rcv;
  case ITEM_PRM_ICE_RCV:        // 氷回復
    return (s32)rcv->ice_rcv;
  case ITEM_PRM_PARALYZE_RCV:     // 麻痺回復
    return (s32)rcv->paralyze_rcv;
  case ITEM_PRM_PANIC_RCV:      // 混乱回復
    return (s32)rcv->panic_rcv;
  case ITEM_PRM_MEROMERO_RCV:     // メロメロ回復
    return (s32)rcv->meromero_rcv;
  case ITEM_PRM_ABILITY_GUARD:    // 能力ガード
    return (s32)rcv->ability_guard;
  case ITEM_PRM_DEATH_RCV:      // 瀕死回復
    return (s32)rcv->death_rcv;
  case ITEM_PRM_ALL_DEATH_RCV:    // 全員瀕死回復
    return (s32)rcv->alldeath_rcv;
  case ITEM_PRM_LV_UP:        // レベルアップ
    return (s32)rcv->lv_up;
  case ITEM_PRM_EVOLUTION:      // 進化
    return (s32)rcv->evolution;
  case ITEM_PRM_ATTACK_UP:      // 攻撃力アップ
    return (s32)rcv->atc_up;
  case ITEM_PRM_DEFENCE_UP:     // 防御力アップ
    return (s32)rcv->def_up;
  case ITEM_PRM_SP_ATTACK_UP:     // 特攻アップ
    return (s32)rcv->spa_up;
  case ITEM_PRM_SP_DEFENCE_UP:    // 特防アップ
    return (s32)rcv->spd_up;
  case ITEM_PRM_AGILITY_UP:     // 素早さアップ
    return (s32)rcv->agi_up;
  case ITEM_PRM_HIT_UP:       // 命中率アップ
    return (s32)rcv->hit_up;
  case ITEM_PRM_CRITICAL_UP:      // クリティカル率アップ
    return (s32)rcv->critical_up;
  case ITEM_PRM_PP_UP:        // PPアップ
    return (s32)rcv->pp_up;
  case ITEM_PRM_PP_3UP:       // PPアップ（３段階）
    return (s32)rcv->pp_3up;
  case ITEM_PRM_PP_RCV:       // PP回復
    return (s32)rcv->pp_rcv;
  case ITEM_PRM_ALL_PP_RCV:     // PP回復（全ての技）
    return (s32)rcv->allpp_rcv;
  case ITEM_PRM_HP_RCV:       // HP回復
    return (s32)rcv->hp_rcv;
  case ITEM_PRM_HP_EXP:       // HP努力値アップ
    return (s32)rcv->hp_exp;
  case ITEM_PRM_POWER_EXP:      // 攻撃努力値アップ
    return (s32)rcv->pow_exp;
  case ITEM_PRM_DEFENCE_EXP:      // 防御努力値アップ
    return (s32)rcv->def_exp;
  case ITEM_PRM_AGILITY_EXP:      // 素早さ努力値アップ
    return (s32)rcv->agi_exp;
  case ITEM_PRM_SP_ATTACK_EXP:    // 特攻努力値アップ
    return (s32)rcv->spa_exp;
  case ITEM_PRM_SP_DEFENCE_EXP:   // 特防努力値アップ
    return (s32)rcv->spd_exp;
  case ITEM_PRM_FRIEND1:        // なつき度１
    return (s32)rcv->friend_exp1;
  case ITEM_PRM_FRIEND2:        // なつき度２
    return (s32)rcv->friend_exp2;
  case ITEM_PRM_FRIEND3:        // なつき度３
    return (s32)rcv->friend_exp3;

  case ITEM_PRM_HP_EXP_POINT:     // HP努力値の値
    return (s32)rcv->prm_hp_exp;
  case ITEM_PRM_POWER_EXP_POINT:    // 攻撃努力値の値
    return (s32)rcv->prm_pow_exp;
  case ITEM_PRM_DEFENCE_EXP_POINT:  // 防御努力値の値
    return (s32)rcv->prm_def_exp;
  case ITEM_PRM_AGILITY_EXP_POINT:  // 素早さ努力値の値
    return (s32)rcv->prm_agi_exp;
  case ITEM_PRM_SP_ATTACK_EXP_POINT:  // 特攻努力値の値
    return (s32)rcv->prm_spa_exp;
  case ITEM_PRM_SP_DEFENCE_EXP_POINT: // 特防努力値の値
    return (s32)rcv->prm_spd_exp;
  case ITEM_PRM_HP_RCV_POINT:     // HP回復値の値
    return (s32)rcv->prm_hp_rcv;
  case ITEM_PRM_PP_RCV_POINT:     // pp回復値の値
    return (s32)rcv->prm_pp_rcv;
  case ITEM_PRM_FRIEND1_POINT:    // なつき度1の値
    return (s32)rcv->prm_friend1;
  case ITEM_PRM_FRIEND2_POINT:    // なつき度2の値
    return (s32)rcv->prm_friend2;
  case ITEM_PRM_FRIEND3_POINT:    // なつき度3の値
    return (s32)rcv->prm_friend3;
  }

  return 0;
}


//============================================================================================
//  技マシン
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		技マシンかどうか
 *
 * @param		item  アイテム番号
 *
 * @retval	"TRUE = 技マシン"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL ITEM_CheckWazaMachine( u16 item )
{
	if( ( item >= ITEM_WAZAMASIN01 && item <= ITEM_HIDENMASIN08 ) ||
			( item >= ITEM_WAZAMASIN93 && item <= ITEM_WAZAMASIN101 ) ){
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * 技マシンで覚えられる技を取得
 *
 * @param item  アイテム番号
 *
 * @return  技番号
 */
//--------------------------------------------------------------------------------------------
const u16 ITEM_GetWazaNo( u16 item )
{
	if( ITEM_CheckWazaMachine( item ) == FALSE ){
    return WAZANO_NULL;
  }

	if( item >= ITEM_WAZAMASIN93 ){
		item = item - ITEM_WAZAMASIN93 + HIDENWAZA_START_POS + HIDENWAZA_MAX;
	}else{
		item -= ITEM_WAZAMASIN01;
	}
  return MachineNo[ item ];
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   選択された技がポケモン交換に適さない技かどうか（秘伝技かどうか）
 *
 * @param waza  技番号
 *
 * @retval  "TRUE = 交換に適さない技(秘伝技"
 * @retval  "FALSE = 交換に適する技(秘伝技以外"
 */
//--------------------------------------------------------------------------------------------
BOOL ITEM_CheckHidenWaza( u16 waza )
{
  u8  i;

  for( i=0; i<HIDENWAZA_MAX; i++ ){
    if( MachineNo[HIDENWAZA_START_POS+i] == waza ){
      return TRUE;
    }
  }
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * 技マシン番号取得
 *
 * @param item  アイテム番号
 *
 * @return  技マシン番号
 *
 * @li	秘伝マシンは通常の技マシンの後にカウントされる
 */
//--------------------------------------------------------------------------------------------
u8 ITEM_GetWazaMashineNo( u16 item )
{
	if( ITEM_CheckWazaMachine( item ) == FALSE ){
    return 0;
  }

	if( item >= ITEM_WAZAMASIN93 ){
		return ( item - ITEM_WAZAMASIN93 + HIDENWAZA_START_POS );
	}else if( item >= ITEM_HIDENMASIN01 ){
		return ( item - ITEM_HIDENMASIN01 + NORMAL_WAZAMACHINE_MAX );
	}
  return ( item - ITEM_WAZAMASIN01 );
}

//--------------------------------------------------------------------------------------------
/**
 * 秘伝マシン番号取得
 *
 * @param item  アイテム番号
 *
 * @return  秘伝マシン番号
 */
//--------------------------------------------------------------------------------------------
u8 ITEM_GetHidenMashineNo( u16 item )
{
	if( item >= ITEM_HIDENMASIN01 && item <= ITEM_HIDENMASIN08 ){
		return ( item - ITEM_HIDENMASIN01 );
	}
  return 0xff;
}

//============================================================================================
//  メールチェック
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * アイテムがメールかどうかをチェック
 *
 * @param item  アイテム番号
 *
 * @retval  "TRUE = メール"
 * @retval  "FALSE = メール以外"
 */
//--------------------------------------------------------------------------------------------
BOOL ITEM_CheckMail( u16 item )
{
  u32 i;

  for( i=0; i<ITEM_MAIL_MAX; i++ ){
    if( ItemMailTable[i] == item ){
      return TRUE;
    }
  }
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * アイテム番号からメールのデザインを取得
 *
 * @param item  アイテム番号
 *
 * @return  デザインID
 */
//--------------------------------------------------------------------------------------------
u8 ITEM_GetMailDesign( u16 item )
{
  u32 i;

  for( i=0; i<ITEM_MAIL_MAX; i++ ){
    if( ItemMailTable[i] == item ){
      return (u8)i;
    }
  }
  return 0;
}

//--------------------------------------------------------------------------------------------
/**
 * メールのデザインからアイテム番号を取得
 *
 * @param design    デザインID
 *
 * @return  アイテム番号
 */
//--------------------------------------------------------------------------------------------
u16 ITEM_MailNumGet( u8 design )
{
  if( design >= ITEM_MAIL_MAX ){
    return 0;
  }
  return ItemMailTable[design];
}


//============================================================================================
//  木の実チェック
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * アイテムが木の実かどうかをチェック
 *
 * @param item  アイテム番号
 *
 * @retval  "TRUE = 木の実"
 * @retval  "FALSE = 木の実以外"
 */
//--------------------------------------------------------------------------------------------
BOOL ITEM_CheckNuts( u16 item )
{
  u32 i;

  for( i=0; i<ITEM_NUTS_MAX; i++ ){
    if( ItemNutsTable[i] == item ){
      return TRUE;
    }
  }
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * 木の実の番号を取得
 *
 * @param item  アイテム番号
 *
 * @return  木の実番号
 */
//--------------------------------------------------------------------------------------------
u8 ITEM_GetNutsNo( u16 item )
{
  if( item < ITEM_KURABONOMI ){
    return 0xff;
  }
  return ( item - ITEM_KURABONOMI );
}

//--------------------------------------------------------------------------------------------
/**
 * 指定IDの木の実のアイテム番号を取得
 *
 * @param id    木の実のID
 *
 * @return  アイテム番号
 */
//--------------------------------------------------------------------------------------------
u16 ITEM_GetNutsID( u8 id )
{
  u32 i;

  if( id >= ITEM_NUTS_MAX ){ return 0xffff; }   // エラー

  return ItemNutsTable[id];
}


//============================================================================================
//  漢方薬チェック
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * アイテムが漢方薬かどうかをチェック
 *
 * @param item  アイテム番号
 *
 * @retval  "TRUE = 漢方薬"
 * @retval  "FALSE = 漢方薬"
 */
//--------------------------------------------------------------------------------------------
BOOL ITEM_CheckKanpouyaku( u16 item )
{
  u32 i;

  if( item == ITEM_TIKARANOKONA || item == ITEM_TIKARANONEKKO ||
    item == ITEM_BANNOUGONA || item == ITEM_HUKKATUSOU ){

    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * アイテムテーブルをすべてメモリに展開
 *
 * @param heapID  展開するメモリヒープID
 *
 * @retval  確保したメモリのアドレス
 */
//--------------------------------------------------------------------------------------------
void  *ITEM_LoadDataTable( HEAPID heapID )
{
  int max;

  max=ITEM_GetIndex(ITEM_DATA_MAX,ITEM_GET_DATA);
  return GFL_ARC_LoadDataAllocOfs(ARCID_ITEMDATA, 0, heapID, 0, sizeof(ITEMDATA)*ITEM_DATA_MAX);
}

//--------------------------------------------------------------------------------------------
/**
 * アイテムテーブルから任意のアドレスを取得
 *
 * @param item  アイテムテーブル
 * @param index とりだすインデックスナンバー
 *
 * @retval  取得したアドレス
 */
//--------------------------------------------------------------------------------------------
ITEMDATA  *ITEM_GetDataPointer(ITEMDATA *item,u16 index)
{
  u8 *ret_item;

  ret_item=(u8 *)item;
  ret_item+=index*sizeof(ITEMDATA);

  return (ITEMDATA *)ret_item;
}

//--------------------------------------------------------------------------------------------
/**
 * アイテムをポケモンに持たせられるか
 *
 * @param item  アイテム番号
 *
 * @retval  "TRUE = 可"
 * @retval  "FALSE = 不可"
 *
 *  基本、金銀で追加されたもの
 */
//--------------------------------------------------------------------------------------------
BOOL ITEM_CheckPokeAdd( u16 item )
{
  if( item == ITEM_SUPIIDOBOORU ||
    item == ITEM_REBERUBOORU ||
    item == ITEM_RUAABOORU ||
    item == ITEM_HEBIIBOORU ||
    item == ITEM_RABURABUBOORU ||
    item == ITEM_HURENDOBOORU ||
    item == ITEM_MUUNBOORU ||
    item == ITEM_KONPEBOORU ||
    item == ITEM_PAAKUBOORU ){
    return FALSE;
  }

  return TRUE;
}


//--------------------------------------------------------------------------------------------
/**
 * 古いアイテムで今は使われていないかどうか検査する
 * @param item  アイテム番号
 * @retval  TRUE  = つかえる
 * @retval  FALSE = つかえない
 */
//--------------------------------------------------------------------------------------------
BOOL ITEM_CheckEnable( u16 item )
{
  //中身は修正予定
  return (ItemDataIndex[item].arc_cgx != NARC_item_icon_item_dumy_NCGR);
}


