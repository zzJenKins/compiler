#ifndef VAR_INFO_H_INCLUDED
#define VAR_INFO_H_INCLUDED

#include <map>
#include <string>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

//scanf 与 printf作为内置函数处理
namespace compiler{
#define NAME_SIZE 200	// 标识符最大长度为200
#define LLNG 350   //input line length
#define TMAX 1000 //tab 符号表的容量最大为1000
#define BMAX 100 //btab 分程序表的最大容量为100
#define AMAX 100 //atab 数组表的最大容量
#define CSMAX 30  // case的最大容量
#define SMAX 6000	// size of string-table

//#define COMAX 800	//size of code
#define MID_KIND 30  //中间代码四元式op的种类
#define MIDCODEMAX 2000 	//中间指令最大条数为2000

#define MIPS_KIND 40 //mips的指令种类
#define MIPSCODEMAX 10000	//mips指令的最大条数为2000

//作业不考虑越界情况,所以整数统一用NMAX
//#define XMAX 0xFFFFFFFF //	数组下标的最大值(无符号整数的最大界限)
#define NMAX 0x7FFFFFFF // 数字的最大值 2**31 - 1, 如果是负数则边界是 -(NMAX + 1) 即 -2147483648

//#define LINELENG 132 //output line length
//#define LINELIMIT 200  //行数限制
#define STACKSIZE 2000 //数据栈的大小

//自定义各种枚举类型
enum symbol{
	//标识符
	ident,

	//数据类型
	intcon, //int
	charcon, //char
	stringcon, //char[] “”

	//保留字	KeyMap中
	charsy,
	intsy,
	voidsy,  //void
	constsy, //const
	mainsy, //main
	ifsy, //if
	defaultsy, //defaut
	whilesy, //while
	switchsy, //switch
	casesy, //case
	returnsy, //return
	printfsy, //printf
	scanfsy, //scanf


	//关系运算符
	lss, // '<'
	leq, // '<='
	gtr, // '>'
	geq, // '>='
	neq, // '!='
	eql, // '=='

	//运算符 SpsMap中
	plus, // '+'
	minus, // '-'
	mult,  // '*'
	rdiv, // '/'
	assign, // '=',赋值运算符中的 =

