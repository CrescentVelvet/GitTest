#include "BezierCurveNew.h"
#include <fstream>
#include <GDebugEngine.h>
#include <cmath>
namespace {
const bool drawDebugMsg = true;

const double P = 1.0;
const double I = 4.0;
const double D = 15.0;

const double TOTAL_ACCEL = 450;    ///the max acceleration that the friction can provide
double accFactorFront = 1.0; /// must lower than 1.0
double accFactorBack = 1.3; /// must bigger than 1.0
const double MIN_FRAME_NUM_BETWEEN_PATH_POINT = 5.0; ///used in validate path dist function
const double CURVE_PRECISION = 1.0;  ///the max distance between two of the path points
const double MIN_IP_CURVATURE = 0.025;
const double MIN_DIST_BETWEEN_SUB_DESTINATION = PARAM::Vehicle::V2::PLAYER_SIZE / 2.0;

const double thetaUpperBound =  PARAM::Math::PI / 5; //模拟搜索路径时给定的最大theta值
const double maxCuratureSetted = 0.00001; //拟合时给定的最大曲率半径

const double minCurvaToCalculate = 0.01;
const double maxCurvaToDelete = 0.13;
const double safeDist = 2.5 * PARAM::Vehicle::V2::PLAYER_SIZE;
const double maxSafeDist = 6.0 * PARAM::Vehicle::V2::PLAYER_SIZE;
///for path evaluate
const double wFirstCollisionDist = 20; //big is better
const double wTotalCurvature = -120.0;  //small is better
const double wTotalDist = -10; //small is better
const double wInitialVelContr = -20.0;
const double wDistNextIP = 10.0;
const double wMaxCurvature = -10;
const double wMinDistToObst = 40;
const double collisionPenalty = -1e8;

ofstream carVisionVel("C://Users//gayty//Desktop//Sydney//carVisionVel.txt");

inline double Max(double a1, double a2) {
    return a1 > a2 ? a1 : a2;
}
}

///==============================================================================================================================//
///                                        BezierSubCurve class implementation
///==============================================================================================================================//

/**
 * @brief BezierSubCurve::initCurver
 * @param start
 * @param end
 * @param curvePrecision
 */
void BezierSubCurve::initCurver(const vector<CGeoPoint>& controlPoints, double curvePrecision) {

    _pathGeoList.clear();
    _numPathPoints = 0;
    _initializedControlPoints = true;
    A0 = controlPoints[0].x();
    A1 = controlPoints[1].x();
    A2 = controlPoints[2].x();
    A3 = controlPoints[3].x();

    B0 = controlPoints[0].y();
    B1 = controlPoints[1].y();
    B2 = controlPoints[2].y();
    B3 = controlPoints[3].y();
    if(isnan(A0) || isnan(A1) || isnan(A2) || isnan(A3) ||
            isnan(B0) || isnan(B1) || isnan(B2) || isnan(B3))
        cout << "Oh shit!!! Control point is not a number!!! ---BezierCurveNew.cpp" << endl;

    _curvePrecision = curvePrecision;
    return;
}

/**
 * @brief BezierSubCurve::curvePath curve the sub-path according to the control points
 */
void BezierSubCurve::curvePath() {
    if(!_initializedControlPoints) {
        _numPathPoints = 0;
        _pathGeoList.clear();
        return;
    }

    double u, eu, unit;
    double p0, p1, p2, p3;
    double x, y;
    ///curve the path with 0.1 interval
    _numPathPoints = 11;
    unit = 1.0 / (_numPathPoints - 1);
    _pathGeoList.clear();
    for(int i = 0; i < _numPathPoints; i++) {
        u = i * unit;
        eu = 1 - u;
        p0 = pow(eu, 3);
        p1 = 3 * u * pow(eu, 2);
        p2 = 3 * pow(u, 2) * eu;
        p3 = pow(u, 3);
        x = p0 * A0 + p1 * A1 + p2 * A2 + p3 * A3;
        y = p0 * B0 + p1 * B1 + p2 * B2 + p3 * B3;
        CGeoPoint* tempPoint = new CGeoPoint(x, y);

        _pathGeoList.push_back(tempPoint);
    }
    while(needInsert()) {
        InsertPathPoints();
    }
    return;
}

