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
file_list += Dir.glob( "../../resource/wazaeffect/**/*.esf" ) # 技エフェクト
file_list.each do |filename|
  puts filename
  file = File::open( filename, "r" )
  if File.file?( file ) == false then 
    file.close
    next 
  end 
  lines = file.readlines
  file.close

  lines.each do |line|
    while line =~ /(SEQ_SE_\w*)/
      label = $1
      if all_se_list.include?( label ) then
        se_hash[ label ] << filename
      end
      line.gsub!( label, "" )
    end
  end
end

# 出力
out_file = File.open( "result.txt", "w" )
all_se_list.each do |label|
  out_file.puts( "#{label}\r" )
  se_hash[ label ].each do |filename|
    out_file.puts( "\t#{filename}\r" )
  end
end
out_file.close
