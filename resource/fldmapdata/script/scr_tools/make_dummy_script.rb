###############################################################
#
#   ダミースクリプトファイル生成
#
#   tamada GAMEFREAK inc.
#   2009.07.15
#
###############################################################

require "fileutils"

class FileNameError < Exception; end

SP_SCRIPT_DUMMY_STRING = <<DUMMY_STRING

//==============================================================
//
//              sp_ZONEIDSYMBOL.ev
//
//    特殊スクリプトファイル：ZONEIDSYMBOL用
//
//==============================================================

  .text

  .include  "scr_seq_def.h"
  .include  "ZONEIDSYMBOL_def.h"

//--------------------------------------------------------------
//              特殊スクリプト
//--------------------------------------------------------------
_SP_EVENT_DATA_END


DUMMY_STRING


SCRIPT_DUMMY_STRING = <<DUMMY_STRING

//======================================================================
//                ZONEIDSYMBOL.ev
//
//    スクリプトファイル：ZONEIDSYMBOL用
//
//======================================================================

  .text

  .include  "scr_seq_def.h"
  .include  "msg_ZONEIDSYMBOL.h"
  //.include  "ZONEIDSYMBOL.h"

//--------------------------------------------------------------
//               スクリプト本体
//--------------------------------------------------------------

//--------------------------------------------------------------
//               スクリプトテーブル
//--------------------------------------------------------------
_EVENT_DATA    ev_ZONEIDSYMBOL
_EVENT_DATA_END


/**************************************************************/
/*            ZONE_CHANGE_LABEL                               */
/**************************************************************/
//ev_zone_ZONEIDSYMBOL:
//  _END

/**************************************************************/
/*                                                            */
/**************************************************************/
ev_ZONEIDSYMBOL:
  //とりあえず最初のラベルだけ生成している
  _END

DUMMY_STRING


HEADER_DUMMY_STRING = <<DUMMY_STRING
//======================================================================
//                ZONEIDSYMBOL_def.h
//
//    スクリプトヘッダファイル：ZONEIDSYMBOL用
//
//======================================================================
#ifndef __ZONEIDSYMBOL_DEF_H__
#define __ZONEIDSYMBOL_DEF_H__

//スクリプトデータID定義
#define SCRID_ZONEIDSYMBOL_DATA_MAX   (0)   /* 最大数 */

#endif  /* __ZONEIDSYMBOL_DEF_H__ */

DUMMY_STRING



def make_dummy(path, sym, content)
  if FileTest.exists?(path) then
    puts "#{path} exists."
  else
    puts "スクリプトファイル#{path} が見つかりません。"
    #puts "scripts for #{sym} not found!!"
    File.open(path, "w"){|file|
      file.puts "#{content.gsub(/ZONEIDSYMBOL/,sym)}"
    }
    puts "スクリプトファイル#{path} を生成しました！！！."
  end
end


###############################################################
#
#   ここから本体
#
###############################################################
if ARGV.length == 0 then
  puts "Usage:"
  puts "ruby make_dummy_script.rb directory symbol"
  exit 0
end
directory = ARGV[0]
symbol = ARGV[1].downcase
unless FileTest.exists?(directory) then
  raise FileNameError, "#{directory}が存在しません"
end
unless FileTest.directory?(directory) then
  raise FileNameError, "#{directory}はディレクトリではありません"
end

filename = directory + "\\" + symbol + ".ev"
sp_filename = directory + "\\sp_" + symbol + ".ev"
def_filename = directory + "\\" + symbol + "_def.h"

make_dummy(filename, symbol, SCRIPT_DUMMY_STRING)
make_dummy(sp_filename, symbol, SP_SCRIPT_DUMMY_STRING)
make_dummy(def_filename, symbol, HEADER_DUMMY_STRING)



