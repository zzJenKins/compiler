/*
中间代码转为mips代码
*/
#include "Mid_to_Mips.h"
#define	GET_REG_ID(n) n+8			//从reg数组下标得到寄存器的实际编号
#define GET_OFFSET(n) -4*n
#define STACK_TOP	"$sp" //堆栈
#define DATA_BASE	"$fp" //数据区
#define GLOBAL_BASE	"$gp"
#define RE_LABEL_REG		"$ra"// jal
#define RE_VALUE_REG		"$v1" //返回值
#define A_REG_NUM   4
#define R_REG_NUM	18
#define CALL_MAX 100
#define S_GLOBAL_MAX 10

typedef std::pair<std::string, int> PAIR;

namespace compiler {
	//fp寄存器指向当前数据区基址
	//gp、fp与sp都是从上到下的,即一开始栈顶在低地址, 栈底在高地址  所以栈指针移动需要用subi达到目的,对于相关的偏移也要用负数
	//把t寄存器与s寄存器统一处理  (8 - 25)
	//全局变量从gp中lw进来

	//每一次函数栈指针移动时记得更改display[1]的值
	//为每一个临时变量申请一个栈空间

	struct reg {
		std::string name;		//该存储的临时变量/局部变量的名字
		int regKind;			//0-临时数字, 1-临时变量, 2-局部变量(包括数组, map里面用array[1]表示其名字), 3-全局变量
		int offAddr;		//若为临时变量(如果之前在栈中存在的话,不存在则设置为-1)、局部变量或者全局变量的话其相对各自数据区的地址偏移, sw的时候用
		int isEmpty;	//标志当前寄存器是否可用
		int isBusy;		//之后根据冲突设置当前指令是否使用
	};

	struct r_reg {
		reg regArray[R_REG_NUM];	////临时寄存器 $8-$25
		int ptr;		//指向当前空闲位置或者当前该被替换的寄存器(FIFO)
		bool isfull;		//表示是否满了
	};

	r_reg R_Regs;


	std::map<std::string, var_info> S_Map;	//局部变量信息，在每次jal之前 和 函数返回调用(label isstart = 3)处清空
	std::map<std::string, int> S_Count_Map;
	int ParaOffset;	//在每次call的时候更新, 在parapassDeal中使用并自增长来表示当前实参应该压入的位置
	std::string Call_Func_Name[CALL_MAX];//call 的函数名栈, 在jal处理查找的时候使用
	int Call_Func_Ptr = 0;
	int Str_Id = 0;	//生成string标签使用

	int FuncVarAllocCount = 0;	//记录当前函数分配了几个全局寄存器, 在每次处理函数开始时清0

	//生成string标签并返回，将对应信息放到str_info_map中
	std::string creatStrLabel(std::string str) {
		std::string label;
		label = "str_" + int_to_string(Str_Id);
		Str_Id++;
		Str_Info_Map[label] = str;
		return label;
	}
	//根据分派到的寄存器号 0-17得到具体的MIPS指令中寄存器的表达方式
	std::string idToReg(int num) {
		return "$" + int_to_string(GET_REG_ID(num));
	}

	/*
		函数功能: 将当前寄存器的值存入内存
		函数参数及作用: 寄存器的数组下标
	*/
	void pushReg(int reg_pos) {
		if (R_Regs.regArray[reg_pos].regKind == 1) {	//临时变量
			enterMipsCode(9, idToReg(reg_pos), DATA_BASE, " ", GET_OFFSET(R_Regs.regArray[reg_pos].offAddr));
			T_Map[R_Regs.regArray[reg_pos].name].inReg = false;	//不在寄存器中
		}
		else if (R_Regs.regArray[reg_pos].regKind == 2) {		//局部变量
			enterMipsCode(9, idToReg(reg_pos), DATA_BASE, " ", GET_OFFSET(R_Regs.regArray[reg_pos].offAddr));
			S_Map[R_Regs.regArray[reg_pos].name].inReg = false;	//不在寄存器中
		}
		else if (R_Regs.regArray[reg_pos].regKind == 3) {		//全局变量
			enterMipsCode(9, idToReg(reg_pos), GLOBAL_BASE, " ", GET_OFFSET(R_Regs.regArray[reg_pos].offAddr));
			S_Map[R_Regs.regArray[reg_pos].name].inReg = false;	//不在寄存器中
		}
	}

