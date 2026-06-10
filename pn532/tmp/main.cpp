#include <nfc/nfc.h>
#include <iostream>

int main() {
    nfc_device *pnd = nullptr;
    nfc_context *context;

    nfc_init(&context);
    if (context == nullptr) {
        std::cerr << "Error inicializando libnfc" << std::endl;
        return -1;
    }

    pnd = nfc_open(context, nullptr);
    if (pnd == nullptr) {
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

    std::cout << "Esperando tarjeta..." << std::endl;

    nfc_target nt;

    // Definimos la modulación: ISO14443A a 106 kbps
    const nfc_modulation nmMifare = {
        .nmt = NMT_ISO14443A,
        .nbr = NBR_106,
    };

    // Pasamos el puntero a nmMifare en lugar de nullptr
    if (nfc_initiator_select_passive_target(pnd, nmMifare, nullptr, 0, &nt) > 0) {
        std::cout << "UID detectado: ";
        for (size_t i = 0; i < nt.nti.nai.szUidLen; i++) {
            printf("%02x", nt.nti.nai.abtUid[i]);
        }
        std::cout << std::endl;
    } else {
        std::cerr << "No se detectó ninguna tarjeta" << std::endl;
    }

    nfc_close(pnd);
    nfc_exit(context);
    return 0;
}
