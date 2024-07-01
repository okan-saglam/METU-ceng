#include <iostream>
// #include <stdlib.h>
#include <vector>
#include <queue>
// #include <pthread.h>
#include "monitor.h"
#include "helper.c"
#include "WriteOutput.c"

using namespace std;

enum Direction{
    ZEROTOONE,
    ONETOZERO
};

// CLASSES

class PathElement{
    public:
        string connector;
        int from;
        int to;
};

class Car{
    public:
        int id;
        int travel_time_between_connectors;
        int path_length;
        // vector<PathElement> path;
        PathElement path[1000];
        pthread_t thread_id;
};

class Connectors: public Monitor{
    public:
        int id;
        int travel_time;
        int max_wait_time;
        virtual void pass(){
            __synchronized__;
        }
};

class NarrowBridge: public Connectors{
    public:
        Condition waitingToPass[2] = {this, this};

        queue<int> waitingCars[2];

        int current_direction = -1;
        int number_of_car_onBridge = 0;
        bool direction_changed = false;
        bool overtime = false;
        unsigned long long now = 0;

        void pass(int current_car_id, int from){
            __synchronized__;
            
            auto findTimeDifference = [](unsigned long long present){
                return GetTimestamp() - present;
            };

            if(current_direction == -1){
                now = GetTimestamp();
                overtime = false;
                current_direction = from;
            }

            int to = !from;
            waitingCars[from].push(current_car_id);
            WriteOutput(current_car_id, 'N', this->id, ARRIVE);
            
            again:
            while(current_car_id != waitingCars[from].front() || (number_of_car_onBridge && ((current_direction != from) || (current_direction == from && overtime)))){
                waitingToPass[from].wait();
                if(current_car_id == waitingCars[from].front()){
                    break;    
                } 
            }
            if(!direction_changed && number_of_car_onBridge && (findTimeDifference(now) + PASS_DELAY) < max_wait_time){
                mutex.unlock();
                sleep_milli(PASS_DELAY);
                mutex.lock();
            }
            else if(!direction_changed && number_of_car_onBridge && (findTimeDifference(now) + PASS_DELAY) >= max_wait_time){
                waitingToPass[from].wait();
                goto again;
            }
            direction_changed = false;

            waitingCars[from].pop();
            number_of_car_onBridge++;
            waitingToPass[from].notifyAll();
            if((findTimeDifference(now) + travel_time) >= this->max_wait_time){
                overtime = true;
            }
        
            WriteOutput(current_car_id, 'N', this->id, START_PASSING);
            mutex.unlock();
            sleep_milli(travel_time);
            mutex.lock();
            WriteOutput(current_car_id, 'N', this->id, FINISH_PASSING);
            number_of_car_onBridge--;
        
            if(waitingCars[from].empty() && !number_of_car_onBridge && findTimeDifference(now) < max_wait_time){
                if(waitingCars[to].empty()){
                    current_direction = -1;
                }
                else{
                    now = GetTimestamp();
                    overtime = false;
                    current_direction = to;
                    direction_changed = true;
                    waitingToPass[to].notifyAll();
                }
            }
            else if(waitingCars[to].size() && !number_of_car_onBridge){
                // cout << findTimeDifference(now) << endl;
                // if(!number_of_car_onBridge){
                //     now = GetTimestamp();
                //     overtime = false;
                //     current_direction = to;
                //     direction_changed = true;
                //     waitingToPass[to].notifyAll();
                // }
                if(findTimeDifference(now) >= this->max_wait_time){
                    now = GetTimestamp();
                    overtime = false;
                    current_direction = to;
                    direction_changed = true;
                    waitingToPass[to].notifyAll();
                }
            }
            else{
                waitingToPass[from].notifyAll();
                // Buraya now = GetTimestamp(); gelebilir
                // now = GetTimestamp();
            }
        }
};

class Ferrie: public Connectors{
    public:
        Condition waitingToPass[2] = {this, this};
        
        int capacity;
        bool isMaxTime = false;
        int number_of_car_onBoat[2] = {0,0};
        int temp[2] = {0, 0};
        int emptyFlag[2] = {0, 0};
        