/**
 * @brief BezierSubCurve::needInsert if the max distance between adjacent path points is bigger than the curvePrecision, return true
 * @return
 */
bool BezierSubCurve::needInsert() {
    double maxInterval = 0;
    for(int i = 0; i < _numPathPoints - 1; i++) {
        double tempDist = _pathGeoList[i]->dist(*_pathGeoList[i + 1]);
        maxInterval = tempDist > maxInterval ? tempDist : maxInterval;
    }
    return maxInterval > _curvePrecision;
}

/**
 * @brief BezierSubCurve::InsertPathPoints double the number of path points
 */
void BezierSubCurve::InsertPathPoints() {
    double u, eu, curv;
    double p0, p1, p2, p3;
    double x, y;
    double unit = 1.0 / ((_numPathPoints - 1) * 2);
    _numPathPoints = 1 + 2 * (_numPathPoints - 1);
    _pathGeoList.resize(_numPathPoints);
    for(int i = _numPathPoints - 1; i > 0 ; i -= 2) {
        _pathGeoList[i] = _pathGeoList[i / 2];
        u = (i - 1) * unit;
        eu = 1 - u;
        p0 = pow(eu, 3);
        p1 = 3 * u * pow(eu, 2);
        p2 = 3 * pow(u, 2) * eu;
        p3 = pow(u, 3);
        x = p0 * A0 + p1 * A1 + p2 * A2 + p3 * A3;
        y = p0 * B0 + p1 * B1 + p2 * B2 + p3 * B3;
        _pathGeoList[i - 1] = new CGeoPoint(x, y);
    }
    return;
}

/**
 * @brief BezierSubCurve::calculateCurvature calculate the curvature of every path point
 */
vector < double > BezierSubCurve::calculateCurvature() {
    double unit = 1.0 / (_numPathPoints - 1);
    double u, eu;
    double xp, xpp, yp, ypp;
    vector < double > curvature;
    curvature.resize(_numPathPoints);
    for(int i = 0; i < _numPathPoints; i++) {
        u = i * unit;
        eu = 1 - u;
        xp = 3 * (A1 - A0) * pow(eu, 2) + 6 * (A2 - A1) * u * eu + 3 * (A3 - A2) * pow(u, 2);
        yp = 3 * (B1 - B0) * pow(eu, 2) + 6 * (B2 - B1) * u * eu + 3 * (B3 - B2) * pow(u, 2);
        xpp = 6 * (A2 - 2.0 * A1 + A0) * eu + 6 * (A3 - 2.0 * A2 + A1) * u;
        ypp = 6 * (B2 - 2.0 * B1 + B0) * eu + 6 * (B3 - 2.0 * B2 + B1) * u;
        curvature[i] = abs((xp * ypp - yp * xpp)) / sqrt(pow(pow(xp, 2) + pow(yp, 2), 3));
//        if(curvature[i] > 0.03) {
//            GDebugEngine::Instance()->gui_debug_msg(*(_pathGeoList[i]), QString("cur: %1").arg(curvature[i]).toLatin1(), COLOR_ORANGE);
//        }
    }
    return curvature;
}


///==============================================================================================================================//
///                                       BezierFullCurve class implementation
///==============================================================================================================================//

/**
 * @brief BezierFullCurve::initCurver
 * @param start
 * @param end
 * @param obs
 * @param capability
 * @param subDest: include the start point and end point
 */
void BezierFullCurve::initCurver(const stateNew& start, const stateNew& end, const PlayerCapabilityT& capability, const vector<CGeoPoint> subDest) {

    if(!_subCurver.empty())
        for(BezierSubCurve* curver : _subCurver)
            delete curver;

    _subCurver.clear();
    _fullGeoPathList.clear();
    _subDestinations.clear();
    _capability = capability;
    _curvedSubDest = 0;
    _start = start;
    _end = end;
    _pathPlanned = false;
    if(start.pos.dist(subDest.front()) > 1e-8) {
        cout << "Oh shit!!! subDest vector should begin with the start pos!!! ---BezierCurveNew.cpp" << endl;
        _subDestinations.push_back(start.pos);
    }

    for(int i = 0; i < subDest.size(); i++) {
        _subDestinations.push_back(subDest[i]);
    }
    if(end.pos.dist(subDest.back()) > 1e-8) {
        cout << "Oh shit!!! subDest vector should end with the end pos!!! ---BezierCurveNew.cpp" << endl;
        _subDestinations.push_back(end.pos);
    }
	if (_subDestinations.size() < 2) {
		cout << "Error subDestination size!!! ---BezierCurveNew.cpp" << endl;
	}
}

