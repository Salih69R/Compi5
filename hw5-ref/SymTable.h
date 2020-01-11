#ifndef COMPI3_SYMTABLE_H
#define COMPI3_SYMTABLE_H

#include <string>
#include <vector>
#include <cstddef>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include "hw3_output.hpp"

extern int yylineno;
using namespace std;
using namespace output;

struct Node{};

struct Var{
    string name;
    string type;
    string reg;
    int size;
    int offset;
    bool f;
    vector<int> truelist;
    vector<int> falselist;
    Var(string name = "", string type = "",  int size = 0, int offset = 0){
        this->name = name;
        this->type = type;
        this->reg = "";
        this->offset = offset;
        this->size = size;
        this->f = false;
        this->truelist = vector<int>();
        this->falselist = vector<int>();
    }
};

struct Func : public Var{
    vector<string> args;
    vector<Var> offVar;

    string ret;
    Func(string name = ""):Var(name){f = true;}
    void isArg(vector<string>* test){
        if(test->size() != args.size()){
            errorPrototypeMismatch(yylineno, name, args); exit(0);
        }
        for (int i = 0; i < args.size(); ++i) {
            if(args[i] != test[0][i] && !(test[0][i]=="BYTE" && args[i]=="INT")){
                errorPrototypeMismatch(yylineno, name, args); exit(0);
            }
        }
    }
};
class SymTable{
public:
    vector<Var> vars;
    vector<Func> funcs;
    SymTable* parent;
    Func* function;
    bool isWhile;
    SymTable(SymTable* parent , Func* f = NULL, bool isWhile = false){
        vars = vector<Var>();
        funcs = vector<Func>();
        this->parent = parent;
        this->isWhile = isWhile || (parent != NULL && parent->isWhile);
        if(f != NULL){ this->function = f;}
        else{
            if(this->parent != NULL) {
                this->function = parent->function;
            } else{
                this->function = NULL;
            }
        }
    }
    Var* findV(string name){
        for(int i = 0; i < vars.size();i++){
            Var& v = vars[i];
            if(name.compare(v.name)==0){
                return &v;
            }
        }
        if(parent== NULL){
            for(int i = 0; i < funcs.size();i++){
                Func& f = funcs[i];
                if(name.compare(f.name)==0){
                    return (Var*)(&funcs[i]);
                }
            }
            return NULL;
        }
        return parent->findV(name);
    }
    Func* findF(string name){
        for(int i = 0; i < funcs.size();i++){
            Func& f = funcs[i];
            //cout << f.name << endl;
            if(name.compare(f.name)==0){
                return &f;
            }
        }
        if(parent== NULL) return NULL;
        return parent->findF(name);
    }
    bool insertV(string name, string type, int offset,int size=0){//maybe 0
        if(findV(name) != NULL){
            errorDef(yylineno,name);
            exit(0);
        }
        Var v;
        v.name = name;
        v.type = type;
        v.offset = offset;
        v.size = size;
        vars.push_back(v);
        return true;
    }
    bool insertF(string name, vector<string> args, string ret){
        if(findF(name) != NULL){
            errorDef(yylineno,name);
            exit(0);
        }
        Func f;
        f.name = name;
        f.args = args;
        f.ret = ret;
        funcs.push_back(f);
        return true;
    }
    void printTable(){/*
        endScope();
        if((vars.size()!=0 && funcs.size()!=0)){
            cout << "SERIOUSS BUGGGGGGGGG" << endl;
        }
        if(vars.size()!=0){
            for(int i=0; i<vars.size(); i++){
                printID(vars[i].name,vars[i].offset,vars[i].type);

            }
        }
        else{
            for(int i=0; i<funcs.size(); i++){
                printID(funcs[i].name,0,makeFunctionType(funcs[i].ret,funcs[i].args));
            }
        }*/
    }
};
#define YYSTYPE Var*
#endif //COMPI3_SYMTABLE_H
