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
	void enterMidCode(int op, std::string z, std::string x, std::string y, int isstart);
	void enterMidOptCode(int op, std::string z, std::string x, std::string y, int isstart);
	void enterMipsCode(int op, std::string z, std::string x, std::string y, int offset);
	void enterMipsOptCode(int op, std::string z, std::string x, std::string y, int offset);
	void printTab();	//��ӡtab��
	void printATab();	//��ӡatab��
	void printBTab();	//��ӡbtab��
	void printMidCode();//��ӡ�м�����
	void printTMap();	//��ӡT_Map
}
#endif