/**
 * @brief BezierFullCurve::planGeoPath to curve a path
 */
void BezierFullCurve::planPath(double curvePrecision) {
    vector < CGeoPoint* > _subGeoPathList;
    pair<vector < CGeoPoint >, vector < CGeoPoint > > cp;
    _fullGeoPathList.clear();

    vector < CGeoPoint > LineCP;
    LineCP.resize(4);
    CVector dir ;

    while(_curvedSubDest + 2 < _subDestinations.size()) {
        cp = calculateControlPoints(_subDestinations[_curvedSubDest], _subDestinations[_curvedSubDest + 1], _subDestinations[_curvedSubDest + 2]);
        if(_fullGeoPathList.empty()) {
            dir = cp.first.front() - _subDestinations[0];
            LineCP[0] = _subDestinations[0];
        } else {
            dir = cp.first.front() - _fullGeoPathList.back();
            LineCP[0] = _fullGeoPathList.back();
        }
        LineCP[1] = LineCP[0] + dir / 3.0;
        LineCP[2] = LineCP[0] + dir * 2.0 / 3.0;
        LineCP[3] = LineCP[0] + dir;
        BezierSubCurve* tempCurver0 = new BezierSubCurve;
        tempCurver0->initCurver(LineCP, curvePrecision);
        tempCurver0->curvePath();
        _subGeoPathList = tempCurver0->getGeoPath();
        for(CGeoPoint* p : _subGeoPathList)
            _fullGeoPathList.push_back(*p);
        _subCurver.push_back(tempCurver0);

        BezierSubCurve* tempCurver1 = new BezierSubCurve;
        tempCurver1->initCurver(cp.first, curvePrecision);
        tempCurver1->curvePath();
        _subGeoPathList = tempCurver1->getGeoPath();
        for(CGeoPoint* p : _subGeoPathList)
            _fullGeoPathList.push_back(*p);
        _subCurver.push_back(tempCurver1);

        BezierSubCurve* tempCurver2 = new BezierSubCurve;
        tempCurver2->initCurver(cp.second, curvePrecision);
        tempCurver2->curvePath();
        _subGeoPathList = tempCurver2->getGeoPath();
        for(CGeoPoint* p : _subGeoPathList)
            _fullGeoPathList.push_back(*p);
        _subCurver.push_back(tempCurver2);
        _curvedSubDest++;
    }

    if(_fullGeoPathList.empty()) {
        dir = _subDestinations[1] - _subDestinations[0];
        LineCP[0] = _subDestinations[0];

    } else {
        dir = _subDestinations.back() - _fullGeoPathList.back();
        LineCP[0] = _fullGeoPathList.back();
    }
    LineCP[1] = LineCP[0] + dir / 3.0;
    LineCP[2] = LineCP[0] + dir * 2.0 / 3.0;
    LineCP[3] = LineCP[0] + dir;
    BezierSubCurve* tempCurver = new BezierSubCurve;
    tempCurver->initCurver(LineCP, curvePrecision);
    tempCurver->curvePath();
    _subGeoPathList = tempCurver->getGeoPath();
    for(CGeoPoint* p : _subGeoPathList)
        _fullGeoPathList.push_back(*p);
    _subCurver.push_back(tempCurver);

    _curvedSubDest = _subDestinations.size();
    _pathPlanned = true;
}

