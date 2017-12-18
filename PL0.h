#include <stdio.h>

#define NRW        26     // number of reserved words
#define TXMAX      1000    // length of identifier table
#define MAXNUMLEN  14     // maximum number of digits in numbers
#define NSYM       19    // maximum number of symbols in array ssym and csym
#define MAXIDLEN   10     // length of identifiers

#define MAXADDRESS 32767  // maximum address
#define MAXLEVEL   32     // maximum depth of nesting block
#define CXMAX      500    // size of code array
#define ENDCX      499

#define MAXSYM     30     // maximum number of symbols  

#define STACKSIZE  1000   // maximum storage

#define STACKSIZE  1000   // maximum storage
#define HEAPSIZE  1000   // maximum storage

enum symtype
{
	SYM_NULL,//0
	SYM_IDENTIFIER,
	SYM_NUMBER,
	SYM_PLUS,
	SYM_MINUS,
	SYM_TIMES,//5
	SYM_SLASH,
	SYM_ODD,
	SYM_EQU,
	SYM_NEQ,
	SYM_LES,//10
	SYM_LEQ,
	SYM_GTR,
	SYM_GEQ,
	SYM_LPAREN,
	SYM_RPAREN,//15
	SYM_COMMA,
	SYM_SEMICOLON,
	SYM_PERIOD,
	SYM_BECOMES,
	SYM_BEGIN,//20
	SYM_END,
	SYM_IF,
	SYM_THEN,
	SYM_WHILE,
	SYM_DO,//25
	SYM_CALL,
	SYM_CONST,
	SYM_VAR,
	SYM_PROCEDURE,
	//以下为添加部分
	SYM_NOT,//!  30
	SYM_LEFTSPAREN,//[
	SYM_RIGHTSPAREN,//]
	SYM_ELSE,
	SYM_ELIF,
	SYM_EXIT,//35
	SYM_RETURN,
	SYM_FOR,
	SYM_AND,//&&
	SYM_OR,//||
	SYM_INC,//++  40
	SYM_DEC,//--
	SYM_ANDBIT,//&
	SYM_ORBIT,//|
	SYM_XOR,//&
	SYM_MOD,//%   45
	SYM_CONTINUE,
	SYM_BREAK,
	SYM_SWITCH,
	SYM_CASE,
	SYM_DEFAULT, //50
	SYM_COLON,//:
	SYM_RANDOM,//random
	SYM_PRINT,
	SYM_GOTO,
	SYM_CALLSTACK,
	SYM_NOTBIT, //~ 55
	SYM_SHL,//<<
	SYM_SHR,//>>
	SYM_PLUSB,//+=
	SYM_MINUSB,//-=
	SYM_TIMESB,//*= 60 
	SYM_SLASHB,// /=
	SYM_ORBITB,//|=
	SYM_XORB,//^=
	SYM_ANDBITB,//&=
	SYM_MODB,//%=   65
	SYM_SHLB,//<<=
	SYM_SHRB,//>>=
	SYM_NOTB,
	SYM_ASK,//?
	SYM_LBRACE,  //{
	SYM_RBRACE,   //}  70
	SYM_ARRAY
};

enum idtype
{
	ID_CONSTANT, ID_VARIABLE, ID_PROCEDURE, ID_RETURN, ID_ARRAY, ID_POINTER
};

enum opcode
{
	LIT, OPR, LOD, STO, CAL, INT, JMP, JZ, RET, LODARR, STOARR, JNZ, JE, JNE, JG, JGE, JL, JLE, BAC, JZS, JNZS
	,  CPY, STOA, LODADD, LODA, PRINT, CALLSTACK, LOD_HEAP, STO_HEAP, LODARR_HEAP, STOARR_HEAP, INCXC, COPY
};

