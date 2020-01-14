#include "structs.hpp"
vector<string> TokensToString(vector<TokenType>& vec , vector<Variable*> params){

    Enum_var* tmp ;
    vector<string> ret = vector<string>();
    for ( int i = 0; i < vec.size() ; i++)
    {
        switch (vec[i]){

            case INT_t : ret.push_back("INT_t"); break;
            case ENUM_t :
                tmp = (Enum_var*)params[i];
                ret.push_back(tmp->enum_type); break;
            case VOID_t : ret.push_back("VOID_t"); break;
            case BOOL_t : ret.push_back("BOOL_t"); break;
            case BYTE_t : ret.push_back("BYTE_t"); break;
            case B_t : ret.push_back("B_t"); break;
            case STRING_t : ret.push_back("STRING_t"); break;
            default : cout << "shouldnt be here - unexpected token in  TokensToString, in i = " << i << endl;
                cout << "wrong type is" << vec[i] << endl;
                exit(0);

                break;
        }
    }

    return ret;
}

vector<string> modifiedTokensToString(vector<TokenType>& vec , vector<Variable*> params){
    vector<string> ret = vector<string>();
    Enum_var* tmp ;
    for ( int i = 0; i < vec.size() ; i++)
    {
        switch (vec[i]){

            case INT_t : ret.push_back("INT"); break;
            case ENUM_t :
                tmp = (Enum_var*)params[i];
                ret.push_back(tmp->enum_type); break;
            case VOID_t : ret.push_back("VOID"); break;
            case BOOL_t : ret.push_back("BOOL"); break;
            case BYTE_t : ret.push_back("BYTE"); break;
            case B_t : ret.push_back("B"); break;
            case STRING_t : ret.push_back("STRING"); break;
            default : cout << "shouldnt be here - unexpected token in  modifedTokensToString, in i = " << i << endl;
                cout << "wrong type is" << vec[i] << endl;
                exit(0);

                break;
        }
    }

    return ret;
}



	int get_enum_valued_order(Enum_var* var){
		
		Enum_class* Eclass = var->enum_class;
		
		for(int i = 0 ; i < Eclass->enum_vals.size() ; i++){
			
			if (Eclass->enum_vals[i] == var->value){
				return i;
			}
		}
		
	//	cout << "Error in get_enum_valued_order" << endl;
	//	cout << "var->name = " << var->name << "		var->value = " << var->value <<endl;
	//	cout << "treated as default, returned 0" << endl;
		return 0;
	}



void p_stack(stack<Variable*> st){

    stack<Variable*> cpy = stack<Variable*>();
    Variable* tmp;

    cout <<  "the size is " << st.size() << ",lineno is " << yylineno << endl;
    cout <<  "             " << "this scope has:" << endl;
    while(!st.empty()){


		if (!st.top())
			cout << " in p_stack : found nullptr in stack" << endl;

        string var = st.top()->name == "" ? "empty string " : st.top()->name;
        cout << "            var name is: " << var << "          , with offset: " << st.top()->offset << endl;
        tmp = st.top();
        st.pop();
        cpy.push(tmp);
    }

    cout << "             *****               " << endl;
    while(!cpy.empty()){

        tmp = cpy.top();
        cpy.pop();
        st.push(tmp);
    }

}

