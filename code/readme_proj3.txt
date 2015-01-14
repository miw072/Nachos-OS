1. Implement demand paging

Since part 2 is built on and changes part 1, our code here reflects later parts' need.
We first modified AddrSpace initialization to only create virtual page table and PTEs(valid bit to be FALSE), left out the zero_out_physical_page and preload_code_n_data_from_file for another method to handle. For this part, it assumes enough memory is available, so preventing programs from proceeding without enough memory is set aside, in our project code, since part 2 handles this by evicting a physical page, prevention is no longer needed. We did allocate physical page as mentioned in problem statements for part 1. However, since allocation will be put into page fault handler in part 2, our project code reflect on the later parts' changes. After we paged in the faulted page, we set the PTE as valid for program to continue. 

In addrspace.cc, FaultPageIn(), LoadHelper(), LocateSegment() are made for part 1 and following parts. For page fault handling, for those pages that are not in the backstore(as in part 2, since backstore is large enough, this is only called at the very beginning to fill up available physical page), FaultPageIn() will be called. FaultPageIn() reads bad address, gets its VPN, zero out its physical page, calls LocateSegment() to specify load occasion, call LoadHelper() to read executable into physical page based on different occasions. Read is first done on code, then if the page is not filled up, it is done on initData. The mechanism is for each faulted page, check if the code and initData in executable needs to be loaded(only these two need loaded if needed, zero out at the beginning are for uninitData and stack). LocateSegment() is used to see where a segment(code or initData) is located related to faulted page (0&5: outside, exclusive; 1: segment starts outside page, ends inside page; 2: segment includes page; 3: segment is included in page; 4: segment starts inside page, ends outside page). LoadHelper() is used to load the executable into physical page based on the cases got from LocateSegment(). Only case 1-4 lead to loading value. 

We used tests from project 2, and it passed the tests. Since part2 will change this part, the general test for this project will be done in later parts and the results should reflect on the correctness of this part.





2. Implement demand paged virtual memory with page replacement

To help with the page replacement. We have two tables initialized in Progtest.cc that track the owners of the physical frames. One table contains ptes that owns the frames and the other contains the pid.

Now We first define a new class BackingStore in the Pcb. In BackingStore class, we have a backstore file to store all the pages that are replaced. The size of the file equals to PageSize*numPages (numPages means ptes in pagetalbe). We also have a shadow table in the BackingStore class. The elements in this table ,which are bool variables, marks whether the pte related to that element is stored in the backstore file. We use pid of each process as the file name of the backstore file. 

The key method in this class is PageIn, PageOut. PageIn will use the pte passed in to calculate the offset in the backstore file. Then it will locate the content belong to the pte and copy that to the physical Page. Then we set the pte.valid TRUE and keep track of the owener. PageOut will first check shadow table and dirty bit. If we don't store it, we make a copy. If we have it stored and it is dirty we also copy it. Otherwise we do nothing.  Finally we set the pte.valid as FALSE.

Then we make some changes to the PageFault handler.  When a PageFault comes, we first try to allocate a physical to it. If there is a free frame, we give it to the pte that triggers the fault. If we don'  have physical frames left we use findvictim to get a victim frames. Then we Pageout the frame and store the content to the owner of the frame. Then we we give the frame to the pte. After we map a physical frame to the pte, we check whether the pte is already backstored and decide whether to copy from backstore using PageIn or from executable using FaultPageIn in problem 1.

In the findvictim function, we apply three replacement policy. In default, we use random. If you give extra argument '0', we use FIFO.( ./nachos -x ../test/sort 0)If you give '1', we use second chance algrithm.

The test results will listed in part3 and part4.





3.Testing
a). First, after we finishing task 2---page replacement, we test our code with few number of phypages. The test program needs much more pages than we offer. We adjust number of phypages to 5.
    For all test programs we completed in project2, we can run them correctly now.
    For some programs that need lots of memory, for example, sort.c, matmult.c,etc. We can run them correctly now. But for we are using random or fifo replacement policy, when number of phypages is small, it takes a lot of time to finish the programs. This proves that our code support page relacement correctly (at least we believe) for single process now.
    For multi-processes condition, we use shell.c to test our code. In shell.c, it calls Join and Exec system call to achieve multi-processes. We first run shell.c, and type ../test/sort.c, then we got the correctly result of sort.c. After this, shell.c continued. Then we also test matmult.c correctly. This proves that our code support multi-processes for the page replacement condition.
