/*
	语法语义分析, 将源程序转换为四元式中间指令
*/

//注意对整数的处理拆开来判断
#include <iostream>
#include "Block_Deal.h"
#include<set>
namespace compiler{
        int Deal_Expression_kind_check = 0;//表达式类型检查标识
	int temp_num = 0;	//生成临时变量时使用
	int return_flag = 0; //在每次进入函数定义的时候设置为0,在return语句中设置为1或2,其中1代表没有返回值,2代表有返回值
	std::fstream judge_file;					//然后在函数定义结束时进行检查,void函数必须为0或1,int函数为2, char函数为3
    symbol	Block_Start_Sys[] = {constsy, voidsy, intsy, charsy};	//4个
    symbol  Statement_Start_Sys[] = { semicolon, ifsy, whilesy,lcbrack, ident, scanfsy, printfsy, switchsy, returnsy}; //9个
	symbol  Statement_End_Sys[] = { semicolon, rparent, rcbrack }; //3个
	symbol  Deal_Expression_Start_Sys[] = {ident, plus, minus, intcon, charcon, lparent}; //6个

	symbol	Follow_Deal_Expression_Sys[] = { semicolon, rparent, rsbrack, lss, leq, gtr, geq, neq, eql, comma};	//10个,最后一个在处理函数调用语句时候才用

	symbol  Deal_Factor_Start_Sys[] = { ident, plus, minus, intcon, charcon, lparent }; //6个
	symbol  Deal_Factor_End_Sys[] = { ident, rsbrack, intcon, charcon, rparent };//5个
//部分函数声明
	std::string Deal_reUseFunc();
	std::string Deal_Expression(bool& is_char);
	std::string Deal_Factor(bool& is_char);
	void statement();
    //跳读源程序直到取到的字符属于给定的符号集st, n为数组大小
	void Skip_Read(symbol st[], int n) {
		while (!find_sy(st, n)) insymbol();
	}
	//生成临时变量
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

