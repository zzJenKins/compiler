#ifndef _PRINTRESULT_H_
#define _PRINTRESULT_H_
#include "Var_Info.h"
namespace compiler {
		//向文件中输出结果文件
	void Print_Mid_Code();
	void Print_Mips_Code();
	void Print_Mid_Opt_Code();	//优化后的中间代码表
	void Print_Mips_Opt_Code();	//优化后的中间代码表

}

#endif