        void pass(int current_car_id, int from){
            __synchronized__;
            
            number_of_car_onBoat[from]++;
            if(number_of_car_onBoat[from] == capacity && !emptyFlag[from]){
                emptyFlag[from] = 1;
                waitingToPass[from].notify();
            }
            else if(emptyFlag[from]){
                waitingToPass[from].wait();
            }
            
            if(number_of_car_onBoat[from] == 1){
                emptyFlag[from] = 0;
                timespec mwt;
                clock_gettime(CLOCK_REALTIME, &mwt);
                mwt.tv_sec = mwt.tv_sec + max_wait_time / 1000;
                mwt.tv_nsec = mwt.tv_nsec + (max_wait_time % 1000) * 1000000;
                if(mwt.tv_nsec >= 1000000000){
                    mwt.tv_sec++;
                    mwt.tv_nsec -= 1000000000;
                }
                waitingToPass[from].timedwait(&mwt);
                isMaxTime = true;
                waitingToPass[from].notify();
            }
            else{
                waitingToPass[from].wait();
                waitingToPass[from].notify();
            }
            
            number_of_car_onBoat[from]--;
            if(!number_of_car_onBoat[from]){
                emptyFlag[from] = 0;
                // temp[from] = 1;
                waitingToPass[from].notify();
            }

            WriteOutput(current_car_id, 'F', this->id, START_PASSING);
            mutex.unlock();
            sleep_milli(this->travel_time);
            mutex.lock();
            WriteOutput(current_car_id, 'F', this->id, FINISH_PASSING);






            // if(emptyFlag[from]){
            //     // cout << "here" << endl;

            //     waitingToPass[from].wait();
            // }

            // number_of_car_onBoat[from]++;
            // if(number_of_car_onBoat[from] == 1){
            //     emptyFlag[from] = 0;
            //     timespec mwt;
            //     clock_gettime(CLOCK_REALTIME, &mwt);
            //     mwt.tv_sec = mwt.tv_sec + max_wait_time / 1000;
            //     mwt.tv_nsec = mwt.tv_nsec + (max_wait_time % 1000) * 1000000;
            //     if(mwt.tv_nsec >= 1000000000){
            //         mwt.tv_sec++;
            //         mwt.tv_nsec -= 1000000000;
            //     }

            //     // if(waitingToPass[from].timedwait(&mwt) == ETIMEDOUT) isMaxTime = true;
            //     waitingToPass[from].timedwait(&mwt);
            //     isMaxTime = true;
            // }
                
            // if(number_of_car_onBoat[from] != capacity && !isMaxTime){
            //     waitingToPass[from].wait();
            // }
            // else if(number_of_car_onBoat[from] != capacity && isMaxTime){
            //     waitingToPass[from].notifyAll();
            //     isMaxTime = false;
            // }
            // else if(number_of_car_onBoat[from] == capacity){
            //     emptyFlag[from] = 1;
            //     waitingToPass[from].notifyAll();
            // } 
            // // else{
            // //     waitingToPass[from].wait();
            // // }
            

            // // else waitingToPass[from].notifyAll();

            // number_of_car_onBoat[from]--;
            // // cout << number_of_car_onBoat[from] << endl;
            // if(!number_of_car_onBoat[from]){
            //     emptyFlag[from] = 0;
            //     waitingToPass[from].notifyAll();
            // }
            
            // WriteOutput(current_car_id, 'F', this->id, START_PASSING);
            // mutex.unlock();
            // sleep_milli(this->travel_time);
            // mutex.lock();
            // WriteOutput(current_car_id, 'F', this->id, FINISH_PASSING);

// ---------------------------------------------------------------------------

            // number_of_car_onBoat[from]++;
            // temp[from]++;

            // if(number_of_car_onBoat[from] == 1){
            //     timespec mwt;
            //     clock_gettime(CLOCK_REALTIME, &mwt);
            //     mwt.tv_sec = mwt.tv_sec + max_wait_time / 1000;
            //     mwt.tv_nsec = mwt.tv_nsec + (max_wait_time % 1000) * 1000000;
            //     if(mwt.tv_nsec >= 1000000000){
            //         mwt.tv_sec++;
            //         mwt.tv_nsec -= 1000000000;
            //     }

            //     if(waitingToPass[from].timedwait(&mwt) == ETIMEDOUT) isMaxTime = true;
            // }
            // while(1){
            //     if(number_of_car_onBoat[from] == capacity && !isMaxTime){
            //         WriteOutput(current_car_id, 'F', this->id, START_PASSING);
            //         waitingToPass[from].notifyAll();
            //         mutex.unlock();
            //         sleep_milli(this->travel_time);
            //         mutex.lock();
            //         WriteOutput(current_car_id, 'F', this->id, FINISH_PASSING);
            //         break;
            //     }
            //     else if(isMaxTime){
            //         waitingToPass[from].notifyAll();
            //         isMaxTime = false;
            //     }
            //     else{
            //         waitingToPass[from].wait();
            //     }
            // }
            // temp[from]--;
            // cout << temp[from] << endl;
            // if(temp[from] == 0){
            //     number_of_car_onBoat[from] = 0;
            // }
        }
};

class CrossRoad: public Connectors{
    public:
        Condition waitingToPass[4] = {this, this, this, this};

        queue<int> waitingCars[4];

        int current_direction = -1;
        int number_of_car_onRoad = 0;
        bool direction_changed = false;
        bool overtime = false;
        unsigned long long now = 0;

