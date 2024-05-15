#include "types.h"
#include "user.h"
#include "stat.h"
#include "fcntl.h"
#include "param.h"
// #include "sys/types.h"
// #include "fs.h"
// #include "defs.h"
// #include "memlayout.h"
#define MAP_PRIVATE 0
#define MAP_SHARED 0
#define MAP_FIXED 0x0000
#define PROT_NONE 0x0
#define MMAPBASE 0x40000000

#ifndef NELEN
#define NELEN(x)(sizeof(x)/sizeof(x[0]))
#endif

#ifndef BSIZE
#define BSIZE 512
#endif

#ifndef PGSIZE 
#define PGSIZE 4096
#endif

#define NTEST 5
#define M2V(x) ((x) + MMAPBASE)    
#define V2M(x) ((x) - MMAPBASE)

char *filename = "README";

int my_strcmp(const char *a, const char *b, int n);


// FILE MAP
void file_invalid_fd_test();
void file_invalid_flags_test();
void file_exceed_size_test();
void file_exceed_count_test();
void file_private_with_fork_test();
void file_shared_with_fork_test();
void file_mapping_with_offset_test();
void file_given_addr_test();
void file_invalid_addr_test();
void file_intermediate_given_addr_test();
void file_exceeds_file_size_test();
void file_mapping_on_wo_file_test();

// MAPPOP | MAPANON도 해줘야지 
void anon_private_test();
void anon_shared_test();
void anon_private_fork_test();
void anon_exceed_size_test();
void anon_exceed_count_test(); 
void anon_zero_size_test();
void anon_private_shared_fork_test();
void anon_intermediate_given_addr_test();



int 
main(int argc, char* argv[])
{

	int fd, pid, print; 
	char * alloc[NTEST]; 
	print = 0;
	// if (argc > 1)
	// print = 0;
	fd = open(filename, O_RDWR); 

	int len[NTEST] = {1*PGSIZE, 2*PGSIZE,   3*PGSIZE, 4*PGSIZE, 5*PGSIZE};
	int idx[NTEST] = {0};
	for (int i = 1; i < (int)NELEN(idx); i++)
	{
		idx[i] = idx[i-1] + len[i-1];
	}
	//= {0,        2*PGSIZE, 3*PGSIZE, 8*PGSIZE, 11*PGSIZE};   
	// fd, anon, fd&pop, anon&pop, fd&pop
	int flag[NTEST] = {0, MAP_ANONYMOUS, MAP_POPULATE, MAP_ANONYMOUS|MAP_POPULATE, MAP_POPULATE};
	int prot[NTEST] = {PROT_READ|PROT_WRITE, PROT_READ|PROT_WRITE, PROT_READ|PROT_WRITE, PROT_READ|PROT_WRITE, PROT_READ|PROT_WRITE};
	int fdarr[NTEST] = {fd, -1, fd, -1, fd};
for (int i = 0;i < (int)NELEN(idx); i++){

      if(i!=-1){
         //printf(1, "%d %d %d %d %d 512\n",idx[i],len[i], prot[i], flag[i], fdarr[i]);
         printf(1, "before mmap: freemem is %d\n", freemem());
         printf(1, "%d: mmap:(M[%d*PGSIZE], %d*PGSIZE, %s, %s)->", i, idx[i]/PGSIZE, len[i]/PGSIZE, flag[i]&MAP_POPULATE ? "pop":"unpop", flag[i]&MAP_ANONYMOUS?"anon":"fd");

         alloc[i] = (char*) mmap((uint)idx[i], len[i], prot[i], flag[i], fdarr[i], 512);
         printf(1, "Mem[%x]\n", alloc[i]);

         printf(1, "after mmap: freemem is %d\n", freemem());
         printf(1, "\n");
      }
	}

   
if (print) {
	printf(1, "\n---------------------------\n");
	alloc[0][4*PGSIZE-1] = '\0';
	printf(1, "%s\n", &alloc[0][3*PGSIZE]);
}

alloc[3][0]  = 'A';
alloc[3][1]  = 'B';
alloc[3][2]  = '\0';
	printf(1, "before fork: freemem is %d\n", freemem());
	pid = fork();

	if (pid) {
		wait();
		printf(1, "\nafter child exit, freemem: %d\n\n", freemem());
		printf(1, "Parent (pid:%d) process\n", getpid()); 
		printf(1, "after fork: freemem is %d \t", freemem());
		printf(1, "\n");
		// pm();
		printf(1,"\n");
	} else {
		// since the real entry of pgdir of child is made on demand 
		alloc[3][2]  = 'C';
		alloc[3][3]  = '\0';
		printf(1, "%s\n", alloc[3]);
		printf(1, "Child (pid:%d) process\n", getpid()); 
		printf(1, "after fork: freemem is %d \t", freemem());
		printf(1, "\n");
		// pm();
		printf(1,"\n");
	}

	for (int i = 0;i < (int)NELEN(idx); i++){
		printf(1, "\n");
		printf(1, "%d: munmap:(M[M2V(%d * PGSIZE)], %d * PGSIZE, %s, %s)\n",
				i, idx[i]/PGSIZE, len[i]/PGSIZE, flag[i]&MAP_POPULATE ?
				"pop":"unpop", flag[i]&MAP_ANONYMOUS?"anon":"fd");

		//if (!(flag[i] & MAP_ANONYMOUS) && (flag[i] & MAP_POPULATE)) {
		//}

		printf(1, "before munmap: freemem is %d\n", freemem());
		if (print) {
         printf(1, "\nasdsad\n\n"); 
		  printf(1, "\n%s\n\n", alloc[i]); 
		  printf(1, "\n");
		}
		printf(1, "unmap_result: %d\n", munmap(M2V(idx[i])));
		printf(1, "after munmap: freemem is %d\n", freemem());
		// pm();
		printf(1, "\n");
	}
	//alloc[3][10] = 0;
   
	if (pid)	
		printf(1, "FINAL freemem: %d\n", freemem());

    close(fd);
	exit();
	// return 0;
}


