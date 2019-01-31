

/*
�м����תΪmips����
*/
#include "Mid_to_Mips.h"
#define	GET_REG_ID(n) n+8			//��reg�����±�õ��Ĵ�����ʵ�ʱ��
#define GET_OFFSET(n) -4*n
#define STACK_TOP	"$sp" //��ջ
#define DATA_BASE	"$fp" //������
#define GLOBAL_BASE	"$gp"
#define RE_LABEL_REG		"$ra"// jal
#define RE_VALUE_REG		"$v1" //����ֵ
#define A_REG_NUM   4
#define R_REG_NUM	18
#define CALL_MAX 100
#define S_GLOBAL_MAX 9

typedef std::pair<std::string, int> PAIR;

namespace compiler {
	//fp�Ĵ���ָ��ǰ��������ַ
	//gp��fp��sp���Ǵ��ϵ��µ�,��һ��ʼջ���ڵ͵�ַ, ջ���ڸߵ�ַ  ����ջָ���ƶ���Ҫ��subi�ﵽĿ��,������ص�ƫ��ҲҪ�ø���
	//��t�Ĵ�����s�Ĵ���ͳһ����  (8 - 25)
	//ȫ�ֱ�����gp��lw����

	//ÿһ�κ���ջָ���ƶ�ʱ�ǵø���display[1]��ֵ
	//Ϊÿһ����ʱ��������һ��ջ�ռ�

	struct reg {
		std::string name;		//�ô洢����ʱ����/�ֲ�����������
		int regKind;			//0-��ʱ����, 1-��ʱ����, 2-�ֲ�����(��������, map������array[1]��ʾ������), 3-ȫ�ֱ���
		int offAddr;		//��Ϊ��ʱ����(���֮ǰ��ջ�д��ڵĻ�,������������Ϊ-1)���ֲ���������ȫ�ֱ����Ļ�����Ը����������ĵ�ַƫ��, sw��ʱ����
		int isEmpty;	//��־��ǰ�Ĵ����Ƿ����
		int isBusy;		//֮����ݳ�ͻ���õ�ǰָ���Ƿ�ʹ��
	};

	struct r_reg {
		reg Regs[R_REG_NUM];	//��ʱ�Ĵ��� $8-$25
		int s_ptr;		//ָ��ǰ����λ�û��ߵ�ǰ�ñ��滻�ļĴ���(FIFO)
		int t_ptr;		//ָ��ǰ����λ�û��ߵ�ǰ�ñ��滻�ļĴ���(FIFO)
		bool s_isfull;		//��ʾ�Ƿ�����
		bool t_isfull;		//��ʾ�Ƿ�����
	};

	r_reg Reg_Set;


	std::map<std::string, var_info> S_Map;	//�ֲ�������Ϣ����ÿ��jal֮ǰ �� �������ص���(label isstart = 3)�����
	std::map<std::string, int> S_Count_Map;
	int ParaOffset;	//��ÿ��call��ʱ�����, ��parapassDeal��ʹ�ò�����������ʾ��ǰʵ��Ӧ��ѹ���λ��
	std::string Call_Func_Name[CALL_MAX];//call �ĺ�����ջ, ��jal������ҵ�ʱ��ʹ��
	int Call_Func_Ptr = 0;
	int Str_Id = 0;	//����string��ǩʹ��
    int temp_cnt=0;
	int Alloc_SCount_In_Func = 0;	//��¼��ǰ���������˼���ȫ�ּĴ���, ��ÿ�δ�������ʼʱ��0
	//����string��ǩ�����أ�����Ӧ��Ϣ�ŵ�str_info_map��
	std::string Get_Label_For_Str(std::string str) {
		std::string label;
		label = "str_" + int_to_string(Str_Id);
		Str_Id++;
		Str_Info_Map[label] = str;
		return label;
	}
	//���ݷ��ɵ��ļĴ����� 0-17�õ������MIPSָ���мĴ����ı�﷽ʽ
	std::string id_Trans_Regnum(int num) {
		return "$" + int_to_string(GET_REG_ID(num));
	}

	/*
		��������: ����ǰ�Ĵ�����ֵ�����ڴ�
		��������������: �Ĵ����������±�
	*/
	void pushReg(int reg_pos) {
		if (Reg_Set.Regs[reg_pos].regKind == 1) {	//��ʱ����
			Enter_Mips_List(9, id_Trans_Regnum(reg_pos), DATA_BASE, " ", GET_OFFSET(Reg_Set.Regs[reg_pos].offAddr));
			T_Map[Reg_Set.Regs[reg_pos].name].inReg = false;	//���ڼĴ�����
		}
		else if (Reg_Set.Regs[reg_pos].regKind == 2) {		//�ֲ�����
			Enter_Mips_List(9, id_Trans_Regnum(reg_pos), DATA_BASE, " ", GET_OFFSET(Reg_Set.Regs[reg_pos].offAddr));
			S_Map[Reg_Set.Regs[reg_pos].name].inReg = false;	//���ڼĴ�����
		}
		else if (Reg_Set.Regs[reg_pos].regKind == 3) {		//ȫ�ֱ���
			Enter_Mips_List(9, id_Trans_Regnum(reg_pos), GLOBAL_BASE, " ", GET_OFFSET(Reg_Set.Regs[reg_pos].offAddr));
			S_Map[Reg_Set.Regs[reg_pos].name].inReg = false;	//���ڼĴ�����
		}
	}

