// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
  uint64 total_pages;
  uint64 used_pages;
} kmem;

struct {
  struct spinlock lock;
  int ref [PHYSTOP / PGSIZE + 1];
} kref;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&kref.lock, "kref");
  kmem.total_pages = ((char*)PHYSTOP - end) / PGSIZE;
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

int
decrement(uint64 pa)
{
  acquire(&kref.lock);
  int tmp = 0;
  if (kref.ref[pa / PGSIZE])
    tmp = kref.ref[pa / PGSIZE]--;
  release(&kref.lock);
  return tmp;
}

void
increment(uint64 pa)
{
  acquire(&kref.lock);
  ++kref.ref[pa / PGSIZE];
  release(&kref.lock);
}

uint64
get_total_pages()
{
  acquire(&kmem.lock);
  uint64 total = kmem.total_pages;
  release(&kmem.lock);
  return total;
}

uint64
get_used_pages()
{
  acquire(&kmem.lock);
  uint64 used = kmem.used_pages;
  release(&kmem.lock);
  return used;
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  int ref;
  if ((ref = decrement((uint64)pa)) > 1)
    return;

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  kmem.used_pages -= (ref? 1 : 0);
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next,
    kmem.used_pages++;
  release(&kmem.lock);

  if(r) {
    memset((char*)r, 5, PGSIZE); // fill with junk
    increment((uint64)r);
  }
  return (void*)r;
}
