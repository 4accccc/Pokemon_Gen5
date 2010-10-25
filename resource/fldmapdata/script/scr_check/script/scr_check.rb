$KCODE = "SJIS"

def AddLabel(ary, str)

  rc = str.index("ev_zone")
  if rc != nil then
    return
  end

  rc = str.index("ev_init")
  if rc != nil then
    return
  end

  rc = str.index("ev_scene")
  if rc != nil then
    return
  end

  rc = str.index("ev_recover")
  if rc != nil then
    return
  end

  rc = str.index("ev_reserve")
  if rc != nil then
    return
  end

  rc = str.index("ev_pos")
  if rc != nil then
    return
  end

  rc = str.index("ev_bg")
  if rc != nil then
    return
  end

  ary << str
end

def SearchCode(code_ary, str)
  code_ary.each{|i|
    target = str.index(i)
    if target != nil then     #見つかった
      return true
    end
  }
  return false
end

ev_file = open(ARGV[0],"r")

code_ary = Array.new
code_ary << "_TALK_OBJ_START"
code_ary << "_TALK_START_TURN_NOT"
code_ary << "_EASY_BALLOONWIN_TALKOBJ_MSG"
code_ary << "_EASY_TALK_ITEM_EVENT"
code_ary << "_EASY_TALK_POKEVOICE"
code_ary << "_TALK_START_SE_PLAY"

label = Array.new
dump_ary = Array.new
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
        AddLabel(label, column[1])
      end
    else
      #_EVENT_DATA_ENDを検出したので、ラベル検索に移る
      seq = 1
#      label.each{ |i|
#        p i
#      }
    end
  when 1 then
    #EVENT_START検出
    ev_start = str.index("EVENT_START")
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
    #EVENT_END検出
    ev_int_end = str.index("INIT_EVENT_END")
    if ev_int_end != nil then
      printf("対応していないコード検出 %s\n",search_label)
      exit(-1)
    end

    ev_end = str.index("EVENT_END")
    if ev_end != nil then
      if find != true then      #検索コード発見できなかった
#        printf("検索コードがみつからない\n")
         dump_ary << search_label    #対象ラベルをプッシュ 
      end
      #イベントエンドが見つかったのでシーケンスを戻す
      seq = 1
    else
      #検索コード調べる
      if find == false then
        find = SearchCode(code_ary, str)
      end
    end
  end
end

if dump_ary.length != 0 then
  printf("========%s========\n",ARGV[0])
  dump_ary.each{|label|
    p label
  }
end
