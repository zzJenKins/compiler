/*
	�﷨�������, ��Դ����ת��Ϊ��Ԫʽ�м�ָ��
*/

//ע��������Ĵ�������ж�
#include <iostream>
#include "Block_Deal.h"
#include<set>
namespace compiler{
        int Deal_Expression_kind_check = 0;//���ʽ���ͼ���ʶ
	int temp_num = 0;	//������ʱ����ʱʹ��
	int return_flag = 0; //��ÿ�ν��뺯�������ʱ������Ϊ0,��return���������Ϊ1��2,����1����û�з���ֵ,2�����з���ֵ
	std::fstream judge_file;					//Ȼ���ں����������ʱ���м��,void��������Ϊ0��1,int����Ϊ2, char����Ϊ3
    symbol	Block_Start_Sys[] = {constsy, voidsy, intsy, charsy};	//4��
    symbol  Statement_Start_Sys[] = { semicolon, ifsy, whilesy,lcbrack, ident, scanfsy, printfsy, switchsy, returnsy}; //9��
	symbol  Statement_End_Sys[] = { semicolon, rparent, rcbrack }; //3��
	symbol  Deal_Expression_Start_Sys[] = {ident, plus, minus, intcon, charcon, lparent}; //6��

	symbol	Follow_Deal_Expression_Sys[] = { semicolon, rparent, rsbrack, lss, leq, gtr, geq, neq, eql, comma};	//10��,���һ���ڴ������������ʱ�����

	symbol  Deal_Factor_Start_Sys[] = { ident, plus, minus, intcon, charcon, lparent }; //6��
	symbol  Deal_Factor_End_Sys[] = { ident, rsbrack, intcon, charcon, rparent };//5��
//���ֺ�������
	std::string Deal_reUseFunc();
	std::string Deal_Expression(bool& is_char);
	std::string Deal_Factor(bool& is_char);
	void statement();
    //����Դ����ֱ��ȡ�����ַ����ڸ����ķ��ż�st, nΪ�����С
	void Skip_Read(symbol st[], int n) {
		while (!find_sy(st, n)) insymbol();
	}
	//������ʱ����
	std::string Creat_Tempvar(){
		std::string tmp = int_to_string(temp_num);
		std::string t_var = "#" + tmp;
		var_info t_info;
		temp_num++;
		t_info.inReg = false;
		t_info.r_addr = -1;
		t_info.stack_addr = -1;
		t_info.isTFirst = true;
		T_Map[t_var] = t_info;
		return t_var;
	}

	//���ɱ�ǩ,����l_kindΪ��ǩ����, ��s�Ļ��������ӱ�ǩ  pcΪ��ǰ�м�ָ���λ��(MidCodeT��ʱ������ֵ),����labelʱʹ��
	std::string Creat_Label(std::string s, int l_kind, int pc){
		std::string label;
		switch(l_kind){
			case 0:	//��������ʼ��ǩ
				label =  s + "_begin_";
				break;
			case 1:	//�����������ǩ
				label =  s + "_end_";
				break;
			case 2: //if��while����ת
				label = "_" + int_to_string(pc) + "_label_";
				break;
			case 3: //switch_end��ǩ
				label = "switch_" + int_to_string(pc) + "_end_";
				break;
			case 4: //call����֮�󷵻ص�ַ��ǩ,���ڱ��������
				label = "call_" + int_to_string(pc) + "_";
				break;
			default:
				label = "ERROR";
				break;
		}
		return label;
	}

	types Sy_Trans_Types(symbol sym){
		switch(sym){
			case intsy:
				return ints;
			case charsy:
				return chars;
			default:
				return notyp;
		}
	}

