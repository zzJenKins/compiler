/*
	dag图代码优化
*/
#include "Optimized_Mid_Code.h"
namespace compiler{
	struct tree_node {
		//叶子节点对应的属性
		std::string name;	//节点所对应的变量名, 叶子结点才会有

		//中间节点对应属性
		int op_id;	// +,-,*,/,= 的操作符对应的midop编号, -1 代表其为叶子节点
		struct tree_node* lchild;	//左孩子node编号
		struct tree_node* rchild; //右孩子node编号
		std::vector<std::string> var_names;	//该节点所对应的变量名称链表

		//共有
		bool is_mid_node;	//是否为中间节点
		std::vector<struct tree_node*> parents; //父节点, 在dealFather中对两个孩子节点的父节点进行更新操作
	};

	std::vector<tree_node*> dag_queue;	//从DAG图导出中间代码时使用
	std::map<std::string, tree_node*> Node_Map;	//Dag图的变量名-节点图, 在每次全部导出之后清空
	std::map<std::string, std::string> F_S_Map;		//F变量名与原名称的对应关系
	std::vector<std::string> temp_var_avilable;	//因为公共子表达式删除闲置下来的临时变量名, 在get_Opt_Code一开始进行生成
	std::map<std::string, std::string> Replace_Map;	//临时变量的新旧变量名称对照表,以函数体为单位对导出后的中间代码段进行变量名称更新, 在get_Opt_Code中生成

	int count_for_f = 0;	//每个局部块处理完之后清零

	//用于处理a0 = a这种情况, 统一用F0、F1等来代替
	std::string Create_f_Name() {
		return "@" + int_to_string(count_for_f++);
	}


	bool In_Node_Map_Or_Not(tree_node* t) {
		for (auto iter = Node_Map.begin(); iter != Node_Map.end();++iter) {
			if (iter->second == t) {
				return true;
			}
		}
		return false;
	}

	void Delete_From_NodeMap(tree_node* t) {
		for (auto iter = Node_Map.begin(); iter != Node_Map.end();) {
			if (iter->second == t) {
				Node_Map.erase(iter++);
			}
			else {
				iter++;
			}
		}
	}

	//从Node_Map中得到一个没有父节点的中间节点， 得到后删除
	tree_node* get_correct_node() {
		auto iter = Node_Map.begin();
		tree_node* t = NULL;
		for (; iter != Node_Map.end();++iter) {

			if (iter->second->is_mid_node && iter->second->parents.size() == 0) {
				t = iter->second;
				break;
			}
		}
		if(t != NULL) Delete_From_NodeMap(t);
		return t;
	}


	//在node_map表中寻找当前变量名是否已经存在, 若存在返回其节点指针
	tree_node* findInNodeMap(std::string name) {
		auto iter = Node_Map.find(name);
		if (iter == Node_Map.end()) return NULL;	//没找到
		else {
			return iter->second;
		}

	}

	bool In_Replace_Map_ornot(std::string name) {
		auto iter = Replace_Map.find(name);
		if (iter == Replace_Map.end()) return false;	//没找到
		else {
			return true;
		}
	}

	//判断当前处理的表达式是否已经有等价的节点, 若有则返回其节点指针
	tree_node* expEqual(int op, tree_node* lchild, tree_node* rchild) {
		auto iter = Node_Map.begin();
		tree_node* temp;
		for (; iter != Node_Map.end(); ++iter) {
			temp = iter->second;
			if ((temp->op_id == op) &&
				(((temp->lchild == lchild) && (temp->rchild == rchild)) || ((temp->lchild == rchild) && (temp->rchild == lchild)))) {
				return temp;
			}
		}
		//没有找到等价的
		return NULL;
	}

