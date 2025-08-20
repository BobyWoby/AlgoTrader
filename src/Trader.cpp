#include "Trader.h"

Trader::Trader(std::string id, std::string key, bool live,
               std::vector<std::string> symbols)
    : ctx(boost::asio::ssl::context::sslv23), m_symbols(std::move(symbols)){
    base_url = (live) ? LIVE_APCA_API_BASE_URL : APCA_API_BASE_URL;
    api_id = id;
    secret_key = key;
}

std::vector<std::string> Trader::fetchData(const std::string symbol,
                                        const std::string start_date,
                                        const std::string next_page_tok){
    Client client(io, ctx);
    std::vector<std::string> res;
    std::string request = APCA_DATA_BASE_URL;
    request += "stocks/";
    request += symbol + "/bars?timeframe=1H&start=" + start_date +
               "&limit=1200&adjustment=raw&feed=iex&sort=desc";
    if (next_page_tok != "") {
        request += "&page_token=" + next_page_tok;
    }

    Response response = client.fetch(request, "GET", headers);

    if (response.status != StatusCode::OK) {
        std::cout << "Reqeust: " << request << "\n";
        std::cout << "Status Code: " << (int)response.status << "\n";
        std::cerr << "Error fetching " << symbol << ": " << response.body
                  << "\n";
        return res;
    }

    // we need to  clean up the  response  cuz fu ig
    std::string jsondata = response.body;

    res.push_back(jsondata);
    rapidjson::Document tmp;
    tmp.Parse(jsondata.c_str());

    if (!tmp.IsObject()) {
        std::cout << "idfk atp\n";
        std::cout << escape_string(jsondata) << "\n";
    }

    return res;
}

std::shared_ptr<std::vector<float>> Trader::day_closes(int days,
                                               std::vector<std::string> data) {
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
                    out->push_back(bars[i]["c"].GetFloat());
                    day_tracker = day;
                    day_changes++;
                }
                if (day_changes >= days) {
                    break;
                }
            }
        }
    }
    return out;
}
