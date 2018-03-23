#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "string.h"

#include "structure.h"
#include "functions.h"

#define TAP_PERC 0.05

int size_float = sizeof(float);
int size_int = sizeof(int);

void borch_ampf(float *,float *,int,float *,float *,float *,float *,float *,float *,float *,float *,float *);
void cb2008_ampf(float *,float *,int,float *,float *,float *,float *,float *,float *,float *,float *,float *,float *,float *);
void cb2014_ampf(float *,float *,int,float *,float *,float *,float *,float *,float *,float *,float *,float *,float *,float *);
void bssa2014_ampf(float *,float *,int,float *,float *,float *,float *,float *,float *,float *,float *,float *,float *,float *);
void ampfac(struct complex *,float *,int);

int getnt_p2(int nt);
int getpeak(float* s,int nt,float* pga);
int taper_norm(float* g,float* dt,int nt,float* tap_per);
int zero(float* s,int n);
int norm(float* g,float* dt,int nt);


int wcc_siteamp14(float* seis, int nt, float dt, float pga, float vs30, float vref, char model[128])
{
struct statdata head1;
float *s1, vsite, vpga;
float *ampf;
int nt_p2;

float tap_per = TAP_PERC;
//float pga = -1.0;

float fmin = 0.1;
float fmax = 15.0;

float flowcap = 0.0;   /* ampf for f<flowcap set equal to ampf[f=flowcap], (caps low-freq amplification level) */

char infile[128];
char outfile[128];

int inbin = 0;
int outbin = 0;

float fmidbot = 0.2;     /* bottom-end of middle frequency range */
float fmid = 1.0;        /* center of middle frequency range */
float fhigh = 3.333;     /* center of high frequency range */
float fhightop = 10.0;   /* top-end of high frequency range */

/*
sprintf(model,"borcherdt");
*/
//sprintf(model,"cb2014");

/*
setpar(ac,av);

mstpar("infile","s",infile);
mstpar("outfile","s",outfile);
mstpar("vref","f",&vref);
mstpar("vsite","f",&vsite);

getpar("model","s",model);
getpar("pga","f",&pga);
vpga = vref;
getpar("vpga","f",&vpga);
getpar("flowcap","f",&flowcap);

getpar("tap_per","f",&tap_per);
getpar("fmin","f",&fmin);
getpar("fmidbot","f",&fmidbot);
getpar("fmid","f",&fmid);
getpar("fhigh","f",&fhigh);
getpar("fhightop","f",&fhightop);
getpar("fmax","f",&fmax);
getpar("inbin","d",&inbin);
getpar("outbin","d",&outbin);

endpar();
*/

//vref = 865;
vsite=vs30;
vpga = vref;

if(strncmp(model,"borcherdt",9) != 0 && strncmp(model,"cb2008",6) != 0 && strncmp(model,"bssa2014",6) != 0)
   sprintf(model,"cb2014");

//s1 = NULL;
//s1 = read_wccseis(infile,&head1,s1,inbin);
s1 = seis;
head1.nt = nt;
head1.dt = dt;

nt_p2 = getnt_p2(head1.nt);
//printf("nt_p2=%d\n", nt_p2);
//s1 = (float *) check_realloc (s1,nt_p2*size_float);

ampf = (float *) check_malloc ((nt_p2/2)*size_float);

if(pga < 0.0)
   getpeak(s1,head1.nt,&pga);
else
   fprintf(stderr,"*** External PGA used: ");

fprintf(stderr,"pga= %13.5e\n",pga);
taper_norm(s1,&head1.dt,head1.nt,&tap_per);
zero(s1+head1.nt,(nt_p2)-head1.nt);
forfft(s1,nt_p2,-1);

if(strncmp(model,"cb2014",6) == 0)
   cb2014_ampf(ampf,&head1.dt,nt_p2,&vref,&vsite,&vpga,&pga,&fmin,&fmidbot,&fmid,&fhigh,&fhightop,&fmax,&flowcap);
else if(strncmp(model,"bssa2014",8) == 0)
   bssa2014_ampf(ampf,&head1.dt,nt_p2,&vref,&vsite,&vpga,&pga,&fmin,&fmidbot,&fmid,&fhigh,&fhightop,&fmax,&flowcap);
else if(strncmp(model,"cb2008",6) == 0)
   cb2008_ampf(ampf,&head1.dt,nt_p2,&vref,&vsite,&vpga,&pga,&fmin,&fmidbot,&fmid,&fhigh,&fhightop,&fmax,&flowcap);
else
   borch_ampf(ampf,&head1.dt,nt_p2,&vref,&vsite,&pga,&fmin,&fmidbot,&fmid,&fhigh,&fhightop,&fmax);

ampfac((struct complex *)s1,ampf,nt_p2);

invfft(s1,nt_p2,1);
norm(s1,&head1.dt,nt_p2);

free(ampf);
//write_wccseis(outfile,&head1,s1,outbin);
}

void borch_ampf(float *ampf,float *dt,int n,float *vref,float *vsite,float *pga,float *fmin,float *fmidbot,float *fmid,float *fhigh,float *fhightop,float *fmax)
{
float ma, mv, fa, fv, fac, df, freq;
float vr;
int i;

vr = (*vref)/(*vsite);

if(*pga < 0.1)
   {
   ma = 0.35;
   mv = 0.65;
   }
else if(*pga < 0.2)
   {
   ma = 0.35 - (*pga - 0.1);
   mv = 0.65 - 0.5*(*pga - 0.1);
   }
else if(*pga < 0.3)
   {
   ma = 0.25 - 1.5*(*pga - 0.2);
   mv = 0.60 - 0.7*(*pga - 0.2);
   }
else if(*pga < 0.4)
   {
   ma = 0.10 - 1.5*(*pga - 0.3);
   mv = 0.53 - 0.8*(*pga - 0.3);
   }
else
   {
   ma = -0.05;
   mv = 0.45;
   }

fa = exp(ma*log(vr));
fv = exp(mv*log(vr));

if(*fmin > *fmidbot)
   *fmin = *fmidbot;
if(*fmax < *fhightop)
   *fmin = *fhightop;

df = 1.0/(n*(*dt));
for(i=1;i<n/2;i++)
   {
   freq = i*df;

   if(freq < *fmin)
      fac = 1.0;

   else if(freq < *fmidbot)
      fac = 1.0 + (freq - *fmin)*(fv - 1.0)/(*fmidbot - *fmin);

   else if(freq < *fmid)
      fac = fv;

   else if(freq < *fhigh)
      fac = fv + (freq - *fmid)*(fa - fv)/(*fhigh - *fmid);

   else if(freq < *fhightop)
      fac = fa;

   else if(freq < *fmax)
      fac = fa + (freq - *fhightop)*(1.0 - fa)/(*fmax - *fhightop);

   else
      fac = 1.0;

   ampf[i] = fac;
   }
}

void ampfac(struct complex *g,float *ampf,int n)
{
int i;

for(i=1;i<n/2;i++)
   {
   g[i].re = ampf[i]*g[i].re;
   g[i].im = ampf[i]*g[i].im;
   }
}

norm(g,dt,nt)
float *g, *dt;
int nt;
{
float fac;

fac = 1.0/((*dt)*nt);
while(nt--)
   {
   g[0] = g[0]*fac;
   g++;
   }
}

zero(s,n)
float *s;
int n;
{
while(n--)
   {
   s[0] = 0.0;
   s++;
   }
}

taper_norm(g,dt,nt,tap_per)
float *g, *dt, *tap_per;
int nt;
{
float fac, df, arg;
int i;
int ntap;

ntap = nt*(*tap_per);

for(i=0;i<nt-ntap;i++)
   g[i] = g[i]*(*dt);

if(ntap > 0)
   {
   df = 3.14159/(float)(ntap);
   for(i=nt-ntap;i<nt;i++)
      {
      arg = (i-(nt-(ntap+1)))*df;
      fac = (*dt)*0.5*(1.0 + cos(arg));
      g[i] = g[i]*fac;
      }
   }
}