pair<vector < CGeoPoint >, vector < CGeoPoint > > BezierFullCurve::calculateControlPoints(CGeoPoint w1, CGeoPoint w2, CGeoPoint w3) {
    double c1 = 7.2364; //定义常量参数
    double c2 = 2.0 / 5 * (sqrt(6) - 1);
    double c3 = (c2 + 4.0) / (c1 + 4);
    double c4 = pow((c2 + 4.0), 2) / (54 * c3);

    CVector dir1 = w2 - w1;
    CVector dir2 = w3 - w2;
    double theta = abs(Utils::Normalize(Utils::Normalize(dir1.dir()) - Utils::Normalize(dir2.dir()))); //搜索出的实际搜索值
    theta = theta > thetaUpperBound ? thetaUpperBound : theta; //实际搜索值不可能超过给定theta
    double dMax = min(dir1.mod(), dir2.mod()) / 2; //拟合允许的最大d值
    double maxCurature = maxCuratureSetted / 1.3;

    double beta = theta / 2;
    double betaUpperBound = thetaUpperBound / 2;
    double maxCuratureLowerBound = c4 * sin(betaUpperBound) / (dMax * pow(cos(betaUpperBound), 2));

    maxCurature = maxCurature < maxCuratureLowerBound ? maxCuratureLowerBound : maxCurature; //限制顺序是： 搜索step限制最大d， 最大d限制最大curature

    CVector u1 = w1 - w2;
    CVector u2 = w3 - w2;
    if(u1.mod() == 0 || u2.mod() == 0)
        cout << "Oh shit!!! Same Point param when calculate control points!!! ---BezierCurveNew.cpp" << endl;
    else {
        u1 = u1 / u1.mod();
        u2 = u2 / u2.mod();
    }

    double d = c4 * sin(beta) / (maxCurature * pow(cos(beta), 2));
    double he, hb, ge, gb, ke, kb;
    ge = gb = c3 * d;
    he = hb = c2 * c3 * d;
    ke = kb = 6 * c3 * cos(beta) * d / (c2 + 4);

    vector < CGeoPoint > first_, second_;
    first_.push_back(w2 + u1 * d);
    first_.push_back(first_.back() + (-u1 * gb));
    first_.push_back(first_.back() + (-u1 * hb));

    second_.push_back(w2 +  u2 * d);;
    second_.push_back(second_.back() + (-u2 * ge));
    second_.push_back(second_.back() + (-u2 * he));

    CVector ud = second_.back() - first_.back();
    first_.push_back(first_.back() + ud * 0.7);
    second_.push_back(first_.back());
    reverse(second_.begin(), second_.end());
    return pair < vector < CGeoPoint >, vector < CGeoPoint > > (first_, second_) ;
}


///==============================================================================================================================//
///                                   BezierController class implementation
///==============================================================================================================================//

void BezierController::initController(const stateNew &start, const stateNew &end, const PlayerCapabilityT &capability, vector<CGeoPoint> viaPoints, runType rT) {

    if(viaPoints.size() < 2) {
        cout << "Oh shit!!! Too few viaPoints!!! --- BezierController initial function!!!" << endl;
        return;
    } else if(viaPoints[0].dist(start.pos) > 1e-8) {
        cout << "Oh shit!!! The first viaPoints should be the start point!!! ---BezierController initial function" << endl;
        return;
    } else if(viaPoints.back().dist(end.pos) > 1e-8) {
        cout << "Oh shit!!! The last viaPoints should be the target point!!! ---BezierController initial function" << endl;
        return;
    }

    initCurver(start, end, capability, viaPoints);
    _arrivedNum = 0;
    _lastOffDist = 0;
    _maxCurvature = 0;
    _runType = rT;
    _lastAdjustDist = 0;
    _lastLastAdjustDist = 0;
    _lastControerOutput = 0;
    _P = P;
    _I = I;
    _D = D;
}

void BezierController::planController() {
    if(!_pathPlanned) planPath(CURVE_PRECISION);

    _curvature.clear();
    vector < double > cur;
    for(BezierSubCurve* subCurver : _subCurver) { //calculate the curvature
        cur = subCurver->calculateCurvature();
        _curvature.insert(_curvature.end(), cur.begin(), cur.end());
    }
    validFullGeoPoints();
    calculateStates();
    validatePrecision();
}

