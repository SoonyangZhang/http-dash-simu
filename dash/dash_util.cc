#include <fstream>
#include <sstream>
#include "split.h"
#include "dash_util.h"
namespace basic{
void ReadSegmentFromFile(std::vector<std::string>& video_log,struct VideoData &video_data){
    for(auto it=video_log.begin();it!=video_log.end();it++){
        const char *name=it->c_str();
        SegmentSize segments;
        std::ifstream file;
        file.open(name);
        if(!file.is_open()){
            return;
        }
        int counter=0;
        int64_t sum_bytes=0;
        while(!file.eof()){
            std::string line;
            getline(file,line);
            /*std::istringstream iss(line);
            int n;
            while (iss >> n){
                segments.push_back(n);
            }*/            
            std::vector<std::string> array = split(line, "\n");
            if(array.size()>0){
                uint64_t size=atoi(array[0].c_str());
                segments.push_back(size);
                counter++;
                sum_bytes+=size;
            }
        }
        double average_bitrate=0.0;
        int64_t ms=video_data.segmentDuration;
        if(ms>0){
            average_bitrate=sum_bytes*1000*8*1.0/(ms*counter);
        }
        file.close();
        video_data.representation.push_back(segments);
        video_data.averageBitrate.push_back(average_bitrate);
    }
}    
}
