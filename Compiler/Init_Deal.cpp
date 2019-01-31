/*
	初始化过程
*/
#include <iostream>
#include "Init_Deal.h"
namespace compiler{
    //初始化索引值和全局变量
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
		MipsTable.mpc = 0;	//mips代码表索引变量
	}

	//Keymap与Spsmap初始化
	void Set_Up(){
		//KeyMap初始化
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

		//SpsMap初始化
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

		//MidOpKind 初始化 目前有21个
		MidOpKind[0] = "+";	// +  z, x, y	加法指令 add/addi 不支持两个数字相加赋予z
		MidOpKind[1] = "-";	// -  z, x, y	减法指令 sub  z = x - y;
		MidOpKind[2] = "*"; // *  z, x, y	乘法指令 mul  z = x * y
		MidOpKind[3] = "/"; // /  z, x, y	除法指令 div	z = x/y
		MidOpKind[4] = "=";	// =  z, (x), y	赋值指令 mov	z = y or z = y[x]

		MidOpKind[5] = "goto"; // goto	z  无条件跳转到label
		MidOpKind[6] = "beq"; // beq	z, x, y  x == y, goto z(z此时为label名称)
		MidOpKind[7] = "bne"; // bne z, x, y  x!=y, goto z
		MidOpKind[8] = "bge"; // bge z, x, y	 x>=y, goto z
		MidOpKind[9] = "bgt"; // bgt z, x, y	x>y, goto z
		MidOpKind[10] = "ble"; // ble z, x, y  x<=y, goto z
		MidOpKind[11] = "blt"; // blt z, x, y  x<y, goto z
		MidOpKind[12] = "ret";  // ret y  函数返回操作,返回值y赋予RET  return temp / return 12
									//    函数返回调用位置, 实际为 PC = S[BASEADDR];
									//    更新运行栈的数据基址BASEADDR = S[BASEADDR + 1]

		MidOpKind[13] = "setlab"; // setlab   y 设置标签名字为y
		MidOpKind[14] = "func"; // func  x, y  函数声明，y为函数名, x为类型 int, char, void
		MidOpKind[15] = "para"; // para	x , y  函数定义时的形参，x为类型，y为形参名
		MidOpKind[16] = "in";	// in   y   调用scanf，将值放到y
		MidOpKind[17] = "out";  // out	x, y	输出字符串x, 表达式的值y, 输出字符串最长为100个字符   当y为char类型时 z = "char"
		MidOpKind[18] = "passpara"; // passpara  y   传递参数操作
		MidOpKind[19] = "call"; // call	y   函数调用操作, y为函数名称
								//相当于 T = BASEADDR;	BASEADDR = SP; push PC+1+paranum + 1(返回调用处地址,别忘了最后还有一个goto语句);
								//push T; push	0;(返回值先设置为0)

		MidOpKind[20] = "=";	//=  z, x, y	赋值指令 mov	z[x] = y
		MidOpKind[21] = "jal";	// jal z    跳转并链接,紧跟call指令使用

		//MipsOpKind 初始化
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

		//有条件跳转指令打印时 打印  op, x, y offset, z
		MipsOpKind[10] = "beq";		//beq  z, x, y/offset:		beq x, y/offset, z				z为label
		MipsOpKind[11] = "bne";		//bne  z, x, y/offset:		bne x, y/offset, z				z为label
		MipsOpKind[12] = "bge";		//bge z, x, y/offset		bge x, y/offset, z			x >=y 时转移到z
		MipsOpKind[13] = "bgt";		//bgt z, x, y/offset		bgt x, y/offset, z			x > y 时转移到z
		MipsOpKind[14] = "ble";		//ble z, x, y/offset		ble x, y/offset, z			x <=y 时转移到z
		MipsOpKind[15] = "blt";		//blt z, x, y/offset		blt x, y/offset, z			x < y 时转移到z

		MipsOpKind[16] = "j";		//j z			无条件跳转
		MipsOpKind[17] = "jr";		//jr z			跳转到寄存器z
		MipsOpKind[18] = "jal";		//jal z			跳转并链接, z为标签名

		MipsOpKind[19] = "li";		//li z, offset
		MipsOpKind[20] = "move";	//move z, x		z = x
		MipsOpKind[21] = "syscall";	//syscall		系统调用
		MipsOpKind[22] = "label";		//一个标签 z
		MipsOpKind[23] = "la";			// la z, x      将标签x的地址赋给z
	}



	//打开源码文件、结果输出文件和错误输出文件
	void Open_File(){
		//用户在控制台输入源文件名称
		std::cout << "请输入txt源代码文件名:" << std::endl;
        std::string inFileName;
		std::cin >> inFileName;
		//打开源文件
		inFile.open(inFileName, std::fstream::in);
		//打开出错输出文件
		erroroutFile.open("errorinfo.txt",std::fstream::out);
		//打开输出词法分析结果的文件
		word_out_file.open("word_output.txt", std::fstream::out);
	}

	void init(){
        Init_Var();
		Set_Up();
		Open_File();
    }
}