		/*
		��������: ��ռĴ���
		��������������:  kind = 0�� ��� $8-$25�Ĵ���, �������ڴ�;		����������ʼʱʹ��
		kind = 1, ��� $8-$25�Ĵ���, ����Ӧ���ݴ����ڴ���;		������������ʱ��jal֮ǰʹ��
		kind = 2, ��� $8-$25�Ĵ���, ��ȫ�ֱ��������ڴ�,���಻����	 ��������ʱʹ��
		kind = 3, ��� ��ʱ�����Ĵ���������ȫ�ּĴ���(funcһ��ʼ����ı���, �����ڿ��ʹ��)
	*/
	void clearRegs(int kind) {
	    bool flag=false;
		switch (kind) {
		case 0://kind = 0�� ��� $8-$25�Ĵ���, �������ڴ�;����������ʼʱʹ��
			for (int i = 0; i < R_REG_NUM; i++) {
				if (Reg_Set.Regs[i].regKind == 1) {
					T_Map[Reg_Set.Regs[i].name].inReg = false;	//���ڼĴ�����
				}
				else if ((Reg_Set.Regs[i].regKind == 2) || (Reg_Set.Regs[i].regKind == 3)) {
					S_Map[Reg_Set.Regs[i].name].inReg = false;	//���ڼĴ�����
				}
				Reg_Set.Regs[i].isEmpty = true;
				Reg_Set.Regs[i].name = " ";
				Reg_Set.Regs[i].offAddr = -1;
				Reg_Set.Regs[i].regKind = -1;
				Reg_Set.Regs[i].isBusy = false;
			}
			//temp_cnt=0;
			Reg_Set.s_ptr = 8;//0-7 16��17����ʱ�Ĵ���
			Reg_Set.t_ptr = 0;//8-15��ȫ�ּĴ���
			Reg_Set.s_isfull = false;
			Reg_Set.t_isfull = false;

			break;
		case 1://kind = 1, ��� $8-$25�Ĵ���, ����Ӧ���ݴ����ڴ���;	������������ʱ��jal֮ǰʹ��
			for (int i = 0; i < R_REG_NUM; i++) {
				if (Reg_Set.Regs[i].isEmpty == false) pushReg(i);
				Reg_Set.Regs[i].isEmpty = true;
				Reg_Set.Regs[i].name = " ";
				Reg_Set.Regs[i].offAddr = -1;
				Reg_Set.Regs[i].regKind = -1;
				Reg_Set.Regs[i].isBusy = false;
			}
			Reg_Set.t_ptr = 0;
			Reg_Set.s_ptr = 8;
			Alloc_SCount_In_Func = 0;
			Reg_Set.t_isfull = false;
			Reg_Set.s_isfull = false;
			break;
		case 2:	//kind = 2, ��� $8-$25�Ĵ���, ��ȫ�ֱ��������ڴ�,���಻����;��������ʱʹ��
			for (int i = 8; i < 8+Alloc_SCount_In_Func; i++) {
				if (Reg_Set.Regs[i].regKind == 3) {
					Enter_Mips_List(9, id_Trans_Regnum(i), GLOBAL_BASE, " ", GET_OFFSET(Reg_Set.Regs[i].offAddr));
				}
			}

			for (int i = 0; i < 8; i++) {
				if (Reg_Set.Regs[i].regKind == 1) {
					T_Map[Reg_Set.Regs[i].name].inReg = false;	//���ڼĴ�����
				}
				else if (Reg_Set.Regs[i].regKind == 2) {
                    flag==true;
					S_Map[Reg_Set.Regs[i].name].inReg = false;	//���ڼĴ�����
				}
				else if (Reg_Set.Regs[i].regKind == 3) {
					pushReg(i);
					S_Map[Reg_Set.Regs[i].name].inReg = false;	//���ڼĴ�����
				}
				Reg_Set.Regs[i].isEmpty = true;
				Reg_Set.Regs[i].name = " ";
				Reg_Set.Regs[i].offAddr = -1;
				Reg_Set.Regs[i].regKind = -1;
				Reg_Set.Regs[i].isBusy = false;
			}

			for (int i = 8+Alloc_SCount_In_Func; i < R_REG_NUM; i++) {
				if (Reg_Set.Regs[i].regKind == 1) {
					T_Map[Reg_Set.Regs[i].name].inReg = false;	//���ڼĴ�����
				}
				else if (Reg_Set.Regs[i].regKind == 2) {
                    flag==true;
					S_Map[Reg_Set.Regs[i].name].inReg = false;	//���ڼĴ�����
				}
				else if (Reg_Set.Regs[i].regKind == 3) {
					pushReg(i);
					S_Map[Reg_Set.Regs[i].name].inReg = false;	//���ڼĴ�����
				}
				Reg_Set.Regs[i].isEmpty = true;
				Reg_Set.Regs[i].name = " ";
				Reg_Set.Regs[i].offAddr = -1;
				Reg_Set.Regs[i].regKind = -1;
				Reg_Set.Regs[i].isBusy = false;
			}
			//temp_cnt=0;
			Reg_Set.t_ptr = 0;
			Reg_Set.t_isfull = false;
            Reg_Set.s_ptr = 8+Alloc_SCount_In_Func;
            if(Alloc_SCount_In_Func<8){
                Reg_Set.s_isfull = false;
            }else{
            Reg_Set.s_isfull = true;
            }

			break;
		case 3://kind = 3, ��� ��ʱ�����Ĵ���������ȫ�ּĴ���(funcһ��ʼ����ı���, �����ڿ��ʹ��)

			for (int i = 0; i < 8; i++) {
				if (Reg_Set.Regs[i].isEmpty == false) pushReg(i);
				Reg_Set.Regs[i].isEmpty = true;
				Reg_Set.Regs[i].name = " ";
				Reg_Set.Regs[i].offAddr = -1;
				Reg_Set.Regs[i].regKind = -1;
				Reg_Set.Regs[i].isBusy = false;

			}

			for (int i = 8+Alloc_SCount_In_Func; i < R_REG_NUM; i++) {
                if (Reg_Set.Regs[i].isEmpty == false) pushReg(i);
				Reg_Set.Regs[i].isEmpty = true;
				Reg_Set.Regs[i].name = " ";
				Reg_Set.Regs[i].offAddr = -1;
				Reg_Set.Regs[i].regKind = -1;
				Reg_Set.Regs[i].isBusy = false;
			}
			//std::cout <<"Alloc_SCount_In_Func="<< Alloc_SCount_In_Func  << std::endl;
			temp_cnt=0;
			Reg_Set.t_ptr = 0;
			Reg_Set.t_isfull = false;
			Reg_Set.s_ptr = 8+Alloc_SCount_In_Func;
			//Reg_Set.s_isfull = false;
			 if(Alloc_SCount_In_Func<8){
                Reg_Set.s_isfull = false;
            }else{
            Reg_Set.s_isfull = true;
            }
			break;
		default:
			//std::cout << "There is a bug in midtomipsdeal--clearRegs, switch-default.kind: " << kind << std::endl;
			break;
		}
	}

	//��Map����Ѱ��,�ҵ�����true���޸�var_info���õ�ֵ, δ�ҵ�����0   kind = 0, T_Map����, kind = 1, S_Map����
	bool findInMap(std::string name, int kind, var_info& vinfo) {
		if (kind == 0) {//��T_Map��Ѱ��
			auto iter = T_Map.find(name);
			if (iter == T_Map.end()) return false;	//û�ҵ�
			else {
				vinfo = iter->second;
				return true;
			}
		}
		else if (kind == 1) {//��S_Map��Ѱ��
			auto iter = S_Map.find(name);
			if (iter == S_Map.end()) return false;	//û�ҵ�
			else {
				vinfo = iter->second;
				return true;
			}
		}
		else {
			//std::cout << "There is a bug in mittomipsdeal--findInMap, kind error" << std::endl;
			return false;
		}
	}

