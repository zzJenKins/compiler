/*
	������varinfo, ���ļ��洢ȫ���õ���һЩС����
*/
#ifndef _FUNCINFO_H_
#define _FUNCINFO_H_
#include <sstream>
#include "Var_Info.h"
#include "Error_Deal.h"
namespace compiler{
	//intת�ַ���
	std::string int_to_string(int n);

	//�ַ���תint
	int string_to_int(std::string s);

	//�ж�string����洢���Ƿ�Ϊnum��ֵ
	bool string_is_num(std::string s);

	//find in tab, �ҵ������±�, �Ҳ�������0
	int find_in_tab(std::string name);

	//�жϵ�ǰsy�Ƿ���st��,nΪ����Ԫ�ظ���,�������ڷ���0,���ڷ���1
	int find_sy(symbol st[], int n);
}

#endif
