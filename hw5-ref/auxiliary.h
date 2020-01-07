//
// Created by najee on 22/06/2018.
//

#ifndef COMPI_HW5_AUXILARY_H
#define COMPI_HW5_AUXILARY_H

#include "bp.hpp"
#include "SymTable.h"
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <set>
#include <stdbool.h>
#define REG_NUM 18
using namespace std;


string opToCommand(string op )
{
    if(op == "+")	{ return ("add "); }
    if(op == "-")	{ return ("sub "); }
    if(op == "*")	{ return ("mul "); }
    if(op == "/")	{ return ("div "); }
    if(op == "==")	{ return ("beq "); }
    if(op == "!=")	{ return ("bne "); }
    if(op == "<") 	{ return ("blt "); }
    if(op == ">") 	{ return ("bgt "); }
    if(op == "<=")	{ return ("ble "); }
    if(op == ">=")	{ return ("bge "); }
}

string opToCommandU(string op )
{
    if(op == "+")	{ return ("addu "); }
    if(op == "-")	{ return ("subu "); }
    if(op == "*")	{ return ("mul "); }
    if(op == "/")	{ return ("div "); }
    if(op == "==")	{ return ("beq "); }
    if(op == "!=")	{ return ("bne "); }
    if(op == "<") 	{ return ("blt "); }
    if(op == ">") 	{ return ("bgt "); }
    if(op == "<=")	{ return ("ble "); }
    if(op == ">=")	{ return ("bge "); }
}
string itos1(int num){
    ostringstream convert;
    convert << num;
    string res(convert.str());
    return res;
}
int emit(string s){
    return CodeBuffer::instance().emit(s);
}
void emitData(string s){
    CodeBuffer::instance().emitData(s);
}
class regPool {
public:
    string regs[REG_NUM];
    bool isAvail[REG_NUM];
    vector<string> allocated;
    regPool(){
        for(int i=0; i<REG_NUM;i++) isAvail[i]=true;
        for(int i=0; i<REG_NUM; i++){
            if(i>=10) regs[i] = "$s" + itos1(i-10);
            else regs[i] = "$t" + itos1(i);
        }
        allocated = vector<string>();
    }
    string regAlloc(){
        for(int i=0; i<REG_NUM; i++){
            if(isAvail[i]){
                isAvail[i] = false;
                allocated.push_back(regs[i]);
                return regs[i];
            }
        }
        //return "NO";
        cout << "you fucked up" << endl;
        throw ;
    }
    void regDealloc(string reg){
        for(int i=0; i<REG_NUM; i++){
            if(regs[i]==reg){
                isAvail[i] = true;
                allocated.pop_back();
                return;
            }
        }
    }

};