// int main(){

   
   /*그리고 그냥 flag 0인 경우에 fork 안되는것도 어떻게 하자 좀.
   */

  // file mapping test

   // file_invalid_fd_test();
   // file_invalid_flags_test();
   // file_exceed_size_test();
   // file_exceed_count_test();
   // file_private_with_fork_test();
   // // // //file_shared_with_fork_test(); //무효
   // file_mapping_with_offset_test();
   // file_given_addr_test();
   // file_invalid_addr_test();
   // file_intermediate_given_addr_test();
   // file_exceeds_file_size_test();
   // file_mapping_on_wo_file_test();

   // anon_private_test();
   // anon_shared_test();  // 실패...
   // anon_private_fork_test();
   // anon_exceed_size_test();
   // anon_exceed_count_test();
   // anon_zero_size_test();  // 조건이랑 다름
   // anon_private_shared_fork_test(); // private만 확인 
   // anon_intermediate_given_addr_test();


//    exit();

// }

void anon_intermediate_given_addr_test() {
  printf(1, "anonymous intermediate provided address test\n");
  char *ret = (char *)mmap((uint)0, 1000, PROT_READ | PROT_WRITE,
                           MAP_POPULATE | MAP_ANONYMOUS, -1, 0);
  if (ret == (void *)0) {
    printf(1, "anonymous intermediate provided address test failed: failed at "
              "first mmap\n");
    exit();
  }
  char *ret2 = (char *)mmap((uint)MMAPBASE/2, 200, PROT_READ | PROT_WRITE,
                            MAP_POPULATE | MAP_ANONYMOUS, -1, 0);
  if (ret2 == (void *)0) {
    printf(1, "anonymous intermediate provided address test failed: failed at "
              "second mmap\n");
    munmap((uint)ret);
    exit();
  }
  char *ret3 = (char *)mmap((uint)(MMAPBASE/2+4096), 1000, PROT_READ | PROT_WRITE,
                            MAP_POPULATE | MAP_ANONYMOUS, -1, 0);
  if (ret3 != (char*)((MMAPBASE/2+4096)+MMAPBASE)){
    printf(1, "%d %d anonymous intermediate provided address test failed: failed at "
              "third mmap\n",(uint)ret3, (uint)((MMAPBASE/2+4096)+MMAPBASE));
    munmap((uint)ret);
    munmap((uint)ret2);
    exit();
  }
  int res = munmap((uint)ret);
  if (res == -1) {
    printf(1, "anonymous overlapping provided address test failed: at first "
              "munmap\n");
    munmap((uint)ret2);
    munmap((uint)ret3);
    exit();
  }
  res = munmap((uint)ret2);
  if (res == -1) {
    printf(1, "anonymous overlapping provided address test failed: at second "
              "munmap\n");
    munmap((uint)ret3);
    exit();
  }
  res = munmap((uint)ret3);
  if (res == -1) {
    printf(1, "anonymous overlapping provided address test failed: at third "
              "munmap\n");
    exit();
  }
  printf(1, "anonymous intermediate provided address test ok\n");
}

