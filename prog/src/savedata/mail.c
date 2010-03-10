/**
 *  @file mail.c
 *  @brief  メールセーブデータ　制御
 *  @author Miyuki Iwasawa
 *  @date 06.02.09
 *
 *  メールセーブデータ反映手順
 *  1,MAIL_SearchNullID()で空き領域を検索、データID取得
 *  2,MailData_CreateWork()でダミーデータ作成用ワークエリアを取得
 *  3,MailData_CreateDataFromSave()などを使ってワークにデータを構築
 *  4,MAIL_AddDataFromWork()と取得済みのデータID,ワークのデータを使って、
 *  　セーブ領域にデータを反映させる
 *  5,ダミーワークエリアを開放する
 */

#include "gflib.h"
#include "savedata/save_tbl.h"
#include "gamesystem/game_data.h"

#include "savedata/mail.h"
#include "savedata/mail_util.h"
//#include "savedata/mail_local.h"
#include "savedata/mystatus.h"

#include "poke_tool/poke_tool.h"
#include "poke_tool/pokeparty.h"
#include "pokeicon/pokeicon.h"

#include "system/pms_data.h"

#include "poke_icon.naix"
#include "poke_tool/monsno_def.h"

//==============================================================================================
// 構造体宣言
//==============================================================================================

//=============================================================
///メールデータ型
// 
//  size=56byte
//
//  構造体サイズを4byte境界に合わせるため、MAIL_ICON配列要素数に+1して
//  パディングを行っています
//=============================================================
typedef struct _MAIL_DATA{
  u32 writerID; //<トレーナーID 4
  u8  sex;      //<主人公の性別 1
  u8  region;   //<国コード 1
  u8  version;  //<カセットバージョン 1
  u8  design;   //<デザインナンバー 1
  STRCODE name[PERSON_NAME_SIZE+EOM_SIZE];  // 16
  MAIL_ICON icon[MAILDAT_ICONMAX];          //<アイコンNO格納場所[]
  u16 form_bit;                             //padding領域をプラチナから3体のポケモンの
                                            // フォルム番号として使用(5bit単位)
  PMS_DATA  msg[MAILDAT_MSGMAX];            //<文章データ
}_MAIL_DATA;


//=============================================================
///メールデータセーブデータブロック定義構造体
//=============================================================
typedef struct _MAIL_BLOCK{

  MAIL_DATA paso[MAIL_STOCK_PASOCOM];   // メール最大数20個

}_MAIL_BLOCK;


///ポケモンアイコンの最大CGXID(金銀でアイコンが増えるならここも変える必要がある)
#define ICON_CGXID_MAX    (NARC_poke_icon_poke_icon_656_freeze_f_NCGR)

#if 0
///プラチナ以降で追加されたポケモンのcgx_id変換テーブル
///(金銀でアイコンが増えるならこのテーブルも増やす必要がある)
static const struct{
  u16 normal_cgx_id;    ///<フォルム0の時のcgxID
  u16 form_cgx_id;    ///<フォルムが変わっている時のcgxID
  u16 monsno;       ///<ポケモン番号
  u8 form_no;       ///<form_cgx_idはフォルムNoいくつのcgxIDなのか
  u8 padding;     //ダミー
} MailIcon_CgxID_ConvTbl[] = {
  {
    NARC_poke_icon_poke_icon_509_NCGR,
    NARC_poke_icon_poke_icon_509_01_NCGR,
    MONSNO_GIRATHINA,
    1,
  },
  {
    NARC_poke_icon_poke_icon_516_NCGR,
    NARC_poke_icon_poke_icon_516_01_NCGR,
    MONSNO_SHEIMI,
    1,
  },
  {
    NARC_poke_icon_poke_icon_519_NCGR,
    NARC_poke_icon_poke_icon_519_01_NCGR,
    MONSNO_ROTOMU,
    1,
  },
  {
    NARC_poke_icon_poke_icon_519_NCGR,
    NARC_poke_icon_poke_icon_519_02_NCGR,
    MONSNO_ROTOMU,
    2,
  },
  {
    NARC_poke_icon_poke_icon_519_NCGR,
    NARC_poke_icon_poke_icon_519_03_NCGR,
    MONSNO_ROTOMU,
    3,
  },
  {
    NARC_poke_icon_poke_icon_519_NCGR,
    NARC_poke_icon_poke_icon_519_04_NCGR,
    MONSNO_ROTOMU,
    4,
  },
  {
    NARC_poke_icon_poke_icon_519_NCGR,
    NARC_poke_icon_poke_icon_519_05_NCGR,
    MONSNO_ROTOMU,
    5,
  },
};
#endif

