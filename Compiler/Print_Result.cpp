#include "Print_Result.h"
namespace compiler {

	void printMidResult() {
		std::fstream mid_result_file;
		mid_result_file.open("16061002_张凯宁_优化前中间代码.txt", std::fstream::out);
		mid_result_file << "RET: func_return_value" << std::endl;
		mid_result_file << std::endl;
		for (int i = 0; i < MidCodeT.mdc; i++) {
			midcode cur_mid = MidCodeT.midcodeArray[i];
			if (cur_mid.op < 4) {
				mid_result_file << "    " << cur_mid.z << " = " << cur_mid.x << " " << MidOpKind[cur_mid.op] << " " << cur_mid.y << std::endl;
			}
			else if (cur_mid.op == 4) {
				if (cur_mid.x == " ") {
					mid_result_file << "    " << cur_mid.z << " = " << cur_mid.y << std::endl;
				}
				else {
					mid_result_file << "    " << cur_mid.z << " = " << cur_mid.y <<"["<<cur_mid.x<<"]"<< std::endl;
				}
			}
			else if (cur_mid.op == 5) {
				mid_result_file << "    " << MidOpKind[cur_mid.op] << "  " << cur_mid.z << std::endl;
			}
			else if (cur_mid.op < 12) {
				mid_result_file << "    " << MidOpKind[cur_mid.op]<<"  "<<cur_mid.x<<", "<<cur_mid.y << ", " << cur_mid.z << std::endl;
			}
			else if (cur_mid.op == 12) {
				mid_result_file << "    " << MidOpKind[cur_mid.op] << "  " << cur_mid.y << std::endl;
			}
			else if (cur_mid.op == 13) {
				mid_result_file << std::endl;
				mid_result_file <<  cur_mid.y << ": " << std::endl;
			}
			else if (cur_mid.op == 14){
			    std::string temp;
                if(cur_mid.x=="chars"){
                    temp="char";
                }else if(cur_mid.x=="ints"){
                    temp="int";
                }else{
                    temp="void";
                }
                mid_result_file << "    " << temp<<"  "<<cur_mid.y << "()"<<std::endl;
			}
            else if (cur_mid.op == 15)
				mid_result_file << "    " << MidOpKind[cur_mid.op] << "  "<< cur_mid.x<<"  "<<cur_mid.y<<std::endl;

			else if (cur_mid.op == 16) {
				mid_result_file << "    " << MidOpKind[cur_mid.op] << "  " << cur_mid.y << std::endl;
			}
			else if (cur_mid.op == 17) {
				if (cur_mid.x != " ") {
					mid_result_file << "    " << MidOpKind[cur_mid.op] << "  " << cur_mid.x << ", " << cur_mid.y << std::endl;
				}
				else {
					mid_result_file << "    " << MidOpKind[cur_mid.op] << "  " << cur_mid.y << std::endl;
				}
			}
			else if(cur_mid.op == 18){
                mid_result_file << "    " << "push" << "  " << cur_mid.y << std::endl;
			}
			else if (cur_mid.op == 19) {
				mid_result_file << "    " << MidOpKind[cur_mid.op] << "  " << cur_mid.y << std::endl;
			}
			else if (cur_mid.op == 20) {
				mid_result_file << "    " << cur_mid.z << "[" << cur_mid.x << "]" << " = " << cur_mid.y << std::endl;
			}
			else if (cur_mid.op == 21) {
				mid_result_file << "    " << MidOpKind[cur_mid.op] << "  " << cur_mid.z << std::endl;
			}
		}
	}