class Gen{
    regPool p;
    int strs;
    vector<int> frames;
public:
    Gen(){
        strs = 0;
        p = regPool();
        frames = vector<int>();
        emitData("divErr: .asciiz \"Error division by zero\\n\"");
        emitData("arrErr: .asciiz \"Error index out of bounds\\n\"");
    }
    string genNum(int num){
        string regName = p.regAlloc();
        emit("li " + regName + "," + itos1(num));
        return regName;
    }
    void openGen(){
        frames.push_back(0);
    }
    void closeGen(){
        emit("add $sp,$sp,"+itos1(4*frames.back()));
        frames.pop_back();
    }
    void breakGen(){
        emit("add $sp,$sp,"+itos1(4*frames.back()));

    }
    void funcDecGen(string id){
        if(id!="main") emit(".globl " + id + "1");
        else emit(".globl " + id);
        if(id!="main") emit(".ent " + id + "1");
        else emit(".ent " + id);
        //if(id == "main"){
        if(id!= "main" )emit(id + "1:");
        else emit(id + ":");
        emit("move $fp, $sp");

        //else emit(id + ":");
    }
    void funcEndGen(string id){
        emit("jr $ra");
        emit(".end " + id + "1");
    }
    void callSetupGen(int argsNum){
        for(vector<string>::iterator i = p.allocated.begin(); i!=p.allocated.end(); i++){
            emit("sub $sp,$sp,4");
            emit("sw ," + *i + ",($sp)");
        }
        //cout << "setup size:" << p.allocated.size() << endl;
        emit("sub $sp,$sp,4");
        emit("sw $fp,($sp)");
        emit("sub $sp,$sp,4");
        emit("sw $ra,($sp)");
        emit("sub $sp,$sp," + itos1(argsNum*4));
    }
    void pushNumArgGen(int offset, string r, int size, int offsetOriginal){
        /*if(size>0){
            for(int i=0; i<size; i++) emit("sw "+r+","+itos1((offset+i)*4)+"($sp)");
        }*/
        //else
        //cout << "here pushing register " << r << " with offset " << offset << endl;
        if(size>0){
            string r2 = p.regAlloc();
            for(int i=0; i<size; i++){
                if(offsetOriginal<0){
                    emit("lw " + r2 + "," + "(" + r + ")");
                    emit("add " + r + "," + r + ",4");
                } //emit("lw " + r2 + "," + itos1((size-1+i)*4*-1) +"(" + r + ")" );
                else{
                    emit("lw " + r2 + "," + "(" + r + ")");
                    emit("sub " + r + "," + r + ",4");
                } //emit("lw " + r2 + "," + itos1((size-1+i)*4) +"(" + r + ")");
                emit("sw " + r2 + ","+itos1((-1*(offset+size)+i)*4)+"($sp)");
            }
            p.regDealloc(r2);
        }
        else{
            emit("sw "+r+","+itos1(-offset*4-4)+"($sp)");
        }
        p.regDealloc(r);
    }
    void pushBoolArgGen(int offset, vector<int>& truelist, vector<int>& falselist){
        //cout << "here pushing with offset " << offset << endl;
        CodeBuffer::instance().bpatch(falselist,CodeBuffer::instance().genLabel());
        emit("sw $0,"+itos1(-offset*4-4)+"($sp)");
        int line = emit("j ");
        CodeBuffer::instance().bpatch(truelist, CodeBuffer::instance().genLabel());
        string r = p.regAlloc();
        emit("li " + r + ",1");
        emit("sw " + r + ","+itos1(-offset*4-4)+"($sp)");
        CodeBuffer::instance().bpatch(CodeBuffer::instance().makelist(line),CodeBuffer::instance().genLabel());
        p.regDealloc(r);
    }
    void callJumpDestroyGen(string name, int argsNum){
        if(name != "print" && name != "printi") emit("jal "+name+ "1");
        else emit("jal " +name );
        emit("add $sp,$sp," + itos1(argsNum*4));
        emit("lw $ra, ($sp)");
        emit("add $sp,$sp,4");
        emit("lw $fp, ($sp)");
        emit("add $sp,$sp,4");
        int tmpSize = p.allocated.size();
        tmpSize*=4;
        //cout << "destroy size:" << p.allocated.size() << endl;
        for(vector<string>::iterator i = p.allocated.begin(); i!=p.allocated.end(); i++){
            emit("lw " + *i + "," + itos1(tmpSize-4) + "($sp)");
            tmpSize-=4;
        }
        emit("add $sp,$sp," + itos1(p.allocated.size()*4));
    }
    string arithmeticGen(string r1, string op, string r2, bool isInt){
        if(op=="/"){
            int missing = emit("bne $0,"+r2+",");
            emit("la "+r1+",divErr");
            emit("sub $sp,$sp,4");
            emit("sw "+r1+",($sp)");
            emit("jal print");
            emit("li $v0,10");
            emit("syscall");
            CodeBuffer::instance().bpatch(CodeBuffer::instance().makelist(missing),CodeBuffer::instance().genLabel());
            if(isInt) emit(opToCommand(op) + r1 + "," + r1 + "," + r2);
            else emit(opToCommandU(op) + r1 + "," + r1 + "," + r2);
        }
        else{
            if(isInt) emit(opToCommand(op) + r1 + "," + r1 + "," + r2);
            else emit(opToCommandU(op) + r1 + "," + r1 + "," + r2);
        }

        if(!isInt) emit("and " + r1 + ",255");
        p.regDealloc(r2);
        return r1;
    }
    string strGen(string str){
        string r = p.regAlloc();
        emitData("str" + itos1(strs) + ": .asciiz " + str + "");
        emit("la " + r + "," + "str" + itos1(strs));
        strs++;
        return r;
    }
    void listGen(vector<int>& list){
        list=CodeBuffer::instance().merge(list,CodeBuffer::instance().makelist(CodeBuffer::instance().emit("j ")));
    }
    void relGen(string r1, string op, string r2, vector<int>& truelist, vector<int>& falselist){
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //check if there is a reason to do merge instead of creating immediatley
        try {
            truelist = CodeBuffer::instance().merge(truelist, CodeBuffer::instance().makelist(emit(opToCommand(op) + r1 + "," + r2 + ",")));
        } catch (...){
            //cout << "1" << endl;
        }
        try{
            falselist = CodeBuffer::instance().merge(falselist,CodeBuffer::instance().makelist(emit("j ")));
        }
        catch (...){
            //cout << "2" << endl;
        }
        try{
            p.regDealloc(r1);
        }
        catch (...){
            //cout << "3" << endl;
        }
        try{
            p.regDealloc(r2);
        }
        catch (...){
            //cout << "4" << endl;
        }
    }
    void prtiGen(){
        emit(".globl printi");
        emit(".ent printi");
        emit("printi:");
        emit("lw $a0,0($sp)");
        emit("li $v0,1");
        emit("syscall");
        emit("jr $ra");
        emit(".end printi");
    }

