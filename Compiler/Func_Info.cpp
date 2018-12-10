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

	//�ж�string����洢���Ƿ�Ϊnum��ֵ
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

	//����name��tab�е�λ�ò�����
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
		if (j == 0){	//û�ҵ�, ����
			std:: cout << "δʶ��ı�ʶ��: " << name << std::endl;
			error(23, lc);	//��ʶ��δ����;
		}
		*/
		return j;
	}

	//�жϵ�ǰsy�Ƿ���st��,nΪ����Ԫ�ظ���,�������ڷ���0,���ڷ���1
	int find_sy(symbol st[], int n){
		for(int i = 0; i < n; i++){
			if(st[i] == sy) return 1;
		}
		return 0;
	}
}
