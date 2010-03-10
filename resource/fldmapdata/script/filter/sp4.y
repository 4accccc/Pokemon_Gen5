#====================================================================================
#
#	シャチプロジェクト用スクリプトフィルタ定義
#
#	2007.04　tamada GAME FREAK inc.
#
#	既存のポケモンスクリプトに変数代入文やIF構文を追加するために、
#	racc（rubyによるyacc=Yet Another Compiler Compiler)を使用した
#	フィルタを生成するための定義ファイル
#====================================================================================



#====================================================================================
#
#
#   文法規則記述部
#
#
#====================================================================================
class PmScript::Parser

#---------------------------------------------
#	演算子優先順位の定義
#---------------------------------------------
prechigh
	left	'/'	'*'
	left	'-'	'+'

  left '==' '!=' '<' '>' '<=' '>='
  left '&&' '||'
preclow

#---------------------------------------------
#	以下はBNF記法による文法+α
#---------------------------------------------
rule

	#---------------------------------------------
	#	「プログラム」は「文のリスト」
	#---------------------------------------------
	program	: stmt_list
					{
						result = RootNode.new( val[0] )
					}

	#---------------------------------------------
	#	「文のリスト」は下記のいずれか：
	#		空行
	#		「文のリスト」と「文」と「終端」
	#		「文のリスト」と「ブロック」
	#		「文のリスト」と「終端」
	#		「文のリスト」と「文」と「コメントリスト」と「終端」
	#		「文のリスト」と「コメントリスト」と「終端」
	#		「文のリスト」と「終端」と「コメントリスト」と「終端」
	#---------------------------------------------
	stmt_list	:
					{
						result = []
					}
				| stmt_list stmt EOL
					{
						result.push val[1]
					}
				| stmt_list stmt_block
					{
						result.push val[1]
					}
				| stmt_list EOL
				| stmt_list stmt comments EOL
					{
						result.push val[1]
						#result.push RawNode.new(val[2])
					}
				| stmt_list comments EOL
					{
						#result.push RawNode.new(val[1])
					}
				| stmt_list EOL comments EOL
					{
						#result.push RawNode.new(val[2])
					}


	#---------------------------------------------
	#	「変数定義リスト」は下記のいずれか：
	#		空行
	#		「変数定義リスト」「終端」
	#		「変数定義リスト」「変数定義」「終端」
	#---------------------------------------------
  def_local_list :
          {
            result = []
          }
        | def_local_list EOL
        | def_local_list comments EOL
        | def_local_list def_local EOL
          {
            result.push val[1]
          }
        | def_local_list def_local comments EOL
          {
            result.push val[1]
          }
	#---------------------------------------------
	#	「関数定義」は下記のいずれか：
	#		「DEFFUNC」「定義キーワード」「識別子」「(」「パラメータ」「)」「終端」「ブロック」
	#		「DEFFUNC」「定義キーワード」「識別子」「(」「)」「終端」「ブロック」
	#---------------------------------------------
	def_func	: DEFFUNC TYPE IDENT '(' param ')' EOL stmt_block
					{
						result = DefFuncNode.new( val[1], val[2], val[4], val[7] )
					}
				| DEFFUNC TYPE IDENT '(' ')' EOL stmt_block
					{
						result = DefFuncNode.new( val[1], val[2], nil, val[6] )
					}

	#---------------------------------------------
	#	「パラメータ」は下記のいずれか」：
	#		「変数参照」
	#		「パラメータ」「,」「変数参照」
	#---------------------------------------------
	param		: VARREF
					{
						result = val
					}
				| param ',' VARREF
					{
						result.push val[2]
					}

	#---------------------------------------------
	#	「ブロック」は
	#		「ブロック開始」「」「」「ブロック終了」
	#---------------------------------------------
  stmt_block  : block_start def_local_list stmt_list block_end
					{
						result = BlockNode.new(val[1], val[2])
					}
				| def_func

	#---------------------------------------------
	#	「ブロック開始」は下記のいずれか：
	#		「{」「終端」
	#		「{」「コメントリスト」「終端」
	#---------------------------------------------
	block_start	: '{' EOL
				| '{' comments EOL
	
	#---------------------------------------------
	#	「ブロック終了」は下記のいずれか：
	#		「}」「終端」
	#		「}」「コメントリスト」「終端」
	#---------------------------------------------
	block_end	: '}' EOL
				| '}' comments EOL

	#---------------------------------------------
	#	「文」は下記のいずれか
	#			ラベル
	#			「命令」
	#			「命令文」
	#			「マクロ」
	#			「if文」
	#			「代入文」
	#			「関数呼び出し」
	#---------------------------------------------
	stmt		: LABEL
					{
						result = RawNode.new(val[0]);
					}
				| COMMAND
					{
						result = CmdNode.new( val, nil )
					}
				| cmd
					{
						result = CmdNode.new( val[0].shift, val[0] )
					}
				| MACRO
					{
						result = RawNode.new(val[0]);
					}
				| if_stmt
					{
					}
				| assign
					{
					}
        | compound_assign
				| callfunc
        | switch_stmt
        | while_stmt
        | def_local

	#---------------------------------------------
	#	「関数呼び出し」は下記のいずれか：
	#		「識別子」「(」「)」
	#		「識別子」「(」「引数」「)」
	#		「変数参照」「=」「識別子」「(」「引数」「)」
	#		「変数参照」「=」「識別子」「(」「)」
	#---------------------------------------------
	callfunc	: IDENT '(' ')'
					{
						result = CallFuncNode.new( nil, val[0], nil )
					}
				| IDENT '(' args ')'
					{
						result = CallFuncNode.new( nil, val[0], val[2] )
					}
				| VARREF '=' IDENT '(' args ')'
					{
						result = CallFuncNode.new( val[0], val[2], val[4] )
					}
				| VARREF '=' IDENT '(' ')'
					{
						result = CallFuncNode.new( val[0], val[2], nil )
					}

	#---------------------------------------------
	#	「コメントリスト」は下記のいずれか：
	#		「コメント」
	#		「コメントリスト」「コメント」
	#---------------------------------------------
	comments	: COMMENT
					{
						result = val
					}
				| comments COMMENT
					{
						result.push val[1]
					}

	#---------------------------------------------
	#	「if文」は下記：
	#	「IF」「比較式」「THEN」「終端」「文の並び」「else文」「ENDIF」
	#---------------------------------------------
	if_stmt		: IF cmp_expr THEN stmt_list else_stmt ENDIF
					{
						result = IfNode.new(val[1],val[3],val[4])
					}

	#---------------------------------------------
	#	「else文」は下記のいずれか：
	#		空白
	#		「ELSE」「文の並び」
	#		「ELSEIF」「比較式」「THEN」「終端」「文の並び」「else文」
	#
	#---------------------------------------------
	else_stmt	:
					{
						result = nil
					}
				| ELSE stmt_list
					{
						result = val[1]
					}
				| ELSIF cmp_expr THEN stmt_list else_stmt
					{
						result = [IfNode.new(val[1], val[3], val[4])]
					}

	#---------------------------------------------
	#	「比較式」は下記のいずれか：
	#		「変数参照」「比較演算子」「数式」
	#		「変数参照」「比較演算子」「変数参照」
	#		「FLAG_ON」「(」「識別子」「)」
	#		「FLAG_OFF」「(」「識別子」「)」
	#		「(」「比較式」「)」
	#		「比較式」「比較演算子」「比較式」
  #
	#---------------------------------------------
	cmp_expr	:	VARREF cmp_ident expr
					{
            left = "_PUSH_WORK #{val[0].sub(/\A\$/,"")}"
            right = "_PUSH_VALUE #{val[2]}"
            result = CompareNode.new(val[1], left, right)
					}
				|	VARREF cmp_ident VARREF
					{
            left = "_PUSH_WORK #{val[0].sub(/\A\$/,"")}"
            right = "_PUSH_WORK #{val[2].sub(/\A\$/,"")}"
            result = CompareNode.new(val[1], left, right)
					}
				|	FLAG_ON '(' IDENT ')'
					{
            left = "_PUSH_FLAG #{val[2]}"
            right = "_PUSH_VALUE TRUE"
            result = CompareNode.new('CMPID_EQ', left, right)
					}
				|	FLAG_ON '(' VARREF ')'
					{
            left = "_PUSH_FLAG #{val[2].sub(/\A\$/,"")}"
            right = "_PUSH_VALUE TRUE"
            result = CompareNode.new('CMPID_EQ', left, right)
					}
				|	FLAG_OFF '(' IDENT ')'
					{
            left = "_PUSH_FLAG #{val[2]}"
            right = "_PUSH_VALUE FALSE"
            result = CompareNode.new('CMPID_EQ', left, right)
					}
				|	FLAG_OFF '(' VARREF ')'
					{
            left = "_PUSH_FLAG #{val[2].sub(/\A\$/,"")}"
            right = "_PUSH_VALUE FALSE"
            result = CompareNode.new('CMPID_EQ', left, right)
					}
        | '(' cmp_expr ')'
          {
            result = val[1]
          }
        | cmp_expr cmp_ident cmp_expr
          {
            result = LogicalCompareNode.new(val[1], val[0], val[2])
          }


	#---------------------------------------------
	#	「比較演算子」の定義
	#---------------------------------------------
	cmp_ident	:	'=='
					{
						result = 'CMPID_EQ'	#result = 'EQ'
					}
				|	'!='
					{
						result = 'CMPID_NE'	#result = 'NE'
					}
				|	'>'
					{
						result = 'CMPID_GT'	#result = 'GT'
					}
				|	'<'
					{
						result = 'CMPID_LT'	#result = 'LT'
					}
				|	'>='
					{
						result = 'CMPID_GT_EQ'	#result = 'GE'
					}
				|	'<='
					{
						result = 'CMPID_LT_EQ'	#result = 'LE'
					}
        | '&&'
          {
            result = 'CMPID_AND'
          }
        | '||'
          {
            result = 'CMPID_OR'
          }


	#---------------------------------------------
  # 「WHILE」は
  # 「WHILE」「比較式」「終端」「文の並び」「ENDWHILE」
	#---------------------------------------------
  while_stmt  : WHILE cmp_expr EOL stmt_list ENDWHILE
          {
            result = WhileNode.new(val[1], val[3])
          }
	#---------------------------------------------
  # 「switch」は
  # 「SWITCH」「変数参照」「終端」「case文のならび」「default文」「ENDSWITCH」
	#---------------------------------------------

  switch_stmt : SWITCH VARREF EOL case_stmt_list default_stmt ENDSWITCH
          {
            result = SwitchNode.new(val[1].sub(/\A\$/,""), val[3], val[4])
          }

	#---------------------------------------------
  # 「case文のならび」は下記のいずれか：
  # 「case文」
  # 「case文のならび」「case文」
  #
	#---------------------------------------------
  case_stmt_list  :
          {
            result = []
          }
        | case_stmt
          {
            result = [ val[0] ]
          }
        | case_stmt_list case_stmt
          {
            result.push val[1]
          }

	#---------------------------------------------
  # 「case文」は
  # 「CASE」「引数」「THEN」「文の並び」
	#---------------------------------------------
  case_stmt : CASE args THEN stmt_list
          {
            result = CaseNode.new(val[1], val[3])
          }

	#---------------------------------------------
  # 「DEFAULT文」は下記のいずれか：
  #
  # 「空白」
  # 「DEFAULT」「文の並び」
	#---------------------------------------------
  default_stmt :
        | DEFAULT stmt_list
          {
            result = val[1]
          }

	#---------------------------------------------
	#	「代入」は
	#		「変数参照」「=」「変数参照」
	#		「変数参照」「=」「プライマリ」
	#		「変数参照」「=」「関数呼び出し」
	#---------------------------------------------
	assign		: VARREF '=' VARREF
					{
						result = AssignNode.new(val[0], val[2], :VARREF)
					}
				| VARREF '=' primary
					{
						result = AssignNode.new(val[0], val[2], nil)
					}
				| VARREF '=' funcall
					{
						result = AssignNode.new(val[0], val[2], funcall)
					}
	#---------------------------------------------
  # 「複合代入」は
  #   
  #   「変数参照」「複合代入演算子」「変数参照」
  #   「変数参照」「複合代入演算子」「プライマリ」
	#---------------------------------------------
  compound_assign : VARREF OP_COMP_ASSIGN VARREF
            {
              result = CompoundAssignNode.new(val[0], val[2], val[1])
            }
          | VARREF OP_COMP_ASSIGN primary
            {
              result = CompoundAssignNode.new(val[0], val[2], val[1])
            }

	#---------------------------------------------
	#	「命令文」は
	#		「命令」「引数」
	#---------------------------------------------
	cmd			: COMMAND args
					{
						result = [val[0],val[1]]
					}
	      |  COMMAND '(' args ')'
					{
						result = [val[0], ['('] + val[2] + [')'] ]
					}
        | COMMAND '(' ')'
          {
            result = [ val[0], "()" ]
          }
        | EVENT_START IDENT
          {
						result = [val[0],val[1]]
          }
        | EVENT_END
          {
            result = [ val[0], nil ]
          }
        | INIT_EVENT_START IDENT
          {
            result = [ val[0], val[1] ]
          }
        | INIT_EVENT_END
          {
            result = [ val[0], nil ]
          }

	#---------------------------------------------
  # 「ローカル変数定義」は
  #   「DEFINE_LOCAL」「識別子」
	#---------------------------------------------
  def_local : DEFINE_LOCAL IDENT
          {
            result = DefLocalVarNode.new( val[1] )
          }

	#---------------------------------------------
	#	「数式」は
	#		「数式」「+」「数式」
	#		「数式」「-」「数式」
	#		「数式」「/」「数式」
	#		「数式」「*」「数式」
	#		「(」「数式」「)」
	#---------------------------------------------
	expr		: primary
				| expr '+' expr
					{
						result = val
					}
				| expr '-' expr
					{
						result = val
					}
				| expr '/' expr
					{
						result = val
					}
				| expr '*' expr
					{
						result = val
					}
				| '(' expr ')'
					{
						result = val
					}

	#---------------------------------------------
	#	「プライマリ」は
	#			数値
	#			識別子
	#			マクロ参照
	#---------------------------------------------
	primary		: NUMBER
				| IDENT
				| MACPARAM


	#---------------------------------------------
	#	「引数」は
	#		「数式」
	#		「変数参照」
	#---------------------------------------------
	arg			: expr
				| VARREF

	#---------------------------------------------
	#	「引数の並び」は
	#		「引数」
	#		「引数の並び」と「,」と「引数」
	#---------------------------------------------
	args		: arg
					{
						result = val
					}
				| args ',' arg
					{
						result.push val[1]
						result.push val[2]
					}


