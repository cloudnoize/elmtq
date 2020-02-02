#include<condition_variable>
#include<mutex>


template<int N,typename T>
class mtQ {
private:
	T arr[N];
	int reader = 0;
	int writer = 0;
	int size = N;
	int stored = 0;
	std::mutex m;
	std::condition_variable cv;
public:
	mtQ(){};
	mtQ(const mtQ<N,T>&) = delete;
    mtQ& operator=(const mtQ& rhs) = delete;

	void pop(T& o){
		std::unique_lock<std::mutex> ul(m);
		cv.wait(ul,[this]{ return stored > 0 ;});
		o = arr[reader];
		reader = (++reader%size);
		--stored;
		cv.notify_one();
	}	

	void push(T& o){
		std::unique_lock<std::mutex> ul(m);
                cv.wait(ul,[this]{ return stored < size;});
		arr[writer] = o ;
		writer = ++writer%size;
		++stored;
		cv.notify_one();		
	}
};
