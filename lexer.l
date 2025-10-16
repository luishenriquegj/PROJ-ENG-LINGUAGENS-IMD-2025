%{
  #include <stdio.h>
%}

%%

[ \t\n]+                 ;
[0-9]+                  {printf("NUMBER(%s)\n",yytext);}
[0-9]+"."[0-9]+           { printf("FLOAT(%s)\n", yytext); }
[0-9]+("+"|"-")[0-9]+i    { printf("COMPLEX(%s)\n", yytext); }
[a-zA-Z][a-zA-Z0-9]*	{printf("ID(%s)\n",yytext);}
\"([^\"\\]|\\.)*\"     { printf("STRING(%s)\n", yytext); }

/* Palavras Chaves */

"def"          {printf("DEF\n");}
"begin"        {printf("BEGIN\n");}
"end"          {printf("END\n");}
"if"           {printf("IF\n");}
"else"         {printf("ELSE\n");}
"for"          {printf("FOR\n");}
"while"        {printf("WHILE\n");}
"try"          {printf("TRY\n");}
"catch"        {printf("CATCH\n");}
"finally"      {printf("FINALLY\n");}
"print"        {printf("PRINT\n");}
"fun"          {printf("FUNCTION\n");}
"not"          {printf("NOT\n");}
"and"          {printf("AND\n");}
"and"          {printf("OR\n");}
"in"           {printf("IN\n");}
"return"       {printf("RETURN\n"); }
"void"         {printf("VOID\n"); }
"int"          {printf("INT\n"); }
"float"        {printf("FLOAT_TYPE\n"); }
"bool"         {printf("BOOL\n"); }
"true"         {printf("TRUE\n"); }
"false"        {printf("FALSE\n"); }
"char"         {printf("CHAR\n"); }
"null"         {printf("NULL\n"); }


/* Funções */

"length"	{ printf("LENGTH\n"); }
"add"		{ printf("ADD\n"); }
"push"		{ printf("PUSH\n"); }
"remove"	{ printf("REMOVE\n"); }
"delete"	{ printf("DELETE\n"); }
"pop"		{ printf("POP\n"); }
"insert"	{ printf("INSERT\n"); }
"clear"		{ printf("CLEAR\n"); }
"size"		{ printf("SIZE\n"); }

\* Funções Trigonométricas *\

"sen"		{ printf("SEN\n"); }
"cos"		{ printf("COS\n");}
"tan"		{ printf("TAN\n");}
"sec"		{ printf("SEC\n");}
"cotan"		{ printf("COTAN\n");}
"cosec"	    { printf("COSEC\n");}
"archsen"	{ printf("ARCHSEN\n"); }
"archcos"	{ printf("ARCHCOS\n");}
"archtan"	{ printf("ARCHTAN\n"); }

\* Operadores e Pontuação *\

"isEmpty"	{ printf("IS EMPTY\n"); }
"abs"		{ printf("ABSOLUTE\n"); }
"transpose"	{ printf("TRANSPOSE\n"); }
"sqrt"		{ printf("SQUARE ROOT\n"); }
"log"		{ printf("LOG\n"); }
"exp"		{ printf("EXPONENTIAL\n"); }
"angle"		{ printf("ANGLE\n"); }
"r\\*cis"		{ printf("POLAR FORM\n"); }
"occursin"	{ printf("OCCURS IN\n"); }

/* Funções Trigonométricas */

"="		{printf("%s\n",yytext(0));}
"+"		{printf("%s\n",yytext(0));}
"-"		{printf("%s\n",yytext(0));}
"<"		{printf("%s\n",yytext(0));}
">"		{printf("%s\n",yytext(0));}
"!"		{printf("%s\n",yytext(0));}
"|"		{printf("%s\n",yytext(0));}
"&"		{printf("%s\n",yytext(0));}
"*"		{printf("%s\n",yytext(0));}
"/"		{printf("%s\n",yytext(0));}
"%"		{printf("%s\n",yytext(0));}
"^"		{printf("%s\n",yytext(0));}
"("		{printf("%s\n",yytext(0));}
")"		{printf("%s\n",yytext(0));}
"["		{printf("%s\n",yytext(0));}
"]"		{printf("%s\n",yytext(0));}
"{"		{printf("%s\n",yytext(0));}
"}"		{printf("%s\n",yytext(0));}
","    	{printf("%s\n",yytext(0));}
";"    	{printf("%s\n",yytext(0));}
"."		{printf("%s\n",yytext(0));}

"&&"		{ printf("AND_OP\n"); }
"||"		{ printf("OR_OP\n"); }
"=="		{ printf("EQUAL\n"); }
"!="		{ printf("NOTEQUAL\n"); }
"<="		{ printf("LESSEQ\n"); }
">="		{ printf("GREATEREQ\n"); }

/* Comentários */

//.*                       ;   /* ignora comentários de linha */
"/*"([^*]|\*+[^*/])*\*+"/" ;   /* ignora comentários de bloco */
"#".*                     ;   /* ignora comentários tipo */


%%

#ifndef yywrap

   int yywrap (void) {return 1;}
#endif

int main(void)
{
  yylex();
  return 0;
}