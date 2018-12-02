/*
	�ʷ�����������
*/
#include "Word_Deal.h"
#include <iostream>

namespace compiler{
    int word_count = 1;


	//����ö������Symbol�ķ��������������
    std::string return_symbol_info(symbol sy){
    	return symbol_info[sy];
    }

        //read next character
    void nextch(){
        if(cc>=ll){
            //�ļ������˻�տ�ʼ���뱾����cc==ll
           // if(inFile.eof()){
              //  exit(1);//�ļ�������,����
            //}
           //�ļ���û�ж��꣬��ȡ�µ�һ��
            for(int i = 0; i < LLNG + 1; i++){ //����ʱʢ�ŵ�ǰ��ȡ��һ���ַ��������ʼ������ǰ150���ַ����'\0'
                line[i] = '\0';
            }
            ll = 0; //���г��Ⱥ���ָ������
            cc = 0;//����ǰ�ж�ȡ�����ַ���Ŀ����
            inFile.getline(line, LLNG, '\n');//��inFile�ж�ȡ�µ�һ��(���150���ַ�)��line�����У�����'\n'����
            lc++;//�����Ѷ��е���Ŀ
            while(line[ll]!='\0')
                ll++; //ͨ��ѭ����'\0'������ǰ��line�ĳ���
            line[ll] = ' '; //һ�е�ĩβ��Ϊ�ո�
        }
        ch = line[cc];//��ȡ��ǰ�еĵ�cc���ַ��ŵ�����ch��
        cc = cc + 1;//�ƶ�ccָ�뵽��һ���ַ����Ա��´ζ�ȡ
    }

