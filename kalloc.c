// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld
int numfreepages=0;

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
  int len;
} kmem;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  freerange(vstart, vend);
  kmem.len = 0;
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);
  numfreepages++;
  r = (struct run*)v;
  r->next = kmem.freelist;
  kmem.freelist = r;
  kmem.len++;
  if(kmem.use_lock)
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  kmem.len--; 
  numfreepages++;
  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)r;
}

int len(struct run * head) 
{
	struct run * cur;
	int ret;
	ret = 0;
	cur = head;
  	for (;cur;cur=cur->next)
		ret++;
	return ret;
}

int freemem(void)
{
    
    //int count;
    // int ret;
    // // acquire(&kmem.lock);
    // // // count = kmem.free_count;
    // // ret = len((struct run*)kmem.freelist);
    // // // cprintf("\n\n------%d------\n\n",ret);
    // // release(&kmem.lock);
    // if (kmem.use_lock){
    //   acquire(&kmem.lock);
    // }
    // cprintf("\nHELLO!\n");
    // ret = len((struct run*)kmem.freelist);
    // cprintf("freemem: %d free pages\n", ret);
    // if (kmem.use_lock)
    //   release(&kmem.lock);

    
    return numfreepages;
}

