//======================================================================
/**
 * @file  musical_script_command.h.h
 * @brief ミュージカルスクリプト用のマクロ定義ファイル
 * @author  Nobuhiko Ariizumi
 * @date  2009/04/08
 */
//======================================================================

#ifndef MUSICAL_SCRIPT_COMMAND_H__
#define MUSICAL_SCRIPT_COMMAND_H__


#endif MUSICAL_SCRIPT_COMMAND_H__

//命令シンボル宣言
//COMMAND_START
#define SCRIPT_ENUM_ScriptFinish    (0)
#define SCRIPT_ENUM_FrameWait       (1)
#define SCRIPT_ENUM_FrameWaitTime     (2)
#define SCRIPT_ENUM_SyncScript      (3)
#define SCRIPT_ENUM_CurtainUp       (4)
#define SCRIPT_ENUM_CurtainDown     (5)
#define SCRIPT_ENUM_CurtainMove     (6)
#define SCRIPT_ENUM_StageMove       (7)
#define SCRIPT_ENUM_StageChangeBg     (8)
#define SCRIPT_ENUM_PokeShow      (9)
#define SCRIPT_ENUM_PokeDir       (10)
#define SCRIPT_ENUM_PokeMove      (11)
#define SCRIPT_ENUM_PokeMoveOffset    (12)
#define SCRIPT_ENUM_PokeStopAnime     (13)
#define SCRIPT_ENUM_PokeStartAnime    (14)
#define SCRIPT_ENUM_PokeChangeAnime   (15)
#define SCRIPT_ENUM_PokeActionJump    (16)
#define SCRIPT_ENUM_PokeMngSetFlag    (17)
#define SCRIPT_ENUM_ObjectCreate    (18)
#define SCRIPT_ENUM_ObjectDelete    (19)
#define SCRIPT_ENUM_ObjectShow      (20)
#define SCRIPT_ENUM_ObjectHide      (21)
#define SCRIPT_ENUM_ObjectMove      (22)
#define SCRIPT_ENUM_EffectCreate    (23)
#define SCRIPT_ENUM_EffectDelete    (24)
#define SCRIPT_ENUM_EffectStart     (25)
#define SCRIPT_ENUM_EffectStop      (26)
#define SCRIPT_ENUM_EffectRepeatStart (27)
#define SCRIPT_ENUM_LightShowCircle   (28)
#define SCRIPT_ENUM_LightHide       (29)
#define SCRIPT_ENUM_LightMove       (30)
#define SCRIPT_ENUM_LightMoveTrace    (31)
#define SCRIPT_ENUM_LightColor      (32)
#define SCRIPT_ENUM_MessageShow     (33)
#define SCRIPT_ENUM_MessageHide     (34)
#define SCRIPT_ENUM_MessageColor    (35)
#define SCRIPT_ENUM_MessageColor    (35)
#define SCRIPT_ENUM_BgmStart        (36)
#define SCRIPT_ENUM_BgmStop         (37)
#define SCRIPT_ENUM_PokeActionRotate    (38)
#define SCRIPT_ENUM_PokeSetFrontBack    (39)
#define SCRIPT_ENUM_PokeDispItem    (40)
#define SCRIPT_ENUM_PokeTransEffect    (41)
#define SCRIPT_ENUM_PokeAttentionOn    (42)
#define SCRIPT_ENUM_PokeAttentionOff   (43)
#define SCRIPT_ENUM_StageStartMainPart   (44)
#define SCRIPT_ENUM_StageFinishMainPart   (45)
#define SCRIPT_ENUM_PokeActionComeNearToTop   (46)
#define SCRIPT_ENUM_Label   (47)
#define SCRIPT_ENUM_SeqBgmStart  (48)
#define SCRIPT_ENUM_SeqBgmStop   (49)
#define SCRIPT_ENUM_PokeAppealScript (50)
#define SCRIPT_ENUM_PokeSubScript (51)
#define SEQ_END             (52)

#ifndef __C_NO_DEF_

//勝手にパディングを入れられないようにする
  .option alignment off
  
#pragma mark [>System Command
//命令マクロ定義
//======================================================================
/**
 * @brief システム：ミュージカル終了
 *
 * #param_num 0
 */
//======================================================================
  .macro  ComScriptFinish
  .short  SCRIPT_ENUM_ScriptFinish
  .endm

