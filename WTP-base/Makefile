all: wReceiver wSender TimeoutDemo seekg

wReceiver:
	g++ -g -o wReceiver wReceiver.cpp

wSender:
	g++ -g -o wSender wSender.cpp

runSender:wSender
	./wSender 127.0.0.1 8080 5 bin.txt sender_log.txt

runReceiver:wReceiver
	./wReceiver 8080 5 /home/daniel-cai/Desktop/CSCI4430/Assignment3/p3/WTP-base/tmp receiver_log.txt

demo:
	g++ -g -o TimeoutDemo TimeoutDemo.cpp

give_permission:
	chmod u+x ./grader/tests

sanity:
	 ./grader/tests loss

seekg:
	g++ -g -o seekg seekg.cpp;
clean:
	rm -f  wReceiver wSender TimeoutDemo seekg