
#include "gtest/gtest.h"
#include "example.h"
#include "filters.h"
#include <Eigen/Core>
#include "haltijamath.h"
#include <complex>
typedef std::complex<float> Complex_t;

using namespace Eigen;

class TestFilter : public ::testing::Test {
protected:
    
    
    virtual void SetUp() {
    }
    
    virtual void TearDown() {
    }
    
};

class DISABLED_TestFilter: public TestFilter {};


TEST_F(TestFilter, TestVecFIRFilter) {
    MatrixXf B(3,1);
    B << 1.0/3.0,1.0/3.0,1.0/3.0;
    
    MatrixXf x(4,1);
    x << 1.0,2.0,3.0,4.0;
    
    MatrixXf y = fir_filter_columns(B,x);
    
    ASSERT_EQ(y.size(),2);
    ASSERT_NEAR(y(0),2.0,1e-5);
    ASSERT_NEAR(y(1),3.0,1e-5);
}


TEST_F(TestFilter, TestMatFIRFilter) {
    MatrixXf B(3,1);
    B << 1.0/3.0,1.0/3.0,1.0/3.0;
    
    MatrixXf x(4,2);
    x << 1.0,2.0,
         3.0,4.0,
         5.0,6.0,
         7.0,8.0;

    
    
    MatrixXf y = fir_filter_columns(B,x);

    ASSERT_EQ(y.rows(),2);
    ASSERT_EQ(y.cols(),2);

    ASSERT_NEAR(y(0,0),3.0,1e-5);
    ASSERT_NEAR(y(1,0),5.0,1e-5);
    ASSERT_NEAR(y(0,1),4.0,1e-5);
    ASSERT_NEAR(y(1,1),6.0,1e-5);
    
}

TEST_F(TestFilter, TestRealFFT) {
    
    Eigen::MatrixXf vec(16,1);
    vec << 1.8799893751161736,
    -0.7249041470960619,
    -1.113410742978105,
    0.7171773660112077,
    -1.1968869931336024,
    -0.5244096229968411,
    -1.4280970832235205,
    -0.9242202325503774,
    -0.657309715091765,
    1.3463116787007392,
    -0.06492054877037527,
    0.4858353357031988,
    0.6493125543234539,
    1.4451507619729766,
    0.45292788012516977,
    -1.9490480921076898;
    
    Eigen::MatrixXcf ref(16,1);
    
    
    ref << Complex_t(-1.6065022259954187),
     Complex_t(1.107867568960744,5.9240192803675225),
     Complex_t(-1.3237726805448715,-2.4675430435105135),
     Complex_t(-1.2695652094186247,0.5268295449526644),
     Complex_t(2.828605716061092,-3.2124042935244734),
     Complex_t(5.166781428168202,0.07626857287292932),
     Complex_t(4.86428087821398,-2.87386722081078),
     Complex_t(5.144112573121431,-1.9113398815404383),
     Complex_t(-1.3502883212697232,0),
     Complex_t(5.1441125731214346,1.9113398815404339),
     Complex_t(4.864280878213986,2.873867220810773),
     Complex_t(5.166781428168206,-0.07626857287293143),
     Complex_t(2.828605716061092,3.2124042935244734),
     Complex_t(-1.269565209418627,-0.5268295449526597),
     Complex_t(-1.3237726805448657,2.4675430435105206),
     Complex_t(1.107867568960741,-5.924019280367519);
    
    Eigen::MatrixXcf output;
    
    ASSERT_TRUE(HaltijaMath::fft(16,vec,output));
  

    for (int i = 0; i < 16; i++) {
        //std::cout << ref(i,0) << "--" << output(i,0) << std::endl;
        ASSERT_NEAR(ref(i,0).real(),output(i,0).real(),1e-4);
        ASSERT_NEAR(ref(i,0).imag(),output(i,0).imag(),1e-4);
    }
    
}