//======================================================================
/**
 * @brief システム：指定フレームウェイト
 *
 * #param_num 1
 * @param frame フレーム数
 *
 * #param VALUE_INT
 */
//======================================================================
  .macro  ComFrameWait  frame
  .short  SCRIPT_ENUM_FrameWait
  .long \frame
  .endm

//======================================================================
/**
 * @brief システム：指定までフレームウェイト
 *
 * #param_num 1
 * @param frame フレーム数
 *
 * #param VALUE_INT
 */
//======================================================================
  .macro  ComFrameWaitTime  frame
  .short  SCRIPT_ENUM_FrameWaitTime
  .long \frame
  .endm

//======================================================================
/**
 * @brief システム：スクリプト全体で同期を取る
 *
 * #param_num 1
 * @param comment コメント(仮
 *
 * #param VALUE_INT
 *
 */
//======================================================================
  .macro  ComSyncScript comment
  .short  SCRIPT_ENUM_SyncScript
  .long \comment
  .endm



#pragma mark [>Curtain Command
//======================================================================
/**
 * @brief カーテン：幕を上げる(固定速度
 *
 * #param_num 1
 * @param autoWait 自動フレームウェイト
 *
 * #param COMBOBOX_TEXT ON OFF
 * #param COMBOBOX_VALUE  1 0
 */
//======================================================================
  .macro  ComCurtainUp autoWait
  .short  SCRIPT_ENUM_CurtainUp
  .long   \autoWait
  .endm

//======================================================================
/**
 * @brief カーテン：幕を下げる(固定速度
 *
 * #param_num 1
 * @param autoWait 自動フレームウェイト
 *
 * #param COMBOBOX_TEXT ON OFF
 * #param COMBOBOX_VALUE  1 0
 */
//======================================================================
  .macro  ComCurtainDown autoWait
  .short  SCRIPT_ENUM_CurtainDown
  .long   \autoWait
  .endm

//======================================================================
/**
 * @brief カーテン：幕を動かす(速度設定可
 *
 * #param_num 3
 * @param frame フレーム数
 * @param heaight Y座標
 * @param autoWait 自動フレームウェイト
 *
 * #param VALUE_INT frame
 * #param VALUE_INT posY
 * #param COMBOBOX_TEXT ON OFF
 * #param COMBOBOX_VALUE  1 0
 */
//======================================================================
  .macro  ComCurtainMove  frame height autoWait
  .short  SCRIPT_ENUM_CurtainMove
  .long \frame
  .long \height
  .long   \autoWait
  .endm

#pragma mark [>Stage Command
//======================================================================
/**
 * @brief ステージ：舞台のスクロール
 *
 * #param_num 3
 * @param frame フレーム数
 * @param pos   X座標
 * @param autoWait 自動フレームウェイト
 *
 * #param VALUE_INT frame
 * #param VALUE_INT posX
 * #param COMBOBOX_TEXT ON OFF
 * #param COMBOBOX_VALUE  1 0
 */
//======================================================================
  .macro  ComStageMove  frame pos autoWait
  .short  SCRIPT_ENUM_StageMove
  .long \frame
  .long \pos
  .long   \autoWait
  .endm

//======================================================================
/**
 * @brief ステージ：背景の読み替え
 *
 * #param_num 1
 * @param bgNo  背景番号
 *
 * #param VALUE_INT bgNo
 */
//======================================================================
  .macro  ComStageChangeBg  bgNo
  .short  SCRIPT_ENUM_StageChangeBg
  .long \bgNo
  .endm

//======================================================================
/**
 * @brief ステージ：メインパートの開始
 *
 * #param_num 0
 *
 */
//======================================================================
  .macro  ComStageStartMainPart
  .short  SCRIPT_ENUM_StageStartMainPart
  .endm

//======================================================================
/**
 * @brief ステージ：メインパートの終了
 *
 * #param_num 0
 *
 */
//======================================================================
  .macro  ComStageFinishMainPart
  .short  SCRIPT_ENUM_StageFinishMainPart
  .endm

#pragma mark [>Pokemon Command
//======================================================================
/**
 * @brief ポケモン：表示切替
 *
 * #param_num 2
 * @param pokeNo  ポケモン番号(-1で事前登録対象)
 * @param flg   ON/OFF
 *
 * #param VALUE_INT pokeNo
 * #param COMBOBOX_TEXT OFF ON
 * #param COMBOBOX_VALUE  0 1
 */
