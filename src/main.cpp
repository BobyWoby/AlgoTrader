#include <bits/chrono.h>
#include <common.h>
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
#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <map>
#include <string>
#include <thread>

#define DAY 86400

// Base URL
// 33 chars
constexpr char APCA_API_BASE_URL[] = "https://paper-api.alpaca.markets/";
constexpr char LIVE_APCA_API_BASE_URL[] = "https://api.alpaca.markets/";

// POST  Request  with data in body
// 43 chars
constexpr char ORDER_URL[] = "https://paper-api.alpaca.markets/v2/orders";
constexpr char LIVE_ORDER_URL[] = "https://api.alpaca.markets/v2/orders";

// GET Request with data in url(append /${stocks}/bars)
// 43 chars
constexpr char BAR_URL[] = "https://data.alpaca.markets/v2/stocks/";

// GET Request, nothing else  needed
constexpr char ACCT_URL[] = "https://api.alpaca.markets/v2/account";
constexpr char LIVE_ACCT_URL[] = "https://api.alpaca.markets/v2/account";

// GET Request, just add the symbol at the end, sell with a DELETE request
constexpr char POSITION_URL[] =
    "https://paper-api.alpaca.markets/v2/positions/";
constexpr char LIVE_POSITION_URL[] =
    "https://api.alpaca.markets/v2/positions/";


// https://data.alpaca.markets
std::string APCA_KEY_ID;
std::string APCA_SECRET_KEY;
std::string log_filepath;
std::vector<std::string> symbols{"AAPL", "MSFT", "SPY"};
std::map<std::string, bool> bought_symbols;

boost::asio::io_context io;
boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);

typedef enum { BUY, SELL, HOLD } StockAction;
std::ofstream logfile;
bool live=false;


Headers headers;
std::string next_batch(const std::string next_page_token) {
    if (next_page_token != "null") {
        // add the next_page_token to the  request
    }
    std::string test_data, buf;
    std::ifstream file("../test_data2.txt");
    while (std::getline(file, buf)) {
        test_data += buf;
    }
    return test_data;
}

float SMA(std::shared_ptr<std::vector<float>> closing_prices) {
    float sum = 0;
    for (auto price : *closing_prices) {
        sum += price;
    }
    return sum / closing_prices->size();
}
void fetch_data(std::vector<std::string>& res, const std::string symbol,
                Client& client, const std::string start_date,
                const std::string next_page_tok = "") {
    // client.debug_mode = true;
    std::string request = BAR_URL;
    request += symbol + "/bars?timeframe=1H&start=" + start_date +
               "&limit=1200&adjustment=raw&feed=iex&sort=desc";
    if (next_page_tok != "") {
        request += "&page_token=" + next_page_tok;
    }
    // std::cout << request << "\n";

    Headers headers;
    headers.headers["Apca-Api-Key-Id"] = APCA_KEY_ID;
    headers.headers["Apca-Api-Secret-Key"] = APCA_SECRET_KEY;
    headers.headers["Accept"] = "application/json";
    // headers.headers["Accept-Encoding"] = "identity";

    Response response = client.fetch(request, "GET", headers);

    if (response.status != StatusCode::OK) {
        std::cout << "Reqeust: " << request << "\n";
        std::cout << "Status Code: " << (int)response.status << "\n";
        std::cerr << "Error fetching " << symbol << ": " << response.body
                  << "\n";
        return;
    }

    // we need to  clean up the  response  cuz fu ig
    std::string jsondata = response.body;
    // std::cout << "JsonData: " << jsondata <<  "\n";

    res.push_back(jsondata);
    rapidjson::Document tmp;
    tmp.Parse(jsondata.c_str());

    if (!tmp.IsObject()) {
        std::cout << "idfk atp\n";
        std::cout << escape_string(jsondata) << "\n";
    }
}