/**
 *  @brief  メールデータサイズ取得
 *
 *  ＊メールデータ一通のサイズ
 */
int MailData_GetDataWorkSize(void)
{
  return sizeof(MAIL_DATA);
}
/**
 *  @brief  メールデータクリア(初期データセット)
 */
void MailData_Clear(MAIL_DATA* dat)
{
  int i;
  
  dat->writerID = 0;
  dat->sex = PM_MALE;
  //dat->region = CasetteLanguage;    //@todo定義が無い
//  dat->version = CasetteVersion; //@todo定義が無い
  dat->design = MAIL_DESIGN_NULL;

  GFL_STD_MemFill16(dat->name, GFL_STR_GetEOMCode(),sizeof(dat->name));

  for(i = 0;i < MAILDAT_ICONMAX;i++){
    dat->icon[i].dat = MAIL_ICON_NULL;
  }
  dat->form_bit = 0;
  for(i = 0;i < MAILDAT_MSGMAX;i++){
    PMSDAT_Clear(&dat->msg[i]);
  }
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_MAILDATA)
  SVLD_SetCrc(GMDATA_ID_MAILDATA);
#endif //CRC_LOADCHECK
}

/**
 *  @brief  メールデータが有効かどうか返す
 *  @retval FALSE 無効
 *  @retval TRUE  有効
 */
BOOL MailData_IsEnable(MAIL_DATA* dat)
{
  if( dat->design >= MAIL_DESIGN_START &&
    dat->design <= MAIL_DESIGN_END){
    return TRUE;
  }
  return FALSE;
}

/**
 *  @brief  メールデータのワークを取得して返す
 *
 *  ＊呼び出し側が責任もって解放すること
 *  
 */
MAIL_DATA* MailData_CreateWork(HEAPID heapID)
{
  MAIL_DATA* p;

  p = GFL_HEAP_AllocMemoryLo(heapID,sizeof(MAIL_DATA)); //sys_AllocMemoryLo(heapID,sizeof(MAIL_DATA));
  MailData_Clear(p);

  return p;
}

/**
 *  @brief  メールデータの構造体コピー
 */
void MailData_Copy(MAIL_DATA* src,MAIL_DATA* dest)
{
  MI_CpuCopy8(src,dest,sizeof(MAIL_DATA));
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_MAILDATA)
  SVLD_SetCrc(GMDATA_ID_MAILDATA);
#endif //CRC_LOADCHECK
}

/**
 *  @brief  メールデータの内容比較
 *
 * @retval  BOOL    一致していたらTRUEを返す
 */
BOOL MailData_Compare(MAIL_DATA* src1,MAIL_DATA* src2)
{
  int i;
  
  if( (src1->writerID != src2->writerID) ||
    (src1->sex != src2->sex) ||
    (src1->region != src2->region) ||
    (src1->version != src2->version) ||
    (src1->design != src2->design) ||
    (src1->form_bit != src2->form_bit) ){
    return FALSE;
  }
  if( GFL_STD_MemComp( src1->name,src2->name,sizeof(src1->name) ) != 0){
    return FALSE;
  }
  for(i = 0;i < MAILDAT_ICONMAX;i++){
    if(src1->icon[i].dat != src2->icon[i].dat){
      return FALSE;
    }
  }
  for(i = 0;i < MAILDAT_MSGMAX;i++){
    if(!PMSDAT_Compare( &src1->msg[i], &src2->msg[i] )){
      return FALSE;
    }
  }
  return TRUE;
}

/**
 *  @brief  デザインNo,ポケモンのポジション、セーブデータ指定してメールを新規作成状態に初期化
 *  @param  dat データを作成するMAIL_DATA構造体型へのポインタ
 *  @param  design_no メールのイメージNo
 *  @param  pos   メールを持たせるポケモンの手持ち内のポジション
 *  @param  save  セーブデータへのポインタ
 */