	/*
		参数:	name 变量名(有可能为数字, 数字肯定为叶子节点)
		返回值类型: tree_node*
		函数作用: 处理操作数的变量名, 若已经存在则返回其节点指针, 若不存在则生成一个叶子结点
	*/
	tree_node* dealChild(std::string name) {
		tree_node * nodep = findInNodeMap(name);
		if (nodep != NULL) return nodep;
		//作为操作数未在之前出现过, 则其为叶子节点
		nodep = new tree_node;
		nodep->op_id = -1;
		nodep->lchild = NULL;
		nodep->rchild = NULL;
		nodep->is_mid_node = false;
		nodep->name = name;

		Node_Map[name] = nodep;
		return nodep;
	}

	/*
		参数:	name 变量名(局部变量或者临时变量, 对于局部变量要同时插入两个map中)
				is_assign在简单的赋值中设置为1, 数组赋值设置为0
		返回值类型: void
		函数作用: 处理表达式结果的变量名, 表达式等价的条件: (op == op_id) && (((lparent == lparent) && (rparent == rparent)) || ((lparent == rparent) && (rparent == lparent)))
		注意: 若变量名已经存在而表达式不等价, 相当于对原变量进行重新定义, 则需要进行类似a0 = a的处理
	*/

	void dealFather(std::string name, int op, tree_node* lchild, tree_node* rchild, bool is_assign) {
		tree_node *z_node = expEqual(op, lchild, rchild);
		if (z_node == NULL) {
			tree_node *temp = findInNodeMap(name);	//查找当前变量名是否已经作为节点使用, 对应a0 = a这种处理情况
			if (temp != NULL) {	//若已经存在该变量名的节点
				if (temp->is_mid_node) {	//中间节点
					std::vector<std::string>::iterator iter = std::find(temp->var_names.begin(), temp->var_names.end(), name);	//查找到在中间节点的位置
					if (iter == temp->var_names.end()) {
						//std::cout << "There is a bug in optimizedeal--dealFather: iter can't be temp->var_names.end()" << std::endl;
						return;
					}
					std::string new_name = Create_f_Name();
					F_S_Map[new_name] = *iter;
					*iter = new_name;
					Node_Map[*iter] = temp;
				}
				else {		//叶子节点
					std::string new_name = Create_f_Name();
					F_S_Map[new_name] = temp->name;
					temp->name = new_name;	//更改变量名
					Node_Map[temp->name] = temp;
				}
			}

			//简单赋值且为 某个值等于中间节点的情况, 则把该值加入到该变量的变量链表中
			if (is_assign && lchild->is_mid_node) {
				lchild->var_names.push_back(name);
				z_node = lchild;
			}
			else {
				z_node = new tree_node;
				z_node->is_mid_node = true;
				z_node->name = " ";
				z_node->lchild = lchild;
				z_node->rchild = rchild;
				z_node->op_id = op;
				z_node->var_names.push_back(name);
				lchild->parents.push_back(z_node);
				if (rchild != NULL)rchild->parents.push_back(z_node);
			}
			/*
				处理name已经存在在Node_Map中的情况
				对于 a0 = a的情况, 找因为局部表达式消除得到的临时变量来替换(若没有则对该代码块不进行优化处理)
			*/
		}
		else {		//已经有等价的表达式
			z_node->var_names.push_back(name);//将变量名加入该节点
		}
		Node_Map[name] = z_node;
	}