    void prtGen(){
        emit(".globl print");
        emit(".ent print");
        emit("print:");
        emit("lw $a0,0($sp)");
        emit("li $v0,4");
        emit("syscall");
        emit("jr $ra");
        emit(".end print");
    }
    // empty register for non-return functions
    void retGen(string r){
        if(r!=""){
            emit("move $v0,"+r);
            p.regDealloc(r);
        }
        emit("move $sp,$fp");
        emit("jr $ra");

    }
    void retBoolGen(vector<int> truelist, vector<int> falselist){
        CodeBuffer::instance().bpatch(truelist,CodeBuffer::instance().genLabel());
        emit("li $v0,1");
        int l = emit("j ");
        CodeBuffer::instance().bpatch(falselist,CodeBuffer::instance().genLabel());
        emit("li $v0,0");
        CodeBuffer::instance().bpatch(CodeBuffer::instance().makelist(l),CodeBuffer::instance().genLabel());
        CodeBuffer::instance().emit("move $sp,$fp");
        emit("jr $ra");
    }
    void retMGen(){
        emit("li $v0,10");
        emit("syscall");
    }
    string callNumExpGen(){
        string r = p.regAlloc();
        emit("move " + r + ",$v0");
        //cout << "here at numExpGen returning register: " << r << endl;
        return r;
    }
    void callBoolExpGen(vector<int>& truelist, vector<int>& falselist){
        //cout << "here at boolExpGen" << endl;
        falselist=CodeBuffer::instance().merge(CodeBuffer::instance().makelist(emit("beq $v0,$0,")),falselist);
        truelist=CodeBuffer::instance().merge(CodeBuffer::instance().makelist(emit("j ")),truelist);
    }
    void idCreatGen(int size){
        //cout << "here at idCreatGen with size: " << size << endl;
        for(int i=0; i<size; i++){
            emit("sub $sp,$sp,4");
            emit("sw $0,0($sp)");
        }
        frames.back() += size;
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //make sure yotam sent parameters correctly
    void idCreatNGen(int size, string r, string op="-", bool isArray=false){
        if(isArray){
            for(int i=0; i<size; i++){
                emit("sub $sp,$sp,4");
                emit("sw (" + r + "),0($sp)");
                emit(opToCommand(op)+ r + "," + r + ",4");
            }
            frames.back() += size;
        }
        else{
            emit("sub $sp,$sp,4");
            emit("sw "+r+",0($sp)");
            frames.back() += 1;
        }
        p.regDealloc(r);
    }
    void limitsCheckGen(string r, int size){
        int missing = emit("bge "+r+",$0,");
        emit("la "+r+",arrErr");
        emit("sub $sp,$sp,4");
        emit("sw "+r+",0($sp)");
        emit("jal print");
        emit("li $v0,10");
        emit("syscall");
        CodeBuffer::instance().bpatch(CodeBuffer::instance().makelist(missing),CodeBuffer::instance().genLabel());
        emit("sub " + r + "," + r + "," + itos1(size));
        missing = emit("blt " + r + ",$0,");
        emit("la "+r+",arrErr");
        emit("sub $sp,$sp,4");
        emit("sw "+r+",0($sp)");
        emit("jal print");
        emit("li $v0,10");
        emit("syscall");
        CodeBuffer::instance().bpatch(CodeBuffer::instance().makelist(missing),CodeBuffer::instance().genLabel());
        emit("add " + r +"," + r + "," + itos1(size));
    }
    string idGetGen(int offset, bool byValue=true, int size=0, string idx=""){
        if(size>0 && idx!=""){
            limitsCheckGen(idx,size);
        }
        string r=p.regAlloc();
        emit("move "+r+",$fp");
        if(size>0) {
            if(offset<0) emit("add " + r + "," + r + "," + itos1((offset+size)*4*(-1)));
            else emit("sub " + r + "," + r + "," + itos1((offset+1)*4));
            if(idx!="") emit("mul " + idx + "," + idx + ",4");
            if (offset < 0 && idx!="") emit("add " + r + "," + r + "," + idx);
            if (offset >=0 && idx!="") emit("sub " + r + "," + r + "," + idx);
        }
        else{
            if(offset<0) emit("add " + r + "," + r + "," + itos1((offset+1)*4*(-1)));
            else emit("sub " + r + "," + r + ","  + itos1((offset+1)*4));
        }
        if(byValue) emit("lw " + r + ",(" + r + ")");
        if(idx!="") p.regDealloc(idx);
        return r;
    }

    void idAssignNGen(int size, string src ,string dest,  int offsetDst=0, int offsetSrc=0, bool isArray=false){
        if(isArray){
            string tmp = p.regAlloc();
            for(int i=0; i<size; i++){
                emit("lw " + tmp + ",(" + src +")");
                emit("sw " + tmp + ",(" + dest +")");
                if(offsetSrc<0) emit(opToCommand("+")+ src + "," + src + ",4");
                else emit(opToCommand("-")+ src + "," + src + ",4");
                if(offsetDst<0) emit(opToCommand("+")+ dest + "," + dest + ",4");
                else emit(opToCommand("-")+ dest + "," + dest + ",4");
            }
            p.regDealloc(tmp);
        }
        else{
            emit("sw "+src+",(" + dest + ")");
        }
        p.regDealloc(src);
        p.regDealloc(dest);
    }
    void idCreatBGen(vector<int>& trueList,vector<int>& falseList){
        frames.back()+=1;
        string r = p.regAlloc();
        string l = CodeBuffer::instance().genLabel();
        CodeBuffer::instance().bpatch(trueList,l);
        emit("sub $sp,$sp,4");
        emit("li "+r+",1");
        emit("sw "+r+",($sp)");
        int line = emit("j ");
        l = CodeBuffer::instance().genLabel();
        CodeBuffer::instance().bpatch(falseList,l);
        emit("sub $sp,$sp,4");
        emit("sw $0,($sp)");
        l = CodeBuffer::instance().genLabel();
        CodeBuffer::instance().bpatch(CodeBuffer::instance().makelist(line),l);
        p.regDealloc(r);
    }
    void idAssignBGen(int offset, vector<int>& trueList,vector<int>& falseList, int size=0, string idx="") {
        if(size>0){
            limitsCheckGen(idx,size);
        }
        string r = p.regAlloc();
        string r2 = p.regAlloc();

        string l = CodeBuffer::instance().genLabel();
        //if(size>0) emit("mul " + idx + "," + idx + ",4");

        CodeBuffer::instance().bpatch(trueList,l);
        emit("move " + r + ",$fp");
        if(size>0) {
            if(offset<0) emit("add " + r + "," + r + "," + itos1((offset+size)*4*(-1)));
            else emit("sub " + r + "," + r + "," + itos1((offset+1)*4));
            emit("mul " + idx + "," + idx + ",4");
            if (offset < 0) emit("add " + r + "," + r + "," + idx);
            else emit("sub " + r + "," + r + "," + idx);
        }
        else{
            if(offset<0) emit("add " + r + "," + r + "," + itos1((offset+1)*4*(-1)));
            else emit("sub " + r + "," + r + ","  + itos1((offset+1)*4));
        }
        /*emit("sub " + r + "," + r + "," + itos1(offset*4+4));
        if(size>0) {
            if (offset < 0) emit("add " + r + "," + r + "," + idx);
            else emit("sub " + r + "," + r + "," + idx);
        }*/
        emit("li "+r2+",1");
        emit("sw "+r2+",("+r+")");
        int line = emit("j ");

        l = CodeBuffer::instance().genLabel();
        //if(size>0) emit("mul " + idx + "," + idx + ",4");
        CodeBuffer::instance().bpatch(falseList,l);
        emit("move " + r + ",$fp");
        if(size>0) {
            if(offset<0) emit("add " + r + "," + r + "," + itos1((offset+size)*4*(-1)));
            else emit("sub " + r + "," + r + "," + itos1((offset+1)*4));
            emit("mul " + idx + "," + idx + ",4");
            if (offset < 0) emit("add " + r + "," + r + "," + idx);
            else emit("sub " + r + "," + r + "," + idx);
        }
        else{
            if(offset<0) emit("add " + r + "," + r + "," + itos1((offset+1)*4*(-1)));
            else emit("sub " + r + "," + r + ","  + itos1((offset+1)*4));
        }
        /*emit("sub " + r + "," + r + "," + itos1(offset*4+4));
        if(size>0) {
            if (offset < 0) emit("add " + r + "," + r + "," + idx);
            else emit("sub " + r + "," + r + "," + idx);
        }*/
        emit("sw $0,(" + r + ")");
        l = CodeBuffer::instance().genLabel();
        CodeBuffer::instance().bpatch(CodeBuffer::instance().makelist(line),l);
        p.regDealloc(r);
        p.regDealloc(idx);
        p.regDealloc(r2);
    }

    void idBoolGetGen(int offset, vector<int>& trueList, vector<int>& falseList, string idx="", int size=0){
        if(size > 0){
            limitsCheckGen(idx,size);
        }
        string r = p.regAlloc();
        emit("move " + r + ",$fp");
        //emit("sub " + r + "," + r + "," + itos1(offset*4+4));
        if(size>0) {
            if(offset<0) emit("add " + r + "," + r + "," + itos1((offset+size)*4*(-1)));
            else emit("sub " + r + "," + r + "," + itos1((offset+1)*4));
            emit("mul " + idx + "," + idx + ",4");
            if (offset < 0) emit("add " + r + "," + r + "," + idx);
            else emit("sub " + r + "," + r + "," + idx);
        }
        else{
            if(offset<0) emit("add " + r + "," + r + "," + itos1((offset+1)*4*(-1)));
            else emit("sub " + r + "," + r + "," + itos1((offset+1)*4));
        }
        emit("lw "+r+","+"("+r+")");
        falseList=CodeBuffer::instance().merge(falseList,CodeBuffer::instance().makelist(emit("beq "+r+",$0,")));
        trueList=CodeBuffer::instance().merge(trueList,CodeBuffer::instance().makelist(emit("j ")));
        p.regDealloc(r);
        if(idx!="") p.regDealloc(idx);
    }

};
#endif //COMPI_HW5_AUXILARY_H
