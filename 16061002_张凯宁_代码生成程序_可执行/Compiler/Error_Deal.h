/*
	������Ϣ�Ѽ����������
*/
#ifndef _ERRORDEAL_H_
#define _ERRORDEAL_H_
#include "Var_Info.h"
namespace compiler{
	void error(int n, int pos); // �Ա��Ϊn�Ĵ�����д�����,���������Ϣ
	void fatal(int n, int pos); // �Ա��Ϊn��ջ���������д���
}
#endif
