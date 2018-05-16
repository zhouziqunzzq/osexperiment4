/*************************************************************************
    > File Name: main.cpp
    > Author: Bittersweet
    > ################### 
    > Mail: zhouziqun@cool2645.com
    > Created Time: 2018年05月15日 星期二 22时15分57秒
 ************************************************************************/
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <ctime>

using namespace std;

class PageHandler {
    protected:
        vector<int> mFrame;    // A vector representing frames in memory
        int maxFrame;   // Max frame number
        int nowFrame;   // mFrame[0:nowFrame] is used
        int disaffect;  // Not hit counter
    public:
        PageHandler() {};
        PageHandler(int frameNum);
        virtual ~PageHandler() {}
        void handle(int *accessSeries, int totalInstruction);
        virtual void handleSingleHit(int pageID, int pos) = 0;
        virtual void printName() = 0;
        void printMFrame();
};

void PageHandler::printMFrame() {
    cout << "Frames in memory: ";
    for (int i = 0; i < nowFrame; ++i) {
        cout << mFrame[i] << " ";
    }
    cout << endl;
}

PageHandler::PageHandler(int frameNum) : mFrame(frameNum) {
    this->maxFrame = frameNum;
    this->nowFrame = 0;
    this->disaffect = 0;
}

void PageHandler::handle(int *accessSeries, int totalInstruction) {
    for (int i = 0; i < totalInstruction; ++i) {
        int pageID = accessSeries[i];
        cout << "==================";
        printName();
        cout << "==================" << endl;
        cout << "Accessing page " << pageID << endl;
        auto rst = find(mFrame.begin(), mFrame.begin() + nowFrame + 1, pageID);
        if (rst == mFrame.begin() + nowFrame + 1) {
            cout << "Miss!" << endl;
            ++disaffect;
            if (nowFrame < maxFrame) {
                mFrame[nowFrame++] = pageID;
            } else {
                mFrame.push_back(pageID);
                mFrame.erase(mFrame.begin());
            }
        } else {
            cout << "Hit!" << endl;
            handleSingleHit(pageID, distance(mFrame.begin(), rst));
        }
        printMFrame();
        //sleep(0.1);
    }
    cout << "================Summary of ";
    printName();
    cout << "================" << endl;
    cout << "Total instructions: " << totalInstruction << endl;
    cout << "Disaffect count: " << disaffect << endl;
    cout << "Hit rate: " << 1 - (double)disaffect / (double)totalInstruction << endl;
    cout << "Miss rate: " << (double)disaffect / (double)totalInstruction << endl;
    cout << endl;
    exit(0);
}

class FIFOPageHandler : public PageHandler {
    public:
        FIFOPageHandler(int frameNum) : PageHandler(frameNum) {
            return;
        }
        void handleSingleHit(int pageID, int pos);
        void printName();
};

void FIFOPageHandler::handleSingleHit(int pageID, int pos) {
    return;
}

void FIFOPageHandler::printName() {
    cout << "FIFOPageHandler";
}

class LRUPageHandler : public PageHandler {
    public:
        LRUPageHandler(int frameNum) : PageHandler(frameNum) {
            return;
        }
        void handleSingleHit(int pageID, int pos);
        void printName();
};

void LRUPageHandler::handleSingleHit(int pageID, int pos) {
    cout << "nowFrame: " << nowFrame << endl;
    mFrame.insert(mFrame.begin() + nowFrame, pageID);
    mFrame.erase(mFrame.begin() + pos);
}

void LRUPageHandler::printName() {
    cout << "LRUPageHandler";
}

int randIn(int min, int max) {
    return min + rand() % (max - min);
}

int main() {
    //LRUPageHandler handler(3);
    //int test[5] = {1, 2, 3, 1, 1};
    //handler.handle(test, 5);
    
    int pid, status; // used for wait()

    srand(time(0));
    cout << "Please input the size of the Frame in memory: ";
    int frameCnt;
    cin >> frameCnt;
    cout << "Please input the number of the access series: ";
    int insCnt;
    cin >> insCnt;
    cout << "Please input the maxium of the random number: ";
    int maxRand;
    cin >> maxRand;
    
    cout << "Generating access series..." << endl;
    int *accessSeries = new int[insCnt];
    for (int i = 0; i < insCnt; ++i) {
        accessSeries[i] = randIn(1, maxRand);
        cout << accessSeries[i] << " ";
    }
    cout << endl;

    pid_t fpid1, fpid2;
    fpid1 = fork();
    if (fpid1 < 0) {
        cout << "Failed to fork, exiting..." << endl;
        exit(-1);
    } else if (fpid1 == 0) {
        // Child process 1
        FIFOPageHandler fifoHandler(frameCnt);
        fifoHandler.handle(accessSeries, insCnt);
    } else {
        pid = wait(&status);
        cout << "Child process " << pid << " exit with status " << status << "." << endl;
        fpid2 = fork();
        if (fpid2 < 0) {
            cout << "Failed to fork, exiting..." << endl;
            exit(-1);
        } else if (fpid2 == 0) {
            // Child process 2
            LRUPageHandler lruHandler(frameCnt);
            lruHandler.handle(accessSeries, insCnt);
        }
    }
    
    pid = wait(&status);
    cout << "Child process " << pid << " exit with status " << status << "." << endl;

    delete[] accessSeries;
    return 0;
}