	//<����˵��> <��������> ����
	void Deal_Const(){
	    bool e=false;
		judge_file<<"This is a const statement"<<std::endl;
		types tmp_typ;
		insymbol();
		if ((sy != intsy) && (sy != charsy)) {
			error(26,lc);
			Skip_Read(Block_Start_Sys, 4);
			return;
		}
		tmp_typ = Sy_Trans_Types(sy);
		do {
			insymbol();
			if (sy != ident) {
				error(11,lc);
				Skip_Read(Block_Start_Sys, 4);
				return;
			}
			e=enter(id, constant, tmp_typ);//�������Ǽ����
			insymbol();

			if (sy != assign) {
				error(20,lc);
				Skip_Read(Block_Start_Sys, 4);
				return;
			}

			insymbol();	//�������ǵȺź���ĵ���

			if ((sy == plus) || (sy == minus) || (sy == intcon)) {
				//����������֧
				if (tmp_typ != ints) {
					error(27,lc);
					Skip_Read(Block_Start_Sys, 4);
					return;
				}
				int flag = 1;
				if ((sy == plus) || (sy == minus)) {
					if (sy == minus) flag = -1;
					insymbol();
				}
				if (sy != intcon) {
					error(27, lc);
					Skip_Read(Block_Start_Sys, 4);
					return;
				}
				inum = inum * flag;
				if(e){
                    enterVar(tab.t - 1, inum);//��������ֵ��¼��tab���޸�adr��
				}

			}
			else if (sy == charcon) {
				if (tmp_typ != chars) {
					error(27,lc);
					Skip_Read(Block_Start_Sys, 4);
					return;
				}
				if(e){
                   enterVar(tab.t - 1, inum);//��������ֵ��¼��tab���޸�adr��
				}

			}
			else {
				error(27, lc);
				Skip_Read(Block_Start_Sys, 4);
				return;
			}

			insymbol();
		} while (sy == comma);
		if (sy != semicolon) {
			error(19, lc-1);
			return;
		}
		insymbol();//��ȡ�ֺź���һ���ַ�
	}
    //<����˵��> <��������> ��������ʱ����nΪ���е�һ����ʶ��������ջ����Ե�ַ
	void Deal_Varible(types typ, int &n){
	    bool e=false;

		judge_file << "This is var_declare statement" << std::endl;
		//����ʱidΪ int/char �����һ����ʶ����sy == comma || sy == semicolon || sy = lsbrack
		if (sy == lsbrack) {//�����������
			e=enter(id, array, typ);//����tab���ָ����+1
			insymbol();
			if ((sy == intcon) && (inum > 0)) {
                if(e){
                    enterVar(tab.t - 1, n);	//������ĵ�һ��Ԫ��������ջS�д�ŵ���Ե�ַ����adr��
                    n += inum;	//inum���ռ䶼�Ǹ�����ģ��޸Ĳ���n��ֵ
                    enterArray(tab.t - 1, typ, inum);//��atab��������������Ϣ
				}

				insymbol();
				if (sy != rsbrack) {
					error(16, lc);
				}
				else {
					insymbol();
				}
			}
			else {
				error(30, lc);
				Skip_Read(Block_Start_Sys,4);
				return;
			}
			if ((sy != comma) && (sy != semicolon)) {
				error(28, lc);
				Skip_Read(Block_Start_Sys, 4);
				return;
			}
		}
		else {
			e=enter(id, variable, typ);
			if(e){
               enterVar(tab.t - 1, n++);
			}
			if ((sy != comma) && (sy != semicolon)) {
				error(28, lc);
				Skip_Read(Block_Start_Sys, 4);
				return;
			}
		}
		//��ֹ������ sy == comma || sy == semicolon
		while (sy == comma) {
			insymbol();
			if (sy != ident) {	//�Ǳ�ʶ��,������һ�д�����һ��
				error(11,lc);
				Skip_Read(Block_Start_Sys, 4);
				return;
			}
			insymbol();
			if ((sy != lsbrack) && (sy != comma) && (sy != semicolon)) {
				error(28, lc);
				Skip_Read(Block_Start_Sys, 4);
				return;
			}
			if (sy == lsbrack) {	//��if������sy=comma or sy = semicolon
				e=enter(id, array, typ);
				insymbol();
				if ((sy == intcon) && (inum != 0)) {
                    if(e){
                        enterVar(tab.t - 1, n);	//������ĵ�һ��Ԫ����Ե�ַ�������
                        n += inum;	//inum���ռ䶼�Ǹ������
                        enterArray(tab.t - 1, typ, inum);
                    }
					insymbol();
					if (sy != rsbrack) {
						error(16, lc);
					}
					else {
						insymbol();
					}
				}
				else {
					error(30, lc);
					Skip_Read(Block_Start_Sys, 4);
					return;
				}
				if ((sy != comma) && (sy != semicolon)) {
					error(28, lc);
					Skip_Read(Block_Start_Sys, 4);
					return;
				}
			}
			else {
				e=enter(id, variable, typ);
				if(e){
                    enterVar(tab.t - 1, n++);
				}

			}
		}
		if (sy != semicolon) {
			error(19,lc);
			return;
		}
		insymbol();//��ȡ�ֺ�֮���һ������
	}
	//<������>
	// ����ֵΪ�����ĸ������á�pv_addrΪĿǰ�������ں�������������Ե�ַ,����ֵΪ���һ��������tab��ĵ�¼λ��
	int Deal_Para(int& para_num, int& pv_addr) {
	    bool e=false;
		//����ʱsy == intsy || sy == charsy
		if ((sy != intsy) && (sy != charsy)) {
			std::cout << "There is a bug in blockdeal--Deal_Para, sy must be intsy or charsy" << std::endl;
		}

		judge_file << "This is a Deal_Para statement" << std::endl;
		types tmp_typ = Sy_Trans_Types(sy);
		insymbol();
		if (sy != ident) {
			error(11, lc);
		}
		else {
			//sy == ident
			e=enter(id, variable, tmp_typ);//��tab�еǼǸò�������Ϣ
			if(e){
                enterVar(tab.t-1,pv_addr++);//�Ǽǵ�ǰ������Sջ��ƫ�ƣ�������pv_addr��ָ��ֵ
                para_num++;//���Ӳ�������
                Enter_Mid_List(15," ",types_info[tmp_typ],id,0);// para	x , y  ��������ʱ���βΣ�xΪ���ͣ�yΪ�β���
			}
			insymbol();
		}
		symbol tmp_sys[] = { comma, rparent };
		Skip_Read(tmp_sys, 2);
		while (sy == comma) {
			insymbol();
			if ((sy != intsy) && (sy != charsy)) {
				error(31,lc);
				Skip_Read(tmp_sys, 2);//�����ҵ�sy==comma|rparent�������µ�һ��ѭ�����
				continue;
			}
			//sy==int|char
			tmp_typ = Sy_Trans_Types(sy);
			insymbol();
			if (sy != ident) {	//������Ǳ�ʶ�������������¿���һ��ѭ��
				error(31, lc);
				Skip_Read(tmp_sys, 2);
				continue;
			}
			//����˴�sy == ident
			e=enter(id, variable, tmp_typ);//��tab�ǼǸñ��������ƺ����͵���Ϣ
			if(e){
                enterVar(tab.t - 1, pv_addr++);//��adr��д����ƫ��
                para_num++;//���Ӳ�������
                Enter_Mid_List(15, " ", types_info[tmp_typ], id, 0);//para x,y
			}

			insymbol();
			if ((sy != comma) && (sy != rparent)) {//��ǰ����һ���ַ�
				error(31, lc);
				Skip_Read(tmp_sys, 2);
			}
		}

		//����˴���������sy == rparent��Ҳ�������������
		if (sy != rparent) {
			std::cout << "There is a bug in blockdeal--Deal_Para, sy must be rparent" << std::endl;
		}
		insymbol();
		//��ȥ��ʱ��syΪ')'����һ������
		return tab.t - 1;
	}
	//<����>
	//����ֵΪ������
	std::string Deal_Factor(bool& is_char) {
	    bool e=false;
		//������ʱ,syΪ Deal_Factor_Start_Sys[]�е�Ԫ��
		std::string z, y, x;
		bool char_tmp = false;
		int pos, tmp;	//tmp ��minus��ʹ��
		symbol tmp_sys[] = { rparent };

		switch (sy) {
		case ident:	//��ʶ������ʶ��[���ʽ]���з���ֵ�����������
			pos = find_in_tab(id);	//��tab���в�ѯ��ǰ��ʶ���Ի����Ϣ
			if (pos == 0) error(23, lc);//û���ҵ��ñ�ʶ��������

			if (pos != 0) {//�ҵ���ʶ������ȡ��Ϣ������ֵ�����ַ�������ʽ
				if (tab.tabArray[pos].type == chars) is_char = true;//��ʶ����һ���ַ�
				switch (tab.tabArray[pos].obj) {
				case constant:	//����ֱ�ӷ�����ֵ
					return int_to_string(tab.tabArray[pos].adr);
				case variable:	//�������ر�����
				     z = Creat_Tempvar();
                Enter_Mid_List(4, z, " ", id, 0);
                return z;
					//return id;
				case array:		//��ʶ��[���ʽ]
				    if(Deal_Expression_kind_check==1 && is_char ==true){
                        is_char = false;
                        Deal_Expression_kind_check=0;
					}
					y = id;//��¼��������
					z = Creat_Tempvar();//����һ���µ���ʱ������
					insymbol();
					if (sy != lsbrack) {
						error(15, lc);//����ȱ��������
						Skip_Read(Deal_Factor_End_Sys, 5);
					}
					insymbol();
					Skip_Read(Deal_Expression_Start_Sys, 6);
					x = Deal_Expression(char_tmp);	//���������ŵı��ʽ,����ʱsyΪFollow_Deal_Expression_Sys[]�е�Ԫ��
					if (char_tmp) {	//tmpΪ��ʾΪ�ַ�����
						error(49, lc);//���������������е��±겻��Ϊchar��
					}
					else {
						if (string_is_num(x)) {//�ַ��������������ִ�
							if (string_to_int(x) >= atab.atabArray[tab.tabArray[pos].ref].high) {//�±�Ĵ�С�����������Ͻ�
								error(49, lc);//����
							}
						}
					}
					if (sy != rsbrack) {
						error(16, lc);
						Skip_Read(Deal_Factor_End_Sys, 5);
					}

					Enter_Mid_List(4, z, x, y, 0);	//z = y[x]
													//��ʱsyΪrsbrack
					return z;//�����������ʾ����������z������ʱ����ı���
				case function://�Ǻ���
					if (tab.tabArray[pos].type == notyp) {	//�޷���ֵ�������ã�����
						error(34, lc);
						Skip_Read(tmp_sys,1);
						return "0";
					}
					if (tab.tabArray[pos].type == chars) is_char = true;//�����ķ���ֵ��һ���ַ�char
					else is_char = false;
					if(Deal_Expression_kind_check==1 && is_char ==true){
                        is_char = false;
                        Deal_Expression_kind_check=0;
					}
					//����Deal_reUseFuncʱ,sy == identsy(������)
					z = Deal_reUseFunc();//�з���ֵ�ĺ����������ķ���ֵ�Ǹú����ķ���ֵ

					//��ʱ syΪ rparent
					return z;	//�����з���ֵ�����ĵ������ĸ�ֵ���������, ��z = *RET
				default:
					std::cout << "There is a bug in blockdeal--Deal_Factor, switch-ident" << std::endl;
					return "0";
				}
			}
			else {	//��ʶ��δ����
				return "0";
			}
			//���򲻿������е�����
			std::cout << "There is a bug in blockdeal-Deal_Factor, switch-ident" << std::endl;
		case plus:	// ����: +��ͷ��������޷�������
			insymbol();
			if (sy == intcon) {
				return int_to_string(inum);//������ת��Ϊ�ַ���������
			}
			else {
				error(33, lc);
				Skip_Read(Deal_Factor_End_Sys, 5);
				return "0";
			}
			//���򲻿������е�����
			std::cout << "There is a bug in blockdeal-Deal_Factor, switch-plus" << std::endl;
		case minus:	// ����: -��������޷�������
			tmp = -1;
			insymbol();
			if (sy == intcon) {
				inum = inum * tmp;//������ŵ�����
				return int_to_string(inum);//������ת��Ϊ�ַ���������
			}
			else {
				error(33, lc);
				Skip_Read(Deal_Factor_End_Sys, 5);
				return "0";
			}
			//���򲻿������е�����
			std::cout << "There is a bug in blockdeal-Deal_Factor, switch-minus" << std::endl;
		case intcon: // ����������+/-��ͷ��ֱ�����޷�������
			is_char = false;
			return int_to_string(inum);
		case charcon:	//�ַ�
			is_char = true;
			return int_to_string(inum);//�����ص����ַ���ֵ
		case lparent://��ͷ��(,˵���Ǳ��ʽ
            Deal_Expression_kind_check=1;
			insymbol();
			if (!find_sy(Deal_Expression_Start_Sys, 6)) {
				error(33, lc);
				Skip_Read(Deal_Expression_Start_Sys, 6);
			}
			//Deal_Expression_lparent_flag=true;
			return Deal_Expression(is_char);//���ر��ʽ��ֵ
		default:
			std::cout << "There is a bug in blockdeal--Deal_Factor, switch-default" << std::endl;
			break;
		}

		//���򲻿������е�����
		std::cout << "There is a bug in blockdeal-Deal_Factor, end" << std::endl;
		return " ";
		//������ʱ,syΪ Deal_Factor_End_Sys[]�е�Ԫ��
	}

	//<��>
	//����ֵΪ���õ������յ�z������
	std::string Deal_Item(int minus_flag, bool& is_char){
		//����ʱsyΪDeal_Factor_Start_Sys[]�е�Ԫ��
		std::string z, x, y;
		//std::cout << is_char << std::endl;
		//std::cout << "Deal_Item�����yǰ"<<is_char << std::endl;
		x = Deal_Factor(is_char);//xΪ��һ������
		//std::cout << "Deal_Item�����y��"<<is_char << std::endl;
		//std::cout << is_char << std::endl;
	//	Deal_Expression_lparent_flag=false;
		if (minus_flag == 1) {		//���ʽ�ĵ�һ����Ż�����
			z = Creat_Tempvar();//������ʱ�������x���෴���Ľ��
			y = x;
			x = "0";
			Enter_Mid_List(1, z, x, y, 0);//z=0-x
			x = z;//x=z
		}
		//�뿪Deal_Factorʱ, syΪDeal_Factor_End_Sys[]�е�һ��
		insymbol();
		while ((sy == mult) || (sy == rdiv)) {
			is_char = false;		// �г˷������,�϶������ַ�������
			int op;
			bool no_use;
			if (sy == mult) op = 2;
			else if (sy == rdiv) op = 3;
			else std::cout << "There is a bug in blockdeal--Deal_Item, while" << std::endl;
			insymbol();
			Skip_Read(Deal_Factor_Start_Sys, 6);
			y = Deal_Factor(no_use);
			//Deal_Expression_lparent_flag=false;
			z = Creat_Tempvar();//������ʱ����z���洢x��y�������ֵ
			if ((op == 3) && string_is_num(y) && (string_to_int(y) == 0)) {//����ǳ������㣬y����Ϊ0
				error(44, lc);
			}
			Enter_Mid_List(op, z, x, y, 0);//���˳������㣬�õ��Ľ��������ʱ�Ĵ���z��
			x = z;

			insymbol();
		}
		return x;
		//����ʱ sy Ϊ�����һ������
	}

