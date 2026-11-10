#include "services/messages.hpp"

#include <cstdio>

namespace openpsapp::messages {

namespace {
struct Texts {
    const char *started;
    const char *already_running;
    const char *port_busy;
    const char *tile_installed;
};

constexpr Texts kPortuguese = {
    "OpenPSApp ativo na porta %d",
    "OpenPSApp já está rodando",
    "OpenPSApp: a porta %d está em uso por outro app",
    "OpenPSApp instalado na aba Mídia",
};
constexpr Texts kSpanish = {
    "OpenPSApp activo en el puerto %d",
    "OpenPSApp ya se está ejecutando",
    "OpenPSApp: el puerto %d está en uso por otra app",
    "OpenPSApp añadido a la pestaña Multimedia",
};
constexpr Texts kEnglish = {
    "OpenPSApp running on port %d",
    "OpenPSApp is already running",
    "OpenPSApp: port %d is used by another app",
    "OpenPSApp added to the Media tab",
};

const Texts &For(const std::string &language) {
    if (language.rfind("pt", 0) == 0) return kPortuguese;
    if (language.rfind("es", 0) == 0) return kSpanish;
    return kEnglish;
}
}

std::string Text(Id id, const std::string &language, int port) {
    const Texts &t = For(language);
    const char *format = id == Id::Started          ? t.started
                         : id == Id::AlreadyRunning ? t.already_running
                         : id == Id::PortBusy       ? t.port_busy
                                                    : t.tile_installed;
    char buf[160];
    std::snprintf(buf, sizeof(buf), format, port);
    return buf;
}

}
