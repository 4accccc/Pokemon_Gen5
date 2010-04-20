$KCODE = "SJIS"

class SE_DATA
  def initialize()
    @Se = ""
    @Frm = 0
  end

  def GetSe()
    return @Se
  end

  def GetFrm()
    return @Frm
  end

  def SetSe( se )
    @Se = se
  end

  def SetFrm( frm )
    @Frm = frm
  end
end

def SetData( index, column, data_ary )
  se_ary = Array.new
  frm_ary = Array.new
  ok = false

  column.each_with_index{ | code, idx |
    if idx % 2 == 0 then
      se_ary << code.to_s
    else
      frm_ary << code.to_i
    end
  }

  num = se_ary.length
  for i in 0..(num-1)
    data = SE_DATA.new
    data.SetSe( se_ary[i] )
    if se_ary[i] == "CI_SE_NONE" then
      data.SetFrm( 0 )
      ok = true
    else
      data.SetFrm( frm_ary[i] )
      ok = false
    end
    data_ary << data
  end

  if ok != true then
    printf("CUTIN SE ERROR :: 終端コードが無い  %d\n",index)
    exit(-1)
  end  
end

def PrintTbl( data_ary, idx, h_file )
  h_file.printf("const CI_SE_TBL Tbl%d[] = {\n",idx)
  data_ary.each{ | i |
    h_file.printf("\t{ %s, %d },\n", i.GetSe(), i.GetFrm())
  }
  h_file.printf("};\n")
  h_file.printf("\n")
end

csv_file = open(ARGV.shift,"r")

h_file_name = ARGV.shift
h_file = open(h_file_name,"w")

h_file.printf("//======================================================================\n")
h_file.printf("/**\n")
h_file.printf(" * @file	%s\n",h_file_name)
h_file.printf(" * @brief	SEタイミング定義ファイル\n")
h_file.printf(" * @note	このファイルはコンバーターから出力されます\n")
h_file.printf(" *\n")
h_file.printf(" */\n")
h_file.printf("//======================================================================\n")
h_file.printf("\n")

h_file.printf("#pragma once\n")
h_file.printf("\n")

data_ary = Array.new

#1行飛ばし
csv_file.gets
idx = 0

while line = csv_file.gets
  #END検出
  if line =~/^#END/ then
		break
	end
  
  column = line.split ","
  column.delete_at(0)
  column.delete_at(0)
  column.delete("\n")
  column.delete("\r")

  data_ary.clear
  SetData(idx, column, data_ary)

  PrintTbl( data_ary, idx, h_file  )
  
  idx+=1
end

h_file.printf("#define CI_SE_TBL_MAX  (%d)\n",idx)

h_file.printf("const CI_SE_TBL * const SeTbl[ CI_SE_TBL_MAX ] = {\n")
for i in 0..(idx-1)
  h_file.printf("\tTbl%d,\n",i)
end
h_file.printf("};\n")

