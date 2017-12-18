// pl0 compiler source code

#pragma warning(disable:4996)

#include <stdio.h>
#include<time.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "PL0.h"
#include "set.c"

void statement(symset fsys);
//////////////////////////////////////////////////////////////////////
// print error message.
void error(int n)
{
	int i;

	printf("      ");
	for (i = 1; i <= cc - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", n, err_msg[n]);
	err++;
} // error

  //////////////////////////////////////////////////////////////////////
/*从源文件中读取一行到行缓冲区中,将换行作为空格存储，每次从行缓冲区读取一个字符*/
void getch(void)
{
	if (cc == ll)
	{
		if (feof(infile))
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx);	// index of current instruction to be generated.
		while ((!feof(infile)) // added & modified by alex 01-02-09
			&& ((ch = getc(infile)) != '\n'))
		{
			printf("%c", ch);
			line[++ll] = ch;	// ll:line length
		} // while
		printf("\n");
		line[++ll] = ' ';
	}
	ch = line[++cc];
} // getch

  //////////////////////////////////////////////////////////////////////
  // gets a symbol from input stream.
/*词法分析相关程序，跳过空格和制表符，分析单词*/
void getsym(void)
{
	/*记录上一个sym*/
	presym = sym;
	int i, k;
	char a[MAXIDLEN + 1];

	while (ch == ' ' || ch == '\t')
		getch();
	/*对于字母数字串，将词法记号赋给sym，单词赋给id，相当于返回一个(id,sym)即(单词，记号)的二元组*/
	if (isalpha(ch))
	{ // symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < MAXIDLEN)
				a[k++] = ch;
			getch();
		} while (isalpha(ch) || isdigit(ch));//字母开头的字母数字串
		a[k] = 0;
		strcpy(id, a);//单词赋给id
		word[0] = id;
		i = NRW;
		while (strcmp(id, word[i--]));
		if (++i)
			sym = wsym[i]; // symbol is a reserved word
		else
			sym = SYM_IDENTIFIER;   // symbol is an identifier
	}
	else if (isdigit(ch))	//对于数字，返回(num,SYM_NUMBER)，num为数字对应的数值
	{ // symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		do
		{
			num = num * 10 + ch - '0';// last number read
			k++;
			getch();
		} while (isdigit(ch));//以数字开头的数字串
		if (k > MAXNUMLEN)
			error(25);     // The number is too great.
	}
	else if (ch == ':')		//特殊符号只需要返回id即可
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_BECOMES; // :=
			getch();
		}
		else
		{
			sym = SYM_COLON;       // illegal?
		}
	}
	else if (ch == '>')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_GEQ;     // >=
			getch();
		}
		else if (ch == '>')
		{
			sym = SYM_SHR;		//>>
			getch();
			if (ch == '=') {
				sym = SYM_SHRB; //>>=
				getch();
			}
		}
		else
		{
			sym = SYM_GTR;     // >
		}
	}
	else if (ch == '<')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_LEQ;     // <=
			getch();
		}
		else if (ch == '<')
		{
			sym = SYM_SHL;
			getch();			//<<
			if (ch == '=')
			{
				sym = SYM_SHLB;	//<<=
				getch();
			}
		}
		else if (ch == '>')
		{
			sym = SYM_NEQ;     // <>
			getch();
		}
		else
		{
			sym = SYM_LES;     // <
		}
	}
	else if (ch == '/')		//判断注释
	{
		getch();
		if (ch == '/') 
		{
			cc = ll;
			getch();
			getsym();
		}
		else if (ch == '*') 
		{
			getch();
			while (1) 
			{
				while (ch != '*')
				{
					getch();
				}
				getch();
				if (ch == '/')
				{
					getch();
					getsym();
					break;
				}
			}
		}
		else if (ch == '=') {
			sym = SYM_SLASHB;
			getch();
		}
		else {
			sym = SYM_SLASH;
			getch();
		}
	}
	else if (ch == '|') //逻辑运算符的判断
	{
		getch();
		if (ch == '|') 
		{
			sym = SYM_OR;
			getch();
		}
		else if (ch == '=')	// |=
		{
			sym = SYM_ORBITB;
			getch();
		}
		else
			sym = SYM_ORBIT;
	}
	else if (ch == '&')
	{
		getch();
		if (ch == '&')
		{
			sym = SYM_AND;
			getch();
		}
		else if (ch == '=')	// &=
		{
			sym = SYM_ANDBITB;
		}
		else
			sym = SYM_ANDBIT;
	}
	else
	{ // other tokens
		i = NSYM;
		csym[0] = ch;
		while (csym[i--] != ch);
		if (++i)
		{
			sym = ssym[i];
			getch();
			if (sym == SYM_PLUS && ch == '+')
			{
				sym = SYM_INC;
				getch();
			}
			else if (sym == SYM_MINUS&&ch == '-')
			{
				sym = SYM_DEC;
				getch();
			}
			else if (sym == SYM_PLUS && ch == '=')
			{
				sym = SYM_PLUSB;
				getch();
			}
			else if (sym == SYM_MINUS && ch == '=')
			{
				sym = SYM_MINUSB;
				getch();
			}
			else if (sym == SYM_MOD && ch == '=')
			{
				sym = SYM_MODB;
				getch();
			}
			else if (sym == SYM_TIMES && ch == '=')
			{
				sym = SYM_TIMESB;
				getch();
			}
			else if (sym == SYM_XOR && ch == '=')
			{
				sym = SYM_XORB;
				getch();
			}
			else if (sym == SYM_NOT && ch == '=')
			{
				sym = SYM_SLASHB;
				getch();
			}
		}
		else
		{
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
} // getsym

  //////////////////////////////////////////////////////////////////////
  // generates (assembles) an instruction.
/*目标代码生成相关函数
x:要生成一行代码的助记符
y,z:两个操作数
*/
void gen(int x, int y, int z)
{
	if (cx > CXMAX) // size of code array
	{
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx++].a = z;
} // gen

  //////////////////////////////////////////////////////////////////////
  // tests if error occurs and skips all symbols that do not belongs to s1 or s2.
/*
s1:当语法分析进入或退出某一语法单元时当前单词符合应属于的集合
s2:在某一出错状态下，可恢复语法分析正常工作的补充单词集合
n:出错信息编号，当当前符号不属于合法的 s1 集合时发出的出错信息
*/
void test(symset s1, symset s2, int n)
{
	symset s;

	if (!inset(sym, s1))
	{
		error(n);
		printf("\nnow is %d\n", sym);
		s = uniteset(s1, s2);
		while (!inset(sym, s))
			getsym();
		destroyset(s);
	}
} // test

  //////////////////////////////////////////////////////////////////////
int dx;  // data allocation index
int dx1;
// enter object(constant, variable or procedre) into table.
/*参数k:欲登陆到符号表的类型*/
void enter(int kind)
{
	mask* mk;
	if (heap_or_stack == STACK) {
		tx++;//符号表产生一个新的空位
		strcpy(table[tx].name, id);
		table[tx].kind = kind;
		switch (kind)
		{
		case ID_CONSTANT:
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			table[tx].value = num;
			break;
		case ID_VARIABLE:
			mk = (mask*)&table[tx];
			mk->level = level;
			mk->address = dx++;
			break;
		case ID_PROCEDURE:
			mk = (mask*)&table[tx];
			mk->arrayAdd = 0;
			mk->level = level;
			break;
		case ID_POINTER:
			mk = (mask*)&table[tx];
			mk->level = level;
			mk->address = dx++;
			break;
		} // switch
	}
	else  //heap_or_stack == HEAP
	{
		tx1++;
		strcpy(table[tx1].name, id);
		table[tx1].kind = kind;
		switch (kind)
		{
		case ID_CONSTANT:
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			table[tx1].value = num;
			break;
		case ID_VARIABLE:
			mk = (mask*)&table[tx1];
			mk->level = level;
			mk->address = dx1++;
			break;
		case ID_PROCEDURE:
			mk = (mask*)&table[tx1];
			mk->level = level;
			break;
		case ID_POINTER:
			mk = (mask*)&table[tx1];
			mk->level = level;
			mk->address = dx1++;
		} // switch
	}
} // enter
/*登陆符号表*/
void enterPara(char *idTemp, int kind)
{
	mask* mk;
	tx++;
	strcpy(table[tx].name, idTemp);
	table[tx].kind = kind;
	switch (kind)
	{
	case(ID_VARIABLE):
		mk = (mask*)&table[tx];
		mk->level = level;
		mk->address = pcount--;
		break;
	case(ID_RETURN):
		mk = (mask*)&table[tx];
		mk->level = level;
		// printf("The pcount is %d ***************  \n",pcount);
		mk->address = pcount + 1;
	}
	// printf("Message of var in table is : name = %s  level = %d  address = %d  \n",table[tx].name,mk->level,(int)(mk->address));
}
  //////////////////////////////////////////////////////////////////////
  // locates identifier in symbol table.
/*
在符号表中查找指定符号所在的位置
参数：要找的符号
*/
int position_label(char *id)
{
	int i;
	strcpy(label[0], id);
	i = l1+1;
	while (strcmp(label[--i], id) != 0);
	return i;
}
int position(char* id)
{
	if (tx1 < tx_bp)  //没有可用的局部变量
	{
		int i;
		strcpy(table[0].name, id);
		i = tx + 1;
		while (strcmp(table[--i].name, id) != 0);
		if (i != 0) heap_or_stack = STACK;
		return i;
	}
	else  //先查局部变量表，再查全局变量表
	{
		int i = tx1;
		while (i >= tx_bp)
		{
			if (strcmp(table[i].name, id) != 0)
				i--;
			else break;
		}
		if (i == tx_bp - 1)
		{
			strcpy(table[0].name, id);
			i = tx + 1;
			while (strcmp(table[--i].name, id) != 0);
			if (i != 0) heap_or_stack = STACK;
			return i;
		}
		heap_or_stack = HEAP;
		return i;
	}
} // position
//数组登入符号表
void enterArray()
{
	mask *mk;
	if (heap_or_stack == STACK)
	{
		table[tx].kind = ID_ARRAY;
		mk = (mask *)&table[tx];
		mk->level = level;
		mk->address = dx;
		mk->arrayAdd = ++adx;
		arrayDim[adx] = 0;
	}
	else  //HEAP
	{
		table[tx1].kind = ID_ARRAY;
		mk = (mask *)&table[tx1];
		mk->level = level;
		mk->address = dx1;
		mk->arrayAdd = ++adx;
		arrayDim[adx] = 0;
	}
}

void arrayDecl()
{
	void initial_list();
	if (presym == SYM_LEFTSPAREN)
	{
		if (sym == SYM_RIGHTSPAREN)  // ']'  //a[]  must be initialed
		{
			initial_flag = 1;
			arrayDim[adx]++;
			if (arrayDim[adx] != 1)
			{
				err++;
				printf("a[][]...,not valid\n");
				exit(1);
			}
			arrayDim[adx + arrayDim[adx]] = 0;
			getsym();
			if (sym == SYM_LEFTSPAREN)
			{
				getsym();
				arrayDecl();
				return;
			}
			else if (sym == SYM_EQU)
			{
				getsym();
				initial_list();
				return;
			}
			else
			{
				err++;
				printf("must be initialed\n");
			}

		}
		else if (sym != SYM_NUMBER)
		{
			printf("expected number when declare array.\n");
			err++;
			return;
		}
		else 	// sym == SYM_NUMBER
		{
			arrayDim[adx]++;
			arrayDim[adx + arrayDim[adx]] = num;
			getsym();
		}
		if (sym != SYM_RIGHTSPAREN)
		{
			printf("expected rightsparen here while declare array at dim %d\n", arrayDim[adx]);
			err++;
			return;
		}
		else
		{
			getsym();
			arrayDecl();
		}
	}
	else if (presym == SYM_RIGHTSPAREN)
	{
		if (initial_flag == 0)
		{
			if (sym == SYM_LEFTSPAREN)
			{
				getsym();
				arrayDecl();
			}
			else if (sym == SYM_EQU)  //=
			{
				getsym();
				initial_list();
				return;
			}
			else // ; or ,
			{
				int count = 1;
				int i;
				for (i = 1; i <= arrayDim[adx]; i++)
				{
					count *= arrayDim[adx + i];
				}
				if (heap_or_stack == STACK)
					dx += count - 1;
				else dx1 += count - 1;
				return;
			}
		}
		else  //initial_flag == 1
		{
			if (sym == SYM_LEFTSPAREN)
			{
				getsym();
				arrayDecl();
			}
			else if (sym == SYM_EQU)//=
			{
				getsym();
				initial_list();
				return;
			}
			else
			{
				err++;
				printf("must be initialed\n");
			}

		}
	}
	else
	{
		printf("expected '[' or ']'\n");
		err++;
		getsym();
		return;
	}
}
/*传入数组声明在符号表中的位置
动作：将数组元素相对于基地址的元素置于栈顶
*/
void calAdd(int i)
{
	void expression_orbit(symset fsys);
	mask *mk = (mask *)&table[i];
	int temp = mk->arrayAdd;

	if (sym == SYM_LEFTSPAREN)//左中括号
	{
		if (presym != SYM_IDENTIFIER && presym != SYM_RIGHTSPAREN)
		{
			printf("expected identifier or rightsparen after leftsparen\n");
			err++;
		}
		else
		{
			getsym();
			expression_orbit(uniteset(createset(SYM_RIGHTSPAREN, SYM_NULL),statbegsys));
			readDim++;//已经读入的数组的维数
			gen(OPR, 0, OPR_ADD);//先加
			if (readDim == arrayDim[temp])
			{
				// gen(LODARR,0,mk->address); //LODARR undeclared!
				if (sym != SYM_RIGHTSPAREN)
				{
					printf("expected rightsparen after expression\n");
					err++;
					return;
				}
				else
				{
					getsym();
					return;
				}
			}
			else
			{
				gen(LIT, 0, arrayDim[temp + readDim + 1]);
				gen(OPR, 0, OPR_MUL);//再乘
				if (sym != SYM_RIGHTSPAREN)
				{
					printf("expected rightsparen after expression\n");
					err++;
					return;
				}
				else
				{
					getsym();
					calAdd(i);
				}
			}
		} //else
	}
	else
	{
		printf("expected leftsparen\n");
		return;
	}
}
 //////////////////////////////////////////////////////////////////////
/*常量声明赋值过程*/
void constdeclaration()
{
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (sym == SYM_EQU || sym == SYM_BECOMES)
		{
			if (sym == SYM_BECOMES)
				error(1); // Found ':=' when expecting '='.
			getsym();
			if (sym == SYM_NUMBER)
			{
				enter(ID_CONSTANT);
				getsym();
			}
			else
			{
				error(2); // There must be a number to follow '='.
			}
		}
		else
		{
			error(3); // There must be an '=' to follow the identifier.
		}
	}
	else	error(4);
	// There must be an identifier to follow 'const', 'var', or 'procedure'.
} // constdeclaration

  //////////////////////////////////////////////////////////////////////
