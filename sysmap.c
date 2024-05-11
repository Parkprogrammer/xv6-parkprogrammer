#include "types.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "mmu.h"
#include "proc.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "fcntl.h"
#include "memlayout.h"


uint mmap(uint addr, int length, int prot, int flags, int fd, int offset){


    struct file *f;
    struct proc *p = myproc();

    if (p->total_mmaps >= 64) {
        return 0;
    }

    if(fd!=-1){ // ANONYMOUS_MAPPING이 아닌경우
        f = fdlookup(fd);
    } else if(fd==-1 && (flags & MAP_ANONYMOUS)){
        // else는 fd가 -1이면서 MAP_ANONUMOUS 인경우 but offset이 0인지는 에러 처리 해줬나?
    } else{
        return 0;
    }
    

    if (!(flags & MAP_ANONYMOUS)) {  // 익명 매핑이 아닌 경우에만 파일 권한을 검사
        if (!f->readable || (prot & PROT_WRITE && !f->writable)) {
            return 0;
        }
    }
    
    // 기본적인 길이 및 위치 검사
    if(length <=0 || offset < 0){
        return 0;
    }
    
    // 매핑 정보를 저장할 구조체의 배열 인덱스를 찾거나 새 위치를 결정
    int i = -1;
    if ((void *)addr != (void *)0 && addr % PGSIZE == 0 && addr >= MMAPBASE && addr + length <= KERNBASE) {
        
        if(flags==MAP_POPULATE)
            i = mmap_file(addr, length, p, 1);
    }

    if(i==-1){
        return 0;
    }

    // 주소가 유효하지 않거나, 주어진 addr에서 매핑이 불가능하면 새 주소를 찾음
    // if (i == -1) {
    //     i = find_mmap_addr(p, length);
    //     if (i == -1) {
    //         return (void *)-1; // 적절한 매핑 주소를 찾을 수 없으면 에러 반환
    //     }
    // }

    // 매핑 정보 저장
    p->mmaps[i].flags = flags;
    p->mmaps[i].prot = PTE_U | prot;
    p->mmaps[i].offset = offset;
    p->mmaps[i].f = f;
    p->mmaps[i].p = p;      // 프로세스를 할당을 다시 해준다고?
    p->total_mmaps += 1;

    // 성공적으로 매핑된 가상 주소 반환
    return (void *)p->mmaps[i].addr;
    

}


// return success(1) | fail(-1)
int munmap(uint addr){


}


void freemem(){


}