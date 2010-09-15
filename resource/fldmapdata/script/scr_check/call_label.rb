$KCODE = "SJIS"

class SeachIF_SW
  @StartCodeAry
  @EndCodeAry
  def initialize
    @StartCodeAry = ["IF","SWITCH"]
    @EndCodeAry = ["ENDIF","ENDSWITCH"]
  end

  def SearchStartCode(str)
    fix_str = Marshal.load(Marshal.dump(str))
    fix_str.gsub!(/ /,"")
    fix_str.gsub!(/\t/,"")

    if fix_str.length == 0 then
      return -1
    end

    @StartCodeAry.each_with_index{|code, idx|
      n = fix_str.index(code)
      if n != nil then
        if n == 0 then
          #検出
          return idx
        end
      end
      return -1
    }
  end

   def SearchEndCode(str, idx)
    fix_str = Marshal.load(Marshal.dump(str))
    fix_str.gsub!(/ /,"")
    fix_str.gsub!(/\t/,"")
  
    code = @EndCodeAry[idx]

    n = fix_str.index(code)
    if n != nil then
      if n == 0 then
        #検出
        return true
      end
    end
    return false
  end

end

def IsRet(str)
  fix_str = Marshal.load(Marshal.dump(str))
  fix_str.gsub!(/ /,"")
  fix_str.gsub!(/\t/,"")

  if fix_str == "_RET" then
    return true
  end

  return false
end


class SubLabelHash
  @ifsw
  @label_hash
  @if_count
  def initialize
    @ifsw = SeachIF_SW.new
    @label_hash = Hash.new
    @if_count = 0
  end

  def MakeHash(filename)
    file = open(filename,"r")

    mode = 0
    label = ""
    ary = Array.new
    while line = file.gets
      str = line.chomp("\n").chomp("\r")
      case mode
      when 0 then     #ラベル探し
        rc = str.include?(":")
        if rc == true then
          m = /:/.match(str)
          label = m.pre_match
          ary.clear       #配列クリア
          mode = 1        #ラベル終了検出開始
        end
      when 1 then         #ラベル終了探し（_RET）
        retval = @ifsw.SearchStartCode(str)
        if retval == 0 then
          #ENDIF探しを開始
          mode = 2
          if @if_count != 0 then
            p "ERROR!  IF カウンタエラー"
            exit(-1)
          end
          @if_count += 1
        elsif retval == 1 then
          #ENDSWITCH探しを開始
          mode = 3
        end

        rc = IsRet(str)
        if rc == true then  #_RET検出　ラベル終了
          #既にラベルあるかをチェック
          rc = @label_hash.key?(label)
          if rc == true then
            printf("ラベルがすでにある　%s\n",label)
            exit(-1)
          end  
          #溜め込んだ文字配列をハッシュに登録
          copy_ary = Marshal.load(Marshal.dump(ary))
          @label_hash[label] = copy_ary
          #次のラベル探し開始
          mode = 0
        end
      when 2 then         #ENDIF探し
        rc = @ifsw.SearchEndCode(str, 0)
        if rc == true then
          @if_count -= 1
          if @if_count == 0 then
            #_RET探しに戻る
            mode = 1
          end
        else
          retval = @ifsw.SearchStartCode(str)
          if retval == 0 then
            #IFみつけてしまった
            @if_count += 1
          end
        end 
      when 3 then         #ENDSWITCH探し
        rc = @ifsw.SearchEndCode(str, 1)
        if rc == true then
          #_RET探しに戻る
          mode = 1
        end
      end

      if mode != 0 then
        ary << str
      end
    end

    file.close

    return GetHash()
  end

  def Dump()
    #ダンプしてみる
    p "============================"
    @label_hash.each{|key, val|
      printf("--%s--\n",key)
      val.each{|i|
        p i
      }
    }
  end

  def GetHash()
    return @label_hash
  end

end   #class


def _SearchCode(code_ary, str, find_code_ary)
  code_ary.each{|i|
    target = str.index(i)
    if target != nil then     #見つかった
      if find_code_ary != nil then
        find_code_ary << i
      end  
      return true
    end
  }
  return false
end

def CallFunc(hash, code_ary, call_label, call_find_code_ary)
  local_ary = Array.new

  #ハッシュからラベルに一致する配列を取得
  ary = hash[call_label]
  if ary == nil then
    printf("not_label %s\n",call_label)
    return
  end

  #配列を検索
  ary.each_with_index{|str,idx|
    local_ary.clear
    #検索コード調べる
    rc = _SearchCode(code_ary, str, local_ary)
    if rc == true then
      #_CALLか？
      not_space_str = str.gsub(/\s/,"")
      target = not_space_str.index("_CALL")
      if (target != nil)&&(target == 0) then     #見つかった
#        printf("str = %s\n",str)
        m = /_/.match(str)
        fix_str = "_" + m.post_match
        m = /\s/.match(fix_str)
        if m != nil then
          call_str = m.pre_match
          label_str = m.post_match
          call_str.gsub!(/\s/,"")
          label_str.gsub!(/\s/,"")
          if call_str == "_CALL" then
#           printf("CALLを発見\n")
            CallFunc(hash, code_ary, label_str, call_find_code_ary)
          end
        end
      else        #_CALL 以外
#        printf("CALL内で発見\n")
        make_str = call_label + "-->" + local_ary[0] + "::" + idx.to_s
        call_find_code_ary << make_str
      end
    end
  }
end

def CallFuncParent(hash, code_ary, str, call_find_code_ary)
  #_CALLか？
  not_space_str = str.gsub(/\s/,"")
  target = not_space_str.index("_CALL")
  if (target != nil)&&(target == 0) then     #見つかった
#    printf("PARENT CALLを発見 %s\n",str)
    m = /_/.match(str)
    fix_str = "_" + m.post_match
    rc = fix_str.index("\s")
    if rc =! nil then
      m = /\s/.match(fix_str)
      if m != nil then
        call_str = m.pre_match
        label_str = m.post_match
        call_str.gsub!(/\s/,"")
        label_str.gsub!(/\s/,"")
#       printf("PARENT CALL部分 %s\n",call_str)
        if call_str == "_CALL" then
          CallFunc(hash, code_ary, label_str, call_find_code_ary)
        end
      else
#        printf("%s は除外します\n",fix_str)
      end
    else
#      printf("%s は除外します\n",fix_str)
    end
  end
end

def GetCallLabel(str)
  #_CALLか？
  not_space_str = str.gsub(/\s/,"")
  target = not_space_str.index("_CALL")
  if (target != nil)&&(target == 0) then     #見つかった
#    printf("PARENT CALLを発見 %s\n",str)
    m = /_/.match(str)
    fix_str = "_" + m.post_match
    rc = fix_str.index("\s")
    if rc =! nil then
      m = /\s/.match(fix_str)
      if m != nil then
        call_str = m.pre_match
        label_str = m.post_match
        call_str.gsub!(/\s/,"")
        label_str.gsub!(/\s/,"")
#       printf("PARENT CALL部分 %s\n",call_str)
        if call_str == "_CALL" then
          return label_str
        end
      else
#        printf("%s は除外します\n",fix_str)
      end
    else
#      printf("%s は除外します\n",fix_str)
    end
  end

  return nil
end

