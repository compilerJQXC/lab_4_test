#include <stdio.h>

#define NRW        16     // number of reserved words
#define TXMAX      500    // length of identifier table
#define MAXNUMLEN  14     // maximum number of digits in numbers
#define NSYM       14     // maximum number of symbols in array ssym and csym
#define MAXIDLEN   10     // length of identifiers

#define MAXADDRESS 32767  // maximum address
#define MAXLEVEL   32     // maximum depth of nesting block
#define CXMAX      500    // size of code array
#define ENDCX      499

#define MAXSYM     30     // maximum number of symbols  

#define STACKSIZE  1000   // maximum storage

enum symtype
{
	SYM_NULL,
	SYM_IDENTIFIER,
	SYM_NUMBER,
	SYM_PLUS,
	SYM_MINUS,
	SYM_TIMES,
	SYM_SLASH,
	SYM_ODD,
	SYM_EQU,
	SYM_NEQ,
	SYM_LES,
	SYM_LEQ,
	SYM_GTR,
	SYM_GEQ,
	SYM_LPAREN,
	SYM_RPAREN,
	SYM_COMMA,
	SYM_SEMICOLON,
	SYM_PERIOD,
	SYM_BECOMES,
	SYM_BEGIN,
	SYM_END,
	SYM_IF,
	SYM_THEN,
	SYM_WHILE,
	SYM_DO,
	SYM_CALL,
	SYM_CONST,
	SYM_VAR,
	SYM_PROCEDURE,
	//以下为添加部分
	SYM_NOT,//!
	SYM_LEFTSPAREN,//[
	SYM_RIGHTSPAREN,//]
	SYM_ELSE,
	SYM_ELIF,
	SYM_EXIT,
	SYM_RETURN,
	SYM_FOR,
	SYM_AND,//&&
	SYM_OR,//||
	SYM_INC,//++
	SYM_DEC,//--
	SYM_ANDBIT,//&
	SYM_ORBIT,//|
	SYM_XOR,//&
	SYM_MOD,//%
};

enum idtype
{
	ID_CONSTANT, ID_VARIABLE, ID_PROCEDURE, ID_RETURN, ID_ARRAY
};

enum opcode
{
	LIT, OPR, LOD, STO, CAL, INT, JMP, JZ, RET, LODARR, STOARR, JNZ, JE, JNE, JG, JGE, JL, JLE, BAC, JZS, JNZS
};

enum oprcode
{
	OPR_RET, OPR_NEG, OPR_ADD, OPR_MIN,
	OPR_MUL, OPR_DIV, OPR_ODD, OPR_EQU,
	OPR_NEQ, OPR_LES, OPR_LEQ, OPR_GTR,
	OPR_GEQ, OPR_AND, OPR_OR, OPR_NOT,   //9.30号添加了与 或 非操作
	OPR_MOD, OPR_ANDBIT, OPR_ORBIT, OPR_XOR   //10.9添加取模操作,按位与，按位或，异或
};


typedef struct
{
	int f; // function code
	int l; // level
	int a; // displacement address
} instruction;

//////////////////////////////////////////////////////////////////////
char* err_msg[] =
{
	/*  0 */    "",
	/*  1 */    "Found ':=' when expecting '='.",
	/*  2 */    "There must be a number to follow '='.",
	/*  3 */    "There must be an '=' to follow the identifier.",
	/*  4 */    "There must be an identifier to follow 'const', 'var', or 'procedure'.",
	/*  5 */    "Missing ',' or ';'.",
	/*  6 */    "Incorrect procedure name.",
	/*  7 */    "Statement expected.",
	/*  8 */    "Follow the statement is an incorrect symbol.",
	/*  9 */    "'.' expected.",
	/* 10 */    "';' expected.",
	/* 11 */    "Undeclared identifier.",
	/* 12 */    "Illegal assignment.",
	/* 13 */    "':=','++'or'--' expected.",
	/* 14 */    "There must be an identifier to follow the 'call'.",
	/* 15 */    "A constant or variable can not be called.",
	/* 16 */    "'then' expected.",
	/* 17 */    "';' or 'end' expected.",
	/* 18 */    "'do' expected.",
	/* 19 */    "Incorrect symbol.",
	/* 20 */    "Relative operators expected.",
	/* 21 */    "Procedure identifier can not be in an expression.",
	/* 22 */    "Missing ')'.",
	/* 23 */    "The symbol can not be followed by a factor.",
	/* 24 */    "The symbol can not be as the beginning of an expression.",
	/* 25 */    "The number is too great.",
	/* 26 */    "There must be an identifier",
	/* 27 */    "There must be an ','or')'",
	/* 28 */    "Missing '(' ",
	/* 29 */    "",
	/* 30 */    "",
	/* 31 */    "",
	/* 32 */    "There are too many levels."
};

//////////////////////////////////////////////////////////////////////
char ch;         // last character read
int  sym;        // last symbol read
char id[MAXIDLEN + 1]; // last identifier read
int  num;        // last number read
int  cc;         // character count
int  ll;         // line length
int  kk;		
int  err;		//出错总次数
int  cx;         // index of current instruction to be generated.
int presym;
int  level = 0;
int  tx = 0;
int dimDecl = 0;
int readDim = 0;

char line[80];

instruction code[CXMAX];	//存放生成的类伪代码表
/*reserved word*/
char* word[NRW + 1] =
{
	"", /* place holder */
	"begin", "call", "const", "do", "end","if",
	"odd", "procedure", "then", "var", "while",
	"for","return","elif","exit","else"
};

int wsym[NRW + 1] =
{
	SYM_NULL, SYM_BEGIN, SYM_CALL, SYM_CONST, SYM_DO, SYM_END,
	SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE,
	SYM_FOR,SYM_RETURN,SYM_ELIF,SYM_EXIT,SYM_ELSE
};
/*其他符号*/
int ssym[NSYM + 1] =
{
	SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES,
	SYM_LPAREN, SYM_RPAREN, SYM_EQU, SYM_COMMA,
	SYM_PERIOD, SYM_SEMICOLON,SYM_NOT,SYM_LEFTSPAREN,
	SYM_RIGHTSPAREN,SYM_XOR,SYM_MOD
};

char csym[NSYM + 1] =
{
	' ', '+', '-', '*', 
	'(', ')', '=', ',', 
	'.', ';','!','[',
	']','^','%'
};

#define MAXINS   21
/*机器指令集合*/
char* mnemonic[MAXINS] =
{
	"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JZ","RET","LODARR","STOARR","JNZ","JE","JNE","JG","JGE","JL","JLE","BAC","JZS","JNZS"
};
/*符号表 CONSTANT*/
typedef struct
{
	char name[MAXIDLEN + 1];
	int  kind;
	int arrayAdd;
	int  value;
} comtab;

comtab table[TXMAX];
/*符号表 VARIABLE，PROCEDURE*/
typedef struct
{
	char  name[MAXIDLEN + 1];
	int   kind;
	int   arrayAdd;
	short level;
	short address;
} mask;

FILE* infile;

int arrayDim[1000];//用来存储数组的维数和相应维数的大小
int adx = 0;		//当前数组的个数
int temp_adx = 0;
int pcount = 0;

// EOF PL0.h