class ScriptPreprocesser

prechigh
	left	'/'	'*'
	left	'-'	'+'
preclow

rule

	program	: stmt_list
					{
						result = RootNode.new( val[0] )
					}

	stmt_list	:
					{
						result = []
					}
				| stmt_list stmt EOL
					{
						result.push val[1]
					}
				| stmt_list EOL
				| stmt_list stmt comments EOL
					{
						result.push val[1]
						result.push RawNode.new(val[2])
					}
				| stmt_list comments EOL
					{
						result.push RawNode.new(val[1])
					}
				| stmt_list EOL comments EOL
					{
						result.push RawNode.new(val[2])
					}


	stmt		: LABEL
					{
						result = RawNode.new(val[0]);
					}
				| COMMAND
					{
						result = CmdNode.new( val, nil )
						#result = RawNode.new(val)
					}
				| cmd
					{
						result = CmdNode.new( val[0].shift, val[0] )
						#str = "	#{val[0].shift} #{val[0]}"
						#result = RawNode.new(str)
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
						result = RawNode.new(val[0]);
					}

	comments	: COMMENT
					{
						result = val
					}
				| comments COMMENT
					{
						result.push val[1]
					}
						
	if_stmt		: IF cmp_expr THEN EOL stmt_list else_stmt ENDIF
					{
						result = IfNode.new(val[1],val[4],val[5])
					}
	else_stmt	:
					{
						result = nil
					}
				| ELSE stmt_list
					{
						result = val[1]
					}
				| ELSIF cmp_expr THEN EOL stmt_list else_stmt
					{
						result = [IfNode.new(val[1], val[4], val[5])]
					}

	cmp_expr	:	VARREF cmp_ident expr
					{
						result = ["_CMPVAL #{val[0].sub(/\A\$/,"")}, #{val[2].sub(/\A\$/,"")}"]
						result.push val[1]
					}
				|	VARREF cmp_ident VARREF
					{
						result = ["_CMPWK #{val[0].sub(/\A\$/,"")}, #{val[2].sub(/\A\$/,"")}"]
						result.push val[1]
					}

	cmp_ident	:	'=='
					{
						result = 'NE'	#result = 'EQ'
					}
				|	'!='
					{
						result = 'EQ'	#result = 'NE'
					}
				|	'>'
					{
						result = 'LE'	#result = 'GT'
					}
				|	'<'
					{
						result = 'GE'	#result = 'LT'
					}
				|	'>='
					{
						result = 'LT'	#result = 'GE'
					}
				|	'<='
					{
						result = 'GT'	#result = 'LE'
					}

	assign		: VARREF '=' VARREF
					{
						result = "_LDWK #{val[0].sub(/\A\$/,"")}, #{val[2].sub(/\A\$/,"")}"
					}
				| VARREF '=' primary
					{
						result = "_LDVAL #{val[0].sub(/\A\$/,"")}, #{val[2]}"
					}

	cmd			: COMMAND args
					{
						result = [val[0],val[1]]
					}
				| COMMAND '(' args ')'
					{
						result = [val[0], val[2]]
					}

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

	primary		: NUMBER
				| IDENT
				| MACPARAM


	args		: expr
					{
						result = val
					}
				| args ',' expr
					{
						result.push val[1]
						result.push val[2]
					}

	
				
end


---- inner

def initialize
end

RESERVED = {
	'IF'	=> :IF,
	'THEN'	=> :THEN,
	'ELSE'	=> :ELSE,
	'ELSIF'	=> :ELSIF,
	'ENDIF'	=> :ENDIF
};

def parse	f
	@q = []
	@nowlineno = 1
	@incomment = false
	f.each do |line|
		line.strip!
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
					pushq [ :COMMENT, $& ]
				when /^\s*$/, /\A\#.*/
					pushq [ :COMMENT, $&]
				when /\A\/\*.*\*\//
					pushq [ :COMMENT, $&]
				when /\A\.[a-zA-Z_]+.*$/
					pushq [ :MACRO, $& ]
				when /\A[a-zA-Z_][a-zA-Z0-9_]*:/
					pushq [ :LABEL, $& ]
				when /\A_[A-Z0-9][a-zA-Z0-9_]*/
					pushq [ :COMMAND, $& ]
				when /\A0x[0-9a-fA-F]+/, /\A\d+/
					pushq [ :NUMBER, $& ]
				when /\A\$[a-zA-Z][a-zA-Z0-9_]*/
					pushq [ :VARREF, $& ]
				when /\A[a-zA-Z_][a-zA-Z0-9_]*/
					if RESERVED.has_key? $& then
						pushq [ RESERVED[$&], $&.intern ]
					else
						pushq [ :IDENT, $& ]
					end
				when /\A\\[a-zA-Z_][a-zA-Z0-9_]*/
					pushq [ :MACPARAM, $& ]
				when /\A==/,/\A!=/,/\A>/,/\A</,/\A<=/,/\A>=/
					pushq [ $&, $& ]
				when /\A\/\*.*/
					pushq [ :COMMENT, $& ]
					@incomment = true
				when /\A[\+\-\*\/(),]/
					pushq [ $&, $& ]
				else
					raise RuntimeError, "#{@fname}:#{@nowlineno}: fatal error!"
				end
				line = $'
				puts "\#NOW LINE #{line}"
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
	puts "\# #{value[0]} #{value[1]}"
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

class CompileError < StandardError; end

class CompileEnviroment
	def initialize
		@lcount = 0
		@lstack = []
	end

	def get_label
		label = sprintf("LABEL%05d", @lcount)
		@lcount += 1
		label
	end
end

class Node
	def	put_list( intp, nodes )
		v = nil
		nodes.each {|i| v = i.compile( intp ) }
		v
	end
end

class RootNode < Node
	def initialize( tree )
		@tree = tree
	end

	def compile( intp )
		put_list( intp, @tree )
	end
end

class CmdNode < Node
	def initialize( cmd, args )
		@cmd = cmd
		@args = args
	end

	def compile( intp )
		if @args then
			puts "\t#{@cmd}\t#{@args}"
		else
			puts "\t#{@cmd}"
		end
	end
end

class RawNode < Node
	def initialize( val )
		@val = val
	end
	def compile( intp )
		puts "#{@val}"
	end
end

class IfNode < Node
	def initialize( cond_node, then_stmt, else_stmt )
		@cond_node = cond_node
		@then_stmt = then_stmt
		@else_stmt = else_stmt
	end

	def compile( intp )
		#put compare command
		puts "\t#{@cond_node[0]}"

		#put if_stmt condition EOL
		label1 = intp.get_label
		puts "\t_IF_JUMP\t#{@cond_node[1]},#{label1}"
		put_list( intp, @then_stmt )
		if @else_stmt then
			label2 = intp.get_label
			puts "\t_JUMP\t#{label2}"
			puts "#{label1}:"
			put_list( intp, @else_stmt )
			puts "#{label2}:"
		else
			puts "#{label1}:"
		end

	end
end

class AssignNode < Node
	def initialize( vname, val )
		@vname = vname
		@val = val
	end
	def compile( intp )
		#put 
		#put
		#put
	end
end


#--- main

parser = ScriptPreprocesser.new
cenv = CompileEnviroment.new
begin
	tree = nil
	if ARGV[0] then
		File.open( ARGV[0] ) do |f|
			tree = parser.parse f
		end
	else
		tree = parser.parse $stdin
	end
	tree.compile cenv
end