		//����һ��r_reg�Ĵ���, ��Ҫʱ�ѸüĴ����洢��ֵ�����ڴ���,����Reg_Set�±�
	int allocR_REG(std::string name, int offaddr, int regkind) {
	    if((regkind==2)||(regkind==3)){//����ȫ�ֱ��� 8-15
            //�ж��Ƿ��ͻ
            if (Reg_Set.Regs[Reg_Set.s_ptr].isBusy) {
                if(Reg_Set.s_ptr == 15){ Reg_Set.s_ptr = 8;}
                else { Reg_Set.s_ptr ++;}
                if (Reg_Set.s_ptr == 8) {
                    Reg_Set.s_ptr = 8+Alloc_SCount_In_Func;
                    Reg_Set.s_isfull = true;		//�൱�� Reg_Set.ptr + 1 = 18��
                }
            }
            int r_id = Reg_Set.s_ptr;
            if (Reg_Set.s_isfull) {		//����ǰ�Ĵ�����ֵ�����ڴ�  sw 9
                pushReg(Reg_Set.s_ptr);
            }
            Reg_Set.Regs[r_id].isEmpty = false;
            Reg_Set.Regs[r_id].name = name;
            Reg_Set.Regs[r_id].offAddr = offaddr;
            Reg_Set.Regs[r_id].regKind = regkind;
            if(Reg_Set.s_ptr == 15){ Reg_Set.s_ptr = 8;}
            else { Reg_Set.s_ptr ++;}
            if (Reg_Set.s_ptr == 8) {
                Reg_Set.s_ptr = 8+Alloc_SCount_In_Func;
                Reg_Set.s_isfull = true;		//�൱�� Reg_Set.ptr + 1 = 18��
            }
            return r_id;
	    }else if (regkind==1){//������ʱ���� 0-7 16��17
	    //�ж��Ƿ��ͻ
            if (Reg_Set.Regs[Reg_Set.t_ptr].isBusy) {
                if(Reg_Set.t_ptr==7){Reg_Set.t_ptr = 16;}
                else if(Reg_Set.t_ptr==17) {Reg_Set.t_ptr = 0;}
                else { Reg_Set.t_ptr ++;}
                if (Reg_Set.t_ptr == 0) {
                    Reg_Set.t_isfull = true;		//�൱�� Reg_Set.ptr + 1 = 18��
                }
            }
            int r_id = Reg_Set.t_ptr;
            if (Reg_Set.t_isfull) {		//����ǰ�Ĵ�����ֵ�����ڴ�  sw 9
                pushReg(Reg_Set.t_ptr);
            }
            Reg_Set.Regs[r_id].isEmpty = false;
            Reg_Set.Regs[r_id].name = name;
            Reg_Set.Regs[r_id].offAddr = offaddr;
            Reg_Set.Regs[r_id].regKind = regkind;
            if(Reg_Set.t_ptr==7){Reg_Set.t_ptr = 16;}
            else if(Reg_Set.t_ptr==17) {Reg_Set.t_ptr = 0;}
            else { Reg_Set.t_ptr ++;}
            if (Reg_Set.t_ptr == 0) {
                Reg_Set.t_isfull = true;		//�൱�� Reg_Set.ptr + 1 = 18��
            }
            return r_id;
	    }

	}

	void deleteBusy() {
		for (int i = 0; i < R_REG_NUM; i++) {
			Reg_Set.Regs[i].isBusy = false;
		}
	}

	//funcAllocRegs���õĹ��ܺ���
	void varCount(std::string name) {
		if (string_is_num(name) || (name[0] == '#') || (name == "*RET")) return;	//���Ǿֲ�����
		else {
			if (find_in_tab(name) == 0) {
				//std::cout << "There is a bug in midtomipsdeal--varCount: name not find in tab" << std::endl;
			}
			else {
				auto iter = S_Count_Map.find(name);
				if (iter == S_Count_Map.end()) {	//û�ҵ�
					S_Count_Map[name] = 1;
				}
				else {	//�ҵ������ô�ʱ+1
					iter->second++;
				}
			}
		}
	}

    bool cmp_by_value(const PAIR& lhs, const PAIR& rhs) {
		return lhs.second > rhs.second;
	}

	void loadScountMap(int mid_pos) {
		int i, r_id, t_pos;
		std::string reg_name;
		var_info tmp_vinfo;
		if (S_Count_Map.size() < S_GLOBAL_MAX) {	//С��9��ֱ�ӷ���
			//std::cout << "s_Count_Map size:" << S_Count_Map.size() << std::endl;
			for (auto iter = S_Count_Map.begin(); iter != S_Count_Map.end(); iter++) {
				t_pos = find_in_tab(iter->first);
				/*
				if (t_pos == 0) {
					std::cout << "There is a bug in midtomipsdeal--loadScountMap:%" << iter->first <<"%"<< std::endl;
					std::cout<<"mid_pos:"<<mid_pos <<MidCodeOptT.midcodeArray[mid_pos].y<< std::endl;
					std::cout<<display[level]<<std::endl;
				}
				*/
				//���ֲ�/ȫ�ֱ����ӿռ����ó���, lwָ��
				if (tab.tabArray[t_pos].lev == 1) {	//�ֲ�
					r_id = allocR_REG(iter->first, tab.tabArray[t_pos].adr, 2);
					reg_name = id_Trans_Regnum(r_id);
					Enter_Mips_List(8, reg_name, DATA_BASE, " ", GET_OFFSET(tab.tabArray[t_pos].adr));
				}
				else {		//ȫ��
					r_id = allocR_REG(iter->first, tab.tabArray[t_pos].adr, 3);
					reg_name = id_Trans_Regnum(r_id);
					Enter_Mips_List(8, reg_name, GLOBAL_BASE, " ", GET_OFFSET(tab.tabArray[t_pos].adr));
				}

				//�����ֲ�/ȫ�ֱ�����map�е���Ϣ
				tmp_vinfo.inReg = true;
				tmp_vinfo.r_addr = r_id;
				tmp_vinfo.stack_addr = tab.tabArray[t_pos].adr;
				S_Map[iter->first] = tmp_vinfo;
				Alloc_SCount_In_Func++;
			}
		}
		else {		//����8����������ô����϶��8��
			std::vector<PAIR> s_count_vec(S_Count_Map.begin(), S_Count_Map.end());
			sort(s_count_vec.begin(), s_count_vec.end(), cmp_by_value);
			for (i = 0; i < S_GLOBAL_MAX; ++i) {
				//std::cout << s_count_vec[i].first << ":" << s_count_vec[i].second << std::endl;
				t_pos = find_in_tab(s_count_vec[i].first);

				//���ֲ�/ȫ�ֱ����ӿռ����ó���, lwָ��
				if (tab.tabArray[t_pos].lev == 1) {	//�ֲ�
					r_id = allocR_REG(s_count_vec[i].first, tab.tabArray[t_pos].adr, 2);
					reg_name = id_Trans_Regnum(r_id);
					Enter_Mips_List(8, reg_name, DATA_BASE, " ", GET_OFFSET(tab.tabArray[t_pos].adr));
				}
				else {		//ȫ��
					r_id = allocR_REG(s_count_vec[i].first, tab.tabArray[t_pos].adr, 3);
					reg_name = id_Trans_Regnum(r_id);
					Enter_Mips_List(8, reg_name, GLOBAL_BASE, " ", GET_OFFSET(tab.tabArray[t_pos].adr));
				}

				//�����ֲ�/ȫ�ֱ�����map�е���Ϣ
				tmp_vinfo.inReg = true;
				tmp_vinfo.r_addr = r_id;
				tmp_vinfo.stack_addr = tab.tabArray[t_pos].adr;
				S_Map[s_count_vec[i].first] = tmp_vinfo;
				Alloc_SCount_In_Func++;

			}

		}
	}

		//�ں�������ʱ����, ʹ�����ü�������Ϊ�����ľֲ���������Ĵ���
	void funcAllocRegs(int func_start) {
		int func_end = 0;
		int i = 0;
		S_Count_Map.clear();
		//std::cout <<MidCodeOptT.midcodeArray[func_start].y << std::endl;
		Alloc_SCount_In_Func=0;
        //ɨ��ú������ڿ�
		for (i = func_start + 1; i < MidCodeOptT.mdc; i++) {
			if (MidCodeOptT.midcodeArray[i].op == 14) break;
		}
		func_end = i;

		//ɨ��ÿ������б���
		for (i = func_start + 1; i < func_end; i++) {
			midcode t = MidCodeOptT.midcodeArray[i];
			if ((t.op >= 0) && (t.op < 4)) {	//+ - * /
				varCount(t.z);
				varCount(t.x);
				varCount(t.y);
			}
			else if (t.op == 4) {	// ��ֵ
				if (t.x == " ") {	//��ͨ��ֵ
					varCount(t.z);
					varCount(t.y);
				}
				else {	//�����������ֵ
					varCount(t.z);
				}
			}
			else if ((t.op > 5) && (t.op < 12)) {
				varCount(t.x);
				varCount(t.y);
			}
			else if ((t.op == 12) || (t.op == 16) || (t.op == 17) || (t.op == 18) || (t.op == 20)) {
				if (t.y != " ") {
					varCount(t.y);
				}
			}
		}
		loadScountMap(func_start);
	}