TEST_F(TestFilter, TestComplexFft) {
    
    Eigen::MatrixXcf vec(16,1);
    vec << Complex_t(-1.7157173775697618,-0.17801589168865242),
            Complex_t(-0.977967612726159,+0.303081509404676),
            Complex_t(-0.35976238756060724,+1.512041481145665),
            Complex_t(0.46394994259434225,-0.034976825651340186),
            Complex_t(-0.4847892326115197,+0.6647209536796226),
            Complex_t(-0.8402156795863868,-1.0941794885317964),
            Complex_t(-0.6239308559846515,+2.6498261024097896),
            Complex_t(0.8907983882588422,+1.963130545150921),
            Complex_t(-0.9993345105198678,-2.9415581631431738),
            Complex_t(0.33003537955413736,+0.5322597152630134),
            Complex_t(0.012348092790486213,+0.29862711683210885),
            Complex_t(0.30381248146123585,+1.3321043625129434),
            Complex_t(-0.8651783885314288,-0.3704251168686251),
            Complex_t(-1.608631062130525,-0.612246031611296),
            Complex_t(-1.01182211978771,-0.6412261021901126),
        Complex_t(-0.9652637053250518,+1.3160899019160552);
    
    Eigen::MatrixXcf ref(16,1);
    
    
    ref << Complex_t(-8.451668647674627,+4.6992540686297986),
            Complex_t(-1.737850936276574,-1.3129735269476512),
           Complex_t (-0.4893725912077347,-3.372340541826879),
           Complex_t (2.415794364470598,+4.933341172620159),
           Complex_t (-7.529284518094078,-2.8544707343399778),
           Complex_t (-0.5864716762167435,+3.2299693878210736),
           Complex_t (-1.4492976152813037,-7.193366858218716),
           Complex_t (-3.8664350800647465,+1.7666262481155088),
           Complex_t (-3.6447049118754955,-2.7112733082765548),
           Complex_t (7.670850282276886,+3.118738748601735),
           Complex_t (-2.6366587471694363,-6.032076603463248),
           Complex_t (1.5262302473567444,+4.270427895578548),
           Complex_t (3.365580040713887,-10.434622898096581),
           Complex_t (-4.071474855790154,+4.4968778526632915),
           Complex_t (-0.8850081141282501,+2.9423044369375484),
            Complex_t (-7.081705282155163,+1.6053303931835063);
    
    Eigen::MatrixXcf output;
    
    ASSERT_TRUE(HaltijaMath::fft(16,vec,output));
    
    
    for (int i = 0; i < 16; i++) {
        //std::cout << ref(i,0) << "--" << output(i,0) << std::endl;
        ASSERT_NEAR(ref(i,0).real(),output(i,0).real(),1e-4);
        ASSERT_NEAR(ref(i,0).imag(),output(i,0).imag(),1e-4);
    }
    
    Eigen::MatrixXf output2;
    ASSERT_TRUE(HaltijaMath::psd(16,vec,output2));

    Eigen::MatrixXf ref2(16,1);
    ref2 << 9.30213715,   5.21253418,   7.84115134,  13.3358438 ,
    14.67492333,  13.81870548,  13.79424409,  10.32192659,
    10.79417486,  16.23163544,  16.00885317,  16.43906026,
    17.71079887,  14.15725268,   8.15469562,   5.61862715;
    
    for (int i = 0; i < 16; i++) {
        //std::cout << ref(i,0) << "--" << output(i,0) << std::endl;
        ASSERT_NEAR(ref2(i,0),output2(i,0),1e-4);
    }
    
    
}

TEST_F(TestFilter,TestCiruclarShift) {
    MatrixXf x(5,1);
    x << 1,2,3,4,5;
    
    MatrixXf result1 = circular_shift_columns(x,1);
    
    ASSERT_TRUE(result1(0,0) == 5);
    ASSERT_TRUE(result1(1,0) == 1);
    ASSERT_TRUE(result1(2,0) == 2);
    ASSERT_TRUE(result1(3,0) == 3);
    ASSERT_TRUE(result1(4,0) == 4);

    MatrixXf result2 = circular_shift_columns(x,-1);

    
    ASSERT_TRUE(result2(0,0) == 2);
    ASSERT_TRUE(result2(1,0) == 3);
    ASSERT_TRUE(result2(2,0) == 4);
    ASSERT_TRUE(result2(3,0) == 5);
    ASSERT_TRUE(result2(4,0) == 1);

}