	//<���ʽ>
	//����ֵΪ���յ�һ����Ԫʽ���ʽ��zֵ, ����: ���ʽ�������Ƿ�Ϊchar����
	std::string Deal_Expression(bool& is_char){
		//������ʽʱsyΪDeal_Expression_Start_Sys[]�е�Ԫ��

		//std::cout << "This is an Deal_Expression statement" << std::endl;
		judge_file << "This is an Deal_Expression statement" << std::endl;

		int minus_flag = 0;
		std::string z, x, y;
		if ((sy == plus) || (sy == minus)) {//������ʽ��ͷ�������ڵ�һ�����+/-
			if (sy == minus) {
				minus_flag = 1;
			}
			insymbol();
		}

		Skip_Read(Deal_Factor_Start_Sys, 6);
		//std::cout << "Deal_Expression�����yǰ"<<is_char << std::endl;
		//std::cout<<sy<<std::endl;
		x = Deal_Item(minus_flag, is_char);//�������
		//std::cout << "Deal_Expression�����y��"<<is_char << std::endl;
		//std::cout << "Deal_Expression�����y��flag"<<Deal_Expression_kind_check << std::endl;
		if(Deal_Expression_kind_check==1){
            is_char = false;
            Deal_Expression_kind_check = 0;
		}
       // std::cout << is_char << std::endl;
		//����ʱsyΪ �����ĵ�һ������
	//	if(Deal_Expression_lparent_flag){
       //     is_char=false;
       //     Deal_Expression_lparent_flag=false;
		//}
		symbol tmp_sys[] = { semicolon, rparent, rsbrack ,plus, minus, lss, leq, gtr, geq, neq, eql, comma};	//��ʱ syӦ��Ϊ follow[Deal_Expression] or �ӷ������
		if (!find_sy(tmp_sys, 12)) {
			error(33, lc);
			//std::cout<<"1"<<std::endl;
			Skip_Read(tmp_sys, 11);
		}
		while ((sy == plus) || (sy == minus)) {
			is_char = false;	// �мӷ��������϶������ַ���
			int op;
			bool no_use;
			if (sy == plus) op = 0;
			else if (sy == minus) op = 1;
			else std::cout << "There is a bug in blockdeal--Deal_Expression, while" << std::endl;
			insymbol();
			y = Deal_Item(0,no_use);//�����Deal_Item��û��-
			Deal_Expression_kind_check = 0;
		//	Deal_Expression_lparent_flag=false;
			//syΪ������һ������,���Բ���insymbol()
			z = Creat_Tempvar();//������ʱ����z

			Enter_Mid_List(op, z, x, y, 0);//��x��y������õ�����ֵ��ֵ���������ʱ����z��
			x = z;//x=z
		}//ѭ���õ����ʽ��ֵ
		if (!find_sy(Follow_Deal_Expression_Sys, 10)) {
			error(33,lc);
			Skip_Read(Follow_Deal_Expression_Sys, 10);
		}
		return x;
		//��Deal_Expressionʱ,syΪFollow_Deal_Expression_Sys[]�е�Ԫ��
	}

	//<����>
	//���ظ���תָ�����ڵ��м�����е�λ��,�Ա����������ת�ı�ǩ
	int Deal_Condition(){			// kind = 0, if �����ĵ���; kind = 1, while�����ĵ���, ������ת��������ͬ
		// ����ʱ, syΪ�����ź�ĵ�һ������
		//std::cout << "This is a condition statement" << std::endl;
		judge_file << "This is a condition statement" << std::endl;
		std::string z, x, y;
		int op;
		bool no_use_x=false;
		bool no_use_y=false;
		symbol relation_op_sys[] = { lss, leq, gtr, geq, neq, eql };	//6��

		if (!find_sy(Deal_Expression_Start_Sys, 6)) {
			error(33, lc);
			Skip_Read(Deal_Expression_Start_Sys, 6);
		}

		x = Deal_Expression(no_use_x);	//������ʽ����
		//��Deal_Expressionʱ,syΪFollow_Deal_Expression_Sys[]�е�Ԫ��

		if (find_sy(relation_op_sys, 6)) {
				//�й�ϵ�����,ע��op��ȡֵ, if��while�ǲ�������������ת
				switch (sy) {
				case lss://<  ��ζ��x >= y��ʱ��ִ��if�ڵ����,������ת
					op = 8;
					break;
				case leq://<=  x > y
					op = 9;
					break;
				case gtr://>  x<=y
					op = 10;
					break;
				case geq://>=  x<y
					op = 11;
					break;
				case neq:// !=  x==y ��ת
					op = 6;
					break;
				case eql://==  x!=y
					op = 7;
					break;
				default:
					std::cout << "There is a bug in blockdeal--Deal_Condition, switch-default" << std::endl;
					break;
				}
			insymbol();
			Skip_Read(Deal_Expression_Start_Sys, 6);
			//std::cout <<"yǰ"<<no_use_y<< std::endl;
			y = Deal_Expression(no_use_y);	//������ʽ����
			//std::cout <<"y��"<<no_use_y<< std::endl;
			//std::cout <<no_use_x<<no_use_y<< std::endl;

			if(no_use_x!=no_use_y){
                error(51,lc);
			}
			if((no_use_x)||(no_use_y)){
                error(53,lc);
			}
		}
		else {
            if(no_use_x){
                error(51,lc);
            }
			op = 6; //����һ�����ʽ, ���� x == 0��ʱ����ת
			y = "0";
		}
        Enter_Mid_List(op, " ", x, y, 0);//����������תָ���ת���ı�ǩ��ʱ�հף��Ⱥ����
		//��ʱsy����Follow_Deal_Expression_Sys[]�е�Ԫ��
		if (sy != rparent) {
			error(50, lc);
		}
		return MidCodeT.mdc - 1; //���ش˴����ɵ�mips��תָ�����ڵ�λ��,����֮�����label��
		// ��ȥʱ, sy == rparent
	}

	//<�������>
	void Deal_ifStatement(){
		//����ʱ sy == ifsy
		//std::cout<<"This is a if statement"<<std::endl;
		judge_file<< "This is a if statement" << std::endl;
		int code_pos = 0;
		std::string j_label;

		insymbol();
		if (sy != lparent) {
			error(13, lc);
			Skip_Read(Statement_Start_Sys, 9);
			return;
		}
		insymbol();
		if (sy == rparent) {
			//˵��û��������䡣����
			error(21, lc);
		}
		else {
			code_pos = Deal_Condition();//code_pos��ȱʧlabel��mips����λ��
			//����ʱ sy == rparent
		}

		if (sy != rparent) std::cout << "There is a bug in blockdeal--Deal_ifStatement, sy must be rparent" << std::endl;

		insymbol();

		if (!find_sy(Statement_Start_Sys, 9)) {
			error(32, lc);
			Skip_Read(Statement_Start_Sys, 9);
		}

		statement();

		j_label = Creat_Label(" ", 2, MidCodeT.mdc);//����������λ������һ��label��ǩ����ǩ���ּ�¼��j_label
		Enter_Mid_List(13, " ", " ",j_label, 1);//������֮�����ñ�ǩj_label
		MidCodeT.midcodeArray[code_pos].z = j_label;//����label,������if�����Ļ�ֱ����ת�����λ�ã���ִ������ת���ɵ�mipsָ��
		//��ȥʱ sy Ϊ Statement_End_Sys�е�Ԫ��
	}

	//ѭ�����
	void Deal_whileStatement(){
	//������sy��while
        judge_file<< "This is a while statement" << std::endl;
		int code_pos = 0;
		std::string label_before_while;
		std::string while_end_label;

		insymbol();//��ȡwhile�����һ���ַ�
		if (sy != lparent) {//�������(
			error(13, lc);//����
			Skip_Read(Statement_Start_Sys, 9);
			return;
		}
		insymbol();
		if (sy == rparent) {
			//˵��û��������䡣����
			error(21, lc);
		}
		else {
            label_before_while=Creat_Label(" ", 2, MidCodeT.mdc);//��while�������֮ǰ����һ��label��ǩ����ǩ���ּ�¼��label_before_while
            Enter_Mid_List(13, " ", " ",label_before_while, 1);//��while�������֮ǰ���ñ�ǩlabel_before_while
			code_pos = Deal_Condition();//code_pos��ȱʧlabel��mips����λ��
			//����ʱ sy == rparent
			if (sy != rparent) std::cout << "There is a bug in blockdeal--Deal_ifStatement, sy must be rparent" << std::endl;
            insymbol();
            if (!find_sy(Statement_Start_Sys, 9)) {
                error(32, lc);
                Skip_Read(Statement_Start_Sys, 9);
            }
			statement();
			Enter_Mid_List(5, label_before_while," ", " ", 1);//����ִ�н�������������ת��while֮ǰ
			while_end_label=Creat_Label(" ", 2, MidCodeT.mdc);//��while��������ת���֮������һ��label��ǩ����ǩ���ּ�¼��while_end_label
            Enter_Mid_List(13, " ", " ",while_end_label, 1);//��while��������ת���֮������while_end_label
			MidCodeT.midcodeArray[code_pos].z = while_end_label;//����label,������while�����Ļ�ֱ����ת�����λ�ã���ִ������ת���ɵ�mipsָ��
		//��ȥʱ sy Ϊ Statement_End_Sys�е�Ԫ��
		}

	}

