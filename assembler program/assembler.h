#pragma once
#include<bits/stdc++.h>
using namespace std;

struct Operand{
	bool Dir , noOperand , isBranch , isValue , immediate , absolute ;
	int reg_num , mode;
	bitset<8> offset;
	bitset<16> indexed_value;
	Operand(){
		isBranch = false ; isValue = false;
		Dir = false , noOperand = true;
		immediate = absolute = false;
		reg_num = 0;
		mode = 0;
	}
};


enum operand_type{
	NO_OPERAND,
	OPERAND_1,
	OPERAND_2,
	BR_OPERAND
};

enum Address_Mode{
	REG,
	AUTOINC,
	AUTODEC,
	INDEXED
};

class Assembler{

	const vector<string> operand_2{ "mov" , "add" , "adc" , "sub" , "suc" , "and" , "or" , "xnor" , "cmp" , "jsr" };
	const vector<string> operand_1{ "inc" , "dec" , "clr" , "inv" , "lsr" , "ror" , "rrc" , "asr" , "lsl" , "rol" ,"rlc" };
	const vector<string> operand_br{ "br" , "beq" , "bne" , "blo" , "bls" , "bhi" , "bhs" };
	const vector<string> operand_no{ "rts" , "interrupt" , "iret" , "hlt" , "nop"};
	// this vector contains instructions read from file
	vector<pair<string , pair<Operand , Operand>>> instr;
	//this vector contains op codes
	vector<bitset<16>> codes;
	// utility functions  //***********************************************************************************//
	bool isDigit(char x){
		for(int i = 0 ; i <= 9 ; i ++ ) if(x-'0' == i ) return true;
		return false;
	}
	int operationType(string s){
		int type = -1;
		for(auto x : operand_2 ){
			if(type != -1) break;
			if(x == s) { type = OPERAND_2; break;}
		}
		for(auto x : operand_1 ){
			if(type != -1) break;
			if(x == s) {type = OPERAND_1; break;}
		}
		for(auto x : operand_br ){
			if(type != -1) break;
			if(x == s) { type = BR_OPERAND; break; }
		}
		for(auto x : operand_no ){
			if(type != -1) break;
			if(x == s) { type = NO_OPERAND; break;}
		}
		return type;
	}

	bitset<16> toBinary(int n){
		bitset<16> b;
		int i = 0;
		while(n > 0){
			if(n%2) b[i] = 1;
			n/=2;
			i++;
		}
		return b;
	}

	string fetchOperation(string s , int& i){

		// fetching operation
		while(s[i] == ' ') ++i;
		string op="";
		while(s[i] != ' ' && s[i] != '\n') op += tolower(s[i++]);
		return op;
	}

	Operand parseString(string s){

		int i = 0;
		// checking if direct or in indirect
		Operand operand;
		while(s[i] == ' ') ++i;

		if(s[i] == '#'){
			++i;
			string num = "";
			while(i < (int) s.size() && s[i] != ' ') num += s[i++];
			operand.immediate = true;
			operand.indexed_value = toBinary(stoi(num));
			operand.Dir = true;
			operand.mode = AUTOINC;
			operand.reg_num = 7;
			return operand;
		}
		else if(isDigit(s[i])){
			string num = "";
			while(i < (int) s.size() && s[i] != ' ') num += s[i++];
			operand.absolute = true;
			operand.indexed_value = toBinary(stoi(num));
			operand.Dir = true;
			operand.mode = INDEXED;
			operand.reg_num = 7;
			return operand;
		}

		if(s[i] != '@') operand.Dir = true;
		if( !operand.Dir  ) i++;
		//checking if autoDecrement or indexed
		while(s[i] == ' ') ++i;
		string oper = "";
		if(s[i] == '-') {
			operand.mode = AUTODEC , i++;
			while(s[i] == ' ' || s[i] == '(') i++;
			while(s[i] != ' ' && s[i] != ')') oper += tolower(s[i++]);
		}
		else{
			string indx = "";
			while(s[i] != ' ' && s[i] != '(' && i< (int) s.size()) indx += tolower(s[i++]);
			while(s[i] == ' ' && i <(int) s.size()) ++i;
			if(s[i] == '(' && indx != "") operand.mode = INDEXED;
			if(i == (int)s.size()) oper = indx;
			else{
				i++;
				while(s[i] ==' ') ++i;
				while(s[i] != ' ' && s[i] != ')') oper += tolower(s[i++]);
				while((s[i] ==' ' || s[i] ==')') && i < (int) s.size() ) ++i;
				if(i < (int) s.size() && s[i] == '+') operand.mode = AUTOINC;
			}
		}
		operand.reg_num = oper[(int)oper.size()-1]-'0';
		return operand;
	}