        void pass(int current_car_id, int from){
            __synchronized__;
            waitingCars[from].push(current_car_id);
            WriteOutput(current_car_id, 'C', this->id, ARRIVE);
            
            auto findTimeDifference = [](unsigned long long present){
                return GetTimestamp() - present;
            };

            if(current_direction == -1){
                now = GetTimestamp();
                overtime = false;
                current_direction = from;
            }


            again:
            while(current_car_id != waitingCars[from].front() || (number_of_car_onRoad && ((current_direction != from) || (current_direction == from && overtime)))){
                waitingToPass[from].wait();
                if(current_car_id == waitingCars[from].front()){
                    break;    
                } 
            }
            if(!direction_changed && number_of_car_onRoad && (findTimeDifference(now) + PASS_DELAY) < max_wait_time){
                mutex.unlock();
                sleep_milli(PASS_DELAY);
                mutex.lock();
            }
            else if(!direction_changed && number_of_car_onRoad && (findTimeDifference(now) + PASS_DELAY) >= max_wait_time){
                waitingToPass[from].wait();
                goto again;
            }
            direction_changed = false;
            
            waitingCars[from].pop();
            number_of_car_onRoad++;
            WriteOutput(current_car_id, 'C', this->id, START_PASSING);
            waitingToPass[from].notifyAll();
            if((GetTimestamp() + travel_time - now) > this->max_wait_time) overtime = true;
            mutex.unlock();
            sleep_milli(travel_time);
            mutex.lock();
            WriteOutput(current_car_id, 'C', this->id, FINISH_PASSING);
            number_of_car_onRoad--;

// && findTimeDifference(now) < max_wait_time

            if(waitingCars[from].empty() && !number_of_car_onRoad){
                int count=0;
                for(int i=from+1 ; ; i++){
                    count++;
                    if(waitingCars[i % 4].size()){
                        now = GetTimestamp();
                        overtime = false;
                        current_direction = i % 4;
                        direction_changed = true;
                        waitingToPass[i % 4].notifyAll();
                        break;
                    }
                    if(count == 3){
                        current_direction = -1;
                        break;
                    }
                }


            }
            else if(waitingCars[from].size() && findTimeDifference(now) < max_wait_time){
                waitingToPass[from].notifyAll();
            }
            // waitingCars[from].size() &&
            else if(findTimeDifference(now) >= this->max_wait_time && !number_of_car_onRoad){
                int temp = current_direction;
                int count = 0;
                for(int i=from+1 ; ; i++){
                    count++;
                    if(waitingCars[i % 4].size()){
                        now = GetTimestamp();
                        overtime = false;   
                        current_direction = i % 4;
                        if(temp != current_direction) direction_changed = true;
                        waitingToPass[i % 4].notifyAll();
                        break;
                    }
                    else{
                        if(count == 3){
                            now = GetTimestamp();
                            waitingToPass[from].notifyAll();
                            break;
                        }
                    }
                }
            }
            else{
                waitingToPass[from].notifyAll();
            }
        }
};

// GLOBALS

// vector<NarrowBridge*> nbs;
// vector<Ferrie*> fs;
// vector<CrossRoad*> crs;
// vector<Car> cars;

// vector<NarrowBridge> nbs;
// vector<Ferrie> fs;
// vector<CrossRoad> crs;
// vector<Car> cars;

NarrowBridge nbs[100];
Ferrie fs[100];
CrossRoad crs[100];
Car cars[1000];



// HELPERS

void divideString(const string pathElement, string& connector, int& id){
    int length;
    string sub1, sub2;
    
    length = pathElement.length();
    sub1 = pathElement.substr(0,1);
    sub2 = pathElement.substr(1,length);

    connector = sub1;
    id = stoi(sub2);
}

void* passFromConnector(void* args){
    Car * my_car;
    my_car = (Car *) args;

    int i;
    int pathLength = my_car->path_length;

    string connector_type[pathLength];
    int connector_id[pathLength];

    for(i=0 ; i<pathLength ; i++){
        divideString(my_car->path[i].connector, connector_type[i], connector_id[i]);
    }

    for(i=0 ; i<pathLength ; i++){
        int current_car_direction = my_car->path[i].from;
        if(connector_type[i] == "N"){
            WriteOutput(my_car->id, 'N', connector_id[i], TRAVEL);
            sleep_milli(my_car->travel_time_between_connectors);
            // WriteOutput(my_car->id, 'N', connector_id[i], ARRIVE);

            nbs[connector_id[i]].pass(my_car->id, current_car_direction);

        }
        else if(connector_type[i] == "F"){
            WriteOutput(my_car->id, 'F', connector_id[i], TRAVEL);
            sleep_milli(my_car->travel_time_between_connectors);
            WriteOutput(my_car->id, 'F', connector_id[i], ARRIVE);
            fs[connector_id[i]].pass(my_car->id, current_car_direction);
        }
        else{
            WriteOutput(my_car->id, 'C', connector_id[i], TRAVEL);
            sleep_milli(my_car->travel_time_between_connectors);
            // WriteOutput(my_car->id, 'C', connector_id[i], ARRIVE);

            crs[connector_id[i]].pass(my_car->id, current_car_direction);

        }
    }

    pthread_exit(NULL);
}