    void returnReg(int regkind){
        if(regkind==1){//��ʱ 0-7  16 17
           // std::cout << "�黹��ʱ����"<< std::endl;
            if(Reg_Set.t_ptr==16){Reg_Set.t_ptr=7;}
            else if(Reg_Set.t_ptr==0) {Reg_Set.t_ptr=17;}
            else {Reg_Set.t_ptr--;}
          /*  if (Reg_Set.Regs[Reg_Set.t_ptr].regKind == 1) {
					T_Map[Reg_Set.Regs[Reg_Set.t_ptr].name].inReg = false;	//���ڼĴ�����
				}
				else if ((Reg_Set.Regs[Reg_Set.t_ptr].regKind == 2) || (Reg_Set.Regs[Reg_Set.t_ptr].regKind == 3)) {
					S_Map[Reg_Set.Regs[Reg_Set.t_ptr].name].inReg = false;	//���ڼĴ�����
				}
				Reg_Set.Regs[Reg_Set.t_ptr].isEmpty = true;
				Reg_Set.Regs[Reg_Set.t_ptr].name = " ";
				Reg_Set.Regs[Reg_Set.t_ptr].offAddr = -1;
				Reg_Set.Regs[Reg_Set.t_ptr].regKind = -1;
				Reg_Set.Regs[Reg_Set.t_ptr].isBusy = false;*/

        }
        else if((regkind==2)||(regkind==3)){//ȫ�� 8-15
           // std::cout << "�黹ȫ�ֱ���"<< std::endl;
             if(Reg_Set.s_ptr==8){Reg_Set.s_ptr=15;}
            else{Reg_Set.s_ptr--;}
          /*  if (Reg_Set.Regs[Reg_Set.s_ptr].regKind == 1) {
					T_Map[Reg_Set.Regs[Reg_Set.s_ptr].name].inReg = false;	//���ڼĴ�����
				}
				else if ((Reg_Set.Regs[Reg_Set.s_ptr].regKind == 2) || (Reg_Set.Regs[Reg_Set.s_ptr].regKind == 3)) {
					S_Map[Reg_Set.Regs[Reg_Set.s_ptr].name].inReg = false;	//���ڼĴ�����
				}
				Reg_Set.Regs[Reg_Set.s_ptr].isEmpty = true;
				Reg_Set.Regs[Reg_Set.s_ptr].name = " ";
				Reg_Set.Regs[Reg_Set.s_ptr].offAddr = -1;
				Reg_Set.Regs[Reg_Set.s_ptr].regKind = -1;
				Reg_Set.Regs[Reg_Set.s_ptr].isBusy = false;*/

        }
    }
		/*
	��������: ����z, y, x
	����˵��:	s Ϊ��������z��y ���� x
	reg_nameΪ����ļĴ�������, ��Ϊ��ʱ�������ֲ���������ȫ�ֱ�����  reg_name = ���ش洢�ļĴ���, ���򲻴���
	num  Ϊ�������ʱ����, ��Ϊ��ʱ������  num = �������ֵ�ֵ, ���򲻶�����д���
	����ֵ: ����ֵΪ1˵��reg_name��ֵ, ����ֵΪ2˵��num��ֵ(��ʱ����), ����ֵΪ0˵���д�
	*/
	int dealOperand(std::string s, std::string& reg_name, int&num) {
		var_info tmp_vinfo;
		std::string array_name;

		if (s == "*RET") {		//����ֵ
			reg_name = RE_VALUE_REG;
			return 1;
		}

		if (string_is_num(s)) {		//��ʱ����
			num = string_to_int(s);
			return 2;
		}
		else if (s[0] == '#') {			//��ʱ����
			if (findInMap(s, 0, tmp_vinfo)) {		//����T_Map��
				if (tmp_vinfo.inReg) {		//�ڼĴ�����
					reg_name = id_Trans_Regnum(tmp_vinfo.r_addr);
					Reg_Set.Regs[tmp_vinfo.r_addr].isBusy = true;
				}
				else {							//���ڴ���
                  //  std::cout<<tmp_vinfo.stack_addr<<std::endl;
					int r_id = allocR_REG(s, tmp_vinfo.stack_addr, 1);
					reg_name = id_Trans_Regnum(r_id);

					Reg_Set.Regs[r_id].isBusy = true;

					//����ʱ������ջ�ռ����ó���, lwָ��
					if (tmp_vinfo.isTFirst) {
						T_Map[s].isTFirst = false;
					}
					else {
						Enter_Mips_List(8, reg_name, DATA_BASE, " ", GET_OFFSET(tmp_vinfo.stack_addr));
					}
					//������ʱ������map�е���Ϣ
					T_Map[s].inReg = true;
					T_Map[s].r_addr = r_id;
				}
			}
			else {
				//std::cout << "There is a bug in midtomipsdeal--dealOperand, T not find in T_Map" << std::endl;
			}
			return 1;
		}
		else {								//�ֲ�����/ȫ�ֱ���
            if(s=="i_1"){
                //std::cout <<"i_1����"<<std::endl;
            }
			if (findInMap(s, 1, tmp_vinfo)) {
                    if(s=="i_1"){
               // std::cout <<"�ڱ���"<<std::endl;
            }
				if (tmp_vinfo.inReg) {		//�ڼĴ�����
				     if(s=="i_1"){
               // std::cout <<"�ڼĴ�����"<<std::endl;
            }
					reg_name = id_Trans_Regnum(tmp_vinfo.r_addr);
					Reg_Set.Regs[tmp_vinfo.r_addr].isBusy = true;
				}
				else {							//���ڴ���
				    if(s=="i_1"){
                //std::cout <<"���ڴ���"<<std::endl;
            }
					int t_pos;	//�ֲ������ڷ��ű��е�λ��
					t_pos = find_in_tab(s);
					if (t_pos == 0) {
						//std::cout << "There is a bug in midtomipsdeal--dealOperand, S-InMap-t_pos can't be 0" << std::endl;
						return 0;
					}
					int r_id;

					//���ֲ�/ȫ�ֱ�����ջ�ռ����ó���, lwָ��
					if (tab.tabArray[t_pos].lev == 1) {	//�ֲ�

						r_id = allocR_REG(s, tmp_vinfo.stack_addr, 2);
						//if(s=="b"){std::cout <<r_id << std::endl;}
						reg_name = id_Trans_Regnum(r_id);
						// if(s=="b"){std::cout <<reg_name << std::endl;}
						Reg_Set.Regs[r_id].isBusy = true;
						Enter_Mips_List(8, reg_name, DATA_BASE, " ", GET_OFFSET(tab.tabArray[t_pos].adr));
					}
					else {		//ȫ��
						r_id = allocR_REG(s, tmp_vinfo.stack_addr, 3);
						reg_name = id_Trans_Regnum(r_id);
						Reg_Set.Regs[r_id].isBusy = true;
						Enter_Mips_List(8, reg_name, GLOBAL_BASE, " ", GET_OFFSET(tab.tabArray[t_pos].adr));
					}

					//���ľֲ�/ȫ�ֱ�����map�е���Ϣ
					S_Map[s].inReg = true;
					S_Map[s].r_addr = r_id;
				}
			}
			else {	//��һ�γ��ָþֲ�����/ȫ�ֱ���
                    if(s=="i_1"){
              //  std::cout <<"��һ�γ���i_1"<<std::endl;
            }
				int t_pos;	//�ֲ������ڷ��ű��е�λ��
				t_pos = find_in_tab(s);
				if (t_pos == 0) {
					//std::cout << "There is a bug in midtomipsdeal--dealOperand, S-NotInMap-t_pos can't be 0" << std::endl;
					return 0;
				}

				int r_id;


				//���ֲ�/ȫ�ֱ����ӿռ����ó���, lwָ��
				if (tab.tabArray[t_pos].lev == 1) {	//�ֲ�
				    if(s=="i_1"){
               // std::cout <<"i_1�Ǿֲ�����"<<std::endl;
            }
					r_id = allocR_REG(s, tab.tabArray[t_pos].adr, 2);
					reg_name = id_Trans_Regnum(r_id);
				//	if(s=="b"){std::cout <<r_id << std::endl;}

				//		 if(s=="b"){std::cout <<reg_name << std::endl;}
					Reg_Set.Regs[r_id].isBusy = true;
					Enter_Mips_List(8, reg_name, DATA_BASE, " ", GET_OFFSET(tab.tabArray[t_pos].adr));
				}
				else {		//ȫ��
					r_id = allocR_REG(s, tab.tabArray[t_pos].adr, 3);
					reg_name = id_Trans_Regnum(r_id);
					Reg_Set.Regs[r_id].isBusy = true;
					Enter_Mips_List(8, reg_name, GLOBAL_BASE, " ", GET_OFFSET(tab.tabArray[t_pos].adr));
				}

				//�����ֲ�/ȫ�ֱ�����map�е���Ϣ
				tmp_vinfo.inReg = true;
				tmp_vinfo.r_addr = r_id;
				tmp_vinfo.stack_addr = tab.tabArray[t_pos].adr;
				S_Map[s] = tmp_vinfo;
				if(s=="i_1"){
              //  std::cout <<"tmp_vinfo.inreg="<<tmp_vinfo.inReg<<std::endl;
            }
			}
			return 1;
		}
	}

