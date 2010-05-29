# SE の全リストを作成
all_se_list = Array.new
file = File::open( "../../prog/include/sound/wb_sound_data.sadl", "r" )
lines = file.readlines
file.close

if lines then 
  lines.each do |line|
    if line =~ /(SEQ_SE_\w*)/ then 
      all_se_list << $1
    end
  end
end

# 全ての SE に対応するハッシュを作成
se_hash = Hash.new 
all_se_list.each do |label|
  se_hash[ label ] = Array.new
end

# 検索
file_list = Dir.glob( "../../prog/src/**/*" ) # プログラムソース
file_list += Dir.glob( "../../resource/fldmapdata/script/**/*.ev" ) # スクリプト
file_list.each do |filename|
  file = File::open( filename, "r" )
  lines = nil
  if File.file?( file ) then
    lines = file.readlines
  end
  file.close

  if lines then 
    lines.each do |line|
      if line =~ /(SEQ_SE_\w*)/ then 
        label = $1
        if all_se_list.include?( label ) then
          se_hash[ label ] << filename
        end
      end
    end
  end
end

# 出力
out_file = File.open( "result.txt", "w" )
all_se_list.each do |label|
  out_file.puts( label )
  se_hash[ label ].each do |filename|
    out_file.puts( "\t#{filename}" )
  end
end
out_file.close
