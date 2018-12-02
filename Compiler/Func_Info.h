/*
	类似于varinfo, 改文件存储全局用到的一些小函数
*/
#ifndef _FUNCINFO_H_
#define _FUNCINFO_H_
#include <sstream>
#include "Var_Info.h"
#include "Error_Deal.h"
namespace compiler{
	//int转字符串
	std::string int_to_string(int n);

	//字符串转int
	int string_to_int(std::string s);

	//判断string对象存储的是否为num数值
	bool string_is_num(std::string s);

	//find in tab, 找到返回下标, 找不到返回0
	int find_in_tab(std::string name);

	//判断当前sy是否在st中,n为数组元素个数,不属于在返回0,属于返回1
	int find_sy(symbol st[], int n);
}

#endif