		// +��-��*��/ �Ĵ���, �������Ϊ�ڼ���ָ��
	void calcuDeal(midcode cur_mid, int mid_pos) {
		std::string z = " ", x = " ", y = " ";
		int z_flag, x_flag, y_flag;
		int result, r_id;		//result x��y��Ϊ���ֵ�ʱ��ֱ�Ӽ������,  r_id �������߼��� xΪ���ֵ�ʱ������Ĵ���ʹ��
		int z_num, x_num, y_num;
		z_flag = dealOperand(cur_mid.z, z, z_num);
		//if (z_flag != 1) std::cout << "There is a bug in midtomipsdeal--calcuDeal, z_flag must be 1. mid_pos:" << mid_pos << std::endl;
		x_flag = dealOperand(cur_mid.x, x, x_num);
		//if (x_flag == 0) std::cout << "There is a bug in midtomipsdeal--calcuDeal, x_flag can't be 0. mid_pos:" << mid_pos << std::endl;
		y_flag = dealOperand(cur_mid.y, y, y_num);
		//if (y_flag == 0) std::cout << "There is a bug in midtomipsdeal--calcuDeal, y_flag can't be 0. mid_pos:" << mid_pos << std::endl;

		switch (cur_mid.op) {
		case 0:	//+
			if ((x_flag == 2) && (y_flag == 2)) {		//ֱ�Ӽ�������
				result = x_num + y_num;
				Enter_Mips_List(19, z, " ", " ", result);					//����liָ��
			}
			else if ((x_flag == 2) && (y_flag != 2)) {
				Enter_Mips_List(4, z, y, " ", x_num);
			}
			else if ((y_flag == 2) && (x_flag != 2)) {
				Enter_Mips_List(4, z, x, " ", y_num);
			}
			else {
				Enter_Mips_List(0, z, x, y, 0);
			}
			break;
		case 1:	//-
			if ((x_flag == 2) && (y_flag == 2)) {		//ֱ�Ӽ�������
				result = x_num - y_num;
				Enter_Mips_List(19, z, " ", " ", result);					//����liָ��
			}
			else if ((x_flag == 2) && (y_flag != 2)) {			// z = x - y   xΪ����
			  //  temp_cnt--;
				r_id = allocR_REG(" ", -1, 1);
				x = id_Trans_Regnum(r_id);
				Enter_Mips_List(19, x, " ", " ", x_num);
				Enter_Mips_List(1, z, x, y, 0);

				//�ͷŸüĴ���
				returnReg(1);
			}
			else if ((y_flag == 2) && (x_flag != 2)) {
				Enter_Mips_List(5, z, x, " ", y_num);				//z = x - y   y Ϊ����
			}
			else {
				Enter_Mips_List(1, z, x, y, 0);
			}
			break;
		case 2:	//*
			if ((x_flag == 2) && (y_flag == 2)) {		//ֱ�Ӽ�������
				result = x_num * y_num;
				Enter_Mips_List(19, z, " ", " ", result);					//����liָ��
			}
			else if ((x_flag == 2) && (y_flag != 2)) {
				Enter_Mips_List(6, z, y, " ", x_num);
			}
			else if ((y_flag == 2) && (x_flag != 2)) {
				Enter_Mips_List(6, z, x, " ", y_num);
			}
			else {
				Enter_Mips_List(2, z, x, y, 0);
			}
			break;
		case 3:	// /
			if ((x_flag == 2) && (y_flag == 2)) {		//ֱ�Ӽ�������
				result = x_num / y_num;
				Enter_Mips_List(19, z, " ", " ", result);					//����liָ��
			}
			else if ((x_flag == 2) && (y_flag != 2)) {			// z = x / y   xΪ����
			   // temp_cnt--;
				r_id = allocR_REG(" ", -1, 1);
				x = id_Trans_Regnum(r_id);
				Enter_Mips_List(19, x, " ", " ", x_num);
				Enter_Mips_List(3, z, x, y, 0);
				//�ͷŸüĴ���
				returnReg(1);
			}
			else if ((y_flag == 2) && (x_flag != 2)) {
				if (y_num == 0) {	//����Ϊ0
					y_num = 1;
				}
				Enter_Mips_List(7, z, x, " ", y_num);				//z = x / y   y Ϊ����
			}
			else {
				Enter_Mips_List(3, z, x, y, 0);
			}
			break;
		default:
			break;
		}
	}

