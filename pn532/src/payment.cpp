#include <nfc/nfc.h>
#include <iostream>
#include <sstream>
#include <curl/curl.h>
//#include "json.hpp"   // Librería nlohmann/json

#include <nlohmann/json.hpp>


using json = nlohmann::json;

// Función callback para recibir respuesta de libcurl
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int main() {
    nfc_context *context;
    nfc_init(&context);
    if (!context) {
        std::cerr << "Error inicializando libnfc" << std::endl;
        return -1;
    }

    nfc_device *pnd = nfc_open(context, NULL);
    if (!pnd) {
        std::cerr << "No se pudo abrir el dispositivo NFC" << std::endl;
        nfc_exit(context);
        return -1;
    }

    if (nfc_initiator_init(pnd) < 0) {
        std::cerr << "Error iniciando modo lector" << std::endl;
        nfc_close(pnd);
        nfc_exit(context);
        return -1;
    }

    nfc_modulation nm;
    nm.nmt = NMT_ISO14443A;
    nm.nbr = NBR_106;

    std::cout << "Esperando tarjeta..." << std::endl;
    nfc_target nt;
    if (nfc_initiator_select_passive_target(pnd, nm, NULL, 0, &nt) > 0) {
        std::ostringstream uid;
        for (size_t i = 0; i < nt.nti.nai.szUidLen; i++) {
            uid << std::hex << (int)nt.nti.nai.abtUid[i];
        }

        std::string uid_str = uid.str();
        std::cout << "UID detectado: " << uid_str << std::endl;

        // Enviamos UID al servidor
        CURL *curl = curl_easy_init();
        if (curl) {
            std::string response;
            std::string json_data = "{\"uid\":\"" + uid_str + "\", \"amount\":10}";

            curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:5000/pay");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK)
                std::cerr << "Error en la request: " << curl_easy_strerror(res) << std::endl;
            else {
                // Parseamos JSON
                try {
                    auto j = json::parse(response);
                    std::string status = j["status"];
                    if (status == "ok") {
                        std::cout << "Pago aprobado. Saldo restante: " << j["remaining"] << std::endl;
                    } else {
                        std::cout << "Pago rechazado: " << j["message"] << std::endl;
                    }
                } catch (json::parse_error &e) {
                    std::cerr << "Error parseando JSON: " << e.what() << std::endl;
                }
            }

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
    }

    nfc_close(pnd);
    nfc_exit(context);
    return 0;
}