getnt_p2(nt)
int nt;
{
int i = 0;

while(nt > 1)
   {
   if(nt%2)
      nt++;

   nt = nt/2;
   i++;
   }

nt = 1;
while(i--)
   nt = nt*2;

return(nt);
}

getpeak(s,nt,pga)
float *s, *pga;
int nt;
{
int i;

for(i=0;i<nt;i++)
   {
   if(s[i] > *pga)
      *pga = s[i];
   if(-s[i] > *pga)
      *pga = -s[i];
   }

*pga = *pga/981.0;
}

void cb2008_ampf(float *ampf,float *dt,int n,float *vref,float *vsite,float *vpga,float *pga,float *fmin,float *fmidbot,float *fmid,float *fhigh,float *fhightop,float *fmax,float *flowcap)
{
float scon_c, scon_n, per[22], c10[22], c11[22], k1[22], k2[22], k3[22];
float ampf0[22];
float a_1100, fs_1100, fs_vpga, fs_vref, fsite;
float df, ampv, afac, freq;
float f0, f1, a0, a1, dadf, ampf_cap;
int i, j;
int nper = 22;

scon_c = 1.88;
scon_n = 1.18;

per[ 0] = 0.0;
per[ 1] = 0.01;
per[ 2] = 0.02;
per[ 3] = 0.03;
per[ 4] = 0.05;
per[ 5] = 0.075;
per[ 6] = 0.10;
per[ 7] = 0.15;
per[ 8] = 0.20;
per[ 9] = 0.25;
per[10] = 0.30;
per[11] = 0.40;
per[12] = 0.50;
per[13] = 0.75;
per[14] = 1.00;
per[15] = 1.50;
per[16] = 2.00;
per[17] = 3.00;
per[18] = 4.00;
per[19] = 5.00;
per[20] = 7.50;
per[21] = 10.0;

c10[ 0] = 1.058;
c10[ 1] = 1.058;
c10[ 2] = 1.102;
c10[ 3] = 1.174;
c10[ 4] = 1.272;
c10[ 5] = 1.438;
c10[ 6] = 1.604;
c10[ 7] = 1.928;
c10[ 8] = 2.194;
c10[ 9] = 2.351;
c10[10] = 2.46;
c10[11] = 2.587;
c10[12] = 2.544;
c10[13] = 2.133;
c10[14] = 1.571;
c10[15] = 0.406;
c10[16] = -0.456;
c10[17] = -0.82;
c10[18] = -0.82;
c10[19] = -0.82;
c10[20] = -0.82;
c10[21] = -0.82;

c11[ 0] = 0.04;
c11[ 1] = 0.04;
c11[ 2] = 0.04;
c11[ 3] = 0.04;
c11[ 4] = 0.04;
c11[ 5] = 0.04;
c11[ 6] = 0.04;
c11[ 7] = 0.04;
c11[ 8] = 0.04;
c11[ 9] = 0.04;
c11[10] = 0.04;
c11[11] = 0.04;
c11[12] = 0.04;
c11[13] = 0.077;
c11[14] = 0.15;
c11[15] = 0.253;
c11[16] = 0.3;
c11[17] = 0.3;
c11[18] = 0.3;
c11[19] = 0.3;
c11[20] = 0.3;
c11[21] = 0.3;

k1[ 0] = 865.0;
k1[ 1] = 865.0;
k1[ 2] = 865.0;
k1[ 3] = 908.0;
k1[ 4] = 1054.0;
k1[ 5] = 1086.0;
k1[ 6] = 1032.0;
k1[ 7] = 878.0;
k1[ 8] = 748.0;
k1[ 9] = 654.0;
k1[10] = 587.0;
k1[11] = 503.0;
k1[12] = 457.0;
k1[13] = 410.0;
k1[14] = 400.0;
k1[15] = 400.0;
k1[16] = 400.0;
k1[17] = 400.0;
k1[18] = 400.0;
k1[19] = 400.0;
k1[20] = 400.0;
k1[21] = 400.0;

k2[ 0] = -1.186;
k2[ 1] = -1.186;
k2[ 2] = -1.219;
k2[ 3] = -1.273;
k2[ 4] = -1.346;
k2[ 5] = -1.471;
k2[ 6] = -1.624;
k2[ 7] = -1.931;
k2[ 8] = -2.188;
k2[ 9] = -2.381;
k2[10] = -2.518;
k2[11] = -2.657;
k2[12] = -2.669;
k2[13] = -2.401;
k2[14] = -1.955;
k2[15] = -1.025;
k2[16] = -0.299;
k2[17] = 0.0;
k2[18] = 0.0;
k2[19] = 0.0;
k2[20] = 0.0;
k2[21] = 0.0;

k3[ 0] = 1.839;
k3[ 1] = 1.839;
k3[ 2] = 1.84;
k3[ 3] = 1.841;
k3[ 4] = 1.843;
k3[ 5] = 1.845;
k3[ 6] = 1.847;
k3[ 7] = 1.852;
k3[ 8] = 1.856;
k3[ 9] = 1.861;
k3[10] = 1.865;
k3[11] = 1.874;
k3[12] = 1.883;
k3[13] = 1.906;
k3[14] = 1.929;
k3[15] = 1.974;
k3[16] = 2.019;
k3[17] = 2.11;
k3[18] = 2.2;
k3[19] = 2.291;
k3[20] = 2.517;
k3[21] = 2.744;

fs_1100 = (c10[0] + k2[0]*scon_n)*log(1100.0/k1[0]);

if((*vpga) < k1[0])   /* 'pga' should really be 'a_1100' below, but this is unknown */
   {
   fs_vpga = c10[0]*log((*vpga)/k1[0]) +
             k2[0]*(log(((*pga) + scon_c*exp(scon_n*log((*vpga)/k1[0])))/((*pga) + scon_c)));
   }
else if((*vpga) < 1100.0)
   fs_vpga = (c10[0] + k2[0]*scon_n)*log((*vpga)/k1[0]);
else
   fs_vpga = (c10[0] + k2[0]*scon_n)*log(1100.0/k1[0]);

a_1100 = (*pga)*exp(fs_1100 - fs_vpga);

ampf_cap = -1.0;
for(i=0;i<nper;i++)
   {
   if((*vsite) < k1[i])
      {
      fsite = c10[i]*log((*vsite)/k1[i]) +
                k2[i]*(log((a_1100 + scon_c*exp(scon_n*log((*vsite)/k1[i])))/(a_1100 + scon_c)));
      }
   else if((*vsite) < 1100.0)
      fsite = (c10[i] + k2[i]*scon_n)*log((*vsite)/k1[i]);
   else
      fsite = (c10[i] + k2[i]*scon_n)*log(1100.0/k1[i]);

   if((*vref) < k1[i])
      {
      fs_vref = c10[i]*log((*vref)/k1[i]) +
                k2[i]*(log((a_1100 + scon_c*exp(scon_n*log((*vref)/k1[i])))/(a_1100 + scon_c)));
      }
   else if((*vref) < 1100.0)
      fs_vref = (c10[i] + k2[i]*scon_n)*log((*vref)/k1[i]);
   else
      fs_vref = (c10[i] + k2[i]*scon_n)*log(1100.0/k1[i]);

   ampf0[i] = exp(fsite - fs_vref);

   if(1.0/per[i] <= (*flowcap))
      {
      if(ampf_cap < 0.0)
         ampf_cap = ampf0[i];
      else
         ampf0[i] = ampf_cap;
      }
   }

   /* go in reverse order so frequencies are increasing */

j = nper - 1;
f0 = 1.0/per[j];
a0 = ampf0[j];
f1 = 1.0/per[j];
a1 = ampf0[j];
dadf = 0.0;

df = 1.0/(n*(*dt));
for(i=1;i<n/2;i++)
   {
   freq = i*df;

   if(freq > f1)
      {
      f0 = f1;
      a0 = a1;

      if(j > 0)
         j--;

      if(per[j] != 0.0)
         f1 = 1.0/per[j];
      else
         f1 = 1000.0;

      a1 = ampf0[j];

      if(f1 != f0)
	 /*
         dadf = (a1-a0)/(f1-f0);
	 */
         dadf = (a1-a0)/log(f1/f0);
      else
         dadf = 0.0;
      }

   /*
   ampv = a0 + dadf*(freq-f0);
   */
   ampv = a0 + dadf*log(freq/f0);

   if(freq < *fmin)
      afac = 1.0;

   else if(freq < *fmidbot)
      /*
      afac = 1.0 + (freq - *fmin)*(ampv - 1.0)/(*fmidbot - *fmin);
      */
      afac = 1.0 + log(freq/(*fmin))*(ampv - 1.0)/log((*fmidbot)/(*fmin));

   else if(freq < *fmid)
      afac = ampv;

   else if(freq < *fhigh)
      afac = ampv;

   else if(freq < *fhightop)
      afac = ampv;

   else if(freq < *fmax)
      /*
      afac = ampv + (freq - *fhightop)*(1.0 - ampv)/(*fmax - *fhightop);
      */
      afac = ampv + log(freq/(*fhightop))*(1.0 - ampv)/log((*fmax)/(*fhightop));

   else
      afac = 1.0;

   ampf[i] = afac;
   }
}

