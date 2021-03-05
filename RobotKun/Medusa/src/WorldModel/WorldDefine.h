#ifndef _WORLD_DEFINE_H_
#define _WORLD_DEFINE_H_
#include <geometry.h>
#include <server.h>
/************************************************************************/
/*                       ObjectPoseT                                    */
/************************************************************************/
class ObjectPoseT {
public:
    ObjectPoseT() : _valid(false), _pos(CGeoPoint(-9999, -9999)) { }
    const CGeoPoint& Pos() const {
        return _pos;
    }
    void SetPos(double x, double y) {
        _pos = CGeoPoint(x, y);
    }
    void SetPos(const CGeoPoint& pos) {
        _pos = pos;
    }
    double X() const {
        return _pos.x();
    }
    double Y() const {
        return _pos.y();
    }
    void SetVel(double x, double y) {
        _vel = CVector(x, y);
    }
    void SetVel(const CVector& vel) {
        _vel = vel;
    }
    void SetRawVel(double x, double y) {
        _rawVel = CVector(x, y);
    }
    void SetRawVel(const CVector& vel) {
        _rawVel = vel;
    }
    void SetAcc(double x, double y) {
        _acc = CVector(x, y);
    }
    void SetAcc(const CVector& acc) {
        _acc = acc;
    }
    const CVector& Vel() const {
        return _vel;
    }
    const CVector& RawVel() const {
        return _rawVel;
    }
    const CVector& Acc() const {
        return _acc;
    }
    double VelX() const {
        return _vel.x();
    }
    double VelY() const {
        return _vel.y();
    }
    double AccX() const {
        return _acc.x();
    }
    double AccY() const {
        return _acc.y();
    }
    void SetValid(bool v) {
        _valid = v;
    }
    bool Valid() const {
        return _valid;
    }
private:
    CGeoPoint _pos;
    CVector _vel;
    CVector _rawVel;
    CVector _acc;
    bool _valid;
};
/************************************************************************/
/*                      VisionObjectT                                   */
/************************************************************************/
class VisionObjectT {
public:
    VisionObjectT() : _rawPos(CGeoPoint(-9999, -9999)) { }
    const CGeoPoint& RawPos() const {
        return _rawPos;
    }
    const CVector3 RealPos() const{
        return _realPos;
    }
    const CGeoPoint& ChipPredictPos() const {
        return _chipPredict;
    }
    // if get pos=CGeoPoint(10000,10000), then it hasn't been calculated already, gh
    const CGeoPoint& BestChipPredictPos() const{
        return _bestChipPredictPos;
    }
    const CGeoPoint& SecondChipPos() const{
        return _secondChipPos;
    }
    const CVector3& chipKickVel() const{
        return _chipKickVel;
    }
    double ChipFlyTime() const{
        return _chipFlyTime;
    }
    double ChipRestFlyTime() const{
        return _chipRestFlyTime;
    }
    // get chip state, gh
    bool ChipBallState() const{
        return _chipBallState;
    }
    double RawDir() const {
        return _rawDir;
    }
    double ImuDir() const {
        return _imuDir;
    }
    double ImuRotateVel() const {
        return _imuRotateVel;
    }
    void SetRealPos(const CVector3& realPos){
        _realPos = CVector3(realPos.x(), realPos.y(), realPos.z());
    }
    void SetRealPos(double x, double y ,double z) {
        _realPos = CVector3(x, y, z);
    }
    void SetChipPredict(const CGeoPoint& chipPos) {
        _chipPredict = chipPos;
    }
    void SetChipPredict(double x, double y) {
        _chipPredict =  CGeoPoint(x, y);
    }
    void SetBestChipPredictPos(double x, double y){
        _bestChipPredictPos = CGeoPoint(x, y);
    }
    void SetSecondChipPos(double x, double y){
        _secondChipPos = CGeoPoint(x, y);
    }
    void SetChipFlyTime(double t){
        _chipFlyTime = t;
    }
    void SetChipRestFlyTime(double t){
        _chipRestFlyTime = t;
    }
    void SetChipKickVel(double x, double y, double z) {
        _chipKickVel = CVector3(x, y, z);
    }
    void SetChipBallState(int state){
        _chipBallState = (state == ballState::_chip_pass);
    }
    void SetRawPos(double x, double y) {
        _rawPos = CGeoPoint(x, y);
    }
    void SetRawPos(const CGeoPoint& pos) {
        _rawPos = pos;
    }
    void SetRawDir(double rawdir) {
        _rawDir = rawdir;
    }
    void SetImuDir(double imudir) {
        //modified 2020.9.17
        if (abs(imudir) < 1e-5) {
            _imuDir = _rawDir;
        }
        else{
            _imuDir = imudir;
        }
    }
    void SetImuRotateVel(double imurotatevel) {
        _imuRotateVel = imurotatevel;
    }
private:
    CGeoPoint _rawPos; // 视觉的原始信息，没有经过预测
    CGeoPoint _chipPredict; // 挑球预测 for every time
    CGeoPoint _bestChipPredictPos; // the best/stable chip predict pos, gh
    CGeoPoint _secondChipPos; // 第二落球點
    CVector3 _realPos;
    CVector3 _chipKickVel;
    double _chipFlyTime;
    double _chipRestFlyTime;
    bool _chipBallState; // is chip or not, gh
    double _rawDir;
    double _imuDir;
    double _imuRotateVel;
};
/************************************************************************/
/*                       MobileVisionT                                  */
/************************************************************************/
class MobileVisionT : public ObjectPoseT, public VisionObjectT {

};
/************************************************************************/
/*                        机器人姿态数据结构                               */
/************************************************************************/
struct PlayerPoseT : public ObjectPoseT { // 目标信息
public:
    bool needChangeDirBias;
    double imuDirBias;
    int countNum;
    double visionDirAve;
    void SetNeedChangeDirBias(bool need) {
        needChangeDirBias = need;
    }
    void SetImuDirBias(double dirbias) {
        imuDirBias = dirbias;
    }
    void SetVisionDirAve(double dirAve) {
        visionDirAve = dirAve;
    }
    void SetCountNum(int num) {
        countNum = num;
    }
    PlayerPoseT() : _dir(0), _rotVel(0), needChangeDirBias(false), imuDirBias(0), countNum(0), visionDirAve(0) { }
    //    PlayerPoseT() : _dir(0), _rotVel(0) { }
    double Dir() const {
        return _dir;
    }
    void SetDir(double d) {
        _dir = d;
    }
    double RotVel() const {
        return _rotVel;
    }
    void SetRotVel(double d) {
        _rotVel = d;
    }
    double RawRotVel() const {
        return _rawRotVel;
    }
    void SetRawRotVel(double d) {
        _rawRotVel = d;
    }
private:
    double _dir; // 朝向
    double _rotVel; // 旋转速度
    double _rawRotVel;
};
/************************************************************************/
/*                      PlayerTypeT                                     */
/************************************************************************/
class PlayerTypeT {
public:
    PlayerTypeT(): _type(0) {}
    void SetType(int t) {
        _type = t;
    }
    int Type() const {
        return _type;
    }
private:
    int _type;
};
/************************************************************************/
/*                       PlayerVisionT                                  */
/************************************************************************/
class PlayerVisionT : public PlayerPoseT, public VisionObjectT, public PlayerTypeT {

};

/************************************************************************/
/*                        机器人能力数据结构                               */
/************************************************************************/
struct PlayerCapabilityT {
    PlayerCapabilityT(): maxAccel(0), maxSpeed(0), maxAngularAccel(0), maxAngularSpeed(0), maxDec(0), maxAngularDec(0) {}
    double maxAccel; // 最大加速度
    double maxSpeed; // 最大速度
    double maxAngularAccel; // 最大角加速度
    double maxAngularSpeed; // 最大角速度
    double maxDec;          // 最大减速度
    double maxAngularDec;   // 最大角减速度
};
#endif // _WORLD_DEFINE_H_
