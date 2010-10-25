require "call_label.rb"

$KCODE = "SJIS"

class SearchData
  def initialize(str)
    @filename = str
    MakeSearchData()
  end

  # 検索データの生成
  def MakeSearchData
    file = File.open( @filename, "r" )
    file_lines = file.readlines
    file_lines.slice!(0) # ヘッダを削除
    file.close 

    # フラグ定義
    readflg         = false
    include_flag     = true
    no_include_flag  = true

    # コマンド配列
    @include_cmd = Array.new
    @no_include_cmd = Array.new


    file_lines.each do |line|
      # 区切りごとに分割
      line.gsub!( "\"", "" )
      items = line.split( /\t/ )

      # 検索条件が書かれている次の行から読み始める
      if readflg == true then

        if items[1] != nil then
          # ラベル名
          if line.include?("ラベル名") then
            @label = items[6]
          end

          # ラベル名
          if line.include?("スタートコマンド") then
            @start_cmd = items[6]
          end

          # ラベル名
          if line.include?("エンドコマンド") then
            @end_cmd = items[6]
          end

          # ラベル名
          if line.include?("出力ファイル名") then
            @put_file = items[6]
          end
        end

        # 含むコマンド追加
        if items[16] != nil then
          if items[16].include?("_END") then
            include_flag = false;
          end
          if include_flag == true then

            @include_cmd << items[16]
          end
        end

        # 含まないコマンド追加
        if items[31] != nil then
          if items[31].include?("_END") then
            no_include_flag = false;
          end
          if no_include_flag == true then
            @no_include_cmd << items[31]
          end
        end

      end

      # データ開始
      if line.include?("検索条件") then
        readflg =true
      end
      # 記述方法以下は読み捨てる
      if line.include?("記述方法") then
        readflg =false
      end

    end
  end

  attr_reader :label,:start_cmd,:end_cmd,:put_file,:include_cmd,:no_include_cmd
end


class ScriptSearch

  # コンストラクタ
  def initialize(ev_name,data,file)
    @search_data = data
    @ev_name = ev_name
    @search_data = Marshal.load(Marshal.dump(data))
    @put_file = file
  end

  def AddLabel(ary, str, target_label)
    rc = str.index(target_label)
    if rc != nil then
      ary << str
    end
  end

  def SearchCode(code_ary, str, find_code_ary)
    code_ary.each{|i|
      target = str.index(/[\s^]#{i}[\s\n\(]/)
      #target = str.index(i)
      if target != nil then     #見つかった
        if find_code_ary != nil then
          find_code_ary << i
        end  
        return true
      end
    }
    return false
  end

  # 検索
  def run
    s = SubLabelHash.new

    # call用ハッシュを生成
    hash = s.MakeHash(@ev_name)

    # 検索evフィル
    ev_file = open(@ev_name,"r")

    label = Array.new
    dump_ary = Array.new
    find_code_ary = Array.new
    no_find_code_ary = Array.new
    call_find_code_ary = Array.new
    no_call_find_code_ary = Array.new
    seq = 0
    find = false
    no_find = false
    endjump = false
    while line = ev_file.gets
      str = line.chomp("\n").chomp("\r")

      case seq
      when 0 then
        #_EVENT_DATA_END検出
        data_end = str.index("_EVENT_DATA_END")
        if data_end == nil then
          column = str.split "\s"
          #「_EVENT_DATA」を配列化
          idx = str.index("_EVENT_DATA")
          if idx != nil then
            AddLabel(label, column[1], @search_data.label)
          end
        else
          #_EVENT_DATA_ENDを検出したので、ラベル検索に移る
          seq = 1
        end
      when 1 then
        #START検出
        ev_start = str.index(@search_data.start_cmd)
        fix_str = Marshal.load(Marshal.dump(str))
        fix_str.gsub!(/\s/,"")
        ev_ng_end = fix_str.index(@search_data.end_cmd)
        if ev_ng_end != nil then
          if ev_ng_end == 0 then
            if endjump == false then
              printf("START検出中にENDを検出　file:%s label:%s\n",@ev_name, column[1])
              exit(-1)
            else
              #ジャンプ解除
              endjump = false
              #          printf("%s は　非対象なのでジャンプ\n",column[1])
            end
          end
        end

        if ev_start != nil then
          if ev_start == 0 then
            column = str.split "\s"
            rc = column[1].index(@search_data.label);
            #if rc == true then
            if rc == 0 then
              #          printf("%sを調査開始\n",column[1])
              search_label = column[1]      #調べるラベルを記憶
              #対象コード検索シーケンスへ
              seq = 2
              find = false
            else
              #非対象ラベル
              endjump = true
            end
          end
        end
      when 2 then
        fix_str = Marshal.load(Marshal.dump(str))
        fix_str.gsub!(/\s/,"")
        ev_ng_end = fix_str.index("NG_END")
        if ev_ng_end != nil then
          if ev_ng_end == 0 then
            printf("対応していないコードを検出　%s\n",search_label)
            exit(-1)
          end
        end

        #END検出
        ev_end = str.index(@search_data.end_cmd)
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
            if @search_data.include_cmd.size > 0 then
              find = SearchCode(@search_data.include_cmd, str, find_code_ary)
              CallFuncParent(hash, @search_data.include_cmd, str, call_find_code_ary)
            end
          end
          # 含まないコードを調べる
          if no_find == false then
            if @search_data.no_include_cmd.size > 0 then
              no_find = SearchCode(@search_data.no_include_cmd, str, no_find_code_ary)
              CallFuncParent(hash, @search_data.no_include_cmd, str, no_call_find_code_ary)
              if no_call_find_code_ary.size != 0 then
                no_find = true
              end
            end
          end
        end
      end
    end

    if no_find == false then
      if dump_ary.length != 0 || call_find_code_ary.length != 0 then
        fname = @ev_name.gsub(/.*ev_tmp\//,"")
        @put_file.printf("========%s========\n",fname)
      end

      if dump_ary.length != 0 then
        dump_ary.each_with_index{|label, idx|
          @put_file.printf("%s ==> %s\n",label, find_code_ary[idx])
        }
      end

      if call_find_code_ary.length != 0 then
        @put_file.printf("●-------CALL-------\n")

        call_find_code_ary.each{|i|
          @put_file.printf( i + "\n")
        }
      end

      if dump_ary.length != 0 || call_find_code_ary.length != 0 then
        @put_file.printf "\n"
      end
    end
  end

end


