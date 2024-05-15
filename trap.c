#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"
#include "fs.h"
#include "sleeplock.h" 
#include "file.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

void
tvinit(void)
{
  int i;
  
  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks += 1000;                // changed to millitick
      
      /* Updating runtime for each timer-interrupt */
      if (myproc() && myproc()->state == RUNNING) {
        // moved to if case below
        //  if(myproc()->pid==3)
        //   cprintf("\n%d\n",myproc()->runtime);
      }

      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;
  case T_PGFLT:
    // 추가로 범위 지정해줘야해?
    //if (rcr2() >= MMAPBASE && rcr2() < KERNBASE) {
    page_fault_handler(tf);
    break;
    //}
    
  //PAGEBREAK: 13
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER){
     myproc()->ptick += 1000;
     myproc()->runtime += 1000;

     int tsum_runnable = findrunnable(&ticks);
     
     if(tsum_runnable==0){
      myproc()->timeslice = 10 * 1000 * 0; // Divide-by-Zero 발생?
     } else{
      // timeslice 문제 (1000을 곱하냐 마냐)
      myproc()->timeslice = 10 * 1000 * ((float)getweight(myproc()->nice)/(float)tsum_runnable); // Divide-by-Zero 발생?
     }
     //cprintf("tsum_runnable: %d, timeslice: %d, pticks: %d\n",(int)tsum_runnable,myproc()->timeslice,myproc()->ptick);

     myproc()->vruntime += 1000 * ((float)getweight(20) / (float)getweight(myproc()->nice));


     if(myproc()->ptick >= myproc()->timeslice){
      // cprintf("**********************");
      yield();
     }

    }

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}

// void page_fault_handler(struct trapframe *tf) {
//     struct proc *p = myproc();
//     uint addr = rcr2();
//     int write_fault = tf->err & 2;

//     for (int i = 0; i < p->total_mmaps; i++) {
//         uint start = p->mmaps[i].addr;
//         uint end = start + (uint)p->mmaps[i].length;

//         if (addr >= start && addr <= end) {
//             // Check if the fault address is within the mapped region
//             pte_t *pte = walkpgdir(p->pgdir, (char *)PGROUNDDOWN(addr), 0);
//             uint pa = pte ? PTE_ADDR(*pte) : 0;

//             if (pa && (*pte & PTE_P)) {
//                 // If the page is present but there's a write fault on a read-only page
//                 if (write_fault && !(*pte & PTE_W)) {
//                     cprintf("Protection Fault: %p\n", rcr2());
//                     myproc()->killed = 1;
//                     return;
//                 }
//                 // If the page is already present and writable, do nothing
//                 return;
//             }

//             if (!pa || !(*pte & PTE_P)) {
//                 // Allocate a new page if the page is not present
//                 char *paddr = kalloc();
//                 if (!paddr) {
//                     myproc()->killed = 1;
//                     return;
//                 }
//                 memset(paddr, 0, PGSIZE);

//                 if (!(p->mmaps[i].flags & MAP_ANONYMOUS)) { // 이 조건도 나중에 바꿔줘야 함!
//                     // File-backed mapping: read the content from the file
//                     int bytes_read = fileread(p->mmaps[i].f, paddr, PGSIZE);
//                     if (bytes_read < 0) {
//                         myproc()->killed = 1;
//                         kfree(paddr);
//                         return;
//                     }
//                   //   cprintf("---%d---",p->mmaps[i].addr);
//                   //   char *data = (char *)paddr;  // 매핑된 가상 주소
//                   //   cprintf("trap Reading mapped data: ");
//                   //   for (int i = 0; i < 50; i++) {
//                   //     cprintf("%d", data[i]);  // 데이터 읽기
//                   //   }
//                   // cprintf("\n");
//                 }

//                 int prot = p->mmaps[i].prot;
//                 if (prot & PROT_WRITE) {
//                     prot |= PTE_W;
//                 }

//                 // Map the new page to the virtual address
//                 if (mappages(p->pgdir, (void *)PGROUNDDOWN(addr), PGSIZE, V2P(paddr), prot) != 0) {
//                     kfree(paddr);
//                     myproc()->killed = 1;
//                     return;
//                 }
//                 return;
//             }
//         }
//     }

//     // If no valid mapping found, kill the process
//     cprintf("Segmentation Fault: %p\n", rcr2());
//     myproc()->killed = 1;
// }

void page_fault_handler(struct trapframe *tf) {
    struct proc *p = myproc();
    uint addr = rcr2();
    int write_fault = tf->err & 2;

    for (int i = 0; i < p->total_mmaps; i++) {


        uint start = p->mmaps[i].addr;
        uint end = start + (uint)p->mmaps[i].length;

        if (addr >= start && addr <= end) {
            // Check if the fault address is within the mapped region
            pte_t *pte = walkpgdir(p->pgdir, (char *)PGROUNDDOWN(addr), 0);
            uint pa = pte ? PTE_ADDR(*pte) : 0;

            if (pa && (*pte & PTE_P)) {
                // If the page is present but there's a write fault on a read-only page
                if (write_fault && !(*pte & PTE_W)) {
                    // Copy-on-Write (COW) mechanism
                    char *newmem = kalloc();
                    if (!newmem) {
                        myproc()->killed = 1;
                        return;
                    }
                    memmove(newmem, (char *)P2V(pa), PGSIZE);

                    // Update the page table entry to point to the new page
                    *pte = V2P(newmem) | PTE_P | PTE_U | PTE_W;
                    lcr3(V2P(p->pgdir));  // Refresh the TLB

                    return;
                }
                // If the page is already present and writable, do nothing
                return;
            }

            if (!pa || !(*pte & PTE_P)) {
                // Allocate a new page if the page is not present
                char *paddr = kalloc();
                if (!paddr) {
                    myproc()->killed = 1;
                    return;
                }
                memset(paddr, 0, PGSIZE);

                if (!(p->mmaps[i].flags & MAP_ANONYMOUS)) { // File-backed mapping: read the content from the file
                    struct file *f = p->mmaps[i].f;
                    uint file_size = f->ip->size;

                    // 파일 크기 확인
                    if (p->mmaps[i].offset > file_size) {
                        cprintf("page_fault_handler: offset %d is beyond file size %d\n", p->mmaps[i].offset, file_size);
                        kfree(paddr);
                        myproc()->killed = 1;
                        return;
                    }

                    // 길이 조정
                    uint length = end - start;
                    if (p->mmaps[i].offset + length > file_size) {
                        length = file_size - p->mmaps[i].offset;
                        cprintf("page_fault_handler: adjusted length to %d\n", length);
                    }

                    ilock(f->ip);
                    int bytes_read = readi(f->ip, paddr, p->mmaps[i].offset + (addr - start), PGSIZE);
                    iunlock(f->ip);

                    if (bytes_read < 0) {
                        myproc()->killed = 1;
                        kfree(paddr);
                        return;
                    }
                }

                int prot = p->mmaps[i].prot;
                if (prot & PROT_WRITE) {
                    prot |= PTE_W;
                }

                // Map the new page to the virtual address
                if (mappages(p->pgdir, (void *)PGROUNDDOWN(addr), PGSIZE, V2P(paddr), prot) != 0) {
                    kfree(paddr);
                    myproc()->killed = 1;
                    return;
                }
                return;
            }
        }
    }

    // If no valid mapping found, kill the process
    cprintf("Segmentation Fault: %p\n", rcr2());
    myproc()->killed = 1;
}

