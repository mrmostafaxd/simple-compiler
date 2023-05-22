%{
    #include <stdlib.h>
    #include <stdio.h>
    #include <stdbool.h>
    #include <string.h>

    int yyerror(const char* s);
%}
%option yylineno

ALPHA [a-zA-Z]
NUM [0-9]
ALPHANUM [a-zA-Z0-9]

%%

"int"                   return INT;
"float"                 return FLOAT;
"char"                  return CHAR;
"bool"                  return BOOL;
"string"                return STRING;
"const"                 return CONST;
"func"                  return FUNC;
"return"                return RETURN;
"void"                  return VOID;
"enum"                  return ENUM;
"true"                  return TRUE;
"false"                 return FALSE;
"sym"                   return SYM;

"if"                    return IF;
"else if"               return ELIF;
"else"                  return ELSE;
"switch"                return SWITCH;
"case"                  return CASE;
"break"                 return BREAK;
"default"               return DEFAULT;
"for"                   return FOR;
"while"                 return WHILE;
"repeat"                return REPEAT;
"until"                 return UNTIL;

"end"                   return END;

"++"                    return INC;
"--"                    return DEC;

">"                     return GT;
">="                    return GE;
"<"                     return LT;
"<="                    return LE;
"=="                    return EQ;
"!="                    return NEQ;

"||"                    return OR;
"&&"                    return AND;
"!"                     return NOT;

\/\/[^\n]*              ;   /* one-line comments */
[=+\-*/^:(){};,]           {return yytext[0];}

{ALPHA}{ALPHANUM}*      {
                            // yylval.intValue = *yytext-'a';
                            yylval.idValue = strdup(yytext);
                            return VARIABLE;
                        }

\-?{NUM}+               {
                            yylval.intValue = atoi(yytext);
                            return INT_NUM;
                        }

\-?{NUM}*"."{NUM}+      {
                            yylval.floatValue = atof(yytext);
                            return FLOAT_NUM;
                        }

\${ALPHA}{ALPHANUM}+    {
                            yylval.enumValue = strdup(yytext);
                            return ENUM_IDENTIFIER;
                        }

\'.?\'                   {
                            yylval.charValue = yytext[1];
                            return CHAR_VALUE;
                        }

["]([^"]|\\(.|\n))*["]  {
                            yylval.stringValue = strdup(yytext);
                            return STRING_VALUE;
                        }

[ \t\n]*        ;       /* skip whitespace */
.             {yyerror("unknown token");}
%%

int yywrap(void) {
    return 1;
}