end
#class PmScript::Parserの終了位置

#====================================================================================
#
#
#   ユーザーコード記述部
#
#
#====================================================================================

---- header
#
#	ポケモン用スクリプトコンパイラ
#

---- inner

def initialize
end

#予約語定義
RESERVED = {
	'IF'	=> :IF,
	'THEN'	=> :THEN,
	'ELSE'	=> :ELSE,
	'ELSIF'	=> :ELSIF,
	'ENDIF'	=> :ENDIF,
	'DEFFUNC'	=> :DEFFUNC,
	'FLAG_ON'	=> :FLAG_ON,
	'FLAG_OFF'	=> :FLAG_OFF,
  'EVENT_START' => :EVENT_START,
  'EVENT_END' => :EVENT_END,
  'INIT_EVENT_START' => :INIT_EVENT_START,
  'INIT_EVENT_END' => :INIT_EVENT_END,
  'SWITCH' => :SWITCH,
  'CASE' => :CASE,
  'DEFAULT' => :DEFAULT,
  'ENDSWITCH' => :ENDSWITCH,
  'WHILE' => :WHILE,
  'ENDWHILE' => :ENDWHILE,
  'DEFINE_LOCAL' => :DEFINE_LOCAL,
};

#予約型定義
RESERVED_TYPE = {
	'LWORK'	=> :LWORK,
	'LABEL' => :LABEL_TYPE,
};