	//递归调用得到以root为根节点的子树
	void getdag_queue(tree_node* root){
		if (root == NULL) {
			//std::cout << "There is a bug in optimizedeal--getdag_queue: root be NULL" << std::endl;
			return;
		}
		if (!root->is_mid_node) {
			//std::cout << "There is a bug in optimizedeal--getdag_queue: root must be midcode" << std::endl;
			return;
		}
		if (root->parents.size() != 0) {
			//std::cout << "There is a bug in optimizedeal--getdag_queue: root'parents must be 0" << std::endl;
			return;
		}

		//将节点拿到队列中
		tree_node* lt = NULL;
		tree_node* rt = NULL;
		dag_queue.push_back(root);

		Delete_From_NodeMap(root);

		//将root左子树和右子树的父节点进行更新
		lt = root->lchild;
		rt = root->rchild;

		if (lt->is_mid_node && In_Node_Map_Or_Not(lt)) {

			std::vector<tree_node*>::iterator iter = std::find(lt->parents.begin(), lt->parents.end(), root);	//查找到在中间节点的位置
			if (iter == lt->parents.end()) {
				//std::cout << "There is a bug in optimizedeal--getdag_queue: iter can't be lt->parents.end()" << std::endl;
				return;
			}
			lt->parents.erase(iter);
		}
		if (rt != NULL && rt->is_mid_node && In_Node_Map_Or_Not(rt)) {
			std::vector<tree_node*>::iterator iter = std::find(rt->parents.begin(), rt->parents.end(), root);
			if (iter == rt->parents.end()) {
				//std::cout << "There is a bug in optimizedeal--getdag_queue: iter can't be rt->parents.end()" << std::endl;
				return;
			}
			rt->parents.erase(iter);
		}

		//选择进入左子树还是右子树还是直接return
		if (lt->is_mid_node && lt->parents.size() == 0 && In_Node_Map_Or_Not(lt)) {
			getdag_queue(lt);
		}
		if (rt != NULL && rt->is_mid_node && rt->parents.size() == 0 && In_Node_Map_Or_Not(lt)) {
			getdag_queue(rt);
		}
		return;

	}


	//若需要被替换的F数量大于多余的临时变量, 返回false， 否则返回true
	bool dealNodeVarnames() {
		int i;
		int fcount = 0, tcount = 0;
		//遍历中间节点对count_for_f计数
		for (i = 0; i < dag_queue.size(); i++) {

			if (dag_queue[i]->var_names.size() == 1) {
			  //  std::cout << "dag_queue[i]->var_names:"<<dag_queue[i]->var_names[0] << std::endl;
				if (dag_queue[i]->var_names[0][0] == '@') {	//只有一个名字且为F打头
					fcount++;
				}
				dag_queue[i]->name = dag_queue[i]->var_names[0];
			}
			else if (dag_queue[i]->var_names.size() > 1) {
				auto iter = dag_queue[i]->var_names.begin();
				bool only_F = false;
				bool have_others = false;
				bool have_svar = false;	//标志该节点是否有局部变量

				//先处理F
				for (; iter != dag_queue[i]->var_names.end(); ) {
					if (iter[0][0] == '@') {
                       // std::cout << "iter:"<<iter[0] << std::endl;
                    //    std::cout << ((iter + 1) == dag_queue[i]->var_names.end()) << std::endl;
                       // std::cout << "F_S_MAP[@0]"<<F_S_Map["@0"] << std::endl;
                                            //    std::cout << "have_others"<<have_others << std::endl;
						if (((iter + 1) == dag_queue[i]->var_names.end()) && (have_others == false)) {
							only_F = true;
							dag_queue[i]->name = *iter;
							fcount++;
							break;
						}
						else {
							//std::cout << "deal F  var_names.size:" << dag_queue[i]->var_names.size() << std::endl;
							iter = dag_queue[i]->var_names.erase(iter);
							fcount++;

							//std::cout << "deal F  var_names.size:" << dag_queue[i]->var_names.size() << std::endl;
							//std::cout <<(iter == dag_queue[i]->var_names.end()) << std::endl;
						}
					}
					else {
                       //     std::cout << "otheriter:"<<iter[0] << std::endl;
						have_others = true;
						iter++;
					}
				}
			//	std::cout << "在这里F_S_MAP[@0]"<<F_S_Map["@0"] << std::endl;
				if (only_F) {
					if (dag_queue[i]->var_names.size() != 1) {
						//std::cout << "There is a bug in optimizedeal--dealNodeVarnames:only_F == true " << std::endl;
						return false;
					}
					else {
						continue;
					}
				}

				//再处理T, 只做计数不进行删除
				for (iter = dag_queue[i]->var_names.begin();iter != dag_queue[i]->var_names.end() ; ) {
					if (iter[0][0] == '#') {
                           // std::cout << "开始对T计数 "<<tcount << std::endl;
						if ((iter + 1 == dag_queue[i]->var_names.end()) && (have_svar == false)) {
							dag_queue[i]->name = *iter;		//最后一个临时变量的名字
							iter++;
						}
						else {

							tcount++;

							iter++;
						}
					}

					else {
						have_svar = true;
						dag_queue[i]->name = *iter;		//变量的名字
						iter++;
					}

				}

			}
			else {
				//std::cout << "There is a bug in optimizedeal--dealNodeVarnames: dag_queue[i]->var_names.size() < 1" << std::endl;
				return false;
			}
		}

		//遍历叶子结点对count_for_f计数
		for (auto iter = Node_Map.begin(); iter != Node_Map.end(); iter++) {
               // std::cout << "叶子节点iter->second->name: "<<iter->second->name << std::endl;
			if (!iter->second->is_mid_node) {
				if (iter->second->name[0] == '@') {
					fcount++;
				}
			}
		}
//std::cout << "tcount_0: "<<tcount << std::endl;
        //std::cout << "fcount_0: "<<fcount << std::endl;
		if ((fcount > tcount )) {
                //std::cout << "tcount "<<tcount << std::endl;
			count_for_f = count_for_f - fcount;
			return false;
		}
		else return true;
	}