// fork syscall with anonymous mapping test
void anon_private_shared_fork_test() {
  printf(1, "anonymous private & shared mapping together with fork test\n");
  int size = 200;
  char data1[200], data2[200];
  for (int i = 0; i < size; i++) {
    data1[i] = 'a';
    data2[i] = 'r';
  }
  char *ret = (char *)mmap((uint)0, 200, PROT_READ | PROT_WRITE,
                   MAP_ANONYMOUS | MAP_PRIVATE, -1, 0); // Private mapping
  if (ret == (void *)0) {
    printf(
        1,
        "11anonymous private & shared mapping together with fork test failed\n");
    exit();
  }
  char *ret2 = (char *)mmap((uint)4096, 200, PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0); // Shared mapping
  if (ret2 == (void *)0) {
    printf(
        1,
        "22anonymous private & shared mapping together with fork test failed\n");
    exit();
  }
  int pid = fork();
  if (pid == 0) {
    for (int i = 0; i < size; i++) {
      ret[i] = 'a';
    }
    for (int i = 0; i < size; i++) {
      ret2[i] = 'r';
    }
    exit();
  } else {
    wait();
    // Private mapping
    if (my_strcmp(ret, data1, size) == 0) {
      printf(1, "1-anonymous private & shared mapping together with fork test "
                "failed\n");
      exit();
    }
    // Shared mapping
    if (my_strcmp(ret2, data2, size) != 0) {
      printf(1, "2-anonymous private & shared mapping together with fork test "
                "failed\n");
      exit();
    }
    int res = munmap((uint)ret);
    if (res == -1) {
      printf(1, "3-anonymous private & shared mapping together with fork test "
                "failed\n");
      exit();
    }
    res = munmap((uint)ret2);
    if (res == -1) {
      printf(1, "4-anonymous private & shared mapping together with fork test "
                "failed\n");
      exit();
    }
    printf(1,
           "anonymous private & shared mapping together with fork test ok\n");
  }
}

// mmap when provided size is 0
void anon_zero_size_test(void) {
  printf(1, "anonymous zero size mapping test\n");
  char *ret = (char *)mmap((uint)0, 0, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ret != (void *)0) {
    printf(1, "anonymous zero size mapping test failed\n");
    exit();
  }
  printf(1, "anonymous zero size mapping test ok\n");
}


// anonymous mapping count test when it exceeds 30
void anon_exceed_count_test(void) {
  printf(1, "anonymous exceed mapping count test\n");
  int size = 4096;
  int count = 70;
  uint arr[70];
  int i = 0;
  for (; i < count; i++) {
    void *ret = (void *)mmap((uint)0 + i*size, size, PROT_READ | PROT_WRITE,
                     MAP_ANONYMOUS | MAP_POPULATE, -1, 0);
    arr[i] = (uint)ret;
    if (ret == (void *)0) {
      printf(1, "\n %d \n",i);
      break;
    }
  }
  if (i == 64) {
    for (int j = 0; j < i; j++) {
      int ret = munmap((uint)arr[j]);
      if (ret == -1) {
        printf(1, "anonymous exceed mapping count test failed: at %d munmap\n",
               j);
        exit();
      }
    }
    printf(1, "anonymous exceed mapping count test ok\n");
  } else {
    printf(1, "anonymous exceed mapping count test failed: %d total mappings\n",
           i);
    exit();
  }
}