/*变量声明处理过程*/
void vardeclaration(void)
{
	mask *mk;
	if (sym == SYM_IDENTIFIER)
	{

		enter(ID_VARIABLE);
		getsym();
		if (sym == SYM_EQU)  //=
		{
			getsym();
			if (sym == SYM_NUMBER)  //生成两条代码
			{
				if (heap_or_stack == HEAP)
				{
					mk = (mask*)&table[tx1];
					gen(LIT, 0, num);
					gen(STO_HEAP, 0, mk->address);
					getsym();
				}
				else    //STACK
				{
					mk = (mask*)&table[tx];
					gen(LIT, 0, num);
					gen(STO, 0, mk->address);
					getsym();
				}
			}
			else
			{
				err++;
				printf("the var must be initialed with number\n");
			}

		}
		else if (sym == SYM_LEFTSPAREN)
		{
			initial_flag = 0;
			getsym();
			enterArray();
			arrayDecl();
		}
	}
	else if (sym == SYM_TIMES)
	{
		getsym();
		if (sym == SYM_IDENTIFIER)
		{
			enter(ID_POINTER);
			getsym();
		}
		else
		{
			printf("expected id as a pointer declaration\n");
			exit(1);
		}
	}
	else
	{
		printf("Error:expected SYM_IDENTIFIER in vardeclaration\n");
		err++;
	}
} // vardeclaration
/*声明goto*/
void gotodeclaration()
{
	if (presym == SYM_IDENTIFIER)
	{
		int i = position_label(id);
		if (i==0||label_cx[i][0] == 0) {
			//还没出现过这个标签
			l1++;
			strcpy(label[l1], id);
			label_cx[l1][0] = cx++;
			cx--;
			label_cx[l1][1] = level;
		}
		else {
			//这个标签出现过,即在前面有goto语句
			label_cx[i][0] = cx++;
			cx--;
			label_cx[i][1] = level;
			int now = label_cx[i][0]; //应该回填的位置
			int j = 0;
			for (j = 0; go[i][j][0] != 0; j++)
			{
				if (go[i][j][1] == level) {
					code[go[i][j][0]].a = now;
				}
			}
		}
	}
}
  
  /*参数列表的分析过程*/
void paraList(symset fsys)
{
	symset set1, set;
	set1 = createset(SYM_RPAREN, SYM_COMMA, SYM_NULL);
	set = createset(SYM_IDENTIFIER, SYM_RPAREN, SYM_TIMES, SYM_NULL);//presym非标识符时的合法的开始符号
	if (presym == SYM_LPAREN)/*前一个符号是左括号*/
	{
		test(set, fsys, 26);//"There must be an identifier"
		if (sym == SYM_IDENTIFIER)/*遇到标识符*/
		{
			mask *mk;
			mk = (mask *)&table[para_now];
			mk->arrayAdd++;
			char idTemp[MAXIDLEN + 1];
			strcpy(idTemp, id);
			getsym();
			paraList(fsys);
			enterPara(idTemp, ID_VARIABLE);
		}
		else if (sym == SYM_RPAREN) 
		{
			return;
		}
		else if (sym == SYM_TIMES)
		{
			getsym();
			if (sym == SYM_IDENTIFIER)
			{
				mask *mk;
				mk = (mask *)&table[para_now];
				mk->arrayAdd++;
				char idTemp[MAXIDLEN + 1];
				strcpy(idTemp, id);
				getsym();
				paraList(fsys);
				enterPara(idTemp, ID_POINTER);
			}
			else
			{
				printf("expected identifier in paraList\n");
				exit(1);
			}
		}
	}
	else if (presym == SYM_COMMA)/*前一个符号是逗号*/
	{
		test(set, fsys, 26);//"There must be an identifier"
		if (sym == SYM_IDENTIFIER)/*当前符号是标识符*/
		{
			mask *mk;
			mk = (mask *)&table[para_now];
			mk->arrayAdd++;
			char idTemp[MAXIDLEN + 1];
			strcpy(idTemp, id);
			getsym();
			paraList(fsys);
			enterPara(idTemp, ID_VARIABLE);
		}
		else if (sym == SYM_TIMES)
		{
			mask *mk;
			mk = (mask *)&table[para_now];
			mk->arrayAdd++;
			getsym();
			if (sym == SYM_IDENTIFIER)
			{
				char idTemp[MAXIDLEN + 1];
				strcpy(idTemp, id);
				getsym();
				paraList(fsys);
				enterPara(idTemp, ID_POINTER);
			}
			else
			{
				printf("expected identifier in paraList\n");
				exit(1);
			}
		}
		else/*否则报错*/
		{
			error(26);//"The paralist is wrong",
		}
	}
	else if (presym == SYM_IDENTIFIER)/*前一个符号是标识符*/
	{
		test(set1, fsys, 27);//There must be an ','or')'
		if (sym == SYM_COMMA)
		{
			getsym();
			paraList(fsys);

		}
		/*不是逗号的话返回*/
	}
	else
	{
		error(26);
	}
	destroyset(set1);
	destroyset(set);
}
  //////////////////////////////////////////////////////////////////////
/*list code generated for this block*/
void listcode(int from, int to)
{
	int i;

	printf("\n");
	for (i = from; i < to; i++)
	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
} // listcode

void procedureCall(symset fsys)
{
	void expression_orbit(symset fsys);
	int i;
	mask *mk;
	if (inset(sym,facbegsys))// 
	{
		gen(INCXC, 0, 0);
		sc_now++;
		expression_orbit(uniteset(createset(SYM_COMMA, SYM_RPAREN, SYM_NULL),fsys));
		procedureCall(fsys);
	}
	else if (sym == SYM_COMMA)
	{
		/*
		if (presym != SYM_IDENTIFIER && presym != SYM_NUMBER && presym != SYM_INC && presym != SYM_DEC)
		{
			printf("Error in procedureCall 2\n");
			error(26);
		}
		else
		{
			getsym();
			procedureCall(fsys);
		}
		*/
		getsym();
		procedureCall(fsys);
	}
	else if (sym == SYM_RPAREN)
	{
		/*
		if (presym != SYM_IDENTIFIER && presym != SYM_LPAREN && presym != SYM_NUMBER && presym != SYM_RIGHTSPAREN && presym != SYM_RPAREN && presym != SYM_INC && presym != SYM_DEC)
		{
			printf("Error in procedureCall 3\n");
			error(26);
		}
		else
		{
			gen(INT, 0, 1);
			getsym();
			return;
		}  // for return value
		*/
		gen(INT, 0, 1);
		getsym();
	}
}

void TimesBody(symset fsys)
{
	if (sym == SYM_TIMES)
	{
		getsym();
		TimesBody(fsys);
		gen(LODA, 0, 0);
	}
	else if (sym == SYM_IDENTIFIER)
	{
		int i = position(id);
		mask *mk = (mask*)&table[i];
		if (i)
		{
			if (heap_or_stack == STACK)
				gen(LOD, level - mk->level, mk->address);
			else gen(LOD_HEAP, 0, mk->address);
			getsym();
		}
		else
		{
			printf("the id not declared in TimesBody\n");
			exit(1);
		}
	}
}

  //////////////////////////////////////////////////////////////////////
