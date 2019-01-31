/*
	��¼ȫ����Ϣ��
*/
#include "Enter_Info.h"
namespace compiler{
	bool enter(std::string name, objecttyp obj, types typ){
		int j, l;
		if(tab.t == TMAX){
			fatal(1,lc);
		}
		tab.tabArray[0].name = name;
		j = btab.btabArray[display[level]].last;
		l = j;
		while (tab.tabArray[j].name != name) {
			j = tab.tabArray[j].link;
		}
		if(j != 0){
			error(24,lc);
			return false;

		}else{
		    if(level==1){//�ģ�����ʹ�ú����������ڲ������������ظ�
                j=btab.btabArray[display[0]].last;//�ģ�����ʹ�ú����������ڲ������������ظ�
                if((tab.tabArray[j].name==name)&&(tab.tabArray[j].obj==function)){//�ģ�����ʹ�ú����������ڲ������������ظ�
                    error(24,lc);//�ģ�����ʹ�ú����������ڲ������������ظ�
                    return false;//�ģ�����ʹ�ú����������ڲ������������ظ�
                }//�ģ�����ʹ�ú����������ڲ������������ظ�
		    }//�ģ�����ʹ�ú����������ڲ������������ظ�
			tab.tabArray[tab.t].name = name;
			tab.tabArray[tab.t].obj = obj;
			tab.tabArray[tab.t].type = typ;
			tab.tabArray[tab.t].ref = 0;
			tab.tabArray[tab.t].adr = 0;
			tab.tabArray[tab.t].lev = level;
			tab.tabArray[tab.t].link = l;

			btab.btabArray[display[level]].last = tab.t;
			tab.t++;
            return true;
		}
	}

	//�ǼǱ��� / ������Ϣ, ����posΪ�ñ���/������tab���е�λ��, adrΪ��������Ե�ַ���߳�����ֵ
	void enterVar(int pos, int adr) {
		tab.tabArray[pos].adr = adr;
	}

	//�ڶ���array[intcon ֮�����, t_pos�Ǹñ�ʶ����tab���е�λ��
	void enterArray(int t_pos, types typ, int high) {
		if (atab.a == AMAX) {
			fatal(3, lc);
		}
		tab.tabArray[t_pos].ref = atab.a;//tab��ref��ָ��atab�Ǽǵ�ַ
		atab.atabArray[atab.a].eltyp = typ;
		atab.atabArray[atab.a].high = high;
		atab.a++;
	}


	//�ڶ�����������ĵ�һ�������ŵ�ʱ�����, t_pos�Ǹñ�ʶ����tab���е�λ��
	void enterFunc(int t_pos, types typ){
		if(btab.b == BMAX){
			fatal(2,lc);
		}
		tab.tabArray[t_pos].ref = btab.b;
		btab.btabArray[btab.b].kind = typ;
		btab.b++;
	}

	//isstart > 0 ��ҪΪ��ڵ�ַ�� ����isstart = 2 ���� mian������ڱ�ǩ, isstart = 3���������÷��ص�ַ, isstart = 4��ʾ��main������ʼλ��
	void Enter_Mid_List(int op, std::string z, std::string x, std::string y, int isstart){
		if (MidCodeT.mdc == MIDCODEMAX) {
			fatal(4,lc);
		}

		MidCodeT.midcodeArray[MidCodeT.mdc].op = op;
		MidCodeT.midcodeArray[MidCodeT.mdc].z = z;
		MidCodeT.midcodeArray[MidCodeT.mdc].x = x;
		MidCodeT.midcodeArray[MidCodeT.mdc].y = y;
		MidCodeT.midcodeArray[MidCodeT.mdc].isstart = isstart;
		MidCodeT.mdc++;
	}

	void Enter_Mid_Opt_List(int op, std::string z, std::string x, std::string y, int isstart) {
		if (MidCodeOptT.mdc == MIDCODEMAX) {
			fatal(6, lc);
		}
		MidCodeOptT.midcodeArray[MidCodeOptT.mdc].op = op;
		MidCodeOptT.midcodeArray[MidCodeOptT.mdc].z = z;
		MidCodeOptT.midcodeArray[MidCodeOptT.mdc].x = x;
		MidCodeOptT.midcodeArray[MidCodeOptT.mdc].y = y;
		MidCodeOptT.midcodeArray[MidCodeOptT.mdc].isstart = isstart;
		MidCodeOptT.mdc++;
	}

	void Enter_Mips_List(int op, std::string z, std::string x, std::string y, int offset) {
		if (MipsTable.mpc == MIPSCODEMAX) {
			fatal(5, lc);
		}
		MipsTable.mipscodeArray[MipsTable.mpc].op = op;
		MipsTable.mipscodeArray[MipsTable.mpc].z = z;
		MipsTable.mipscodeArray[MipsTable.mpc].x = x;
		MipsTable.mipscodeArray[MipsTable.mpc].y = y;
		MipsTable.mipscodeArray[MipsTable.mpc].offset = offset;
		MipsTable.mpc++;
	}

}