		/*
		函数功能: 清空寄存器
		函数参数及作用:  kind = 0， 清空 $8-$25寄存器, 不存入内存;		函数声明开始时使用
		kind = 1, 清空 $8-$25寄存器, 将相应内容存入内存中;		调用其他函数时在jal之前使用
		kind = 2, 清空 $8-$25寄存器, 将全局变量存入内存,其余不存入	 函数返回时使用
		kind = 3, 清空 临时变量寄存器，保留全局寄存器(func一开始分配的保留, 函数内跨块使用)
	*/
	void clearRegs(int kind) {
		switch (kind) {
		case 0://kind = 0， 清空 $8-$25寄存器, 不存入内存;函数声明开始时使用
			for (int i = 0; i < R_REG_NUM; i++) {
				if (R_Regs.regArray[i].regKind == 1) {
					T_Map[R_Regs.regArray[i].name].inReg = false;	//不在寄存器中
				}
				else if ((R_Regs.regArray[i].regKind == 2) || (R_Regs.regArray[i].regKind == 3)) {
					S_Map[R_Regs.regArray[i].name].inReg = false;	//不在寄存器中
				}
				R_Regs.regArray[i].isEmpty = true;
				R_Regs.regArray[i].name = " ";
				R_Regs.regArray[i].offAddr = -1;
				R_Regs.regArray[i].regKind = -1;
				R_Regs.regArray[i].isBusy = false;
			}
			R_Regs.ptr = 0;
			R_Regs.isfull = false;
			break;
		case 1://kind = 1, 清空 $8-$25寄存器, 将相应内容存入内存中;		调用其他函数时在jal之前使用
			for (int i = 0; i < R_REG_NUM; i++) {
				if (R_Regs.regArray[i].isEmpty == false) pushReg(i);
				R_Regs.regArray[i].isEmpty = true;
				R_Regs.regArray[i].name = " ";
				R_Regs.regArray[i].offAddr = -1;
				R_Regs.regArray[i].regKind = -1;
				R_Regs.regArray[i].isBusy = false;
			}
			R_Regs.ptr = 0;
			FuncVarAllocCount = 0;
			R_Regs.isfull = false;
			break;
		case 2:	//kind = 2, 清空 $8-$25寄存器, 将全局变量存入内存,其余不存入	 函数返回时使用
			for (int i = 0; i < FuncVarAllocCount; i++) {
				if (R_Regs.regArray[i].regKind == 3) {
					enterMipsCode(9, idToReg(i), GLOBAL_BASE, " ", GET_OFFSET(R_Regs.regArray[i].offAddr));
				}
			}

			for (int i = FuncVarAllocCount; i < R_REG_NUM; i++) {
				if (R_Regs.regArray[i].regKind == 1) {
					T_Map[R_Regs.regArray[i].name].inReg = false;	//不在寄存器中
				}
				else if (R_Regs.regArray[i].regKind == 2) {
					S_Map[R_Regs.regArray[i].name].inReg = false;	//不在寄存器中
				}
				else if (R_Regs.regArray[i].regKind == 3) {
					pushReg(i);
					S_Map[R_Regs.regArray[i].name].inReg = false;	//不在寄存器中
				}
				R_Regs.regArray[i].isEmpty = true;
				R_Regs.regArray[i].name = " ";
				R_Regs.regArray[i].offAddr = -1;
				R_Regs.regArray[i].regKind = -1;
				R_Regs.regArray[i].isBusy = false;
			}
			R_Regs.ptr = FuncVarAllocCount;
			R_Regs.isfull = false;
			break;
		case 3://kind = 3, 清空 临时变量寄存器，保留全局寄存器(func一开始分配的保留, 函数内跨块使用)
			for (int i = FuncVarAllocCount; i < R_REG_NUM; i++) {
				if (R_Regs.regArray[i].isEmpty == false) pushReg(i);
				R_Regs.regArray[i].isEmpty = true;
				R_Regs.regArray[i].name = " ";
				R_Regs.regArray[i].offAddr = -1;
				R_Regs.regArray[i].regKind = -1;
				R_Regs.regArray[i].isBusy = false;
			}
			R_Regs.ptr = FuncVarAllocCount;
			R_Regs.isfull = false;
			break;
		default:
			//std::cout << "There is a bug in midtomipsdeal--clearRegs, switch-default.kind: " << kind << std::endl;
			break;
		}
	}