// anonymous mapping when size exceeds KERNBASE
void anon_exceed_size_test() {
  printf(1, "anonymous exceed mapping size test\n");
  int size = 600 * 1024 * 1024; // 600 MB
  char *ret = (char *)mmap((uint)0, size, PROT_READ | PROT_WRITE,
                           MAP_ANONYMOUS | MAP_POPULATE, -1, 0);
  if (ret != (void *)0) {
    printf(1, "anonymous exceed mapping size test failed\n");
    munmap((uint)ret);
    exit();
  }
  printf(1, "anonymous exceed mapping size test ok\n");
}

// Private mapping with fork
void anon_private_fork_test() {
  printf(1, "anonymous private mapping with fork test\n");
  char temp[200];
  for (int i = 0; i < 200; i++) {
    temp[i] = 'a';
  }
  int size = 200;
  char *ret = (char *)mmap((uint)0, size, PROT_READ | PROT_WRITE,
                   MAP_POPULATE | MAP_ANONYMOUS, -1, 0); // Shared mapping
  if (ret == (void *)0) {
    printf(1, "anonymous private mapping with fork test failed\n");
    exit();
  }
  int pid = fork();
  if (pid == 0) {
    for (int i = 0; i < size; i++) {
      ret[i] = 'a';
    }
    exit();
  } else {
    wait();
    if (my_strcmp(temp, ret, size) == 0) {
      printf(1, "anonymous private mapping with fork test failed\n");
      exit();
    }
    printf(1, "anonymous private mapping with fork test ok\n");
    munmap((uint)ret);
  }
}


// Shared mapping test
void anon_shared_test() {
  printf(1, "anonymous shared mapping test\n");
  int size = 10000;
  int *ret = (int *)mmap((uint)0, size, PROT_READ | PROT_WRITE,
                  MAP_POPULATE|MAP_ANONYMOUS, -1,
                  0); // Shared mapping
   // for (int i = 0; i < size / 4; i++) {
   //    ret[i] = i;
   //    // printf(1, "%d\n",ret[i]);
   //  }
  if (ret == (void *)0) {
    printf(1, "1-anonymous shared mapping test failed\n");
    exit();
  }
  int pid = fork();
  if (pid == 0) {
    for (int i = 0; i < size / 4; i++) {
      ret[i] = i;
      // printf(1, "%d\n",ret[i]);
    }
    exit();
  } else {
    wait();
    for (int i = 0; i < size / 4; i++) {
      // ret[i] = i;
      // printf(1, "%d\n",ret[i]);
    }
    for (int i = 0; i < size / 4; i++) {
      if (ret[i] != i) {
      //   printf(1, "%d\n",i);
        printf(1, "2-anonymous shared mapping test failed\n");
        exit();
      }
    }
    int res = munmap((uint)ret);
    if (res == -1) {
      printf(1, "3-anonymous shared mapping test failed\n");
      exit();
    }
    printf(1, "4-anonymous shared mapping test ok\n");
  }
}

// Simple private anonymous mapping test with maping having both read and write
// permission and size greater than two pages
void anon_private_test() {
  printf(1, "anonymous private mapping test\n");
  int size = 10000;
  int *ret = (int *)mmap((uint)0, size, PROT_READ | PROT_WRITE,
                         MAP_ANONYMOUS | MAP_POPULATE, -1, 0);
  if (ret == (void *)0) {
    printf(1, "1-anonymous private mapping test failed\n");
    exit();
  }
  
  for (int i = 0; i < size / 4; i++) {
    ret[i] = i;
  }

  int res = munmap((uint)ret);
  if (res == -1) {
    printf(1, "2-anonymous private mapping test failed\n");
    exit();
  }
  printf(1, "anonymous private mapping test ok\n");
}