    void insymbol(){
        int k;
        while((ch == ' ')||(ch == '\t')){   //�����ո��tab
            nextch();
        }

        if((ch >= '0') && (ch <= '9')){   //��ʼ��0���޷�����������ʶ��, 0��ͷ�����ݴ�, ����0123����123�洢,�������������Ϣ
            char firstchar = ch;//��¼��һ�����֣��Ա��ж��Ƿ�ǰ��
            inum = 0;
            sy = intcon;    //����sy��ʾΪ����

            do{
                inum = inum * 10 + (ch - '0');
                nextch();
                if(inum > NMAX){ //���ֳ����޶���С
					error(1,lc);
                }
                if (cc == 1) break;	//�����ˣ�ֱ�ӳ���
            }while((ch >= '0') && (ch <= '9'));//���������
            if((firstchar == '0') && (inum != 0)){//�����һ��������0�����ֱ�����0������ǰ��0
				error(9, lc);
                word_out_file<<word_count<<" error: 0 can't be the first, but we accept it"<<std::endl;
            }
            word_out_file<<word_count++<<' '<<return_symbol_info(sy)<<' '<<inum<<std::endl;//��ӡ���ּ�ֵ
        }else if((ch == '_')||((ch >= 'a') && (ch <= 'z'))||((ch >= 'A') && (ch <= 'Z'))){//�����ʶ��,ֻ������ĸ('_',a-z,A-Z)��������,���ҵ�һ����������ĸ
            k = 0;
            for(int t = 0; t < NAME_SIZE; t++) id[t] = '\0';//��ձ�ʶ����¼����
            do{
                if (k < NAME_SIZE){ //�����������ֻ���ĸ���Ҳ�������������¼��id��
                   id[k] = ch;
                    ++k;
                    nextch();
                }
				if (cc == 1) break;	//�����ˣ�ֱ�ӳ���
            }while((ch == '_')||((ch >= 'a') && (ch <= 'z'))||((ch >= 'A') && (ch <= 'Z'))||((ch >= '0') && (ch <= '9')));
            //�ڹؼ��ֱ��в�ѯ�Ƿ��и�����
            auto iter = KeyMap.find(id);
            //û���ҵ�
            if(iter == KeyMap.end()) sy = ident;
            //�ҵ���
            else sy = symbol(iter->second);
            word_out_file<<word_count++<<' '<<return_symbol_info(sy)<<' '<<id<<std::endl;
        }else if(ch == '<'){        //��ʼС�ں�ʶ��
            nextch();
            if ((ch == '=')&&(cc!=1)){ //��֧�ֻ��е�����º����=
                sy = leq;   //С�ڵ��ں�
                nextch();
                word_out_file<<word_count++<<' '<<return_symbol_info(sy)<<' '<<"<="<<std::endl;
            }else{//���л��ߺ�����Ĳ���=
                sy = lss;   // С�ں�
                word_out_file<<word_count++<<' '<<return_symbol_info(sy)<<' '<<"<"<<std::endl;
            }
        }else if((ch == '>')&&(cc!=1)){
            nextch();
            if(ch == '='){
                sy = geq;   //���ڵ��ں�
                nextch();
                word_out_file<<word_count++<<' '<<return_symbol_info(sy)<<' '<<">="<<std::endl;
            }else{
                sy = gtr;   //���ں�
                word_out_file<<word_count++<<' '<<return_symbol_info(sy)<<' '<<">"<<std::endl;
            }
        }else if(ch == '\''){   //�����ſ�ʼ�����ַ�
            nextch();
            if(ch == '\''){//'����ֱ�Ӹ�'��˵���ǿ��ַ�
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
            }else{//''�м���һ���������ַ�
                sy = charcon;
                inum = 0;
                error(8, lc);
                word_out_file<<word_count++<<' '<<"error: illegal char in char_def"<<std::endl;
                nextch();
            }

            if(ch != '\''){//��'��Ե�'
                error(3,lc);
                word_out_file<<word_count++<<' '<<"error:must be '\''"<<std::endl;
            }else {
                nextch(); //�ǵ������������ȡ��һ���ַ�
            }
        }else if(ch == '\"'){   //˫���ſ�ʼ�����ַ���
            k = 0;
            nextch();
            while((ch != '\"') && (cc != 1)){//û�л�����û�ж���"
                if((ch == 32) || (ch == 33) || ((ch >= 35) && (ch <= 126))){    //�Ϸ��ַ�
                    if(ch == 92){
                        stab.stringArray[stab.sx + k] = ch;
                        ++k;
                    }
                    stab.stringArray[stab.sx + k] = ch;
                    ++k;
                }else{//�������Ϸ��ַ������Թ����ַ�������ȡ
                    error(4,lc);
                    word_out_file<<word_count++<<ch<<std::endl;
                    word_out_file<<word_count++<<"error: illegal char in string, which will be skiped"<<std::endl;
                }
                if(stab.sx + k == SMAX){//�����
                    fatal(0, lc);
                    word_out_file<<word_count++<<' '<<"error: stab is full"<<std::endl;
                }
                nextch();
            }
            if(ch == '\"'){ //�ַ�����"��������
                sy = stringcon;
                inum = stab.sx; //�洢�ַ�����ʼʱ���±�
                sleng = k; //�ַ����ĳ���
                stab.sx = stab.sx + k;//�ƶ��ڱ���ָ��
                nextch();//��ȡ�µ�ch

                //�ʷ�������ӡʹ��
                char stemp[SMAX];//ʹ��һ���µ���ʱ���鱣��˴ζ������ַ������������

                for(int i = 0; i < k; i++){
                    stemp[i] = stab.stringArray[inum + i];
                }
                stemp[k] = '\0';//�ַ�����\0����
                word_out_file<<word_count++<<' '<<return_symbol_info(sy)<<' '<<stemp<<std::endl;


            }else{          //cc = 1,δ����˫���žͻ�����,���б���
                error(5, lc-1);
                word_out_file<<word_count++<<' '<<"error: no  \" but reach next line"<<std::endl;
            }
        }else if(ch == '!'){ // ����!=,'!'�����������
            nextch();       // !��=֮�䲻�ܷ���Ҳ���ܻ���
            if((ch == '=')&&(cc!=1)){
                sy = neq;
                nextch();
                word_out_file<<word_count++<<' '<<return_symbol_info(sy)<<' '<<"!="<<std::endl;
            }else{
                error(6,lc);
                sy = neq; //�ݴ�,������Ϊ != ����
                word_out_file<<word_count++<<' '<<"error: \'!\' can\'t appear alone"<<std::endl;
            }

		}
		else if (ch == '=') {
			nextch();
			if ((ch == '=')&&(cc!=1)) {//==���ܷ���Ҳ���ܻ���
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