/*因子处理过程*/
/*fsys：如果出错可用来恢复语法分析的符号集合*/
void factor(symset fsys)
{
	void expression_ask(symset fsys);
	void statement(symset fsys);
	int i;
	symset set;

	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.

	if (inset(sym, facbegsys))
	{
		if (sym == SYM_IDENTIFIER)/*如果遇到标识符*/
		{
			int j = position(id);
			int h_s_temp = heap_or_stack;
			mask *mk = (mask *)&table[j];
			if (j)
			{

				if (table[j].kind == ID_PROCEDURE)//procedure Call
				{
					// printf("Is that correct? the ooxx is %s \n",table[j].name);
					getsym();
					if (sym == SYM_LPAREN)
					{
						getsym();
						procedureCall(fsys);
						gen(CAL, level - mk->level, mk->address);
					}
				}
				//**********20171102************
				else if (table[j].kind == ID_ARRAY)
				{
					getsym();
					gen(LIT, 0, 0);
					readDim = 0;
					calAdd(j);
					if (sym == SYM_BECOMES)
					{
						getsym();
						expression_ask(fsys);
						if (h_s_temp == STACK)
							gen(STOARR, level - mk->level, mk->address);
						else gen(STOARR_HEAP, 0, mk->address);
						gen(CPY, 0, 2);
					}
					else if (sym == SYM_INC)
					{
						gen(COPY, 0, 2);
						if (h_s_temp == STACK)
							gen(LODARR, level - mk->level, mk->address);
						else gen(LODARR_HEAP, 0, mk->address);
						gen(LIT, 0, 1);
						gen(OPR, 0, OPR_ADD);
						if (h_s_temp == STACK)
						{
							gen(STOARR, level - mk->level, mk->address);
							gen(LODARR, level - mk->level, mk->address);
						}
						else
						{
							gen(STOARR, level - mk->level, mk->address);
							gen(LODARR, level - mk->level, mk->address);
						}
						gen(LIT, 0, 1);
						gen(OPR, 0, OPR_MIN);
						getsym();
					}
					else if (sym == SYM_DEC)
					{
						gen(COPY, 0, 2);
						if (h_s_temp == STACK)
							gen(LODARR, level - mk->level, mk->address);
						else gen(LODARR_HEAP, 0, mk->address);
						gen(LIT, 0, 1);
						gen(OPR, 0, OPR_MIN);
						if (h_s_temp == STACK)
						{
							gen(STOARR, level - mk->level, mk->address);
							gen(LODARR, level - mk->level, mk->address);
						}
						else
						{
							gen(STOARR, level - mk->level, mk->address);
							gen(LODARR, level - mk->level, mk->address);
						}
						gen(LIT, 0, 1);
						gen(OPR, 0, OPR_ADD);
						getsym();
					}
					else
					{
						if (h_s_temp == STACK)
							gen(LODARR, level - mk->level, mk->address);
						else gen(LODARR_HEAP, 0, mk->address);
					}
				}
				//******************************
				else if (table[j].kind == ID_CONSTANT)
				{
					gen(LIT, 0, table[j].value);
					getsym();
				}
				else // normal variable
				{
					// printf("Is that correct ? kind = %d \n",table[j].kind);
					mask* mk;
					mk = (mask*)&table[j];
					getsym();
					if (sym == SYM_BECOMES)
					{
						getsym();
						expression_ask(fsys);
						if (h_s_temp == STACK)
							gen(STO, level - mk->level, mk->address);
						else gen(STO_HEAP, 0, mk->address);
						gen(CPY, 0, 1);
					}
					else if (sym == SYM_INC)
					{
						if (h_s_temp == STACK)
						{

								gen(LOD, level - mk->level, mk->address);
								gen(LOD, level - mk->level, mk->address);
								gen(LIT, 0, 1);
								gen(OPR, 0, OPR_ADD);
								gen(STO, level - mk->level, mk->address);
						}
						else
						{
							gen(LOD_HEAP, 0, mk->address);
							printf("target in SYM_INC\n");
							gen(LOD_HEAP, 0, mk->address);
							gen(LIT, 0, 1);
							gen(OPR, 0, OPR_ADD);
							gen(STO_HEAP, 0, mk->address);
						}
						getsym();
					}
					else if (sym == SYM_DEC)
					{
						if (h_s_temp == STACK)
						{
							gen(LOD, level - mk->level, mk->address);
							gen(LOD, level - mk->level, mk->address);
							gen(LIT, 0, 1);
							gen(OPR, 0, OPR_MIN);
							gen(STO, level - mk->level, mk->address);
						}
						else
						{
							gen(LOD_HEAP, 0, mk->address);
							gen(LOD_HEAP, 0, mk->address);
							gen(LIT, 0, 1);
							gen(OPR, 0, OPR_MIN);
							gen(STO_HEAP, 0, mk->address);
						}
						getsym();
					}
					else
					{
						if (h_s_temp == STACK)
							gen(LOD, level - mk->level, mk->address);
						else
							gen(LOD_HEAP, 0, mk->address);
					}
					// printf("After return q(n the sym is : %d\n",sym);
				}
			}
			else
			{
				printf("Error : false in factor that Undeclared id\n");
				getsym();
			}
		}
		else if (sym == SYM_NUMBER)
		{
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			/*生成指令，把数字的字面值放在栈顶*/
			gen(LIT, 0, num);
			getsym();
		}
		else if (sym == SYM_RANDOM) 
		{
			getsym();
			if (sym != SYM_LPAREN)
			{
				error(28);
			}
			getsym();
			if (sym == SYM_RPAREN) 
			{
				int r = rand();
				gen(LIT, 0, r);
			}
			else {
				if (sym == SYM_NUMBER)
				{
					int r = rand() % num;
					gen(LIT, 0, r);
					getsym();//get LPAREN
				}
				else
				{
					printf("unexpected in random\n");
					err++;
				}
			}
			getsym();
		}
		else if (sym == SYM_LPAREN)
		{
			getsym();
			/*在这里，表达式的预期符号有）因此加入SYM_RPAREN*/
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			expression_ask(set);
			destroyset(set);
			if (sym == SYM_RPAREN)/*表达式处理完后，应该遇到右括号*/
			{
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}
		}
		else if (sym == SYM_MINUS) // UMINUS,  Expr -> '-' Expr||!Expr
		{
			getsym();
			expression_ask(fsys);
			/*生成指令，将栈顶元素取反*/
			gen(OPR, 0, OPR_NEG);
		}

		test(fsys, createset(SYM_LPAREN, SYM_RPAREN, SYM_NULL), 23);
	} // if
} // factor
////////////////////////////////////////////////////////////////////
void pre_factor(symset fsys)
{
	void expression_ask(symset fsys);
	void statement(symset fsys);
	int i;
	symset set;
	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.
	if (sym == SYM_NOT)
	{
		getsym();
		expression_ask(fsys);
		/*生成指令，添加下面的非*/
		gen(OPR, 0, OPR_NOT);
	}
	else if (sym == SYM_INC) // ++
	{
		getsym();
		if (sym != SYM_IDENTIFIER)
		{
			printf("expected id here \n");
			err++;
			getsym();
		}
		else
		{
			int i = position(id);
			int h_o_s = heap_or_stack;
			mask *mk = (mask *)&table[i];
			if (table[i].kind == ID_VARIABLE) 
			{
				if (h_o_s == STACK)
					gen(LOD, level - mk->level, mk->address);//将变量值置于栈顶
				else gen(LOD_HEAP, 0, mk->address);
				gen(LIT, 0, 1);//常数置于栈顶
				gen(OPR, 0, OPR_ADD);//相加
				if (h_o_s == STACK)
				{
					gen(STO, level - mk->level, mk->address);//将栈顶的值赋予变量
					gen(LOD, level - mk->level, mk->address);//将变量值置于栈顶
				}
				else
				{
					gen(STO_HEAP, 0, mk->address);//将栈顶的值赋予变量
					gen(LOD_HEAP, 0, mk->address);//将变量值置于栈顶
				}
				getsym();
			}
			else if (table[i].kind == ID_ARRAY)
			{
				getsym();
				gen(LIT, 0, 0);
				readDim = 0;
				calAdd(i);
				gen(COPY, 0, 2);
				if (heap_or_stack == STACK)
					gen(LODARR, level - mk->level, mk->address);
				else gen(LODARR_HEAP, 0, mk->address);
				gen(LIT, 0, 1);
				gen(OPR, 0, OPR_ADD);
				if (heap_or_stack == STACK)
				{
					gen(STOARR, level - mk->level, mk->address);
					gen(LODARR, level - mk->level, mk->address);
				}
				else
				{
					gen(STOARR, level - mk->level, mk->address);
					gen(LODARR, level - mk->level, mk->address);
				}
			}
		}
	}
	else if (sym == SYM_DEC)
	{
		getsym();
		if (sym != SYM_IDENTIFIER)
		{
			printf("expected id here \n");
			err++;
			getsym();
		}
		else
		{
			int i = position(id);
			mask *mk = (mask *)&table[i];
			if (table[i].kind == ID_VARIABLE)
			{
				if (heap_or_stack == STACK)
					gen(LOD, level - mk->level, mk->address);//将变量值置于栈顶
				else gen(LOD_HEAP, 0, mk->address);
				gen(LIT, 0, 1);//常数置于栈顶
				gen(OPR, 0, OPR_MIN);//相加
				if (heap_or_stack == STACK)
				{
					gen(STO, level - mk->level, mk->address);//将栈顶的值赋予变量
					gen(LOD, level - mk->level, mk->address);//将变量值置于栈顶
				}
				else
				{
					gen(STO_HEAP, 0, mk->address);//将栈顶的值赋予变量
					gen(LOD_HEAP, 0, mk->address);//将变量值置于栈顶
				}
				getsym();
			}
			else if (table[i].kind == ID_ARRAY)
			{
				getsym();
				gen(LIT, 0, 0);
				readDim = 0;
				calAdd(i);
				gen(COPY, 0, 2);
				if (heap_or_stack == STACK)
					gen(LODARR, level - mk->level, mk->address);
				else gen(LODARR_HEAP, 0, mk->address);
				gen(LIT, 0, 1);
				gen(OPR, 0, OPR_MIN);
				if (heap_or_stack == STACK)
				{
					gen(STOARR, level - mk->level, mk->address);
					gen(LODARR, level - mk->level, mk->address);
				}
				else
				{
					gen(STOARR, level - mk->level, mk->address);
					gen(LODARR, level - mk->level, mk->address);
				}
			}
		}
	}
	else if (sym == SYM_ANDBIT)//取地址符
	{
		getsym();
		if (sym == SYM_IDENTIFIER)
		{
			int i = position(id);
			if (i || table[i].kind == ID_ARRAY || table[i].kind == ID_VARIABLE)
			{
				mask *mk = (mask *)&table[i];
				gen(LODADD, level - mk->level, mk->address);
				getsym();
			}
			else
			{
				printf("There are some errors in SYM_ANDBIT factor\n");
				exit(1);
			}
		}
		else
		{
			printf("expected SYM_IDENTIFIER after LODADD SYM_ANDBIT\n");
			exit(1);
		}
	}
	else if (sym == SYM_TIMES)//取值
	{
		getsym();
		TimesBody(uniteset(createset(SYM_TIMES, SYM_NULL), fsys));
		gen(LODA, 0, 0);
	}
	else if (sym == SYM_NOTBIT)
	{
		getsym();
		expression_ask(fsys);
		gen(OPR, 0, OPR_NOTBIT);
	}
	else {
		factor(fsys);
	}
}
/////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
void term(symset fsys)
{
	int mulop;
	symset set;

	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_MOD ,SYM_NULL));
	pre_factor(set);
	while (sym == SYM_TIMES || sym == SYM_SLASH || sym == SYM_MOD)
	{
		mulop = sym;
		getsym();
		pre_factor(set);
		if (mulop == SYM_TIMES)
		{
			gen(OPR, 0, OPR_MUL);
		}
		else if(mulop == SYM_SLASH)
		{
			gen(OPR, 0, OPR_DIV);
		}
		else
		{
			gen(OPR, 0, OPR_MOD);
		}
	} // while
	destroyset(set);
} // term

  //////////////////////////////////////////////////////////////////////
void expression(symset fsys)
{
	int addop;
	/*addop用来记录算术表达式*/
	symset set;

	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_NULL));

	term(set);
	while (sym == SYM_PLUS || sym == SYM_MINUS)
	{
		addop = sym;
		getsym();
		term(set);
		if (addop == SYM_PLUS)
		{
			gen(OPR, 0, OPR_ADD);
		}
		else
		{
			gen(OPR, 0, OPR_MIN);
		}
	} // while

	destroyset(set);
} // expression
/*添加按位与，按位或，按位异或*/
void expression_sh(symset fsys)
{
	int addop;
	symset set;
	set = uniteset(fsys, createset(SYM_SHL, SYM_SHR, SYM_NULL));
	expression(set);
	while (sym == SYM_SHL || sym == SYM_SHR)
	{
		addop = sym;
		getsym();
		expression(set);
		if (addop == SYM_SHL)
		{
			gen(OPR, 0, OPR_SHL);
		}
		else
		{
			gen(OPR, 0, OPR_SHR);
		}
	}
	destroyset(set);
}
void expression_andbit(symset fsys)//&
{
	symset set;

	set = uniteset(fsys, createset(SYM_ANDBIT, SYM_NULL));
	expression_sh(set);
	while (sym == SYM_ANDBIT)
	{
		getsym();
		expression_sh(set);
		gen(OPR, 0, OPR_ANDBIT);
	}
	destroyset(set);
}
void expression_xorbit(symset fsys)//^
{
	symset set;
	set = uniteset(fsys, createset(SYM_XOR, SYM_NULL));
	expression_andbit(set);
	while (sym == SYM_XOR)
	{
		getsym();
		expression_andbit(set);
		gen(OPR, 0, OPR_XOR);
	}
	destroyset(set);
}

void expression_orbit(symset fsys)//|
{
	symset set;
	set = uniteset(fsys, createset(SYM_ORBIT, SYM_NULL));
	expression_xorbit(set);
	while (sym == SYM_ORBIT)
	{
		getsym();
		expression_xorbit(set);
		gen(OPR, 0, OPR_OR);
	}
	destroyset(set);
}
/*左移右移*/

  //////////////////////////////////////////////////////////////////////