// Trying to map using writeonly file
void file_mapping_on_wo_file_test() {
  printf(1, "file mapping on write only file\n");
  int fd = open(filename, O_WRONLY);
  if (fd == -1) {
    printf(1, "file mapping on write only file failed\n");
    exit();
  }
  int size = 2000;
  char *ret =
      (char *)mmap((uint)0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (ret == (void *)0) {
    printf(1, "file mapping on write only file ok\n");
    return;
  }
  printf(1, "file mapping on write only file failed\n");
  exit();
}

// file backed mapping when mapping size exceeds total file size
void file_exceeds_file_size_test() {
  printf(1, "File exceed file size test\n");
  // README file size 2286 bytes but we are mapping 8000 bytes and then writing
  // upto 8000 bytes in it
  int fd = open("README", O_RDWR);
  if (fd == -1) {
    printf(1, "File exceed file size test failed\n");
    exit();
  }
  char buf[3000];
  int n = read(fd, buf, 2286);
  if (n != 2286) {
    printf(1, "File exceed file size test failed\n");
    exit();
  }
  int size = 8000;
  char *ret =
      (char *)mmap((uint)0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (ret == (void *)0) {
    printf(1, "File exceed file size test failed\n");
    exit();
  }
  if (my_strcmp(ret, buf, 2286) != 0) {
    printf(1, "File exceed file size test failed\n");
    exit();
  }
  int res = munmap((uint)ret);
  if (res == -1) {
    printf(1, "File exceed file size test failed\n");
    exit();
  }
  printf(1, "File exceed file size test ok\n");
}

// test when the file backed mapping is possible between two mappings
void file_intermediate_given_addr_test() {
  printf(1, "file backed intermediate provided address test\n");
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    printf(1, "file backed intermediate provided address test failed\n");
    exit();
  }
  char *ret =
      (char *)mmap((uint)0, 1000, PROT_READ | PROT_WRITE, 0, fd, 0);
  if (ret == (void *)0) {
    printf(1, "file backed intermediate provided address test failed: failed "
              "at first mmap\n");
    exit();
  }
  char *ret2 = (char *)mmap((uint)0x10003000, 200, PROT_READ | PROT_WRITE,
                            0, fd, 0);
  if (ret2 == (void *)0) {
    printf(1, "file backed intermediate provided address test failed: failed "
              "at second mmap\n");
    munmap((uint)ret);
    exit();
  }
  char *ret3 = (char *)mmap((uint)0x10002000, 1000, PROT_READ | PROT_WRITE,
                            0, fd, 0);
  if (ret3 != (char*)(0x10002000+MMAPBASE)) {
    printf(1, "%d file backed intermediate provided address test failed: failed "
              "at third mmap\n",(uint)ret3);
    munmap((uint)ret);
    munmap((uint)ret2);
    exit();
  }
//   char *buf = (char *)ret;  // 매핑된 가상 주소
//     printf(1,"Reading mapped data-buf: ");
//     for (int i = 0; i < 1000; i++) {
//       printf(1,"%c", buf[i]);  // 데이터 읽기
//     }
//    printf(1,"\n");
//    char *buf2 = (char *)ret2;  // 매핑된 가상 주소
//     printf(1,"Reading mapped data-buf: ");
//     for (int i = 0; i < 1000; i++) {
//       printf(1,"%c", buf2[i]);  // 데이터 읽기
//     }
//    printf(1,"\n");
//    char *buf3 = (char *)ret3;  // 매핑된 가상 주소
//     printf(1,"Reading mapped data-buf: ");
//     for (int i = 0; i < 1000; i++) {
//       printf(1,"%c", buf3[i]);  // 데이터 읽기
//     }
//    printf(1,"\n");
  int res = munmap((uint)ret);
  if (res == -1) {
    printf(1, "file backed overlapping provided address test failed: at first "
              "munmap\n");
    munmap((uint)ret2);
    munmap((uint)ret3);
    exit();
  }
  res = munmap((uint)ret2);
  if (res == -1) {
    printf(1, "file backed overlapping provided address test failed: at second "
              "munmap\n");
    munmap((uint)ret3);
    exit();
  }
  res = munmap((uint)ret3);
  if (res == -1) {
    printf(1, "file backed overlapping provided address test failed: at third "
              "munmap\n");
    exit();
  }
  close(fd);
  printf(1, "file backed intermediate provided address test ok\n");
}


// file backed mmap when provided address is less than MMAPBASE
void file_invalid_addr_test(void) {
  printf(1, "file backed invalid provided address test\n");
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    printf(1, "1-file backed invalid provided address test failed\n");
    exit();
  }
  char *ret = (char *)mmap((uint)0, 203, PROT_READ | PROT_WRITE,
                           MAP_POPULATE, fd, 0);
  if (ret != (void *)0) {
    printf(1, "2-file backed invalid provided address test failed\n");
    munmap((uint)ret);
    exit();
  }
  printf(1, "3-file backed invalid provided address test ok\n");
  close(fd);
}

