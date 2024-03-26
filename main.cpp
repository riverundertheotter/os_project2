/*
 * Programming Assignment 2
 * River Cook
 * CS4348 Operating Systems Dr. Mittal
 *
 * ----------------------------------------------------------------------------
 */

#include <iostream>
#include <atomic>
#include <cstdlib>
#include <thread>
#include <vector>
#include <cmath>
#include <memory>
#include <ctype.h>

#include "my_make_unique.h"

class PetersonLock {
public: 
	std::atomic<bool> flag[2];
	std::atomic<int> turn;

	PetersonLock() {
		turn = 0;
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
			std::this_thread::yield();
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
	TournamentTree(int n) : thread_count(n) {
		int depth = std::ceil(std::log2(n));
		int size = std::pow(2, depth + 1) - 1;
		for (int i = 0; i < size; i++) {
			tree.push_back(make_unique<PetersonLock>()); // create PetersonLock instances
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
		std::cout << "Thread " << thread_id << " has acquired lock.\n";
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
		std::cout << "Thread " << thread_id << " is returning the lock.\n";
	}
};

class TestAndSet {
	std::atomic<bool> flag;

public:
	TestAndSet() {
		flag = false;
	}
	void lock(int thread_id) {
		bool expected = false;
		// busy wait loop
		while (!flag.compare_exchange_strong(expected, true, std::memory_order_acquire)) {
			expected = false; // reset expected since modified by compare_exchange_strong
			std::this_thread::yield();
		}
		std::cout << "Thread " << thread_id << " has secured lock.\n";
	}

	void unlock(int thread_id) {
		flag.store(false, std::memory_order_release);
		std::cout << "Thread " << thread_id << " has released lock.\n";
	}
};

class FetchAndIncrement {
public:	
	std::atomic<int> token{0};
	std::atomic<int> turn{0};

	void lock(int thread_id) {
		// fetch current token num and increment for next thread
		int myturn = token.fetch_add(1, std::memory_order_relaxed);

		// busy wait until thread's turn
		while (turn.load(std::memory_order_acquire) != myturn) {
			std::this_thread::yield(); // yield to avoid busy waiting too aggressively.
		}
		std::cout << "Thread " << thread_id <<  " has secured lock.\n";

	}

	void unlock(int thread_id) {
		// move to next thread
		turn.fetch_add(1, std::memory_order_release);
		std::cout << "Thread " << thread_id << " has released lock.\n";
	}
};

void critical_sectionFAI(int thread_id, FetchAndIncrement& lock) {
	lock.lock(thread_id);
	std::cout << "Thread " << thread_id << " is in critical section.\n";
	lock.unlock(thread_id);
}

void critical_sectionTAS(int thread_id, TestAndSet& lock) {
	lock.lock(thread_id);
	std::cout << "Thread " << thread_id << " is in critical section.\n";
	lock.unlock(thread_id);
}

void critical_sectionTT(int thread_id, TournamentTree& tree) {
	tree.lock(thread_id);
	std::cout << "Thread " << thread_id << " is in critical section.\n";
	tree.unlock(thread_id);
}

// checks validity of args
int CheckArgs(int argc, char*argv[]) {
	// right num of arguments?
	if (argc != 3) {
		std::cout << "Usage: main <algorithm_type (int)> <nthreads(int)>\n";
		return 1;
	}
	if (std::atoi(argv[1]) < 0 || std::atoi(argv[1]) > 2) {
		std::cout << "Algorithm choices\n0: Test Tree\n1: Test and Set\n2:Fetch and Increment\n";
		return 1;
	} else if (std::atoi(argv[2]) <= 0) {
		std::cout << "Must give 1 or more threads to main.\n";
		return 1;
	}
	return 0;
}

void RunTournamentTree(int nthreads) {
	std::cout << "Entering the tournament tree with " << nthreads << " threads.\n";
	TournamentTree tree(nthreads);
	std::vector<std::thread> threads;

	for (int i = 0; i < nthreads; i++) {
		threads.emplace_back(critical_sectionTT, i, std::ref(tree));
	}

	for (auto& thread: threads) {
		thread.join();
	}		 
}

void RunTestAndSet(int nthreads) {
	std::cout << "Entering Test and Set with " << nthreads << " threads.\n";
	TestAndSet lock;
	std::vector<std::thread> threads;
	for (int i = 0; i < nthreads; i++) {
		threads.emplace_back(critical_sectionTAS, i, std::ref(lock));
	}

	for (auto& thread : threads) {
		thread.join();
	}

}

void RunFetchAndIncrement(int nthreads) {
	std::cout << "Entering Fetch and Increment with " << nthreads << " threads.\n";
	FetchAndIncrement lock;
	std::vector<std::thread> threads;

	for (int i = 0; i < nthreads; i++) {
		threads.emplace_back(critical_sectionFAI, i, std::ref(lock));
	}

	for (auto& thread: threads) {
		thread.join();
	}
}

int main(int argc, char *argv[]) {
	int args_valid = CheckArgs(argc, argv);
	if (args_valid == 1) {
		return 1;
	}

	int nthreads = std::atoi(argv[2]);
	int algorithm_type = std::atoi(argv[1]);

	switch(algorithm_type) {
		case 0: {
				RunTournamentTree(nthreads);
				break;
		}
		case 1: {
				RunTestAndSet(nthreads);
				break;
		}
		case 2: {
				RunFetchAndIncrement(nthreads);
				break;
		}
	}

	return 0;
}
