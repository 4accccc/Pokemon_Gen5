#################################################################################
#
# @brief  イベントデータ チェックツール
#
# @file   event_check.rb
# @author obata
# @date   2010.04.24
#
#################################################################################
require "mev_analyzer.rb"
require "mev_parser.rb"
require "obj_event_section_parser.rb"
require "obj_event_parser.rb"

PATH_TO_DIR     = "../data/"
TARGET_FILENAME = "*.mev"
RESULT_FILENAME = "event_check_result.txt"

mev_files = Array.new


# すべての.mevファイルを読み込む
print( "< Load Start!! > --> " )
filenames = Dir[ PATH_TO_DIR + TARGET_FILENAME ]
filenames.each do |filename|

  print( "#{File.basename( filename )} --> " )
  STDOUT.flush

  mev_data = MEVFile.new
  mev_data.load( filename )
  mev_files << mev_data

  puts( "OK!" )
  STDOUT.flush

end
puts( "< Load Finish!! >" )


# 解析
mev_analyzer = MEVAnalyzer.new
mev_analyzer.analyze_flag_trainer = true
mev_analyzer.analyze( mev_files )
