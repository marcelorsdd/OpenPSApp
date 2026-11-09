#include "platform/platform.hpp"

#include <sys/syscall.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>
#include <memory>

#include <ps5/kernel.h>

#include "util/thread.hpp"

extern "C" {
int sceUserServiceInitialize(void *params);
int sceUserServiceGetForegroundUser(int32_t *user_id);
int sceUserServiceGetUserName(int32_t user_id, char *name, size_t max_size);

struct NotifyRequest {
    char reserved[45];
    char message[3075];
};
int sceKernelSendNotificationRequest(int device, NotifyRequest *req, size_t size, int blocking);

struct PkgInfo {
    char content_id[0x30];
    int content_type;
    int content_platform;
};
struct PkgMetaInfo {
    const char *uri;
    const char *ex_uri;
    const char *playgo_scenario_id;
    const char *content_id;
    const char *content_name;
    const char *icon_url;
};
struct PkgPlayGoInfo {
    char languages[30][8];
    char playgo_scenario_ids[64][3];
    char content_ids[64][0x30];
    long unknown[810];
};
int sceSystemServiceParamGetInt(int param_id, int *value);
int sceRegMgrGetStr(int entity, char *value, size_t size);
int sceRegMgrGetBin(int entity, void *value, size_t size);
int sceKernelGetOpenPsId(void *out);
int sceAppInstUtilInitialize(void);
int sceAppInstUtilAppInstallPkg(const char *path, PkgInfo *info);
int sceAppInstUtilInstallByPackage(PkgMetaInfo *meta, PkgInfo *info, PkgPlayGoInfo *playgo);
}

namespace playtrack::platform {
namespace {
constexpr uint64_t kSystemAuthId = 0x4801000000000013ULL;
}

const char *Name() { return "ps5"; }

bool Escalate(std::string *err) {
    pid_t pid = getpid();
    if (!kernel_get_proc(pid)) {
        if (err) *err = "kernel_get_proc failed: kernel R/W not available";
        return false;
    }

    bool ok = true;
    ok = kernel_set_ucred_uid(pid, 0) == 0 && ok;
    ok = kernel_set_ucred_ruid(pid, 0) == 0 && ok;
    ok = kernel_set_ucred_svuid(pid, 0) == 0 && ok;
    ok = kernel_set_ucred_rgid(pid, 0) == 0 && ok;
    ok = kernel_set_ucred_svgid(pid, 0) == 0 && ok;

    if (intptr_t root_vnode = kernel_get_root_vnode()) {
        ok = kernel_set_proc_rootdir(pid, root_vnode) == 0 && ok;
        ok = kernel_set_proc_jaildir(pid, root_vnode) == 0 && ok;
    }

    ok = kernel_set_ucred_authid(pid, kSystemAuthId) == 0 && ok;

    uint8_t caps[16];
    std::memset(caps, 0xff, sizeof(caps));
    ok = kernel_set_ucred_caps(pid, caps) == 0 && ok;

    uint8_t attrs[32] = {};
    attrs[0] = 0x80;
    ok = kernel_set_ucred_attrs(pid, attrs) == 0 && ok;

    if (!ok && err) *err = "one or more kernel_set_ucred_* calls failed";
    return ok;
}

void SetProcessName(const char *name) { syscall(SYS_thr_set_name, -1, name); }

const std::string &Root() {
    static const std::string root;
    return root;
}

std::optional<User> ForegroundUser() {
    sceUserServiceInitialize(nullptr);
    int32_t id = 0;
    if (sceUserServiceGetForegroundUser(&id) != 0 || id <= 0) return std::nullopt;

    User user;
    user.id = id;
    char hex[16];
    std::snprintf(hex, sizeof(hex), "%08x", static_cast<uint32_t>(id));
    user.id_hex = hex;
    char name[64] = {};
    if (sceUserServiceGetUserName(id, name, sizeof(name)) == 0) user.name = name;
    return user;
}

int SystemLanguageId() {
    constexpr int kParamLanguage = 1;
    int value = -1;
    return sceSystemServiceParamGetInt(kParamLanguage, &value) == 0 ? value : -1;
}

namespace {
constexpr int kMaxUserSlots = 16;
constexpr int kRegUserName = 125829632;
constexpr int kRegAccountId = 125830400;
constexpr int kRegSlotStride = 65536;

std::string Hex(const unsigned char *data, size_t size) {
    static const char kDigits[] = "0123456789abcdef";
    std::string out;
    for (size_t i = 0; i < size; ++i) {
        out += kDigits[data[i] >> 4];
        out += kDigits[data[i] & 0xf];
    }
    return out;
}
}

std::string AccountIdHex(const User &user) {
    for (int slot = 0; slot < kMaxUserSlots; ++slot) {
        char name[32] = {};
        if (sceRegMgrGetStr(kRegUserName + slot * kRegSlotStride, name, sizeof(name)) != 0 || user.name != name) continue;
        unsigned char id[8] = {};
        if (sceRegMgrGetBin(kRegAccountId + slot * kRegSlotStride, id, sizeof(id)) != 0) return "";
        unsigned char number[8];
        for (int i = 0; i < 8; ++i) number[i] = id[7 - i];
        return Hex(number, sizeof(number));
    }
    return "";
}

std::string DeviceIdHex() {
    unsigned char id[16] = {};
    return sceKernelGetOpenPsId(id) == 0 ? Hex(id, sizeof(id)) : "";
}

bool InstallPkg(const std::string &path, std::string *err) {
    std::string visible = path.rfind("/data/", 0) == 0 ? "/user" + path : path;
    char code[64];

    int rc = sceAppInstUtilInitialize();
    if (rc != 0) {
        std::snprintf(code, sizeof(code), "sceAppInstUtilInitialize rc=0x%08x", static_cast<unsigned>(rc));
        if (err) *err = code;
        return false;
    }
    PkgInfo info {};
    rc = sceAppInstUtilAppInstallPkg(visible.c_str(), &info);
    if (rc == 0) return true;

    std::string uri = "file://" + visible;
    PkgMetaInfo meta {};
    meta.uri = uri.c_str();
    meta.ex_uri = "";
    meta.playgo_scenario_id = "";
    meta.content_id = "";
    meta.content_name = "PlayTrack";
    meta.icon_url = "";
    PkgInfo info2 {};

    auto playgo = std::make_unique<PkgPlayGoInfo>();
    int rc2 = sceAppInstUtilInstallByPackage(&meta, &info2, playgo.get());
    if (rc2 == 0) return true;

    std::snprintf(code, sizeof(code), "AppInstallPkg rc=0x%08x, InstallByPackage rc=0x%08x", static_cast<unsigned>(rc),
                  static_cast<unsigned>(rc2));
    if (err) *err = code;
    return false;
}

void Notify(const std::string &message) {
    Spawn([message]() {
        NotifyRequest req{};
        std::snprintf(req.message, sizeof(req.message), "%s", message.c_str());
        sceKernelSendNotificationRequest(0, &req, sizeof(req), 0);
    });
}

}
