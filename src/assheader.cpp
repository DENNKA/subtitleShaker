#include "assheader.h"

ASSHeader::ASSHeader(){
    format.resize(FormatEnd);
}

void ASSHeader::setHeader(std::vector<std::string>& header, std::string version){
    this->header = header;
    bool findedSubtitleShaker = false;
    for (int i = 0; i < header.size(); i++){
        auto str = header[i];
        /*if (str.c
            auto elems = split(str, ' ');
            playResX = std::stoi(elems[1]);
            auto elems = split(str, ' ');
            playResY = std::stoi(elems[1]);
        }*/
        #define ifCompare(x) if (str.compare(0, x.size(), x) == 0)
        #define strErase(x) str.erase(str.begin(), str.begin() + x.size() + 1)
        ifCompare(textPlayResX){
            format[PlayResX] = operation.split(str, ' ')[1];
        }
        ifCompare(textPlayResY){
            format[PlayResY] = operation.split(str, ' ')[1];
        }
        ifCompare(textPlayResY){
            format[PlayResY] = operation.split(str, ' ')[1];
        }
        ifCompare(textSubtitleShaker){
            findedSubtitleShaker = true;
        }
    }
    if (!findedSubtitleShaker){
        this->header.push_back(textSubtitleShaker);
        this->header.push_back("Version: " + version);
        this->header.push_back("Link: github.com/DENNKA/SubtitleShaker.git");
        this->header.push_back("");
    }
}

const std::vector<std::string>& ASSHeader::getHeader(){return header;};

ASSHeader::~ASSHeader()
{
    //dtor
}