	/*
		参数: 无
		返回值类型: void
		函数作用: 利用启发式算法依据Node_Map 和 SVar_map导出优化后的中间代码, 首先从last_Z开始
	*/
	bool get_Opt_Code() {
		//顺序插入, 倒序导出

		tree_node* t = get_correct_node();

		while (t != NULL) {
			//std::cout << "======" << std::endl;
			getdag_queue(t);
			t = get_correct_node();

		}
		//std::cout << "get_Opt_Code--after getdag_queue" << std::endl;
		//此处t == NULL
		if (dealNodeVarnames()) {
			//std::cout << "get_Opt_Code--after dealNodeVarnames" << std::endl;
			for (auto iter = F_S_Map.begin(); iter != F_S_Map.end(); ++iter) {
          //  std::cout << "iter->first："<<iter->first << std::endl;
				Enter_Mid_Opt_List(4, iter->first, " ", iter->second, 0);
			}

			while (dag_queue.size() != 0) {
                  //  std::cout << "dag_queue.size():" <<dag_queue.size()<< std::endl;
				t = dag_queue.back();	//倒序导出
//std::cout << "t->name：" <<t->name<< std::endl;
				dag_queue.erase(dag_queue.end() - 1);
				if (t == NULL) {
					//std::cout << "There is a bug in optimizedeal--get_Opt_Code: t == NULL" << std::endl;
					return false;
				}
				if (!t->is_mid_node) {
					//std::cout << "There is a bug in optimizedeal--get_Opt_Code: t is not mid node" << std::endl;
					return false;
				}
				if (t->name == " ") {
					//std::cout << "There is a bug in optimizedeal--get_Opt_Code: t->name = " "" << std::endl;
					return false;
				}
				if (t->var_names.size() < 1) {
					//std::cout << "There is a bug in optimizedeal--get_Opt_Code: t->var_names.size() < 1 " << std::endl;
					return false;
				}

				if (t->rchild != NULL) {
                       // std::cout << "zwbt->name："<<t->name << std::endl;
					Enter_Mid_Opt_List(t->op_id, t->name, t->lchild->name, t->rchild->name, 0);
				}
				else {	//简单赋值语句
                 //      std::cout << "233333t->name："<<t->name << std::endl;
					Enter_Mid_Opt_List(t->op_id, t->name, " ", t->lchild->name, 0);
				}
				//std::cout << "get_Opt_Code--after Enter_Mid_Opt_List" << std::endl;
				if (t->var_names.size() > 1) {
					auto iter = t->var_names.begin();
					for (; iter != t->var_names.end();) {
						if (*iter == t->name) {
							if(iter[0][0] != '@')Replace_Map[*iter] = t->name;
							iter++;
						}
						else {
							if (iter[0][0] == '#') {
                                 //   std::cout << "有temp_var_avilable：" << std::endl;
								temp_var_avilable.push_back(*iter);
								Replace_Map[*iter] = t->name;
							}
							else if (iter[0][0] == '@') {
								//std::cout << "There is a bug in optimizedeal--get_Opt_Code: iter[0][0] can't be F";
								return false;
							}
							else {
							   // std::cout << "233333*iter："<<*iter << std::endl;
								Enter_Mid_Opt_List(4, *iter, " ", t->name, 0);
								Replace_Map[*iter] = *iter;
							}
							iter++;
						}
					}
				}

			}

			return true;
		}
		else {
			return false;
		}
		/*
			导出时首先计算count_for_f与被替换掉的临时变量个数大小, 若被替换掉的 T个数 < count_for_f, 说明优化没有意义, 则返回false，
			依旧用源程序代码,  Replace_Map中不插入新的东西, 否则正常优化处理
			这一点PPT上可以用来展示
		*/


		/*
			处理赋值语句导出的时候需要注意一下数组赋值和普通赋值
		*/
	}

