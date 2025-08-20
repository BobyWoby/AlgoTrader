#pragma once
#include <http.h>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <response.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <thread>


//Position: v2/positions/
//Account: v2/acccount/
//Orders: v2/orders/

// Base URL
// 33 chars
constexpr char APCA_API_BASE_URL[] = "https://paper-api.alpaca.markets/";
constexpr char LIVE_APCA_API_BASE_URL[] = "https://api.alpaca.markets/";
constexpr char APCA_DATA_BASE_URL[] = "https://data.alpaca.markets/";

typedef enum { BUY, SELL, HOLD } StockAction;
//
// Headers headers;
// std::string next_batch(const std::string next_page_token) {
//     if (next_page_token != "null") {
//         // add the next_page_token to the  request
//     }
//     std::string test_data, buf;
//     std::ifstream file("../test_data2.txt");
//     while (std::getline(file, buf)) {
//         test_data += buf;
//     }
//     return test_data;
// }
//
// float SMA(std::shared_ptr<std::vector<float>> closing_prices) {
//     float sum = 0;
//     for (auto price : *closing_prices) {
//         sum += price;
//     }
//     return sum / closing_prices->size();
// }
//
// // returns the percentage change of SPY from 40 days ago to now
// float index_data(const std::string start_date, const std::string end_date) {
//     std::string request = BAR_URL;
//     request += "SPY/bars?timeframe=1D&start=" + start_date +
//                "&end=" + end_date +
//                "&limit=1000&adjustment=raw&feed=iex&sort=desc";
//     Client c = Client(io, ctx);
//     Headers headers;
//     headers.headers["Apca-Api-Key-Id"] = APCA_KEY_ID;
//     headers.headers["Apca-Api-Secret-Key"] = APCA_SECRET_KEY;
//     headers.headers["Accept"] = "application/json";
//     Response response = c.fetch(request, "GET", headers);
//
//     if (response.status != StatusCode::OK) {
//         std::cout << "Reqeust: " << request << "\n";
//         std::cout << "Status Code: " << (int)response.status << "\n";
//         std::cerr << "Error fetching SPY: " << response.body << "\n";
//         return 0;
//     }
//     std::string jsondata = response.body;
//     rapidjson::Document tmp;
//     tmp.Parse(jsondata.c_str());
//     float past = tmp["bars"][0]["c"].GetFloat();
//
//     if (!tmp.IsObject()) {
//         std::cout << "idfk atp\n";
//         std::cout << escape_string(jsondata) << "\n";
//     }
//     Client c2 = Client(io, ctx);
//     request = BAR_URL;
//     request += "/v2/stocks/SPY/bars/latest";
//     response = c2.fetch(request, "GET", headers);
//     jsondata = response.body;
//     tmp.Parse(jsondata.c_str());
//     float present = tmp["bar"]["c"].GetFloat();
//     return present / past;
// }
//
// void fetch_data(std::vector<std::string>& res, const std::string symbol,
//                 Client& client, const std::string start_date,
//                 const std::string next_page_tok = "") {
//     // client.debug_mode = true;
//     std::string request = BAR_URL;
//     request += symbol + "/bars?timeframe=1H&start=" + start_date +
//                "&limit=1200&adjustment=raw&feed=iex&sort=desc";
//     if (next_page_tok != "") {
//         request += "&page_token=" + next_page_tok;
//     }
//     // std::cout << request << "\n";
//
//     Headers headers;
//     headers.headers["Apca-Api-Key-Id"] = APCA_KEY_ID;
//     headers.headers["Apca-Api-Secret-Key"] = APCA_SECRET_KEY;
//     headers.headers["Accept"] = "application/json";
//     // headers.headers["Accept-Encoding"] = "identity";
//
//     Response response = client.fetch(request, "GET", headers);
//
//     if (response.status != StatusCode::OK) {
//         std::cout << "Reqeust: " << request << "\n";
//         std::cout << "Status Code: " << (int)response.status << "\n";
//         std::cerr << "Error fetching " << symbol << ": " << response.body
//                   << "\n";
//         return;
//     }
//
//     // we need to  clean up the  response  cuz fu ig
//     std::string jsondata = response.body;
//     // std::cout << "JsonData: " << jsondata <<  "\n";
//
//     res.push_back(jsondata);
//     rapidjson::Document tmp;
//     tmp.Parse(jsondata.c_str());
//
//     if (!tmp.IsObject()) {
//         std::cout << "idfk atp\n";
//         std::cout << escape_string(jsondata) << "\n";
//     }
// }
// std::shared_ptr<std::vector<float>> day_closes(int days,
//                                                std::vector<std::string> data) {
//     std::shared_ptr<std::vector<float>> out =
//         std::make_shared<std::vector<float>>();
//
//     int day_changes = 0;
//     char day_tracker = ' ';
//
//     for (auto& file : data) {
//         rapidjson::Document doc;
//         doc.Parse(file.c_str());
//         if (doc["bars"].IsArray()) {
//             const rapidjson::Value& bars = doc["bars"];
//
//             for (rapidjson::SizeType i = 0; i < bars.Size(); i++) {
//                 // iterates through each bar
//                 if (!bars[i].IsObject() || !bars[i]["t"].IsString() ||
//                         !bars[i]["t"].IsString()) {
//                     std::cout << "i: " << i << ", file:  " << file << "\n";
//                 }
//                 const rapidjson::Value& tmp1 = bars[i];
//                 char day = tmp1["t"].GetString()[9];
//                 if (day != day_tracker) {
//                     out->push_back(bars[i]["c"].GetFloat());
//                     day_tracker = day;
//                     day_changes++;
//                 }
//                 if (day_changes >= days) {
//                     break;
//                 }
//             }
//         }
//     }
//     return out;
// }
//
// std::shared_ptr<std::vector<float>> closing_prices(
//     int days, std::vector<std::string> data) {
//     std::shared_ptr<std::vector<float>> out =
//         std::make_shared<std::vector<float>>();
//
//     int day_changes = 0;
//     char day_tracker = ' ';
//     for (auto& file : data) {
//         rapidjson::Document doc;
//         doc.Parse(file.c_str());
//         if (doc["bars"].IsArray()) {
//             const rapidjson::Value& bars = doc["bars"];
//
//             for (rapidjson::SizeType i = 0; i < bars.Size(); i++) {
//                 // iterates through each bar
//                 if (!bars[i].IsObject() || !bars[i]["t"].IsString() ||
//                     !bars[i]["t"].IsString()) {
//                     std::cout << "i: " << i << ", file:  " << file << "\n";
//                 }
//                 const rapidjson::Value& tmp1 = bars[i];
//                 char day = tmp1["t"].GetString()[9];
//                 if (day != day_tracker) {
//                     day_tracker = day;
//                     day_changes++;
//                 }
//                 if (day_changes >= days) {
//                     break;
//                 }
//                 out->push_back(bars[i]["c"].GetFloat());
//             }
//         }
//     }
//
//     return out;
// }
// std::string date_offset(int days_ago) {
//     std::string start_time;
//
//     char buf[1000];
//     using namespace std::chrono_literals;
//     auto now = std::chrono::system_clock::now();
//     auto duration = DAY * (days_ago * 1s);
//     std::time_t t_t = std::chrono::system_clock::to_time_t(now - duration);
//     std::tm* startinfo = std::localtime(&t_t);
//     std::strftime(buf, 1000, "%Y-%m-%d", startinfo);
//     start_time = std::string(buf) + "T00%3A00%3A00Z";
//     return start_time;
// }
//
// void logStock(std::string headline, std::string symbol, std::string message) {
//     std::chrono::system_clock::time_point now =
//         std::chrono::system_clock::now();
//     std::time_t now_c = std::chrono::system_clock::to_time_t(now);
//     std::tm now_tm = *std::localtime(&now_c);
//
//     char buf[100];
//     strftime(buf, 100, "%Y-%m-%d %H:%M:%S", &now_tm);
//     std::string formatted_time = buf;
//
//     std::string str = "------------" + headline + "------------\n";
//     str += "Timestamp: " + formatted_time + "\n";
//     str += "Symbol:  " + symbol + "\n";
//     str += message + "\n";
//
//     for (int i = 0; i < 24 + headline.length(); i++) {
//         str += "-";
//     }
//     str += "\n";
//
//     std::cout << str;
//     if (log_filepath != "") {
//         logfile << str;
//     }
// }
//
//
