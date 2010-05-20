$KCODE = "SJIS"

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

AreaHash = Hash.new
AreaHash.clear

#ゾーンテーブルを読み込む
zone_csv = open(ARGV.shift,"r")

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
    #エリアを取得
    area = column[2]
    #ゾーン取得
    zone = column[0]
    #取得エリアはプッシュ済みか？
    
    if AreaHash.key?(area) == false then     #未プッシュ
      zone_ary =  Array.new
      zone_ary.clear
    else
      #ポップ
      zone_ary = AreaHash[area]
    end
    #取得ゾーンをプッシュ
    zone_ary << zone
    #プッシュ
    AreaHash[area] = zone_ary
  end  
end

#MEVファイルを開いて、ＯＢＪコードをエリア別に格納していく

AreaHash.each{|key,val|
  str = key + ".txt"
  text = open(str,"w")
  vec = Array.new
  vec.clear
  val.each{|zone|  
    pass = "../eventdata/data/" + zone + ".mev"
    mev = open(pass,"r")
    MakeObjCodeVec( mev, vec )
  }
  #重複削除
  vec.uniq!
  vec.each{|code|
    text.printf("%s\n", code)
  }
}
