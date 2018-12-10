/*
	ȫ�ֱ����Ķ���
*/

#include "Var_Info.h"
namespace compiler{
    std::string symbol_info[37] = {"ident","intcon","charcon","stringcon","charsy","intsy","voidsy","constsy","mainsy",
	"ifsy","whilesy","switchsy","casesy","defaultsy","scanfsy","printfsy","returnsy","lss","leq","gtr","geq","neq",
	"eql","plus","minus","mult","rdiv","assign","lparent","rparent","lsbrack","rsbrack","lcbrack",
	"rcbrack","comma","semicolon","colon"};
    std::string objecttyp_info[4] = {"constant", "variable", "function", "array    "};
	std::string types_info[3] = {"ints", "chars", "notyp"};

	std::fstream inFile;	//source-code file
	std::fstream midoutFile; //mid-code file
	std::fstream mipsoutFile; //mips-code file
	std::fstream erroroutFile;	//error-info file
	std::fstream word_out_file; //�ʷ���������ļ�

	std::map<std::string, symbol> KeyMap;
	std::map<char, symbol> SpsMap;
	tabt tab;	//���ű�tab
	atabt atab;	//������Ϣ������
	btabt btab; //�ֳ����
	//ctabt ctab; //������
	//displayt display; //�ֳ���������
	stabt stab; //�ַ���������

	midcodetable MidCodeT;	//�м�����
	midcodetable MidCodeOptT;	//�Ż�����м�����
	stacke Stack;	//ջ�ռ�stack
	mipscodetable MipsTable;	//mips�����
	mipscodetable MipsTableOpt;		//�Ż����mips�����

	uint32_t display[2]; //��Ϊ�����������,display[0]ֵΪ0,btab.btabArray[display[0]].lastΪȫ�ֱ������һ����ʶ��λ��
						//display[1]�洢��ǰ�ֲ�����������btab��λ��, ��btab.btabArray[display[1]]�ҵ���ǰ����������

	char ch; //last character read from source program
	int inum; //intvalue from insymbol
	int sleng;
	int cc; //character counter
	int lc; //program location counter
	int ll; //length of current line
	char line[LLNG+1]; //input line, last char is '\0'
	int errpos; //erropos
	symbol sy; //last symbol read by insymbol
	char id[NAME_SIZE]; //indentifier from insymbol
	int	level;//��ǰ�ֳ����
	bool IsError = false;

	std::string MidOpKind[MID_KIND];
	std::string MipsOpKind[MIPS_KIND];

	std::map<std::string, var_info> T_Map;	//��ʱ��������Ϣ, ��������ʱ����ʱ����
	std::map<std::string, std::string> Str_Info_Map;

}
