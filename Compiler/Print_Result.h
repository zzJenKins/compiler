#ifndef _PRINTRESULT_H_
#define _PRINTRESULT_H_
#include "Var_Info.h"
namespace compiler {
		//向文件中输出结果文件
	void printMidResult();
	void printMipsResult();
	void printMidOptResult();	//优化后的中间代码表
	void printMipsOptResult();	//优化后的中间代码表

}

#endif
