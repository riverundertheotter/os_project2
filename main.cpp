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

#include <iostream>
#include <atomic>
#include <cstdlib>
#include <thread>
#include <vector>
#include <cmath>
#include <memory>

class PetersonLock {
public: 
	std::atomic<bool> flag[2];
	std::atomic<int> turn;

	PetersonLock() : turn(0){
		flag[0] = false;
		flag[1] = false;
	}

	void lock(int thread_id) {
		// using modulo for binary tree
		int other = 1 - thread_id % 2;
		flag[thread_id % 2] = true;
		turn = thread_id % 2;
		while (flag[other] && turn == thread_id % 2) {
			// busy wait
		}
	}

	void unlock(int thread_id) {
		flag[thread_id % 2] = false;
	}
};

class TournamentTree {
	std::vector<std::unique_ptr<PetersonLock>> tree;
	int height;
	int thread_count;

public:
	//
	// what is this line doing?
	//
	TournamentTree(int n) : thread_count(n) {
		int depth = std::ceil(std::log2(n));
		int size = std::pow(2, depth + 1) - 1; //
		for (int i = 0; i < size; i++) {
			tree.push_back(std::make_unique<PetersonLock>()); // create PetersonLock instances
		}
		height = depth;
	}

	void lock(int thread_id) {
		int node = thread_id + std::pow(2, height) - 1; // convert thread_id to leaf index
		while (node > 0) {
			int parent = (node - 1) / 2;
			tree[parent]->lock(thread_id);
			node = parent;
		}
	}

	void unlock(int thread_id) {
		int node = 0; // starting at root
		std::vector<int> path;
		// compute path from root to leaf
		for (int level = 0; level < height; level++) {
			path.push_back(node);
			node = node * 2 + 1 + (thread_id >> (height - level - 1) & 1);
		}

		for (auto it = path.rbegin(); it != path.rend(); it++) {
			tree[*it]->unlock(thread_id);
		}
	}
};

void critical_section(int thread_id, TournamentTree& tree) {
	tree.lock(thread_id);
	std::cout << "Thread " << thread_id << " locked.\n";

	// critical section
	std::cout << "Thread " << thread_id << " is in critical section.\n";

	tree.unlock(thread_id);
	std::cout << "Thread " << thread_id << " has released lock.\n";
}

// checks validity of args
int CheckArgs(int argc, char*argv[]) {
	// right num of arguments?
	if (argc != 3) {
		std::cout << "Usage: main <algorithm_type (int)> <nthreads(int)>\n";
		return 1;
	}
	// valid argument type?
	if (std::atoi(argv[1]) < 0 || std::atoi(argv[1]) > 2) {
		std::cout << "Algorithm choices\n0: Test Tree\n1: Test and Set\n2:Fetch and Increment\n";
		return 1;
	}
	return 0;
}

void TestAndSet() {
	return;
}

void FetchAndIncrement() {
	return;
}

int main(int argc, char *argv[]) {
	int args_valid = CheckArgs(argc, argv);
	if (args_valid == 1) {
		return 1;
	}

	int nthreads = std::atoi(argv[2]);
	int algorithm_type = std::atoi(argv[1]);

	switch(algorithm_type) {
		case 0: 
				std::cout << "Entering the tournament tree with " << nthreads << " threads.\n";
				TournamentTree tree(nthreads);
				std::vector<std::thread> threads;

				for (int i = 0; i < nthreads; i++) {
					threads.emplace_back(critical_section, i, std::ref(tree));
				}

				for (auto& t: threads) {
					t.join();
				}

				break;
	//	case 2: //Test_And_Set();
	//	case 3: //Fetch_And_Increment();
	}

	return 0;
}