//======================================================================
  .macro  ComPokeShow pokeNo  flg
  .short  SCRIPT_ENUM_PokeShow
  .long \pokeNo
  .long \flg
  .endm

//======================================================================
/**
 * @brief ポケモン：方向設定
 *
 * #param_num 2
 * @param pokeNo  ポケモン番号(-1で事前登録対象)
 * @param dir   向き(左：右)
 *
 * #param VALUE_INT pokeNo
 * #param COMBOBOX_TEXT 左  右
 * #param COMBOBOX_VALUE  0 1
 */
//======================================================================
  .macro  ComPokeDir  pokeNo  dir
  .short  SCRIPT_ENUM_PokeDir
  .long \pokeNo
  .long \dir
  .endm

//======================================================================
/**
 * @brief ポケモン：移動
 *
 * #param_num 4
 * @param pokeNo  ポケモン番号(-1で事前登録対象)
 * @param frame フレーム数
 * @param pos   座標
 * @param autoWait 自動フレームウェイト
 *
 * #param VALUE_INT pokeNo
 * #param VALUE_INT frame
 * #param VALUE_VECFX32 posX posY posZ
 * #param COMBOBOX_TEXT ON OFF
 * #param COMBOBOX_VALUE  1 0
 */
//======================================================================
  .macro  ComPokeMove pokeNo  frame posX  posY  posZ autoWait
  .short  SCRIPT_ENUM_PokeMove
  .long \pokeNo
  .long \frame
  .long \posX
  .long \posY
  .long \posZ
  .long   \autoWait
 .endm

//======================================================================
/**
 * @brief ポケモン：移動(相対座標)
 *
 * #param_num 4
 * @param pokeNo  ポケモン番号(-1で事前登録対象)
 * @param frame フレーム数
 * @param offset  オフセット
 * @param autoWait 自動フレームウェイト
 *
 * #param VALUE_INT pokeNo
 * #param VALUE_INT frame
 * #param VALUE_VECFX32 offsetX offsetY offsetZ
 * #param COMBOBOX_TEXT ON OFF
 * #param COMBOBOX_VALUE  1 0
 */
//======================================================================
  .macro  ComPokeMoveOffset pokeNo  frame offsetX offsetY offsetZ autoWait
  .short  SCRIPT_ENUM_PokeMoveOffset
  .long \pokeNo
  .long \frame
  .long \offsetX
  .long \offsetY
  .long \offsetZ
  .long   \autoWait
  .endm

//======================================================================
/**
 * @brief ポケモン：アニメ停止
 *
 * #param_num 1
 * @param pokeNo  ポケモン番号(-1で事前登録対象)
 *
 * #param VALUE_INT pokeNo
 */
//======================================================================
  .macro  ComPokeStopAnime  pokeNo
  .short  SCRIPT_ENUM_PokeStopAnime
  .long \pokeNo
  .endm

//======================================================================
/**
 * @brief ポケモン：アニメ開始
 *
 * #param_num 1
 * @param pokeNo  ポケモン番号(-1で事前登録対象)
 *
 * #param VALUE_INT pokeNo
 */
//======================================================================
  .macro  ComPokeStartAnime pokeNo
  .short  SCRIPT_ENUM_PokeStartAnime
  .long \pokeNo
  .endm

//======================================================================
/**
 * @brief ポケモン：アニメ変更
 *
 * #param_num 2
 * @param pokeNo  ポケモン番号(-1で事前登録対象)
 * @param animeNo アニメ番号
 *
 * #param VALUE_INT pokeNo
 * #param VALUE_INT animeNo
 */
//======================================================================
  .macro  ComPokeChangeAnime  pokeNo  animeNo
  .short  SCRIPT_ENUM_PokeChangeAnime
  .long \pokeNo
  .long \animeNo
  .endm
  
//======================================================================
/**
 * @brief ポケモン：正面・背面切り替え
 *
 * #param_num 2
 * @param pokeNo  ポケモン番号(-1で事前登録対象)
 * @param dir   向き(正面：背面)
 *
 * #param VALUE_INT pokeNo
 * #param COMBOBOX_TEXT 正面  背面
 * #param COMBOBOX_VALUE  1 0
 */
//======================================================================
  .macro  ComPokeSetFrontBack  pokeNo  dir
  .short  SCRIPT_ENUM_PokeSetFrontBack
  .long \pokeNo
  .long \dir
  .endm
  
