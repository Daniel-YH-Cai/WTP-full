all: wReceiver wSender

wReceiver:
	g++ -g -o wReceiver wReceiver.cpp

wSender:
	g++ -g -o wSender wSender.cpp

runS:
	./wSender 127.0.0.1 8080 5 bin.txt sender_log.txt

runR:
	./wReceiver 8080 5 /home/daniel-cai/Desktop/CSCI4430/Assignment3/p3/WTP-opt/tmp receiver_log.txt
clean:
	rm -f  wReceiver wSender
