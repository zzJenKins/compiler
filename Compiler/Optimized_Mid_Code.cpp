/*
	dagͼ�����Ż�
*/
#include "Optimized_Mid_Code.h"
namespace compiler{
	struct tree_node {
		//Ҷ�ӽڵ��Ӧ������
		std::string name;	//�ڵ�����Ӧ�ı�����, Ҷ�ӽ��Ż���

		//�м�ڵ��Ӧ����
		int op_id;	// +,-,*,/,= �Ĳ�������Ӧ��midop���, -1 ������ΪҶ�ӽڵ�
		struct tree_node* lchild;	//����node���
		struct tree_node* rchild; //�Һ���node���
		std::vector<std::string> var_names;	//�ýڵ�����Ӧ�ı�����������

		//����
		bool is_mid_node;	//�Ƿ�Ϊ�м�ڵ�
		std::vector<struct tree_node*> parents; //���ڵ�, ��dealFather�ж��������ӽڵ�ĸ��ڵ���и��²���
	};

	std::vector<tree_node*> dag_queue;	//��DAGͼ�����м����ʱʹ��
	std::map<std::string, tree_node*> Node_Map;	//Dagͼ�ı�����-�ڵ�ͼ, ��ÿ��ȫ������֮�����
	std::map<std::string, std::string> F_S_Map;		//F��������ԭ���ƵĶ�Ӧ��ϵ
	std::vector<std::string> temp_var_avilable;	//��Ϊ�����ӱ��ʽɾ��������������ʱ������, ��get_Opt_Codeһ��ʼ��������
	std::map<std::string, std::string> Replace_Map;	//��ʱ�������¾ɱ������ƶ��ձ�,�Ժ�����Ϊ��λ�Ե�������м����ν��б������Ƹ���, ��get_Opt_Code������

	int count_for_f = 0;	//ÿ���ֲ��鴦����֮������

	//���ڴ���a0 = a�������, ͳһ��F0��F1��������
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

	//��Node_Map�еõ�һ��û�и��ڵ���м�ڵ㣬 �õ���ɾ��
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


	//��node_map����Ѱ�ҵ�ǰ�������Ƿ��Ѿ�����, �����ڷ�����ڵ�ָ��
	tree_node* findInNodeMap(std::string name) {
		auto iter = Node_Map.find(name);
		if (iter == Node_Map.end()) return NULL;	//û�ҵ�
		else {
			return iter->second;
		}

	}

	bool In_Replace_Map_ornot(std::string name) {
		auto iter = Replace_Map.find(name);
		if (iter == Replace_Map.end()) return false;	//û�ҵ�
		else {
			return true;
		}
	}

