#include "bp.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <stdbool.h>
#include "structs.hpp"
#define REG_NUM 100
using namespace std;



//-----------------------------helpers-------------------------------//
string opToCommandU(string op )
{
    if(op == "+")	{ return ("add "); }
    if(op == "-")	{ return ("sub "); }
    if(op == "*")	{ return ("mul "); }
    if(op == "==")	{ return ("eq "); }
    if(op == "!=")	{ return ("ne "); }	
    if(op == "/")	{ return ("udiv "); }
    if(op == "<") 	{ return ("ult "); }
    if(op == ">") 	{ return ("ugt "); }
    if(op == "<=")	{ return ("ule "); } 
    if(op == ">=")	{ return ("uge "); }
	
	cout << "error in opToCommandU" << endl;
	throw -1;
}

string opToCommand(string op )
{
    if(op == "+")	{ return ("add "); }
    if(op == "-")	{ return ("sub "); }
    if(op == "*")	{ return ("mul "); }
    if(op == "==")	{ return ("eq "); }
    if(op == "!=")	{ return ("ne "); }
    if(op == "/")	{ return ("sdiv "); }
    if(op == "<") 	{ return ("slt "); }
    if(op == ">") 	{ return ("sgt "); }
    if(op == "<=")	{ return ("sle "); }
    if(op == ">=")	{ return ("sge "); }
	
	
	cout << "error in opToCommand, op is : " <<  op << endl;
	throw -1;
}




string paramsToString(vector<string> args){
	
	string ret = "";
		
	for(int i=0;i < args.size(); ++i){
			
		if(i!=args.size()-1)
			ret += args[i] + ",";
		else
			ret += args[i];
	}
		
	return ret;
}
	
	
string TokenTypeToLlvmType(TokenType type){
	
	switch(type){
			
			case BOOL_t: 	return "i1"   ; break;
			case INT_t:   	return "i32"	; break;
			case BYTE_t:   	return "i8"	; break;
			case VOID_t:   	return "void"	; break;
			case ENUM_t:   	return "i32"	; break;
			default : cout << "error in TypeToLvmTypes, type is : " << type << endl;
			exit(0);
			break;
	
	}
	
}
	
	
vector<string> TypeToLvmTypes(vector<TokenType> paramTypes){
	
	vector<string> ret = vector<string>();
	
	for (auto i = paramTypes.begin() ; i!= paramTypes.end() ; i++){
		
		ret.push_back( TokenTypeToLlvmType(*i) );
		
	}
	return ret;
	
}	

//assumes all regs in all args have the value, this is done in the father function
string print_args(vector<TokenType> types , vector<Node*> args){
	
		string ret = "";
		string val = "";
		int size = args.size(); // cuz they are reversed
		for(int i = 0 ; i < size ; i++){
			val = args[size - i - 1]->is_Var ? args[size - i - 1]->reg : args[size - i - 1]->value;
			if(i!=args.size()-1){
				
				ret +=  TokenTypeToLlvmType(types[i]) + " " + val + " , ";
			}
			else{
				ret += TokenTypeToLlvmType(types[i]) + " " + val;
			}
		}
	return ret;
}

//TODO: the registers names nmow are "$t0-9" and "$s10-49", check if we need to change to "%t0-49"
class regPool {
public:
    string regs[REG_NUM];
    bool isAvail[REG_NUM];
   
	
	
    regPool(){
        for(int i=0; i<REG_NUM;i++) isAvail[i]=true;
        for(int i=0; i<REG_NUM; i++){
           regs[i] = "%" + to_string(i);
        }
        
    }
	
    string regAlloc(){
        for(int i=0; i<REG_NUM; i++){
            if(isAvail[i]){
                isAvail[i] = false;
               
                return regs[i];
            }
        }
        //return "NO";
        cout << "no more registers available (max regs in use is " << REG_NUM << endl;
        throw -1;
    }
	
	   void regDealloc(string reg){
        for(int i=0; i<REG_NUM; i++){
            if(regs[i]==reg){
                isAvail[i] = true;
                return;
            }
        }
    }
	
	
	
    void regDeallocAll(){
        for(int i=0; i<REG_NUM; i++)
                isAvail[i] = true;
        }
    

};

class Genreator{

public:	
	regPool pool;
	Genreator(){	
		pool = regPool();	
	}
	
