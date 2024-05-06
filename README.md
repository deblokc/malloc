# Malloc

This is an implemenation of the library malloc (`man 3 malloc`). It includes `malloc`, `free`, `calloc` and `realloc`.

## Implementation

To create usable memory we uses the `mmap` syscall. We could just mmap the size we are asked and return it but it would be **very** inefficient. We are going to allocate way more each times and manages theses pages of memory to suit the needs of the program.

We have three categories of page size : `TINY (0 - 256)`, `SMALL (256 - 4096` and `BIG (4096 and above)`
Each allocations for `SMALL` and `TINY` chunks are going to be big enough to hold at least 100 of them. `BIG` are in their own chunks.
We keep tracks of pages as a chained list with the size, to allow easier search of a corresponding page.

When the user calls the function, we first iterate through our list to check if we have a page. If we do, we iterate through the chunk in this page and use the first unused chunk we find. If we don't find any page corresponding we create one.

## Some optimisations

Each returned chunk needs to be 16 bytes aligned. If we just return the precise size the user asked for, we would loose some size because chunks are not aligned between one another. The size of our struct is also a problem, since it's between chunks it adds a padding that forces us to realigne every time.

```
struct {
  struct *next;   8 bytes
  size_t size;    8 bytes
  bool   is_free; 1
  size_t padding; 8 bytes
};
```

 -------------------------------------------------------------------------------------------------------------------------
| 0x0               | 0x19                            | 0x20             | 0x2d              | 0x46               | 0x50  |
| struct (25 bytes) | padding for alignment (7 bytes) | chunk (13 bytes) | struct (25 bytes) | padding (10 bytes) | chunk |
 -------------------------------------------------------------------------------------------------------------------------

 This make us loose a lot of space ! We can improve this by always return a pointer which size is a multiple of 16 and making our struct a multiple of 16. For example, we could either change `bool` to an `int` which would make the size of this struct 32 bytes, but we can do better.
 Since our chunks are always going to be modulo 16, the lowest bits in size are always going to be 0. We can use them as flag, and reduce the size to 16 bytes.

```
struct {
  struct *next;  8 bytes
  size_t  size;  8 bytes
};
```
 
  -------------------------------------------------------------------------------------------
 | 0x0               | 0x10                                      | 0x20              | 0x30  |
 | struct (16 bytes) | chunk (asked for 13 bytes, is getting 16) | struct (16 bytes) | chunk |
  -------------------------------------------------------------------------------------------

Another optimisation we can implement is `defragmentation`. When memory is used and then freed, it can create a lot of small adjacent free chunks. For example, we would like to allocate 64 bytes of memory but there are just a lot of small chunks.

 --------------------------------------------------------------------------------------------------------------
| 16 bytes free chunk | 32 bytes free chunk | 32 bytes free chunk | 16 bytes used chunk | 128 bytes free chunk |
 --------------------------------------------------------------------------------------------------------------

 In the above scenario our algorithm would take the 128 bytes free chunk, divide it into 2 smaller chunk and use the first one. But we had enough spaces in free chunk before ! To resolve this issue, we will use our `free` and `realloc` function to check for free neighbour and if we find any we merge them to create a bigger free chunk, that can be split later if needed.


Our realloc function also does this when reallocating a chunk, checking if any adjacent chunk is free and can be merged to avoid needing to copy the content of the chunk to a bigger chunk.


## Testing

You can compile the project by using `make`. You can also compile some tests by running `make test`. To run those test using my library, use the `run.sh` script.
For example, to test with ls use `./run.sh ls`


## Ideas for improvement

To further improve this implementation, we could add the use of `bins` which keeps recently freed chunk to reuse them faster. We could also have a better multithread gestion, because locking a mutex each time a thread needs to malloc isn't very fast. Each thread could have it's own page to allow real multithread malloc.