int main(){
    int i;
    int number_of_nb;
    int number_of_f;
    int number_of_cr;
    int number_of_cars;
    
    // TAKING THE INPUT:

    cin >> number_of_nb;
    // cout << number_of_nb;
    // nbs = (NarrowBridge*) malloc(number_of_nb * sizeof(NarrowBridge));
    for(i=0 ; i<number_of_nb ; i++){
        // NarrowBridge element;
        // element.id = i;
        // cin >> element.travel_time;
        // cin >> element.max_wait_time;
        // nbs.push_back(element);
        // nbs[i] = element;

        nbs[i].id = i;
        cin >> nbs[i].travel_time;
        cin >> nbs[i].max_wait_time;

    }

    cin >> number_of_f;
    // fs = (Ferrie*) malloc(number_of_f * sizeof(Ferrie));
    for(i=0 ; i<number_of_f ; i++){
        // Ferrie element;
        // element.id = i;
        // cin >> element.travel_time;
        // cin >> element.max_wait_time;
        // cin >> element.capacity;
        // fs.push_back(element);
        // fs[i] = element;

        fs[i].id = i;
        cin >> fs[i].travel_time;
        cin >> fs[i].max_wait_time;
        cin >> fs[i].capacity;
    }

    cin >> number_of_cr;
    // crs = (CrossRoad*) malloc(number_of_cr * sizeof(CrossRoad));
    for(i=0 ; i<number_of_cr ; i++){
        // CrossRoad element;
        // element.id = i;
        // cin >> element.travel_time;
        // cin >> element.max_wait_time;
        // crs.push_back(element);
        // crs[i] = element;

        crs[i].id = i;
        cin >> crs[i].travel_time;
        cin >> crs[i].max_wait_time;
    }
    
    cin >> number_of_cars;
    // Car cars[number_of_cars];
    for(i=0 ; i<number_of_cars ; i++){
        int j;
        // Car element;
        // element.id = i;
        // cin >> element.travel_time_between_connectors;
        // cin >> element.path_length;

        cars[i].id = i;
        cin >> cars[i].travel_time_between_connectors;
        cin >> cars[i].path_length;

        // element.path = (PathElement*) malloc((element.path_length) * sizeof(PathElement));
        for(j=0 ; j<cars[i].path_length ; j++){
            // PathElement path_element;
            // cin >> path_element.connector;
            // cin >> path_element.from;
            // cin >> path_element.to;

            cin >> cars[i].path[j].connector;
            cin >> cars[i].path[j].from;
            cin >> cars[i].path[j].to;

            // cars[i].path.push_back(path_element);
        }
        // cars.push_back(element);
        // cars[i] = element;
    }

    // PRINTING THE INPUT:

    // cout << number_of_nb << endl;
    // for(i=0 ; i<number_of_nb ; i++){
    //     cout << nbs[i].id << " " << nbs[i].travel_time << " " << nbs[i].max_wait_time << endl;
    // }
    // cout << "Bridges ended" << endl;

    // cout << endl;

    // cout << number_of_f << endl;
    // for(i=0 ; i<number_of_f ; i++){
    //     cout << fs[i].id << " " << fs[i].travel_time << " " << fs[i].max_wait_time << " " << fs[i].capacity << endl;
    // }
    // cout << "Ferries ended" << endl;

    // cout << endl;

    // cout << number_of_cr << endl;
    // for(i=0 ; i<number_of_cr ; i++){
    //     cout << crs[i].id << " " << crs[i].travel_time << " " << crs[i].max_wait_time << endl;
    // }
    // cout << "Cross Roads ended" << endl;
    
    // cout << endl;

    // cout << number_of_cars << endl;
    // for(i=0 ; i<number_of_cars ; i++){
    //     cout << cars[i].id << " " << cars[i].path_length << endl;
    //     for(int j=0 ; j<cars[i].path_length ; j++){
    //         cout << cars[i].path[j].connector << " " << cars[i].path[j].from << " " << cars[i].path[j].to << endl;
    //     }
    // }

    InitWriteOutput();
    
    for(i=0 ; i<number_of_cars ; i++){
        pthread_create(&cars[i].thread_id, NULL, passFromConnector, (void *) &cars[i]);
    }

    for(i=0 ; i<number_of_cars ; i++){
        pthread_join(cars[i].thread_id, NULL);
    }
    
    return 0;
}