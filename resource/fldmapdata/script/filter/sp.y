class ScriptPreprocesser

prechigh
	left	'/'	'*'
	left	'-'	'+'
preclow

rule

	program	: stmt_list

	stmt_list	:
				| stmt_list stmt EOL
				| stmt_list EOL


	stmt		: LABEL
					{
						puts "#{val[0]}"
					}
				| COMMAND
					{
						puts "	#{val}"
					}
				| cmd
					{
						puts "	#{val[0].shift} #{val[0]}"
					}
				| MACRO
					{
						puts "#{val[0]}"
					}
				| COMMENT
					{
						puts "#{val[0]}"
					}
				| if_stmt
					{
						puts "	#{val[0][0][0]}"
						puts "	_IF_JUMP	#{val[0][0][1]}, #{val[0][1]}"
						#puts "	_IF_JUMP	#{val[0][0]}, #{val[0][1]}"
					}
				| else_stmt
					{
						puts "	_JUMP	#{val[0][1]}"
						puts "#{val[0][0]}:"
					}
				| endif_stmt
					{
						puts "#{val[0]}:"
					}
				| assign
					{
						puts "	#{val[0]}"
					}
						
	if_stmt		: IF cmp_expr THEN 
					{
						label = sprintf("L%05d", @lcount)
						@lstack.push label
						@lcount += 1
						result = [val[1], label]
					}
	else_stmt	: ELSE 
					{
						if @lstack.size == 0 then raise RuntimeError, 'no if but else' end
						here = @lstack.pop
						label = sprintf("L%05d", @lcount)
						@lstack.push label
						@lcount += 1
						result = [here, label]
					}
	endif_stmt	: ENDIF 
					{
						if @lstack.size == 0 then raise RuntimeError, 'no if but endif' end
						result = @lstack.pop
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
				| expr '-' expr
				| expr '/' expr
				| expr '*' expr
				| '(' expr ')'

	primary		: NUMBER
				| IDENT


	args		: primary
					{
						result = val
					}
				| args ',' primary
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
	@lcount = 0
	@lstack = []
	f.each do |line|
		line.strip!
		until line.empty? do
			case line
			when /^\s*$/, /\A\#.*/, /\/\*.*\*\//, /\/\/.*$/
				@q.push [ :COMMENT, $&]
			when /\A\s+/
				;
			when /\.[a-zA-Z_]+.*$/
				@q.push [ :MACRO, $& ]
			when /\A[a-zA-Z][a-zA-Z0-9_]*:\z/
				@q.push [ :LABEL, $& ]
			when /\A_[a-zA-Z][a-zA-Z0-9_]*/
				@q.push [ :COMMAND, $& ]
			when /\A\d+/
				@q.push [ :NUMBER, $& ]
			when /\A\$[a-zA-Z][a-zA-Z0-9_]*/
				@q.push [ :VARREF, $& ]
			when /\A[a-zA-Z][a-zA-Z0-9_]*/
				if RESERVED.has_key? $& then
					@q.push [ RESERVED[$&], $&.intern ]
				else
					@q.push [ :IDENT, $& ]
				end
			when /\A==/,/\A!=/,/\A>/,/\A</,/\A<=/,/\A>=/
				@q.push [ $&, $& ]
			when /\A./
				@q.push [ $&, $& ]
			else
				puts "#{line}"
				raise RuntimeError, 'fatal error!'
			end
			line = $'
		end
		@q.push [ :EOL, nil ]
	end
	@q.push [ false, nil ]
	
	@yydebug = true
	do_parse
end

def next_token
	@q.shift
end


---- footer
parser = ScriptPreprocesser.new
if ARGV[0] then
  File.open( ARGV[0] ) do |f|
    parser.parse f
  end
else
  parser.parse $stdin
end

