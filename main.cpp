#include <iostream>
#include "cmath"
#include "global.h"
/**
 * du/dt + Vx*du/dx + Vy*du/dy=0
 */

using namespace std;

const int N = 100;

const double H = 1.0/N;

const double TMAX = 1.0;
const double TAU = 1.e-6;

VECTOR u[N+2][N+2];
Point  c[N+2][N+2]; // центры ячеек

VECTOR u1[N+2][N+2]; // сюда суммируем интегралы

Point cellGP[4];
double cellGW[4];

// матрица масс
double			****matrA;
double			****matrInvA;


const int FUNC_COUNT = 3;

VECTOR getF(int i, int j, Point pt)
{
    if (FUNC_COUNT == 3 )
    {
        VECTOR v(3);
        v[0] = 1.0;
        v[1] = (pt.x - c[i][j].x)/H;
        v[2] = (pt.y - c[i][j].y)/H;
        return v;
    } else {
        std::cout << "ERROR: wrong basic functions count!\n";
        return 1;
    }
}

VECTOR getDFDX(int iCell, Point pt)
{
    if (FUNC_COUNT == 3 )
    {
        VECTOR v(3);
        v[0] = 0.0;
        v[1] = 1.0/H;
        v[2] = 0.0;
        return v;
    } else {
        std::cout << "ERROR: wrong basic functions count!\n";
        return 1;
    }
}

VECTOR getDFDY(int iCell, Point pt)
{
    if (FUNC_COUNT == 3 )
    {
        VECTOR v(3);
        v[0] = 0.0;
        v[1] = 0.0;
        v[2] = 1.0/H;
        return v;
    } else {
        std::cout << "ERROR: wrong basic functions count!\n";
        return 1;
    }
}

inline double getU(int i, int j, Point pt) { return VECTOR::SCALAR_PROD(u[i][j], getF(i, j, pt)); }


int main() {

    // заполняем вспомогательные массивы
    double sqrt3 = 1.0/sqrt(3.0);

    cellGP[0].x = -sqrt3;
    cellGP[0].y = -sqrt3;

    cellGP[1].x =  sqrt3;
    cellGP[1].y = -sqrt3;

    cellGP[2].x = sqrt3;
    cellGP[2].y = sqrt3;

    cellGP[3].x = -sqrt3;
    cellGP[3].y =  sqrt3;

    matrA = new double***[N+2];
    matrInvA = new double***[N+2];
    for (int i = 0; i < N+2; i++) {
        matrA[i] = new double**[N+2];
        matrInvA[i] = new double**[N+2];
        for (int j = 0; j < N+2; j++) {
            matrA[i][j] = new double*[FUNC_COUNT];
            matrInvA[i][j] = new double*[FUNC_COUNT];
            for (int k = 0; k < FUNC_COUNT; k++) {
                matrA[i][j][k] = new double[FUNC_COUNT];
                matrInvA[i][j][k] = new double[FUNC_COUNT];
            }
        }
    }

    // вычисляем матрицу масс
    for (int i = 1; i <= N; i++) {
        for (int j = 1; j <= N; j++) {
            double **a = matrA[i][j];
            for (int m = 0; m < FUNC_COUNT; m++) {
                for (int n = 0; n < FUNC_COUNT; n++) {
                    a[m][n] = 0.0;
                }
            }
            for (int igp = 0; igp < 4; igp++) {
                Point gp = cellGP[i][j];
                gp *= H; // todo: неоходимо изменить если не квадратные ячейки
                gp += c[i][j];
                gp *= 0.5;
                VECTOR phi1 = getF(i,j, gp);
                VECTOR phi2 = getF(i,j, gp);
                for (int m = 0; m < FUNC_COUNT; m++) {
                    for (int n = 0; n < FUNC_COUNT; n++) {
                        a[m][n] += phi1[m] * phi2[n] *  cellGW[igp];
                    }
                }
            }
        }
    }

    int step =0;
    double t = 0.0;
    while (t < TMAX) {
        step++;
        t += TAU;


        // вычисляем двойной интеграл
        for (int i = 1; i < N+1; i++) {
            for (int j = 0; j < N+1; j++) {
                VECTOR s(FUNC_COUNT);
                s = 0.0;
                for (int igp = 0; igp < 4; igp++) {
                    Point gp = cellGP[i][j];
                    gp *= H; // todo: неоходимо изменить если не квадратные ячейки
                    gp += c[i][j];
                    gp *= 0.5;
                    s += (getU(i,j,gp)*getDFDX(i,j,gp)+getU(i,j,gp)*getDFDY(i,j,gp));
                    s *= cellGW[igp];
                }

                u1[i][j] = s;
            }
        }


        // вычисляем криволинейный интеграл по границе квадрата

        // x-direction
        for (int i = 0; i < N+1; i++) {
            for (int j = 1; j < N+1; j++) {
                Point gp1, gp2;
                gp1 = c[i][j];
                gp1.x += 0.5*H;
                gp2 = gp1;
                gp1.y -= sqrt3*H*0.5;
                gp2.y += sqrt3*H*0.5;
                double flux1 = getU(i, j, gp1);
                double flux2 = getU(i, j, gp2);

                VECTOR sL(FUNC_COUNT), sR(FUNC_COUNT), tmp(FUNC_COUNT);
                tmp = getF(i,j,gp1);
                tmp *= flux1;
                sL += tmp;
                tmp = getF(i,j,gp1);
                tmp *= flux2;
                sL += tmp;
                sL *= H*0.5;

                tmp = getF(i,j,gp2);
                tmp *= flux1;
                sR += tmp;
                tmp = getF(i,j,gp2);
                tmp *= flux2;
                sR += tmp;
                sR *= H*0.5;

                u1[i][j]   -= sL;
                u1[i+1][j] += sR;
            }
        }

        // y-direction
        for (int i = 1; i < N+1; i++) {
            for (int j = 0; j < N+1; j++) {
                Point gp1, gp2;
                gp1 = c[i][j];
                gp1.y += 0.5*H;
                gp2 = gp1;
                gp1.x -= sqrt3*H*0.5;
                gp2.x += sqrt3*H*0.5;
                double flux1 = getU(i, j, gp1);
                double flux2 = getU(i, j, gp2);

                VECTOR sL(FUNC_COUNT), sR(FUNC_COUNT), tmp(FUNC_COUNT);
                tmp = getF(i,j,gp1);
                tmp *= flux1;
                sL += tmp;
                tmp = getF(i,j,gp1);
                tmp *= flux2;
                sL += tmp;
                sL *= H*0.5;

                tmp = getF(i,j,gp2);
                tmp *= flux1;
                sR += tmp;
                tmp = getF(i,j,gp2);
                tmp *= flux2;
                sR += tmp;
                sR *= H*0.5;

                u1[i][j]   -= sL;
                u1[i][j+1] += sR;
            }
        }


        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                u1[i][j] *= matrInvA[i][j];
                u1[i][j] *= TAU;

                u[i][j] += u1[i][j];
            }
        }

        // вывод через определенное количество шагов
        // ....
    }


    // завершение приложения, очистка памяти..

    return 0;
}