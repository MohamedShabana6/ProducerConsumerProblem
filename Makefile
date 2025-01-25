all: producer consumer  

producer : producer.cpp
	@g++  -o producer producer.cpp  
	
consumer : consumer.cpp
	@g++  -o consumer consumer.cpp 


clean:
	@ipcs 
	@rm -f producer
	@rm -f consumer