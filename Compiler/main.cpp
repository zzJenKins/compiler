
#include "main.h"
using namespace compiler;
int main(){
	init();
	block();
	Print_Mid_Code();
	midtomips(0);
	Print_Mips_Code();
//if(!IsError){
	//Dagͼ�Ż�
	Make_Opt();
	Print_Mid_Opt_Code();
	//�Ĵ����Ż�
	midtomips(1);
	Print_Mips_Opt_Code();
	//}
	return 0;
}