	void Gen_Printers(){
		CodeBuffer::instance().emit(" declare i32 @printf(i8*, ...)");
		CodeBuffer::instance().emit(" declare void @exit(i32)  ");

		string int_format = " c\"\%d\\0A\\00\"";
		string str_format = " c\"\%s\\0A\\00\"";
		CodeBuffer::instance().emit(" @.int_specifier = constant [4 x i8]" + int_format  );
		CodeBuffer::instance().emit(" @.str_specifier = constant [4 x i8]" +  str_format  );


		CodeBuffer::instance().emit("  define void @printi(i32) { ");
		CodeBuffer::instance().emit("  call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0), i32 %0) ");
		CodeBuffer::instance().emit("  ret void  } ");

		CodeBuffer::instance().emit("  define void @print(i8*) { ");
		CodeBuffer::instance().emit("  call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0), i8* %0) ");
		CodeBuffer::instance().emit("  ret void } ");

	}
	
	
	void funcDecGen(string id , string return_type , vector<string> params, vector<Variable*> args){
     
		CodeBuffer::instance().emit("define " +  return_type + " @" + id + "(" +  paramsToString(params)  +   ") {" );
		CodeBuffer::instance().emit(id + ":");//label so we start counting from 0
		CodeBuffer::instance().emit("");//newline
		
		//now save the params on stack in order to allow changing them inside the function
		int size = params.size();
		string reg_i;
		for(int i = 0; i < size; ++i){//param i is saved in reg %i at this point
			
			reg_i = Var_Alloc(args[i]->type);
			CodeBuffer::instance().emit("	store " + TokenTypeToLlvmType(args[i]->type) + " %" + to_string(i) + " , " + TokenTypeToLlvmType(args[i]->type) + "* " + reg_i);
			args[i]->reg= reg_i;
		}
	
    }
	
	/*string genNum(int num,TokenType type){
		//check about the 0
        string regName = pool.regAlloc(0);
        CodeBuffer::instance().emit("	"+regName + " = "+ "add " + TokenTypeToLlvmType(type) + " " + "0" + " , " + to_string(num));
        return regName;
    }*/
	
	
	
	//allocates an i32 on stack and returns the label to it, no need to dealloc this because it's on the function stack, used for local vars
	string Var_Alloc(TokenType type = INT_t){
		
		string reg = RegAlloc();
		CodeBuffer::instance().emit("	" + reg + " = " + "alloca " + TokenTypeToLlvmType(type));
		
		
		return reg;
	}
	string RegAlloc(){
		return pool.regAlloc();
	}
	
	
	void clear_regs(){
		
		pool.regDeallocAll();
	}
	
	
	//despite it's name, doesn't really alloc anything
	void Var_Alloc_And_Assign(string r1,TokenType r1_type,Node* r2,vector<Node*>* reg_args = nullptr,Enum_class* tmp = nullptr){
		
		
		//cout<< "r1_type -> " << r1_type << endl;
		string tmp_reg, load_str, line;//helper strings
		
		if(r2->type != FUNCTION_t){
			line = r2->is_Var ? TokenTypeToLlvmType(r2->type)+ "* " + r2->reg   : TokenTypeToLlvmType(r2->type) + " "+ r2->value; 
			
			if(r2->type == ENUM_t){
			int ordered_num = get_enum_valued_order(tmp->enum_vals,r2->value);	
				CodeBuffer::instance().emit("	store i32 " + to_string(ordered_num) + " , i32*" + " "+r1); 
				return;
				
			}
			
			else if(r2->is_Var){
				tmp_reg =RegAlloc();
				load_str = tmp_reg + " = " + "load " + TokenTypeToLlvmType(r2->type) + " , " + line;
				line = TokenTypeToLlvmType(r2->type)+ " " + tmp_reg;
				CodeBuffer::instance().emit("	" + load_str);
			}
				CodeBuffer::instance().emit("	store " + line + " , " + TokenTypeToLlvmType(r1_type) + "* " + r1);
			
		}
		else{
			Function* f = (Function*)r2;
			string fun_id = f->name;
			TokenType ret_type= f->returnType;
			vector<TokenType> params = f->paramTypes;
			vector<Node*> args = vector<Node*>();
			vector<Node*> to_del_args = vector<Node*>();
			
			Variable* node;
			for(auto i : *reg_args){
				
				if(i->is_Var){
					tmp_reg =RegAlloc();
					line = TokenTypeToLlvmType(i->type)+ "* " + i->reg;
					load_str = tmp_reg + " = " + "load " + TokenTypeToLlvmType(i->type) + " , " + line;
					line = TokenTypeToLlvmType(i->type)+ " " + tmp_reg;
					CodeBuffer::instance().emit("	" + load_str);

					node =new Variable(i->type, i->name, 1, i->value, tmp_reg);//we sent offset = 1 cause fuck offsets
					args.push_back(node);
					to_del_args.push_back(node);

					//CodeBuffer::instance().emit("			/////   	i->name = " + i->name + " tmp reg = " + tmp_reg );
				}else{
					args.push_back(i);				
				}			
			}
			
			
			tmp_reg = RegAlloc();
			CodeBuffer::instance().emit("	"+ tmp_reg + " = " + "call " + TokenTypeToLlvmType(ret_type) + " @" + fun_id + "(" + print_args(params,args) + ")");
			CodeBuffer::instance().emit("	store " + TokenTypeToLlvmType(ret_type) + " " + tmp_reg + " , " + TokenTypeToLlvmType(r1_type) + "* " + r1);
			for(int i = 0; i < to_del_args.size(); ++i)
				delete to_del_args[i];
		
		}
	
	
	}
	
