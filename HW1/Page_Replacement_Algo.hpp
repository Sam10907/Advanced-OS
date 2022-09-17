#include <iostream>
#include <cstdlib>
#include <ctime>
#include <deque>
using namespace std;
class Page{
    public:
        int num;
        bool ref;
        bool dirty;
        unsigned char *ref_string;
        int count;
        bool chance;
        int ref_count;
};
void FIFO_algo(Page*,int);
void Optimal_algo(Page*,int);
void ARB_algo(Page*,unsigned char*,int);