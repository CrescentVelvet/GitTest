TEMPLATE = app

CONFIG += c++11

QT += core qml quick network

CONFIG += c++11 console

DEFINES += QT_DEPRECATED_WARNINGS

# python module for windows and linux
# if you change this switch, you may need to clear the cache
#DEFINES += USE_PYTHON_MODULE
DEFINES += USE_CUDA_MODULE
#DEFINES += USE_OPENMP

ZSS_LIBS =
ZSS_INCLUDES =

CONFIG(debug, debug|release) {
    #CONFIG += console
    TARGET = MedusaD
    DESTDIR = $$PWD/../ZBin
    MOC_DIR = ./temp
    OBJECTS_DIR = ./temp
}
CONFIG(release, debug|release) {
    TARGET = Medusa
    DESTDIR = $$PWD/../ZBin
    MOC_DIR = ./temp
    OBJECTS_DIR = ./temp
}

win32 {
    if(contains(DEFINES,USE_OPENMP)){
        QMAKE_CXXFLAGS += -openmp
    }
    win32-msvc*:QMAKE_CXXFLAGS += /wd"4819"
    # Third party library dir
    THIRD_PARTY_DIR = $$PWD/../ZBin/3rdParty
    contains(QMAKE_TARGET.arch, x86_64){
        message("64-bit")
        CONFIG(release,debug|release){
            ZSS_LIBS += $${THIRD_PARTY_DIR}/protobuf/lib/x64/libprotobuf.lib \
                        $${THIRD_PARTY_DIR}/tolua++/lib/x64/tolua++.lib \
                        $${THIRD_PARTY_DIR}/lua/lib/x64/lua5.1.lib
        }
        CONFIG(debug,debug|release){
            ZSS_LIBS += $${THIRD_PARTY_DIR}/protobuf/lib/x64/libprotobufd.lib \
                        $${THIRD_PARTY_DIR}/tolua++/lib/x64/tolua++D.lib \
                        $${THIRD_PARTY_DIR}/lua/lib/x64/lua5.1.lib
        }
    } else {
        message("32-bit")
        CONFIG(release,debug|release){
            ZSS_LIBS += $${THIRD_PARTY_DIR}/protobuf/lib/x86/libprotobuf.lib \
                        $${THIRD_PARTY_DIR}/tolua++/lib/x86/tolua++.lib \
                        $${THIRD_PARTY_DIR}/lua/lib/x86/lua5.1.lib
        }
        CONFIG(debug,debug|release){
            ZSS_LIBS += $${THIRD_PARTY_DIR}/protobuf/lib/x86/libprotobufd.lib \
                        $${THIRD_PARTY_DIR}/tolua++/lib/x86/tolua++.lib \
                        $${THIRD_PARTY_DIR}/lua/lib/x86/lua5.1.lib
        }
    }

    ZSS_INCLUDES += \
        $${THIRD_PARTY_DIR}/protobuf/include \
        $${THIRD_PARTY_DIR}/Eigen \

    # Python module
    if(contains(DEFINES,USE_PYTHON_MODULE)){
        message( "Using Python Module" )
        SOURCES += \
            src/PythonModule/PythonModule.cpp
        HEADERS += \
            src/PythonModule/PythonModule.h
        ZSS_INCLUDES += \
            $${THIRD_PARTY_DIR}/python/include \
            F:/Anaconda3/include
        CONFIG(release,debug|release){
            ZSS_LIBS += \
                $${THIRD_PARTY_DIR}/python/lib/boost_python36-vc140-mt-x64-1_69.lib \
                F:/Anaconda3/libs/python36.lib
        }
        CONFIG(debug,debug|release){
            ZSS_LIBS += \
                $${THIRD_PARTY_DIR}/python/lib/boost_python36-vc140-mt-gd-x64-1_69.lib \
                F:/Anaconda3/libs/python36.lib
        }
    }# end of Python module

    # CUDA module
    if(contains(DEFINES,USE_CUDA_MODULE)){
        message( "Using CUDA Module" )
        HEADERS += src/CUDAModule/CUDAModule.h
        SOURCES += src/CUDAModule/CUDAModule.cpp
        # CUDA settings <-- may change depending on your system
        CUDA_OBJECTS_DIR += ./temp/cuda
        CUDA_SOURCES += src/CUDAModule/SkillUtills.cu
#        CUDA_SOURCES += src/CUDAModule/CMmotion.cu \
#                        src/CUDAModule/SkillUtills.cu
        CUDA_DIR = "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v9.0"           # Path to cuda toolkit install
        SYSTEM_NAME = x64           # Depending on your system either 'Win32', 'x64', or 'Win64'
        SYSTEM_TYPE = 64            # '32' or '64', depending on your system
        # Type of CUDA architecture
        #Pascal (CUDA 8 and later):
        #61:GTX 1080, GTX 1070, GTX 1060, GTX 1050, Titan Xp
        #62:Tegra (Jetson) TX2
        #Volta (CUDA 9 and later):
        #70:GTX 1180 (GV104), Titan V
        #Turing (CUDA 10 and later):
        #75:GTX 1660 Ti, RTX 2060, RTX 2070, RTX 2080, Titan RTX
        CUDA_ARCH = compute_61
        CUDA_CODE = sm_61
        NVCC_OPTIONS = --use_fast_math

        #OTHER_FILES +=  $$CUDA_SOURCES

        # include paths
        INCLUDEPATH += $$CUDA_DIR/include

        # library directories
        QMAKE_LIBDIR += $$CUDA_DIR/lib/$$SYSTEM_NAME

        # The following library conflicts with something in Cuda
        QMAKE_LFLAGS_RELEASE = /NODEFAULTLIB:msvcrt.lib
        QMAKE_LFLAGS_DEBUG   = /NODEFAULTLIB:msvcrtd.lib

        MSVCRT_LINK_FLAG_DEBUG   = "/MDd"
        MSVCRT_LINK_FLAG_RELEASE = "/MD"

        CUDA_INC = $$join(INCLUDEPATH,'" -I"','-I"','"')

        CUDA_LIB_NAMES += cuda cudart MSVCRT

        for(lib, CUDA_LIB_NAMES) {
            CUDA_LIBS += $$lib.lib
        }
        for(lib, CUDA_LIB_NAMES) {
            NVCC_LIBS += -l$$lib
        }
        LIBS += $$NVCC_LIBS

        CONFIG(release,debug|release){
            cuda.input = CUDA_SOURCES
            cuda.output = $$CUDA_OBJECTS_DIR/${QMAKE_FILE_BASE}_cuda.obj
            cuda.commands = $$CUDA_DIR/bin/nvcc.exe $$NVCC_OPTIONS $$CUDA_INC $$NVCC_LIBS \
                            --machine $$SYSTEM_TYPE -arch=$$CUDA_ARCH -code=$$CUDA_CODE \
                            --compile -cudart static \
                            -Xcompiler $$MSVCRT_LINK_FLAG_RELEASE \
                            --optimize 3 \
                            -c -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME}
            cuda.dependency_type = TYPE_C
            QMAKE_EXTRA_COMPILERS += cuda
        }
        CONFIG(debug,debug|release){
            cuda.input = CUDA_SOURCES
            cuda.output = $$CUDA_OBJECTS_DIR/${QMAKE_FILE_BASE}_cuda.obj
            cuda.commands = $$CUDA_DIR/bin/nvcc.exe -D_DEBUG $$NVCC_OPTIONS $$CUDA_INC $$NVCC_LIBS \
                            --machine $$SYSTEM_TYPE -arch=$$CUDA_ARCH -code=$$CUDA_CODE \
                            --compile -cudart static \
                            -Xcompiler $$MSVCRT_LINK_FLAG_DEBUG \
                            --optimize 3 \
                            -c -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME}
            cuda.dependency_type = TYPE_C
            QMAKE_EXTRA_COMPILERS += cuda_d
        }
    }# end of CUDA module

}

