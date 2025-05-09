#include <iostream>

#include "LaunchCommandReceiver.h"

int main() {

    //송수신 모듈 만들기
    LaunchCommand::LaunchCommandReceiver LaunchCommandReceiverImpl("127.0.0.1", 8080);
    LaunchCommandReceiverImpl.start();
    
    return 0;
}
