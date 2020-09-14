/* Geramos arquivos "tp.in" e um ("pra_init.txt") que será usado pelo swift/tools/init_tp.x, 
para partículas que fazem parte de um disco dinamicamente frio */

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <math.h>

using namespace std;

/* Função que calcula o seno de um ângulo */
double s(double angle)
{
    double x;
    const double pi = 3.141592653589793;
    int nper;

    // Reduzimos o ângulo à faixa de 0 a 2pi
    nper = angle/(2*pi);
    x = angle - nper*(2*pi);
    if(x < 0)
        x = x + 2*pi;
    return sin(x);
}

/* Função que calcula o coseno de um ângulo */
double c(double x)
{
    double sx, cx;
    const double pi = 3.141592653589793;

    sx = s(x);
    cx = sqrt(1 - pow(sx, 2));
    if((x > pi/2) & (x < 3*pi/2))
        cx = -cx;
    return cx;
}

/* Função que calcula a anomalia excêntrica, E, de uma elipse (M = anomalia média) */
double EqKepler1(double e,double M)
{
    int iflag, nper, niter, NMAX;
    const double pi = 3.141592653589793;
    double sM, cM, x, esx, ecx, f, fp, fpp, fppp, dx, E, esxM, ecxM;

    if(e < 0.18)
    // Para uma rapidez e precisão suficiente
    {
        sM = s(M);
        cM = c(M);
        x = M + e*sM*(1 + e*(cM + e*(1 - 1.5*pow(sM, 2))));
        esx = e*s(x);
        ecx = e*c(x);
        f = x - esx - M;
        fp = 1 - ecx;
        fpp = esx;
        fppp = ecx;
        dx = -f/fp;
        dx = -f/(fp + dx*fpp/2);
        dx = -f/(fp + dx*fpp/2 + pow(dx, 2)*fppp/6);

        E = x + dx;
    }
    else
    {
        if(e <= 0.8)
        // 0.18 <= e <= 0.8
        {
            sM = s(M);
            cM = c(M);
            x = M + e*sM*(1 + e*(cM + e*(1 - 1.5*pow(sM, 2))));
            esx = e*s(x);
            ecx = e*c(x);
            f = x - esx - M;
            fp = 1 - ecx;
            fpp = esx;
            fppp = ecx;
            dx = -f/fp;
            dx = -f/(fp + dx*fpp/2);
            dx = -f/(fp + dx*fpp/2 + pow(dx, 2)*fppp/6);
            E = x + dx;
            // Fazer outra iteração
            x = E;
            esx = e*s(x);
            ecx = e*c(x);
            f = x - esx - M;
            fp = 1 - ecx;
            fpp = esx;
            fppp = ecx;
            dx = -f/fp;
            dx = -f/(fp + dx*fpp/2);
            dx = -f/(fp + dx*fpp/2 + pow(dx, 2)*fppp/6);

            E = x + dx;
        }
        else
        // e > 0.8 Este pode não convergir suficientemente rápido
        {
            // Modificamos M de modo que E e M estejam na faixa de 0 a 2pi.
            // Se M está entre pi e 2pi resolvemos para 2pi - M e usamos simetria
            // para retornar a resposta certa
            iflag = 0;
            nper = M/(2*pi);
            M = M - nper*(2*pi);
            if(M < 0)
                M = M + 2*pi;
            if(M > pi)
            {
                M = 2*pi - M;
                iflag = 1;
            }

            x = pow(6*M, 1/3) - M;
            NMAX = 3;
            for(niter = 1; niter <= NMAX; niter++)
            {
                esxM = e*s(x + M);
                ecxM = e*c(x + M);
                f = x - esxM;
                fp = 1 - ecxM;
                dx = -f/fp;
                dx = -f/(fp + 0.5*dx*esxM);
                dx = -f/(fp + 0.5*dx*(esxM + 0.3333333333333333*ecxM*dx));
                x = x + dx;
            }
            E = M + x;
            if(iflag == 1)
            {
                E = 2*pi - E;
                M = 2*pi - M;
            }
        }
    }
    return E;
}

/* Função que calcula a anomalia excêntrica, Z, de uma parábola (Q = anomalia média) */
double EqKepler2(double Q)
{
    int iflag;
    double x, tmp, Z;

    iflag = 0;
    if(Q < 0)
    {
        iflag = 1;
        Q = -Q;
    }
    if(Q < 0.001)
        Z = Q*(1 - pow(Q, 2)*(1 - pow(Q, 2))/3);
    else
    {
        x = 0.5*(3*Q + sqrt(9*pow(Q, 2) + 4));
        tmp = pow(x, 1/3);
        Z = tmp - 1/tmp;
    }

    if(iflag == 1)
    {
        Z = -Z;
        Q = -Q;
    }
    return Z;
}

