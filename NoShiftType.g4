grammar NoShiftType;

program: (stmt)* EOF;

stmt:	varDecl
	| assignment
	| print
	| ifstmt
	| whilestmt
        ;
        
expr: 	'(' expr ')'					#parenthesisExpr
	| left=expr op=(ASTERISK | SLASH) right=expr	#mulDivExpr
	| left=expr op=(PLUS | MINUS) right=expr	#plusMinusExpr
	| left=expr compOperator right=expr		#compExpr
	| ID						#idExp
	| NUM						#numExpr
	;

// описания отдельных выражений и утверждений
varDecl: (INTEGER_TYPE | LOGIC_TYPE | STRING_TYPE) ID ASSIGN expr ';' ;

assignment: ID ASSIGN expr ';'	;

compOperator: op=(LESS | EQUAL | NOT_EQUAL | GREATER ) ;

print: 'print' '(' expr ')' ';'			;

ifstmt:		'if' '(' expr ')' stmt  elsestmt? ;

elsestmt:	'else' stmt 			;

whilestmt:	'while' '(' expr ')' stmt	;

// список токенов
ID		: [a-zA-Z_] [a-zA-Z_0-9]* ;
NUM		: [0-9]+ ;


ASTERISK            : 'mu' ;
SLASH               : 'di' ;
PLUS                : 'ad' ;
MINUS               : 'su' ;

ASSIGN              : 'eq' ;
EQUAL               : 'is' ;
NOT_EQUAL           : 'nq' ;
LESS                : 'ls' ;
GREATER             : 'gr' ;

INTEGER_TYPE		: 'd' ;
LOGIC_TYPE			: 'l' ;
STRING_TYPE			: 's' ;

SPACE               : [ \r\n\t]+ -> skip;
LINE_COMMENT        : '//' ~[\n\r]* -> skip;