	void printMipsResult() {
		std::fstream mips_result_file;
		mips_result_file.open("16061002_张凯宁_优化前目标代码.txt", std::fstream::out);
		mips_result_file << "# Please set to begin at \"main\"" << std::endl;
		mips_result_file << ".globl main" << std::endl;
		mips_result_file << ".data" << std::endl;
		//存放字符串以便打印
		auto iter = Str_Info_Map.begin();
		while (iter != Str_Info_Map.end()) {
			mips_result_file << "    " << iter->first << ": .asciiz \"" << iter->second << "  \"" << std::endl;
			++iter;
		}
		mips_result_file << ".text" << std::endl;
		for (int i = 0; i < MipsTable.mpc; i++) {
			mipscode cur_mips = MipsTable.mipscodeArray[i];
			if (cur_mips.op < 4) {
				//if (cur_mips.op == 2) std::cout << "1" << std::endl;
				mips_result_file << "    " << MipsOpKind[cur_mips.op] << "  " << cur_mips.z << ", " << cur_mips.x << ", " << cur_mips.y << std::endl;
			}
			else if (cur_mips.op < 8) {
				//if (cur_mips.op == 6) std::cout << "2" << std::endl;
				mips_result_file << "    " << MipsOpKind[cur_mips.op] << "  " << cur_mips.z << ", " << cur_mips.x << ", " << cur_mips.offset << std::endl;
			}
			else if (cur_mips.op < 10) {
				mips_result_file << "    " << MipsOpKind[cur_mips.op] << "  " << cur_mips.z << ", " << cur_mips.offset<<"("<<cur_mips.x<<")"<< std::endl;
			}
			else if (cur_mips.op < 16) {
				if (cur_mips.y == " ") {		//offset
					mips_result_file << "    " << MipsOpKind[cur_mips.op] << "  " << cur_mips.x << ", "<< cur_mips.offset << ", " << cur_mips.z << std::endl;
				}
				else {				// y
					mips_result_file << "    " << MipsOpKind[cur_mips.op] << "  " << cur_mips.x << ", " << cur_mips.y << ", " << cur_mips.z << std::endl;
				}
			}
			else if (cur_mips.op < 19) {
				mips_result_file << "    " << MipsOpKind[cur_mips.op] << "  " << cur_mips.z << std::endl;
			}
			else if (cur_mips.op == 19) {
				mips_result_file << "    " << MipsOpKind[cur_mips.op] << "  " << cur_mips.z << ", " << cur_mips.offset << std::endl;
			}
			else if (cur_mips.op == 20) {
				mips_result_file << "    " << MipsOpKind[cur_mips.op] << "  " << cur_mips.z << ", " << cur_mips.x << std::endl;
			}
			else if (cur_mips.op == 21) {
				mips_result_file << "    " << "syscall" << std::endl;
			}
			else if (cur_mips.op == 22) {
				mips_result_file << std::endl;
				mips_result_file << cur_mips.z << ":" << std::endl;
			}
			else if (cur_mips.op == 23) {
				mips_result_file << "    " << MipsOpKind[cur_mips.op] << "  " << cur_mips.z << ", " << cur_mips.x << std::endl;
			}
		}

		mips_result_file << "exit:" << std::endl;

	}

	void printMidOptResult() {
		std::fstream mid_result_file;
		mid_result_file.open("16061002_张凯宁_优化后中间代码.txt", std::fstream::out);
		mid_result_file << "RET: func_return_value" << std::endl;
		mid_result_file << std::endl;
		for (int i = 0; i < MidCodeOptT.mdc; i++) {
			midcode cur_mid = MidCodeOptT.midcodeArray[i];
			if (cur_mid.op < 4) {
				mid_result_file << "    " << cur_mid.z << " = " << cur_mid.x << " " << MidOpKind[cur_mid.op] << " " << cur_mid.y << std::endl;
			}
			else if (cur_mid.op == 4) {
				if (cur_mid.x == " ") {
					mid_result_file << "    " << cur_mid.z << " = " << cur_mid.y << std::endl;
				}
				else {
					mid_result_file << "    " << cur_mid.z << " = " << cur_mid.y << "[" << cur_mid.x << "]" << std::endl;
				}
			}
			else if (cur_mid.op == 5) {
				mid_result_file << "    " << MidOpKind[cur_mid.op] << "  " << cur_mid.z << std::endl;
			}
			else if (cur_mid.op < 12) {
				mid_result_file << "    " << MidOpKind[cur_mid.op] << "  " << cur_mid.x << ", " << cur_mid.y << ", " << cur_mid.z << std::endl;
			}
			else if (cur_mid.op == 12) {
				mid_result_file << "    " << MidOpKind[cur_mid.op] << "  " << cur_mid.y << std::endl;
			}
			else if (cur_mid.op == 13) {
				mid_result_file << std::endl;
				mid_result_file << cur_mid.y << ": " << std::endl;
			}
			else if (cur_mid.op < 16) {
				if (cur_mid.op == 14) mid_result_file << std::endl;
				mid_result_file << "    " << MidOpKind[cur_mid.op] << "  " << cur_mid.x << ", " << cur_mid.y << std::endl;
			}
			else if (cur_mid.op == 16) {
				mid_result_file << "    " << MidOpKind[cur_mid.op] << "  " << cur_mid.y << std::endl;
			}
			else if (cur_mid.op == 17) {
				if (cur_mid.x != " ") {
					mid_result_file << "    " << MidOpKind[cur_mid.op] << "  " << cur_mid.x << ", " << cur_mid.y << std::endl;
				}
				else {
					mid_result_file << "    " << MidOpKind[cur_mid.op] << "  " << cur_mid.y << std::endl;
				}
			}
			else if (cur_mid.op < 20) {
				mid_result_file << "    " << MidOpKind[cur_mid.op] << "  " << cur_mid.y << std::endl;
			}
			else if (cur_mid.op == 20) {
				mid_result_file << "    " << cur_mid.z << "[" << cur_mid.x << "]" << " = " << cur_mid.y << std::endl;
			}
			else if (cur_mid.op == 21) {
				mid_result_file << "    " << MidOpKind[cur_mid.op] << "  " << cur_mid.z << std::endl;
			}
		}
	}