	//在Map表中寻找,找到返回true并修改var_info引用的值, 未找到返回0   kind = 0, T_Map中找, kind = 1, S_Map中找
	bool findInMap(std::string name, int kind, var_info& vinfo) {
		if (kind == 0) {//在T_Map中寻找
			auto iter = T_Map.find(name);
			if (iter == T_Map.end()) return false;	//没找到
			else {
				vinfo = iter->second;
				return true;
			}
		}
		else if (kind == 1) {//在S_Map中寻找
			auto iter = S_Map.find(name);
			if (iter == S_Map.end()) return false;	//没找到
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

		//申请一个r_reg寄存器, 必要时把该寄存器存储的值存入内存中,返回R_Regs下标
	int allocR_REG(std::string name, int offaddr, int regkind) {
		//判断是否冲突
		if (R_Regs.regArray[R_Regs.ptr].isBusy) {
			R_Regs.ptr = (R_Regs.ptr + 1) % R_REG_NUM;
			if (R_Regs.ptr == 0) {
				R_Regs.ptr = FuncVarAllocCount;
				R_Regs.isfull = true;		//相当于 R_Regs.ptr + 1 = 18了
			}
		}
		int r_id = R_Regs.ptr;
		if (R_Regs.isfull) {		//将当前寄存器的值放入内存  sw 9
			pushReg(R_Regs.ptr);
		}
		R_Regs.regArray[r_id].isEmpty = false;
		R_Regs.regArray[r_id].name = name;
		R_Regs.regArray[r_id].offAddr = offaddr;
		R_Regs.regArray[r_id].regKind = regkind;
		R_Regs.ptr = (R_Regs.ptr + 1) % R_REG_NUM;
		if (R_Regs.ptr == 0) {
			R_Regs.ptr = FuncVarAllocCount;
			R_Regs.isfull = true;		//相当于 R_Regs.ptr + 1 = 18了
		}
		return r_id;
	}

	void deleteBusy() {
		for (int i = 0; i < R_REG_NUM; i++) {
			R_Regs.regArray[i].isBusy = false;
		}
	}

	//funcAllocRegs调用的功能函数
	void varCount(std::string name) {
		if (string_is_num(name) || (name[0] == 'T') || (name == "RET")) return;	//不是局部变量
		else {
			if (find_in_tab(name) == 0) {
				//std::cout << "There is a bug in midtomipsdeal--varCount: name not find in tab" << std::endl;
			}
			else {
				auto iter = S_Count_Map.find(name);
				if (iter == S_Count_Map.end()) {	//没找到
					S_Count_Map[name] = 1;
				}
				else {	//找到了引用此时+1
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
		if (S_Count_Map.size() < S_GLOBAL_MAX) {	//小于10个直接分配
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
				//将局部/全局变量从空间中拿出来, lw指令
				if (tab.tabArray[t_pos].lev == 1) {	//局部
					r_id = allocR_REG(iter->first, tab.tabArray[t_pos].adr, 2);
					reg_name = idToReg(r_id);
					enterMipsCode(8, reg_name, DATA_BASE, " ", GET_OFFSET(tab.tabArray[t_pos].adr));
				}
				else {		//全局
					r_id = allocR_REG(iter->first, tab.tabArray[t_pos].adr, 3);
					reg_name = idToReg(r_id);
					enterMipsCode(8, reg_name, GLOBAL_BASE, " ", GET_OFFSET(tab.tabArray[t_pos].adr));
				}

				//新增局部/全局变量在map中的信息
				tmp_vinfo.inReg = true;
				tmp_vinfo.r_addr = r_id;
				tmp_vinfo.stack_addr = tab.tabArray[t_pos].adr;
				S_Map[iter->first] = tmp_vinfo;
				FuncVarAllocCount++;
			}
		}
		else {		//大于10个则分配引用次数较多的10个
			std::vector<PAIR> s_count_vec(S_Count_Map.begin(), S_Count_Map.end());
			sort(s_count_vec.begin(), s_count_vec.end(), cmp_by_value);
			for (i = 0; i < S_GLOBAL_MAX; ++i) {
				//std::cout << s_count_vec[i].first << ":" << s_count_vec[i].second << std::endl;
				t_pos = find_in_tab(s_count_vec[i].first);

				//将局部/全局变量从空间中拿出来, lw指令
				if (tab.tabArray[t_pos].lev == 1) {	//局部
					r_id = allocR_REG(s_count_vec[i].first, tab.tabArray[t_pos].adr, 2);
					reg_name = idToReg(r_id);
					enterMipsCode(8, reg_name, DATA_BASE, " ", GET_OFFSET(tab.tabArray[t_pos].adr));
				}
				else {		//全局
					r_id = allocR_REG(s_count_vec[i].first, tab.tabArray[t_pos].adr, 3);
					reg_name = idToReg(r_id);
					enterMipsCode(8, reg_name, GLOBAL_BASE, " ", GET_OFFSET(tab.tabArray[t_pos].adr));
				}

				//新增局部/全局变量在map中的信息
				tmp_vinfo.inReg = true;
				tmp_vinfo.r_addr = r_id;
				tmp_vinfo.stack_addr = tab.tabArray[t_pos].adr;
				S_Map[s_count_vec[i].first] = tmp_vinfo;
				FuncVarAllocCount++;
			}

		}
	}

		//在函数声明时调用, 使用引用计数方法为函数的局部变量分配寄存器
	void funcAllocRegs(int func_start) {

		int func_end = 0;
		int i = 0;
		S_Count_Map.clear();
		//std::cout <<MidCodeOptT.midcodeArray[func_start].y << std::endl;
		FuncVarAllocCount = 0;

		//扫描该函数所在块
		for (i = func_start + 1; i < MidCodeOptT.mdc; i++) {
			if (MidCodeOptT.midcodeArray[i].op == 14) break;
		}
		func_end = i;

		//扫描该块内所有变量
		for (i = func_start + 1; i < func_end; i++) {
			midcode t = MidCodeOptT.midcodeArray[i];
			if ((t.op >= 0) && (t.op < 4)) {	//+ - * /
				varCount(t.z);
				varCount(t.x);
				varCount(t.y);
			}
			else if (t.op == 4) {	// 赋值
				if (t.x == " ") {	//普通赋值
					varCount(t.z);
					varCount(t.y);
				}
				else {	//数组给变量赋值
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


		/*
	函数作用: 处理z, y, x
	参数说明:	s 为传进来的z、y 或者 x
	reg_name为传入的寄存器引用, 若为临时变量、局部变量或者全局变量则  reg_name = 返回存储的寄存器, 否则不处理
	num  为传入的临时数字, 若为临时数字则  num = 返回数字的值, 否则不对其进行处理
	返回值: 返回值为1说明reg_name有值, 返回值为2说明num有值(临时数字), 返回值为0说明有错
	*/
	int dealOperand(std::string s, std::string& reg_name, int&num) {
		var_info tmp_vinfo;
		std::string array_name;

		if (s == "RET") {		//返回值
			reg_name = RE_VALUE_REG;
			return 1;
		}

		if (string_is_num(s)) {		//临时数字
			num = string_to_int(s);
			return 2;
		}
		else if (s[0] == '#') {			//临时变量
			if (findInMap(s, 0, tmp_vinfo)) {		//查找T_Map表
				if (tmp_vinfo.inReg) {		//在寄存器中
					reg_name = idToReg(tmp_vinfo.r_addr);

					R_Regs.regArray[tmp_vinfo.r_addr].isBusy = true;

				}
				else {							//在内存中
					int r_id = allocR_REG(s, tmp_vinfo.stack_addr, 1);
					reg_name = idToReg(r_id);

					R_Regs.regArray[r_id].isBusy = true;

					//将临时变量从栈空间中拿出来, lw指令
					if (tmp_vinfo.isTFirst) {
						T_Map[s].isTFirst = false;
					}
					else {
						enterMipsCode(8, reg_name, DATA_BASE, " ", GET_OFFSET(tmp_vinfo.stack_addr));
					}
					//更改临时变量在map中的信息
					T_Map[s].inReg = true;
					T_Map[s].r_addr = r_id;
				}
			}
			else {
				//std::cout << "There is a bug in midtomipsdeal--dealOperand, T not find in T_Map" << std::endl;
			}
			return 1;
		}
		else {								//局部变量/全局变量
			if (findInMap(s, 1, tmp_vinfo)) {
				if (tmp_vinfo.inReg) {		//在寄存器中
					reg_name = idToReg(tmp_vinfo.r_addr);
					R_Regs.regArray[tmp_vinfo.r_addr].isBusy = true;
				}
				else {							//在内存中
					int t_pos;	//局部变量在符号表中的位置
					t_pos = find_in_tab(s);
					if (t_pos == 0) {
						//std::cout << "There is a bug in midtomipsdeal--dealOperand, S-InMap-t_pos can't be 0" << std::endl;
						return 0;
					}
					int r_id;

					//将局部/全局变量从栈空间中拿出来, lw指令
					if (tab.tabArray[t_pos].lev == 1) {	//局部
						r_id = allocR_REG(s, tmp_vinfo.stack_addr, 2);
						reg_name = idToReg(r_id);
						R_Regs.regArray[r_id].isBusy = true;
						enterMipsCode(8, reg_name, DATA_BASE, " ", GET_OFFSET(tab.tabArray[t_pos].adr));
					}
					else {		//全局
						r_id = allocR_REG(s, tmp_vinfo.stack_addr, 3);
						reg_name = idToReg(r_id);
						R_Regs.regArray[r_id].isBusy = true;
						enterMipsCode(8, reg_name, GLOBAL_BASE, " ", GET_OFFSET(tab.tabArray[t_pos].adr));
					}

					//更改局部/全局变量在map中的信息
					S_Map[s].inReg = true;
					S_Map[s].r_addr = r_id;
				}
			}
			else {	//第一次出现该局部变量/全局变量
				int t_pos;	//局部变量在符号表中的位置
				t_pos = find_in_tab(s);
				if (t_pos == 0) {
					//std::cout << "There is a bug in midtomipsdeal--dealOperand, S-NotInMap-t_pos can't be 0" << std::endl;
					return 0;
				}

				int r_id;


				//将局部/全局变量从空间中拿出来, lw指令
				if (tab.tabArray[t_pos].lev == 1) {	//局部
					r_id = allocR_REG(s, tab.tabArray[t_pos].adr, 2);
					reg_name = idToReg(r_id);
					R_Regs.regArray[r_id].isBusy = true;
					enterMipsCode(8, reg_name, DATA_BASE, " ", GET_OFFSET(tab.tabArray[t_pos].adr));
				}
				else {		//全局
					r_id = allocR_REG(s, tab.tabArray[t_pos].adr, 3);
					reg_name = idToReg(r_id);
					R_Regs.regArray[r_id].isBusy = true;
					enterMipsCode(8, reg_name, GLOBAL_BASE, " ", GET_OFFSET(tab.tabArray[t_pos].adr));
				}

				//新增局部/全局变量在map中的信息
				tmp_vinfo.inReg = true;
				tmp_vinfo.r_addr = r_id;
				tmp_vinfo.stack_addr = tab.tabArray[t_pos].adr;
				S_Map[s] = tmp_vinfo;
			}
			return 1;
		}
	}

		// +、-、*、/ 的处理, 传入参数为第几条指令
	void calcuDeal(midcode cur_mid, int mid_pos) {
		std::string z = " ", x = " ", y = " ";
		int z_flag, x_flag, y_flag;
		int result, r_id;		//result x与y均为数字的时候直接计算出来,  r_id 除法或者减法 x为数字的时候申请寄存器使用
		int z_num, x_num, y_num;
		z_flag = dealOperand(cur_mid.z, z, z_num);
		//if (z_flag != 1) std::cout << "There is a bug in midtomipsdeal--calcuDeal, z_flag must be 1. mid_pos:" << mid_pos << std::endl;
		x_flag = dealOperand(cur_mid.x, x, x_num);
		//if (x_flag == 0) std::cout << "There is a bug in midtomipsdeal--calcuDeal, x_flag can't be 0. mid_pos:" << mid_pos << std::endl;
		y_flag = dealOperand(cur_mid.y, y, y_num);
		//if (y_flag == 0) std::cout << "There is a bug in midtomipsdeal--calcuDeal, y_flag can't be 0. mid_pos:" << mid_pos << std::endl;

		switch (cur_mid.op) {
		case 0:	//+
			if ((x_flag == 2) && (y_flag == 2)) {		//直接计算出结果
				result = x_num + y_num;
				enterMipsCode(19, z, " ", " ", result);					//加载li指令
			}
			else if ((x_flag == 2) && (y_flag != 2)) {
				enterMipsCode(4, z, y, " ", x_num);
			}
			else if ((y_flag == 2) && (x_flag != 2)) {
				enterMipsCode(4, z, x, " ", y_num);
			}
			else {
				enterMipsCode(0, z, x, y, 0);
			}
			break;
		case 1:	//-
			if ((x_flag == 2) && (y_flag == 2)) {		//直接计算出结果
				result = x_num - y_num;
				enterMipsCode(19, z, " ", " ", result);					//加载li指令
			}
			else if ((x_flag == 2) && (y_flag != 2)) {			// z = x - y   x为数字
				r_id = allocR_REG(" ", -1, 0);
				x = idToReg(r_id);
				enterMipsCode(19, x, " ", " ", x_num);
				enterMipsCode(1, z, x, y, 0);
				//释放该寄存器
				if (R_Regs.ptr == 0) R_Regs.ptr = 17;
				else R_Regs.ptr--;
			}
			else if ((y_flag == 2) && (x_flag != 2)) {
				enterMipsCode(5, z, x, " ", y_num);				//z = x - y   y 为数字
			}
			else {
				enterMipsCode(1, z, x, y, 0);
			}
			break;
		case 2:	//*
			if ((x_flag == 2) && (y_flag == 2)) {		//直接计算出结果
				result = x_num * y_num;
				enterMipsCode(19, z, " ", " ", result);					//加载li指令
			}
			else if ((x_flag == 2) && (y_flag != 2)) {
				enterMipsCode(6, z, y, " ", x_num);
			}
			else if ((y_flag == 2) && (x_flag != 2)) {
				enterMipsCode(6, z, x, " ", y_num);
			}
			else {
				enterMipsCode(2, z, x, y, 0);
			}
			break;
		case 3:	// /
			if ((x_flag == 2) && (y_flag == 2)) {		//直接计算出结果
				result = x_num / y_num;
				enterMipsCode(19, z, " ", " ", result);					//加载li指令
			}
			else if ((x_flag == 2) && (y_flag != 2)) {			// z = x / y   x为数字
				r_id = allocR_REG(" ", -1, 0);
				x = idToReg(r_id);
				enterMipsCode(19, x, " ", " ", x_num);
				enterMipsCode(3, z, x, y, 0);
				//释放该寄存器
				if (R_Regs.ptr == 0) R_Regs.ptr = 17;
				else R_Regs.ptr--;
			}
			else if ((y_flag == 2) && (x_flag != 2)) {
				if (y_num == 0) {	//除数为0
					y_num = 1;
				}
				enterMipsCode(7, z, x, " ", y_num);				//z = x / y   y 为数字
			}
			else {
				enterMipsCode(3, z, x, y, 0);
			}
			break;
		default:
			break;
		}
	}

		//赋值指令处理
	void assignDeal(midcode cur_mid, int mid_pos) {
		std::string z = " ", x = " ", y = " ", t = " ";
		int z_flag, x_flag, y_flag;
		int t_pos, r_id;		//result x与y均为数字的时候直接计算出来,  r_id 除法或者减法 x为数字的时候申请寄存器使用
		int z_num, x_num, y_num;

		switch (cur_mid.op) {
		case 4:				// z = y 或者 z = y[x]
			z_flag = dealOperand(cur_mid.z, z, z_num);
			//if (z_flag != 1) std::cout << "There is a bug in midtomipsdeal--assignDeal, switch-4-z_flag must be 1. mid_pos:" << mid_pos << std::endl;
			if (cur_mid.x == " ") {		//z = y的赋值
				y_flag = dealOperand(cur_mid.y, y, y_num);
				//if (y_flag == 0) std::cout << "There is a bug in midtomipsdeal--assignDeal, switch-4-y_flag can't be 0. mid_pos:" << mid_pos << std::endl;
				if (y_flag == 1) {
					enterMipsCode(20, z, y, " ", 0);	//move
				}
				else {
					//std::cout << mid_pos << " " << cur_mid.z << " " << cur_mid.y << std::endl;
					enterMipsCode(19, z, " ", " ", y_num);
				}
			}
			else {		// z = y[x]的赋值		lw
				t_pos = find_in_tab(cur_mid.y);
				//if (t_pos == 0) std::cout << "There is a bug in midtomipsdeal--assignDeal, switch-4-t_pos can't be 0. mid_pos:" << mid_pos << std::endl;
				y_num = tab.tabArray[t_pos].adr;	//数组第一个元素的相对地址

				x_flag = dealOperand(cur_mid.x, x, x_num);
				//if (x_flag == 0) std::cout << "There is a bug in midtomipsdeal--assignDeal, switch-4-x_flag can't be 0. mid_pos" << mid_pos << std::endl;
				if (x_flag == 2) {			// x为数的时候

					y_num = y_num + x_num;
					if (tab.tabArray[t_pos].lev == 1) {		//局部
						enterMipsCode(8, z, DATA_BASE, " ", GET_OFFSET(y_num));					//lw
					}
					else {									//全局
						enterMipsCode(8, z, GLOBAL_BASE, " ", GET_OFFSET(y_num));					//lw
					}
				}
				else {	//x 为变量名     $fp/$gp + GET_OFFSET(y_num + $x)
					r_id = allocR_REG(" ", -1, 0);
					t = idToReg(r_id);
					enterMipsCode(4, t, x, " ", y_num);
					enterMipsCode(6, t, t, " ", -4);		// -4 * (y_num + $x)
					if (tab.tabArray[t_pos].lev == 1) {		//局部
						enterMipsCode(0, t, t, DATA_BASE, 0);
						enterMipsCode(8, z, t, " ", 0);
					}
					else {			//全局
						enterMipsCode(0, t, t, GLOBAL_BASE, 0);
						enterMipsCode(8, z, t, " ", 0);
					}
					//释放该寄存器
					if (R_Regs.ptr == 0) R_Regs.ptr = 17;
					else R_Regs.ptr--;
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
			if (y_flag == 2) {			//数字
				r_id = allocR_REG(" ", -1, 0);
				y = idToReg(r_id);
				enterMipsCode(19, y, " ", " ", y_num);		//li
															//释放该寄存器
															//if (R_Regs.ptr == 0) R_Regs.ptr = 17;
															//else R_Regs.ptr--;
			}

			// 到此y 为要存储到栈空间的寄存器 不能被释放！！！要不之后x为变量时候会直接申请导致冲掉之前的y

			if (x_flag == 2) {		//x为数字
				z_num = z_num + x_num;

				if (tab.tabArray[t_pos].lev == 1) {	//局部
					enterMipsCode(9, y, DATA_BASE, " ", GET_OFFSET(z_num));
				}
				else {		//全局
					enterMipsCode(9, y, GLOBAL_BASE, " ", GET_OFFSET(z_num));
				}
			}
			else {		// 此时x为变量
				r_id = allocR_REG(" ", -1, 0);
				t = idToReg(r_id);
				enterMipsCode(4, t, x, " ", z_num);
				enterMipsCode(6, t, t, " ", -4);		// -4 * (z_num + $x)
				if (tab.tabArray[t_pos].lev == 1) {		//局部
					enterMipsCode(0, t, t, DATA_BASE, 0);
					enterMipsCode(9, y, t, " ", 0);
				}
				else {			//全局
					enterMipsCode(0, t, t, GLOBAL_BASE, 0);
					enterMipsCode(9, y, t, " ", 0);
				}
				//释放该寄存器 ?????
				if (R_Regs.ptr == 0) R_Regs.ptr = 17;
				else R_Regs.ptr--;

			}
			break;
		default:
			//std::cout << "There is a bug in midtomipsdeal--assignDeal, switch--default. mid_pos:" << mid_pos << std::endl;
			break;
		}
	}

		//无条件跳转指令处理
	void noConJumpDeal(midcode cur_mid, int mid_pos) {
		std::string y;			//y操作数在ret时使用
		int offset_1, offset_2, t_pos, b_pos; //在jal中用到
		int  y_flag, y_num;
		switch (cur_mid.op) {
		case 5:	//goto
			clearRegs(3);
			enterMipsCode(16, cur_mid.z, " ", " ", 0);		//j label
			break;
		case 12: //ret
			if (cur_mid.y != " ") {		//处理返回值
				y_flag = dealOperand(cur_mid.y, y, y_num);
				//if (y_flag == 0) std::cout << "There is a bug in midtomipsdeal--noConJumpDeal, switch-12." << std::endl;
				if (y_flag == 1) {
					enterMipsCode(20, RE_VALUE_REG, y, " ", 0);
				}
				else {
					enterMipsCode(19, RE_VALUE_REG, " ", " ", y_num);
				}
			}
			clearRegs(2);	//处理寄存器
			//S_Map.clear();
			//clearRegs(3);
			enterMipsCode(17, RE_LABEL_REG, " ", " ", 0);
			break;
		case 21: //jal
			clearRegs(1);		//函数调用时跳转之前将现场保留, 将寄存器清空
			S_Map.clear();		//清空S_Map


			t_pos = find_in_tab(Call_Func_Name[--Call_Func_Ptr]);
			//if (t_pos == 0) std::cout << "There is a bug in midtomipsdeal--noConditionJumpDeal, switch--21" << std::endl;
			b_pos = tab.tabArray[t_pos].ref;
			offset_1 = 3 + btab.btabArray[b_pos].paranum;	//fp = sp - (3+paranum)
			offset_2 = btab.btabArray[b_pos].varsize + btab.btabArray[b_pos].t_varnum;

			enterMipsCode(5, DATA_BASE, STACK_TOP, " ", GET_OFFSET(offset_1));			// 移动基址fp = sp-3-paranum
			enterMipsCode(4, STACK_TOP, STACK_TOP, " ", GET_OFFSET(offset_2));		//addi 移动栈顶指针
			enterMipsCode(18, cur_mid.z, " ", " ", 0);		//jal label
			break;
		default:
			//std::cout << "There is a bug in midtomipsdeal--noConJumpDeal, switch--default" << std::endl;
			break;
		}

	}

		//条件跳转指令处理
	void conditionJumpDeal(midcode cur_mid, int mid_pos) {
		std::string  x, y;
		int op;
		int x_flag, y_flag;
		int r_id;		//r_id 除法或者减法 x为数字的时候申请寄存器使用
		int x_num, y_num;

		op = cur_mid.op + 4;
		//if (op < 10 || op > 15) std::cout << "There is a bug in midThere is a bug in midtomipsdeal--conditionJumpDeal, op out of range. mid_pos:" << mid_pos << std::endl;

		x_flag = dealOperand(cur_mid.x, x, x_num);
		//if (x_flag == 0) std::cout << "There is a bug in midtomipsdeal--conditionJumpDeal, x_flag can't be 0. mid_pos:" << mid_pos << std::endl;
		y_flag = dealOperand(cur_mid.y, y, y_num);
		//if (y_flag == 0) std::cout << "There is a bug in midtomipsdeal--conditionJumpDeal, y_flag can't be 0. mid_pos:" << mid_pos << std::endl;


		if (x_flag == 2) {		//x为数字
			r_id = allocR_REG(" ", -1, 0);
			x = idToReg(r_id);
			enterMipsCode(19, x, " ", " ", x_num); //将值加载到寄存器中
												   //释放该寄存器
			if (R_Regs.ptr == 0) R_Regs.ptr = 17;
			else R_Regs.ptr--;
		}

		clearRegs(3);	//跳转之前清空寄存器

		if (y_flag == 2) {			//y为数字
			enterMipsCode(op, cur_mid.z, x, " ", y_num);  // x与offset比较
		}
		else {
			enterMipsCode(op, cur_mid.z, x, y, 0);		// x 与 y 比较
		}

	}


	//标签的处理
	void setlabDeal(midcode cur_mid, int mid_pos, bool opt) {
		int offset, t_pos, b_pos;			// b_pos为函数在btab表中偏移
		int i = 0;
		bool loadS = false;
		switch (cur_mid.isstart) {
		case 1:			//普通的lab标签
			clearRegs(3);		//清空寄存器(代码基本块的开端)
			enterMipsCode(22, cur_mid.y, " ", " ", 0);

			break;
		case 2:			//main 函数入口标签, 在此处处理main空间的指针移动, 因为没有函数call main，对于其他函数则在call的时候处理
			enterMipsCode(22, cur_mid.y, " ", " ", 0);
			t_pos = find_in_tab("main");
			b_pos = tab.tabArray[t_pos].ref;
			offset = 3 + btab.btabArray[b_pos].varsize + btab.btabArray[b_pos].t_varnum;
			enterMipsCode(23, RE_LABEL_REG, "exit", " ", 0);				//设置程序出口
			enterMipsCode(20, DATA_BASE, STACK_TOP, " ", 0);
			//addi 负数
			enterMipsCode(4, STACK_TOP, STACK_TOP, " ", GET_OFFSET(offset));
			if (opt) funcAllocRegs(mid_pos);
			break;
		case 3:			//call函数后面返回标签

			//clearRegs(2);	//处理寄存器
			//S_Map.clear();

			enterMipsCode(22, cur_mid.y, " ", " ", 0);
			enterMipsCode(20, STACK_TOP, DATA_BASE, " ", 0);				//恢复栈顶空间
			enterMipsCode(8, DATA_BASE, DATA_BASE, " ", GET_OFFSET(1));		// 恢复调用函数的fp
			enterMipsCode(8, RE_LABEL_REG, DATA_BASE, " ", GET_OFFSET(0));		//恢复调用函数的返回地址
			//if(opt) funcAllocRegs(mid_pos);
			if (opt && mid_pos + 1 != MidCodeOptT.mdc) loadScountMap(mid_pos);
			break;
		case 4:				//函数开始标签, 只是打印标签不做处理
			enterMipsCode(22, cur_mid.y, " ", " ", 0);
			//分配全局寄存器
			if (opt) funcAllocRegs(mid_pos);

			break;
		default:
			//std::cout << "There is a bug in midtomipsdeal--setlabDeal, switch--default. mid_pos:" << mid_pos << std::endl;
			break;
		}
	}

		//函数声明的处理
	void funcDeal(midcode cur_mid, int mid_pos, bool opt) {
		int t_pos, b_pos;
		t_pos = find_in_tab(cur_mid.y);
		b_pos = tab.tabArray[t_pos].ref;
		display[level] = b_pos;				//设置当前数据局部域
		clearRegs(0);
		S_Map.clear();

	}

		//输入的处理
	void inDeal(midcode cur_mid, int mid_pos) {
		std::string y;
		int t_pos, y_flag, y_num, v0_para;		//根据标识符的属性标记输入的是字符(11)还是整数(5)
		t_pos = find_in_tab(cur_mid.y);
		if (t_pos == 0) {
			//std::cout << "There is a bug in midtomips--inDeal, t_pos can't be 0" << std::endl;
			return;			//标识符未定义则返回
		}
		y_flag = dealOperand(cur_mid.y, y, y_num);
		//if (y_flag != 1) std::cout << "There is a bug in midtomipsdeal--inDeal, y_flag must be 1. mid_pos:" << mid_pos << std::endl;

		//判断输入的标识符类型是整数还是字符
		if (tab.tabArray[t_pos].type == ints) {
			v0_para = 5;
		}
		else if (tab.tabArray[t_pos].type == chars) {
			v0_para = 12;
		}
		else {
			//std::cout << "There is a bug in midtomipsdeal--inDeal, y not ints or chars. mid_pos:" << mid_pos << std::endl;
		}
		enterMipsCode(19, "$v0", " ", " ", v0_para);
		enterMipsCode(21, " ", " ", " ", 0);
		enterMipsCode(20, y, "$v0", " ", 0);
	}

		//输出的处理
	void outDeal(midcode cur_mid, int mid_pos) {
		std::string x, y, x_label;
		int y_flag, y_num;
		if (cur_mid.x != " ") {	//有字符串
			x = cur_mid.x;
			x_label = creatStrLabel(x);
			enterMipsCode(19, "$v0", " ", " ", 4);
			enterMipsCode(23, "$a0", x_label, " ", 0); //???
			enterMipsCode(21, " ", " ", " ", 0);
		}
		if (cur_mid.y != " ") {	//有表达式
			y_flag = dealOperand(cur_mid.y, y, y_num);
			if (cur_mid.z != " ") {	//z == char  打印字符
				enterMipsCode(19, "$v0", " ", " ", 11);
			}
			else {
				enterMipsCode(19, "$v0", " ", " ", 1);
			}
			if (y_flag == 2) {
				enterMipsCode(19, "$a0", " ", " ", y_num);
			}
			else {
				enterMipsCode(20, "$a0", y, " ", 0);
			}
			enterMipsCode(21, " ", " ", " ", 0);
		}

		//在每一个输出语句后面打印换行符
		enterMipsCode(19, "$v0", " ", " ", 11);
		enterMipsCode(19, "$a0", " ", " ", '\n');
		enterMipsCode(21, " ", " ", " ", 0);
	}

		//传递参数处理,将参数存入内存中 sw 9
	void passparaDeal(midcode cur_mid, int mid_pos) {
		std::string y;
		int y_flag, y_num;
		y_flag = dealOperand(cur_mid.y, y, y_num);
		if (y_flag == 0) {
			//std::cout << "There is a bug in midtomipsdeal--passparaDeal, y_flag can't be 0. mid_pos:" << mid_pos << std::endl;
			return;
		}
		if (y_flag == 2) {		//数字
			enterMipsCode(19, "$a1", " ", " ", y_num);			// 此时的栈顶sp指向参数区开始地方
			enterMipsCode(9, "$a1", STACK_TOP, " ", 0);
			enterMipsCode(4, STACK_TOP, STACK_TOP, " ", GET_OFFSET(1));		//addi 栈指针上移
		}
		else {
			enterMipsCode(9, y, STACK_TOP, " ", 0);
			enterMipsCode(4, STACK_TOP, STACK_TOP, " ", GET_OFFSET(1));		//addi 栈指针上移1格
		}
	}


		//函数调用的处理
	void callDeal(midcode cur_mid, int mid_pos) {
		//ParaOffset = 3;
		if (Call_Func_Ptr == CALL_MAX) {
			std::cout << "midtomipsdeal---get CALL_MAX, and exit" << std::endl;
			exit(1);
		}
		Call_Func_Name[Call_Func_Ptr++] = cur_mid.y;
		enterMipsCode(9, RE_LABEL_REG, DATA_BASE, " ", GET_OFFSET(0));		//将当前$ra保存在原函数
		enterMipsCode(9, DATA_BASE, STACK_TOP, " ", GET_OFFSET(1));		//设置当前被调用函数的prev_abp
		enterMipsCode(4, STACK_TOP, STACK_TOP, " ", GET_OFFSET(3));		//addi 移动栈顶指针内务区到内务区顶
	}

		//本文件的主函数
	void midtomips(int kind) {
		midcode cur_mid;
		int midend;
		level = 1;
		//初始化变量
		clearRegs(0);
		MipsTable.mpc = 0;
		S_Map.clear();
		S_Count_Map.clear();
		ParaOffset = 0;	//在每次call的时候更新, 在parapassDeal中使用并自增长来表示当前实参应该压入的位置
		Call_Func_Ptr = 0;
		Str_Id = 0;	//生成string标签使用
		for (auto iter = T_Map.begin(); iter != T_Map.end(); iter++) {
			iter->second.isTFirst = true;
		}

		int FuncVarAllocCount = 0;	//记录当前函数分配了几个全局寄存器, 在每次处理函数开始时清0

		Str_Info_Map.clear();
		if (kind == 0) midend = MidCodeT.mdc;
		//if (kind == 0) midend = MidCodeOptT.mdc;
		else if (kind == 1) midend = MidCodeOptT.mdc;


		for (int i = 0; i < midend; i++) {
			if (kind == 0) cur_mid = MidCodeT.midcodeArray[i];
			//if (kind == 0) cur_mid = MidCodeOptT.midcodeArray[i];
			else if(kind == 1) cur_mid = MidCodeOptT.midcodeArray[i];
			switch (cur_mid.op) {
			case 0:		// +、-、*、/ 四则运算的处理
			case 1:
			case 2:
			case 3:
				calcuDeal(cur_mid, i);
				break;
			case 4:
			case 20:
				assignDeal(cur_mid, i);
				break;
			case 5:		//无条件跳转语句的处理
			case 12:
			case 21:
				noConJumpDeal(cur_mid, i);
				break;
			case 6://条件跳转语句的处理
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
				conditionJumpDeal(cur_mid, i);
				break;
			case 13:						//setlab 的处理
				if(kind == 0)setlabDeal(cur_mid, i, false);
				else if(kind == 1) setlabDeal(cur_mid, i, true);
				break;
			case 14:					//对函数声明的处理, 设置当前display区为该函数名所对应的空间
				if(kind == 0) funcDeal(cur_mid, i, false);
				else if (kind == 1) funcDeal(cur_mid, i, true);
				break;
			case 15:					//对参数声明的处理, 直接略过
				break;
			case 16:						//对输入的处理
				inDeal(cur_mid, i);
				break;
			case 17:						//对输出的处理
				outDeal(cur_mid, i);
				break;
			case 18:						//传递参数的时候使用
				passparaDeal(cur_mid, i);
				break;
			case 19:						//对函数调用call的处理
				callDeal(cur_mid, i);
				break;
			default:
				//std::cout << "There is a bug in midtomipsdeal--midtomips, switch--default" << std::endl;
				break;
			}
			//每一行处理结束清除busy
			deleteBusy();//???
		}
	}



}