	/*
		参数:	mid_exp_start: 局部表达式在MidCodeT表中的开始位置
				mid_exp_end: 局部表达式在MidCodeT表中的结束位置 (指向的指令op不是 + - * / =)
		返回值: void
		函数作用: 对中间代码的一个局部表达式进行优化, 优化范围[mid_exp_start, mid_exp_end)
	*/
	void Deal_Basicblock(int mid_exp_start, int mid_exp_end) {
		//std::cout << mid_exp_start << " " << mid_exp_end << std::endl;
		int i;
		tree_node *lchild = NULL, *rchild = NULL;
		midcode cur_mid;

		//生成树的过程
		for (i = mid_exp_start; i < mid_exp_end; i++) {
			cur_mid = MidCodeT.midcodeArray[i];
			if (cur_mid.op == 4) {
				if (cur_mid.x == " ") {		// z = y 设置一个中间节点
					//对于 a = 5, b = a, c = b这种只设置 一个叶子节点5和一个中间节点a, b, c都在a的中间节点表中
					lchild = dealChild(cur_mid.y);
					//std::cout << "Deal_Basicblock--after cur_mid.op == 4--dealChild" << std::endl;
					rchild = NULL;
					dealFather(cur_mid.z, cur_mid.op, lchild, rchild, true);
					//std::cout << "Deal_Basicblock--after cur_mid.op == 4--dealFather" << std::endl;
				}
				else {			//z = y[x] 对于数组赋值, 左孩子为数组偏移, 右孩子为数组名

					lchild = dealChild(cur_mid.x);
					rchild = dealChild(cur_mid.y);
					dealFather(cur_mid.z, cur_mid.op, lchild, rchild, false);
				}
			}
			else {
				lchild = dealChild(cur_mid.x);	//处理操作数x
				rchild = dealChild(cur_mid.y);	//处理操作数y
				dealFather(cur_mid.z, cur_mid.op, lchild, rchild, false);
			}
		}

		//std::cout << "Deal_Basicblock--after creat tree" << std::endl;

		//从DAG图中导出的过程  主要依据Node_Map 和 SVar_Map
		if (get_Opt_Code()) {
			F_S_Map.clear();
			Node_Map.clear();
			return;
		}
		else {		//不进行优化, 将原中间代码导出
			for (i = mid_exp_start; i < mid_exp_end; i++) {
				Enter_Mid_Opt_List(MidCodeT.midcodeArray[i].op, MidCodeT.midcodeArray[i].z, MidCodeT.midcodeArray[i].x, MidCodeT.midcodeArray[i].y, MidCodeT.midcodeArray[i].isstart);
			}
			F_S_Map.clear();
			Node_Map.clear();
		}
	}