std::shared_ptr<std::vector<float>> closing_prices(
    int days, std::vector<std::string> data) {
    std::shared_ptr<std::vector<float>> out =
        std::make_shared<std::vector<float>>();

    int day_changes = 0;
    char day_tracker = ' ';
    for (auto& file : data) {
        rapidjson::Document doc;
        doc.Parse(file.c_str());
        if (doc["bars"].IsArray()) {
            const rapidjson::Value& bars = doc["bars"];

            for (rapidjson::SizeType i = 0; i < bars.Size(); i++) {
                // iterates through each bar
                if (!bars[i].IsObject() || !bars[i]["t"].IsString() ||
                    !bars[i]["t"].IsString()) {
                    std::cout << "i: " << i << ", file:  " << file << "\n";
                }
                const rapidjson::Value& tmp1 = bars[i];
                char day = tmp1["t"].GetString()[9];
                if (day != day_tracker) {
                    day_tracker = day;
                    day_changes++;
                }
                if (day_changes >= days) {
                    break;
                }
                out->push_back(bars[i]["c"].GetFloat());
            }
        }
    }

    return out;
}
std::string date_offset(int days_ago) {
    std::string start_time;

    char buf[1000];
    using namespace std::chrono_literals;
    auto now = std::chrono::system_clock::now();
    auto duration = DAY * (days_ago * 1s);
    std::time_t t_t = std::chrono::system_clock::to_time_t(now - duration);
    std::tm* startinfo = std::localtime(&t_t);
    std::strftime(buf, 1000, "%Y-%m-%d", startinfo);
    start_time = std::string(buf) + "T00%3A00%3A00Z";
    return start_time;
}

void logStock(std::string headline, std::string symbol, std::string message) {
    std::chrono::system_clock::time_point now =
        std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_c);

    char buf[100];
    strftime(buf, 100, "%Y-%m-%d %H:%M:%S", &now_tm);
    std::string formatted_time = buf;

    std::string str = "------------" + headline + "------------\n";
    str += "Timestamp: " + formatted_time + "\n";
    str += "Symbol:  " + symbol + "\n";
    str += message + "\n";

    for (int i = 0; i < 24 + headline.length(); i++) {
        str += "-";
    }
    str += "\n";

    std::cout << str;
    if (log_filepath != "") {
        logfile << str;
    }
}

// TODO: maybe add in a majority vote with some other indicators like RSI and
// MACD Also available could be parabolic SAR, or generally just stuff from the
// trend section of the wikipedia page for technical indicators
StockAction eval(std::string symbol) {
    Client c(io, ctx);
    std::vector<std::string> data;
    auto forty_days_ago = date_offset(40);
    fetch_data(data, symbol, c, forty_days_ago);

    float forty_SMA = SMA(closing_prices(40, data));
    float twenty_SMA = SMA(closing_prices(20, data));

    std::string s_forty = std::to_string(forty_SMA);

    std::string s_twenty = std::to_string(twenty_SMA);

    std::string str = "40 day SMA: " + s_forty +
                      "\n"
                      "20 day SMA: " +
                      s_twenty;
    logStock("Checking SMA", symbol, str);

    Client cli(io, ctx);
    if (twenty_SMA > forty_SMA && !bought_symbols[symbol]) {
        std::cout << "Running buy block\n";
        // when the short term  initially crosses  over the  long term, we  want
        // to buy the stock, as it will likely be an upward trend

        Response res = cli.fetch(POSITION_URL + symbol, "GET",
                                 headers);  // get the position
        rapidjson::Document tmp;
        if (res.status == StatusCode::NOT_FOUND) {
            bought_symbols[symbol] = true;
            std::cout << "Finished buy block(buying)\n";
            return BUY;
        }
        // size-to-buy = cash / (price / some_constant) [some_constant ?= 0.25]
        // size-to-buy should be a quantity

        std::cout << "Finished buy block (already owned)\n";
        return HOLD;
    } else if (forty_SMA > twenty_SMA) {
        std::cout << "Running sell block\n";
        // when the short term  initially crosses under the  long term, we  want
        // to sell the stock, as it will likely be an downward trend
        return SELL;
    }
    return HOLD;
}