void cb2014_ampf(float *ampf,float *dt,int n,float *vref,float *vsite,float *vpga,float *pga,float *fmin,float *fmidbot,float *fmid,float *fhigh,float *fhightop,float *fmax,float *flowcap)
{
float scon_c, scon_n, per[22], c11[22], k1[22], k2[22];
float ampf0[22];
float a_1100, fs_1100, fs_vpga, fs_vref, fsite;
float df, ampv, afac, freq;
float f0, f1, a0, a1, dadf, ampf_cap;
int i, j;
int nper = 22;

scon_c = 1.88;
scon_n = 1.18;

per[ 0] = 0.0;
per[ 1] = 0.01;
per[ 2] = 0.02;
per[ 3] = 0.03;
per[ 4] = 0.05;
per[ 5] = 0.075;
per[ 6] = 0.10;
per[ 7] = 0.15;
per[ 8] = 0.20;
per[ 9] = 0.25;
per[10] = 0.30;
per[11] = 0.40;
per[12] = 0.50;
per[13] = 0.75;
per[14] = 1.00;
per[15] = 1.50;
per[16] = 2.00;
per[17] = 3.00;
per[18] = 4.00;
per[19] = 5.00;
per[20] = 7.50;
per[21] = 10.0;

c11[ 0] = 1.090;
c11[ 1] = 1.094;
c11[ 2] = 1.149;
c11[ 3] = 1.290;
c11[ 4] = 1.449;
c11[ 5] = 1.535;
c11[ 6] = 1.615;
c11[ 7] = 1.877;
c11[ 8] = 2.069;
c11[ 9] = 2.205;
c11[10] = 2.306;
c11[11] = 2.398;
c11[12] = 2.355;
c11[13] = 1.995;
c11[14] = 1.447;
c11[15] = 0.330;
c11[16] = -0.514;
c11[17] = -0.848;
c11[18] = -0.793;
c11[19] = -0.748;
c11[20] = -0.664;
c11[21] = -0.576;

k1[ 0] = 865.0;
k1[ 1] = 865.0;
k1[ 2] = 865.0;
k1[ 3] = 908.0;
k1[ 4] = 1054.0;
k1[ 5] = 1086.0;
k1[ 6] = 1032.0;
k1[ 7] = 878.0;
k1[ 8] = 748.0;
k1[ 9] = 654.0;
k1[10] = 587.0;
k1[11] = 503.0;
k1[12] = 457.0;
k1[13] = 410.0;
k1[14] = 400.0;
k1[15] = 400.0;
k1[16] = 400.0;
k1[17] = 400.0;
k1[18] = 400.0;
k1[19] = 400.0;
k1[20] = 400.0;
k1[21] = 400.0;

k2[ 0] = -1.186;
k2[ 1] = -1.186;
k2[ 2] = -1.219;
k2[ 3] = -1.273;
k2[ 4] = -1.346;
k2[ 5] = -1.471;
k2[ 6] = -1.624;
k2[ 7] = -1.931;
k2[ 8] = -2.188;
k2[ 9] = -2.381;
k2[10] = -2.518;
k2[11] = -2.657;
k2[12] = -2.669;
k2[13] = -2.401;
k2[14] = -1.955;
k2[15] = -1.025;
k2[16] = -0.299;
k2[17] = 0.0;
k2[18] = 0.0;
k2[19] = 0.0;
k2[20] = 0.0;
k2[21] = 0.0;

fs_1100 = (c11[0] + k2[0]*scon_n)*log(1100.0/k1[0]);

if((*vpga) < k1[0])   /* 'pga' should really be 'a_1100' below, but this is unknown */
   {
   fs_vpga = c11[0]*log((*vpga)/k1[0]) +
             k2[0]*(log(((*pga) + scon_c*exp(scon_n*log((*vpga)/k1[0])))/((*pga) + scon_c)));
   }
else if((*vpga) < 1100.0)
   fs_vpga = (c11[0] + k2[0]*scon_n)*log((*vpga)/k1[0]);
else
   fs_vpga = (c11[0] + k2[0]*scon_n)*log(1100.0/k1[0]);

a_1100 = (*pga)*exp(fs_1100 - fs_vpga);

ampf_cap = -1.0;
for(i=0;i<nper;i++)
   {
   if((*vsite) < k1[i])
      {
      fsite = c11[i]*log((*vsite)/k1[i]) +
                k2[i]*(log((a_1100 + scon_c*exp(scon_n*log((*vsite)/k1[i])))/(a_1100 + scon_c)));
      }
   else if((*vsite) < 1100.0)
      fsite = (c11[i] + k2[i]*scon_n)*log((*vsite)/k1[i]);
   else
      fsite = (c11[i] + k2[i]*scon_n)*log(1100.0/k1[i]);

   if((*vref) < k1[i])
      {
      fs_vref = c11[i]*log((*vref)/k1[i]) +
                k2[i]*(log((a_1100 + scon_c*exp(scon_n*log((*vref)/k1[i])))/(a_1100 + scon_c)));
      }
   else if((*vref) < 1100.0)
      fs_vref = (c11[i] + k2[i]*scon_n)*log((*vref)/k1[i]);
   else
      fs_vref = (c11[i] + k2[i]*scon_n)*log(1100.0/k1[i]);

   ampf0[i] = exp(fsite - fs_vref);

   if(1.0/per[i] <= (*flowcap))
      {
      if(ampf_cap < 0.0)
         ampf_cap = ampf0[i];
      else
         ampf0[i] = ampf_cap;
      }
   }

   /* go in reverse order so frequencies are increasing */

j = nper - 1;
f0 = 1.0/per[j];
a0 = ampf0[j];
f1 = 1.0/per[j];
a1 = ampf0[j];
dadf = 0.0;

df = 1.0/(n*(*dt));
for(i=1;i<n/2;i++)
   {
   freq = i*df;

   if(freq > f1)
      {
      f0 = f1;
      a0 = a1;

      if(j > 0)
         j--;

      if(per[j] != 0.0)
         f1 = 1.0/per[j];
      else
         f1 = 1000.0;

      a1 = ampf0[j];

      if(f1 != f0)
	 /*
         dadf = (a1-a0)/(f1-f0);
	 */
         dadf = (a1-a0)/log(f1/f0);
      else
         dadf = 0.0;
      }

   /*
   ampv = a0 + dadf*(freq-f0);
   */
   ampv = a0 + dadf*log(freq/f0);

   if(freq < *fmin)
      afac = 1.0;

   else if(freq < *fmidbot)
      /*
      afac = 1.0 + (freq - *fmin)*(ampv - 1.0)/(*fmidbot - *fmin);
      */
      afac = 1.0 + log(freq/(*fmin))*(ampv - 1.0)/log((*fmidbot)/(*fmin));

   else if(freq < *fmid)
      afac = ampv;

   else if(freq < *fhigh)
      afac = ampv;

   else if(freq < *fhightop)
      afac = ampv;

   else if(freq < *fmax)
      /*
      afac = ampv + (freq - *fhightop)*(1.0 - ampv)/(*fmax - *fhightop);
      */
      afac = ampv + log(freq/(*fhightop))*(1.0 - ampv)/log((*fmax)/(*fhightop));

   else
      afac = 1.0;

   ampf[i] = afac;
   }
}