void MAILDATA_CreateFromSaveData(MAIL_DATA* dat,u8 design_no,u8 pos, GAMEDATA* gamedata)
{
  u8  i,ct,pal=0,s;
  u16 monsno;
  u32 icon,egg,form;
  MYSTATUS  *my;
  POKEPARTY *party;
  POKEMON_PARAM* pp;
  
  MailData_Clear(dat);
  dat->design = design_no;

  //セーブデータから
  party = GAMEDATA_GetMyPokemon( gamedata );
  my    = GAMEDATA_GetMyStatus( gamedata );

  //自機の名前
  GFL_STD_MemCopy( MyStatus_GetMyName(my), dat->name, sizeof(dat->name));
  //性別
  dat->sex = (u8)MyStatus_GetMySex(my);
  //トレーナーID
  dat->writerID = MyStatus_GetID(my);

  //ポケモンアイコン取得
  dat->form_bit = 0;
  for(i=pos,ct = 0;i < PokeParty_GetPokeCount(party);i++){
    pp = PokeParty_GetMemberPointer(party,i);
    monsno = PP_Get(pp,ID_PARA_monsno,NULL);
    egg = PP_Get(pp,ID_PARA_tamago_flag,NULL);
    form = PP_Get(pp,ID_PARA_form_no,NULL);
    //icon = PokeIconCgxArcIndexGetByPP(pp);
    icon = POKEICON_GetCgxArcIndex( PP_GetPPPPointerConst(pp) );
//    pal = PokeIconPaletteNumberGet(monsno,egg);
//    pal = PokeIconPalNumGet(monsno,form,egg);   //@todo  移植できてない
    
    dat->icon[ct].cgxID = (u16)icon;
    dat->icon[ct].palID = pal;
#if 0 //@todo 移植できてない
    //プラチナ以降で追加されたアイコンの場合のノーマルフォルム変換(フォルム番号は別領域へ退避)
    for(s = 0; s < NELEMS(MailIcon_CgxID_ConvTbl); s++){
      if(MailIcon_CgxID_ConvTbl[s].form_cgx_id == dat->icon[ct].cgxID && 
          MailIcon_CgxID_ConvTbl[s].form_no == form){
        dat->icon[ct].cgxID = MailIcon_CgxID_ConvTbl[s].normal_cgx_id;
        dat->icon[ct].palID = PokeIconPalNumGet( monsno, 0, egg );  //フォルム0のパレット
        dat->form_bit |= MailIcon_CgxID_ConvTbl[s].form_no << (ct*5);
        break;
      }
    }
#endif
    ct++;
    if(ct >= MAILDAT_ICONMAX){
      break;
    }
  }
}

/**
 *  @brief  メールポケモンイベント専用メールデータ生成
 *
 *  @param  pp      メールを持たせるポケモンのポケパラ
 *  @param  design_no メールのイメージNo
 *  @param  oya_sex   親の性別
 *  @param  oya_name  親の名前
 *  @param  oya_id    親のトレーナーID
 *
 *  @com  メモリをAllocして返すので、呼び出し側が解放すること！
 */
MAIL_DATA* MailData_MailEventDataMake(POKEMON_PARAM* pp,
  u8 design_no,u8 oya_sex,STRBUF* oya_name,u8 oya_id, HEAPID heapID)
{
  MAIL_DATA* dat=MailData_CreateWork(heapID);

  MailData_Clear(dat);
  dat->design = design_no;

  //名前
  GFL_STD_MemCopy( oya_name, dat->name, BUFLEN_PERSON_NAME );

  //性別
  dat->sex = oya_sex;
  //トレーナーID
  dat->writerID = oya_id;
  
  //簡易文データセット
  PMSDAT_SetSentence( &dat->msg[0], 1, 7 );
  PMSDAT_SetWord( &dat->msg[0], 0, 1349 );
  PMSDAT_SetWord( &dat->msg[0], 1, PMS_WORD_NULL );
  
  PMSDAT_SetSentence( &dat->msg[1], 1, 15 );
  PMSDAT_SetWord( &dat->msg[1], 0, 1371 );
  PMSDAT_SetWord( &dat->msg[1], 1, PMS_WORD_NULL );
  
  PMSDAT_SetSentence( &dat->msg[2], 2, 1 );
  PMSDAT_SetWord( &dat->msg[2], 0, 41 );
  PMSDAT_SetWord( &dat->msg[2], 1, PMS_WORD_NULL );
  
  //ポケモンアイコン取得
  dat->form_bit = 0;
  {
    u8  pal=0;
    u16 monsno;
    u32 icon,egg,form;

    monsno = PP_Get(pp,ID_PARA_monsno,NULL);
    egg = PP_Get(pp,ID_PARA_tamago_flag,NULL);
    form = PP_Get(pp,ID_PARA_form_no,NULL);
//    icon = PokeIconCgxArcIndexGetByPP(pp);
    icon = POKEICON_GetCgxArcIndex( PP_GetPPPPointerConst(pp) );
//    pal = PokeIconPalNumGet(monsno,form,egg);   @todo移植できてない
    
    dat->icon[0].cgxID = (u16)icon;
    dat->icon[0].palID = pal;
  }
  
  return dat;
}