void Symbol_Table::p_sys_stack(vector<Scope> sys) {

    for (auto i = sys.begin(); i != sys.end(); ++i) {
      //  cout << "       *******        " << endl;
        p_stack((*i).local_table);
       // cout << "       *******        " << endl;
    }
}



    Function::Function(string name , TokenType returnType): Variable(FUNCTION_t, name), returnType(returnType) {
		paramTypes = vector<TokenType>();
	}

    void Function::addParam(Variable* param) {

        if(name == param->name)
        {
            errorDef(yylineno,param->name);
            exit(0);
        }
        for (auto var : params)
        {

            if(var->name == param->name){
                errorDef(yylineno, param->name);
                exit(0);
            }
        }


		paramTypes.push_back(param->type);
        param->offset = -1 * (params.size() + 1);

		params.push_back(param);
	}


	//TODO: remove this
	string typeListToString(const std::vector<string>& argTypes);


	void Function::ValidateParameters(vector<Node*>& callerParams , Symbol_Table table) {

		/*
		cout << "		validating params of func " << this->name << endl;
		for (int j = 0; j < callerParams.size(); j++) {
			cout << "      callerParams[" << j << "] = " << callerParams[j]->type;
			cout << "      paramTypes[" << j << "] = " << paramTypes[j] << endl;
		}
		*/

		if (callerParams.size() != paramTypes.size()) {
			//cout << " callerParams.size() = " << callerParams.size() << "		paramTypes.size() = " << paramTypes.size() << endl;
			
            auto tmp = modifiedTokensToString(paramTypes,params);
            errorPrototypeMismatch(yylineno, name, tmp);
			exit(0);
		}

		for (int i = 0; i < paramTypes.size(); i++) {

			

			//need to check if this callerParam is a call
			if(  (callerParams)[paramTypes.size() - i - 1]->type == FUNCTION_t  ){
				Function* f = (Function*)(callerParams)[paramTypes.size() - i - 1];
				if( f->returnType !=  paramTypes[i]  &&     !( f->returnType == BYTE_t && paramTypes[i] == INT_t) ){
				
					//cout << "f->returnType = " << f->returnType << endl;
					auto tmp = modifiedTokensToString(paramTypes,params);
					errorPrototypeMismatch(yylineno, name, tmp );
					exit(0);
				}
			}
			else if(!(paramTypes[i] == (callerParams)[paramTypes.size() - i - 1]->type) && !((callerParams)[paramTypes.size() - i - 1]->type == BYTE_t && paramTypes[i] == INT_t)){


			//	cout << "(callerParams)[paramTypes.size() - i - 1]->type) = " <<(callerParams)[paramTypes.size() - i - 1]->type << endl;
                auto tmp = modifiedTokensToString(paramTypes,params);
                errorPrototypeMismatch(yylineno, name, tmp );
				exit(0);
            }

			
			
            if (paramTypes[i] == ENUM_t){

		//		for (int j = 0; j < callerParams.size(); j++) {
		//			cout << "      callerParams[" << paramTypes.size() - j - 1 << "] = " << callerParams[paramTypes.size() - j - 1]->type;
			//		cout << "      paramTypes[" << j << "] = " << paramTypes[j] << endl;
			//	}
				Enum_var* var = (Enum_var*)params[i];
				Enum_var* caller_var = (Enum_var*)callerParams[paramTypes.size() - i - 1];
					
				if (caller_var->type != ENUM_t) {
					auto tmp = modifiedTokensToString(paramTypes, params);
					errorPrototypeMismatch(yylineno, name, tmp);
					exit(0);
				}


			//	cout << "hii" << endl;
                    
                   
				Enum_class* tmp = (Enum_class*)table.getVar(var->enum_type);
				
			//	cout << "\t hii 2" << endl;


                Enum_class* caller_tmp = (Enum_class*)table.getVar(caller_var->enum_type);
				//cout << "\t hii 3" << endl;
				
					//cout <<  "var->name = " << var->name << "  tmp->name = " << tmp->name << "(callerParams)[paramTypes.size() - i - 1]->name = " << (callerParams)[paramTypes.size() - i - 1]->name << endl;
                    if (!tmp->contains((callerParams)[paramTypes.size() - i - 1]->name) && caller_tmp->name != tmp->name){
                         auto tmp = modifiedTokensToString(paramTypes,params);
                         errorPrototypeMismatch(yylineno, name, tmp );
                        exit(0);
                    }

					//cout << "\t\t bye" << endl;

            }
        }
	}




Scope::Scope(ScopeType type,  Function* parentFunc ):  curFunc(parentFunc) 
{


         if (type == GLOBAL ){
            curFunc = nullptr;
            isLoop = false;
            local_table = stack<Variable*>();
            return ;
        }


        if(type == FUNCTION && parentFunc == nullptr){
            cout << "error: in Scope Constructor, must give parent function for scope of type function, you fucker" << endl;
        }
        isLoop = (type == LOOP);
        local_table = stack<Variable*>();
    
    
}
        
       void Scope::insertVar(Variable* var){
      //     cout << "adding " << var->name << " to the local table" << endl;
       local_table.push(var);
          // p_stack(local_table);
    }