void BezierController::validFullGeoPoints() {
    vector < CGeoPoint > fullPath;
    vector < double > cur;
    int i = 0;
    double tempDist = 0.0;
    _pathSegLength.clear();
    while(i < _fullGeoPathList.size()) {
        fullPath.push_back(_fullGeoPathList[i]);
        cur.push_back(_curvature[i]);
        i++;
        if(i < _fullGeoPathList.size()) {
            tempDist = _fullGeoPathList[i].dist(fullPath.back());
            _pathSegLength.push_back(tempDist);
        }
        while(i < _fullGeoPathList.size() && tempDist < 1e-8) {
            i++;
            tempDist = _fullGeoPathList[i].dist(fullPath.back());
        }
    }
    _fullGeoPathList.swap(fullPath);
    _curvature.swap(cur);
}

void BezierController::calculateVMax() {
    _vMax.clear();
    _maxCurvature = 0.0;
    double curMin = TOTAL_ACCEL / pow(_capability.maxSpeed, 2);
    for(double cur : _curvature) {
        if(cur > _maxCurvature)
            _maxCurvature = cur;
        if(cur > curMin) {
            double tempVel = double(sqrt(TOTAL_ACCEL / cur));
            _vMax.push_back(tempVel > _capability.maxSpeed ? _capability.maxSpeed : tempVel);
        } else {
            _vMax.push_back(_capability.maxSpeed);
        }
    }
}

void BezierController::calculateStates() {
    double minVel, currentTime;
    calculateVMax();
    vector < vector < double > > velList ;
    vector < vector < double > > timeList;
    calculateVelAndTimeList(velList, timeList);

    CVector front, behind;
    int pointNum = _fullGeoPathList.size();

    _stateList.resize(pointNum);
    _stateList[0] = _start;
    _time.resize(pointNum);
    _time[0] = _totalTime = 0.0;
    for ( int i = 1; i < pointNum - 1; i++) {
        minVel = 1e8;
        for(int j = 0; j < velList.size(); j++) {
            if(velList[j][i] < minVel) {
                minVel = velList[j][i];
                currentTime = timeList[j][i];
            }
        }
        _stateList[i].pos = _fullGeoPathList[i];
        front = _fullGeoPathList[i - 1] - _fullGeoPathList[i];
        behind = _fullGeoPathList[i + 1] - _fullGeoPathList[i];
        front = front / front.mod();
        behind = behind / behind.mod();
        _stateList[i].vel = behind - front;
        _stateList[i].vel = _stateList[i].vel / _stateList[i].vel.mod() * minVel;
        _stateList[i].rotVel = 0;
        _stateList[i].orient = _end.orient;
        _time[i] = currentTime;
        _totalTime += currentTime;
    }
    _stateList[pointNum - 1] = _end;

    currentTime = 0;
    for(int j = 0; j < velList.size(); j++)
        if(timeList[j][pointNum - 1] > currentTime)
            currentTime = timeList[j][pointNum - 1];
    _time[pointNum - 1] = currentTime;
    _totalTime += currentTime;
}

void BezierController::calculateVelAndTimeList(vector < vector < double > >& velList, vector < vector < double > >& timeList) {
    vector < int > IP = findInflecPoint();
    vector < double > tempVel, tempTime;
    double centriAccel, motionAccel, tempDist;
    int front, back, pointNum;

    pointNum = _curvature.size();
    tempVel.resize(pointNum);
    tempTime.resize(pointNum);
    for (int itor : IP) {
        if(itor == 0)
            tempVel[itor] = _start.vel.mod();
        else if(itor == pointNum - 1) {
            tempVel[itor] = _end.vel.mod();
        } else
            tempVel[itor] = _vMax[itor];
        front = back = itor;
        tempTime[0] = 0.0;
        while (front > 0) {
            centriAccel = pow(tempVel[front], 2) * _curvature[front];
            double root = pow(TOTAL_ACCEL, 2) - pow(centriAccel, 2);
            if(root <= 1e-8) {
                motionAccel = 0;
            } else {
                motionAccel = sqrt(root) * accFactorFront;
            }
            tempDist = _fullGeoPathList[front].dist(_fullGeoPathList[front - 1]);
            tempVel[front - 1] = sqrt(pow(tempVel[front], 2) + 2 * tempDist * motionAccel);
            if(tempVel[front - 1] > _vMax[front - 1])
                tempVel[front - 1] = _vMax[front - 1];
            tempTime[front] = (tempVel[front - 1] - tempVel[front]) / motionAccel;
            if(tempTime[front] < 0.0)
                tempTime[front] = 0.0;
            front--;
        }
        while (back < pointNum - 1) {
            centriAccel = pow(tempVel[back], 2) * _curvature[back];

            double root = pow(TOTAL_ACCEL, 2) - pow(centriAccel, 2);
            if(root <= 1e-8) {
                motionAccel = 0;
            } else {
                motionAccel = sqrt(root) * accFactorBack;
            }
            tempDist = _fullGeoPathList[back].dist(_fullGeoPathList[back + 1]);
            tempVel[back + 1] = sqrt(pow(tempVel[back], 2) + 2 * tempDist * motionAccel);
            if(tempVel[back + 1] > _vMax[back + 1])
                tempVel[back + 1] = _vMax[back + 1];
            tempTime[back + 1] = (tempVel[back + 1] - tempVel[back]) / motionAccel;
            if(tempTime[back + 1] < 0.0)
                tempTime[back + 1] = 0.0;
            back++;
        }
        velList.push_back(tempVel);
        timeList.push_back(tempTime);
    }
//    return pair < vector < vector < double > >, vector < vector < double > > > (velList, timeList);
}