/**
 *  @brief  メールデータ　トレーナーID取得
 */
u32 MailData_GetWriterID(const MAIL_DATA* dat)
{
  return dat->writerID;
}
/**
 *  @brief  メールデータ　トレーナーID適用
 */
void MailData_SetWriterID(MAIL_DATA* dat,u32 id)
{
  dat->writerID = id;
}

/**
 *  @brief  メールデータ　ライター名取得
 */
STRCODE* MailData_GetWriterName(MAIL_DATA* dat)
{
  return &(dat->name[0]);
}
/**
 *  @brief  メールデータ　ライター名適用
 */
void MailData_SetWriterName(MAIL_DATA* dat,STRCODE* name)
{
  GFL_STD_MemCopy( name, dat->name, BUFLEN_PERSON_NAME);
}

/**
 *  @brief  メールデータ　ライターの性別を取得
 */
u8  MailData_GetWriterSex(const MAIL_DATA* dat)
{
  return dat->sex;
}
/**
 *  @brief  メールデータ　ライターの性別を適用
 */
void MailData_SetWriterSex(MAIL_DATA* dat,const u8 sex)
{
  dat->sex = sex;
}

/**
 *  @brief  メールデータ　デザインNo取得
 */
u8  MailData_GetDesignNo(const MAIL_DATA* dat)
{
  return dat->design;
}
/**
 *  @brief  メールデータ　デザインNo適用
 */
void MailData_SetDesignNo(MAIL_DATA* dat,const u8 design)
{
  if(design >= MAIL_DESIGN_MAX){
    return;
  }
  dat->design = design;
}

/**
 *  @brief  メールデータ　国コード取得
 */
u8  MailData_GetCountryCode(const MAIL_DATA* dat)
{
  return dat->region;
}
/**
 *  @brief  メールデータ　国コード適用
 */
void MailData_SetCountryCode(MAIL_DATA* dat,const u8 code)
{
  dat->region = code;
}

/**
 *  @brief  メールデータ　カセットバージョン取得
 */
u8  MailData_GetCasetteVersion(const MAIL_DATA* dat)
{
  return dat->version;
}
/**
 *  @brief  メールデータ　カセットバージョン適用
 */
void MailData_SetCasetteVersion(MAIL_DATA* dat,const u8 version)
{
  dat->version = version;
}

/**
 *  @brief  メールデータ　メールアイコンパラメータの取得(インデックス指定版)
 *
 *  @param  mode  MAIL_ICONPRM_CGX:cgxNoの取得
 *          MAIL_ICONPRM_PAL:pltNoの取得
 *          MAIL_ICONPRM_ALL:u16型(MAIL_ICON型へキャスト可)で双方の値を返す
 *
 *  ＊アイコンCgxIDとモンスターNoは同一ではありません。注意！
 */
