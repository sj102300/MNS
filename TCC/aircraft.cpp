//
//#include "aircraft.h"
//
//bool AirCraft::isEnemy() {
//
//}
//
//TCC::Position AirCraft::getImpactPoint() {
//	// bool�� �����ϰ�, ����� reference �Ķ���ͷ� ��ȯ
//	//bool computeImpactPoint(
//	//    double tx, double ty,         // �װ��� ��ġ
//	//    double dx_t, double dy_t,     // �װ��� ���� ����
//	//    double mx, double my,         // �̻��� ��ġ
//	//    double& impact_x, double& impact_y, double& impact_t,
//	//    double vt = 1.0,              // �װ��� �ӵ� (km/s)
//	//    double vm = 2.0               // �̻��� �ӵ� (km/s)
//	//) {
//	//    // �װ��� ���� ���� ����ȭ
//	//    double norm = sqrt(dx_t * dx_t + dy_t * dy_t);
//	//    dx_t /= norm;
//	//    dy_t /= norm;
//
//	//    // t�� ���� ���������� ���
//	//    double a = vt * vt - vm * vm;
//	//    double b = 2 * vt * ((tx - mx) * dx_t + (ty - my) * dy_t);
//	//    double c = (tx - mx) * (tx - mx) + (ty - my) * (ty - my);
//
//	//    double discriminant = b * b - 4 * a * c;
//	//    if (discriminant < 0) {
//	//        return false; // ��� �Ұ�
//	//    }
//
//	//    double sqrtD = sqrt(discriminant);
//	//    double t1 = (-b + sqrtD) / (2 * a);
//	//    double t2 = (-b - sqrtD) / (2 * a);
//	//    double t = (t1 > 0) ? t1 : ((t2 > 0) ? t2 : -1);
//	//    if (t < 0) return false; // �̷��� ��� �Ұ�
//
//	//    // impact point ���
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
