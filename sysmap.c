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

// for문의 total_mmaps 종료 조건 -1에서 0으로 변환

uint mmap(uint addr, int length, int prot, int flags, int fd, int offset){

    struct file *f;
    struct proc *p = myproc();
    int midx = p->total_mmaps;

    if (p->total_mmaps >= 64) {
        return 0;
    }

    if(fd!=-1){ // ANONYMOUS_MAPPING이 아닌경우
        
        f = fdlookup(fd);

    } else if(fd==-1 && ((flags & MAP_POPULATE) || !(flags & MAP_POPULATE|MAP_ANONYMOUS))){
        // else는 fd가 -1이면서 MAP_ANONUMOUS 인경우 but offset이 0인지는 에러 처리 해줬나?

        // Checking for regions in mmap_area array for blank sections
        int q=-1;
        if ((void *)addr != (void *)0 && addr % PGSIZE == 0 && addr >= MMAPBASE && addr + length <= KERNBASE) {
        
            for(int j=0;j<p->total_mmaps;j++){
                // 정말로 overlap을 안고려한다고요??
                if(p->mmaps[j].addr == addr)
                    return 0;

                q = mmap_anon(addr, length, p, prot, flags);
            }
        }
        if (q<0)
            return 0;

        p->mmaps[midx].addr = addr;  
        p->mmaps[midx].flags = flags;
        p->mmaps[midx].prot = PTE_U | prot;
        // p->mmaps[midx].prot = prot;
        p->mmaps[midx].p = p;      // 프로세스를 할당을 다시 해준다고?
        p->total_mmaps += 1;
        return p->mmaps[midx].addr;

    } else{
        return 0;
    }
    

    if (!(flags & MAP_ANONYMOUS) || !(flags & MAP_POPULATE|MAP_ANONYMOUS)) {  // 익명 매핑이 아닌 경우에만 파일 권한을 검사
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
        
        // Checking for regions in mmap_area array for blank sections
        for(int j=0;j<p->total_mmaps;j++){
            // 정말로 overlap을 안고려한다고요??
            if(p->mmaps[j].addr == addr)
                return 0;
        }
        
        i = mmap_file(addr, length, p, prot, flags);
    }

    if(i==-1){
        return 0;
    }

    // 매핑 정보 저장
    p->mmaps[midx].addr  = addr;
    p->mmaps[midx].flags = flags;
    p->mmaps[midx].prot = PTE_U | prot;
    // p->mmaps[midx].prot = prot;
    p->mmaps[midx].offset = offset;
    p->mmaps[midx].f = f;
    p->mmaps[midx].p = p;      // 프로세스를 할당을 다시 해준다고?
    p->total_mmaps += 1;

    // 성공적으로 매핑된 가상 주소 반환
    return (void *)p->mmaps[i].addr;
    

}


// return success(1) | fail(-1)
int munmap(uint addr){
    struct proc *p = myproc();
    int i;

    // 탐색하여 해당 시작 주소를 갖는 mmap_area 찾기
    for (i = 0; i < p->total_mmaps; i++) {
        if (p->mmaps[i].addr == addr) {
            struct mmap_area *area = &p->mmaps[i];

            // 각 페이지에 대해 처리
            for (uint current_addr = area->addr; current_addr < area->addr + area->length; current_addr += PGSIZE) {
                pte_t *pte = walkpgdir(p->pgdir, (void *)current_addr, 0);
                if (pte && (*pte & PTE_P)) {
                    // 페이지가 할당된 경우, 물리 페이지 해제
                    char *phys_page = P2V(PTE_ADDR(*pte));
                    memset(phys_page, 1, PGSIZE);  // 페이지를 1로 채우기
                    kfree(phys_page);  // 페이지를 자유 리스트에 반환
                    *pte = 0;  // 페이지 테이블 엔트리 초기화
                }
            }

            // mmap_area 구조체 제거
            for (int j = i; j < p->total_mmaps - 1; j++) {
                p->mmaps[j] = p->mmaps[j + 1];
            }
            p->total_mmaps--;  // 매핑된 영역 수 감소

            return 1;  // 성공적으로 해제
        }
    }

    return -1;  // 해당 주소로 시작하는 mmap_area 없음

}

// The main function for file-mapping flags for POP O/X
int mmap_file(uint addr, int length, struct proc *p, int prot,int flags){

    // PGROUNDOWN or UP just in case
    uint current_addr = PGROUNDDOWN(addr);
    uint end_addr = PGROUNDUP(addr + length);
    
    // POPULATE MAAPING
    if(flags==MAP_POPULATE){
        
        for (; current_addr < end_addr; current_addr += PGSIZE) {
            char *paddr = kalloc();  // 물리 메모리 페이지 할당
            if (!paddr)
                return -1;  // 물리 페이지 할당 실패 처리
            if (mappages(p->pgdir, (void*)current_addr, PGSIZE, V2P(paddr), PTE_U | prot) != 0){
                // if (mappages(p->pgdir, (void*)current_addr, PGSIZE, V2P(paddr), prot) != 0){
                // mappages failed
                deallocuvm(p->pgdir, paddr - PGSIZE, paddr);
                kfree(paddr);
                return -1;  // mappages 실패 처리
            }
        }

        return 0;
    }


    if(flags==0){   // 뭐야 정말로 아무것도 안하네;
        return 0;
    }


    return -1;  // return 0 in parent function

}

