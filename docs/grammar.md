```ebnf
program         ::= { importDecl | functionDecl | classDecl | statement }

importDecl      ::= "import" identifier ";" ;
functionDecl    ::= { annotation } "function" identifier "(" [ parameterList ] ")" "->" type block ;
classDecl       ::= "class" identifier "{" { functionDecl } "}" ;
statement       ::= varDecl | exprStmt | returnStmt | ifStmt | forStmt | echoStmt | resetStmt | measureStmt ;

varDecl         ::= [ "final" ] [ annotation ] type identifier [ "=" expression ] ";" ;
returnStmt      ::= "return" [ expression ] ";" ;
ifStmt          ::= "if" "(" expression ")" block ;
forStmt         ::= "for" "(" ( varDecl | exprStmt | ";" ) expression ";" expression ")" block ;
echoStmt        ::= "echo" "(" expression ")" ";" ;
resetStmt       ::= "reset" expression ";" ;
measureStmt     ::= "measure" expression ";" ;
exprStmt        ::= expression ";" ;

annotation      ::= "@" ( "quantum" | "adjoint" | "state" "(" ( char | string ) ")" ) ;
parameterList   ::= parameter { "," parameter } ;
parameter       ::= type identifier ;

expression      ::= assignment ;
assignment      ::= comparison [ "=" assignment ] ;
comparison      ::= addition { ( ">" | "<" | ">=" | "<=" ) addition } ;
addition        ::= multiplication { ( "+" | "-" ) multiplication } ;
multiplication  ::= unary { ( "*" | "/" | "%" ) unary } ;
unary           ::= "-" unary | call ;
call            ::= primary { "(" argumentList ")" } ;
primary         ::= literal | identifier | "measure" expression | "(" expression ")" ;
argumentList    ::= [ expression { "," expression } ] ;
literal         ::= integer | float | char | string ;

type            ::= primitiveType [ "[]" ] ;
primitiveType   ::= "int" | "float" | "string" | "char" | "bit" | "qubit" | "void" ;