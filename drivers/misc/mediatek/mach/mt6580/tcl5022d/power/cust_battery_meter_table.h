#ifndef _CUST_BATTERY_METER_TABLE_H
#define _CUST_BATTERY_METER_TABLE_H

#include <mach/mt_typedefs.h>

// ============================================================
// define
// ============================================================
#define BAT_NTC_10 1
#define BAT_NTC_47 0

#if (BAT_NTC_10 == 1)
#define RBAT_PULL_UP_R             16900	
#define RBAT_PULL_DOWN_R		   27000	
#endif

#if (BAT_NTC_47 == 1)
#define RBAT_PULL_UP_R             61900	
#define RBAT_PULL_DOWN_R		  100000	
#endif
#define RBAT_PULL_UP_VOLT          1800



// ============================================================
// ENUM
// ============================================================

// ============================================================
// structure
// ============================================================

// ============================================================
// typedef
// ============================================================
typedef struct _BATTERY_PROFILE_STRUC
{
    kal_int32 percentage;
    kal_int32 voltage;
} BATTERY_PROFILE_STRUC, *BATTERY_PROFILE_STRUC_P;

typedef struct _R_PROFILE_STRUC
{
    kal_int32 resistance; // Ohm
    kal_int32 voltage;
} R_PROFILE_STRUC, *R_PROFILE_STRUC_P;

typedef enum
{
    T1_0C,
    T2_25C,
    T3_50C
} PROFILE_TEMPERATURE;

// ============================================================
// External Variables
// ============================================================

// ============================================================
// External function
// ============================================================

// ============================================================
// <DOD, Battery_Voltage> Table
// ============================================================
#if (BAT_NTC_10 == 1)
    BATT_TEMPERATURE Batt_Temperature_Table[] = {
        {-20,68237},
        {-15,53650},
        {-10,42506},
        { -5,33892},
        {  0,27219},
        {  5,22021},
        { 10,17926},
        { 15,14674},
        { 20,12081},
        { 25,10000},
        { 30,8315},
        { 35,6948},
        { 40,5834},
        { 45,4917},
        { 50,4161},
        { 55,3535},
        { 60,3014}
    };
#endif

#if (BAT_NTC_47 == 1)
    BATT_TEMPERATURE Batt_Temperature_Table[] = {
        {-20,483954},
        {-15,360850},
        {-10,271697},
        { -5,206463},
        {  0,158214},
        {  5,122259},
        { 10,95227},
        { 15,74730},
        { 20,59065},
        { 25,47000},
        { 30,37643},
        { 35,30334},
        { 40,24591},
        { 45,20048},
        { 50,16433},
        { 55,13539},
        { 60,11210}        
    };
#endif

// T0 -10C
BATTERY_PROFILE_STRUC battery_profile_t0[] =
{
	{0    , 4299},
	{2    , 4273},
	{3    , 4250},
	{5    , 4223},
	{7    , 4193},
	{9    , 4151},
	{10   , 4122},
	{12   , 4104},
	{14   , 4090},
	{15   , 4074},
	{17   , 4055},
	{19   , 4034},
	{20   , 4014},
	{22   , 3996},
	{24   , 3980},
	{26   , 3967},
	{27   , 3956},
	{29   , 3944},
	{31   , 3931},
	{32   , 3918},
	{34   , 3903},
	{36   , 3888},
	{37   , 3875},
	{39   , 3862},
	{41   , 3851},
	{43   , 3840},
	{44   , 3829},
	{46   , 3821},
	{48   , 3814},
	{49   , 3808},
	{51   , 3803},
	{53   , 3799},
	{54   , 3798},
	{56   , 3796},
	{58   , 3793},
	{60   , 3791},
	{61   , 3789},
	{63   , 3787},
	{65   , 3784},
	{66   , 3782},
	{68   , 3778},
	{70   , 3775},
	{71   , 3771},
	{73   , 3765},
	{75   , 3759},
	{77   , 3753},
	{78   , 3746},
	{80   , 3737},
	{82   , 3729},
	{83   , 3719},
	{85   , 3710},
	{87   , 3702},
	{88   , 3696},
	{90   , 3692},
	{92   , 3686},
	{94   , 3680},
	{95   , 3665},
	{97   , 3629},
	{99   , 3566},
	{100  , 3454},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400}
};      
        