//======================================================================
/**
 * @brief ポケモン：装備品表示切り替え
 *
 * #param_num 2
 * @param pokeNo  ポケモン番号(-1で事前登録対象)
 * @param flg   ON/OFF
 *
 * #param VALUE_INT pokeNo
 * #param COMBOBOX_TEXT OFF ON
 * #param COMBOBOX_VALUE  0 1
 */
//======================================================================
  .macro  ComPokeSetDispItem pokeNo  flg
  .short  SCRIPT_ENUM_PokeDispItem
  .long \pokeNo
  .long \flg
  .endm
  
//======================================================================
/**
 * @brief ポケモン：変身エフェクト再生
 *
 * #param_num 1
 * @param pokeNo  ポケモン番号(-1で事前登録対象)
 *
 * #param VALUE_INT pokeNo
 */
//======================================================================
  .macro  ComPokeTransEffect pokeNo
  .short  SCRIPT_ENUM_PokeTransEffect
  .long \pokeNo
  .endm

//======================================================================
/**
 * @brief ポケモン：注目ポケモン設定(解除前に連続して呼べます
 *
 * #param_num 1
 * @param pokeNo  ポケモン番号(-1不可)
 *
 * #param VALUE_INT pokeNo
 */
//======================================================================
  .macro  ComPokeAttentionOn pokeNo
  .short  SCRIPT_ENUM_PokeAttentionOn
  .long \pokeNo
  .endm

//======================================================================
/**
 * @brief ポケモン：注目ポケモン解除
 *
 * #param_num 0
 *
 */
//======================================================================
  .macro  ComPokeAttentionOff
  .short  SCRIPT_ENUM_PokeAttentionOff
  .endm

//======================================================================
/**
 * @brief ポケモン：アピールスクリプト実行
 *
 * #param_num 2
 * @param pokeNo ポケモン番号(-1不可)
 * @param scriptNo スクリプト番号
 *
 * #param VALUE_INT pokeNo
 * #param VALUE_INT scriptNo
 */
//======================================================================
  .macro  ComPokeAppealScript pokeNo scriptNo
  .short  SCRIPT_ENUM_PokeAppealScript
  .long   \pokeNo
  .long   \scriptNo
  .endm


//======================================================================
/**
 * @brief ポケモン：サブスクリプト実行
 *
 * #param_num 2
 * @param pokeNo ポケモン番号(-1で事前登録対象)
 * @param scriptNo スクリプト番号
 *
 * #param VALUE_INT pokeNo
 * #param VALUE_INT scriptNo
 */
//======================================================================
  .macro  ComPokeSubScript pokeNo scriptNo 
  .short  SCRIPT_ENUM_PokeSubScript
  .long   \pokeNo
  .long   \scriptNo
  .endm


#pragma mark [>Pokemon Action Command
//======================================================================
/**
 * @brief ポケモンアクション：跳ねる
 *
 * #param_num 5
 * @param pokeNo  ポケモン番号(-1で事前登録対象)
 * @param interval  間隔
 * @param repeat  回数
 * @param height  高さ
 * @param autoWait 自動フレームウェイト
 *
 * #param VALUE_INT pokeNo
 * #param VALUE_INT interval
 * #param VALUE_INT repeat
 * #param VALUE_FX32
 * #param COMBOBOX_TEXT ON OFF
 * #param COMBOBOX_VALUE  1 0
 */
//======================================================================
  .macro  ComPokeActionJump pokeNo  interval  repeat  height autoWait
  .short  SCRIPT_ENUM_PokeActionJump
  .long \pokeNo
  .long \interval
  .long \repeat
  .long \height
  .long   \autoWait
  .endm

//======================================================================
/**
 * @brief ポケモンアクション：回る
 *
 * #param_num 5
 * @param pokeNo  ポケモン番号(-1で事前登録対象)
 * @param frame   フレーム数
 * @param angle   開始角度(360度計算
 * @param angle   回転角度(マイナスで逆回転・720で2回転
 * @param autoWait 自動フレームウェイト
 *
 * #param VALUE_INT pokeNo
 * #param VALUE_INT frame
 * #param VALUE_INT startAngle
 * #param VALUE_INT rotateAngle
 * #param COMBOBOX_TEXT ON OFF
 * #param COMBOBOX_VALUE  1 0
 */
