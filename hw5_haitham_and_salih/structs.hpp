//
// Created by haitham on 12/8/2019.
//

#ifndef COMPI_3_STRUCTS_HPP
#define COMPI_3_STRUCTS_HPP

#include <iostream>
#include <stack>
#include <string>
#include <vector>
#include <assert.h>
#include "hw3_output.hpp"
#include "bp.hpp"
extern int yylineno;


using std::cout;
using std::endl;
using std::stack;
using std::string;
using std::vector;
using namespace output;

class Symbol_Table;



/******************************************************************************/

typedef enum ScopeType_t{
    NORMAL , LOOP , GLOBAL , FUNCTION
} ScopeType;

typedef enum tokentype_t { VOID_t =0,Enumerator_t = 1, INT_t  = 2, ENUM_t = 3, ENUM_CLASS_t = 4,
     BYTE_t = 5, B_t = 6, BOOL_t = 7,
     AND_t = 8,OR_t = 9, NOT_t = 10 
     , TRUE_t = 11, FALSE_t = 12, RETURN_t =13,
     IF_t =14, ELSE_t =15, WHILE_t =16, BREAK_t =17,
     CONTINUE_t =18, SC_t =19,COMMA_t =20,
     LPAREN_t  =21, RPAREN_t =22, LBRACE_t =23,
     RBRACE_t =24, ASSIGN_t =25, COMMENT_t=26, ID_t=27, NUM_t=28, STRING_t=29 ,ADDOPS_t=30, MULTOPS_t =31 ,EQUALOP_t =32, RELATIONOP_t =33, FUNCTION_t  =34, UNDEFYET_t =35} TokenType;




/******************************************************************************/

class Enum_var;
int get_enum_valued_order(Enum_var* var);

/******************************************************************************/
/**
 * The rest of the token structs will inherit from this Node
 */
class Node{

public:
    string name;
    string value;
	string reg;
	bool is_Var; // this is for the dec and assing 
    TokenType type;
	string var;
	int size;
	

    vector<pair<int,BranchLabelIndex>> truelist;
    vector<pair<int,BranchLabelIndex>> falselist;
	vector<pair<int,BranchLabelIndex>> nextlist;
	vector<pair<int,BranchLabelIndex>> breaklist;
	vector<pair<int,BranchLabelIndex>> continuelist;
	
Node( TokenType type, string name = "",string value = "" , string reg = ""):name(name), type(type), value(value), reg(reg) , is_Var(false) {
	truelist = vector<pair<int,BranchLabelIndex>>();
	falselist = vector<pair<int,BranchLabelIndex>>();
	nextlist = vector<pair<int,BranchLabelIndex>>();
	breaklist = vector<pair<int,BranchLabelIndex>>();
	continuelist = vector<pair<int,BranchLabelIndex>>();
};

};
#define YYSTYPE Node*




class Variable : public Node{
public:
    int offset;
	
    Variable(TokenType type , string name = "", int offset = 0 , string value = "", string reg = ""):Node(type, name,value,reg), offset(offset){
		
		is_Var = true;
	};
};

class Enum_class : public Variable{

public:
	vector<string> enum_vals;
	Enum_class(string name) : Variable(ENUM_CLASS_t,name,0,"") , enum_vals() {};
    bool contains(string val);
};



class Enum_var : public Variable {
public:

	string enum_type;
	Enum_class* enum_class;
	Enum_var(TokenType type , Enum_class* enum_class , string name = "" , int offset = 0 , string value = "" , string reg = "")
	: Variable(type,name,offset,value,reg) 
	{
		if(enum_class == nullptr){
			cout << "		Error in Enum_Var cons, containing class is nullptr" << endl;
			exit(0);
		}
		this->enum_class = enum_class;
		enum_type = enum_class->name;
	};
};

class Function : public Variable{

public:
	TokenType returnType;
	vector<TokenType> paramTypes;
	vector<Variable*> params;

	Function(string name , TokenType returnType);
	void addParam(Variable* param);
	void ValidateParameters(vector<Node*>& callerParams,Symbol_Table table);

};



vector<string> TokensToString(vector<TokenType>& vec , vector<Variable*> params);


class Scope{
public:

    stack<Variable*> local_table;
    //ScopeType type; 

    bool isLoop;
    Function* curFunc;
    
    Scope(ScopeType type, Function* parentFunc = nullptr);
    void insertVar(Variable* var);
    Variable* getVar(string gname);
    ~Scope();



};



class Symbol_Table{

public:
    stack<int> offset_stack;
    vector<Scope> scopes_table;

	Symbol_Table() : offset_stack() , scopes_table(){};
    Variable* getVar(string name);
    void openScope(ScopeType type, Function* parentFunc = nullptr );
    void insertVar(Variable* var);
    void insertFunc(Function* f);//assumes f is full
    void closeScope();
	bool CheckIfEnumInGlobalScope(Enum_class* cls);
	void p_sys_stack(vector<Scope> sys);
	bool isThereEnumContains(string enum_val);
	Enum_class* FindEnumContains(string enum_val);
    
};


vector<string> modifiedTokensToString(vector<TokenType>& vec , vector<Variable*> params);

#endif //COMPI_3_STRUCTS_HPP
