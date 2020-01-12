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

typedef enum tokentype_t { VOID_t ,Enumerator_t, INT_t , ENUM_t , ENUM_CLASS_t,
     BYTE_t, B_t, BOOL_t,
     AND_t ,OR_t , NOT_t 
     , TRUE_t , FALSE_t , RETURN_t ,
     IF_t , ELSE_t , WHILE_t , BREAK_t ,
     CONTINUE_t , SC_t ,COMMA_t ,
     LPAREN_t , RPAREN_t, LBRACE_t,
     RBRACE_t , ASSIGN_t , COMMENT_t, ID_t, NUM_t, STRING_t ,ADDOPS_t, MULTOPS_t ,EQUALOP_t, RELATIONOP_t , FUNCTION_t , UNDEFYET_t} TokenType;




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
	

    vector<int> truelist;
    vector<int> falselist;
	
Node( TokenType type, string name = "",string value = "" , string reg = ""):name(name), type(type), value(value), reg(reg) , is_Var(false) {
	truelist = vector<int>();
	falselist = vector<int>();
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
