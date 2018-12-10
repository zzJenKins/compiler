#include "Func_Info.h"

namespace compiler{
	//int->string
	std::string int_to_string(int n){
		std::string func_s;
		std::stringstream k;
		k << n;
		k >> func_s;
		return func_s;
	}

	//string->int
	int string_to_int(std::string s){
		int result = 0;
		std::stringstream k;
		k << s;
		k >> result;
		return result;
	}

	//判断string对象存储的是否为num数值
	bool string_is_num(std::string s) {
		std::stringstream sin(s);
		double d;
		char c;
		if (!(sin >> d)) {
			return false;
		}
		if (sin >> c)
		{
			return false;
		}
		return true;
	}

	//查找name在tab中的位置并返回
	int find_in_tab(std::string name){
		int i = level;
		int j;
		tab.tabArray[0].name = name;
		do{
			j = btab.btabArray[display[i]].last;
			while(tab.tabArray[j].name != name) j = tab.tabArray[j].link;
			i = i - 1;
		}while((i >= 0) && (j == 0));
		/*
		if (j == 0){	//没找到, 报错
			std:: cout << "未识别的标识符: " << name << std::endl;
			error(23, lc);	//标识符未定义;
		}
		*/
		return j;
	}

	//判断当前sy是否在st中,n为数组元素个数,不属于在返回0,属于返回1
	int find_sy(symbol st[], int n){
		for(int i = 0; i < n; i++){
			if(st[i] == sy) return 1;
		}
		return 0;
	}
}
