#include <iostream>
#include <cstring>
using namespace std;
#define STACK_SIZE 50

int call_stack[STACK_SIZE];
char info_stack[STACK_SIZE][20];
int SP=-1;
int FP=-1;

void push_value(int value, const char* info) {
    SP++;
    call_stack[SP]=value; 
    strcpy(info_stack[SP], info);
}

void push_SFP(const char* info) {
    SP++;
    call_stack[SP]=FP;
    strcpy(info_stack[SP], info);
}

void push_RA() {
    SP++;
    call_stack[SP]=-1;
    strcpy(info_stack[SP], "Return Address");
}

int pop() {
    int val=call_stack[SP];
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
    push_value(100, "var_1");
    print_stack();

    func2(11, 13);
    pop(); 
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
    push_value(200, "var_2");
    print_stack();

    func3(77);
    pop();
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
    push_value(300, "var_3");
    push_value(400, "var_4");
    print_stack();
	
    pop();
    pop();
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
