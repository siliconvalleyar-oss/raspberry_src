#include <iostream>
#include <sstream>
#include <iomanip>
#include <curl/curl.h>
#include <json.hpp>  // nlohmann::json

#include <btc.hpp>

namespace BTC {

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total_size = size * nmemb;
    output->append((char*)contents, total_size);
    return total_size;
}

void Btc_t::function(std::string& name) {
    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        const char* url = "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin,ethereum&vs_currencies=usd,usdt";
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "Error al realizar la solicitud HTTP: " << curl_easy_strerror(res) << std::endl;
        } else {
            // Parsear con nlohmann::json
            auto root = nlohmann::json::parse(response);

            double bitcoinPriceUSD = root["bitcoin"]["usd"];
            double ethereumPriceUSD = root["ethereum"]["usd"];
            double bitcoinPriceUSDT = root["bitcoin"]["usdt"];

            std::ostringstream stream;
            stream << std::fixed << std::setprecision(0) << bitcoinPriceUSD;
            name = "BTC " + stream.str();

            std::cout << "Precio del Bitcoin (USD): " << bitcoinPriceUSD << std::endl;
            std::cout << "Precio de Ethereum (USD): " << ethereumPriceUSD << std::endl;
            std::cout << "Precio del Bitcoin (USDT): " << bitcoinPriceUSDT << std::endl;
        }

        curl_easy_cleanup(curl);
        curl_global_cleanup();
    }

    return;
}

}