//======================================================================
  .macro  ComPokeActionRotate pokeNo  frame  startAngle rotateAngle autoWait
  .short  SCRIPT_ENUM_PokeActionRotate
  .long \pokeNo
  .long \frame
  .long \startAngle
  .long \rotateAngle
  .long   \autoWait
  .endm

//======================================================================
/**
 * @brief ポケモンアクション：トップに寄る
 *
 * #param_num 2
 * @param frame   フレーム数
 * @param autoWait 自動フレームウェイト
 *
 * #param VALUE_INT frame
 * #param COMBOBOX_TEXT ON OFF
 * #param COMBOBOX_VALUE  1 0
 */
//======================================================================
  .macro  ComPokeActionComeNearToTop frame autoWait
  .short  SCRIPT_ENUM_PokeActionComeNearToTop
  .long \frame
  .long   \autoWait
  .endm
  

#pragma mark [>Pokemon Manage Command
//======================================================================
/**
 * @brief ポケモン管理：動作番号設定
 *
 * #param_num 4
 * @param flg1  ポケモン１のフラグ
 * @param flg2  ポケモン２のフラグ
 * @param flg3  ポケモン３のフラグ
 * @param flg4  ポケモン４のフラグ
 *
 * #param COMBOBOX_TEXT OFF ON
 * #param COMBOBOX_VALUE  0 1
 * #param COMBOBOX_TEXT OFF ON
 * #param COMBOBOX_VALUE  0 1
 * #param COMBOBOX_TEXT OFF ON
 * #param COMBOBOX_VALUE  0 1
 * #param COMBOBOX_TEXT OFF ON
 * #param COMBOBOX_VALUE  0 1
 */
//======================================================================
  .macro  ComPokeMngSetFlg  flg1  flg2  flg3  flg4
  .short  SCRIPT_ENUM_PokeMngSetFlag
  .long \flg1
  .long \flg2
  .long \flg3
  .long \flg4
  .endm


#pragma mark [>Object Command
//======================================================================
/**
 * @brief オブジェクト：作成
 *
 * #param_num 2
 * @param objNo オブジェクト管理番号
 * @param type  オブジェクト番号
 *
 * #param VALUE_INT objNo
 * #param VALUE_INT objType
 */
//======================================================================
  .macro  ComObjectCreate objNo type
  .short  SCRIPT_ENUM_ObjectCreate
  .long \objNo
  .long \type
  .endm

//======================================================================
/**
 * @brief オブジェクト：削除
 *
 * #param_num 1
 * @param objNo オブジェクト管理番号
 *
 * #param VALUE_INT objNo
 */
//======================================================================
  .macro  ComObjectDelete objNo
  .short  SCRIPT_ENUM_ObjectDelete
  .long \objNo
  .endm

//======================================================================
/**
 * @brief オブジェクト：表示
 *
 * #param_num 1
 * @param objNo オブジェクト管理番号
 *
 * #param VALUE_INT objNo
 */
//======================================================================
  .macro  ComObjectShow objNo
  .short  SCRIPT_ENUM_ObjectShow
  .long \objNo
  .endm

//======================================================================
/**
 * @brief オブジェクト：非表示
 *
 * #param_num 1
 * @param objNo オブジェクト管理番号
 *
 * #param VALUE_INT objNo
 */
//======================================================================
  .macro  ComObjectHide objNo
  .short  SCRIPT_ENUM_ObjectHide
  .long \objNo
  .endm

//======================================================================
/**
 * @brief オブジェクト：移動
 *
 * #param_num 3
 * @param objNo オブジェクト管理番号
 * @param frame フレーム数
 * @param pos   座標
 *
 * #param VALUE_INT objNo
 * #param VALUE_INT frame
 * #param VALUE_VECFX32 posX posY posZ
 */
//======================================================================
  .macro  ComObjectMove objNo frame posX  posY  posZ
  .short  SCRIPT_ENUM_ObjectMove
  .long \objNo
  .long \frame
  .long \posX
  .long \posY
  .long \posZ
  .endm


#pragma mark [>Effect Command
//======================================================================
/**
 * @brief エフェクト：作成
 *
 * #param_num 2
 * @param effNo エフェクト管理番号
 * @param type  エフェクト番号
 *
 * #param VALUE_INT effectNo
 * #param VALUE_INT effectType
 */
//======================================================================
  .macro  ComEffectCreate effNo type
  .short  SCRIPT_ENUM_EffectCreate
  .long \effNo
  .long \type
  .endm