// T1 0C 
BATTERY_PROFILE_STRUC battery_profile_t1[] =
{  
	{0    , 4312},
	{2    , 4256},
	{3    , 4217},
	{5    , 4195},
	{6    , 4174},
	{8    , 4157},
	{10   , 4140},
	{11   , 4122},
	{13   , 4106},
	{14   , 4091},
	{16   , 4079},
	{18   , 4064},
	{19   , 4047},
	{21   , 4030},
	{22   , 4012},
	{24   , 3994},
	{26   , 3979},
	{27   , 3967},
	{29   , 3957},
	{30   , 3945},
	{32   , 3935},
	{34   , 3921},
	{35   , 3908},
	{37   , 3894},
	{38   , 3878},
	{40   , 3865},
	{42   , 3852},
	{43   , 3840},
	{45   , 3829},
	{46   , 3820},
	{48   , 3814},
	{50   , 3806},
	{51   , 3800},
	{53   , 3794},
	{55   , 3789},
	{56   , 3786},
	{58   , 3784},
	{59   , 3782},
	{61   , 3781},
	{63   , 3779},
	{64   , 3778},
	{66   , 3776},
	{67   , 3774},
	{69   , 3771},
	{71   , 3768},
	{72   , 3764},
	{74   , 3761},
	{75   , 3758},
	{77   , 3752},
	{79   , 3746},
	{80   , 3738},
	{82   , 3729},
	{83   , 3719},
	{85   , 3706},
	{87   , 3694},
	{88   , 3686},
	{90   , 3681},
	{91   , 3677},
	{93   , 3673},
	{95   , 3669},
	{96   , 3657},
	{98   , 3613},
	{99   , 3523},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400},
	{100  , 3400} 
};           

// T2 25C
BATTERY_PROFILE_STRUC battery_profile_t2[] =
{
	{0    ,  4333},
	{1    ,  4312},
	{3    ,  4293},
	{4    ,  4275},
	{6    ,  4257},
	{7    ,  4238},
	{9    ,  4223},
	{10   ,  4205},
	{12   ,  4190},
	{13   ,  4171},
	{15   ,  4156},
	{16   ,  4141},
	{18   ,  4125},
	{19   ,  4110},
	{21   ,  4095},
	{22   ,  4082},
	{24   ,  4067},
	{25   ,  4052},
	{27   ,  4037},
	{28   ,  4022},
	{30   ,  4010},
	{31   ,  3997},
	{33   ,  3985},
	{34   ,  3976},
	{36   ,  3964},
	{37   ,  3955},
	{39   ,  3943},
	{40   ,  3933},
	{42   ,  3921},
	{43   ,  3904},
	{45   ,  3887},
	{46   ,  3872},
	{48   ,  3860},
	{49   ,  3850},
	{51   ,  3842},
	{52   ,  3836},
	{54   ,  3829},
	{55   ,  3823},
	{57   ,  3817},
	{58   ,  3811},
	{60   ,  3806},
	{61   ,  3802},
	{63   ,  3798},
	{64   ,  3793},
	{66   ,  3790},
	{67   ,  3787},
	{69   ,  3784},
	{70   ,  3781},
	{72   ,  3781},
	{73   ,  3778},
	{75   ,  3775},
	{76   ,  3771},
	{77   ,  3765},
	{79   ,  3759},
	{80   ,  3753},
	{82   ,  3744},
	{83   ,  3735},
	{85   ,  3723},
	{86   ,  3710},
	{88   ,  3695},
	{89   ,  3690},
	{91   ,  3687},
	{92   ,  3685},
	{94   ,  3680},
	{95   ,  3672},
	{97   ,  3628},
	{98   ,  3543},
	{100  ,  3400},
	{100  ,  3400},
	{100  ,  3400},
	{100  ,  3400},
	{100  ,  3400},
	{100  ,  3400},
	{100  ,  3400},
	{100  ,  3400},
	{100  ,  3400},
	{100  ,  3400},
	{100  ,  3400},
	{100  ,  3400},
	{100  ,  3400} 
};     

