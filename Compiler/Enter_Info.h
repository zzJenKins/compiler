/*
	登录全局信息表
*/
#ifndef _ENTERINFO_H_
#define _ENTERINFO_H_
#include "Var_Info.h"
#include "Error_Deal.h"
namespace compiler{
	bool enter(std::string name, objecttyp obj, types typ); //登记标识符信息
	void enterVar(int pos, int adr);	//登记变量/常量信息
	void enterArray(int t_pos, types typ, int high);	//登记数组信息
	void enterFunc(int t_pos, types typ); //登记函数信息
	void Enter_Mid_List(int op, std::string z, std::string x, std::string y, int isstart);
	void Enter_Mid_Opt_List(int op, std::string z, std::string x, std::string y, int isstart);
	void Enter_Mips_List(int op, std::string z, std::string x, std::string y, int offset);
	//void enterMipsOptCode(int op, std::string z, std::string x, std::string y, int offset);
}
#endif
