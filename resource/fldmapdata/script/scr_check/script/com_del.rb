#コメントを削除して、再出力する saito

$KCODE = "SJIS"

#ret: 0ヒット無し　1:「//」がヒット 2:「/*」がヒット
def CheckCommentStartCode(sra_sta, sra_sra)
  if (sra_sta == nil)&&(sra_sra == nil) then
    #コメント無し
    return 0
  elsif (sra_sta == nil)&&(sra_sra != nil) then
    #「//」がヒット
    return 1
  elsif (sra_sta != nil)&&(sra_sra == nil) then
    #「/*」がヒット
    return 2
  elsif (sra_sta > sra_sra ) then
    #「//」が優先ヒット
    return 1
  elsif (sra_sta < sra_sra ) then
    #「/*」が優先ヒット
    return 2
  end

  return 0
end

def CheckComSraSta_StaSra(str, sra_sta)
  sra_sta_local = sra_sta
  begin
    sta_sra = str.index("*/")
    if sta_sra != nil then   #見つかった
      #「*/」まで削除
      if sra_sta_local >= sta_sra then
        printf("パースエラー")
        exit -1
      end  
      size = sta_sra - sra_sta_local + 2
      str[sra_sta_local,size] = ""
    else                      #見つからなかった
      #以降を削除→次の行へ
      l = str.length
      size = l - sra_sta_local
      str[sra_sta_local,size] = ""  #todo
      return 1      #「*/」検索モードで終わる
    end

    sra_sta_local = str.index("/*")
  end while sra_sta_local != nil

  return 0          #通常モードで終わる
end

#printf("%s --> %s\n",ARGV[0],ARGV[1])
ev_file = open(ARGV[0],"r")
tmp_file = open(ARGV[1],"w")

mode = 0
while line = ev_file.gets
  str = line.chomp("\n").chomp("\r")

  #モードで分岐
  if mode == 0 then               #通常モード
    #「/*」を検索
    sra_sta = str.index("/*")
    #「//」を検索
    sra_sra = str.index("//")
    #どっちが先に出てきたか？
    rc = CheckCommentStartCode(sra_sta, sra_sra)

    if rc == 1 then       #「//」の場合、以降を削除→次の行へ
      l = str.length
      size = l-sra_sra
      str[sra_sra,size] = "" #todo
      if sra_sra != 0 then    #行開始が「//」でないときはプリントする
        tmp_file.printf("%s\n",str)
      end
      mode = 0
      next
    elsif rc == 2 then    #「/*」の場合、「*/」を検索
      mode = CheckComSraSta_StaSra(str, sra_sta)
      tmp_file.printf("%s\n",str)
    else                  #コメント無し
      tmp_file.printf("%s\n",str)
    end
  else                        #「*/」検索中モード
    #「*/」を検索
#    printf("コメント行 %s\n",str)
    sta_sra = str.index("*/")
    if sta_sra == nil then    #見つからない→この行は全部コメントなので書かない
      next
    else                      #発見「*/」までを削除
      size = sta_sra+2
      str[0,size] = ""
      tmp_file.printf("%s\n",str)
      mode = 0
    end
  end
end

