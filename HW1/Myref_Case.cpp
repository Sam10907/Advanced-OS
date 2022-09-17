#include "Page_Replacement_Algo.hpp"

int main(){
    //My reference string
    srand(time(NULL));
    int locality = 1000; //固定locality為1000
    int  interval1 = 100; //固定數字範圍
    int start1 = rand() % 1200 + 1;
    int ref[interval1];
    for(int k = 0 ; k < interval1 ; k++,start1++){
        if(start1 > 1200) start1 = start1 % 1200;
        ref[k] = start1;
    }
    Page *p = (Page*) malloc(300000 * sizeof(Page));
    unsigned char *uc = (unsigned char*) malloc(300000*sizeof(unsigned char));
    int initial1 = 0;
    for(int i = 0 ; i < 300000 ; i++){
        int index = rand() % interval1;
        p[i].num = ref[index];
        p[i].ref = 0;
        p[i].dirty = 0;
        uc[i] = 0;
        p[i].ref_string = &uc[i];
        if(i == (initial1 + locality - 1)){
            for(int j = initial1 ; j < ((initial1 + i) / 2) ; j++) p[j].dirty = 1; //設定dirty bit,dirty rate = 0.5
            initial1 = i + 1;
            start1 = rand() % 1200 + 1;
            for(int k = 0 ; k < interval1 ; k++,start1++){
                if(start1 > 1200) start1 = start1 % 1200;
                ref[k] = start1;
            }
        }
    }
    //開始統計結果
    cout << "   FIFO   Optimal   ARB" << endl;
    for(int i = 30 ; i <= 150 ; i += 30){
        cout << i << "   ";
        FIFO_algo(p,i);
        Optimal_algo(p,i);
        ARB_algo(p,uc,i);
    }
    delete p;
    delete uc;
}