/*
	全局变量的定义
*/

#include "Var_Info.h"
namespace compiler{
    std::string symbol_info[37] = {"ident","intcon","charcon","stringcon","charsy","intsy","voidsy","constsy","mainsy",
	"ifsy","whilesy","switchsy","casesy","defaultsy","scanfsy","printfsy","returnsy","lss","leq","gtr","geq","neq",
	"eql","plus","minus","mult","rdiv","assign","lparent","rparent","lsbrack","rsbrack","lcbrack",
	"rcbrack","comma","semicolon","colon"};
    std::string objecttyp_info[4] = {"constant", "variable", "function", "array    "};
	std::string types_info[3] = {"ints", "chars", "notyp"};

	std::fstream inFile;	//source-code file
	std::fstream midoutFile; //mid-code file
	std::fstream mipsoutFile; //mips-code file
	std::fstream erroroutFile;	//error-info file
	std::fstream word_out_file; //词法分析输出文件

	std::map<std::string, symbol> KeyMap;
	std::map<char, symbol> SpsMap;
	tabt tab;	//符号表tab
	atabt atab;	//数组信息向量表
	btabt btab; //分程序表
	//ctabt ctab; //常量表
	//displayt display; //分程序索引表
	stabt stab; //字符串常量表

	midcodetable MidCodeT;	//中间代码表
	midcodetable MidCodeOptT;	//优化后的中间代码表
	stacke Stack;	//栈空间stack
	mipscodetable MipsTable;	//mips代码表
	mipscodetable MipsTableOpt;		//优化后的mips代码表

	uint32_t display[2]; //作为索引表的索引,display[0]值为0,btab.btabArray[display[0]].last为全局变量最后一个标识符位置
						//display[1]存储当前局部作用域函数在btab中位置, 即btab.btabArray[display[1]]找到当前函数作用域

	char ch; //last character read from source program
	int inum; //intvalue from insymbol
	int sleng;
	int cc; //character counter
	int lc; //program location counter
	int ll; //length of current line
	char line[LLNG+1]; //input line, last char is '\0'
	int errpos; //erropos
	symbol sy; //last symbol read by insymbol
	char id[NAME_SIZE]; //indentifier from insymbol
	int	level;//当前分程序层
	bool IsError = false;

	std::string MidOpKind[MID_KIND];
	std::string MipsOpKind[MIPS_KIND];

	std::map<std::string, var_info> T_Map;	//临时变量的信息, 在生成临时变量时处理
	std::map<std::string, std::string> Str_Info_Map;

}