unix:!macx{
    ZSS_INCLUDES += \
        /usr/include/eigen3 \
        /usr/local/include
    ZSS_LIBS += \
        -llua5.1 \
        -ldl \
        -ltolua++5.1 \
        /usr/local/lib/libprotobuf.a

    if(contains(DEFINES,USE_OPENMP)){
        QMAKE_CXXFLAGS += -fopenmp
        QMAKE_LFLAGS += -fopenmp
    }

    if(contains(DEFINES,USE_PYTHON_MODULE)){
        message( "Using Python Module" )
        SOURCES += \
            src/PythonModule/PythonModule.cpp
        HEADERS += \
            src/PythonModule/PythonModule.h
        ZSS_LIBS += \
            -lpython3 \
            -lboost_python3
        ZSS_INCLUDES += \
            /usr/include/python3
    }

    # CUDA module
    if(contains(DEFINES,USE_CUDA_MODULE)){
        message( "Using CUDA Module" )
        HEADERS += src/CUDAModule/CUDAModule.h
        SOURCES += src/CUDAModule/CUDAModule.cpp
        # CUDA settings <-- may change depending on your system
        CUDA_OBJECTS_DIR += ./temp/cuda
        CUDA_SOURCES += src/CUDAModule/SkillUtills.cu
        CUDA_DIR = "/usr/local/cuda" # Path to cuda toolkit install
        SYSTEM_NAME = ubuntu
        SYSTEM_TYPE = 64
        CUDA_ARCH = compute_61
        CUDA_CODE = sm_61
        NVCC_OPTIONS = --use_fast_math -Xcompiler
        NVCCFLAGS = $$NVCC_OPTIONS
        # include paths
        INCLUDEPATH += $$CUDA_DIR/include

        # library directories
        QMAKE_LIBDIR += $$CUDA_DIR/lib64/
        #CUDA_INC = $$join(INCLUDEPATH,'" -I"','-I"','"')

        CUDA_LIBS = -L$$CUDA_DIR/lib64 -lcudart

        NVCC_LIBS = $$join(CUDA_LIBS,' -l','-l', '')

        LIBS += $$CUDA_LIBS

        CUDA_INC = $$join(INCLUDEPATH,' -I','-I',' ')

        cuda.commands = $$CUDA_DIR/bin/nvcc -m64 -arch=$$CUDA_ARCH -c $$NVCCFLAGS \
                        $$CUDA_INC $$LIBS  ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_OUT} \
                        2>&1 | sed -r \"s/\\(([0-9]+)\\)/:\\1/g\" 1>&2
        cuda.dependency_type = TYPE_C
        cuda.depend_command = $$CUDA_DIR/bin/nvcc -M $$CUDA_INC $$NVCCFLAGS  ${QMAKE_FILE_NAME}

        cuda.input = CUDA_SOURCES
        cuda.output = ${OBJECTS_DIR}${QMAKE_FILE_BASE}_cuda.o
        # Tell Qt that we want add more stuff to the Makefile
        QMAKE_EXTRA_COMPILERS += cuda
    }# end of CUDA module

}

