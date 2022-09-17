#include "Page_Replacement_Algo.hpp"

int main(){
    srand(time(NULL));
    //random reference string
    int start = rand() % 1200 + 1;
    int interval = rand() % 20 + 1;
    int dirty_rate = 0.5;
    int initial = 0;
    Page *p =(Page*) malloc(300000 * sizeof(Page));
    unsigned char *uc = (unsigned char*) malloc(300000*sizeof(unsigned char)); //用於ARB algo
    for(int i = 0, index = 1; i < 300000 ; i++, index++){
        p[i].num = start;
        p[i].ref = 0;
        p[i].dirty = 0;
        uc[i] = 0;
        p[i].ref_string = &uc[i];
        p[i].ref_count = 0;
        p[i].chance = 0;
        start++;
        if(start > 1200) start %= 1200;
        if(index == interval){
            index = 0;
            for(int j = initial ; j < (initial + interval / 2) ; j++) p[j].dirty = 1; //dirty bit設定,dirty rate = 0.5
            initial = i + 1;
            start = rand() % 1200 + 1;
            interval = rand() % 20 + 1;
        }
    }
    //開始統計結果
    cout << "   FIFO   Optimal   ARB" << endl;
    for(int i = 30 ; i <= 150 ; i += 30){
        cout << i << "   ";
        FIFO_algo(p,i); //FIFO algo
        Optimal_algo(p,i); //Optimal algo
        ARB_algo(p,uc,i); //ARB algo (additional reference bit)
    }
    delete p;
    delete uc;
}