		//��ֵָ���
	void assignDeal(midcode cur_mid, int mid_pos) {
		std::string z = " ", x = " ", y = " ", t = " ";
		int z_flag, x_flag, y_flag;
		int t_pos, r_id;		//result x��y��Ϊ���ֵ�ʱ��ֱ�Ӽ������,  r_id �������߼��� xΪ���ֵ�ʱ������Ĵ���ʹ��
		int z_num, x_num, y_num;

		switch (cur_mid.op) {
		case 4:				// z = y ���� z = y[x]
			z_flag = dealOperand(cur_mid.z, z, z_num);
			//if (z_flag != 1) std::cout << "There is a bug in midtomipsdeal--assignDeal, switch-4-z_flag must be 1. mid_pos:" << mid_pos << std::endl;
			if (cur_mid.x == " ") {		//z = y�ĸ�ֵ
				y_flag = dealOperand(cur_mid.y, y, y_num);
				//if (y_flag == 0) std::cout << "There is a bug in midtomipsdeal--assignDeal, switch-4-y_flag can't be 0. mid_pos:" << mid_pos << std::endl;
				if (y_flag == 1) {
					Enter_Mips_List(20, z, y, " ", 0);	//move
				}
				else {
					//std::cout << mid_pos << " " << cur_mid.z << " " << cur_mid.y << std::endl;
					Enter_Mips_List(19, z, " ", " ", y_num);
				}
			}
			else {		// z = y[x]�ĸ�ֵ		lw
				t_pos = find_in_tab(cur_mid.y);
				//if (t_pos == 0) std::cout << "There is a bug in midtomipsdeal--assignDeal, switch-4-t_pos can't be 0. mid_pos:" << mid_pos << std::endl;
				y_num = tab.tabArray[t_pos].adr;	//�����һ��Ԫ�ص���Ե�ַ

				x_flag = dealOperand(cur_mid.x, x, x_num);
				//if (x_flag == 0) std::cout << "There is a bug in midtomipsdeal--assignDeal, switch-4-x_flag can't be 0. mid_pos" << mid_pos << std::endl;
				if (x_flag == 2) {			// xΪ����ʱ��

					y_num = y_num + x_num;
					if (tab.tabArray[t_pos].lev == 1) {		//�ֲ�
						Enter_Mips_List(8, z, DATA_BASE, " ", GET_OFFSET(y_num));					//lw
					}
					else {									//ȫ��
						Enter_Mips_List(8, z, GLOBAL_BASE, " ", GET_OFFSET(y_num));					//lw
					}
				}
				else {	//x Ϊ������     $fp/$gp + GET_OFFSET(y_num + $x)
                   //     temp_cnt--;
					r_id = allocR_REG(" ", -1, 1);

					t = id_Trans_Regnum(r_id);

					//std::cout <<t << std::endl;
					//std::cout <<Reg_Set.Regs[r_id].isEmpty << std::endl;
					Enter_Mips_List(4, t, x, " ", y_num);
					Enter_Mips_List(6, t, t, " ", -4);		// -4 * (y_num + $x)
					if (tab.tabArray[t_pos].lev == 1) {		//�ֲ�
						Enter_Mips_List(0, t, t, DATA_BASE, 0);
						Enter_Mips_List(8, z, t, " ", 0);
					}
					else {			//ȫ��
						Enter_Mips_List(0, t, t, GLOBAL_BASE, 0);
						Enter_Mips_List(8, z, t, " ", 0);
					}
					//�ͷŸüĴ���
					if(r_id==6){
                       // std::cout <<Reg_Set.Regs[r_id].isEmpty << std::endl;
					}
					returnReg(1);
					//�ͷŸüĴ���
					if(Reg_Set.t_ptr==6){
                        //std::cout <<Reg_Set.Regs[Reg_Set.t_ptr].isEmpty << std::endl;
					}
				}
			}
			break;
		case 20:			//z[x] = y   sw
			t_pos = find_in_tab(cur_mid.z);
			//if (t_pos == 0) std::cout << "There is a bug in midtomipsdeal--assignDeal, switch-20-t_pos can't be 0. mid_pos:" << mid_pos << std::endl;
			z_num = tab.tabArray[t_pos].adr;
			x_flag = dealOperand(cur_mid.x, x, x_num);
			//if (x_flag == 0) std::cout << "There is a bug in midtomipsdeal--assignDeal, switch-20-x_flag can't be 0. mid_pos" << mid_pos << std::endl;


			y_flag = dealOperand(cur_mid.y, y, y_num);
			//if (y_flag == 0) std::cout << "There is a bug in midtomipsdeal--assignDeal, switch-20-y_flag can't be 0. mid_pos" << mid_pos << std::endl;
			if (y_flag == 2) {	//����
			   // std::cout << Reg_Set.t_ptr << std::endl;
			 //  temp_cnt--;
				r_id = allocR_REG(" ", -1, 1);

               // std::cout << r_id << std::endl;
				y = id_Trans_Regnum(r_id);

				Enter_Mips_List(19, y, " ", " ", y_num);		//li
															//�ͷŸüĴ���
															//if (Reg_Set.ptr == 0) Reg_Set.ptr = 17;
															//else Reg_Set.ptr--;
			}

			// ����y ΪҪ�洢��ջ�ռ�ļĴ��� ���ܱ��ͷţ�����Ҫ��֮��xΪ����ʱ���ֱ�����뵼�³��֮ǰ��y

			if (x_flag == 2) {		//xΪ����
				z_num = z_num + x_num;

				if (tab.tabArray[t_pos].lev == 1) {	//�ֲ�
					Enter_Mips_List(9, y, DATA_BASE, " ", GET_OFFSET(z_num));
				}
				else {		//ȫ��
					Enter_Mips_List(9, y, GLOBAL_BASE, " ", GET_OFFSET(z_num));
				}
				returnReg(1);
			}
			else {		// ��ʱxΪ����
                   // temp_cnt--;
				r_id = allocR_REG(" ", -1, 1);
				t = id_Trans_Regnum(r_id);
				Enter_Mips_List(4, t, x, " ", z_num);
				Enter_Mips_List(6, t, t, " ", -4);		// -4 * (z_num + $x)
				if (tab.tabArray[t_pos].lev == 1) {		//�ֲ�
					Enter_Mips_List(0, t, t, DATA_BASE, 0);
					Enter_Mips_List(9, y, t, " ", 0);
				}
				else {			//ȫ��
					Enter_Mips_List(0, t, t, GLOBAL_BASE, 0);
					Enter_Mips_List(9, y, t, " ", 0);
				}
				//�ͷŸüĴ��� ?????
				returnReg(1);
			//	returnReg(1);
			}
			break;
		default:
			//std::cout << "There is a bug in midtomipsdeal--assignDeal, switch--default. mid_pos:" << mid_pos << std::endl;
			break;
		}
	}

		//��������תָ���
	void noConJumpDeal(midcode cur_mid, int mid_pos) {
		std::string y;			//y��������retʱʹ��
		int offset_1, offset_2, t_pos, b_pos; //��jal���õ�
		int  y_flag, y_num;
		switch (cur_mid.op) {
		case 5:	//goto
			clearRegs(3);
			Enter_Mips_List(16, cur_mid.z, " ", " ", 0);		//j label
			break;
		case 12: //ret
			if (cur_mid.y != " ") {		//������ֵ
				y_flag = dealOperand(cur_mid.y, y, y_num);
				//if (y_flag == 0) std::cout << "There is a bug in midtomipsdeal--noConJumpDeal, switch-12." << std::endl;
				if (y_flag == 1) {
					Enter_Mips_List(20, RE_VALUE_REG, y, " ", 0);
				}
				else {
					Enter_Mips_List(19, RE_VALUE_REG, " ", " ", y_num);
				}
			}
			clearRegs(2);	//����Ĵ���
			//S_Map.clear();
			//clearRegs(3);
			Enter_Mips_List(17, RE_LABEL_REG, " ", " ", 0);
			break;
		case 21: //jal
			clearRegs(1);		//��������ʱ��ת֮ǰ���ֳ�����, ���Ĵ������
			S_Map.clear();		//���S_Map


			t_pos = find_in_tab(Call_Func_Name[--Call_Func_Ptr]);
			//if (t_pos == 0) std::cout << "There is a bug in midtomipsdeal--noConditionJumpDeal, switch--21" << std::endl;
			b_pos = tab.tabArray[t_pos].ref;
			offset_1 = 3 + btab.btabArray[b_pos].paranum;	//fp = sp - (3+paranum)
			offset_2 = btab.btabArray[b_pos].varsize + btab.btabArray[b_pos].t_varnum;

			Enter_Mips_List(5, DATA_BASE, STACK_TOP, " ", GET_OFFSET(offset_1));			// �ƶ���ַfp = sp-3-paranum
			Enter_Mips_List(4, STACK_TOP, STACK_TOP, " ", GET_OFFSET(offset_2));		//addi �ƶ�ջ��ָ��
			Enter_Mips_List(18, cur_mid.z, " ", " ", 0);		//jal label
			//Enter_Mips_List(5, STACK_TOP, STACK_TOP, " ", GET_OFFSET(offset_2));		//addi �ƶ�ջ��ָ��
          //  Enter_Mips_List(4, DATA_BASE, STACK_TOP, " ", GET_OFFSET(offset_1));			// �ƶ���ַfp = sp-3-paranum
			break;
		default:
			//std::cout << "There is a bug in midtomipsdeal--noConJumpDeal, switch--default" << std::endl;
			break;
		}

	}