//======================================================================
/**
 * @brief エフェクト：破棄
 *
 * #param_num 1
 * @param effNo エフェクト管理番号
 *
 * #param VALUE_INT effectNo
 */
//======================================================================
  .macro  ComEffectDelete effNo
  .short  SCRIPT_ENUM_EffectDelete
  .long \effNo
  .endm

//======================================================================
/**
 * @brief エフェクト：再生
 *
 * #param_num 3
 * @param effNo エフェクト管理番号
 * @param emitNo  エミッタ番号
 * @param pos   座標
 *
 * #param VALUE_INT effectNo
 * #param VALUE_INT emmiterNo
 * #param VALUE_VECFX32 posX posY posZ
 */
//======================================================================
  .macro  ComEffectStart  effNo emitNo  posX  posY  posZ
  .short  SCRIPT_ENUM_EffectStart
  .long \effNo
  .long \emitNo
  .long \posX
  .long \posY
  .long \posZ
  .endm

//======================================================================
/**
 * @brief エフェクト：停止
 *
 * #param_num 2
 * @param effNo エフェクト管理番号
 * @param emitNo  エミッタ番号
 *
 * #param VALUE_INT effectNo
 * #param VALUE_INT emmiterNo
 */
//======================================================================
  .macro  ComEffectStop effNo emitNo
  .short  SCRIPT_ENUM_EffectStop
  .long \effNo
  .long \emitNo
  .endm

//======================================================================
/**
 * @brief エフェクト：連続再生
 *
 * #param_num 6
 * @param effNo エフェクト管理番号
 * @param emitNo  エミッタ番号
 * @param interval  間隔
 * @param repeat  回数
 * @param startPos  開始座標
 * @param endPos  終了座標(必ず開始座標が小さくなるようにしてください
 *
 * #param VALUE_INT effectNo
 * #param VALUE_INT emmiterNo
 * #param VALUE_INT interval
 * #param VALUE_INT repeat
 * #param VALUE_VECFX32 startPosX startPosY startPosZ
 * #param VALUE_VECFX32 endPosX endPosY endPosZ
 */
//======================================================================
  .macro  ComEffectRepeatStart  effNo emitNo  interval  repeat  posX1 posY1 posZ1 posX2 posY2 posZ2
  .short  SCRIPT_ENUM_EffectRepeatStart
  .long \effNo
  .long \emitNo
  .long \interval
  .long \repeat
  .long \posX1
  .long \posY1
  .long \posZ1
  .long \posX2
  .long \posY2
  .long \posZ2
  .endm

#pragma mark [>Light Command
//======================================================================
/**
 * @brief スポットライト：作成
 *
 * #param_num 2
 * @param lightNo スポットライト管理番号
 * @param radian  半径(未動作)
 *
 * #param VALUE_INT lightNo
 * #param VALUE_INT radian(invalid)
 */
//======================================================================
  .macro  ComLightShowCircle  lightNo radian
  .short  SCRIPT_ENUM_LightShowCircle
  .long \lightNo
  .long \radian
  .endm

//======================================================================
/**
 * @brief スポットライト：破棄
 *
 * #param_num 1
 * @param lightNo スポットライト管理番号
 *
 * #param VALUE_INT lightNo
 */
//======================================================================
  .macro  ComLightHide  lightNo
  .short  SCRIPT_ENUM_LightHide
  .long \lightNo
  .endm

//======================================================================
/**
 * @brief スポットライト：移動
 *
 * #param_num 3
 * @param lightNo スポットライト管理番号
 * @param frame フレーム数
 * @param pos   座標(Z無効)
 *
 * #param VALUE_INT lightNo
 * #param VALUE_INT frame
 * #param VALUE_VECFX32 posX posY posZ(invalid!)
 */
//======================================================================
  .macro  ComLightMove  lightNo frame posX  posY  posZ
  .short  SCRIPT_ENUM_LightMove
  .long \lightNo
  .long \frame
  .long \posX
  .long \posY
  .long \posZ
  .endm

//======================================================================
/**
 * @brief スポットライト：ポケモン追従移動
 *
 * #param_num 4
 * @param lightNo スポットライト管理番号
 * @param pokeNo  ポケモン番号(-1不可)
 * @param frame フレーム数
 * @param ofs   オフセット(Z無効)
 *
 * #param VALUE_INT lightNo
 * #param VALUE_INT pokeNo
 * #param VALUE_INT frame
 * #param VALUE_VECFX32 offsetX offsetY offsetZ(invalid!)
 */
