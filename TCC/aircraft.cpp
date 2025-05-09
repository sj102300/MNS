//
//#include "aircraft.h"
//
//bool AirCraft::isEnemy() {
//
//}
//
//TCC::Position AirCraft::getImpactPoint() {
//	// bool을 리턴하고, 결과는 reference 파라미터로 반환
//	//bool computeImpactPoint(
//	//    double tx, double ty,         // 항공기 위치
//	//    double dx_t, double dy_t,     // 항공기 방향 벡터
//	//    double mx, double my,         // 미사일 위치
//	//    double& impact_x, double& impact_y, double& impact_t,
//	//    double vt = 1.0,              // 항공기 속도 (km/s)
//	//    double vm = 2.0               // 미사일 속도 (km/s)
//	//) {
//	//    // 항공기 방향 벡터 정규화
//	//    double norm = sqrt(dx_t * dx_t + dy_t * dy_t);
//	//    dx_t /= norm;
//	//    dy_t /= norm;
//
//	//    // t에 대한 이차방정식 계수
//	//    double a = vt * vt - vm * vm;
//	//    double b = 2 * vt * ((tx - mx) * dx_t + (ty - my) * dy_t);
//	//    double c = (tx - mx) * (tx - mx) + (ty - my) * (ty - my);
//
//	//    double discriminant = b * b - 4 * a * c;
//	//    if (discriminant < 0) {
//	//        return false; // 요격 불가
//	//    }
//
//	//    double sqrtD = sqrt(discriminant);
//	//    double t1 = (-b + sqrtD) / (2 * a);
//	//    double t2 = (-b - sqrtD) / (2 * a);
//	//    double t = (t1 > 0) ? t1 : ((t2 > 0) ? t2 : -1);
//	//    if (t < 0) return false; // 미래에 요격 불가
//
//	//    // impact point 계산
//	//    impact_x = tx + vt * t * dx_t;
//	//    impact_y = ty + vt * t * dy_t;
//	//    impact_t = t;
//
//	//    return true;
//	//}
//
//}
//
//bool AirCraft::canEngageTarget() {
//
//}
