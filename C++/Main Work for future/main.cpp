// Старая задача
#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>
#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

double c = 3e+10, OMEGA_P_0 = 3e+9, J0 = 1;                         // Глобальные переменные (скорость света, максимальная плазменная частота и плотность тока)

double fdtd( double THETA, double NU_TILDA, double R2_TILDA, double DELTA )
{
    double R1_tilda = R2_TILDA * ( 1 - DELTA );                     // Внутренний (обезразмеренный) радиус цилиндра
    double NU = NU_TILDA * OMEGA_P_0;                               // Частота соударений
    double R1 = R1_tilda * c / OMEGA_P_0;                           // Внутренний радиус цилиндра
    double R2 = R2_TILDA * c / OMEGA_P_0;                           // Внешний радиус цилиндра
    double dr = 0.01 * NU_TILDA * ( R2 - R1 );                      // Шаг по пространству
    double dt = dr / ( c * 2 );                                     // Шаг по времени
    double dt_tilda = dt * OMEGA_P_0;                               // Шаг по времени обезразмеренный
    double T_MAX = 60;                                              // Расчетное (обезразмеренное) время
    int N_TIME = T_MAX / ( dt * OMEGA_P_0 );                        // Кол-во шагов по времени

    //N_TIME = 5;
    vector <long double> T( N_TIME );                               // Вектор времени
    for ( int i = 0; i < N_TIME; i++ )                              //
    {                                                               //
        T[i] = dt * i;                                              //
    }                                                               //


    double R_MAX = R2 * 60;                                         // Расчетное пространство (по r)
    int NR = ( R_MAX + dr ) / dr;                                   // Кол-во шагов по пространству
    vector <double> r( NR ), r_tilda( NR ), r_alt( NR );            // Обозначаем r и 1/r
    for ( int i = 0; i < NR; i++ )                                  //
    {                                                               //
        r[i] = dr + dr * i;                                         //
        r_alt[i] = 1 / r[i];                                        //
        r_tilda[i] = r[i] * OMEGA_P_0 / c;                          //
    }                                                               //

    int NR1 =  R1 / dr, NR2 = R2 / dr;                              // Начало и конец неоднородного слоя (в кол-ве узлов)

    // Точка проверки значения поля от времени
    int FIELD_CHECK_POINT = NR2, FIELD_CHECK_POINT_TILDA = FIELD_CHECK_POINT * dr * OMEGA_P_0 / c;
    if ( NR1 == 0 ) NR1 = 1;


    vector <double> Fr( NR );                                       // Обозначаем F(r)
    for ( int i = 0; i < NR1; i++ )
    {
        Fr[i] = 1;
    }
    for ( int i = NR1; i < NR2; i++ )
    {
        Fr[i] = ( cos( (double)M_PI * (double)( i - NR1 ) / ( 2 * (int)( NR2 - NR1 ) ) ) ) * ( cos( (double)M_PI * (double)( i - NR1 ) / ( 2 * ( NR2 - NR1 ) ) ) );
    }
    for ( int i = NR2; i < NR; i++ )
    {
        Fr[i] = 0;
    }


    int N_PML = 10 * NR2;                                           // Поглощающий слой
    vector <double> sigma( NR );                                    // Начало поглощения
    for ( int i = 0; i < NR - N_PML; i++ )
    {
        sigma[i] = 1;
    }
    for ( int i = NR - N_PML; i < NR; i++ )
    {
        sigma[i] = cos( (double)( M_PI / 2 ) * ( i - ( NR - N_PML ) ) / ( NR - ( NR - N_PML ) ) );
        //sigma[i] = 1;
    }
    sigma[NR - 1] = 0;

    vector <double> Er( NR ), Ephi( NR ), Ez( NR );                     // Проекции электрических полей
    vector <double> Hr( NR ), Hphi( NR ), Hz( NR );                     // Проекции магнитных полей
    vector <double> Jr( NR ), Jphi( NR ), Jz( NR );                     // Проекции токовых компонент
    vector <double> Ept( N_TIME ), Hzt( N_TIME );                       // E_phi(t), H_z(t)

    //Начальные условия
    for ( int i = 0; i < NR; i++ )
    {
        Er[i] = 0;
        Ephi[i] = 0;

        Hz[i] = 0;

        Jr[i] =  Fr[i];
        Jphi[i] = - Fr[i];
    }

    ofstream fout;
    string PATH = "main_data.dat";
    fout.open( PATH );
    fout << left << setw( 11 ) << "T" << "\t";
    fout << left << setw( 11 ) << "Er(t)" << "\t" << left << setw( 11 ) << "Ephi(t)" << "\t" << left << setw( 11 ) << "Hz(t)" << "\t";
    fout << left << setw( 11 ) << "Jr(t)" << "\t" << left << setw( 11 ) << "Jphi(t)";
    fout << endl;

    for ( int n = 0; n < N_TIME; n++ )
    {
        for ( int i = 0; i < NR; i++ )
        {
            Jr[i] = Jr[i] * (1 - dt_tilda * NU_TILDA) + dt_tilda * Fr[i] * Er[i];
        }
        //Jp
        for ( int i = 0; i < NR; i++ )
        {
            Jphi[i] = Jphi[i] * ( 1 - dt_tilda * NU_TILDA ) + dt_tilda * Fr[i] * Ephi[i];
        }
        //Er
        Er[0] = Er[1];
        for ( int i = 1; i < NR; i++ )
        {
            Er[i] = sigma[i] * ( Er[i] + ( dt_tilda / r_tilda[i] ) * Hz[i] - dt_tilda * Jr[i] );
        }
        //Ephi
        Ephi[0] = Ephi[1];
        for ( int i = 1; i < NR; i++ )
        {
            Ephi[i] = sigma[i] * ( Ephi[i] - ( dt_tilda / ( dr * OMEGA_P_0 ) ) * ( Hz[i] - Hz[i-1] ) - dt_tilda * Jphi[i] );
        }
        //Hz
        for ( int i = 0; i < NR - 1; i++ )
        {
            Hz[i] = sigma[i] * ( Hz[i] - ( dt_tilda / r_tilda[i] ) * ( Er[i] + ( r_tilda[i + 1] * Ephi[i + 1] - r_tilda[i] * Ephi[i] ) / ( dr * OMEGA_P_0 ) ) );
        }
        //Hz[ NR - 1 ] = sigma[ NR - 1 ] * ( Hz[ NR - 1 ] - ( c * dt * r_alt[ NR - 1 ] ) * Er[ NR - 1 ]);
        Hz[ NR - 1 ] = 0;
        Ept[n] = Ephi[FIELD_CHECK_POINT];
        Hzt[n] = Hz[FIELD_CHECK_POINT];

        cout << "Loading... " << ( n * 100 / N_TIME ) + 1 << "/" << 100 << "%\r";
        fout << left << setw( 11 ) << dt_tilda * n << "\t";
        fout << left << setw( 11 ) << Er[FIELD_CHECK_POINT] << "\t" << left << setw( 11 ) << Ephi[FIELD_CHECK_POINT] << "\t" << left << setw( 11 ) << left << setw( 11 ) << Hz[FIELD_CHECK_POINT] << "\t";
        fout << left << setw( 11 ) << Jr[( NR2 + NR1 ) / 2] << "\t" << left << setw( 11 ) << Jphi[( NR2 + NR1 ) / 2] ;
        fout << endl;
    }
    fout.close();

    double I = 0;
    for ( int i = 1; i < N_TIME; i++ )
    {
        I += ( Ept[i] * Hzt[i] );
    }
    I += ( Ept[0] * Hzt[0] + Ept[N_TIME-1] * Hzt[N_TIME - 1] ) / 2;

    double W_zap, W_izl;
    W_zap = ( R2_TILDA * R2_TILDA + R1_tilda * R1_tilda ) ;
    W_izl = ( dt_tilda / 4 ) * FIELD_CHECK_POINT_TILDA * dr * OMEGA_P_0 * I;

    cout << "\rWell done!         \n";
    cout << "File saved in path: " << PATH << endl;

    return W_izl / W_zap;
}


int main()
{
    setlocale( LC_ALL,"Rus" ); // Русский язык в консоли

    // Основные параметры задачи
    double THETA_MULTIPLICATOR, NU_TILDA, R2_TILDA, DELTA;
    do
    {
        cout << "Theta miltiplicator (PI x | |) = ";
        cin >> THETA_MULTIPLICATOR;
    }
    while ( THETA_MULTIPLICATOR < 0 || THETA_MULTIPLICATOR > 0.5);
    cout << "nu = ";
    cin >> NU_TILDA;
    cout << "R2 = ";
    cin >> R2_TILDA;
    do
    {
        cout << "Delta = ";
        cin >> DELTA;
    }
    while ( DELTA < 0 || DELTA > 1 );

    cout << fdtd( THETA_MULTIPLICATOR * M_PI, NU_TILDA, R2_TILDA, DELTA );
    cout << "\a" <<endl;

    //cout << THETA_MULTIPLICATOR * M_PI << "  " << cos( THETA_MULTIPLICATOR * M_PI );

    //fstream
    //ifstream
    //ofstream


    //if (Fr.size() == r.size( ) ) cout << "\nYES";

    //cout << r.size() << "\t" << Fr.size() << "\t" << Nr << endl;




    return 0;
}
