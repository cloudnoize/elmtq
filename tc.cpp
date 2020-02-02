#include <boost/asio.hpp>
#include <iostream>
#include<thread>
#include<vector>
#include<condition_variable>
#include<mutex>
#include<boost/array.hpp>
#include<elmtq.hpp>
#include<string>
using namespace boost;


class cmd{
	private:
		char command;
		std::string key;
	public:
		explicit cmd(const char* buf,const int size) throw(std::string){
			if(size < 4){
				throw "buf too snall";
			}
			for(int i = 0 ; i < size ;++i){
				char type = buf[i];
				switch (type){
					case 0:
						command = buf[++i];
						break;
					case 1:
						auto l = *(short*)(&buf[++i]);
						if(l+i > size){
							throw "oor";
						}
						key.reserve(l);
						for(auto& c: key){
							c = buf[++i];
						}
						break;
				}
			}
		}

		void print(){
			std::string scmd;
			switch(command){
				case 0:
					scmd = "get";
					break;
				case 1:
					scmd = "set";
					break;
			}
			std::cout << " cmd is " << scmd << " cmd payload " << key << std::endl;
		}


};


void handler(mtQ<4,asio::ip::tcp::socket*>& sq){
	std::cout << "thread " << std::this_thread::get_id() << "\n";
	while(true){
		asio::ip::tcp::socket* usocket;
		sq.pop(usocket);
		std::string sbuf;
		sbuf.reserve(128);
		char buf[128];
      		boost::system::error_code error;
		while(true){
			try{
      				size_t len = usocket->read_some(boost::asio::buffer(buf,128), error);
				cmd c(buf,len);
				c.print();
			}catch(...){
				std::cout << "caught ex\n";
				break;
			}
			
		}
	
	}

}

int main()
{
  // The size of the queue containing the pending connection
  // requests.
  const int BACKLOG_SIZE = 30;

  // Step 1. Here we assume that the server application has
  // already obtained the protocol port number.
  unsigned short port_num = 3333;

  // Step 2. Creating a server endpoint.
  asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(),
    port_num);

  asio::io_service ios;

  mtQ<4,asio::ip::tcp::socket*> sq;

  std::vector<std::thread> vt;
  for(int i = 0 ; i < 5 ; i++){
	std::thread t(handler,std::ref(sq));
	vt.push_back(std::move(t));
  }

  try {
    // Step 3. Instantiating and opening an acceptor socket.
    asio::ip::tcp::acceptor acceptor(ios, ep.protocol());

    // Step 4. Binding the acceptor socket to the 
    // server endpint.
    acceptor.bind(ep);

    // Step 5. Starting to listen for incoming connection
    // requests.
    acceptor.listen(BACKLOG_SIZE);


    // Step 7. Processing the next connection request and 
    // connecting the active socket to the client.i
    while(true){
	 // Step 6. Creating an active socket.
   	auto sock = new  asio::ip::tcp::socket(ios);
    	acceptor.accept(*sock);

    	sq.push(sock);
    }

    for(auto&  t : vt){
    	t.join();
    }
    // At this point 'sock' socket is connected to 
    //the client application and can be used to send data to
    // or receive data from it.
  }
  catch (system::system_error &e) {
    std::cout << "Error occured! Error code = " << e.code()
      << ". Message: " << e.what();

    return e.code().value();
  }

  return 0;
}
