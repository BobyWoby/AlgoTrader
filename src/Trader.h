#pragma once

#include <http.h>

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
#include <vector>

#include "helpers.h"

class Trader {
   private:
    std::string api_id, secret_key, base_url;
    std::vector<std::string> m_symbols;  // symbols to trade
    Headers headers;
    boost::asio::io_context io;
    boost::asio::ssl::context ctx;

    std::vector<float> parseCloses(std::vector<std::string> data);
    std::vector<std::string> fetchData(const std::string symbol,
                                       const std::string start_date,
                                       const std::string next_page_tok = "");
    std::shared_ptr<std::vector<float>> day_closes(
        int days, std::vector<std::string> data);

   public:
    Trader(std::string id, std::string key, bool live,
           std::vector<std::string> symbols);

    float RSI(std::vector<std::string> data, std::string symbol, int period);
    float SMA(std::vector<std::string> data, std::string symbol);

    StockAction evalStock(std::string symbol);
    void trade();
};
