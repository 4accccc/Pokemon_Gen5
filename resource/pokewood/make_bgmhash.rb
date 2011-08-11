#------------------------------------------------------------
# sound_data.sadlからBGM定義だけを取り出してhashnoにする
#
# 2011/08/11 by mori
#
#------------------------------------------------------------

OPEN_FILE_NAME = "../sound/swan_sound_data.sadl"
MAKE_HASH_NAME = "$bgm_hash"


if ARGV[0]==nil then
  printf("出力ファイル名を指定してください\n")
  exit
end

bgm_hash = Hash.new

# sound_data.sadlを1行ずつ解釈
File.open(OPEN_FILE_NAME){|file|
  file.each{|line|
    #「SEQ_BGM_*」のみを取り出してハッシュを作成
    words = line.split(nil)
    if words[1] =~ /\ASEQ_BGM_/ then
      bgm_hash[words[1]] = words[2].to_i
    end
  }
}

#p bgm_hash


# ハッシュファイルを出力

File.open( ARGV[0], "w"){|file|
  file.printf("#-----------------------------------------------------------\n")
  file.printf("# BGM_HASH \n")
  file.printf("# 自動生成ファイルです \n")
  file.printf("#-----------------------------------------------------------\n")
  file.printf("%s = {\n", MAKE_HASH_NAME)

  #ハッシュ表の数だけ出力
  bgm_hash.each{|key, value|
    file.printf("    \"%s\" => %d,\n", key, value )
  }

  file.printf("}\n")
}