void condition(symset fsys)
{
	/*relop用来记录逻辑表达式*/
	int relop;
	symset set;

	if (sym == SYM_ODD)
	{
		getsym();
		expression_orbit(fsys);
		gen(OPR, 0, 6);
	}
	else
	{
		set = uniteset(relset, fsys);
		expression_orbit(set);
		destroyset(set);
		if (!inset(sym, relset))
		{
			/*error(20);*/ // 删去这里的出错判断，令条件泛化为表达式
		}
		else
		{
			relop = sym;
			getsym();
			expression_orbit(fsys);
			switch (relop)
			{
			case SYM_EQU:
				gen(OPR, 0, OPR_EQU);
				break;
			case SYM_NEQ:
				gen(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				gen(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				gen(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				gen(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				gen(OPR, 0, OPR_LEQ);
				break;
			} // switch
		} // else
	} // else
} // condition
//添加逻辑算符
void conditions_and(symset fsys)
{
	symset set;
	set = uniteset(fsys, createset(SYM_AND, SYM_NULL));
	condition(set);
	while (sym == SYM_AND)
	{
		getsym();
		condition(set);
		gen(OPR, 0, OPR_AND);
	}
	destroyset(set);
}
void conditions_or(symset fsys)
{
	symset set;
	set = uniteset(fsys, createset(SYM_OR, SYM_NULL));
	conditions_and(set);
	while (sym == SYM_OR)
	{
		getsym();
		conditions_and(set);
		gen(OPR, 0, OPR_OR);
	}
	destroyset(set);
}
void expression_ask(symset fsys)//三目运算符
{
	symset set1, set;
	set1 = createset(SYM_ASK, SYM_COLON, SYM_NULL);
	set = uniteset(set1, fsys);

	conditions_or(set);
	if (sym == SYM_ASK)
	{
		gen(JZ, 0, 0);
		int f = cx - 1;
		getsym();
		expression_ask(set);//E1
		if (sym != SYM_COLON)
		{
			printf("Missing :");
			err++;
		}
		getsym();
		gen(JMP, 0, 0);
		int end = cx - 1;
		code[f].a = cx;
		expression_ask(set);//E2
		code[end].a = cx;
	}
	destroyset(set);
}
/*短路条件*/
void short_condition(symset fsys, list *trueList, list *falseList)
{
	int relop;
	symset set;
	if (sym == SYM_ODD)
	{
		getsym();
		expression_ask(fsys);
		gen(OPR, 0, 6);  //OPR_ODD
	}
	else
	{
		set = uniteset(relset, fsys);
		expression_ask(set);
		destroyset(set);
		if (!inset(sym, relset))
		{
			list *tail = trueList->tail;
			list *pt = (list *)malloc(sizeof(list));
			tail->next = pt;
			pt->cx = cx;
			pt->next = NULL;
			trueList->tail = pt;

			tail = falseList->tail;
			list *pf = (list *)malloc(sizeof(list));
			tail->next = pf;
			pf->cx;
			pf->next = NULL;
			falseList->tail = pf;

			gen(JNZ, 0, 0);
		}
		else
		{
			relop = sym;
			getsym();
			expression_ask(fsys);
			list *tail = trueList->tail;
			list *pt = (list *)malloc(sizeof(list));
			tail->next = pt;
			pt->cx = cx;
			pt->next = NULL;
			trueList->tail = pt;

			tail = falseList->tail;
			list *pf = (list *)malloc(sizeof(list));
			tail->next = pf;
			pf->cx = cx;
			pf->next = NULL;
			falseList->tail = pf;

			switch (relop)
			{
			case SYM_EQU:
				gen(JE, 0, 0);
				break;
			case SYM_NEQ:
				gen(JNE, 0, 0);
				break;
			case SYM_LES:
				gen(JL, 0, 0);
				break;
			case SYM_LEQ:
				gen(JLE, 0, 0);
				break;
			case SYM_GEQ:
				gen(JGE, 0, 0);
				break;
			case SYM_GTR:
				gen(JG, 0, 0);
				break;
			} // switch
		} // else
	} // else

} // condition

void short_condition_and(symset fsys, list *trueList, list *falseList)
{
	list *trueList_1 = (list *)malloc(sizeof(list));
	list *p;
	trueList_1->next = NULL;
	trueList_1->tail = trueList_1;
	symset set;
	set = uniteset(fsys, createset(SYM_AND, SYM_NULL));
	short_condition(set, trueList_1, falseList);
	
	while (sym == SYM_AND)
	{
		getsym();
		p = trueList_1->next;
		while (1)
		{
			if (p == NULL)break;
			code[p->cx].a = cx;
			p = p->next;
		}
		trueList_1->next = NULL;
		trueList_1->tail = trueList_1;
		short_condition(set, trueList_1, falseList);
	}
	p = trueList_1->next;
	list *tail = trueList->tail;
	if (p != NULL)
	{
		tail->next = p;
		while (p->next != NULL)p = p->next;
		trueList->tail = p;
	}
	tail = trueList->tail;
	tail->next = NULL;
	destroyset(set);
}

void short_condition_or(symset fsys, list *trueList, list *falseList)
{
	list *falseList_1 = (list *)malloc(sizeof(list));
	list *p;
	falseList_1->next = NULL;
	falseList_1->tail = falseList_1;
	symset set;
	set = uniteset(createset(SYM_OR, SYM_NULL), fsys);
	short_condition_and(set, trueList, falseList_1);
	while (sym == SYM_OR)
	{
		getsym();
		p = falseList_1->next;
		while (1)
		{
			if (p == NULL)break;
			code[p->cx].l = cx;
			p = p->next;
		}
		falseList_1->next = NULL;
		falseList_1->tail = falseList_1;

		short_condition_and(set, trueList, falseList_1);
	}
	p = falseList_1->next;
	list *tail = falseList->tail;
	if (p != NULL)
	{
		tail->next = p;
		while (p->next != NULL)p = p->next;
		falseList->tail = p;
	}
	tail = falseList->tail;
	tail->next = NULL;
	destroyset(set);
}
  //////////////////////////////////////////////////////////////////////
void SwitchBody(symset fsys, int lastcx)
{
	if (sym != SYM_CASE && sym != SYM_DEFAULT)
	{
		printf("expected case or default in SwitchBody\n");
		exit(1);
	}
	if (sym == SYM_CASE)
	{

		getsym();
		if (sym != SYM_NUMBER)
		{
			printf("expected number after case\n");
			exit(1);
		}
		if (lastcx != 0)code[lastcx].a = cx;
		gen(LIT, 0, num);
		int cxTemp = cx;
		gen(JNE, cx + 1, 0);
		getsym();
		if (sym != SYM_COLON)
		{
			printf("expected : in case\n");
			exit(1);
		}
		getsym();
		statement(fsys);
		while (inset(sym, statbegsys))
		{
			statement(fsys);
		}
		SwitchBody(fsys, cxTemp);
	}
	else // default
	{
		getsym();
		if (sym != SYM_COLON)
		{
			printf("expected : in case\n");
			exit(1);
		}
		getsym();
		if (lastcx != 0)code[lastcx].a = cx;
		statement(fsys);
		while (inset(sym, statbegsys))
		{
			statement(fsys);
		}
	}
}
  //////////////////////////////////////////////////////////////////////
void statement(symset fsys)
{
	void statement(symset fsys);
	int i, j=0, cx1, cx2, cx3[100];
	symset set1, set;
	int retOffset;
	if (sym == SYM_CALLSTACK)
	{
		gen(CALLSTACK, 0, 0);
		getsym();
		if (sym != SYM_LPAREN)
		{
			error(28);
		}
		getsym();
		if (sym != SYM_RPAREN)
		{
			error(27);
		}
		getsym();
		if (sym == SYM_SEMICOLON)
		{
			getsym();
		}
		else {
			error(10);//"';' expected.",
		}
	}
	else if (sym == SYM_RETURN)
	{
		mask *mk;
		int j = position(id);
		if (j)
		{

			mk = (mask *)&table[j];
			retOffset = mk->address;
		}
		getsym();
		expression_ask(fsys);
		gen(STO, 0, -1);
		gen(RET, 0, retOffset); //2017.10.30
		gen(BAC, 0, 1);
		if (sym == SYM_SEMICOLON)
		{
			getsym();
		}
		else {
			error(10);//"';' expected.",
		}
	}
	else if (sym == SYM_IDENTIFIER)
	{ // variable assignment
		mask* mk;
		i = position(id);
		int h_s_temp = heap_or_stack;
		if (!i)
		{
			//printf("test2\n");
			getsym();
			if (sym == SYM_COLON) //GOTO
			{

				gotodeclaration();
				getsym(); //
				statement(fsys);
			}
			else error(11); // Undeclared identifier.
		}
		else if (table[i].kind == ID_PROCEDURE)
		{
			
			mk = (mask*)&table[i];
			getsym();
			set = uniteset(fsys, createset(SYM_LPAREN, SYM_RPAREN, SYM_SEMICOLON, SYM_NULL));
			test(createset(SYM_LPAREN, SYM_NULL), set, 28);//There must be an '(' after procedure"
			if (sym == SYM_LPAREN)
			{
				getsym();
				sc_now = 0;
				procedureCall(set);
				if (sc_now != mk->arrayAdd)
				{
					printf("形参实参个数不同\n");
					err++;
				}
				gen(CAL, level, mk->address); // 2017.10.30 level - mk->level change to level
			}
			else
			{
				printf("expect ( here \n");
			}
			
			destroyset(set);
			test(createset(SYM_SEMICOLON, SYM_NULL), fsys, 10);//"';' expected."
			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else if (sym != SYM_RPAREN)
			{
				error(22);
			}
		}
		else if (table[i].kind == ID_ARRAY)
		{
			getsym();
			gen(LIT, 0, 0);
			readDim = 0;
			calAdd(i);
			mk = (mask*)&table[i];
			if (sym == SYM_BECOMES)
			{
				getsym();
				expression_ask(uniteset(createset(SYM_RIGHTSPAREN,SYM_NULL),fsys));
				if (h_s_temp == STACK)
					gen(STOARR, level - mk->level, mk->address);
				else gen(STOARR_HEAP, 0, mk->address);
			}
			else if (sym == SYM_INC)
			{
				gen(COPY, 0, 1);
				if (h_s_temp == STACK)
					gen(LODARR, level - mk->level, mk->address);
				else gen(LODARR_HEAP, 0, mk->address);
				gen(LIT, 0, 1);
				gen(OPR, 0, OPR_ADD);
				if (h_s_temp == STACK)
				{
					gen(STOARR, level - mk->level, mk->address);
				}
				else
				{
					gen(STOARR_HEAP, 0, mk->address);
				}
				getsym();
			}
			else if (sym == SYM_DEC)
			{
				gen(COPY, 0, 1);
				if (h_s_temp == STACK)
					gen(LODARR, level - mk->level, mk->address);
				else gen(LODARR_HEAP, 0, mk->address);
				gen(LIT, 0, 1);
				gen(OPR, 0, OPR_MIN);
				if (h_s_temp == STACK)
				{
					gen(STOARR, level - mk->level, mk->address);
				}
				else
				{
					gen(STOARR_HEAP, 0, mk->address);
				}
				getsym();
			}
			else if (sym == SYM_PLUSB)
			{
				getsym();
				gen(COPY, 0, 1);
				if (h_s_temp == STACK)
					gen(LODARR, level - mk->level, mk->address);
				else gen(LODARR_HEAP, 0, mk->address);
				expression_ask(fsys);
				gen(OPR, 0, OPR_ADD);
				if (h_s_temp == STACK)
				{
					gen(STOARR, level - mk->level, mk->address);
				}
				else
				{
					gen(STOARR_HEAP, 0, mk->address);
				}
			}
			else if (sym == SYM_MINUSB)
			{
				getsym();
				gen(COPY, 0, 1);
				if (h_s_temp == STACK)
					gen(LODARR, level - mk->level, mk->address);
				else gen(LODARR_HEAP, 0, mk->address);
				expression_ask(fsys);
				gen(OPR, 0, OPR_MIN);
				if (h_s_temp == STACK)
				{
					gen(STOARR, level - mk->level, mk->address);
				}
				else
				{
					gen(STOARR_HEAP, 0, mk->address);
				}
			}
			else if (sym == SYM_TIMESB)
			{
				getsym();
				gen(COPY, 0, 1);
				if (h_s_temp == STACK)
					gen(LODARR, level - mk->level, mk->address);
				else gen(LODARR_HEAP, 0, mk->address);
				expression_ask(fsys);
				gen(OPR, 0, OPR_MUL);
				if (h_s_temp == STACK)
				{
					gen(STOARR, level - mk->level, mk->address);
				}
				else
				{
					gen(STOARR_HEAP, 0, mk->address);
				}
			}
			else if (sym == SYM_SLASHB)
			{
				getsym();
				gen(COPY, 0, 1);
				if (h_s_temp == STACK)
					gen(LODARR, level - mk->level, mk->address);
				else gen(LODARR_HEAP, 0, mk->address);
				expression_ask(fsys);
				gen(OPR, 0, OPR_DIV);
				if (h_s_temp == STACK)
				{
					gen(STOARR, level - mk->level, mk->address);
				}
				else
				{
					gen(STOARR_HEAP, 0, mk->address);
				}
			}
			else if (sym == SYM_MODB)
			{
				getsym();
				gen(COPY, 0, 1);
				if (h_s_temp == STACK)
					gen(LODARR, level - mk->level, mk->address);
				else gen(LODARR_HEAP, 0, mk->address);
				expression_ask(fsys);
				gen(OPR, 0, OPR_MOD);
				if (h_s_temp == STACK)
				{
					gen(STOARR, level - mk->level, mk->address);
				}
				else
				{
					gen(STOARR_HEAP, 0, mk->address);
				}
			}
			else if (sym == SYM_ANDBITB)
			{
				getsym();
				gen(COPY, 0, 1);
				if (h_s_temp == STACK)
					gen(LODARR, level - mk->level, mk->address);
				else gen(LODARR_HEAP, 0, mk->address);
				expression_ask(fsys);
				gen(OPR, 0, OPR_ANDBIT);
				if (h_s_temp == STACK)
				{
					gen(STOARR, level - mk->level, mk->address);
				}
				else
				{
					gen(STOARR_HEAP, 0, mk->address);
				}
			}
			else if (sym == SYM_SHLB)
			{
				getsym();
				gen(COPY, 0, 1);
				if (h_s_temp == STACK)
					gen(LODARR, level - mk->level, mk->address);
				else gen(LODARR_HEAP, 0, mk->address);
				expression_ask(fsys);
				gen(OPR, 0, OPR_SHL);
				if (h_s_temp == STACK)
				{
					gen(STOARR, level - mk->level, mk->address);
				}
				else
				{
					gen(STOARR_HEAP, 0, mk->address);
				}
			}
			else if (sym == SYM_SHRB)
			{
				getsym();
				gen(COPY, 0, 1);
				if (h_s_temp == STACK)
					gen(LODARR, level - mk->level, mk->address);
				else gen(LODARR_HEAP, 0, mk->address);
				expression_ask(fsys);
				gen(OPR, 0, OPR_SHR);
				if (h_s_temp == STACK)
				{
					gen(STOARR, level - mk->level, mk->address);
				}
				else
				{
					gen(STOARR_HEAP, 0, mk->address);
				}
			}
			else if (sym == SYM_ORBITB)
			{
				getsym();
				gen(COPY, 0, 1);
				if (h_s_temp == STACK)
					gen(LODARR, level - mk->level, mk->address);
				else gen(LODARR_HEAP, 0, mk->address);
				expression_ask(fsys);
				gen(OPR, 0, OPR_ORBIT);
				if (h_s_temp == STACK)
				{
					gen(STOARR, level - mk->level, mk->address);
				}
				else
				{
					gen(STOARR_HEAP, 0, mk->address);
				}
			}
			else if (sym == SYM_XORB)
			{
				getsym();
				gen(COPY, 0, 1);
				if (h_s_temp == STACK)
					gen(LODARR, level - mk->level, mk->address);
				else gen(LODARR_HEAP, 0, mk->address);
				expression_ask(fsys);
				gen(OPR, 0, OPR_XOR);
				if (h_s_temp == STACK)
				{
					gen(STOARR, level - mk->level, mk->address);
				}
				else
				{
					gen(STOARR_HEAP, 0, mk->address);
				}
			}
			else if (sym == SYM_NOTB)
			{
				getsym();
				gen(COPY, 0, 1);
				if (h_s_temp == STACK)
					gen(LODARR, level - mk->level, mk->address);
				else gen(LODARR_HEAP, 0, mk->address);
				expression_ask(fsys);
				gen(OPR, 0, OPR_NOTBIT);
				if (h_s_temp == STACK)
				{
					gen(STOARR, level - mk->level, mk->address);
				}
				else
				{
					gen(STOARR_HEAP, 0, mk->address);
				}
			}
			else
			{
				error(13); // ':=' expected.
			}
			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else if (sym != SYM_RPAREN)
			{
				printf("here\n");
				error(22);
			}
		}
		else if (table[i].kind != ID_VARIABLE)
		{
			error(12); // Illegal assignment.
			i = 0;
		}
		else {		//table[i].kind == ID_VARIABLE
			getsym();
			mk = (mask*)&table[i];
			if (sym == SYM_BECOMES)
			{
				getsym();
				expression_ask(fsys);
				/*层次差+偏移量寻址*/
				if (i)
				{
					if (h_s_temp == STACK)
						gen(STO, level - mk->level, mk->address);
					else gen(STO_HEAP, 0, mk->address);
				}
			}
			else if (sym==SYM_INC)
			{
				if (h_s_temp == STACK) 
				{
					gen(LOD, level - mk->level, mk->address);
				}
				else
				{
					gen(LOD_HEAP, 0, mk->address);
				}
				gen(LIT, 0, 1);
				gen(OPR, 0, OPR_ADD);
				if (h_s_temp == STACK)
					gen(STO, level - mk->level, mk->address);
				else gen(STO_HEAP, 0, mk->address);
				getsym();
			}
			else if (sym == SYM_DEC)
			{
				if (h_s_temp == STACK)
				{
					gen(LOD, level - mk->level, mk->address);
				}
				else
				{
					gen(LOD_HEAP, 0, mk->address);
				}
				gen(LIT, 0, 1);
				gen(OPR, 0, OPR_MIN);
				if (h_s_temp == STACK)
					gen(STO, level - mk->level, mk->address);
				else gen(STO_HEAP, 0, mk->address);
				getsym();
			}
			else if (sym == SYM_PLUSB)
			{
				getsym();
				if (i)
				{
					if (h_s_temp == STACK)
						gen(LOD, level - mk->level, mk->address);
					else
						gen(LOD_HEAP, 0, mk->address);
				}
				expression_ask(fsys);
				gen(OPR, 0, OPR_ADD);
				/*层次差+偏移量寻址*/
				if (i)
				{
					if (h_s_temp == STACK)
						gen(STO, level - mk->level, mk->address);
					else
						gen(STO_HEAP, 0, mk->address);
				}
			}
			else if (sym == SYM_MINUSB)
			{
				getsym();
				if (i)
				{
					if (h_s_temp == STACK)
						gen(LOD, level - mk->level, mk->address);
					else
						gen(LOD_HEAP, 0, mk->address);
				}
				expression_ask(fsys);
				gen(OPR, 0, OPR_MIN);
				/*层次差+偏移量寻址*/
				if (i)
				{
					if (h_s_temp == STACK)
						gen(STO, level - mk->level, mk->address);
					else
						gen(STO_HEAP, 0, mk->address);
				}
			}
			else if (sym == SYM_ORBITB)
			{
				getsym();
				if (i)
				{
					if (h_s_temp == STACK)
						gen(LOD, level - mk->level, mk->address);
					else
						gen(LOD_HEAP, 0, mk->address);
				}
				expression_ask(fsys);
				gen(OPR, 0, OPR_ORBIT);
				/*层次差+偏移量寻址*/
				if (i)
				{
					if (h_s_temp == STACK)
						gen(STO, level - mk->level, mk->address);
					else
						gen(STO_HEAP, 0, mk->address);
				}
			}
			else if (sym == SYM_TIMESB)
			{
				getsym();
				if (i)
				{
					if (h_s_temp == STACK)
						gen(LOD, level - mk->level, mk->address);
					else
						gen(LOD_HEAP, 0, mk->address);
				}
				expression_ask(fsys);
				gen(OPR, 0, OPR_MUL);
				/*层次差+偏移量寻址*/
				if (i)
				{
					if (h_s_temp == STACK)
						gen(STO, level - mk->level, mk->address);
					else
						gen(STO_HEAP, 0, mk->address);
				}
			}
			else if (sym == SYM_SLASHB)
			{
				getsym();
				if (i)
				{
					if (h_s_temp == STACK)
						gen(LOD, level - mk->level, mk->address);
					else
						gen(LOD_HEAP, 0, mk->address);
				}
				expression_ask(fsys);
				gen(OPR, 0, OPR_DIV);
				/*层次差+偏移量寻址*/
				if (i)
				{
					if (h_s_temp == STACK)
						gen(STO, level - mk->level, mk->address);
					else
						gen(STO_HEAP, 0, mk->address);
				}
			}
			else if (sym == SYM_MODB)
			{
				getsym();
				if (i)
				{
					if (h_s_temp == STACK)
						gen(LOD, level - mk->level, mk->address);
					else
						gen(LOD_HEAP, 0, mk->address);
				}
				expression_ask(fsys);
				gen(OPR, 0, OPR_MOD);
				/*层次差+偏移量寻址*/
				if (i)
				{
					if (h_s_temp == STACK)
						gen(STO, level - mk->level, mk->address);
					else
						gen(STO_HEAP, 0, mk->address);
				}
			}
			else if (sym == SYM_XORB)
			{
				getsym();
				if (i)
				{
					if (h_s_temp == STACK)
						gen(LOD, level - mk->level, mk->address);
					else
						gen(LOD_HEAP, 0, mk->address);
				}
				expression_ask(fsys);
				gen(OPR, 0, OPR_XOR);
				/*层次差+偏移量寻址*/
				if (i)
				{
					if (h_s_temp == STACK)
						gen(STO, level - mk->level, mk->address);
					else
						gen(STO_HEAP, 0, mk->address);
				}
			}
			else if (sym == SYM_ANDBITB)
			{
				getsym();
				if (i)
				{
					if (h_s_temp == STACK)
						gen(LOD, level - mk->level, mk->address);
					else
						gen(LOD_HEAP, 0, mk->address);
				}
				expression_ask(fsys);
				gen(OPR, 0, OPR_AND);
				/*层次差+偏移量寻址*/
				if (i)
				{
					if (h_s_temp == STACK)
						gen(STO, level - mk->level, mk->address);
					else
						gen(STO_HEAP, 0, mk->address);
				}
			}
			else if (sym == SYM_SHLB)
			{
				getsym();
				if (i)
				{
					if (h_s_temp == STACK)
						gen(LOD, level - mk->level, mk->address);
					else
						gen(LOD_HEAP, 0, mk->address);
				}
				expression_ask(fsys);
				gen(OPR, 0, OPR_SHL);
				/*层次差+偏移量寻址*/
				if (i)
				{
					if (h_s_temp == STACK)
						gen(STO, level - mk->level, mk->address);
					else
						gen(STO_HEAP, 0, mk->address);
				}
			}
			else if (sym == SYM_SHRB)
			{
				getsym();
				if (i)
				{
					if (h_s_temp == STACK)
						gen(LOD, level - mk->level, mk->address);
					else
						gen(LOD_HEAP, 0, mk->address);
				}
				expression_ask(fsys);
				gen(OPR, 0, OPR_SHR);
				/*层次差+偏移量寻址*/
				if (i)
				{
					if (h_s_temp == STACK)
						gen(STO, level - mk->level, mk->address);
					else
						gen(STO_HEAP, 0, mk->address);
				}
			}
			else if (sym == SYM_NOTB)
			{
				getsym();
				if (i)
				{
					if (h_s_temp == STACK)
						gen(LOD, level - mk->level, mk->address);
					else
						gen(LOD_HEAP, 0, mk->address);
				}
				expression_ask(fsys);
				gen(OPR, 0, OPR_NOT);
				/*层次差+偏移量寻址*/
				if (i)
				{
					if (h_s_temp == STACK)
						gen(STO, level - mk->level, mk->address);
					else
						gen(STO_HEAP, 0, mk->address);
				}
			}
			else
			{
				printf("sym is %d presym is %d\n", sym, presym);
				error(13); // "':=','++'or'--' expected.",
			}
			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else if (sym != SYM_RPAREN)
			{
				error(22);
			}
		}
		
	}
	else if (sym == SYM_SWITCH)
	{
		breakLevel++;
		breakCx[breakLevel] = (int *)malloc(50 * sizeof(int));
		breakCx[breakLevel][0] = 0;
		getsym();
		if (sym != SYM_LPAREN)
		{
			printf("expected ( in switch while sym is %d\n", sym);
			exit(1);
		}
		getsym();
		if (sym == SYM_IDENTIFIER)
		{
			int i = position(id);
			if (i)
			{
				mask *mk = (mask *)&table[i];
				if (heap_or_stack == STACK)
					gen(LOD, level - mk->level, mk->address);
				else gen(LOD_HEAP, 0, mk->address);
			}
			else
			{
				printf("the id undeclared!\n");
			}
		}
		else if (sym == SYM_NUMBER)
		{
			gen(LIT, 0, num);
		}
		getsym();
		if (sym != SYM_RPAREN)
		{
			printf("expected ) in switch while sym is %d\n", sym);
			printf("Only a number or identifier is permitted\n");
			exit(1);
		}
		getsym();
		if (sym != SYM_BEGIN)
		{
			printf("expected begin in switch while sym is %d\n", sym);
			exit(1);
		}
		getsym();
		SwitchBody(fsys, 0);
		for (i = breakCx[breakLevel][0]; i>0; i--)
		{
			printf("i is %d\n", breakCx[breakLevel][i]);
			code[breakCx[breakLevel][i]].a = cx;
		}
		free(breakCx[breakLevel]);
		breakLevel--;

		if (sym != SYM_END)
		{
			printf("expected end in switch \n ");
			exit(1);
		}
		getsym();
		if (sym != SYM_SEMICOLON)
		{
			printf("expected ; in switch end \n");
			exit(1);
		}
		getsym();
	}
	else if (sym == SYM_INC)
	{
		getsym();
		if (sym != SYM_IDENTIFIER)
		{
			printf("expected id here \n");
			err++;
			getsym();
		}
		else
		{
			int i = position(id);
			mask *mk = (mask *)&table[i];
			if (table[i].kind == ID_VARIABLE)
			{
				if (heap_or_stack == STACK)
					gen(LOD, level - mk->level, mk->address);//将变量值置于栈顶
				else gen(LOD_HEAP, 0, mk->address);
				gen(LIT, 0, 1);//常数置于栈顶
				gen(OPR, 0, OPR_ADD);//相加
				if (heap_or_stack == STACK)
				{
					gen(STO, level - mk->level, mk->address);//将栈顶的值赋予变量
				}
				else
				{
					gen(STO_HEAP, 0, mk->address);//将栈顶的值赋予变量
				}
				getsym();
			}
			else if (table[i].kind == ID_ARRAY)
			{
				getsym();
				gen(LIT, 0, 0);
				readDim = 0;
				calAdd(i);
				gen(COPY, 0, 1);
				if (heap_or_stack == STACK)
					gen(LODARR, level - mk->level, mk->address);
				else gen(LODARR_HEAP, 0, mk->address);
				gen(LIT, 0, 1);
				gen(OPR, 0, OPR_ADD);
				if (heap_or_stack == STACK)
				{
					gen(STOARR, level - mk->level, mk->address);
				}
				else
				{
					gen(STOARR, level - mk->level, mk->address);
				}
			}
		}
		if (sym == SYM_SEMICOLON)
		{
			getsym();
		}
		else if (sym != SYM_RPAREN)
		{
			error(22);
		}
	}
	else if (sym == SYM_DEC)
	{
		getsym();
		if (sym != SYM_IDENTIFIER)
		{
			printf("expected id here \n");
			err++;
			getsym();
		}
		else
		{
			int i = position(id);
			mask *mk = (mask *)&table[i];
			if (table[i].kind == ID_VARIABLE)
			{
				if (heap_or_stack == STACK)
					gen(LOD, level - mk->level, mk->address);//将变量值置于栈顶
				else gen(LOD_HEAP, 0, mk->address);
				gen(LIT, 0, 1);//常数置于栈顶
				gen(OPR, 0, OPR_MIN);//相加
				if (heap_or_stack == STACK)
				{
					gen(STO, level - mk->level, mk->address);//将栈顶的值赋予变量
				}
				else
				{
					gen(STO_HEAP, 0, mk->address);//将栈顶的值赋予变量
				}
				getsym();
			}
			else if (table[i].kind == ID_ARRAY)
			{
				getsym();
				gen(LIT, 0, 0);
				readDim = 0;
				calAdd(i);
				gen(COPY, 0, 1);
				if (heap_or_stack == STACK)
					gen(LODARR, level - mk->level, mk->address);
				else gen(LODARR_HEAP, 0, mk->address);
				gen(LIT, 0, 1);
				gen(OPR, 0, OPR_MIN);
				if (heap_or_stack == STACK)
				{
					gen(STOARR, level - mk->level, mk->address);
				}
				else
				{
					gen(STOARR, level - mk->level, mk->address);
				}
			}
		}
		if (sym == SYM_SEMICOLON)
		{
			getsym();
		}
		else if (sym != SYM_RPAREN)
		{
			error(22);
		}
	}
	else if (sym == SYM_TIMES)
	{
		getsym();
		TimesBody(fsys);
		if (sym == SYM_BECOMES)
		{
			getsym();
			expression_ask(fsys);
			gen(STOA, 0, 0);
			if (sym != SYM_SEMICOLON)
			{
				printf("expected ; in the end of SYM_TIMES \n");
				exit(1);
			}
			else getsym();
		}
	}
/*	else if (sym == SYM_CALL)
	{ // procedure call
		getsym();
		if (sym != SYM_IDENTIFIER)
		{
			error(14); // There must be an identifier to follow the 'call'.
		}
		else
		{
			if (!(i = position(id)))
			{
				error(11); // Undeclared identifier.
			}
			
			else if (table[i].kind == ID_PROCEDURE)
			{
				mask* mk;
				mk = (mask*)&table[i];
				gen(CAL, level - mk->level, mk->address);
			}
			else
			{
				error(15); // A constant or variable can not be called. 
			}
			getsym();
		}
	}
*/
	else if (sym == SYM_EXIT)
	{
		getsym();
		if (sym != SYM_LPAREN)
		{
			printf("expected leftsparen after exit \n");
			err++;
		}
		getsym();
		if (sym != SYM_RPAREN)
		{
			printf("expected rightsparen after exit \n");
			err++;
		}
		getsym();
		gen(JMP, 0, ENDCX);
		if (sym == SYM_SEMICOLON)
		{
			getsym();
		}
		else {
			error(10);//"';' expected.",
		}
	}
	else if (sym == SYM_FOR)
	{
		int j;
		loopLevel++;
		breakLevel++;
		breakCx[breakLevel] = (int *)malloc(50 * sizeof(int));
		breakCx[breakLevel][0] = 0;
		instruction codeTemp[100];
		int cxTemp, tempCodeCount;
		int CFalseAdd, ENext;
		getsym();
		if (sym != SYM_LPAREN)
		{
			printf("expected '(' after for declaration\n");
			err++;
		}
		else
		{
			getsym();
			if (sym != SYM_IDENTIFIER)
			{
				printf("expected identifier in the first field of for declaration\n");
				err++;
			}
			else  // E1
			{
				int i = position(id);
				mask *mk = (mask *)&table[i];
				if (i)
				{
					if (table[i].kind == ID_VARIABLE)
					{
						getsym();
						if (sym == SYM_BECOMES)
						{
							getsym();
							expression_ask(fsys);
							if(heap_or_stack==STACK)
							gen(STO, level - mk->level, mk->address);
							else gen(STO_HEAP, 0, mk->address);
						}
					}
					else // table[i].kind == ID_ARRAY
					{
						getsym();
						gen(LIT, 0, 0);
						readDim = 0;
						calAdd(i);
						if (sym == SYM_BECOMES)
						{
							getsym();
							expression_ask(fsys);
							if (heap_or_stack == STACK)
								gen(STOARR, level - mk->level, mk->address);
							else gen(STOARR_HEAP, 0, mk->address);
						}
					}
				}
				else
				{
					printf("Identifier undeclared!\n");
					err++;
				}
			}
			if (sym != SYM_SEMICOLON)
			{
				printf("expected ';' after the first field in for statement\n");
				err++;
			}
			else  // Condition
			{
				loopCx[loopLevel] = cx;
				ENext = cx;
				getsym();
				condition(fsys);
				CFalseAdd = cx;
				gen(JZ, 0, 0);
			}
			if (sym != SYM_SEMICOLON)
			{
				printf("expected ';' after the second field in for statement\n");
				err++;
			}
			else  // E2
			{
				cxTemp = cx;
				getsym();
				if (sym != SYM_IDENTIFIER && sym != SYM_DEC && sym != SYM_INC)
				{
					printf("expected identifier in the first field of for declaration\n");
					err++;
				}
				if (sym == SYM_INC)
				{
					getsym();
					if (sym != SYM_IDENTIFIER)
					{
						printf("expected id here \n");
						err++;
						getsym();
					}
					else
					{
						int i = position(id);
						mask *mk = (mask *)&table[i];
						if (heap_or_stack == STACK)
						{
							gen(LOD, level - mk->level, mk->address);
							gen(LIT, 0, 1);
							gen(OPR, 0, OPR_ADD);
							gen(STO, level - mk->level, mk->address);
						}
						else
						{
							gen(LOD_HEAP, 0, mk->address);
							gen(LIT, 0, 1);
							gen(OPR, 0, OPR_ADD);
							gen(STO_HEAP, 0, mk->address);
						}getsym();
					}
				}
				else if (sym == SYM_DEC)
				{
					getsym();
					if (sym != SYM_IDENTIFIER)
					{
						printf("expected id here \n");
						err++;
						getsym();
					}
					else
					{
						int i = position(id);
						mask *mk = (mask *)&table[i];
						if (heap_or_stack == STACK)
						{
							gen(LOD, level - mk->level, mk->address);
							gen(LIT, 0, 1);
							gen(OPR, 0, OPR_MIN);
							gen(STO, level - mk->level, mk->address);
						}
						else
						{
							gen(LOD_HEAP, 0, mk->address);
							gen(LIT, 0, 1);
							gen(OPR, 0, OPR_MIN);
							gen(STO_HEAP, 0, mk->address);
						}
						getsym();
					}
				}
				else
				{
					int i = position(id);
					mask *mk = (mask *)&table[i];
					if (i)
					{
						if (table[i].kind == ID_VARIABLE)
						{
							getsym();
							if (sym == SYM_BECOMES)
							{
								getsym();
								expression_ask(fsys);
								if (heap_or_stack == STACK)
									gen(STO, level - mk->level, mk->address);
								else gen(STO_HEAP, 0, mk->address);
							}
							if (sym == SYM_INC)
							{
								if(heap_or_stack==STACK)
								gen(LOD, level - mk->level, mk->address);
								else gen(LOD_HEAP, 0, mk->address);
								gen(LIT, 0, 1);
								gen(OPR, 0, OPR_ADD);
								if(heap_or_stack==STACK)
								gen(STO, level - mk->level, mk->address);
								else gen(STO_HEAP, 0, mk->address);
								getsym();
							}
							else if (sym == SYM_DEC)
							{
								if (heap_or_stack == STACK)
									gen(LOD, level - mk->level, mk->address);
								else gen(LOD_HEAP, 0, mk->address); gen(LIT, 0, 1);
								gen(OPR, 0, OPR_MIN);
								if (heap_or_stack == STACK)
									gen(STO, level - mk->level, mk->address);
								else gen(STO_HEAP, 0, mk->address); getsym();
							}
						}
						else // table[i].kind == ID_ARRAY
						{
							getsym();
							gen(LIT, 0, 0);
							readDim = 0;
							calAdd(i);
							mk = (mask*)&table[i];
							if (sym == SYM_BECOMES)
							{
								getsym();
								expression_ask(fsys);
								if(heap_or_stack==STACK)
								gen(STOARR, level - mk->level, mk->address);
								else gen(STOARR_HEAP, 0, mk->address);
							}
						}
					}
					else
					{
						printf("Identifier undeclared!\n");
						err++;
					}
				}
				tempCodeCount = cx - cxTemp;
				for (j = 0; j<tempCodeCount; j++)
				{
					codeTemp[j].f = code[cxTemp + j].f;
					codeTemp[j].l = code[cxTemp + j].l;
					codeTemp[j].a = code[cxTemp + j].a;
				}
				cx = cxTemp;
			}
			if (sym != SYM_RPAREN)
			{
				printf("expected SYM_RPAREN\n");
				err++;
			}
			else // body
			{
				getsym();
				statement(fsys);
				for (j = 0; j<tempCodeCount; j++)
				{
					code[cx].f = codeTemp[j].f;
					code[cx].l = codeTemp[j].l;
					code[cx++].a = codeTemp[j].a;
				}
				gen(JMP, 0, ENext);
				code[CFalseAdd].a = cx;
				for (j = breakCx[breakLevel][0]; j>0; j--)
				{
					code[j].a = cx;
				}
				free(breakCx[breakLevel]);
				breakLevel--;
				loopLevel--;
			}
		}
	}
	else if (sym == SYM_CONTINUE)
	{
		if (loopLevel == 0)
		{
			printf("Not in a loop !\n");
			exit(1);
		}
		gen(JMP, 0, loopCx[loopLevel]);
		getsym();
		if (sym != SYM_SEMICOLON)
		{
			printf("expect ; in continue while sym is %d\n", sym);
			exit(1);
		}
		else getsym();
	}
	else if (sym == SYM_BREAK)
	{
		if (breakLevel == 0)
		{
			printf("Not in a Break!\n");
			exit(1);
		}
		breakCx[breakLevel][0]++;
		int count = breakCx[breakLevel][0];
		breakCx[breakLevel][count] = cx;
		gen(JMP, 0, 0);
		getsym();
		if (sym != SYM_SEMICOLON)
		{
			printf("expected ; in break while sym is %d\n", sym);
			exit(1);
		}
		getsym();
	}
	/*
	else if (sym == SYM_IF)
	{ // if statement
		getsym();
		set1 = createset(SYM_THEN, SYM_DO, SYM_RPAREN, SYM_NULL);
		set = uniteset(set1, fsys);
		int cxTemp;
		list *trueList_1 = (list *)malloc(sizeof(list));
		trueList_1->next = NULL;
		trueList_1->tail = trueList_1;
		list *falseList_1 = (list *)malloc(sizeof(list));
		falseList_1->next = NULL;
		falseList_1->tail = falseList_1;
		if (sym != SYM_LPAREN)
		{
			printf("expect ( after if \n");
			err++;
		}
		else
		{
			getsym();
			short_condition_or(set, trueList_1, falseList_1);

			if (sym != SYM_RPAREN)
			{
				printf("expect ) in if expression while the sym is %d \n", sym);
				err++;
			}
			else getsym();

		}
		destroyset(set1);
		destroyset(set);

		list *p = trueList_1->next;
		while (p != NULL)
		{
			code[p->cx].a = cx;
			p = p->next;
		}//为真跳转到这里
		statement(uniteset(createset(SYM_ELSE,SYM_NULL),fsys));
		cxTemp = cx;
		gen(JMP, 0, 0);
		//为假跳转到这里
		if (sym == SYM_ELSE)
		{
			getsym();
			list *p = falseList_1->next;
			while (p != NULL)
			{
				code[p->cx].l = cx;
				p = p->next;
			}
			statement(fsys);
			code[cxTemp].a = cx;
		}
		else
		{
			list *p = falseList_1->next;
			while (p != NULL)
			{
				code[p->cx].l = cx;
				p = p->next;
			}
			code[cxTemp].a = cx;
		}
	}
	*/
	else if (sym ==  SYM_IF)
	{ // if statement
		j = 0;
		list *trueList_1 = (list *)malloc(sizeof(list));
		trueList_1->next = NULL;
		trueList_1->tail = trueList_1;
		list *falseList_1 = (list *)malloc(sizeof(list));
		falseList_1->next = NULL;
		falseList_1->tail = falseList_1;
		getsym();
		if (sym != SYM_LPAREN)
		{
			error(28);
		}
		getsym();
		short_condition_or(uniteset(createset(SYM_RPAREN, SYM_ELIF, SYM_ELSE, SYM_NULL), fsys),trueList_1,falseList_1);
		if (sym != SYM_RPAREN)
		{
			error(22);
		}
		getsym();
		list *p = trueList_1->next;
		while (p != NULL)
		{
			code[p->cx].a = cx;
			p = p->next;
		}//为真
		statement(uniteset(createset(SYM_RPAREN, SYM_ELIF, SYM_ELSE, SYM_NULL), fsys));
		cx3[j] = cx;//记录每一个的elif和if内的语句结束的地址，如果执行了这些语句，需要跳转到最后
		gen(JMP, 0, 0);
		j++;
		p = falseList_1->next;
		while (p != NULL)
		{
			code[p->cx].l = cx;
			p = p->next;
		}//if为假
		if (sym == SYM_ELIF)
		{
			while (sym == SYM_ELIF)
			{
				getsym();
				if (sym != SYM_LPAREN)
				{
					error(28);
				}
				getsym();
				list *trueList_2 = (list *)malloc(sizeof(list));
				trueList_2->next = NULL;
				trueList_2->tail = trueList_2;
				list *falseList_2 = (list *)malloc(sizeof(list));
				falseList_2->next = NULL;
				falseList_2->tail = falseList_2;
				short_condition_or(uniteset(createset(SYM_RPAREN, SYM_ELIF, SYM_ELSE, SYM_NULL), fsys),trueList_2,falseList_2);
				if (sym != SYM_RPAREN)
				{
					error(22);
				}
				getsym();
				list *p = trueList_2->next;
				while (p != NULL)
				{
					code[p->cx].a = cx;
					p = p->next;
				}//为真
				statement(uniteset(createset(SYM_RPAREN, SYM_ELIF, SYM_ELSE, SYM_NULL), fsys));
				cx3[j] = cx;//记录每一个的elif和if内的语句结束的地址，如果执行了这些语句，需要跳转到最后
				gen(JMP, 0, 0);
				j++;
				p = falseList_2->next;
				while (p != NULL)
				{
					code[p->cx].l = cx;
					p = p->next;
				}//为真
			}
		}
		if (sym == SYM_ELSE)
		{
			getsym();
			statement(fsys);
		}
		for (i = 0; i < j; i++)
		{
			code[cx3[i]].a = cx;//if-elif-else 的末尾
		}
		
	}
	else if (sym == SYM_BEGIN)
	{ // block
		getsym();
		int txTemp = tx1;
		int dxTemp = dx1;
		if (sym == SYM_VAR)  //局部变量
		{ // variable declarations
			initial_flag = 0;
			heap_or_stack = HEAP;
			do
			{
				getsym();
				vardeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					vardeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			} while (sym == SYM_VAR);
		} // if
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);
		while (inset(sym, statbegsys))
		{
			statement(set);
		} // while
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_END)
		{
			tx1 = txTemp;
			dx1 = dxTemp;
			getsym();
		}
		else
		{
			// printf("sym in end is %d",sym);
			error(17); // ';' or 'end' expected.
		}
		if (sym != SYM_SEMICOLON && sym != SYM_PERIOD)
		{
			error(17);
		}
		else if (sym == SYM_SEMICOLON)getsym();
	}
	else if (sym == SYM_WHILE)
	{ // while statement
		int i;
		breakLevel++;
		breakCx[breakLevel] = (int *)malloc(50 * sizeof(int));
		breakCx[breakLevel][0] = 0;
		loopLevel++;
		getsym();
		list *trueList = (list*)malloc(sizeof(list));
		list *falseList = (list *)malloc(sizeof(list));
		trueList->next = NULL;
		trueList->tail = trueList;
		falseList->next = NULL;
		falseList->tail = falseList;
		int cxTemp;

		if (sym != SYM_LPAREN)
		{
			printf("expected ( in while but the sym is %d\n", sym);
			exit(1);
		}
		getsym();

		cxTemp = cx;
		loopCx[loopLevel] = cx;
		short_condition_or(fsys, trueList, falseList);
		if (sym != SYM_RPAREN)
		{
			printf("expected ) in while but the sym is %d\n", sym);
			exit(1);
		}
		getsym();
		list *p = trueList->next;
		while (p != NULL)
		{
			code[p->cx].a = cx;
			p = p->next;
		}

		statement(fsys);
		gen(JMP, 0, cxTemp);

		p = falseList->next;
		while (p != NULL)
		{
			code[p->cx].l = cx;
			p = p->next;
		}
		loopLevel--;
		for (i = breakCx[breakLevel][0]; i>0; i--)
		{
			code[i].a = cx;
		}
		free(breakCx[breakLevel]);
		breakLevel--;
	}
	else if (sym == SYM_PRINT)
	{
		//printf("test");
		getsym();
		getsym();
		if (sym == SYM_RPAREN)
		{
			gen(PRINT, 0, 0);
		}
		else
		{
			while (sym != SYM_RPAREN)
			{
				int i = position(id);
				mask *mk;
				mk = (mask*)&table[i];
				if(heap_or_stack==STACK)
				gen(LOD, level - mk->level, mk->address);
				else gen(LOD_HEAP, 0, mk->address);
				gen(PRINT, i, 0);
				//	printf("%s = %d^^^",mk->name,stack[top]);
				getsym();
				if (sym == SYM_COMMA)
					getsym();
			}
		}
		getsym();
		getsym();
	}
	else if (sym == SYM_GOTO)
	{
		//printf("test\n");
		getsym();
		int i = position_label(id);
		if (i == 0 || label_cx[i][0] == 0)
		{
			//这个标签没有出现过
			l1++;
			strcpy(label[l1], id);
			int j = 0;
			while (go[l1][j++][0] != 0);
			j--;
			gen(JMP, 0, 0);
			go[l1][j][0] = cx - 1;
			go[l1][j][1] = level;
		}
		else
		{
			//这个标签出现过
			int now = label_cx[i][0];
			gen(JMP, 0, now);
		}
		getsym();
		if (sym != SYM_SEMICOLON)
		{
			error(10);
		}
		getsym();
	}
	
	test(uniteset(createset(SYM_RETURN, SYM_NULL),fsys), phi, 19);//"Incorrect symbol."
} // statement

  //////////////////////////////////////////////////////////////////////
void block(symset fsys)
{
	/*cx0记录本层开始时代码段分配位置*/
	int cx0; // initial code index
	mask* mk;
	int block_dx;
	int savedTx;
	symset set1, set;
	instruction codeTemp[1000];
	int cxTemp;
	int tempCodeCount;
	/*置初值3，每一层最开始的位置分别放置静态链，动态链，以及机器状态（IP）*/
	dx = 3;		// data allocation index

	block_dx = dx;
	/*符号表记录下当前层代码的开始位置*/
	mk = (mask*)&table[tx];
	mk->address = cx;
	/*生成跳转指令，等待回填*/
	gen(JMP, 0, 0);
	/*如果当前过程的嵌套层次数大于最大允许的嵌套层次数*/

	if (level > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	/*确保对于level>0的函数一定有（）*/
	if (level > 0) {//不是主函数，判断并进行错误恢复
		set = uniteset(fsys, createset(SYM_RPAREN, SYM_SEMICOLON, SYM_NULL));
		test(createset(SYM_LPAREN, SYM_NULL), set, 28);
		destroyset(set);
	}
	/*如果有括号，进入参数分析部分*/
	if (sym == SYM_LPAREN)
	{
		getsym();
		/*为了错误恢复，引入set*/
		set = uniteset(fsys, createset(SYM_RPAREN, SYM_SEMICOLON, SYM_NULL));
		para_now = tx;
		pcount = -2; 
		paraList(set);
		printf("\n当前函数的参数为：%s %d\n",table[para_now].name, table[para_now].arrayAdd);
		destroyset(set);
		enterPara("return", ID_RETURN);
		if (sym == SYM_RPAREN)
		{
			getsym();
		}
		else
		{
			error(22);//"Missing ')'."
		}
	}
	/*循环处理所有的声明部分*/
	do
	{
		if (sym == SYM_ARRAY)
		{
			getsym();
			if (sym == SYM_IDENTIFIER)//SYM_LEFTSPAREN,SYM_RIGHTSPAREN,
			{
				enterArray();
				getsym();
				if (sym == SYM_LEFTSPAREN)
				{
					getsym();
					arrayDecl();
				}
			}
		}
		if (sym == SYM_CONST)
		{ // constant declarations
			do
			{
				getsym();
				constdeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();    //there may be several const vars behind const
					constdeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			/*******10.9勘误，这里之前写的sym == SYM_IDENTIFIER,明显不对*******/
			while (sym == SYM_CONST);
		} // if
		cxTemp = cx;
		if (sym == SYM_VAR)  //本block初始化的代码全部在这里面生成，把这里面生成的代码拷贝到INT指令后面
		{ // variable declarations
			heap_or_stack = STACK;
			do
			{
				getsym();
				vardeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					vardeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			/*******10.9勘误，这里之前写的sym == SYM_IDENTIFIER,明显不对*******/
			while (sym == SYM_VAR);
		} // if
		tempCodeCount = cx - cxTemp;
		int j;
		for (j = 0; j<tempCodeCount; j++)
		{
			codeTemp[j].f = code[cxTemp + j].f;
			codeTemp[j].l = code[cxTemp + j].l;
			codeTemp[j].a = code[cxTemp + j].a;
		}
		cx = cxTemp;

		block_dx = dx; //save dx before handling procedure call!
		while (sym == SYM_PROCEDURE)
		{ // procedure declarations
			getsym();
			if (sym == SYM_IDENTIFIER)
			{
				enter(ID_PROCEDURE);
				getsym();
			}
			else
			{
				error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
			}


			level++;
			savedTx = tx;
			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset(set1, fsys);
			block(set);
			destroyset(set1);
			destroyset(set);
			tx = savedTx;
			level--;

			
		} // while
		dx = block_dx; //restore dx after handling procedure call!
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		// test(set, declbegsys, 7);
		destroyset(set1);
		destroyset(set);
	}
	while (inset(sym, declbegsys));
	/*代码回填*/
	code[mk->address].a = cx;
	mk->address = cx;
	cx0 = cx;
	gen(INT, 0, block_dx);

	int i;
	for (i = 0; i<tempCodeCount; i++)
	{
		code[cx].f = codeTemp[i].f;
		code[cx].l = codeTemp[i].l;
		code[cx++].a = codeTemp[i].a;
	}

	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);
	statement(set);
	destroyset(set1);
	destroyset(set);
	gen(OPR, 0, OPR_RET); // return
} // block

  //////////////////////////////////////////////////////////////////////
/*通过静态链求出数据区基地址的函数base
1.要求说明数据区所在层和当前层的层差
2.返回值：要求的数据区基址
*/
int base(int stack[], int currentLevel, int levelDiff)
{
	int b = currentLevel;

	while (levelDiff--)
		b = stack[b];
	return b;
} // base

  //////////////////////////////////////////////////////////////////////
  // interprets and executes codes.
void interpret()
{
	int pc;        // program counter
	int b,j;         // program, base, and top-stack register
	instruction i; // instruction register

	int heap[HEAPSIZE];  //堆，动态分配内存，给begin end块内定义的变量或数组分配内存
	int heap_temp[HEAPSIZE]; //调用函数之前备份堆，返回的时候恢复堆

	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 3;
	call_stack[0].adress = top;
	call_stack[0].xn = 0;
	stack[1] = stack[2] = stack[3] = 0;
	mask *mk;
	do
	{
		i = code[pc++];
		switch (i.f)
		{
		case LIT:
			stack[++top] = i.a;
			break;
		case INCXC:
			call_stack[cal + 1].xn++;
			break;
		case COPY:
			while (i.a > 0) {
				i.a--;
				top++;
				stack[top] = stack[top - 1];
			}
			break;
		case OPR:
			switch (i.a) // operator
			{
			case OPR_RET:
				call_stack[cal].sn = 0;
				call_stack[cal].xn = 0;
				call_stack[cal].adress = 0;
				cal--;
				top = b - 1;
				pc = stack[top + 3];
				b = stack[top + 2];
				for (j = 0; j < HEAPSIZE; j++)
					heap[j] = heap_temp[j];
				break;
			case OPR_NEG:
				stack[top] = -stack[top];
				break;
			case OPR_ADD:
				top--;
				stack[top] += stack[top + 1];
				break;
			case OPR_MIN:
				top--;
				stack[top] -= stack[top + 1];
				break;
			case OPR_MUL:
				top--;
				stack[top] *= stack[top + 1];
				break;
			case OPR_DIV:
				top--;
				if (stack[top + 1] == 0)
				{
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] /= stack[top + 1];
				break;
			case OPR_ODD:
				stack[top] %= 2;
				break;
			case OPR_EQU:
				top--;
				stack[top] = stack[top] == stack[top + 1];
				break;
			case OPR_NEQ:
				top--;
				stack[top] = stack[top] != stack[top + 1];
			case OPR_LES:
				top--;
				stack[top] = stack[top] < stack[top + 1];
				break;
			case OPR_GEQ:
				top--;
				stack[top] = stack[top] >= stack[top + 1];
			case OPR_GTR:
				top--;
				stack[top] = stack[top] > stack[top + 1];
				break;
			case OPR_LEQ:
				top--;
				stack[top] = stack[top] <= stack[top + 1];
				break;
				/*下面为添加的运算相应的执行方案*/
			case OPR_MOD:
				top--;
				stack[top] %= stack[top + 1];
				break;
			case OPR_AND:
				top--;
				stack[top] = stack[top] && stack[top + 1];
				break;
			case OPR_OR:
				top--;
				stack[top] = stack[top] || stack[top + 1];
				break;
			case OPR_ANDBIT:
				top--;
				stack[top] = stack[top] & stack[top + 1];
				break;
			case OPR_ORBIT:
				top--;
				stack[top] = stack[top] | stack[top + 1];
				break;
			case OPR_XOR:
				top--;
				stack[top] = stack[top] ^ stack[top + 1];
				break;
			case OPR_NOT:
				stack[top] = !stack[top];
				break;
			case OPR_NOTBIT:
				stack[top] = ~stack[top];
				break;
			case OPR_SHL:
				top--;
				stack[top] = stack[top] << stack[top + 1];
				break;
			case OPR_SHR:
				top--;
				stack[top] = stack[top] >> stack[top + 1];
				break;
			} // switch
			break;
		case LOD:
			stack[++top] = stack[base(stack, b, i.l) + i.a];
			break;
		case LOD_HEAP:
			stack[++top] = heap[i.a]; // push var on heap
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
			printf("%d\n", stack[top]);
			top--;
			break;
			/*添加PRINT*/
		case STO_HEAP: // store var on heap
			heap[i.a] = stack[top];
			printf("%d\n", stack[top]);
			top--;
			break;
		case PRINT:
			if (i.l)
			{

				mk = (mask*)&table[i.l];
				printf("%s = %d\n", mk->name, stack[top]);
				top--;
			}
			else printf("\n");
			break;
		case CAL:
			call_stack[++cal].adress = top;
			stack[top + 1] = base(stack, b, i.l);//静态链
			// generate new block mark
			stack[top + 2] = b;//动态链
			stack[top + 3] = pc;//返回地址
			b = top + 1;
			pc = i.a;
			for (j = 0; j < HEAPSIZE; j++)
				heap_temp[j] = heap[j];
			break;
		case INT:
			top += i.a;
			/*
			if (i.a == 1 && code[pc].f==CAL) 
			{
				int j = 2;
				while (1) {
					if (code[pc - j].f == LIT) {
						call_stack[cal+1].xn++;
						j++;
					}//常数
					else if (code[pc - j].f == LOD) {
						call_stack[cal+1].xn++;
						j++;
					}//变量
					else if (code[pc - j].f == LODARR) {//数组
						call_stack[cal+1].xn++;
						j++;
						while (1) {
							if (code[pc - j].f == OPR && code[pc - j - 1].f == LIT) {
								if (code[pc - j - 2].f == LIT) {
									j += 3;
									break;
								}
								else if(code[pc-j].f==LIT){
									j++;
									break;
								}else{
									j++;
								}
							}
						}
					}
					else if (code[pc - j].f == LODADD) {//传地址
						j++;
						call_stack[cal+1].xn++;
					}
					else if (code[pc - j].f == LODA) {//指针
						j++;
						call_stack[cal+1].xn++;
						while (code[pc - j].f == LODA)
							j++;
						j++;
					}
					else {
						break;
					}

				}
			}
			*/
			if (i.a >= 3) 
			{
				call_stack[cal].sn += i.a - 3;
			}
			break;
		case JMP:
			pc = i.a;
			break;
			/*添加下面的指令*/
		case RET:
			call_stack[cal].sn = 0;
			call_stack[cal].xn = 0;
			call_stack[cal].adress = 0;
			cal--;
			stack[b + i.a] = stack[b - 1];
			pc = stack[b + 2];
			int bTemp = b;
			b = stack[b + 1];
			top = bTemp + i.a;
			for (j = 0; j < HEAPSIZE; j++)
				heap[j] = heap_temp[j];
			break;
		case STOARR:
			/*将次栈顶的值作为偏移量*/
			stack[base(stack, b, i.l) + i.a + stack[top - 1]] = stack[top];
			 printf("%d\n",stack[top]);
			top -= 2;
			break;
		case LODARR_HEAP:
			stack[top] = heap[i.a + stack[top]];
			// for(int k=0;k<20;k++)printf("%-3d ",stack[k]);
			break;
		case STOARR_HEAP:
			heap[i.a + stack[top - 1]] = stack[top];
			// for(int k=0;k<20;k++)printf("%-3d ",stack[k]);
			// printf("\n");
			printf("%d\n", stack[top]);
			top -= 2;
			break;
		case LODARR:
			stack[top] = stack[base(stack, b, i.l) + i.a + stack[top]];
			// for(int k=0;k<20;k++)printf("%-3d ",stack[k]);
			break;
		case JZ:
			if (stack[top] == 0)
				pc = i.a;
			top--;
			break;
		case JZS:
			if (stack[top] == 0)
				pc = i.a;
			break;
		case JNZS:
			if (stack[top] != 0)
				pc = i.a;
			break;
		case JNZ:
			if (stack[top] != 0)
				pc = i.a;
			top--;
			break;
		case JE:
			if (stack[top - 1] == stack[top])
				pc = i.a;
			break;
		case JNE:
			if (stack[top - 1] != stack[top])
				pc = i.a;
			break;
		case JG:
			if (stack[top - 1] > stack[top])
				pc = i.a;
			break;
		case JGE:
			if (stack[top - 1] >= stack[top])
				pc = i.a;
			break;
		case JL:
			if (stack[top - 1] < stack[top])
				pc = i.a;
			break;
		case JLE:
			if (stack[top - 1] <= stack[top])
				pc = i.a;
			break;
		case BAC:
			top -= i.a;
			break;
		case CPY:
			stack[top] = stack[top + i.a];
			break;
		case STOA:
			stack[stack[top - 1]] = stack[top];
			printf("%d\n", stack[top]);
			top -= 2;
			break;
		case LODA:
			stack[top] = stack[stack[top]];
			break;
		case LODADD:
			stack[++top] = base(stack, b, i.l) + i.a;
			break;
		case CALLSTACK:
			printf("************stack***********\n");
			for (j = 0; j <= cal; j++) {
				printf("第%d个函数:\n", j);
				printf("静态链位置:%d\n", call_stack[j].adress + 1);
				printf("动态链位置:%d\n", call_stack[j].adress + 2);
				printf("返回地址pc位置:%d\n", call_stack[j].adress + 3);
				printf("形参个数: %d\n", call_stack[j].xn);
				printf("函数内部全局变量所占空间大小: %d\n", call_stack[j].sn);
			}
			for (j = 0; j <= top; j++) {
				printf("%d ", stack[j]);
			}
			printf("\n*************end************\n");
			break;
		}// switch
	} while (pc);

	printf("End executing PL/0 program.\n");
} // interpret
  //////////////////////////////////////////////////////////////////////
void initial_list()
{
	int stack[1000];
	int index = 0;
	int l = 0;
	if (sym == SYM_LBRACE)
	{
		l++;
		getsym();
		if (initial_flag == 1)  arrayDim[adx + 1]++;
		while (l && l <= arrayDim[adx])
		{
			if (sym == SYM_LBRACE)
			{
				l++;
				if (l == arrayDim[adx])
				{
					getsym();
					int count = 0;
					while (sym != SYM_RBRACE)
					{
						if (sym == SYM_NUMBER)
						{
							count++;
							if (count > arrayDim[adx + arrayDim[adx]])
							{
								err++;
								printf("too many numbers\n");
								exit(1);
							}
							stack[index++] = num;
							getsym();
						}
						else if (sym == SYM_COMMA) getsym();
					}
					int temp = arrayDim[adx + arrayDim[adx]] - count;
					if (temp > 0)
					{
						int i;
						for (i = 0; i < temp; i++) stack[index++] = 0;
					}
				}
			}
			else if (sym == SYM_RBRACE)  //'}'
			{
				l--;
				getsym();
			}
			else if (sym == SYM_COMMA)  //','
			{
				if (l == 1 && initial_flag == 1)
					arrayDim[adx + 1]++;
				getsym();
			}
			else  //number
			{
				stack[index++] = num;
				getsym();
			}

		}
		if (l > arrayDim[adx])
		{
			err++;
			printf("error: braces around scalar initializer\n");
			exit(1);
		}
		//生成初始化数组的代码
		int c = 1;
		int i;
		for (i = 1; i <= arrayDim[adx]; i++)
		{
			c *= arrayDim[adx + i];
		}
		if (heap_or_stack == STACK)
			dx += c - 1;
		else dx1 += c - 1;
		if (index + 1 < c)
		{
			for (i = index + 1; i < c; i++)  stack[i] = 0;
		}
		mask* mk;
		if (heap_or_stack == STACK)
		{
			mk = (mask*)&table[tx];
			for (i = 0; i < c; i++)
			{
				gen(LIT, 0, i);
				gen(LIT, 0, stack[i]);
				gen(STOARR, 0, mk->address);
			}
		}
		else  //HEAP
		{
			mk = (mask*)&table[tx1];
			for (i = 0; i < c; i++)
			{
				gen(LIT, 0, i);
				gen(LIT, 0, stack[i]);
				gen(STOARR_HEAP, 0, mk->address);
			}
		}
	}
	else
	{
		err++;
		printf(" '{' expected\n");
	}
}
  //////////////////////////////////////////////////////////////////////
int main(int argc,char *argv[])
{
	FILE* hbin;
	char s[80];
	int i;
	symset set, set1, set2;
	if ((infile = fopen(argv[1], "r")) == NULL)
	{
		printf("File %s can't be opened.\n", argv[1]);
		exit(1);
	}
	/*和exit相关*/
	code[ENDCX].f = OPR;
	code[ENDCX].l = 0;
	code[ENDCX].a = OPR_RET;
	
	phi = createset(SYM_NULL);
	relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);

	// create begin symbol sets
	declbegsys = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE, SYM_RETURN, SYM_IDENTIFIER,SYM_SWITCH, SYM_EXIT, SYM_FOR, SYM_CONTINUE, SYM_BREAK, SYM_SWITCH, SYM_TIMES, SYM_PRINT, SYM_GOTO, SYM_CALLSTACK, SYM_INC, SYM_DEC,SYM_NULL);
	facbegsys = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_NOT ,SYM_INC, SYM_DEC,SYM_TIMES,SYM_RANDOM,SYM_ANDBIT, SYM_NOTBIT, SYM_NULL);

	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	kk = MAXIDLEN;

	getsym();

	set1 = createset(SYM_PERIOD, SYM_NULL);
	set2 = uniteset(declbegsys, statbegsys);
	set = uniteset(set1, set2);
	block(set);
	destroyset(set1);
	destroyset(set2);
	destroyset(set);
	destroyset(phi);
	destroyset(relset);
	destroyset(declbegsys);
	destroyset(statbegsys);
	destroyset(facbegsys);

	if (sym != SYM_PERIOD)//一个完整的程序由程序体和句号组成
		error(9); // '.' expected.
	if (err == 0)
	{
		hbin = fopen("hbin.txt", "w");
		for (i = 0; i < cx; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
	}
	if (err == 0)
		interpret();
	else
		printf("There are %d error(s) in PL/0 program.\n", err);
	listcode(0, cx);
} // main

  //////////////////////////////////////////////////////////////////////
  // eof pl0.c