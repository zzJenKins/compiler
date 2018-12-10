
#include "main.h"
using namespace compiler;
int main(){
	init();
	block();
	printMidResult();

	midtomips(0);
	printMipsResult();

	std::cout<<"end"<<std::endl;
	return 0;
}