	void genReturn(TokenType fun_ret_type , Node* val = nullptr){
		
		if (fun_ret_type == VOID_t){
			CodeBuffer::instance().emit("	ret void");
			
		}else if (val->is_Var){
				if (val->type == FUNCTION_t){
					val->reg = RegAlloc();
				}
				string line = TokenTypeToLlvmType(fun_ret_type) + "* " + val->reg;
				string tmp_reg =RegAlloc();
				string load_str = tmp_reg + " = " + "load " + TokenTypeToLlvmType(fun_ret_type) + " , " + line;
				
				CodeBuffer::instance().emit("	" + load_str);
				
			
			CodeBuffer::instance().emit("	ret " + TokenTypeToLlvmType(fun_ret_type) + " " + tmp_reg);
		}else{
			CodeBuffer::instance().emit("	ret " + TokenTypeToLlvmType(fun_ret_type) + " " + val->value);

		}
	}
	
	void genCall(string fun_id , TokenType fun_retype , vector<TokenType> params , vector<Node*> reg_args , bool flag = true){
		if (flag == false)
			return;
		
		string tmp_reg, load_str, line;//helper strings
		vector<Node*> args = vector<Node*>();
		vector<Node*> to_del_args = vector<Node*>();
		Variable* node;
			for(auto i : reg_args){
				
				if(i->is_Var){
					tmp_reg =RegAlloc();
					line = TokenTypeToLlvmType(i->type)+ "* " + i->reg;
					load_str = tmp_reg + " = " + "load " + TokenTypeToLlvmType(i->type) + " , " + line;
					line = TokenTypeToLlvmType(i->type)+ " " + tmp_reg;
					CodeBuffer::instance().emit("	" + load_str);

					node =new Variable(i->type, i->name, 1, i->value, tmp_reg);//we sent offset = 1 cause fuck offsets
					args.push_back(node);
					to_del_args.push_back(node);

					
				}else{
					args.push_back(i);				
				}			
			}
			
		CodeBuffer::instance().emit("	call " + TokenTypeToLlvmType(fun_retype) + " @" + fun_id + "(" + print_args(params,args) + ")");
		for(int i = 0; i < to_del_args.size(); ++i)
				delete to_del_args[i];
	}
	
	
	
	
	
	
	
	//--------------------EXP code generating methods-----------------
	
	//returns the reg it updated, deallocs the regs it got as params, the addops dont ignore signed and unsigned
	string genAddOp(string r1, string op, string r2, TokenType resType){
		
		string r = pool.regAlloc();
		
		CodeBuffer::instance().emit("	"+ r + " = "+ opToCommand(op)  + " " + TokenTypeToLlvmType(resType) + " " + r1 + " , " + r2 );
		pool.regDealloc(r1);
		pool.regDealloc(r2);
        return r;		
	}
	
	//returns the reg it updated, deallocs the regs it got as params, the addops dont ignore signed and unsigned
	string genMultOp(string r1, string op, string r2, TokenType resType){
		
		
		string lvmOP;
		if(resType == INT_t)
			lvmOP = opToCommand(op);
		else//BYTE_t
			lvmOP = opToCommandU(op);//unsigned

		string r = pool.regAlloc();


		CodeBuffer::instance().emit("	"+ r + " = "+ lvmOP  + " " + TokenTypeToLlvmType(resType) + " " + r1 + " , " + r2 );
		pool.regDealloc(r1);
		pool.regDealloc(r2);
        return r;		
	}
	
	
	
	
	//-------------------------------------Enum-stuff ------------------------------//
	
	int get_enum_valued_order(vector<string> enum_vals , string val){
		
		for(int i = 0 ; i < enum_vals.size() ; i++){
			
			if (enum_vals[i] == val){
				return i;
			}
		}
		
		cout << "Error in get_enum_valued_oreder" << endl;
		exit(0);
	}
	
	void assign_enum_var();
	
	void gen_and_assign_enum_var();
	
	
};
