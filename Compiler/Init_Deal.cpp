/*
	��ʼ������
*/
#include <iostream>
#include "Init_Deal.h"
namespace compiler{
    //��ʼ������ֵ��ȫ�ֱ���
	void Init_Var(){
        lc = 0;
		ll = 0;
		cc = 0;
		ch = ' ';
		errpos = 0;
		level = 0;
		display[0] = 0;
		tab.t = 1;	//tab table
		tab.tabArray[0].link = 0;
		atab.a = 0; // atab table
		btab.b = 1; //btab table
		btab.btabArray[0].last = 0;
		stab.sx = 0; //stab table
		MidCodeT.mdc = 0; //MidCodeT
		MipsTable.mpc = 0;	//mips�������������
	}

	//Keymap��Spsmap��ʼ��
	void Set_Up(){
		//KeyMap��ʼ��
		KeyMap["int"] = intsy;
		KeyMap["char"] = charsy;
		KeyMap["void"] = voidsy;
		KeyMap["const"] = constsy;
		KeyMap["main"] = mainsy;
		KeyMap["if"] = ifsy;
		KeyMap["while"] = whilesy;
		KeyMap["switch"] = switchsy;
		KeyMap["case"] = casesy;
		KeyMap["default"] = defaultsy;
		KeyMap["scanf"] = scanfsy;
        KeyMap["printf"] = printfsy;
        KeyMap["return"] = returnsy;

		//SpsMap��ʼ��
		SpsMap['+'] = plus;
		SpsMap['-'] = minus;
		SpsMap['*'] = mult;
		SpsMap['/'] = rdiv;
		//SpsMap['='] = assign;
		SpsMap['('] = lparent;
		SpsMap[')'] = rparent;
		SpsMap['['] = lsbrack;
		SpsMap[']'] = rsbrack;
		SpsMap['{'] = lcbrack;
		SpsMap['}'] = rcbrack;
		SpsMap[','] = comma;
		//SpsMap['\''] = apos;
		//SpsMap['\"'] = douq;
		SpsMap[';'] = semicolon;
		SpsMap[':'] = colon;

		//MidOpKind ��ʼ�� Ŀǰ��21��
		MidOpKind[0] = "+";	// +  z, x, y	�ӷ�ָ�� add/addi ��֧������������Ӹ���z
		MidOpKind[1] = "-";	// -  z, x, y	����ָ�� sub  z = x - y;
		MidOpKind[2] = "*"; // *  z, x, y	�˷�ָ�� mul  z = x * y
		MidOpKind[3] = "/"; // /  z, x, y	����ָ�� div	z = x/y
		MidOpKind[4] = "=";	// =  z, (x), y	��ֵָ�� mov	z = y or z = y[x]

		MidOpKind[5] = "goto"; // goto	z  ��������ת��label
		MidOpKind[6] = "beq"; // beq	z, x, y  x == y, goto z(z��ʱΪlabel����)
		MidOpKind[7] = "bne"; // bne z, x, y  x!=y, goto z
		MidOpKind[8] = "bge"; // bge z, x, y	 x>=y, goto z
		MidOpKind[9] = "bgt"; // bgt z, x, y	x>y, goto z
		MidOpKind[10] = "ble"; // ble z, x, y  x<=y, goto z
		MidOpKind[11] = "blt"; // blt z, x, y  x<y, goto z
		MidOpKind[12] = "ret";  // ret y  �������ز���,����ֵy����RET  return temp / return 12
									//    �������ص���λ��, ʵ��Ϊ PC = S[BASEADDR];
									//    ��������ջ�����ݻ�ַBASEADDR = S[BASEADDR + 1]

		MidOpKind[13] = "setlab"; // setlab   y ���ñ�ǩ����Ϊy
		MidOpKind[14] = "func"; // func  x, y  ����������yΪ������, xΪ���� int, char, void
		MidOpKind[15] = "para"; // para	x , y  ��������ʱ���βΣ�xΪ���ͣ�yΪ�β���
		MidOpKind[16] = "in";	// in   y   ����scanf����ֵ�ŵ�y
		MidOpKind[17] = "out";  // out	x, y	����ַ���x, ���ʽ��ֵy, ����ַ����Ϊ100���ַ�   ��yΪchar����ʱ z = "char"
		MidOpKind[18] = "passpara"; // passpara  y   ���ݲ�������
		MidOpKind[19] = "call"; // call	y   �������ò���, yΪ��������
								//�൱�� T = BASEADDR;	BASEADDR = SP; push PC+1+paranum + 1(���ص��ô���ַ,�����������һ��goto���);
								//push T; push	0;(����ֵ������Ϊ0)

		MidOpKind[20] = "=";	//=  z, x, y	��ֵָ�� mov	z[x] = y
		MidOpKind[21] = "jal";	// jal z    ��ת������,����callָ��ʹ��

		//MipsOpKind ��ʼ��
		MipsOpKind[0] = "addu";		//add z, x, y:	z = x + y
		MipsOpKind[1] = "sub";		//sub z, x, y:  z = x - y
		MipsOpKind[2] = "mul";		//mul z, x, y:  z = x * y
		MipsOpKind[3] = "div";		//div z, x, y:  z = x / y

		MipsOpKind[4] = "addi";		//addi z, x, offset: z = x + offset
		MipsOpKind[5] = "subi";		//subi z, x, offset: z = x - offset
		MipsOpKind[6] = "mul";		//mul  z, x, offset: z = x * offset
		MipsOpKind[7] = "div";		//div  z, x, offset: z = x / offset

		MipsOpKind[8] = "lw";		//lw   z, x, offset:  lw z, offset(x)
		MipsOpKind[9] = "sw";		//sw   z, x, offset:  sw z, offset(x)

		//��������תָ���ӡʱ ��ӡ  op, x, y offset, z
		MipsOpKind[10] = "beq";		//beq  z, x, y/offset:		beq x, y/offset, z				zΪlabel
		MipsOpKind[11] = "bne";		//bne  z, x, y/offset:		bne x, y/offset, z				zΪlabel
		MipsOpKind[12] = "bge";		//bge z, x, y/offset		bge x, y/offset, z			x >=y ʱת�Ƶ�z
		MipsOpKind[13] = "bgt";		//bgt z, x, y/offset		bgt x, y/offset, z			x > y ʱת�Ƶ�z
		MipsOpKind[14] = "ble";		//ble z, x, y/offset		ble x, y/offset, z			x <=y ʱת�Ƶ�z
		MipsOpKind[15] = "blt";		//blt z, x, y/offset		blt x, y/offset, z			x < y ʱת�Ƶ�z

		MipsOpKind[16] = "j";		//j z			��������ת
		MipsOpKind[17] = "jr";		//jr z			��ת���Ĵ���z
		MipsOpKind[18] = "jal";		//jal z			��ת������, zΪ��ǩ��

		MipsOpKind[19] = "li";		//li z, offset
		MipsOpKind[20] = "move";	//move z, x		z = x
		MipsOpKind[21] = "syscall";	//syscall		ϵͳ����
		MipsOpKind[22] = "label";		//һ����ǩ z
		MipsOpKind[23] = "la";			// la z, x      ����ǩx�ĵ�ַ����z
	}



	//��Դ���ļ����������ļ��ʹ�������ļ�
	void Open_File(){
		//�û��ڿ���̨����Դ�ļ�����
		std::cout << "������txtԴ�����ļ���:" << std::endl;
        std::string inFileName;
		std::cin >> inFileName;
		//��Դ�ļ�
		inFile.open(inFileName, std::fstream::in);
		//�򿪳�������ļ�
		erroroutFile.open("errorinfo.txt",std::fstream::out);
		//������ʷ�����������ļ�
		word_out_file.open("word_output.txt", std::fstream::out);
	}

	void init(){
        Init_Var();
		Set_Up();
		Open_File();
    }
}