	//生成标签,参数l_kind为标签类型, 在s的基础上增加标签  pc为当前中间指令的位置(MidCodeT此时的索引值),索引label时使用
	std::string Creat_Label(std::string s, int l_kind, int pc){
		std::string label;
		switch(l_kind){
			case 0:	//函数类起始标签
				label =  s + "_begin_";
				break;
			case 1:	//函数类结束标签
				label =  s + "_end_";
				break;
			case 2: //if、while等跳转
				label = "_" + int_to_string(pc) + "_label_";
				break;
			case 3: //switch_end标签
				label = "switch_" + int_to_string(pc) + "_end_";
				break;
			case 4: //call函数之后返回地址标签,用于标记入口语句
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

	//<常量说明> <常量定义> 处理
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
			e=enter(id, constant, tmp_typ);//将常量登记入表
			insymbol();

			if (sy != assign) {
				error(20,lc);
				Skip_Read(Block_Start_Sys, 4);
				return;
			}

			insymbol();	//读到的是等号后面的单词

			if ((sy == plus) || (sy == minus) || (sy == intcon)) {
				//进入整数分支
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
                    enterVar(tab.t - 1, inum);//将常量的值登录进tab，修改adr域
				}

			}
			else if (sy == charcon) {
				if (tmp_typ != chars) {
					error(27,lc);
					Skip_Read(Block_Start_Sys, 4);
					return;
				}
				if(e){
                   enterVar(tab.t - 1, inum);//将常量的值登录进tab，修改adr域
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
		insymbol();//读取分号后面一个字符
	}
    //<变量说明> <变量定义> 处理，传入时参数n为该行第一个标识符在数据栈的相对地址
	void Deal_Varible(types typ, int &n){
	    bool e=false;

		judge_file << "This is var_declare statement" << std::endl;
		//进入时id为 int/char 后面第一个标识符，sy == comma || sy == semicolon || sy = lsbrack
		if (sy == lsbrack) {//是数组的声明
			e=enter(id, array, typ);//填入tab表格，指针已+1
			insymbol();
			if ((sy == intcon) && (inum > 0)) {
                if(e){
                    enterVar(tab.t - 1, n);	//把数组的第一个元素再运行栈S中存放的相对地址填入adr域
                    n += inum;	//inum个空间都是该数组的，修改参数n的值
                    enterArray(tab.t - 1, typ, inum);//向atab中填入改数组的信息
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
		//截止到这里 sy == comma || sy == semicolon
		while (sy == comma) {
			insymbol();
			if (sy != ident) {	//非标识符,跳读完一行处理下一行
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
			if (sy == lsbrack) {	//该if结束后sy=comma or sy = semicolon
				e=enter(id, array, typ);
				insymbol();
				if ((sy == intcon) && (inum != 0)) {
                    if(e){
                        enterVar(tab.t - 1, n);	//把数组的第一个元素相对地址存放起来
                        n += inum;	//inum个空间都是该数组的
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
		insymbol();//读取分号之后第一个单词
	}
	//<参数表>
	// 传入值为参数的个数引用、pv_addr为目前变量处于函数数据区的相对地址,返回值为最后一个参数在tab表的登录位置
	int Deal_Para(int& para_num, int& pv_addr) {
	    bool e=false;
		//进入时sy == intsy || sy == charsy
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
			e=enter(id, variable, tmp_typ);//向tab中登记该参数的信息
			if(e){
                enterVar(tab.t-1,pv_addr++);//登记当前参数在S栈的偏移，并增加pv_addr的指针值
                para_num++;//增加参数个数
                Enter_Mid_List(15," ",types_info[tmp_typ],id,0);// para	x , y  函数定义时的形参，x为类型，y为形参名
			}
			insymbol();
		}
		symbol tmp_sys[] = { comma, rparent };
		Skip_Read(tmp_sys, 2);
		while (sy == comma) {
			insymbol();
			if ((sy != intsy) && (sy != charsy)) {
				error(31,lc);
				Skip_Read(tmp_sys, 2);//重新找到sy==comma|rparent，开启新的一轮循环检查
				continue;
			}
			//sy==int|char
			tmp_typ = Sy_Trans_Types(sy);
			insymbol();
			if (sy != ident) {	//如果不是标识符则跳读，重新开启一次循环
				error(31, lc);
				Skip_Read(tmp_sys, 2);
				continue;
			}
			//到达此处sy == ident
			e=enter(id, variable, tmp_typ);//向tab登记该变量的名称和类型等信息
			if(e){
                enterVar(tab.t - 1, pv_addr++);//向adr填写参数偏移
                para_num++;//增加参数个数
                Enter_Mid_List(15, " ", types_info[tmp_typ], id, 0);//para x,y
			}

			insymbol();
			if ((sy != comma) && (sy != rparent)) {//向前看下一个字符
				error(31, lc);
				Skip_Read(tmp_sys, 2);
			}
		}

		//到达此处的条件是sy == rparent，也即参数声明完毕
		if (sy != rparent) {
			std::cout << "There is a bug in blockdeal--Deal_Para, sy must be rparent" << std::endl;
		}
		insymbol();
		//出去的时候sy为')'后面一个单词
		return tab.t - 1;
	}
	//<因子>
	//返回值为操作数
	std::string Deal_Factor(bool& is_char) {
	    bool e=false;
		//进因子时,sy为 Deal_Factor_Start_Sys[]中的元素
		std::string z, y, x;
		bool char_tmp = false;
		int pos, tmp;	//tmp 在minus中使用
		symbol tmp_sys[] = { rparent };

		switch (sy) {
		case ident:	//标识符、标识符[表达式]、有返回值函数调用语句
			pos = find_in_tab(id);	//在tab表中查询当前标识符以获得信息
			if (pos == 0) error(23, lc);//没有找到该标识符，报错

			if (pos != 0) {//找到标识符，获取信息，返回值都以字符串的形式
				if (tab.tabArray[pos].type == chars) is_char = true;//标识符是一个字符
				switch (tab.tabArray[pos].obj) {
				case constant:	//常量直接返回数值
					return int_to_string(tab.tabArray[pos].adr);
				case variable:	//变量返回变量名
				     z = Creat_Tempvar();
                Enter_Mid_List(4, z, " ", id, 0);
                return z;
					//return id;
				case array:		//标识符[表达式]
				    if(Deal_Expression_kind_check==1 && is_char ==true){
                        is_char = false;
                        Deal_Expression_kind_check=0;
					}
					y = id;//记录下数组名
					z = Creat_Tempvar();//创建一个新的临时变量名
					insymbol();
					if (sy != lsbrack) {
						error(15, lc);//报错，缺少中括号
						Skip_Read(Deal_Factor_End_Sys, 5);
					}
					insymbol();
					Skip_Read(Deal_Expression_Start_Sys, 6);
					x = Deal_Expression(char_tmp);	//处理中括号的表达式,出来时sy为Follow_Deal_Expression_Sys[]中的元素
					if (char_tmp) {	//tmp为表示为字符类型
						error(49, lc);//报错，数组中括号中的下标不能为char型
					}
					else {
						if (string_is_num(x)) {//字符串的内容是数字串
							if (string_to_int(x) >= atab.atabArray[tab.tabArray[pos].ref].high) {//下标的大小超过了数组上界
								error(49, lc);//报错
							}
						}
					}
					if (sy != rsbrack) {
						error(16, lc);
						Skip_Read(Deal_Factor_End_Sys, 5);
					}

					Enter_Mid_List(4, z, x, y, 0);	//z = y[x]
													//此时sy为rsbrack
					return z;//返回运算的显示结果保存变量z，是临时申请的变量
				case function://是函数
					if (tab.tabArray[pos].type == notyp) {	//无返回值函数调用，报错
						error(34, lc);
						Skip_Read(tmp_sys,1);
						return "0";
					}
					if (tab.tabArray[pos].type == chars) is_char = true;//函数的返回值是一个字符char
					else is_char = false;
					if(Deal_Expression_kind_check==1 && is_char ==true){
                        is_char = false;
                        Deal_Expression_kind_check=0;
					}
					//进入Deal_reUseFunc时,sy == identsy(函数名)
					z = Deal_reUseFunc();//有返回值的函数调用语句的返回值是该函数的返回值

					//此时 sy为 rparent
					return z;	//返回有返回值函数的调用最后的赋值结果操作数, 即z = *RET
				default:
					std::cout << "There is a bug in blockdeal--Deal_Factor, switch-ident" << std::endl;
					return "0";
				}
			}
			else {	//标识符未定义
				return "0";
			}
			//程序不可能运行到这里
			std::cout << "There is a bug in blockdeal-Deal_Factor, switch-ident" << std::endl;
		case plus:	// 整数: +开头，后面跟无符号整数
			insymbol();
			if (sy == intcon) {
				return int_to_string(inum);//将整数转换为字符串并返回
			}
			else {
				error(33, lc);
				Skip_Read(Deal_Factor_End_Sys, 5);
				return "0";
			}
			//程序不可能运行到这里
			std::cout << "There is a bug in blockdeal-Deal_Factor, switch-plus" << std::endl;
		case minus:	// 整数: -，后面跟无符号整数
			tmp = -1;
			insymbol();
			if (sy == intcon) {
				inum = inum * tmp;//加入符号的因素
				return int_to_string(inum);//将整数转换为字符串并返回
			}
			else {
				error(33, lc);
				Skip_Read(Deal_Factor_End_Sys, 5);
				return "0";
			}
			//程序不可能运行到这里
			std::cout << "There is a bug in blockdeal-Deal_Factor, switch-minus" << std::endl;
		case intcon: // 整数，不含+/-开头，直接是无符号整数
			is_char = false;
			return int_to_string(inum);
		case charcon:	//字符
			is_char = true;
			return int_to_string(inum);//但返回的是字符的值
		case lparent://开头是(,说明是表达式
            Deal_Expression_kind_check=1;
			insymbol();
			if (!find_sy(Deal_Expression_Start_Sys, 6)) {
				error(33, lc);
				Skip_Read(Deal_Expression_Start_Sys, 6);
			}
			//Deal_Expression_lparent_flag=true;
			return Deal_Expression(is_char);//返回表达式的值
		default:
			std::cout << "There is a bug in blockdeal--Deal_Factor, switch-default" << std::endl;
			break;
		}

		//程序不可能运行到这里
		std::cout << "There is a bug in blockdeal-Deal_Factor, end" << std::endl;
		return " ";
		//出因子时,sy为 Deal_Factor_End_Sys[]中的元素
	}

	//<项>
	//返回值为项处理得到的最终的z操作数
	std::string Deal_Item(int minus_flag, bool& is_char){
		//进入时sy为Deal_Factor_Start_Sys[]中的元素
		std::string z, x, y;
		//std::cout << is_char << std::endl;
		//std::cout << "Deal_Item里面的y前"<<is_char << std::endl;
		x = Deal_Factor(is_char);//x为第一个因子
		//std::cout << "Deal_Item里面的y后"<<is_char << std::endl;
		//std::cout << is_char << std::endl;
	//	Deal_Expression_lparent_flag=false;
		if (minus_flag == 1) {		//表达式的第一个项才会遇到
			z = Creat_Tempvar();//产生临时变量存放x的相反数的结果
			y = x;
			x = "0";
			Enter_Mid_List(1, z, x, y, 0);//z=0-x
			x = z;//x=z
		}
		//离开Deal_Factor时, sy为Deal_Factor_End_Sys[]中的一个
		insymbol();
		while ((sy == mult) || (sy == rdiv)) {
			is_char = false;		// 有乘法运算符,肯定不是字符类型了
			int op;
			bool no_use;
			if (sy == mult) op = 2;
			else if (sy == rdiv) op = 3;
			else std::cout << "There is a bug in blockdeal--Deal_Item, while" << std::endl;
			insymbol();
			Skip_Read(Deal_Factor_Start_Sys, 6);
			y = Deal_Factor(no_use);
			//Deal_Expression_lparent_flag=false;
			z = Creat_Tempvar();//产生临时变量z，存储x和y做运算的值
			if ((op == 3) && string_is_num(y) && (string_to_int(y) == 0)) {//如果是除法运算，y不能为0
				error(44, lc);
			}
			Enter_Mid_List(op, z, x, y, 0);//做乘除法运算，得到的结果存在临时寄存器z中
			x = z;

			insymbol();
		}
		return x;
		//结束时 sy 为项后面一个单词
	}

	//<表达式>
	//返回值为最终的一个四元式表达式的z值, 传参: 表达式处理完是否为char类型
	std::string Deal_Expression(bool& is_char){
		//进入表达式时sy为Deal_Expression_Start_Sys[]中的元素

		//std::cout << "This is an Deal_Expression statement" << std::endl;
		judge_file << "This is an Deal_Expression statement" << std::endl;

		int minus_flag = 0;
		std::string z, x, y;
		if ((sy == plus) || (sy == minus)) {//如果表达式开头是作用于第一个项的+/-
			if (sy == minus) {
				minus_flag = 1;
			}
			insymbol();
		}

		Skip_Read(Deal_Factor_Start_Sys, 6);
		//std::cout << "Deal_Expression里面的y前"<<is_char << std::endl;
		//std::cout<<sy<<std::endl;
		x = Deal_Item(minus_flag, is_char);//进入项处理
		//std::cout << "Deal_Expression里面的y后"<<is_char << std::endl;
		//std::cout << "Deal_Expression里面的y后flag"<<Deal_Expression_kind_check << std::endl;
		if(Deal_Expression_kind_check==1){
            is_char = false;
            Deal_Expression_kind_check = 0;
		}
       // std::cout << is_char << std::endl;
		//出来时sy为 项后面的第一个单词
	//	if(Deal_Expression_lparent_flag){
       //     is_char=false;
       //     Deal_Expression_lparent_flag=false;
		//}
		symbol tmp_sys[] = { semicolon, rparent, rsbrack ,plus, minus, lss, leq, gtr, geq, neq, eql, comma};	//此时 sy应该为 follow[Deal_Expression] or 加法运算符
		if (!find_sy(tmp_sys, 12)) {
			error(33, lc);
			//std::cout<<"1"<<std::endl;
			Skip_Read(tmp_sys, 11);
		}
		while ((sy == plus) || (sy == minus)) {
			is_char = false;	// 有加法运算符则肯定不是字符型
			int op;
			bool no_use;
			if (sy == plus) op = 0;
			else if (sy == minus) op = 1;
			else std::cout << "There is a bug in blockdeal--Deal_Expression, while" << std::endl;
			insymbol();
			y = Deal_Item(0,no_use);//后面的Deal_Item都没有-
			Deal_Expression_kind_check = 0;
		//	Deal_Expression_lparent_flag=false;
			//sy为项后面的一个单词,所以不用insymbol()
			z = Creat_Tempvar();//产生临时变量z

			Enter_Mid_List(op, z, x, y, 0);//将x于y做计算得到的数值赋值到申请的临时变量z中
			x = z;//x=z
		}//循环得到表达式的值
		if (!find_sy(Follow_Deal_Expression_Sys, 10)) {
			error(33,lc);
			Skip_Read(Follow_Deal_Expression_Sys, 10);
		}
		return x;
		//出Deal_Expression时,sy为Follow_Deal_Expression_Sys[]中的元素
	}

	//<条件>
	//返回该跳转指令所在的中间代码中的位置,以便后续回填跳转的标签
	int Deal_Condition(){			// kind = 0, if 条件的调用; kind = 1, while条件的调用, 两者跳转的条件不同
		// 进入时, sy为左括号后的第一个单词
		//std::cout << "This is a condition statement" << std::endl;
		judge_file << "This is a condition statement" << std::endl;
		std::string z, x, y;
		int op;
		bool no_use_x=false;
		bool no_use_y=false;
		symbol relation_op_sys[] = { lss, leq, gtr, geq, neq, eql };	//6个

		if (!find_sy(Deal_Expression_Start_Sys, 6)) {
			error(33, lc);
			Skip_Read(Deal_Expression_Start_Sys, 6);
		}

		x = Deal_Expression(no_use_x);	//进入表达式处理
		//出Deal_Expression时,sy为Follow_Deal_Expression_Sys[]中的元素

		if (find_sy(relation_op_sys, 6)) {
				//有关系运算符,注意op的取值, if和while是不符合条件才跳转
				switch (sy) {
				case lss://<  意味着x >= y的时候不执行if内的语句,进行跳转
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
				case neq:// !=  x==y 跳转
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
			//std::cout <<"y前"<<no_use_y<< std::endl;
			y = Deal_Expression(no_use_y);	//进入表达式处理
			//std::cout <<"y后"<<no_use_y<< std::endl;
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
			op = 6; //单独一个表达式, 则在 x == 0的时候跳转
			y = "0";
		}
        Enter_Mid_List(op, " ", x, y, 0);//生成条件跳转指令，跳转到的标签暂时空白，等候回填
		//此时sy属于Follow_Deal_Expression_Sys[]中的元素
		if (sy != rparent) {
			error(50, lc);
		}
		return MidCodeT.mdc - 1; //返回此次生成的mips跳转指令所在的位置,便于之后回填label域
		// 出去时, sy == rparent
	}

	//<条件语句>
	void Deal_ifStatement(){
		//进入时 sy == ifsy
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
			//说明没有条件语句。报错
			error(21, lc);
		}
		else {
			code_pos = Deal_Condition();//code_pos是缺失label的mips语句的位置
			//出来时 sy == rparent
		}

		if (sy != rparent) std::cout << "There is a bug in blockdeal--Deal_ifStatement, sy must be rparent" << std::endl;

		insymbol();

		if (!find_sy(Statement_Start_Sys, 9)) {
			error(32, lc);
			Skip_Read(Statement_Start_Sys, 9);
		}

		statement();

		j_label = Creat_Label(" ", 2, MidCodeT.mdc);//在语句块最后的位置生成一个label标签，标签名字记录在j_label
		Enter_Mid_List(13, " ", " ",j_label, 1);//在语句块之后设置标签j_label
		MidCodeT.midcodeArray[code_pos].z = j_label;//回填label,不符合if条件的会直接跳转到这个位置，不执行语句块转换成的mips指令
		//出去时 sy 为 Statement_End_Sys中的元素
	}

	//循环语句
	void Deal_whileStatement(){
	//进入是sy是while
        judge_file<< "This is a while statement" << std::endl;
		int code_pos = 0;
		std::string label_before_while;
		std::string while_end_label;

		insymbol();//读取while后面的一个字符
		if (sy != lparent) {//如果不是(
			error(13, lc);//报错
			Skip_Read(Statement_Start_Sys, 9);
			return;
		}
		insymbol();
		if (sy == rparent) {
			//说明没有条件语句。报错
			error(21, lc);
		}
		else {
            label_before_while=Creat_Label(" ", 2, MidCodeT.mdc);//在while条件语句之前生成一个label标签，标签名字记录在label_before_while
            Enter_Mid_List(13, " ", " ",label_before_while, 1);//在while条件语句之前设置标签label_before_while
			code_pos = Deal_Condition();//code_pos是缺失label的mips语句的位置
			//出来时 sy == rparent
			if (sy != rparent) std::cout << "There is a bug in blockdeal--Deal_ifStatement, sy must be rparent" << std::endl;
            insymbol();
            if (!find_sy(Statement_Start_Sys, 9)) {
                error(32, lc);
                Skip_Read(Statement_Start_Sys, 9);
            }
			statement();
			Enter_Mid_List(5, label_before_while," ", " ", 1);//语句块执行结束后，无条件跳转到while之前
			while_end_label=Creat_Label(" ", 2, MidCodeT.mdc);//在while无条件跳转语句之后生成一个label标签，标签名字记录在while_end_label
            Enter_Mid_List(13, " ", " ",while_end_label, 1);//在while无条件跳转语句之后设置while_end_label
			MidCodeT.midcodeArray[code_pos].z = while_end_label;//回填label,不符合while条件的会直接跳转到这个位置，不执行语句块转换成的mips指令
		//出去时 sy 为 Statement_End_Sys中的元素
		}

	}

	//<赋值语句>
	//array_flag = 1, 表示当前ident代表数组; 0代表为变量
	void Deal_assignStatement(int array_flag){
		//进入时sy == ident
		//std::cout << "This is a assign statement" << std::endl;
		judge_file << "This is a assign statement" << std::endl;
		std::string z, y, x = " ";
		symbol tmp_sys[] = { semicolon };//结尾是分号
		int op;
		bool is_char = false;
		int z_pos;		//z在符号表中的位置
		z = id;//查找当前标识符在tab中的信息
		z_pos = find_in_tab(z);
		if (z_pos == 0) error(23, lc);
		if (array_flag == 1) {//是数组的赋值 z[x]=y;
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
			x = Deal_Expression(no_use);	//处理中括号的表达式,出来时sy为Follow_Deal_Expression_Sys[]中的元素
			if (no_use) {	//tmp为表示为字符类型，数组下标不能为字符类型，故错误
				//std::cout << "1" << std::endl;
				error(49, lc);
			}
			else {
				if (string_is_num(x)) {//若下标为数值
					if (string_to_int(x) >= atab.atabArray[tab.tabArray[z_pos].ref].high) {//如果下标超过了上界也要报错
						//std::cout << "2" << std::endl;
						error(49, lc);
					}
				}
			}
			if (sy != rsbrack) {//之后不是]
				error(16, lc);
				Skip_Read(tmp_sys, 1);
				return;
			}
		}
		else {
			op = 4;//普通的赋值 z=y

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

		if (tab.tabArray[z_pos].type == chars) {//如果z的类型是char但y的类型不是char 报错
			if (!is_char) {
				error(46,lc - 1);
			}
		}
		if (tab.tabArray[z_pos].type == ints) {//如果z的类型是char但y的类型不是char 报错
			if (is_char) {
				error(46,lc - 1);
			}
		}
		Enter_Mid_List(op, z, x, y, 0);//做赋值运算

		if (sy != semicolon) {//结尾不是分号
			error(19, lc);
			Skip_Read(tmp_sys, 1);
		}

		//结束时 sy = semicolon
	}

	//<有返回值函数调用语句>
	//涉及的四元式指令  call, push, jal,  set call_label,  z = *RET,
	//返回值为 z = *RET 的z
	std::string Deal_reUseFunc(){
		//进入时 sy == ident(函数名)
		//std::cout << "This is a Deal_reUseFunc" << std::endl;
		judge_file << "This is a Deal_reUseFunc" << std::endl;

		int id_pos = find_in_tab(id);	//id_pos为该函数名在tab表中的位置
		if(id_pos == 0) error(23, lc);
		int paranum = btab.btabArray[tab.tabArray[id_pos].ref].paranum;//定义时参数数目
		int paracount = 0;//调用时参数的数目
		std::string para, z;
		std::string func_name = id ;//函数名
		symbol tmp_sys[] = { rparent };

		std::map <int, types> para_typ_map;//创建一个下标和定义参数类型一一对应的map
		tabe tmp = tab.tabArray[btab.btabArray[tab.tabArray[id_pos].ref].lastpar];//该函数定义的最后一个参数
		for (int i = paranum - 1; i > -1; i--) {//逆着挨个把参数的类型逐个放到map中
			para_typ_map[i] = tmp.type;
			tmp = tab.tabArray[tmp.link];
		}

		Enter_Mid_List(19," ", " ", func_name,0);	//call指令调用
		insymbol();//开始判断参数表

		if (sy != lparent) {//没有(
			error(13, lc);
			Skip_Read(tmp_sys, 1);
			return " ";
		}
		insymbol();
		if (sy == rparent) { //无参数调用的情况
			if (paracount != paranum) {//如果定义时参数数目不是0，报错
				error(36, lc);
				return " ";
			}
		}
		else if(find_sy(Deal_Expression_Start_Sys, 6)){//逐个核对类型
			bool is_char = false;
			para = Deal_Expression(is_char);//para记录传入的实参
			//	std::cout << is_char << std::endl;
			//std::cout << types_info[para_typ_map[paracount]] << std::endl;
			if (is_char) {//实际参数是char
				if (para_typ_map[paracount] != chars) {//定义时不是char，报错
					error(45, lc);
				}
			}
			else {//实际参数是数值
				if (para_typ_map[paracount] != ints) {//定义时不是int，报错
					error(45, lc);
				}
			}

			Enter_Mid_List(18, " ", " ", para, 0);	//push指令 passpara y
			paracount++;//参数个数增加
			while (sy == comma) {//逐个核对
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
				Enter_Mid_List(18, " ", " ", para, 0);	//push指令
				paracount++;
			}
			if (paracount != paranum) {//定义的参数个数和实际传入的参数个数不一致，报错
				error(36, lc);
				Skip_Read(tmp_sys, 1);
				return " ";
			}
			if (sy != rparent) {//参数表后面跟的不是) 报错
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
		Enter_Mid_List(21,func_name + "_begin_"," "," ", 0);		//jal语句，跳转到定义函数时的(func_name)_begin标签
		z = Creat_Tempvar();//生成一个临时变量承接返回值

		Enter_Mid_List(13," "," ",Creat_Label(" ",4,MidCodeT.mdc),3);//?

		//有返回值调用函数才有
		Enter_Mid_List(4,z, " ", "*RET",0);				//z = *RET
		//出去时sy == rparent
		return z;
	}

	//<无返回值函数调用语句>
	//涉及的四元式指令  call, push, jal, set call_label
	void Deal_voUseFunc(){
		//进入时 sy == ident(函数名)
		//std::cout << "This is a Deal_voUseFunc" << std::endl;
		judge_file << "This is a Deal_voUseFunc" << std::endl;

		int id_pos = find_in_tab(id);	//id_pos为该函数名在tab表中的位置
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

		Enter_Mid_List(19, " ", " ", func_name, 0);	//call指令调用
		insymbol();

		if (sy != lparent) {
			error(13, lc);
			Skip_Read(tmp_sys, 1);
			return;
		}
		insymbol();
		if (sy == rparent) { //无参数调用的情况
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
			Enter_Mid_List(18, " ", " ", para, 0);	//push指令
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
				Enter_Mid_List(18, " ", " ", para, 0);	//push指令
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
		Enter_Mid_List(21, func_name + "_begin_", " ", " ", 0);		//jal语句

		Enter_Mid_List(13, " ", " ", Creat_Label(" ", 4, MidCodeT.mdc), 3);//?
		//出去时sy == rparent
	}

	//<读语句>
	void Deal_scanfStatement(){
		//进入时 sy == scanfsy
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
		//出来时 sy == semicolon
	}

	//<情况表>
	//参数x为switch中的表达式,switch_end_label为switch结束的位置标签,用于每一个case后面的break
	void Deal_caseTable(std::string x, std::string switch_end_label, bool is_char){
		//进入时sy == lcbrack（{）
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

				if (is_char) error(47, lc);//待判断是char型，case带运算符，报错
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
			Enter_Mid_List(7, " ", x, y, 0);	//等于的时候继续执行,不等于的时候跳转,所以此处的跳转指令应该为不等号

			insymbol();
			if (!find_sy(Statement_Start_Sys, 9)) {
				error(32, lc);
				Skip_Read(Statement_Start_Sys, 9);
			}
			statement();

			//case语句处理完,跳转到switch结束位置
			Enter_Mid_List(5, switch_end_label, " ", " ", 0);
			j_label = Creat_Label(" ", 2, MidCodeT.mdc);
			Enter_Mid_List(13, " ", " ", j_label, 1);
			//跳转指令回填,跳到case结束的位置,即goto语句的后面
			MidCodeT.midcodeArray[pos].z = j_label;		//此处的j_label 为pc-1,因为在前面又填了一个goto语句

			//此处sy为Statement_End_Sys中的一个
			insymbol();
		} while (sy == casesy);
		//std::cout<<"Now there are "<<case_int_list.size()<<" elements in the set case_int_list"<<std::endl;
		//std::cout<<"Now there are "<<case_char_list.size()<<" elements in the set case_char_list"<<std::endl;
		case_int_list.clear();
		case_char_list.clear();
        if(sy == defaultsy){//如果当前不是case了，可能是有default
            insymbol();//再读一个字符，应该是冒号
            if (sy != colon) {
				error(43, lc);
				Skip_Read(tmp_sys, 1);
				return;
			}
			insymbol();//然后是语句的处理
			if (!find_sy(Statement_Start_Sys, 9)) {
				error(32, lc);
				Skip_Read(Statement_Start_Sys, 9);
			}
			statement();//生成语句，出来的时候为Statement_End_Sys里面的内容，如; }
			insymbol();//读入下一个
        }
		if (sy != rcbrack) {//结束switch的{}
			error(43,lc);
			Skip_Read(tmp_sys, 1);
		}
		//结束时sy == rcbrack
	}
	//<写语句>
	void Deal_printfStatement(){
		//进入时 sy == printfsy
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
		if (sy == stringcon) {//输出一个字符串
			char stemp[SMAX];
			for (int i = 0; i < sleng; i++) {
				stemp[i] = stab.stringArray[inum + i];//从stringArray中取出字符串
			}
			stemp[sleng] = '\0';
			x = stemp;
			insymbol();
			if (sy == comma) {//逗号，还有表达式
				insymbol();
				if (find_sy(Deal_Expression_Start_Sys, 6)) {
					y = Deal_Expression(is_char);
					if (is_char) z = "char";//表达式是一个字符，则输出时输出字符
					if (sy != rparent) {
						error(14, lc);
						Skip_Read(tmp_sys, 1);
						return;
					}
					Enter_Mid_List(17, z, x, y, 0);//输出中间代码
					insymbol(); //读后面的分号
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
			else {//只有一个字符串
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
		else if (find_sy(Deal_Expression_Start_Sys, 6)) {//表达式
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
		//出去时 sy == semicolon
	}

	//<情况语句>
	void Deal_switchStatement() {
		//进入时 sy == switchsy
		//std::cout << "This is a switch statement" << std::endl;
		judge_file << "This is switch statement" << std::endl;
		std::string z, x, y;	//x作为switch()括号里的操作数,y代表各个case的操作数
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
		//出Deal_Expression时 sy为follow(Deal_Expression)里面的东西
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
		//出Deal_caseTable时sy == rcbrack

		Enter_Mid_List(13, " ", " ", switch_end_label, 1);
		//出来时 sy 为rcbrack
	}

	//<返回语句>
	void Deal_returnStatement(){
		//进入时 sy == returnsy
		//std::cout << "This is a return statement" << std::endl;
		judge_file << "This is return statement" << std::endl;
		symbol tmp_sys[] = { semicolon };
		std::string y;

		bool is_char = false;
		insymbol();

		if (sy == semicolon) {	//单独一个return
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
			//出Deal_Expression时sy为Follow_Deal_Expression_Sys中的元素
			if(is_char)return_flag = 3;	//返回值为chars
			else return_flag = 2;	//返回值为ints
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
		//出去时 sy == semicolon
	}

	//<语句>
	void statement() {
		//进入statement时, sy为Statement_Start_Sys中的元素
		// 调用地方: statement(),statementCompond, Deal_ifStatement, do_Deal_whileStatement,
		int pos;
		std::string z, x, y;
		switch (sy) {
		case semicolon:	//单独一个分号处理
			//std::cout << "This is a single ; in statement" << std::endl;
			break;
		case ifsy:	//条件语句处理
			//进入时sy == if
			Deal_ifStatement();
			break;//此时 sy 为Statement_End_Sys中的元素
		case whilesy:	//循环语句
			//进入时sy == dosy
			Deal_whileStatement();
			break;//此时sy == rparent
		case lcbrack:	//{ 语句 } 处理
			insymbol();
			while (sy != rcbrack) {
				Skip_Read(Statement_Start_Sys, 9);
				//进入statement时, sy为Statement_Start_Sys中的元素
				statement();	//进入语句处理函数
				//出statement时, sy 为 Statement_End_Sys中的元素
				insymbol();	//读语句后面的一个单词
			}
			break;	//此时sy == rcbrack
		case ident:		//有返回值函数调用、无返回值函数调用、赋值语句处理
			pos = find_in_tab(id);	//在tab表中查询当前单词的信息
			if(pos == 0) error(23, lc);
			if (pos != 0) {
				switch (tab.tabArray[pos].obj) {
				case constant://常量开头做运算都是不合法的
					error(37, lc);
					Skip_Read(Statement_End_Sys, 3);
					break;
				case variable:	//赋值语句
					Deal_assignStatement(0);
					//此时sy == semicolon
					break;
				case array:		//标识符[表达式]  赋值语句
					Deal_assignStatement(1);
					//此时sy == semicolon
					break;
				case function:
					if (tab.tabArray[pos].type == notyp) {	//无返回值函数调用
						//进入Deal_voUseFunc时,sy == identsy(函数名)
						Deal_voUseFunc();
					}
					else {
						//进入Deal_reUseFunc时,sy == identsy(函数名)
						Deal_reUseFunc();
					}

					//此时 sy为 rparent
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
			break;	//此时 sy == semicolon
		case scanfsy:	//读语句处理
			//进入时sy == scanfsy
			Deal_scanfStatement();
			break;	//此时 sy == semicolon
		case printfsy:	//写语句处理
			//进入时sy == printfsy
			Deal_printfStatement();
			break;//此时sy == semicolon
		case switchsy:	//情况语句处理
			//进入时 sy == switchsy
			Deal_switchStatement();
			if (sy != rcbrack) std::cout << "There is a bug in blockdeal--statement, after Deal_switchStatement()" << std::endl;
			break; //此时 sy == rcbrack;
		case returnsy:	//返回语句处理
			//进入时 sy == returnsy
			Deal_returnStatement();
			break;//此时 sy == semicolon
		default:
			error(32, lc);
			Skip_Read(Statement_End_Sys, 3);
			break;
		}
		if (!find_sy(Statement_End_Sys, 3)) std::cout << "There is a bug in blockdeal--statement, at the end of the function" << std::endl;

		//出statement时, sy为 Statement_End_Sys中的一个
	}
	//<复合语句>
	//传入值为函数局部变量所占空间的大小、返回值类型、目前已声明变量所处函数数据区相对地址,返回值为该函数最后一个局部变量在tab中的位置
	int statementCompond(int& var_size, types& return_typ, int& pv_addr){
		//入口时sy指的是左括号之后的第一个单词
		symbol statementCompond_start_sys[] = { constsy, intsy, charsy, semicolon, ifsy, whilesy, lcbrack, ident, scanfsy, printfsy, switchsy, returnsy, rcbrack};//13个
		types tmp_type;
		int start_pv_addr = pv_addr;//记录处理该函数前已声明的变量占据数据区的相对地址
		int var_flag = 0;	//是否有局部变量的标志位,用于后面的return返回

		if (!find_sy(statementCompond_start_sys, 13)) {
			error(32, lc);
			Skip_Read(statementCompond_start_sys, 13);
		}

		//局部常量声明
		while (sy == constsy) {
			Deal_Const();	//从Deal_Const里面出来时sy为';'后面的单词
		}
		Skip_Read(statementCompond_start_sys, 13);	//跳读, 此时sy不可能为constsy,否则出不了循环



		//局部变量声明
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

			//test sy 是否为 comma, semicolon, lsbrack 都不是的话报错
			symbol tmpsys[] = { comma, semicolon, lsbrack };
			if (!find_sy(tmpsys, 3)) {
				error(28, lc);
				Skip_Read(statementCompond_start_sys, 13);
				//std::cout << "This is a var_declare statement" << std::endl;
				judge_file << "This is var_declare statement" << std::endl;
				continue;
			}

			//此时 sy == comma || sy == semicolon || sy == lsbrack
			Deal_Varible(tmp_type, pv_addr);	//出来时读到的是';'之后的单词
			var_flag = 1;//var_flag为1说明该复合语句内部还声明了其他局部变量
		}
		var_size = pv_addr - start_pv_addr; //局部变量所占的空间大小

		//从此往下不会再进行enter操作，只会陆续插入一些中间代码

		//此处为变量声明的分号结束之后的第一个单词
		symbol tmp_sys[] = { semicolon, ifsy, whilesy, lcbrack, ident, scanfsy, printfsy, switchsy, returnsy, rcbrack };
		Skip_Read(tmp_sys, 10);

		while (sy != rcbrack) {
			Skip_Read(Statement_Start_Sys, 9);//找到各种语句的开头字符
			//进入statement时, sy为Statement_Start_Sys中的元素
			statement();	//进入语句处理函数
			//出statement时, sy 为 Statement_End_Sys中的元素
			insymbol();
			if ((sy != rcbrack) && (!find_sy(Statement_Start_Sys, 9))) {
				error(48,lc);
			}
		}


		if (var_flag == 1) return tab.t - 1;//如果复合语句内（即函数内）又声明了局部变量，则外部display区指向的btab区的last信息也要做修改
		else return btab.btabArray[btab.b-1].lastpar;//没有声明局部变量，故last也等于btab里面的lastpar即声明的参数的位置
		//出复合语句处理时,sy == rcbrack
	}


	//<无返回值函数定义>
	void Deal_voFunc(types typ){
		//进该模块时,sy == lparent
		//std::cout << "This is a voidFunc declare statement" << std::endl;
		judge_file << "This is a voidFunc declare statement" << std::endl;
		int para_num = 0, var_size = 0,	pv_addr = 3;	//pv_addr 相对于函数基址的相对地址, 0、1、2为内务区
		types return_typ = notyp;//最后比对return_typ与typ是否一致  typ为声明时的返回值类型
		std::string func_name = id;

		int init_t_num, last_t_num;		//该函数定义中第一个和(最后一个临时变量 + 1) 的编号, 用于后面得到t_varnum和各个临时变量的相对地址

		init_t_num = temp_num;//记录下读取该函数前的临时变量计数

		return_flag = 0;//无返回值

		enter(id, function, typ);//在tab中填入函数名信息
            enterFunc(tab.t - 1, typ);

		level++;	//进入分程序层(局部)
		display[level] = btab.b - 1;	//更新display区当前层分程序的索引表

		//压入函数声明四元式指令
		Enter_Mid_List(14, " ", types_info[typ], func_name, 0);//func 返回值类型,函数名称

		insymbol();

		if (sy == rparent) {	//无参数
			btab.btabArray[btab.b - 1].lastpar = 0;
			btab.btabArray[btab.b - 1].paranum = 0;
			insymbol();
		}
		else if ((sy == intsy) || (sy == charsy)) {	//有参数
			//在参数表中压入相应的四元式指令
			btab.btabArray[btab.b - 1].lastpar = Deal_Para(para_num, pv_addr);//处理参数表,记录参数个数和地址，返回值为最后一个参数在tab中的位置
			//出来后为')'之后的单词
			btab.btabArray[btab.b - 1].paranum = para_num;//参数个数
		}
		else {
			//出错跳读直到遇见左大括号
			error(31, lc);
			symbol tmp_sys[] = { lcbrack };
			Skip_Read(tmp_sys, 1);
		}
		if (sy != lcbrack) {
			error(17, lc);
			symbol tmp_sys[] = { lcbrack };
			Skip_Read(tmp_sys, 1);
		}

		//到达此处的条件是遇到左大括号

		//函数参数声明结束,将goto语句压入四元式
		//Enter_Mid_List(5, func_name + "_end_", " ", " ", 0);

		//函数语句开始
		Enter_Mid_List(13, " ", " ", Creat_Label(func_name, 0, MidCodeT.mdc), 4);		//创建函数开始语句标签（4），并setlabel

		insymbol();
		if (sy == rcbrack) {
			//相当于复合语句直接为空
			btab.btabArray[btab.b - 1].last = btab.btabArray[btab.b - 1].lastpar;//没有局部变量，所以最后一个标识符也就是最后一个参数
			btab.btabArray[btab.b - 1].varsize = 0;//局部变量个数为0
			//结束函数跳转四元式 本来是在复合语句中处理,但这里没有进入所以要处理一下
			Enter_Mid_List(12, " ", " ", " ", 0);// ret y  函数返回操作,返回值y赋予*RET  return temp / return 12
		}
		else {
			//进入复合语句,此时sy为左括号之后的第一个单词
			btab.btabArray[btab.b - 1].last = statementCompond(var_size, return_typ, pv_addr);//修改最后一个标识符的位置和varsize
			btab.btabArray[btab.b - 1].varsize = var_size;//修改局部变量在S栈中所占存储单元的数目
			if ((return_flag != 0) && (return_flag != 1)) error(12, lc - 1);//无返回值函数定义不能有返回值
			if (MidCodeT.midcodeArray[MidCodeT.mdc - 1].op != 12) {//如果无返回值函数定义的复合语句里面没有return语句
				//函数最后一条不是返回语句
				Enter_Mid_List(12, " ", " ", " ", 0);	//void的函数直接增加一条,要不跳不回去
			}
		}

		last_t_num = temp_num;//得到目前临时变量的计数
		btab.btabArray[btab.b - 1].t_varnum = last_t_num - init_t_num;//设置该函数中涉及到的临时变量的个数

		//设置T_MAP
		for (int i = init_t_num; i < last_t_num; i++) {
			int offset = pv_addr + i - init_t_num;//从涉及参数表和局部变量的pv_addr开始 继续计算偏移地址
			T_Map["#" + int_to_string(i)].stack_addr = offset;//T-MAP中以 #-编号 为键值，修改对应var_info中的相对栈的地址偏移
		}

		//此处sy == rcbrack才是从复合语句中正常出来
		if (sy != rcbrack) {
			std::cout << "There is a bug in blockdeal--Deal_voFunc, after the transfer of statementCompond" << std::endl;
		}
		//在此处压入函数结束标签
		//Enter_Mid_List(13, " ", " ", Creat_Label(func_name, 1, MidCodeT.mdc), 1);	//创建label与PC的对应关系时mdc指的刚好是setlab的位置,还未进行++操作
		level--;	//退出分程序层
		insymbol();	//读取'}'后的下一个单词
	}

	//<有返回值函数定义>
	void Deal_reFunc(types typ){
		//进该模块时,sy == lparent
		//std::cout << "This is a returnFunc declare statement"<<std::endl;
		judge_file << "This is a returnFunc declare statement" << std::endl;
		int para_num = 0, var_size = 0, pv_addr = 3;	//pv_addr为参数、局部变量相对于当前数据区的相对地址 从3开始, 0、1、2的位置为内务区
		types return_typ = notyp;//最后比对return_typ与typ是否一致  typ为声明时的返回值类型
		std::string func_name = id;
		int init_t_num, last_t_num;

		init_t_num = temp_num;

		return_flag = 0;

		enter(id, function, typ);
        enterFunc(tab.t - 1, typ);


		//进入时level = 0, 出去时也得是0, 函数内部处理为1
		level++; //当前分程序层为1
		display[level] = btab.b - 1;	//更新当前层分程序的索引表

		//压入函数声明四元式指令
		Enter_Mid_List(14," ",types_info[typ],func_name, 0);

		insymbol();
		if (sy == rparent) {	//无参数
			btab.btabArray[btab.b - 1].lastpar = 0;
			btab.btabArray[btab.b - 1].paranum = 0;
			insymbol();
		}
		else if ((sy == intsy) || (sy == charsy)) {	//有参数
			//在参数表中压入相应的四元式指令
			btab.btabArray[btab.b-1].lastpar = Deal_Para(para_num, pv_addr);//处理参数表,出来后为')'之后的单词
			btab.btabArray[btab.b - 1].paranum = para_num;
		}
		else {
			//出错跳读直到遇见左大括号
			error(31, lc);
			symbol tmp_sys[] = { lcbrack };
			Skip_Read(tmp_sys,1);
		}
		//从参数表处理出来读的单词是 ')'后面一个单词
		if (sy != lcbrack) {
			error(17,lc);
			symbol tmp_sys[] = { lcbrack };
			Skip_Read(tmp_sys, 1);
		}

		//到达此处的条件是遇到左大括号

		//函数参数声明结束,将goto语句压入四元式
		//Enter_Mid_List(5, func_name + "_end_", " ", " ", 0);

		//函数开始label,压入四元式 setlab
		Enter_Mid_List(13," ", " ",Creat_Label(func_name,0,MidCodeT.mdc),4);

		insymbol();
		if (sy == rcbrack) {
			//相当于复合语句直接为空,就不进去了,免得尴尬
			btab.btabArray[btab.b - 1].last = btab.btabArray[btab.b - 1].lastpar;
			btab.btabArray[btab.b - 1].varsize = 0;
			error(12, lc - 1);	//有返回值函数才会报错
			//结束函数跳转四元式 本来是在复合语句中处理,但这里没有进入所以要处理一下
			Enter_Mid_List(12, " ", " ", " ", 0);
		}
		else {
			//进入复合语句
			btab.btabArray[btab.b - 1].last = statementCompond(var_size, return_typ, pv_addr);
			btab.btabArray[btab.b - 1].varsize = var_size;//局部变量所占空间的大小
			if (return_flag == 2) {	//返回值为ints
				if (typ != ints) error(12, lc - 1);
			}
			else if (return_flag == 3) {	//返回值为chars
				if (typ != chars) error(12, lc - 1);
			}
			else {//复合语句中没有返回语句或者返回的值不是2/3型
				error(12, lc - 1);
			}
			if (MidCodeT.midcodeArray[MidCodeT.mdc - 1].op != 12) {
				//函数最后一条不是返回语句
				Enter_Mid_List(12, " ", " ", " ", 0);	//直接增加一条,返回调用位置
			}
		}

		last_t_num = temp_num;
		btab.btabArray[btab.b - 1].t_varnum = last_t_num - init_t_num;

		//设置T_MAP
		for (int i = init_t_num; i < last_t_num; i++) {
			int offset = pv_addr + i - init_t_num;
			T_Map["#" + int_to_string(i)].stack_addr = offset;
		}



		//此处sy == rcbrack才是从复合语句中正常出来
		if (sy != rcbrack) {
			std::cout << "There is a bug in blockdeal--Deal_reFunc, after the transfer of statementCompond" << std::endl;
		}


		level--; //退出当前分程序层

		insymbol();	//'}'的下一个单词
	}


	//<主运行函数处理>
	void Deal_Main(){
		//进入该模块时 sy == lparent
		//std::cout << "This is a main declare statement" << std::endl;
		judge_file << "This is a main declare statement" << std::endl;
		int para_num = 0, var_size = 0, pv_addr = 3;	//pv_addr 相对于函数基址的相对地址, 3个内务区,为了call指令翻译为mips的一致性
		types return_typ = notyp;//最后比对return_typ与typ是否一致  typ为声明时的返回值类型
		std::string func_name = "main";
		int init_t_num, last_t_num;

		init_t_num = temp_num;


		enter(func_name, function, notyp);
        enterFunc(tab.t - 1, notyp);



		level++;	//进入分程序层(局部)
		display[level] = btab.b - 1;	//更新当前层分程序的索引表

		//压入函数声明四元式指令
		Enter_Mid_List(14, " ", types_info[notyp], func_name, 0);

		insymbol();
		if (sy != rparent) {	//main函数无参数
			error(10, lc);

		}
		//直接跳读到左大括号
		symbol tmp_sys[] = { lcbrack };
		Skip_Read(tmp_sys, 1);


		//函数开始label,压入四元式 setlab
		Enter_Mid_List(13, " ", " ", "main", 2);		//2标志为main函数开始位置

		//到达此处的条件是遇到左大括号
		insymbol();
		if (sy == rcbrack) {
			//相当于复合语句直接为空
			btab.btabArray[btab.b - 1].last = btab.btabArray[btab.b - 1].lastpar;
			btab.btabArray[btab.b - 1].varsize = 0;
		}
		else {
			//进入复合语句
			btab.btabArray[btab.b - 1].last = statementCompond(var_size, return_typ, pv_addr);
			btab.btabArray[btab.b - 1].varsize = var_size;
			if (return_typ != notyp)	error(12, lc);	//声明类型与返回类型不匹配

		}

		last_t_num = temp_num;
		btab.btabArray[btab.b - 1].t_varnum = last_t_num - init_t_num;

		//设置T_MAP
		for (int i = init_t_num; i < last_t_num; i++) {
			int offset = pv_addr + i - init_t_num;
			T_Map["#" + int_to_string(i)].stack_addr = offset;
		}

		//此处sy == rcbrack才是从复合语句中正常出来
		if (sy != rcbrack) {
			std::cout << "There is a bug in blockdeal--Deal_Main, after the transfer of statementCompond" << std::endl;

		}

		level--;	//退出分程序层
	}

	// <程序> 处理
	void block(){
		types tmp_type;
		symbol tmp_sy;	//判别是否为miansy的时候使用
		int global_var_addr = 0, global_var_num = 0;
		judge_file.open("gram_output.txt", std::fstream::out);
		insymbol();
		Skip_Read(Block_Start_Sys, 4);
		//进行const处理
		while(sy == constsy){
			Deal_Const();	//从Deal_Const里面出来时sy为';'后面的单词

		}

		//全局变量处理
		while(1){
			Skip_Read(Block_Start_Sys, 4);//检测 sy 是否为 constsy, intsy, charsy 或者 voidsy, 都不是的话跳读
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
			//test sy 是否为 comma, semicolon, lparent, lsbrack 都不是的话报错
			symbol tmpsys[] = { comma, semicolon, lparent, lsbrack};
			if (!find_sy(tmpsys, 4)) {
				error(28, lc);
				Skip_Read(Block_Start_Sys, 4);
				//std::cout << "This is a var_declare statement" << std::endl;
				judge_file << "This is var_declare statement" << std::endl;
				continue;
			}

			if(sy == lparent) break;	//函数定义
			//进变量定义之前的条件
			if (tmp_type == notyp) {	//变量只能是char或者int类型
				error(28, lc);
				Skip_Read(Block_Start_Sys, 4);
				//std::cout << "This is a var_declare statement" << std::endl;
				judge_file << "This is var_declare statement" << std::endl;
				continue;
			}
			Deal_Varible(tmp_type,global_var_addr);	//出来时读到得是';'之后的单词
		}
	//	std::cout <<"tab.t-1:"<< tab.t-1 << std::endl;
	//	std::cout <<"Stack.topaddr:"<<Stack.topaddr<< std::endl;
		//处理btab.btabArray[0],把全局加进去
		btab.btabArray[0].last = tab.t - 1;
		btab.btabArray[0].varsize = global_var_addr; //全局变量所占数据区的总大小

		//把全局量压栈
		for (int i = 0; i < tab.t; i++) {
			Stack.space[Stack.topaddr++] = tab.tabArray[i].adr;//把全局量的值压进栈
		}
		Stack.globalvaraddr = 0;	//全局量开始的位置为0

		//从全局变量处理出来的条件:  sy == lparent
		if (sy != lparent) {
			std::cout << "There is a bug in blockdeal, before funcDeal" << std::endl;
		}

		if (tmp_sy != mainsy) {
			//处理出来以后的第一个函数
			if (tmp_type == notyp) {	//无返回值函数定义
				Deal_voFunc(tmp_type);
			}
			else {		//有返回函数定义
				Deal_reFunc(tmp_type);
			}
			//从函数处理出来后为'}'后面一个单词
			//循环处理剩下的函数,从int、char或者void开始
			while (1) {
				Skip_Read(Block_Start_Sys, 4);	//检测 sy 是否为 constsy, intsy, charsy 或者 voidsy, 都不是的话跳读
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

				//在此位置 sy == lparent;
				if (tmp_sy == mainsy) break;	//主运行函数
				if (tmp_type == notyp) {	//无返回值函数定义
					Deal_voFunc(tmp_type);
				}
				else {		//有返回函数定义
					Deal_reFunc(tmp_type);
				}

			}
		}
				//程序运行到此步骤的条件是tmp_sy为mainsy
		//且 sy == lparent
		if (tmp_sy != mainsy) {
			std::cout << "There is a bug in blockdeal, before Deal_Main" << std::endl;
		}
		//std::cout << "Begin main deal" << std::endl;
		//main函数处理,此时id为main,sy为mainsy
		if (tmp_type != notyp) {	//此时出现notyp只能是void,因为在上述while里面的main函数break前面进行了Skip_Read检测
			error(10, lc);
		}
		//进入时sy == lparent
		Deal_Main();

		//退出时 sy == rcbrack

	}
	}