	//<��ֵ���>
	//array_flag = 1, ��ʾ��ǰident��������; 0����Ϊ����
	void Deal_assignStatement(int array_flag){
		//����ʱsy == ident
		//std::cout << "This is a assign statement" << std::endl;
		judge_file << "This is a assign statement" << std::endl;
		std::string z, y, x = " ";
		symbol tmp_sys[] = { semicolon };//��β�Ƿֺ�
		int op;
		bool is_char = false;
		int z_pos;		//z�ڷ��ű��е�λ��
		z = id;//���ҵ�ǰ��ʶ����tab�е���Ϣ
		z_pos = find_in_tab(z);
		if (z_pos == 0) error(23, lc);
		if (array_flag == 1) {//������ĸ�ֵ z[x]=y;
			op = 20;//z[x]=y;
			insymbol();
			if (sy != lsbrack) {
				error(15, lc);
				Skip_Read(tmp_sys, 1);
				return;
			}
			insymbol();
			Skip_Read(Deal_Expression_Start_Sys, 6);
			bool no_use = false;
			x = Deal_Expression(no_use);	//���������ŵı��ʽ,����ʱsyΪFollow_Deal_Expression_Sys[]�е�Ԫ��
			if (no_use) {	//tmpΪ��ʾΪ�ַ����ͣ������±겻��Ϊ�ַ����ͣ��ʴ���
				//std::cout << "1" << std::endl;
				error(49, lc);
			}
			else {
				if (string_is_num(x)) {//���±�Ϊ��ֵ
					if (string_to_int(x) >= atab.atabArray[tab.tabArray[z_pos].ref].high) {//����±곬�����Ͻ�ҲҪ����
						//std::cout << "2" << std::endl;
						error(49, lc);
					}
				}
			}
			if (sy != rsbrack) {//֮����]
				error(16, lc);
				Skip_Read(tmp_sys, 1);
				return;
			}
		}
		else {
			op = 4;//��ͨ�ĸ�ֵ z=y

		}

		insymbol();
		if (sy != assign) {
			error(38, lc);
			Skip_Read(tmp_sys, 1);
			return;
		}
		insymbol();
		Skip_Read(Deal_Expression_Start_Sys, 6);
		y = Deal_Expression(is_char);

		if (tab.tabArray[z_pos].type == chars) {//���z��������char��y�����Ͳ���char ����
			if (!is_char) {
				error(46,lc - 1);
			}
		}
		if (tab.tabArray[z_pos].type == ints) {//���z��������char��y�����Ͳ���char ����
			if (is_char) {
				error(46,lc - 1);
			}
		}
		Enter_Mid_List(op, z, x, y, 0);//����ֵ����

		if (sy != semicolon) {//��β���Ƿֺ�
			error(19, lc);
			Skip_Read(tmp_sys, 1);
		}

		//����ʱ sy = semicolon
	}

	//<�з���ֵ�����������>
	//�漰����Ԫʽָ��  call, push, jal,  set call_label,  z = *RET,
	//����ֵΪ z = *RET ��z
	std::string Deal_reUseFunc(){
		//����ʱ sy == ident(������)
		//std::cout << "This is a Deal_reUseFunc" << std::endl;
		judge_file << "This is a Deal_reUseFunc" << std::endl;

		int id_pos = find_in_tab(id);	//id_posΪ�ú�������tab���е�λ��
		if(id_pos == 0) error(23, lc);
		int paranum = btab.btabArray[tab.tabArray[id_pos].ref].paranum;//����ʱ������Ŀ
		int paracount = 0;//����ʱ��������Ŀ
		std::string para, z;
		std::string func_name = id ;//������
		symbol tmp_sys[] = { rparent };

		std::map <int, types> para_typ_map;//����һ���±�Ͷ����������һһ��Ӧ��map
		tabe tmp = tab.tabArray[btab.btabArray[tab.tabArray[id_pos].ref].lastpar];//�ú�����������һ������
		for (int i = paranum - 1; i > -1; i--) {//���Ű����Ѳ�������������ŵ�map��
			para_typ_map[i] = tmp.type;
			tmp = tab.tabArray[tmp.link];
		}

		Enter_Mid_List(19," ", " ", func_name,0);	//callָ�����
		insymbol();//��ʼ�жϲ�����

		if (sy != lparent) {//û��(
			error(13, lc);
			Skip_Read(tmp_sys, 1);
			return " ";
		}
		insymbol();
		if (sy == rparent) { //�޲������õ����
			if (paracount != paranum) {//�������ʱ������Ŀ����0������
				error(36, lc);
				return " ";
			}
		}
		else if(find_sy(Deal_Expression_Start_Sys, 6)){//����˶�����
			bool is_char = false;
			para = Deal_Expression(is_char);//para��¼�����ʵ��
			//	std::cout << is_char << std::endl;
			//std::cout << types_info[para_typ_map[paracount]] << std::endl;
			if (is_char) {//ʵ�ʲ�����char
				if (para_typ_map[paracount] != chars) {//����ʱ����char������
					error(45, lc);
				}
			}
			else {//ʵ�ʲ�������ֵ
				if (para_typ_map[paracount] != ints) {//����ʱ����int������
					error(45, lc);
				}
			}

			Enter_Mid_List(18, " ", " ", para, 0);	//pushָ�� passpara y
			paracount++;//������������
			while (sy == comma) {//����˶�
				insymbol();
				is_char=false;
				if (!find_sy(Deal_Expression_Start_Sys, 6)) {
					error(33,lc);
					Skip_Read(tmp_sys, 1);
					return " ";
				}
				para = Deal_Expression(is_char);
					//std::cout << is_char << std::endl;
			//std::cout << types_info[para_typ_map[paracount]] << std::endl;
				if (is_char) {
					if (para_typ_map[paracount] != chars) {
						error(45, lc);
					}
				}
				else {
					if (para_typ_map[paracount] != ints) {
						error(45, lc);
					}
				}
				Enter_Mid_List(18, " ", " ", para, 0);	//pushָ��
				paracount++;
			}
			if (paracount != paranum) {//����Ĳ���������ʵ�ʴ���Ĳ���������һ�£�����
				error(36, lc);
				Skip_Read(tmp_sys, 1);
				return " ";
			}
			if (sy != rparent) {//�����������Ĳ���) ����
				error(14, lc);
				Skip_Read(tmp_sys, 1);
				return " ";
			}
		}
		else {
			Skip_Read(tmp_sys, 1);
			return " ";
		}

		if (sy != rparent) std::cout << "There is a bug in blockdeal--Deal_reUseFunc, sy must be rparent" << std::endl;
		Enter_Mid_List(21,func_name + "_begin_"," "," ", 0);		//jal��䣬��ת�����庯��ʱ��(func_name)_begin��ǩ
		z = Creat_Tempvar();//����һ����ʱ�����нӷ���ֵ

		Enter_Mid_List(13," "," ",Creat_Label(" ",4,MidCodeT.mdc),3);//?

		//�з���ֵ���ú�������
		Enter_Mid_List(4,z, " ", "*RET",0);				//z = *RET
		//��ȥʱsy == rparent
		return z;
	}

	//<�޷���ֵ�����������>
	//�漰����Ԫʽָ��  call, push, jal, set call_label
	void Deal_voUseFunc(){
		//����ʱ sy == ident(������)
		//std::cout << "This is a Deal_voUseFunc" << std::endl;
		judge_file << "This is a Deal_voUseFunc" << std::endl;

		int id_pos = find_in_tab(id);	//id_posΪ�ú�������tab���е�λ��
		if (id_pos == 0) error(23, lc);
		int paranum = btab.btabArray[tab.tabArray[id_pos].ref].paranum;
		int paracount = 0;
		std::string para, z;
		std::string func_name = id;
		symbol tmp_sys[] = { rparent };

		std::map <int, types> para_typ_map;
		tabe tmp = tab.tabArray[btab.btabArray[tab.tabArray[id_pos].ref].lastpar];
		for (int i = paranum - 1; i > -1; i--) {
			para_typ_map[i] = tmp.type;
			tmp = tab.tabArray[tmp.link];
		}

		Enter_Mid_List(19, " ", " ", func_name, 0);	//callָ�����
		insymbol();

		if (sy != lparent) {
			error(13, lc);
			Skip_Read(tmp_sys, 1);
			return;
		}
		insymbol();
		if (sy == rparent) { //�޲������õ����
			if (paracount != paranum) {
				error(36, lc);
				return;
			}
		}
		else if (find_sy(Deal_Expression_Start_Sys, 6)) {
			bool is_char = false;
			para = Deal_Expression(is_char);

			//std::cout << is_char << std::endl;
			//std::cout << types_info[para_typ_map[paracount]] << std::endl;
			if (is_char) {
				if (para_typ_map[paracount] != chars) {
					error(45, lc);
				}
			}
			else {
				if (para_typ_map[paracount] != ints) {
					error(45, lc);
				}
			}
			Enter_Mid_List(18, " ", " ", para, 0);	//pushָ��
			paracount++;
			while (sy == comma) {
                is_char=false;
				insymbol();
				if (!find_sy(Deal_Expression_Start_Sys, 6)) {
					error(33, lc);
					Skip_Read(tmp_sys, 1);
					return;
				}
				para = Deal_Expression(is_char);
				if (is_char) {
					if (para_typ_map[paracount] != chars) {
						error(45, lc);
					}
				}
				else {
					if (para_typ_map[paracount] != ints) {
						error(45, lc);
					}
				}
				Enter_Mid_List(18, " ", " ", para, 0);	//pushָ��
				paracount++;
			}
			if (paracount != paranum) {
				error(36, lc);
				Skip_Read(tmp_sys, 1);
				return;
			}
			if (sy != rparent) {
				error(14, lc);
				Skip_Read(tmp_sys, 1);
				return;
			}
		}
		else {
			Skip_Read(tmp_sys, 1);
			return ;
		}

		if (sy != rparent) std::cout << "There is a bug in blockdeal--Deal_reUseFunc, sy must be rparent" << std::endl;
		Enter_Mid_List(21, func_name + "_begin_", " ", " ", 0);		//jal���

		Enter_Mid_List(13, " ", " ", Creat_Label(" ", 4, MidCodeT.mdc), 3);//?
		//��ȥʱsy == rparent
	}

