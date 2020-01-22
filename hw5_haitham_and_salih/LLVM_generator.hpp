
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <stdbool.h>
#include "structs.hpp"
#define REG_NUM 3000
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

string boolValToInt(string value){
	return value == "false" ? "0" : "1";
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
	
	
string TokenTypeToLlvmType(TokenType type, Node* node_in_case_type_can_be_function = nullptr){
	
	Function* f = (Function*) node_in_case_type_can_be_function;
	switch(type){
			
			case BOOL_t: 	return "i1"   ; break;
			case INT_t:   	return "i32"	; break;
			//byte to 32
			case BYTE_t:   	return "i32"	; break;
			case VOID_t:   	return "void"	; break;
			case ENUM_t:   	return "i32"	; break;
			case STRING_t:   	return "i8*"	; break;
			case FUNCTION_t: return TokenTypeToLlvmType(f->returnType); break;
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
	
	//basically, use this only after generating a ret op to skip the register it needs
	void regSkip(){
        for(int i=0; i<REG_NUM; i++){
            if(isAvail[i]){
                isAvail[i] = false;
               
                return;
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
	
	
	
    void regDeallocAll(int exept_first_n_regs = 0){
		
		for(int i = 0; i < exept_first_n_regs; i++)
			isAvail[i] = false;
		
        for(int i= exept_first_n_regs ; i<REG_NUM; i++)
            isAvail[i] = true;
        }
    

};











//-------------------------------------------------------------------------------------------------

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
		string str_format = " c\"\%s\" ";
		CodeBuffer::instance().emit(" @.int_specifier = constant [4 x i8]" + int_format  );
		CodeBuffer::instance().emit(" @.str_specifier = constant [2 x i8]" +  str_format  );


		CodeBuffer::instance().emit("  define void @printi(i32) { ");
		CodeBuffer::instance().emit("  call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0), i32 %0) ");
		CodeBuffer::instance().emit("  ret void  } ");

		CodeBuffer::instance().emit("  define void @print(i8*) { ");
		CodeBuffer::instance().emit("  call i32 (i8*, ...) @printf(i8* getelementptr ([2 x i8], [2 x i8]* @.str_specifier, i32 0, i32 0), i8* %0) ");
		CodeBuffer::instance().emit("  ret void } ");
		
		CodeBuffer::instance().emit("  define void @checkdiv(i32) { ");
		CodeBuffer::instance().emit("	%cond = icmp eq i32 %0 , 0" );
		CodeBuffer::instance().emit("	br i1 %cond , label %zero_div , label %good");
		CodeBuffer::instance().emit("zero_div:");
		CodeBuffer::instance().emit("	%str = alloca [ 24 x i8]");
		CodeBuffer::instance().emit("	store [24 x i8] c\"Error division by zero\\0A\\00\" , [24 x i8]* %str");
		CodeBuffer::instance().emit("	%new_str = getelementptr [24 x i8] , [24 x i8]* %str, i64 0, i64 0");
		CodeBuffer::instance().emit("	call void @print(i8* %new_str)");
		CodeBuffer::instance().emit("	call void @exit(i32 0)");
		CodeBuffer::instance().emit("	br label %good");
		CodeBuffer::instance().emit("good:");
		
		CodeBuffer::instance().emit("  ret void  } ");

	}
	
	
	void funcDecGen(string id , string return_type , vector<string> params, vector<Variable*> args){
     
		CodeBuffer::instance().emit("define " +  return_type + " @" + id + "(" +  paramsToString(params)  +   ") {" );
		CodeBuffer::instance().emit(id + ":");//label so we start counting from 0
		CodeBuffer::instance().emit("");//newline
		
		//now save the params on stack in order to allow changing them inside the function
		int size = params.size();
		string reg_i;
		
		clear_regs(size);
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
	
	
	void clear_regs(int exept_first_n_regs = 0){
		
		pool.regDeallocAll(exept_first_n_regs);
	}
	
	
	
	
	string genCall(string fun_id , TokenType fun_retype , vector<TokenType> params , vector<Node*> reg_args){
		
		string ret_reg,tmp_reg, load_str, line , arr_size_inpxels;//helper strings
		
		if (reg_args.size() == 0){
				
			if( fun_retype == VOID_t){
				CodeBuffer::instance().emit("	call " + TokenTypeToLlvmType(fun_retype) + " @" + fun_id + "( )");
				return "";
			}
			else{
				tmp_reg = RegAlloc();
				
				CodeBuffer::instance().emit("	" + tmp_reg + " = call " + TokenTypeToLlvmType(fun_retype) + " @" + fun_id + "( )");
				ret_reg = RegAlloc();
				CodeBuffer::instance().emit("	"+ret_reg + " = alloca " + TokenTypeToLlvmType(fun_retype));
				CodeBuffer::instance().emit("	store " + TokenTypeToLlvmType(fun_retype)+" " + tmp_reg +" , "+TokenTypeToLlvmType(fun_retype)+"* " + ret_reg);
				return ret_reg;
			}
		}
		
		
		vector<Node*> args = vector<Node*>();
		vector<Node*> to_del_args = vector<Node*>();
		Variable* node;
		for(auto i : reg_args){
			if(i->type == ENUM_t){
				i->reg = to_string( get_enum_valued_order( (Enum_var*)  i) );   //notice that we do not need another register for an enum variable, we use it's own variable
				args.push_back(i);
			}
			else if(i->is_Var && i->type != FUNCTION_t){
				tmp_reg =RegAlloc();
				line = TokenTypeToLlvmType(i->type, i)+ "* " + i->reg;
				load_str = tmp_reg + " = " + "load " + TokenTypeToLlvmType(i->type, i) + " , " + line;
				CodeBuffer::instance().emit("	" + load_str);

				node =new Variable(i->type, i->name, 1, i->value, tmp_reg);//we sent offset = 1 cause fuck offsets
				args.push_back(node);
				to_del_args.push_back(node);

					
			}else{
				if (i->type == STRING_t){
					tmp_reg =RegAlloc();
					string str_without_chopticks = i->value.substr(1,i->value.size()-2);
					string arr_size = to_string((i->value).length());
					line = tmp_reg + " = alloca [" + arr_size + " x i8]";
					CodeBuffer::instance().emit("	" + line);
					
					string llvm_str = "c\"" + str_without_chopticks + "\\0A\\00\"";
				 arr_size_inpxels = "["+arr_size  + " x i8]";
					line = "	store "+arr_size_inpxels+" " + llvm_str + " , " + arr_size_inpxels + "* " + tmp_reg; 
					CodeBuffer::instance().emit( line);
					string nd_tmp_reg = RegAlloc();
					CodeBuffer::instance().emit("	" + nd_tmp_reg + " = getelementptr " + arr_size_inpxels+" , " + arr_size_inpxels+"* "+ tmp_reg + ", i64 0, i64 0");
					
					node =new Variable(i->type, i->name, 1, str_without_chopticks, nd_tmp_reg);//we sent offset = 1 cause fuck offsets
					args.push_back(node);
					to_del_args.push_back(node);
					
				}else{
				args.push_back(i);	
				}				
			}			
		}
		
		
		//CodeBuffer::instance().emit("		ret type for "+ TokenTypeToLlvmType(fun_retype));
		if( fun_retype == VOID_t)
			CodeBuffer::instance().emit("	call " + TokenTypeToLlvmType(fun_retype) + " @" + fun_id + "(" + print_args(params,args) + ")");
		else{
			tmp_reg = RegAlloc();
			CodeBuffer::instance().emit("	" + tmp_reg + " = call " + TokenTypeToLlvmType(fun_retype) + " @" + fun_id + "(" + print_args(params,args) + ")");
			ret_reg = RegAlloc();
			CodeBuffer::instance().emit("	" + ret_reg+" = alloca " + TokenTypeToLlvmType(fun_retype));
			CodeBuffer::instance().emit("	store " + TokenTypeToLlvmType(fun_retype)+" " + tmp_reg +" , "+TokenTypeToLlvmType(fun_retype)+"* " + ret_reg);
		}
		
		for(int i = 0; i < to_del_args.size(); ++i)
				delete to_del_args[i];
			
		return ret_reg;
	}
	
	
	
	void genReturn(Function* func , Node* val = nullptr){
		
		TokenType fun_ret_type = func->returnType;
	
	
		if (fun_ret_type == VOID_t){
			CodeBuffer::instance().emit("	ret void");
			
		} else if(val == nullptr){
			
			CodeBuffer::instance().emit("	ret " + TokenTypeToLlvmType(fun_ret_type) + " 0");//return default val
		} else if (val->is_Var){
				if (val->type == FUNCTION_t){
					val->reg = RegAlloc();
				}
				string line = TokenTypeToLlvmType(fun_ret_type) + "* " + val->reg;
				string tmp_reg =RegAlloc();
				string load_str = tmp_reg + " = " + "load " + TokenTypeToLlvmType(fun_ret_type) + " , " + line;
				
				CodeBuffer::instance().emit("	" + load_str);
				
			func->reg = tmp_reg;
			CodeBuffer::instance().emit("	ret " + TokenTypeToLlvmType(fun_ret_type) + " " + tmp_reg);
		}else{
			func->reg = val->value;
			CodeBuffer::instance().emit("	ret " + TokenTypeToLlvmType(fun_ret_type) + " " + val->value);

		}
		
		
		
	//after ret, the block needs to end in order for lvm to compile, so a return inside an if body for example means having more code after it (the br out of the body for example) 
	//simply adding a label won't do because we need to jump to that label, that means more code, 
	//closing the block with '}' also won't because the rest of the code will be outside the function body if so
		pool.regSkip();
		
		
		
	}
	

	
	
	//despite it's name, doesn't really alloc anything
	void Var_Alloc_And_Assign(string r1,TokenType r1_type,Node* r2,vector<Node*>* reg_args = nullptr)
	{
		
		
		//cout<< "r1_type -> " << r1_type << endl;
		string tmp_reg, load_str, line;//helper strings
		
		if(r2->type != FUNCTION_t){
			line = r2->is_Var ? TokenTypeToLlvmType(r2->type, r2)+ "* " + r2->reg   : TokenTypeToLlvmType(r2->type, r2) + " "+ r2->value; 
			
			if(r2->type == ENUM_t){
				if (r2->is_Var){
						if( r2->reg == ""){
							r2->reg = Var_Alloc();
							int ordered_num = get_enum_valued_order( (Enum_var*) r2);	
							CodeBuffer::instance().emit("	store i32 " + to_string(ordered_num) + " , i32*" + " "+r2->reg); 
					

							
						}
						//r2->reg = r2->reg == "" ? RegAlloc() : r2->reg;
						string tmp_reg = RegAlloc();
						CodeBuffer::instance().emit("	"+tmp_reg + " = load i32 , i32* " + r2->reg);
						//r1 = Var_Alloc();
						CodeBuffer::instance().emit("	store i32 " + tmp_reg + " , i32* " + r1);
						return;
					
				}
				else{
					int ordered_num = get_enum_valued_order( (Enum_var*) r2);	
					CodeBuffer::instance().emit("	store i32 " + to_string(ordered_num) + " , i32*" + " "+r1); 
					cout << "error in else in enum in var alloc and assing" << endl;
					
				return;
				}
				
			}
			
			else if(r2->is_Var){
				tmp_reg =RegAlloc();
				load_str = tmp_reg + " = " + "load " + TokenTypeToLlvmType(r2->type, r2) + " , " + line;
				line = TokenTypeToLlvmType(r2->type, r2)+ " " + tmp_reg;
				CodeBuffer::instance().emit("	" + load_str);
			}
				CodeBuffer::instance().emit("	store " + line + " , " + TokenTypeToLlvmType(r1_type, r2) + "* " + r1);
			
		}
		else{
			Function* f = (Function*)r2;
			string fun_id = f->name;
			TokenType ret_type= f->returnType;
			vector<TokenType> params = f->paramTypes;
			
		//	f->reg = genCall(fun_id, ret_type, params, *reg_args);
			CodeBuffer::instance().emit("	store " + TokenTypeToLlvmType(ret_type) + " " + f->reg + " , " + TokenTypeToLlvmType(r1_type) + "* " + r1);
			
		
		}
	
	}
	
	
	

	
	
	
	//--------------------EXP code generating methods-----------------
	
	//returns the reg it updated, deallocs the regs it got as params, the addops dont ignore signed and unsigned
	string genAddOp(Node* r1,Node* r2,string op,TokenType res_type = INT_t){
		
		 //incase signed or not
		 string final_op = res_type == INT_t ? opToCommand(op) : opToCommandU(op);
		
		string r1_save_reg , r2_save_reg;
		string load_line , add_line;
		//cout << "okay" << endl;
		
		r1_save_reg = RegAlloc();
		string type = TokenTypeToLlvmType(r1->type, r1);
		if(r1->is_Var && r1->type != FUNCTION_t)
			load_line = r1_save_reg + " = load " + type + ", "+ type + "* " + r1->reg ;
		else if(r1->type == FUNCTION_t) 
			load_line = r1_save_reg + " = add " + type + " 0 , " + r1->reg ;
		else
			load_line = r1_save_reg + " = add " + type + " 0 , " + r1->value ;
		
		CodeBuffer::instance().emit("	" + load_line);
		//cout << "okay2" << endl;
		
		
		r2_save_reg = RegAlloc();
		type = TokenTypeToLlvmType(r2->type,r2);
		if(r2->is_Var && r1->type != FUNCTION_t)
			load_line = r2_save_reg + " = load " + type + ", "+ type + "* " + r2->reg ;
		else if(r2->type == FUNCTION_t) 
			load_line = r2_save_reg + " = add " + type + " 0 , " + r2->reg ;
		else
			load_line = r2_save_reg + " = add " + type + " 0 , " + r2->value ;
			
		CodeBuffer::instance().emit("	" + load_line);
		
		//cout << "okay3" << endl;
		string res_reg_as_int = RegAlloc();
		
		string res_line_as_int = res_reg_as_int+" = " + final_op + " " +TokenTypeToLlvmType(r1->type,r1) + " " + r1_save_reg + " , " + r2_save_reg;
		
		CodeBuffer::instance().emit("	" + res_line_as_int);
		if(r1->type == BYTE_t && r2->type == BYTE_t){
			string trunc_reg = RegAlloc();
			string zext_reg = RegAlloc();
			CodeBuffer::instance().emit("	" + trunc_reg + " = trunc i32 " + res_reg_as_int + " to i8");
			CodeBuffer::instance().emit("	" + zext_reg + " = zext i8 " + trunc_reg + " to i32");
			res_reg_as_int = zext_reg;//now this reg has the value
			
		}
		//cout << "okay4" << endl;
		string res_reg = RegAlloc();
		CodeBuffer::instance().emit("	" + res_reg + " = alloca " + TokenTypeToLlvmType(r1->type,r1));
		CodeBuffer::instance().emit("	store "+ TokenTypeToLlvmType(r1->type,r1) + " " + res_reg_as_int + " , " + TokenTypeToLlvmType(r1->type,r1)+"* " + res_reg);

	//	cout << "okay5" << endl;
		return res_reg;	
	}
	
	
	
	
	
	
	
	
	
	
	///////////////////// mefne bakarah and bool stuf/////////////////////////////
	
	
	int genIF(Node* BoolExp){
		
		string cond_reg;
		
		if (BoolExp->is_Var){
			cond_reg = RegAlloc();
			CodeBuffer::instance().emit("	" + cond_reg + " = load i1, i1* " + BoolExp->reg);
		}
		else 
			cond_reg = BoolExp->reg;
		
	
		int i1 = CodeBuffer::instance().emit("	br i1 " + cond_reg + " , label @, label @");
		
		return i1;
		
	}
	
	
	
	void CheckDivZero(Node* mkam){
		
		string val_reg = RegAlloc();
		if(mkam->is_Var){
			
			CodeBuffer::instance().emit("	" + val_reg + " = load i32 , " + TokenTypeToLlvmType(mkam->type) + "* " + mkam->reg);
		}else{
			CodeBuffer::instance().emit("	" + val_reg + " = add " + TokenTypeToLlvmType(mkam->type) + " 0 , " + mkam->value);
		}
		
		//Error division by zero
		
		CodeBuffer::instance().emit("	call void @checkdiv(i32 " + val_reg + " )");
		//check this
		
		
	}
	
	
	
	//use this function only if you know the result (true or false) and it is saved in boolExp->val already as string
	void genBool(Node* boolExp,Node* r2 = nullptr, bool isNot = false){
		if(boolExp->is_Var){
			CodeBuffer::instance().emit("	" + boolExp->reg + " = alloca i1" );
			string tmp_reg;
			
			if(isNot && r2 != nullptr){
				string oldReg = RegAlloc();
				CodeBuffer::instance().emit("	"+oldReg + " = load i1 , i1* " + r2->reg );
				tmp_reg = RegAlloc();
				CodeBuffer::instance().emit("	"+tmp_reg + " = add i1 1 , " + oldReg );
				
			}else if (r2 != nullptr) {
				if(r2->is_Var){
				tmp_reg = RegAlloc();
				CodeBuffer::instance().emit("	"+tmp_reg + " = load i1 , i1* " + r2->reg );
				}else{
					tmp_reg = r2->value;
					
				}
			}
			
			if (tmp_reg != "")
				CodeBuffer::instance().emit("	store i1 " + tmp_reg + " , i1* " + boolExp->reg );
			
		}
		else
		CodeBuffer::instance().emit("	" + boolExp->reg + " = add i1 0 , " + boolValToInt(boolExp->value) );
	}
	
	
	string genBoolFromRelOp(Node* E1 , Node* E2, string op, string* resultVal){
		string reg = RegAlloc();
		CodeBuffer::instance().emit("	" + reg + " = alloca i1");
		
		//finding type to know if using signed or unsigned ops
		TokenType res_type = E1->type == BYTE_t && E2->type == BYTE_t ? BYTE_t : INT_t;
		
		string final_op = res_type == INT_t ? opToCommand(op) : opToCommandU(op);
		
		string r1_save_reg = RegAlloc();
		string r2_save_reg = RegAlloc();;
		
		if(E1->is_Var){
			//r1_save_reg = RegAlloc();
			CodeBuffer::instance().emit("	" + r1_save_reg + " = load i32, i32* " + E1->reg);
		}
		if(!(E1->is_Var)){
			CodeBuffer::instance().emit("	" + r1_save_reg + " = add i32  0 , " + E1->value);
			
		}
		if(E2->is_Var){
			//r2_save_reg = RegAlloc();
			CodeBuffer::instance().emit("	" + r2_save_reg + " = load i32, i32* " + E2->reg);
		}
		if(!(E2->is_Var)){
			CodeBuffer::instance().emit("	" + r2_save_reg + " = add i32  0 , " + E2->value);
			
		}
		
		string tmp_reg = RegAlloc();
		CodeBuffer::instance().emit("	" + tmp_reg + " = icmp " + final_op + " " + TokenTypeToLlvmType(res_type) + " "+ r1_save_reg + " , " + r2_save_reg);
		CodeBuffer::instance().emit("	store i1 " + tmp_reg +" , i1* " + reg);
		return reg;
		
		
		//first label is true, second is false
		
		/*
		string trueL = CodeBuffer::instance().genLabel();
		
		CodeBuffer::instance().emit("	" + reg + " = add 0 , 1");
		int i2 = CodeBuffer::instance().emit("br label @");//to label Done
		
		string falseL = CodeBuffer::instance().genLabel();
		
		CodeBuffer::instance().emit("	" + reg + " = add 0 , 0");
		int i3 = CodeBuffer::instance().emit("br label @");//to label Done
		
		string DoneL = CodeBuffer::instance().genLabel();
		
		
		pair<int,BranchLabelIndex> p11 = pair<int,BranchLabelIndex>(i1,FIRST);
		auto listTrue = CodeBuffer::instance().makelist(p11);	
		CodeBuffer::instance().bpatch(listTrue, trueL);		
		
		pair<int,BranchLabelIndex> p12 = pair<int,BranchLabelIndex>(i1,SECOND);
		auto listFalse = CodeBuffer::instance().makelist(p12);
		CodeBuffer::instance().bpatch(listFalse, falseL);	
		
		pair<int,BranchLabelIndex> p2 = pair<int,BranchLabelIndex>(i2,FIRST);
		pair<int,BranchLabelIndex> p3 = pair<int,BranchLabelIndex>(i3,FIRST);
		auto listDone1 = CodeBuffer::instance().makelist(p2);
		auto listDone2 = CodeBuffer::instance().makelist(p3);
		
		auto listDone = CodeBuffer::instance().merge(listDone1,listDone2);
		CodeBuffer::instance().bpatch(listDone, DoneL);
		*/
	}
	
	
	string genExpressionFinalResult(Node* exp, string& TLabel, string& FLabel) {
		
		string trueLabel = CodeBuffer::instance().genLabel();
		int addr1 = CodeBuffer::instance().emit("	br label @");
		string falseLabel = CodeBuffer::instance().genLabel();;
		int addr2 = CodeBuffer::instance().emit("	br label @");
		CodeBuffer::instance().bpatch(exp->truelist, trueLabel);
		CodeBuffer::instance().bpatch(exp->falselist, falseLabel);
		string result_label = CodeBuffer::instance().genLabel();
		CodeBuffer::instance().bpatch(CodeBuffer::instance().makelist({addr1,FIRST}), result_label);
		CodeBuffer::instance().bpatch(CodeBuffer::instance().makelist({addr2,FIRST}), result_label);
		string result_reg = RegAlloc();
		CodeBuffer::instance().emit("	" + result_reg + " = phi i1 [ 0, %"+falseLabel+" ], [ 1, %"+trueLabel+" ]");
		
		//string result_reg_final = RegAlloc();
		//CodeBuffer::instance().emit("	" + result_reg_final + " = zext i1 "+ result_reg+" to i32");
		
		
		TLabel = trueLabel;
		FLabel = falseLabel;
		return result_reg;
}
	
	
	
	//-------------------------------------Enum-stuff ------------------------------//
	

	
	
	
	
};