	/*
		参数:	mid_func_start: 函数在MidCodeT表中的开始标签位置
				mid_func_end: 下一个函数在MidCodeT表中的开始标签位置
		返回值: void
		函数作用: 对中间代码的一个函数体进行表达式优化, 优化范围[mid_func_start, mid_func_end) 只处理 + - * / =
	*/
	void Deal_Func(int mid_func_start, int mid_func_end) {
	    //std::cout << "开始" << std::endl;
		//std::cout << "mid_func_start:" << mid_func_start << " mid_func_end:" << mid_func_end << std::endl;
		int mid_exp_start, mid_exp_end;
		int midopt_func_start = MidCodeOptT.mdc;
		int i;
	//	int flag=0;
		for (i = mid_func_start; i < mid_func_end; i++) {
              //  std::cout << "这是第几次进这个循环？" << flag << std::endl;
              //  flag+=1;
			if ((MidCodeT.midcodeArray[i].op >= 0) && (MidCodeT.midcodeArray[i].op <= 4)) {
				mid_exp_start = i;
				//处理连续的表达式
				for (; (MidCodeT.midcodeArray[i].op >= 0) && (MidCodeT.midcodeArray[i].op <= 4) && i < mid_func_end; i++);
				mid_exp_end = i;	//跳出来时i不满足条件
				if (mid_exp_end - mid_exp_start > 1) {
					Deal_Basicblock(mid_exp_start, mid_exp_end);
					Enter_Mid_Opt_List(MidCodeT.midcodeArray[i].op, MidCodeT.midcodeArray[i].z, MidCodeT.midcodeArray[i].x, MidCodeT.midcodeArray[i].y, MidCodeT.midcodeArray[i].isstart);
		//		std::cout << "1中间代码：" <<MidCodeT.midcodeArray[i].op<<MidCodeT.midcodeArray[i].z<<MidCodeT.midcodeArray[i].x<<MidCodeT.midcodeArray[i].y<< MidCodeT.midcodeArray[i].isstart<< std::endl;
				}
				else {
					Enter_Mid_Opt_List(MidCodeT.midcodeArray[i-1].op, MidCodeT.midcodeArray[i-1].z, MidCodeT.midcodeArray[i-1].x, MidCodeT.midcodeArray[i-1].y, MidCodeT.midcodeArray[i-1].isstart);
					Enter_Mid_Opt_List(MidCodeT.midcodeArray[i].op, MidCodeT.midcodeArray[i].z, MidCodeT.midcodeArray[i].x, MidCodeT.midcodeArray[i].y, MidCodeT.midcodeArray[i].isstart);
		//			std::cout << "2中间代码：" <<MidCodeT.midcodeArray[i].op<<MidCodeT.midcodeArray[i].z<<MidCodeT.midcodeArray[i].x<<MidCodeT.midcodeArray[i].y<< MidCodeT.midcodeArray[i].isstart<< std::endl;
				}
			}
			else {

          //      std::cout << "3中间代码：" <<MidCodeT.midcodeArray[i].op<<MidCodeT.midcodeArray[i].z<<MidCodeT.midcodeArray[i].x<<MidCodeT.midcodeArray[i].y<< MidCodeT.midcodeArray[i].isstart<< std::endl;
				Enter_Mid_Opt_List(MidCodeT.midcodeArray[i].op, MidCodeT.midcodeArray[i].z, MidCodeT.midcodeArray[i].x, MidCodeT.midcodeArray[i].y, MidCodeT.midcodeArray[i].isstart);
			}
		}
		//if (count_for_f > temp_var_avilable.size()) std::cout << "count_for_f:"<<count_for_f<<" temp_var_avilable.size():"<<temp_var_avilable.size()<< std::endl;
		for (i = 0; i < count_for_f; i++) {
			std::string Name_of_f = "@" + int_to_string(i);
			if (temp_var_avilable.size() == 0) {
			//	std::cout << "There is a bug in optimizedeal--Deal_Func: temp_var_avilable.size() == 0" << std::endl;
				return;
			}
			std::string Name_of_t = temp_var_avilable.back();
			temp_var_avilable.erase(temp_var_avilable.end() - 1);
			Replace_Map[Name_of_f] = Name_of_t;


		}


		//根据Replace_Map更新变量名T
		for (i = midopt_func_start; i < MidCodeOptT.mdc; i++) {
			midcode cur_midopt = MidCodeOptT.midcodeArray[i];
			if (cur_midopt.op > 5 && cur_midopt.op < 12) {
				if (In_Replace_Map_ornot(cur_midopt.x) && cur_midopt.x[0] == '#') {
					MidCodeOptT.midcodeArray[i].x = Replace_Map[cur_midopt.x];
				}
				if (In_Replace_Map_ornot(cur_midopt.y) && cur_midopt.y[0] == '#') {
					MidCodeOptT.midcodeArray[i].y = Replace_Map[cur_midopt.y];
				}
			}
			else if ((cur_midopt.op == 12) ||
				(cur_midopt.op == 17) ||
				(cur_midopt.op == 18) ||
				(cur_midopt.op == 20)) {
				if (In_Replace_Map_ornot(cur_midopt.y) && cur_midopt.y[0] == '#') {
					MidCodeOptT.midcodeArray[i].y = Replace_Map[cur_midopt.y];
				}
			}
		}
//std::cout << "执行到这里了" << std::endl;
		//根据Replace_Map更新变量名F
		for (i = midopt_func_start; i < MidCodeOptT.mdc; i++) {
			midcode cur_midopt = MidCodeOptT.midcodeArray[i];
        //    std::cout << "cur_midopt：" <<MidCodeOptT.midcodeArray[i].z<< std::endl;
       //     std::cout << "Replace_Map[@0]：" <<Replace_Map["@0"]<< std::endl;
			if (cur_midopt.op < 12) {
				if (cur_midopt.z[0] == '@') {
					MidCodeOptT.midcodeArray[i].z = Replace_Map[cur_midopt.z];
				}
				if (cur_midopt.x[0] == '@') {
					MidCodeOptT.midcodeArray[i].x = Replace_Map[cur_midopt.x];
				}
				if (cur_midopt.y[0] == '@') {
					MidCodeOptT.midcodeArray[i].y = Replace_Map[cur_midopt.y];
				}
			}
			else if ((cur_midopt.op == 12) ||
				(cur_midopt.op == 17) ||
				(cur_midopt.op == 18) ||
				(cur_midopt.op == 20)) {
				if (cur_midopt.y[0] == '@') {
					MidCodeOptT.midcodeArray[i].y = Replace_Map[cur_midopt.y];
				}
			}
		}
		//清空vector和Map
		count_for_f = 0;
		Replace_Map.clear();
		dag_queue.clear();
		temp_var_avilable.clear();
	}

	//对外接口函数
	void Make_Opt() {
		int mid_func_start, mid_func_end;
		int i;
		for (i = 0; i < MidCodeT.mdc; i++) {
			if (MidCodeT.midcodeArray[i].op == 14) {	//函数开始位置标签
				mid_func_start = i;
				i++;
				for (; i < MidCodeT.mdc; i++) {
					if (MidCodeT.midcodeArray[i].op == 14) break;
				}
				//出循环的条件: MidCodeT.midcodeArray[i].isstart == 4 或者 i = MidCodeT.mdc
				// 此时为下一个函数的开始位置
				mid_func_end = i;
				//调用函数块优化
				Deal_Func(mid_func_start, mid_func_end);
				//如果没有if语句那么在main的结束之后会陷入一个 i-- i++ 的死循环
				if(i != MidCodeT.mdc) i--;	//保证下一个开始位置标签不会被跳过, 上一个的end = 下一个的start
			}
		}
	}

}