void setup(std::string env) {
    std::ifstream file(env);
    std::string line;
    bool overwrite = false;
    while (std::getline(file, line)) {
        if(line.at(0) == '#') continue;
        auto field = split(line, " ");
        if (field.at(0) == "APCA_KEY_ID") {
            APCA_KEY_ID = field.at(1);
        } else if (field.at(0) == "APCA_SECRET_KEY") {
            APCA_SECRET_KEY = field.at(1);
        } else if (field.at(0) == "LOGFILE") {
            log_filepath = field.at(1);
        } else if (field.at(0) == "OVERWRITE") {
            overwrite = true;
        } else if (field.at(0) == "SYMBOL") {
            symbols.push_back(field.at(1));
        } else if(field.at(0) == "LIVE"){
            live = true;
        }
    }
    if (APCA_KEY_ID == "" || APCA_SECRET_KEY == "") {
        std::cerr << "Couldn't find Alpaca Key pair!\n";
        exit(1);
    }
    headers.headers["Apca-Api-Key-Id"] = APCA_KEY_ID;
    headers.headers["Apca-Api-Secret-Key"] = APCA_SECRET_KEY;
    headers.headers["Accept"] = "application/json";

    if (overwrite && log_filepath != "") {
        logfile = std::ofstream(log_filepath);
        logfile << "";
    } else if (log_filepath != "") {
        logfile = std::ofstream(log_filepath, std::ios::app);
    }
    ctx.load_verify_file("/etc/ssl/cert.pem");
    ctx.set_default_verify_paths();
    for (auto symbol : symbols) {
        bought_symbols[symbol] = false;
    }
}

void perform_action(
    const std::vector<std::pair<std::string, StockAction>>& symbol_buf) {
    std::cout << "action list size: " << symbol_buf.size() << "\n";
    Client acct_client(io, ctx);
    std::string url = live ? LIVE_ACCT_URL : ACCT_URL;
    Response acct = acct_client.fetch(url, "GET", headers);
    rapidjson::Document acct_json;
    acct_json.Parse(acct.body.c_str());
    float cash =
        std::stof(acct_json["non_marginable_buying_power"].GetString());

    std::string buy_list;
    std::cout << "cash: " << cash << "\n";

    // size-to-buy = cash / (price / some_constant)
    // [some_constant ?= 1.0 / symbols.size()]
    // size-to-buy should be a quantity(shares)

    for (auto symbol : symbol_buf) {
        if (symbol.second == BUY) {
            if (buy_list != "") {
                buy_list += ",";
            }
            buy_list += symbol.first;
        } else if (symbol.second == SELL) {
            logStock("Sell Report", symbol.first, "");
            bought_symbols[symbol.first] = false;
            // close the position
            Client sell_client(io, ctx);
            std::string url = live ? LIVE_POSITION_URL : POSITION_URL;
            sell_client.fetch(url + symbol.first, "DELETE", headers);
        }
    }
    // buy all  symbols needed here
    rapidjson::Document buy_info;
    buy_info.SetObject();
    rapidjson::Document::AllocatorType& allocator = buy_info.GetAllocator();

    buy_info.AddMember("type", "market", allocator);
    buy_info.AddMember("time_in_force", "day", allocator);
    buy_info.AddMember("qty", 0, allocator);
    buy_info.AddMember("symbol", "", allocator);
    buy_info.AddMember("side", "buy", allocator);
    Headers buy_headers = headers;

    float positionSizing = 1.0 / symbols.size();
    rapidjson::Document stock_info;
    for (auto sym : split(buy_list, ",")) {
        Client sym_info_client(io, ctx);
        Response sym_info = sym_info_client.fetch(
            BAR_URL + sym + "/quotes/latest", "GET", headers);
        stock_info.Parse(sym_info.body.c_str());

        if (stock_info.IsObject()) {
            if (stock_info.HasMember("quote") &&
                stock_info["quote"].HasMember("ap") &&
                stock_info["quote"]["ap"].GetFloat() != 0) {
                float shares = cash / (stock_info["quote"]["ap"].GetFloat() /
                                       positionSizing);

                rapidjson::Value& qty = buy_info["qty"];
                qty.SetFloat(shares);
                rapidjson::Value& s = buy_info["symbol"];
                s.SetString(sym.c_str(), allocator);

                // Convert JSON document to string
                rapidjson::StringBuffer strbuf;
                rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
                buy_info.Accept(writer);

                std::cout << strbuf.GetString() << "\n";
                buy_headers.headers["Content-Length"] =
                    std::to_string(strbuf.GetLength());

                Client buy_client(io, ctx);
                // buy_client.debug_mode = true;
                std::string url = live? LIVE_ORDER_URL : ORDER_URL;
                Response buy_res = buy_client.fetch(
                    url, "POST", buy_headers, strbuf.GetString());
                if (buy_res.status == StatusCode::OK) {
                    bought_symbols[sym] = true;
                    logStock("Buy Report", sym, "");
                    cash -= shares * stock_info["quote"]["ap"].GetFloat();
                } else {
                    std::cout << buy_res.body << "\n";
                }
            }
        }
    }
}

