// pl0 compiler source code

#pragma warning(disable:4996)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "PL0.h"
#include "set.c"


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
/*��Դ�ļ��ж�ȡһ�е��л�������,��������Ϊ�ո�洢��ÿ�δ��л�������ȡһ���ַ�*/
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
/*�ʷ�������س��������ո���Ʊ������������*/
void getsym(void)
{
	/*��¼��һ��sym*/
	presym = sym;
	int i, k;
	char a[MAXIDLEN + 1];

	while (ch == ' ' || ch == '\t')
		getch();
	/*������ĸ���ִ������ʷ��ǺŸ���sym�����ʸ���id���൱�ڷ���һ��(id,sym)��(���ʣ��Ǻ�)�Ķ�Ԫ��*/
	if (isalpha(ch))
	{ // symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < MAXIDLEN)
				a[k++] = ch;
			getch();
		} while (isalpha(ch) || isdigit(ch));//��ĸ��ͷ����ĸ���ִ�
		a[k] = 0;
		strcpy(id, a);//���ʸ���id
		word[0] = id;
		i = NRW;
		while (strcmp(id, word[i--]));
		if (++i)
			sym = wsym[i]; // symbol is a reserved word
		else
			sym = SYM_IDENTIFIER;   // symbol is an identifier
	}
	else if (isdigit(ch))	//�������֣�����(num,SYM_NUMBER)��numΪ���ֶ�Ӧ����ֵ
	{ // symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		do
		{
			num = num * 10 + ch - '0';// last number read
			k++;
			getch();
		} while (isdigit(ch));//�����ֿ�ͷ�����ִ�
		if (k > MAXNUMLEN)
			error(25);     // The number is too great.
	}
	else if (ch == ':')		//�������ֻ��Ҫ����id����
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_BECOMES; // :=
			getch();
		}
		else
		{
			sym = SYM_NULL;       // illegal?
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
	else if (ch == '/')		//�ж�ע��
	{
		getch();
		if (ch == '/') 
		{
			cc = ll;
			getch();
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
		else {
			sym = SYM_SLASH;
			getch();
		}
	}
	else if (ch == '|') //�߼���������ж�
	{
		getch();
		if (ch == '|') 
		{
			sym = SYM_OR;
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
/*Ŀ�����������غ���
x:Ҫ����һ�д�������Ƿ�
y,z:����������
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
s1:���﷨����������˳�ĳһ�﷨��Ԫʱ��ǰ���ʷ���Ӧ���ڵļ���
s2:��ĳһ����״̬�£��ɻָ��﷨�������������Ĳ��䵥�ʼ���
n:������Ϣ��ţ�����ǰ���Ų����ںϷ��� s1 ����ʱ�����ĳ�����Ϣ
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

// enter object(constant, variable or procedre) into table.
/*����k:����½�����ű������*/
void enter(int kind)
{
	mask* mk;

	tx++;//���ű����һ���µĿ�λ
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
		mk->level = level;
		break;
	} // switch
} // enter
/*��½���ű�*/
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
�ڷ��ű��в���ָ���������ڵ�λ��
������Ҫ�ҵķ���
*/
int position(char* id)
{
	int i;
	strcpy(table[0].name, id);
	i = tx + 1;
	while (strcmp(table[--i].name, id) != 0);
	return i;
} // position
//���������ű�
void enterArray()
{
	mask *mk;
	table[tx].kind = ID_ARRAY;//����ǰ���������Ͷ���Ϊ����
	mk = (mask *)&table[tx];
	mk->level = level;
	mk->address = dx-1;		//��enter���ű��ʱ��dx�Ѿ�++��
	mk->arrayAdd = ++adx;	
	temp_adx = adx;			//temp_adx��Զָ�������ά��
	arrayDim[adx] = 0;		//�����ά����0
}
void arrayDecl()
{
	if (presym == SYM_LEFTSPAREN)
	{
		if (sym != SYM_NUMBER)
		{
			printf("expected number when declare array.\n");
			err++;
			return;
		}
		else 	// sym == SYM_LEFTSPAREN
		{
			arrayDim[temp_adx]++;	//�����ά������1
			adx++;
			arrayDim[adx] = num;	//�洢�����Ӧά���Ĵ�С
			getsym();
		}
		if (sym != SYM_RIGHTSPAREN)
		{
			printf("expected rightsparen here while declare array at dim %d\n", arrayDim[temp_adx]);
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
		if (sym == SYM_LEFTSPAREN)
		{
			getsym();
			arrayDecl();
		}
		else // ; or ,
		{
			int count = 1;
			int i = 1;
			for(i = 1; i <= arrayDim[temp_adx]; i++)
			{
				count *= arrayDim[temp_adx + i];
			}//������������Ŀռ��С
			dx += count - 1;
			return;
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
/*�������������ڷ��ű��е�λ��
������������Ԫ������ڻ���ַ��Ԫ������ջ��
*/
void calAdd(int i)
{
	void expression_orbit(symset fsys);
	mask *mk = (mask *)&table[i];
	int temp = mk->arrayAdd;

	if (sym == SYM_LEFTSPAREN)//��������
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
			readDim++;//�Ѿ�����������ά��
			gen(OPR, 0, OPR_ADD);//�ȼ�
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
				gen(OPR, 0, OPR_MUL);//�ٳ�
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
/*����������ֵ����*/
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
/*���������������*/
void vardeclaration(void)
{
	if (sym == SYM_IDENTIFIER)
	{
		enter(ID_VARIABLE);
		getsym();
		if (sym == SYM_LEFTSPAREN)
		{
			getsym();
			enterArray();
			arrayDecl();
		}
	}
	else
	{
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration
/*�����б�ķ�������*/
void paraList(symset fsys)
{
	symset set1, set;
	set1 = createset(SYM_RPAREN, SYM_COMMA, SYM_NULL);
	set = createset(SYM_IDENTIFIER, SYM_RPAREN, SYM_NULL);//presym�Ǳ�ʶ��ʱ�ĺϷ��Ŀ�ʼ����
	if (presym == SYM_LPAREN)/*ǰһ��������������*/
	{
		test(set, fsys, 26);//"There must be an identifier"
		if (sym == SYM_IDENTIFIER)/*������ʶ��*/
		{
			char idTemp[MAXIDLEN + 1];
			strcpy(idTemp, id);
			getsym();
			paraList(fsys);
			enterPara(idTemp, ID_VARIABLE);
		}
	}
	else if (presym == SYM_COMMA)/*ǰһ�������Ƕ���*/
	{
		test(set, fsys, 26);//"There must be an identifier"
		if (sym == SYM_IDENTIFIER)/*��ǰ�����Ǳ�ʶ��*/
		{
			char idTemp[MAXIDLEN + 1];
			strcpy(idTemp, id);
			getsym();
			paraList(fsys);
			enterPara(idTemp, ID_VARIABLE);
		}
		else/*���򱨴�*/
		{
			error(26);//"The paralist is wrong",
		}
	}
	else if (presym == SYM_IDENTIFIER)/*ǰһ�������Ǳ�ʶ��*/
	{
		test(set1, fsys, 27);//There must be an ','or')'
		if (sym == SYM_COMMA)
		{
			getsym();
			paraList(fsys);

		}
		/*���Ƕ��ŵĻ�����*/
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


  //////////////////////////////////////////////////////////////////////
/*���Ӵ������*/
/*fsys���������������ָ��﷨�����ķ��ż���*/
void factor(symset fsys)
{
	void expression_orbit(symset fsys);
	int i;
	symset set;

	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.

	if (inset(sym, facbegsys))
	{
		if (sym == SYM_IDENTIFIER)/*���������ʶ��*/
		{
			if ((i = position(id)) == 0)/*����ű�0����û���ҵ�*/
			{
				error(11); // Undeclared identifier.
			}
			else
			{
				mask* mk = (mask*)&table[i];
				switch (table[i].kind)
				{				
				case ID_CONSTANT:
					/*�����ʶ����Ӧ���ǳ�����ֵΪval������LITָ���ֵ����ջ��*/
					gen(LIT, 0, table[i].value);
					getsym();
					break;
				case ID_ARRAY:
					getsym();
					gen(LIT, 0, 0);
					readDim = 0;//��ʼ��
					calAdd(i);
					mk = (mask *)&table[i];
					gen(LODARR, level - mk->level, mk->address);
					break;
				case ID_VARIABLE:
					/*�����ʶ���Ǳ�����������LODָ��*/
					/*��λ�ڵ�ǰ��ε�ƫ�Ƶ�ַΪaddress��ֵ����ջ��*/
					mk = (mask*)&table[i];
					gen(LOD, level - mk->level, mk->address);//��������ֵ����ջ��
					getsym();
					/*�������++*/
					if (sym == SYM_INC)
					{
						gen(LOD, level - mk->level, mk->address);//��������ֵ����ջ��
						gen(LIT, 0, 1);//��������ջ��
						gen(OPR, 0, OPR_ADD);//���
						gen(STO, level - mk->level, mk->address);//��ջ����ֵ�������
						getsym();
					}
					else if (sym == SYM_DEC)
					{
						gen(LOD, level - mk->level, mk->address);
						gen(LIT, 0, 1);
						gen(OPR, 0, OPR_MIN);
						gen(STO, level - mk->level, mk->address);
						getsym();
					}
					break;
				case ID_PROCEDURE:
					set = uniteset(fsys, createset(SYM_LPAREN, SYM_RPAREN, SYM_SEMICOLON, SYM_NULL));
					getsym();
					test(createset(SYM_LPAREN, SYM_NULL), set, 28);//There must be an '(' after procedure"
					if (sym == SYM_LPAREN)
					{
						getsym();
						procedureCall(set);
						printf("mk->address is %d\n", mk->address);
						gen(CAL, level - mk->level, mk->address);
					}
					destroyset(set);
					test(createset(SYM_SEMICOLON, SYM_NULL), fsys, 10);//"';' expected."
					break;
				} // switch
			}
		}
		else if (sym == SYM_NUMBER)
		{
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			/*����ָ������ֵ�����ֵ����ջ��*/
			gen(LIT, 0, num);
			getsym();
		}
		else if (sym == SYM_LPAREN)
		{
			getsym();
			/*��������ʽ��Ԥ�ڷ����У���˼���SYM_RPAREN*/
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			expression_orbit(set);
			destroyset(set);
			if (sym == SYM_RPAREN)/*���ʽ�������Ӧ������������*/
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
			expression_orbit(fsys);
			/*����ָ���ջ��Ԫ��ȡ��*/
			gen(OPR, 0, OPR_NEG);
		}
		else if (sym == SYM_NOT) 
		{
			getsym();
			expression_orbit(fsys);
			/*����ָ��������ķ�*/
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
				mask *mk = (mask *)&table[i];
				gen(LOD, level - mk->level, mk->address);//������ֵ����ջ��
				gen(LIT, 0, 1);//��������ջ��
				gen(OPR, 0, OPR_ADD);//���
				gen(STO, level - mk->level, mk->address);//��ջ����ֵ�������
				gen(LOD, level - mk->level, mk->address);//������ֵ����ջ��
				getsym();
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
				gen(LOD, level - mk->level, mk->address);
				gen(LIT, 0, 1);
				gen(OPR, 0, OPR_MIN);
				gen(STO, level - mk->level, mk->address);
				gen(LOD, level - mk->level, mk->address);
				getsym();
			}
		}
		test(fsys, createset(SYM_LPAREN, SYM_RPAREN, SYM_NULL), 23);
	} // if
} // factor

  //////////////////////////////////////////////////////////////////////
void term(symset fsys)
{
	int mulop;
	symset set;

	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_MOD ,SYM_NULL));
	factor(set);
	while (sym == SYM_TIMES || sym == SYM_SLASH || sym == SYM_MOD)
	{
		mulop = sym;
		getsym();
		factor(set);
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
	/*addop������¼�������ʽ*/
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
/*��Ӱ�λ�룬��λ�򣬰�λ���*/
void expression_andbit(symset fsys)//&
{
	symset set;

	set = uniteset(fsys, createset(SYM_ANDBIT, SYM_NULL));
	expression(set);
	while (sym == SYM_ANDBIT)
	{
		getsym();
		expression(set);
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
  //////////////////////////////////////////////////////////////////////
void condition(symset fsys)
{
	/*relop������¼�߼����ʽ*/
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
			/*error(20);*/ // ɾȥ����ĳ����жϣ�����������Ϊ���ʽ
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
//����߼����
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
void short_condition_and(symset fsys)
{
	int cxTemp[1000], cxTempCount;
	cxTempCount = 0;
	condition(uniteset(createset(SYM_AND, SYM_NULL), fsys));
	cxTemp[++cxTempCount] = cx;
	gen(JZS, 0, 0);
	while (sym == SYM_AND)
	{
		getsym();
		gen(BAC, 0, 1);
		condition(uniteset(createset(SYM_AND, SYM_NULL), fsys));
		cxTemp[++cxTempCount] = cx;
		gen(JZS, 0, 0);
	}
	int i;
	for (i = cxTempCount; i >= 1; i--)
	{
		code[cxTemp[i]].a = cx;
	}

}

void short_condition_or(symset fsys)
{
	int cxTemp[1000], cxTempCount;
	cxTempCount = 0;
	short_condition_and(uniteset(createset(SYM_OR, SYM_NULL), fsys));
	cxTemp[++cxTempCount] = cx;
	gen(JNZS, 0, 0);
	while (sym == SYM_OR)
	{
		getsym();
		gen(BAC, 0, 1);
		short_condition_and(uniteset(createset(SYM_OR, SYM_NULL), fsys));
		cxTemp[++cxTempCount] = cx;
		gen(JNZS, 0, 0);
	}
	int i;
	for (i = cxTempCount; i >= 1; i--)
	{
		code[cxTemp[i]].a = cx;
	}
}
  //////////////////////////////////////////////////////////////////////
void statement(symset fsys)
{
	void statement(symset fsys);
	int i, j=0, cx1, cx2, cx3[100];
	symset set1, set;
	int retOffset;
	if (sym == SYM_RETURN)
	{
		mask *mk;
		int j = position(id);
		if (j)
		{

			mk = (mask *)&table[j];
			retOffset = mk->address;
		}
		getsym();
		expression_orbit(fsys);
		gen(STO, 0, -1);
		gen(RET, 0, retOffset); //2017.10.30
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
		if (!(i = position(id)))
		{
			error(11); // Undeclared identifier.
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
				procedureCall(set);
				gen(CAL, level, mk->address); // 2017.10.30 level - mk->level change to level
			}
			else
			{
				printf("expect ( here \n");
			}
			destroyset(set);
			test(createset(SYM_SEMICOLON, SYM_NULL), fsys, 10);//"';' expected."
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
				expression_orbit(uniteset(createset(SYM_RIGHTSPAREN,SYM_NULL),fsys));
				gen(STOARR, level - mk->level, mk->address);
			}
			else
			{
				error(13); // ':=' expected.
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
				expression_orbit(fsys);
				/*��β�+ƫ����Ѱַ*/
				if (i)
				{
					gen(STO, level - mk->level, mk->address);
				}
			}
			else if (sym==SYM_INC)
			{
				gen(LOD, level - mk->level, mk->address);
				gen(LOD, level - mk->level, mk->address);
				gen(LIT, 0, 1);
				gen(OPR, 0, OPR_ADD);
				gen(STO, level - mk->level, mk->address);
				getsym();
			}
			else if (sym == SYM_DEC)
			{
				gen(LOD, level - mk->level, mk->address);
				gen(LOD, level - mk->level, mk->address);
				gen(LIT, 0, 1);
				gen(OPR, 0, OPR_MIN);
				gen(STO, level - mk->level, mk->address);
				getsym();
			}
			else
			{
				error(13); // "':=','++'or'--' expected.",
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
			gen(LOD, level - mk->level, mk->address);
			gen(LIT, 0, 1);
			gen(OPR, 0, OPR_ADD);
			gen(STO, level - mk->level, mk->address);
			gen(LOD, level - mk->level, mk->address);
			getsym();
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
			gen(LOD, level - mk->level, mk->address);
			gen(LIT, 0, 1);
			gen(OPR, 0, OPR_MIN);
			gen(STO, level - mk->level, mk->address);
			gen(LOD, level - mk->level, mk->address);
			getsym();
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
			if (sym != SYM_IDENTIFIER)/*��for�е�һ�����ֻ���Ǳ�ʶ����ͷ*/
			{
				printf("expected identifier in the first field of for declaration\n");
				err++;
			}
			else  // E1
			{
				statement(uniteset(createset(SYM_SEMICOLON, SYM_NULL), fsys));
			}
			if (sym != SYM_SEMICOLON)
			{
				printf("expected ';' after the first field in for statement\n");
				err++;
			}
			else  // Condition
			{
				ENext = cx;
				getsym();
				short_condition_or(fsys);
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
				statement(uniteset(createset(SYM_SEMICOLON, SYM_RPAREN, SYM_NULL), fsys));
				tempCodeCount = cx - cxTemp;
				/*ʵ�ִ����ƶ�*/
				int j;
				for ( j = 0; j<tempCodeCount; j++)
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
				int i;
				for (i = 0; i<tempCodeCount; i++)
				{
					code[cx].f = codeTemp[i].f;
					code[cx].l = codeTemp[i].l;
					code[cx++].a = codeTemp[i].a;
				}
				gen(JMP, 0, ENext);
				code[CFalseAdd].a = cx;
			}
		}
	}
	else if (sym == SYM_IF)
	{ // if statement
		j = 0;
		while (sym == SYM_IF || sym == SYM_ELIF)
		{
			getsym();
			if (sym != SYM_LPAREN)
			{
				error(28);
			}
			getsym();
			short_condition_or(uniteset(createset(SYM_RPAREN, SYM_ELIF, SYM_ELSE, SYM_NULL), fsys));
			if (sym != SYM_RPAREN)
			{
				error(22);
			}
			getsym();
			cx1 = cx;
			gen(JZ, 0, 0);//Ϊ0����ת
			statement(uniteset(createset(SYM_RPAREN, SYM_ELIF, SYM_ELSE, SYM_NULL), fsys));
			cx3[j] = cx;//��¼ÿһ����elif��if�ڵ��������ĵ�ַ�����ִ������Щ��䣬��Ҫ��ת�����
			gen(JMP, 0, 0);
			code[cx1].a = cx;//���ǰif����elif
			j++;
		}
		if (sym == SYM_ELSE)
		{
			getsym();
			statement(fsys);
		}
		for (i = 0; i < j; i++)
		{
			code[cx3[i]].a = cx;//if-elif-else ��ĩβ
		}
		/*
		getsym();
		set1 = createset(SYM_THEN, SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		conditions_or(set);
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_THEN)
		{
			getsym();
		}
		else
		{
			error(16); // 'then' expected.
		}
		cx1 = cx;
		gen(JPC, 0, 0);
		statement(fsys);
		code[cx1].a = cx;
		*/
	}
	else if (sym == SYM_BEGIN)
	{ // block
		getsym();
		/*��ʱ��fsys�����Ѿ����˷ֺź�end�������������*/
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
			getsym();
		}
		else
		{
			error(17); //  'end' expected.
		}
	}
	else if (sym == SYM_WHILE)
	{ // while statement
		cx1 = cx;
		getsym();
		if (sym != SYM_LPAREN)
		{
			error(28);//"Missing '(' "
		}
		getsym();
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		short_condition_or(set);
		destroyset(set1);
		destroyset(set);
		if (sym != SYM_RPAREN)
		{
			error(22);
		}
		getsym();
		cx2 = cx;
		gen(JZ, 0, 0);
		if (sym == SYM_DO)
		{
			getsym();
		}
		else
		{
			error(18); // 'do' expected.
		}
		statement(fsys);
		gen(JMP, 0, cx1);
		code[cx2].a = cx;
	}
	test(uniteset(createset(SYM_RETURN, SYM_NULL),fsys), phi, 19);//"Incorrect symbol."
} // statement

  //////////////////////////////////////////////////////////////////////
void block(symset fsys)
{
	/*cx0��¼���㿪ʼʱ����η���λ��*/
	int cx0; // initial code index
	mask* mk;
	int block_dx;
	int savedTx;
	symset set1, set;


	/*�ó�ֵ3��ÿһ���ʼ��λ�÷ֱ���þ�̬������̬�����Լ�����״̬��IP��*/
	dx = 3;		// data allocation index

	block_dx = dx;
	/*���ű��¼�µ�ǰ�����Ŀ�ʼλ��*/
	mk = (mask*)&table[tx];
	mk->address = cx;
	/*������תָ��ȴ�����*/
	gen(JMP, 0, 0);
	/*�����ǰ���̵�Ƕ�ײ����������������Ƕ�ײ����*/

	if (level > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	/*ȷ������level>0�ĺ���һ���У���*/
	if (level > 0) {//�������������жϲ����д���ָ�
		set = uniteset(fsys, createset(SYM_RPAREN, SYM_SEMICOLON, SYM_NULL));
		test(createset(SYM_LPAREN, SYM_NULL), set, 28);
		destroyset(set);
	}
	/*��������ţ����������������*/
	if (sym == SYM_LPAREN)
	{
		getsym();
		/*Ϊ�˴���ָ�������set*/
		set = uniteset(fsys, createset(SYM_RPAREN, SYM_SEMICOLON, SYM_NULL));
		pcount = -2;
		paraList(set);
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
	/*ѭ���������е���������*/
	do
	{
		if (sym == SYM_CONST)
		{ // constant declarations
			getsym();
			do
			{
				constdeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
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
			} while (sym == SYM_CONST);
		} // if

		if (sym == SYM_VAR)
		{ // variable declarations
			getsym();
			do
			{
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

			if (sym == SYM_SEMICOLON)
			{
				getsym();
				set1 = createset(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = uniteset(statbegsys, set1);
				test(set, fsys, 6);	//err_6:"Incorrect procedure name.",
				destroyset(set1);
				destroyset(set);
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}
		} // while
		dx = block_dx; //restore dx after handling procedure call!
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		test(set, declbegsys, 7);	//"Statement expected."
		destroyset(set1);
		destroyset(set);
	} while (inset(sym, declbegsys));

	/*�������*/
	code[mk->address].a = cx;
	mk->address = cx;
	cx0 = cx;
	gen(INT, 0, block_dx);
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);
	statement(set);
	destroyset(set1);
	destroyset(set);
	gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8); // test for error: Follow the statement is an incorrect symbol.
//	listcode(cx0, cx);
} // block

  //////////////////////////////////////////////////////////////////////
/*ͨ����̬���������������ַ�ĺ���base
1.Ҫ��˵�����������ڲ�͵�ǰ��Ĳ��
2.����ֵ��Ҫ�����������ַ
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
	int stack[STACKSIZE];
	int top;       // top of stack
	int b;         // program, base, and top-stack register
	instruction i; // instruction register

	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 3;
	int z = 0;
	for (z = 0; z<STACKSIZE; z++)stack[z] = 0;
	stack[1] = stack[2] = stack[3] = 0;
	do
	{
		i = code[pc++];
		switch (i.f)
		{
		case LIT:
			stack[++top] = i.a;
			break;
		case OPR:
			switch (i.a) // operator
			{
			case OPR_RET:
				top = b - 1;
				pc = stack[top + 3];
				b = stack[top + 2];
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
			/*����Ϊ��ӵ�������Ӧ��ִ�з���*/
			case OPR_MOD:
				top--;
				stack[top] %=  stack[top + 1];
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
			} // switch
			break;
		case LOD:
			stack[++top] = stack[base(stack, b, i.l) + i.a];
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
			printf("%d\n", stack[top]);
			top--;
			break;
		case CAL:
			stack[top + 1] = base(stack, b, i.l);//��̬��
			// generate new block mark
			stack[top + 2] = b;//��̬��
			stack[top + 3] = pc;//���ص�ַ
			b = top + 1;
			pc = i.a;
			break;
		case INT:
			top += i.a;
			break;
		case JMP:
			pc = i.a;
			break;
/*��������ָ��*/
		case RET:
			stack[b + i.a] = stack[b - 1];
			pc = stack[b + 2];
			top = b + i.a;
			b = stack[b + 1];
			break;
		case STOARR:
			/*����ջ����ֵ��Ϊƫ����*/
			stack[base(stack, b, i.l) + i.a + stack[top - 1]] = stack[top];
			// for(int k=0;k<20;k++)printf("%-3d ",stack[k]);
			// printf("\n");
			printf("%d\n the offset is %d\n", stack[top],stack[top-1]);
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
		} // switch
	} while (pc);

	printf("End executing PL/0 program.\n");
} // interpret

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
	/*��exit���*/
	code[ENDCX].f = OPR;
	code[ENDCX].l = 0;
	code[ENDCX].a = OPR_RET;
	
	phi = createset(SYM_NULL);
	relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);

	// create begin symbol sets
	declbegsys = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE, SYM_RETURN,SYM_ELSE,SYM_IDENTIFIER, SYM_INC, SYM_DEC, SYM_FOR, SYM_EXIT, SYM_NULL);
	facbegsys = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_NOT ,SYM_INC, SYM_DEC,SYM_NULL);

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

	if (sym != SYM_PERIOD)//һ�������ĳ����ɳ�����;�����
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