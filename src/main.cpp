#include <bits/chrono.h>
#include <common.h>
#include <http.h>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/writer.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <chrono>
#include <cinttypes>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#define DAY 86400

// Base URL
// 33 chars
constexpr char APCA_API_BASE_URL[] = "https://paper-api.alpaca.markets";

// POST  Request  with data in body
// 43 chars
constexpr char ORDER_URL[] = "https://paper-api.alpaca.markets/v2/orders";

// GET Request with data in url(append /${stocks}/bars)
// 43 chars
constexpr char BAR_URL[] = "https://paper-api.alpaca.markets/v2/stocks";

std::string APCA_KEY_ID;
std::string APCA_SECRET_KEY;

boost::asio::io_context io;
boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);

std::string next_batch(const std::string next_page_token){
    if(next_page_token != "null"){
        // add the next_page_token to the  request
    }
    std::string test_data, buf;
    std::ifstream file("../test_data2.txt");
    while (std::getline(file, buf)) {
        test_data += buf;
    }
    return  test_data;
}

std::vector<float> closing_prices(int days) {
    std::vector<float> out;
    std::string test_data, buf;
    std::ifstream file("../test_data.txt");
    while (std::getline(file, buf)) {
        test_data += buf;
    }
    std::vector<std::string> responses;
    responses.push_back(test_data);
    rapidjson::Document tmp;
    tmp.Parse(test_data.c_str());
    if (strcmp(tmp["next_page_token"].GetString(), "null")) {
        std::string next_page_token(tmp["next_page_token"].GetString());
        std::cout << "theres  a next  page\n";
        bool breakpoint = true;
        while(true){
            // keep processing
            std::string data = next_batch(next_page_token);
            responses.push_back(data);

            rapidjson::Document tmp2;
            tmp2.Parse(data.c_str());
            if(tmp2["next_page_token"].IsNull()){
                break;
            }
            if(!strcmp(tmp2["next_page_token"].GetString(), "null")){
                break;
            }
        }
    }


    int day_changes = 0;
    char day_tracker = ' ';
    for(auto &file : responses){
        rapidjson::Document doc;
        doc.Parse(file.c_str());
        if (doc["bars"].IsArray()) {
            const rapidjson::Value& bars = doc["bars"];
            for (rapidjson::SizeType i = 0; i < bars.Size(); i++) {
                // iterates through each bar
                char day = bars[i]["t"].GetString()[9];
                if (day != day_tracker) {
                    day_tracker = day;
                    day_changes++;
                }
                if (day_changes >= days) {
                    break;
                }
                out.push_back(bars[i]["c"].GetFloat());
            }
            if (day_changes < days) {
                // call it again we know we only need two
            }
        }
    }

    return out;
}

bool eval(std::string symbol) {
    Client c(io, ctx);
    std::string start_time;
    {
        char buf[1000];
        using namespace std::chrono_literals;
        auto now = std::chrono::system_clock::now();
        auto duration = DAY * 50s;
        std::time_t t_t = std::chrono::system_clock::to_time_t(now - duration);
        std::tm* startinfo = std::localtime(&t_t);
        std::strftime(buf, 1000, "%Y-%m-%d", startinfo);
        start_time = std::string(buf) + "T00%3A00%3A00Z";
    }

    return false;
}

void  setup(){
    std::ifstream file("../.env");
    std::string line;
    while (std::getline(file, line)) {
        auto field = split(line, " ");
        if (field.at(0) == "APCA_KEY_ID") {
            APCA_KEY_ID = field.at(1);
        } else if (field.at(0) == "APCA_SECRET_KEY") {
            APCA_SECRET_KEY = field.at(1);
        }
    }
    if (APCA_KEY_ID == "" || APCA_SECRET_KEY == "") {
        std::cerr << "Couldn't find Alpaca Key pair!\n";
        exit(1);
    }
    ctx.load_verify_file("/etc/ssl/cert.pem");
    ctx.set_default_verify_paths();

}

int main() {
    setup();
    std::cout << APCA_KEY_ID << "\n";
    std::cout << APCA_SECRET_KEY << "\n";


    auto closeList = closing_prices(50);
    for (auto closing : closeList) {
        std::cout << closing << "\n";
    }
    std::cout <<  "Now running  20 days\n";

    closeList = closing_prices(20);
    for (auto closing : closeList) {
        std::cout << closing << "\n";
    }
    return 0;
}