INCLUDEPATH += \
    $$ZSS_INCLUDES

LIBS += \
    $$ZSS_LIBS

INCLUDEPATH += \
    src \
    src/Algorithm \
    src/LuaModule \
    src/Main \
    src/MotionControl \
    src/Network \
    src/PathPlan \
    src/RefereeBox \
    src/Simulator \
    src/Strategy \
    src/Wireless \
    src/WorldModel \
    src/CUDAModel \
    src/Utils \
    src/Vision \
    src/Vision/mediator \
    src/PointCalculation \
    src/Strategy/bayes \
    src/Strategy/defence \
    src/Strategy/rolematch \
    src/Strategy/skill \
    src/OtherLibs \
    share \
    share/proto/cpp

SOURCES += \
    src/Algorithm/BallSpeedModel.cpp \
    src/Algorithm/BallStatus.cpp \
    src/Algorithm/BestPlayer.cpp \
    src/Algorithm/CollisionDetect.cpp \
    src/Algorithm/Compensate.cpp \
    src/Algorithm/ContactChecker.cpp \
    src/Algorithm/KickDirection.cpp \
    src/Algorithm/KickParam.cpp \
    src/Algorithm/PenaltyPosCleaner.cpp \
    src/Algorithm/ShootRangeList.cpp \
    src/Algorithm/kickregulation.cpp \
    src/Algorithm/ShootModule.cpp \
    src/LuaModule/LuaModule.cpp \
    src/LuaModule/lua_zeus.cpp \
    src/Main/ActionModule.cpp \
    src/Main/DecisionModule.cpp \
    src/Main/Global.cpp \
    src/Main/OptionModule.cpp \
    src/MotionControl/CMmotion.cpp \
    src/MotionControl/ControlModel.cpp \
    src/MotionControl/CubicEquation.cpp \
    src/MotionControl/DynamicsSafetySearch.cpp \
    src/MotionControl/noneTrapzodalVelTrajectory.cpp \
    src/MotionControl/QuadraticEquation.cpp \
    src/MotionControl/QuarticEquation.cpp \
    src/MotionControl/TrapezoidalVelTrajectory.cpp \
    src/OtherLibs/cmu/obstacle.cpp \
    src/OtherLibs/cmu/path_planner.cpp \
    src/OtherLibs/cornell/Trajectory.cpp \
    src/OtherLibs/cornell/TrajectoryStructs.cpp \
    src/OtherLibs/cornell/TrajectorySupport.cpp \
    src/PathPlan/ChipBallJudge.cpp \
    src/PathPlan/PathPlanner.cpp \
    src/PathPlan/PredictTrajectory.cpp \
    src/PathPlan/RRTPathPlanner.cpp \
    src/PointCalculation/AssistPoint.cpp \
    src/PointCalculation/AtomPos.cpp \
    src/PointCalculation/CornerAreaPos.cpp \
    src/PointCalculation/DefaultPos.cpp \
    src/PointCalculation/DefPos1G2D.cpp \
    src/PointCalculation/DefPos2015.cpp \
    src/PointCalculation/GoaliePosV1.cpp \
    src/PointCalculation/IndirectDefender.cpp \
    src/PointCalculation/KickOffDefPosV2.cpp \
    src/PointCalculation/MarkingPosV2.cpp \
    src/PointCalculation/MarkingTouchPos.cpp \
    src/PointCalculation/SupportPos.cpp \
    src/PointCalculation/TandemPos.cpp \
    src/PointCalculation/TouchKickPos.cpp \
    src/PointCalculation/WaitKickPos.cpp \
    src/RefereeBox/RefereeBoxIf.cpp \
    share/proto/cpp/grSim_Commands.pb.cc \
    share/proto/cpp/grSimMessage.pb.cc \
    share/proto/cpp/grSim_Packet.pb.cc \
    share/proto/cpp/grSim_Replacement.pb.cc \
    share/proto/cpp/messages_robocup_ssl_detection.pb.cc \
    share/proto/cpp/messages_robocup_ssl_geometry_legacy.pb.cc \
    share/proto/cpp/messages_robocup_ssl_geometry.pb.cc \
    share/proto/cpp/messages_robocup_ssl_refbox_log.pb.cc \
    share/proto/cpp/messages_robocup_ssl_wrapper_legacy.pb.cc \
    share/proto/cpp/messages_robocup_ssl_wrapper.pb.cc \
    share/proto/cpp/ssl_game_controller_auto_ref.pb.cc \
    share/proto/cpp/ssl_game_controller_common.pb.cc \
    share/proto/cpp/ssl_game_controller_team.pb.cc \
    share/proto/cpp/ssl_game_event_2019.pb.cc \
    share/proto/cpp/ssl_game_event.pb.cc \
    share/proto/cpp/ssl_referee.pb.cc \
    share/proto/cpp/vision_detection.pb.cc \
    share/proto/cpp/zss_cmd.pb.cc \
    share/proto/cpp/zss_debug.pb.cc \
    share/proto/cpp/zss_rec.pb.cc \
    share/proto/cpp/zss_train.pb.cc \
    share/parammanager.cpp \
    share/geometry.cpp \
    src/Strategy/bayes/BayesFilter.cpp \
    src/Strategy/bayes/BayesParam.cpp \
    src/Strategy/bayes/BayesReader.cpp \
    src/Strategy/bayes/MatchState.cpp \
    src/Strategy/defence/AttributeSet.cpp \
    src/Strategy/defence/DefenceInfo.cpp \
    src/Strategy/defence/EnemyDefendTacticAnalys.cpp \
    src/Strategy/defence/EnemyDefendTacticArea.cpp \
    src/Strategy/defence/EnemySituation.cpp \
    src/Strategy/defence/OppAttributesFactory.cpp \
    src/Strategy/defence/OppPlayer.cpp \
    src/Strategy/defence/OppRoleFactory.cpp \
    src/Strategy/defence/OppRoleMatcher.cpp \
    src/Strategy/defence/Trigger.cpp \
    src/Strategy/rolematch/matrix.cpp \
    src/Strategy/rolematch/munkres.cpp \
    src/Strategy/rolematch/munkresTacticPositionMatcher.cpp \
    src/Strategy/skill/PlayerTask.cpp \
    src/Strategy/skill/AdvanceBallV1.cpp \
    src/Strategy/skill/AdvanceBallV2.cpp \
    src/Strategy/skill/BasicPlay.cpp \
    src/Strategy/skill/ChaseKickV2.cpp \
    src/Strategy/skill/CrazyPush.cpp \
    src/Strategy/skill/DribbleTurn.cpp \
    src/Strategy/skill/DribbleTurnKick.cpp \
    src/Strategy/skill/Factory.cpp \
    src/Strategy/skill/GoAndTurnKickV3.cpp \
    src/Strategy/skill/GoAvoidShootLine.cpp \
    src/Strategy/skill/GoImmortalRush.cpp \
    src/Strategy/skill/GotoPosition.cpp \
    src/Strategy/skill/InterceptBallV3.cpp \
    src/Strategy/skill/InterceptTouch.cpp \
    src/Strategy/skill/Marking.cpp \
    src/Strategy/skill/MarkingFront.cpp \
    src/Strategy/skill/MarkingTouch.cpp \
    src/Strategy/skill/OpenSpeed.cpp \
    src/Strategy/skill/PenaltyDef2017V2.cpp \
    src/Strategy/skill/PenaltyDefV2.cpp \
    src/Strategy/skill/PenaltyKick2017V1.cpp \
    src/Strategy/skill/PenaltyKick2017V2.cpp \
    src/Strategy/skill/ReceivePass.cpp \
    src/Strategy/skill/SlowGetBall.cpp \
    src/Strategy/skill/SmartGotoPosition.cpp \
    src/Strategy/skill/GetBallV3.cpp \
    src/Strategy/skill/Speed.cpp \
    src/Strategy/skill/StaticGetBallNew.cpp \
    src/Strategy/skill/StopRobot.cpp \
    src/Strategy/skill/TouchKick.cpp \
    src/Strategy/skill/zdrag.cpp \
    src/Utils/BufferCounter.cpp \
    src/Utils/DefendUtils.cpp \
    src/Utils/FreeKickUtils.cpp \
    src/Utils/GDebugEngine.cpp \
    src/Utils/NormalPlayUtils.cpp \
    src/Utils/param.cpp \
    src/Utils/utils.cpp \
    src/Vision/BallPredictor.cpp \
    src/Vision/CollisionSimulator.cpp \
    src/Vision/RobotPredictError.cpp \
    src/Vision/RobotPredictor.cpp \
    src/Vision/RobotsCollision.cpp \
    src/Vision/VisionModule.cpp \
    src/Vision/VisionReceiver.cpp \
    src/Wireless/PlayerCommandV2.cpp \
    src/Wireless/RobotSensor.cpp \
    src/WorldModel/GetLuaData.cpp \
    src/WorldModel/PlayInterface.cpp \
    src/WorldModel/RobotCapability.cpp \
    src/WorldModel/WorldModel_basic.cpp \
    src/WorldModel/WorldModel_enemy.cpp \
    src/WorldModel/WorldModel_lua.cpp \
    src/WorldModel/WorldModel_utils.cpp \
    src/Main/zeus_main.cpp \
    src/Utils/SkillUtils.cpp \
    src/Strategy/skill/InterceptBallV7.cpp \
    src/Strategy/skill/ZMarking.cpp \
    src/Strategy/skill/ZChaseKick.cpp \
    src/Strategy/skill/OpenSpeedCircle.cpp \
    src/Strategy/skill/ZGoalie.cpp \
    src/Strategy/skill/ZPass.cpp \
    src/Strategy/skill/FetchBall.cpp \
    src/Strategy/skill/ZSupport.cpp \
    src/Strategy/skill/ZBreak.cpp \
    src/Strategy/skill/ZAttack.cpp \
    src/Strategy/skill/ZCirclePass.cpp \
    src/Strategy/skill/GetBallV4.cpp \
    src/Simulator/CommandInterface.cpp \
    src/Strategy/skill/zmessi.cpp \
    src/PathPlan/ObstacleNew.cpp \
    src/Strategy/skill/GotoPositionNew.cpp \
    src/PathPlan/BezierMotion.cpp \
    src/Strategy/skill/HoldBall.cpp \
    src/Strategy/skill/GoAndTurnKickV4.cpp \
    src/Strategy/skill/TouchKickV2.cpp \
    src/Strategy/skill/ZBlocking.cpp \
    src/Algorithm/receivePos.cpp \
    src/Algorithm/messidecision.cpp \
    src/Algorithm/runpos.cpp \
    src/CUDAModule/drawscore.cpp \
    src/PathPlan/BezierCurveNew.cpp \
    src/Strategy/skill/CircleGoto.cpp \
    src/PathPlan/KDTreeNew.cpp \
    src/PathPlan/RRTPathPlannerNew.cpp \
    src/Strategy/skill/SmartGotoPositionV3.cpp \
    src/Algorithm/ballmodel.cpp