// file backed mmap when the valid address is provided by user
void file_given_addr_test() {
  printf(1, "file backed valid provided address test\n");
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    printf(1, "file backed shared apping with fork test failed\n");
    exit();
  }
  char *ret = (char *)mmap((uint)0x60001000, 200, PROT_READ | PROT_WRITE,
                           0, fd, 0);

   char *buf = (char *)ret;  // 매핑된 가상 주소
    printf(1,"Reading mapped data-buf: ");
    for (int i = 0; i < 200; i++) {
      printf(1,"%c", buf[i]);  // 데이터 읽기
    }
   printf(1,"\n");
  if (ret == (void *)0) {
    printf(1, "file backed valid provided address test failed: at mmap\n");
    exit();
  }
  int res = munmap((uint)ret);
  if (res == -1) {
    printf(1, "file backed valid provided address test failed: at munmap\n");
    exit();
  }
  close(fd);
  printf(1, "file backed valid provided address test ok\n");
}

// Simple private file backed mapping test
void file_mapping_with_offset_test() {
  printf(1, "file backed mapping with offset test\n");
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    printf(1, "file backed mapping with offset test failed: at open\n");
    exit();
  }
  int size = 1000;
  char buf[1000];
  int n = read(fd, buf, 200); // Move to offset 200
  if (n != 200) {
    printf(1, "file backed mapping with offset test failed: at read\n");
    exit();
  }
  if (read(fd, buf, size) != size) {
    printf(1, "file backed mapping with offset test failed: at read\n");
    exit();
  }
  printf(1,"Reading mapped data-buf1: \n");
    for (int i = 0; i < 50; i++) {
      printf(1,"%c", buf[i]);  // 데이터 읽기
    }
   printf(1,"\nReading mapped data-done: \n");
  // Offset is 200
//   close(fd);
  int fd2 = open(filename, O_RDWR);
  char *ret = (char *)mmap((uint)0, size, PROT_READ | PROT_WRITE, MAP_POPULATE,
                           fd2, 200);
   char* data = ret;
   printf(1,"Reading mapped data-buf2: \n");
    for (int i = 0; i < 50; i++) {
      printf(1,"%c", data[i]);  // 데이터 읽기
    }
   printf(1,"\nReading mapped data-done: \n");
  if (ret == (void *)0) {
    printf(1, "1-file backed mapping with offset test failed\n");
    exit();
  }
  if (my_strcmp(buf, ret, size) != 0) {
    printf(1, "2-file backed mapping with offset test failed\n");
    exit();
  }
  for (int i = 0; i < 40; i++) {
    ret[i] = 'p';
    buf[i] = 'p';
  }
  if (my_strcmp(buf, ret, size) != 0) {
    printf(1, "3-file backed mapping with offset test failed\n");
    exit();
  }
  int res = munmap((uint)ret);
  if (res == -1) {
    printf(1, "4-file backed mapping with offset test failed\n");
    exit();
  }
  close(fd);
  printf(1, "file backed mapping with offset test ok\n");
}



