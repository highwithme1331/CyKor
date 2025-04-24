#include <iostream>
#include <cstring>
using namespace std;
#define INF 987654231
#define STACK_SIZE 50

int call_stack[STACK_SIZE];
char info_stack[STACK_SIZE][20];
int SP=-INF;
int FP=-INF;

bool isEmpty() {
    return (SP==-INF);
}

bool isFull() {
    return (SP+1==STACK_SIZE);
}

int size() {
    return SP+1; 
}

int top() {
    if(isEmpty()) {
        cout<<"Stack is empty"<<"\n";
        return 0;
    }

    return call_stack[SP];
}

void clear() {
    SP=-INF;
    FP=-INF;
    
    for(int i=0; i<STACK_SIZE; i++) {
        call_stack[i]=0;
        strcpy(info_stack[i], "");
    }
}

void push_value(int value, const char* info) {
    if(SP+1>=STACK_SIZE) {
        cout<<"Stack Overflow : "<<info<<"\n";
        return;
    }
	
    if(SP==-INF)
        SP=0;

    else
        SP++;
	
    call_stack[SP]=value;
    strcpy(info_stack[SP], info);
}

void push_SFP(const char* info) {
    if(SP+1>=STACK_SIZE) {
        cout<<"Stack Overflow : "<<info<<"\n";
        return;
    }
	
    if(SP==-INF)
        SP=0;

    else
        SP++;

    call_stack[SP]=FP;
    strcpy(info_stack[SP], info);
}

void push_RA() {
    if(SP+1>=STACK_SIZE) {
        cout<<"Stack Overflow"<<"\n";
        return;
    }
	
    if(SP==-INF)
        SP=0;

    else
        SP++;

    call_stack[SP]=-1;
    strcpy(info_stack[SP], "Return Address");
}

int pop() {
    if(SP==-INF) {
        cout<<"Stack Underflow";
        return 0;
    }

    int val=call_stack[SP];
    
    if(SP==-1)
        SP=-INF;

    else
        SP--;

    return val;
}

void print_stack() {
    if(SP==-1) {
        cout<<"Stack is empty."<<"\n";
        return;
    }

    cout<<"====== Current Call Stack ======"<<"\n";
	
    for(int i=SP; i>=0; i--) {
        if(call_stack[i]!=-1)
            cout<<i<<" : "<<info_stack[i]<<" = "<<call_stack[i];

        else
            cout<<i<<" : "<<info_stack[i];

        if(i==SP)
			cout<<"    <=== [esp]";

        else if(i==FP) 
			cout<<"    <=== [ebp]";
		
        cout<<"\n";
    }
	
    cout<<"================================"<<"\n\n";
}

void func1(int arg1, int arg2, int arg3);
void func2(int arg1, int arg2);
void func3(int arg1);

void func1(int arg1, int arg2, int arg3) {
    push_value(arg3, "arg3");
    push_value(arg2, "arg2");
    push_value(arg1, "arg1");
    push_RA();
    push_SFP("func1 SFP");
    FP=SP;
    if(SP+1>=STACK_SIZE) {
        cout<<"Stack Overflow"<<"\n";
        return;
    }
    SP++;
    call_stack[SP]=100;
    strcpy(info_stack[SP], "var_1");
    print_stack();

    func2(11, 13);
    SP--;
    SP=FP;
    int before_FP=pop();
    FP=before_FP;
    pop();
    pop();
    pop();
    pop();
    print_stack();
}

void func2(int arg1, int arg2) {
    push_value(arg2, "arg2");
    push_value(arg1, "arg1");
    push_RA();
    push_SFP("func2 SFP");
    FP=SP;
    if(SP+1>=STACK_SIZE) {
        cout<<"Stack Overflow"<<"\n";
        return;
    }
    SP++;
    call_stack[SP]=200;
    strcpy(info_stack[SP], "var_2");
    print_stack();
	
    func3(77);
    SP--;
    SP=FP;
    int before_FP=pop();
    FP=before_FP;
    pop();
    pop();
    pop();
    print_stack();
}

void func3(int arg1) {
    push_value(arg1, "arg1");
    push_RA();
    push_SFP("func3 SFP");
    FP=SP;
    if(SP+1>=STACK_SIZE) {
        cout<<"Stack Overflow"<<"\n";
        return;
    }
    SP++;
    call_stack[SP]=300;
    strcpy(info_stack[SP], "var_3");
    if(SP+1>=STACK_SIZE) {
        cout<<"Stack Overflow in func3 (var_4)"<<"\n";
        return;
    }
    SP++;
    call_stack[SP]=400;
    strcpy(info_stack[SP], "var_4");
    print_stack();

    SP--;
    SP--;
    SP=FP;
    int before_FP=pop();
    FP=before_FP;
    pop();
    pop();
    print_stack();
}

int main() {
    func1(1, 2, 3);
    print_stack();
    return 0;
}