//======================================================================
  .macro  ComLightMoveTrace lightNo pokeNo  frame ofsX  ofsY  ofsZ
  .short  SCRIPT_ENUM_LightMoveTrace
  .long \lightNo
  .long \pokeNo
  .long \frame
  .long \ofsX
  .long \ofsY
  .long \ofsZ
  .endm

//======================================================================
/**
 * @brief スポットライト：色設定
 *
 * #param_num 5
 * @param lightNo スポットライト管理番号
 * @param colR  色：Ｒ
 * @param colG  色：Ｇ
 * @param colB  色：Ｂ
 * @param alpha 透明度
 *
 * #param VALUE_INT lightNo
 * #param VALUE_INT colR
 * #param VALUE_INT colG
 * #param VALUE_INT colB
 * #param VALUE_INT alpha
 */
//======================================================================
  .macro  ComLightColor lightNo colR  colG  colB  alpha
  .short  SCRIPT_ENUM_LightColor
  .long \lightNo
  .long \colR
  .long \colG
  .long \colB
  .long \alpha
  .endm

#pragma mark [>Message Command
//======================================================================
/**
 * @brief メッセージ：表示
 *
 * #param_num 2
 * @param msgNo メッセージ番号
 * @param speed メッセージ速度(1文字辺りのフレーム数)
 *
 * #param VALUE_INT messageNo
 * #param VALUE_INT messageSpeed
 */
//======================================================================
  .macro  ComMessageShow  msgNo speed
  .short  SCRIPT_ENUM_MessageShow
  .long \msgNo
  .long \speed
  .endm

//======================================================================
/**
 * @brief メッセージ：非表示
 *
 * #param_num 0
 *
 */
//======================================================================
  .macro  ComMessageHide  
  .short  SCRIPT_ENUM_MessageHide
  .endm

//======================================================================
/**
 * @brief メッセージ：色変更
 *
 * #param_num 3
 * @param colLetter 文字色
 * @param colShadow 影色
 * @param colBack 背景色
 *
 * #param VALUE_INT colLetter
 * #param VALUE_INT colShadow
 * #param VALUE_INT colBack
 */
//======================================================================
  .macro  ComMessageColor colLetter colShadow colBack
  .short  SCRIPT_ENUM_MessageColor
  .long \colLetter
  .long \colShadow
  .long \colBack
  .endm

#pragma mark [>Music Command
//======================================================================
/**
 * @brief BGM：再生
 *
 * #param_num 1
 * @param bgmNo BGM番号
 *
 * #param VALUE_INT bgmNo
 */
//======================================================================
  .macro  ComBgmStart bgmNo
  .short  SCRIPT_ENUM_BgmStart
  .long \bgmNo
  .endm

//======================================================================
/**
 * @brief BGM：停止
 *
 * #param_num 0
 *
 */
//======================================================================
  .macro  ComBgmStop
  .short  SCRIPT_ENUM_BgmStop
  .endm

//======================================================================
/**
 * @brief SEQ_BGM：再生
 *
 * #param_num 1
 * @param bgmNo BGM番号
 *
 * #param VALUE_INT bgmNo
 */
//======================================================================
  .macro  ComSeqBgmStart bgmNo
  .short  SCRIPT_ENUM_SeqBgmStart
  .long \bgmNo
  .endm

//======================================================================
/**
 * @brief SEQ_BGM：停止
 *
 * #param_num 0
 *
 */
//======================================================================
  .macro  ComSeqBgmStop
  .short  SCRIPT_ENUM_SeqBgmStop
  .endm

#pragma mark [>Other Command
//======================================================================
/**
 * @brief ■■■■■■ラベル■■■■■■
 *
 * #param_num 1
 * @param labelNo ラベル番号
 *
 * #param VALUE_INT labelNo
 */
//======================================================================
  .macro  ComSEQ_Label labelNo
  .short  SCRIPT_ENUM_Label
  .long \labelNo
  .endm

//======================================================================
/**
 * @brief ミュージカル終了(ダミー
 *
 * #param_num 0
 */
//======================================================================
  .macro  ComSEQ_END
  .short  SCRIPT_ENUM_ScriptFinish
  .endm

#endif //__C_NO_DEF_

