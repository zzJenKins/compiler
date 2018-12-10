/*
	语法语义分析, 将源程序转换为四元式中间指令
*/
#include "Func_Info.h"
#include "Word_Deal.h"
#include "Enter_Info.h"
#include "Var_Info.h"
#include "Print_Result.h"
#ifndef _BLOCKDEAL_H_
#define _BLOCKDEAL_H_
namespace compiler{
	void block();	//处理 文法中"程序",并生成中间代码
}
#endif