	void printMipsOptResult() {
		std::fstream mips_result_file;
		mips_result_file.open("result/16061002_张凯宁_优化后目标代码.txt", std::fstream::out);
		mips_result_file << "# Please set to begin at \"main\"" << std::endl;
		mips_result_file << ".globl main" << std::endl;
		mips_result_file << ".data" << std::endl;
		//存放字符串以便打印
		auto iter = Str_Info_Map.begin();
		while (iter != Str_Info_Map.end()) {
			mips_result_file << "    " << iter->first << ": .asciiz \"" << iter->second << "  \"" << std::endl;
			++iter;
		}
		mips_result_file << ".text" << std::endl;
		for (int i = 0; i < MipsTable.mpc; i++) {
			mipscode cur_mips = MipsTable.mipscodeArray[i];
			if (cur_mips.op < 4) {
				//if (cur_mips.op == 2) std::cout << "1" << std::endl;
				mips_result_file << "    " << MipsOpKind[cur_mips.op] << "  " << cur_mips.z << ", " << cur_mips.x << ", " << cur_mips.y << std::endl;
			}
			else if (cur_mips.op < 8) {
				//if (cur_mips.op == 6) std::cout << "2" << std::endl;
				mips_result_file << "    " << MipsOpKind[cur_mips.op] << "  " << cur_mips.z << ", " << cur_mips.x << ", " << cur_mips.offset << std::endl;
			}
			else if (cur_mips.op < 10) {
				mips_result_file << "    " << MipsOpKind[cur_mips.op] << "  " << cur_mips.z << ", " << cur_mips.offset << "(" << cur_mips.x << ")" << std::endl;
			}
			else if (cur_mips.op < 16) {
				if (cur_mips.y == " ") {		//offset
					mips_result_file << "    " << MipsOpKind[cur_mips.op] << "  " << cur_mips.x << ", " << cur_mips.offset << ", " << cur_mips.z << std::endl;
				}
				else {				// y
					mips_result_file << "    " << MipsOpKind[cur_mips.op] << "  " << cur_mips.x << ", " << cur_mips.y << ", " << cur_mips.z << std::endl;
				}
			}
			else if (cur_mips.op < 19) {
				mips_result_file << "    " << MipsOpKind[cur_mips.op] << "  " << cur_mips.z << std::endl;
			}
			else if (cur_mips.op == 19) {
				mips_result_file << "    " << MipsOpKind[cur_mips.op] << "  " << cur_mips.z << ", " << cur_mips.offset << std::endl;
			}
			else if (cur_mips.op == 20) {
				mips_result_file << "    " << MipsOpKind[cur_mips.op] << "  " << cur_mips.z << ", " << cur_mips.x << std::endl;
			}
			else if (cur_mips.op == 21) {
				mips_result_file << "    " << "syscall" << std::endl;
			}
			else if (cur_mips.op == 22) {
				mips_result_file << std::endl;
				mips_result_file << cur_mips.z << ":" << std::endl;
			}
			else if (cur_mips.op == 23) {
				mips_result_file << "    " << MipsOpKind[cur_mips.op] << "  " << cur_mips.z << ", " << cur_mips.x << std::endl;
			}
		}

		mips_result_file << "exit:" << std::endl;

	}



}
