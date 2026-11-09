#include "services/messages.hpp"

#include <cstdio>

namespace playtrack::messages {

namespace {
struct Texts {
    const char *started;
    const char *already_running;
    const char *port_busy;
    const char *tile_installed;
};

constexpr Texts kPortuguese = {
    "PlayTrack ativo na porta %d",
    "PlayTrack já está rodando",
    "PlayTrack: a porta %d está em uso por outro app",
    "PlayTrack instalado na aba Mídia",
};
constexpr Texts kSpanish = {
    "PlayTrack activo en el puerto %d",
    "PlayTrack ya se está ejecutando",
    "PlayTrack: el puerto %d está en uso por otra app",
    "PlayTrack añadido a la pestaña Multimedia",
};
constexpr Texts kEnglish = {
    "PlayTrack running on port %d",
    "PlayTrack is already running",
    "PlayTrack: port %d is used by another app",
    "PlayTrack added to the Media tab",
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
