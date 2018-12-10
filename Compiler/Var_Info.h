#ifndef VAR_INFO_H_INCLUDED
#define VAR_INFO_H_INCLUDED

#include <map>
#include <string>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

//scanf �� printf��Ϊ���ú�������
namespace compiler{
#define NAME_SIZE 200	// ��ʶ����󳤶�Ϊ200
#define LLNG 350   //input line length
#define TMAX 1000 //tab ���ű���������Ϊ1000
#define BMAX 100 //btab �ֳ������������Ϊ100
#define AMAX 100 //atab �������������
#define CSMAX 30  // case���������
#define SMAX 6000	// size of string-table

//#define COMAX 800	//size of code
#define MID_KIND 30  //�м������Ԫʽop������
#define MIDCODEMAX 2000 	//�м�ָ���������Ϊ2000

#define MIPS_KIND 40 //mips��ָ������
#define MIPSCODEMAX 10000	//mipsָ����������Ϊ2000

//��ҵ������Խ�����,��������ͳһ��NMAX
//#define XMAX 0xFFFFFFFF //	�����±�����ֵ(�޷���������������)
#define NMAX 0x7FFFFFFF // ���ֵ����ֵ 2**31 - 1, ����Ǹ�����߽��� -(NMAX + 1) �� -2147483648

//#define LINELENG 132 //output line length
//#define LINELIMIT 200  //��������
#define STACKSIZE 2000 //����ջ�Ĵ�С

//�Զ������ö������
enum symbol{
	//��ʶ��
	ident,

	//��������
	intcon, //int
	charcon, //char
	stringcon, //char[] ����

	//������	KeyMap��
	charsy,
	intsy,
	voidsy,  //void
	constsy, //const
	mainsy, //main
	ifsy, //if
	defaultsy, //defaut
	whilesy, //while
	switchsy, //switch
	casesy, //case
	returnsy, //return
	printfsy, //printf
	scanfsy, //scanf


	//��ϵ�����
	lss, // '<'
	leq, // '<='
	gtr, // '>'
	geq, // '>='
	neq, // '!='
	eql, // '=='

	//����� SpsMap��
	plus, // '+'
	minus, // '-'
	mult,  // '*'
	rdiv, // '/'
	assign, // '=',��ֵ������е� =