void bssa2014_ampf(float *ampf,float *dt,int n,float *vref,float *vsite,float *vpga,float *pga,float *fmin,float *fmidbot,float *fmid,float *fhigh,float *fhightop,float *fmax,float *flowcap)
{
float per[106], cc[106], vc[106], v760[106], f1[106], f3[106], f4[106], f5[106], f6[106], f7[106];
float ampf0[106];
float pga760, flin, fnon, vmin, f2, fs_vpga, fs_vref, fsite;
float df, ampv, afac, freq;
float fr0, fr1, a0, a1, dadf, ampf_cap;
int i, j;
int nper = 106;

per[  0] = 0.000;
per[  1] = 0.010;
per[  2] = 0.020;
per[  3] = 0.022;
per[  4] = 0.025;
per[  5] = 0.029;
per[  6] = 0.030;
per[  7] = 0.032;
per[  8] = 0.035;
per[  9] = 0.036;
per[ 10] = 0.040;
per[ 11] = 0.042;
per[ 12] = 0.044;
per[ 13] = 0.045;
per[ 14] = 0.046;
per[ 15] = 0.048;
per[ 16] = 0.050;
per[ 17] = 0.055;
per[ 18] = 0.060;
per[ 19] = 0.065;
per[ 20] = 0.067;
per[ 21] = 0.070;
per[ 22] = 0.075;
per[ 23] = 0.080;
per[ 24] = 0.085;
per[ 25] = 0.090;
per[ 26] = 0.095;
per[ 27] = 0.100;
per[ 28] = 0.110;
per[ 29] = 0.120;
per[ 30] = 0.130;
per[ 31] = 0.133;
per[ 32] = 0.140;
per[ 33] = 0.150;
per[ 34] = 0.160;
per[ 35] = 0.170;
per[ 36] = 0.180;
per[ 37] = 0.190;
per[ 38] = 0.200;
per[ 39] = 0.220;
per[ 40] = 0.240;
per[ 41] = 0.250;
per[ 42] = 0.260;
per[ 43] = 0.280;
per[ 44] = 0.290;
per[ 45] = 0.300;
per[ 46] = 0.320;
per[ 47] = 0.340;
per[ 48] = 0.350;
per[ 49] = 0.360;
per[ 50] = 0.380;
per[ 51] = 0.400;
per[ 52] = 0.420;
per[ 53] = 0.440;
per[ 54] = 0.450;
per[ 55] = 0.460;
per[ 56] = 0.480;
per[ 57] = 0.500;
per[ 58] = 0.550;
per[ 59] = 0.600;
per[ 60] = 0.650;
per[ 61] = 0.667;
per[ 62] = 0.700;
per[ 63] = 0.750;
per[ 64] = 0.800;
per[ 65] = 0.850;
per[ 66] = 0.900;
per[ 67] = 0.950;
per[ 68] = 1.000;
per[ 69] = 1.100;
per[ 70] = 1.200;
per[ 71] = 1.300;
per[ 72] = 1.400;
per[ 73] = 1.500;
per[ 74] = 1.600;
per[ 75] = 1.700;
per[ 76] = 1.800;
per[ 77] = 1.900;
per[ 78] = 2.000;
per[ 79] = 2.200;
per[ 80] = 2.400;
per[ 81] = 2.500;
per[ 82] = 2.600;
per[ 83] = 2.800;
per[ 84] = 3.000;
per[ 85] = 3.200;
per[ 86] = 3.400;
per[ 87] = 3.500;
per[ 88] = 3.600;
per[ 89] = 3.800;
per[ 90] = 4.000;
per[ 91] = 4.200;
per[ 92] = 4.400;
per[ 93] = 4.600;
per[ 94] = 4.800;
per[ 95] = 5.000;
per[ 96] = 5.500;
per[ 97] = 6.000;
per[ 98] = 6.500;
per[ 99] = 7.000;
per[100] = 7.500;
per[101] = 8.000;
per[102] = 8.500;
per[103] = 9.000;
per[104] = 9.500;
per[105] = 10.000;

cc[  0] = -0.5150;
cc[  1] = -0.5257;
cc[  2] = -0.5362;
cc[  3] = -0.5403;
cc[  4] = -0.5410;
cc[  5] = -0.5391;
cc[  6] = -0.5399;
cc[  7] = -0.5394;
cc[  8] = -0.5358;
cc[  9] = -0.5315;
cc[ 10] = -0.5264;
cc[ 11] = -0.5209;
cc[ 12] = -0.5142;
cc[ 13] = -0.5067;
cc[ 14] = -0.4991;
cc[ 15] = -0.4916;
cc[ 16] = -0.4850;
cc[ 17] = -0.4788;
cc[ 18] = -0.4735;
cc[ 19] = -0.4687;
cc[ 20] = -0.4646;
cc[ 21] = -0.4616;
cc[ 22] = -0.4598;
cc[ 23] = -0.4601;
cc[ 24] = -0.4620;
cc[ 25] = -0.4652;
cc[ 26] = -0.4688;
cc[ 27] = -0.4732;
cc[ 28] = -0.4787;
cc[ 29] = -0.4853;
cc[ 30] = -0.4931;
cc[ 31] = -0.5022;
cc[ 32] = -0.5126;
cc[ 33] = -0.5244;
cc[ 34] = -0.5392;
cc[ 35] = -0.5569;
cc[ 36] = -0.5758;
cc[ 37] = -0.5962;
cc[ 38] = -0.6192;
cc[ 39] = -0.6426;
cc[ 40] = -0.6658;
cc[ 41] = -0.6897;
cc[ 42] = -0.7133;
cc[ 43] = -0.7356;
cc[ 44] = -0.7567;
cc[ 45] = -0.7749;
cc[ 46] = -0.7902;
cc[ 47] = -0.8048;
cc[ 48] = -0.8186;
cc[ 49] = -0.8298;
cc[ 50] = -0.8401;
cc[ 51] = -0.8501;
cc[ 52] = -0.8590;
cc[ 53] = -0.8685;
cc[ 54] = -0.8790;
cc[ 55] = -0.8903;
cc[ 56] = -0.9011;
cc[ 57] = -0.9118;
cc[ 58] = -0.9227;
cc[ 59] = -0.9338;
cc[ 60] = -0.9453;
cc[ 61] = -0.9573;
cc[ 62] = -0.9692;
cc[ 63] = -0.9811;
cc[ 64] = -0.9924;
cc[ 65] = -1.0033;
cc[ 66] = -1.0139;
cc[ 67] = -1.0250;
cc[ 68] = -1.0361;
cc[ 69] = -1.0467;
cc[ 70] = -1.0565;
cc[ 71] = -1.0655;
cc[ 72] = -1.0736;
cc[ 73] = -1.0808;
cc[ 74] = -1.0867;
cc[ 75] = -1.0904;
cc[ 76] = -1.0923;
cc[ 77] = -1.0925;
cc[ 78] = -1.0908;
cc[ 79] = -1.0872;
cc[ 80] = -1.0819;
cc[ 81] = -1.0753;
cc[ 82] = -1.0682;
cc[ 83] = -1.0605;
cc[ 84] = -1.0521;
cc[ 85] = -1.0435;
cc[ 86] = -1.0350;
cc[ 87] = -1.0265;
cc[ 88] = -1.0180;
cc[ 89] = -1.0101;
cc[ 90] = -1.0028;
cc[ 91] = -0.9949;
cc[ 92] = -0.9859;
cc[ 93] = -0.9748;
cc[ 94] = -0.9613;
cc[ 95] = -0.9456;
cc[ 96] = -0.9273;
cc[ 97] = -0.9063;
cc[ 98] = -0.8822;
cc[ 99] = -0.8551;
cc[100] = -0.8249;
cc[101] = -0.7990;
cc[102] = -0.7620;
cc[103] = -0.7230;
cc[104] = -0.6840;
cc[105] = -0.6440;

vc[  0] = 925.00;
vc[  1] = 930.00;
vc[  2] = 967.50;
vc[  3] = 964.23;
vc[  4] = 961.65;
vc[  5] = 959.61;
vc[  6] = 959.71;
vc[  7] = 956.83;
vc[  8] = 955.39;
vc[  9] = 954.35;
vc[ 10] = 953.91;
vc[ 11] = 954.10;
vc[ 12] = 955.15;
vc[ 13] = 957.18;
vc[ 14] = 960.17;
vc[ 15] = 963.44;
vc[ 16] = 967.06;
vc[ 17] = 970.75;
vc[ 18] = 973.97;
vc[ 19] = 976.38;
vc[ 20] = 977.78;
vc[ 21] = 978.02;
vc[ 22] = 977.23;
vc[ 23] = 974.98;
vc[ 24] = 972.16;
vc[ 25] = 969.48;
vc[ 26] = 966.90;
vc[ 27] = 964.90;
vc[ 28] = 963.89;
vc[ 29] = 964.03;
vc[ 30] = 965.34;
vc[ 31] = 967.71;
vc[ 32] = 970.89;
vc[ 33] = 974.53;
vc[ 34] = 977.78;
vc[ 35] = 979.37;
vc[ 36] = 979.38;
vc[ 37] = 978.42;
vc[ 38] = 975.61;
vc[ 39] = 971.31;
vc[ 40] = 965.97;
vc[ 41] = 960.05;
vc[ 42] = 954.24;
vc[ 43] = 948.77;
vc[ 44] = 943.90;
vc[ 45] = 940.75;
vc[ 46] = 939.61;
vc[ 47] = 939.66;
vc[ 48] = 940.74;
vc[ 49] = 943.02;
vc[ 50] = 945.83;
vc[ 51] = 949.18;
vc[ 52] = 952.96;
vc[ 53] = 957.31;
vc[ 54] = 962.25;
vc[ 55] = 967.61;
vc[ 56] = 972.54;
vc[ 57] = 977.09;
vc[ 58] = 981.13;
vc[ 59] = 984.26;
vc[ 60] = 986.32;
vc[ 61] = 987.12;
vc[ 62] = 986.52;
vc[ 63] = 984.70;
vc[ 64] = 981.17;
vc[ 65] = 976.97;
vc[ 66] = 972.90;
vc[ 67] = 969.79;
vc[ 68] = 967.51;
vc[ 69] = 965.94;
vc[ 70] = 965.20;
vc[ 71] = 965.38;
vc[ 72] = 966.44;
vc[ 73] = 968.24;
vc[ 74] = 969.94;
vc[ 75] = 971.24;
vc[ 76] = 971.65;
vc[ 77] = 970.45;
vc[ 78] = 966.44;
vc[ 79] = 959.61;
vc[ 80] = 950.34;
vc[ 81] = 939.03;
vc[ 82] = 926.85;
vc[ 83] = 914.07;
vc[ 84] = 900.07;
vc[ 85] = 885.63;
vc[ 86] = 871.15;
vc[ 87] = 856.21;
vc[ 88] = 840.97;
vc[ 89] = 826.47;
vc[ 90] = 812.92;
vc[ 91] = 799.72;
vc[ 92] = 787.55;
vc[ 93] = 776.05;
vc[ 94] = 765.55;
vc[ 95] = 756.97;
vc[ 96] = 735.74;
vc[ 97] = 728.14;
vc[ 98] = 726.30;
vc[ 99] = 728.24;
vc[100] = 731.96;
vc[101] = 735.81;
vc[102] = 739.50;
vc[103] = 743.07;
vc[104] = 746.55;
vc[105] = 750.00;

v760[  0] = 760;
v760[  1] = 760;
v760[  2] = 760;
v760[  3] = 760;
v760[  4] = 760;
v760[  5] = 760;
v760[  6] = 760;
v760[  7] = 760;
v760[  8] = 760;
v760[  9] = 760;
v760[ 10] = 760;
v760[ 11] = 760;
v760[ 12] = 760;
v760[ 13] = 760;
v760[ 14] = 760;
v760[ 15] = 760;
v760[ 16] = 760;
v760[ 17] = 760;
v760[ 18] = 760;
v760[ 19] = 760;
v760[ 20] = 760;
v760[ 21] = 760;
v760[ 22] = 760;
v760[ 23] = 760;
v760[ 24] = 760;
v760[ 25] = 760;
v760[ 26] = 760;
v760[ 27] = 760;
v760[ 28] = 760;
v760[ 29] = 760;
v760[ 30] = 760;
v760[ 31] = 760;
v760[ 32] = 760;
v760[ 33] = 760;
v760[ 34] = 760;
v760[ 35] = 760;
v760[ 36] = 760;
v760[ 37] = 760;
v760[ 38] = 760;
v760[ 39] = 760;
v760[ 40] = 760;
v760[ 41] = 760;
v760[ 42] = 760;
v760[ 43] = 760;
v760[ 44] = 760;
v760[ 45] = 760;
v760[ 46] = 760;
v760[ 47] = 760;
v760[ 48] = 760;
v760[ 49] = 760;
v760[ 50] = 760;
v760[ 51] = 760;
v760[ 52] = 760;
v760[ 53] = 760;
v760[ 54] = 760;
v760[ 55] = 760;
v760[ 56] = 760;
v760[ 57] = 760;
v760[ 58] = 760;
v760[ 59] = 760;
v760[ 60] = 760;
v760[ 61] = 760;
v760[ 62] = 760;
v760[ 63] = 760;
v760[ 64] = 760;
v760[ 65] = 760;
v760[ 66] = 760;
v760[ 67] = 760;
v760[ 68] = 760;
v760[ 69] = 760;
v760[ 70] = 760;
v760[ 71] = 760;
v760[ 72] = 760;
v760[ 73] = 760;
v760[ 74] = 760;
v760[ 75] = 760;
v760[ 76] = 760;
v760[ 77] = 760;
v760[ 78] = 760;
v760[ 79] = 760;
v760[ 80] = 760;
v760[ 81] = 760;
v760[ 82] = 760;
v760[ 83] = 760;
v760[ 84] = 760;
v760[ 85] = 760;
v760[ 86] = 760;
v760[ 87] = 760;
v760[ 88] = 760;
v760[ 89] = 760;
v760[ 90] = 760;
v760[ 91] = 760;
v760[ 92] = 760;
v760[ 93] = 760;
v760[ 94] = 760;
v760[ 95] = 760;
v760[ 96] = 760;
v760[ 97] = 760;
v760[ 98] = 760;
v760[ 99] = 760;
v760[100] = 760;
v760[101] = 760;
v760[102] = 760;
v760[103] = 760;
v760[104] = 760;
v760[105] = 760;

f1[  0] = 0;
f1[  1] = 0;
f1[  2] = 0;
f1[  3] = 0;
f1[  4] = 0;
f1[  5] = 0;
f1[  6] = 0;
f1[  7] = 0;
f1[  8] = 0;
f1[  9] = 0;
f1[ 10] = 0;
f1[ 11] = 0;
f1[ 12] = 0;
f1[ 13] = 0;
f1[ 14] = 0;
f1[ 15] = 0;
f1[ 16] = 0;
f1[ 17] = 0;
f1[ 18] = 0;
f1[ 19] = 0;
f1[ 20] = 0;
f1[ 21] = 0;
f1[ 22] = 0;
f1[ 23] = 0;
f1[ 24] = 0;
f1[ 25] = 0;
f1[ 26] = 0;
f1[ 27] = 0;
f1[ 28] = 0;
f1[ 29] = 0;
f1[ 30] = 0;
f1[ 31] = 0;
f1[ 32] = 0;
f1[ 33] = 0;
f1[ 34] = 0;
f1[ 35] = 0;
f1[ 36] = 0;
f1[ 37] = 0;
f1[ 38] = 0;
f1[ 39] = 0;
f1[ 40] = 0;
f1[ 41] = 0;
f1[ 42] = 0;
f1[ 43] = 0;
f1[ 44] = 0;
f1[ 45] = 0;
f1[ 46] = 0;
f1[ 47] = 0;
f1[ 48] = 0;
f1[ 49] = 0;
f1[ 50] = 0;
f1[ 51] = 0;
f1[ 52] = 0;
f1[ 53] = 0;
f1[ 54] = 0;
f1[ 55] = 0;
f1[ 56] = 0;
f1[ 57] = 0;
f1[ 58] = 0;
f1[ 59] = 0;
f1[ 60] = 0;
f1[ 61] = 0;
f1[ 62] = 0;
f1[ 63] = 0;
f1[ 64] = 0;
f1[ 65] = 0;
f1[ 66] = 0;
f1[ 67] = 0;
f1[ 68] = 0;
f1[ 69] = 0;
f1[ 70] = 0;
f1[ 71] = 0;
f1[ 72] = 0;
f1[ 73] = 0;
f1[ 74] = 0;
f1[ 75] = 0;
f1[ 76] = 0;
f1[ 77] = 0;
f1[ 78] = 0;
f1[ 79] = 0;
f1[ 80] = 0;
f1[ 81] = 0;
f1[ 82] = 0;
f1[ 83] = 0;
f1[ 84] = 0;
f1[ 85] = 0;
f1[ 86] = 0;
f1[ 87] = 0;
f1[ 88] = 0;
f1[ 89] = 0;
f1[ 90] = 0;
f1[ 91] = 0;
f1[ 92] = 0;
f1[ 93] = 0;
f1[ 94] = 0;
f1[ 95] = 0;
f1[ 96] = 0;
f1[ 97] = 0;
f1[ 98] = 0;
f1[ 99] = 0;
f1[100] = 0;
f1[101] = 0;
f1[102] = 0;
f1[103] = 0;
f1[104] = 0;
f1[105] = 0;

f3[  0] = 0.1;
f3[  1] = 0.1;
f3[  2] = 0.1;
f3[  3] = 0.1;
f3[  4] = 0.1;
f3[  5] = 0.1;
f3[  6] = 0.1;
f3[  7] = 0.1;
f3[  8] = 0.1;
f3[  9] = 0.1;
f3[ 10] = 0.1;
f3[ 11] = 0.1;
f3[ 12] = 0.1;
f3[ 13] = 0.1;
f3[ 14] = 0.1;
f3[ 15] = 0.1;
f3[ 16] = 0.1;
f3[ 17] = 0.1;
f3[ 18] = 0.1;
f3[ 19] = 0.1;
f3[ 20] = 0.1;
f3[ 21] = 0.1;
f3[ 22] = 0.1;
f3[ 23] = 0.1;
f3[ 24] = 0.1;
f3[ 25] = 0.1;
f3[ 26] = 0.1;
f3[ 27] = 0.1;
f3[ 28] = 0.1;
f3[ 29] = 0.1;
f3[ 30] = 0.1;
f3[ 31] = 0.1;
f3[ 32] = 0.1;
f3[ 33] = 0.1;
f3[ 34] = 0.1;
f3[ 35] = 0.1;
f3[ 36] = 0.1;
f3[ 37] = 0.1;
f3[ 38] = 0.1;
f3[ 39] = 0.1;
f3[ 40] = 0.1;
f3[ 41] = 0.1;
f3[ 42] = 0.1;
f3[ 43] = 0.1;
f3[ 44] = 0.1;
f3[ 45] = 0.1;
f3[ 46] = 0.1;
f3[ 47] = 0.1;
f3[ 48] = 0.1;
f3[ 49] = 0.1;
f3[ 50] = 0.1;
f3[ 51] = 0.1;
f3[ 52] = 0.1;
f3[ 53] = 0.1;
f3[ 54] = 0.1;
f3[ 55] = 0.1;
f3[ 56] = 0.1;
f3[ 57] = 0.1;
f3[ 58] = 0.1;
f3[ 59] = 0.1;
f3[ 60] = 0.1;
f3[ 61] = 0.1;
f3[ 62] = 0.1;
f3[ 63] = 0.1;
f3[ 64] = 0.1;
f3[ 65] = 0.1;
f3[ 66] = 0.1;
f3[ 67] = 0.1;
f3[ 68] = 0.1;
f3[ 69] = 0.1;
f3[ 70] = 0.1;
f3[ 71] = 0.1;
f3[ 72] = 0.1;
f3[ 73] = 0.1;
f3[ 74] = 0.1;
f3[ 75] = 0.1;
f3[ 76] = 0.1;
f3[ 77] = 0.1;
f3[ 78] = 0.1;
f3[ 79] = 0.1;
f3[ 80] = 0.1;
f3[ 81] = 0.1;
f3[ 82] = 0.1;
f3[ 83] = 0.1;
f3[ 84] = 0.1;
f3[ 85] = 0.1;
f3[ 86] = 0.1;
f3[ 87] = 0.1;
f3[ 88] = 0.1;
f3[ 89] = 0.1;
f3[ 90] = 0.1;
f3[ 91] = 0.1;
f3[ 92] = 0.1;
f3[ 93] = 0.1;
f3[ 94] = 0.1;
f3[ 95] = 0.1;
f3[ 96] = 0.1;
f3[ 97] = 0.1;
f3[ 98] = 0.1;
f3[ 99] = 0.1;
f3[100] = 0.1;
f3[101] = 0.1;
f3[102] = 0.1;
f3[103] = 0.1;
f3[104] = 0.1;
f3[105] = 0.1;

f4[  0] = -0.1500;
f4[  1] = -0.1483;
f4[  2] = -0.1471;
f4[  3] = -0.1477;
f4[  4] = -0.1496;
f4[  5] = -0.1525;
f4[  6] = -0.1549;
f4[  7] = -0.1574;
f4[  8] = -0.1607;
f4[  9] = -0.1641;
f4[ 10] = -0.1678;
f4[ 11] = -0.1715;
f4[ 12] = -0.1760;
f4[ 13] = -0.1810;
f4[ 14] = -0.1862;
f4[ 15] = -0.1915;
f4[ 16] = -0.1963;
f4[ 17] = -0.2014;
f4[ 18] = -0.2066;
f4[ 19] = -0.2120;
f4[ 20] = -0.2176;
f4[ 21] = -0.2232;
f4[ 22] = -0.2287;
f4[ 23] = -0.2337;
f4[ 24] = -0.2382;
f4[ 25] = -0.2421;
f4[ 26] = -0.2458;
f4[ 27] = -0.2492;
f4[ 28] = -0.2519;
f4[ 29] = -0.2540;
f4[ 30] = -0.2556;
f4[ 31] = -0.2566;
f4[ 32] = -0.2571;
f4[ 33] = -0.2571;
f4[ 34] = -0.2562;
f4[ 35] = -0.2544;
f4[ 36] = -0.2522;
f4[ 37] = -0.2497;
f4[ 38] = -0.2466;
f4[ 39] = -0.2432;
f4[ 40] = -0.2396;
f4[ 41] = -0.2357;
f4[ 42] = -0.2315;
f4[ 43] = -0.2274;
f4[ 44] = -0.2232;
f4[ 45] = -0.2191;
f4[ 46] = -0.2152;
f4[ 47] = -0.2112;
f4[ 48] = -0.2070;
f4[ 49] = -0.2033;
f4[ 50] = -0.1996;
f4[ 51] = -0.1958;
f4[ 52] = -0.1922;
f4[ 53] = -0.1884;
f4[ 54] = -0.1840;
f4[ 55] = -0.1793;
f4[ 56] = -0.1749;
f4[ 57] = -0.1704;
f4[ 58] = -0.1658;
f4[ 59] = -0.1610;
f4[ 60] = -0.1558;
f4[ 61] = -0.1503;
f4[ 62] = -0.1446;
f4[ 63] = -0.1387;
f4[ 64] = -0.1325;
f4[ 65] = -0.1262;
f4[ 66] = -0.1197;
f4[ 67] = -0.1126;
f4[ 68] = -0.1052;
f4[ 69] = -0.0977;
f4[ 70] = -0.0902;
f4[ 71] = -0.0827;
f4[ 72] = -0.0753;
f4[ 73] = -0.0679;
f4[ 74] = -0.0604;
f4[ 75] = -0.0534;
f4[ 76] = -0.0470;
f4[ 77] = -0.0414;
f4[ 78] = -0.0361;
f4[ 79] = -0.0314;
f4[ 80] = -0.0271;
f4[ 81] = -0.0231;
f4[ 82] = -0.0196;
f4[ 83] = -0.0165;
f4[ 84] = -0.0136;
f4[ 85] = -0.0112;
f4[ 86] = -0.0093;
f4[ 87] = -0.0075;
f4[ 88] = -0.0058;
f4[ 89] = -0.0044;
f4[ 90] = -0.0032;
f4[ 91] = -0.0023;
f4[ 92] = -0.0016;
f4[ 93] = -0.0010;
f4[ 94] = -0.0006;
f4[ 95] = -0.0003;
f4[ 96] = -0.0001;
f4[ 97] = 0.0000;
f4[ 98] = 0.0000;
f4[ 99] = 0.0000;
f4[100] = -0.0001;
f4[101] = 0.0001;
f4[102] = 0.0001;
f4[103] = 0.0001;
f4[104] = 0.0001;
f4[105] = 0.0000;

f5[  0] = -0.00701;
f5[  1] = -0.00701;
f5[  2] = -0.00728;
f5[  3] = -0.00732;
f5[  4] = -0.00736;
f5[  5] = -0.00737;
f5[  6] = -0.00735;
f5[  7] = -0.00731;
f5[  8] = -0.00721;
f5[  9] = -0.00717;
f5[ 10] = -0.00698;
f5[ 11] = -0.00687;
f5[ 12] = -0.00677;
f5[ 13] = -0.00672;
f5[ 14] = -0.00667;
f5[ 15] = -0.00656;
f5[ 16] = -0.00647;
f5[ 17] = -0.00625;
f5[ 18] = -0.00607;
f5[ 19] = -0.00593;
f5[ 20] = -0.00588;
f5[ 21] = -0.00582;
f5[ 22] = -0.00573;
f5[ 23] = -0.00567;
f5[ 24] = -0.00563;
f5[ 25] = -0.00561;
f5[ 26] = -0.00560;
f5[ 27] = -0.00560;
f5[ 28] = -0.00562;
f5[ 29] = -0.00567;
f5[ 30] = -0.00572;
f5[ 31] = -0.00574;
f5[ 32] = -0.00578;
f5[ 33] = -0.00585;
f5[ 34] = -0.00591;
f5[ 35] = -0.00597;
f5[ 36] = -0.00602;
f5[ 37] = -0.00608;
f5[ 38] = -0.00614;
f5[ 39] = -0.00626;
f5[ 40] = -0.00638;
f5[ 41] = -0.00644;
f5[ 42] = -0.00650;
f5[ 43] = -0.00660;
f5[ 44] = -0.00665;
f5[ 45] = -0.00670;
f5[ 46] = -0.00680;
f5[ 47] = -0.00689;
f5[ 48] = -0.00693;
f5[ 49] = -0.00697;
f5[ 50] = -0.00705;
f5[ 51] = -0.00713;
f5[ 52] = -0.00719;
f5[ 53] = -0.00726;
f5[ 54] = -0.00729;
f5[ 55] = -0.00732;
f5[ 56] = -0.00738;
f5[ 57] = -0.00744;
f5[ 58] = -0.00758;
f5[ 59] = -0.00773;
f5[ 60] = -0.00787;
f5[ 61] = -0.00792;
f5[ 62] = -0.00800;
f5[ 63] = -0.00812;
f5[ 64] = -0.00822;
f5[ 65] = -0.00830;
f5[ 66] = -0.00836;
f5[ 67] = -0.00841;
f5[ 68] = -0.00844;
f5[ 69] = -0.00847;
f5[ 70] = -0.00842;
f5[ 71] = -0.00829;
f5[ 72] = -0.00806;
f5[ 73] = -0.00771;
f5[ 74] = -0.00723;
f5[ 75] = -0.00666;
f5[ 76] = -0.00603;
f5[ 77] = -0.00540;
f5[ 78] = -0.00479;
f5[ 79] = -0.00378;
f5[ 80] = -0.00302;
f5[ 81] = -0.00272;
f5[ 82] = -0.00246;
f5[ 83] = -0.00208;
f5[ 84] = -0.00183;
f5[ 85] = -0.00167;
f5[ 86] = -0.00158;
f5[ 87] = -0.00155;
f5[ 88] = -0.00154;
f5[ 89] = -0.00152;
f5[ 90] = -0.00152;
f5[ 91] = -0.00152;
f5[ 92] = -0.00150;
f5[ 93] = -0.00148;
f5[ 94] = -0.00146;
f5[ 95] = -0.00144;
f5[ 96] = -0.00140;
f5[ 97] = -0.00138;
f5[ 98] = -0.00137;
f5[ 99] = -0.00137;
f5[100] = -0.00137;
f5[101] = -0.00137;
f5[102] = -0.00137;
f5[103] = -0.00137;
f5[104] = -0.00136;
f5[105] = -0.00136;

f6[  0] = -9.9;
f6[  1] = -9.9;
f6[  2] = -9.9;
f6[  3] = -9.9;
f6[  4] = -9.9;
f6[  5] = -9.9;
f6[  6] = -9.9;
f6[  7] = -9.9;
f6[  8] = -9.9;
f6[  9] = -9.9;
f6[ 10] = -9.9;
f6[ 11] = -9.9;
f6[ 12] = -9.9;
f6[ 13] = -9.9;
f6[ 14] = -9.9;
f6[ 15] = -9.9;
f6[ 16] = -9.9;
f6[ 17] = -9.9;
f6[ 18] = -9.9;
f6[ 19] = -9.9;
f6[ 20] = -9.9;
f6[ 21] = -9.9;
f6[ 22] = -9.9;
f6[ 23] = -9.9;
f6[ 24] = -9.9;
f6[ 25] = -9.9;
f6[ 26] = -9.9;
f6[ 27] = -9.9;
f6[ 28] = -9.9;
f6[ 29] = -9.9;
f6[ 30] = -9.9;
f6[ 31] = -9.9;
f6[ 32] = -9.9;
f6[ 33] = -9.9;
f6[ 34] = -9.9;
f6[ 35] = -9.9;
f6[ 36] = -9.9;
f6[ 37] = -9.9;
f6[ 38] = -9.9;
f6[ 39] = -9.9;
f6[ 40] = -9.9;
f6[ 41] = -9.9;
f6[ 42] = -9.9;
f6[ 43] = -9.9;
f6[ 44] = -9.9;
f6[ 45] = -9.9;
f6[ 46] = -9.9;
f6[ 47] = -9.9;
f6[ 48] = -9.9;
f6[ 49] = -9.9;
f6[ 50] = -9.9;
f6[ 51] = -9.9;
f6[ 52] = -9.9;
f6[ 53] = -9.9;
f6[ 54] = -9.9;
f6[ 55] = -9.9;
f6[ 56] = -9.9;
f6[ 57] = -9.9;
f6[ 58] = -9.9;
f6[ 59] = -9.9;
f6[ 60] = 0.006;
f6[ 61] = 0.026;
f6[ 62] = 0.055;
f6[ 63] = 0.092;
f6[ 64] = 0.140;
f6[ 65] = 0.195;
f6[ 66] = 0.252;
f6[ 67] = 0.309;
f6[ 68] = 0.367;
f6[ 69] = 0.425;
f6[ 70] = 0.481;
f6[ 71] = 0.536;
f6[ 72] = 0.588;
f6[ 73] = 0.638;
f6[ 74] = 0.689;
f6[ 75] = 0.736;
f6[ 76] = 0.780;
f6[ 77] = 0.824;
f6[ 78] = 0.871;
f6[ 79] = 0.920;
f6[ 80] = 0.969;
f6[ 81] = 1.017;
f6[ 82] = 1.060;
f6[ 83] = 1.099;
f6[ 84] = 1.135;
f6[ 85] = 1.164;
f6[ 86] = 1.188;
f6[ 87] = 1.211;
f6[ 88] = 1.234;
f6[ 89] = 1.253;
f6[ 90] = 1.271;
f6[ 91] = 1.287;
f6[ 92] = 1.300;
f6[ 93] = 1.312;
f6[ 94] = 1.323;
f6[ 95] = 1.329;
f6[ 96] = 1.345;
f6[ 97] = 1.350;
f6[ 98] = 1.349;
f6[ 99] = 1.342;
f6[100] = 1.329;
f6[101] = 1.308;
f6[102] = 1.282;
f6[103] = 1.252;
f6[104] = 1.218;
f6[105] = 1.183;

f7[  0] = -9.9;
f7[  1] = -9.9;
f7[  2] = -9.9;
f7[  3] = -9.9;
f7[  4] = -9.9;
f7[  5] = -9.9;
f7[  6] = -9.9;
f7[  7] = -9.9;
f7[  8] = -9.9;
f7[  9] = -9.9;
f7[ 10] = -9.9;
f7[ 11] = -9.9;
f7[ 12] = -9.9;
f7[ 13] = -9.9;
f7[ 14] = -9.9;
f7[ 15] = -9.9;
f7[ 16] = -9.9;
f7[ 17] = -9.9;
f7[ 18] = -9.9;
f7[ 19] = -9.9;
f7[ 20] = -9.9;
f7[ 21] = -9.9;
f7[ 22] = -9.9;
f7[ 23] = -9.9;
f7[ 24] = -9.9;
f7[ 25] = -9.9;
f7[ 26] = -9.9;
f7[ 27] = -9.9;
f7[ 28] = -9.9;
f7[ 29] = -9.9;
f7[ 30] = -9.9;
f7[ 31] = -9.9;
f7[ 32] = -9.9;
f7[ 33] = -9.9;
f7[ 34] = -9.9;
f7[ 35] = -9.9;
f7[ 36] = -9.9;
f7[ 37] = -9.9;
f7[ 38] = -9.9;
f7[ 39] = -9.9;
f7[ 40] = -9.9;
f7[ 41] = -9.9;
f7[ 42] = -9.9;
f7[ 43] = -9.9;
f7[ 44] = -9.9;
f7[ 45] = -9.9;
f7[ 46] = -9.9;
f7[ 47] = -9.9;
f7[ 48] = -9.9;
f7[ 49] = -9.9;
f7[ 50] = -9.9;
f7[ 51] = -9.9;
f7[ 52] = -9.9;
f7[ 53] = -9.9;
f7[ 54] = -9.9;
f7[ 55] = -9.9;
f7[ 56] = -9.9;
f7[ 57] = -9.9;
f7[ 58] = -9.9;
f7[ 59] = -9.9;
f7[ 60] = 0.004;
f7[ 61] = 0.017;
f7[ 62] = 0.036;
f7[ 63] = 0.059;
f7[ 64] = 0.088;
f7[ 65] = 0.120;
f7[ 66] = 0.152;
f7[ 67] = 0.181;
f7[ 68] = 0.208;
f7[ 69] = 0.233;
f7[ 70] = 0.256;
f7[ 71] = 0.276;
f7[ 72] = 0.294;
f7[ 73] = 0.309;
f7[ 74] = 0.324;
f7[ 75] = 0.337;
f7[ 76] = 0.350;
f7[ 77] = 0.364;
f7[ 78] = 0.382;
f7[ 79] = 0.404;
f7[ 80] = 0.427;
f7[ 81] = 0.451;
f7[ 82] = 0.474;
f7[ 83] = 0.495;
f7[ 84] = 0.516;
f7[ 85] = 0.534;
f7[ 86] = 0.551;
f7[ 87] = 0.570;
f7[ 88] = 0.589;
f7[ 89] = 0.609;
f7[ 90] = 0.629;
f7[ 91] = 0.652;
f7[ 92] = 0.674;
f7[ 93] = 0.697;
f7[ 94] = 0.719;
f7[ 95] = 0.738;
f7[ 96] = 0.778;
f7[ 97] = 0.803;
f7[ 98] = 0.815;
f7[ 99] = 0.816;
f7[100] = 0.809;
f7[101] = 0.795;
f7[102] = 0.777;
f7[103] = 0.754;
f7[104] = 0.729;
f7[105] = 0.703;

if((*vpga) < vc[0])
   fs_vpga = cc[0]*log((*vpga)/v760[0]);
else
   fs_vpga = cc[0]*log(vc[0]/v760[0]);

pga760 = (*pga)*exp(-fs_vpga);  /* negative because we're taking it out */

ampf_cap = -1.0;
for(i=0;i<nper;i++)
   {
   if((*vsite) < vc[i])
      flin = cc[i]*log((*vsite)/v760[i]);
   else
      flin = cc[i]*log(vc[i]/v760[i]);

   if((*vsite) < v760[i])
      vmin = (*vsite);
   else
      vmin = v760[i];

   f2 = f4[i]*(exp(f5[i]*(vmin - 360.0)) - exp(f5[i]*(v760[i] - 360.0)));

   fnon = f1[i] + f2*log((pga760 + f3[i])/f3[i]);

   fsite = flin + fnon;

   if((*vref) < vc[i])
      flin = cc[i]*log((*vref)/v760[i]);
   else
      flin = cc[i]*log(vc[i]/v760[i]);

   if((*vref) < v760[i])
      vmin = (*vref);
   else
      vmin = v760[i];

   f2 = f4[i]*(exp(f5[i]*(vmin - 360.0)) - exp(f5[i]*(v760[i] - 360.0)));

   fnon = f1[i] + f2*log((pga760 + f3[i])/f3[i]);

   fs_vref = flin + fnon;

   ampf0[i] = exp(fsite - fs_vref);

   if(1.0/per[i] <= (*flowcap))
      {
      if(ampf_cap < 0.0)
         ampf_cap = ampf0[i];
      else
         ampf0[i] = ampf_cap;
      }
   }

   /* go in reverse order so frequencies are increasing */

j = nper - 1;
fr0 = 1.0/per[j];
a0 = ampf0[j];
fr1 = 1.0/per[j];
a1 = ampf0[j];
dadf = 0.0;

df = 1.0/(n*(*dt));
for(i=1;i<n/2;i++)
   {
   freq = i*df;

   if(freq > fr1)
      {
      fr0 = fr1;
      a0 = a1;

      if(j > 0)
         j--;

      if(per[j] != 0.0)
         fr1 = 1.0/per[j];
      else
         fr1 = 1000.0;

      a1 = ampf0[j];

      if(fr1 != fr0)
         dadf = (a1-a0)/log(fr1/fr0);
      else
         dadf = 0.0;
      }

   ampv = a0 + dadf*log(freq/fr0);

   if(freq < *fmin)
      afac = 1.0;

   else if(freq < *fmidbot)
      afac = 1.0 + log(freq/(*fmin))*(ampv - 1.0)/log((*fmidbot)/(*fmin));

   else if(freq < *fmid)
      afac = ampv;

   else if(freq < *fhigh)
      afac = ampv;

   else if(freq < *fhightop)
      afac = ampv;

   else if(freq < *fmax)
      afac = ampv + log(freq/(*fhightop))*(1.0 - ampv)/log((*fmax)/(*fhightop));

   else
      afac = 1.0;

   ampf[i] = afac;
   }
}