def parse( f )
	@q = []
	@nowlineno = 1
	@incomment = false
	f.each do |line_org|
		line = line_org.strip
		until line.empty? do
			if @incomment == true then
				if line =~ /.*\*\// then
					@incomment = false
					pushq [ :COMMENT, $&]
					line = $'
				else
					pushq [ :COMMENT, line]
					line = ""
				end
			else
				case line
				when /\A\s+/
					;
				when /\A\/\/.*$/
          #Ｃ＋＋形式コメント定義
					pushq [ :COMMENT, $& ]

				when /^\s*$/, /\A\#.*/
          #アセンブラコメント定義
					pushq [ :COMMENT, $&]

				when /\A\/\*.*\*\//
          #Ｃ形式コメント定義
					pushq [ :COMMENT, $&]

				when /\A\.[a-zA-Z_]+.*$/
          #アセンブラマクロ定義
					pushq [ :MACRO, $& ]

				when /\A[a-zA-Z_][a-zA-Z0-9_]*:/
          #アセンブララベル定義
					pushq [ :LABEL, $& ]

				when /\A_[A-Z0-9][a-zA-Z0-9_]*/
          #スクリプトコマンド定義（＿で開始する）
					pushq [ :COMMAND, $& ]

				when /\A0x[0-9a-fA-F]+/, /\A\d+/
          #数値定義（0xで始まる１６進数、あるいは１０進数）
					pushq [ :NUMBER, $& ]

				when /\A\$[a-zA-Z][a-zA-Z0-9_]*/
          #変数定義（＄で始まる）
					pushq [ :VARREF, $& ]

				when /\A[a-zA-Z_][a-zA-Z0-9_]*/
          #識別子定義あるいは型定義
					if RESERVED.has_key? $& then
						pushq [ RESERVED[$&], $&.intern ]
					elsif RESERVED_TYPE.has_key? $& then
						pushq [ :TYPE, $&.intern ]
					else
						pushq [ :IDENT, $& ]
					end

				when /\A\\[a-zA-Z_][a-zA-Z0-9_]*/
          # \から始まる識別子はアセンブラマクロパラメータ
					pushq [ :MACPARAM, $& ]

				when /\A==/,/\A!=/,/\A\<=/,/\A\>=/,/\A>/,/\A</,/\A\&\&/,/\A\|\|/
          # 比較演算子
					pushq [ $&, $& ]

        when /\A\+=/, /\A\-=/, /\A\*=/, /\A\/=/, /\A\%=/
          # 複合代入演算子
          pushq [ :OP_COMP_ASSIGN, $& ]

				when /\A\/\*.*/
          #C形式コメント開始
					pushq [ :COMMENT, $& ]
					@incomment = true

				when /\A[\+\-\*\/=(){},]/
          #演算子、カッコなどの記号
					pushq [ $&, $& ]
				else
					raise RuntimeError, "#{@fname}:#{@nowlineno}: fatal error! \{#{line_org}\}"
				end
				line = $'
				printf( "\#NOW LINE(%4d) %s\n", @nowlineno, line)
			end
		end
		pushq [ :EOL, nil ]
		@nowlineno += 1
	end
	pushq [ false, nil ]
	
	@yydebug = true
	do_parse
end

def next_token
	value = @q.shift
	@lineno = value.last
	value.delete_at(2)
	value
end

def pushq value
	value << @nowlineno
	@q.push value
end


def on_error( t, v, values )
    if v then
      line = v[0]
      v = v[1]
    else
      line = 'last'
    end
    raise Racc::ParseError, "#{@fname}:#{@lineno}: syntax error on #{v.inspect}"
  end


---- footer