enum oprcode
{
	OPR_RET, OPR_NEG, OPR_ADD, OPR_MIN,
	OPR_MUL, OPR_DIV, OPR_ODD, OPR_EQU,
	OPR_NEQ, OPR_LES, OPR_LEQ, OPR_GTR,
	OPR_GEQ, OPR_AND, OPR_OR, OPR_NOT,   //9.30号添加了与 或 非操作
	OPR_MOD, OPR_ANDBIT, OPR_ORBIT, OPR_XOR,   //10.9添加取模操作,按位与，按位或，异或
	OPR_NOTBIT,OPR_SHL,OPR_SHR
};

enum h_s
{
	HEAP,
	STACK
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
	/* 17 */    "'end' expected.",
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

int  tx1 = 500;//堆变量的符号表下标从500开始
int  tx_bp = 501;  //堆使用这个变量
int  heap_or_stack;  //handle on stack or heap
int initial_flag;

int loopCx[10];
int loopLevel = 0;
int *breakCx[10];
int breakLevel = 0;
int stack[STACKSIZE] = { 0 };
int top;       // top of stack

char line[80];

instruction code[CXMAX];	//存放生成的类伪代码表
/*reserved word*/
char* word[NRW + 1] =
{
	"", /* place holder */
	"begin", "call", "const", "do", "end","if",
	"odd", "procedure", "then", "var", "while",
	"for","return","elif","exit","else","continue",
	"break","switch","case","default","random","print","goto",
	"callstack","array"
};

int wsym[NRW + 1] =
{
	SYM_NULL, SYM_BEGIN, SYM_CALL, SYM_CONST, SYM_DO, SYM_END,
	SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE,
	SYM_FOR,SYM_RETURN,SYM_ELIF,SYM_EXIT,SYM_ELSE,SYM_CONTINUE,
	SYM_BREAK,SYM_SWITCH,SYM_CASE,SYM_DEFAULT,SYM_RANDOM,SYM_PRINT,SYM_GOTO,
	SYM_CALLSTACK,SYM_ARRAY
};
/*其他符号*/
int ssym[NSYM + 1] =
{
	SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES,
	SYM_LPAREN, SYM_RPAREN, SYM_EQU, SYM_COMMA,
	SYM_PERIOD, SYM_SEMICOLON,SYM_NOT,SYM_LEFTSPAREN,
	SYM_RIGHTSPAREN,SYM_XOR,SYM_MOD,SYM_COLON,SYM_NOTBIT,
	SYM_ASK,SYM_LBRACE,SYM_RBRACE
};

char csym[NSYM + 1] =
{
	' ', '+', '-', '*',
	'(', ')', '=', ',',
	'.', ';','!','[',
	']','^','%',':','~',
	'?','{','}'
};

#define MAXINS   33
/*机器指令集合*/
char* mnemonic[MAXINS] =
{
	"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JZ","RET","LODARR","STOARR","JNZ","JE","JNE","JG","JGE","JL","JLE","BAC","JZS","JNZS"
	,"CPY","STOA","LODADD","LODA","PRINT","CALLSTACK","LOD_HEAP","STO_HEAP","LODARR_HEAP","STOARR_HEAP","INCXC","COPY"
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

typedef struct list_def
{
	int cx;
	struct list_def *next;
	struct list_def *tail;
}list;

typedef struct call_n {
	int sn;		//局部变量数量
	int xn;			//形参数量
	int adress;
}call_node;

call_node call_stack[50] = { {0,0,0} };
int cal = 0;  //默认第一个函数为主函数

FILE* infile;

int MulAssignment[50] = { 0 };
int mulAssignCount = 0;
//下面和goto语句有关
char label[100][10] = { 0 };	//label的id
int label_cx[100][2]= { 0 };		//相应label需要回填的cx值以及label对应的level
int l1 = 0 ;			//l1为label的数量
int go[100][60][2] = { 0 };		//label的cx
//以下和数组有关
int arrayDim[1000] = { 0 };//用来存储数组的维数和相应维数的大小
int adx = 0;		//当前数组的个数
int temp_adx = 0;
int pcount = 0;
/*PROCEDURE在符号表中的位置*/
int para_now = 0;
/*实参个数*/
int sc_now = 0;

// EOF PL0.h