	//标点符号	SpsMap中
	lparent, // '('
	rparent, // ')'
	lsbrack, // '['
	rsbrack, // ']'
	lcbrack, // '{'
	rcbrack, // '}'
	comma, // ','
	//apos, // 单引号
	//douq, // 双引号
	semicolon, // ';'
	colon //':'
};
//便于词法分析打印类型
extern std::string symbol_info[37];

//标识符种类(参照pascal_S)
enum objecttyp {constant, variable, function, array};
extern std::string objecttyp_info[4];

//数据类型
enum types {ints, chars, notyp};
extern std::string types_info[3];



//indentifier table
struct tabe{
			std::string name;//标识符名字,取前十个字符
			objecttyp obj; //标识符种类
			types type; //标识符的数据类型
			int adr;
			uint32_t ref; //指向数组在atab中的位置(登录位置指针值)或者函数名在btab中的位置
			uint32_t link;//指向同一个分程序中上一个标识符在符号表中的位置
			int lev;	//该符号所在的分程序层,在根据符号表查找所在运行栈地址时用到, 局部和全局的基址不同
};

struct tabt{
			struct tabe tabArray[TMAX];
			uint32_t t;	//索引变量t
};

//array-table
//全部用int来存数组元素
struct atabe{
			types eltyp; //数组元素类型,只能为int或者char
			uint32_t high; //数组元素的上界,数组元素从0开始(int a[10],上界为10, 数组下标小于10)
};

struct atabt{
			struct atabe atabArray[AMAX];
			uint32_t a;	//索引变量a
};

//block-table
struct btabe{
			uint32_t last; //指向该分程序中说明的当前(最后)一个标识符在tab表中的位置
			uint32_t lastpar; //函数的最后一个参数在tab中的位置
			types kind; // notyp代表无返回值, ints返回值为int, chars代表返回值为char
			uint32_t paranum; //参数个数
			uint32_t varsize; //局部变量大小(btab.btabArray[0].varnum 存储全局变量大小),在对运行栈函数分配空间时用
			int t_varnum;	//临时变量的个数

};

struct btabt{
			struct btabe btabArray[BMAX];
			uint32_t b; //索引变量b
};

//middle-code table

struct midcode{
			//形如 z = x op y  或者 op, z, x, y,
		   int op; //四元式操作符,配合MidOpKind表来使用
		   std::string z;
		   std::string x;
		   std::string y;
		   int isstart; //标志该四元式中间代码是否为入口语句, >0 均为入口语句
						//其中1标志为普通的跳转标签, 2标志为main函数的入口位置, 3表示函数调用返回的位置, 4表示函数开始标签
};

struct midcodetable{
			struct midcode midcodeArray[MIDCODEMAX];
			uint32_t mdc;	//索引变量
};

//mips-code table

struct mipscode {
	int op; //操作符
	std::string z;			//label名或者寄存器
	std::string x;
	std::string y;
	int offset;	//在需要地址偏移或者使用立即数时采用
};

struct mipscodetable {
	struct mipscode mipscodeArray[MIPSCODEMAX];
	int mpc;	//索引变量
};


//string-table
struct stabt{
			char stringArray[SMAX];
			uint32_t sx;
};

//stack
struct stacke{
			int space[STACKSIZE];	//数据栈
			uint32_t topaddr;		//当前栈顶指针
			uint32_t cbaseaddr;		//当前数据区基地址
			uint32_t globalvaraddr; //全局变量 开始位置
};

//在midtomipsdeal中用到 T_Map, S_Map 使用
struct var_info {
	bool inReg;		//是否在寄存器中, 1表示是, 0表示不是
	int r_addr;		//寄存器的数组下标 0 - 17
	int stack_addr;	//相对基地址偏移
	bool isTFirst;		//临时变量才会用到, 是否为第一次使用(第一次不用lw), 在blockdeal生成临时变量时初始化, 一旦变为false则变不回去了
};



//全局文件
extern std::fstream inFile;	//source-code file
extern std::fstream midoutFile; //mid-code file
extern std::fstream mipsoutFile; //mips-code file
extern std::fstream erroroutFile;	//error-info file
extern std::fstream word_out_file; //用于词法分析第一次需要

//全局表、栈
extern std::map<std::string, symbol> KeyMap;  // 保留字与symbol之间的对应关系
extern std::map<char, symbol> SpsMap; // '+', '-' 等符号与symbol的对应关系
extern tabt tab;	//符号表tab
extern atabt atab; //数组信息向量表
extern btabt btab; //分程序表
//extern ctabt ctab; //常量表
//extern displayt display; //分程序索引表
extern stabt stab; //字符串常量表

//中间代码和Mips代码表
extern midcodetable MidCodeT;	//中间代码表
extern midcodetable MidCodeOptT;	//优化后的中间代码表
extern stacke Stack;	//栈空间stack
extern mipscodetable MipsTable;		//mips代码表
extern mipscodetable MipsTableOpt;		//优化后的mips代码表

extern uint32_t display[2]; //作为索引表的索引,display[0]值为0,btab.btabArray[display[0]].last为全局变量最后一个标识符位置
							//display[1]存储当前局部作用域函数在btab中位置, 即btab.btabArray[display[1]]找到当前函数作用域

//全局变量
extern char ch; //last character read from source program
extern int inum; //intvalue from insymbol
extern int sleng; //string length
extern int cc; //character counter
extern int lc; //program line location counter
extern int ll; //length of current line
extern char line[LLNG + 1];//input line, last char is ' '
extern int errpos; //error positon
extern symbol sy; //last symbol read by insymbol
extern char id[NAME_SIZE]; //indentifier from insymbol
extern int level;	//当前分程序层,只存在0和1两层
extern bool IsError;

//中间代码四元式指令对照表
extern std::string MidOpKind[MID_KIND];	//中间代码op数与指令对照表

//mips指令对照表
extern std::string MipsOpKind[MIPS_KIND];	//mips代码op数与指令对照表

extern std::map<std::string, var_info> T_Map;	//临时变量的信息, 在生成临时变量时处理

extern std::map<std::string, std::string> Str_Info_Map;		//要打印的字符串信息, 在midtomipsdeal中存放，在printresult中使用
}
#endif