	bitset<4> encode_2_operand(string s){
		bitset<4> b;
		int l = 3 , r = 0;
		for(auto x : operand_2){
			if(x == s){
				b[r] = 1;
				break;
			}
			else{
				if(l == r){
					b[r] = 1; l-- ; r=0;
				}
				else r++;
			}
		}
		return b;
	}

	bitset<4> encode_1_operand(string s){
		bitset<4> b;
		if(s == "rlc") return b;
		int l = 3 , r = 0;
		for(auto x : operand_1){
			if(x == s){
				b[r] = 1;
				break;
			}
			else{
				if(l == r) b[r] = 1 , l-- , r=0;
				else r++;
			}
		}
		return b;
	}

	bitset<3> encode_br_opernad(string s){
		bitset<3> b;
		if(s == "bhs") return b;
		int l = 2 , r = 0;
		for(auto x : operand_br){
			if(x == s){
				b[r] = 1;
				break;
			}
			else{
				if(l == r) b[r] = 1 , l-- , r=0;
				else r++;
			}
		}
		return b;
	}



public:

	Assembler(string file_name){
		ifstream cin; cin.open(file_name);
		string s;
		getline(cin , s);
		int address = stoi(&s[1]);
		while(getline(cin , s)){

			if(s=="") continue;
			// incrementing address.
			address++;
			int i =0;
			string op = fetchOperation(s,i);
			//check if NOP
			if(op == "nop"){
				continue;
			}
			//check if hlt
			if(op == "hlt"){
				Operand opp;
				instr.push_back({op,{opp,opp}});
				continue;
			}
			//check if variable
			if(op[0] == '#'){
				int j = 1;
				while(op[j] == ' ') ++j;
				bool neg = false;
				if(op[j] == '-') neg = true , j++;
				bitset<16> b = toBinary(stoi(&op[j]));
				//two's complement if offset is negative........................
				bool first_one=false; 
				for(int i = 0 ; i <= 15 && neg; i++ ){
					if(first_one){
						b[i] = !b[i];
					}
					if(b[i]) first_one = true;
				}
				Operand opp;
				opp.isValue = true;
				opp.indexed_value = b;
				instr.push_back({"",{opp,opp}});
				continue;
			}

			int op_type = operationType(op);
			// check if Branch operand..
			if(op_type == BR_OPERAND){
				while(s[i] == ' ') ++i;
				string offset = "";
				bool neg = false;
				if(s[i] == '-') neg = true , i++;
				while(i < (int) s.size()) offset += s[i++];
				int offset_ = stoi(offset);
				bitset<16> b = toBinary(offset_);
				//two's complement if offset is negative........................
				bool first_one=false; 
				for(int i = 0 ; i <= 7 && neg; i++ ){
					if(first_one){
						b[i] = !b[i];
					}
					if(b[i]) first_one = true;
				}

				Operand operand1;
				for(int i = 0 ; i <= 7 ; ++i)
					operand1.offset[i] = b[i];
				operand1.isBranch = true;
				instr.push_back({op , {operand1 , operand1}});
				continue;
			}

			string s1 = "";
			while(s[i] != ',' && i < (int)s.size()) s1 += s[i++];
			Operand operand1 = parseString(s1);

			operand1.noOperand = false;
			Operand operand2;
			if(i<(int) s.size()){
				i++;
				string s2 = "";
				while(i<(int) s.size()) s2 += s[i++];
				operand2 = parseString(s2);
				operand2.noOperand = false;
			}
			instr.push_back({op , {operand1 , operand2}});
			if(operand1.absolute){
				Operand opp;
				opp.isValue = true;
				opp.indexed_value = toBinary((operand1.indexed_value).to_ulong()-(address+1));
				instr.push_back({"" , {opp , opp}});
				address++;
			}
			if(operand2.absolute){
				Operand opp;
				opp.isValue = true;
				opp.indexed_value = toBinary((operand2.indexed_value).to_ulong()-(address+1));
				instr.push_back({"" , {opp , opp}});
				address++;
			}

			if(operand1.immediate){
				Operand opp;
				opp.isValue = true;
				opp.indexed_value = operand1.indexed_value;
				instr.push_back({"" , {opp , opp}});
				address++;
			}

			if(operand1.mode == INDEXED && !operand1.absolute){
				string x;
				getline(cin , x);
				int j = 0;
				while(x[j] == ' ') ++j;
				string num ="";
				while(j < (int) x.size() && x[j] !=' ') num += x[j++];
				bitset<16> b = toBinary(stoi(num));
				Operand operand;
				operand.isValue = true;
				operand.indexed_value = b;
				instr.push_back({"" , {operand , operand}});
			}
			if(operand2.mode == INDEXED && !operand2.absolute){
				string x;
				getline(cin , x);
				int j = 0;
				while(x[j] == ' ') ++j;
				string num ="";
				while(j < (int) x.size() && x[j] !=' ') num += x[j++];
				bitset<16> b = toBinary(stoi(num));
				Operand operand;
				operand.isValue = true;
				operand.indexed_value = b;
				instr.push_back({"" , {operand , operand}});
			}
		}

	}

