#include "CurrentTime.h"
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
using namespace std;

int main(int argc, char **argv){

	double beg = currentTime();
	double begMS = currentTimeMS();
	cout<<"now time(seconds) = "<<beg<<endl;
	cout<<"now time(MS) = "<<currentTimeMS()<<endl;
	cout<<"now sleep 1 second ..."<<endl;
	sleep(1);
	cout<<"time cast = "<<currentTime()-beg<<endl;
	cout<<"time case in (MS) = "<<currentTimeMS() - begMS<<endl;
	return 0;
}
