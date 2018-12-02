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
	void enterMidCode(int op, std::string z, std::string x, std::string y, int isstart);
	void enterMidOptCode(int op, std::string z, std::string x, std::string y, int isstart);
	void enterMipsCode(int op, std::string z, std::string x, std::string y, int offset);
	void enterMipsOptCode(int op, std::string z, std::string x, std::string y, int offset);
	void printTab();	//打印tab表
	void printATab();	//打印atab表
	void printBTab();	//打印btab表
	void printMidCode();//打印中间代码表
	void printTMap();	//打印T_Map
}
#endif