// T3 50C
BATTERY_PROFILE_STRUC battery_profile_t3[] =
{
	{0    ,  4340},
	{2    ,  4321},
	{3    ,  4302},
	{5    ,  4285},
	{6    ,  4269},
	{8    ,  4251},
	{9    ,  4233},
	{11   ,  4216},
	{12   ,  4199},
	{14   ,  4182},
	{15   ,  4166},
	{17   ,  4150},
	{18   ,  4135},
	{20   ,  4118},
	{21   ,  4102},
	{23   ,  4086},
	{24   ,  4072},
	{26   ,  4056},
	{27   ,  4043},
	{29   ,  4028},
	{30   ,  4016},
	{32   ,  4002},
	{33   ,  3989},
	{35   ,  3977},
	{36   ,  3965},
	{38   ,  3954},
	{39   ,  3942},
	{41   ,  3931},
	{42   ,  3919},
	{44   ,  3905},
	{45   ,  3887},
	{47   ,  3871},
	{48   ,  3859},
	{50   ,  3849},
	{51   ,  3841},
	{53   ,  3833},
	{54   ,  3826},
	{56   ,  3819},
	{57   ,  3813},
	{59   ,  3806},
	{60   ,  3801},
	{62   ,  3796},
	{63   ,  3791},
	{65   ,  3787},
	{66   ,  3782},
	{68   ,  3778},
	{70   ,  3774},
	{71   ,  3768},
	{73   ,  3759},
	{74   ,  3750},
	{76   ,  3746},
	{77   ,  3741},
	{79   ,  3735},
	{80   ,  3727},
	{82   ,  3722},
	{83   ,  3713},
	{85   ,  3701}, 
	{86   ,  3690},
	{88   ,  3674},
	{89   ,  3670},
	{91   ,  3667},
	{92   ,  3665},
	{94   ,  3662},
	{95   ,  3656},
	{97   ,  3621},
	{98   ,  3544},
	{100  ,  3420},
	{100  ,  3400},
	{100  ,  3400},
	{100  ,  3400},
	{100  ,  3400},
	{100  ,  3400},
	{100  ,  3400},
	{100  ,  3400},
	{100  ,  3400},
	{100  ,  3400},
	{100  ,  3400},
	{100  ,  3400},
	{100  ,  3400},
	{100  ,  3400} 
};           

// battery profile for actual temperature. The size should be the same as T1, T2 and T3
BATTERY_PROFILE_STRUC battery_profile_temperature[] =
{
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0}
}; 
 
// ============================================================
// <Rbat, Battery_Voltage> Table
// ============================================================
// T0 -10C
R_PROFILE_STRUC r_profile_t0[] =
{
	{803   , 4299},
	{803   , 4273},
	{830   , 4250},
	{840   , 4223},
	{855   , 4193},
	{873   , 4151},
	{933   , 4122},
	{958   , 4104},
	{988   , 4090},
	{1003  , 4074},
	{1008  , 4055},
	{1000  , 4034},
	{995   , 4014},
	{990   , 3996},
	{993   , 3980},
	{1000  , 3967},
	{1010  , 3956},
	{1018  , 3944},
	{1023  , 3931},
	{1025  , 3918},
	{1020  , 3903},
	{1015  , 3888},
	{1015  , 3875},
	{1018  , 3862},
	{1020  , 3851},
	{1023  , 3840},
	{1023  , 3829},
	{1033  , 3821},
	{1043  , 3814},
	{1055  , 3808},
	{1070  , 3803},
	{1090  , 3799},
	{1120  , 3798},
	{1153  , 3796},
	{1183  , 3793},
	{1218  , 3791},
	{1260  , 3789},
	{1305  , 3787},
	{1348  , 3784},
	{1398  , 3782},
	{1438  , 3778},
	{1473  , 3775},
	{1510  , 3771},
	{1538  , 3765},
	{1558  , 3759},
	{1583  , 3753},
	{1608  , 3746},
	{1625  , 3737},
	{1648  , 3729},
	{1670  , 3719},
	{1685  , 3710},
	{1710  , 3702},
	{1743  , 3696},
	{1788  , 3692},
	{1848  , 3686},
	{1930  , 3680},
	{2030  , 3665},
	{2143  , 3629},
	{2310  , 3566},
	{2645  , 3454},
	{3273  , 3400},
	{3123  , 3400},
	{3048  , 3400},
	{2940  , 3400},
	{2650  , 3400},
	{2690  , 3400},
	{2455  , 3400},
	{2605  , 3400},
	{2503  , 3400},
	{2235  , 3400},
	{2308  , 3400},
	{2360  , 3400},
	{2425  , 3400},
	{1940  , 3400},
	{1943  , 3400},
	{1963  , 3400},
	{1998  , 3400},
	{1985  , 3400},
	{2003  , 3400},
	{2005  , 3400}
};      