	//�жϵ�ǰ����ı��ʽ�Ƿ��Ѿ��еȼ۵Ľڵ�, �����򷵻���ڵ�ָ��
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
		//û���ҵ��ȼ۵�
		return NULL;
	}

	/*
		����:	name ������(�п���Ϊ����, ���ֿ϶�ΪҶ�ӽڵ�)
		����ֵ����: tree_node*
		��������: ����������ı�����, ���Ѿ������򷵻���ڵ�ָ��, ��������������һ��Ҷ�ӽ��
	*/
	tree_node* dealChild(std::string name) {
		tree_node * nodep = findInNodeMap(name);
		if (nodep != NULL) return nodep;
		//��Ϊ������δ��֮ǰ���ֹ�, ����ΪҶ�ӽڵ�
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
		����:	name ������(�ֲ�����������ʱ����, ���ھֲ�����Ҫͬʱ��������map��)
				is_assign�ڼ򵥵ĸ�ֵ������Ϊ1, ���鸳ֵ����Ϊ0
		����ֵ����: void
		��������: ������ʽ����ı�����, ���ʽ�ȼ۵�����: (op == op_id) && (((lparent == lparent) && (rparent == rparent)) || ((lparent == rparent) && (rparent == lparent)))
		ע��: ���������Ѿ����ڶ����ʽ���ȼ�, �൱�ڶ�ԭ�����������¶���, ����Ҫ��������a0 = a�Ĵ���
	*/

	void dealFather(std::string name, int op, tree_node* lchild, tree_node* rchild, bool is_assign) {
		tree_node *z_node = expEqual(op, lchild, rchild);
		if (z_node == NULL) {
			tree_node *temp = findInNodeMap(name);	//���ҵ�ǰ�������Ƿ��Ѿ���Ϊ�ڵ�ʹ��, ��Ӧa0 = a���ִ������
			if (temp != NULL) {	//���Ѿ����ڸñ������Ľڵ�
				if (temp->is_mid_node) {	//�м�ڵ�
					std::vector<std::string>::iterator iter = std::find(temp->var_names.begin(), temp->var_names.end(), name);	//���ҵ����м�ڵ��λ��
					if (iter == temp->var_names.end()) {
						//std::cout << "There is a bug in optimizedeal--dealFather: iter can't be temp->var_names.end()" << std::endl;
						return;
					}
					std::string new_name = Create_f_Name();
					F_S_Map[new_name] = *iter;
					*iter = new_name;
					Node_Map[*iter] = temp;
				}
				else {		//Ҷ�ӽڵ�
					std::string new_name = Create_f_Name();
					F_S_Map[new_name] = temp->name;
					temp->name = new_name;	//���ı�����
					Node_Map[temp->name] = temp;
				}
			}

			//�򵥸�ֵ��Ϊ ĳ��ֵ�����м�ڵ�����, ��Ѹ�ֵ���뵽�ñ����ı���������
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
				����name�Ѿ�������Node_Map�е����
				���� a0 = a�����, ����Ϊ�ֲ����ʽ�����õ�����ʱ�������滻(��û����Ըô���鲻�����Ż�����)
			*/
		}
		else {		//�Ѿ��еȼ۵ı��ʽ
			z_node->var_names.push_back(name);//������������ýڵ�
		}
		Node_Map[name] = z_node;
	}

	//�ݹ���õõ���rootΪ���ڵ������
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

		//���ڵ��õ�������
		tree_node* lt = NULL;
		tree_node* rt = NULL;
		dag_queue.push_back(root);

		Delete_From_NodeMap(root);

		//��root���������������ĸ��ڵ���и���
		lt = root->lchild;
		rt = root->rchild;

		if (lt->is_mid_node && In_Node_Map_Or_Not(lt)) {

			std::vector<tree_node*>::iterator iter = std::find(lt->parents.begin(), lt->parents.end(), root);	//���ҵ����м�ڵ��λ��
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

		//ѡ�������������������������ֱ��return
		if (lt->is_mid_node && lt->parents.size() == 0 && In_Node_Map_Or_Not(lt)) {
			getdag_queue(lt);
		}
		if (rt != NULL && rt->is_mid_node && rt->parents.size() == 0 && In_Node_Map_Or_Not(lt)) {
			getdag_queue(rt);
		}
		return;

	}


	//����Ҫ���滻��F�������ڶ������ʱ����, ����false�� ���򷵻�true
	bool dealNodeVarnames() {
		int i;
		int fcount = 0, tcount = 0;
		//�����м�ڵ��count_for_f����
		for (i = 0; i < dag_queue.size(); i++) {

			if (dag_queue[i]->var_names.size() == 1) {
			  //  std::cout << "dag_queue[i]->var_names:"<<dag_queue[i]->var_names[0] << std::endl;
				if (dag_queue[i]->var_names[0][0] == '@') {	//ֻ��һ��������ΪF��ͷ
					fcount++;
				}
				dag_queue[i]->name = dag_queue[i]->var_names[0];
			}
			else if (dag_queue[i]->var_names.size() > 1) {
				auto iter = dag_queue[i]->var_names.begin();
				bool only_F = false;
				bool have_others = false;
				bool have_svar = false;	//��־�ýڵ��Ƿ��оֲ�����

				//�ȴ���F
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
			//	std::cout << "������F_S_MAP[@0]"<<F_S_Map["@0"] << std::endl;
				if (only_F) {
					if (dag_queue[i]->var_names.size() != 1) {
						//std::cout << "There is a bug in optimizedeal--dealNodeVarnames:only_F == true " << std::endl;
						return false;
					}
					else {
						continue;
					}
				}

				//�ٴ���T, ֻ������������ɾ��
				for (iter = dag_queue[i]->var_names.begin();iter != dag_queue[i]->var_names.end() ; ) {
					if (iter[0][0] == '#') {
                           // std::cout << "��ʼ��T���� "<<tcount << std::endl;
						if ((iter + 1 == dag_queue[i]->var_names.end()) && (have_svar == false)) {
							dag_queue[i]->name = *iter;		//���һ����ʱ����������
							iter++;
						}
						else {

							tcount++;

							iter++;
						}
					}

					else {
						have_svar = true;
						dag_queue[i]->name = *iter;		//����������
						iter++;
					}

				}

			}
			else {
				//std::cout << "There is a bug in optimizedeal--dealNodeVarnames: dag_queue[i]->var_names.size() < 1" << std::endl;
				return false;
			}
		}

		//����Ҷ�ӽ���count_for_f����
		for (auto iter = Node_Map.begin(); iter != Node_Map.end(); iter++) {
               // std::cout << "Ҷ�ӽڵ�iter->second->name: "<<iter->second->name << std::endl;
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
		����: ��
		����ֵ����: void
		��������: ��������ʽ�㷨����Node_Map �� SVar_map�����Ż�����м����, ���ȴ�last_Z��ʼ
	*/
	bool get_Opt_Code() {
		//˳�����, ���򵼳�

		tree_node* t = get_correct_node();

		while (t != NULL) {
			//std::cout << "======" << std::endl;
			getdag_queue(t);
			t = get_correct_node();

		}
		//std::cout << "get_Opt_Code--after getdag_queue" << std::endl;
		//�˴�t == NULL
		if (dealNodeVarnames()) {
			//std::cout << "get_Opt_Code--after dealNodeVarnames" << std::endl;
			for (auto iter = F_S_Map.begin(); iter != F_S_Map.end(); ++iter) {
          //  std::cout << "iter->first��"<<iter->first << std::endl;
				Enter_Mid_Opt_List(4, iter->first, " ", iter->second, 0);
			}

			while (dag_queue.size() != 0) {
                  //  std::cout << "dag_queue.size():" <<dag_queue.size()<< std::endl;
				t = dag_queue.back();	//���򵼳�
//std::cout << "t->name��" <<t->name<< std::endl;
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
                       // std::cout << "zwbt->name��"<<t->name << std::endl;
					Enter_Mid_Opt_List(t->op_id, t->name, t->lchild->name, t->rchild->name, 0);
				}
				else {	//�򵥸�ֵ���
                 //      std::cout << "233333t->name��"<<t->name << std::endl;
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
                                 //   std::cout << "��temp_var_avilable��" << std::endl;
								temp_var_avilable.push_back(*iter);
								Replace_Map[*iter] = t->name;
							}
							else if (iter[0][0] == '@') {
								//std::cout << "There is a bug in optimizedeal--get_Opt_Code: iter[0][0] can't be F";
								return false;
							}
							else {
							   // std::cout << "233333*iter��"<<*iter << std::endl;
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
			����ʱ���ȼ���count_for_f�뱻�滻������ʱ����������С, �����滻���� T���� < count_for_f, ˵���Ż�û������, �򷵻�false��
			������Դ�������,  Replace_Map�в������µĶ���, ���������Ż�����
			��һ��PPT�Ͽ�������չʾ
		*/


		/*
			����ֵ��䵼����ʱ����Ҫע��һ�����鸳ֵ����ͨ��ֵ
		*/
	}

	/*
		����:	mid_exp_start: �ֲ����ʽ��MidCodeT���еĿ�ʼλ��
				mid_exp_end: �ֲ����ʽ��MidCodeT���еĽ���λ�� (ָ���ָ��op���� + - * / =)
		����ֵ: void
		��������: ���м�����һ���ֲ����ʽ�����Ż�, �Ż���Χ[mid_exp_start, mid_exp_end)
	*/
	void Deal_Basicblock(int mid_exp_start, int mid_exp_end) {
		//std::cout << mid_exp_start << " " << mid_exp_end << std::endl;
		int i;
		tree_node *lchild = NULL, *rchild = NULL;
		midcode cur_mid;

		//�������Ĺ���
		for (i = mid_exp_start; i < mid_exp_end; i++) {
			cur_mid = MidCodeT.midcodeArray[i];
			if (cur_mid.op == 4) {
				if (cur_mid.x == " ") {		// z = y ����һ���м�ڵ�
					//���� a = 5, b = a, c = b����ֻ���� һ��Ҷ�ӽڵ�5��һ���м�ڵ�a, b, c����a���м�ڵ����
					lchild = dealChild(cur_mid.y);
					//std::cout << "Deal_Basicblock--after cur_mid.op == 4--dealChild" << std::endl;
					rchild = NULL;
					dealFather(cur_mid.z, cur_mid.op, lchild, rchild, true);
					//std::cout << "Deal_Basicblock--after cur_mid.op == 4--dealFather" << std::endl;
				}
				else {			//z = y[x] �������鸳ֵ, ����Ϊ����ƫ��, �Һ���Ϊ������

					lchild = dealChild(cur_mid.x);
					rchild = dealChild(cur_mid.y);
					dealFather(cur_mid.z, cur_mid.op, lchild, rchild, false);
				}
			}
			else {
				lchild = dealChild(cur_mid.x);	//���������x
				rchild = dealChild(cur_mid.y);	//���������y
				dealFather(cur_mid.z, cur_mid.op, lchild, rchild, false);
			}
		}

		//std::cout << "Deal_Basicblock--after creat tree" << std::endl;

		//��DAGͼ�е����Ĺ���  ��Ҫ����Node_Map �� SVar_Map
		if (get_Opt_Code()) {
			F_S_Map.clear();
			Node_Map.clear();
			return;
		}
		else {		//�������Ż�, ��ԭ�м���뵼��
			for (i = mid_exp_start; i < mid_exp_end; i++) {
				Enter_Mid_Opt_List(MidCodeT.midcodeArray[i].op, MidCodeT.midcodeArray[i].z, MidCodeT.midcodeArray[i].x, MidCodeT.midcodeArray[i].y, MidCodeT.midcodeArray[i].isstart);
			}
			F_S_Map.clear();
			Node_Map.clear();
		}
	}


	/*
		����:	mid_func_start: ������MidCodeT���еĿ�ʼ��ǩλ��
				mid_func_end: ��һ��������MidCodeT���еĿ�ʼ��ǩλ��
		����ֵ: void
		��������: ���м�����һ����������б��ʽ�Ż�, �Ż���Χ[mid_func_start, mid_func_end) ֻ���� + - * / =
	*/
	void Deal_Func(int mid_func_start, int mid_func_end) {
	    //std::cout << "��ʼ" << std::endl;
		//std::cout << "mid_func_start:" << mid_func_start << " mid_func_end:" << mid_func_end << std::endl;
		int mid_exp_start, mid_exp_end;
		int midopt_func_start = MidCodeOptT.mdc;
		int i;
	//	int flag=0;
		for (i = mid_func_start; i < mid_func_end; i++) {
              //  std::cout << "���ǵڼ��ν����ѭ����" << flag << std::endl;
              //  flag+=1;
			if ((MidCodeT.midcodeArray[i].op >= 0) && (MidCodeT.midcodeArray[i].op <= 4)) {
				mid_exp_start = i;
				//���������ı��ʽ
				for (; (MidCodeT.midcodeArray[i].op >= 0) && (MidCodeT.midcodeArray[i].op <= 4) && i < mid_func_end; i++);
				mid_exp_end = i;	//������ʱi����������
				if (mid_exp_end - mid_exp_start > 1) {
					Deal_Basicblock(mid_exp_start, mid_exp_end);
					Enter_Mid_Opt_List(MidCodeT.midcodeArray[i].op, MidCodeT.midcodeArray[i].z, MidCodeT.midcodeArray[i].x, MidCodeT.midcodeArray[i].y, MidCodeT.midcodeArray[i].isstart);
		//		std::cout << "1�м���룺" <<MidCodeT.midcodeArray[i].op<<MidCodeT.midcodeArray[i].z<<MidCodeT.midcodeArray[i].x<<MidCodeT.midcodeArray[i].y<< MidCodeT.midcodeArray[i].isstart<< std::endl;
				}
				else {
					Enter_Mid_Opt_List(MidCodeT.midcodeArray[i-1].op, MidCodeT.midcodeArray[i-1].z, MidCodeT.midcodeArray[i-1].x, MidCodeT.midcodeArray[i-1].y, MidCodeT.midcodeArray[i-1].isstart);
					Enter_Mid_Opt_List(MidCodeT.midcodeArray[i].op, MidCodeT.midcodeArray[i].z, MidCodeT.midcodeArray[i].x, MidCodeT.midcodeArray[i].y, MidCodeT.midcodeArray[i].isstart);
		//			std::cout << "2�м���룺" <<MidCodeT.midcodeArray[i].op<<MidCodeT.midcodeArray[i].z<<MidCodeT.midcodeArray[i].x<<MidCodeT.midcodeArray[i].y<< MidCodeT.midcodeArray[i].isstart<< std::endl;
				}
			}
			else {

          //      std::cout << "3�м���룺" <<MidCodeT.midcodeArray[i].op<<MidCodeT.midcodeArray[i].z<<MidCodeT.midcodeArray[i].x<<MidCodeT.midcodeArray[i].y<< MidCodeT.midcodeArray[i].isstart<< std::endl;
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


		//����Replace_Map���±�����T
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
//std::cout << "ִ�е�������" << std::endl;
		//����Replace_Map���±�����F
		for (i = midopt_func_start; i < MidCodeOptT.mdc; i++) {
			midcode cur_midopt = MidCodeOptT.midcodeArray[i];
        //    std::cout << "cur_midopt��" <<MidCodeOptT.midcodeArray[i].z<< std::endl;
       //     std::cout << "Replace_Map[@0]��" <<Replace_Map["@0"]<< std::endl;
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
		//���vector��Map
		count_for_f = 0;
		Replace_Map.clear();
		dag_queue.clear();
		temp_var_avilable.clear();
	}

	//����ӿں���
	void Make_Opt() {
		int mid_func_start, mid_func_end;
		int i;
		for (i = 0; i < MidCodeT.mdc; i++) {
			if (MidCodeT.midcodeArray[i].op == 14) {	//������ʼλ�ñ�ǩ
				mid_func_start = i;
				i++;
				for (; i < MidCodeT.mdc; i++) {
					if (MidCodeT.midcodeArray[i].op == 14) break;
				}
				//��ѭ��������: MidCodeT.midcodeArray[i].isstart == 4 ���� i = MidCodeT.mdc
				// ��ʱΪ��һ�������Ŀ�ʼλ��
				mid_func_end = i;
				//���ú������Ż�
				Deal_Func(mid_func_start, mid_func_end);
				//���û��if�����ô��main�Ľ���֮�������һ�� i-- i++ ����ѭ��
				if(i != MidCodeT.mdc) i--;	//��֤��һ����ʼλ�ñ�ǩ���ᱻ����, ��һ����end = ��һ����start
			}
		}
	}

}