TEST_F(TestFilter,TestIIRImpulseAndStep) {

    const float yimpulse[] = {
        0.00952576237619500,
        0.0352982092110525,
        0.0626448161521111,
        0.0805943039397268,
        0.0908716450286346,
        0.0950519563315830,
        0.0945388971513134,
        0.0905551377482544,
        0.0841421674758108,
        0.0761670556442436,
        0.0673341252077569,
        0.0581998350950060,
        0.0491894816017802,
        0.0406146161647289,
        0.0326903322052783,
        0.0255517959254450,
        0.0192695849908189,
        0.0138635562420496,
        0.00931509110864064,
        0.00557766800502834,
        0.00258578773781457};
    
    const float ystep[] = {
        0.00952576237619500,
        0.0448239715872475,
        0.107468787739359,
        0.188063091679085,
        0.278934736707720,
        0.373986693039303,
        0.468525590190617,
        0.559080727938871,
        0.643222895414682,
        0.719389951058925,
        0.786724076266682,
        0.844923911361688,
        0.894113392963469,
        0.934728009128197,
        0.967418341333475,
        0.992970137258920,
        1.01223972224974,
        1.02610327849179,
        1.03541836960043,
        1.04099603760546};
    
    MatrixXf B(3,1);
    MatrixXf A(3,1);
    
    A << 1.000000000000000, -1.705552145544084, 0.743655195048866;
    B << 0.009525762376195,  0.019051524752390, 0.009525762376195;

    IIRFilter<MatrixXf, MatrixXf> filter(B,A,1);
    
    MatrixXf x(1,1);
    for (int i = 0; i < 20; i++) {
        x(0,0) = 0.0f;
        if (i == 0) {
            x(0,0) = 1.0f;
        }
        
        MatrixXf y = filter.filter(x);
        ASSERT_NEAR(y(0,0),yimpulse[i],1e-4);
        //std::cout << y << std::endl;
        
    }
    
    IIRFilter<MatrixXf, MatrixXf> filter2(B,A,1);
    
    for (int i = 0; i < 20; i++) {
        x(0,0) = 1.0f;
        
        MatrixXf y = filter2.filter(x);
        ASSERT_NEAR(y(0,0),ystep[i],1e-4);
        //std::cout << y << std::endl;
        
    }

}

TEST_F(TestFilter,TestIIRFilter) {

    //B = [ 0.85284624 -1.70569249  0.85284624];
    //A = [ 1.         -1.68391975  0.72746523]
    
    MatrixXf B(3,1);
    MatrixXf A(3,1);
    
    B << 0.85284624, -1.70569249,  0.85284624;
    A << 1.,         -1.68391975,  0.72746523;

    IIRFilter<MatrixXf, MatrixXf> filter(B,A,1);
    
    
    const float yref[20] = {0.85284624,  0.58327839,  0.36177801,  0.18489039,  0.04815965,
        -0.05340434, -0.1249631 , -0.17157802, -0.19801732, -0.20862822,
        -0.20726247, -0.19724359, -0.18136613, -0.16191816, -0.14071964,
        -0.11917074, -0.09830532, -0.0788457 , -0.06125613, -0.0457929};
    
    MatrixXf x(1,1);
    for (int i = 0; i < 20; i++) {
        x(0,0) = 1.0f;
        MatrixXf y = filter.filter(x);
        ASSERT_NEAR(y(0,0),yref[i],1e-4);
        //std::cout << y << std::endl;

    }
    
    const float yref2[20] = {0.        ,  0.85284624,  1.43612463,  1.79790264,  1.98279303,
        2.03095267,  1.97754833,  1.85258524,  1.68100721,  1.4829899 ,
        1.27436167,  1.0670992 ,  0.86985561,  0.68848948,  0.52657132,
        0.38585168,  0.26668094,  0.16837562,  0.08952992,  0.0282738};
    
    
    IIRFilter<MatrixXf, MatrixXf> filter2(B,A,1);

    for (int i = 0; i < 20; i++) {
        x(0,0) = i;
        MatrixXf y = filter2.filter(x);
        ASSERT_NEAR(y(0,0),yref2[i],1e-4);
        //std::cout << y << std::endl;
        
    }

    
    IIRFilter<MatrixXf, MatrixXf> filter3(B,A,2);
    MatrixXf x2(1,2);
    for (int i = 0; i < 20; i++) {
        x2(0,0) = 1;
        x2(0,1) = i;
        MatrixXf y = filter3.filter(x2);
        ASSERT_NEAR(y(0,0),yref[i],1e-4);
        ASSERT_NEAR(y(0,1),yref2[i],1e-4);

        //std::cout << y << std::endl;
        
    }

    
}
