#include "Page_Replacement_Algo.hpp"

void FIFO_algo(Page *p,int frames){
    deque<Page> dq; //用做page frame buffer
    int interrupt = 0; //中斷次數
    int write_disk = 0; //寫回硬碟次數
    int page_fault = 0; //page fault次數
    for(int i = 0 ; i < 300000 ; i++){
        deque<Page>::iterator itr = dq.begin();
        bool find = 0;
        while(itr != dq.end()){ //檢查frame是否有匹配到目前scan到page number
            if(itr -> num == p[i].num){
                find = 1;
                break;
            }
            itr++;
        }
        if(find) continue; //有找到
        //沒找到
        if(dq.size() >= frames){ //如果frame size滿了，用FIFO algo處理
            if(dq.begin() -> dirty){ 
                write_disk++;
                interrupt++;
            }
            dq.pop_front();
        }
        dq.push_back(p[i]);
        page_fault++;
    }
    cout << page_fault << "   ";
    cout << interrupt << "   ";
    cout << write_disk << "   ";
    dq.clear();
}

void Optimal_algo(Page *p,int frames){
    deque<Page> dq;
    int interrupt = 0; //中斷次數
    int write_disk = 0; //寫回硬碟次數
    int page_fault = 0; //page fault次數
    for(int i = 0 ; i < 300000 ; i++){
        deque<Page>::iterator itr = dq.begin();
        bool find = 0;
        while(itr != dq.end()){ //檢查frame是否有匹配到目前scan到page number
            if(itr -> num == p[i].num){
                find = 1;
                break;
            }
            itr++;
        }
        if(find) continue; //有找到
        if(dq.size() >= frames){  //如果frame size滿了，用Optimal algo處理
            int count = 1;
            for(int j = 0 ; j < dq.size() ; j++){
                for(int k = i + 1 ; k < 300000 ; k++,count++){
                    if(p[k].num == dq[j].num){
                        dq[j].count = count;
                        break;
                    }
                }
                dq[j].count = count;
                count = 1;
            }
            int max_count = dq[0].count;
            int index = 0;
            for(int j = 1 ; j < dq.size() ; j++){
                if(dq[j].count > max_count){
                    index = j;
                    max_count = dq[j].count;
                }
            }
            itr = dq.begin();
            itr += index;
            if(itr -> dirty){ 
                write_disk++;
                interrupt++;
            }
            dq.erase(itr);
        }
        dq.push_back(p[i]);
        page_fault++;
    }
    cout << page_fault << "   ";
    cout << interrupt << "   ";
    cout << write_disk << "   ";
    dq.clear();
}

void ARB_algo(Page *p,unsigned char *uc,int frames){
    deque<Page> dq;
    int interrupt = 0; //中斷次數
    int write_disk = 0; //寫回硬碟次數
    int page_fault = 0; //page fault次數
    for(int i = 0 ; i < 300000 ; i++){ 
        if(i > 0 && !(i % 150)){ //每當迴圈跑150次時，更新reference table，然後將ref bit設為0
            for(int j = 0 ; j < 300000 ; j++){
                *p[j].ref_string = (*p[j].ref_string >> 1) + (128 * p[j].ref);
                p[j].ref = 0;
            }
            interrupt++;
        }
        deque<Page>::iterator itr = dq.begin();
        bool find = 0;
        while(itr != dq.end()){ //檢查frame是否有匹配到目前scan到page number
            if(itr -> num == p[i].num){
                find = 1;
                break;
            }
            itr++;
        }
        if(find){ //有找到
            p[i].ref = 1; 
            continue;
        }
        if(dq.size() >= frames){ //如果frame size滿了，用ARB algo處理
            unsigned char min = *dq[0].ref_string;
            int index = 0;
            for(int j = 1 ; j < dq.size() ; j++){
                if(*dq[j].ref_string < min){
                    index = j;
                    min = *dq[j].ref_string;
                }
            }
            itr = dq.begin();
            itr += index;
            if(itr -> dirty){
                interrupt++;
                write_disk++;
            }
            dq.erase(itr);
        }
        p[i].ref = 1;
        dq.push_back(p[i]);
        page_fault++;
    }
    cout << page_fault << "   ";
    cout << interrupt << "   ";
    cout << write_disk << endl;
    dq.clear();
    for(int i = 0 ; i < 300000 ; i++){ 
        uc[i] = 0;
        p[i].ref = 0;
    }
}