int mmap_anon(uint mmapaddr, int length, struct proc *p, int prot, int flags){

    // 사실 여기서 다른 flag 에러핸들링 할 수도
    if(flags & MAP_POPULATE|MAP_ANONYMOUS){
        for (int i = 0; i < length; i+=PGSIZE)
        {
            if (map_page_anon(mmapaddr + i, p, prot) < 0)
                return -1;
        }
    }

    if (flags & MAP_ANONYMOUS){
        return 0;
    }
    


    return 0;

}

int map_page_anon(uint mmapaddr, struct proc *p, int prot){

    char *mapped_page = kalloc();
    if (!mapped_page) {
        return -1;
    }
    memset(mapped_page, 0, PGSIZE);
    if (mappages(p->pgdir, (void *)mmapaddr, PGSIZE, V2P(mapped_page), prot) <
      0) {
        deallocuvm(p->pgdir, mmapaddr - PGSIZE, mmapaddr);
        kfree(mapped_page);
        return -1;
    }
  
    return 0;

}

void copy_mmap_struct(struct mmap_area *dest, struct mmap_area *src) {
  dest->addr = src->addr;
  dest->length = src->length;
  dest->flags = src->flags;
  dest->prot = src->prot;
  dest->f = src->f;
  dest->offset = src->offset;
  dest->p = src->p;         // 이거 이렇게 해도 되는거임??
}

int copy_maps(struct proc *parent, struct proc *child) {
    pte_t *pte;
    int i = 0;
    while (i < parent->total_mmaps) {
        uint addr = parent->mmaps[i].addr;
        int protection = parent->mmaps[i].prot;
        int flags = parent->mmaps[i].flags;
        uint length = parent->mmaps[i].length;
        uint start = addr;
        for (; start < addr + length; start += PGSIZE) {
            pte = walkpgdir(parent->pgdir, (char *)start, 0);
            uint pa = pte ? PTE_ADDR(*pte) : 0;
            if (pa == 0) {
                if (!(parent->mmaps[i].flags & MAP_ANONYMOUS)) {
                    // File mapping
                    uint current_addr = PGROUNDDOWN(start);
                    char *paddr = kalloc();
                    if (!paddr) {
                        return -1;
                    }
                    memset(paddr, 0, PGSIZE);
                    if (readi(parent->mmaps[i].f, paddr, PGSIZE, parent->mmaps[i].offset + (start - addr)) < 0) {
                        kfree(paddr);
                        return -1;
                    }
                    int prot = parent->mmaps[i].prot;
                    if (prot & PROT_WRITE) {
                        prot |= PTE_W;
                    }
                    if (mappages(child->pgdir, (void *)current_addr, PGSIZE, V2P(paddr), prot) != 0) {
                        deallocuvm(child->pgdir, paddr - PGSIZE, paddr);
                        kfree(paddr);
                        cprintf("CopyMaps: mappages failed\n");
                        return -1;
                    }
                } else if (parent->mmaps[i].flags & MAP_POPULATE) {
                    // Anonymous mapping with MAP_POPULATE
                    uint current_addr = PGROUNDDOWN(start);
                    char *paddr = kalloc();
                    if (!paddr) {
                        return -1;
                    }
                    memset(paddr, 0, PGSIZE);
                    int prot = parent->mmaps[i].prot;
                    if (prot & PROT_WRITE) {
                        prot |= PTE_W;
                    }
                    if (mappages(child->pgdir, (void *)current_addr, PGSIZE, V2P(paddr), prot) != 0) {
                        deallocuvm(child->pgdir, paddr - PGSIZE, paddr);
                        kfree(paddr);
                        cprintf("CopyMaps: mappages failed\n");
                        return -1;
                    }
                } else {
                    // Anonymous mapping without MAP_POPULATE
                    if (map_page_anon(child, start, protection) < 0) {
                        return -1;
                    }
                }
            } else {
                char *parentmem = (char *)P2V(pa);
                if (mappages(child->pgdir, (void *)start, PGSIZE, V2P(parentmem), protection) < 0) {
                    cprintf("CopyMaps: mappages failed\n");
                    return -1;
                }
            }
        }
        copy_mmap_struct(&child->mmaps[i], &parent->mmaps[i]);
        i += 1;
    }
    child->total_mmaps = parent->total_mmaps;
    return 0;
}