	//<�����>
	void Deal_scanfStatement(){
		//����ʱ sy == scanfsy
		//std::cout << "This is a scanf statement" << std::endl;
		judge_file << "This is scanf statement" << std::endl;
		symbol tmp_sys[] = { semicolon };
		insymbol();
		if (sy != lparent) {
			error(13, lc);
			Skip_Read(tmp_sys, 1);
			return;
		}
		do {
			insymbol();
			if (sy != ident) {
				error(11, lc);
				Skip_Read(tmp_sys, 1);
				return;
			}
			int t = find_in_tab(id);
			if(t == 0) error(23, lc);
			Enter_Mid_List(16," ", " ",id,0);
			insymbol();
			if ((sy != comma) && (sy != rparent)) {
				error(39,lc);
				Skip_Read(tmp_sys, 1);
				return;
			}
		} while (sy == comma);
		if (sy != rparent)	std::cout << "There is a bug in blockdeal--Deal_scanfStatement" << std::endl;
		insymbol();
		if (sy != semicolon) {
			error(19, lc);
			Skip_Read(tmp_sys, 1);
		}
		//����ʱ sy == semicolon
	}

	//<�����>
	//����xΪswitch�еı��ʽ,switch_end_labelΪswitch������λ�ñ�ǩ,����ÿһ��case�����break
	void Deal_caseTable(std::string x, std::string switch_end_label, bool is_char){
		//����ʱsy == lcbrack��{��
		//std::cout << "This is case-table statement" << std::endl;
		judge_file << "This is case-table statement" << std::endl;
		std::string z, y, j_label;
		std::set<int> case_int_list;
        std::set<char> case_char_list;
       // std::set<int>::iterator int_set_iter;
        //std::set<char>::iterator char_set_iter;
		int pos = 0;
		symbol tmp_sys[] = { rcbrack };

		insymbol();
		if (sy != casesy) {//case
			error(43, lc);
			Skip_Read(tmp_sys, 1);
			return;
		}

		do {
			//std::cout << "This is a case statement" << std::endl;
			judge_file << "This is a case statement" << std::endl;
			insymbol();
			switch (sy) {
			case plus:

				if (is_char) error(47, lc);//���ж���char�ͣ�case�������������
				insymbol();
				if(case_int_list.find(inum)==case_int_list.end()){case_int_list.insert(inum);}
				else {
                    error(52, lc);
                }

				if (sy == intcon) {
					y = int_to_string(inum);
				}
				else {
					error(22, lc);
					Skip_Read(tmp_sys, 1);
					return;
				}
				break;
			case minus:
				if (is_char) error(47, lc);
				insymbol();


				if (sy == intcon) {
					inum = inum * -1;
					if(case_int_list.find(inum)==case_int_list.end()){case_int_list.insert(inum);}
					else {
                    error(52, lc);
                    }
					y = int_to_string(inum);
				}
				else {
					error(22, lc);
					Skip_Read(tmp_sys, 1);
					return;
				}
				break;
			case intcon:
				if (is_char) error(47, lc);
				if(case_int_list.find(inum)==case_int_list.end()){case_int_list.insert(inum);}
				else {
                    error(52, lc);
                }
				y = int_to_string(inum);
				break;
			case charcon:
				if (!is_char) error(47, lc);
				if(case_char_list.find(inum)==case_char_list.end()){case_char_list.insert(inum);}
				else {
                    error(52, lc);
                }
				y = int_to_string(inum);
				break;
			default:
				error(22,lc);
				Skip_Read(tmp_sys, 1);
				return;
			}
			insymbol();
			if (sy != colon) {
				error(43, lc);
				Skip_Read(tmp_sys, 1);
				return;
			}

			pos = MidCodeT.mdc;
			Enter_Mid_List(7, " ", x, y, 0);	//���ڵ�ʱ�����ִ��,�����ڵ�ʱ����ת,���Դ˴�����תָ��Ӧ��Ϊ���Ⱥ�

			insymbol();
			if (!find_sy(Statement_Start_Sys, 9)) {
				error(32, lc);
				Skip_Read(Statement_Start_Sys, 9);
			}
			statement();

			//case��䴦����,��ת��switch����λ��
			Enter_Mid_List(5, switch_end_label, " ", " ", 0);
			j_label = Creat_Label(" ", 2, MidCodeT.mdc);
			Enter_Mid_List(13, " ", " ", j_label, 1);
			//��תָ�����,����case������λ��,��goto���ĺ���
			MidCodeT.midcodeArray[pos].z = j_label;		//�˴���j_label Ϊpc-1,��Ϊ��ǰ��������һ��goto���

			//�˴�syΪStatement_End_Sys�е�һ��
			insymbol();
		} while (sy == casesy);
		//std::cout<<"Now there are "<<case_int_list.size()<<" elements in the set case_int_list"<<std::endl;
		//std::cout<<"Now there are "<<case_char_list.size()<<" elements in the set case_char_list"<<std::endl;
		case_int_list.clear();
		case_char_list.clear();
        if(sy == defaultsy){//�����ǰ����case�ˣ���������default
            insymbol();//�ٶ�һ���ַ���Ӧ����ð��
            if (sy != colon) {
				error(43, lc);
				Skip_Read(tmp_sys, 1);
				return;
			}
			insymbol();//Ȼ�������Ĵ���
			if (!find_sy(Statement_Start_Sys, 9)) {
				error(32, lc);
				Skip_Read(Statement_Start_Sys, 9);
			}
			statement();//������䣬������ʱ��ΪStatement_End_Sys��������ݣ���; }
			insymbol();//������һ��
        }
		if (sy != rcbrack) {//����switch��{}
			error(43,lc);
			Skip_Read(tmp_sys, 1);
		}
		//����ʱsy == rcbrack
	}
	//<д���>
	void Deal_printfStatement(){
		//����ʱ sy == printfsy
		//std::cout << "This is a printf statement" << std::endl;
		judge_file << "This is printf statement" << std::endl;
		std::string z = " ", x = " ", y = " ";
		symbol tmp_sys[] = { semicolon };
		bool is_char = false;
		insymbol();
		if (sy != lparent) {
			error(13, lc);
			Skip_Read(tmp_sys, 1);
			return;
		}
		insymbol();
		if (sy == stringcon) {//���һ���ַ���
			char stemp[SMAX];
			for (int i = 0; i < sleng; i++) {
				stemp[i] = stab.stringArray[inum + i];//��stringArray��ȡ���ַ���
			}
			stemp[sleng] = '\0';
			x = stemp;
			insymbol();
			if (sy == comma) {//���ţ����б��ʽ
				insymbol();
				if (find_sy(Deal_Expression_Start_Sys, 6)) {
					y = Deal_Expression(is_char);
					if (is_char) z = "char";//���ʽ��һ���ַ��������ʱ����ַ�
					if (sy != rparent) {
						error(14, lc);
						Skip_Read(tmp_sys, 1);
						return;
					}
					Enter_Mid_List(17, z, x, y, 0);//����м����
					insymbol(); //������ķֺ�
					if (sy != semicolon) {
						error(19, lc);
						Skip_Read(tmp_sys, 1);
						return;
					}
				}
				else {
					error(40, lc);
					Skip_Read(tmp_sys, 1);
					return;
				}
			}
			else {//ֻ��һ���ַ���
				if (sy != rparent) {
					error(14, lc);
				}
				else {
					insymbol();
				}
				if (sy != semicolon) {
					error(19, lc);
				}
				Skip_Read(tmp_sys, 1);
				Enter_Mid_List(17, " ", x, " ", 0);
			}
		}
		else if (find_sy(Deal_Expression_Start_Sys, 6)) {//���ʽ
			y = Deal_Expression(is_char);
			if (is_char) z = "char";
			if (sy != rparent) {
				error(14, lc);
				Skip_Read(tmp_sys, 1);
				return;
			}
			Enter_Mid_List(17, z," ",y,0);
			insymbol();
			if (sy != semicolon) {
				error(19, lc);
				Skip_Read(tmp_sys, 1);
				return;
			}
		}
		else {
			error(40, lc);
			Skip_Read(tmp_sys, 1);
			return;
		}
		//��ȥʱ sy == semicolon
	}