Variable* Scope::getVar(string gname){
    stack<Variable*> cpy_stack = stack<Variable*>();
    Variable *curVar, *foundVar = nullptr;


        while(!local_table.empty()){
            if(local_table.top()->name == gname){
                foundVar = local_table.top();
                break;
            }
            curVar = local_table.top();

            local_table.pop();
            cpy_stack.push(curVar);
        }


         while (!cpy_stack.empty()){
            curVar = cpy_stack.top();

            local_table.push(curVar);
            cpy_stack.pop();
         }
         return foundVar;
    }

    Scope::~Scope(){

        while(!local_table.empty()){

            local_table.pop();
        }
    }

bool Symbol_Table::CheckIfEnumInGlobalScope(Enum_class* cls){



    return (scopes_table[0].getVar(cls->name) != nullptr);


}

bool Enum_class::contains(string val){

    for (auto i = enum_vals.begin(); i != enum_vals.end() ; ++i) {
		if (*i == val){
          return true;
        }
    }

    return false;
}



  Variable* Symbol_Table::getVar(string name){

        //stack<Scope> cpy_stack = stack<Scope>();
        //cout << "size is ======" << scopes_table.size()<<endl;
      //cout << "top is " << scopes_table[scopes_table.size()-1 < 0 ? 0 : scopes_table.size()-1].local_table.top()->name<<endl;
        for(int i = (scopes_table.size()-1 < 0 ? 0 : scopes_table.size()-1); i >=0; i--){


            auto cur = scopes_table[i].getVar(name);
            if(cur != nullptr)
                return cur;
        }
        return nullptr;
    }


    void Symbol_Table::openScope(ScopeType type, Function* parentFunc){


        Scope curScope = Scope(type,parentFunc );

        if(parentFunc == nullptr && type != GLOBAL){
            //also for normal
            curScope.curFunc = scopes_table[(scopes_table.size()-1 < 0 ? 0 :scopes_table.size()-1)].curFunc;//get cur function from father scope
         }
         if(type == NORMAL){
             curScope.isLoop = scopes_table[(scopes_table.size()-1 < 0 ? 0 :scopes_table.size()-1)].isLoop;
         }


        scopes_table.push_back(curScope);

        offset_stack.push(offset_stack.empty() ? 0 : offset_stack.top());


		//p_sys_stack(scopes_table);



    }

    void Symbol_Table::insertVar(Variable* var){

        scopes_table[(scopes_table.size()-1 < 0 ? 0 :scopes_table.size()-1)].insertVar(var);

        if(var->type == ENUM_CLASS_t){
            return;
        }
        int last_offset = offset_stack.top(); 
        offset_stack.pop(); 
        offset_stack.push(1 + last_offset); 

        var->offset = last_offset;

       // cout << "stack after inserting var" << endl;
        //p_stack(scopes_table[(scopes_table.size()-1 < 0 ? 0 :scopes_table.size()-1)].local_table);
    }


    void Symbol_Table::insertFunc(Function* f) {

       // cout << "adding fun : " << f->name << endl;
        scopes_table[(scopes_table.size()-1 < 0 ? 0 :scopes_table.size()-1)].insertVar(f);



        openScope(FUNCTION, f);//no need to open scope in parser
        for (int i = 0; i < f->paramTypes.size(); i++)
        {
            scopes_table[(scopes_table.size()-1 < 0 ? 0 :scopes_table.size()-1)].insertVar(f->params[i]);
        }


      //  cout << "stack after inserting fun" << endl;
        //p_stack(scopes_table[(scopes_table.size()-1 < 0 ? 0 :scopes_table.size()-1)].local_table);


	}


    void Symbol_Table::closeScope(){


      //  endScope();
        stack<Variable*> st = scopes_table[scopes_table.size() -1 < 0 ? 0 : scopes_table.size()-1].local_table;
        stack<Variable*> cpy = stack<Variable*>();
        stack<Enum_class*> enums = stack<Enum_class*>();
        Variable* tmp;
        while(!st.empty()){

            tmp = st.top();
            if(tmp->type == ENUM_CLASS_t){
                enums.push((Enum_class*)tmp);
                st.pop();
                continue;
            }
            st.pop();
            cpy.push(tmp);
        }

        //now we have the reversed stack
        Enum_var* tmp_var;
        Enum_class* tmp_class;
        Function* tmp_function;
        string func_type_string;
        string rettype_string;
        vector<string> string_vec= vector<string>();

        vector<TokenType> tokentype_vec= vector<TokenType>();
        string enum_name;
//       while(!cpy.empty()){
//
//            Variable* var = cpy.top();
//            switch (var->type){
//                case INT_t : printID(var->name, var->offset, "INT"); break;
//                case BOOL_t : printID(var->name, var->offset, "BOOL"); break;
//                case BYTE_t : printID(var->name, var->offset, "BYTE"); break;
//                case ENUM_t :
//                    tmp_var = (Enum_var*)var;
//                    printID(var->name, var->offset,tmp_var->enum_type ); break;
//
//                case ENUM_CLASS_t:cout << "erroe" << endl;break;
//                   // tmp_class = (Enum_class*)var;
//                   // enum_name = tmp_class->name.substr(5, tmp_class->name.size()-1);
//                    //printEnumType(enum_name, tmp_class->enum_vals); break;
//
//
//                case FUNCTION_t:
//                    tmp_function = (Function*)var;
//
//                    tokentype_vec.push_back(tmp_function->returnType);
//                    rettype_string = modifiedTokensToString(tokentype_vec,tmp_function->params)[0];
//                    tokentype_vec.pop_back();
//                    string_vec = ( modifiedTokensToString(tmp_function->paramTypes,tmp_function->params) );
//                    func_type_string = makeFunctionType( rettype_string,string_vec);
//
//                    printID(tmp_function->name,0,func_type_string);break;
//                case STRING_t: cout << "i dont know this"<< endl; break;
//                default : "problem in ending scope" ; break;
//
//
//            }
//            cpy.pop();
//        }
//
//        while(!enums.empty()){
//
//            if(enums.top()->type != ENUM_CLASS_t){
//                cout << "error in enums while"<<endl;
//
//            }
//             Enum_class* enum_tmp = enums.top();
//
//            printEnumType(enum_tmp->name.substr(5, enum_tmp->name.size()-1), enum_tmp->enum_vals);
//                enums.pop();
//        }


        scopes_table.pop_back();
        offset_stack.pop();


    }