// T1 0C
R_PROFILE_STRUC r_profile_t1[] =
{
	{418   , 4312},
	{417   , 4256},
	{483   , 4217},
	{503   , 4195},
	{505   , 4174},
	{513   , 4157},
	{520   , 4140},
	{520   , 4122},
	{525   , 4106},
	{540   , 4091},
	{565   , 4079},
	{583   , 4064},
	{585   , 4047},
	{583   , 4030},
	{572   , 4012},
	{568   , 3994},
	{563   , 3979},
	{565   , 3967},
	{573   , 3957},
	{580   , 3945},
	{585   , 3935},
	{580   , 3921},
	{578   , 3908},
	{573   , 3894},
	{565   , 3878},
	{558   , 3865},
	{555   , 3852},
	{553   , 3840},
	{550   , 3829},
	{555   , 3820},
	{565   , 3814},
	{568   , 3806},
	{575   , 3800},
	{583   , 3794},
	{588   , 3789},
	{605   , 3786},
	{628   , 3784},
	{653   , 3782},
	{680   , 3781},
	{708   , 3779},
	{748   , 3778},
	{790   , 3776},
	{838   , 3774},
	{888   , 3771},
	{940   , 3768},
	{988   , 3764},
	{1043  , 3761},
	{1090  , 3758},
	{1135  , 3752},
	{1173  , 3746},
	{1208  , 3738},
	{1243  , 3729},
	{1270  , 3719},
	{1298  , 3706},
	{1318  , 3694},
	{1343  , 3686},
	{1375  , 3681},
	{1423  , 3677},
	{1478  , 3673},
	{1563  , 3669},
	{1670  , 3657},
	{1785  , 3613},
	{1983  , 3523},
	{2530  , 3400},
	{3098  , 3400},
	{2940  , 3400},
	{2805  , 3400},
	{2620  , 3400},
	{2423  , 3400},
	{2470  , 3400},
	{2153  , 3400},
	{2258  , 3400},
	{2360  , 3400},
	{2113  , 3400},
	{2170  , 3400},
	{2215  , 3400},
	{2270  , 3400},
	{1800  , 3400},
	{1788  , 3400},
	{1805  , 3400} 
};     