u16 MailData_GetIconParamByIndex(const MAIL_DATA* dat,u8 index,u8 mode, u16 form_bit)
{
  MAIL_ICON mi;
  int s;
  
  if(index < MAILDAT_ICONMAX){
    mi = dat->icon[index];
#if 0   //@todo 移植できていない
    //プラチナ以降で追加されたアイコンのフォルムIndexへ変換
    for(s = 0; s < NELEMS(MailIcon_CgxID_ConvTbl); s++){
      if(MailIcon_CgxID_ConvTbl[s].normal_cgx_id == mi.cgxID && 
          MailIcon_CgxID_ConvTbl[s].form_no == ((form_bit >> (index*5)) & 0x1f)){
        mi.cgxID = MailIcon_CgxID_ConvTbl[s].form_cgx_id;
        mi.palID = PokeIconPalNumGet( 
          MailIcon_CgxID_ConvTbl[s].monsno, MailIcon_CgxID_ConvTbl[s].form_no, 0 );
        break;
      }
    }
#endif
    if(mi.cgxID > ICON_CGXID_MAX){
      mi.cgxID = NARC_poke_icon_poke_icon_000_m_NCGR;
      mi.palID = 0;
    }
    switch(mode){
    case MAIL_ICONPRM_CGX:
      return mi.cgxID;
    case MAIL_ICONPRM_PAL:
      return mi.palID;
    case MAIL_ICONPRM_ALL:
    default:
      return mi.dat;
    }
  }else{
    return 0;
  }
}

/**
 *  @brief  メールデータ　form_bit取得
 */
u16 MailData_GetFormBit(const MAIL_DATA* dat)
{
  return dat->form_bit;
}

/**
 *  @brief  メールデータ　簡易文取得(インデックス指定版)
 */
PMS_DATA* MailData_GetMsgByIndex(MAIL_DATA* dat,u8 index)
{
  if(index < MAILDAT_MSGMAX){
    return &(dat->msg[index]);
  }else{
    return &(dat->msg[0]);
  }
}
/**
 *  @brief  メールデータ　簡易文セット(インデックス指定版)
 */
void MailData_SetMsgByIndex(MAIL_DATA* dat,PMS_DATA* pms,u8 index)
{
  if(index >= MAILDAT_MSGMAX){
    return;
  }
  PMSDAT_Copy(&dat->msg[index],pms);
}

/**
 *  @brief  メールデータ　簡易文文字列取得(インデックス指定)
 *
 *  @param  dat MAIL_DATA*
 *  @param  index 簡易文インデックス
 *  @param  buf   取得した文字列ポインタの格納場所
 *
 *  @retval FALSE 文字列の取得に失敗(または簡易文が有効なデータではない)
 *  
 *  @li bufに対して内部でメモリを確保しているので、呼び出し側が明示的に解放すること
 *  @li FALSEが返った場合、bufはNULLクリアされる
 */
BOOL MailData_GetMsgStrByIndex(const MAIL_DATA* dat,u8 index,STRBUF* buf,HEAPID heapID)
{
  if(index >= MAILDAT_MSGMAX){
    buf = NULL;
    return FALSE;
  }
  
  if(!PMSDAT_IsEnabled(&dat->msg[index])){
    buf = NULL;
    return FALSE;
  }

  buf = PMSDAT_ToString(&dat->msg[index],heapID);
  return TRUE;
}

//=================================================================
//
//=================================================================
//ローカル関数プロトタイプ
static int mail_GetNullData(MAIL_DATA* array,int num);
static int mail_GetNumEnable(MAIL_DATA* array,int num);
static MAIL_DATA* mail_GetAddress(MAIL_BLOCK* bloc,MAILBLOCK_ID blockID,int dataID);

/**
 *  @brief  セーブデータブロックへのポインタを取得
MAIL_BLOCK* SaveData_GetMailBlock(SAVE_CONTROL_WORK* sv)
{
  return SaveControl_DataPtrGet( sv, GMDATA_ID_MAILDATA );
}
 */

//=============================================================================================
/**
 * @brief GAMEDATAからメールデータブロックを返す
 *
 * @param   gamedata    
 *
 * @retval  MAIL_BLOCK*   
 */
//=============================================================================================
MAIL_BLOCK* GAMEDATA_GetMailBlock( GAMEDATA *gamedata )
{
  SAVE_CONTROL_WORK *sv = GAMEDATA_GetSaveControlWork( gamedata );
  return SaveControl_DataPtrGet( sv, GMDATA_ID_MAILDATA );
  
}
/**
 *  @brief  メールセーブデータブロックサイズ取得
 *
 *  ＊メールデータ一通のサイズではないので注意！
 */
int MAIL_GetBlockWorkSize(void)
{
  return sizeof(MAIL_DATA)*MAIL_STOCK_MAX;
}
/**
 *  @brief  メールセーブデータブロック初期化
 */
