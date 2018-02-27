# pvp
## Pairwise P Value utility

This is a command line version of [US-SOMO](http://somo.uthscsa.edu)'s [CorMap](http://somo.uthscsa.edu/manual/cormap.html) facility.

## Dependencies

[Qt5](https://www.qt.io/) development tools

## Compile

```
qmake
make
```
## Run

```
pvp [options] files
```

Running without options displays full current options

## example

[Using Aldolase test data](http://somo.uthscsa.edu/sampledata.php)

```
$ pvp -m 0.01 -M 0.1 ../testdata/Aldolase_25_11_SOMO/aldo_pH7p5_Elution1_0022_?.dat
minq is set to 0.01
maxq is set to 0.1
../testdata/Aldolase_25_11_SOMO/aldo_pH7p5_Elution1_0022_0.dat
../testdata/Aldolase_25_11_SOMO/aldo_pH7p5_Elution1_0022_1.dat
../testdata/Aldolase_25_11_SOMO/aldo_pH7p5_Elution1_0022_2.dat
../testdata/Aldolase_25_11_SOMO/aldo_pH7p5_Elution1_0022_3.dat
../testdata/Aldolase_25_11_SOMO/aldo_pH7p5_Elution1_0022_4.dat
../testdata/Aldolase_25_11_SOMO/aldo_pH7p5_Elution1_0022_5.dat
../testdata/Aldolase_25_11_SOMO/aldo_pH7p5_Elution1_0022_6.dat
../testdata/Aldolase_25_11_SOMO/aldo_pH7p5_Elution1_0022_7.dat
../testdata/Aldolase_25_11_SOMO/aldo_pH7p5_Elution1_0022_8.dat
../testdata/Aldolase_25_11_SOMO/aldo_pH7p5_Elution1_0022_9.dat
10 files loaded

Alpha is 0.01

Pairwise P value map color definitions:
  P is the pairwise P value as determined by a CorMap analysis
  Green corresponds to         P >= 0.05
  Yellow corresponds to 0.05 > P >= 0.01
  Red corresponds to    0.01 > P
Axes ticks correspond to Ref. as listed below

P values:
  33.3% green (8.9%) + yellow (24.4%) pairs
  66.7% red pairs

Average one-to-all P value 0.01924 ±0.01261 (65.5%) % red 66.7% ±19.6 (29.4%)
Red cluster count 2, average size 15.00 ±0.00 (0.0%), average size as pct of total area 33.3% ±0.0
Red cluster maximum size 27 (60.0%) has 1 occurrence and begins at [1,8].

 Ref. : Name                         Avg. P value    Min. P Value      % Red
    1 : aldo_pH7p5_Elution1_0022_0     0.02586         0.0005454        55.6%
    2 : aldo_pH7p5_Elution1_0022_1     0.04102         0.0002708        33.3%
    3 : aldo_pH7p5_Elution1_0022_2     0.00394         1.22e-07         88.9%
    4 : aldo_pH7p5_Elution1_0022_3     0.02363         0.0005454        55.6%
    5 : aldo_pH7p5_Elution1_0022_4     0.03419         0.008939         55.6%
    6 : aldo_pH7p5_Elution1_0022_5     0.01888         2.459e-07        55.6%
    7 : aldo_pH7p5_Elution1_0022_6     0.02079         2.459e-07        77.8%
    8 : aldo_pH7p5_Elution1_0022_7     0.003335        1.22e-07        100.0%
    9 : aldo_pH7p5_Elution1_0022_8     0.006988        1.645e-05        77.8%
   10 : aldo_pH7p5_Elution1_0022_9     0.01379         0.001098         66.7%
```
& in pvpout.png: ![Image of pvpout.png](doc/sample-pvpout.png)


