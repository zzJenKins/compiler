
#include "main.h"
using namespace compiler;
int main(){
	init();
	block();
	Print_Mid_Code();
	midtomips(0);
	Print_Mips_Code();
//if(!IsError){
	//Dag图优化
	Make_Opt();
	Print_Mid_Opt_Code();
	//寄存器优化
	midtomips(1);
	Print_Mips_Opt_Code();
	//}
	return 0;
}