bool isEnumInScope(Scope sc , string enum_val){

    stack<Variable*> cpy = stack<Variable*>();
    Variable* tmp;
    Enum_class* tmp_class;
    bool found = false;

    while(!sc.local_table.empty()){

        if (sc.local_table.top()->type == ENUM_CLASS_t){

            tmp_class = (Enum_class*)sc.local_table.top();
            if (tmp_class->contains(enum_val)){
                found = true;
                break;
            }
        }

        tmp = sc.local_table.top();
        sc.local_table.pop();
        cpy.push(tmp);
    }

    while(!cpy.empty()){

        tmp = cpy.top();
        cpy.pop();
        sc.local_table.push(tmp);
    }

    return found;

}


bool Symbol_Table::isThereEnumContains(string enum_val){

	

    for (auto i= scopes_table.begin(); i != scopes_table.end()  ; i++) {

        if(isEnumInScope(*i,enum_val)){
            return true;
        }
		
    }
    return false;


}





Enum_class* FindEnumInScope(Scope sc, string enum_val) {

	stack<Variable*> cpy = stack<Variable*>();
	Variable* tmp;
	Enum_class* tmp_class;
	Enum_class* found = nullptr;

	while (!sc.local_table.empty()) {

		if (sc.local_table.top()->type == ENUM_CLASS_t) {

			tmp_class = (Enum_class*)sc.local_table.top();
			if (tmp_class->contains(enum_val)) {
				found = tmp_class;
				break;
			}
		}

		tmp = sc.local_table.top();
		sc.local_table.pop();
		cpy.push(tmp);
	}

	while (!cpy.empty()) {

		tmp = cpy.top();
		cpy.pop();
		sc.local_table.push(tmp);
	}

	return found;

}




Enum_class* Symbol_Table::FindEnumContains(string enum_val) {

	Enum_class* tmp = nullptr;


	for (auto i = scopes_table.begin(); i != scopes_table.end(); i++) {

		if (isEnumInScope(*i, enum_val)) {
			return FindEnumInScope(*i, enum_val);
		}

	}
	return nullptr;


}





