/*
	词法分析及处理
*/
#include "Word_Deal.h"
#include <iostream>

namespace compiler{
    int word_count = 1;


	//根据枚举类型Symbol的返回其具体类型名
    std::string return_symbol_info(symbol sy){
    	return symbol_info[sy];
    }

        //read next character
    void nextch(){
        if(cc>=ll){
            //文件读完了或刚开始进入本程序cc==ll
           // if(inFile.eof()){
              //  exit(1);//文件读完了,结束
            //}
           //文件还没有读完，读取新的一行
            for(int i = 0; i < LLNG + 1; i++){ //将暂时盛放当前读取的一行字符的数组初始化，对前150个字符填充'\0'
                line[i] = '\0';
            }
            ll = 0; //将行长度和行指针清零
            cc = 0;//将当前行读取过的字符数目清零
            inFile.getline(line, LLNG, '\n');//从inFile中读取新的一行(最多150个字符)到line数组中，读入'\n'结束
            lc++;//增加已读行的数目
            while(line[ll]!='\0')
                ll++; //通过循环到'\0'计数当前行line的长度
            line[ll] = ' '; //一行的末尾置为空格
        }
        ch = line[cc];//读取当前行的第cc个字符放到变量ch中
        cc = cc + 1;//移动cc指针到下一个字符，以便下次读取
    }

