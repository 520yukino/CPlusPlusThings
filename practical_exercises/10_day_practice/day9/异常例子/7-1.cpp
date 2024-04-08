//Eg10-9.cpp
#include<iostream>
using namespace std;
class B{
    double *pb[10];
    public:
        B(){
            cout<<"int B constructor..."<<endl;
            for (int i=0;i<12;i++){
                pb[i]=new double[200000000];
                if(pb[i]==0)
                    throw i;
                else 
                    cout<<"Allocated 200000000 doubles in pb["<<i<<"]"<<endl;
            }
        }
};
int main(){
    try{
        B b;
    }
    catch(int e){
        cout<<"catch an exception when allocated pb["<<e<<"]"<<endl;   
    }
    system("pause");
}
