
# medal_data.xlsファイルの列番号を記入する
COL_NO                = 0
COL_ID                = 1
COL_NAME              = 2
COL_RECID             = 3
COL_COUNT             = 4
COL_HINT_CONDHITION   = 5 
COL_HINT_LEVEL        = 6 
COL_GRAPHIC_ID        = 7 
COL_EXPLANATION_KANA  = 8 
COL_EXPLANATION_KANZI = 9
COL_HINT_KANA         = 10
COL_HINT_KANZI        = 11 
COL_HINT_COMMENT      = 12

# グラフィックID定義
MDL_GRAPHIC_ID = Hash.new
MDL_GRAPHIC_ID["FIELD_LV1"]       = 0
MDL_GRAPHIC_ID["FIELD_LV2"]       = 1
MDL_GRAPHIC_ID["FIELD_LV3"]       = 2
MDL_GRAPHIC_ID["FIELD_LV4"]       = 3
MDL_GRAPHIC_ID["BATTLE_LV1"]      = 4
MDL_GRAPHIC_ID["BATTLE_LV2"]      = 5
MDL_GRAPHIC_ID["BATTLE_LV3"]      = 6
MDL_GRAPHIC_ID["BATTLE_LV4"]      = 7
MDL_GRAPHIC_ID["APPLICATION_LV1"] = 8
MDL_GRAPHIC_ID["APPLICATION_LV2"] = 9
MDL_GRAPHIC_ID["APPLICATION_LV3"] = 10
MDL_GRAPHIC_ID["APPLICATION_LV4"] = 11
MDL_GRAPHIC_ID["MISSION_LV1"]     = 12
MDL_GRAPHIC_ID["MISSION_LV2"]     = 13
MDL_GRAPHIC_ID["MISSION_LV3"]     = 14
MDL_GRAPHIC_ID["MISSION_LV4"]     = 15
MDL_GRAPHIC_ID["SPECIAL_LV1"]     = 16
MDL_GRAPHIC_ID["SPECIAL_LV2"]     = 17
MDL_GRAPHIC_ID["SPECIAL_LV3"]     = 18
MDL_GRAPHIC_ID["SPECIAL_LV4"]     = 19

# ヒントタイプ定義(プログラム内で直接記述しているので未使用)
MDL_HINT_CONDITION = Hash.new
MDL_HINT_CONDITION["なし"]                 = "HINT_CONDITION_NONE"
MDL_HINT_CONDITION["最初から"]             = "HINT_CONDITION_FIRST"
MDL_HINT_CONDITION["ヒントレベルから計算"] = "HINT_CONDITION_HINT_LEVEL"
MDL_HINT_CONDITION["プログラム側で開示"]   = "HINT_CONDITION_PROGRAM"

# ヒントタイプID
MDL_HINT_CONDITION_ID = Hash.new
MDL_HINT_CONDITION_ID["なし"]                 = 0
MDL_HINT_CONDITION_ID["最初から"]             = 1
MDL_HINT_CONDITION_ID["ヒントレベルから計算"] = 2
MDL_HINT_CONDITION_ID["プログラム側で開示"]   = 3


# メダルタイプヘッダ部コメント
MDLID_CMT = <<-EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//
//   @file		%s
//   @brief   メダルタイプ定義ファイル
//   @author	Masayuki Onoue
//   @date		2011.01.07
//
//  **{ /resource/medal/converter/medal_converter.rbが出力したデータです }**
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

EOS

# メダルレコードヘッダ部コメント
MDLREC_CMT = <<-EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//
//   @file		%s
//   @brief   メダル用レコードID定義ファイル
//   @author	Masayuki Onoue
//   @date		2011.01.07
//
//  **{ /resource/medal/converter/medal_converter.rbが出力したデータです }**
//  record.hに定義されている、SMALL_REC_MAX+10000からナンバリングしている
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

EOS

# メダルソートヘッダ部コメント
MDLSORT_CMT = <<-EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//
//   @file		%s
//   @brief   medalsortファイル(メダル名あいうえお順)
//   @author	Masayuki Onoue
//   @date		2011.01.07
//
//  **{ /resource/medal/converter/medal_converter.rbが出力したデータです }**
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

u32 MedalSortList[] = {
EOS


# レコード定義ファイルコメント
RECRBDEF_CMT = <<-EOS
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
##
##
##   @file		%s
##   @brief   レコード定義ファイル 
##   @author	Masayuki Onoue
##   @date		2011.01.11
##
##  **{ /resource/medal/converter/record_hash_maker.rbが出力したデータです }**
##  
##
##]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

RECORD_H_HASH = {
EOS