    void insymbol(){
        int k;
        while((ch == ' ')||(ch == '\t')){   //跳过空格和tab
            nextch();
        }

        if((ch >= '0') && (ch <= '9')){   //开始对0和无符号整数进行识别, 0开头的数容错, 例如0123按照123存储,但是输出错误信息
            char firstchar = ch;//记录第一个数字，以便判断是否前零
            inum = 0;
            sy = intcon;    //设置sy表示为整数

            do{
                inum = inum * 10 + (ch - '0');
                nextch();
                if(inum > NMAX){ //数字超过限定大小
					error(1,lc);
                }
                if (cc == 1) break;	//换行了，直接出来
            }while((ch >= '0') && (ch <= '9'));//后面接数字
            if((firstchar == '0') && (inum != 0)){//如果第一个数字是0而数字本身不是0，即有前导0
				error(9, lc);
                word_out_file<<word_count<<" error: 0 can't be the first, but we accept it"<<std::endl;
            }
            word_out_file<<word_count++<<' '<<return_symbol_info(sy)<<' '<<inum<<std::endl;//打印数字及值
        }else if((ch == '_')||((ch >= 'a') && (ch <= 'z'))||((ch >= 'A') && (ch <= 'Z'))){//处理标识符,只能是字母('_',a-z,A-Z)或者数字,而且第一个必须是字母
            k = 0;
            for(int t = 0; t < NAME_SIZE; t++) id[t] = '\0';//清空标识符记录数组
            do{
                if (k < NAME_SIZE){ //满足后面跟数字或字母，且不超过最长，将其记录到id中
                   id[k] = ch;
                    ++k;
                    nextch();
                }
				if (cc == 1) break;	//换行了，直接出来
            }while((ch == '_')||((ch >= 'a') && (ch <= 'z'))||((ch >= 'A') && (ch <= 'Z'))||((ch >= '0') && (ch <= '9')));
            //在关键字表中查询是否有该名字
            auto iter = KeyMap.find(id);
            //没有找到
            if(iter == KeyMap.end()) sy = ident;
            //找到了
            else sy = symbol(iter->second);
            word_out_file<<word_count++<<' '<<return_symbol_info(sy)<<' '<<id<<std::endl;
        }else if(ch == '<'){        //开始小于号识别
            nextch();
            if ((ch == '=')&&(cc!=1)){ //不支持换行的情况下后面跟=
                sy = leq;   //小于等于号
                nextch();
                word_out_file<<word_count++<<' '<<return_symbol_info(sy)<<' '<<"<="<<std::endl;
            }else{//换行或者后面跟的不是=
                sy = lss;   // 小于号
                word_out_file<<word_count++<<' '<<return_symbol_info(sy)<<' '<<"<"<<std::endl;
            }
        }else if((ch == '>')&&(cc!=1)){
            nextch();
            if(ch == '='){
                sy = geq;   //大于等于号
                nextch();
                word_out_file<<word_count++<<' '<<return_symbol_info(sy)<<' '<<">="<<std::endl;
            }else{
                sy = gtr;   //大于号
                word_out_file<<word_count++<<' '<<return_symbol_info(sy)<<' '<<">"<<std::endl;
            }
        }else if(ch == '\''){   //单引号开始处理字符
            nextch();
            if(ch == '\''){//'后面直接跟'，说明是空字符
                error(2,lc);
                sy = charcon;
                inum = 0;
                word_out_file<<word_count++<<' '<<"error: char is empty"<<std::endl;
                //nextch();
            }else if((ch == '+') || (ch == '-') || (ch == '*') || (ch == '/') || (ch == '_') || ((ch >= '0') && (ch <= '9'))
                        || ((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z'))){
                sy = charcon;
                inum = ch;
                word_out_file<<word_count++<<' '<<return_symbol_info(sy)<<' '<<ch<<std::endl;
                nextch();
            }else{//''中间是一个非正常字符
                sy = charcon;
                inum = 0;
                error(8, lc);
                word_out_file<<word_count++<<' '<<"error: illegal char in char_def"<<std::endl;
                nextch();
            }

            if(ch != '\''){//与'配对的'
                error(3,lc);
                word_out_file<<word_count++<<' '<<"error:must be '\''"<<std::endl;
            }else {
                nextch(); //是单引号则继续读取下一个字符
            }
        }else if(ch == '\"'){   //双引号开始处理字符串
            k = 0;
            nextch();
            while((ch != '\"') && (cc != 1)){//没有换行且没有读到"
                if((ch == 32) || (ch == 33) || ((ch >= 35) && (ch <= 126))){    //合法字符
                    if(ch == 92){
                        stab.stringArray[stab.sx + k] = ch;
                        ++k;
                    }
                    stab.stringArray[stab.sx + k] = ch;
                    ++k;
                }else{//读到不合法字符，将略过该字符继续读取
                    error(4,lc);
                    word_out_file<<word_count++<<ch<<std::endl;
                    word_out_file<<word_count++<<"error: illegal char in string, which will be skiped"<<std::endl;
                }
                if(stab.sx + k == SMAX){//表格满
                    fatal(0, lc);
                    word_out_file<<word_count++<<' '<<"error: stab is full"<<std::endl;
                }
                nextch();
            }
            if(ch == '\"'){ //字符串以"正常结束
                sy = stringcon;
                inum = stab.sx; //存储字符串开始时的下标
                sleng = k; //字符串的长度
                stab.sx = stab.sx + k;//移动在表格的指针
                nextch();//获取新的ch

                //词法分析打印使用
                char stemp[SMAX];//使用一个新的临时数组保存此次读到的字符串传入输出流

                for(int i = 0; i < k; i++){
                    stemp[i] = stab.stringArray[inum + i];
                }
                stemp[k] = '\0';//字符串以\0结束
                word_out_file<<word_count++<<' '<<return_symbol_info(sy)<<' '<<stemp<<std::endl;


            }else{          //cc = 1,未读到双引号就换行了,进行报错
                error(5, lc-1);
                word_out_file<<word_count++<<' '<<"error: no  \" but reach next line"<<std::endl;
            }
        }else if(ch == '!'){ // 处理!=,'!'单独出现算错
            nextch();       // !和=之间不能分离也不能换行
            if((ch == '=')&&(cc!=1)){
                sy = neq;
                nextch();
                word_out_file<<word_count++<<' '<<return_symbol_info(sy)<<' '<<"!="<<std::endl;
            }else{
                error(6,lc);
                sy = neq; //容错,将！作为 != 处理
                word_out_file<<word_count++<<' '<<"error: \'!\' can\'t appear alone"<<std::endl;
            }

		}
		else if (ch == '=') {
			nextch();
			if ((ch == '=')&&(cc!=1)) {//==不能分离也不能换行
				sy = eql;
				word_out_file << word_count++ << ' ' << "eql" << ' ' << "==" << std::endl;
				nextch();
			}
			else {
				sy = assign;
				word_out_file << word_count++ << ' ' << "assign" << ' ' << '=' << std::endl;
			}
		}
		else if ((ch == '+') || (ch == '-') || (ch == '*') || (ch == '/') || (ch == '(')
                || (ch == ')') || (ch == '[') || (ch == ']') || (ch == '{') || (ch == '}') || (ch == ',')
                || (ch == ';') || (ch == ':')){

            sy = SpsMap[ch];
            word_out_file<<word_count++<<' '<<return_symbol_info(sy)<<' '<<ch<<std::endl;
            nextch();
        }else {
            error(7,lc);
            word_out_file<<word_count++<<' '<<"error: illegal char in program"<<std::endl;
			nextch();
        }


    }
}