HEADERS += \
    src/Algorithm/BallSpeedModel.h \
    src/Algorithm/BallStatus.h \
    src/Algorithm/BestPlayer.h \
    src/Algorithm/CollisionDetect.h \
    src/Algorithm/Compensate.h \
    src/Algorithm/ContactChecker.h \
    src/Algorithm/KickDirection.h \
    src/Algorithm/KickParam.h \
    src/Algorithm/PenaltyPosCleaner.h \
    src/Algorithm/ShootRangeList.h \
    src/Algorithm/kickregulation.h \
    src/Algorithm/ShootModule.h \
    src/LuaModule/lauxlib.h \
    src/LuaModule/luaconf.h \
    src/LuaModule/lua.h \
    src/LuaModule/lualib.h \
    src/LuaModule/LuaModule.h \
    src/LuaModule/tolua++.h \
    src/Main/ActionModule.h \
    src/Main/DecisionModule.h \
    src/Main/Global.h \
    src/Main/OptionModule.h \
    src/Main/TaskMediator.h \
    src/MotionControl/CMmotion.h \
    src/MotionControl/ControlModel.h \
    src/MotionControl/CubicEquation.h \
    src/MotionControl/DynamicsSafetySearch.h \
    src/MotionControl/noneTrapzodalVelTrajectory.h \
    src/MotionControl/QuadraticEquation.h \
    src/MotionControl/QuarticEquation.h \
    src/MotionControl/TrapezoidalVelTrajectory.h \
    src/OtherLibs/cmu/constants.h \
    src/OtherLibs/cmu/fast_alloc.h \
    src/OtherLibs/cmu/kdtree.h \
    src/OtherLibs/cmu/obstacle.h \
    src/OtherLibs/cmu/path_planner.h \
    src/OtherLibs/cmu/util.h \
    src/OtherLibs/cmu/vector.h \
    src/OtherLibs/cornell/Trajectory.h \
    src/OtherLibs/cornell/TrajectoryStructs.h \
    src/OtherLibs/cornell/TrajectorySupport.h \
    src/OtherLibs/nlopt/nlopt.h \
    src/OtherLibs/nlopt/nlopt.hpp \
    src/PathPlan/ChipBallJudge.h \
    src/PathPlan/PathPlanner.h \
    src/PathPlan/PredictTrajectory.h \
    src/PathPlan/RRTPathPlanner.h \
    src/PointCalculation/AssistPoint.h \
    src/PointCalculation/AtomPos.h \
    src/PointCalculation/CornerAreaPos.h \
    src/PointCalculation/DefaultPos.h \
    src/PointCalculation/DefPos1G2D.h \
    src/PointCalculation/DefPos2015.h \
    src/PointCalculation/GoaliePosV1.h \
    src/PointCalculation/IndirectDefender.h \
    src/PointCalculation/KickOffDefPosV2.h \
    src/PointCalculation/MarkingPosV2.h \
    src/PointCalculation/MarkingTouchPos.h \
    src/PointCalculation/SupportPos.h \
    src/PointCalculation/TouchKickPos.h \
    src/PointCalculation/WaitKickPos.h \
    src/RefereeBox/game_state.h \
    src/RefereeBox/playmode.h \
    src/RefereeBox/RefereeBoxIf.h \
    src/RefereeBox/referee_commands.h \
    share/proto/cpp/grSim_Commands.pb.h \
    share/proto/cpp/grSimMessage.pb.h \
    share/proto/cpp/grSim_Packet.pb.h \
    share/proto/cpp/grSim_Replacement.pb.h \
    share/proto/cpp/messages_robocup_ssl_detection.pb.h \
    share/proto/cpp/messages_robocup_ssl_geometry_legacy.pb.h \
    share/proto/cpp/messages_robocup_ssl_geometry.pb.h \
    share/proto/cpp/messages_robocup_ssl_refbox_log.pb.h \
    share/proto/cpp/messages_robocup_ssl_wrapper_legacy.pb.h \
    share/proto/cpp/messages_robocup_ssl_wrapper.pb.h \
    share/proto/cpp/ssl_game_controller_auto_ref.pb.h \
    share/proto/cpp/ssl_game_controller_common.pb.h \
    share/proto/cpp/ssl_game_controller_team.pb.h \
    share/proto/cpp/ssl_game_event_2019.pb.h \
    share/proto/cpp/ssl_game_event.pb.h \
    share/proto/cpp/ssl_referee.pb.h \
    share/proto/cpp/vision_detection.pb.h \
    share/proto/cpp/zss_cmd.pb.h \
    share/proto/cpp/zss_debug.pb.h \
    share/proto/cpp/zss_rec.pb.h \
    share/proto/cpp/zss_train.pb.h \
    share/parammanager.h \
    share/singleton.hpp \
    share/geometry.h \
    src/Simulator/server.h \
    src/Simulator/ServerInterface.h \
    src/Strategy/bayes/BayesFilter.h \
    src/Strategy/bayes/BayesParam.h \
    src/Strategy/bayes/BayesReader.h \
    src/Strategy/bayes/MatchState.h \
    src/Strategy/defence/Attribute.h \
    src/Strategy/defence/AttributeSet.h \
    src/Strategy/defence/DefenceInfo.h \
    src/Strategy/defence/EnemyDefendTacticAnalys.h \
    src/Strategy/defence/EnemyDefendTacticArea.h \
    src/Strategy/defence/EnemySituation.h \
    src/Strategy/defence/OppAttributesFactory.h \
    src/Strategy/defence/OppPlayer.h \
    src/Strategy/defence/OppRoleFactory.h \
    src/Strategy/defence/OppRole.h \
    src/Strategy/defence/OppRoleMatcher.h \
    src/Strategy/defence/Trigger.h \
    src/Strategy/rolematch/matrix.h \
    src/Strategy/rolematch/munkres.h \
    src/Strategy/rolematch/munkresTacticPositionMatcher.h \
    src/Strategy/skill/AdvanceBallV1.h \
    src/Strategy/skill/AdvanceBallV2.h \
    src/Strategy/skill/BasicPlay.h \
    src/Strategy/skill/ChaseKickV2.h \
    src/Strategy/skill/CrazyPush.h \
    src/Strategy/skill/DribbleTurn.h \
    src/Strategy/skill/DribbleTurnKick.h \
    src/Strategy/skill/Factory.h \
    src/Strategy/skill/GetBallV4.h \
    src/Strategy/skill/GoAndTurnKickV3.h \
    src/Strategy/skill/GoAvoidShootLine.h \
    src/Strategy/skill/GoImmortalRush.h \
    src/Strategy/skill/GotoPosition.h \
    src/Strategy/skill/InterceptBallV3.h \
    src/Strategy/skill/InterceptTouch.h \
    src/Strategy/skill/MarkingFront.h \
    src/Strategy/skill/Marking.h \
    src/Strategy/skill/MarkingTouch.h \
    src/Strategy/skill/OpenSpeed.h \
    src/Strategy/skill/PenaltyDef2017V2.h \
    src/Strategy/skill/PenaltyDefV2.h \
    src/Strategy/skill/PenaltyKick2017V1.h \
    src/Strategy/skill/PenaltyKick2017V2.h \
    src/Strategy/skill/PlayerTask.h \
    src/Strategy/skill/ReceivePass.h \
    src/Strategy/skill/SlowGetBall.h \
    src/Strategy/skill/SmartGotoPosition.h \
    src/Strategy/skill/GetBallV3.h \
    src/Strategy/skill/Speed.h \
    src/Strategy/skill/StaticGetBallNew.h \
    src/Strategy/skill/StopRobot.h \
    src/Strategy/skill/TouchKick.h \
    src/Strategy/skill/zdrag.h \
    src/Utils/BufferCounter.h \
    src/Utils/ClassFactory.h \
    src/Utils/DataQueue.hpp \
    src/Utils/DefendUtils.h \
    src/Utils/FreeKickUtils.h \
    src/Utils/GDebugEngine.h \
    src/Utils/misc_types.h \
    src/Utils/MultiThread.h \
    src/Utils/NormalPlayUtils.h \
    src/Utils/os_param.h \
    src/Utils/param.h \
    src/Utils/Semaphore.h \
    src/Utils/singleton.h \
    src/Utils/SkillUtils.h \
    src/Utils/utils.h \
    src/Utils/ValueRange.h \
    src/Utils/weerror.h \
    src/Vision/BallPredictor.h \
    src/Vision/CollisionSimulator.h \
    src/Vision/mediator/net/message.h \
    src/Vision/RobotPredictData.h \
    src/Vision/RobotPredictError.h \
    src/Vision/RobotPredictor.h \
    src/Vision/RobotsCollision.h \
    src/Vision/VisionModule.h \
    src/Vision/VisionReceiver.h \
    src/Wireless/CommandFactory.h \
    src/Wireless/CommControl.h \
    src/Wireless/PlayerCommand.h \
    src/Wireless/PlayerCommandV2.h \
    src/Wireless/RobotCommand.h \
    src/Wireless/RobotSensor.h \
    src/WorldModel/DribbleStatus.h \
    src/WorldModel/GetLuaData.h \
    src/WorldModel/KickStatus.h \
    src/WorldModel/PlayInterface.h \
    src/WorldModel/RobotCapability.h \
    src/WorldModel/robot_power.h \
    src/WorldModel/WorldDefine.h \
    src/WorldModel/WorldModel.h \
    share/parammanager.h \
    share/singleton.hpp \
    share/staticparams.h \
    src/Utils/SkillUtils.h \
    src/Strategy/skill/Factory.h \
    src/Strategy/skill/InterceptBallV7.h \
    src/Strategy/skill/ZMarking.h \
    src/Strategy/skill/ZChaseKick.h \
    src/Strategy/skill/OpenSpeedCircle.h \
    src/Strategy/skill/ZGoalie.h \
    src/Strategy/skill/ZCirclePass.h \
    src/Strategy/skill/ZPass.h \
    src/Strategy/skill/FetchBall.h \
    src/Strategy/skill/ZSupport.h \
    src/Strategy/skill/ZBreak.h \
    src/Strategy/skill/ZAttack.h \
    src/Strategy/skill/ZCirclePass.h \
    src/Simulator/CommandInterface.h \
    src/Strategy/skill/zmessi.h \
    src/PathPlan/ObstacleNew.h \
    src/Strategy/skill/GotoPositionNew.h \
    src/PathPlan/BezierMotion.h \
    src/Strategy/skill/HoldBall.h \
    src/Strategy/skill/GoAndTurnKickV4.h \
    src/Strategy/skill/TouchKickV2.h \
    src/Strategy/skill/ZBlocking.h \
    src/Algorithm/receivePos.h \
    src/Algorithm/messidecision.h \
    src/Algorithm/runpos.h \
    src/CUDAModule/drawscore.h \
    src/PathPlan/BezierCurveNew.h \
    src/Strategy/skill/CircleGoto.h \
    src/PathPlan/FastAllocator.h \
    src/PathPlan/KDTreeNew.h \
    src/PathPlan/RRTPathPlannerNew.h \
    src/Strategy/skill/SmartGotoPositionV3.h \
    src/Algorithm/ballmodel.h\



win32-msvc*: QMAKE_LFLAGS += /FORCE:MULTIPLE

QMAKE_CXXFLAGS += -utf-8
unix:!macx{
    QMAKE_CXXFLAGS += -Wno-comment -Wno-reorder -Wno-conversion-null
}

#message($$INCLUDEPATH)

#LD_LIBRARY_PATH=dir：$LD_LIBRARY_PATH
#export LD_LIBRARY_PATH