b). Then we test our code to demonstrate that the OS only initialize the pages the user programs refrence to. To explain it clearly, let's forget about the the replacement(we have already proven we made it). And only concentrate on the page fault number and page in number. We adjust number of phypages to 32. Note that for all test programs we write, we create an array with 256 int element which needs 8 pages in total(this part of data is in data section, not stack). First, we run somepage.c with commenting all instructions in main function, we get 3 page fault and 2 page in. Then we reference a[0], we get 4 page fault and 3 page in. Then we reference a[230] which is on another page, we get 5 page fault and 4 page in. This proves that every time we run a user program, OS handles page fault and only initialize the pages we reference to.  Furthermore, we test allpage.c(reference all pages of data) and we get 12 page fault and 11 page in.
c). Next we shrink number of phypages to 2 and use test program goodlocal.c, poorlocal.c and randlocal.c to represent programs that (1) generate good locality (most references are to a small subset of pages), and (2) generate poor locality (everything is referenced repeatedly in a pattern), and (3) random locality (pages are effectively referenced randomly). All these programs need more than 2 pages. We can run these programs correctly and this demonstrates the page replacement policy and correct handling of dirty pages.




4. 
In this part, we added two counters to the Statistics class, numPageOuts and numPageIns to count the number of pagein and number of pageout when page fault exceptions occur. This number will display as the machine halt. The following table is a summary of the value of paging counters with FIFO and random replacement algorithm regarding different test programs.

P.S: (set Tab Width 4 to get nice table) 
-------------------------------------------------------
Physical memory size: 32 pages.						  |
Page replacement policy: Random.					  |
-------------------------------------------------------
Program	  |	 PageFaults	 |	 PageOuts	|	PageIns   |
-------------------------------------------------------
halt      |      3       |		0		|	   2	  |
-------------------------------------------------------   	
matmult   |		 99		 |		46		|	   61	  |
-------------------------------------------------------
sort 	  |		 956	 |		843		|	   923	  |
-------------------------------------------------------
allpage	  |		 12		 |		0		|	   11	  |
-------------------------------------------------------   	
somepage  |		 5		 |		0		|	   4	  |
-------------------------------------------------------
goodlocal |		 5  	 |		0		|	   4	  |
-------------------------------------------------------
poorlocal |		 12  	 |		0		|	   11	  |
-------------------------------------------------------
randlocal |		 7  	 |		0		|	   6	  |
-------------------------------------------------------

-------------------------------------------------------
Physical memory size: 32 pages.						  |
Page replacement policy: FIFO.						  |
-------------------------------------------------------
Program	  |	 PageFaults	 |	 PageOuts	|	PageIns   |
-------------------------------------------------------
halt	  |		 3		 |		0		|	   2	  |
-------------------------------------------------------   	
matmult   |		 107	 |		54		|	   69	  |
-------------------------------------------------------
sort 	  |		 4832	 |		4366	|	   4799	  |
-------------------------------------------------------
allpage	  |		 12		 |		0		|	   11	  |
-------------------------------------------------------   	
somepage  |		 5		 |		0		|	   4	  |
-------------------------------------------------------
goodlocal |		 5  	 |		0		|	   4	  |
-------------------------------------------------------
poorlocal |		 12  	 |		0		|	   11	  |
-------------------------------------------------------
randlocal |		 7  	 |		0		|	   6	  |
-------------------------------------------------------

-------------------------------------------------------
Physical memory size: 32 pages.						  |
Page replacement policy: LRU approximation(SC).		  |
-------------------------------------------------------
Program	  |	 PageFaults	 |	 PageOuts	|	PageIns   |
-------------------------------------------------------
halt	  |		 3		 |		0		|	   2	  |
-------------------------------------------------------   	
matmult   |		 98 	 |		51		|	   60	  |
-------------------------------------------------------
sort 	  |		 3080	 |		2841	|	   3047	  |
-------------------------------------------------------
allpage	  |		 12		 |		0		|	   11	  |
-------------------------------------------------------   	
somepage  |		 5		 |		0		|	   4	  |
-------------------------------------------------------
goodlocal |		 5  	 |		0		|	   4	  |
-------------------------------------------------------
poorlocal |		 12  	 |		0		|	   11	  |
-------------------------------------------------------
randlocal |		 7  	 |		0		|	   6	  |
-------------------------------------------------------
