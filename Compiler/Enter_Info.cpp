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
	void enterMidCode(int op, std::string z, std::string x, std::string y, int isstart){
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

	void enterMidOptCode(int op, std::string z, std::string x, std::string y, int isstart) {
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

	void enterMipsCode(int op, std::string z, std::string x, std::string y, int offset) {
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

	void enterMipsOptCode(int op, std::string z, std::string x, std::string y, int offset) {
		if (MipsTableOpt.mpc == MIPSCODEMAX) {
			fatal(5, lc);
		}
		MipsTableOpt.mipscodeArray[MipsTableOpt.mpc].op = op;
		MipsTableOpt.mipscodeArray[MipsTableOpt.mpc].z = z;
		MipsTableOpt.mipscodeArray[MipsTableOpt.mpc].x = x;
		MipsTableOpt.mipscodeArray[MipsTableOpt.mpc].y = y;
		MipsTableOpt.mipscodeArray[MipsTableOpt.mpc].offset = offset;
		MipsTableOpt.mpc++;
	}



	void printTab() {
		std::fstream tab_out_file;
		tab_out_file.open("tab_out.txt", std::fstream::out);
		tab_out_file <<"index\t" <<"name\t" << "obj\t\t" << "type\t" << "adr\t" << "ref\t" << "link\t" << "lev\t" << std::endl;
	for (int i = 0; i < tab.t; i++) {
			tab_out_file <<i<<"\t" <<tab.tabArray[i].name<<"\t"<< objecttyp_info[tab.tabArray[i].obj]<<"\t"<<types_info[tab.tabArray[i].type]<<"\t"
				<<tab.tabArray[i].adr<<"\t"<<tab.tabArray[i].ref<<"\t"<<tab.tabArray[i].link<<"\t"
				<<tab.tabArray[i].lev<<"\t"<<std::endl;
		}
	}

	void printATab() {
		std::fstream atab_out_file;
		atab_out_file.open("atab_out.txt", std::fstream::out);
		atab_out_file <<"index\t"<< "eltyp\t" << "high\t" << std::endl;
		for (int i = 0; i < atab.a; i++) {
			atab_out_file << i << "\t" << types_info[atab.atabArray[i].eltyp] << "\t" << atab.atabArray[i].high << "\t" << std::endl;
		}
	}

	void printBTab() {
		std::fstream btab_out_file;
		btab_out_file.open("btab_out.txt", std::fstream::out);
		btab_out_file << "index\t" << "last\t" << "lastpar\t" << "kind\t" << "paranum\t" << "varsize\t" << "t_varnum\t"<<std::endl;
		for (int i = 0; i < btab.b; i++) {
			btab_out_file << i << "\t" << btab.btabArray[i].last << "\t" << btab.btabArray[i].lastpar << "\t" << types_info[btab.btabArray[i].kind] << "\t"
				<< btab.btabArray[i].paranum << "\t" << btab.btabArray[i].varsize << "\t" << btab.btabArray[i].t_varnum<<"\t"<<std::endl;

		}
	}

	void printTMap() {
		std::fstream tmap_out_file;
		auto iter = T_Map.begin();
		tmap_out_file.open("tmap_out.txt", std::fstream::out);
		tmap_out_file << "key\t" << "value-inReg\t" << "value-r_addr\t" << "value-stack_addr\t" << std::endl;
		while (iter != T_Map.end()) {
			tmap_out_file << iter->first << "\t" << iter->second.inReg << "\t" << iter->second.r_addr << "\t" << iter->second.stack_addr << "\t" << std::endl;
			iter++;
		}
	}

	void printMidCode() {
		std::fstream MidCode_file;
		MidCode_file.open("midcode_out.txt", std::fstream::out);
		for (int i = 0; i < MidCodeT.mdc; i++) {
			MidCode_file << MidOpKind[MidCodeT.midcodeArray[i].op] << "\t" << MidCodeT.midcodeArray[i].z << " " << MidCodeT.midcodeArray[i].x
				<< " " << MidCodeT.midcodeArray[i].y  << "\t"<< MidCodeT.midcodeArray[i].isstart<< std::endl;
		}
	}
}