// T2 25C
R_PROFILE_STRUC r_profile_t2[] =
{
	{138   ,  4333},
	{138   ,  4312},
	{145   ,  4293},
	{145   ,  4275},
	{145   ,  4257},
	{145   ,  4238},
	{150   ,  4223},
	{145   ,  4205},
	{150   ,  4190},
	{145   ,  4171},
	{153   ,  4156},
	{153   ,  4141},
	{155   ,  4125},
	{160   ,  4110},
	{160   ,  4095},
	{162   ,  4082},
	{168   ,  4067},
	{175   ,  4052},
	{178   ,  4037},
	{175   ,  4022},
	{180   ,  4010},
	{183   ,  3997},
	{183   ,  3985},
	{190   ,  3976},
	{193   ,  3964},
	{200   ,  3955},
	{200   ,  3943},
	{205   ,  3933},
	{198   ,  3921},
	{188   ,  3904},
	{168   ,  3887},
	{153   ,  3872},
	{145   ,  3860},
	{143   ,  3850},
	{138   ,  3842},
	{143   ,  3836},
	{145   ,  3829},
	{145   ,  3823},
	{145   ,  3817},
	{145   ,  3811},
	{145   ,  3806},
	{145   ,  3802},
	{150   ,  3798},
	{145   ,  3793},
	{145   ,  3790},
	{145   ,  3787},
	{145   ,  3784},
	{145   ,  3781},
	{160   ,  3781},
	{163   ,  3778},
	{170   ,  3775},
	{175   ,  3771},
	{183   ,  3765},
	{198   ,  3759},
	{213   ,  3753},
	{220   ,  3744},
	{240   ,  3735},
	{253   ,  3723},
	{273   ,  3710},
	{283   ,  3695},
	{300   ,  3690},
	{323   ,  3687},
	{355   ,  3685},
	{398   ,  3680},
	{450   ,  3672},
	{480   ,  3628},
	{558   ,  3543},
	{700   ,  3400},
	{2303  ,  3400},
	{2563  ,  3400},
	{2390  ,  3400},
	{2248  ,  3400},
	{2123  ,  3400},
	{2013  ,  3400},
	{1955  ,  3400},
	{1998  ,  3400},
	{1893  ,  3400},
	{1905  ,  3400},
	{1945  ,  3400},
	{1753  ,  3400} 
}; 

// T3 50C
R_PROFILE_STRUC r_profile_t3[] =
{
	{70    ,  4340},
	{70    ,  4321},
	{68    ,  4302},
	{70    ,  4285},
	{73    ,  4269},
	{73    ,  4251},
	{70    ,  4233},
	{73    ,  4216},
	{73    ,  4199},
	{73    ,  4182},
	{70    ,  4166},
	{78    ,  4150},
	{78    ,  4135},
	{78    ,  4118},
	{78    ,  4102},
	{78    ,  4086},
	{80    ,  4072},
	{77    ,  4056},
	{83    ,  4043},
	{82    ,  4028},
	{88    ,  4016},
	{88    ,  4002},
	{88    ,  3989},
	{95    ,  3977},
	{95    ,  3965},
	{100   ,  3954},
	{103   ,  3942},
	{110   ,  3931},
	{110   ,  3919},
	{110   ,  3905},
	{93    ,  3887},
	{80    ,  3871},
	{75    ,  3859},
	{73    ,  3849},
	{75    ,  3841},
	{73    ,  3833},
	{75    ,  3826},
	{73    ,  3819},
	{75    ,  3813},
	{73    ,  3806},
	{78    ,  3801},
	{80    ,  3796},
	{83    ,  3791},
	{85    ,  3787},
	{85    ,  3782},
	{85    ,  3778},
	{88    ,  3774},
	{83    ,  3768},
	{75    ,  3759},
	{70    ,  3750},
	{73    ,  3746},
	{78    ,  3741},
	{75    ,  3735},
	{73    ,  3727},
	{78    ,  3722},
	{75    ,  3713},
	{73    ,  3701},
	{78    ,  3690},
	{78    ,  3674},
	{80    ,  3670},
	{83    ,  3667},
	{85    ,  3665},
	{90    ,  3662},
	{95    ,  3656},
	{103   ,  3621},
	{110   ,  3544},
	{123   ,  3420},
	{290   ,  3400},
	{2023  ,  3400},
	{1880  ,  3400},
	{1753  ,  3400},
	{1643  ,  3400},
	{1685  ,  3400},
	{1595  ,  3400},
	{1620  ,  3400},
	{1458  ,  3400},
	{1458  ,  3400},
	{1468  ,  3400},
	{1478  ,  3400},
	{1495  ,  3400} 
}; 

// r-table profile for actual temperature. The size should be the same as T1, T2 and T3
R_PROFILE_STRUC r_profile_temperature[] =
{
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0},
	{0  , 0}
};

// ============================================================
// function prototype
// ============================================================
int fgauge_get_saddles(void);
BATTERY_PROFILE_STRUC_P fgauge_get_profile(kal_uint32 temperature);

int fgauge_get_saddles_r_table(void);
R_PROFILE_STRUC_P fgauge_get_profile_r_table(kal_uint32 temperature);

#endif	//#ifndef _CUST_BATTERY_METER_TABLE_H

