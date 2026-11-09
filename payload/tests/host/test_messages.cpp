#include "services/messages.hpp"
#include "test.hpp"

using namespace playtrack;

TEST(notifications_follow_the_console_language) {
    CHECK_EQ(messages::Text(messages::Id::Started, "pt-BR", 7777), std::string("PlayTrack ativo na porta 7777"));
    CHECK_EQ(messages::Text(messages::Id::Started, "pt-PT", 7777), std::string("PlayTrack ativo na porta 7777"));
    CHECK_EQ(messages::Text(messages::Id::AlreadyRunning, "es-419", 7777), std::string("PlayTrack ya se está ejecutando"));
    CHECK_EQ(messages::Text(messages::Id::TileInstalled, "en-GB", 7777), std::string("PlayTrack added to the Media tab"));
    CHECK_EQ(messages::Text(messages::Id::PortBusy, "ja-JP", 7778), std::string("PlayTrack: port 7778 is used by another app"));
}