		//������תָ���
	void conditionJumpDeal(midcode cur_mid, int mid_pos) {
		std::string  x, y;
		int op;
		int x_flag, y_flag;
		int r_id;		//r_id �������߼��� xΪ���ֵ�ʱ������Ĵ���ʹ��
		int x_num, y_num;
        var_info tmp_info;
		op = cur_mid.op + 4;
		//if (op < 10 || op > 15) std::cout << "There is a bug in midThere is a bug in midtomipsdeal--conditionJumpDeal, op out of range. mid_pos:" << mid_pos << std::endl;
         if(cur_mid.x=="i_1"){
            findInMap(cur_mid.x, 1, tmp_info);
        //    std::cout<<"���x��s�е�inreg"<<tmp_info.inReg<<std::endl;
           // std::cout<<"tep_info�еļĴ���id"<<tmp_info.r_addr<<std::endl;
         }
		x_flag = dealOperand(cur_mid.x, x, x_num);
		//if (x_flag == 0) std::cout << "There is a bug in midtomipsdeal--conditionJumpDeal, x_flag can't be 0. mid_pos:" << mid_pos << std::endl;
		y_flag = dealOperand(cur_mid.y, y, y_num);
		//if (y_flag == 0) std::cout << "There is a bug in midtomipsdeal--conditionJumpDeal, y_flag can't be 0. mid_pos:" << mid_pos << std::endl;


		if (x_flag == 2) {		//xΪ����
		    //temp_cnt--;
			r_id = allocR_REG(" ",-1, 1);
			x = id_Trans_Regnum(r_id);
			Enter_Mips_List(19, x, " ", " ", x_num); //��ֵ���ص��Ĵ�����
												   //�ͷŸüĴ���
			returnReg(1);
		}

		clearRegs(3);	//��ת֮ǰ��ռĴ���

		if (y_flag == 2) {			//yΪ����
			Enter_Mips_List(op, cur_mid.z, x, " ", y_num);  // x��offset�Ƚ�
		}
		else {
			Enter_Mips_List(op, cur_mid.z, x, y, 0);		// x �� y �Ƚ�
		}

	}


	//��ǩ�Ĵ���
	void setlabDeal(midcode cur_mid, int mid_pos, bool opt) {
		int offset, t_pos, b_pos;			// b_posΪ������btab����ƫ��
		int i = 0;
		bool loadS = false;
		var_info tmp_info;
		switch (cur_mid.isstart) {
		case 1:			//��ͨ��lab��ǩ
            findInMap("i_1", 1, tmp_info);
          //  std::cout<<"������ñ�ǩǰx��s�е�inreg"<<tmp_info.inReg<<std::endl;
           // std::cout<<"tep_info�еļĴ���id"<<tmp_info.r_addr<<std::endl;

			clearRegs(3);		//��ռĴ���(���������Ŀ���)
            findInMap("i_1", 1, tmp_info);
          //  std::cout<<"������ñ�ǩ��3��ռĴ�����x��s�е�inreg"<<tmp_info.inReg<<std::endl;
			Enter_Mips_List(22, cur_mid.y, " ", " ", 0);

			break;
		case 2:			//main ������ڱ�ǩ, �ڴ˴�����main�ռ��ָ���ƶ�, ��Ϊû�к���call main������������������call��ʱ����
			Enter_Mips_List(22, cur_mid.y, " ", " ", 0);
			t_pos = find_in_tab("main");
			b_pos = tab.tabArray[t_pos].ref;
			offset = 3 + btab.btabArray[b_pos].varsize + btab.btabArray[b_pos].t_varnum;
			Enter_Mips_List(23, RE_LABEL_REG, "exit", " ", 0);				//���ó������
			Enter_Mips_List(20, DATA_BASE, STACK_TOP, " ", 0);
			//addi ����
			Enter_Mips_List(4, STACK_TOP, STACK_TOP, " ", GET_OFFSET(offset));
			if (opt) funcAllocRegs(mid_pos);
			break;
		case 3:			//call�������淵�ر�ǩ

			//clearRegs(2);	//����Ĵ���
			//S_Map.clear();

			Enter_Mips_List(22, cur_mid.y, " ", " ", 0);
			Enter_Mips_List(20, STACK_TOP, DATA_BASE, " ", 0);				//�ָ�ջ���ռ�
			Enter_Mips_List(8, DATA_BASE, DATA_BASE, " ", GET_OFFSET(1));		// �ָ����ú�����fp
			Enter_Mips_List(8, RE_LABEL_REG, DATA_BASE, " ", GET_OFFSET(0));		//�ָ����ú����ķ��ص�ַ
			//Enter_Mips_List(8, DATA_BASE, STACK_TOP, " ", GET_OFFSET(1));		// �ָ����ú�����fp
			//Enter_Mips_List(8, RE_LABEL_REG, DATA_BASE, " ", GET_OFFSET(0));		//�ָ����ú����ķ��ص�ַ

			//if(opt) funcAllocRegs(mid_pos);
			if (opt && mid_pos + 1 != MidCodeOptT.mdc) loadScountMap(mid_pos);
			break;
		case 4:				//������ʼ��ǩ, ֻ�Ǵ�ӡ��ǩ��������
			Enter_Mips_List(22, cur_mid.y, " ", " ", 0);
			//����ȫ�ּĴ���
			if (opt) funcAllocRegs(mid_pos);

			break;
		default:
			//std::cout << "There is a bug in midtomipsdeal--setlabDeal, switch--default. mid_pos:" << mid_pos << std::endl;
			break;
		}
	}

		//���������Ĵ���
	void funcDeal(midcode cur_mid, int mid_pos, bool opt) {
		int t_pos, b_pos;
		t_pos = find_in_tab(cur_mid.y);
		b_pos = tab.tabArray[t_pos].ref;
		display[level] = b_pos;				//���õ�ǰ���ݾֲ���
		clearRegs(0);
		S_Map.clear();

	}

