/*
	��¼ȫ����Ϣ��
*/
#ifndef _ENTERINFO_H_
#define _ENTERINFO_H_
#include "Var_Info.h"
#include "Error_Deal.h"
namespace compiler{
	bool enter(std::string name, objecttyp obj, types typ); //�ǼǱ�ʶ����Ϣ
	void enterVar(int pos, int adr);	//�ǼǱ���/������Ϣ
	void enterArray(int t_pos, types typ, int high);	//�Ǽ�������Ϣ
	void enterFunc(int t_pos, types typ); //�ǼǺ�����Ϣ
	void Enter_Mid_List(int op, std::string z, std::string x, std::string y, int isstart);
	void Enter_Mid_Opt_List(int op, std::string z, std::string x, std::string y, int isstart);
	void Enter_Mips_List(int op, std::string z, std::string x, std::string y, int offset);
	//void enterMipsOptCode(int op, std::string z, std::string x, std::string y, int offset);
}
#endif
