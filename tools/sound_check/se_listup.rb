require 'fileutils'
# SE の全リストを作成
all_se_list = Array.new
file = File::open( "../../prog/include/sound/swan_sound_data.sadl", "r" )
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
file_list += Dir.glob( "../../resource/fld3d_ci_effect/ci_se_tbl.cdat" ) # 3Dカットイン
file_list += Dir.glob( "../../resource/demo3d/data/*.cdat" ) # 3Dデモ
file_list += Dir.glob( "../../resource/pokewood_cutin/pokewood_cutin_data.xls" ) # ポケウッドカットイン
#file_list += Dir.glob( "../../resource/iss/**/*.xls" ) # ISS(BGMのみ)
#file_list += Dir.glob( "../../resource/**/*.xls" ) # エクセル
file_list.each do |filename|
  puts filename
  default_name = filename.clone
  is_excel = false
  
  # エクセルファイルの場合はコンバート後、検索をする
  if File::extname(filename) == ".xls" then
    cp_file_name = "./tmp/"+File::basename(filename)
    out_file_name = "./tmp/#{ File::basename(filename,".*") }.txt"
    FileUtils.cp(filename, cp_file_name )
    command = "ruby ../exceltool/xls2xml/tab_out_direct.rb #{ "./tmp/"+File::basename(filename) } > #{ out_file_name }"
    res = `#{ command }`
    filename = out_file_name
    is_excel = true
  end

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
        # エクセルファイルの場合はコンバート前のファイル名を出力
        if is_excel == false then
          se_hash[ label ] << filename
        else
          se_hash[ label ] << default_name
        end
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

# 使っていないもののみ出力
out_file = File.open( "no_use_list.txt", "w" )
# 件数を出力
no_use_cnt = 0
se_hash.each_key do |label|
  if se_hash[label].size == 0 then
    no_use_cnt+=1
  end
end
out_file.puts "未使用SE件数：%d\n"%no_use_cnt

se_hash.each_key do |label|
  if se_hash[label].size == 0 then
    out_file.puts( "#{label}\n" )
  end
end
out_file.close