		//����Ĵ���
	void inDeal(midcode cur_mid, int mid_pos) {
		std::string y;
		int t_pos, y_flag, y_num, v0_para;		//���ݱ�ʶ�������Ա����������ַ�(11)��������(5)
		t_pos = find_in_tab(cur_mid.y);
		if (t_pos == 0) {
			//std::cout << "There is a bug in midtomips--inDeal, t_pos can't be 0" << std::endl;
			return;			//��ʶ��δ�����򷵻�
		}
		y_flag = dealOperand(cur_mid.y, y, y_num);
		//if (y_flag != 1) std::cout << "There is a bug in midtomipsdeal--inDeal, y_flag must be 1. mid_pos:" << mid_pos << std::endl;

		//�ж�����ı�ʶ�����������������ַ�
		if (tab.tabArray[t_pos].type == ints) {
			v0_para = 5;
		}
		else if (tab.tabArray[t_pos].type == chars) {
			v0_para = 12;
		}
		else {
			//std::cout << "There is a bug in midtomipsdeal--inDeal, y not ints or chars. mid_pos:" << mid_pos << std::endl;
		}
		Enter_Mips_List(19, "$v0", " ", " ", v0_para);
		Enter_Mips_List(21, " ", " ", " ", 0);
		Enter_Mips_List(20, y, "$v0", " ", 0);
	}

		//����Ĵ���
	void outDeal(midcode cur_mid, int mid_pos) {
		std::string x, y, x_label;
		int y_flag, y_num;
		if (cur_mid.x != " ") {	//���ַ���
			x = cur_mid.x;
			x_label = Get_Label_For_Str(x);
			Enter_Mips_List(19, "$v0", " ", " ", 4);
			Enter_Mips_List(23, "$a0", x_label, " ", 0); //???
			Enter_Mips_List(21, " ", " ", " ", 0);
		}
		if (cur_mid.y != " ") {	//�б��ʽ
			y_flag = dealOperand(cur_mid.y, y, y_num);
			if (cur_mid.z != " ") {	//z == char  ��ӡ�ַ�
				Enter_Mips_List(19, "$v0", " ", " ", 11);
			}
			else {
				Enter_Mips_List(19, "$v0", " ", " ", 1);
			}
			if (y_flag == 2) {
				Enter_Mips_List(19, "$a0", " ", " ", y_num);
			}
			else {
				Enter_Mips_List(20, "$a0", y, " ", 0);
			}
			Enter_Mips_List(21, " ", " ", " ", 0);
		}

		//��ÿһ������������ӡ���з�
		Enter_Mips_List(19, "$v0", " ", " ", 11);
		Enter_Mips_List(19, "$a0", " ", " ", '\n');
		Enter_Mips_List(21, " ", " ", " ", 0);
	}

		//���ݲ�������,�����������ڴ��� sw 9
	void passparaDeal(midcode cur_mid, int mid_pos) {
		std::string y;
		int y_flag, y_num;
		y_flag = dealOperand(cur_mid.y, y, y_num);
		if (y_flag == 0) {
			//std::cout << "There is a bug in midtomipsdeal--passparaDeal, y_flag can't be 0. mid_pos:" << mid_pos << std::endl;
			return;
		}
		if (y_flag == 2) {		//����
			Enter_Mips_List(19, "$a1", " ", " ", y_num);			// ��ʱ��ջ��spָ���������ʼ�ط�
			Enter_Mips_List(9, "$a1", STACK_TOP, " ", 0);
			Enter_Mips_List(4, STACK_TOP, STACK_TOP, " ", GET_OFFSET(1));		//addi ջָ������
		}
		else {
			Enter_Mips_List(9, y, STACK_TOP, " ", 0);
			Enter_Mips_List(4, STACK_TOP, STACK_TOP, " ", GET_OFFSET(1));		//addi ջָ������1��
		}
	}


		//�������õĴ���
	void callDeal(midcode cur_mid, int mid_pos) {
		//ParaOffset = 3;
		if (Call_Func_Ptr == CALL_MAX) {
			std::cout << "midtomipsdeal---get CALL_MAX, and exit" << std::endl;
			exit(1);
		}
		Call_Func_Name[Call_Func_Ptr++] = cur_mid.y;
		Enter_Mips_List(9, RE_LABEL_REG, DATA_BASE, " ", GET_OFFSET(0));		//����ǰ$ra������ԭ����
		Enter_Mips_List(9, DATA_BASE, STACK_TOP, " ", GET_OFFSET(1));		//���õ�ǰ�����ú�����prev_abp
		Enter_Mips_List(4, STACK_TOP, STACK_TOP, " ", GET_OFFSET(3));		//addi �ƶ�ջ��ָ������������������
	}

		//���ļ���������
	void midtomips(int kind) {
		midcode cur_mid;
		int midend;
		level = 1;
		int Alloc_SCount_In_Func = 0;	//��¼��ǰ���������˼���ȫ�ּĴ���, ��ÿ�δ�������ʼʱ��0
		//��ʼ������
		clearRegs(0);
		MipsTable.mpc = 0;
		S_Map.clear();
		S_Count_Map.clear();
		ParaOffset = 0;	//��ÿ��call��ʱ�����, ��parapassDeal��ʹ�ò�����������ʾ��ǰʵ��Ӧ��ѹ���λ��
		Call_Func_Ptr = 0;
		Str_Id = 0;	//����string��ǩʹ��
		for (auto iter = T_Map.begin(); iter != T_Map.end(); iter++) {
			iter->second.isTFirst = true;
		}


		Str_Info_Map.clear();
		if (kind == 0) midend = MidCodeT.mdc;
		//if (kind == 0) midend = MidCodeOptT.mdc;
		else if (kind == 1) midend = MidCodeOptT.mdc;


		for (int i = 0; i < midend; i++) {
			if (kind == 0) cur_mid = MidCodeT.midcodeArray[i];
			//if (kind == 0) cur_mid = MidCodeOptT.midcodeArray[i];
			else if(kind == 1) cur_mid = MidCodeOptT.midcodeArray[i];
			switch (cur_mid.op) {
			case 0:		// +��-��*��/ ��������Ĵ���
			case 1:
			case 2:
			case 3:
				calcuDeal(cur_mid, i);
				break;
			case 4:
			case 20:
				assignDeal(cur_mid, i);
				break;
			case 5:		//��������ת���Ĵ���
			case 12:
			case 21:
				noConJumpDeal(cur_mid, i);
				break;
			case 6://������ת���Ĵ���
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
				conditionJumpDeal(cur_mid, i);
				break;
			case 13:						//setlab �Ĵ���
				if(kind == 0)setlabDeal(cur_mid, i, false);
				else if(kind == 1) setlabDeal(cur_mid, i, true);
				break;
			case 14:					//�Ժ��������Ĵ���, ���õ�ǰdisplay��Ϊ�ú���������Ӧ�Ŀռ�
				if(kind == 0) funcDeal(cur_mid, i, false);
				else if (kind == 1) funcDeal(cur_mid, i, true);
				break;
			case 15:					//�Բ��������Ĵ���, ֱ���Թ�
				break;
			case 16:						//������Ĵ���
				inDeal(cur_mid, i);
				break;
			case 17:						//������Ĵ���
				outDeal(cur_mid, i);
				break;
			case 18:						//���ݲ�����ʱ��ʹ��
				passparaDeal(cur_mid, i);
				break;
			case 19:						//�Ժ�������call�Ĵ���
				callDeal(cur_mid, i);
				break;
			default:
				//std::cout << "There is a bug in midtomipsdeal--midtomips, switch--default" << std::endl;
				break;
			}
			//ÿһ�д���������busy
			deleteBusy();
		}
	}



}