	//<������>
	void Deal_switchStatement() {
		//����ʱ sy == switchsy
		//std::cout << "This is a switch statement" << std::endl;
		judge_file << "This is switch statement" << std::endl;
		std::string z, x, y;	//x��Ϊswitch()������Ĳ�����,y�������case�Ĳ�����
		std::string switch_end_label = Creat_Label(" ",3,MidCodeT.mdc);
		symbol tmp_sys[] = { rcbrack };
		bool is_char = false;
		insymbol();
		if (sy != lparent) {
			error(13, lc);
			Skip_Read(tmp_sys, 1);
			return;
		}
		insymbol();
		if (!find_sy(Deal_Expression_Start_Sys, 6)) {
			error(42, lc);
			Skip_Read(tmp_sys, 1);
			return;
		}
		x = Deal_Expression(is_char);//switch(x)
		//��Deal_Expressionʱ syΪfollow(Deal_Expression)����Ķ���
		if (sy != rparent) {
			error(14, lc);
			Skip_Read(tmp_sys, 1);
			return;
		}
		insymbol();
		if (sy != lcbrack) {//{
			error(17, lc);
			Skip_Read(tmp_sys, 1);
			return;
		}
		Deal_caseTable(x, switch_end_label, is_char);
		//��Deal_caseTableʱsy == rcbrack

		Enter_Mid_List(13, " ", " ", switch_end_label, 1);
		//����ʱ sy Ϊrcbrack
	}

	//<�������>
	void Deal_returnStatement(){
		//����ʱ sy == returnsy
		//std::cout << "This is a return statement" << std::endl;
		judge_file << "This is return statement" << std::endl;
		symbol tmp_sys[] = { semicolon };
		std::string y;

		bool is_char = false;
		insymbol();

		if (sy == semicolon) {	//����һ��return
			return_flag = 1;
			Enter_Mid_List(12," ", " "," ", 0);

			return;
		}
		else if(sy == lparent){
			insymbol();
			if (!find_sy(Deal_Expression_Start_Sys, 6)) {
				error(33, lc);
				Skip_Read(tmp_sys, 1);
				return;
			}
			y = Deal_Expression(is_char);
			//��Deal_ExpressionʱsyΪFollow_Deal_Expression_Sys�е�Ԫ��
			if(is_char)return_flag = 3;	//����ֵΪchars
			else return_flag = 2;	//����ֵΪints
			Enter_Mid_List(12, " ", " ", y, 0);

			if (sy != rparent) {
				error(41, lc);
				Skip_Read(tmp_sys, 1);
				return;
			}
			insymbol();
			if (sy != semicolon) {
				error(19,lc);
				Skip_Read(tmp_sys, 1);
				return;
			}
		}
		else {
			error(41, lc);
			Skip_Read(tmp_sys, 1);
			return;
		}
		//��ȥʱ sy == semicolon
	}

	//<���>
	void statement() {
		//����statementʱ, syΪStatement_Start_Sys�е�Ԫ��
		// ���õط�: statement(),statementCompond, Deal_ifStatement, do_Deal_whileStatement,
		int pos;
		std::string z, x, y;
		switch (sy) {
		case semicolon:	//����һ���ֺŴ���
			//std::cout << "This is a single ; in statement" << std::endl;
			break;
		case ifsy:	//������䴦��
			//����ʱsy == if
			Deal_ifStatement();
			break;//��ʱ sy ΪStatement_End_Sys�е�Ԫ��
		case whilesy:	//ѭ�����
			//����ʱsy == dosy
			Deal_whileStatement();
			break;//��ʱsy == rparent
		case lcbrack:	//{ ��� } ����
			insymbol();
			while (sy != rcbrack) {
				Skip_Read(Statement_Start_Sys, 9);
				//����statementʱ, syΪStatement_Start_Sys�е�Ԫ��
				statement();	//������䴦����
				//��statementʱ, sy Ϊ Statement_End_Sys�е�Ԫ��
				insymbol();	//���������һ������
			}
			break;	//��ʱsy == rcbrack
		case ident:		//�з���ֵ�������á��޷���ֵ�������á���ֵ��䴦��
			pos = find_in_tab(id);	//��tab���в�ѯ��ǰ���ʵ���Ϣ
			if(pos == 0) error(23, lc);
			if (pos != 0) {
				switch (tab.tabArray[pos].obj) {
				case constant://������ͷ�����㶼�ǲ��Ϸ���
					error(37, lc);
					Skip_Read(Statement_End_Sys, 3);
					break;
				case variable:	//��ֵ���
					Deal_assignStatement(0);
					//��ʱsy == semicolon
					break;
				case array:		//��ʶ��[���ʽ]  ��ֵ���
					Deal_assignStatement(1);
					//��ʱsy == semicolon
					break;
				case function:
					if (tab.tabArray[pos].type == notyp) {	//�޷���ֵ��������
						//����Deal_voUseFuncʱ,sy == identsy(������)
						Deal_voUseFunc();
					}
					else {
						//����Deal_reUseFuncʱ,sy == identsy(������)
						Deal_reUseFunc();
					}

					//��ʱ syΪ rparent
					insymbol();
					if (sy != semicolon) error(19, lc);
					//sy == semicolon
					break;
				default:
					std::cout << "There is a bug in blockdeal--statement, ident-switch-obj" << std::endl;
					break;
				}
			}
			else {
				Skip_Read(Statement_End_Sys, 3);
			}
			break;	//��ʱ sy == semicolon
		case scanfsy:	//����䴦��
			//����ʱsy == scanfsy
			Deal_scanfStatement();
			break;	//��ʱ sy == semicolon
		case printfsy:	//д��䴦��
			//����ʱsy == printfsy
			Deal_printfStatement();
			break;//��ʱsy == semicolon
		case switchsy:	//�����䴦��
			//����ʱ sy == switchsy
			Deal_switchStatement();
			if (sy != rcbrack) std::cout << "There is a bug in blockdeal--statement, after Deal_switchStatement()" << std::endl;
			break; //��ʱ sy == rcbrack;
		case returnsy:	//������䴦��
			//����ʱ sy == returnsy
			Deal_returnStatement();
			break;//��ʱ sy == semicolon
		default:
			error(32, lc);
			Skip_Read(Statement_End_Sys, 3);
			break;
		}
		if (!find_sy(Statement_End_Sys, 3)) std::cout << "There is a bug in blockdeal--statement, at the end of the function" << std::endl;

		//��statementʱ, syΪ Statement_End_Sys�е�һ��
	}
	//<�������>
	//����ֵΪ�����ֲ�������ռ�ռ�Ĵ�С������ֵ���͡�Ŀǰ��������������������������Ե�ַ,����ֵΪ�ú������һ���ֲ�������tab�е�λ��
	int statementCompond(int& var_size, types& return_typ, int& pv_addr){
		//���ʱsyָ����������֮��ĵ�һ������
		symbol statementCompond_start_sys[] = { constsy, intsy, charsy, semicolon, ifsy, whilesy, lcbrack, ident, scanfsy, printfsy, switchsy, returnsy, rcbrack};//13��
		types tmp_type;
		int start_pv_addr = pv_addr;//��¼����ú���ǰ�������ı���ռ������������Ե�ַ
		int var_flag = 0;	//�Ƿ��оֲ������ı�־λ,���ں����return����

		if (!find_sy(statementCompond_start_sys, 13)) {
			error(32, lc);
			Skip_Read(statementCompond_start_sys, 13);
		}

		//�ֲ���������
		while (sy == constsy) {
			Deal_Const();	//��Deal_Const�������ʱsyΪ';'����ĵ���
		}
		Skip_Read(statementCompond_start_sys, 13);	//����, ��ʱsy������Ϊconstsy,���������ѭ��



		//�ֲ���������
		while ((sy == intsy) || (sy == charsy)) {

			tmp_type = Sy_Trans_Types(sy);
			insymbol();
			if (sy != ident) {
				error(11, lc);
				Skip_Read(statementCompond_start_sys, 13);
				//std::cout << "This is a var_declare statement" << std::endl;
				judge_file << "This is var_declare statement" << std::endl;
				continue;
			}

			insymbol();

			//test sy �Ƿ�Ϊ comma, semicolon, lsbrack �����ǵĻ�����
			symbol tmpsys[] = { comma, semicolon, lsbrack };
			if (!find_sy(tmpsys, 3)) {
				error(28, lc);
				Skip_Read(statementCompond_start_sys, 13);
				//std::cout << "This is a var_declare statement" << std::endl;
				judge_file << "This is var_declare statement" << std::endl;
				continue;
			}

			//��ʱ sy == comma || sy == semicolon || sy == lsbrack
			Deal_Varible(tmp_type, pv_addr);	//����ʱ��������';'֮��ĵ���
			var_flag = 1;//var_flagΪ1˵���ø�������ڲ��������������ֲ�����
		}
		var_size = pv_addr - start_pv_addr; //�ֲ�������ռ�Ŀռ��С

		//�Ӵ����²����ٽ���enter������ֻ��½������һЩ�м����

		//�˴�Ϊ���������ķֺŽ���֮��ĵ�һ������
		symbol tmp_sys[] = { semicolon, ifsy, whilesy, lcbrack, ident, scanfsy, printfsy, switchsy, returnsy, rcbrack };
		Skip_Read(tmp_sys, 10);

		while (sy != rcbrack) {
			Skip_Read(Statement_Start_Sys, 9);//�ҵ��������Ŀ�ͷ�ַ�
			//����statementʱ, syΪStatement_Start_Sys�е�Ԫ��
			statement();	//������䴦����
			//��statementʱ, sy Ϊ Statement_End_Sys�е�Ԫ��
			insymbol();
			if ((sy != rcbrack) && (!find_sy(Statement_Start_Sys, 9))) {
				error(48,lc);
			}
		}


		if (var_flag == 1) return tab.t - 1;//�����������ڣ��������ڣ��������˾ֲ����������ⲿdisplay��ָ���btab����last��ϢҲҪ���޸�
		else return btab.btabArray[btab.b-1].lastpar;//û�������ֲ���������lastҲ����btab�����lastpar�������Ĳ�����λ��
		//��������䴦��ʱ,sy == rcbrack
	}