vector < int > BezierController::findInflecPoint() {
    vector < int > IP;
    IP.push_back(0);
    int pointNum = _curvature.size();

    for (int i = 1; i < pointNum - 1; i++)
        if(_curvature[i] > MIN_IP_CURVATURE && _curvature[i] > _curvature[i - 1] && _curvature[i] > _curvature[i + 1]) {
            if(drawDebugMsg){
                GDebugEngine::Instance()->gui_debug_x(_fullGeoPathList[i], COLOR_PURPLE);
                GDebugEngine::Instance()->gui_debug_msg(_fullGeoPathList[i], QString("%1").arg(_curvature[i]).toLatin1(), COLOR_PURPLE);
            }
            IP.push_back(i);
        }
    IP.push_back(pointNum - 1);
    return IP;
}

stateNew BezierController::getNextState(const stateNew& currentState, double& nextCurva) {

    stateNew nextState;
    if(!_pathPlanned) {
        _arrivedNum = 0;
        nextCurva = 0.0;
        return _end;
    }
    if(_arrivedNum == _stateList.size() - 1) {
        nextCurva = _curvature.back();
        return _end;
    }
    int nearestIndex = 0;
    double nearestDist = 1e8;
    for(int i = 0; i < _stateList.size(); i++) {
        double tempDist = _stateList[i].pos.dist(currentState.pos);
        if(tempDist < nearestDist) {
            nearestDist = tempDist;
            nearestIndex = i;
        }
    }

    if(nearestIndex < _stateList.size() - 1) {
        _arrivedNum = nearestIndex + 1;
        nextCurva = _curvature[_arrivedNum];
        nextState = _stateList[_arrivedNum];
    } else {
        _arrivedNum = _stateList.size() - 1;
        nextCurva = _curvature.back();
        nextState = _end;
    }


    return nextState;
}

bool BezierController::isOffLine(const CGeoPoint &currentPosition, double thresholdL, double thresholdS) {
    if(_fullGeoPathList.empty())
        return true;
    double L, S;
    CGeoPoint start = _fullGeoPathList[_arrivedNum - 1],
              end = _fullGeoPathList[_arrivedNum + 1];
    distanceToSegment(start, end, currentPosition, L, S);
    bool OffLine = L > thresholdL || S > thresholdS;
    _lastOffDist = L;

    return OffLine;
}

/**
 * @brief BezierController::distanceToSegment to calculate the distance between a point and a segment
 * @param segStart: the start point of the segment
 * @param segEnd: the end point of the segment
 * @param targetPoint:
 * @param L: vertical distance
 * @param S: parallel distance
 */
