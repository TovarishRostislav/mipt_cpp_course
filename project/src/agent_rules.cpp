#include "agent_rules.h"
#include "fields.h"
#include "rules.h"

#include <string>

namespace nano_edr {
namespace{
const std::string* GetFilePath(const Event& event) {
    if (event.type() == "file_move") {
        return FindField(event, "to");
    }
    return FindField(event, "path");
}
}


bool ScriptHostFromTemp(const Event& event) {
    if (!IsProcessStart(event)) return false;

    std::string image = NormalizePath(GetRequiredField(event, "image"));
    bool is_script_host = image.ends_with("wscript.exe") || image.ends_with("cscript.exe");
    if (!is_script_host) return false;

    if (!CommandLineContains(event, "\\appdata\\local\\temp\\") &&
        !CommandLineContains(event, "\\windows\\temp\\")) {
        return false;
    }

    return true;
}
bool LolbinDownload(const Event& event) {
    if (!IsProcessStart(event)) return false;

    std::string image = NormalizePath(GetRequiredField(event,"image"));
    bool is_lolbin = image.ends_with("certutil.exe") || image.ends_with("bitsadmin.exe");
    if (!is_lolbin) return false;

    return CommandLineContains(event, "urlcache") ||
           CommandLineContains(event, "transfer") ||
           CommandLineContains(event, "http:") ||
           CommandLineContains(event, "https:");
}
bool HiddenPowershell(const Event& event) {
     if (!IsProcessStart(event)) return false;

     std::string image = NormalizePath(GetRequiredField(event, "image"));
     bool is_powershell = image.ends_with("powershell.exe") || image.ends_with("pwsh.exe");

     if (!is_powershell) return false;

     return CommandLineContains(event, "-w hidden") ||
           CommandLineContains(event, "-windowstyle hidden") ||
           CommandLineContains(event, "-enc") ||
           CommandLineContains(event, "-encodedcommand");
}
bool AutostartWrite(const Event& event) {
    if(!IsFileWrite(event)) return false;

    const std::string* p = GetFilePath(event);
    if(!p) return false;    
    std::string normalized = NormalizePath(*p);
    return normalized.find("\\start menu\\programs\\startup\\") != std::string::npos;

}
bool RansomExtension(const Event& event) {
    if(!IsFileWrite(event)) return false;

    const std::string* p = GetFilePath(event);
    if(!p) return false;
    std::string normalized = NormalizePath(*p);

    const std::string suffix = ".locked";

    return normalized.ends_with(suffix);
}

constexpr Rule kRules[] = {
    {"script_host_from_temp", ScriptHostFromTemp, Severity::kHigh},
    {"lolbin_download",       LolbinDownload,     Severity::kHigh},
    {"hidden_powershell",     HiddenPowershell,   Severity::kMedium},
    {"autostart_write",       AutostartWrite,     Severity::kHigh},
    {"ransom_extension",      RansomExtension,    Severity::kCritical},
};


const Rule* AgentRules() {
    return kRules;
}
size_t AgentRuleCount() {
     return std::size(kRules);
}

} 