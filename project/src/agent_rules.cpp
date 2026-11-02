#include "agent_rules.h"

#include <memory>
#include <vector>

#include "conditions.h"
#include "rule.h"
#include "rule_engine.h"

namespace nano_edr {

RuleEngine BuildAgentRules() {
    RuleEngine engine;


    {
        auto rule = std::make_unique<MatchRule>("script_host_from_temp",
                                                Severity::kHigh);   

        rule->AddCondition(std::make_unique<EventTypeIs>(
            std::vector<EventType>{EventType::kProcessStart}));

        auto images = std::make_unique<AnyOf>();
        images->Add(std::make_unique<FieldEndsWith>("image", "wscript.exe"));
        images->Add(std::make_unique<FieldEndsWith>("image", "cscript.exe"));
        rule->AddCondition(std::move(images));

        auto temp = std::make_unique<AnyOf>();
        temp->Add(std::make_unique<FieldContains>("cmdline", "\\appdata\\local\\temp\\"));
        temp->Add(std::make_unique<FieldContains>("cmdline", "/appdata/local/temp/"));
        temp->Add(std::make_unique<FieldContains>("cmdline", "\\windows\\temp\\"));
        temp->Add(std::make_unique<FieldContains>("cmdline", "/windows/temp/"));
        temp->Add(std::make_unique<FieldContains>("cmdline", "%temp%"));
        temp->Add(std::make_unique<FieldContains>("cmdline", "%tmp%"));
        rule->AddCondition(std::move(temp));

        rule->AddAction("kill_process");
        engine.AddRule(std::move(rule));
    }

    {
        auto rule = std::make_unique<MatchRule>("lolbin_download",
                                                Severity::kHigh);

        rule->AddCondition(std::make_unique<EventTypeIs>(
            std::vector<EventType>{EventType::kProcessStart}));

        auto images = std::make_unique<AnyOf>();
        images->Add(std::make_unique<FieldEndsWith>("image", "certutil.exe"));
        images->Add(std::make_unique<FieldEndsWith>("image", "bitsadmin.exe"));
        rule->AddCondition(std::move(images));

        auto net = std::make_unique<AnyOf>();
        net->Add(std::make_unique<FieldContains>("cmdline", "urlcache"));
        net->Add(std::make_unique<FieldContains>("cmdline", "transfer"));
        net->Add(std::make_unique<FieldContains>("cmdline", "http:"));
        net->Add(std::make_unique<FieldContains>("cmdline", "https:"));
        rule->AddCondition(std::move(net));

        rule->AddAction("kill_process");
        engine.AddRule(std::move(rule));
    }

    {
        auto rule = std::make_unique<MatchRule>("hidden_powershell",
                                                Severity::kMedium);

        rule->AddCondition(std::make_unique<EventTypeIs>(
            std::vector<EventType>{EventType::kProcessStart}));

        auto images = std::make_unique<AnyOf>();
        images->Add(std::make_unique<FieldEndsWith>("image", "powershell.exe"));
        images->Add(std::make_unique<FieldEndsWith>("image", "pwsh.exe"));
        rule->AddCondition(std::move(images));

        auto flags = std::make_unique<AnyOf>();
        flags->Add(std::make_unique<FieldContains>("cmdline", "-w hidden"));
        flags->Add(std::make_unique<FieldContains>("cmdline", "-windowstyle hidden"));
        flags->Add(std::make_unique<FieldContains>("cmdline", "-enc"));
        flags->Add(std::make_unique<FieldContains>("cmdline", "-encodedcommand"));
        rule->AddCondition(std::move(flags));

        rule->AddAction("kill_process");
        engine.AddRule(std::move(rule));
    }

    {
        auto rule = std::make_unique<MatchRule>("autostart_write",
                                                Severity::kHigh);

        rule->AddCondition(std::make_unique<EventTypeIs>(
            std::vector<EventType>{EventType::kFileCreate,
                                   EventType::kFileWrite,
                                   EventType::kFileMove}));

        auto paths = std::make_unique<AnyOf>();
        paths->Add(std::make_unique<FieldContains>("path", "\\start menu\\programs\\startup\\"));
        paths->Add(std::make_unique<FieldContains>("path", "/start menu/programs/startup/"));
        paths->Add(std::make_unique<FieldContains>("to",   "\\start menu\\programs\\startup\\"));
        paths->Add(std::make_unique<FieldContains>("to",   "/start menu/programs/startup/"));
        rule->AddCondition(std::move(paths));

        rule->AddAction("quarantine_file");
        engine.AddRule(std::move(rule));
    }

    {
        auto rule = std::make_unique<MatchRule>("ransom_extension",
                                                Severity::kCritical);

        rule->AddCondition(std::make_unique<EventTypeIs>(
            std::vector<EventType>{EventType::kFileCreate,
                                   EventType::kFileWrite,
                                   EventType::kFileMove}));

        auto paths = std::make_unique<AnyOf>();
        paths->Add(std::make_unique<FieldEndsWith>("path", ".locked"));
        paths->Add(std::make_unique<FieldEndsWith>("to",   ".locked"));
        rule->AddCondition(std::move(paths));

        rule->AddAction("quarantine_file");
        engine.AddRule(std::move(rule));
    }

    {
        auto beacon = std::make_unique<SequenceRule>("script_host_beacon",
                                                     Severity::kCritical, 10000);

        auto start = std::make_unique<AllOf>();
        start->Add(std::make_unique<EventTypeIs>(
            std::vector<EventType>{EventType::kProcessStart}));

        auto images = std::make_unique<AnyOf>();
        images->Add(std::make_unique<FieldEndsWith>("image", "wscript.exe"));
        images->Add(std::make_unique<FieldEndsWith>("image", "cscript.exe"));
        start->Add(std::move(images));

        auto temp = std::make_unique<AnyOf>();
        temp->Add(std::make_unique<FieldContains>("cmdline", "\\appdata\\local\\temp\\"));
        temp->Add(std::make_unique<FieldContains>("cmdline", "/appdata/local/temp/"));
        temp->Add(std::make_unique<FieldContains>("cmdline", "\\windows\\temp\\"));
        temp->Add(std::make_unique<FieldContains>("cmdline", "/windows/temp/"));
        temp->Add(std::make_unique<FieldContains>("cmdline", "%temp%"));
        temp->Add(std::make_unique<FieldContains>("cmdline", "%tmp%"));
        start->Add(std::move(temp));

        beacon->SetFirst(std::move(start));
        beacon->SetSecond(std::make_unique<EventTypeIs>(
            std::vector<EventType>{EventType::kNetConnect}));

        beacon->AddAction("kill_process");
        engine.AddRule(std::move(beacon));
    }

    return engine;
}




}

