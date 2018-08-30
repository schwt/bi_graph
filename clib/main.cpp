#include <iostream>
#include "tt.h"
using namespace std ;

int main() {
     Base <int>b(4) ;
     cout<<b.getA() ;
     Base <double> bc(4) ;
     bc.setA(4.3) ;
     cout<<bc.getA()  << endl;
 return 0 ;  
}