	//<�޷���ֵ��������>
	void Deal_voFunc(types typ){
		//����ģ��ʱ,sy == lparent
		//std::cout << "This is a voidFunc declare statement" << std::endl;
		judge_file << "This is a voidFunc declare statement" << std::endl;
		int para_num = 0, var_size = 0,	pv_addr = 3;	//pv_addr ����ں�����ַ����Ե�ַ, 0��1��2Ϊ������
		types return_typ = notyp;//���ȶ�return_typ��typ�Ƿ�һ��  typΪ����ʱ�ķ���ֵ����
		std::string func_name = id;

		int init_t_num, last_t_num;		//�ú��������е�һ����(���һ����ʱ���� + 1) �ı��, ���ں���õ�t_varnum�͸�����ʱ��������Ե�ַ

		init_t_num = temp_num;//��¼�¶�ȡ�ú���ǰ����ʱ��������

		return_flag = 0;//�޷���ֵ

		enter(id, function, typ);//��tab�����뺯������Ϣ
            enterFunc(tab.t - 1, typ);

		level++;	//����ֳ����(�ֲ�)
		display[level] = btab.b - 1;	//����display����ǰ��ֳ����������

		//ѹ�뺯��������Ԫʽָ��
		Enter_Mid_List(14, " ", types_info[typ], func_name, 0);//func ����ֵ����,��������

		insymbol();

		if (sy == rparent) {	//�޲���
			btab.btabArray[btab.b - 1].lastpar = 0;
			btab.btabArray[btab.b - 1].paranum = 0;
			insymbol();
		}
		else if ((sy == intsy) || (sy == charsy)) {	//�в���
			//�ڲ�������ѹ����Ӧ����Ԫʽָ��
			btab.btabArray[btab.b - 1].lastpar = Deal_Para(para_num, pv_addr);//���������,��¼���������͵�ַ������ֵΪ���һ��������tab�е�λ��
			//������Ϊ')'֮��ĵ���
			btab.btabArray[btab.b - 1].paranum = para_num;//��������
		}
		else {
			//��������ֱ�������������
			error(31, lc);
			symbol tmp_sys[] = { lcbrack };
			Skip_Read(tmp_sys, 1);
		}
		if (sy != lcbrack) {
			error(17, lc);
			symbol tmp_sys[] = { lcbrack };
			Skip_Read(tmp_sys, 1);
		}

		//����˴��������������������

		//����������������,��goto���ѹ����Ԫʽ
		//Enter_Mid_List(5, func_name + "_end_", " ", " ", 0);

		//������俪ʼ
		Enter_Mid_List(13, " ", " ", Creat_Label(func_name, 0, MidCodeT.mdc), 4);		//����������ʼ����ǩ��4������setlabel

		insymbol();
		if (sy == rcbrack) {
			//�൱�ڸ������ֱ��Ϊ��
			btab.btabArray[btab.b - 1].last = btab.btabArray[btab.b - 1].lastpar;//û�оֲ��������������һ����ʶ��Ҳ�������һ������
			btab.btabArray[btab.b - 1].varsize = 0;//�ֲ���������Ϊ0
			//����������ת��Ԫʽ �������ڸ�������д���,������û�н�������Ҫ����һ��
			Enter_Mid_List(12, " ", " ", " ", 0);// ret y  �������ز���,����ֵy����*RET  return temp / return 12
		}
		else {
			//���븴�����,��ʱsyΪ������֮��ĵ�һ������
			btab.btabArray[btab.b - 1].last = statementCompond(var_size, return_typ, pv_addr);//�޸����һ����ʶ����λ�ú�varsize
			btab.btabArray[btab.b - 1].varsize = var_size;//�޸ľֲ�������Sջ����ռ�洢��Ԫ����Ŀ
			if ((return_flag != 0) && (return_flag != 1)) error(12, lc - 1);//�޷���ֵ�������岻���з���ֵ
			if (MidCodeT.midcodeArray[MidCodeT.mdc - 1].op != 12) {//����޷���ֵ��������ĸ����������û��return���
				//�������һ�����Ƿ������
				Enter_Mid_List(12, " ", " ", " ", 0);	//void�ĺ���ֱ������һ��,Ҫ��������ȥ
			}
		}

		last_t_num = temp_num;//�õ�Ŀǰ��ʱ�����ļ���
		btab.btabArray[btab.b - 1].t_varnum = last_t_num - init_t_num;//���øú������漰������ʱ�����ĸ���

		//����T_MAP
		for (int i = init_t_num; i < last_t_num; i++) {
			int offset = pv_addr + i - init_t_num;//���漰������;ֲ�������pv_addr��ʼ ��������ƫ�Ƶ�ַ
			T_Map["#" + int_to_string(i)].stack_addr = offset;//T-MAP���� #-��� Ϊ��ֵ���޸Ķ�Ӧvar_info�е����ջ�ĵ�ַƫ��
		}

		//�˴�sy == rcbrack���ǴӸ����������������
		if (sy != rcbrack) {
			std::cout << "There is a bug in blockdeal--Deal_voFunc, after the transfer of statementCompond" << std::endl;
		}
		//�ڴ˴�ѹ�뺯��������ǩ
		//Enter_Mid_List(13, " ", " ", Creat_Label(func_name, 1, MidCodeT.mdc), 1);	//����label��PC�Ķ�Ӧ��ϵʱmdcָ�ĸպ���setlab��λ��,��δ����++����
		level--;	//�˳��ֳ����
		insymbol();	//��ȡ'}'�����һ������
	}

	//<�з���ֵ��������>
	void Deal_reFunc(types typ){
		//����ģ��ʱ,sy == lparent
		//std::cout << "This is a returnFunc declare statement"<<std::endl;
		judge_file << "This is a returnFunc declare statement" << std::endl;
		int para_num = 0, var_size = 0, pv_addr = 3;	//pv_addrΪ�������ֲ���������ڵ�ǰ����������Ե�ַ ��3��ʼ, 0��1��2��λ��Ϊ������
		types return_typ = notyp;//���ȶ�return_typ��typ�Ƿ�һ��  typΪ����ʱ�ķ���ֵ����
		std::string func_name = id;
		int init_t_num, last_t_num;

		init_t_num = temp_num;

		return_flag = 0;

		enter(id, function, typ);
        enterFunc(tab.t - 1, typ);


		//����ʱlevel = 0, ��ȥʱҲ����0, �����ڲ�����Ϊ1
		level++; //��ǰ�ֳ����Ϊ1
		display[level] = btab.b - 1;	//���µ�ǰ��ֳ����������

		//ѹ�뺯��������Ԫʽָ��
		Enter_Mid_List(14," ",types_info[typ],func_name, 0);

		insymbol();
		if (sy == rparent) {	//�޲���
			btab.btabArray[btab.b - 1].lastpar = 0;
			btab.btabArray[btab.b - 1].paranum = 0;
			insymbol();
		}
		else if ((sy == intsy) || (sy == charsy)) {	//�в���
			//�ڲ�������ѹ����Ӧ����Ԫʽָ��
			btab.btabArray[btab.b-1].lastpar = Deal_Para(para_num, pv_addr);//���������,������Ϊ')'֮��ĵ���
			btab.btabArray[btab.b - 1].paranum = para_num;
		}
		else {
			//��������ֱ�������������
			error(31, lc);
			symbol tmp_sys[] = { lcbrack };
			Skip_Read(tmp_sys,1);
		}
		//�Ӳ�������������ĵ����� ')'����һ������
		if (sy != lcbrack) {
			error(17,lc);
			symbol tmp_sys[] = { lcbrack };
			Skip_Read(tmp_sys, 1);
		}

		//����˴��������������������

		//����������������,��goto���ѹ����Ԫʽ
		//Enter_Mid_List(5, func_name + "_end_", " ", " ", 0);

		//������ʼlabel,ѹ����Ԫʽ setlab
		Enter_Mid_List(13," ", " ",Creat_Label(func_name,0,MidCodeT.mdc),4);

		insymbol();
		if (sy == rcbrack) {
			//�൱�ڸ������ֱ��Ϊ��,�Ͳ���ȥ��,�������
			btab.btabArray[btab.b - 1].last = btab.btabArray[btab.b - 1].lastpar;
			btab.btabArray[btab.b - 1].varsize = 0;
			error(12, lc - 1);	//�з���ֵ�����Żᱨ��
			//����������ת��Ԫʽ �������ڸ�������д���,������û�н�������Ҫ����һ��
			Enter_Mid_List(12, " ", " ", " ", 0);
		}
		else {
			//���븴�����
			btab.btabArray[btab.b - 1].last = statementCompond(var_size, return_typ, pv_addr);
			btab.btabArray[btab.b - 1].varsize = var_size;//�ֲ�������ռ�ռ�Ĵ�С
			if (return_flag == 2) {	//����ֵΪints
				if (typ != ints) error(12, lc - 1);
			}
			else if (return_flag == 3) {	//����ֵΪchars
				if (typ != chars) error(12, lc - 1);
			}
			else {//���������û�з��������߷��ص�ֵ����2/3��
				error(12, lc - 1);
			}
			if (MidCodeT.midcodeArray[MidCodeT.mdc - 1].op != 12) {
				//�������һ�����Ƿ������
				Enter_Mid_List(12, " ", " ", " ", 0);	//ֱ������һ��,���ص���λ��
			}
		}

		last_t_num = temp_num;
		btab.btabArray[btab.b - 1].t_varnum = last_t_num - init_t_num;

		//����T_MAP
		for (int i = init_t_num; i < last_t_num; i++) {
			int offset = pv_addr + i - init_t_num;
			T_Map["#" + int_to_string(i)].stack_addr = offset;
		}



		//�˴�sy == rcbrack���ǴӸ����������������
		if (sy != rcbrack) {
			std::cout << "There is a bug in blockdeal--Deal_reFunc, after the transfer of statementCompond" << std::endl;
		}


		level--; //�˳���ǰ�ֳ����

		insymbol();	//'}'����һ������
	}


