###############################################################
#
#
#
#
###############################################################
$KCODE = "SJIS"
#---------------------------------------------------
#---------------------------------------------------
def read_table infile
	mml = Array.new
	area_count = 0
	mml_count = 0
	now_id = nil
	line = infile.gets	#	読み飛ばし
	while line = infile.gets
		#p "#{line}"
		m = /(MMLID_\w+)\W(\d+)\W([0-9A-Z_]+)\b/.match line
    #p m
		if m == nil 
			puts "NOT HIT!"
			break
		else
			areaID = m[1]
			mmlID = m[3]
			unless mmlID =~ /^[0-9A-Z_]+$/
				STDERR.puts "動作モデルIDが正しくありません！：#{mmlID}\n"
				exit 1
			end
			if now_id == nil
				now_id = areaID
				mml[area_count] = Array.new
				mml[area_count][mml_count] = areaID
			elsif now_id != areaID
				area_count += 1
				mml_count = 0
				now_id = areaID
				mml[area_count] = Array.new
				mml[area_count][mml_count] = areaID
			end
			mml_count += 1
			mml[area_count][mml_count] = mmlID
		end
	end
	return mml
end

#---------------------------------------------------
#---------------------------------------------------
def make_asm_hash ar, hash, sym
  if ar.length > 30 then
    printf("ERROR:管理できる人数を超えています %d\n", ar.length);
    exit -1
  end

  ary = Array.new

	ar.each{|id|
#		printf("\t%s\n", id)
    ary << id
	}
  hash[sym] = ary
end

def MakeObjCodeVec( file, vec )
  entry = false
  while line = file.gets
    column = line.split ","
    if entry == true then
      vec << column[0].chomp("\n").chomp("\r")
      entry = false
    else
      if column[0].index("#OBJ") == 0 then
        entry = true
      else
        entry = false
      end
    end
  end  
end


#MEVファイルを開いて、ＯＢＪコードをエリア別に格納していく
def MakeObjAry(zone, vec)
  pass = "../eventdata/data/" + zone + ".mev"
  mev = open(pass,"r")
  MakeObjCodeVec( mev, vec )
  
  #重複削除
  vec.uniq!
end




#---------------------------------------------------
#---------------------------------------------------
#入力ファイルから定義を読み込む
infile = File.open(ARGV[0],"r")
mml = read_table infile
infile.close

#ダミー用定義を追加
mml << ["mmlid_noentry"]

AreaHash = Hash.new
AreaHash.clear

linecount = 0
mml.each{|ar|
	sym = ar[0].sub(/MMLID_AREA/,"AREA_ID")

#  printf("%s\n",sym)
	#定義ごとにファイル生成
	make_asm_hash ar[1..ar.length - 1], AreaHash, sym

	linecount += 1
}

ary = Array.new

#ゾーンテーブルを読み込む
zone_csv = open(ARGV[1],"r")

#一行読み飛ばし
zone_csv.gets
while line = zone_csv.gets
  #END検出
  if line =~/^END/ then
		break
	end
  column = line.split ","

  #マトリックスＩＤが「wb」の項を対称にする
  if column[4] == "wb" then
    ary.clear
    #エリアを取得
    area = column[2]
    #ゾーン取得
    zone = column[0]

    MakeObjAry(zone, ary)

    target = AreaHash[area]

    printf("zone %s  %s search..\n",zone, area)

    #作成した配列要素が全て、エリアに入っているかを調べる
    ary.each{|i|
      rc = target.include?(i)
      if rc == false then
        printf("ERROR:%s not!\n",i);
        exit -1
      else
        printf("%s exist!\n",i);
      end
    }
  end  
end


