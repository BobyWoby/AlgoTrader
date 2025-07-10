#include <chrono>
#include <ctime>
#include <iostream>
#include <bits/chrono.h>
#include <common.h>
#include <http.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/rapidjson.h>
#include <fstream>
#include <string>
#define DAY 86400

std::string APCA_KEY_ID;
std::string APCA_SECRET_KEY;

int main(){
    std::ifstream file("../.env");
    std::string line; 
    while(std::getline(file, line)){
        auto field  =  split(line, " ");
        if(field.at(0) == "APCA_KEY_ID"){
            APCA_KEY_ID = field.at(1);
        }else if(field.at(0) == "APCA_SECRET_KEY"){
            APCA_SECRET_KEY = field.at(1);
        }
    }
    if(APCA_KEY_ID ==  "" || APCA_SECRET_KEY == ""){
        std::cerr << "Couldn't find Alpaca Key pair!\n";
        return 1;
    }
    std::string start_time;
    {
        char buf[1000];
        using namespace std::chrono_literals;
        auto now = std::chrono::system_clock::now();
        auto  duration= DAY * 50s;
        std::time_t t_t = std::chrono::system_clock::to_time_t(now - duration);
        std::tm *startinfo = std::localtime(&t_t);
        std::strftime(buf, 1000, "%Y-%m-%d", startinfo);
        start_time = std::string(buf) + "T00%3A00%3A00Z";
    }

    std::cout << APCA_KEY_ID <<  "\n";
    std::cout << APCA_SECRET_KEY <<  "\n";
    std::cout << start_time <<  "\n";


    std::string APCA_API_BASE_URL = "https://paper-api.alpaca.markets";
    return 0;
}