void BezierController::distanceToSegment(const CGeoPoint &segStart, const CGeoPoint &segEnd, const CGeoPoint &targetPoint, double& L, double& S) {
    double x1 = segStart.x(),
           x2 = segEnd.x(),
           y1 = segStart.y(),
           y2 = segEnd.y(),
           x0 = targetPoint.x(),
           y0 = targetPoint.y();
    double k, interX, interY;
    if(x2 != x1) {
        k = (y2 - y1) / (x2 - x1);
        interX = (x0 / k + k * x1 + y0 - y1) / (k + 1 / k);
        interY = k * (interX - x1) + y1;
    } else {
        interX = x1;
        interY = y0;
    }
    CGeoPoint interPoint(interX, interY);
    CVector direc1 = segStart - interPoint,
            direc2 = segEnd - interPoint,
            segVec = segEnd - segStart,
            segTarget2Inter = interPoint - targetPoint;
    S = direc1 * direc2 > 0 ? (direc1 * segVec > 0 ? direc1.mod() : direc2.mod()) : 0;
    L = segTarget2Inter.mod();
}

void BezierController::validatePrecision() {

    vector < double > pathSegLength;

    vector < CGeoPoint > fullPathList;
    vector < stateNew > stateList;
    _originCurvature.clear();
    double nextSegLength = 0.0;
    int i = 0;
    while(i < _stateList.size() - 1) {
        nextSegLength = 0.0;
        stateList.push_back(_stateList[i]);
        fullPathList.push_back(_fullGeoPathList[i]);
        _originCurvature.push_back(_curvature[i]);
        nextSegLength += _pathSegLength[i];
        i++;
        while(i < _stateList.size() - 1 && stateList.back().pos.dist(_stateList[i].pos) * 2 / (stateList.back().vel.mod() + _stateList[i].vel.mod()) < MIN_FRAME_NUM_BETWEEN_PATH_POINT / PARAM::Vision::FRAME_RATE) {
            nextSegLength += _pathSegLength[i];
            i++;
        }
        pathSegLength.push_back(nextSegLength);
    }
    stateList.push_back(_end);
    fullPathList.push_back(_end.pos);
	_originCurvature.push_back(_curvature.back());
    _pathSegLength.swap(pathSegLength);
    _stateList.swap(stateList);
    _fullGeoPathList.swap(fullPathList);
    _curvature.swap(_originCurvature);

}

double BezierController::getTotalDist() {
    int beginItor = _arrivedNum == 0 ? _arrivedNum :_arrivedNum - 1;
    double totalDist = 0.0;
    for(int i = beginItor; i < _pathSegLength.size(); i++)
        totalDist += _pathSegLength[i];
    return totalDist;
}