	//������	SpsMap��
	lparent, // '('
	rparent, // ')'
	lsbrack, // '['
	rsbrack, // ']'
	lcbrack, // '{'
	rcbrack, // '}'
	comma, // ','
	//apos, // ������
	//douq, // ˫����
	semicolon, // ';'
	colon //':'
};
//���ڴʷ�������ӡ����
extern std::string symbol_info[37];

//��ʶ������(����pascal_S)
enum objecttyp {constant, variable, function, array};
extern std::string objecttyp_info[4];

//��������
enum types {ints, chars, notyp};
extern std::string types_info[3];



//indentifier table
struct tabe{
			std::string name;//��ʶ������,ȡǰʮ���ַ�
			objecttyp obj; //��ʶ������
			types type; //��ʶ������������
			int adr;
			uint32_t ref; //ָ��������atab�е�λ��(��¼λ��ָ��ֵ)���ߺ�������btab�е�λ��
			uint32_t link;//ָ��ͬһ���ֳ�������һ����ʶ���ڷ��ű��е�λ��
			int lev;	//�÷������ڵķֳ����,�ڸ��ݷ��ű������������ջ��ַʱ�õ�, �ֲ���ȫ�ֵĻ�ַ��ͬ
};

struct tabt{
			struct tabe tabArray[TMAX];
			uint32_t t;	//��������t
};

//array-table
//ȫ����int��������Ԫ��
struct atabe{
			types eltyp; //����Ԫ������,ֻ��Ϊint����char
			uint32_t high; //����Ԫ�ص��Ͻ�,����Ԫ�ش�0��ʼ(int a[10],�Ͻ�Ϊ10, �����±�С��10)
};

struct atabt{
			struct atabe atabArray[AMAX];
			uint32_t a;	//��������a
};

//block-table
struct btabe{
			uint32_t last; //ָ��÷ֳ�����˵���ĵ�ǰ(���)һ����ʶ����tab���е�λ��
			uint32_t lastpar; //���������һ��������tab�е�λ��
			types kind; // notyp�����޷���ֵ, ints����ֵΪint, chars������ֵΪchar
			uint32_t paranum; //��������
			uint32_t varsize; //�ֲ�������С(btab.btabArray[0].varnum �洢ȫ�ֱ�����С),�ڶ�����ջ��������ռ�ʱ��
			int t_varnum;	//��ʱ�����ĸ���

};

struct btabt{
			struct btabe btabArray[BMAX];
			uint32_t b; //��������b
};

//middle-code table

struct midcode{
			//���� z = x op y  ���� op, z, x, y,
		   int op; //��Ԫʽ������,���MidOpKind����ʹ��
		   std::string z;
		   std::string x;
		   std::string y;
		   int isstart; //��־����Ԫʽ�м�����Ƿ�Ϊ������, >0 ��Ϊ������
						//����1��־Ϊ��ͨ����ת��ǩ, 2��־Ϊmain���������λ��, 3��ʾ�������÷��ص�λ��, 4��ʾ������ʼ��ǩ
};

struct midcodetable{
			struct midcode midcodeArray[MIDCODEMAX];
			uint32_t mdc;	//��������
};

//mips-code table

struct mipscode {
	int op; //������
	std::string z;			//label�����߼Ĵ���
	std::string x;
	std::string y;
	int offset;	//����Ҫ��ַƫ�ƻ���ʹ��������ʱ����
};

struct mipscodetable {
	struct mipscode mipscodeArray[MIPSCODEMAX];
	int mpc;	//��������
};


//string-table
struct stabt{
			char stringArray[SMAX];
			uint32_t sx;
};

//stack
struct stacke{
			int space[STACKSIZE];	//����ջ
			uint32_t topaddr;		//��ǰջ��ָ��
			uint32_t cbaseaddr;		//��ǰ����������ַ
			uint32_t globalvaraddr; //ȫ�ֱ��� ��ʼλ��
};

//��midtomipsdeal���õ� T_Map, S_Map ʹ��
struct var_info {
	bool inReg;		//�Ƿ��ڼĴ�����, 1��ʾ��, 0��ʾ����
	int r_addr;		//�Ĵ����������±� 0 - 17
	int stack_addr;	//��Ի���ַƫ��
	bool isTFirst;		//��ʱ�����Ż��õ�, �Ƿ�Ϊ��һ��ʹ��(��һ�β���lw), ��blockdeal������ʱ����ʱ��ʼ��, һ����Ϊfalse��䲻��ȥ��
};



//ȫ���ļ�
extern std::fstream inFile;	//source-code file
extern std::fstream midoutFile; //mid-code file
extern std::fstream mipsoutFile; //mips-code file
extern std::fstream erroroutFile;	//error-info file
extern std::fstream word_out_file; //���ڴʷ�������һ����Ҫ

//ȫ�ֱ�ջ
extern std::map<std::string, symbol> KeyMap;  // ��������symbol֮��Ķ�Ӧ��ϵ
extern std::map<char, symbol> SpsMap; // '+', '-' �ȷ�����symbol�Ķ�Ӧ��ϵ
extern tabt tab;	//���ű�tab
extern atabt atab; //������Ϣ������
extern btabt btab; //�ֳ����
//extern ctabt ctab; //������
//extern displayt display; //�ֳ���������
extern stabt stab; //�ַ���������

//�м�����Mips�����
extern midcodetable MidCodeT;	//�м�����
extern midcodetable MidCodeOptT;	//�Ż�����м�����
extern stacke Stack;	//ջ�ռ�stack
extern mipscodetable MipsTable;		//mips�����
extern mipscodetable MipsTableOpt;		//�Ż����mips�����

extern uint32_t display[2]; //��Ϊ�����������,display[0]ֵΪ0,btab.btabArray[display[0]].lastΪȫ�ֱ������һ����ʶ��λ��
							//display[1]�洢��ǰ�ֲ�����������btab��λ��, ��btab.btabArray[display[1]]�ҵ���ǰ����������

//ȫ�ֱ���
extern char ch; //last character read from source program
extern int inum; //intvalue from insymbol
extern int sleng; //string length
extern int cc; //character counter
extern int lc; //program line location counter
extern int ll; //length of current line
extern char line[LLNG + 1];//input line, last char is ' '
extern int errpos; //error positon
extern symbol sy; //last symbol read by insymbol
extern char id[NAME_SIZE]; //indentifier from insymbol
extern int level;	//��ǰ�ֳ����,ֻ����0��1����
extern bool IsError;

//�м������Ԫʽָ����ձ�
extern std::string MidOpKind[MID_KIND];	//�м����op����ָ����ձ�

//mipsָ����ձ�
extern std::string MipsOpKind[MIPS_KIND];	//mips����op����ָ����ձ�

extern std::map<std::string, var_info> T_Map;	//��ʱ��������Ϣ, ��������ʱ����ʱ����

extern std::map<std::string, std::string> Str_Info_Map;		//Ҫ��ӡ���ַ�����Ϣ, ��midtomipsdeal�д�ţ���printresult��ʹ��
}
#endif
