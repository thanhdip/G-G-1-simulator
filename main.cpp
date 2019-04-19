//GG1 Sumulator
#include <iostream>
#include <queue>
#include <vector>
#include <random>
#include <fstream>

using namespace std;

struct job
{
    int arrivalTime;
    int departureTime;
    int timeInQueue;
    int timeInServer; 
};

struct server
{
    job currentJob;
    int serviceTime;
    int timeEmpty;
    bool empty;
};

int nextJobTime(int type, mt19937& gen)
{
    exponential_distribution<double> exponen(.005); // Mean = 200lps = 1/(lambda = .005). Every 200lps or 2 seconds
    uniform_int_distribution<int> uniform(100,300); //100lps to 300lps. Mean 200lps or 2 seconds

    if(type == 0)
    {
        return exponen(gen);
    }
    else if(type == 1)
    {
        return 200;
    }
    else if(type == 2)
    {
        return uniform(gen);
    }
    return -1;
}

int getSeviceTime(int type, mt19937& gen)
{
    exponential_distribution<double> exponen(.01); // Mean = 100 = 1/(lambda = .01) Every 100lps or 1 second
    uniform_int_distribution<int> uniform(100,200); //100lps to 200lps. Mean 100 lps or 1 second

    if(type == 0)
    {
        return exponen(gen);
    }
    else if(type == 1)
    {
        return 100;
    }
    else if(type == 2)
    {
        return uniform(gen);
    }
    return -1;
}

/* The simulation will be a loop where 1 increment of the loop is 1/100 of a second.
 * Which means every 100 loops is a second. 100lps = 1sec
 */

int main()
{
    //Output to csv to make data easier to manage
    ofstream csvFile;
    csvFile.open("poissonInterarrival.csv");

    //RNG
    random_device rd;
    mt19937 gen(rd());

    //Queue
    queue<job> jobQueue;
    vector<job> jobDone;
    vector<int> queueSizes;
    int maxQLen = 0;

    //Server
    server serv;
    serv.empty = true;
    serv.timeEmpty = 0;

    //Setup
    int interArrivalType = 0; //0 - Exponential, 1 - constant, 2 - Uniform
    int serviceTimeType = 0;
    int runtime = 10000000; //10,000,000lps = 100,000sec


//SIMULATOR
//=======================================================================================================================================
    int nextJob  = 0; //Time where first job will come in
    for(int currentTime = 0; currentTime < runtime; currentTime++)
    {
    //QUEUE
        //Check if it is time to add job to queue
        nextJob--;
        if(nextJob <= 0)
        {
            nextJob = nextJobTime(interArrivalType,gen);
            job newJob;
            newJob.arrivalTime = currentTime; //Set arrivalTime
            jobQueue.push(newJob); 
        }

        queueSizes.push_back(jobQueue.size());
        if(maxQLen < jobQueue.size()){maxQLen = jobQueue.size();}

    //SERVER
        if(serv.empty)
        {
            if(jobQueue.empty())
            {
                serv.timeEmpty++;
            }
            else
            {
                serv.serviceTime = getSeviceTime(serviceTimeType, gen);
                serv.currentJob = jobQueue.front();
                serv.empty = false;
                jobQueue.pop();
                serv.currentJob.timeInQueue = currentTime - serv.currentJob.arrivalTime; //Set timeInQueue
            }
        }
        else
        {
            serv.serviceTime--;
            if(serv.serviceTime <= 0)
            {
                serv.currentJob.departureTime = currentTime; //Set departureTime
                serv.currentJob.timeInServer = (serv.currentJob.departureTime - serv.currentJob.arrivalTime) - serv.currentJob.timeInQueue;
                jobDone.push_back(serv.currentJob);
                serv.empty = true;
            }   
        }
        
    }

//=======================================================================================================================================

    double interarrivalTimeTotal = 0;
    double serviceTimeTotal = 0;
    double responseTimeTotal = 0;

    int prev = 0;

    for(job j : jobDone)
    {
        interarrivalTimeTotal += j.arrivalTime - prev;
        prev = j.arrivalTime;
        serviceTimeTotal += j.timeInServer;
        responseTimeTotal += j.departureTime - j.arrivalTime;
    }

    double queueSizesTotal;
    for(int q : queueSizes)
    {
        queueSizesTotal += q;
    }    

    std::cout << "Max Queue Length: " << maxQLen << endl;
    std::cout << "Avg Queue Length: " << queueSizesTotal/(double)queueSizes.size() << endl;
    std::cout << "Response Time: " << responseTimeTotal/(double)jobDone.size() << endl;
    std::cout << "Service Time: " << serviceTimeTotal/(double)jobDone.size() << endl;
    std::cout << "Interarival Time: " << interarrivalTimeTotal/(double)jobDone.size() << endl;
    std::cout << "Server Util: " << 1 - (serv.timeEmpty/(double)runtime) << endl;
    std::cout << "# Jobs Done: " << jobDone.size() << endl;
    std::cout << "# Jobs In Queue: " << jobQueue.size() << endl;

    csvFile.close();
    return 0;
}