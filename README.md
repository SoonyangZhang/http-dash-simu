# http-dash-simu
evaluate the adaptation algorithms on mininet.  
From CmakeLists.txt, you could find it depends on   
the code in https://github.com/SoonyangZhang/net  
```  
include_directories(${CMAKE_SOURCE_DIR}/logging)  
include_directories(${CMAKE_SOURCE_DIR}/base)  
include_directories(${CMAKE_SOURCE_DIR}/tcp)     
```
Download the project and put the folders (logging,base,tcp) under http-dash-simu.   

The adaptation algorithms are refered from https://github.com/haraldott/dash   
and https://github.com/djvergad/dash 
Thanks for their contribution.  
Eight algorithms are implemented:  
- [x] festive: Improving Fairness, Efficiency, and Stability in HTTP-based Adaptive Video Streaming with FESTIVE  
- [x] panda: Probe and Adapt: Rate Adaptation for HTTP Video Streaming At Scale  
- [x] tobasco: Adaptation Algorithm for Adaptive Streaming over HTTP  
- [x] osmp: QDASH: A QoE-aware DASH system  
- [x] raahs: Rate adaptation for adaptive HTTP streaming  
- [x] FDASH: A Fuzzy-Based MPEG/DASH Adaptation Algorithm   
- [x] sftm: Rate adaptation for dynamic adaptive streaming over HTTP in content distribution network  
- [x] svaa: Towards agile and smooth video adaptation in dynamic HTTP streaming  


# Build
```
cd build  
cmake ..
make
```

Test  
```
cd build   
sudo su  
python 2h1s.py  
```

During the emulation, the pairs (time, downloaded segments, downloaded video quality) will be loggged out.  
Plot the results:  
```
chmod 777 multiplot-quality.sh  
./multiplot-quality.sh  
```

![avatar](https://github.com/SoonyangZhang/http-dash-simu/blob/main/results/dash-pic-1.png)  
![avatar](https://github.com/SoonyangZhang/http-dash-simu/blob/main/results/dash-pic-2.png)  