/* Função que calcula a anomalia excêntrica, F, de uma hipérbole (N = anomalia média) */
double EqKepler3(double e, double N)
{
    int i, IMAX, iflag;
    double abN, tmp, x, shx, chx, eshx, echx, f, fp, fpp, fppp, dx, F;
    double a, b, sq, biga, bigb, x2, a0, a1, a3, a5, a7, a9, a11, b1, b3, b5, b7, b9, b11;

    abN = N;
    if(N < 0)
        abN = -abN;

    if(abN < 0.636*e - 0.6)
    // Só é bom para valores baixos de N (N < 0.636*e - 0.6)
    {
        a11 = 156;
        a9 = 17160;
        a7 = 1235520;
        a5 = 51891840;
        a3 = 1037836800;
        b11 = 11*a11;
        b9 = 9*a9;
        b7 = 7*a7;
        b5 = 5*a5;
        b3 = 3*a3;

        // Colocar iflag diferente de zero se N < 0, caso em que resolve para -N, e
        // mudar o signo da resposta final para F
        iflag = 0;
        if(N < 0)
        {
            iflag = 1;
            N = -N;
        }

        a1 = 6227020800*(1 - 1/e);
        a0 = -6227020800*N/e;
        b1 = a1;

        a = 6*(e - 1)/e;
        b = -6*N/e;
        sq = sqrt(0.25*pow(b, 2) + pow(a, 3)/27);
        biga = pow(-0.5*b + sq, 0.3333333333333333);
        bigb = -pow(0.5*b + sq, 0.3333333333333333);
        x = biga + bigb;
        F = x;

	// Se N for tiny (ou zero), não há necessidade de ir além do cúbico
        if(N < pow(2, -127))
            goto here;

        IMAX = 10;
        for(i = 1; i <= IMAX; i++)
        {
            x2 = x*x;
            f = a0 + x*(a1 + x2*(a3 + x2*(a5 + x2*(a7 + x2*(a9 + x2*(a11 + x2))))));
            fp = b1 + x2*(b3 + x2*(b5 + x2*(b7 + x2*(b9 + x2*(b11 + 13*x2)))));
            dx = -f/fp;
            F = x + dx;
            // Se têm convergido até aqui não há nenhuma razão para continuar
            if(fabs(dx) <= pow(2, -127))
            {
                // Retorno normal aqui
                cout<<"Converge em iteração "<<i - 1<<endl;
                goto here;
            }
            x = F;
        }
        // Retorno anormal aqui. Fomos através do ciclo IMAX vezes sem convergência
        cout<<"Retornando sem convergência completa"<<endl;
        here:

        // Verifica se N era originalmente negativo
        if(iflag == 1)
        {
            F = -F;
            N = -N;
        }
    }
    else
    {
        if(N < 0)
        {
            tmp = -2*N/e + 1.8;
            x = -log(tmp);
        }
        else
        {
            tmp = 2*N/e + 1.8;
            x = log(tmp);
        }

        IMAX = 10;
        for(i = 1; i <= IMAX; i++)
        {
            shx = sinh(x);
            chx = sqrt(1 + pow(shx, 2));
            eshx = e*shx;
            echx = e*chx;
            f = eshx - x - N;
            fp = echx - 1;
            fpp = eshx;
            fppp = echx;
            dx = -f/fp;
            dx = -f/(fp + dx*fpp/2);
            dx = -f/(fp + dx*fpp/2 + pow(dx, 2)*fppp/6);
            F = x + dx;
            //Se têm convergido aqui não faz sentido continuar
            if(fabs(dx) <= pow(2, -127))
            {
                cout<<"Converge em iteração "<<i - 1<<endl;
                break;
            }
            x = F;
        }
        cout<<"Retornando sem convergência completa"<<endl;
    }
    return F;
}

