#ifndef BEZIERCURVENEW_H
#define BEZIERCURVENEW_H
#include "ObstacleNew.h"
#include <vector>
#include <utility>
using namespace std;
enum runType {
    TAKE_BALL = 0,
    RUN_ROTATE_AND_RUN = 1,
    FAST_ROTATE = 2,
    RUN_ROTATE = 3
};

///==============================================================================================================================//
///                                        BezierSubCurve class definition
///==============================================================================================================================//
class BezierSubCurve {///to curve a cubic bezier spline between two sub-distinations
    bool _initializedControlPoints;
    int _numPathPoints;         ///the total number of path points
    double A0, A1, A2, A3;       ///coordinates of four control points
    double B0, B1, B2, B3;
    double _curvePrecision;     ///the max distance between two path points
    vector < CGeoPoint* > _pathGeoList;

  private:
    bool needInsert(void);          ///if the curve is not accurate enough
    void InsertPathPoints(void);
  public:
    BezierSubCurve(): _initializedControlPoints(false) {}
    ~BezierSubCurve() {
        for (CGeoPoint* p : _pathGeoList) delete p;
        vector< CGeoPoint* > ().swap(_pathGeoList);
    }
    void initCurver(const vector < CGeoPoint > & controlPoints, double curvePrecision);
    void curvePath(void);       ///only curve the first path
    vector < double > calculateCurvature(void);
    vector < CGeoPoint* > getGeoPath(void)const {
        return _pathGeoList;
    }
};

///==============================================================================================================================//
///                                        BezierFullCurve class definition
///==============================================================================================================================//

class BezierFullCurve {///curve the full path using several bezier splines
  protected:
    bool _pathPlanned;
    stateNew _start;
    stateNew _end;

    PlayerCapabilityT _capability;
    vector < BezierSubCurve* > _subCurver;
    vector < CGeoPoint > _subDestinations;
    vector < CGeoPoint > _fullGeoPathList;

    int _curvedSubDest;
    pair<vector < CGeoPoint >, vector < CGeoPoint > > calculateControlPoints(CGeoPoint w1, CGeoPoint w2, CGeoPoint w3);

  public:
    BezierFullCurve() {}

    ~BezierFullCurve() {
        for(BezierSubCurve* curver : _subCurver) delete curver;
        vector < BezierSubCurve* > ().swap(_subCurver);
    }
    void initCurver(const stateNew& start, const stateNew& end, const PlayerCapabilityT& capability, const vector<CGeoPoint> subDest);
    void planPath(double curvePrecision);
    vector < CGeoPoint > getFullGeoPathList(void) const {
        return _fullGeoPathList;
    }
};

///==============================================================================================================================//
///                                       BezierController class definition
///==============================================================================================================================//
class BezierController: public BezierFullCurve {
    int _arrivedNum;
    double _lastOffDist;
    double _totalTime;
    double _maxCurvature;

    double _P;
    double _I;
    double _D;
    double _lastAdjustDist;
    double _lastLastAdjustDist;
    double _lastControerOutput;


    runType _runType;
    vector < double > _pathSegLength;
    vector < stateNew > _stateList;
    vector < double > _curvature;
    vector < double > _originCurvature;
    vector < double > _vMax;
    vector < double > _time;
  private:

    void calculateStates(void);
    void calculateVMax(void);
    vector < int > findInflecPoint(void);
    /*pair < vector < vector < double > >, vector < vector < double > > >*/void calculateVelAndTimeList(vector < vector < double > >& velList, vector < vector < double > >& timeList);
    void distanceToSegment(const CGeoPoint &segStart, const CGeoPoint &segEnd, const CGeoPoint &targetPoint, double& L, double& S);
    void validatePrecision(void);
    void validFullGeoPoints(void);
    double getTotalDist(void);
  public:
    BezierController() {}
    ///the viaPoints should include the start point and end point
    void initController(const stateNew& start, const stateNew& end, const PlayerCapabilityT& capability, vector<CGeoPoint> viaPoints, runType rT = FAST_ROTATE);
    void planController();
    double getTime(void);
    double evaluatePath(const stateNew& currentState, ObstaclesNew* obst, bool DebugMsgPos);
    bool isOffLine(const CGeoPoint &currentPosition, double thresholdL, double thresholdS);
    stateNew getNextState(const stateNew& currentState, double& nextCurva);
    vector < stateNew > getStateList(void)const {
        return _stateList;
    }
    int getArrivedNum(void) const {
        return _arrivedNum;
    }

};


#endif // BEZIERCURVENEW_H