// shared file backed mapping with fork test
void file_shared_with_fork_test() {
  printf(1, "file backed shared mapping with fork test\n");
  int size = 200;
  char buf[200];
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    printf(1, "1-file backed shared apping with fork test failed\n");
    exit();
  }
  if (read(fd, buf, size) != size) {
    printf(1, "2-file backed shared mapping with fork test failed\n");
    exit();
  }
  char *ret2 = (char*)mmap((uint)0, size, PROT_READ | PROT_WRITE, MAP_POPULATE, fd, 0);
  int pid = fork();
  if (pid == 0) {
    for (int i = 0; i < 50; i++) {
      ret2[i] = 'o';
    }
    if (my_strcmp(ret2, buf, size) == 0) {
      printf(1, "3-file backed shared mapping with fork test failed\n");
      exit();
    }
    exit();
  } else {
    wait();
    // The data written in child process should persist here

    
    if (my_strcmp(ret2, buf, size) == 0) {
      printf(1, "4-file backed shared mapping with fork test failed\n");
      exit();
    }
    int res = munmap((uint)ret2);
    if (res == -1) {
      printf(1, "5-file backed shared mapping with fork test failed\n");
      exit();
    }
    close(fd);
    // Check if data is written into file
    int fd2 = open(filename, O_RDWR);
    for (int i = 0; i < 50; i++) {
      buf[i] = 'o';
    }
    char buf2[200];
    if (read(fd2, buf2, size) != size) {
      printf(1, "6-file backed shared mapping with fork test failed\n");
      exit();
    }
    // char *buf = (char *)ret2;  // 매핑된 가상 주소
    printf(1,"Reading mapped data-buf: ");
    for (int i = 0; i < 200; i++) {
      printf(1,"%c", buf[i]);  // 데이터 읽기
    }
   printf(1,"\n");
   printf(1,"Reading mapped data-buf2: ");
    for (int i = 0; i < 200; i++) {
      printf(1,"%c", buf2[i]);  // 데이터 읽기
    }
   printf(1,"\n");
    if (my_strcmp(buf2, buf, size) != 0) {
      printf(1, "7-file backed shared mapping with fork test failed\n");
      exit();
    }
    printf(1, "8-file backed shared mapping with fork test ok\n");
    close(fd2);
  }
}

// private file backed mapping with fork test
void file_private_with_fork_test() {
  printf(1, "file backed private mapping with fork test\n");
  int size = 200;
  char buf[200];
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    printf(1, "file backed private mapping with fork test failed\n");
    exit();
  }
  if (read(fd, buf, size) != size) {
    printf(1, "file backed private mapping with fork test failed\n");
    exit();
  }
  // char *ret = (char*)mmap((uint)0, 200, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  /* 잠깐 여기서 일단 MAP_POPULATE 먼저 테스트 해주는데 
     문제가 MAP_POP없이 0으로 들어올때 PAGEFAULT 핸들러가 작업을 제대로 안해주는것 같다.
  */
  close(fd);
  fd = open(filename, O_RDWR);
  char *ret = (char*)mmap((uint)0, 200, PROT_READ | PROT_WRITE, MAP_POPULATE, fd, 0);
//   char *data = (char *)buf;  // 매핑된 가상 주소
//     printf(1,"Reading mapped data: ");
//     for (int i = 0; i < 50; i++) {
//       printf(1,"%c", data[i]);  // 데이터 읽기
//     }
//    printf(1,"\n");
//   printf(1,"%d\n",(uint)ret);
  int pid = fork();
//   char *data2 = (char *)ret;  // 매핑된 가상 주소
//     printf(1,"Reading mapped data2: ");
//     for (int i = 0; i < 50; i++) {
//       printf(1,"%c", data2[i]);  // 데이터 읽기
//     }
//    printf(1,"\n");
  
  
  
  if (pid == 0) {
    
    for (int i = 0; i < 50; i++) {
      ret[i] = 'n';
    }
   //  printf(1, "%s\n", buf);
    
    // The mapping should not be same as we have edited the data
    if (my_strcmp(ret, buf, size) == 0) {
      printf(1, "file backed private mapping with fork test failed\n");
      exit();
    }
    exit();
  } else {
    wait();
    // As it is private mapping therefore it should be same as read
   //  printf(1,"%d\n",(uint)ret);
   //  char *data3 = (char *)ret;  // 매핑된 가상 주소
   //  printf(1,"Reading mapped data3: ");
   //  for (int i = 0; i < 50; i++) {
   //    printf(1,"%c", data3[i]);  // 데이터 읽기
   //  }
   // printf(1,"\n");
    if (my_strcmp(ret, buf, size) != 0) {
      printf(1, "file backed private mapping with fork test failed\n");
      exit();
    }
    //printf(1, "PASS\n");
    int res = munmap((uint)ret);
    if (res == -1) {
      printf(1, "file backed private mapping with fork test failed\n");
      exit();
    }
    printf(1, "file backed private mapping with fork test ok\n");
  }
}