	//<�����к�������>
	void Deal_Main(){
		//�����ģ��ʱ sy == lparent
		//std::cout << "This is a main declare statement" << std::endl;
		judge_file << "This is a main declare statement" << std::endl;
		int para_num = 0, var_size = 0, pv_addr = 3;	//pv_addr ����ں�����ַ����Ե�ַ, 3��������,Ϊ��callָ���Ϊmips��һ����
		types return_typ = notyp;//���ȶ�return_typ��typ�Ƿ�һ��  typΪ����ʱ�ķ���ֵ����
		std::string func_name = "main";
		int init_t_num, last_t_num;

		init_t_num = temp_num;


		enter(func_name, function, notyp);
        enterFunc(tab.t - 1, notyp);



		level++;	//����ֳ����(�ֲ�)
		display[level] = btab.b - 1;	//���µ�ǰ��ֳ����������

		//ѹ�뺯��������Ԫʽָ��
		Enter_Mid_List(14, " ", types_info[notyp], func_name, 0);

		insymbol();
		if (sy != rparent) {	//main�����޲���
			error(10, lc);

		}
		//ֱ���������������
		symbol tmp_sys[] = { lcbrack };
		Skip_Read(tmp_sys, 1);


		//������ʼlabel,ѹ����Ԫʽ setlab
		Enter_Mid_List(13, " ", " ", "main", 2);		//2��־Ϊmain������ʼλ��

		//����˴��������������������
		insymbol();
		if (sy == rcbrack) {
			//�൱�ڸ������ֱ��Ϊ��
			btab.btabArray[btab.b - 1].last = btab.btabArray[btab.b - 1].lastpar;
			btab.btabArray[btab.b - 1].varsize = 0;
		}
		else {
			//���븴�����
			btab.btabArray[btab.b - 1].last = statementCompond(var_size, return_typ, pv_addr);
			btab.btabArray[btab.b - 1].varsize = var_size;
			if (return_typ != notyp)	error(12, lc);	//���������뷵�����Ͳ�ƥ��

		}

		last_t_num = temp_num;
		btab.btabArray[btab.b - 1].t_varnum = last_t_num - init_t_num;

		//����T_MAP
		for (int i = init_t_num; i < last_t_num; i++) {
			int offset = pv_addr + i - init_t_num;
			T_Map["#" + int_to_string(i)].stack_addr = offset;
		}

		//�˴�sy == rcbrack���ǴӸ����������������
		if (sy != rcbrack) {
			std::cout << "There is a bug in blockdeal--Deal_Main, after the transfer of statementCompond" << std::endl;

		}

		level--;	//�˳��ֳ����
	}

	// <����> ����
	void block(){
		types tmp_type;
		symbol tmp_sy;	//�б��Ƿ�Ϊmiansy��ʱ��ʹ��
		int global_var_addr = 0, global_var_num = 0;
		judge_file.open("gram_output.txt", std::fstream::out);
		insymbol();
		Skip_Read(Block_Start_Sys, 4);
		//����const����
		while(sy == constsy){
			Deal_Const();	//��Deal_Const�������ʱsyΪ';'����ĵ���

		}

		//ȫ�ֱ�������
		while(1){
			Skip_Read(Block_Start_Sys, 4);//��� sy �Ƿ�Ϊ constsy, intsy, charsy ���� voidsy, �����ǵĻ�����
			if (sy == constsy) {
				error(29, lc);
				insymbol();
				Skip_Read(Block_Start_Sys, 4);
				continue;
			}
			tmp_type = Sy_Trans_Types(sy);
			insymbol();
			tmp_sy = sy;
			if ((sy != ident) && (sy != mainsy)) {
				error(11, lc);
				Skip_Read(Block_Start_Sys, 4);
				//std::cout << "This is a var_declare statement" << std::endl;
				judge_file << "This is var_declare statement" << std::endl;
				continue;
			}
			insymbol();
			//test sy �Ƿ�Ϊ comma, semicolon, lparent, lsbrack �����ǵĻ�����
			symbol tmpsys[] = { comma, semicolon, lparent, lsbrack};
			if (!find_sy(tmpsys, 4)) {
				error(28, lc);
				Skip_Read(Block_Start_Sys, 4);
				//std::cout << "This is a var_declare statement" << std::endl;
				judge_file << "This is var_declare statement" << std::endl;
				continue;
			}

			if(sy == lparent) break;	//��������
			//����������֮ǰ������
			if (tmp_type == notyp) {	//����ֻ����char����int����
				error(28, lc);
				Skip_Read(Block_Start_Sys, 4);
				//std::cout << "This is a var_declare statement" << std::endl;
				judge_file << "This is var_declare statement" << std::endl;
				continue;
			}
			Deal_Varible(tmp_type,global_var_addr);	//����ʱ��������';'֮��ĵ���
		}
	//	std::cout <<"tab.t-1:"<< tab.t-1 << std::endl;
	//	std::cout <<"Stack.topaddr:"<<Stack.topaddr<< std::endl;
		//����btab.btabArray[0],��ȫ�ּӽ�ȥ
		btab.btabArray[0].last = tab.t - 1;
		btab.btabArray[0].varsize = global_var_addr; //ȫ�ֱ�����ռ���������ܴ�С

		//��ȫ����ѹջ
		for (int i = 0; i < tab.t; i++) {
			Stack.space[Stack.topaddr++] = tab.tabArray[i].adr;//��ȫ������ֵѹ��ջ
		}
		Stack.globalvaraddr = 0;	//ȫ������ʼ��λ��Ϊ0

		//��ȫ�ֱ����������������:  sy == lparent
		if (sy != lparent) {
			std::cout << "There is a bug in blockdeal, before funcDeal" << std::endl;
		}

		if (tmp_sy != mainsy) {
			//��������Ժ�ĵ�һ������
			if (tmp_type == notyp) {	//�޷���ֵ��������
				Deal_voFunc(tmp_type);
			}
			else {		//�з��غ�������
				Deal_reFunc(tmp_type);
			}
			//�Ӻ������������Ϊ'}'����һ������
			//ѭ������ʣ�µĺ���,��int��char����void��ʼ
			while (1) {
				Skip_Read(Block_Start_Sys, 4);	//��� sy �Ƿ�Ϊ constsy, intsy, charsy ���� voidsy, �����ǵĻ�����
				if (sy == constsy) {
					error(29, lc);
					insymbol();
					Skip_Read(Block_Start_Sys, 4);
					continue;
				}
				tmp_type = Sy_Trans_Types(sy);
				insymbol();
				tmp_sy = sy;

				if ((sy != ident) && (sy != mainsy)) {
					error(11, lc);
					Skip_Read(Block_Start_Sys, 4);
					continue;
				}

				insymbol();
				if (sy != lparent) {
					error(13, lc);
					Skip_Read(Block_Start_Sys, 4);
					continue;
				}

				//�ڴ�λ�� sy == lparent;
				if (tmp_sy == mainsy) break;	//�����к���
				if (tmp_type == notyp) {	//�޷���ֵ��������
					Deal_voFunc(tmp_type);
				}
				else {		//�з��غ�������
					Deal_reFunc(tmp_type);
				}

			}
		}
				//�������е��˲����������tmp_syΪmainsy
		//�� sy == lparent
		if (tmp_sy != mainsy) {
			std::cout << "There is a bug in blockdeal, before Deal_Main" << std::endl;
		}
		//std::cout << "Begin main deal" << std::endl;
		//main��������,��ʱidΪmain,syΪmainsy
		if (tmp_type != notyp) {	//��ʱ����notypֻ����void,��Ϊ������while�����main����breakǰ�������Skip_Read���
			error(10, lc);
		}
		//����ʱsy == lparent
		Deal_Main();

		//�˳�ʱ sy == rcbrack

	}
	}