void MAIL_Init(MAIL_BLOCK* dat)
{
  int i = 0;

  for(i = 0;i < MAIL_STOCK_PASOCOM;i++){
    MailData_Clear(&dat->paso[i]);
  }
}

/**
 *  @brief  空いているメールデータIDを取得
 *
 *  @param  id 追加したいメールブロックID
 *
 *  @return int データを追加できる場合は参照ID
 *        追加できない場合はマイナス値が返る
 */
int MAIL_SearchNullID(MAIL_BLOCK* block,MAILBLOCK_ID id)
{
  return mail_GetNullData(block->paso,MAIL_STOCK_PASOCOM);
}

/**
 *  @brief  メールデータを削除
 *
 *  @param  blockID ブロックのID
 *  @param  dataID  データID
 */
void MAIL_DelMailData(MAIL_BLOCK* block,MAILBLOCK_ID blockID,int dataID)
{
  MAIL_DATA* pd = NULL;
  
  pd = mail_GetAddress(block,blockID,dataID);
  if(pd != NULL){
    MailData_Clear(pd);
  }
}

/**
 *  @brief  メールデータをセーブブロックに追加
 *
 *  ＊引き渡したMAIL_DATA構造体型データの中身がセーブデータに反映されるので
 *  　おかしなデータを入れないように注意！
 */
void MAIL_AddMailFormWork(MAIL_BLOCK* block,MAILBLOCK_ID blockID,int dataID,MAIL_DATA* src)
{
  MAIL_DATA* pd = NULL;
  
  pd = mail_GetAddress(block,blockID,dataID);
  if(pd != NULL){
    MailData_Copy(src,pd);
  }
}

/**
 *  @brief  指定ブロックに有効データがいくつあるか返す
 */
int MAIL_GetEnableDataNum(MAIL_BLOCK* block,MAILBLOCK_ID blockID)
{
  return mail_GetNumEnable(block->paso,MAIL_STOCK_PASOCOM);
}

/**
 *  @brief  メールデータのコピーを取得
 *
 *  ＊内部でメモリを確保するので、呼び出し側が責任持って領域を開放すること
 *  ＊無効IDを指定した場合、空データを返す
 */
MAIL_DATA* MAIL_AllocMailData(MAIL_BLOCK* block,MAILBLOCK_ID blockID,int dataID,HEAPID heapID)
{
  MAIL_DATA* src = NULL;
  MAIL_DATA* dest;
  
  src = mail_GetAddress(block,blockID,dataID);
  dest = MailData_CreateWork(heapID);
  if(src != NULL){
    MailData_Copy(src,dest);
  }
  return dest;
}

/**
 *  @brief  メールデータのコピーを取得
 *
 *  ＊あらかじめ確保したMAIL_DATA型メモリにセーブデータをコピーして取得
 */
void MAIL_GetMailData(MAIL_BLOCK* block,MAILBLOCK_ID blockID,int dataID,MAIL_DATA* dest)
{
  MAIL_DATA* src = NULL;
  
  src = mail_GetAddress(block,blockID,dataID);
  if(src == NULL){
    MailData_Clear(dest);
  }else{
    MailData_Copy(src,dest);
  }
}

//=================================================================
//
//=================================================================

/**
 *  @brief  手持ちブロックの空きを探して返す
 *
 *  @param  array メールデータ配列へのポインタ
 *  @param  num   第一引数で渡した配列の要素数
 */
static int mail_GetNullData(MAIL_DATA* array,int num)
{
  int i = 0;

  for(i = 0;i < num;i++){
    if(!MailData_IsEnable(&array[i])){
      return i;
    }
  }
  return MAILDATA_NULLID;
}

/**
 *  @brief  有効データの数を探して返す
 */
static int mail_GetNumEnable(MAIL_DATA* array,int num)
{
  int i = 0;
  int ct = 0;
  
  for(i = 0;i < num;i++){
    if(MailData_IsEnable(&array[i])){
      ct++;
    }
  }
  return ct;
}

/**
 *  @brief  指定IDを持つブロック内のメールデータへのポインタを返す
 */
static MAIL_DATA* mail_GetAddress(MAIL_BLOCK* block,MAILBLOCK_ID blockID,int dataID)
{
  if(dataID < MAIL_STOCK_PASOCOM){
    return &(block->paso[dataID]);
  }
  return NULL;
}
