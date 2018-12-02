 /*
	词法分析及处理
*/

#ifndef WORD_DEAL_H_INCLUDED
#define WORD_DEAL_H_INCLUDED
#include "Var_Info.h"
#include "Error_Deal.h"
#include "Enter_Info.h"
#include <iomanip>
namespace compiler{
    extern int word_count;   //用于词法分析第一次需要
    void insymbol(); //read next symbol
}
#endif
