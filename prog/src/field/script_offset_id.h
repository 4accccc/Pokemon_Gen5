//======================================================================
/**
 * @file  scr_offset.h
 * @bfief  スクリプトIDのオフセット定義
 * @author  Satoshi Nohara
 * @date  06.05.26
 */
//======================================================================
#ifndef SCR_OFFSET_H
#define SCR_OFFSET_H

//======================================================================
//  スクリプトIDオフセット
//
//  定義追加ルール
//  #define ID_"スクリプトファイル名"_OFFSET
//  #define ID_"スクリプトファイル名"_OFFSET_END
//
//  script.cの読み込むスクリプトファイル、
//  メッセージファイルを対応させて下さい。
//======================================================================
//#define SCRID_NULL                 (0) //ダミースクリプトID
#define ID_START_SCR_OFFSET          (1) //開始スクリプトIDのオフセット
#define ID_COMMON_SCR_OFFSET         (2000) //共通スクリプトIDのオフセット
#define ID_COMMON_SCR_OFFSET_END     (2499) //共通スクリプトIDのオフセット終了
#define ID_TRAINER_OFFSET            (3000) //トレーナースクリプトIDのオフセット
#define ID_TRAINER_OFFSET_END        (4999) //トレーナースクリプトIDのオフセット終了
#define ID_TRAINER_2VS2_OFFSET       (5000) //2vs2トレーナースクリプトIDのオフセット
#define ID_TRAINER_2VS2_OFFSET_END   (6999) //2vs2トレーナースクリプトIDのオフセット終了
#define ID_HIDE_ITEM_OFFSET          (8000) //隠しアイテムスクリプトIDのオフセット
#define ID_HIDE_ITEM_OFFSET_END      (8799) //隠しアイテムスクリプトIDのオフセット終了
#define	ID_INIT_SCR_OFFSET           (9600) //ゲーム開始スクリプトIDのオフセット
#define	ID_INIT_SCR_OFFSET_END (9699) //ゲーム開始スクリプトIDのオフセット終了

#endif  /* SCR_OFFSET_H */
