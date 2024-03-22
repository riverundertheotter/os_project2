/*
 * Programming Assignment 2
 * River Cook
 * CS4348 Operating Systems Dr. Mittal
 *
 * Takes two arguments:
 * - Algorithm Type (int 0-2)
 * - n Threads (int 0 -> inf)
 *
 * implements mututal exclusion for n threads. using Petersen's algorithm
 * need to use the atomic library, make shared variables atomic type.
 * can generalize for n threads.
 *
 * apparently, TAS and FAI instructions are implemented somehow 
 * in the atomic library?
 * 
 * needs to run on cs1 cs2 servers
 * ----------------------------------------------------------------------------
 * Petersen's Algorithm
 *
 * shared variables: Boolean flag[2] = {false, false}; Integer turn;
 * (announce intention to other processes)
 * flag[myid] = true;
 * (yield to other processes)
 * turn = 1 - myid;
 * // wait to see if other process also wants the lock and it is not your turn
 * while (flag[1 - myid] and (turn != myid)) do 
 * begin 
 *  ; (critical section?
 * end while
 * release : flag[myid] = false
 * ----------------------------------------------------------------------------
 * Tournament Tree 
 *
 * 	With N leaf nodes
 * 		each internal tree node is an instance of Petersen's algorithm
 * 		each process is mapped to a distinct leaf node
 * 	Ascend the tree, acquiring lock at each internal node until the root
 * 	node is locked
 *
 * 	Descend the tree, releasing the lock at each internal node in the path
 * 	starting from the root node (reverse of previous step)
 * ----------------------------------------------------------------------------
 * These two methods employ read-modify-write (RMW) instructions
 * Perhaps this is what Dr. Mittal meant by the atomic library?
 * ----------------------------------------------------------------------------
 * Test and Set 
 * 
 * shared var Boolean lock = false;
 * ACQUIRE: 
 * while TAS(lock) do
 * begin 
 * 	// lock is not free
 * end while
 * RELEASE: 
 * // resets lock
 * lock = false
 * ----------------------------------------------------------------------------
 * Fetch and Increment shared var Integer token = 0, turn = 0;
 * 
 * ACQUIRE:
 * // get a token number
 * myturn = FAI(token);
 * while turn != myturn do
 * begin 
 * 	// lock is not free
 * 	;
 * end while
 *
 * RELEASE
 * // advance turn
 * turn = turn + 1;
 * ----------------------------------------------------------------------------
 */