// file backed mapping count test when it exceeds 30
void file_exceed_count_test() {
  printf(1, "file backed exceed mapping count test\n");
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    printf(1, "file backed exceed mapping count test failed\n");
    exit();
  }

  fd = -1;  // for annon-test
  int size = 4096;
  int count = 70;
  uint arr[70];
  int i = 0;
  for (; i < count; i++) {
   //  printf(1,"%d\n",i*size);
    void *ret = (void *)mmap((uint)(i*size), size, PROT_READ | PROT_WRITE,
                     MAP_ANONYMOUS | MAP_PRIVATE, fd, 0);
    arr[i] = (uint)ret;
   //  printf(1,
   //             "how many maps?: %d mmap\n",
   //             i+1);
    if (ret == (void *)0) {
      break;
    }
  }
  if (i == 64) {
    for (int j = 0; j < i; j++) {
      int ret = munmap((uint)arr[j]);
      if (ret == -1) {
        printf(1,
               "file backed exceed mapping count test failed: at %d munmap\n",
               j);
        exit();
      }
    }
    printf(1, "file backed exceed mapping count test ok\n");
    close(fd);
  } else {
    printf(1,
           "1-file backed exceed mapping count test failed: %d total mappings\n",
           i);
    exit();
  }
}


// file backend mapping when size exceeds KERNBASE
void file_exceed_size_test() {
  printf(1, "file backed exceed mapping size test\n");
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    printf(1, "file backed exceed mapping size test failed: at open\n");
    exit();
  }
  int size = 600 * 1024 * 1024; // 600 MB
  char *ret = (char *)mmap((uint)0, size, PROT_READ | PROT_WRITE,
                           MAP_ANONYMOUS | MAP_PRIVATE, fd, 0);
  if (ret != (void *)0) {
    printf(1, "file backed exceed mapping size test failed\n");
    munmap((uint)size);
    exit();
  }
  close(fd);
  printf(1, "file backed exceed mapping size test ok\n");
}

// When invalid fd is provided to mapping
void file_invalid_fd_test() {
  printf(1, "file backed mapping invalid file descriptor test\n");
  int size = 100;
  int fds[3] = {
      -1, 10,
      18}; // Negative fd, fd in range but does not exist, fd out of range
  for (int i = 0; i < 3; i++) {
    char *ret = (char *)mmap((uint)0, size, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE, fds[i], 0);
    if (ret == (void *)0) {      // 우리의 test는 fail이 0이니까
      continue;
    }
    printf(1, "file backed mapping invalid file descriptor test failed\n");
    exit();
  }
  printf(1, "file backed mapping invalid file descriptor test ok\n");
}

// When invalid flags are provided to mapping
void file_invalid_flags_test() {
  printf(1, "file backed mapping invalid flags test\n");
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    printf(1, "file backed mapping invalid flags test failed: at open\n");
    exit();
  }
  int size = 100;
  char *ret = (char *)mmap((uint)0, size, PROT_READ | PROT_WRITE, 12, fd, 0);
  if (ret == (void *)0) {
    printf(1, "file backed mapping invalid flags test ok\n");
    close(fd);
    return;
  }
  printf(1, "file backed mapping invalid flags test failed\n");
  exit();
}


// Utility strcmp function
int my_strcmp(const char *a, const char *b, int n) {
  for (int i = 0; i < n; i++) {
    if (a[i] != b[i]) {
      return 1;
    }
  }
  return 0;
}