#include "response.h"
#include "fields.h"

#include <algorithm>
#include <cctype>
#include <print>
#include <string>

namespace nano_edr {

static bool IsSystemFile(const std::string& path) {
    std::string lower = path;
    std::transform(lower.begin(), lower.end(), lower.begin(), 
                   [](unsigned char c){ return std::tolower(c); });
    
    return lower.starts_with("c:\\windows\\") || 
           lower.starts_with("c:\\program files");
}

ResponsePlanner::ResponsePlanner(const EntityModel* model, std::string quarantine_dir)
    : model_(model), quarantine_dir_(quarantine_dir) {}

ResponsePlan ResponsePlanner::Plan(const Detection& detection) const {
    ResponsePlan plan;  

    const std::vector<std::string> tree = model_->GetProcessTree(detection.pid);
    
    for(const std::string& name : detection.actions){
        if(name == "kill_process"){
            for(auto it = tree.rbegin(); it != tree.rend(); ++it){
                const std::string& pid = *it;
                Action action;
                action.kind = ActionKind::kKillProcess;
                action.pid = pid;

                const ProcessRecord* p = model_->FindProcess(pid);
                action.expected_start = p ? p->start : Timestamp{0};
                
                action.reason = detection.rule;
                if (pid != detection.pid) {
                    action.reason += ", потомок " + detection.pid;
                }
                plan.Add(action);
            }
        } else if(name == "quarantine_source") {
            const FileRecord* source = model_->FindSpawnSource(detection.pid);
            if(source != nullptr){
                std::string to = QuarantinePath(source->path, detection.pid);
                if (!to.empty() && !IsSystemFile(source->path)) {
                    Action action;
                    action.kind = ActionKind::kQuarantineFile;
                    action.path = source->path;
                    action.to = to;
                    action.reason = detection.rule + ", источник запуска";
                    plan.Add(action);
                }
            }
        } else if (name == "quarantine_image") {
            const ProcessRecord* proc = model_->FindProcess(detection.pid);
            if (proc != nullptr) {
                std::string to = QuarantinePath(proc->image, detection.pid);
                if (!to.empty() && !IsSystemFile(proc->image)) {
                    Action action;
                    action.kind = ActionKind::kQuarantineFile;
                    action.path = proc->image;
                    action.to = to;
                    action.reason = detection.rule + ", образ процесса";
                    plan.Add(action);
                }
            }
        } else if (name == "delete_dropped") {
            for (const std::string& pid : tree) {
                const auto files = model_->GetFilesCreatedBy(pid, true); 
                for (const auto& f : files) {
                    if (!IsSystemFile(f)) {
                        Action action;
                        action.kind = ActionKind::kDeleteFile;
                        action.path = f;
                        action.reason = detection.rule + ", принесён " + pid;
                        plan.Add(action);
                    }
                }
            }
        } else if(name == "delete_evidence") {
            for (const auto& ev_ptr : detection.evidence){
                const Event& ev = *ev_ptr;
                const std::string* path_ptr = nullptr;
                
                if(ev.event_type() == EventType::kFileMove){
                    path_ptr = FindField(ev, "to");
                }else {
                    path_ptr = FindField(ev, "path");
                }
                
                if(path_ptr != nullptr && !IsSystemFile(*path_ptr)){
                    Action action;
                    action.kind = ActionKind::kDeleteFile;
                    action.path = *path_ptr;
                    action.reason = detection.rule + ", файл из детекта";
                    plan.Add(action);
                }
            }
        }
    }
    return plan;
}

std::string ResponsePlanner::QuarantinePath(const std::string& path, const std::string& pid) const {
    if(quarantine_dir_.empty()){
        return "";
    }
    size_t pos = path.find_last_of("\\/");
    std::string filename = (pos == std::string::npos) ? path : path.substr(pos + 1);
    return quarantine_dir_ + "\\" + pid + "-" + filename;
}

Responder::Responder(OsHandle* os, bool dry_run) : os_(os), dry_run_(dry_run) {}

void Responder::Execute(const ResponsePlan& plan) {
    for (const Action& action : plan.actions()) {
        ++stats_.planned;
        bool repeat = false;
        for (const std::string& target : done_) {
            if (target == action.target()) { repeat = true; break; }
        }
        if (repeat) { ++stats_.skipped_repeat; continue; }
        done_.push_back(action.target());

        if (dry_run_ || os_ == nullptr) {
            std::print("[PLAN] {}\n", ToString(action));
            continue;
        }
        if (action.kind == ActionKind::kKillProcess) Kill(action);
        else if (action.kind == ActionKind::kQuarantineFile) MoveToQuarantine(action);
        else if (action.kind == ActionKind::kDeleteFile) Delete(action);
    }
}

}