double BezierController::evaluatePath(const stateNew& currentState, ObstaclesNew* obst, bool DebugMsgPos) {
    double firstCollisionDist = 0.0,  totalCurvature = 0.0, totalDistRatio = 0.0, velContriant = 0.0, distNextState = 0.0, distNextIP = 0.0;
    double pathScore;
    int posParam = DebugMsgPos ? 1 : -1;
    if(!_pathPlanned)
        return (-1e8);
    double nextCurva = 0.0;

    stateNew nextState = getNextState(currentState, nextCurva);
    double totalLength = getTotalDist();

    /// calculate first collision dist
    int backLineNum = 2;
    bool hasCollision = false;
    CGeoPoint checkPoint;
    for(int i = _arrivedNum; i < _stateList.size(); i++) {
        if(backLineNum > 0) {
            backLineNum--;
            checkPoint = currentState.pos;
        } else {
            checkPoint = _stateList[i - 1].pos;
        }
        if(i > 0 && obst->check(checkPoint, _stateList[i].pos))
            firstCollisionDist += _pathSegLength[i - 1];
        else {
            hasCollision = true;
            break;
        }
    }

    double colPenalty = 0.0;
    if(hasCollision && firstCollisionDist < 200)
        colPenalty = collisionPenalty;
    firstCollisionDist /= totalLength;
    /// max curvature of the whole path
    double maxCurva = 0.0;
    /// calculate total curvature
    for(int i = 0; i < _originCurvature.size(); i++) {
        if(_originCurvature[i] > MIN_IP_CURVATURE / 2) {
            totalCurvature += _originCurvature[i];
        }
        if(_originCurvature[i] > maxCurva)
            maxCurva = _originCurvature[i];
//        if(_curvature[i] > MIN_IP_CURVATURE / 2.0 && drawDebugMsg) {
//            GDebugEngine::Instance()->gui_debug_msg(_fullGeoPathList[i], QString("%1").arg(_curvature[i]).toLatin1(), COLOR_PURPLE);
//        }
    }

    if(maxCurva < minCurvaToCalculate) {
        maxCurva = 0.0;
    }
    else if(maxCurva < maxCurvaToDelete) {
        maxCurva = exp((maxCurva) * 10);
    }
    else {
        maxCurva = 1e8;
    }

    ///min dist to obst
    double minDistToObst = 1e8;
    for(int j = _arrivedNum; j < _stateList.size(); j++) {
        for(int i = 0; i < obst->obs.size(); i++) {
            double tempDist = obst->obs[i].closestDist(_stateList[j].pos);
            if(tempDist < minDistToObst)
                minDistToObst = tempDist;
        }
    }
    if(minDistToObst < safeDist)
        minDistToObst = -1.0;
    else if(minDistToObst < maxSafeDist)
        minDistToObst = (minDistToObst - safeDist) / (maxSafeDist - safeDist);
    else
        minDistToObst = 1.0;

    /// calculate total dist
    totalDistRatio = totalLength / currentState.pos.dist(_end.pos);

    ///initial vel constraient
    if(_arrivedNum > 0 && currentState.vel.mod() > 1e-8)
        velContriant = fabs(Utils::Normalize(currentState.vel.dir() - (_stateList[_arrivedNum].pos - _stateList[_arrivedNum - 1].pos).dir())) / PARAM::Math::PI;
    if(velContriant < 5.0 / 180) {
        velContriant = 0.0;
    }
    else {
        velContriant = exp((velContriant) * 5);
    }

    ///dist between current state and the next state
    distNextState = nextState.pos.dist(currentState.pos);

    ///dist next IP
    for(int i = _arrivedNum; i < _curvature.size(); i++) {
        if(_curvature[i] > MIN_IP_CURVATURE) {
            distNextIP = _stateList[i].pos.dist(currentState.pos);
            break;
        }
    }
    distNextIP /= totalLength;

    pathScore = wFirstCollisionDist * firstCollisionDist +
                wTotalCurvature * totalCurvature +
                wTotalDist * totalDistRatio +
                wInitialVelContr * velContriant +
                wMinDistToObst * minDistToObst +
                wMaxCurvature * maxCurva +
                wDistNextIP * distNextIP +
                colPenalty;
    if(drawDebugMsg) {
//        cout << "firstCollisionDist: " << wFirstCollisionDist * firstCollisionDist << endl
//             << "totalCurvature:     " << wTotalCurvature * totalCurvature << endl
//             << "totalDist:          " << wTotalDist * totalDistRatio << endl
//             << "velContriant:       " << wInitialVelContr * velContriant << endl
//             << "distNextState:      " << wDistNextState * distNextState << endl
//             << "distNextIP:         " << wDistNextIP * distNextIP << endl
//             << "totalScore:         " << pathScore  << endl << endl;
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(300 * posParam - 20, -400), QString("firstCollisionDist: %1").arg(wFirstCollisionDist * firstCollisionDist).toLatin1(), COLOR_PURPLE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(300 * posParam - 20, -380), QString("totalCurvature:     %1").arg(wTotalCurvature * totalCurvature).toLatin1(), COLOR_PURPLE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(300 * posParam - 20, -360), QString("totalDistRatio:     %1").arg(wTotalDist * totalDistRatio).toLatin1(), COLOR_PURPLE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(300 * posParam - 20, -340), QString("velContriant:       %1").arg(wInitialVelContr * velContriant).toLatin1(), COLOR_PURPLE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(300 * posParam - 20, -320), QString("maxCurva:           %1").arg(wMaxCurvature * maxCurva).toLatin1(), COLOR_PURPLE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(300 * posParam - 20, -300), QString("distNextIP:         %1").arg(wDistNextIP * distNextIP).toLatin1(), COLOR_PURPLE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(300 * posParam - 20, -280), QString("minDistToObst:      %1").arg(wMinDistToObst * minDistToObst).toLatin1(), COLOR_PURPLE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(300 * posParam - 20, -260), QString("totalScore:         %1").arg(pathScore).toLatin1(), COLOR_PURPLE);
    }
    return pathScore;
}

double BezierController::getTime() {
    double time = 0;
    for(int i = _arrivedNum; i < _time.size(); i++)
        time += _time[i];
    return time;
}


