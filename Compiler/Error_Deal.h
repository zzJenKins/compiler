/*
	错误信息搜集及处理程序
*/
#ifndef _ERRORDEAL_H_
#define _ERRORDEAL_H_
#include "Var_Info.h"
namespace compiler{
	void error(int n, int pos); // 对标号为n的错误进行错误处理,输出错误信息
	void fatal(int n, int pos); // 对标号为n的栈溢出错误进行处理
}
#endif