std::time_t parseISO(std::string iso_str) {
    std::time_t out;
    struct std::tm tm;
    std::istringstream ss(iso_str.substr(0, 19).c_str());
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (ss.fail()) {
        std::cerr << "Failed to parse string: " << iso_str.substr(0, 19)
                  << "\n";
    }

    out = timegm(&tm);

    std::string offset = iso_str.substr(19);
    int offset_sign = (offset[0] == '+') ? 1 : -1;
    int offset_hours = std::stoi(offset.substr(1, 2));
    int offset_minutes = std::stoi(offset.substr(4, 2));
    int offset_seconds =
        offset_sign * (offset_hours * 3600 + offset_minutes * 60);

    out -= offset_seconds;
    return out;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "You need to input an .env file!";
        setup(".poorenv");
        // return 1;
    } else {
        setup(argv[1]);
    }
    auto now = std::chrono::system_clock::now();
    auto last_min = now;
    using namespace std::chrono_literals;

    // amount of time  to wait between ticks
    std::chrono::seconds wait_time = 5s;

    int check_market_counter = 0;
    std::vector<std::pair<std::string, StockAction>> stock_list;
    stock_list.reserve(symbols.size());
    bool is_open = false;

    struct std::tm tm;
    std::time_t next_close;

    Client market_clock_client(io, ctx);
    while (true) {
        std::cout << "tick" << "\n";
        {
            now = std::chrono::system_clock::now();
            auto time_since_last_min = now - last_min;
            if (time_since_last_min < wait_time) {
                std::this_thread::sleep_for(wait_time - time_since_last_min);
                continue;
            }
        }
        if (!is_open) {
            // market_clock_client.debug_mode = true;
            Response clock_res = market_clock_client.fetch(
                "https://paper-api.alpaca.markets/v2/clock", "GET", headers);
            if (clock_res.status != StatusCode::OK) {
                continue;
            }
            rapidjson::Document json;
            json.Parse(clock_res.body.c_str());
            if (!json.IsObject()) {
                continue;
            }
            is_open = json["is_open"].GetBool();
            std::string time_str = json["next_close"].GetString();

            next_close = parseISO(time_str);

            auto now = std::chrono::system_clock::now();
            std::time_t now_t = std::chrono::system_clock::to_time_t(now);
            std::tm* now_tm = std::localtime(&now_t);
            char now_buffer[] = "2025-07-14 16:00:00";
            std::strftime(now_buffer, 32, "%F %T", now_tm);

            std::tm* close_tm = std::localtime(&next_close);
            char buffer[] = "2025-07-14 16:00:00";
            std::strftime(buffer, 32, "%F %T", close_tm);

            int diff = std::difftime(std::chrono::system_clock::to_time_t(
                                         std::chrono::system_clock::now()),
                                     next_close);
            std::cout << diff << "\n";
            std::cout << "Now: " << now_buffer << "\n";
            std::cout << "Market Status: " << std::to_string(is_open) << "\n";
            std::cout << "next close: " << buffer << "\n";
            std::cout << "next close str: " << time_str << "\n";
            std::cout << "next open: " << json["next_open"].GetString()
                      << "\n\n";

            auto next_open = parseISO(json["next_open"].GetString());
            int sleep_for =
                std::difftime(next_open, std::chrono::system_clock::to_time_t(
                                             std::chrono::system_clock::now()));
            if (!is_open) {
                std::cout << "Sleeping for: " << sleep_for << "\n";
                std::this_thread::sleep_for(sleep_for * 1s);
            }

        } else {
            if (std::difftime(std::chrono::system_clock::to_time_t(
                                  std::chrono::system_clock::now()),
                              next_close) > 0) {
                is_open = false;
                continue;
            }
            for (auto symbol : symbols) {
                StockAction action = eval(symbol);
                if (action != HOLD) {
                    // add the symbol to our buy/sell list
                    stock_list.push_back({symbol, action});
                }
            }
            if (stock_list.size() > 0) {
                perform_action(stock_list);
                stock_list.clear();
            }
        }

        last_min = now;
    }
    return 0;
}
