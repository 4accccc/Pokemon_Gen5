
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

# ヒントタイプ定義(プログラム内で直接記述しているので未使用)
MDL_HINT_CONDITION = Hash.new
MDL_HINT_CONDITION["なし"]                 = "MEDALDATA_HINT_NONE"
MDL_HINT_CONDITION["最初から"]             = "MEDALDATA_HINT_FIRST"
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
//  record.hに定義されている、10000からナンバリングしている
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

EOS

# メダルレコードフッター部コメント
MDLREC_CMT_END = <<-EOS

#ifndef __ASM_NO_DEF_  //これ以降はアセンブラでは無視
typedef  int MedalID;
#endif __ASM_NO_DEF_

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


# レコード定義ファイルコメント
GRATYPE_CMT = <<-EOS
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
##
##
##   @file		%s
##   @brief   グラフィックタイプ定義ファイル 
##   @author	Masayuki Onoue
##   @date		2011.01.14
##
##  **{ /resource/medal/converter/gratype_hash_maker.rbが出力したデータです }**
##  
##
##]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

GRAPHICTYPE_HASH = {
EOS