/* Função que transforma orbel2xv */
void el2xv(double a, double e, double i, double w, double W, double M, double gM, double &x, double &y, double &z, double &vx, double &vy, double &vz)
{
    double cw, sw, cW, sW, ci, si, D11, D12, D13, D21, D22, D23;
    double E, cE, sE, Z, F, shF, chF, sqe, sqgMa, xfac1, xfac2, ri, vfac1, vfac2;

    // Calculando senos e cosenos de "w", "W" e "i" por funções
    sw = s(w);
    cw = c(w);
    sW = s(W);
    cW = c(W);
    si = s(i);
    ci = c(i);

    // Gerando matrizes de rotação
    D11 = cw*cW - sw*sW*ci;
    D12 = cw*sW + sw*cW*ci;
    D13 = sw*si;
    D21 = -sw*cW - cw*sW*ci;
    D22 = -sw*sW + cw*cW*ci;
    D23 = cw*si;

    // 1. Caso Elipse
    if((e >= 0) & (e < 1))
    {
        E = EqKepler1(e, M);
        sE = s(E);
        cE = c(E);
        sqe = sqrt(1 - pow(e, 2));
        sqgMa = sqrt(gM*a);
        xfac1 = a*(cE - e);
        xfac2 = a*sqe*sE;
        ri = pow(a*(1 - e*cE), -1);
        vfac1 = -ri*sqgMa*sE;
        vfac2 = ri*sqgMa*sqe*cE;
    }

    // 2. Caso Parábola
    if(e == 1)
    {
        Z = EqKepler2(M);     
        sqgMa = sqrt(2*gM*a);
        xfac1 = a*(1 - pow(Z, 2));
        xfac2 = 2*a*Z;      
        ri = pow(a*(1 + pow(Z, 2)), -1);
        vfac1 = -ri*sqgMa*Z;
        vfac2 = ri*sqgMa;     
    }

    // 3. Caso Hipérbole
    if(e > 1)
    {
        F = EqKepler3(e, M);
        shF = sinh(F);
        chF = sqrt(1 + pow(shF, 2));
        sqe = sqrt(pow(e, 2) - 1);
        sqgMa = sqrt(gM*a);
        xfac1 = a*(e - chF);
        xfac2 = a*sqe*shF;
        ri = pow(a*(e*chF - 1), -1);
        vfac1 = -ri*sqgMa*shF;
        vfac2 = ri*sqgMa*sqe*chF;
    }

    // Conversão
    x = D11*xfac1 + D21*xfac2;
    y = D12*xfac1 + D22*xfac2;
    z = D13*xfac1 + D23*xfac2;
    vx = D11*vfac1 + D21*vfac2;
    vy = D12*vfac1 + D22*vfac2;
    vz = D13*vfac1 + D23*vfac2;
}

int main()
{

    int ntp;
    double a_disk_ini, a_disk_final, gM;

    /* ********************************* Definir *********************************/
    /*
    ntp:	 	Número de tp's
    a_disk_ini: 	Semieixo maior interno do disco primordial em au
    a_disk_final:	Semieixo maior externo do disco primordial em au
    gM: 		g vezes a massa central, sendo g = 1 e M o valor da massa,
    			onde 1 massa solar = pow(0.01720209895, 2)
    */

    ntp = 500;
    a_disk_ini = 43; 
    a_disk_final = 50; 
    gM = pow(0.01720209895, 2);

    /* ***************************************************************************/

    const double pi = 3.141592653589793;
    int j;
    double a, e, i, w, W, M, passo, xhel, yhel, zhel, vxhel, vyhel, vzhel;

    a = a_disk_ini;
    passo = (a_disk_final - a_disk_ini)/(ntp - 1);

    ofstream arquivo1("tp.in");
    ofstream arquivo2("pra_init.txt");
    arquivo1.precision(15);
    arquivo1<<ntp<<endl;
//    arquivo1<<scientific<<ntp<<endl;
    for(j = 0; j < ntp; j++)
    {
            // Geramos excentridade e ângulos aleatórios, os últimos em graus
            e = (static_cast <float> (rand())/ static_cast <float> (RAND_MAX))*0.01; 
            i = (static_cast <float> (rand())/ static_cast <float> (RAND_MAX))*0.1; 
            w = (static_cast <float> (rand())/ static_cast <float> (RAND_MAX))*360; 
            W = (static_cast <float> (rand())/ static_cast <float> (RAND_MAX))*360; 
            M = (static_cast <float> (rand())/ static_cast <float> (RAND_MAX))*360;

            arquivo2<<left<<setw(3)<<j+1<<"  "<<right<<setw(7)<<a<<"  "<<setw(11)<<e<<"  "<<setw(11)<<i<<"  "<<setw(7)<<W<<"  "<<setw(7)<<w<<"  "<<setw(7)<<M<<endl;
            
            // Os ângulos devem ser convertidos em radianos para usar a função el2xv
            el2xv(a, e, i*pi/180, w*pi/180, W*pi/180, M*pi/180, gM, xhel, yhel, zhel, vxhel, vyhel, vzhel);

            arquivo1<<left<<setw(25)<<xhel<<" "<<right<<setw(25)<<yhel<<" "<<setw(25)<<zhel<<endl<<left<<setw(25)<<vxhel<<" "<<right<<setw(25)<<vyhel<<" "<<setw(25)<<vzhel<<endl<<0<<" "<<0<<" "<<0<<" "<<0<<" "<<0<<" "<<0<<" "<<0<<" "<<0<<" "<<0<<" "<<0<<" "<<0<<" "<<0<<" "<<0<<endl<<"0.0d0"<<" "<<"0.0d0"<<" "<<"0.0d0"<<" "<<"0.0d0"<<" "<<"0.0d0"<<endl<<"0.0d0"<<" "<<"0.0d0"<<" "<<"0.0d0"<<" "<<"0.0d0"<<" "<<"0.0d0"<<endl<<"0.0d0"<<" "<<"0.0d0"<<" "<<"0.0d0"<<endl;
            
             a = a + passo;
    }
    arquivo1.close();
    arquivo2.close();
    return 0;
}
