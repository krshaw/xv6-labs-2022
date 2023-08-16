// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKETS 13

struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  struct buf buckets[NBUCKETS];
  struct spinlock bucket_locks[NBUCKETS];
} bcache;

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock, "bcache");

  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    initsleeplock(&b->lock, "buffer");
  }
  for (int i = 0; i < NBUCKETS; ++i) {
      // set the prev and next of the first element equal to the first element
      bcache.buckets[i].prev = bcache.buckets + i;
      bcache.buckets[i].next = bcache.buckets + i;
      initlock(bcache.bucket_locks + i, "bcache.bucket");
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  const int bucket = blockno % NBUCKETS;
  acquire(&bcache.bucket_locks[bucket]);
  // Is the block already cached?
  for(b = bcache.buckets[bucket].next; b != &bcache.buckets[bucket]; b = b->next) {
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.bucket_locks[bucket]);
      acquiresleep(&b->lock);
      return b;
    }
  }
  release(&bcache.bucket_locks[bucket]);
  // its ok to serialize finding an unused buffer to re-use
  acquire(&bcache.lock);
  acquire(&bcache.bucket_locks[bucket]);
  // at this point need to search through all the buffers to find one with refcnt == 0
  // two cases: it might not be in a bucket yet, and it might be in the same bucket we are moving it to
  // check the first case by checking if it has a next and prev unset
  // check the second case by checking if it has the same hash
  // if the second case is true, then don't attempt to acquire the lock (we already hold it!)
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
      // lots of repetitiveness, refactor
      if(b->next == 0 && b->prev == 0){
        b->next = bcache.buckets[bucket].next;
        b->prev = &bcache.buckets[bucket];
        bcache.buckets[bucket].next->prev = b;
        bcache.buckets[bucket].next = b;
        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        release(&bcache.bucket_locks[bucket]);
        release(&bcache.lock);
        acquiresleep(&b->lock);
        return b;
      }
      const int other_bucket = b->blockno % NBUCKETS;
      // if its the same bucket, we already have the lock so its safe to check refcnt
      if (other_bucket == bucket && b->refcnt == 0) {
        // already in this hash bucket, so no need to mess with prev/next pointers
        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        release(&bcache.bucket_locks[bucket]);
        release(&bcache.lock);
        acquiresleep(&b->lock);
        return b;
      }
      // else, it might be part of a different bucket
      // the first for loop might take this buffer from us
      // this is why despite holding the global lock, we still need to hold the bucket lock
      if (other_bucket != bucket) {
        acquire(&bcache.bucket_locks[other_bucket]);
      }

      // if we enter this branch, we are part of another bucket because the above branch checked if refcnt==0 and its in the same bucket
      if(b->refcnt == 0) {
        // we are recycling a buf from another, so we need to and remove it from its current list
        b->next->prev = b->prev;
        b->prev->next = b->next;
        // then prepend b to this hashbucket list
        b->next = bcache.buckets[bucket].next;
        b->prev = &bcache.buckets[bucket];
        bcache.buckets[bucket].next->prev = b;
        bcache.buckets[bucket].next = b;
        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        release(&bcache.bucket_locks[other_bucket]);
        release(&bcache.bucket_locks[bucket]);
        release(&bcache.lock);
        acquiresleep(&b->lock);
        return b;
      }
      if (other_bucket != bucket)
        release(&bcache.bucket_locks[other_bucket]);
  }

  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  // if the buffer doesn't contain a copy of the block, that means 
  // its not cached so this is a recycled buffer. read data from disk onto it
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  const int bucket = b->blockno % NBUCKETS;
  acquire(&bcache.bucket_locks[bucket]);
  b->refcnt--;
  release(&bcache.bucket_locks[bucket]);
}

void
bpin(struct buf *b) {
  const int bucket = b->blockno % NBUCKETS;
  acquire(&bcache.bucket_locks[bucket]);
  b->refcnt++;
  release(&bcache.bucket_locks[bucket]);
}

void
bunpin(struct buf *b) {
  const int bucket = b->blockno % NBUCKETS;
  acquire(&bcache.bucket_locks[bucket]);
  b->refcnt--;
  release(&bcache.bucket_locks[bucket]);
}


