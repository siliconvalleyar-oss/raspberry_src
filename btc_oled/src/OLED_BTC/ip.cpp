#include <iostream>
#include <string>
#include <curl/curl.h>
#include <ip_global.hpp>

namespace IP_GLOBAL{
    
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total_size = size * nmemb;
    output->append(static_cast<char*>(contents), total_size);
    return total_size;
}

std::string IpGlobal_t::obtenerIPGlobal() {
    CURL* curl;
    CURLcode res;
    std::string respuesta;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api64.ipify.org/");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respuesta);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return respuesta;
}

int IpGlobal_t::global() {
    try {
        std::string ipGlobal = obtenerIPGlobal();
        std::cout << "IP Global: " << ipGlobal << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}

}