#include <iostream>
#include <fstream>

#ifdef USE_PYTHON_MODULE
#include "PythonModule/PythonModule.h"
#endif

#ifdef USE_CUDA_MODULE
#include "CUDAModule/CUDAModule.h"
#endif

#include <ServerInterface.h>
#include <weerror.h>
#include <WorldModel.h>
#include "DecisionModule.h"
#include <VisionReceiver.h>

#include <GDebugEngine.h>
#include "bayes/MatchState.h"
#include "Global.h"
#include "DefendUtils.h"
#include "Compensate.h"
#include <mutex>
#include "CollisionDetect.h"
#include "CommandInterface.h"
#include "parammanager.h"
#include "zss_debug.pb.h"
#include "Semaphore.h"
#include "RefereeBoxIf.h"
#include "ActionModule.h"
#include <QCoreApplication>
#ifdef _OPENMP
#include "omp.h"
#endif
/*! \mainpage Zeus - Run for number one
*
* \section Introduction
*
*	ZJUNlict is robot soccer team of Robocup in Small Size League.
*
* \section Strategy
*	Frame : GameState -> Play -> Agent -> Skill -> Motion Control -> Wireless
*
* \subsection step1: GameState
* \subsection step2: Play
* \subsection step3: Agent
* \subsection step4: Skill
* \subsection step5: Motion Control : PathPlan and Trajectory Generation
*
* etc...
*/

extern Semaphore visionEvent;
extern std::mutex decisionMutex;
/// <summary> For GPU. </summary>
std::mutex* _best_visiondata_copy_mutex = nullptr;
std::mutex* _value_getter_mutex = nullptr;

using Param::Latency::TOTAL_LATED_FRAME;

bool VERBOSE_MODE = true;
bool IS_SIMULATION = false;
bool wireless_off = false;
bool record_run_pos_on = false;
namespace {
COptionModule *option;
//VisionReceiver *receiver;
CDecisionModule *decision;
CActionModule *action;
CServerInterface::VisualInfo visionInfo;
//RefRecvMsg refRecvMsg;
}

int runLoop() {
#ifdef USE_PYTHON_MODULE
    PythonModule::instance();
#endif
    ZSS::ZParamManager::instance()->loadParam(IS_SIMULATION, "Alert/IsSimulation", false);
    initializeSingleton();
    option = new COptionModule();
    CCommandInterface::instance(option);
//    receiver = VisionReceiver::instance(option);
    vision->registerOption(option);
    vision->startReceiveThread();
    decision = new CDecisionModule(vision);
    action = new CActionModule(vision, decision);
    WORLD_MODEL->registerVision(vision);
//    MATCH_STATE->initialize(option, vision);
    _best_visiondata_copy_mutex = new std::mutex();
    _value_getter_mutex = new std::mutex();
//    GPUBestAlgThread::Instance()->initialize(VisionModule::Instance());
//    CollisionDetect::Instance()->initialize(VisionModule::Instance());
    RefereeBoxInterface::Instance();
#ifdef USE_CUDA_MODULE
    ZCUDAModule::instance()->initialize(VisionModule::Instance());
#endif
    while (true) {
        vision->setNewVision();
#ifdef USE_PYTHON_MODULE
        PythonModule::instance()->run();
#endif
//        decisionMutex.lock();
        decision->DoDecision(false);
        if (! wireless_off) {
            action->sendAction();
        } else {
            action->sendNoAction();
        }
//        decisionMutex.unlock();
//        GDebugEngine::Instance()->send(IS_SIMULATION && option->MyColor() == TEAM_YELLOW); // Simulation Yellow no debug messages
        GDebugEngine::Instance()->send(option->MyColor() == TEAM_BLUE); //Show two teams debug messages
//        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
int main(int argc, char* argv[]) {
    QCoreApplication a(argc, argv);
    std::thread t(runLoop);
    t.detach();
#ifdef _OPENMP
    std::cout << "Using OpenMP" << std::endl;
    omp_set_num_threads(3);
#endif
    return a.exec();
}
