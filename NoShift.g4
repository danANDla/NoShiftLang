grammar NoShift;

prog: (stmt)* EOF;

stmt:	varDecl
	| assignment
	| print
	| ifstmt
	| whilestmt
    | block
    ;
        
expr: 	LEFT_PARENTH expr RIGHT_PARENTH		    	#parenthesisExpr
	| left=expr op=(ASTERISK | SLASH) right=expr	#mulDivExpr
	| left=expr op=(PLUS | MINUS) right=expr		#plusMinusExpr
	| left=expr compOperator right=expr				#compExpr
	| left=expr op=(LOGAND | LOGOR | LOGXOR ) right=expr	#logicExpr
	| ID											#idExp
	| NUM											#numExpr
	| MINUS expr									    #invNumExpr
    | STR                                           #strExpr
    | LOGIC_C                                       #logicConstExpr
	;

// описания отдельных выражений и утверждений
varDecl: (INTEGER_TYPE | LOGIC_TYPE | STRING_TYPE) ID ASSIGN expr ';' ;

assignment: ID ASSIGN expr ';'	;

compOperator: op=(LESS | EQUAL | NOT_EQUAL | GREATER ) ;

print: 'print' LEFT_PARENTH expr RIGHT_PARENTH ';'			;
ifstmt:		'if' LEFT_PARENTH expr RIGHT_PARENTH stmt elsestmt? ;
elsestmt:	'else' stmt 			;
whilestmt:	'while' LEFT_PARENTH expr RIGHT_PARENTH stmt	;
block:      'be'  stmt*  'en';



ASTERISK            : 'mu' ;
SLASH               : 'di' ;
PLUS                : 'ad' ;
MINUS               : '-' | 'su';

ASSIGN              : 'eq' ;
EQUAL               : 'is' ;
NOT_EQUAL           : 'nq' ;
LESS                : 'ls' ;
GREATER             : 'gr' ;

LOGAND              : 'and';
LOGOR               : 'or';
LOGXOR              : 'xor';

INTEGER_TYPE		: 'd' ;
LOGIC_TYPE			: 'l' ;
STRING_TYPE			: 's' ;

LEFT_PARENTH		: 'll' ;
RIGHT_PARENTH		: 'rr' ;

LOGIC_C : 'true' | 'false';
STR     : '\'' ~[\n\r]* '\'' ;
ID		: [a-zA-Z_] [a-zA-Z_0-9]* ;
NUM		: [0-9]+ ;

SPACE               : [ \r\n\t]+ -> skip;
LINE_COMMENT        : '//' ~[\n\r]* -> skip;