	void encode(){
		for(auto instruction : instr){
			bitset<16> code;
			if(instruction.second.first.isValue){
				codes.push_back(instruction.second.first.indexed_value);
				continue;
			}

			string op = instruction.first;
			
			// handling hlt
			if(op == "hlt"){
				code[15]=1; code[14] =1; code[13] =1; code[12] = 1;
				codes.push_back(code);
				continue;
			}
			int op_type = operationType(op);
			if(op_type == OPERAND_2){
				bitset<4> b = encode_2_operand(op);
				int j=0;
				for(int i = 12 ; i < 16 ; i++) code[i] = b[j++];
				if(!instruction.second.first.Dir) code[9] = 1;
				if(!instruction.second.second.Dir) code[3] = 1;
				int mode_src  =  instruction.second.first.mode;
				int mode_dest = instruction.second.second.mode;
				if(mode_src == AUTOINC) code[10] = 1;
				else if(mode_src == AUTODEC) code[11] = 1;
				else if(mode_src == INDEXED) code[10] = 1 , code[11] = 1;
				if(mode_dest == AUTOINC) code[4] = 1;
				else if(mode_dest == AUTODEC) code[5] = 1;
				else if(mode_dest == INDEXED) code[4] = 1 , code[5] = 1;
				bitset<16> src_num = toBinary(instruction.second.first.reg_num);
				bitset<16> dest_num = toBinary(instruction.second.second.reg_num);
				j = 0;
				for(int i = 0 ; i <= 2 ; i++) code[i] = dest_num[j++];
				j = 0;
				for(int i = 6 ; i <= 8 ; i++) code[i] = src_num[j++];
			}
			else if(op_type == OPERAND_1){
				bitset<4> b = encode_1_operand(op);
				code[10] = 1;
				int j = 0;
				for(int i = 6 ; i<=9; ++i) code[i] = b[j++];
				if(!instruction.second.first.Dir) code[3] = 1;
				int mode_src  =  instruction.second.first.mode;
				if(mode_src == AUTOINC) code[4] = 1;
				else if(mode_src == AUTODEC) code[5] = 1;
				else if(mode_src == INDEXED) code[4] = 1 , code[5] = 1;
				bitset<16> src_num = toBinary(instruction.second.first.reg_num);
				j = 0;
				for(int i = 0 ; i <= 2 ; i++) code[i] = src_num[j++];
			}
			else if(op_type == BR_OPERAND){
				code[11] = 1;
				bitset<3> b = encode_br_opernad(op);
				code[10] = b[2] , code[9] = b[1] , code[8] = b[0];
				bitset<8> offset = instruction.second.first.offset;
				for(int i = 7 ; i >= 0 ; i--) code[i] = offset[i];
			}

			codes.push_back(code);
		}
	}

	void Debug_print_instr(){
		freopen("analysed.txt" , "w" , stdout);
		for(auto x : instr){
			cout<<x.first<<" ";
			printf("%d %d %d -- %d %d %d\n"  , x.second.first.Dir , x.second.first.mode , x.second.first.reg_num ,x.second.second.Dir , x.second.second.mode , x.second.second.reg_num );
		}
	}

	void Debug_print_codes(){
		ofstream cout;
		cout.open("../ram1.dat");
		for(auto x : codes) cout<<x.to_string()<<endl;
		cout.close();
	}

};

