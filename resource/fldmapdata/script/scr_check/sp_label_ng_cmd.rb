def AddLabel(ary, str, target_label)
  rc = str.index(target_label)
  if rc != nil then
    ary << str
  end
end

def SearchCode(code_ary, str, find_code_ary)
  code_ary.each{|i|
    target = str.index(i)
    if target != nil then     #見つかった
      find_code_ary << i
      return true
    end
  }
  return false
end

def MakeSearchCodeList(list_txt, code_ary)
  list_file = open(list_txt,"r")
  while line = list_file.gets
    str = line.chomp("\n").chomp("\r")
    code_ary << str
  end
end

ev_file = open(ARGV[0],"r")

list_txt = ARGV[1]

target_label = ARGV[2]

#検索したいコードのリスト作成
code_ary = Array.new
MakeSearchCodeList(list_txt, code_ary)

label = Array.new
dump_ary = Array.new
find_code_ary = Array.new

seq = 0
find = false
while line = ev_file.gets
  str = line.chomp("\n").chomp("\r")

  case seq
  when 0 then
    #_EVENT_DATA_END検出
    data_end = str.index("_EVENT_DATA_END")
    if data_end == nil then
      column = str.split " "
      #「_EVENT_DATA」を配列化
      idx = str.index("_EVENT_DATA")
      if idx != nil then
        AddLabel(label, column[1], target_label)
      end
    else
      #_EVENT_DATA_ENDを検出したので、ラベル検索に移る
      seq = 1
#      label.each{ |i|
#        p i
#      }
    end
  when 1 then
    #INIT_EVENT_START検出
    ev_start = str.index("INIT_EVENT_START")
    if ev_start != nil then
      if ev_start == 0 then
        column = str.split " "
        rc = label.include?(column[1])
        if rc == true then
#          printf("%sを調査開始\n",column[1])
          search_label = column[1]      #調べるラベルを記憶
          #対象コード検索シーケンスへ
          seq = 2
          find = false
        end
      end
    end
  when 2 then
    #INIT_EVENT_END検出
    ev_end = str.index("INIT_EVENT_END")
    if ev_end != nil then
      if find != false then      #検索コード発見した
#        printf("検索コードがみつかった\n")
         dump_ary << search_label    #対象ラベルをプッシュ
      end
      #イベントエンドが見つかったのでシーケンスを戻す
      seq = 1
    else
      #検索コード調べる
      if find == false then
        find = SearchCode(code_ary, str, find_code_ary)
      end
    end
  end
end

if dump_ary.length != 0 then
  printf("========%s========\n",ARGV[0])
  dump_ary.each_with_index{|label, idx|
    printf("%s ==> %s\n",label, find_code_ary[idx])
  }
end


